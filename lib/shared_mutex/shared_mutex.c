#include "shared_mutex.h"

#if LIB_USE_SHARED_MUTEX

#define LOCAL_LOG_PRINT_ENABLE 0

#if LOCAL_LOG_PRINT_ENABLE
#warning LOCAL_LOG_PRINT_ENABLE
#define LOCAL_LOG_PRINT		  LOG_DEBUG
#define LOCAL_LOG_COLOR_PRINT LOG_COLOR
#else /* LOCAL_LOG_PRINT_ENABLE */
#define LOCAL_LOG_PRINT(_f_, ...)
#define LOCAL_LOG_COLOR_PRINT(c, _f_, ...)
#endif /* LOCAL_LOG_PRINT_ENABLE */

void SharedMutex_Init(SharedMutex_t* pAccess, void* pEvent,
					  SharedMutex_WaitEvent_Fptr_t fpWaitEvent,
					  SharedMutex_SignalEvent_Fptr_t fpSignalEvent) {
	*pAccess = (SharedMutex_t){
		.LockType		   = SHARED_MUTEX_LOCK_NO,
		.pEvent			   = pEvent,
		.WaitEvent		   = fpWaitEvent,
		.SignalEvent	   = fpSignalEvent,
		.ConcurrentReaders = 0,
		.RejectNewReaders  = false,
		.LockKey		   = 0,
	};
}

RET_STATE_t SharedMutex_SharedLock(SharedMutex_t* pAccess, u32 waitMs) {
	if (!pAccess) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	RET_STATE_t lockResult = RET_STATE_UNDEF;

	SYS_CRITICAL_ON();
	SHARED_MUTEX_LOCK_TYPE_t lockType = pAccess->LockType;
	bool rejected					  = pAccess->RejectNewReaders;
	SYS_CRITICAL_OFF();

	if (rejected) {
		LOCAL_LOG_COLOR_PRINT(TERM_COLOR_YELLOW, "New readers rejected!");
		return RET_STATE_ERR_BUSY;
	}

	switch (lockType) {
		case SHARED_MUTEX_LOCK_EXCLUSIVE: {
			if (!pAccess->WaitEvent) {
				lockResult = RET_STATE_ERR_BUSY;
				break;
			}

			bool eventReceived = pAccess->WaitEvent(pAccess->pEvent, waitMs);
			if (!eventReceived) {
				lockResult = RET_STATE_ERR_BUSY;
				LOCAL_LOG_COLOR_PRINT(TERM_COLOR_YELLOW, "Exclusive lock await timeout");
				break;
			}

			SYS_CRITICAL_ON();
			if (pAccess->LockType == SHARED_MUTEX_LOCK_EXCLUSIVE || pAccess->RejectNewReaders) {
				SYS_CRITICAL_OFF();
				lockResult = RET_STATE_ERR_BUSY;
				break;
			}

			if (pAccess->LockType == SHARED_MUTEX_LOCK_NO) {
				pAccess->LockType		   = SHARED_MUTEX_LOCK_SHARED;
				pAccess->ConcurrentReaders = 1;
				lockResult				   = RET_STATE_SUCCESS;
			} else {
				pAccess->ConcurrentReaders++;
				lockResult = RET_STATE_SUCCESS;
			}
			SYS_CRITICAL_OFF();
			break;
		}

		case SHARED_MUTEX_LOCK_NO:
			SYS_CRITICAL_ON();
			pAccess->LockType		   = SHARED_MUTEX_LOCK_SHARED;
			pAccess->ConcurrentReaders = 1;
			lockResult				   = RET_STATE_SUCCESS;
			SYS_CRITICAL_OFF();
			break;

		case SHARED_MUTEX_LOCK_SHARED:
			SYS_CRITICAL_ON();
			pAccess->ConcurrentReaders++;
			lockResult = RET_STATE_SUCCESS;
			SYS_CRITICAL_OFF();
			break;

		default:
			lockResult = RET_STATE_ERR_PARAM;
	}

	LOCAL_LOG_COLOR_PRINT(TERM_COLOR_GREEN, "SharedLock status: %s, concurrent readers: %d",
						  RetState_GetStr(lockResult), pAccess->ConcurrentReaders);
	return lockResult;
}

RET_STATE_t SharedMutex_SharedUnlock(SharedMutex_t* pAccess) {
	if (!pAccess) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_ON();

	RET_STATE_t unlockResult = RET_STATE_UNDEF;
	switch (pAccess->LockType) {
		case SHARED_MUTEX_LOCK_NO:
			unlockResult = RET_STATE_SUCCESS;
			break;

		case SHARED_MUTEX_LOCK_SHARED:
			if (pAccess->ConcurrentReaders == 0) {
				LOCAL_LOG_COLOR_PRINT(TERM_COLOR_RED, "ConcurrentReaders is zero!");
				PANIC();
			}
			if (--pAccess->ConcurrentReaders == 0)
				pAccess->LockType = SHARED_MUTEX_LOCK_NO;

			unlockResult = RET_STATE_SUCCESS;
			break;

		case SHARED_MUTEX_LOCK_EXCLUSIVE:
			unlockResult = RET_STATE_ERR_BUSY;
			break;

		default:
			unlockResult = RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_OFF();

	if (unlockResult == RET_STATE_SUCCESS && pAccess->SignalEvent)
		pAccess->SignalEvent(pAccess->pEvent);

	LOCAL_LOG_COLOR_PRINT(TERM_COLOR_GREEN, "SharedUnlock status: %s, concurrent readers: %d",
						  RetState_GetStr(unlockResult), pAccess->ConcurrentReaders);
	return unlockResult;
}

u32 SharedMutex_GetConcurrentReaders(SharedMutex_t* pAccess) {
	if (!pAccess) {
		PANIC();
		return 0;
	}

	return pAccess->ConcurrentReaders;
}

RET_STATE_t SharedMutex_ExclusiveLock(SharedMutex_t* pAccess, u32 waitMs, u32* pLockKey) {
	if (!pAccess) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	/**
	 * If SharedMutex already locked exclusively, wait for release
	 */
	if (pAccess->LockType == SHARED_MUTEX_LOCK_EXCLUSIVE) {
		if (!pAccess->WaitEvent) {
			return RET_STATE_ERR_BUSY;
		}

		bool eventReceived = pAccess->WaitEvent(pAccess->pEvent, waitMs);
		if (!eventReceived) {
			LOCAL_LOG_COLOR_PRINT(TERM_COLOR_YELLOW, "ExclusiveLock operation rejected: is locked");
			return RET_STATE_ERR_BUSY;
		}

		if (pAccess->LockType == SHARED_MUTEX_LOCK_EXCLUSIVE) {
			return RET_STATE_ERR_BUSY;
		}
	}

	/**
	 * Firstly acquire shared lock to prevent data modifications
	 */
	RET_STATE_t sharedLockRes = SharedMutex_SharedLock(pAccess, waitMs);
	RETURN_IF_UNSUCCESS(sharedLockRes);

	pAccess->RejectNewReaders = true;

	/**
	 * Wait for all other shared readers to finish
	 */
	while (pAccess->ConcurrentReaders != 1) {
		if (!pAccess->WaitEvent) {
			SharedMutex_SharedUnlock(pAccess);
			pAccess->RejectNewReaders = false;
			return RET_STATE_ERR_BUSY;
		}

		bool eventReceived = pAccess->WaitEvent(pAccess->pEvent, waitMs);
		if (!eventReceived) {
			SharedMutex_SharedUnlock(pAccess);
			pAccess->RejectNewReaders = false;

			if (pAccess->SignalEvent)
				pAccess->SignalEvent(pAccess->pEvent);

			LOCAL_LOG_COLOR_PRINT(TERM_COLOR_YELLOW, "Await old readers timeout");
			return RET_STATE_ERR_BUSY;
		}
	}

	SYS_CRITICAL_ON();
	pAccess->RejectNewReaders = false;
	pAccess->LockType		  = SHARED_MUTEX_LOCK_EXCLUSIVE;
	while (!pAccess->LockKey)
		pAccess->LockKey = EMBLIB_RAND_GET();

	if (pLockKey)
		*pLockKey = pAccess->LockKey;

	SYS_CRITICAL_OFF();
	LOCAL_LOG_COLOR_PRINT(TERM_COLOR_GREEN, "ExclusiveLock successful, lockID: %8x",
						  pAccess->LockKey);

	return RET_STATE_SUCCESS;
}

RET_STATE_t SharedMutex_ExclusiveUnlock(SharedMutex_t* pAccess) {
	if (!pAccess) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_ON();

	RET_STATE_t unlockResult = RET_STATE_UNDEF;
	switch (pAccess->LockType) {
		case SHARED_MUTEX_LOCK_NO:
			unlockResult = RET_STATE_SUCCESS;
			break;

		case SHARED_MUTEX_LOCK_SHARED:
			unlockResult = RET_STATE_ERR_BUSY;
			break;

		case SHARED_MUTEX_LOCK_EXCLUSIVE:
			if (pAccess->ConcurrentReaders != 1) {
				LOCAL_LOG_COLOR_PRINT(TERM_COLOR_RED, "ConcurrentReaders is not 1!");
				PANIC();
			}
			pAccess->ConcurrentReaders--;
			pAccess->LockType = SHARED_MUTEX_LOCK_NO;
			pAccess->LockKey  = 0;

			unlockResult = RET_STATE_SUCCESS;
			break;

		default:
			unlockResult = RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_OFF();

	if (unlockResult == RET_STATE_SUCCESS && pAccess->SignalEvent)
		pAccess->SignalEvent(pAccess->pEvent);

	LOCAL_LOG_COLOR_PRINT(TERM_COLOR_GREEN, "ExclusiveUnlock status: %s, concurrent readers: %d",
						  RetState_GetStr(unlockResult), pAccess->ConcurrentReaders);
	return unlockResult;
}

bool SharedMutex_IsLocked(SharedMutex_t* pAccess, u32 extLockKey) {
	if (!pAccess) {
		PANIC();
		return false;
	}

	SYS_CRITICAL_ON();

	bool isLocked = false;
	if (pAccess->LockType != SHARED_MUTEX_LOCK_NO) {
		isLocked = true;

		if (pAccess->LockType == SHARED_MUTEX_LOCK_EXCLUSIVE && pAccess->LockKey == extLockKey) {
			isLocked = false;
			LOCAL_LOG_COLOR_PRINT(TERM_COLOR_GREEN, "External lock key the same with internal: %8x",
								  pAccess->LockKey);
		}
	}

	SYS_CRITICAL_OFF();
	LOCAL_LOG_COLOR_PRINT(TERM_COLOR_GREEN, "Is locked: %d, lock type: %d", isLocked,
						  pAccess->LockType);
	return isLocked;
}

#endif /* LIB_USE_SHARED_MUTEX */

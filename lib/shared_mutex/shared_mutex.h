#ifndef __SHARED_MUTEX_H
#define __SHARED_MUTEX_H

#include "def_sys.h"
#include "def_types.h"
#include "lib_cfg.h"

#if LIB_USE_SHARED_MUTEX

#include "rand.h"
#define EMBLIB_RAND_GET() Rand_GetNum()

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	SHARED_MUTEX_LOCK_NO		= 0x00,
	SHARED_MUTEX_LOCK_SHARED	= 0x01,
	SHARED_MUTEX_LOCK_EXCLUSIVE = 0x02
} SHARED_MUTEX_LOCK_TYPE_t;

/**
 * @brief Blocking wait for synchronization event with timeout
 *
 * @param[in] pEvent pointer to synchronization object (semaphore, event group, etc.)
 * @param[in] timeoutMs maximum wait time in milliseconds
 * @retval true event received
 * @retval false timeout expired
 */
typedef bool (*SharedMutex_WaitEvent_Fptr_t)(void* pEvent, u32 timeoutMs);

/**
 * @brief Signal synchronization event (wake up waiting threads)
 *
 * @param[in] pEvent pointer to synchronization object
 */
typedef void (*SharedMutex_SignalEvent_Fptr_t)(void* pEvent);

typedef struct {
	SHARED_MUTEX_LOCK_TYPE_t LockType;
	void* pEvent;
	SharedMutex_WaitEvent_Fptr_t WaitEvent;
	SharedMutex_SignalEvent_Fptr_t SignalEvent;
	u32 ConcurrentReaders;
	bool RejectNewReaders;
	u32 LockKey;
} SharedMutex_t;

/**
 * @brief Initialize SharedMutex object
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @param[in] pEvent pointer to synchronization object (semaphore, event group, etc.),
 * can be NULL for non-blocking mode
 * @param[in] fpWaitEvent blocking wait callback, can be NULL
 * @param[in] fpSignalEvent signal/notify callback, can be NULL
 *
 * @note fpWaitEvent and fpSignalEvent must be both NULL or both non-NULL
 */
void SharedMutex_Init(SharedMutex_t* pAccess, void* pEvent,
					  SharedMutex_WaitEvent_Fptr_t fpWaitEvent,
					  SharedMutex_SignalEvent_Fptr_t fpSignalEvent);

/**
 * @brief Acquire shared lock (multiple readers allowed, writers blocked)
 *
 * If the mutex is in LOCK_EXCLUSIVE state, waits for release via WaitEvent.
 * If WaitEvent is NULL and mutex is busy, returns ERR_BUSY immediately.
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @param[in] waitMs timeout in milliseconds for WaitEvent
 * @retval RET_STATE_ERR_PARAM pAccess is NULL
 * @retval RET_STATE_ERR_BUSY mutex locked exclusively or new readers rejected or timeout
 * @retval RET_STATE_SUCCESS shared lock acquired
 */
RET_STATE_t SharedMutex_SharedLock(SharedMutex_t* pAccess, u32 waitMs);

/**
 * @brief Release shared lock
 *
 * Decrements ConcurrentReaders counter. When it reaches zero, transitions
 * to LOCK_NO state and signals waiting threads via SignalEvent.
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @retval RET_STATE_ERR_PARAM pAccess is NULL
 * @retval RET_STATE_ERR_BUSY mutex is in exclusive lock state
 * @retval RET_STATE_SUCCESS shared lock released
 */
RET_STATE_t SharedMutex_SharedUnlock(SharedMutex_t* pAccess);

/**
 * @brief Get current number of concurrent readers
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @return concurrent readers counter (0 if pAccess is NULL)
 */
u32 SharedMutex_GetConcurrentReaders(SharedMutex_t* pAccess);

/**
 * @brief Acquire exclusive lock (full lock, no readers or writers allowed)
 *
 * Internally acquires shared lock first, then sets RejectNewReaders flag and
 * waits for all other readers to release. On success, generates a random
 * LockKey for lock holder identification.
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @param[in] waitMs timeout in milliseconds for WaitEvent
 * @param[out] pLockKey (optional) receives lock key for private operations bypass
 * @retval RET_STATE_ERR_PARAM pAccess is NULL
 * @retval RET_STATE_ERR_BUSY mutex already exclusively locked or readers didn't
 * release within timeout
 * @retval RET_STATE_SUCCESS exclusive lock acquired
 */
RET_STATE_t SharedMutex_ExclusiveLock(SharedMutex_t* pAccess, u32 waitMs, u32* pLockKey);

/**
 * @brief Release exclusive lock
 *
 * Clears LockKey and transitions to LOCK_NO state.
 * Signals waiting threads via SignalEvent.
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @retval RET_STATE_ERR_PARAM pAccess is NULL
 * @retval RET_STATE_ERR_BUSY mutex is in shared lock state (wrong unlock type)
 * @retval RET_STATE_SUCCESS exclusive lock released
 */
RET_STATE_t SharedMutex_ExclusiveUnlock(SharedMutex_t* pAccess);

/**
 * @brief Check if SharedMutex is locked
 *
 * Returns true if mutex is in any locked state. Exception: if mutex is in
 * LOCK_EXCLUSIVE state and extLockKey matches the internal LockKey, returns
 * false (lock holder can pass through).
 *
 * @param[in] pAccess pointer to SharedMutex object
 * @param[in] extLockKey lock key to validate against internal LockKey
 * @retval true mutex is locked
 * @retval false mutex is unlocked or extLockKey matches lock holder
 */
bool SharedMutex_IsLocked(SharedMutex_t* pAccess, u32 extLockKey);

#endif /* LIB_USE_SHARED_MUTEX */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SHARED_MUTEX_H */

#include "ring_list.h"

#if LIB_USE_RING_LIST || LIB_USE_RING_LIST_WITH_SHARED_MUTEX

#define LOCAL_DEBUG_PRINT_ENABLE 0

#if LOCAL_DEBUG_PRINT_ENABLE
#warning LOCAL_DEBUG_PRINT_ENABLE
#define LOCAL_DEBUG_PRINT EMBLIB_DEBUG_LOG_PRINT
#else /* LOCAL_DEBUG_PRINT_ENABLE */
#define LOCAL_DEBUG_PRINT(_f_, ...)
#endif /* LOCAL_DEBUG_PRINT_ENABLE */

/**
 * Example of RingList structure
 * Capacity: 6
 *
 * data   |8|9|4|5|6|7|	4 - oldest sample (will be overwritten in InsertCell()), 5 - oldest sample which you can read through PeekCell(), 9 - newest data sample
 * type   | |L|H|T| | |	L - the latest data sample , H - Head, T - Tail (the oldest sample)
 * idx_r  |2|3|4|0|1|5|	In ringList_t Tail (T) every time have index 0, Head (H) index = Capacity - 1, Latest value (L) index = Capacity - 2
 * idx_l  |0|1|2|3|4|5|	It's linear buffer indexing
 *
 * Head - place to put new data, so before this moment newest data will be at cell with index (Head - 1) == (Capacity - 2)
 */

static void cut_idx_by_cap(RingList_t* pRingList, u16* pIdx) {
	if (*pIdx >= pRingList->Capacity)
		*pIdx -= pRingList->Capacity;
}

/**
 * @brief RingList initialization
 *
 * @param pRingList pointer to the static RingList variable
 * @param pLineList line buffer for RingList data keep
 * @param cellSize size of separate RingList cell
 * @param listSize max number of cells in RingList
 * @return RET_STATE_t [RET_STATE_ERR_PARAM, ]
 */
RET_STATE_t RingList_Init(RingList_t* pRingList, void* pLineList, u16 cellSize, u16 listSize) {
	if (!pRingList || !pLineList || cellSize == 0 || listSize < RING_LIST_MIN_ELEMENTS_NUM) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_ON();

	pRingList->List		= pLineList;
	pRingList->Capacity = listSize;
	pRingList->CellSize = cellSize;

	pRingList->IdxHead	 = 0;
	pRingList->IdxTail	 = 0;
	pRingList->InsertCnt = 0;

#ifdef LIB_USE_RING_LIST_WITH_SHARED_MUTEX
	SharedMutex_Init(&pRingList->SharedMutex, NULL, NULL, NULL);
#endif /* LIB_USE_RING_LIST_WITH_SHARED_MUTEX */

	SYS_CRITICAL_OFF();
	return RET_STATE_SUCCESS;
}

void* RingList_GetLineList(RingList_t* pRingList) {
	return pRingList->List;
}

u16 RingList_GetCellSize(RingList_t* pRingList) {
	return pRingList->CellSize;
}

u16 RingList_GetCapacity(RingList_t* pRingList) {
	return pRingList->Capacity;
}

u16 RingList_GetInsertCnt(RingList_t* pRingList) {
	return pRingList->InsertCnt;
}

/**
 * @brief Get number of existing elements in RingList
 * There are possible to insert (Capacity - 1) elements
 * number in RingList
 *
 * @param pRingList pointer to the main instance
 * @retval u16 number of ready-to-read elements
 */
u16 RingList_GetExistingElementsNum(RingList_t* pRingList) {
	SYS_CRITICAL_ON();
	u16 elNum = pRingList->InsertCnt >= pRingList->Capacity - 1 ? pRingList->Capacity - 1
																: pRingList->InsertCnt;
	SYS_CRITICAL_OFF();

	return elNum;
}

/**
 * @brief Get latest ring index which were inserted
 * into RingList
 *
 * @param pRingList pointer to the main instance
 * @return u16 latest rind index
 */
u16 RingList_GetLatestRingIdx(RingList_t* pRingList) {
	SYS_CRITICAL_ON();
	if (pRingList->InsertCnt == 0) {
		SYS_CRITICAL_OFF();
		return 0;
	}

	u16 latestIdx	 = pRingList->InsertCnt - 1;
	u16 realCapacity = pRingList->Capacity - 2;
	if (latestIdx >= realCapacity)
		latestIdx = realCapacity;
	SYS_CRITICAL_OFF();

	return latestIdx;
}

/**
 * @brief Check if RingList is full
 *
 * @param pRingList pointer to the main instance
 * @return [true, false] is the RingList is full or not
 */
bool RingList_IsFull(RingList_t* pRingList) {
	SYS_CRITICAL_ON();
	bool isFull = (bool)(pRingList->InsertCnt >= pRingList->Capacity);
	SYS_CRITICAL_OFF();

	return isFull;
}

/**
 * @brief Flush ring list to initial state
 *
 * @param pRingList pointer to the main instance
 * @retval RET_STATE_ERR_BUSY ring list locked
 * @retval RET_STATE_SUCCESS ring list cleared
 */
#ifdef LIB_USE_RING_LIST_WITH_SHARED_MUTEX
RET_STATE_t RingList_Flush(RingList_t* pRingList, u32 lockKey) {
	if (SharedMutex_IsLocked(&pRingList->SharedMutex, lockKey))
		return RET_STATE_ERR_BUSY;
#else
RET_STATE_t RingList_Flush(RingList_t* pRingList) {
#endif /* LIB_USE_RING_LIST_WITH_SHARED_MUTEX */
	if (!pRingList) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_ON();
	pRingList->IdxTail = pRingList->IdxHead = 0;
	pRingList->InsertCnt					= 0;
	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

/**
 * @brief Put new element on the head position of RingList
 *
 * @param pRingList pointer to the main instance
 * @param lockKey function call id for atomic access to ringList
 * @param pCell pointer to the external data needs to be copied
 * @return RET_STATE_t [RET_STATE_ERR_PARAM, RET_STATE_SUCCESS]
 */
#ifdef LIB_USE_RING_LIST_WITH_SHARED_MUTEX
RET_STATE_t RingList_InsertCell(RingList_t* pRingList, void* pCell, u32 lockKey) {
#else
RET_STATE_t RingList_InsertCell(RingList_t* pRingList, void* pCell) {
#endif /* LIB_USE_RING_LIST_WITH_SHARED_MUTEX */
	if (!pRingList || !pCell) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

#ifdef LIB_USE_RING_LIST_WITH_SHARED_MUTEX
	if (SharedMutex_IsLocked(&pRingList->SharedMutex, lockKey))
		return RET_STATE_ERR_BUSY;
#endif /* LIB_USE_RING_LIST_WITH_SHARED_MUTEX */

	SYS_CRITICAL_ON();

	u8* pTmp = (u8*)pRingList->List;
	pTmp	 = &pTmp[pRingList->IdxHead * pRingList->CellSize];
	memcpy((void*)pTmp, pCell, pRingList->CellSize);

	pRingList->InsertCnt++;
	pRingList->IdxHead++;
	cut_idx_by_cap(pRingList, &pRingList->IdxHead);

	if (pRingList->IdxHead == pRingList->IdxTail) {
		pRingList->IdxTail++;
		cut_idx_by_cap(pRingList, &pRingList->IdxTail);
	}

	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

/**
 * @brief Get element (copy) from RingList by it ring index
 *
 * @param pRingList pointer to the main instance
 * @param pCell pointer to data buff where data needs to be copied
 * @param ringIdx ring index of data needs to be copied
 * @return RET_STATE_t [RET_STATE_ERR_PARAM, RET_STATE_ERR_EMPTY, RET_STATE_SUCCESS]
 */
RET_STATE_t RingList_PeekCell(RingList_t* pRingList, void* pCell, u16 ringIdx) {
	if (!pRingList || !pCell) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_ON();
	if (pRingList->InsertCnt == 0) {
		SYS_CRITICAL_OFF();
		return RET_STATE_ERR_EMPTY;
	}

	if (ringIdx >= pRingList->InsertCnt)
		ringIdx = pRingList->InsertCnt - 1;

	u16 tmpLinIdx = pRingList->IdxTail + ringIdx;
	cut_idx_by_cap(pRingList, &tmpLinIdx);

	u8* pTmpList = (u8*)pRingList->List;
	pTmpList	 = &pTmpList[tmpLinIdx * pRingList->CellSize];
	memcpy(pCell, (void*)pTmpList, pRingList->CellSize);

	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingList_CopyToLineBuff(RingList_t* pRingList, void* pLineList, u16 listSize) {
	if (!pRingList || !pLineList) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	SYS_CRITICAL_ON();
	if (pRingList->InsertCnt == 0) {
		SYS_CRITICAL_OFF();
		return RET_STATE_ERR_EMPTY;
	}

	u8* pLineListStart = (u8*)pRingList->List;
	for (u16 listNum = 0; listNum < listSize; listNum++) {
		u16 tmpLinIdx = pRingList->IdxTail + listNum;
		cut_idx_by_cap(pRingList, &tmpLinIdx);

		u8* pTmpList = &pLineListStart[tmpLinIdx * pRingList->CellSize];
		memcpy(pLineList + (listNum * pRingList->CellSize), (void*)pTmpList, pRingList->CellSize);
	}

	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

#endif /* LIB_USE_RING_LIST || LIB_USE_RING_LIST_WITH_SHARED_MUTEX */

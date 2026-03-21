#include "ring_list.h"

/**
 * Example of RingList structure
 * Capacity: 6
 *
 * data   |8|9|4|5|6|7|	4 - oldest sample (will be overwritten in InsertCell()), 5 - oldest sample which you can read through ExtractCell(), 9 - newest data sample
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
 * @param lineList line buffer for RingList data keep
 * @param cellSize size of separate RingList cell 
 * @param listSize max number of cells in RingList 
 * @return RET_STATE_t [RET_STATE_ERR_PARAM, ]
 */
RET_STATE_t RingList_Init(RingList_t* pRingList, void* lineList, u16 cellSize, u16 listSize) {
	if (!pRingList || !lineList || cellSize == 0 || listSize < RING_LIST_MIN_ELEMENTS_NUM)
		EXIT(RET_STATE_ERR_PARAM);

	SYS_CRITICAL_ON();
	pRingList->List		= lineList;
	pRingList->Capacity = listSize;
	pRingList->CellSize = cellSize;

	pRingList->IdxHead	 = 0;
	pRingList->IdxTail	 = 0;
	pRingList->InsertCnt = 0;
	pRingList->MutexID	 = RING_LIST_AVOID_MUTEX;
	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

/**
 * @brief Get line buffer of RingList 
 * 
 * @param pRingList pointer to the main instance
 * @return void* addr of line buffer
 */
void* RingList_GetLineList(RingList_t* pRingList) {
	return pRingList->List;
}

/**
 * @brief Get RingList cell size 
 * 
 * @param pRingList pointer to the main instance
 * @return u16 size of cell in bytes
 */
u16 RingList_GetCellSize(RingList_t* pRingList) {
	return pRingList->CellSize;
}

/**
 * @brief  Get RingList cells number
 * 
 * @param pRingList pointer to the main instance
 * @return u16 RingList capacity (cells number)
 */
u16 RingList_GetCapacity(RingList_t* pRingList) {
	return pRingList->Capacity;
}

/**
 * @brief Get total numer of data insertions in RingList
 * 
 * @param pRingList pointer to the main instance
 * @return u16 total numer of data insertions
 */
u16 RingList_GetInsertCnt(RingList_t* pRingList) {
	return pRingList->InsertCnt;
}

/**
 * @brief Get number of existing elements in RingList
 * 
 * @param pRingList pointer to the main instance
 * @return u16 number of existing elements
 */
u16 RingList_GetExistingElementsNum(RingList_t* pRingList) {
	/**
	 * There are possible to insert (Capacity - 1) elements
	 * number in RingList
	 */
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

	u16 latestIdx	  = pRingList->InsertCnt - 1;
	u16 savedElements = pRingList->Capacity - 2;
	if (latestIdx >= savedElements)
		latestIdx = savedElements;
	SYS_CRITICAL_OFF();

	return latestIdx;
}

void RingList_GetHead(RingList_t* pRingList, void* pHead) {
	u8* tmpListPtr = (u8*)pRingList->List;
	tmpListPtr	   = &tmpListPtr[pRingList->IdxHead * pRingList->CellSize];
	memcpy(pHead, (void*)tmpListPtr, pRingList->CellSize);
}

void RingList_GetTail(RingList_t* pRingList, void* pTail) {
	u8* tmpListPtr = (u8*)pRingList->List;
	tmpListPtr	   = &tmpListPtr[pRingList->IdxTail * pRingList->CellSize];
	memcpy(pTail, (void*)tmpListPtr, pRingList->CellSize);
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
 * @brief Lock RingList from some operations 
 * 
 * @param pRingList pointer to the main instance 
 * @param mutID function call id for atomic access to ringList
 * @return [true, false] if lock was success or not
 */
bool RingList_Lock(RingList_t* pRingList, u32 mutID) {
	SYS_CRITICAL_ON();
	bool res = (bool)(pRingList->MutexID == RING_LIST_AVOID_MUTEX);
	if (res)
		pRingList->MutexID = mutID;
	SYS_CRITICAL_OFF();

	return res;
}

/**
 * @brief Unlock RingList for some operations
 * 
 * @param pRingList pointer to the main instance 
 * @param mutID function call id for atomic access to ringList
 * @return [true, false] if unlock was success or not
 */
bool RingList_Unlock(RingList_t* pRingList, u32 mutID) {
	SYS_CRITICAL_ON();
	bool res = (bool)(pRingList->MutexID == mutID);
	if (res)
		pRingList->MutexID = RING_LIST_AVOID_MUTEX;
	SYS_CRITICAL_OFF();

	return res;
}

/**
 * @brief Check if RingList is locked by known ID
 * 
 * @param pRingList pointer to the main instance 
 * @param mutID function call id for atomic access to ringList
 * @return [true, false]
 */
static bool RingList_IsLockedByID(RingList_t* pRingList, u32 mutID) {
	SYS_CRITICAL_ON();
	bool isLocked = (bool)(pRingList->MutexID == mutID);
	SYS_CRITICAL_OFF();

	return isLocked;
}

static bool RingList_IsLockedByID_FromISR(RingList_t* pRingList, u32 mutID) {
	UBaseType_t uxSavedInterruptStatus = SYS_CRITICAL_ON_ISR();
	bool isLocked					   = (bool)(pRingList->MutexID == mutID);
	SYS_CRITICAL_OFF_ISR(uxSavedInterruptStatus);

	return isLocked;
}

/**
 * @brief Flush ring list to initial state
 * 
 * @param pRingList pointer to the main instance 
 */
RET_STATE_t RingList_Flush(RingList_t* pRingList, u32 mutID) {
	/**
	 * If blocked by another ID
	 */
	if (!RingList_IsLockedByID(pRingList, mutID))
		return RET_STATE_ERR_BUSY;

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
 * @param mutID function call id for atomic access to ringList
 * @param pCell pointer to the external data needs to be copied
 * @return RET_STATE_t [RET_STATE_ERR_PARAM, RET_STATE_SUCCESS]
 */
static RET_STATE_t RingList_InsertCell_Universal(RingList_t* pRingList, u32 mutID, void* pCell,
												 bool isFromISR) {
	if (!pRingList || !pCell)
		EXIT(RET_STATE_ERR_PARAM);

	/**
	 * If blocked by another ID
	 */

	UBaseType_t uxSavedInterruptStatus;

	if (isFromISR) {
		if (!RingList_IsLockedByID_FromISR(pRingList, mutID))
			return RET_STATE_ERR_BUSY;
		uxSavedInterruptStatus = SYS_CRITICAL_ON_ISR();
	} else {
		if (!RingList_IsLockedByID(pRingList, mutID))
			return RET_STATE_ERR_BUSY;
		SYS_CRITICAL_ON();
	}

	u8* tmpListPtr = (u8*)pRingList->List;
	tmpListPtr	   = &tmpListPtr[pRingList->IdxHead * pRingList->CellSize];
	memcpy((void*)tmpListPtr, pCell, pRingList->CellSize);

	pRingList->InsertCnt++;
	pRingList->IdxHead++;
	cut_idx_by_cap(pRingList, &pRingList->IdxHead);

	if (pRingList->IdxHead == pRingList->IdxTail) {
		pRingList->IdxTail++;
		cut_idx_by_cap(pRingList, &pRingList->IdxTail);
	}

	if (isFromISR)
		SYS_CRITICAL_OFF_ISR(uxSavedInterruptStatus);
	else
		SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingList_InsertCell(RingList_t* pRingList, u32 mutID, void* pCell) {
	return RingList_InsertCell_Universal(pRingList, mutID, pCell, false);
}

RET_STATE_t RingList_InsertCell_fromISR(RingList_t* pRingList, u32 mutID, void* pCell) {
	return RingList_InsertCell_Universal(pRingList, mutID, pCell, true);
}

/**
 * @brief Get element (copy) from RingList by it ring index
 * 
 * @param pRingList pointer to the main instance
 * @param pCell pointer to data buff where data needs to be copied
 * @param ringIdx ring index of data needs to be copied
 * @param pLinIdx optional linear index of internal buff which holds data
 * @return RET_STATE_t [RET_STATE_ERR_PARAM, RET_STATE_ERR_EMPTY, RET_STATE_SUCCESS]
 */
RET_STATE_t RingList_ExtractCell(RingList_t* pRingList, void* pCell, u16 ringIdx, u16* pLinIdx) {
	if (!pRingList || !pCell)
		EXIT(RET_STATE_ERR_PARAM);

	SYS_CRITICAL_ON();
	if (pRingList->InsertCnt == 0) {
		SYS_CRITICAL_OFF();
		return RET_STATE_ERR_EMPTY;
	}

	if (ringIdx >= pRingList->InsertCnt)
		ringIdx = pRingList->InsertCnt - 1;

	u16 tmpLinIdx = pRingList->IdxTail + ringIdx;
	cut_idx_by_cap(pRingList, &tmpLinIdx);

	u8* tmpListPtr = (u8*)pRingList->List;
	tmpListPtr	   = &tmpListPtr[tmpLinIdx * pRingList->CellSize];
	memcpy(pCell, (void*)tmpListPtr, pRingList->CellSize);

	if (pLinIdx)
		*pLinIdx = tmpLinIdx;
	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingList_CopyToLineBuff(RingList_t* pRingList, void* pLineList, u16 listSize) {
	if (!pRingList || !pLineList)
		EXIT(RET_STATE_ERR_PARAM);

	SYS_CRITICAL_ON();
	if (pRingList->InsertCnt == 0) {
		SYS_CRITICAL_OFF();
		return RET_STATE_ERR_EMPTY;
	}

	u8* lineListStart = (u8*)pRingList->List;
	for (u16 listNum = 0; listNum < listSize; listNum++) {
		u16 tmpLinIdx = pRingList->IdxTail + listNum;
		cut_idx_by_cap(pRingList, &tmpLinIdx);

		u8* tmpListPtr = &lineListStart[tmpLinIdx * pRingList->CellSize];
		memcpy(pLineList + (listNum * pRingList->CellSize), (void*)tmpListPtr, pRingList->CellSize);
	}

	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}
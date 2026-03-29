#include "ring_buff.h"

#if LIB_USE_RING_BUFF

#define RING_BUFF_DEF_TMO_MS 3

/**
 * @brief Cut head or tail fields with known about rBuff capacity.
 * Use only with local copy of variable!
 */
static void ring_buff_cut_ptr(RingBuff_t* pRingBuff, u8** ppRingBuffPtr) {
	if (*ppRingBuffPtr >= pRingBuff->Buff + pRingBuff->Capacity)
		*ppRingBuffPtr -= pRingBuff->Capacity;
}

/*
 * Can works with real rBuff structure
 * deltaPtr variable is the difference between two pointers.
 */
static u16 ring_buff_compute_diff_ptr(RingBuff_t* pRingBuff, u8* pRingBuffPtr) {
	u16 deltaPtr;

	SYS_CRITICAL_ON();
	if (pRingBuff->Tail < pRingBuffPtr)
		deltaPtr = pRingBuffPtr - pRingBuff->Tail;
	else if (pRingBuff->Tail > pRingBuffPtr)
		deltaPtr = pRingBuff->Capacity - (pRingBuff->Tail - pRingBuffPtr);
	else
		deltaPtr = 0;
	SYS_CRITICAL_OFF();

	return deltaPtr;
}

static void ring_buff_dec_count(RingBuff_t* pRingBuff, u16 number) {
	SYS_CRITICAL_ON();
	pRingBuff->Count -= number;
	SYS_CRITICAL_OFF();
}

static void ring_buff_inc_count(RingBuff_t* pRingBuff, u16 number) {
	SYS_CRITICAL_ON();
	pRingBuff->Count += number;
	SYS_CRITICAL_OFF();
}

static void ring_buff_print_rx_data_stub(void) {};

RET_STATE_t RingBuff_Init(RingBuff_t* pRingBuff, u8* pMainBuff, u16 buffSize,
						  RingBuff_PrintRxDataClbk_t printRxDataClbk) {

	if (!pRingBuff || !pMainBuff) {
		PANIC();
		return RET_STATE_ERR_PARAM;
	}

	pRingBuff->Buff		= pMainBuff;
	pRingBuff->Head		= &pMainBuff[0];
	pRingBuff->Tail		= &pMainBuff[0];
	pRingBuff->Count	= 0;
	pRingBuff->Capacity = buffSize;
	memset(pRingBuff->Buff, 0, pRingBuff->Capacity);
	pRingBuff->PrintRxData = ring_buff_print_rx_data_stub;
	if (printRxDataClbk)
		pRingBuff->PrintRxData = printRxDataClbk;

	return RET_STATE_SUCCESS;
}

/*
 * Getters for RingBuff_t structure
 */
u16 RingBuff_GetCnt(RingBuff_t* pRingBuff) {
	return pRingBuff->Count;
}
u16 RingBuff_GetCap(RingBuff_t* pRingBuff) {
	return pRingBuff->Capacity;
}
u8* RingBuff_GetHead(RingBuff_t* pRingBuff) {
	return pRingBuff->Head;
}
u8* RingBuff_GetTail(RingBuff_t* pRingBuff) {
	return pRingBuff->Tail;
}
u8* RingBuff_GetMainBuffPtr(RingBuff_t* pRingBuff) {
	return pRingBuff->Buff;
}
void RingBuff_Print(RingBuff_t* pRingBuff) {
	pRingBuff->PrintRxData();
}

RET_STATE_t RingBuff_WaitBuffFill(RingBuff_t* pRingBuff, u32 tmo, u16 charNum) {

	ASSERT_CHECK(pRingBuff);

	u32 endSearchTime = SYS_TICK_GET_MS_CNT() + tmo;
	while (SYS_TICK_GET_MS_CNT() < endSearchTime) {
		if (RingBuff_GetCnt(pRingBuff) >= charNum)
			return RET_STATE_SUCCESS;

		SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
	}

	return RET_STATE_ERR_TIMEOUT;
}

RET_STATE_t RingBuff_Flush(RingBuff_t* pRingBuff) {

	ASSERT_CHECK(pRingBuff);

	SYS_CRITICAL_ON();
	pRingBuff->Tail	 = pRingBuff->Head;
	pRingBuff->Count = 0;
	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingBuff_RemoteToNewTail(RingBuff_t* pRingBuff, u8* pNewTail) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pNewTail);

	u16 charNum = ring_buff_compute_diff_ptr(pRingBuff, pNewTail);

	SYS_CRITICAL_ON();
	pRingBuff->Tail += charNum;
	ring_buff_cut_ptr(pRingBuff, &pRingBuff->Tail);
	pRingBuff->Count -= charNum;
	SYS_CRITICAL_OFF();

	return RET_STATE_SUCCESS;
}

//------------------------------------------------------------------------------

RET_STATE_t RingBuff_InterruptCallback(RingBuff_t* pRingBuff, u16 bytesReceived) {

	if (bytesReceived == 0)
		return RET_STATE_ERR_EMPTY;

	ASSERT_CHECK(pRingBuff);

	pRingBuff->Count += bytesReceived;
	pRingBuff->Head += bytesReceived;

	/**
	 * rebase head to start if it's out of capacity
	 */
	ring_buff_cut_ptr(pRingBuff, &pRingBuff->Head);

	if (pRingBuff->Count > pRingBuff->Capacity) {
		pRingBuff->Tail	 = pRingBuff->Head;
		pRingBuff->Count = pRingBuff->Capacity;
	}

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingBuff_Line2Ring_Copy(RingBuff_t* pRingBuff, u8* pSrcBuff, u16 charNum, u16* pRwCnt) {

	if (pRwCnt != NULL)
		*pRwCnt = 0;

	if (charNum == 0)
		return RET_STATE_SUCCESS;

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pSrcBuff);

	SYS_CRITICAL_ON();
	u8* locHead	 = pRingBuff->Head;
	u16 locCount = pRingBuff->Count;
	SYS_CRITICAL_OFF();

	u16 wasReadWriteCnt = 0;

	if (charNum + locCount > pRingBuff->Capacity)
		charNum = pRingBuff->Capacity - locCount;

	for (; wasReadWriteCnt < charNum; wasReadWriteCnt++) {
		*locHead++ = *pSrcBuff++;
		ring_buff_cut_ptr(pRingBuff, &locHead);
	}

	SYS_CRITICAL_ON();
	ring_buff_inc_count(pRingBuff, wasReadWriteCnt);
	pRingBuff->Head = locHead;
	SYS_CRITICAL_OFF();

	if (pRwCnt != NULL)
		*pRwCnt = wasReadWriteCnt;

	return RET_STATE_SUCCESS;
}

/*
 *[input] pRingBuff:	ring buffer address
 *[input] pDstBuff:	destination buffer address
 *[input] charNum:	length of destination buffer
 *[i/o]	  pRwCnt:	count of written bytes from ring buffer
 *[output] Return State
 */
RET_STATE_t RingBuff_Ring2Line_Copy(RingBuff_t* pRingBuff, u8* pDstBuff, u16 charNum, u16* pRwCnt) {

	if (pRwCnt != NULL)
		*pRwCnt = 0;

	if (charNum == 0)
		return RET_STATE_SUCCESS;

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pDstBuff);

	SYS_CRITICAL_ON();
	u8* pLocTail = pRingBuff->Tail;
	u16 locCount = pRingBuff->Count;
	SYS_CRITICAL_OFF();
	if (locCount == 0)
		return RET_STATE_ERR_EMPTY;

	u16 wasReadWriteCnt = 0;

	if (charNum > locCount)
		charNum = locCount;

	for (; wasReadWriteCnt < charNum; wasReadWriteCnt++) {
		*pDstBuff++ = *pLocTail++;
		ring_buff_cut_ptr(pRingBuff, &pLocTail);
	}

	SYS_CRITICAL_ON();
	ring_buff_dec_count(pRingBuff, wasReadWriteCnt);
	pRingBuff->Tail = pLocTail;
	SYS_CRITICAL_OFF();

	if (pRwCnt != NULL)
		*pRwCnt = wasReadWriteCnt;

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingBuff_Ring2Ring_Copy(RingBuff_t* pRingBuffSrc, RingBuff_t* pRingBuffSDst,
									u16 charNum, u16* pRwCnt) {

	if (pRwCnt != NULL)
		*pRwCnt = 0;

	if (charNum == 0)
		return RET_STATE_SUCCESS;

	ASSERT_CHECK(pRingBuffSrc);
	ASSERT_CHECK(pRingBuffSDst);

	SYS_CRITICAL_ON();
	u8* locTailSrc	= pRingBuffSrc->Tail;
	u8* locHeadDst	= pRingBuffSDst->Head;
	u16 locCountSrc = pRingBuffSrc->Count;
	u16 locCountDst = pRingBuffSDst->Count;
	SYS_CRITICAL_OFF();

	if (locCountSrc == 0)
		return RET_STATE_ERR_EMPTY;

	u16 wasReadWriteCnt = 0;

	if (charNum > locCountSrc)
		charNum = locCountSrc;
	if (charNum + locCountDst > pRingBuffSDst->Capacity)
		charNum = pRingBuffSDst->Capacity - locCountDst;

	for (; wasReadWriteCnt < charNum; wasReadWriteCnt++) {
		*locHeadDst++ = *locTailSrc++;
		ring_buff_cut_ptr(pRingBuffSrc, &locTailSrc);
		ring_buff_cut_ptr(pRingBuffSDst, &locHeadDst);
	}

	SYS_CRITICAL_ON();
	ring_buff_dec_count(pRingBuffSrc, wasReadWriteCnt);
	ring_buff_inc_count(pRingBuffSDst, wasReadWriteCnt);
	pRingBuffSrc->Tail	= locTailSrc;
	pRingBuffSDst->Head = locHeadDst;
	SYS_CRITICAL_OFF();

	if (pRwCnt != NULL)
		*pRwCnt = wasReadWriteCnt;

	return RET_STATE_SUCCESS;
}

RET_STATE_t RingBuff_Buff_Search(RingBuff_t* pRingBuff, u32 timeout, u8* pMatchBuff, u16 buffLen) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pMatchBuff);

	u8* locTail	   = pRingBuff->Tail;
	u16 wasReadCnt = 0, ptr = 0;
	u32 endSearchTime = SYS_TICK_GET_MS_CNT() + timeout;

	while (SYS_TICK_GET_MS_CNT() <= endSearchTime) {
		if (pRingBuff->Count < buffLen) {
			SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);	 //Wait more data
			continue;
		} else {
			while (*locTail != pMatchBuff[0]) {
				if (wasReadCnt >= pRingBuff->Count - buffLen) {
					wasReadCnt = 0;
					locTail	   = pRingBuff->Tail;
					SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
					break;
				}

				wasReadCnt++;
				locTail++;

				ring_buff_cut_ptr(pRingBuff, &locTail);
			}
		}

		if (*locTail == pMatchBuff[0]) {
			for (ptr = 0; ptr < buffLen; ptr++) {
				if (*locTail != pMatchBuff[ptr])
					break;

				locTail++;
				ring_buff_cut_ptr(pRingBuff, &locTail);
			}

			wasReadCnt += ptr;

			if (ptr == buffLen) {
				ring_buff_dec_count(pRingBuff, wasReadCnt);
				pRingBuff->Tail = locTail;

				return RET_STATE_SUCCESS;
			}
		}
	}

	return RET_STATE_ERR_TIMEOUT;
}

RET_STATE_t RingBuff_Str_Search(RingBuff_t* pRingBuff, u32 timeout, char* pMatchStr) {
	return RingBuff_Buff_Search(pRingBuff, timeout, (u8*)pMatchStr, (u16)strlen(pMatchStr));
}

RET_STATE_t RingBuff_Buff_Peek(RingBuff_t* pRingBuff, u8* pMatchBuff, u16 buffLen, u8** ppMatchBuff,
							   u16* pReadCnt) {
	if (pReadCnt != NULL)
		*pReadCnt = 0;

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pMatchBuff);

	SYS_CRITICAL_ON();
	u8* locTail	 = pRingBuff->Tail;
	u16 locCount = pRingBuff->Count;
	SYS_CRITICAL_OFF();

	u16 availableLen = locCount - buffLen;
	u16 wasReadCnt = 0, ptr = 0;

	if (locCount < buffLen)
		return RET_STATE_ERR_EMPTY;

	while (wasReadCnt <= availableLen) {
		if (*locTail != pMatchBuff[0]) {
			wasReadCnt++;
			locTail++;
			ring_buff_cut_ptr(pRingBuff, &locTail);
		} else {
			if (ppMatchBuff != NULL)
				*ppMatchBuff = locTail;

			for (ptr = 0; ptr < buffLen; ptr++, locTail++) {
				ring_buff_cut_ptr(pRingBuff, &locTail);
				if (pMatchBuff[ptr] == '~')
					continue;
				if (pMatchBuff[ptr] != *locTail)
					break;
			}

			wasReadCnt += ptr;

			if (ptr == buffLen) {
				if (pReadCnt != NULL)
					*pReadCnt = wasReadCnt - ptr;
				return RET_STATE_SUCCESS;
			}
		}
	}

	if (ppMatchBuff != NULL)
		*ppMatchBuff = NULL;

	return RET_STATE_ERR_EMPTY;
}

RET_STATE_t RingBuff_Str_Peek(RingBuff_t* pRingBuff, char* pMatchStr, u8** ppMatchStr,
							  u16* pReadCnt) {
	return RingBuff_Buff_Peek(pRingBuff, (u8*)pMatchStr, (u16)strlen(pMatchStr), ppMatchStr,
							  pReadCnt);
}

RET_STATE_t RingBuff_Buff_WaitForCompare(RingBuff_t* pRingBuff, u32 timeout, u32* pTimeSpent,
										 u8* pCompareBuff, u16 buffLen, u8** ppMatchBuff,
										 u16* pReadCnt) {

	RET_STATE_t retState = RET_STATE_ERR_TIMEOUT;
	u32 endSearchTime	 = SYS_TICK_GET_MS_CNT() + timeout;

	while (SYS_TICK_GET_MS_CNT() < endSearchTime) {
		retState = RingBuff_Buff_Peek(pRingBuff, pCompareBuff, buffLen, ppMatchBuff, pReadCnt);
		if (retState == RET_STATE_SUCCESS)
			break;

		SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
	}

	if (pTimeSpent != NULL)
		*pTimeSpent = SYS_TICK_GET_MS_CNT() - (endSearchTime - timeout);

	return retState;
}

RET_STATE_t RingBuff_Str_WaitForCompare(RingBuff_t* pRingBuff, u32 timeout, u32* pTimeSpent,
										char* compareStr, u8** ppMatchStr, u16* pReadCnt) {

	return RingBuff_Buff_WaitForCompare(pRingBuff, timeout, pTimeSpent, (u8*)compareStr,
										(u16)strlen(compareStr), ppMatchStr, pReadCnt);
}

/**
 * @brief 	Peek data chunk from the ring buffer without moving head or tail.
 * 			Ring buffer be like this: ...<data><one><data to peek><two>...,
 * 			and destination buffer gets the <data to peek> bytes.
 * @param 	pRingBuff Ring buffer pointer.
 * @param 	pDstBuff Destination buffer pointer.
 * @param 	dstBuffMaxSize Maximum number of bytes that destination buffer can
 * 			fit.
 * @param	entryNum If the ring buffer data contains more than one chunk that
 * 			is guarded by the ppOneTwo strings, the function caller can specify
 * 			which entry it is interested in. The numeration starts with '1'.
 * @param 	ppOneTwo Casting this pointer we get the array of strings that
 * 			guards data chunks to be transfer.
 * @param 	pRwCnt Pointer to the value that counts the bytes that has been
 * 			transfered to the destination buffer from the ring list after
 * 			successful function execution.
 * @return	RET_STATE_t Returns RET_STATE_SUCCESS if the 'data to peak' has been
 * 			found and all of these found bytes (or just part of them, - see
 * 			pRwCnt) have been successfully transfered.
 */
RET_STATE_t RingBuff_Str_PeekBetween(RingBuff_t* pRingBuff, u8* pDstBuff, u16 dstBuffMaxSize,
									 u8 entryNum, char** ppOneTwo, u16* pRwCnt) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pDstBuff);
	ASSERT_CHECK(ppOneTwo);

	/* Enter critical section right there because we have to keep the ring
	 * buffer copy that we are going to create to be synchronized with the
	 * original ring buffer */
	SYS_CRITICAL_ON();
	RET_STATE_t retState;
	RingBuff_t locRingBuff = *pRingBuff;
	u8* pMatchStr		   = NULL;
	char* pOne			   = ppOneTwo[0];
	for (u32 i = 0; i < entryNum; i++) {
		/* Trying to find the first string that guards the 'data to peek' */
		retState = RingBuff_Str_Peek(&locRingBuff, pOne, &pMatchStr, NULL);
		if (retState != RET_STATE_SUCCESS) {
			SYS_CRITICAL_OFF();
			return retState;
		}
		/* ...<data><one><data to peek><two>... */
		RingBuff_RemoteToNewTail(&locRingBuff, pMatchStr + strlen(pOne));
		/* <data to peek><two>... */
	}

	u16 pReadCnt = 0;
	/* Now let's see if the second guarding string exist further in the
	 * ring buffer and how many bytes is there to it */
	retState	 = RingBuff_Str_Peek(&locRingBuff, ppOneTwo[1], &pMatchStr, &pReadCnt);
	if (retState != RET_STATE_SUCCESS) {
		SYS_CRITICAL_OFF();
		return retState;
	}
	if (pReadCnt > dstBuffMaxSize)
		pReadCnt = dstBuffMaxSize;

	retState = RingBuff_Ring2Line_Copy(&locRingBuff, pDstBuff, pReadCnt, pRwCnt);
	SYS_CRITICAL_OFF();

	return retState;
}

//------------------------------------------------------------------------------

RET_STATE_t RingBuff_Ring2Line_CopyBeforeEntry(RingBuff_t* pRingBuff, u32 timeout, u8* pDstBuff,
											   u16 dstBuffMaxSize, char* pStringEntry,
											   u16* pRwCnt) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pDstBuff);
	ASSERT_CHECK(pStringEntry);

	if (pRwCnt != NULL)
		*pRwCnt = 0;

	RET_STATE_t retState = RET_STATE_UNDEF;
	u8* pStrEntry		 = NULL;
	u32 endSearchTime	 = SYS_TICK_GET_MS_CNT() + timeout;

	while (SYS_TICK_GET_MS_CNT() < endSearchTime) {
		retState = RingBuff_Str_Peek(pRingBuff, pStringEntry, &pStrEntry, NULL);
		if (retState == RET_STATE_SUCCESS) {
			u16 charNum = ring_buff_compute_diff_ptr(pRingBuff, pStrEntry);
			if (charNum > dstBuffMaxSize)
				charNum = dstBuffMaxSize;

			retState = RingBuff_Ring2Line_Copy(pRingBuff, pDstBuff, charNum, pRwCnt);
			return retState;
		}

		SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
	}

	return RET_STATE_ERR_TIMEOUT;
}

RET_STATE_t RingBuff_Str_SearchEntryBetween(RingBuff_t* pRingBuff, u32 timeout, u8* pDstBuff,
											u16 dstBuffMaxSize, char** ppOneTwo, u16* pRwCnt) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(pDstBuff);
	ASSERT_CHECK(ppOneTwo);

	if (pRwCnt != NULL)
		*pRwCnt = 0;

	RET_STATE_t retState = RET_STATE_UNDEF;

	retState = RingBuff_Str_Search(pRingBuff, timeout, ppOneTwo[0]);
	if (retState == RET_STATE_SUCCESS) {
		retState = RingBuff_Ring2Line_CopyBeforeEntry(pRingBuff, timeout, (u8*)pDstBuff,
													  dstBuffMaxSize, ppOneTwo[1], pRwCnt);
		if (retState == RET_STATE_SUCCESS && pRwCnt != NULL)
			pDstBuff[*pRwCnt] = '\0';
	}

	return retState;
}

RET_STATE_t RingBuff_Str_SetSearch(RingBuff_t* pRingBuff, u32 timeout, char** ppSearchSequence,
								   u16 sequenceNum, s16* pMatchStr) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(ppSearchSequence);
	ASSERT_CHECK(sequenceNum);
	ASSERT_CHECK(pMatchStr);

	s16 winStr = -1;

	RET_STATE_t retState = RET_STATE_UNDEF;
	u16 readCnt			 = 0;
	u32 endSearchTime	 = SYS_TICK_GET_MS_CNT() + timeout;

	while (SYS_TICK_GET_MS_CNT() < endSearchTime) {
		for (u32 strNum = 0; strNum < sequenceNum; strNum++) {
			retState = RingBuff_Str_Peek(pRingBuff, ppSearchSequence[strNum], NULL, &readCnt);
			if (retState == RET_STATE_SUCCESS) {
				winStr = strNum;
				break;
			}
		}

		if (winStr != -1) {
			retState =
				RingBuff_Str_Search(pRingBuff, RING_BUFF_DEF_TMO_MS, ppSearchSequence[winStr]);
			if (retState == RET_STATE_SUCCESS) {
				*pMatchStr = winStr;
				return retState;
			}
		}

		SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
	}

	*pMatchStr = -1;
	return RET_STATE_ERR_TIMEOUT;
}

RET_STATE_t RingBuff_Str_SetPeek(RingBuff_t* pRingBuff, u32 timeout, char** ppSearchSequence,
								 u16 sequenceNum, s16* pMatchStr) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(ppSearchSequence);
	ASSERT_CHECK(sequenceNum);
	ASSERT_CHECK(pMatchStr);

	s16 winStr = -1;

	RET_STATE_t retState = RET_STATE_UNDEF;
	u16 readCnt			 = 0;
	u32 endSearchTime	 = SYS_TICK_GET_MS_CNT() + timeout;

	while (SYS_TICK_GET_MS_CNT() < endSearchTime) {
		for (u32 strNum = 0; strNum < sequenceNum; strNum++) {
			retState = RingBuff_Str_Peek(pRingBuff, ppSearchSequence[strNum], NULL, &readCnt);
			if (retState == RET_STATE_SUCCESS) {
				winStr = strNum;
				break;
			}
		}

		if (winStr != -1) {
			*pMatchStr = winStr;
			return RET_STATE_SUCCESS;
		}

		SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
	}

	*pMatchStr = -1;
	return RET_STATE_ERR_TIMEOUT;
}

RET_STATE_t RingBuff_Str_SetPeekFirst(RingBuff_t* pRingBuff, u32 timeout, char** ppSearchSequence,
									  u16 sequenceNum, s16* pMatchStr) {

	ASSERT_CHECK(pRingBuff);
	ASSERT_CHECK(ppSearchSequence);
	ASSERT_CHECK(sequenceNum);
	ASSERT_CHECK(pMatchStr);

	s16 winStr = -1;

	RET_STATE_t retState = RET_STATE_UNDEF;
	u16 readCnt			 = 0;
	u32 endSearchTime	 = SYS_TICK_GET_MS_CNT() + timeout;

	u16 readCntMin = 0xFFFF;

	while (SYS_TICK_GET_MS_CNT() < endSearchTime) {
		for (u32 strNum = 0; strNum < sequenceNum; strNum++) {
			retState = RingBuff_Str_Peek(pRingBuff, ppSearchSequence[strNum], NULL, &readCnt);
			if (retState == RET_STATE_SUCCESS) {
				if (readCnt < readCntMin) {
					readCntMin = readCnt;
					winStr	   = strNum;
				}
			}
		}

		if (winStr != -1) {
			*pMatchStr = winStr;
			return RET_STATE_SUCCESS;
		}

		SYS_DELAY_MS(RING_BUFF_DEF_TMO_MS);
	}

	*pMatchStr = -1;
	return RET_STATE_ERR_TIMEOUT;
}

#endif /* LIB_USE_RING_BUFF */

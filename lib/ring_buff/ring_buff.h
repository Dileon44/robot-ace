#ifndef __RINGBUFF_H
#define __RINGBUFF_H

#include "def_sys.h"
#include "def_types.h"
#include "lib_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if LIB_USE_RING_BUFF

typedef void (*RingBuff_PrintRxDataClbk_t)(void);

typedef struct {
	u8* Buff;	   //Pointer to static buffer in RAM
	u8* Head;	   //Address where the new entry will be placed
	u8* Tail;	   //Address from which reading will be carried out
	u16 Count;	   //Number of unread entries
	u16 Capacity;  //Full space for useful data
	RingBuff_PrintRxDataClbk_t PrintRxData;
} RingBuff_t;

//Low-level
RET_STATE_t RingBuff_Init(RingBuff_t* pRingBuff, u8* pMainBuff, u16 buffSize,
						  RingBuff_PrintRxDataClbk_t printRxDataClbk);
u16 RingBuff_GetCnt(RingBuff_t* pRingBuff);
u16 RingBuff_GetCap(RingBuff_t* pRingBuff);
u8* RingBuff_GetHead(RingBuff_t* pRingBuff);
u8* RingBuff_GetTail(RingBuff_t* pRingBuff);
u8* RingBuff_GetMainBuffPtr(RingBuff_t* pRingBuff);
void RingBuff_Print(RingBuff_t* pRingBuff);

RET_STATE_t RingBuff_WaitBuffFill(RingBuff_t* pRingBuff, u32 timeout, u16 charNum);
RET_STATE_t RingBuff_Flush(RingBuff_t* pRingBuff);
RET_STATE_t RingBuff_RemoteToNewTail(RingBuff_t* pRingBuff, u8* pNewTail);

//Mid-level
RET_STATE_t RingBuff_InterruptCallback(RingBuff_t* pRingBuff, u16 bytesReceived);
RET_STATE_t RingBuff_Line2Ring_Copy(RingBuff_t* pRingBuff, u8* srcBuff, u16 charNum, u16* pRwCnt);
RET_STATE_t RingBuff_Ring2Line_Copy(RingBuff_t* pRingBuff, u8* pDstBuff, u16 charNum, u16* pRwCnt);
RET_STATE_t RingBuff_Ring2Ring_Copy(RingBuff_t* pRingBuffSrc, RingBuff_t* pRingBuffDst, u16 charNum,
									u16* pRwCnt);
RET_STATE_t RingBuff_Buff_Search(RingBuff_t* pRingBuff, u32 timeout, u8* pMatchBuff, u16 buffLen);
RET_STATE_t RingBuff_Str_Search(RingBuff_t* pRingBuff, u32 timeout, char* pMatchStr);
RET_STATE_t RingBuff_Buff_Peek(RingBuff_t* pRingBuff, u8* pMatchBuff, u16 buffLen, u8** ppMatchBuff,
							   u16* pReadCnt);
RET_STATE_t RingBuff_Str_Peek(RingBuff_t* pRingBuff, char* pMatchStr, u8** ppMatchStr,
							  u16* pReadCnt);
RET_STATE_t RingBuff_Buff_WaitForCompare(RingBuff_t* pRingBuff, u32 timeout, u32* pTimeSpent,
										 u8* pCompareBuff, u16 buffLen, u8** ppMatchBuff,
										 u16* pReadCnt);
RET_STATE_t RingBuff_Str_WaitForCompare(RingBuff_t* pRingBuff, u32 timeout, u32* pTimeSpent,
										char* compareStr, u8** ppMatchStr, u16* pReadCnt);
RET_STATE_t RingBuff_Str_PeekBetween(RingBuff_t* pRingBuff, u8* pDstBuff, u16 dstBuffMaxSize,
									 u8 entryNum, char** ppOneTwo, u16* pRwCnt);

//High-level
RET_STATE_t RingBuff_Ring2Line_CopyBeforeEntry(RingBuff_t* pRingBuff, u32 timeout, u8* pDstBuff,
											   u16 dstBuffMaxSize, char* pStringEntry, u16* pRwCnt);
RET_STATE_t RingBuff_Str_SearchEntryBetween(RingBuff_t* pRingBuff, u32 timeout, u8* pDstBuff,
											u16 dstBuffMaxSize, char** ppOneTwo, u16* pRwCnt);
RET_STATE_t RingBuff_Str_SetSearch(RingBuff_t* pRingBuff, u32 timeout, char** ppSearchSequence,
								   u16 sequenceNum, s16* pMatchStr);
RET_STATE_t RingBuff_Str_SetPeek(RingBuff_t* pRingBuff, u32 timeout, char** ppSearchSequence,
								 u16 sequenceNum, s16* pMatchStr);
RET_STATE_t RingBuff_Str_SetPeekFirst(RingBuff_t* pRingBuff, u32 timeout, char** ppSearchSequence,
									  u16 sequenceNum, s16* pMatchStr);

#endif /* LIB_USE_RING_BUFF */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RINGBUFF_H */

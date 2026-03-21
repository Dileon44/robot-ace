#ifndef __RINGLIST_H
#define __RINGLIST_H

#include "main.h"

#define RING_LIST_MIN_ELEMENTS_NUM 2
#define RING_LIST_AVOID_MUTEX	   0

typedef struct {
	void* List;
	u16 IdxHead;  //add new data in lineList with this index
	u16 IdxTail;  //get existed data from lineList with this index
	u16 CellSize;
	u16 Capacity;
	u32 InsertCnt;
	u32 MutexID;
} RingList_t;

RET_STATE_t RingList_Init(RingList_t* pRingList, void* lineList, u16 cellSize, u16 listSize);
void* RingList_GetLineList(RingList_t* pRingList);
u16 RingList_GetCellSize(RingList_t* pRingList);
u16 RingList_GetCapacity(RingList_t* pRingList);
u16 RingList_GetInsertCnt(RingList_t* pRingList);
void RingList_GetHead(RingList_t* pRingList, void* pHead);
void RingList_GetTail(RingList_t* pRingList, void* pTail);

u16 RingList_GetExistingElementsNum(RingList_t* pRingList);
u16 RingList_GetLatestRingIdx(RingList_t* pRingList);
bool RingList_IsFull(RingList_t* pRingList);

bool RingList_Lock(RingList_t* pRingList, u32 mutID);
bool RingList_Unlock(RingList_t* pRingList, u32 mutID);

RET_STATE_t RingList_Flush(RingList_t* pRingList, u32 mutID);
RET_STATE_t RingList_InsertCell(RingList_t* pRingList, u32 mutID, void* pCell);
RET_STATE_t RingList_InsertCell_fromISR(RingList_t* pRingList, u32 mutID, void* pCell);
RET_STATE_t RingList_ExtractCell(RingList_t* pRingList, void* pCell, u16 ringIdx, u16* pLinIdx);
RET_STATE_t RingList_CopyToLineBuff(RingList_t* pRingList, void* pLineList, u16 listSize);

#endif /* __RINGLIST_H */

#ifndef __RINGLIST_H
#define __RINGLIST_H

#include "shared_mutex.h"
#include "wsh_emblib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if WSH_USE_COLLECTIONS_RING_LIST

#define RING_LIST_MIN_ELEMENTS_NUM 2
#define RING_LIST_AVOID_MUTEX	   0

typedef struct {
	void* List;
	u16 IdxHead;  //add new data in lineList with this index
	u16 IdxTail;  //get existed data from lineList with this index
	u16 CellSize;
	u16 Capacity;
	u32 InsertCnt;
	SharedMutex_t SharedMutex;
} RingList_t;

RET_STATE_t RingList_Init(RingList_t* pRingList, void* pLineList, u16 cellSize, u16 listSize);
void* RingList_GetLineList(RingList_t* pRingList);
u16 RingList_GetCellSize(RingList_t* pRingList);
u16 RingList_GetCapacity(RingList_t* pRingList);
u16 RingList_GetInsertCnt(RingList_t* pRingList);

u16 RingList_GetExistingElementsNum(RingList_t* pRingList);
u16 RingList_GetLatestRingIdx(RingList_t* pRingList);
bool RingList_IsFull(RingList_t* pRingList);

RET_STATE_t RingList_Flush(RingList_t* pRingList, u32 lockKey);
RET_STATE_t RingList_InsertCell(RingList_t* pRingList, void* pCell, u32 lockKey);
RET_STATE_t RingList_PeekCell(RingList_t* pRingList, void* pCell, u16 ringIdx);
RET_STATE_t RingList_CopyToLineBuff(RingList_t* pRingList, void* pLineList, u16 listSize);

#endif /* WSH_USE_COLLECTIONS_RING_LIST */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RINGLIST_H */

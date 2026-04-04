#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#include "main.h"

TaskHandle_t WatchDog_GetTaskHandle(void);
void WatchDog_Init(void);
void WatchDog_RstCnt(void);
void WatchDog_TaskCreateIfNotExists(u32 tmo);
void WatchDog_TaskCreateOrProlongate(u32 tmo);
void WatchDog_TaskDeleteIfExists(void);
void FreeRTOS_WatchDog_InitComponents(bool resources, bool tasks);

#endif /* __WATCHDOG_H */

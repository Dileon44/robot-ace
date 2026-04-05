#include "watchdog.h"
#include "debug.h"
#include "platform.h"

#if DEBUG_ENABLE
#define LOCAL_DEBUG_PRINT_ENABLE 0
#endif /* DEBUG_ENABLE */

#if LOCAL_DEBUG_PRINT_ENABLE
#warning LOCAL_DEBUG_PRINT_ENABLE
#define LOCAL_DEBUG_PRINT DEBUG_LOG_PRINT
#else
#define LOCAL_DEBUG_PRINT(_f_, ...)
#endif

#if DEBUG_ENABLE
#define WATCHDOG_MAX_TIMEOUT_MS (DELAY_1_DAY)
#else /* DEBUG_ENABLE */
#define WATCHDOG_MAX_TIMEOUT_MS (3 * DELAY_1_MINUTE)
#endif /* DEBUG_ENABLE */

static TaskHandle_t WatchDog_Handle;
static TickType_t WatchDog_EndTs;

TaskHandle_t WatchDog_GetTaskHandle(void) {
	return WatchDog_Handle;
}

void WatchDog_Init(void) {
	Pl_WatchDog_Init();
}

void WatchDog_RstCnt(void) {
	Pl_WatchDog_RstCnt();
}

static void vTask_WatchDog_Process(void* pvParameters) {
	WatchDog_EndTs = xTaskGetTickCount() + (u32)(pvParameters);

	for (;;) {
		WatchDog_RstCnt();
		vTaskDelay(3 * DELAY_1_SECOND);

		/**
		 * Delete watchdog task after timeout
		 * User needs to reset iwds himself 
		 */
		if (xTaskGetTickCount() > WatchDog_EndTs) {
			WatchDog_TaskDeleteIfExists();
		}
	}
}

void WatchDog_TaskCreateIfNotExists(u32 tmo) {
	if (!WatchDog_Handle) {
		BaseType_t taskState =
			xTaskCreate(vTask_WatchDog_Process, "watchdog-process", WATCHDOG_TASK_STACK, (void*)tmo,
						WATCHDOG_TASK_PRIORITY, &WatchDog_Handle);

		if (taskState != pdPASS) {
			PANIC();
		}
	}
}

void WatchDog_TaskCreateOrProlongate(u32 tmo) {
	if (!WatchDog_Handle)
		WatchDog_TaskCreateIfNotExists(tmo);
	else
		WatchDog_EndTs += tmo;
}

void WatchDog_TaskDeleteIfExists(void) {
	if (WatchDog_Handle) {
		vTaskDelete(WatchDog_Handle);
		WatchDog_Handle = NULL;
	}
}

void FreeRTOS_WatchDog_InitComponents(bool resources, bool tasks) {
	if (resources) {
	}

	if (tasks) {
		WatchDog_TaskCreateIfNotExists(WATCHDOG_MAX_TIMEOUT_MS);
	}
}

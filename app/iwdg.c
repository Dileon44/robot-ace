#include "iwdg.h"
#include "platform.h"
#include "conf/tasks_stack_and_priority.h"
#include "debug_interface.h"

#define IWDG_MAX_TIMEOUT_MS	(3 * DELAY_1_SECOND) // ?

static TaskHandle_t WatchDog_Handle = NULL;

void IWDG_Init(void) {
	Pl_IWDG_Init();
}

void IWDG_ResetCnt(void) {
	Pl_IWDG_ReloadCounter();
}

static void vTask_WatchDogProcess(void* pvParameters) {
	// TickType_t timeout = xTaskGetTickCount() + IWDG_MAX_TIMEOUT_MS;

	// uint8_t testDataToSend[8];
	// for (uint8_t i = 0; i < 8; i++) {
	// 	testDataToSend[i] = i;
	// }
	// MX_USB_Device_Init();
	
	for(;;) {
		IWDG_ResetCnt();
		// DEBUG_PRINT("Hello!\r\n");
		vTaskDelay(1000);
		
		// CDC_Transmit_FS(testDataToSend, 8);

		// if(xTaskGetTickCount() > timeout)
		// {
		// 	// need do test: WatchDog_Handle != NULL ?
		// 	vTaskDelete(WatchDog_Handle);
		// }
	}
}

void IWDG_TaskCreate(void) {
	if(!WatchDog_Handle) {
			const char* taskName = "Watchdog Driver";
		xTaskCreate(
			vTask_WatchDogProcess,
			taskName,
			WATCHDOG_TASK_STACK,
			NULL,
			WATCHDOG_TASK_PRIORITY,
			&WatchDog_Handle
		);
	}
}

void IWDG_TaskDelete(void) {
	if(WatchDog_Handle) {
		vTaskDelete(WatchDog_Handle);
	}
}

void FreeRTOS_IWDG_InitComponents(void) {
	IWDG_TaskCreate();
}

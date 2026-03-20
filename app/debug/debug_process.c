#include "debug_process.h"
#include "debug_interface.h"
#include "platform.h"
#include "conf/tasks_stack_and_priority.h"

static TaskHandle_t DebugProcess_HandleRx = NULL;

static void vTask_DebugProcess(void* pvParameters) {
    for(;;) {
		Pl_Led_Toggle();
		DEBUG_PRINT("Test\r\n");
		vTaskDelay(1000);
	}
}

void DebugProcess_TaskCreate(void) {
	if(!DebugProcess_HandleRx) {
		const char* taskName = "Debug Process";
		xTaskCreate(
			vTask_DebugProcess,
			taskName,
			DEBUG_PROCESS_TASK_STACK,
			NULL,
			DEBUG_PROCESS_TASK_PRIORITY,
			&DebugProcess_HandleRx
		);
	}
}

void FreeRTOS_DebugProcess_InitComponents() {
	DebugProcess_TaskCreate();
}

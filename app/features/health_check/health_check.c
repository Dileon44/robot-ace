#include "health_check.h"
#include "debug.h"
#include "delay.h"
#include "encoder_m.h"
#include "platform.h"

#if HEALTH_CHECK

#if DEBUG_ENABLE
#define LOCAL_DEBUG_PRINT_ENABLE 0
#endif /* DEBUG_ENABLE */

#if LOCAL_DEBUG_PRINT_ENABLE
#warning LOCAL_DEBUG_PRINT_ENABLE
#define LOCAL_DEBUG_PRINT DEBUG_LOG_PRINT
#else /* DEBUG_ENABLE */
#define LOCAL_DEBUG_PRINT(_f_, ...)
#endif /* DEBUG_ENABLE */

static TaskHandle_t HealthCheck_Handle;

static void vTask_HealthCheck_Process(void* pvParameters) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));

		Pl_Led_Toggle();

		volatile u32 start = Delay_TimeMilliSec_Get();
		DEBUG_PRINT("%3.1f;%u\r\n", EncoderM_GetAngleDeg(), Delay_TimeMilliSec_Get());
		// vTaskDelay(20);
		volatile u32 end  = Delay_TimeMilliSec_Get();
		volatile u32 diff = end - start;
		if (diff > 2) {
			PANIC();
		}
	}
}

void FreeRTOS_HealthCheck_InitComponents(bool resources, bool tasks) {
	if (resources) {
	}

	if (tasks) {
		xTaskCreate(vTask_HealthCheck_Process, "health-check-process", HEALTH_CHECK_TASK_STACK,
					NULL, HEALTH_CHECK_TASK_PRIORITY, &HealthCheck_Handle);
	}
}

#else /* HEALTH_CHECK */

void FreeRTOS_HealthCheck_InitComponents(bool resources, bool tasks) {
}

#endif /* HEALTH_CHECK */

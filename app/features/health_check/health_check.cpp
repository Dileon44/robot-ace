#include "health_check.h"
#include "bsp.h"
#include "delay.h"
#include "log.h"
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
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));

		bsp::encoder::IDriver* encoder = bsp::encoder::GetPtr();
		if (encoder) {
			DEBUG_PRINT(
				"HealthCheck: encoder angle=%.2f deg, status=0x%02x, AGC=%u, magnitude=%u, "
				"magnet=%s\r\n",
				encoder->GetAngleDeg(), encoder->GetStatus(), encoder->GetAGC(),
				encoder->GetMagnitude(), encoder->IsMagnetDetected() ? "detected" : "not detected");
		} else {
			DEBUG_PRINT("HealthCheck: encoder not attached");
		}

		Pl_Led_Toggle();
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

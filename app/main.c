#include "main.h"
#include "bsp.h"
#include "delay.h"
#include "health_check.h"
#include "log.h"
#include "platform.h"
#include "shell_root.h"
#include "watchdog.h"

#if DEBUG_ENABLE
#warning DEBUG_ENABLE
#endif /* DEBUG_ENABLE */

void HardFault_Clbk(u32 pcVal) {
	// BkpStorage_SetRegister(BKP_REG_SYS_FAULT_EXEPTION_ADDR, pcVal);
}

void ErrorHandler(const char* pFile, int line) {
	DISCARD_UNUSED(pFile);
	DISCARD_UNUSED(line);

	Pl_IrqOff();
	PL_SET_BKPT();
	while (true) {
	};
}

void assert_failed(uint8_t* file, uint32_t line) {
	DISCARD_UNUSED(file);
	DISCARD_UNUSED(line);
	PANIC();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
	ErrorHandler(pcTaskName, 0);
}

void FreeRTOS_InitComponents(bool resources, bool tasks) {
#if !DEBUG_ENABLE
	FreeRTOS_WatchDog_InitComponents(resources, tasks);
#endif /* DEBUG_ENABLE */
	FreeRTOS_HealthCheck_InitComponents(resources, tasks);
	FreeRTOS_Debug_InitComponents(resources, tasks);
	FreeRTOS_ShellRoot_InitComponents(resources, tasks);
}

int main(void) {
#if !DEBUG_ENABLE
	WatchDog_Init();
#endif /* DEBUG_ENABLE */

	Pl_Init(HardFault_Clbk);
	Pl_SysCpuCnt_Init();

	FreeRTOS_InitComponents(true, false);

	Pl_Led_Init();
	Delay_Init();
	Debug_Init();
	// Bsp_Init();

	FreeRTOS_InitComponents(false, true);
	vTaskStartScheduler();

	for (;;) {
		PANIC();
	}

	return 0;
}

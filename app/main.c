#include "main.h"
#include "delay.h"
#include "iwdg.h"
#include "platform.h"

#if DEBUG_ENABLE
#warning DEBUG_ENABLE
#endif /* DEBUG_ENABLE */

u8 ucHeap[configTOTAL_HEAP_SIZE];

void HardFault_Clbk(u32 pcVal) {
	// BkpStorage_SetRegister(BKP_REG_SYS_FAULT_EXEPTION_ADDR, pcVal);
	__NOP();
}

void ErrorHandler(char* pFile, int line) {
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

void FreeRTOS_InitComponents(bool resources, bool tasks) {
}

int main(void) {
#if !DEBUG_ENABLE
	// IWDG_Init();
#endif /* !DEBUG_ENABLE */

	FreeRTOS_InitComponents(true, false);
	Pl_Init(HardFault_Clbk);
	Pl_LedDebug_Init();

	Delay_Init();

	FreeRTOS_InitComponents(false, true);
	vTaskStartScheduler();

	for (;;) {
		PANIC();
	}

	return 0;
}

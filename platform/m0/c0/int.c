#include "int.h"

static Pl_HardFault_Clbk_t HardFault_Clbk = Pl_Stub_HardFaultClbk;

void HardFault_SetCallback(Pl_HardFault_Clbk_t pHardFault_Clbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(HardFault_Clbk, pHardFault_Clbk);
}

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
	struct {
		u32 r0;
		u32 r1;
		u32 r2;
		u32 r3;
		u32 r12;
		u32 lr;
		u32 pc;
		u32 psr;
	}* stack_ptr;

	asm("TST lr, #4 \n"
		"ITE EQ \n"
		"MRSEQ %[ptr], MSP  \n"
		"MRSNE %[ptr], PSP  \n"
		: [ptr] "=r"(stack_ptr));

	HardFault_Clbk(stack_ptr->pc);

	ERROR_HANDLER();
}

void MemManage_Handler(void) {
	while (1)
		;
}

void BusFault_Handler(void) {
	while (1)
		;
}

void UsageFault_Handler(void) {
	while (1)
		;
}

void DebugMon_Handler(void) {
}

/*
void SVC_Handler(void)
{
	//RTOS
}

void PendSV_Handler(void)
{
	//RTOS
}

void SysTick_Handler(void)
{
	//RTOS
}
*/

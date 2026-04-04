#ifndef __SYS_H
#define __SYS_H

#include "main.h"
#include "platform.h"
#include "platform_inc_m0.h"

typedef enum {
	SYS_RESET_FLAG_UNKNOWN = 0,
	SYS_RESET_FLAG_IWDG,
	SYS_RESET_FLAG_LP,
	SYS_RESET_FLAG_PIN,
	SYS_RESET_FLAG_OBL,
	SYS_RESET_FLAG_SOFT,
	SYS_RESET_FLAG_WWDG,
	SYS_RESET_FLAG_BOR,
} SYS_RESET_FLAG_t;

const char* Sys_ResetFlag_GetStr(void);
SYS_RESET_FLAG_t Sys_ResetFlag_Get(void);
void Sys_ResetFlag_Clear(void);

u32 Sys_LSE_IsReadyAndClkSrc(void);
u32 Sys_LSI_IsReadyAndClkSrc(void);
void Sys_RealTimeClock_Config(void);
void Sys_EnableBkpAccess(void);
void Sys_MainClock_Config(void);
u32* Sys_UID_GetStrAndPtr(char* pDst);
void Sys_CPU_GetStrAndPtr(char* pDst);
void Sys_StandbyMode_Enter(void);
bool Sys_CounterCPU_Init(void);
u32 Sys_CounterCPU_Get(void);
void Sys_MCU_Reset(void);
void Sys_NVIC_SetPrioEnable(IRQn_Type irq, u16 prio);
void Sys_NVIC_Disable(IRQn_Type irq);

#endif /* __SYS_H */

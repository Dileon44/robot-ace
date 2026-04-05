#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "main.h"
#include "platform_inc_m0.h"

#ifndef PL_NOP
#define PL_NOP() __NOP()
#endif /* PL_NOP */

#ifndef PL_SHELL_HISTORY_DATA
#define PL_SHELL_HISTORY_DATA __attribute__((section(".SHELL_HISTORY_DATA")))
#endif /* PL_SHELL_HISTORY_DATA */

#ifndef PL_SET_BKPT
#define PL_SET_BKPT(v) __BKPT(v)
#endif /* PL_SET_BKPT */

#define PL_USART_DEF_TMO 50

typedef enum { GPIO_RST, GPIO_SET, GPIO_REV } GPIO_ACTION_t;

typedef struct {
	bool Sys;
	bool Bsp;
	bool DelayMs;
	bool LedSys;
	bool SerialDebug;
	bool LsiClk;
	bool LseClk;
	bool Hsi48Clk;
	bool Rtc;
	bool Iwdg;
	bool CpuCounter;
	bool Usb;
	bool Adc;
} Pl_IsInit_t;

extern Pl_IsInit_t Pl_IsInit;

typedef struct {
	u32 P;
	u32 Q;
	u32 R;
} Pl_Pll_t;

typedef struct {
	u32 SYSCLK;
	u32 HCLK;
	u32 APB1;
	u32 APB2;
	Pl_Pll_t PLL;
	u32 ADC;
} Pl_SysClock_t;

extern Pl_SysClock_t Pl_SysClk;

typedef void (*Pl_Common_Clbk_t)(void);
typedef void (*Pl_HardFault_Clbk_t)(u32 pcVal);
typedef void (*Pl_USART_ClbkTx_t)(void);
typedef void (*Pl_USART_ClbkRx_t)(void);
typedef void (*Pl_Sensors_ADCLowFreqClbk_t)(u16* buffPtr, u16 buffLen);
typedef void (*Pl_ADC_Motor_SensHighFreqClbk_t)(u16* buffPtr, u16 buffLen);
typedef void (*Pl_TIM_Motor_SpeedControlClbk_t)(void);
typedef void (*Pl_Motor_HallToggleTimeCalcClbk_t)(void);

__STATIC_FORCEINLINE void Pl_IrqOn(void) {
	__enable_irq();
}

__STATIC_FORCEINLINE void Pl_IrqOff(void) {
	__disable_irq();
}

void Pl_Stub_CommonClbk(void);
void Pl_Stub_HardFaultClbk(u32 pcVal);
void Pl_Stub_Sensors_ADCLowFreqClbk(u16* buffPtr, u16 buffLen);
void Pl_Stub_Motor_AdcHighFreqClbk(u16* buffPtr, u16 buffLen);

bool Pl_Init(Pl_HardFault_Clbk_t pHardFault_Clbk);

void Pl_SysCpuCnt_Init(void);
u32 Pl_SysCpuCnt_Get(void);

bool Pl_DelayMs_Init(Pl_Common_Clbk_t pDelayTimerClbk);
bool Pl_DelayMs_DeInit(void);
void Pl_DelayMs_SuspendTimer(void);
void Pl_DelayMs_ResumeTimer(void);
u32 Pl_DelayMs_GetUsCnt(void);
u32 Pl_DelayMs_GetMsCnt(void);

void Pl_JumpToAddr(u32 appAddr);
void Pl_SoftReset(void);

void Pl_WatchDog_Init(void);
void Pl_WatchDog_RstCnt(void);

void Pl_Debug_Init(u8* pTxBuff, u16 TxBuffLen, Pl_USART_ClbkTx_t pTxClbk, u8* pRxBuff,
				   u16 RxBuffLen, Pl_USART_ClbkRx_t pRxClbk);
void Pl_Debug_Enable_Tx(void);
void Pl_Debug_Enable_Rx(void);
void Pl_Debug_Disable_Tx(void);
void Pl_Debug_Disable_Rx(void);
void Pl_Debug_SetDataLengthTx(u32 NbData);
void Pl_Debug_SetDataLengthRx(u32 NbData);
u32 Pl_Debug_GetDataLengthTx(void);
u32 Pl_Debug_GetDataLengthRx(void);
u8 Pl_Debug_GetRxByte(void);
RET_STATE_t Pl_Debug_TxData(u8* pBuff, u16 size);

void Pl_Led_Init(void);
void Pl_Led_Set(void);
void Pl_Led_Reset(void);
void Pl_Led_Toggle(void);

#endif /* __PLATFORM_H */

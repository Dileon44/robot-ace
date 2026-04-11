#include "platform.h"
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "int.h"
#include "platform_inc_m0.h"
#include "platform_int_cfg_m0.h"
#include "sys.h"
#include "tim.h"
#include "usart.h"

/**
 * 4 bits for pre-emption priority,
 * 0 bits for subpriority (for FreeRTOS)
 */
#define NVIC_PRIO_GROUP_0 ((u32)0x00000007)
#define NVIC_PRIO_GROUP_1 ((u32)0x00000006)
#define NVIC_PRIO_GROUP_2 ((u32)0x00000005)
#define NVIC_PRIO_GROUP_3 ((u32)0x00000004)
#define NVIC_PRIO_GROUP_4 ((u32)0x00000003)

void Pl_Stub_CommonClbk(void) {
}

void Pl_Stub_HardFaultClbk(u32 pcVal) {
	DISCARD_UNUSED(pcVal);
}

Pl_IsInit_t Pl_IsInit;
Pl_SysClock_t Pl_SysClk;

bool Pl_Init(Pl_HardFault_Clbk_t pHardFault_Clbk) {
	HardFault_SetCallback(pHardFault_Clbk);

	NVIC_SetPriorityGrouping(NVIC_PRIO_GROUP_4);

	Sys_MainClock_Config();

	// Flash caches must be enabled after the correct flash latency is set by Sys_MainClock_Config
	LL_FLASH_EnableInstCache();
	LL_FLASH_EnableDataCache();
	LL_FLASH_EnablePrefetch();

	LL_RCC_ClocksTypeDef RCC_Clocks;
	LL_RCC_GetSystemClocksFreq(&RCC_Clocks);
	Pl_SysClk.SYSCLK = RCC_Clocks.SYSCLK_Frequency;
	Pl_SysClk.HCLK	 = RCC_Clocks.HCLK_Frequency;
	Pl_SysClk.APB1	 = RCC_Clocks.PCLK1_Frequency;
	Pl_SysClk.APB2	 = RCC_Clocks.PCLK2_Frequency;

	Pl_IsInit.Sys = true;
	return Pl_IsInit.Sys;
}

void Pl_SysCpuCnt_Init(void) {
	Pl_IsInit.CpuCounter = Sys_CounterCPU_Init();
	Sys_CounterCPU_Init();
}

u32 Pl_SysCpuCnt_Get(void) {
	return Sys_CounterCPU_Get();
}

u32* Pl_UID_GetStrAndPtr(char* pDst) {
	return Sys_UID_GetStrAndPtr(pDst);
}

void Pl_CPU_GetStrAndPtr(char* pDst) {
	Sys_CPU_GetStrAndPtr(pDst);
}

u32 Pl_MCU_GetFlashSize(void) {
	return Sys_MCU_GetFlashSize();
}

void Pl_JumpToAddr(u32 appAddr) {
	u32 appJumpAddr;
	void (*pGoToApp)(void);

	appJumpAddr = *((volatile u32*)(appAddr + 4));
	pGoToApp	= (void (*)(void))appJumpAddr;
	SCB->VTOR	= appAddr;
	__set_MSP(*((volatile u32*)appAddr));  //stack pointer (to RAM) for app on this addr
	pGoToApp();
}

void Pl_SoftReset(void) {
	Sys_MCU_Reset();
}

const char* Pl_GetRstFlagStr(void) {
	const char* pTmpStr = Sys_ResetFlag_GetStr();
	Sys_ResetFlag_Clear();

	return pTmpStr;
}

bool Pl_DelayMs_Init(Pl_Common_Clbk_t pDelayTimerClbk) {
	Pl_IsInit.DelayMs = TIM_Delay_Init(pDelayTimerClbk);
	Sys_NVIC_SetPrioEnable(TIM_DELAY_IRQ, NVIC_IRQ_PRIO_TIM_DELAY);
	return Pl_IsInit.DelayMs;
}

bool Pl_DelayMs_DeInit(void) {
	Pl_IsInit.DelayMs = !TIM_Delay_DeInit();
	Sys_NVIC_Disable(TIM_DELAY_IRQ);
	return !Pl_IsInit.DelayMs;
	return true;
}

void Pl_DelayMs_SuspendTimer(void) {
	TIM_Delay_Disable();
}

void Pl_DelayMs_ResumeTimer(void) {
	TIM_Delay_Enable();
}

u32 Pl_DelayMs_GetUsCnt(void) {
	return TIM_Delay_GetCnt();
}

u32 Pl_DelayMs_GetMsCnt(void) {
	return TIM_Delay_GetOvrflCnt();
}

void Pl_WatchDog_Init(void) {
	LL_IWDG_Enable(IWDG);
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_128);
	LL_IWDG_SetReloadCounter(IWDG, 3500);
	while (LL_IWDG_IsReady(IWDG) != 1) {
	}

	LL_IWDG_ReloadCounter(IWDG);
}

void Pl_WatchDog_RstCnt(void) {
	LL_IWDG_ReloadCounter(IWDG);
}

void Pl_Debug_Init(u8* pTxBuff, u16 TxBuffLen, Pl_USART_ClbkTx_t pTxClbk, u8* pRxBuff,
				   u16 RxBuffLen, Pl_USART_ClbkRx_t pRxClbk) {
	GPIO_Debug_Init();

	USART_Debug_Init(pRxClbk);
	USART_Debug_Irq_Enable();

	DMA_Debug_Init_Tx((void*)USART_Debug_GetUSART(), pTxBuff, TxBuffLen, pTxClbk);
	DMA_Debug_Init_Rx((void*)USART_Debug_GetUSART(), pRxBuff, RxBuffLen, pRxClbk);
	DMA_Debug_Irq_Enable();
}

RET_STATE_t Pl_Debug_TxData(u8* pBuff, u16 size) {
	return USART_Debug_TxData(pBuff, size, PL_USART_DEF_TMO);
}

void Pl_Debug_Enable_Tx(void) {
	DMA_Debug_Enable_Tx();
}

void Pl_Debug_Enable_Rx(void) {
	DMA_Debug_Enable_Rx();
}

void Pl_Debug_Disable_Tx(void) {
	DMA_Debug_Disable_Tx();
}

void Pl_Debug_Disable_Rx(void) {
	DMA_Debug_Disable_Rx();
}

void Pl_Debug_SetDataLengthRx(u32 NbData) {
	DMA_Debug_SetDataLengthRx(NbData);
}

void Pl_Debug_SetDataLengthTx(u32 NbData) {
	DMA_Debug_SetDataLengthTx(NbData);
}

u32 Pl_Debug_GetDataLengthTx(void) {
	return DMA_Debug_GetDataLengthTx();
}

u32 Pl_Debug_GetDataLengthRx(void) {
	return DMA_Debug_GetDataLengthRx();
}

u8 Pl_Debug_GetRxByte(void) {
	return USART_Debug_GetRxByte();
}

void Pl_Led_Init(void) {
	GPIO_Led_Init();
}

void Pl_Led_Set(void) {
	GPIO_Led_Set();
}

void Pl_Led_Reset(void) {
	GPIO_Led_Reset();
}

void Pl_Led_Toggle(void) {
	GPIO_Led_Toggle();
}

bool Pl_Encoder_Init(void) {
	GPIO_Encoder_Init();
	Pl_IsInit.I2cSensor = I2C_Sensor_Init();
	return Pl_IsInit.I2cSensor;
}

bool Pl_Encoder_DeInit(void) {
	Pl_IsInit.I2cSensor = !I2C_Sensor_DeInit();
	GPIO_Encoder_DeInit();
	return !Pl_IsInit.I2cSensor;
}

void Pl_Encoder_Dir_Set(bool clockwise) {
	GPIO_Encoder_Dir_Set(clockwise);
}

RET_STATE_t Pl_I2cEncoder_Read(u8 devAddr, u8 wordAddr, u8* pData, u16 len) {
	return I2C_Sensor_Read(devAddr, wordAddr, pData, len);
}

RET_STATE_t Pl_I2cEncoder_Write(u8 devAddr, u8 wordAddr, const u8* pData, u16 len) {
	return I2C_Sensor_Write(devAddr, wordAddr, pData, len);
}

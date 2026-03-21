#include "platform.h"
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "int.h"
#include "platform_inc_m0.h"
#include "platform_int_cfg_m0.h"
#include "sys.h"
#include "tim.h"
#include "usart.h"

#define NVIC_PRIO_GROUP_4 \
	0x00000003U /*!< 4 bits for pre-emption priority, 0 bits for subpriority (for FreeRTOS)*/

void Pl_Stub_CommonClbk(void) {
}

void Pl_Stub_HardFaultClbk(u32 pcVal) {
	DISCARD_UNUSED(pcVal);
}

void Pl_Stub_HallClbk(u32 idHall) {
	DISCARD_UNUSED(idHall);
}

void Pl_Stub_Sensors_ADCLowFreqClbk(u16* buffPtr, u16 buffLen) {
	DISCARD_UNUSED(buffPtr);
	DISCARD_UNUSED(buffLen);
}

void Pl_Stub_Motor_AdcHighFreqClbk(u16* buffPtr, u16 buffLen) {
	DISCARD_UNUSED(buffPtr);
	DISCARD_UNUSED(buffLen);
}

void Pl_Init(Pl_HardFault_Clbk_t pHardFault_Clbk) {
	HardFault_SetCallback(pHardFault_Clbk);

	LL_FLASH_EnableInstCache();
	LL_FLASH_EnableDataCache();
	LL_FLASH_EnablePrefetch();
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
	while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
		;
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
	NVIC_SetPriorityGrouping(NVIC_PRIO_GROUP_4);

	Sys_MainClock_Config();
}

void Pl_Delay_Init(Pl_Common_Clbk_t pDelayTimerClbk) {
	TIM_Delay_Init(pDelayTimerClbk);
	TIM_Delay_Irq_Enable();
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

/*========================= Platform IWDG functions ==========================*/

void Pl_IWDG_Init(void) {
	LL_IWDG_Enable(IWDG);
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_128);
	LL_IWDG_SetReloadCounter(IWDG, 3500);
	while (LL_IWDG_IsReady(IWDG) != 1) {
	}

	LL_IWDG_ReloadCounter(IWDG);
}

void Pl_IWDG_ReloadCounter(void) {
	LL_IWDG_ReloadCounter(IWDG);
}

/*========================= Platform USART functions =========================*/

void Pl_USART_Debug_Init(u8* pTxBuff, u16 TxBuffLen, Pl_USART_ClbkTx_t pTxClbk, u8* pRxBuff,
						 u16 RxBuffLen, Pl_USART_ClbkRx_t pRxClbk) {
	GPIO_USART_Debug_Tx_Init();
	GPIO_USART_Debug_Rx_Init();

	USART_Debug_Init(pRxClbk);
	USART_Debug_Irq_Enable();

	DMA_Debug_Init_Tx((void*)USART_Debug_GetUSART(), pTxBuff, TxBuffLen, pTxClbk);
	DMA_Debug_Init_Rx((void*)USART_Debug_GetUSART(), pRxBuff, RxBuffLen, pRxClbk);
	DMA_Debug_Irq_Enable();
}

__INLINE RET_STATE_t Pl_USART_Debug_TxData(u8* pBuff, u16 size) {
	SYS_CRITICAL_ON();
	RET_STATE_t retState = USART_Debug_TxData(pBuff, size, PL_USART_DEF_TIMEOUT);
	SYS_CRITICAL_OFF();
	return retState;
}

__INLINE void Pl_USART_Debug_Enable_Tx(void) {
	DMA_Debug_Enable_Tx();
}

__INLINE void Pl_USART_Debug_Enable_Rx(void) {
	DMA_Debug_Enable_Rx();
}

__INLINE void Pl_USART_Debug_Disable_Tx(void) {
	DMA_Debug_Disable_Tx();
}

__INLINE void Pl_USART_Debug_Disable_Rx(void) {
	DMA_Debug_Disable_Rx();
}

__INLINE void Pl_USART_Debug_SetDataLengthRx(u32 NbData) {
	DMA_Debug_SetDataLengthRx(NbData);
}

__INLINE void Pl_USART_Debug_SetDataLengthTx(u32 NbData) {
	DMA_Debug_SetDataLengthTx(NbData);
}

__INLINE u32 Pl_USART_Debug_GetDataLengthTx(void) {
	return DMA_Debug_GetDataLengthTx();
}

__INLINE u32 Pl_USART_Debug_GetDataLengthRx(void) {
	return DMA_Debug_GetDataLengthRx();
}

__INLINE u8 Pl_USART_Debug_GetRxByte(void) {
	return USART_Debug_GetRxByte();
}

/*========================= Platform Motor functions =========================*/

u32 Pl_Motor_GetStateHalls(void) {
	return GPIO_Motor_GetStateHalls();
}

void Pl_Motor_SetKeyAL(void) {
	GPIO_Motor_SetKeyAL(GPIO_SET);
}

void Pl_Motor_SetKeyBL(void) {
	GPIO_Motor_SetKeyBL(GPIO_SET);
}

void Pl_Motor_SetKeyCL(void) {
	GPIO_Motor_SetKeyCL(GPIO_SET);
}

void Pl_Motor_ResetKeyAL(void) {
	GPIO_Motor_SetKeyAL(GPIO_RST);
}

void Pl_Motor_ResetKeyBL(void) {
	GPIO_Motor_SetKeyBL(GPIO_RST);
}

void Pl_Motor_ResetKeyCL(void) {
	GPIO_Motor_SetKeyCL(GPIO_RST);
}

void Pl_Motor_ControlGpioInit(void) {
	GPIO_Motor_AH_Init();
	GPIO_Motor_BH_Init();
	GPIO_Motor_CH_Init();
	GPIO_Motor_AL_Init();
	GPIO_Motor_BL_Init();
	GPIO_Motor_CL_Init();
}

void Pl_Motor_SensorsInit(Pl_Motor_ClbkHall_t pHallClbk) {
	GPIO_Motor_Hall_Init(pHallClbk);
	GPIO_Motor_HallIrqEnable();
}

void Pl_Motor_PWMInit(Pl_TIM_PWM_Clbk_t pTimPwmCntTopClbk, Pl_TIM_PWM_Clbk_t pTimPwmCntBottomClbk) {
	TIM_PWM_Init(pTimPwmCntTopClbk, pTimPwmCntBottomClbk);
	TIM_PWM_Irq_Enable();
	TIM_PWM_Enable();
}

void Pl_Motor_Init(Pl_TIM_PWM_Clbk_t pTimPwmCntTopClbk, Pl_TIM_PWM_Clbk_t pTimPwmCntBottomClbk,
				   Pl_TIM_Motor_SpeedControlClbk_t pSpeedControlClbk,
				   Pl_ADC_Motor_SensHighFreqClbk_t pAdcHighFreqClbk) {
	Pl_Motor_ControlGpioInit();
	Pl_Motor_PWMInit(pTimPwmCntTopClbk, pTimPwmCntBottomClbk);
	// Pl_Motor_SensorsInit(pHallClbk);
	Pl_TIM_SpeedControl_Init(pSpeedControlClbk);
	Pl_ADC_HighFreqSens_Init(pAdcHighFreqClbk);
}

__INLINE void Pl_Motor_PWMA_Enable(void) {
	TIM_PWMA_Enable();
}

__INLINE void Pl_Motor_PWMA_Disable(void) {
	TIM_PWMA_Disable();
}

__INLINE void Pl_Motor_PWMB_Enable(void) {
	TIM_PWMB_Enable();
}

__INLINE void Pl_Motor_PWMB_Disable(void) {
	TIM_PWMB_Disable();
}

__INLINE void Pl_Motor_PWMC_Enable(void) {
	TIM_PWMC_Enable();
}

__INLINE void Pl_Motor_PWMC_Disable(void) {
	TIM_PWMC_Disable();
}

__INLINE void Pl_Motor_SetComparePWMA(u32 compareValue) {
	TIM_PWMA_SetCompare(compareValue);
}

__INLINE void Pl_Motor_SetComparePWMB(u32 compareValue) {
	TIM_PWMB_SetCompare(compareValue);
}

__INLINE void Pl_Motor_SetComparePWMC(u32 compareValue) {
	TIM_PWMC_SetCompare(compareValue);
}

__INLINE u32 Pl_Motor_GetComparePWM(void) {
	return TIM_PWM_GetCompare();
}

/*======================== Platform Sensors functions ========================*/

void Pl_ADC_LowFreqSens_Init(Pl_Sensors_ADCLowFreqClbk_t pADCLowFreqClbk) {
	TIM_ADCLowFreq_Init();
	TIM_ADCLowFreq_Enable();
	GPIO_ADCLowFreq_Init();
	DMA_ADCLowFreq_Init((void*)ADC_GetLowFreqAdc(), pADCLowFreqClbk);
	DMA_ADCLowFreq_Irq_Enable();
	ADC_LowFreq_Init();
}

void Pl_ADC_HighFreqSens_Init(Pl_ADC_Motor_SensHighFreqClbk_t pAdcHighFreqClbk) {
	GPIO_ADCHighFreq_Init();
	ADC_HighFreq_Init(pAdcHighFreqClbk);
	DMA_AdcHighFreq_Init((void*)ADC_GetHighFreqAdc(), pAdcHighFreqClbk);
	DMA_AdcHighFreq_Irq_Enable();
}

__INLINE u32 Pl_TIM_SpeedControl_GetPrescaler(void) {
	return TIM_SpeedControl_GetPrescaler();
}

__INLINE u32 Pl_TIM_SpeedControl_GetAutoReload(void) {
	return TIM_SpeedControl_GetAutoReload();
}

void Pl_TIM_HallToggleTimeCalc_Init(Pl_Motor_HallToggleTimeCalcClbk_t pHallToggleTimeCalcClbk) {
	TIM_HallToggleTimeCalc_Init(pHallToggleTimeCalcClbk);
	TIM_HallToggleTimeCalc_IrqEnable();
	TIM_HallToggleTimeCalc_Enable();
}

__INLINE void Pl_TIM_HallToggleTimeCalc_Enable(void) {
	TIM_HallToggleTimeCalc_Enable();
}

__INLINE void Pl_TIM_HallToggleTimeCalc_Disable(void) {
	TIM_HallToggleTimeCalc_Disable();
}

__INLINE void Pl_TIM_HallToggleTimeCalc_ResetCnt(void) {
	TIM_HallToggleTimeCalc_SetCnt(0);
}

u32 Pl_TIM_HallToggleTimeCalc_GetCnt(void) {
	return TIM_HallToggleTimeCalc_GetCnt();
}

void Pl_TIM_SpeedControl_Init(Pl_TIM_Motor_SpeedControlClbk_t pSpeedControlClbk) {
	TIM_SpeedControl_Init(pSpeedControlClbk);
	TIM_SpeedControl_Irq_Enable();
	TIM_SpeedControl_Enable();
}

void Pl_DMA_AdcHighFreq_Enable(void) {
	DMA_AdcHighFreq_Enable();
}

__INLINE void Pl_ADC_StartHighFreqConv(void) {
	ADC_StartHighFreqConv();
}

__INLINE void Pl_LedDebug_Init(void) {
	GPIO_LedDebug_Init();
}

__INLINE void Pl_Led_Set(void) {
	GPIO_Led_Set();
}

__INLINE void Pl_Led_Reset(void) {
	GPIO_Led_Reset();
}

__INLINE void Pl_Led_Toggle(void) {
	GPIO_Led_Toggle();
}

void Pl_Hall1_CallIrq(void) {
	GPIO_Hall1_CallIrq();
}
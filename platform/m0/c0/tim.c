#include "tim.h"
#include "gpio.h"
#include "sys.h"

// #define TIM_FREQ					170000000 // RCC_GetHCLKClockFreq(RCC_PLL_GetFreqDomain_SYS())

#define TIM_DELAY			TIM6
#define TIM_DELAY_IRQ		TIM6_DAC_IRQn
#define TIM_DELAY_IRQ_HDL	TIM6_DAC_IRQHandler
#define TIM_DELAY_CLK_EN()	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6)
#define TIM_DELAY_CLK_DIS() LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM6)

#define TIM_PWM			 TIM1
#define TIM_PWMA		 LL_TIM_CHANNEL_CH1
#define TIM_PWMB		 LL_TIM_CHANNEL_CH2
#define TIM_PWMC		 LL_TIM_CHANNEL_CH3
#define TIM_PWM_IRQ		 TIM1_UP_TIM16_IRQn
#define TIM_PWM_IRQ_HDL	 TIM1_UP_TIM16_IRQHandler
#define TIM_PWM_CLK_EN() LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1)

#define TIM_ADC_LOW_FREQ TIM3

#define TIM_SPEED_CONTROL		   TIM7
#define TIM_SPEED_CONTROL_IRQ	   TIM7_IRQn
#define TIM_SPEED_CONTROL_IRQ_HDL  TIM7_IRQHandler
#define TIM_SPEED_CONTROL_CLK_EN() LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7)

#define TIM_HALL_TOGGLE_TIME_CALC		   TIM15
#define TIM_HALL_TOGGLE_TIME_CALC_IRQ	   TIM1_BRK_TIM15_IRQn
#define TIM_HALL_TOGGLE_TIME_CALC_IRQ_HDL  TIM1_BRK_TIM15_IRQHandler
#define TIM_HALL_TOGGLE_TIME_CALC_CLK_EN() LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM15)

static Pl_Common_Clbk_t DelayTimerClbk									  = Pl_Stub_CommonClbk;
static Pl_Motor_HallToggleTimeCalcClbk_t TIM_Motor_HallToggleTimeCalcClbk = Pl_Stub_CommonClbk;
static Pl_TIM_PWM_Clbk_t TIM_PwmCntTopClbk								  = Pl_Stub_CommonClbk;
static Pl_TIM_PWM_Clbk_t TIM_PwmCntBottomClbk							  = Pl_Stub_CommonClbk;
static Pl_TIM_Motor_SpeedControlClbk_t TIM_Motor_SpeedControlClbk		  = Pl_Stub_CommonClbk;
volatile static u32 TimDelayOverflowsCnt								  = 0;

void TIM_Delay_Init(Pl_Common_Clbk_t pDelayTimerClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(DelayTimerClbk, pDelayTimerClbk);
	TIM_DELAY_CLK_EN();

	LL_TIM_InitTypeDef TIM_InitStruct;
	TIM_InitStruct.Prescaler   = __LL_TIM_CALC_PSC(SystemCoreClock, FREQ_1_MHZ);
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload =
		__LL_TIM_CALC_ARR(SystemCoreClock, TIM_InitStruct.Prescaler, FREQ_1_KHZ);
	TIM_InitStruct.ClockDivision	 = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM_DELAY, &TIM_InitStruct);
	LL_TIM_EnableIT_UPDATE(TIM_DELAY);
	LL_TIM_EnableCounter(TIM_DELAY);
}

bool TIM_Delay_DeInit(void) {
	LL_TIM_DisableCounter(TIM_DELAY);
	LL_TIM_DisableIT_UPDATE(TIM_DELAY);

	LL_TIM_InitTypeDef TIM_InitStruct;
	LL_TIM_StructInit(&TIM_InitStruct);
	LL_TIM_Init(TIM_DELAY, &TIM_InitStruct);
	TimDelayOverflowsCnt = 0;
	DelayTimerClbk		 = Pl_Stub_CommonClbk;

	TIM_DELAY_CLK_DIS();

	return true;
}

__INLINE void TIM_Delay_Disable(void) {
	LL_TIM_DisableCounter(TIM_DELAY);
}

__INLINE void TIM_Delay_Enable(void) {
	LL_TIM_EnableCounter(TIM_DELAY);
}

__INLINE u32 TIM_Delay_GetCnt(void) {
	return LL_TIM_GetCounter(TIM_DELAY);
}

__INLINE u32 TIM_Delay_GetOvrflCnt(void) {
	return TimDelayOverflowsCnt;
}

void TIM_Delay_Irq_Enable(void) {
	Sys_NVIC_SetPrioEnable(TIM_DELAY_IRQ, NVIC_IRQ_PRIO_TIM_DELAY);
}

void TIM_DELAY_IRQ_HDL(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM_DELAY)) {
		DelayTimerClbk();
		TimDelayOverflowsCnt++;
		LL_TIM_ClearFlag_UPDATE(TIM_DELAY);
	}
}

void TIM_PWM_Init(Pl_TIM_PWM_Clbk_t pTimPwmCntTopClbk, Pl_TIM_PWM_Clbk_t pTimPwmCntBottomClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(TIM_PwmCntTopClbk, pTimPwmCntTopClbk);
	ASSIGN_NOT_NULL_VAL_TO_PTR(TIM_PwmCntBottomClbk, pTimPwmCntBottomClbk);
	TIM_PWM_CLK_EN();

	LL_TIM_InitTypeDef TIM_InitStruct		   = {0};
	LL_TIM_OC_InitTypeDef TIM_OC_InitStruct	   = {0};
	LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

	u32 autoreloadTest =
		__LL_TIM_CALC_ARR(170000000, TIM_InitStruct.Prescaler, 12500) / 2;	// 13599
	// u32 compareTest = autoreloadTest / 2;

	TIM_InitStruct.Prescaler   = 1;	 // __LL_TIM_CALC_PSC(SystemCoreClock, 2 * FREQ_1_MHZ); //
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_CENTER_UP_DOWN;	 // LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload =
		autoreloadTest;	 // __LL_TIM_CALC_ARR(SystemCoreClock, TIM_InitStruct.Prescaler, 12500); //
	TIM_InitStruct.ClockDivision	 = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM_PWM, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(TIM_PWM);
	LL_TIM_OC_EnablePreload(TIM_PWM, TIM_PWMA);
	TIM_OC_InitStruct.OCMode	   = LL_TIM_OCMODE_PWM1;
	TIM_OC_InitStruct.OCState	   = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState	   = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 0;
	TIM_OC_InitStruct.OCPolarity   = LL_TIM_OCPOLARITY_HIGH;
	TIM_OC_InitStruct.OCNPolarity  = LL_TIM_OCPOLARITY_HIGH;
	TIM_OC_InitStruct.OCIdleState  = LL_TIM_OCIDLESTATE_LOW;
	TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
	LL_TIM_OC_Init(TIM_PWM, TIM_PWMA, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM_PWM, TIM_PWMA);
	LL_TIM_OC_EnablePreload(TIM_PWM, TIM_PWMB);
	LL_TIM_OC_Init(TIM_PWM, TIM_PWMB, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM_PWM, TIM_PWMB);
	LL_TIM_OC_EnablePreload(TIM_PWM, TIM_PWMC);
	LL_TIM_OC_Init(TIM_PWM, TIM_PWMC, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM_PWM, TIM_PWMC);
	LL_TIM_SetTriggerOutput(TIM_PWM, LL_TIM_TRGO_RESET);
	LL_TIM_SetTriggerOutput2(TIM_PWM, LL_TIM_TRGO2_RESET);
	LL_TIM_DisableMasterSlaveMode(TIM_PWM);
	TIM_BDTRInitStruct.OSSRState	   = LL_TIM_OSSR_DISABLE;
	TIM_BDTRInitStruct.OSSIState	   = LL_TIM_OSSI_DISABLE;
	TIM_BDTRInitStruct.LockLevel	   = LL_TIM_LOCKLEVEL_OFF;
	TIM_BDTRInitStruct.DeadTime		   = 0;
	TIM_BDTRInitStruct.BreakState	   = LL_TIM_BREAK_DISABLE;
	TIM_BDTRInitStruct.BreakPolarity   = LL_TIM_BREAK_POLARITY_HIGH;
	TIM_BDTRInitStruct.BreakFilter	   = LL_TIM_BREAK_FILTER_FDIV1;
	TIM_BDTRInitStruct.BreakAFMode	   = LL_TIM_BREAK_AFMODE_INPUT;
	TIM_BDTRInitStruct.Break2State	   = LL_TIM_BREAK2_DISABLE;
	TIM_BDTRInitStruct.Break2Polarity  = LL_TIM_BREAK2_POLARITY_HIGH;
	TIM_BDTRInitStruct.Break2Filter	   = LL_TIM_BREAK2_FILTER_FDIV1;
	TIM_BDTRInitStruct.Break2AFMode	   = LL_TIM_BREAK_AFMODE_INPUT;
	TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_ENABLE;
	LL_TIM_BDTR_Init(TIM_PWM, &TIM_BDTRInitStruct);

	LL_TIM_EnableIT_UPDATE(TIM_PWM);

	// TIM_PWMA_Enable();
	// TIM_PWMB_Enable();
	// TIM_PWMC_Enable();
}

__INLINE void TIM_PWM_Disable(void) {
	LL_TIM_DisableCounter(TIM_PWM);
}

__INLINE void TIM_PWM_Enable(void) {
	LL_TIM_EnableCounter(TIM_PWM);
}

__INLINE u32 TIM_PWM_GetCnt(void) {
	return LL_TIM_GetCounter(TIM_PWM);
}

__INLINE void TIM_PWM_SetCnt(u32 counter) {
	LL_TIM_SetCounter(TIM_PWM, counter);
}

__INLINE void TIM_PWMA_Disable(void) {
	LL_TIM_CC_DisableChannel(TIM_PWM, TIM_PWMA);
}

__INLINE void TIM_PWMA_Enable(void) {
	LL_TIM_CC_EnableChannel(TIM_PWM, TIM_PWMA);
}

__INLINE void TIM_PWMA_SetCompare(u32 compareValue) {
	LL_TIM_OC_SetCompareCH1(TIM_PWM, compareValue);
}

__INLINE void TIM_PWMB_Disable(void) {
	LL_TIM_CC_DisableChannel(TIM_PWM, TIM_PWMB);
}

__INLINE void TIM_PWMB_Enable(void) {
	LL_TIM_CC_EnableChannel(TIM_PWM, TIM_PWMB);
}

__INLINE void TIM_PWMB_SetCompare(u32 compareValue) {
	LL_TIM_OC_SetCompareCH2(TIM_PWM, compareValue);
}

__INLINE void TIM_PWMC_Disable(void) {
	LL_TIM_CC_DisableChannel(TIM_PWM, TIM_PWMC);
}

__INLINE void TIM_PWMC_Enable(void) {
	LL_TIM_CC_EnableChannel(TIM_PWM, TIM_PWMC);
}

__INLINE void TIM_PWMC_SetCompare(u32 compareValue) {
	LL_TIM_OC_SetCompareCH3(TIM_PWM, compareValue);
}

__INLINE u32 TIM_PWM_GetCompare(void) {
	return LL_TIM_OC_GetCompareCH1(TIM_PWM);
}

void TIM_PWM_Irq_Enable(void) {
	Sys_NVIC_SetPrioEnable(TIM_PWM_IRQ, NVIC_IRQ_PRIO_TIM_SPEED_CONTROL);

	// Sys_NVIC_SetPrioEnable(TIM_PWM_IRQ, NVIC_IRQ_PRIO_TIM_SPEED_CONTROL);
	// Sys_NVIC_SetPrioEnable(TIM_PWM_IRQ, NVIC_IRQ_PRIO_TIM_SPEED_CONTROL);

	// NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
	// NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
	// NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
	// NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);
}

void TIM_PWM_IRQ_HDL(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM_PWM)) {
		if (LL_TIM_GetCounterMode(TIM1) == LL_TIM_COUNTERMODE_CENTER_UP_DOWN) {
			// Проверка направления счёта: вверх или вниз
			if (LL_TIM_GetDirection(TIM1) == LL_TIM_COUNTERDIRECTION_UP) {
				TIM_PwmCntTopClbk();
			} else {
				TIM_PwmCntBottomClbk();
			}
		}

		LL_TIM_ClearFlag_UPDATE(TIM_PWM);
	}
}

void TIM_ADCLowFreq_Init(void) {
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

	LL_TIM_InitTypeDef TIM_InitStruct = {0};
	TIM_InitStruct.Prescaler		  = 260;
	TIM_InitStruct.CounterMode		  = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload		  = 65384;
	TIM_InitStruct.ClockDivision	  = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM_ADC_LOW_FREQ, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(TIM_ADC_LOW_FREQ);
	LL_TIM_SetClockSource(TIM_ADC_LOW_FREQ, LL_TIM_CLOCKSOURCE_INTERNAL);
	LL_TIM_SetTriggerOutput(TIM_ADC_LOW_FREQ, LL_TIM_TRGO_UPDATE);
	LL_TIM_DisableMasterSlaveMode(TIM_ADC_LOW_FREQ);
}

__INLINE void TIM_ADCLowFreq_Enable(void) {
	LL_TIM_EnableCounter(TIM_ADC_LOW_FREQ);
}

__INLINE void TIM_ADCLowFreq_Disable(void) {
	LL_TIM_DisableCounter(TIM_ADC_LOW_FREQ);
}

__INLINE void TIM_ADCLowFreq_SetPrescaler(uint32_t prescaler) {
	LL_TIM_SetPrescaler(TIM_ADC_LOW_FREQ, prescaler);
}

__INLINE void TIM_ADCLowFreq_SetAutoReload(uint32_t autoReload) {
	LL_TIM_SetAutoReload(TIM_ADC_LOW_FREQ, autoReload);
}

void TIM_HallToggleTimeCalc_Init(Pl_Motor_HallToggleTimeCalcClbk_t pHallToggleTimeCalcClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(TIM_Motor_HallToggleTimeCalcClbk, pHallToggleTimeCalcClbk);

	LL_TIM_InitTypeDef TIM_InitStruct = {0};

	TIM_HALL_TOGGLE_TIME_CALC_CLK_EN();

	TIM_InitStruct.Prescaler		 = 1297;
	TIM_InitStruct.CounterMode		 = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload		 = 65535;
	TIM_InitStruct.ClockDivision	 = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM_HALL_TOGGLE_TIME_CALC, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(TIM_HALL_TOGGLE_TIME_CALC);
	LL_TIM_SetClockSource(TIM_HALL_TOGGLE_TIME_CALC, LL_TIM_CLOCKSOURCE_INTERNAL);
	LL_TIM_SetTriggerOutput(TIM_HALL_TOGGLE_TIME_CALC, LL_TIM_TRGO_RESET);
	LL_TIM_DisableMasterSlaveMode(TIM_HALL_TOGGLE_TIME_CALC);
	LL_TIM_EnableIT_UPDATE(TIM_HALL_TOGGLE_TIME_CALC);
}

void TIM_HallToggleTimeCalc_IrqEnable(void) {
	Sys_NVIC_SetPrioEnable(TIM_HALL_TOGGLE_TIME_CALC_IRQ, NVIC_IRQ_PRIO_TIMER_HALL_TOGGLE);
}

__INLINE void TIM_HallToggleTimeCalc_Enable(void) {
	LL_TIM_EnableCounter(TIM_HALL_TOGGLE_TIME_CALC);
}

__INLINE void TIM_HallToggleTimeCalc_Disable(void) {
	LL_TIM_DisableCounter(TIM_HALL_TOGGLE_TIME_CALC);
}

__INLINE void TIM_HallToggleTimeCalc_SetCnt(u32 cntValue) {
	LL_TIM_SetCounter(TIM_HALL_TOGGLE_TIME_CALC, cntValue);
}

u32 TIM_HallToggleTimeCalc_GetCnt(void) {
	return LL_TIM_GetCounter(TIM_HALL_TOGGLE_TIME_CALC);
}

void TIM_HALL_TOGGLE_TIME_CALC_IRQ_HDL(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM_HALL_TOGGLE_TIME_CALC)) {
		TIM_Motor_HallToggleTimeCalcClbk();
		LL_TIM_ClearFlag_UPDATE(TIM_HALL_TOGGLE_TIME_CALC);
	}
}

// void TIM_SpeedControl1_Init(Pl_Motor_HallToggleTimeCalcClbk_t pHallToggleTimeCalcClbk) {
// 	ASSIGN_NOT_NULL_VAL_TO_PTR(TIM_Motor_HallToggleTimeCalcClbk, pHallToggleTimeCalcClbk);

// 	LL_TIM_InitTypeDef TIM_InitStruct = { 0 };

// 	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM17);
// 	NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
// 	NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);
// 	TIM_InitStruct.Prescaler = 2594;
// 	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
// 	TIM_InitStruct.Autoreload = 65535;
// 	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
// 	TIM_InitStruct.RepetitionCounter = 0;
// 	LL_TIM_Init(TIM17, &TIM_InitStruct);
// 	LL_TIM_EnableARRPreload(TIM17);

// 	LL_TIM_SetTriggerOutput(TIM17, LL_TIM_TRGO_RESET);
// 	LL_TIM_DisableMasterSlaveMode(TIM17);
// 	LL_TIM_EnableIT_UPDATE(TIM17);
// }

void TIM_SpeedControl_Irq_Enable(void) {
	Sys_NVIC_SetPrioEnable(TIM_SPEED_CONTROL_IRQ, NVIC_IRQ_PRIO_TIM_SPEED_CONTROL);
}

void TIM_SpeedControl_Init(Pl_TIM_Motor_SpeedControlClbk_t pSpeedControlClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(TIM_Motor_SpeedControlClbk, pSpeedControlClbk);

	TIM_SPEED_CONTROL_CLK_EN();

	// LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);
	// NVIC_SetPriority(TIM7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
	// NVIC_EnableIRQ(TIM7_IRQn);

	LL_TIM_InitTypeDef TIM_InitStruct = {0};
	TIM_InitStruct.Prescaler		  = 2;
	TIM_InitStruct.CounterMode		  = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload		  = 56665;
	LL_TIM_Init(TIM_SPEED_CONTROL, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(TIM_SPEED_CONTROL);
	LL_TIM_SetTriggerOutput(TIM_SPEED_CONTROL, LL_TIM_TRGO_RESET);
	LL_TIM_DisableMasterSlaveMode(TIM_SPEED_CONTROL);
	LL_TIM_EnableIT_UPDATE(TIM_SPEED_CONTROL);
	TIM_SpeedControl_Enable();
}

__INLINE void TIM_SpeedControl_Enable(void) {
	LL_TIM_EnableCounter(TIM_SPEED_CONTROL);
}

__INLINE void TIM_SpeedControl_Disable(void) {
	LL_TIM_DisableCounter(TIM_SPEED_CONTROL);
}

__INLINE void TIM_SpeedControl_SetPrescaler(u32 prescaler) {
	LL_TIM_SetPrescaler(TIM_SPEED_CONTROL, prescaler);
}

__INLINE void TIM_SpeedControl_SetAutoReload(u32 autoReload) {
	LL_TIM_SetAutoReload(TIM_SPEED_CONTROL, autoReload);
}

__INLINE u32 TIM_SpeedControl_GetPrescaler(void) {
	return LL_TIM_GetPrescaler(TIM_SPEED_CONTROL);
}

__INLINE u32 TIM_SpeedControl_GetAutoReload(void) {
	return LL_TIM_GetAutoReload(TIM_SPEED_CONTROL);
}

void TIM_SPEED_CONTROL_IRQ_HDL(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM_SPEED_CONTROL)) {
		TIM_Motor_SpeedControlClbk();
		LL_TIM_ClearFlag_UPDATE(TIM_SPEED_CONTROL);
	}
}
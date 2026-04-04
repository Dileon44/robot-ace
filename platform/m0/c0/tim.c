#include "tim.h"

#define TIM_DELAY			TIM6
#define TIM_DELAY_IRQ_HDL	TIM6_DAC_IRQHandler
#define TIM_DELAY_CLK_EN()	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6)
#define TIM_DELAY_CLK_DIS() LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM6)

static Pl_Common_Clbk_t TimDelay_Clbk	  = Pl_Stub_CommonClbk;
volatile static u32 TimDelay_OverflowsCnt = 0;

bool TIM_Delay_Init(Pl_Common_Clbk_t pDelayTimerClbk) {
	if (!Pl_IsInit.Sys) {
		PANIC();
		return false;
	}

	ASSIGN_NOT_NULL_VAL_TO_PTR(TimDelay_Clbk, pDelayTimerClbk);

	LL_TIM_InitTypeDef TIM_InitStruct;
	LL_TIM_StructInit(&TIM_InitStruct);

	TIM_DELAY_CLK_EN();
	u32 timClkAPB1 = Pl_SysClk.APB1;

	TIM_InitStruct.Prescaler   = __LL_TIM_CALC_PSC(timClkAPB1, FREQ_1_MHZ);
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = __LL_TIM_CALC_ARR(timClkAPB1, TIM_InitStruct.Prescaler, FREQ_1_KHZ);
	TIM_InitStruct.ClockDivision	 = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM_DELAY, &TIM_InitStruct);

	LL_TIM_EnableIT_UPDATE(TIM_DELAY);
	LL_TIM_EnableCounter(TIM_DELAY);

	return true;
}

bool TIM_Delay_DeInit(void) {
	LL_TIM_DisableCounter(TIM_DELAY);
	LL_TIM_DisableIT_UPDATE(TIM_DELAY);

	LL_TIM_InitTypeDef TIM_InitStruct;
	LL_TIM_StructInit(&TIM_InitStruct);
	LL_TIM_Init(TIM_DELAY, &TIM_InitStruct);
	TimDelay_OverflowsCnt = 0;
	TimDelay_Clbk		  = Pl_Stub_CommonClbk;

	TIM_DELAY_CLK_DIS();

	return true;
}

void TIM_Delay_Disable(void) {
	LL_TIM_DisableCounter(TIM_DELAY);
}

void TIM_Delay_Enable(void) {
	LL_TIM_EnableCounter(TIM_DELAY);
}

__INLINE u32 TIM_Delay_GetCnt(void) {
	return LL_TIM_GetCounter(TIM_DELAY);
}

__INLINE u32 TIM_Delay_GetOvrflCnt(void) {
	return TimDelay_OverflowsCnt;
}

void TIM_DELAY_IRQ_HDL(void) {
	if (LL_TIM_IsActiveFlag_UPDATE(TIM_DELAY)) {
		TimDelay_OverflowsCnt++;
		TimDelay_Clbk();
		LL_TIM_ClearFlag_UPDATE(TIM_DELAY);
	}
}

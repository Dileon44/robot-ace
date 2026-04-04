#ifndef __TIM_H
#define __TIM_H

#include "main.h"
#include "platform.h"

#define TIM_DELAY_IRQ TIM6_DAC_IRQn

bool TIM_Delay_Init(Pl_Common_Clbk_t pDelayTimerClbk);
bool TIM_Delay_DeInit(void);
void TIM_Delay_Disable(void);
void TIM_Delay_Enable(void);
u32 TIM_Delay_GetCnt(void);
u32 TIM_Delay_GetOvrflCnt(void);
#endif /* __TIM_H */
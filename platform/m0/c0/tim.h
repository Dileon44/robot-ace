#ifndef __TIM_H
#define __TIM_H

#include "main.h"
#include "platform.h"

void TIM_Delay_Init(Pl_Common_Clbk_t pDelayTimerClbk);
void TIM_Delay_Disable(void);
void TIM_Delay_Enable(void);
u32 TIM_Delay_GetCnt(void);
void TIM_Delay_Irq_Enable(void);

void TIM_PWM_Init(Pl_TIM_PWM_Clbk_t pTimPwmCntTopClbk, Pl_TIM_PWM_Clbk_t pTimPwmCntBottomClbk);
u32 TIM_PWM_GetCnt(void);
void TIM_PWM_SetCnt(u32 counter);
void TIM_PWM_Disable(void);
void TIM_PWM_Enable(void);
void TIM_PWMA_Disable(void);
void TIM_PWMA_Enable(void);
void TIM_PWMA_SetCompare(u32 compareValue);
void TIM_PWMB_Disable(void);
void TIM_PWMB_Enable(void);
void TIM_PWMB_SetCompare(u32 compareValue);
void TIM_PWMC_Disable(void);
void TIM_PWMC_Enable(void);
void TIM_PWMC_SetCompare(u32 compareValue);
u32 TIM_PWM_GetCompare(void);
void TIM_PWM_Irq_Enable(void);

void TIM_ADCLowFreq_Init(void);
void TIM_ADCLowFreq_Enable(void);
void TIM_ADCLowFreq_Disable(void);
void TIM_ADCLowFreq_SetPrescaler(u32 prescaler);
void TIM_ADCLowFreq_SetAutoReload(u32 AutoReload);

void TIM_SpeedControl_Init(Pl_TIM_Motor_SpeedControlClbk_t pSpeedControlClbk);
void TIM_SpeedControl_Enable(void);
void TIM_SpeedControl_Disable(void);
void TIM_SpeedControl_SetPrescaler(u32 prescaler);
void TIM_SpeedControl_SetAutoReload(u32 autoReload);
u32 TIM_SpeedControl_GetPrescaler(void);
u32 TIM_SpeedControl_GetAutoReload(void);
void TIM_SpeedControl_Irq_Enable(void);

void TIM_HallToggleTimeCalc_Init(Pl_Motor_HallToggleTimeCalcClbk_t pHallToggleTimeCalcClbk);
void TIM_HallToggleTimeCalc_IrqEnable(void);
void TIM_HallToggleTimeCalc_Enable(void);
void TIM_HallToggleTimeCalc_Disable(void);
void TIM_HallToggleTimeCalc_SetCnt(u32 cntValue);
u32 TIM_HallToggleTimeCalc_GetCnt(void);

#endif /* __TIM_H */
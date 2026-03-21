#ifndef __ADC_H
#define __ADC_H

#include "main.h"
#include "platform.h"
#include "platform_inc_m0.h"

void ADC_LowFreq_Init(void);
ADC_TypeDef* ADC_GetLowFreqAdc(void);
u16* ADC_GetLowFreqSensBuffPtr(void);
u16 ADC_GetLowFreqSensBuffLen(void);

void ADC_HighFreq_Init(Pl_ADC_Motor_SensHighFreqClbk_t pAdcHighFreqClbk);
ADC_TypeDef* ADC_GetHighFreqAdc(void);
u16* ADC_GetHighFreqSensBuffPtr(void);
u16 ADC_GetHighFreqSensBuffLen(void);

u16 ADC_CalcDigitToVoltage(u16 digit);
void ADC_StartHighFreqConv(void);

#endif /* __ADC_H */
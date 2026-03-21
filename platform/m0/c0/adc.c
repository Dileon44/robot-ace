#include "adc.h"

#define ADC_SENS_HIGH_FREQ_VREF_mV	  3300
#define ADC_SENS_HIGH_FREQ_RESOLUTION LL_ADC_RESOLUTION_12B

#define ADC_SENS_LOW_FREQ		  ADC1
#define ADC_LOW_FREQ_CHANNELS_NUM 2
#define ADC_LOW_FREQ_SAMPLE_TIME  LL_ADC_SAMPLINGTIME_2CYCLES_5
#define ADC_THROTTLE_CH			  LL_ADC_CHANNEL_1
#define ADC_BRAKE_CH			  LL_ADC_CHANNEL_2

#define ADC_SENS_HIGH_FREQ		   ADC2
#define ADC_HIGH_FREQ_CHANNELS_NUM 3
#define ADC_HIGH_FREQ_SAMPLE_TIME  LL_ADC_SAMPLINGTIME_2CYCLES_5
#define ADC_CUR_A_CH			   LL_ADC_CHANNEL_14
#define ADC_CUR_B_CH			   LL_ADC_CHANNEL_12
#define ADC_CUR_C_CH			   LL_ADC_CHANNEL_5
// #define ADC_TEPM_MOTOR_CH   LL_ADC_CHANNEL_3
// #define ADC_TEPM_MOSFET_CH  LL_ADC_CHANNEL_5

#define ADC_SENS_IRQ_HDL  ADC1_2_IRQHandler
#define ADC_SENS_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12)

u16 ADC_LowFreq_Buff[ADC_LOW_FREQ_CHANNELS_NUM];
u16 ADC_HighFreq_Buff[ADC_HIGH_FREQ_CHANNELS_NUM];

static Pl_ADC_Motor_SensHighFreqClbk_t ADC_SensHighFreqClbk = Pl_Stub_Motor_AdcHighFreqClbk;

u16 ADC_CalcDigitToVoltage(u16 digit) {
	return __LL_ADC_CALC_DATA_TO_VOLTAGE(ADC_SENS_HIGH_FREQ_VREF_mV, digit,
										 ADC_SENS_HIGH_FREQ_RESOLUTION);
}

ADC_TypeDef* ADC_GetLowFreqAdc(void) {
	return ADC_SENS_LOW_FREQ;
}

u16* ADC_GetLowFreqSensBuffPtr(void) {
	return (u16*)ADC_LowFreq_Buff;
}

u16 ADC_GetLowFreqSensBuffLen(void) {
	return NUM_ELEMENTS(ADC_LowFreq_Buff);
}

void ADC_LowFreq_Init(void) {

	LL_ADC_InitTypeDef ADC_InitStruct			  = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct	  = {0};
	LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

	LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_PLL);
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12);

	ADC_InitStruct.Resolution	 = LL_ADC_RESOLUTION_12B;
	ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADC_InitStruct.LowPowerMode	 = LL_ADC_LP_MODE_NONE;
	LL_ADC_Init(ADC_SENS_LOW_FREQ, &ADC_InitStruct);
	ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM3_TRGO;
	ADC_REG_InitStruct.SequencerLength =
		LL_ADC_REG_SEQ_SCAN_ENABLE_2RANKS;	// LL_ADC_REG_SEQ_SCAN_DISABLE;
	ADC_REG_InitStruct.SequencerDiscont =
		LL_ADC_REG_SEQ_DISCONT_2RANKS;	// LL_ADC_REG_SEQ_DISCONT_DISABLE;
	ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
	ADC_REG_InitStruct.DMATransfer	  = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
	ADC_REG_InitStruct.Overrun		  = LL_ADC_REG_OVR_DATA_PRESERVED;
	LL_ADC_REG_Init(ADC_SENS_LOW_FREQ, &ADC_REG_InitStruct);
	LL_ADC_SetGainCompensation(ADC_SENS_LOW_FREQ, 0);
	LL_ADC_SetOverSamplingScope(ADC_SENS_LOW_FREQ, LL_ADC_OVS_DISABLE);
	ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
	LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC_SENS_LOW_FREQ), &ADC_CommonInitStruct);
	LL_ADC_REG_SetTriggerEdge(ADC_SENS_LOW_FREQ, LL_ADC_REG_TRIG_EXT_RISING);

	LL_ADC_DisableDeepPowerDown(ADC_SENS_LOW_FREQ);
	LL_ADC_EnableInternalRegulator(ADC_SENS_LOW_FREQ);

	uint32_t wait_loop_index;
	wait_loop_index =
		((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
	while (wait_loop_index != 0) {
		wait_loop_index--;
	}

	LL_ADC_REG_SetSequencerRanks(ADC_SENS_LOW_FREQ, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_1);
	LL_ADC_SetChannelSamplingTime(ADC_SENS_LOW_FREQ, LL_ADC_CHANNEL_1, ADC_LOW_FREQ_SAMPLE_TIME);
	LL_ADC_SetChannelSingleDiff(ADC_SENS_LOW_FREQ, LL_ADC_CHANNEL_1, LL_ADC_SINGLE_ENDED);

	LL_ADC_REG_SetSequencerRanks(ADC_SENS_LOW_FREQ, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_2);
	LL_ADC_SetChannelSamplingTime(ADC_SENS_LOW_FREQ, LL_ADC_CHANNEL_2, ADC_LOW_FREQ_SAMPLE_TIME);
	LL_ADC_SetChannelSingleDiff(ADC_SENS_LOW_FREQ, LL_ADC_CHANNEL_2, LL_ADC_SINGLE_ENDED);

	// LL_ADC_EnableIT_EOS(ADC_SENS_LOW_FREQ);
	// LL_ADC_EnableIT_EOC(ADC_SENS_LOW_FREQ);
	LL_ADC_Enable(ADC_SENS_LOW_FREQ);
	LL_ADC_REG_StartConversion(ADC_SENS_LOW_FREQ);
}

// void ADC1_2_IRQHandler(void) {
//     if(LL_ADC_IsActiveFlag_EOS(ADC_SENS_LOW_FREQ)) {
//         LL_ADC_ClearFlag_EOS(ADC_SENS_LOW_FREQ);
//         volatile u32 value = LL_ADC_REG_ReadConversionData32(ADC_SENS_LOW_FREQ);
//         u16* buffADC = ADC_GetLowFreqSensBuffPtr();
// 		u16 buffLen = ADC_GetLowFreqSensBuffLen();
//         ADC_SensLowFreqClbk(buffADC, buffLen);

//         // u16 tmp = LL_ADC_REG_ReadConversionData12(ADC_SENS_LOW_FREQ);
// 		// u16* buffADC = ADC_GetLowFreqSensBuffPtr();
// 		// u16 buffLen = ADC_GetLowFreqSensBuffLen();
// 		// for(u32 idx = 0; idx < buffLen; idx++) {
//         //     // u16 tmp = ADC_CalcDigitToVoltage(buffADC[idx]);
// 		// 	buffADC[idx] = tmp;
// 		// }
// 		// ADC_SensLowFreqClbk(buffADC, buffLen);
// 	}
// }

/* HDL for single ADC without DMA */
// void ADC1_2_IRQHandler(void) {
//     if (LL_ADC_IsActiveFlag_EOS(ADC_SENS_LOW_FREQ)) {
//         LL_ADC_ClearFlag_EOS(ADC_SENS_LOW_FREQ);
//         volatile u32 value = LL_ADC_REG_ReadConversionData32(ADC_SENS_LOW_FREQ);
//         u16* buffADC = ADC_GetLowFreqSensBuffPtr();
// 		u16 buffLen = ADC_GetLowFreqSensBuffLen();
//         ADC_SensLowFreqClbk(buffADC, buffLen);
//     }
// }

void ADC_HighFreq_Init(Pl_ADC_Motor_SensHighFreqClbk_t pAdcHighFreqClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(ADC_SensHighFreqClbk, pAdcHighFreqClbk);

	LL_ADC_InitTypeDef ADC_InitStruct			  = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct	  = {0};
	LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

	LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_PLL);
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12);

	ADC_InitStruct.Resolution	 = LL_ADC_RESOLUTION_12B;
	ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADC_InitStruct.LowPowerMode	 = LL_ADC_LP_MODE_NONE;
	LL_ADC_Init(ADC_SENS_HIGH_FREQ, &ADC_InitStruct);
	ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
	ADC_REG_InitStruct.SequencerLength =
		LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS;	// LL_ADC_REG_SEQ_SCAN_DISABLE;
	ADC_REG_InitStruct.SequencerDiscont =
		LL_ADC_REG_SEQ_DISCONT_3RANKS;	// LL_ADC_REG_SEQ_DISCONT_DISABLE;
	ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
	ADC_REG_InitStruct.DMATransfer =
		LL_ADC_REG_DMA_TRANSFER_UNLIMITED;	// LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
	ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
	LL_ADC_REG_Init(ADC_SENS_HIGH_FREQ, &ADC_REG_InitStruct);
	LL_ADC_SetGainCompensation(ADC_SENS_HIGH_FREQ, 0);
	LL_ADC_SetOverSamplingScope(ADC_SENS_HIGH_FREQ, LL_ADC_OVS_DISABLE);

	ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
	ADC_CommonInitStruct.Multimode	 = LL_ADC_MULTI_INDEPENDENT;
	LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC_SENS_HIGH_FREQ), &ADC_CommonInitStruct);

	LL_ADC_REG_SetSequencerRanks(ADC_SENS_HIGH_FREQ, LL_ADC_REG_RANK_1, ADC_CUR_A_CH);
	LL_ADC_SetChannelSamplingTime(ADC_SENS_HIGH_FREQ, ADC_CUR_A_CH, ADC_HIGH_FREQ_SAMPLE_TIME);
	LL_ADC_SetChannelSingleDiff(ADC_SENS_HIGH_FREQ, ADC_CUR_A_CH, LL_ADC_SINGLE_ENDED);

	LL_ADC_REG_SetSequencerRanks(ADC_SENS_HIGH_FREQ, LL_ADC_REG_RANK_2, ADC_CUR_B_CH);
	LL_ADC_SetChannelSamplingTime(ADC_SENS_HIGH_FREQ, ADC_CUR_B_CH, ADC_HIGH_FREQ_SAMPLE_TIME);
	LL_ADC_SetChannelSingleDiff(ADC_SENS_HIGH_FREQ, ADC_CUR_B_CH, LL_ADC_SINGLE_ENDED);

	LL_ADC_REG_SetSequencerRanks(ADC_SENS_HIGH_FREQ, LL_ADC_REG_RANK_3, ADC_CUR_C_CH);
	LL_ADC_SetChannelSamplingTime(ADC_SENS_HIGH_FREQ, ADC_CUR_C_CH, ADC_HIGH_FREQ_SAMPLE_TIME);
	LL_ADC_SetChannelSingleDiff(ADC_SENS_HIGH_FREQ, ADC_CUR_C_CH, LL_ADC_SINGLE_ENDED);

	// NVIC_SetPriority(ADC1_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0)); // ????????????
	// NVIC_EnableIRQ(ADC1_2_IRQn);

	LL_ADC_DisableDeepPowerDown(ADC_SENS_HIGH_FREQ);
	LL_ADC_EnableInternalRegulator(ADC_SENS_HIGH_FREQ);

	uint32_t wait_loop_index;
	wait_loop_index =
		1000000;  // ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
	while (wait_loop_index != 0) {
		wait_loop_index--;
	}

	// if (LL_ADC_IsEnabled(ADC_SENS_HIGH_FREQ)) {
	//     LL_ADC_Disable(ADC_SENS_HIGH_FREQ);
	//     while (LL_ADC_IsEnabled(ADC_SENS_HIGH_FREQ)); // Ожидание отключения[1][3]
	// }

	// LL_ADC_StartCalibration(ADC_SENS_HIGH_FREQ, LL_ADC_SINGLE_ENDED);
	// while(LL_ADC_IsCalibrationOnGoing(ADC_SENS_HIGH_FREQ));
	// uint32_t timeout = 1000; // Таймаут на случай ошибок
	// while (LL_ADC_IsCalibrationOnGoing(ADC2) && timeout--);
	// if (timeout == 0) {
	//     // Обработка ошибки
	// }

	// LL_ADC_EnableIT_EOS(ADC_SENS_HIGH_FREQ);
	// LL_ADC_EnableIT_EOC(ADC_SENS_HIGH_FREQ);
	LL_ADC_Enable(ADC_SENS_HIGH_FREQ);
	LL_ADC_REG_StartConversion(ADC_SENS_HIGH_FREQ);
}

ADC_TypeDef* ADC_GetHighFreqAdc(void) {
	return ADC_SENS_HIGH_FREQ;
}

u16* ADC_GetHighFreqSensBuffPtr(void) {
	return (u16*)ADC_HighFreq_Buff;
}

u16 ADC_GetHighFreqSensBuffLen(void) {
	return NUM_ELEMENTS(ADC_HighFreq_Buff);
}

void ADC_StartHighFreqConv(void) {
	LL_ADC_REG_StartConversion(ADC_SENS_HIGH_FREQ);
}

// void ADC1_2_IRQHandler(void) {
//     if (LL_ADC_IsActiveFlag_EOS(ADC_SENS_HIGH_FREQ)) {

//         // volatile u32 value = LL_ADC_REG_ReadConversionData32(ADC_SENS_HIGH_FREQ);
//         volatile u16 value = LL_ADC_REG_ReadConversionData12(ADC_SENS_HIGH_FREQ);
//         u16* buffADC = ADC_GetHighFreqSensBuffPtr();
// 		u16 buffLen = ADC_GetHighFreqSensBuffLen();

//         // for(u32 idx = 0; idx < buffLen; idx++) {
// 		// 	buffADC[idx] = ADC_CalcDigitToVoltage(buffADC[idx]);
// 		// }
//         ADC_SensHighFreqClbk(buffADC, buffLen);

//         LL_ADC_REG_StopConversion(ADC_SENS_HIGH_FREQ);

//         LL_ADC_ClearFlag_EOS(ADC_SENS_HIGH_FREQ);
//     }
// }
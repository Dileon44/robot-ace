#include "sensors.h"
#include "conf/tasks_stack_and_priority.h"
#include "ring_list.h"

#include "platform_inc_m0.h"
#include "stm32g4xx_ll_tim.h"

#include "debug_interface.h"

#define SENSORS_THROT_KOEF_VOLT_TO_PSENT	0.07143f
#define SENSORS_THROT_OFFSET_VOLT_TO_PSENT	71.43f
#define SENSORS_THROT_LOWER_THRESHOLD_mVOLT 1000u
#define SENSORS_THROT_UPPER_THRESHOLD_mVOLT 2400u

#define SENSORS_BRAKE_KOEF_VOLT_TO_PSENT	0.07143f
#define SENSORS_BRAKE_OFFSET_VOLT_TO_PSENT	71.43f
#define SENSORS_BRAKE_LOWER_THRESHOLD_mVOLT 1000u
#define SENSORS_BRAKE_UPPER_THRESHOLD_mVOLT 2400u

#define SENSORS_KOEF_PSENT_TO_KMPH 4.0f

#define SENSORS_1_SECOND		1
#define SENSORS_1_MIN			60 * SENSORS_1_SECOND
#define SENSORS_KOEF_SEC_TO_MIN SENSORS_1_MIN* SETTINGS_F_ADC_CONV_HZ * 1.0f

Sensors_t sensors = {
	.throttlePercent = 0u,
	.brakePercent	 = 0u,

	.halls	   = 0,
	.hallsPrev = 0,
	// .Sensors_ThrottleFilt = {
	//     .in = 0,
	//     .out = 0,
	//     .idxLastElem = 0,
	// },
	.filterThrottle =
		{
			.input	= 0.0f,
			.output = 0.0f,
			.T		= 0.1f,	 // t / T: 0.1 / 0.2 = 0.5
			.calc	= TFilter_Calc,
		},
	.filterBrake =
		{
			.input	= 0.0f,
			.output = 0.0f,
			.T		= 0.1f,	 // t / T: 0.1 / 0.2 = 0.5
			.calc	= TFilter_Calc,
		},
};

// static Sensors_MovingAverageFilt_t Sensors_ThrottleFilt = {
//     .in = 0,
//     .out = 0,
//     .idxLastElem = 0,
// };

// static Sensors_MovingAverageFilt_t Sensors_BrakeFilt = {
//     .in = 0,
//     .out = 0,
//     .idxLastElem = 0,
// };

// Sensors_t sensors = {
//     .throttlePercent  = 0u,
//     .brakePercent     = 0u,
//     .speedCurrentKMPH = 0.0f,
//     .speedCurrentRPM  = 0.0f,
//     .cntToggleHall    = 0u,
// };

static TaskHandle_t Sensors_Handle = NULL;
// static SemaphoreHandle_t Sensors_Semaphore = NULL;
// static TaskHandle_t xTaskToNotify = NULL;

u16 MovingAverageFilter(Sensors_MovingAverageFilt_t* filterInfo) {
	filterInfo->out =
		filterInfo->out + (u32)(((filterInfo->in - filterInfo->buff[filterInfo->idxLastElem]) /
								 SENSORS_FILT_BUF_ELEM_MAX_NUM));

	if (filterInfo->out > 2500 && filterInfo->out < 2700) {
		filterInfo->out = 2500;
	} else if (filterInfo->out < 1100 && filterInfo->out > 700) {
		filterInfo->out = 900;
	} else if (filterInfo->out < 700 || filterInfo->out > 2700) {
		// error
	}

	filterInfo->buff[filterInfo->idxLastElem] = filterInfo->in;
	filterInfo->idxLastElem = (filterInfo->idxLastElem + 1) % SENSORS_FILT_BUF_ELEM_MAX_NUM;

	return filterInfo->out;
}

u8 Sensors_mVolt2Persent(u16 mVolt, float koefScale, float offset, u16 lowerThreshold,
						 u16 upperThreshold) {
	if (mVolt <= lowerThreshold) {
		return 0;
	} else if (mVolt >= upperThreshold) {
		return 100;
	} else {
		return (u8)(koefScale * (float)mVolt - offset);
	}
}

float Sensors_ThrottlePersent2KMPH(u8 persent) {
	return persent / SENSORS_KOEF_PSENT_TO_KMPH;
}

void Sensors_LowFreqCallback(u16* buffPtr, u16 buffLen) {
	// sensors.Sensors_ThrottleFilt.in = buffPtr[0];
	// Sensors_BrakeFilt.in = buffPtr[1];

	// MovingAverageFilter(&(sensors.Sensors_ThrottleFilt));
	// MovingAverageFilter(&Sensors_BrakeFilt);

	// BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	// xSemaphoreGiveFromISR(Sensors_Semaphore, &xHigherPriorityTaskWoken);

	// if (xHigherPriorityTaskWoken == pdTRUE) {
	//     portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	// }

	sensors.filterThrottle.input = buffPtr[0];
	sensors.filterThrottle.calc(&sensors.filterThrottle);

	sensors.filterBrake.input = buffPtr[1];
}

// void Sensors_SpeedCalcCallback(void) {
//     sensors.speedCurrentRPM = (SENSORS_KOEF_SEC_TO_MIN * sensors.cntToggleHall) / 120.0f; // SETTINGS_NUM_MOTOR_MAGNET;
//     sensors.speedCurrentKMPH = 2 * 3.1416f * 0.13f * sensors.speedCurrentRPM * 60.0f / 1000.0f;
//     sensors.cntToggleHall = 0;
// }

void Sensors_Init(Pl_Motor_ClbkHall_t pHallClbk,
				  Pl_Motor_HallToggleTimeCalcClbk_t Sensors_HallToggleTimeCalcCallback) {
	Pl_Motor_SensorsInit(pHallClbk);
	Pl_TIM_HallToggleTimeCalc_Init(Sensors_HallToggleTimeCalcCallback);
	Pl_ADC_LowFreqSens_Init(Sensors_LowFreqCallback);
}

Sensors_t* Sensors_GetPtrSensors(void) {
	return &sensors;
}

static void vTask_SensorsProcess(void* pvParameters) {

	for (;;) {
		// xSemaphoreTake(Sensors_Semaphore, 10);

		// u32 mVoltThrottle = MovingAverageFilter(&Sensors_ThrottleFilt);
		// u32 mVoltBrake = MovingAverageFilter(&Sensors_BrakeFilt);

		// sensors.throttlePercent = Sensors_mVolt2Persent(
		//     mVoltThrottle,
		//     SENSORS_THROT_KOEF_VOLT_TO_PSENT,
		//     SENSORS_THROT_OFFSET_VOLT_TO_PSENT,
		//     SENSORS_THROT_LOWER_THRESHOLD_mVOLT,
		//     SENSORS_THROT_UPPER_THRESHOLD_mVOLT
		// );

		// sensors.brakePercent = Sensors_mVolt2Persent(
		//     mVoltBrake,
		//     SENSORS_BRAKE_KOEF_VOLT_TO_PSENT,
		//     SENSORS_BRAKE_OFFSET_VOLT_TO_PSENT,
		//     SENSORS_BRAKE_LOWER_THRESHOLD_mVOLT,
		//     SENSORS_BRAKE_UPPER_THRESHOLD_mVOLT
		// );

		// sensors.speedDesiredKMPH = Sensors_ThrottlePersent2KMPH(sensors.throttlePercent);

		// float diff =  sensors.speedDesiredKMPH -  sensors.speedCurrentKMPH;

		// if (diff < -0.2 || diff > 0.2) {
		//     compareValueSign += 4 * diff;

		//     if (compareValueSign < 0) {
		//         compareValue = 0;
		//     } else if (compareValueSign > 13499) {
		//         compareValue = 13499;
		//     } else {
		//         compareValue = compareValueSign;
		//     }
		// }

		// if (compareValuePrev != compareValue) {
		//     Pl_Motor_SetComparePWMA(compareValue);
		//     Pl_Motor_SetComparePWMB(compareValue);
		//     Pl_Motor_SetComparePWMC(compareValue);
		//     compareValuePrev = compareValue;
		// }

		// DEBUG_PRINT("1: %d\r\n2: %d\r\n\n", (u32)sensors.speedDesiredKMPH, (u32)sensors.speedCurrentKMPH);
		// DEBUG_PRINT("%.1f\r\n\n", /*Pl_TIM_HallToggleTimeCalc_GetCnt()*/ sensors.speedCurrentKMPHRaw);

		// u8* bufTx = DebugInterface_GetPtrBuffTx();
		// // memset(bufTx, '\0', DebugInterface_GetLenBuffTx());

		// u32 len = sprintf((char*)bufTx, "%.2f; %.2f\r\n\0", sensors.speedCurrentKMPHRaw, sensors.speedCurrentKMPHFilt);
		// Pl_USART_Debug_SetDataLengthTx(len);
		// Pl_USART_Debug_Enable_Tx();

		// RingBuff_t* ringBuffRxPtr = DebugInterface_GetRingBuffRx();
		// RET_STATE_t retState = RingBuff_Str_Search(ringBuffRxPtr, 50, strStop);
		// if (retState == RET_STATE_SUCCESS) {
		//     Pl_Test_GPIO_Toggle();
		//     Pl_Motor_SetComparePWMA(0);
		//     Pl_Motor_SetComparePWMB(0);
		//     Pl_Motor_SetComparePWMC(0);
		// }

		// DEBUG_PRINT("TEST\r\n");
		vTaskDelay(300);
	}
}

void Sensors_TaskCreate(void) {
	if (!Sensors_Handle) {
		const char* taskName = "Sensors control";
		xTaskCreate(vTask_SensorsProcess, taskName, SENSORS_CONTROL_TASK_STACK, NULL,
					SENSORS_CONTROL_TASK_PRIORITY, &Sensors_Handle);
	}
}

void Sensors_TaskDelete(void) {
	if (Sensors_Handle) {
		vTaskDelete(Sensors_Handle);
	}
}

void FreeRTOS_Sensors_InitComponents(void) {
	// Sensors_Semaphore = xSemaphoreCreateBinary();
	Sensors_TaskCreate();
}

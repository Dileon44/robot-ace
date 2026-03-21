#include "uart_process.h"
#include "conf/tasks_stack_and_priority.h"
#include "debug_interface.h"
#include "motor.h"
#include "platform.h"
#include "sensors.h"

#define MOTOR_UART_PROCESS_TASK_PERIOD 30

static TaskHandle_t UartProcess_HandleRx = NULL;
static TaskHandle_t UartProcess_HandleTx = NULL;

Uart_Process_RxData_t Uart_Process_RxData = {
	.Buttons =
		{
			.isStop = true,
		},
	.UartSendData =
		{
			.isSend = true,
		},
	.CtrlSrc = UART_PROCESS_CTRL_SRC_UART,
	.CtrlType =
		{
			.type = UART_PROCESS_CTRL_TYPE_DUTY,
			.duty =
				{
					.ShapeInCurrent = UART_PROCESS_SHAPE_IN_STEP,
					.Step =
						{
							.speedDesired = 0.0f,
						},
					.Sin =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
					.Meander =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
					.Sawtooth =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
				},
			.current =
				{
					.ShapeInCurrent = UART_PROCESS_SHAPE_IN_STEP,
					.Step =
						{
							.speedDesired = 0.0f,
						},
					.Sin =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
					.Meander =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
					.Sawtooth =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
				},
			.speed =
				{
					.ShapeInCurrent = UART_PROCESS_SHAPE_IN_STEP,
					.Step =
						{
							.speedDesired = 0.0f,
						},
					.Sin =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
					.Meander =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
					.Sawtooth =
						{
							.ampl = 0.0f,
							.freq = 0.0f,
						},
				},
		},
	// .ShapeIn = {
	//     .ShapeInCurrent = UART_PROCESS_SHAPE_IN_STEP,
	//     .Step = {
	//         .speedDesired = 0.0f,
	//     },
	//     .Sin = {
	//         .ampl = 0.0f,
	//         .freq = 0.0f,
	//     },
	//     .Meander = {
	//         .ampl = 0.0f,
	//         .freq = 0.0f,
	//     },
	//     .Sawtooth = {
	//         .ampl = 0.0f,
	//         .freq = 0.0f,
	//     },
	// },
	.PrintParams =
		{
			.speedDesired_kmph = false,
			.speedRaw_kmph	   = false,
			.speedFilt_kmph	   = false,
			.outPI_kmph		   = false,
			.rpm			   = false,
			.duty			   = false,
			.Ides			   = false,
			.Icur			   = false,
			.Ipi			   = false,
			.numActive		   = 0,
		},
	.isCallIqrHall1 = false,
};

typedef enum {
	UART_PROCESS_PRINT_PARAMS_SPDES,
	UART_PROCESS_PRINT_PARAMS_SPRAW,
	UART_PROCESS_PRINT_PARAMS_SPFILT,
	UART_PROCESS_PRINT_PARAMS_OUTPI,
	UART_PROCESS_PRINT_PARAMS_RPM,
	UART_PROCESS_PRINT_PARAMS_DUTY,
	UART_PROCESS_PRINT_PARAMS_IA,
	UART_PROCESS_PRINT_PARAMS_IB,
	UART_PROCESS_PRINT_PARAMS_IC,
	UART_PROCESS_PRINT_PARAMS_I_DES,
	UART_PROCESS_PRINT_PARAMS_I_CUR,
	UART_PROCESS_PRINT_PARAMS_I_PI,
	UART_PROCESS_PRINT_PARAMS_SIZE,
} UART_PROCESS_PRINT_PARAMS_t;

RET_STATE_t Uart_Process_RxSearchPushButtons(RingBuff_t* ringBuffRx) {
	char* sequenceSearchBuff[2];
	sequenceSearchBuff[0] = "stop\r\n";
	sequenceSearchBuff[1] = "start\r\n";
	s16 winString		  = -1;
	RET_STATE_t retState  = RET_STATE_UNDEF;

	retState = RingBuff_Str_SetSearch(ringBuffRx, DELAY_1_MILLISECOND, sequenceSearchBuff,
									  NUM_ELEMENTS(sequenceSearchBuff), &winString);

	if (retState != RET_STATE_SUCCESS || winString == -1) {
		return retState;
	} else {
		Uart_Process_RxData.Buttons.isStop = (winString == 0);
	}

	return retState;
}

RET_STATE_t Uart_Process_RxSearchUartEnable(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	retState = RingBuff_Str_Search(ringBuff, DELAY_1_MILLISECOND, "uart:");

	if (retState == RET_STATE_SUCCESS) {
		u8 uartEnableStr[1] = "";
		retState = RingBuff_Ring2Line_CopyBeforeEntry(ringBuff, DELAY_1_MILLISECOND, uartEnableStr,
													  1, "\r\n", NULL);

		if (retState == RET_STATE_SUCCESS) {
			Uart_Process_RxData.UartSendData.isSend = (uartEnableStr[0] == '1');
		}
	}

	return retState;
}

RET_STATE_t Uart_Process_RxSearchCtrlSrc(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	u8 shapeInStr[9] = "";
	char* sequenceSearchBuff[2];
	sequenceSearchBuff[0] = "CtrlSrc:";
	sequenceSearchBuff[1] = "\r\n";
	u16 rwCnt			  = 0;

	retState = RingBuff_Str_SearchEntryBetween(ringBuff, DELAY_1_MILLISECOND, shapeInStr, 8,
											   sequenceSearchBuff, &rwCnt);

	if (retState == RET_STATE_SUCCESS) {
		if (rwCnt == 4) {
			Uart_Process_RxData.CtrlSrc = UART_PROCESS_CTRL_SRC_UART;
		} else if (rwCnt == 8) {
			Uart_Process_RxData.CtrlSrc = UART_PROCESS_CTRL_SRC_THROTTLE;
		} else {
			retState = RET_STATE_ERR_PARAM;
		}
	}

	return retState;
}

Uart_Process_ShapeIn_t* Uart_Process_GetCtrlType(
	UART_PROCESS_CTRL_TYPE_t ctrlType) {  // , Uart_Process_ShapeIn_t* ctrlTypeShape
	switch (ctrlType) {
		case UART_PROCESS_CTRL_TYPE_DUTY:
			return &Uart_Process_RxData.CtrlType.duty;
		case UART_PROCESS_CTRL_TYPE_CURRENT:
			return &Uart_Process_RxData.CtrlType.current;
		case UART_PROCESS_CTRL_TYPE_SPEED:
			return &Uart_Process_RxData.CtrlType.speed;
		default:
			return NULL;
	}
	// switch (ctrlType) {
	//     case UART_PROCESS_CTRL_TYPE_DUTY:
	//         ctrlTypeShape = &(Uart_Process_RxData.CtrlType.duty);
	//         RET_STATE_SUCCESS;
	//     case UART_PROCESS_CTRL_TYPE_CURRENT:
	//         ctrlTypeShape = &Uart_Process_RxData.CtrlType.current;
	//         return RET_STATE_SUCCESS;
	//     case UART_PROCESS_CTRL_TYPE_SPEED:
	//         ctrlTypeShape = &Uart_Process_RxData.CtrlType.speed;
	//         return RET_STATE_SUCCESS;
	//     default:
	//         ctrlTypeShape = NULL;
	//         return RET_STATE_ERROR;
	// }
}

RET_STATE_t Uart_Process_RxSearchCtrlType(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	u8 shapeInStr[9] = "";
	char* sequenceSearchBuff[2];
	sequenceSearchBuff[0] = "CtrlType:";
	sequenceSearchBuff[1] = "\r\n";
	u16 rwCnt			  = 0;

	retState = RingBuff_Str_SearchEntryBetween(ringBuff, DELAY_1_MILLISECOND, shapeInStr, 8,
											   sequenceSearchBuff, &rwCnt);

	if (retState == RET_STATE_SUCCESS) {
		if (rwCnt == 4) {
			Uart_Process_RxData.CtrlType.type = UART_PROCESS_CTRL_TYPE_DUTY;
		} else if (rwCnt == 7) {
			Uart_Process_RxData.CtrlType.type = UART_PROCESS_CTRL_TYPE_CURRENT;
		} else if (rwCnt == 5) {
			Uart_Process_RxData.CtrlType.type = UART_PROCESS_CTRL_TYPE_SPEED;
		} else {
			retState = RET_STATE_ERR_PARAM;
		}
	}

	return retState;
}

RET_STATE_t Uart_Process_RxSearchShapeIn(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	u8 shapeInStr[9] = "";
	char* sequenceSearchBuff[2];
	sequenceSearchBuff[0] = "ShapeIn:";
	sequenceSearchBuff[1] = "\r\n";
	u16 rwCnt			  = 0;

	retState = RingBuff_Str_SearchEntryBetween(ringBuff, DELAY_1_MILLISECOND, shapeInStr, 8,
											   sequenceSearchBuff, &rwCnt);

	if (retState == RET_STATE_SUCCESS) {
		UART_PROCESS_SHAPE_IN_t shapeInCurrent = UART_PROCESS_SHAPE_IN_STEP;

		if (rwCnt == 4) {
			shapeInCurrent = UART_PROCESS_SHAPE_IN_STEP;
		} else if (rwCnt == 3) {
			shapeInCurrent = UART_PROCESS_SHAPE_IN_SIN;
		} else if (rwCnt == 7) {
			shapeInCurrent = UART_PROCESS_SHAPE_IN_MEANDER;
		} else if (rwCnt == 8) {
			shapeInCurrent = UART_PROCESS_SHAPE_IN_SAWTOOTH;
		} else {
			retState = RET_STATE_ERR_PARAM;
		}

		// Uart_Process_ShapeIn_t* pCtrlType = NULL;
		Uart_Process_ShapeIn_t* pCtrlType =
			Uart_Process_GetCtrlType(Uart_Process_RxData.CtrlType.type);
		if (retState == RET_STATE_SUCCESS) {
			pCtrlType->ShapeInCurrent = shapeInCurrent;
		}
	}

	return retState;
}

RET_STATE_t Uart_Process_RxSearchShapeInAmpl(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	char* sequenceSearchBuff[4];
	sequenceSearchBuff[0] = "StepSpDes:";
	sequenceSearchBuff[1] = "SinAmpl:";
	sequenceSearchBuff[2] = "MndrAmpl:";
	sequenceSearchBuff[3] = "SawAmpl:";
	s16 winString		  = -1;

	retState = RingBuff_Str_SetSearch(ringBuff, DELAY_1_MILLISECOND, sequenceSearchBuff,
									  NUM_ELEMENTS(sequenceSearchBuff), &winString);

	if (retState != RET_STATE_SUCCESS || winString == -1) {
		return retState;
	} else {
		char amplStr[4] = "";
		retState = RingBuff_Ring2Line_CopyBeforeEntry(ringBuff, DELAY_1_MILLISECOND, (u8*)amplStr,
													  4, "\r\n", NULL);

		if (retState == RET_STATE_SUCCESS) {
			char* endptr;
			float ampl = strtof(amplStr, &endptr);
			Uart_Process_ShapeIn_t* pCtrlType =
				Uart_Process_GetCtrlType(Uart_Process_RxData.CtrlType.type);
			if (retState == RET_STATE_SUCCESS) {
				switch (winString) {
					case 0:
						pCtrlType->Step.speedDesired = ampl;
						break;
					case 1:
						pCtrlType->Sin.ampl = ampl;
						break;
					case 2:
						pCtrlType->Meander.ampl = ampl;
						break;
					case 3:
						pCtrlType->Sawtooth.ampl = ampl;
						break;
					default:
						retState = RET_STATE_ERR_PARAM;
				}
			}
		} else {
			retState = RET_STATE_ERROR;
		}
	}

	return retState;
}

RET_STATE_t Uart_Process_RxSearchShapeInFreq(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	char* sequenceSearchBuff[3];
	sequenceSearchBuff[0] = "SinFreq:";
	sequenceSearchBuff[1] = "MndrFreq:";
	sequenceSearchBuff[2] = "SawFreq:";
	s16 winString		  = -1;

	retState = RingBuff_Str_SetSearch(ringBuff, DELAY_1_MILLISECOND, sequenceSearchBuff,
									  NUM_ELEMENTS(sequenceSearchBuff), &winString);

	if (retState != RET_STATE_SUCCESS || winString == -1) {
		return retState;
	} else {
		char freqStr[4] = "";
		retState = RingBuff_Ring2Line_CopyBeforeEntry(ringBuff, DELAY_1_MILLISECOND, (u8*)freqStr,
													  4, "\r\n", NULL);

		if (retState == RET_STATE_SUCCESS) {
			char* endptr;
			float freq = strtof(freqStr, &endptr);
			Uart_Process_ShapeIn_t* pCtrlType =
				Uart_Process_GetCtrlType(Uart_Process_RxData.CtrlType.type);
			if (retState == RET_STATE_SUCCESS) {
				switch (winString) {
					case 0:
						pCtrlType->Sin.freq = freq;
						break;
					case 1:
						pCtrlType->Meander.freq = freq;
						break;
					case 2:
						pCtrlType->Sawtooth.freq = freq;
						break;
					default:
						retState = RET_STATE_ERR_PARAM;
				}
			}
		} else {
			retState = RET_STATE_ERROR;
		}
	}

	return retState;
}

RET_STATE_t Uart_Process_RxSearchPrintParams(RingBuff_t* ringBuff) {
	RET_STATE_t retState = RET_STATE_UNDEF;

	char* sequenceSearchBuff[12];
	sequenceSearchBuff[0]  = "spdes:";
	sequenceSearchBuff[1]  = "spraw:";
	sequenceSearchBuff[2]  = "spfilt:";
	sequenceSearchBuff[3]  = "outPI:";
	sequenceSearchBuff[4]  = "rpm:";
	sequenceSearchBuff[5]  = "duty:";
	sequenceSearchBuff[6]  = "Ia:";
	sequenceSearchBuff[7]  = "Ib:";
	sequenceSearchBuff[8]  = "Ic:";
	sequenceSearchBuff[9]  = "Ides:";
	sequenceSearchBuff[10] = "Icur:";
	sequenceSearchBuff[11] = "Ipi:";
	s16 winString		   = -1;

	retState = RingBuff_Str_SetSearch(ringBuff, DELAY_1_MILLISECOND, sequenceSearchBuff,
									  NUM_ELEMENTS(sequenceSearchBuff), &winString);

	if (retState != RET_STATE_SUCCESS || winString == -1) {
		return retState;
	} else {
		char enableStr[1] = "";
		retState = RingBuff_Ring2Line_CopyBeforeEntry(ringBuff, DELAY_1_MILLISECOND, (u8*)enableStr,
													  1, "\r\n", NULL);

		if (retState == RET_STATE_SUCCESS) {
			bool enable = (enableStr[0] == '1');

			switch (winString) {
				case UART_PROCESS_PRINT_PARAMS_SPDES:
					if (Uart_Process_RxData.PrintParams.speedDesired_kmph && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.speedDesired_kmph && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.speedDesired_kmph = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_SPRAW:
					if (Uart_Process_RxData.PrintParams.speedRaw_kmph && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.speedRaw_kmph && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.speedRaw_kmph = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_SPFILT:
					if (Uart_Process_RxData.PrintParams.speedFilt_kmph && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.speedFilt_kmph && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.speedFilt_kmph = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_OUTPI:
					if (Uart_Process_RxData.PrintParams.outPI_kmph && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.outPI_kmph && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.outPI_kmph = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_RPM:
					if (Uart_Process_RxData.PrintParams.rpm && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.rpm && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.rpm = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_DUTY:
					if (Uart_Process_RxData.PrintParams.duty && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.duty && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.duty = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_IA:
					if (Uart_Process_RxData.PrintParams.Ia && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.Ia && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.Ia = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_IB:
					if (Uart_Process_RxData.PrintParams.Ib && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.Ib && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.Ib = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_IC:
					if (Uart_Process_RxData.PrintParams.Ic && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.Ic && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.Ic = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_I_DES:
					if (Uart_Process_RxData.PrintParams.Ides && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.Ides && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.Ides = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_I_CUR:
					if (Uart_Process_RxData.PrintParams.Icur && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.Icur && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.Icur = enable;
					break;
				case UART_PROCESS_PRINT_PARAMS_I_PI:
					if (Uart_Process_RxData.PrintParams.Ipi && !enable) {
						Uart_Process_RxData.PrintParams.numActive -= 1;
					} else if (!Uart_Process_RxData.PrintParams.Ipi && enable) {
						Uart_Process_RxData.PrintParams.numActive += 1;
					}
					Uart_Process_RxData.PrintParams.Ipi = enable;
					break;
				default:
					retState = RET_STATE_ERR_PARAM;
			}
		} else {
			retState = RET_STATE_ERROR;
		}
	}

	return retState;
}

RET_STATE_t Uart_Process_SetDesiredParam(UART_PROCESS_CTRL_TYPE_t ctrlType, float* desiredParam) {
	switch (ctrlType) {
		case UART_PROCESS_CTRL_TYPE_DUTY:
			Motor_GetMotorPtr()->speedDesiredKMPH = *desiredParam;
			break;
		case UART_PROCESS_CTRL_TYPE_CURRENT:
			Motor_GetMotorPtr()->currentDesired = *desiredParam;
			break;
		case UART_PROCESS_CTRL_TYPE_SPEED:
			break;
		default:
			return RET_STATE_ERR_PARAM;
	}

	return RET_STATE_SUCCESS;
}

RET_STATE_t Uart_Process_UpdateControlData() {
	RET_STATE_t retState = RET_STATE_SUCCESS;
	Motor_t* pMotor		 = Motor_GetMotorPtr();
	pMotor->stopMotor	 = Uart_Process_RxData.Buttons.isStop;

	if (Uart_Process_RxData.CtrlSrc == UART_PROCESS_CTRL_SRC_UART) {
		if (Uart_Process_RxData.Buttons.isStop) {
			pMotor->speedDesiredKMPH = 0.0f;
			Pl_Motor_PWMA_Disable();
			Pl_Motor_PWMB_Disable();
			Pl_Motor_PWMC_Disable();
			Pl_Motor_ResetKeyAL();
			Pl_Motor_ResetKeyBL();
			Pl_Motor_ResetKeyCL();
			Pl_Motor_SetComparePWMA(0);
			Pl_Motor_SetComparePWMB(0);
			Pl_Motor_SetComparePWMC(0);
			Uart_Process_RxData.isCallIqrHall1 = false;
			pMotor->currentDesired			   = 0.0f;
			pMotor->controlInfo.dutyCycle	   = 0.0f;
			Motor_ResetCurrentPI();
			Motor_GetMotorPtr()->ctrlType = Uart_Process_RxData.CtrlType.type;
		} else {
			Uart_Process_ShapeIn_t* pCtrlType =
				Uart_Process_GetCtrlType(Uart_Process_RxData.CtrlType.type);
			if (retState == RET_STATE_SUCCESS) {
				switch (pCtrlType->ShapeInCurrent) {
					case UART_PROCESS_SHAPE_IN_STEP:
						Uart_Process_SetDesiredParam(Uart_Process_RxData.CtrlType.type,
													 &(pCtrlType->Step.speedDesired));
						// sens->speedDesiredKMPH = pCtrlType->Step.speedDesired;
						break;
					case UART_PROCESS_SHAPE_IN_SIN:
						//
						break;
					case UART_PROCESS_SHAPE_IN_MEANDER:
						//
						break;
					case UART_PROCESS_SHAPE_IN_SAWTOOTH:
						//
						break;
					default:
						retState = RET_STATE_ERR_PARAM;
				}
			}

			if (!Uart_Process_RxData.isCallIqrHall1) {
				Uart_Process_RxData.isCallIqrHall1 = true;
				Pl_Hall1_CallIrq();
			}
		}
	}

	return retState;
}

static void vTask_MotorUartProcessRecieve(void* pvParameters) {
	for (;;) {
		RingBuff_t* ringBuffRx = DebugInterface_GetRingBuffRx();

		// 1. поиск stop/start
		Uart_Process_RxSearchPushButtons(ringBuffRx);

		// 2. поиск uart on/off
		Uart_Process_RxSearchUartEnable(ringBuffRx);

		// 3. поиск источника управления
		Uart_Process_RxSearchCtrlSrc(ringBuffRx);

		Uart_Process_RxSearchCtrlType(ringBuffRx);

		// 4. поиск фигуры входного сигнала
		Uart_Process_RxSearchShapeIn(ringBuffRx);

		// 5. поиск настроек входного сигнала
		Uart_Process_RxSearchShapeInAmpl(ringBuffRx);
		Uart_Process_RxSearchShapeInFreq(ringBuffRx);

		// 6. поиск print params
		Uart_Process_RxSearchPrintParams(ringBuffRx);

		// обновление всех данных
		Uart_Process_UpdateControlData();

		vTaskDelay(25);
	}
}

void Uart_Process_Rx_TaskCreate(void) {
	if (!UartProcess_HandleRx) {
		const char* taskName = "Motor UART Rx process";
		xTaskCreate(vTask_MotorUartProcessRecieve, taskName, MOTOR_UART_PROCESS_RX_TASK_STACK, NULL,
					MOTOR_UART_PROCESS_RX_TASK_PRIORITY, &UartProcess_HandleRx);
	}
}

static void vTask_MotorUartProcessTransmit(void* pvParameters) {
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, MOTOR_UART_PROCESS_TASK_PERIOD);
		// DEBUG_PRINT("TEST\r\n");
		if (Uart_Process_RxData.UartSendData.isSend) {
			Sensors_t* sensPtr		   = Sensors_GetPtrSensors();
			ControlInfo_t* ctrlInfoPtr = Motor_GetControlInfoPtr();
			Motor_t* motor			   = Motor_GetMotorPtr();

			char* ptrBuffTx = (char*)DebugInterface_GetPtrBuffTx();
			memset(ptrBuffTx, 0, DebugInterface_GetLenBuffTx());
			if (Uart_Process_RxData.PrintParams.numActive != 0) {
				s32 numComma = Uart_Process_RxData.PrintParams.numActive - 1;
				u32 n		 = 0;
				if (Uart_Process_RxData.PrintParams.speedDesired_kmph) {
					if (numComma > 0) {
						n += sprintf(ptrBuffTx, "spdes:%.2f,", motor->speedDesiredKMPH);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx, "spdes:%.2f", motor->speedDesiredKMPH);
					}
				}

				if (Uart_Process_RxData.PrintParams.speedRaw_kmph) {
					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "spraw:%.2f,", motor->speedCurrentKMPHRaw);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "spraw:%.2f", motor->speedCurrentKMPHRaw);
					}
				}

				if (Uart_Process_RxData.PrintParams.speedFilt_kmph) {
					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "spfilt:%.2f,", motor->speedCurrentKMPHFilt);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "spfilt:%.2f", motor->speedCurrentKMPHFilt);
					}
				}

				if (Uart_Process_RxData.PrintParams.outPI_kmph) {
					if (numComma > 0) {
						// n += sprintf(ptrBuffTx + n, "outPI:%.2f,", ctrlInfoPtr->deltaSpeedPI_kmph);
						n += sprintf(ptrBuffTx + n, "outPI:%.2f,", sensPtr->filterThrottle.input);
						--numComma;
					} else {
						// n += sprintf(ptrBuffTx + n, "outPI:%.2f", ctrlInfoPtr->deltaSpeedPI_kmph);
						n += sprintf(ptrBuffTx + n, "outPI:%.2f", sensPtr->filterThrottle.input);
					}
				}

				if (Uart_Process_RxData.PrintParams.rpm) {
					if (numComma > 0) {
						// n += sprintf(ptrBuffTx + n, "rpm:%.2f,", ctrlInfoPtr->rpm);
						n += sprintf(ptrBuffTx + n, "rpm:%d,",
									 ctrlInfoPtr->direction);  // sensPtr->filterThrottle.output
						--numComma;
					} else {
						// n += sprintf(ptrBuffTx + n, "rpm:%.2f", ctrlInfoPtr->rpm);
						n += sprintf(ptrBuffTx + n, "rpm:%d",
									 ctrlInfoPtr->direction);  // sensPtr->filterThrottle.output
					}
				}

				if (Uart_Process_RxData.PrintParams.duty) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "duty:%.2f,",
									 ctrlInfoPtr->dutyCycle * 100.0f / 6799.0f);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "duty:%.2f",
									 ctrlInfoPtr->dutyCycle * 100.0f / 6799.0f);
					}
				}

				if (Uart_Process_RxData.PrintParams.Ia) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "Ia:%.2f,", ctrlInfoPtr->curA);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "Ia:%.2f", ctrlInfoPtr->curA);
					}
				}

				if (Uart_Process_RxData.PrintParams.Ib) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "Ib:%.2f,", ctrlInfoPtr->curB);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "Ib:%.2f", ctrlInfoPtr->curB);
					}
				}

				if (Uart_Process_RxData.PrintParams.Ic) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "Ic:%.2f,", ctrlInfoPtr->curC);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "Ic:%.2f", ctrlInfoPtr->curC);
					}
				}

				if (Uart_Process_RxData.PrintParams.Ides) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "Ides:%.2f,", motor->filterIdes.output);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "Ides:%.2f", motor->filterIdes.output);
					}
				}

				if (Uart_Process_RxData.PrintParams.Icur) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "Icur:%.2f,", motor->currentMax);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "Icur:%.2f", motor->currentMax);
					}
				}

				if (Uart_Process_RxData.PrintParams.Ipi) {

					if (numComma > 0) {
						n += sprintf(ptrBuffTx + n, "Ipi:%.2f,", motor->Ipi);
						--numComma;
					} else {
						n += sprintf(ptrBuffTx + n, "Ipi:%.2f", motor->Ipi);
					}
				}

				sprintf(ptrBuffTx + n, "\r\n\0");
				Pl_USART_Debug_Enable_Tx();
			}
		}
	}
}

void Uart_Process_Tx_TaskCreate(void) {
	if (!UartProcess_HandleTx) {
		const char* taskName = "Motor UART Tx process";
		xTaskCreate(vTask_MotorUartProcessTransmit, taskName, MOTOR_UART_PROCESS_TX_TASK_STACK,
					NULL, MOTOR_UART_PROCESS_TX_TASK_PRIORITY, &UartProcess_HandleTx);
	}
}

void FreeRTOS_MotorUartProcess_InitComponents() {
	Uart_Process_Rx_TaskCreate();
	Uart_Process_Tx_TaskCreate();
}
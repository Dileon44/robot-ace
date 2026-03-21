#include "motor.h"
#include "control/bldc.h"
#include "filter.h"
#include "platform.h"
#include "tasks_stack_and_priority.h"

#define NUM_TOGGLE_ALL_HALLS_PER_REV 120  // 120
#define PERIOD_MCU					 5.8825f
#define TIM_SPEED_CONTROL_PRESC		 1297.0f
#define SECOND_PER_HOUR				 3600.0f
#define M_PER_KM					 1000.0f
#define WHEEL_DIAMETER				 0.13f	// 0.13f
#define NUM_PI						 3.14159f
#define WHEEL_CIRCUMFERENCE_M		 2 * NUM_PI* WHEEL_DIAMETER
#define NS_PER_SECOND				 1000000000.0f
#define PERIOD_CNT_SPEED_CONTROL	 TIM_SPEED_CONTROL_PRESC* PERIOD_MCU
#define KOEF_TIM_CNT_TO_KMPH                                                                \
	NS_PER_SECOND / (TIM_SPEED_CONTROL_PRESC * PERIOD_MCU * NUM_TOGGLE_ALL_HALLS_PER_REV) * \
		WHEEL_CIRCUMFERENCE_M* SECOND_PER_HOUR / M_PER_KM

#define MOTOR_SPEED_PID_OUT_MAX
#define MOTOR_SPEED_KOEF_PI_TO_DUTY_CYCLE 650.0f

#define MOTOR_SENS_CUR_A_OFFSET_mV	1603.5f
#define MOTOR_SENS_CUR_B_OFFSET_mV	1603.5f
#define MOTOR_SENS_CUR_C_OFFSET_mV	1604.5f
#define MOTOR_SENS_CUR_KOEF			20.0f
#define MOTOR_SENS_CUR_R_SHUNT_mOhm 2.0f

Motor_t motor = {
	.controlInfo =
		{
			.direction =
				0,	// 0 - front, 1 - reverse. Надо проверить, когда колесо стоит на самокате.
			.directionWheel		 = 0,
			.ControlTypeFunc	 = BLDC_Control,
			.timSpeedIsInterrupt = 0,
			.deltaSpeed_kmph	 = 0.0f,
			.deltaSpeedPI_kmph	 = 0.0f,
			.dutyCycle			 = 0.0f,
			.rpm				 = 0.0f,
			.curA				 = 0.0f,
			.curB				 = 0.0f,
			.curC				 = 0.0f,
		},
	.filterCurA =
		{
			.input	= 0.0f,
			.output = 0.0f,
			.T		= 0.08f,  // t / T: 0.00016f / 0.0005f
			.calc	= TFilter_Calc,
		},
	.filterCurB =
		{
			.input	= 0.0f,
			.output = 0.0f,
			.T		= 0.08f,  // t / T: 0.00016f / 0.001f
			.calc	= TFilter_Calc,
		},
	.filterCurC =
		{
			.input	= 0.0f,
			.output = 0.0f,
			.T		= 0.08f,  // t / T: 0.00016f / 0.001f
			.calc	= TFilter_Calc,
		},
	.filterIdes =
		{
			.input	= 0.0f,
			.output = 0.0f,
			.T		= 0.00016f,	 // t / T: 0.00016f / 1.0f
			.calc	= TFilter_Calc,
		},

	.currentDesired = 0.0f,
	.currentMax		= 0.0f,
	.Ipi			= 0.0f,

	.speedDesiredKMPH	  = 0.0f,
	.speedCurrentKMPHRaw  = 0.0f,
	.speedCurrentKMPHFilt = 0.0f,

	.ctrlType  = UART_PROCESS_CTRL_TYPE_DUTY,
	.stopMotor = false,
};

ControlInfo_t* Motor_GetControlInfoPtr(void) {
	return &motor.controlInfo;
}

Motor_t* Motor_GetMotorPtr(void) {
	return &motor;
}

int Motor_DetermineDirection(int prev, int curr) {
	static const int forwardSequence[6] = {1, 3, 2, 6, 4, 5};  // 5, 4, 6, 2, 3, 1

	for (int i = 0; i < 6; i++) {
		if (prev == forwardSequence[i]) {
			int nextExpected = forwardSequence[(i + 1) % 6];
			int prevExpected = forwardSequence[(i - 1 + 6) % 6];
			if (curr == nextExpected) {
				return 1;
			}
			if (curr == prevExpected) {
				return -1;
			}
		}
	}

	return 0;
}

void Motor_HallCallback(u32 idHall) {
	Sensors_t* sensors = Sensors_GetPtrSensors();

	if (motor.controlInfo.timSpeedIsInterrupt) {
		motor.controlInfo.timSpeedIsInterrupt = 0;
		motor.speedCurrentKMPHRaw			  = 0.0;
	} else {
		u32 cnt = Pl_TIM_HallToggleTimeCalc_GetCnt();
		if (cnt > 20) {	 // надо проверить работу без этого условия
			motor.speedCurrentKMPHRaw = KOEF_TIM_CNT_TO_KMPH / (cnt + 1);
		}
	}

	Pl_TIM_HallToggleTimeCalc_ResetCnt();

	sensors->halls = Pl_Motor_GetStateHalls();

	motor.controlInfo.ControlTypeFunc(Sensors_GetPtrSensors()->halls,
									  motor.controlInfo.directionWheel, motor.stopMotor);

	if (sensors->halls != sensors->hallsPrev) {
		motor.controlInfo.direction = Motor_DetermineDirection(sensors->hallsPrev, sensors->halls);
		sensors->hallsPrev			= sensors->halls;
	}
}

void Motor_HallToggleTimeCalcCallback(void) {
	motor.speedCurrentKMPHRaw = 0.0f;
}

TFilter FilterSpeed = {
	.input	= 0.0f,
	.output = 0.0f,
	.T		= 0.05f,  // t / T: 0.001f / 0.02
	.calc	= TFilter_Calc,
};

TFilter FilterSpeedDes = {
	.input	= 0.0f,
	.output = 0.0f,
	.T		= 0.0005f,	// t / T: 0.001f / 2
	.calc	= TFilter_Calc,
};

volatile struct PI_t SpeedPI = {
	.Kp		  = 1.0f,
	.Ki		  = 0.0f,
	.Kd		  = 0.0f,
	.dt		  = 0.00016f,
	.in		  = 0.0f,
	.out	  = 0.0f,
	.outPrev  = 0.0f,
	.outMax	  = 100.0f,
	.outMin	  = -100.0f,
	.integral = 0.0f,
	.calc	  = PID_Calc,
};

void Motor_TimSpeedControlClbk(void) {
	FilterSpeed.input = motor.speedCurrentKMPHRaw;
	FilterSpeed.calc(&FilterSpeed);
	motor.speedCurrentKMPHFilt = FilterSpeed.output;

	SpeedPI.in = motor.speedDesiredKMPH - motor.speedCurrentKMPHFilt;
	SpeedPI.calc(&SpeedPI);
	motor.controlInfo.deltaSpeedPI_kmph = SpeedPI.out;

	FilterSpeedDes.input = motor.speedDesiredKMPH;
	FilterSpeedDes.calc(&FilterSpeedDes);

	if (motor.ctrlType == UART_PROCESS_CTRL_TYPE_DUTY && !motor.stopMotor) {
		u32 dutyCycle = 68.0f * FilterSpeedDes.output;
		if (dutyCycle < 0) {
			dutyCycle = 0;
		} else if (dutyCycle > 6799) {
			dutyCycle = 6799;
		}
		motor.controlInfo.dutyCycle = (float)dutyCycle;

		Pl_Motor_SetComparePWMA(dutyCycle);
		Pl_Motor_SetComparePWMB(dutyCycle);
		Pl_Motor_SetComparePWMC(dutyCycle);
	}
}

void Motor_SensHighFreqClbk(u16* buffPtr, u16 buffLen) {
	motor.filterCurA.input = (float)buffPtr[0];
	motor.filterCurA.calc(&motor.filterCurA);
	motor.controlInfo.curA = (motor.filterCurA.output - MOTOR_SENS_CUR_A_OFFSET_mV) /
							 MOTOR_SENS_CUR_KOEF / MOTOR_SENS_CUR_R_SHUNT_mOhm;
	// motor.controlInfo.curA = buffPtr[0];

	motor.filterCurB.input = (float)buffPtr[1];
	motor.filterCurB.calc(&motor.filterCurB);
	motor.controlInfo.curB = (motor.filterCurB.output - MOTOR_SENS_CUR_B_OFFSET_mV) /
							 MOTOR_SENS_CUR_KOEF / MOTOR_SENS_CUR_R_SHUNT_mOhm;
	// motor.controlInfo.curB = buffPtr[1];

	motor.filterCurC.input = (float)buffPtr[2];
	motor.filterCurC.calc(&motor.filterCurC);
	motor.controlInfo.curC = (motor.filterCurC.output - MOTOR_SENS_CUR_C_OFFSET_mV) /
							 MOTOR_SENS_CUR_KOEF / MOTOR_SENS_CUR_R_SHUNT_mOhm;
	// motor.controlInfo.curC = buffPtr[2];
}

void Motor_TimPwmCntTopCallback(void) {
	// Pl_DMA_AdcHighFreq_Enable();
	Pl_ADC_StartHighFreqConv();
}

void Motor_DefineCurMax(void) {
	float Ia = (motor.controlInfo.curA < 0) ? -motor.controlInfo.curA : motor.controlInfo.curA;
	float Ib = (motor.controlInfo.curB < 0) ? -motor.controlInfo.curB : motor.controlInfo.curB;
	float Ic = (motor.controlInfo.curC < 0) ? -motor.controlInfo.curC : motor.controlInfo.curC;

	motor.currentMax = (Ia + Ib + Ic) / 2;
	// if (Ib > motor.currentMax) {
	//     motor.currentMax = Ib;
	// }

	// if (Ic > motor.currentMax) {
	//     motor.currentMax = Ic;
	// }

	// if (motor.currentMax < 0.05f) {
	//     motor.currentMax = 0.0f;
	// }
}

struct PI_t currentPI = {
	.Kp		  = 0.05f,
	.Ki		  = 100.0f,
	.Kd		  = 0.0f,
	.dt		  = 0.00016f,
	.in		  = 0.0f,
	.out	  = 0.0f,
	.outPrev  = 0.0f,
	.outMax	  = 40.0f,
	.outMin	  = -40.0f,
	.integral = 0.0f,
	.calc	  = PID_Calc,
};

void Motor_ResetCurrentPI(void) {
	currentPI.integral = 0.0f;
	currentPI.in	   = 0.0f;
	currentPI.out	   = 0.0f;
	currentPI.outPrev  = 0.0f;
}

void Motor_TimPwmCntBottomCallback(void) {
	if (motor.ctrlType != UART_PROCESS_CTRL_TYPE_CURRENT) {
		return;
	}

	// определить максимальный ток
	Motor_DefineCurMax();

	// фильтрация требуемого тока
	motor.filterIdes.input = motor.currentDesired;
	motor.filterIdes.calc(&motor.filterIdes);

	if (motor.stopMotor || motor.currentDesired < 0.1f) {
		currentPI.in	   = 0.0f;
		currentPI.integral = 0.0f;
	} else {
		// if (motor.currentMax > 0.05 || motor.currentDesired > 0.1f) {
		currentPI.in = motor.filterIdes.output - motor.currentMax;
		// }
	}

	// вычислить ПИ-регулятор
	currentPI.calc(&currentPI);
	motor.Ipi = currentPI.out;

	// поменять коэф заполнения
	u32 dutyCycle = 170.0f * currentPI.out;
	if (dutyCycle < 0) {
		dutyCycle = 0;
	} else if (dutyCycle > 6799) {
		dutyCycle = 6799;
	}
	motor.controlInfo.dutyCycle = (float)dutyCycle;

	Pl_Motor_SetComparePWMA(dutyCycle);
	Pl_Motor_SetComparePWMB(dutyCycle);
	Pl_Motor_SetComparePWMC(dutyCycle);
}

void Motor_Init(void) {
	Pl_Motor_Init(Motor_TimPwmCntTopCallback, Motor_TimPwmCntBottomCallback,
				  Motor_TimSpeedControlClbk, Motor_SensHighFreqClbk);
	Sensors_Init(Motor_HallCallback, Motor_HallToggleTimeCalcCallback);
}
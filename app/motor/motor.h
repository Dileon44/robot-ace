#ifndef __MOTOR_H
#define __MOTOR_H

#include "controller.h"
#include "main.h"
#include "sensors.h"
#include "uart_process.h"

#define MOTOR_SPEED_KM_H_MAX 25.0f

typedef struct {
	bool directionWheel;
	u32 halls;
	void (*ControlTypeFunc)(u32, bool, bool);
	u8 timSpeedIsInterrupt;
	float deltaSpeed_kmph;
	float deltaSpeedPI_kmph;
	float dutyCycle;
	float rpm;
	float curA;
	float curB;
	float curC;
	s32 direction;
} ControlInfo_t;

// typedef void (*ControlTypeFunc_t)(ControlInfo_t*);

typedef struct {
	ControlInfo_t controlInfo;
	TFilter filterCurA;
	TFilter filterCurB;
	TFilter filterCurC;
	TFilter filterIdes;

	float currentDesired;
	float currentMax;
	float Ipi;

	float speedDesiredKMPH;
	float speedCurrentKMPHRaw;
	float speedCurrentKMPHFilt;

	UART_PROCESS_CTRL_TYPE_t ctrlType;
	bool stopMotor;
} Motor_t;

ControlInfo_t* Motor_GetControlInfoPtr(void);
Motor_t* Motor_GetMotorPtr(void);
void Motor_ResetCurrentPI(void);

void Motor_Init(void);
void Motor_TaskCreate(void);
void Motor_TaskDelete(void);
static void vTask_MotorProcess(void* pvParameters);
void FreeRTOS_Motor_InitComponents(void);

#endif /* __MOTOR_H */
#ifndef __UART_PROCESS_H
#define __UART_PROCESS_H

#include "main.h"

typedef enum {
	UART_PROCESS_CTRL_SRC_UART,
	UART_PROCESS_CTRL_SRC_THROTTLE,
} UART_PROCESS_CTRL_SRC_t;

typedef enum {
	UART_PROCESS_SHAPE_IN_STEP,
	UART_PROCESS_SHAPE_IN_SIN,
	UART_PROCESS_SHAPE_IN_MEANDER,
	UART_PROCESS_SHAPE_IN_SAWTOOTH,
} UART_PROCESS_SHAPE_IN_t;

typedef struct {
	float speedDesired;
} Uart_Process_ShapeInStep_t;

typedef struct {
	float ampl;
	float freq;
} Uart_Process_ShapeInSin_t;

typedef struct {
	float ampl;
	float freq;
} Uart_Process_ShapeInMeander_t;

typedef struct {
	float ampl;
	float freq;
} Uart_Process_ShapeInSawtooth_t;

typedef struct {
	UART_PROCESS_SHAPE_IN_t ShapeInCurrent;
	Uart_Process_ShapeInStep_t Step;
	Uart_Process_ShapeInSin_t Sin;
	Uart_Process_ShapeInMeander_t Meander;
	Uart_Process_ShapeInSawtooth_t Sawtooth;
} Uart_Process_ShapeIn_t;

typedef enum {
	UART_PROCESS_CTRL_TYPE_DUTY,
	UART_PROCESS_CTRL_TYPE_CURRENT,
	UART_PROCESS_CTRL_TYPE_SPEED,
} UART_PROCESS_CTRL_TYPE_t;

// typedef struct {
//     Uart_Process_ShapeIn_t shape;
// } Uart_Process_CtrlType_Duty_t;
// typedef struct {
//     Uart_Process_ShapeIn_t shape;
// } Uart_Process_CtrlType_Current_t;
// typedef struct {
//     Uart_Process_ShapeIn_t shape;
// } Uart_Process_CtrlType_Speed_t;

typedef struct {
	UART_PROCESS_CTRL_TYPE_t type;
	Uart_Process_ShapeIn_t duty;
	Uart_Process_ShapeIn_t current;
	Uart_Process_ShapeIn_t speed;
} Uart_Process_CtrlType_t;

typedef struct {
	struct Uart_Process_Buttons_t {
		bool isStop;
	} Buttons;

	struct Uart_Process_SendData_t {
		bool isSend;
	} UartSendData;

	UART_PROCESS_CTRL_SRC_t CtrlSrc;

	Uart_Process_CtrlType_t CtrlType;
	// struct Uart_Process_ShapeIn_t {
	//     UART_PROCESS_SHAPE_IN_t ShapeInCurrent;

	//     struct Uart_Process_ShapeInStep_t {
	//         float speedDesired;
	//     } Step;

	//     struct Uart_Process_ShapeInSin_t {
	//         float ampl;
	//         float freq;
	//     } Sin;

	//     struct Uart_Process_ShapeInMeander_t {
	//         float ampl;
	//         float freq;
	//     } Meander;

	//     struct Uart_Process_ShapeInSawtooth_t {
	//         float ampl;
	//         float freq;
	//     } Sawtooth;
	// } ShapeIn;

	struct Uart_Process_PrintParams_t {
		bool speedDesired_kmph;
		bool speedRaw_kmph;
		bool speedFilt_kmph;
		bool outPI_kmph;
		bool rpm;
		bool duty;
		bool Ia;
		bool Ib;
		bool Ic;
		bool Ides;
		bool Icur;
		bool Ipi;
		u32 numActive;
	} PrintParams;

	bool isCallIqrHall1;
} Uart_Process_RxData_t;

static void vTask_MotorUartProcessRecieve(void* pvParameters);
void Uart_Process_Rx_TaskCreate(void);
void FreeRTOS_MotorUartProcess_InitComponents();

#endif /* __UART_PROCESS_H */
#ifndef __SENSORS_H
#define __SENSORS_H

#include "filter.h"
#include "main.h"
#include "platform.h"

#define SENSORS_FILT_BUF_ELEM_MAX_NUM 5

typedef struct {
	u16 in;
	u32 out;
	u16 idxLastElem;
	u16 buff[SENSORS_FILT_BUF_ELEM_MAX_NUM];
} Sensors_MovingAverageFilt_t;

typedef struct {
	u8 throttlePercent;
	u8 brakePercent;

	u32 halls;
	u32 hallsPrev;
	// Sensors_MovingAverageFilt_t Sensors_ThrottleFilt;
	TFilter filterThrottle;
	TFilter filterBrake;

} Sensors_t;

void Sensors_Init(Pl_Motor_ClbkHall_t pHallClbk,
				  Pl_Motor_HallToggleTimeCalcClbk_t Sensors_HallToggleTimeCalcCallback);
Sensors_t* Sensors_GetPtrSensors(void);
void FreeRTOS_Sensors_InitComponents(void);
void Sensors_TaskDelete(void);

#endif /* __SENSORS_H */

#ifndef __BLDC_H
#define __BLDC_H

#include "../motor.h"
#include "main.h"

void BLDC_Control(u32 halls, bool direction, bool isStop);

#endif /* __BLDC_H */
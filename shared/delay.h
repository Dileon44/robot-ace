#ifndef __DELAY_H
#define __DELAY_H

#include "def_types.h"

#define DELAY_MAX_TIME 0xFFFFFFFFU

void Delay_Init(void);
void Delay_WaitTime_MilliSec(u32 time);
u32 Delay_TimeMilliSec_Get(void);
u64 Delay_TimeMicroSec_Get(void);
double Delay_TimeAccurate_Get(void);
void Delay_SuspendTimer(void);
void Delay_ResumeTimer(void);
void Delay_TimIntCallback(void);

#endif /* __DELAY_H */

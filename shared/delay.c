#include "delay.h"
#include "platform.h"

volatile static u32 MilliSecAfterStart = 0;

static void Delay_TimIntCallback(void) {
	//HAL_IncTick();
	MilliSecAfterStart++;
}

bool Delay_Init(void) {
	return Pl_DelayMs_Init(Delay_TimIntCallback);
}

bool Delay_DeInit(void) {
	return Pl_DelayMs_DeInit();
}

void Delay_WaitTime_MilliSec(u32 ms) {
	u32 startTime = Delay_TimeMilliSec_Get();
	while ((Delay_TimeMilliSec_Get() - startTime) < ms) {
	}
}

void Delay_WaitTime_MicroSec(u64 us) {
	u64 startTime = Delay_TimeMicroSec_Get();
	while ((Delay_TimeMicroSec_Get() - startTime) < us) {
	}
}

u32 Delay_TimeMilliSec_Get(void) {
	return MilliSecAfterStart;
}

u64 Delay_TimeMicroSec_Get(void) {
	//one u32 value can hold ~1.19 of an hour
	return (u64)(MilliSecAfterStart) * (u64)1000 + (u64)Pl_DelayMs_GetUsCnt();
}

double Delay_TimeAccurate_Get(void) {
	return (double)MilliSecAfterStart + (double)Pl_DelayMs_GetUsCnt() * 0.001;
}

void Delay_SuspendTimer(void) {
	Pl_DelayMs_SuspendTimer();
}

void Delay_ResumeTimer(void) {
	Pl_DelayMs_ResumeTimer();
}

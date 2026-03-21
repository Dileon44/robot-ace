#include "delay.h"
#include "platform.h"

volatile static u32 MilliSecAfterStart = 0;

void Delay_Init(void) {
	Pl_Delay_Init(Delay_TimIntCallback);
}

void Delay_WaitTime_MilliSec(u32 time) {
	// u32 startTime = MilliSecAfterStart;
	// while((MilliSecAfterStart - startTime) < time);
}

u32 Delay_TimeMilliSec_Get(void) {
	return MilliSecAfterStart;
}

u64 Delay_TimeMicroSec_Get(void) {	//one u32 value can hold ~1.19 of an hour
	// return (u64)(MilliSecAfterStart) * (u64)1000 + (u64)Pl_Delay_GetCnt();
	return 0;
}

double Delay_TimeAccurate_Get(void) {
	// return (double)MilliSecAfterStart + (double)Pl_Delay_GetCnt() * 0.001;
	return 0.0;
}

void Delay_SuspendTimer(void) {
	// Pl_Delay_SuspendTimer();
}

void Delay_ResumeTimer(void) {
	// Pl_Delay_ResumeTimer();
}

void Delay_TimIntCallback(void) {
	MilliSecAfterStart++;
}


#ifndef __DEF_SYS_H
#define __DEF_SYS_H

#include "def_types.h"
#include "main_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MAIN_CFG_USE_RTOS
#define SYS_USE_RTOS 1
#else
#define SYS_USE_BAREMETAL 1
#endif
// #define SUPPRESS_WARNINGS

// clang-format off

#ifndef SYS_CRITICAL
#if SYS_USE_BAREMETAL
#include "platform.h"
#define SYS_CRITICAL_ON()				Pl_IrqOff()	 // no nesting calls
#define SYS_CRITICAL_OFF()				Pl_IrqOn()
#define SYS_CRITICAL_ON_ISR()			Pl_IrqOff()
#define SYS_CRITICAL_OFF_ISR(a)			Pl_IrqOn()
#define SYS_OS_IS_RUNNING()				(false)
#elif SYS_USE_RTOS
#include "def_rtos.h"
#define SYS_CRITICAL_ON()				taskENTER_CRITICAL()
#define SYS_CRITICAL_OFF()				taskEXIT_CRITICAL()
#define SYS_CRITICAL_ON_ISR()			taskENTER_CRITICAL_FROM_ISR()
#define SYS_CRITICAL_OFF_ISR(a)			taskEXIT_CRITICAL_FROM_ISR(a)
#define SYS_OS_IS_RUNNING()				(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
#else
#ifndef SUPPRESS_WARNINGS
#warning SYS_CRITICAL functions are not defined!
#endif /* SUPPRESS_WARNINGS */
#define SYS_CRITICAL_ON()
#define SYS_CRITICAL_OFF()
#define SYS_CRITICAL_ON_ISR()
#define SYS_CRITICAL_OFF_ISR(a)
#define SYS_OS_IS_RUNNING() (false)
#endif
#endif /* SYS_CRITICAL */

#ifndef SYS_DELAY
#if SYS_USE_BAREMETAL
#include "delay.h"
#define SYS_TICK_GET_MS_CNT()			Delay_TimeMilliSec_Get()
#define SYS_DELAY_MS(a)					Delay_WaitTime_MilliSec(a)
#define SYS_DELAY_UNTIL_MS(prev, incr)	do {              		\
											(void)(prev); 		\
											SYS_DELAY_MS(incr);	\
										} while (0)
#define SYS_MAX_TIMEOUT					(0xFFFFFFFFUL)
#elif SYS_USE_RTOS
#include "def_rtos.h"
#define SYS_TICK_GET_MS_CNT()			xTaskGetTickCount()
#define SYS_DELAY_MS(a)					vTaskDelay(a)
#define SYS_DELAY_UNTIL_MS(prev, incr)	vTaskDelayUntil(prev, incr)
#define SYS_MAX_TIMEOUT					portMAX_DELAY
#else
#ifndef SUPPRESS_WARNINGS
#warning SYS_DELAY functions are not defined!
#endif /* SUPPRESS_WARNINGS */
#define SYS_TICK_GET_MS_CNT()			(0)
#define SYS_DELAY_MS(a)		  			((void)(a))
#define SYS_DELAY_UNTIL_MS(prev, incr)	do {				\
											(void)(prev); 	\
											(void)(incr); 	\
										} while (0)
#define SYS_MAX_TIMEOUT					(0xFFFFFFFFUL)
#endif
#endif /* SYS_DELAY */

#ifndef PL_DELAY
#include "delay.h"
#define PL_DELAY_MS(a)					Delay_WaitTime_MilliSec(a)	//HAL_Delay(a)
#define PL_DELAY_US(a)					Delay_WaitTime_MicroSec(a)	
#define PL_GET_MS_CNT()					Delay_TimeMilliSec_Get()	//HAL_GetTick()
#define PL_GET_US_CNT()					Delay_TimeMicroSec_Get()
#define PL_MAX_TIMEOUT					(0xFFFFFFFFUL)
#endif /* PL_DELAY */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DEF_SYS_H */

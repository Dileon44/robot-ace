
#ifndef __DEF_SYS_H
#define __DEF_SYS_H

#include "delay.h"
#define PL_DELAY_MS(a)	(Delay_WaitTime_MilliSec(a))
#define PL_GET_MS_CNT() (Delay_TimeMilliSec_Get())
#define PL_GET_US_CNT() (Delay_TimeMicroSec_Get())

#if USE_OS_DELAY
#include "FreeRTOS.h"
#include "task.h"
#define SYS_DELAY_MS(a)		  (vTaskDelay(a))
#define SYS_TICK_GET_MS_CNT() (xTaskGetTickCount())
#define SYS_MAX_TIMEOUT		  (portMAX_DELAY)
#elif USE_BARE_METAL_DELAY
#define SYS_DELAY_MS(a)		  (Delay_WaitTime_MilliSec(a))
#define SYS_TICK_GET_MS_CNT() (Delay_TimeMilliSec_Get())
#define SYS_MAX_TIMEOUT		  (0xFFFFFFFFUL)
#else
#define SYS_TICK_GET_MS_CNT()
#define SYS_DELAY_MS(a)
#define SYS_MAX_TIMEOUT
#endif /* USE_OS_DELAY */

#ifndef SYS_CRITICAL
#if USE_OS
#define SYS_CRITICAL_ON()		(taskENTER_CRITICAL())
#define SYS_CRITICAL_OFF()		(taskEXIT_CRITICAL())
#define SYS_CRITICAL_ON_ISR()	(taskENTER_CRITICAL_FROM_ISR())
#define SYS_CRITICAL_OFF_ISR(x) (taskEXIT_CRITICAL_FROM_ISR(x))
#define SYS_OS_IS_RUNNING()		(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
#elif USE_BARE_METAL
#include "platform.h"
#define SYS_CRITICAL_ON()		(Pl_IrqOff())
#define SYS_CRITICAL_OFF()		(Pl_IrqOn())
#define SYS_CRITICAL_ON_ISR()	(Pl_IrqOff())
#define SYS_CRITICAL_OFF_ISR(a) (Pl_IrqOn())
#define SYS_OS_IS_RUNNING()		(false)
#else
#define SYS_CRITICAL_ON()
#define SYS_CRITICAL_OFF()
#define SYS_CRITICAL_ON_ISR() ((u32)(0))
#define SYS_CRITICAL_OFF_ISR(x)
#define SYS_OS_IS_RUNNING() (false)
#endif
#endif /* SYS_CRITICAL */

#endif /* __DEF_SYS_H */

#ifndef __DEF_RTOS_H
#define __DEF_RTOS_H

#include "main_cfg.h"

#ifdef USE_OS
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "rtos_tasks_stack_and_prio.h"
#endif /* USE_OS */

#endif /* __DEF_RTOS_H */
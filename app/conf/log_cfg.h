/**
 * @file log_cfg.h
 * @brief Host configuration of the portable log core (lib/log).
 *
 * Every macro here is an optional `#ifndef` override consumed by log.h. The core
 * has working defaults for all of them (no timers, "n/a" for time/date and task
 * name), so this file only wires in what this project actually has.
 */

#ifndef __LOG_CFG_H
#define __LOG_CFG_H

#include "dbg_cfg.h"
#include "def_rtos.h"
#include "delay.h"

// clang-format off

/**
 * Module switch. 0 -> every print macro becomes a do-nothing statement and the
 * API collapses to stubs, so the project still builds without a log transport.
 */
#ifndef LOG_ENABLE
#define LOG_ENABLE						1
#endif /* LOG_ENABLE */

/* Initial log level (DEBUG_QUICK_ENABLE in dbg_cfg.h may set it earlier). */
#ifndef LOG_DEF_LVL
#if DEBUG_ENABLE
#define LOG_DEF_LVL						LOG_LVL_DEBUG
#else /* DEBUG_ENABLE */
#define LOG_DEF_LVL						LOG_LVL_INFO
#endif /* DEBUG_ENABLE */
#endif /* LOG_DEF_LVL */

/* 1 -> short file name in the log header, 0 -> full __FILE__ path. */
#define LOG_USE_FILE_NAME				1

/* Stack buffer of LOG_RAW_DIRECT — tasks here run on small stacks. */
#define LOG_DIRECT_BUFF_SIZE			256

/* Free-running timers behind the log header and LOG_EXEC_TIME_*. */
#define LOG_TIMER_MILLISEC_GET()		Delay_TimeMilliSec_Get()
#define LOG_TIMER_MICROSEC_GET()		Delay_TimeMicroSec_Get()

/* Current task name in the log header. */
#define LOG_TASK_NAME_STR_SIZE			24
#define LOG_TASK_NAME_STR_GET(a, b)		do { \
											TaskHandle_t _th = xTaskGetCurrentTaskHandle(); \
											snprintf((a), (b), "%s", _th ? pcTaskGetName(_th) : "n/a"); \
										} while (0)

/*
 * No RTC on this board yet, so the time/date field stays out of the log header
 * (LOG_USE_TIME_DATE defaults to 0) instead of printing "n/a" on every line.
 * Once an RTC exists, three lines turn it on:
 *
 *   #define LOG_USE_TIME_DATE			 1
 *   #define LOG_TD_STR_SIZE			 16
 *   #define LOG_TIME_DATE_STR_GET(a, b) Rtc_GetTimeDateStr(a, b)
 */

// clang-format on

#endif /* __LOG_CFG_H */

/**
 * @file log.h
 * @brief Logging core.
 *
 * This is the portable core of the log module: log macros, log-level table and
 * state, the `_write` retarget and the transport dispatch. It carries NO
 * platform (UART/DMA) and NO RTOS (queues/tasks) code — those live in the host
 * glue (see log_root.h / log_root.c):
 *
 *   - the physical transport is injected as a vtable via Log_TransportRegister;
 *   - asynchronous output is injected as a sink via Log_SetOutputSink (when a
 *     sink is set, `_write` hands bytes to it — e.g. an RTOS queue — instead of
 *     transmitting synchronously).
 *
 * With neither registered the module still works in a minimal, synchronous mode
 * (printf -> _write -> transport). Everything host-specific (timers, task name,
 * time/date, default log level, file-name mode) is configured through
 * log_cfg.h via the LOG_* overrides below.
 */

#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>

#include "def_macro.h"
#include "def_types.h"
#include "log_cfg.h"

#include "term.h"

// Module switch. When 0 the print macros collapse to no-ops and the API becomes
// stubs (log.c / log_io.c) — so a project can build its structure with the
// application snippets before the hardware transport is written. Defaults to 1.
#ifndef LOG_ENABLE
#define LOG_ENABLE 1
#endif /* LOG_ENABLE */

// clang-format off

/* -- Host-configurable hooks (override in log_cfg.h) ------------------------ */

#ifndef LOG_DEF_LVL
#define LOG_DEF_LVL					LOG_LVL_INFO
#endif /* LOG_DEF_LVL */

/* 1 -> short file name in the header, 0 -> full __FILE__ path. */
#ifndef LOG_USE_FILE_NAME
#define LOG_USE_FILE_NAME			1
#endif /* LOG_USE_FILE_NAME */

/* Local buffer of LOG_RAW_DIRECT — lives on the caller's stack, keep it small. */
#ifndef LOG_DIRECT_BUFF_SIZE
#define LOG_DIRECT_BUFF_SIZE		256
#endif /* LOG_DIRECT_BUFF_SIZE */

/* Free-running timers used in the log header (ms tick + us for exec timing). */
#ifndef LOG_TIMER_MILLISEC_GET
#define LOG_TIMER_MILLISEC_GET()	0
#endif /* LOG_TIMER_MILLISEC_GET */

#ifndef LOG_TIMER_MICROSEC_GET
#define LOG_TIMER_MICROSEC_GET()	0
#endif /* LOG_TIMER_MICROSEC_GET */

/*
 * Time/date field of the log header, off by default: without a host RTC it only
 * prints a placeholder on every single line. Turn it on together with a real
 * LOG_TIME_DATE_STR_GET once an RTC exists.
 */
#ifndef LOG_USE_TIME_DATE
#define LOG_USE_TIME_DATE			0
#endif /* LOG_USE_TIME_DATE */

#ifndef LOG_TD_STR_SIZE
#define LOG_TD_STR_SIZE				8
#endif /* LOG_TD_STR_SIZE */

#ifndef LOG_TIME_DATE_STR_GET
#define LOG_TIME_DATE_STR_GET(a, b)	snprintf(a, b, "n/a")
#endif /* LOG_TIME_DATE_STR_GET */

/* Current task name for the log header (RTOS). Defaults to "n/a". */
#ifndef LOG_TASK_NAME_STR_SIZE
#define LOG_TASK_NAME_STR_SIZE		4
#endif /* LOG_TASK_NAME_STR_SIZE */

#ifndef LOG_TASK_NAME_STR_GET
#define LOG_TASK_NAME_STR_GET(a, b)	snprintf(a, b, "n/a")
#endif /* LOG_TASK_NAME_STR_GET */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * The time/date field is spliced in as three pieces — format, local declaration
 * and the printf argument — so LOG_USE_TIME_DATE drops it completely instead of
 * leaving a placeholder on every line. LOG_TD_ARG carries its own comma.
 */
#if LOG_USE_TIME_DATE
#define LOG_TD_FMT		"[%s] "
#define LOG_TD_DECL()	char _td_str[LOG_TD_STR_SIZE] = ""; \
						LOG_TIME_DATE_STR_GET(_td_str, LOG_TD_STR_SIZE)
#define LOG_TD_ARG		_td_str,
#else /* LOG_USE_TIME_DATE */
#define LOG_TD_FMT		""
#define LOG_TD_DECL()	do { } while(0)
#define LOG_TD_ARG
#endif /* LOG_USE_TIME_DATE */

#define LOG_HEADER_FMT	LOG_TD_FMT "[%lu] [fi: %s, th: %s, fn: %s, ln: %u]:\r\n\t  " // [Time/Date] [TICK_CNT] [FILENAME, TASK, FUNCTION, LINE]

/* -- Log levels ------------------------------------------------------------- */

#define LOG_LVL_TABLE()\
X_ENTRY(LOG_LVL_TRACE,		"  TRACE",	TERM_COLOR_CYAN,	"🟪") \
X_ENTRY(LOG_LVL_DEBUG,		"  DEBUG",	TERM_COLOR_BLUE,	"🟦") \
X_ENTRY(LOG_LVL_INFO,		"   INFO",	TERM_COLOR_GREEN,	"🟩") \
X_ENTRY(LOG_LVL_WARNING,	"   WARN",	TERM_COLOR_YELLOW,	"🟨") \
X_ENTRY(LOG_LVL_ERROR,		"  ERROR",	TERM_COLOR_RED,		"🟧") \
X_ENTRY(LOG_LVL_CRITIC,		" CRITIC",	TERM_COLOR_RED,		"🟥") \
X_ENTRY(LOG_LVL_DISABLE,	"DISABLE",	TERM_COLOR_WHITE,	"⬛") \

#define X_ENTRY(lvl, lvl_str, lvl_color, lvl_emoji) lvl,
typedef enum {
	LOG_LVL_TABLE()
} LOG_LVL_t;
#undef X_ENTRY

typedef struct {
	const char* Str;
	const char* Color;
	const char* Emoji;
} LogLvl_t;

typedef struct {
	char* Ptr;
	u32 Len;
} LogMsg_t;

/* -- Transport / sink injection --------------------------------------------- */

/*
 * Physical transport backend (UART/DMA/...). Implemented and registered by the
 * host (see log_root.c). All fields are optional: an unregistered transport
 * makes the corresponding Log_* call a safe no-op.
 */
typedef struct {
	bool (*init)(void);
	bool (*deinit)(void);
	bool (*transmit)(const u8* pBuff, u32 size);
	bool (*is_ready)(void);
} Log_Transport_t;

/*
 * Output sink for `_write`. When registered (host RTOS driver), stdout bytes are
 * handed to it instead of being transmitted synchronously — e.g. copied into a
 * queue drained by a low-priority task. NULL => synchronous transmit.
 */
typedef void (*Log_OutputSink_t)(const char* pBuff, u32 len);

/*
 * Defined regardless of LOG_ENABLE: the short file name is useful outside the
 * log header too, and Log_CutFilePath stays linkable in both branches.
 */
#if LOG_USE_FILE_NAME
#define LOG_FILENAME							Log_CutFilePath(__FILE__)
#else /* LOG_USE_FILE_NAME */
#define LOG_FILENAME							__FILE__
#endif /* LOG_USE_FILE_NAME */

/* -- Print macros ----------------------------------------------------------- */

#if LOG_ENABLE

#define LOG_RAW_DIRECT(_f_, ...)				do { \
														char _log_str[LOG_DIRECT_BUFF_SIZE] = ""; \
														snprintf(_log_str, sizeof(_log_str), (_f_), ##__VA_ARGS__); \
														Log_TransmitBuff(_log_str, strlen(_log_str)); \
													} while(0)

#define LOG_RAW_DIRECT_NL(_f_, ...)				do { \
														LOG_RAW_DIRECT(_f_ "\r\n", ##__VA_ARGS__); \
													} while(0)

#define LOG_RAW(_f_, ...)						do { \
														printf((_f_), ##__VA_ARGS__); \
														fflush(stdout); \
													} while(0)

#define LOG_RAW_COLOR(c, _f_, ...)				do { \
														LOG_RAW(c _f_ TERM_RESET_STYLE, ##__VA_ARGS__); \
													} while(0)

#define LOG_RAW_NL(_f_, ...)					do { \
														LOG_RAW(_f_ "\r\n", ##__VA_ARGS__); \
													} while(0)

#define LOG_RAW_COLOR_NL(c, _f_, ...)			do { \
														LOG_RAW_NL(c _f_ TERM_RESET_STYLE, ##__VA_ARGS__); \
													} while(0)

#define LOG_FUNC_ENTER()						do { \
														LOG_RAW("%s(): ENTER\r\n", __FUNCTION__); \
													} while(0)

#define LOG_EXEC_TIME_START()					u64 _t0 = LOG_TIMER_MICROSEC_GET()

#define LOG_EXEC_TIME_STOP()					float _dt = (float)(LOG_TIMER_MICROSEC_GET() - _t0); \
													LOG_RAW("%s(): DURATION %.3fms\r\n", \
														__FUNCTION__, _dt / 1000.0f)

#define LOG_FUNC_EXIT(rs)						do { \
														LOG_RAW("%s(): EXIT with [%s]\r\n\r\n", \
															__FUNCTION__, RetState_GetStr(rs)); \
													} while(0)

#define LOG(_f_, ...)							do { \
														LOG_TD_DECL(); \
														char _tn_str[LOG_TASK_NAME_STR_SIZE] = ""; \
														LOG_TASK_NAME_STR_GET(_tn_str, LOG_TASK_NAME_STR_SIZE); \
														LOG_RAW(LOG_HEADER_FMT _f_ "\r\n", \
																LOG_TD_ARG \
																(unsigned long)LOG_TIMER_MILLISEC_GET(), \
																LOG_FILENAME, \
																_tn_str, \
																__FUNCTION__, \
																(unsigned int)__LINE__, \
																##__VA_ARGS__); \
													} while(0)

#define LOG_COLOR(c, _f_, ...)					do { \
														LOG(c _f_ TERM_RESET_STYLE, ##__VA_ARGS__); \
													} while(0)

#define LOG_LVL(l, _f_, ...)					do { \
														if(l >= LOG_LVL_DISABLE) \
															break; \
														if(l >= Log_Lvl_Get()) \
														{ \
															LOG_TD_DECL(); \
															char _tn_str[LOG_TASK_NAME_STR_SIZE] = ""; \
															LOG_TASK_NAME_STR_GET(_tn_str, LOG_TASK_NAME_STR_SIZE); \
															LOG_RAW("[%s%s%s] " TERM_COLOR_CYAN LOG_HEADER_FMT TERM_RESET_STYLE _f_ "\r\n", \
																	Log_Lvl_GetColor(l), \
																	Log_Lvl_GetStr(l), \
																	TERM_RESET_STYLE, \
																	LOG_TD_ARG \
																	(unsigned long)LOG_TIMER_MILLISEC_GET(), \
																	LOG_FILENAME, \
																	_tn_str, \
																	__FUNCTION__, \
																	(unsigned int)__LINE__, \
																	##__VA_ARGS__); \
														} \
													} while(0)

/* Same as LOG_LVL but without the current-level filter — for the
 * LOCAL_LOG_PRINT pattern, where a module enables its own traces by hand. */
#define LOG_LVL_LOCAL(l, _f_, ...)				do { \
														if(l >= LOG_LVL_DISABLE) \
															break; \
														LOG_TD_DECL(); \
														char _tn_str[LOG_TASK_NAME_STR_SIZE] = ""; \
														LOG_TASK_NAME_STR_GET(_tn_str, LOG_TASK_NAME_STR_SIZE); \
														LOG_RAW("[%s%s%s] " TERM_COLOR_CYAN LOG_HEADER_FMT TERM_RESET_STYLE _f_ "\r\n", \
																Log_Lvl_GetColor(l), \
																Log_Lvl_GetStr(l), \
																TERM_RESET_STYLE, \
																LOG_TD_ARG \
																(unsigned long)LOG_TIMER_MILLISEC_GET(), \
																LOG_FILENAME, \
																_tn_str, \
																__FUNCTION__, \
																(unsigned int)__LINE__, \
																##__VA_ARGS__); \
													} while(0)

#else /* LOG_ENABLE */

/* Module disabled: every print macro is a do-nothing statement. */
#define LOG_RAW_DIRECT(...)						do { } while(0)
#define LOG_RAW_DIRECT_NL(...)					do { } while(0)
#define LOG_RAW(...)							do { } while(0)
#define LOG_RAW_COLOR(...)						do { } while(0)
#define LOG_RAW_NL(...)							do { } while(0)
#define LOG_RAW_COLOR_NL(...)					do { } while(0)
#define LOG_FUNC_ENTER()						do { } while(0)
#define LOG_EXEC_TIME_START()					do { } while(0)
#define LOG_EXEC_TIME_STOP()					do { } while(0)
#define LOG_FUNC_EXIT(rs)						do { (void)(rs); } while(0)
#define LOG(...)								do { } while(0)
#define LOG_COLOR(...)							do { } while(0)
#define LOG_LVL(...)							do { } while(0)
#define LOG_LVL_LOCAL(...)						do { } while(0)

#endif /* LOG_ENABLE */

/* -- Per-level convenience wrappers (forward to LOG_LVL / its no-op) --------- */
#define LOG_TRACE(_f_, ...)		LOG_LVL(LOG_LVL_TRACE, _f_, ##__VA_ARGS__)
#define LOG_DEBUG(_f_, ...)		LOG_LVL(LOG_LVL_DEBUG, _f_, ##__VA_ARGS__)
#define LOG_INFO(_f_, ...)		LOG_LVL(LOG_LVL_INFO, _f_, ##__VA_ARGS__)
#define LOG_WARNING(_f_, ...)	LOG_LVL(LOG_LVL_WARNING, _f_, ##__VA_ARGS__)
#define LOG_ERROR(_f_, ...)		LOG_LVL(LOG_LVL_ERROR, _f_, ##__VA_ARGS__)
#define LOG_CRITIC(_f_, ...)	LOG_LVL(LOG_LVL_CRITIC, _f_, ##__VA_ARGS__)
// clang-format on

/** @brief Register the physical transport backend (UART/DMA). Host-provided. */
void Log_TransportRegister(const Log_Transport_t* pcTransport);

/** @brief Register/clear the `_write` output sink. NULL => synchronous transmit. */
void Log_SetOutputSink(Log_OutputSink_t sink);

/** @brief Init the registered transport (hardware bring-up). */
bool Log_Init(void);
bool Log_DeInit(void);
bool Log_HardwareIsInit(void);

/** @brief Transmit a buffer synchronously through the registered transport. */
bool Log_TransmitBuff(char* pBuff, u32 size);

void Log_Lvl_Set(LOG_LVL_t lvl);
LOG_LVL_t Log_Lvl_Get(void);
char Log_Lvl_GetChar(LOG_LVL_t lvl);
const char* Log_Lvl_GetStr(LOG_LVL_t lvl);
const char* Log_Lvl_GetColor(LOG_LVL_t lvl);
const char* Log_Lvl_GetEmoji(LOG_LVL_t lvl);

/** @brief Strips the directory part of a path — used by the log header. */
const char* Log_CutFilePath(const char* pPath);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LOG_H */

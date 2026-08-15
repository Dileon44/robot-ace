#ifndef __DEBUG_H
#define __DEBUG_H

#include "delay.h"
#include "main.h"
#include "ring_buff.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// clang-format off

/* ============================================================================
 * ANSI escape codes
 * ============================================================================ */
#define ESC_END_LINE              "\r\n"
#define ESC_SAVE_CURSOR           "\e7"
#define ESC_RESTORE_CURSOR        "\e8"
#define ESC_CLEAR_RIGHT_FROM_CURS "\e[0K"
#define ESC_RIGHT_ARROW           "\e[C"
#define ESC_LEFT_ARROW            "\e[D"
#define ESC_COLOR_RED             "\e[31m"
#define ESC_COLOR_GREEN           "\e[32m"
#define ESC_COLOR_YELLOW          "\e[33m"
#define ESC_COLOR_BLUE            "\e[34m"
#define ESC_COLOR_MAGENTA         "\e[35m"
#define ESC_COLOR_CYAN            "\e[36m"
#define ESC_COLOR_WHITE           "\e[37m"
#define ESC_RESET_STYLE           "\e[0m"
#define ECS_CLR_SCREEN            "\e[1;1H\e[2J"
#define ECS_SET_MODE_BOLD         "\e[1m"
#define ECS_RESET_MODE_BOLD       "\e[22m"

/* ============================================================================
 * Log levels  —  X-macro table
 * ============================================================================ */
#define DEBUG_LVL_TABLE() \
	X_ENTRY(LOG_LVL_TRACE,   "  TRACE", ESC_COLOR_CYAN)   \
	X_ENTRY(LOG_LVL_DEBUG,   "  DEBUG", ESC_COLOR_BLUE)   \
	X_ENTRY(LOG_LVL_INFO,    "   INFO", ESC_COLOR_GREEN)  \
	X_ENTRY(LOG_LVL_WARNING, "   WARN", ESC_COLOR_YELLOW) \
	X_ENTRY(LOG_LVL_ERROR,   "  ERROR", ESC_COLOR_RED)    \
	X_ENTRY(LOG_LVL_CRITIC,  " CRITIC", ESC_COLOR_RED)    \
	X_ENTRY(LOG_LVL_DISABLE, "DISABLE", ESC_COLOR_WHITE)

#define X_ENTRY(lvl, lvl_str, lvl_color) lvl,
typedef enum {
	DEBUG_LVL_TABLE()
} LOG_LVL_t;
#undef X_ENTRY

/* ============================================================================
 * Print macros
 * ============================================================================ */
#define DEBUG_PRINT(_f_, ...) \
	do { printf((_f_), ##__VA_ARGS__); fflush(stdout); } while (0)

#define DEBUG_COLOR_PRINT(_c_, _f_, ...) \
	do { DEBUG_PRINT(_c_ _f_ ESC_RESET_STYLE, ##__VA_ARGS__); } while (0)

#define DEBUG_PRINT_NL(_f_, ...) \
	do { DEBUG_PRINT(_f_ "\r\n", ##__VA_ARGS__); } while (0)

#define DEBUG_COLOR_PRINT_NL(_c_, _f_, ...) \
	do { DEBUG_PRINT_NL(_c_ _f_ ESC_RESET_STYLE, ##__VA_ARGS__); } while (0)

/* ============================================================================
 * Leveled log macro:
 *   [LEVEL] [ms] [file, task, func, line]:
 *       message
 * ============================================================================ */
#define DEBUG_TASK_NAME_SIZE 24

#define DEBUG_LOG_LVL_PRINT(_l_, _f_, ...) \
	do { \
		if ((_l_) >= LOG_LVL_DISABLE) break; \
		if ((_l_) >= Debug_LogLvl_Get()) { \
			char _tn_[DEBUG_TASK_NAME_SIZE] = ""; \
			TaskHandle_t _th_ = xTaskGetCurrentTaskHandle(); \
			snprintf(_tn_, sizeof(_tn_), "%s", _th_ ? pcTaskGetName(_th_) : "n/a"); \
			DEBUG_PRINT("[%s%s%s] [%lu] [fi:%s, th:%s, fn:%s, ln:%u]:\r\n\t  " _f_ "\r\n", \
				Debug_LogLvl_GetColor(_l_), \
				Debug_LogLvl_GetStr(_l_), \
				ESC_RESET_STYLE, \
				(u32)Delay_TimeMilliSec_Get(), \
				__FILE__, \
				_tn_, \
				__FUNCTION__, \
				(u32)__LINE__, \
				##__VA_ARGS__); \
		} \
	} while (0)

#define DEBUG_LOG_PRINT(_f_, ...) DEBUG_LOG_LVL_PRINT(LOG_LVL_DEBUG, _f_, ##__VA_ARGS__)

// clang-format on

/* ============================================================================
 * API
 * ============================================================================ */
void        Debug_Init(void);
RET_STATE_t Debug_Write(u8* ptr, u16 len);
RingBuff_t* Debug_GetRxBuff(void);

bool Debug_HardwareIsInit(void);
char Debug_ReceiveSymbol(u32 tmo);
bool Debug_SendChar(char ch, u32 tmo);
bool Debug_SendCharFromISR(char ch, BaseType_t* pWoken);
bool Debug_SendString(char* pStr, u32 tmo);

void        Debug_LogLvl_Set(LOG_LVL_t lvl);
LOG_LVL_t   Debug_LogLvl_Get(void);
const char* Debug_LogLvl_GetStr(LOG_LVL_t lvl);
const char* Debug_LogLvl_GetColor(LOG_LVL_t lvl);

TaskHandle_t Debug_GetSendHandle(void);
void         FreeRTOS_Debug_InitComponents(bool resources, bool tasks);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DEBUG_H */

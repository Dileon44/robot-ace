/**
 * @file log_root.h
 * @brief Host-side platform/OS glue for the log core.
 *
 * The reusable, RTOS-free core lives in log.h / log.c / log_io.c. This file adds
 * the pieces the core deliberately does not carry, mirroring how shell_root.c
 * wraps the RTOS-free wsh-shell:
 *
 *   - a physical serial transport (USART + DMA) registered into the core;
 *   - an asynchronous TX path: `_write` -> queue -> low-priority drain task
 *     (Log_SetOutputSink), so logging never blocks the caller;
 *   - an RX path (DMA ring buffer + symbol injection queue) feeding the shell.
 */

#ifndef __LOG_ROOT_H
#define __LOG_ROOT_H

#include "log.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void Log_PrintMainInfo(void);
void Log_PrintSysInfo(void);

/** @brief Create RTOS resources (queues, transport + sink) / the drain task. */
void FreeRTOS_LogSend_InitComponents(bool resources, bool tasks);

/** @brief Handle of the output-drain task (NULL without RTOS). */
TaskHandle_t LogSend_GetTaskHandle(void);

/** @brief Count of log messages dropped (alloc failure or full send queue). */
u32 Log_GetDroppedMsgCount(void);

/* -- RX path (feeds the shell) ---------------------------------------------- */

bool Log_SendChar(char ch, u32 waitTmo);
bool Log_SendCharFromISR(char ch, BaseType_t* pWoken);
bool Log_SendString(char* pStr, u32 waitTmo);
char Log_ReceiveSymbol(u32 delay);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LOG_ROOT_H */

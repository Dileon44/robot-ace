#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cmsis_compiler.h"

/**
 * @brief
 * Debug features
 * All macro in production release must be zeroes
 */
#define PANIC_CHECK_ENABLE		1
#if DEBUG_ENABLE
//------------------------------------------------------------------------------------------------------------
#ifndef ERR_HANDLE_ENABLE
#define ERR_HANDLE_ENABLE		0	//Enable assertion like an EXIT(RET_STATE_ERR_PARAM)
#endif
#endif /* DEBUG_ENABLE */

/**
 * @brief
 * Low-level features
 */
//------------------------------------------------------------------------------------------------------------
#ifndef USE_BARE_METAL
#define USE_BARE_METAL			0	// Use bare metal
#endif
//------------------------------------------------------------------------------------------------------------
#ifndef USE_BARE_METAL_DELAY
#define USE_BARE_METAL_DELAY	0	// Use only bare metal delays
#endif
//------------------------------------------------------------------------------------------------------------
#ifndef USE_OS
#define USE_OS					1	//Include FreeRTOS for API usage
#endif
//------------------------------------------------------------------------------------------------------------
#ifndef USE_OS_DELAY
#define USE_OS_DELAY			1	//Use FreeRTOS API to perform delays
#endif
//------------------------------------------------------------------------------------------------------------

#if USE_OS
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#endif /* USE_OS */

#include "def_types.h"
#include "def_macro.h"
#include "def_sys.h"

void ErrorHandler(char *pFile, int line);

#if PANIC_CHECK_ENABLE
#define PANIC()					do { \
									ErrorHandler(__FILE__, __LINE__); \
								} while(0)
#else /* PANIC_CHECK_ENABLE */
#define PANIC()
#endif /* PANIC_CHECK_ENABLE */

/**
 * ASSERT_CHECK() macro will return true if its checked parameter is true,
 * and will perform some (emergency) action if the checked parameter
 * is suddenly false
 */
#if ASSERT_CHECK_ENABLE
#define ASSERT_CHECK(x)			do { \
									if((x) == 0) { \
										PANIC(); \
									} \
								} while(0)
#else /* ASSERT_CHECK_ENABLE */
#define ASSERT_CHECK(x)	
#endif /* ASSERT_CHECK_ENABLE */

#if ERR_HANDLE_ENABLE
#define ERROR_HANDLER()	do { \
								ErrorHandler(__FILE__, __LINE__); \
							} while(0)
#else /* ERR_HANDLE_ENABLE */
#define ERROR_HANDLER()
#endif /* ERR_HANDLE_ENABLE */

#define EXIT(ret)			do { \
								switch(ret) \
								{ \
									case RET_STATE_ERR_PARAM: \
										ERROR_HANDLER(); \
										break; \
									default: \
										break; \
								} \
								return ret;	\
							} while(0)

// #define configASSERT(x) ASSERT_CHECK(x)

#endif /* __MAIN_H */

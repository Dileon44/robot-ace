#ifndef __MAIN_H
#define __MAIN_H

#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 0
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cmsis_compiler.h"

#if DEBUG_ENABLE

//------------------------------------------------------------------------------
#ifndef ERR_HANDLE_ENABLE
/** @brief Enable assertion like an EXIT(RET_STATE_ERR_PARAM) */
#define ERR_HANDLE_ENABLE 0
#endif
//------------------------------------------------------------------------------
#ifndef ASSERT_CHECK_ENABLE
/** @brief Enable assertion */
#define ASSERT_CHECK_ENABLE 0
#endif
//------------------------------------------------------------------------------
#ifndef PANIC_CHECK_ENABLE
/** @brief Enable panic */
#define PANIC_CHECK_ENABLE 0
#endif
//------------------------------------------------------------------------------
#endif /* DEBUG_ENABLE */

#include "def_macro.h"
#include "def_sys.h"
#include "def_types.h"

void ErrorHandler(char* pFile, int line);

#if PANIC_CHECK_ENABLE
#define PANIC()                           \
	do {                                  \
		ErrorHandler(__FILE__, __LINE__); \
	} while (0)
#else /* PANIC_CHECK_ENABLE */
#define PANIC()
#endif /* PANIC_CHECK_ENABLE */

/**
 * ASSERT_CHECK() macro will return true if its checked parameter is true,
 * and will perform some (emergency) action if the checked parameter
 * is suddenly false
 */
#if ASSERT_CHECK_ENABLE
#define ASSERT_CHECK(x) \
	do {                \
		if ((x) == 0) { \
			PANIC();    \
		}               \
	} while (0)
#else /* ASSERT_CHECK_ENABLE */
#define ASSERT_CHECK(x)
#endif /* ASSERT_CHECK_ENABLE */

#if ERR_HANDLE_ENABLE
#define ERROR_HANDLER()                   \
	do {                                  \
		ErrorHandler(__FILE__, __LINE__); \
	} while (0)
#else /* ERR_HANDLE_ENABLE */
#define ERROR_HANDLER()
#endif /* ERR_HANDLE_ENABLE */

#define EXIT(ret)                     \
	do {                              \
		switch (ret) {                \
			case RET_STATE_ERR_PARAM: \
				ERROR_HANDLER();      \
				break;                \
			default:                  \
				break;                \
		}                             \
		return ret;                   \
	} while (0)

#endif /* __MAIN_H */

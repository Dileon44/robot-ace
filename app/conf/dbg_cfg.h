#ifndef __ASSERT_H
#define __ASSERT_H

/**
 * Quick debug enable
 */
#define DEBUG_QUICK_ENABLE 0

#if DEBUG_QUICK_ENABLE
#define DEBUG_ENABLE			 1
#define PANIC_CHECK_ENABLE		 1
#define APP_ASSERT_CHECK_ENABLE	 1
#define RTOS_ASSERT_CHECK_ENABLE 1
#define DEBUG_DEF_LOG_LVL		 LOG_LVL_DEBUG
#endif /* DEBUG_QUICK_ENABLE */
//------------------------------------------------------------------------------

/**
 * Debug features
 * All macro in production release must be zeroes
 */
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 0
#endif /* DEBUG_ENABLE */

#if DEBUG_ENABLE
#ifndef PANIC_CHECK_ENABLE
#define PANIC_CHECK_ENABLE 0
#endif /* PANIC_CHECK_ENABLE */

#ifndef APP_ASSERT_CHECK_ENABLE
#define APP_ASSERT_CHECK_ENABLE 0
#endif /* APP_ASSERT_CHECK_ENABLE */

#ifndef RTOS_ASSERT_CHECK_ENABLE
#define RTOS_ASSERT_CHECK_ENABLE 0
#endif /* RTOS_ASSERT_CHECK_ENABLE */
#endif /* DEBUG_ENABLE */

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// const char*: __FILE__ is a string literal, C++ forbids binding it to char*
extern void ErrorHandler(const char* pFile, int line);

#ifdef __cplusplus
}
#endif /* __cplusplus */

//------------------------------------------------------------------------------

#if PANIC_CHECK_ENABLE
#define PANIC()                           \
	do {                                  \
		ErrorHandler(__FILE__, __LINE__); \
	} while (0)
#else /* PANIC_CHECK_ENABLE */
#define PANIC()
#endif /* PANIC_CHECK_ENABLE */

//------------------------------------------------------------------------------

/**
 * ASSERT_CHECK() macro will return true if its checked parameter is true,
 * and will perform some (emergency) action if the checked parameter
 * is suddenly false
 */
#if APP_ASSERT_CHECK_ENABLE
#define ASSERT_CHECK(x) \
	do {                \
		if ((x) == 0) { \
			PANIC();    \
		}               \
	} while (0)
#else /* APP_ASSERT_CHECK_ENABLE */
#define ASSERT_CHECK(x) \
	do {                \
		(void)(x);      \
	} while (0)
#endif /* APP_ASSERT_CHECK_ENABLE */

//------------------------------------------------------------------------------

#ifdef USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
#define assert_param(expr) ASSERT_CHECK(expr)
#else /* USE_FULL_ASSERT */
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

//------------------------------------------------------------------------------

#define configASSERT(x) ASSERT_CHECK(x)

#if RTOS_ASSERT_CHECK_ENABLE
#define configCHECK_FOR_STACK_OVERFLOW 1
#else /* RTOS_ASSERT_CHECK_ENABLE */
#define configCHECK_FOR_STACK_OVERFLOW 0
#endif /* RTOS_ASSERT_CHECK_ENABLE */

#endif /* __ASSERT_H */

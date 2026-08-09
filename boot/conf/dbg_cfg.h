#ifndef __ASSERT_H
#define __ASSERT_H

/**
 * Quick debug enable
 */
#define DEBUG_QUICK_ENABLE 0

#if DEBUG_QUICK_ENABLE
#define DEBUG_ENABLE           1
#define PANIC_CHECK_ENABLE     1
#define ASSERT_CHECK_ENABLE    1
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

#ifndef ASSERT_CHECK_ENABLE
#define ASSERT_CHECK_ENABLE 0
#endif /* ASSERT_CHECK_ENABLE */
#endif /* DEBUG_ENABLE */

//------------------------------------------------------------------------------

// const char*: __FILE__ is a string literal, C++ forbids binding it to char*
extern void ErrorHandler(const char* pFile, int line);

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
#if ASSERT_CHECK_ENABLE
#define ASSERT_CHECK(x) \
	do {                \
		if ((x) == 0) { \
			PANIC();    \
		}               \
	} while (0)
#else /* ASSERT_CHECK_ENABLE */
#define ASSERT_CHECK(x) \
	do {                \
		(void)(x);      \
	} while (0)
#endif /* ASSERT_CHECK_ENABLE */

//------------------------------------------------------------------------------

#ifdef USE_FULL_ASSERT
#define assert_param(expr) ASSERT_CHECK(expr)
#else /* USE_FULL_ASSERT */
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#endif /* __ASSERT_H */

#ifndef __MAIN_CFG_H
#define __MAIN_CFG_H

/**
 * @brief Enabling RTOS or Bare-Metal
 */
#ifndef USE_OS
#define USE_OS
#endif /* USE_OS */

/**
 * @brief System periodic health check
 */
#ifndef HEALTH_CHECK
#define HEALTH_CHECK 1
#endif /* HEALTH_CHECK */

/**
 * @brief Enable periodic RTOS stack and tasks stack checkout
 */
#ifndef RTOS_ANALYZER
#define RTOS_ANALYZER 1
#endif /* RTOS_ANALYZER */

/**
 * @brief Enable UART shell interface (wsh-shell)
 * Auth + auto-exit timer protect it in production builds.
 */
#ifndef SHELL_INTERFACE
#define SHELL_INTERFACE 1
#endif /* SHELL_INTERFACE */

#endif /* __MAIN_CFG_H */

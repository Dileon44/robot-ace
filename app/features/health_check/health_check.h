#ifndef __HEALTH_CHECK_H
#define __HEALTH_CHECK_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Implemented in health_check.cpp but called from main.c — needs C linkage.
void FreeRTOS_HealthCheck_InitComponents(bool resources, bool tasks);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HEALTH_CHECK_H */
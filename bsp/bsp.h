#ifndef __BSP_H
#define __BSP_H

#include "bsp_cfg.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RET_STATE_t Bsp_Init(void);
RET_STATE_t Bsp_DeInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_H */

#ifndef __BSP_H
#define __BSP_H

#include "bsp_cfg.h"
#include "main.h"

#ifdef __cplusplus
// C++ only: encoder classes for application code — bsp::encoder::GetPtr()->GetAngleDeg().
// Included before the extern "C" block on purpose: these are C++ declarations
// and must keep C++ linkage.
#include "encoder/encoder.hpp"

extern "C" {
#endif /* __cplusplus */

RET_STATE_t Bsp_Init(void);
RET_STATE_t Bsp_DeInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_H */

#ifndef __RAND_H
#define __RAND_H

#include "def_types.h"
#include "lib_cfg.h"

#if LIB_USE_RAND

void Rand_Init(void);
u32 Rand_GetNum(void);
void Rand_GetBuff(u32* pBuff, u32 maxNum);
s32 Rand_GetNumBetween(s32 min, s32 max);
bool Rand_GetBool(void);
void Rand_GetBuffBetween(u32* pBuff, u32 maxNum, s32 min, s32 max);
u32 Rand_GetStr(char* pBuff, u32 maxNum);

#endif /* LIB_USE_RAND */

#endif /* __RAND_H */

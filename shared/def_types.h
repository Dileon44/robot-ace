
#ifndef __DEF_TYPES_H
#define __DEF_TYPES_H
// clang-format off

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include "cmsis_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef struct {
	u32 Err;
	u32 Luck;
} ErrorCounter_t;

typedef void (*VoidFunc_t)(void);

#define RET_STATE_TABLE()\
X_ENTRY(RET_STATE_UNDEF,		"UNDEF")\
X_ENTRY(RET_STATE_SUCCESS,		"SUCCESS")\
X_ENTRY(RET_STATE_ERR_MEMORY,	"ERR_MEMORY")\
X_ENTRY(RET_STATE_ERR_CRC,		"ERR_CRC")\
X_ENTRY(RET_STATE_ERR_EMPTY,	"ERR_EMPTY")\
X_ENTRY(RET_STATE_ERR_PARAM,	"ERR_PARAM")\
X_ENTRY(RET_STATE_ERR_BUSY,		"ERR_BUSY")\
X_ENTRY(RET_STATE_ERR_OVERFLOW,	"ERR_OVERFLOW")\
X_ENTRY(RET_STATE_ERR_TIMEOUT,	"ERR_TIMEOUT")\
X_ENTRY(RET_STATE_ERROR,		"ERROR")\
X_ENTRY(RET_STATE_WARNING,		"WARNING")

#define X_ENTRY(state, state_str) state,
typedef enum {
	RET_STATE_TABLE()
} RET_STATE_t;
#undef X_ENTRY

#define RETURN_IF_UNSUCCESS(a)	if((a) != RET_STATE_SUCCESS) return(a)

const char* RetState_GetStr(RET_STATE_t retState);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DEF_TYPES_H */

#ifndef __DEF_TYPES_H
#define __DEF_TYPES_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef enum { BIT_RESET = 0, BIT_SET = 1 } BIT_t;

typedef enum { STATE_DISABLE = 0, STATE_ENABLE = !STATE_DISABLE } FUNCTIONAL_STATE_t;

#define RET_STATE_TABLE()                     \
	X(RET_STATE_UNDEF, "UNDEF")               \
	X(RET_STATE_SUCCESS, "SUCCESS")           \
	X(RET_STATE_ERR_MEMORY, "ERR_MEMORY")     \
	X(RET_STATE_ERR_CRC, "ERR_CRC")           \
	X(RET_STATE_ERR_EMPTY, "ERR_EMPTY")       \
	X(RET_STATE_ERR_PARAM, "ERR_PARAM")       \
	X(RET_STATE_ERR_BUSY, "ERR_BUSY")         \
	X(RET_STATE_ERR_OVERFLOW, "ERR_OVERFLOW") \
	X(RET_STATE_ERR_TIMEOUT, "ERR_TIMEOUT")   \
	X(RET_STATE_ERROR, "ERROR")               \
	X(RET_STATE_WARNING, "WARNING")

#define X(a, b) a,
typedef enum { RET_STATE_TABLE() } RET_STATE_t;
#undef X

const char* RetState_GetStr(RET_STATE_t retState);

#endif /* __DEF_TYPES_H */

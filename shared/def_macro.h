#ifndef __DEF_MACRO_H
#define __DEF_MACRO_H
// clang-format off

#define DELAY_1_MILSEC							(1)
#define DELAY_1_SECOND							(1000 * DELAY_1_MILSEC)
#define DELAY_1_MINUTE							(60 * DELAY_1_SECOND)
#define DELAY_1_HOUR							(60 * DELAY_1_MINUTE)
#define DELAY_1_DAY								(24 * DELAY_1_HOUR)

#define DELAY_1_SECOND_SEC						(1)
#define DELAY_1_MINUTE_SEC						(60 * DELAY_1_SECOND_SEC)
#define DELAY_1_HOUR_SEC						(60 * DELAY_1_MINUTE_SEC)
#define DELAY_1_DAY_SEC							(24 * DELAY_1_HOUR_SEC)

#define FREQ_1_HZ								(1)
#define FREQ_10_HZ								(10 * FREQ_1_HZ)
#define FREQ_100_HZ								(10 * FREQ_10_HZ)
#define FREQ_1_KHZ								(10 * FREQ_100_HZ)
#define FREQ_10_KHZ								(10 * FREQ_1_KHZ)
#define FREQ_100_KHZ							(10 * FREQ_10_KHZ)
#define FREQ_1_MHZ								(10 * FREQ_100_KHZ)
#define FREQ_10_MHZ								(10 * FREQ_1_MHZ)
#define FREQ_100_MHZ							(10 * FREQ_10_MHZ)
#define FREQ_1_GHZ								(10 * FREQ_100_MHZ)

#define DATA_1_BYTE								(1)
#define DATA_1_KBYTE							(1024 * DATA_1_BYTE)
#define DATA_1_MBYTE							(1024 * DATA_1_KBYTE)
#define DATA_1_GBYTE							(1024 * DATA_1_MBYTE)

#define NUM_ELEMENTS(a)							(sizeof(a) / sizeof(a[0]))
#define BYTES_TO_BITS(a)						((a) * 8)
#define WORDS_TO_BYTES(a)						((a) * sizeof(u32))
#define BYTES_TO_WORDS(a)						((a) / sizeof(u32))
#define SWAP(type, a, b)						do { \
													type t; t = a; a = b; b = t; \
												} while(0)

#define IS_EVEN(val)							(val % 2 == 0)
#define IS_ODD(val)								(!IS_EVEN(val))
#define CUT_MIN_MAX(val, min, max)				((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#define CUT_NORM_FLOAT(val)						(CUT_MIN_MAX(val, 0.0f, 1.0f))
#define GET_PCNT_OF_VAL(pcnt, val)				((s32)((float)(val) * (float)(pcnt > 100 ? 100 : pcnt) / 100.0f))
#define GET_MAX(a, b)							((a) > (b) ? (a) : (b))
#define GET_MIN(a, b)							((a) < (b) ? (a) : (b))

#define BIT_SET(reg, bit)						((reg) |=  (bit))
#define BIT_CLEAR(reg, bit)						((reg) &= ~(bit))
#define BIT_READ(reg, bit)						((reg) &   (bit))
#define REG_MODIFY(reg, CLEARMASK, SETMASK)		(BIT_SET((reg), (((BIT_READ(reg)) & (~(CLEARMASK))) | (SETMASK))))

#define DISCARD_UNUSED(a)						((void)(a))
#define ASSIGN_NOT_NULL_VAL_TO_PTR(ptr, val)	do { \
														if(val != NULL) \
															ptr = val; \
												} while(0)

#endif /* __DEF_MACRO_H */

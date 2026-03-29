#include "rand.h"

#if LIB_USE_RAND

#include "platform.h"
#define EMBLIB_RAND_SEED() Pl_SysCpuCnt_Get()

void Rand_Init(void) {
	srand(EMBLIB_RAND_SEED());
}

u32 Rand_GetNum(void) {
	return (u32)rand();
}

void Rand_GetBuff(u32* pBuff, u32 maxNum) {
	ASSERT_CHECK(pBuff);
	ASSERT_CHECK(maxNum);

	for (u32 idx = 0; idx < maxNum; idx++)
		pBuff[idx] = Rand_GetNum();
}

s32 Rand_GetNumBetween(s32 min, s32 max) {
	ASSERT_CHECK(min < max);
	if (max < min)
		SWAP(s32, max, min);

	return min + Rand_GetNum() % (max - min + 1);
}

bool Rand_GetBool(void) {
	return (Rand_GetNum() & 1) != 0;
}

void Rand_GetBuffBetween(u32* pBuff, u32 maxNum, s32 min, s32 max) {
	ASSERT_CHECK(pBuff);
	ASSERT_CHECK(maxNum);

	for (u32 idx = 0; idx < maxNum; idx++)
		pBuff[idx] = Rand_GetNumBetween(min, max);
}

u32 Rand_GetStr(char* pBuff, u32 maxNum) {
	if (!pBuff) {
		PANIC();
		return 0;
	}

	u32 tmpRng;
	u8* ptr = (u8*)&tmpRng;
	u32 cnt = 0, cyclesNum = 0;

	while (cnt < maxNum) {
		tmpRng = Rand_GetNum();

		for (u32 i = 0; i < sizeof(u32); i++) {
			//probability is (255 / (28 + 28 + 10)) == 3.86
			if (((ptr[i] >= 'a') && (ptr[i] <= 'z')) || ((ptr[i] >= 'A') && (ptr[i] <= 'Z')) ||
				((ptr[i] >= '0') && (ptr[i] <= '9'))) {
				*pBuff++ = ptr[i];
				if (++cnt >= maxNum)
					break;
			}
		}

		cyclesNum++;
	}

	*pBuff = '\0';
	return cyclesNum;
}

#endif /* LIB_USE_RAND */

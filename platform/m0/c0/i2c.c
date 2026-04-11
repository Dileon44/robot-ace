#include "i2c.h"

#define I2C_F_SCL_STANDARD	100000
#define I2C_F_SCL_FAST		400000
#define I2C_F_SCL_FAST_PLUS 1000000

#define I2C_TIMING_SCLL_AND_SCLH_SUM_MAX 512
#define I2C_TIMING_SCLDEL_MAX			 0xF
#define I2C_TRANSFER_SIZE_MAX			 255u

// STM32H7 analog filter is enabled by default (CR1.ANFOFF=0).
// t_SYNC1 + t_SYNC2 ≈ 4*tI2CCLK + 2*tAF; using tAF_min guarantees f_actual ≤ f_target.
#define I2C_ANALOG_FILTER_DELAY_MIN_NS 50u

#define I2C_SENSOR			 I2C1
#define I2C_SENSOR_CLK_EN()	 LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1)
#define I2C_SENSOR_CLK_DIS() LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1)

typedef u32 (*flagFunc)(const I2C_TypeDef* I2Cx);

static bool I2C_WaitFlagSet(I2C_TypeDef* pI2Cx, u32 tmo, flagFunc func) {
	u32 endTime = Pl_DelayMs_GetMsCnt() + tmo;
	while (!func(pI2Cx)) {
		if (Pl_DelayMs_GetMsCnt() > endTime) {
			PANIC();
			return false;
		}
	}

	return true;
}

static bool I2C_WaitFlagReset(I2C_TypeDef* pI2Cx, u32 tmo, flagFunc func) {
	u32 endTime = Pl_DelayMs_GetMsCnt() + tmo;
	while (func(pI2Cx)) {
		if (Pl_DelayMs_GetMsCnt() > endTime) {
			PANIC();
			return false;
		}
	}

	return true;
}

static bool I2C_Read_Common(I2C_TypeDef* pI2Cx, u8 devAddr, u8 reg, u8* pData, u16 len, u32 tmo) {
	if ((len == 0u) || (len > I2C_TRANSFER_SIZE_MAX)) {
		return false;
	}

	if (I2C_WaitFlagReset(pI2Cx, tmo, LL_I2C_IsActiveFlag_BUSY) != true)
		return false;

	LL_I2C_DisableSMBusPEC(pI2Cx);
	LL_I2C_DisableAutoEndMode(pI2Cx);
	LL_I2C_SetSlaveAddr(pI2Cx, devAddr << 1);

	LL_I2C_AcknowledgeNextData(pI2Cx, LL_I2C_ACK);
	LL_I2C_SetTransferRequest(pI2Cx, LL_I2C_REQUEST_WRITE);
	LL_I2C_SetTransferSize(pI2Cx, 1);

	LL_I2C_GenerateStartCondition(pI2Cx);

	if (I2C_WaitFlagSet(pI2Cx, tmo, LL_I2C_IsActiveFlag_TXE) != true) {
		LL_I2C_GenerateStopCondition(pI2Cx);
		return false;
	}

	LL_I2C_AcknowledgeNextData(pI2Cx, LL_I2C_ACK);
	LL_I2C_TransmitData8(pI2Cx, reg);

	if (I2C_WaitFlagSet(pI2Cx, tmo, LL_I2C_IsActiveFlag_TXE) != true) {
		LL_I2C_GenerateStopCondition(pI2Cx);
		return false;
	}

	LL_I2C_AcknowledgeNextData(pI2Cx, LL_I2C_NACK);
	LL_I2C_SetTransferRequest(pI2Cx, LL_I2C_REQUEST_READ);
	LL_I2C_SetTransferSize(pI2Cx, len);

	LL_I2C_GenerateStartCondition(pI2Cx);

	for (u32 i = 0; i < len; i++) {
		if (I2C_WaitFlagSet(pI2Cx, tmo, LL_I2C_IsActiveFlag_RXNE) != true) {
			LL_I2C_GenerateStopCondition(pI2Cx);
			return false;
		}

		*(pData + i) = LL_I2C_ReceiveData8(pI2Cx);
	}

	LL_I2C_GenerateStopCondition(pI2Cx);
	return true;
}

static bool I2C_Write_Common(I2C_TypeDef* pI2Cx, u8 devAddr, u8 reg, u8* pData, u16 dataLen,
							 u32 tmo) {
	if ((dataLen == 0u) || ((dataLen + 1u) > I2C_TRANSFER_SIZE_MAX)) {
		return false;
	}

	if (I2C_WaitFlagReset(pI2Cx, tmo, LL_I2C_IsActiveFlag_BUSY) != true)
		return false;

	LL_I2C_DisableSMBusPEC(pI2Cx);
	LL_I2C_DisableAutoEndMode(pI2Cx);
	LL_I2C_SetSlaveAddr(pI2Cx, devAddr << 1);

	LL_I2C_AcknowledgeNextData(pI2Cx, LL_I2C_ACK);
	LL_I2C_SetTransferRequest(pI2Cx, LL_I2C_REQUEST_WRITE);
	LL_I2C_SetTransferSize(pI2Cx, dataLen + 1);

	LL_I2C_GenerateStartCondition(pI2Cx);

	if (I2C_WaitFlagSet(pI2Cx, tmo, LL_I2C_IsActiveFlag_TXE) != true) {
		LL_I2C_GenerateStopCondition(pI2Cx);
		return false;
	}

	LL_I2C_TransmitData8(pI2Cx, reg);

	if (I2C_WaitFlagSet(pI2Cx, tmo, LL_I2C_IsActiveFlag_TXE) != true) {
		LL_I2C_GenerateStopCondition(pI2Cx);
		return false;
	}

	for (u32 i = 0; i < dataLen; i++) {
		LL_I2C_TransmitData8(pI2Cx, *(pData + i));

		if (I2C_WaitFlagSet(pI2Cx, tmo, LL_I2C_IsActiveFlag_TXE) != true) {
			LL_I2C_GenerateStopCondition(pI2Cx);
			return false;
		}
	}

	LL_I2C_GenerateStopCondition(pI2Cx);
	return true;
}

u32 I2C_CalculateTiming(u32 pclk, u32 fscl) {
	u32 presc = pclk / (fscl * I2C_TIMING_SCLL_AND_SCLH_SUM_MAX);

	// Minimum data-setup time per I2C spec mode
	u32 tScldelMin_ns;
	if (fscl <= I2C_F_SCL_STANDARD) {
		tScldelMin_ns = 250u;
	} else if (fscl <= I2C_F_SCL_FAST) {
		tScldelMin_ns = 100u;
	} else {
		tScldelMin_ns = 50u;
	}

	u64 scldelCycles = (u64)tScldelMin_ns * pclk;
	u64 scldelDen	 = (u64)(presc + 1u) * 1000000000ULL;
	s32 scldelTmp	 = (s32)((scldelCycles + scldelDen - 1u) / scldelDen) - 1;

	u32 scldel;
	if (scldelTmp < 0) {
		scldel = 0u;
	} else if ((u32)scldelTmp > I2C_TIMING_SCLDEL_MAX) {
		scldel = I2C_TIMING_SCLDEL_MAX;
	} else {
		scldel = (u32)scldelTmp;
	}

	u32 sdadel		 = 0u;
	u64 target_ns	 = 1000000000ULL / fscl;
	u64 tPresc_ns	 = (u64)(presc + 1u) * 1000000000ULL / pclk;
	u64 syncTotal_ns = 4ULL * 1000000000ULL / pclk + 2ULL * I2C_ANALOG_FILTER_DELAY_MIN_NS;
	u64 sclAvail_ns	 = (target_ns > syncTotal_ns) ? (target_ns - syncTotal_ns) : 0ULL;
	u32 sclh = (sclAvail_ns > 0ULL)
				   ? (u32)((sclAvail_ns + 2ULL * tPresc_ns - 1ULL) / (2ULL * tPresc_ns)) - 1u
				   : 0u;
	u32 scll = sclh;

	return __LL_I2C_CONVERT_TIMINGS(presc, scldel, sdadel, sclh, scll);
}

bool I2C_Sensor_Init(void) {
	if (!Pl_IsInit.Sys) {
		PANIC();
		return false;
	}

	if (Pl_IsInit.I2cSensor) {
		return true;
	}

	I2C_SENSOR_CLK_EN();

	LL_I2C_DisableOwnAddress2(I2C_SENSOR);
	LL_I2C_DisableGeneralCall(I2C_SENSOR);
	LL_I2C_DisableClockStretching(I2C_SENSOR);

	LL_I2C_InitTypeDef I2C_InitStruct;
	LL_I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.PeripheralMode  = LL_I2C_MODE_I2C;
	I2C_InitStruct.Timing		   = I2C_CalculateTiming(Pl_SysClk.APB1, I2C_F_SCL_FAST_PLUS);
	I2C_InitStruct.OwnAddress1	   = 0;
	I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
	I2C_InitStruct.OwnAddrSize	   = LL_I2C_OWNADDRESS1_7BIT;

	if (LL_I2C_Init(I2C_SENSOR, &I2C_InitStruct) != SUCCESS) {
		PANIC();
		return false;
	}

	LL_I2C_SetOwnAddress2(I2C_SENSOR, 0, LL_I2C_OWNADDRESS2_NOMASK);

	return true;
}

bool I2C_Sensor_DeInit(void) {
	if (!Pl_IsInit.Sys) {
		PANIC();
		return false;
	}

	if (!Pl_IsInit.I2cSensor) {
		return true;
	}

	LL_I2C_Disable(I2C_SENSOR);
	if (LL_I2C_DeInit(I2C_SENSOR) != SUCCESS) {
		PANIC();
		return false;
	}

	I2C_SENSOR_CLK_DIS();

	return true;
}

RET_STATE_t I2C_Sensor_Read(u8 devAddr, u8 wordAddr, u8* pData, u16 len) {
	if (I2C_Read_Common(I2C_SENSOR, devAddr, wordAddr, pData, len, 10)) {
		return RET_STATE_SUCCESS;
	}

	return RET_STATE_ERROR;
}

RET_STATE_t I2C_Sensor_Write(u8 devAddr, u8 wordAddr, const u8* pData, u16 len) {
	if (I2C_Write_Common(I2C_SENSOR, devAddr, wordAddr, (u8*)pData, len, 100)) {
		return RET_STATE_SUCCESS;
	}

	return RET_STATE_ERROR;
}

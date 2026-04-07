#include "encoder_m_as5600__interface.h"
#include "encoder_m_as5600_reg_map.h"
#include "platform.h"

#if BSP_CFG_USE_ENCODER_M

static bool As5600_Interface_Init(void) {
	bool isEncoderInit = Pl_Encoder_Init();
	Pl_Encoder_Dir_Set(false); /* default: clockwise */
	return isEncoderInit;
}

static bool As5600_Interface_DeInit(void) {
	return Pl_Encoder_DeInit();
}

static RET_STATE_t As5600_Interface_Read(u8 wordAddr, u8* pData, u16 len) {
	return Pl_I2cEncoder_Read(EncoderM_As5600_Interface.I2cAddr, wordAddr, pData, len);
}

static RET_STATE_t As5600_Interface_Write(u8 wordAddr, const u8* pData, u16 len) {
	return Pl_I2cEncoder_Write(EncoderM_As5600_Interface.I2cAddr, wordAddr, pData, len);
}

static void As5600_Interface_SetDir(bool clockwise) {
	Pl_Encoder_Dir_Set(clockwise);
}

EncoderM_Interface_t EncoderM_As5600_Interface = {
	.I2cAddr = 0x36u, /* AS5600 fixed 7-bit I2C address */

	.Init	= As5600_Interface_Init,
	.DeInit = As5600_Interface_DeInit,
	.Read	= As5600_Interface_Read,
	.Write	= As5600_Interface_Write,
	.SetDir = As5600_Interface_SetDir,
};

#endif /* BSP_CFG_USE_ENCODER_M */

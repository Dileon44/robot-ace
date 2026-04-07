#include "encoder_m_as5600.h"
#include "debug.h"
#include "encoder_m_as5600_reg_map.h"

#if BSP_CFG_USE_ENCODER_M

static EncoderM_Interface_t* As5600_InterfacePtr;

static bool As5600_SetInterface(EncoderM_Interface_t* pInterface) {
	As5600_InterfacePtr = pInterface;
	return true;
}

static bool As5600_Init(void) {
	// Configure CONF for FOC: PM=NOM, HYST=OFF, SF=2x, FTH=6LSB, WD=OFF
	u16 confValue = AS5600_CONF_FOC_VALUE;
	u8 confBuf[2] = {
		(u8)((confValue >> 8) & 0x3Fu),
		(u8)(confValue & 0xFFu),
	};

	RET_STATE_t rs = As5600_InterfacePtr->Write(AS5600_REG_CONF_H, confBuf, 2);
	if (rs != RET_STATE_SUCCESS) {
		DEBUG_COLOR_PRINT_NL(ESC_COLOR_RED, "AS5600: CONF write failed");
		return false;
	}

	PL_DELAY_MS(10);  // Delay for CONF to take effect

	// Verify magnet detection
	u8 status = 0;
	rs		  = As5600_InterfacePtr->Read(AS5600_REG_STATUS, &status, 1);
	if (rs != RET_STATE_SUCCESS) {
		DEBUG_COLOR_PRINT_NL(ESC_COLOR_RED, "AS5600: STATUS read failed");
		return false;
	}

	if (!(status & AS5600_STATUS_MD_BIT)) {
		DEBUG_COLOR_PRINT_NL(ESC_COLOR_YELLOW, "AS5600: magnet not detected (STATUS=0x%02x)",
							 status);
	} else {
		DEBUG_COLOR_PRINT_NL(ESC_COLOR_GREEN, "AS5600: magnet OK (STATUS=0x%02x)", status);
	}

	return true;
}

static bool As5600_DeInit(void) {
	As5600_InterfacePtr = NULL;
	return true;
}

static u16 As5600_GetRawAngle(void) {
	u8 buf[2]	   = {0};
	RET_STATE_t rs = As5600_InterfacePtr->Read(AS5600_REG_RAW_ANGLE_H, buf, 2);
	if (rs != RET_STATE_SUCCESS) {
		return 0;
	}
	return (u16)(((u16)(buf[0] & 0x0Fu) << 8) | buf[1]);
}

static u16 As5600_GetAngle(void) {
	u8 buf[2]	   = {0};
	RET_STATE_t rs = As5600_InterfacePtr->Read(AS5600_REG_ANGLE_H, buf, 2);
	if (rs != RET_STATE_SUCCESS) {
		return 0;
	}
	return (u16)(((u16)(buf[0] & 0x0Fu) << 8) | buf[1]);
}

static float As5600_GetAngleDeg(void) {
	return (float)As5600_GetRawAngle() * (360.0f / 4096.0f);
}

static u8 As5600_GetStatus(void) {
	u8 status = 0;
	As5600_InterfacePtr->Read(AS5600_REG_STATUS, &status, 1);
	return status;
}

static bool As5600_IsMagnetDetected(void) {
	return (As5600_GetStatus() & AS5600_STATUS_MD_BIT) != 0u;
}

static u8 As5600_GetAGC(void) {
	u8 agc = 0;
	As5600_InterfacePtr->Read(AS5600_REG_AGC, &agc, 1);
	return agc;
}

static u16 As5600_GetMagnitude(void) {
	u8 buf[2]	   = {0};
	RET_STATE_t rs = As5600_InterfacePtr->Read(AS5600_REG_MAGNITUDE_H, buf, 2);
	if (rs != RET_STATE_SUCCESS) {
		return 0;
	}
	return (u16)(((u16)(buf[0] & 0x0Fu) << 8) | buf[1]);
}

EncoderM_t EncoderM_As5600 = {
	.Module = ENCODER_M_AS5600,
	.Name	= "as5600",

	.SetInterface = As5600_SetInterface,

	.Init			  = As5600_Init,
	.DeInit			  = As5600_DeInit,
	.GetRawAngle	  = As5600_GetRawAngle,
	.GetAngle		  = As5600_GetAngle,
	.GetAngleDeg	  = As5600_GetAngleDeg,
	.GetStatus		  = As5600_GetStatus,
	.IsMagnetDetected = As5600_IsMagnetDetected,
	.GetAGC			  = As5600_GetAGC,
	.GetMagnitude	  = As5600_GetMagnitude,
};

#endif /* BSP_CFG_USE_ENCODER_M */

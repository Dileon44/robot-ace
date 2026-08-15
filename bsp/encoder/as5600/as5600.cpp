#include "as5600.hpp"
#include "as5600_bus.hpp"
#include "as5600_reg_map.h"
#include "log.h"

#if BSP_CFG_USE_ENCODER

namespace bsp::encoder {

bool As5600::Init() {
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "AS5600: i2c addr 0x%02x", bus_.Address());

	// Configure CONF for FOC: PM=NOM, HYST=OFF, SF=2x, FTH=6LSB, WD=OFF
	u16 confValue = AS5600_CONF_FOC_VALUE;
	u8  confBuf[2] = {
		 static_cast<u8>((confValue >> 8) & 0x3Fu),
		 static_cast<u8>(confValue & 0xFFu),
	};

	RET_STATE_t rs = bus_.Write(AS5600_REG_CONF_H, confBuf, 2);
	if (rs != RET_STATE_SUCCESS) {
		DEBUG_COLOR_PRINT_NL(ESC_COLOR_RED, "AS5600: CONF write failed");
		return false;
	}

	PL_DELAY_MS(10);  // Delay for CONF to take effect

	// Verify magnet detection
	u8 status = 0;
	rs        = bus_.Read(AS5600_REG_STATUS, &status, 1);
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

bool As5600::DeInit() {
	return true;
}

// *this still has its exact type here, so the encoder's typed slot is filled
// without any cast — that is the whole point of routing through this hook.
void As5600::OnAttached() {
	SetDriver(*this);
}

void As5600::OnDetached() {
	ClearDriver();
}

u16 As5600::ReadReg12(u8 regAddr) {
	u8          buf[2] = {0};
	RET_STATE_t rs     = bus_.Read(regAddr, buf, 2);
	if (rs != RET_STATE_SUCCESS) {
		return 0;
	}
	return static_cast<u16>((static_cast<u16>(buf[0] & 0x0Fu) << 8) | buf[1]);
}

u16 As5600::GetRawAngle() {
	return ReadReg12(AS5600_REG_RAW_ANGLE_H);
}

u16 As5600::GetAngle() {
	return ReadReg12(AS5600_REG_ANGLE_H);
}

float As5600::GetAngleDeg() {
	return static_cast<float>(GetRawAngle()) * (360.0f / 4096.0f);
}

u8 As5600::GetStatus() {
	u8 status = 0;
	bus_.Read(AS5600_REG_STATUS, &status, 1);
	return status;
}

bool As5600::IsMagnetDetected() {
	return (GetStatus() & AS5600_STATUS_MD_BIT) != 0u;
}

u8 As5600::GetAGC() {
	u8 agc = 0;
	bus_.Read(AS5600_REG_AGC, &agc, 1);
	return agc;
}

u16 As5600::GetMagnitude() {
	return ReadReg12(AS5600_REG_MAGNITUDE_H);
}

void As5600::SetDirection(bool clockwise) {
	bus_.SetDirection(clockwise);
}

/* Constant-initialized: no .init_array entry, nothing touches hardware before main() */
constinit As5600 As5600Dev{As5600BusDev};

}  // namespace bsp::encoder

#endif /* BSP_CFG_USE_ENCODER */

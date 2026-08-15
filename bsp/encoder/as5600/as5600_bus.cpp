#include "as5600_bus.hpp"
#include "platform.h"

namespace bsp::encoder {

bool As5600Bus::Init() {
	bool isEncoderInit = Pl_Encoder_Init();
	Pl_Encoder_Dir_Set(false); /* default: clockwise */
	return isEncoderInit;
}

bool As5600Bus::DeInit() {
	return Pl_Encoder_DeInit();
}

RET_STATE_t As5600Bus::Read(u8 regAddr, u8* pData, u16 len) {
	return Pl_I2cEncoder_Read(I2C_ADDR, regAddr, pData, len);
}

RET_STATE_t As5600Bus::Write(u8 regAddr, const u8* pData, u16 len) {
	return Pl_I2cEncoder_Write(I2C_ADDR, regAddr, pData, len);
}

void As5600Bus::SetDirection(bool clockwise) {
	Pl_Encoder_Dir_Set(clockwise);
}

/* Constant-initialized: no .init_array entry, nothing touches hardware before main() */
constinit As5600Bus As5600BusDev{};

}  // namespace bsp::encoder

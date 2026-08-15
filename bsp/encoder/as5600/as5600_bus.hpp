#ifndef __AS5600_BUS_HPP
#define __AS5600_BUS_HPP

#include "bsp_cfg.h"
#include "encoder.hpp"

#if BSP_CFG_USE_ENCODER

namespace bsp::encoder {

class As5600Bus final : public IBus {
public:
	constexpr As5600Bus() = default;

	bool        Init() override;
	bool        DeInit() override;
	RET_STATE_t Read(u8 regAddr, u8* pData, u16 len) override;
	RET_STATE_t Write(u8 regAddr, const u8* pData, u16 len) override;
	void        SetDirection(bool clockwise) override;

	u8 Address() const override {
		return I2C_ADDR;
	}

private:
	static constexpr u8 I2C_ADDR = 0x36u;  // AS5600 fixed 7-bit I2C address
};

extern As5600Bus As5600BusDev;

}  // namespace bsp::encoder

#endif /* BSP_CFG_USE_ENCODER */

#endif /* __AS5600_BUS_HPP */

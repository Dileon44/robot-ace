#ifndef __ENCODER_M_AS5600__INTERFACE_HPP
#define __ENCODER_M_AS5600__INTERFACE_HPP

#include "bsp_cfg.h"
#include "encoder_m.hpp"

#if BSP_CFG_USE_ENCODER_M

namespace bsp {

class As5600Bus final : public IEncoderBus {
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

extern As5600Bus EncoderAs5600Bus;

}  // namespace bsp

#endif /* BSP_CFG_USE_ENCODER_M */

#endif /* __ENCODER_M_AS5600__INTERFACE_HPP */

#ifndef __ENCODER_M_AS5600_HPP
#define __ENCODER_M_AS5600_HPP

#include "bsp_cfg.h"
#include "encoder_m.hpp"

#if BSP_CFG_USE_ENCODER_M

namespace bsp::encoder {

class As5600 final : public IDriver {
public:
	// constexpr and hardware-free: the instance is constant-initialized, so it
	// carries no .init_array entry and nothing runs before Pl_Init(). All chip
	// access happens in Init(), called from Bsp_Init() out of main().
	constexpr explicit As5600(IBus& bus) : bus_(bus) {}

	bool  Init() override;
	bool  DeInit() override;
	void  OnAttached() override;
	void  OnDetached() override;
	u16   GetRawAngle() override;
	u16   GetAngle() override;
	float GetAngleDeg() override;
	u8    GetStatus() override;
	bool  IsMagnetDetected() override;
	u8    GetAGC() override;
	u16   GetMagnitude() override;
	void  SetDirection(bool clockwise) override;

	const char* Name() const override {
		return "as5600";
	}

private:
	// Reads a big-endian 12-bit value from regAddr (high byte first).
	u16 ReadReg12(u8 regAddr);

	IBus& bus_;
};

extern As5600 As5600Dev;

}  // namespace bsp::encoder

#endif /* BSP_CFG_USE_ENCODER_M */

#endif /* __ENCODER_M_AS5600_HPP */

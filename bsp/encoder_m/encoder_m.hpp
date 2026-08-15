#ifndef __ENCODER_M_HPP
#define __ENCODER_M_HPP

#include "bsp_cfg.h"
#include "bsp_module.hpp"
#include "main.h"

/* ============================================================================
 * Abstract encoder module
 *
 * Replaces the hand-written vtable structs (EncoderM_Interface_t / EncoderM_t):
 * the compiler now builds the dispatch table, and `override` makes a mismatched
 * signature a compile error instead of a NULL entry found at runtime.
 *
 * Init/DeInit/Name/OnAttached come from IModule, so Bsp_Init() can bring this
 * module up through the same generic loop as every other one.
 * ============================================================================ */

namespace bsp::encoder {

/* --------------------------------------------------------------------------
 * Transport — how a driver reaches its chip.
 * Implementations live in encoder_m_<chip>__interface.cpp and call Pl_* directly.
 * -------------------------------------------------------------------------- */
class IBus : public IModuleBus {
  public:
	virtual RET_STATE_t Read(u8 regAddr, u8* pData, u16 len)        = 0;
	virtual RET_STATE_t Write(u8 regAddr, const u8* pData, u16 len) = 0;
	virtual void        SetDirection(bool clockwise)                = 0;
	virtual u8          Address() const                             = 0;

  protected:
	~IBus() = default;
};

/* --------------------------------------------------------------------------
 * Chip driver — implemented in encoder_m_<chip>.cpp
 * -------------------------------------------------------------------------- */
class IDriver : public IModule {
  public:
	virtual u16   GetRawAngle()                = 0;
	virtual u16   GetAngle()                   = 0;
	virtual float GetAngleDeg()                = 0;
	virtual u8    GetStatus()                  = 0;
	virtual bool  IsMagnetDetected()           = 0;
	virtual u8    GetAGC()                     = 0;
	virtual u16   GetMagnitude()               = 0;
	virtual void  SetDirection(bool clockwise) = 0;

  protected:
	~IDriver() = default;
};

/* --------------------------------------------------------------------------
 * Typed access to the attached driver.
 *
 * The slot is filled from the driver's OnAttached(), where the concrete type is
 * still known — that is why no cast or RTTI is needed to get IDriver* back out
 * of the type-erased BSP module table.
 * -------------------------------------------------------------------------- */
void SetDriver(IDriver& driver);  // internal: call only from IDriver::OnAttached()
void ClearDriver();                // internal: call only from IDriver::OnDetached()

bool IsAttached();

// PANIC() and nullptr when nothing is attached — caller must check.
IDriver* GetPtr();

}  // namespace bsp::encoder

#endif /* __ENCODER_M_HPP */

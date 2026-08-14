#ifndef __ENCODER_M_HPP
#define __ENCODER_M_HPP

#include "bsp_cfg.h"
#include "main.h"

/* ============================================================================
 * Abstract encoder module
 *
 * Replaces the hand-written vtable structs (EncoderM_Interface_t / EncoderM_t):
 * the compiler now builds the dispatch table, and `override` makes a mismatched
 * signature a compile error instead of a NULL entry found at runtime.
 * ============================================================================ */

namespace bsp {

enum class EncoderType : u8 {
	Undef  = 0,
	As5600 = 1,
};

/* --------------------------------------------------------------------------
 * Transport — how a driver reaches its chip.
 * Implementations live in encoder_m_<chip>__interface.cpp and call Pl_* directly.
 * -------------------------------------------------------------------------- */
class IEncoderBus {
  public:
	virtual bool Init()												= 0;
	virtual bool DeInit()											= 0;
	virtual RET_STATE_t Read(u8 regAddr, u8* pData, u16 len)		= 0;
	virtual RET_STATE_t Write(u8 regAddr, const u8* pData, u16 len) = 0;
	virtual void SetDirection(bool clockwise)						= 0;
	virtual u8 Address() const										= 0;

  protected:
	// Protected and non-virtual: every instance has static storage duration and
	// is never deleted, so a virtual destructor would only cost a vtable slot.
	~IEncoderBus() = default;
};

/* --------------------------------------------------------------------------
 * Chip driver — implemented in encoder_m_<chip>.cpp
 * -------------------------------------------------------------------------- */
class IEncoder {
  public:
	virtual bool Init()						  = 0;
	virtual bool DeInit()					  = 0;
	virtual u16 GetRawAngle()				  = 0;
	virtual u16 GetAngle()					  = 0;
	virtual float GetAngleDeg()				  = 0;
	virtual u8 GetStatus()					  = 0;
	virtual bool IsMagnetDetected()			  = 0;
	virtual u8 GetAGC()						  = 0;
	virtual u16 GetMagnitude()				  = 0;
	virtual void SetDirection(bool clockwise) = 0;
	virtual EncoderType Type() const		  = 0;
	virtual const char* Name() const		  = 0;

  protected:
	~IEncoder() = default;
};

/* --------------------------------------------------------------------------
 * Module-level access.
 *
 * The attach guard that used to be repeated in every EncoderM_* wrapper now
 * lives in Get() alone: callers obtain the driver once and call it directly.
 * -------------------------------------------------------------------------- */
namespace encoder {

RET_STATE_t Attach(IEncoder& driver, IEncoderBus& bus);
RET_STATE_t Detach();

bool IsAttached();

// PANIC() and nullptr when nothing is attached.
IEncoder* Get();

}  // namespace encoder
}  // namespace bsp

#endif /* __ENCODER_M_HPP */

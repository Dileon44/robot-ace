#ifndef __CRITICAL_SECTION_HPP
#define __CRITICAL_SECTION_HPP

#include "def_sys.h"

/* ============================================================================
 * RAII guards for critical sections — C++ only.
 *
 * Replaces the paired SYS_CRITICAL_ON() / SYS_CRITICAL_OFF() calls: the section
 * closes on every exit path, so an early `return` — including the one hidden
 * inside RETURN_IF_UNSUCCESS() — can no longer leave interrupts masked forever.
 *
 * C code keeps using the SYS_CRITICAL_* macros unchanged.
 *
 * Cost is the same two instructions as the raw macro pair: the guard object is
 * a local that never escapes, so the compiler keeps its state in a register (or
 * nothing at all under RTOS) and emits no constructor/destructor calls.
 * ============================================================================ */

namespace sys {

namespace detail {

#if SYS_USE_RTOS

/* FreeRTOS carries its own nesting counter (uxCriticalNesting), so nothing has
 * to be handed from enter to exit and guards nest freely. */
struct CriticalState {};

inline CriticalState CriticalEnter() {
	taskENTER_CRITICAL();
	return {};
}

inline void CriticalExit(CriticalState) {
	taskEXIT_CRITICAL();
}

using IsrCriticalState = UBaseType_t;

inline IsrCriticalState IsrCriticalEnter() {
	return taskENTER_CRITICAL_FROM_ISR();
}

inline void IsrCriticalExit(IsrCriticalState mask) {
	taskEXIT_CRITICAL_FROM_ISR(mask);
}

#else /* bare-metal */

/* Deliberately NOT Pl_IrqOff() / Pl_IrqOn(): Pl_IrqOn() is an unconditional
 * __enable_irq(), so with two nested sections the inner one would re-enable
 * interrupts while the outer is still open — the nesting hazard that
 * def_sys.h warns about with "no nesting calls". Saving and restoring PRIMASK
 * nests correctly by construction and costs the same two instructions. */
using CriticalState = u32;

inline CriticalState CriticalEnter() {
	const u32 priMask = __get_PRIMASK();
	__disable_irq();
	return priMask;
}

inline void CriticalExit(CriticalState priMask) {
	__set_PRIMASK(priMask);
}

using IsrCriticalState = CriticalState;

inline IsrCriticalState IsrCriticalEnter() {
	return CriticalEnter();
}

inline void IsrCriticalExit(IsrCriticalState priMask) {
	CriticalExit(priMask);
}

#endif /* SYS_USE_RTOS */

}  // namespace detail

/* --------------------------------------------------------------------------
 * Task context.
 *
 *     {
 *         sys::CriticalSection cs;
 *         count_ -= n;
 *         tail_   = newTail;
 *     }  // closes here, on every path out of the scope
 *
 * Always give the guard a name. `sys::CriticalSection();` builds a temporary
 * that is destroyed at the end of that same statement, so the section protects
 * nothing — and GCC 15 issues no diagnostic for it (verified), [[nodiscard]]
 * included: that attribute only fires when a guard is returned from a function.
 * -------------------------------------------------------------------------- */
class [[nodiscard]] CriticalSection {
  public:
	CriticalSection() : state_(detail::CriticalEnter()) {
	}

	~CriticalSection() {
		detail::CriticalExit(state_);
	}

	// A guard stands for one scope; copying or moving it would close the
	// section somewhere other than where it was opened.
	CriticalSection(const CriticalSection&)			   = delete;
	CriticalSection& operator=(const CriticalSection&) = delete;
	CriticalSection(CriticalSection&&)				   = delete;
	CriticalSection& operator=(CriticalSection&&)	   = delete;

  private:
	detail::CriticalState state_;
};

/* --------------------------------------------------------------------------
 * ISR context.
 *
 * taskENTER_CRITICAL_FROM_ISR() returns a mask that must be handed back to the
 * matching exit call — that mask is exactly the state this guard carries, so it
 * cannot be lost or passed to the wrong call.
 * -------------------------------------------------------------------------- */
class [[nodiscard]] CriticalSectionIsr {
  public:
	CriticalSectionIsr() : state_(detail::IsrCriticalEnter()) {
	}

	~CriticalSectionIsr() {
		detail::IsrCriticalExit(state_);
	}

	CriticalSectionIsr(const CriticalSectionIsr&)			 = delete;
	CriticalSectionIsr& operator=(const CriticalSectionIsr&) = delete;
	CriticalSectionIsr(CriticalSectionIsr&&)				 = delete;
	CriticalSectionIsr& operator=(CriticalSectionIsr&&)		 = delete;

  private:
	detail::IsrCriticalState state_;
};

}  // namespace sys

#endif /* __CRITICAL_SECTION_HPP */

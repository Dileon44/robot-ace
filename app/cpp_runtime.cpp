// C++ runtime traps.
//
// The firmware runs without a heap: FreeRTOS owns its own heap_4 pool, and any
// call reaching operator new would go to newlib malloc instead — not thread-safe
// against the scheduler and non-deterministic. Every allocation path is therefore
// routed into ErrorHandler() so a stray `new` fails loudly on the bench rather
// than degrading silently in the field.
//
// ErrorHandler() is called directly rather than through PANIC(): PANIC() expands
// to nothing when PANIC_CHECK_ENABLE is 0 (production builds), which would leave
// operator new quietly returning nullptr.

#include "main.h"

#include <cstddef>

namespace {

[[noreturn]] void CppRuntime_Trap(const char* pFile, int line) {
	ErrorHandler(pFile, line);
	// ErrorHandler never returns, but it is not marked [[noreturn]] in C.
	for (;;) {
	}
}

}  // namespace

/* ============================================================================
 * Heap — must never be reached
 * ============================================================================ */

void* operator new(std::size_t) {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void* operator new[](std::size_t) {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void* operator new(std::size_t, std::align_val_t) {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void* operator new[](std::size_t, std::align_val_t) {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete(void*) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete[](void*) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete(void*, std::size_t) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete[](void*, std::size_t) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete(void*, std::align_val_t) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete[](void*, std::align_val_t) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete(void*, std::size_t, std::align_val_t) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

void operator delete[](void*, std::size_t, std::align_val_t) noexcept {
	CppRuntime_Trap(__FILE__, __LINE__);
}

/* ============================================================================
 * Pure virtual call — object used before/after its constructor ran
 * ============================================================================ */

extern "C" void __cxa_pure_virtual(void) {
	CppRuntime_Trap(__FILE__, __LINE__);
}

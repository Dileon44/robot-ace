#include "bsp.h"
#include "bsp_module.hpp"
#include "log.h"

#if BSP_CFG_USE_ENCODER_M
#include "encoder_m_as5600.hpp"
#include "encoder_m_as5600__interface.hpp"
#endif /* BSP_CFG_USE_ENCODER_M */

namespace {

/* ============================================================================
 * Hardware selection point.
 *
 * One row per BSP module: which driver and which transport. Array order is init
 * order. Adding a module means adding a row and an include — Bsp_Init() below
 * never changes.
 *
 * Fields are typed as IModule* / IModuleBus*, so plugging in a class that does
 * not implement the full contract is a compile error rather than a half-filled
 * vtable struct discovered at runtime.
 *
 * const: the table lives in .rodata (Flash), not in RAM.
 * ============================================================================ */
constinit const bsp::ModuleEntry Bsp_Modules[] = {
#if BSP_CFG_USE_ENCODER_M
	{
		.module = &bsp::encoder::As5600Dev,
		.bus    = &bsp::encoder::As5600BusDev,
	},
#endif /* BSP_CFG_USE_ENCODER_M */
};

}  // namespace

RET_STATE_t Bsp_Init(void) {
	DEBUG_PRINT_NL("\r\nBSP attaching:");

	RET_STATE_t result = RET_STATE_SUCCESS;

	for (const bsp::ModuleEntry& entry : Bsp_Modules) {
		// Keep going after a failure: every module must reach a defined state so
		// the board is never left half-initialized.
		if (bsp::AttachModule(entry) != RET_STATE_SUCCESS) {
			result = RET_STATE_ERROR;
		}
	}

	return result;
}

RET_STATE_t Bsp_DeInit(void) {
	RET_STATE_t result = RET_STATE_SUCCESS;

	for (const bsp::ModuleEntry& entry : Bsp_Modules) {
		if (bsp::DetachModule(entry) != RET_STATE_SUCCESS) {
			result = RET_STATE_ERROR;
		}
	}

	return result;
}

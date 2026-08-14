#include "bsp.h"
#include "log.h"

#if BSP_CFG_USE_ENCODER_M
#include "encoder_m_as5600.hpp"
#include "encoder_m_as5600__interface.hpp"
#endif /* BSP_CFG_USE_ENCODER_M */

namespace {

/* ============================================================================
 * Hardware selection point.
 *
 * To fit a different encoder chip, change the two initializers below (and the
 * include above) — same as the old BspInfo_t did. What is new: the fields are
 * typed as IEncoder* / IEncoderBus*, so plugging in a class that does not
 * implement the full interface is now a compile error instead of a half-filled
 * vtable struct discovered at runtime.
 * ============================================================================ */
struct BspInfo {
#if BSP_CFG_USE_ENCODER_M
	bsp::IEncoder*    encoderPtr;
	bsp::IEncoderBus* encoderBusPtr;
#endif /* BSP_CFG_USE_ENCODER_M */
};

constinit BspInfo bspInfo = {
#if BSP_CFG_USE_ENCODER_M
	.encoderPtr    = &bsp::EncoderAs5600,
	.encoderBusPtr = &bsp::EncoderAs5600Bus,
#endif /* BSP_CFG_USE_ENCODER_M */
};

}  // namespace

RET_STATE_t Bsp_Init(void) {
	DEBUG_PRINT_NL("\r\nBSP attaching:");

#if BSP_CFG_USE_ENCODER_M
	bsp::encoder::Attach(*bspInfo.encoderPtr, *bspInfo.encoderBusPtr);
#endif /* BSP_CFG_USE_ENCODER_M */

	return RET_STATE_SUCCESS;
}

RET_STATE_t Bsp_DeInit(void) {
#if BSP_CFG_USE_ENCODER_M
	bsp::encoder::Detach();
#endif /* BSP_CFG_USE_ENCODER_M */

	return RET_STATE_SUCCESS;
}

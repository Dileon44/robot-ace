#include "bsp.h"
#include "log.h"

#if BSP_CFG_USE_ENCODER_M
#include "encoder_m.h"
#include "encoder_m_as5600.h"
#include "encoder_m_as5600__interface.h"
#endif /* BSP_CFG_USE_ENCODER_M */

typedef struct {
#if BSP_CFG_USE_ENCODER_M
	EncoderM_Info_t EncoderM_Info;
#endif
} BspInfo_t;

static BspInfo_t BspInfo = {
#if BSP_CFG_USE_ENCODER_M
	.EncoderM_Info = {
		.ModulePtr    = &EncoderM_As5600,
		.InterfacePtr = &EncoderM_As5600_Interface,
	},
#endif
};

RET_STATE_t Bsp_Init(void) {
	DEBUG_PRINT_NL("\r\nBSP attaching:");

#if BSP_CFG_USE_ENCODER_M
	EncoderM_Init(&BspInfo.EncoderM_Info);
#endif

	return RET_STATE_SUCCESS;
}

RET_STATE_t Bsp_DeInit(void) {
#if BSP_CFG_USE_ENCODER_M
	EncoderM_DeInit();
#endif

	return RET_STATE_SUCCESS;
}

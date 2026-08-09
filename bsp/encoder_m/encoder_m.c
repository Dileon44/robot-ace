#include "encoder_m.h"
#include "log.h"

static EncoderM_Interface_t* EncoderM_InterfacePtr;
static EncoderM_t*           EncoderM_Ptr;
static bool                  EncoderM_IsAttached;

RET_STATE_t EncoderM_Init(EncoderM_Info_t* pInfo) {
	if (!pInfo->InterfacePtr || !pInfo->ModulePtr) {
		DEBUG_COLOR_PRINT_NL(ESC_COLOR_RED, "Encoder module isn't attached");
		EncoderM_IsAttached = false;
		return RET_STATE_ERROR;
	}

	EncoderM_InterfacePtr = pInfo->InterfacePtr;
	EncoderM_Ptr          = pInfo->ModulePtr;

	if (!EncoderM_InterfacePtr->Init()) {
		PANIC();
		return RET_STATE_ERROR;
	}

	EncoderM_Ptr->SetInterface(EncoderM_InterfacePtr);

	if (!EncoderM_Ptr->Init()) {
		PANIC();
		return RET_STATE_ERROR;
	}

	DEBUG_COLOR_PRINT_NL(ESC_COLOR_GREEN, "Encoder BSP module attached");
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "- enum: %d", EncoderM_Ptr->Module);
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "- name: %s", EncoderM_Ptr->Name);
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "- addr: 0x%02x", EncoderM_InterfacePtr->I2cAddr);

	EncoderM_IsAttached = true;
	return RET_STATE_SUCCESS;
}

RET_STATE_t EncoderM_DeInit(void) {
	if (!EncoderM_IsAttached) {
		return RET_STATE_SUCCESS;
	}

	if (!EncoderM_InterfacePtr || !EncoderM_Ptr) {
		return RET_STATE_ERROR;
	}

	EncoderM_Ptr->DeInit();

	if (!EncoderM_InterfacePtr->DeInit()) {
		return RET_STATE_ERROR;
	}

	EncoderM_IsAttached = false;
	return RET_STATE_SUCCESS;
}

u16 EncoderM_GetRawAngle(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return 0;
	}
	return EncoderM_Ptr->GetRawAngle();
}

u16 EncoderM_GetAngle(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return 0;
	}
	return EncoderM_Ptr->GetAngle();
}

float EncoderM_GetAngleDeg(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return 0.0f;
	}
	return EncoderM_Ptr->GetAngleDeg();
}

u8 EncoderM_GetStatus(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return 0;
	}
	return EncoderM_Ptr->GetStatus();
}

bool EncoderM_IsMagnetDetected(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return false;
	}
	return EncoderM_Ptr->IsMagnetDetected();
}

u8 EncoderM_GetAGC(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return 0;
	}
	return EncoderM_Ptr->GetAGC();
}

u16 EncoderM_GetMagnitude(void) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return 0;
	}
	return EncoderM_Ptr->GetMagnitude();
}

void EncoderM_SetDirection(bool clockwise) {
	if (!EncoderM_IsAttached) {
		PANIC();
		return;
	}
	EncoderM_InterfacePtr->SetDir(clockwise);
}

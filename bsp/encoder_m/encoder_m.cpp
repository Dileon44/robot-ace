#include "encoder_m.hpp"
#include "log.h"

namespace bsp::encoder {

namespace {

IEncoder*    driverPtr_  = nullptr;
IEncoderBus* busPtr_     = nullptr;
bool         isAttached_ = false;

}  // namespace

RET_STATE_t Attach(IEncoder& driver, IEncoderBus& bus) {
	driverPtr_  = &driver;
	busPtr_     = &bus;
	isAttached_ = false;

	if (!busPtr_->Init()) {
		PANIC();
		return RET_STATE_ERROR;
	}

	if (!driverPtr_->Init()) {
		PANIC();
		return RET_STATE_ERROR;
	}

	DEBUG_COLOR_PRINT_NL(ESC_COLOR_GREEN, "Encoder BSP module attached");
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "- enum: %d", static_cast<int>(driverPtr_->Type()));
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "- name: %s", driverPtr_->Name());
	DEBUG_COLOR_PRINT_NL(ESC_COLOR_MAGENTA, "- addr: 0x%02x", busPtr_->Address());

	isAttached_ = true;
	return RET_STATE_SUCCESS;
}

RET_STATE_t Detach() {
	if (!isAttached_) {
		return RET_STATE_SUCCESS;
	}

	driverPtr_->DeInit();

	if (!busPtr_->DeInit()) {
		return RET_STATE_ERROR;
	}

	isAttached_ = false;
	return RET_STATE_SUCCESS;
}

bool IsAttached() {
	return isAttached_;
}

IEncoder* Get() {
	if (!isAttached_) {
		PANIC();
		return nullptr;
	}
	return driverPtr_;
}

}  // namespace bsp::encoder

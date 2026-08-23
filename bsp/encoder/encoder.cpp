#include "encoder.hpp"
#include "log.h"

namespace bsp::encoder {

namespace {

// nullptr means "not attached": the slot is filled only after a successful
// Init(), so application code can never reach a chip that failed to come up.
constinit IDriver* driverPtr_ = nullptr;

}  // namespace

void SetDriver(IDriver& driver) {
	driverPtr_ = &driver;
}

void ClearDriver() {
	driverPtr_ = nullptr;
}

bool IsAttached() {
	return driverPtr_ != nullptr;
}

IDriver* GetPtr() {
	if (!driverPtr_) {
		PANIC();
		return nullptr;
	}
	return driverPtr_;
}

}  // namespace bsp::encoder

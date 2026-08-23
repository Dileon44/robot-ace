#include "bsp_module.hpp"
#include "log.h"

namespace bsp {

RET_STATE_t AttachModule(const ModuleEntry& entry) {
	// Only the log calls below read it, so it goes unused with LOG_ENABLE=0.
	const char* name = entry.module->Name();
	DISCARD_UNUSED(name);

	if (!entry.bus->Init()) {
		LOG_RAW_COLOR_NL(TERM_COLOR_RED, "- %s: bus init failed", name);
		return RET_STATE_ERROR;
	}

	if (!entry.module->Init()) {
		LOG_RAW_COLOR_NL(TERM_COLOR_RED, "- %s: init failed", name);
		entry.bus->DeInit();
		return RET_STATE_ERROR;
	}

	// Only now does the module become reachable through its typed getter, so a
	// chip that failed to answer can never be handed to application code.
	entry.module->OnAttached();

	LOG_RAW_COLOR_NL(TERM_COLOR_GREEN, "- %s: attached", name);
	return RET_STATE_SUCCESS;
}

RET_STATE_t DetachModule(const ModuleEntry& entry) {
	entry.module->OnDetached();
	entry.module->DeInit();

	if (!entry.bus->DeInit()) {
		return RET_STATE_ERROR;
	}

	return RET_STATE_SUCCESS;
}

}  // namespace bsp

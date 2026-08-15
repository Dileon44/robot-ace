#include "main.h"
#include "platform.h"

#if DEBUG_ENABLE
#warning DEBUG_ENABLE
#endif

int main(void) {
	// Pl_JumpToAddr();
}

void ErrorHandler(const char* pFile, int line) {
	DISCARD_UNUSED(pFile);
	DISCARD_UNUSED(line);

	Pl_IrqOff();
	PL_SET_BKPT();
	while (1)
		;
}

#include "def_types.h"

#define X_ENTRY(state, state_str) state_str,
static const char* RetStateBuff[] = {RET_STATE_TABLE()};
#undef X_ENTRY

const char* RetState_GetStr(RET_STATE_t retState) {
	return RetStateBuff[retState];
}

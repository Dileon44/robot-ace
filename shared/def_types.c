#include "def_types.h"

#define X(a, b) b,
static const char* retStateBuff[] = {RET_STATE_TABLE()};
#undef X

const char* RetState_GetStr(RET_STATE_t retState) {
	return retStateBuff[retState];
}

#include "shell_commands.h"

extern const WshShellCmd_t Shell_ResetCmd;
extern const WshShellCmd_t Shell_DebugLogCmd;

static const WshShellCmd_t* Shell_CmdTable[] = {
	&Shell_ResetCmd,
	&Shell_DebugLogCmd,
};

bool Shell_Commands_Init(WshShell_t* pShell) {
	return WshShellRetState_TranslateToProject(
		WshShellCmd_Attach(&(pShell->Commands), Shell_CmdTable, NUM_ELEMENTS(Shell_CmdTable)));
}

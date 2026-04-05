#include "shell_users.h"
#include "wsh_shell_user.h"

static const WshShellUser_t Shell_UserTable[] = {
	{
		.Login	= "admin",
		.Salt	= "a0523cb065ee08c1",
		.Hash	= "0632cee0",  //1234
		.Groups = WSH_SHELL_CMD_GROUP_ALL,
		.Rights = WSH_SHELL_OPT_ACCESS_ADMIN,
	},
};

RET_STATE_t Shell_Users_Init(WshShell_t* pShell) {
	return WshShellRetState_TranslateToProject(WshShellUser_Attach(
		&(pShell->Users), Shell_UserTable, NUM_ELEMENTS(Shell_UserTable), NULL));
}

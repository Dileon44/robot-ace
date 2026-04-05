#include "wsh_shell_cfg.h"
#include "debug.h"
#include "platform.h"
#include "wsh_shell.h"

/* clang-format off */
#define CMD_DEBUG_LOG_OPT_TABLE() \
X_CMD_ENTRY(CMD_DEBUG_LOG_OPT_DEF, WSH_SHELL_OPT_NO(WSH_SHELL_OPT_ACCESS_ANY, "Switch logs print")) \
X_CMD_ENTRY(CMD_DEBUG_LOG_OPT_HELP, WSH_SHELL_OPT_HELP()) \
X_CMD_ENTRY(CMD_DEBUG_LOG_OPT_SET, WSH_SHELL_OPT_STR(WSH_SHELL_OPT_ACCESS_WRITE, "-l", "--lvl", "Set log level")) \
X_CMD_ENTRY(CMD_DEBUG_LOG_OPT_END, WSH_SHELL_OPT_END())
/* clang-format on */

#define X_CMD_ENTRY(en, m) en,
typedef enum { CMD_DEBUG_LOG_OPT_TABLE() CMD_DEBUG_LOG_OPT_ENUM_SIZE } CMD_DEBUG_LOG_OPT_t;
#undef X_CMD_ENTRY

#define X_CMD_ENTRY(enum, opt) {enum, opt},
WshShellOption_t DebugOptArr[] = {CMD_DEBUG_LOG_OPT_TABLE()};
#undef X_CMD_ENTRY

static WSH_SHELL_RET_STATE_t ShellCmdDebugLog(const WshShellCmd_t* pcCmd, WshShell_Size_t argc,
											  const WshShell_Char_t* pArgv[], void* pShellCtx) {
	if ((argc > 0 && !pArgv) || !pcCmd)
		return WSH_SHELL_RET_STATE_ERROR;

	WshShell_t* pParentShell	 = (WshShell_t*)pShellCtx;
	static LOG_LVL_t savedLogLvl = LOG_LVL_INFO;

	for (WshShell_Size_t tokenPos = 0; tokenPos < argc;) {
		WshShellOption_Ctx_t optCtx =
			WshShellCmd_ParseOpt(pcCmd, argc, pArgv, pParentShell->CurrUser->Rights, &tokenPos);
		if (!optCtx.Option)
			return WSH_SHELL_RET_STATE_ERR_EMPTY;

		switch (optCtx.Option->ID) {
			case CMD_DEBUG_LOG_OPT_HELP:
				WshShellCmd_PrintOptionsOverview(pcCmd);
				return WSH_SHELL_RET_STATE_SUCCESS;

			case CMD_DEBUG_LOG_OPT_DEF: {
				LOG_LVL_t currLvl = Debug_LogLvl_Get();
				if (currLvl != LOG_LVL_DISABLE) {
					savedLogLvl = currLvl;
					Debug_LogLvl_Set(LOG_LVL_DISABLE);
					WSH_SHELL_PRINT_INFO("Debug mode deactivated: ");
				} else {
					Debug_LogLvl_Set(savedLogLvl);
					WSH_SHELL_PRINT_INFO("Debug mode reactivated: ");
				}

			} break;

			case CMD_DEBUG_LOG_OPT_SET: {
				LOG_LVL_t reqLvl = LOG_LVL_DISABLE;
				char lvlStr		 = '\0';
				WshShellCmd_GetOptValue(&optCtx, argc, pArgv, sizeof(lvlStr),
										(WshShell_Size_t*)&lvlStr);
				switch (lvlStr) {
					case 'T':
					case 't':
						reqLvl = LOG_LVL_TRACE;
						break;
					case 'D':
					case 'd':
						reqLvl = LOG_LVL_DEBUG;
						break;
					case 'I':
					case 'i':
						reqLvl = LOG_LVL_INFO;
						break;
					case 'W':
					case 'w':
						reqLvl = LOG_LVL_WARNING;
						break;
					case 'E':
					case 'e':
						reqLvl = LOG_LVL_ERROR;
						break;
					case 'C':
					case 'c':
						reqLvl = LOG_LVL_CRITIC;
						break;
					default:
						reqLvl = LOG_LVL_WARNING;
						WSH_SHELL_PRINT_WARN("Invalid debug level! Switched on level 'W'\r\n");
						return WSH_SHELL_RET_STATE_ERROR;
				}

				LOG_LVL_t currLvl = Debug_LogLvl_Get();
				if (currLvl == reqLvl) {
					WSH_SHELL_PRINT_INFO("The same lvl selected: ");
				} else {
					Debug_LogLvl_Set(reqLvl);
					WSH_SHELL_PRINT_INFO("New lvl selected: ");
				}

				break;
			}

			default:
				return WSH_SHELL_RET_STATE_ERROR;
		}
	}

	WSH_SHELL_PRINT_INFO("[%s%s%s]\r\n", Debug_LogLvl_GetColor(Debug_LogLvl_Get()),
						 Debug_LogLvl_GetStr(Debug_LogLvl_Get()), ESC_RESET_STYLE);

	return WSH_SHELL_RET_STATE_SUCCESS;
}

const WshShellCmd_t Shell_DebugLogCmd = {
	.Groups	 = WSH_SHELL_CMD_GROUP_SERVICE,
	.Name	 = "log",
	.Descr	 = "Enable system log/debug output",
	.Options = DebugOptArr,
	.OptNum	 = CMD_DEBUG_LOG_OPT_ENUM_SIZE,
	.Handler = ShellCmdDebugLog,
};

/* clang-format off */
#define CMD_RESET_OPT_TABLE() \
X_CMD_ENTRY(CMD_RESET_OPT_DEF,  WSH_SHELL_OPT_NO(WSH_SHELL_OPT_ACCESS_EXECUTE, "Soft reset")) \
X_CMD_ENTRY(CMD_RESET_OPT_HELP, WSH_SHELL_OPT_HELP()) \
X_CMD_ENTRY(CMD_RESET_OPT_END,  WSH_SHELL_OPT_END())
/* clang-format on */

#define X_CMD_ENTRY(en, m) en,
typedef enum { CMD_RESET_OPT_TABLE() CMD_RESET_OPT_ENUM_SIZE } CMD_RESET_OPT_t;
#undef X_CMD_ENTRY

#define X_CMD_ENTRY(enum, opt) {enum, opt},
WshShellOption_t ResetOptArr[] = {CMD_RESET_OPT_TABLE()};
#undef X_CMD_ENTRY

static WSH_SHELL_RET_STATE_t ShellCmdReset(const WshShellCmd_t* pcCmd, WshShell_Size_t argc,
										   const WshShell_Char_t* pArgv[], void* pShellCtx) {
	if ((argc > 0 && !pArgv) || !pcCmd)
		return WSH_SHELL_RET_STATE_ERROR;

	WshShell_t* pParentShell = (WshShell_t*)pShellCtx;

	for (WshShell_Size_t tokenPos = 0; tokenPos < argc;) {
		WshShellOption_Ctx_t optCtx =
			WshShellCmd_ParseOpt(pcCmd, argc, pArgv, pParentShell->CurrUser->Rights, &tokenPos);
		if (!optCtx.Option)
			return WSH_SHELL_RET_STATE_ERR_EMPTY;

		switch (optCtx.Option->ID) {
			case CMD_RESET_OPT_DEF:
				Pl_SoftReset();
				break;

			case CMD_RESET_OPT_HELP:
				WshShellCmd_PrintOptionsOverview(pcCmd);
				break;

			default:
				return WSH_SHELL_RET_STATE_ERROR;
		}
	}

	return WSH_SHELL_RET_STATE_SUCCESS;
}

const WshShellCmd_t Shell_ResetCmd = {
	.Groups	 = WSH_SHELL_CMD_GROUP_SERVICE,
	.Name	 = "rst",
	.Descr	 = "Reset MCU",
	.Options = ResetOptArr,
	.OptNum	 = CMD_RESET_OPT_ENUM_SIZE,
	.Handler = ShellCmdReset,
};

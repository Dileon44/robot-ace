#include "shell_root.h"

#if SHELL_INTERFACE

#include "debug.h"
#include "device_name.h"
#include "mem_wrapper.h"
#include "platform.h"
#include "rtos_analyzer.h"
#include "shell_commands.h"
#include "shell_users.h"
#include "wsh_shell.h"

#if DEBUG_ENABLE
#define WSH_SHELL_AUTO_EXIT_TMO (DELAY_1_HOUR * 1)
#else /* DEBUG_ENABLE */
#define WSH_SHELL_AUTO_EXIT_TMO (DELAY_1_MINUTE * 3)
#endif /* DEBUG_ENABLE */

static WshShell_t ShellRoot;
static TaskHandle_t ShellProcess_Handle;
static TimerHandle_t ShellExit_Timer;
static LOG_LVL_t Shell_PrevLogLvl;

TaskHandle_t ShellRoot_GetTaskHandle(void) {
	return ShellProcess_Handle;
}

bool ShellRoot_SendCommand(char* pCmd, u32 waitTmo) {
	ASSERT_CHECK(pCmd);

	if (!pCmd)
		return false;

	if (!WshShell_IsAuth(&ShellRoot))
		return false;

	return Debug_SendString(pCmd, waitTmo);
}

static WshShellHistory_t PL_SHELL_HISTORY_DATA ShellRoot_HistoryStorage;

static WshShellHistory_t ShellRootHistory_Read(void) {
	return ShellRoot_HistoryStorage;
}

static void ShellRootHistory_Write(WshShellHistory_t history) {
	memcpy((void*)&ShellRoot_HistoryStorage, (void*)&history, sizeof(WshShellHistory_t));
}

static void ShellRoot_AuthClbk(void* pCtx) {
	DISCARD_UNUSED(pCtx);

	Shell_PrevLogLvl = Debug_LogLvl_Get();
	Debug_LogLvl_Set(LOG_LVL_ERROR);
	xTimerStart(ShellExit_Timer, 0);
}

static void ShellRoot_DeAuthClbk(void* pCtx) {
	DISCARD_UNUSED(pCtx);

	xTimerStop(ShellExit_Timer, 0);
	Debug_LogLvl_Set(Shell_PrevLogLvl);
}

static void ShellRoot_SymInClbk(void* pCtx) {
	DISCARD_UNUSED(pCtx);

	if (WshShell_IsAuth(&ShellRoot)) {
		BaseType_t status = xTimerChangePeriod(ShellExit_Timer, WSH_SHELL_AUTO_EXIT_TMO, 0);
		ASSERT_CHECK(status == pdPASS);
	}
}

static WshShellExtCallbacks_t ShellRoot_Callbacks = {
	.Auth	  = ShellRoot_AuthClbk,
	.DeAuth	  = ShellRoot_DeAuthClbk,
	.SymbolIn = ShellRoot_SymInClbk,
};

void ShellRoot_Init(void) {
	if (WshShell_Init(&ShellRoot, DeviceName_GetPtr(), NULL, &ShellRoot_Callbacks) !=
		WSH_SHELL_RET_STATE_SUCCESS) {
		PANIC();
		return;
	}

	if (Shell_Users_Init(&ShellRoot) != RET_STATE_SUCCESS) {
		PANIC();
		return;
	}

	if (Shell_Commands_Init(&ShellRoot) != RET_STATE_SUCCESS) {
		PANIC();
		return;
	}

	WshShellHistory_Init(&ShellRoot.HistoryIO, ShellRootHistory_Read, ShellRootHistory_Write);
}

static void vTask_Shell_Process(void* pvParameters) {
	vTaskDelay(2000);
	while (!Debug_HardwareIsInit())
		vTaskDelay(100);

	ShellRoot_Init();

#if DEBUG_ENABLE
	WshShell_Auth(&ShellRoot, "admin", "1234");
#endif /* DEBUG_ENABLE */

	for (;;) {
		char symbol = 0;
		symbol		= Debug_ReceiveSymbol(DELAY_1_SECOND);
		if (symbol)
			WshShell_InsertChar(&ShellRoot, symbol);

		// vTaskDelay(RTOS_MIN_TIMEOUT_MS);
	}
}

static void ShellRoot_ResetTimerClbk(TimerHandle_t xTimer) {
	WshShell_DeAuth(&ShellRoot, (WshShell_Char_t*)pcTimerGetName(ShellExit_Timer));
}

void FreeRTOS_ShellRoot_InitComponents(bool resources, bool tasks) {
	if (resources) {

		ShellExit_Timer = xTimerCreate("tim-shell-autoexit", WSH_SHELL_AUTO_EXIT_TMO, pdFALSE, NULL,
									   ShellRoot_ResetTimerClbk);
	}

	if (tasks) {
		RTOS_Analyzer_CreateTask(vTask_Shell_Process, "shell-interface", SHELL_TASK_STACK, NULL,
								 SHELL_TASK_PRIORITY, &ShellProcess_Handle);
	}
}

#else /* SHELL_INTERFACE */

TaskHandle_t ShellRoot_GetTaskHandle(void) {
	return NULL;
}

bool ShellRoot_SendChar(char ch, u32 waitTmo) {
	DISCARD_UNUSED(ch);
	DISCARD_UNUSED(waitTmo);
	return false;
}

bool ShellRoot_SendCharFromISR(char ch, BaseType_t* pWoken) {
	DISCARD_UNUSED(ch);
	if (pWoken)
		*pWoken = pdFALSE;
	return false;
}

bool ShellRoot_SendCommand(char* pCmd, u32 waitTmo) {
	DISCARD_UNUSED(pCmd);
	DISCARD_UNUSED(waitTmo);
	return false;
}

void ShellRoot_Init(void) {
}

void FreeRTOS_ShellRoot_InitComponents(bool resources, bool tasks) {
	DISCARD_UNUSED(resources);
	DISCARD_UNUSED(tasks);
}

#endif /* SHELL_INTERFACE */

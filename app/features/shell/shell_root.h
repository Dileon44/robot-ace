#ifndef __SHELL_ROOT_H
#define __SHELL_ROOT_H

#include "main.h"

TaskHandle_t ShellRoot_GetTaskHandle(void);
bool ShellRoot_SendCommand(char* pCmd, u32 waitTmo);
void ShellRoot_Init(void);
void FreeRTOS_ShellRoot_InitComponents(bool resources, bool tasks);

#endif /* __SHELL_ROOT_H */

#ifndef __GPIO_H
#define __GPIO_H

#include "main.h"
#include "platform.h"

void GPIO_Debug_Init(void);

void GPIO_Led_Init(void);
void GPIO_Led_Set(void);
void GPIO_Led_Reset(void);
void GPIO_Led_Toggle(void);

void GPIO_Encoder_Init(void);
void GPIO_Encoder_DeInit(void);
void GPIO_Encoder_Dir_Set(bool clockwise);

#endif /* __GPIO_H */
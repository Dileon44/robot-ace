#ifndef __USART_H
#define __USART_H

#include "main.h"
#include "platform.h"

#define USART_DEBUG			 USART2
#define USART_DEBUG_BAUDRATE 230400	 // 921600 // 115200
#define USART_DEBUG_IRQ_HD	 USART2_IRQHandler
#define USART_DEBUG_IRQ		 USART2_IRQn
#define USART_DEBUG_CLK_EN() LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)

static bool USART_FlagIsActive(USART_TypeDef* uart, u32 timeout,
							   u32 (*flagFunc)(const USART_TypeDef* uart));
RET_STATE_t USART_TxData(USART_TypeDef* uart, u8* buff, u16 buffSize, u32 timeout);

void USART_Debug_Init(Pl_USART_ClbkTx_t pTxClbk);
void USART_Debug_Irq_Enable(void);
u8 USART_Debug_GetRxByte();
USART_TypeDef* USART_Debug_GetUSART(void);
RET_STATE_t USART_Debug_TxData(u8* buff, u16 buffSize, u32 timeout);

#endif /* __USART_H */
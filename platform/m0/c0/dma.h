#ifndef __DMA_H
#define __DMA_H

#include "main.h"
#include "platform.h"

#define DMA_DEBUG_TX			DMA1
#define DMA_DEBUG_TX_CHANNEL	LL_DMA_CHANNEL_1
#define DMA_DEBUG_TX_IRQ		DMA1_Channel1_IRQn
#define DMA_DEBUG_TX_IRQ_HDL	DMA1_Channel1_IRQHandler
#define DMA_DEBUG_TX_IS_TC()	LL_DMA_IsActiveFlag_TC1(DMA_DEBUG_TX)
#define DMA_DEBUG_TX_CLEAR_TC() LL_DMA_ClearFlag_TC1(DMA_DEBUG_TX)

#define DMA_DEBUG_RX			DMA1
#define DMA_DEBUG_RX_CHANNEL	LL_DMA_CHANNEL_2
#define DMA_DEBUG_RX_IRQ		DMA1_Channel2_IRQn
#define DMA_DEBUG_RX_IRQ_HDL	DMA1_Channel2_IRQHandler
#define DMA_DEBUG_RX_IS_TC()	LL_DMA_IsActiveFlag_TC2(DMA_DEBUG_RX)
#define DMA_DEBUG_RX_CLEAR_TC() LL_DMA_ClearFlag_TC2(DMA_DEBUG_RX)
#define DMA_DEBUG_RX_IS_HT()	LL_DMA_IsActiveFlag_HT2(DMA_DEBUG_RX)
#define DMA_DEBUG_RX_CLEAR_HT() LL_DMA_ClearFlag_HT2(DMA_DEBUG_RX)

void DMA_Debug_Init_Tx(void* interface, u8* buff, u16 buffSize, Pl_USART_ClbkTx_t pTxClbk);
void DMA_Debug_Init_Rx(void* interface, u8* buff, u16 buffSize, Pl_USART_ClbkRx_t pRxClbk);
void DMA_Debug_Irq_Enable(void);
void DMA_Debug_Enable_Tx(void);
void DMA_Debug_Enable_Rx(void);
void DMA_Debug_Disable_Tx(void);
void DMA_Debug_Disable_Rx(void);
void DMA_Debug_SetDataLengthTx(u32 NbData);
void DMA_Debug_SetDataLengthRx(u32 NbData);
u32 DMA_Debug_GetDataLengthTx(void);
u32 DMA_Debug_GetDataLengthRx(void);
#endif /* __DMA_H */
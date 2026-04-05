#include "dma.h"
#include "adc.h"
#include "sys.h"
#include "usart.h"

#define DMA_SENSORS_ADC_LOW_FREQ			DMA1
#define DMA_SENSORS_ADC_LOW_FREQ_CHANNEL	LL_DMA_CHANNEL_3
#define DMA_SENSORS_ADC_LOW_FREQ_IRQ		DMA1_Channel3_IRQn
#define DMA_SENSORS_ADC_LOW_FREQ_IRQ_HDL	DMA1_Channel3_IRQHandler
#define DMA_SENSORS_ADC_LOW_FREQ_IS_TC()	LL_DMA_IsActiveFlag_TC3(DMA_SENSORS_ADC_LOW_FREQ)
#define DMA_SENSORS_ADC_LOW_FREQ_CLEAR_TC() LL_DMA_ClearFlag_TC3(DMA_SENSORS_ADC_LOW_FREQ)
#define DMA_ADC_LOW_FREQ_CLK_EN_DMAMUX()	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
#define DMA_ADC_LOW_FREQ_CLK_EN()			LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

#define DMA_ADC_HIGH_FREQ				  DMA2
#define DMA_ADC_HIGH_FREQ_CHANNEL		  LL_DMA_CHANNEL_1
#define DMA_ADC_HIGH_FREQ_IRQ			  DMA2_Channel1_IRQn
#define DMA_ADC_HIGH_FREQ_IRQ_HDL		  DMA2_Channel1_IRQHandler
#define DMA_ADC_HIGH_FREQ_IS_TC()		  LL_DMA_IsActiveFlag_TC1(DMA_ADC_HIGH_FREQ)
#define DMA_ADC_HIGH_FREQ_CLEAR_TC()	  LL_DMA_ClearFlag_TC1(DMA_ADC_HIGH_FREQ)
#define DMA_ADC_HIGH_FREQ_CLK_EN_DMAMUX() LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
#define DMA_ADC_HIGH_FREQ_CLK_EN()		  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

static Pl_USART_ClbkTx_t USART_Debug_TxClbk = Pl_Stub_CommonClbk;
static Pl_USART_ClbkRx_t USART_Debug_RxClbk = Pl_Stub_CommonClbk;

void DMA_Debug_Init_Tx(void* interface, u8* buff, u16 buffSize, Pl_USART_ClbkTx_t pTxClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(USART_Debug_TxClbk, pTxClbk);

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

	LL_DMA_InitTypeDef DMA_InitStruct;

	DMA_InitStruct.PeriphOrM2MSrcAddress =
		LL_USART_DMA_GetRegAddr((USART_TypeDef*)interface, LL_USART_DMA_REG_DATA_TRANSMIT);
	DMA_InitStruct.MemoryOrM2MDstAddress  = (u32)buff;
	DMA_InitStruct.Direction			  = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	DMA_InitStruct.Mode					  = LL_DMA_MODE_NORMAL;
	DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
	DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
	DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
	DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	DMA_InitStruct.NbData				  = buffSize;
	DMA_InitStruct.Priority				  = LL_DMA_PRIORITY_HIGH;
	DMA_InitStruct.PeriphRequest		  = LL_DMAMUX_REQ_USART2_TX;

	LL_DMA_Init(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL, &DMA_InitStruct);

	LL_DMA_EnableIT_TC(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL);
}

void DMA_Debug_Init_Rx(void* interface, u8* buff, u16 buffSize, Pl_USART_ClbkRx_t pRxClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(USART_Debug_RxClbk, pRxClbk);

	LL_DMA_InitTypeDef DMA_InitStruct;

	DMA_InitStruct.PeriphOrM2MSrcAddress =
		LL_USART_DMA_GetRegAddr((USART_TypeDef*)interface, LL_USART_DMA_REG_DATA_RECEIVE);
	DMA_InitStruct.MemoryOrM2MDstAddress  = (u32)buff;
	DMA_InitStruct.Direction			  = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
	DMA_InitStruct.Mode					  = LL_DMA_MODE_CIRCULAR;
	DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
	DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
	DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
	DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	DMA_InitStruct.NbData				  = buffSize;
	DMA_InitStruct.Priority				  = LL_DMA_PRIORITY_MEDIUM;
	DMA_InitStruct.PeriphRequest		  = LL_DMAMUX_REQ_USART2_RX;

	LL_DMA_Init(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL, &DMA_InitStruct);

	LL_DMA_EnableIT_HT(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL);
	LL_DMA_EnableIT_TC(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL);
}

void DMA_Debug_Enable_Tx(void) {
	if (!LL_DMA_IsEnabledChannel(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL)) {
		LL_DMA_EnableChannel(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL);
	}
}

void DMA_Debug_Enable_Rx(void) {
	if (!LL_DMA_IsEnabledChannel(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL)) {
		LL_DMA_EnableChannel(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL);
	}
}

void DMA_Debug_Disable_Tx(void) {
	if (LL_DMA_IsEnabledChannel(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL)) {
		LL_DMA_DisableChannel(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL);
	}
}

void DMA_Debug_Disable_Rx(void) {
	if (LL_DMA_IsEnabledChannel(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL)) {
		LL_DMA_DisableChannel(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL);
	}
}

__INLINE void DMA_Debug_SetDataLengthTx(u32 NbData) {
	LL_DMA_SetDataLength(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL, NbData);
}

__INLINE void DMA_Debug_SetDataLengthRx(u32 NbData) {
	LL_DMA_SetDataLength(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL, NbData);
}

__INLINE u32 DMA_Debug_GetDataLengthTx(void) {
	return LL_DMA_GetDataLength(DMA_DEBUG_TX, DMA_DEBUG_TX_CHANNEL);
}

__INLINE u32 DMA_Debug_GetDataLengthRx(void) {
	return LL_DMA_GetDataLength(DMA_DEBUG_RX, DMA_DEBUG_RX_CHANNEL);
}

void DMA_Debug_Irq_Enable(void) {
	Sys_NVIC_SetPrioEnable(DMA_DEBUG_TX_IRQ, NVIC_IRQ_PRIO_USART_DEBUG_DMA_TX);
	Sys_NVIC_SetPrioEnable(DMA_DEBUG_RX_IRQ, NVIC_IRQ_PRIO_USART_DEBUG_DMA_RX);
}

void DMA_DEBUG_TX_IRQ_HDL(void) {
	if (DMA_DEBUG_TX_IS_TC()) {
		DMA_DEBUG_TX_CLEAR_TC();
		LL_DMA_ClearFlag_TE1(DMA_DEBUG_TX);
		USART_Debug_TxClbk();
	}
}

void DMA_DEBUG_RX_IRQ_HDL(void) {
	if (DMA_DEBUG_RX_IS_HT()) {
		DMA_DEBUG_RX_CLEAR_HT();
		USART_Debug_RxClbk();
	}
	if (DMA_DEBUG_RX_IS_TC()) {
		DMA_DEBUG_RX_CLEAR_TC();
		USART_Debug_RxClbk();
	}
}

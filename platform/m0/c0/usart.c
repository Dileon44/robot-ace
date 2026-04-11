#include "usart.h"
#include "sys.h"

#define USART_DEBUG			 USART2
#define USART_DEBUG_BAUDRATE 2000000  // 921600	 // 230400
#define USART_DEBUG_IRQ_HD	 USART2_IRQHandler
#define USART_DEBUG_CLK_EN() LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)

static Pl_USART_ClbkRx_t USART_Debug_RxClbk = Pl_Stub_CommonClbk;

void USART_Debug_Init(Pl_USART_ClbkRx_t pRxClbk) {
	ASSIGN_NOT_NULL_VAL_TO_PTR(USART_Debug_RxClbk, pRxClbk);

	LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_SYSCLK);
	USART_DEBUG_CLK_EN();

	LL_USART_InitTypeDef USART_InitStruct = {0};
	USART_InitStruct.PrescalerValue		  = LL_USART_PRESCALER_DIV1;
	USART_InitStruct.BaudRate			  = USART_DEBUG_BAUDRATE;
	USART_InitStruct.DataWidth			  = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits			  = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity				  = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection	  = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl  = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling		  = LL_USART_OVERSAMPLING_8;

	LL_USART_Init(USART_DEBUG, &USART_InitStruct);
	LL_USART_ConfigAsyncMode(USART_DEBUG);
	LL_USART_EnableDMAReq_RX(USART_DEBUG);
	LL_USART_EnableDMAReq_TX(USART_DEBUG);
	LL_USART_EnableIT_IDLE(USART_DEBUG);

	LL_USART_Enable(USART_DEBUG);
	while ((!(LL_USART_IsActiveFlag_TEACK(USART_DEBUG))) ||
		   (!(LL_USART_IsActiveFlag_REACK(USART_DEBUG))))
		;
}

static bool USART_FlagIsActive(USART_TypeDef* uart, u32 timeout,
							   u32 (*flagFunc)(const USART_TypeDef* uart)) {
	u32 timeoutEnd = PL_GET_MS_CNT() + timeout;
	while (!flagFunc(uart)) {
		if (PL_GET_MS_CNT() > timeoutEnd) {
			PANIC();
			return true;
		}
	}

	return false;
}

RET_STATE_t USART_TxData(USART_TypeDef* uart, u8* buff, u16 buffSize, u32 timeout) {
	if (USART_FlagIsActive(uart, timeout, LL_USART_IsActiveFlag_TXE)) {
		return RET_STATE_ERR_TIMEOUT;
	}

	for (u32 i = 0; i < buffSize; i++) {
		LL_USART_TransmitData8(uart, *buff++);
		if (USART_FlagIsActive(uart, timeout, LL_USART_IsActiveFlag_TXE)) {
			return RET_STATE_ERR_TIMEOUT;
		}
	}

	return RET_STATE_SUCCESS;
}

__INLINE RET_STATE_t USART_Debug_TxData(u8* buff, u16 buffSize, u32 timeout) {
	return USART_TxData(USART_DEBUG, buff, buffSize, timeout);
}

USART_TypeDef* USART_Debug_GetUSART(void) {
	return USART_DEBUG;
}

__INLINE u8 USART_Debug_GetRxByte() {
	return LL_USART_ReceiveData8(USART_DEBUG);
}

void USART_Debug_Irq_Enable(void) {
	Sys_NVIC_SetPrioEnable(USART_DEBUG_IRQ, NVIC_IRQ_PRIO_USART_DEBUG_IDLE);
}

void USART_DEBUG_IRQ_HD(void) {
	if (LL_USART_IsActiveFlag_IDLE(USART_DEBUG)) {
		LL_USART_ClearFlag_IDLE(USART_DEBUG);
		USART_Debug_RxClbk();
	}
}

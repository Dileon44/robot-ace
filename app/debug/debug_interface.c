#include "debug_interface.h"
#include "debug_process.h"
#include "platform.h"

static u8 DEBUG_INTERFACE_USART_TX_BUFF[USART_TX_BUFF_MAX_SIZE] = {0};
static u8 DEBUG_INTERFACE_USART_RX_BUFF[USART_RX_BUFF_MAX_SIZE] = {0};

RingBuff_t DebugInterface_Usart_RingBuff;

static void DebugInterface_SerialCallbackTx(void) {
	Pl_USART_Debug_Disable_Tx();
}

static void DebugInterface_SerialCallbackRx(void) {
	u16 rxCnt	 = NUM_ELEMENTS(DEBUG_INTERFACE_USART_RX_BUFF) - Pl_USART_Debug_GetDataLengthRx();
	u16 newRxCnt = 0;
	static u16 oldRxCnt = 0;

	if (rxCnt >= oldRxCnt)
		newRxCnt = rxCnt - oldRxCnt;
	else
		newRxCnt = (NUM_ELEMENTS(DEBUG_INTERFACE_USART_RX_BUFF) - oldRxCnt) + rxCnt;

	RingBuff_InterruptCallback(&DebugInterface_Usart_RingBuff, newRxCnt);

	oldRxCnt = rxCnt;
}

void DebugInterface_Init(void) {
	RingBuff_Init(&DebugInterface_Usart_RingBuff, DEBUG_INTERFACE_USART_RX_BUFF,
				  CAPASITY_RING_BUFF_USART_RX, NULL);

	Pl_USART_Debug_Init(DEBUG_INTERFACE_USART_TX_BUFF, NUM_ELEMENTS(DEBUG_INTERFACE_USART_TX_BUFF),
						DebugInterface_SerialCallbackTx, DEBUG_INTERFACE_USART_RX_BUFF,
						NUM_ELEMENTS(DEBUG_INTERFACE_USART_RX_BUFF),
						DebugInterface_SerialCallbackRx);

	DebugInterface_USART_Enable();
}

void DebugInterface_USART_Enable(void) {
	Pl_USART_Debug_Enable_Tx();
	Pl_USART_Debug_Enable_Rx();
}

u8* DebugInterface_GetPtrBuffTx(void) {
	return &DEBUG_INTERFACE_USART_TX_BUFF[0];
}

u8* DebugInterface_GetPtrBuffRx(void) {
	return &DEBUG_INTERFACE_USART_RX_BUFF[0];
}

u32 DebugInterface_GetLenBuffTx(void) {
	return (u32)USART_TX_BUFF_MAX_SIZE;
}

RingBuff_t* DebugInterface_GetRingBuffRx(void) {
	return &DebugInterface_Usart_RingBuff;
}

u32 DebugInterface_CalcNumberRxData(void) {
	return NUM_ELEMENTS(DEBUG_INTERFACE_USART_RX_BUFF) - Pl_USART_Debug_GetDataLengthRx();
}

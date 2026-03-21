#ifndef __DEBUG_INTERFACE_H
#define __DEBUG_INTERFACE_H

#include "main.h"
#include "ring_buff.h"
// #include "platform.h"

// int _write(int fd, char* ptr, int len) {
// 	Pl_USART_Debug_TxData((u8*)ptr, (u16)len);
// 	return len;
// }

#define DEBUG_PRINT(_f_, ...)         \
	do {                              \
		printf((_f_), ##__VA_ARGS__); \
		fflush(stdout);               \
	} while (0)

#define USART_TX_BUFF_MAX_SIZE ((u8)128)
#define USART_RX_BUFF_MAX_SIZE ((u8)64)

#define CAPASITY_RING_BUFF_USART_RX USART_RX_BUFF_MAX_SIZE

static void DebugInterface_SerialCallbackTx(void);
static void DebugInterface_SerialCallbackRx(void);
void DebugInterface_Init(void);
void DebugInterface_USART_Enable(void);
u8* DebugInterface_GetPtrBuffTx(void);
u8* DebugInterface_GetPtrBuffRx(void);
u32 DebugInterface_GetLenBuffTx(void);
RingBuff_t* DebugInterface_GetRingBuffRx(void);
u32 DebugInterface_CalcNumberRxData(void);

#endif /* __DEBUG_INTERFACE_H */
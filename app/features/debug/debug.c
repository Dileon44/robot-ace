#include "debug.h"
#include "platform.h"
#include "ring_buff.h"

/* ============================================================================
 * Config
 * ============================================================================ */
#define DBG_TX_BUFF_SIZE  256
#define DBG_RX_BUFF_SIZE  256
#define DBG_TX_TIMEOUT_MS DELAY_1_SECOND
#define DBG_TX_QUEUE_SIZE 20
#define DBG_RX_SYM_Q_SIZE 64

#ifndef DEBUG_DEF_LOG_LVL
#define DEBUG_DEF_LOG_LVL LOG_LVL_DEBUG
#endif

/* ============================================================================
 * Log level table  (built from X-macro in debug.h)
 * ============================================================================ */
typedef struct {
	const char* Str;
	const char* Color;
} LogLvl_t;

#define X_ENTRY(lvl, lvl_str, lvl_color) {lvl_str, lvl_color},
static const LogLvl_t Debug_LvlTable[] = {DEBUG_LVL_TABLE()};
#undef X_ENTRY

#define DEBUG_LVL_TABLE_SIZE (sizeof(Debug_LvlTable) / sizeof(Debug_LvlTable[0]))

/* ============================================================================
 * TX message struct (heap-allocated, sent through Debug_SendQueue)
 * ============================================================================ */
typedef struct {
	char* Ptr;
	u16 Len;
} DebugMsg_t;

/* ============================================================================
 * Static data
 * ============================================================================ */
static u8 DEBUG_TX_BUFF[DBG_TX_BUFF_SIZE];
static u8 DEBUG_RX_BUFF[DBG_RX_BUFF_SIZE];

/* UART / DMA */
static RingBuff_t Debug_RxBuff;
static u32 Debug_DmaRxPos;
static SemaphoreHandle_t Debug_TxSem;
static SemaphoreHandle_t Debug_TxMtx;

/* State */
static bool Debug_IsInit		= false;
static LOG_LVL_t Debug_LogLevel = LOG_LVL_DEBUG;

/* RX — notifies shell task on new UART data */
static SemaphoreHandle_t Debug_RxSem = NULL;

/* RX — programmatic character injection (Debug_SendChar / Debug_SendString) */
static QueueHandle_t Debug_RxSymbolQueue = NULL;

/* TX — async send queue + task handle */
static QueueHandle_t Debug_SendQueue = NULL;
static TaskHandle_t Debug_SendHandle = NULL;

/* ============================================================================
 * ISR callbacks
 * ============================================================================ */

/* Called from DMA TC ISR (priority 5, maskable) */
static void Debug_TxClbk(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(Debug_TxSem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Called from DMA HT/TC and USART IDLE ISRs (priority 5, maskable).
 * Calculates how many new bytes DMA wrote since last call, forwards them
 * to the ring buffer, and unblocks the shell task. */
static void Debug_RxClbk(void) {
	u32 curPos	= (u32)(DBG_RX_BUFF_SIZE - Pl_Debug_GetDataLengthRx());
	u32 prevPos = Debug_DmaRxPos;
	u16 newBytes;

	if (curPos >= prevPos) {
		newBytes = (u16)(curPos - prevPos);
	} else {
		newBytes = (u16)(DBG_RX_BUFF_SIZE - prevPos + curPos);
	}

	Debug_DmaRxPos = curPos;

	if (newBytes > 0) {
		RingBuff_InterruptCallback(&Debug_RxBuff, newBytes);

		if (Debug_RxSem) {
			BaseType_t xWoken = pdFALSE;
			xSemaphoreGiveFromISR(Debug_RxSem, &xWoken);
			portYIELD_FROM_ISR(xWoken);
		}
	}
}

/* ============================================================================
 * Init
 * ============================================================================ */
/* Called from main() directly — hardware only, no RTOS objects, no Delay calls. */
void Debug_Init(void) {
	Debug_DmaRxPos = 0;
	Debug_LogLevel = DEBUG_DEF_LOG_LVL;

	RingBuff_Init(&Debug_RxBuff, DEBUG_RX_BUFF, DBG_RX_BUFF_SIZE, NULL);

	Pl_Debug_Init(DEBUG_TX_BUFF, DBG_TX_BUFF_SIZE, Debug_TxClbk, DEBUG_RX_BUFF, DBG_RX_BUFF_SIZE,
				  Debug_RxClbk);

	Pl_Debug_SetDataLengthRx(DBG_RX_BUFF_SIZE);
	Pl_Debug_Enable_Rx();

	Debug_IsInit = true;
}

/* ============================================================================
 * Debug_Write — low-level blocking DMA TX (used by the send task)
 * ============================================================================ */
RET_STATE_t Debug_Write(u8* ptr, u16 len) {
	if (!len) {
		return RET_STATE_ERR_PARAM;
	}

	/* Before RTOS is running or before init — polling fallback */
	if (!Debug_TxSem || !Debug_TxMtx || !SYS_OS_IS_RUNNING()) {
		return Pl_Debug_TxData(ptr, len);
	}

	if (xSemaphoreTake(Debug_TxMtx, DBG_TX_TIMEOUT_MS) != pdTRUE) {
		return RET_STATE_ERR_BUSY;
	}

	u16 txLen = (len > DBG_TX_BUFF_SIZE) ? (u16)DBG_TX_BUFF_SIZE : len;
	memcpy(DEBUG_TX_BUFF, ptr, txLen);

	Pl_Debug_Disable_Tx();
	Pl_Debug_SetDataLengthTx(txLen);
	Pl_Debug_Enable_Tx();

	if (xSemaphoreTake(Debug_TxSem, DBG_TX_TIMEOUT_MS) != pdTRUE) {
		Pl_Debug_Disable_Tx();
		xSemaphoreGive(Debug_TxMtx);
		return RET_STATE_ERR_TIMEOUT;
	}

	xSemaphoreGive(Debug_TxMtx);
	return RET_STATE_SUCCESS;
}

/* ============================================================================
 * _write override — routes printf/fwrite through async TX queue when RTOS is
 * running; falls back to polling before the scheduler starts.
 * Overrides the __WEAK stub in shared/syscalls.c.
 * ============================================================================ */
int _write(int fd, char* ptr, int len) {
	DISCARD_UNUSED(fd);

	if (len <= 0) {
		return len;
	}

	if (!Debug_SendQueue || !SYS_OS_IS_RUNNING()) {
		/* Pre-RTOS or queue not yet created — blocking polling */
		Pl_Debug_TxData((u8*)ptr, (u16)len);
		return len;
	}

	char* pBuff = pvPortMalloc((size_t)len);
	if (!pBuff) {
		/* Silent drop — cannot PANIC inside the printf/libc chain */
		return len;
	}

	memcpy(pBuff, ptr, (size_t)len);

	DebugMsg_t msg = {.Ptr = pBuff, .Len = (u16)len};
	if (xQueueSend(Debug_SendQueue, &msg, 0) != pdTRUE) {
		vPortFree(pBuff);
	}

	return len;
}

/* ============================================================================
 * RX — ring buffer accessor
 * ============================================================================ */
RingBuff_t* Debug_GetRxBuff(void) {
	return &Debug_RxBuff;
}

/* ============================================================================
 * RX — init status
 * ============================================================================ */
bool Debug_HardwareIsInit(void) {
	return Debug_IsInit;
}

/* ============================================================================
 * RX — receive one character with timeout.
 * Checks ring buffer first (real UART data), then injection queue, then blocks
 * on the RX semaphore and retries. Returns 0 on timeout.
 * ============================================================================ */
char Debug_ReceiveSymbol(u32 tmo) {
	for (;;) {
		/* 1. Real UART data from DMA ring buffer */
		if (RingBuff_GetCnt(&Debug_RxBuff) > 0) {
			u8 byte		= 0;
			u16 readCnt = 0;
			RingBuff_Ring2Line_Copy(&Debug_RxBuff, &byte, 1, &readCnt);
			if (readCnt > 0) {
				return (char)byte;
			}
		}

		/* 2. Programmatically injected character (Debug_SendChar / Debug_SendString) */
		if (Debug_RxSymbolQueue) {
			char ch = 0;
			if (xQueueReceive(Debug_RxSymbolQueue, &ch, 0) == pdTRUE) {
				return ch;
			}
		}

		/* 3. Block until UART ISR or injection wakes us, then retry */
		if (!Debug_RxSem) {
			return 0;
		}
		if (xSemaphoreTake(Debug_RxSem, tmo) != pdTRUE) {
			return 0;
		}
	}
}

/* ============================================================================
 * RX — programmatic character injection
 * ============================================================================ */
bool Debug_SendChar(char ch, u32 tmo) {
	if (!Debug_RxSymbolQueue) {
		return false;
	}
	bool ret = (xQueueSend(Debug_RxSymbolQueue, &ch, tmo) == pdTRUE);
	if (ret && Debug_RxSem) {
		xSemaphoreGive(Debug_RxSem);
	}
	return ret;
}

bool Debug_SendCharFromISR(char ch, BaseType_t* pWoken) {
	if (!Debug_RxSymbolQueue) {
		if (pWoken) {
			*pWoken = pdFALSE;
		}
		return false;
	}
	bool ret = (xQueueSendFromISR(Debug_RxSymbolQueue, &ch, pWoken) == pdTRUE);
	if (ret && Debug_RxSem) {
		xSemaphoreGiveFromISR(Debug_RxSem, pWoken);
	}
	return ret;
}

bool Debug_SendString(char* pStr, u32 tmo) {
	ASSERT_CHECK(pStr);

	if (!pStr || !Debug_RxSymbolQueue) {
		return false;
	}

	u32 len = strlen(pStr);
	for (u32 i = 0; i < len; i++) {
		if (xQueueSend(Debug_RxSymbolQueue, &pStr[i], tmo) != pdTRUE) {
			return false;
		}
	}

	if (Debug_RxSem) {
		xSemaphoreGive(Debug_RxSem);
	}
	return true;
}

/* ============================================================================
 * Log level API
 * ============================================================================ */
void Debug_LogLvl_Set(LOG_LVL_t lvl) {
	Debug_LogLevel = lvl;
}

LOG_LVL_t Debug_LogLvl_Get(void) {
	return Debug_LogLevel;
}

const char* Debug_LogLvl_GetStr(LOG_LVL_t lvl) {
	if ((u32)lvl >= DEBUG_LVL_TABLE_SIZE) {
		return "?????";
	}
	return Debug_LvlTable[lvl].Str;
}

const char* Debug_LogLvl_GetColor(LOG_LVL_t lvl) {
	if ((u32)lvl >= DEBUG_LVL_TABLE_SIZE) {
		return ESC_RESET_STYLE;
	}
	return Debug_LvlTable[lvl].Color;
}

/* ============================================================================
 * Debug send task — drains the TX queue, sending chunks over DMA
 * ============================================================================ */
TaskHandle_t Debug_GetSendHandle(void) {
	return Debug_SendHandle;
}

static void vTask_DebugSend_Process(void* pvParameters) {
	DISCARD_UNUSED(pvParameters);

	DebugMsg_t msg;
	for (;;) {
		xQueueReceive(Debug_SendQueue, &msg, portMAX_DELAY);

		for (;;) {
			/* Send in chunks matching the DMA TX buffer */
			u16 remaining = msg.Len;
			char* pData	  = msg.Ptr;

			while (remaining > 0) {
				u16 chunk = (remaining > (u16)DBG_TX_BUFF_SIZE) ? (u16)DBG_TX_BUFF_SIZE : remaining;
				Debug_Write((u8*)pData, chunk);
				pData += chunk;
				remaining -= chunk;
			}

			vPortFree(msg.Ptr);

			/* Drain any pending messages without blocking */
			if (xQueueReceive(Debug_SendQueue, &msg, 0) != pdTRUE) {
				break;
			}
		}

/* Brief yield in production so the send task doesn't starve motor tasks
 * when there is a high volume of log output. Skipped in debug builds. */
#if !DEBUG_ENABLE
		vTaskDelay(5);
#endif
	}
}

/* ============================================================================
 * FreeRTOS integration
 * ============================================================================ */
void FreeRTOS_Debug_InitComponents(bool resources, bool tasks) {
	if (resources) {
		Debug_TxSem			= xSemaphoreCreateBinary();
		Debug_TxMtx			= xSemaphoreCreateMutex();
		Debug_RxSem			= xSemaphoreCreateBinary();
		Debug_SendQueue		= xQueueCreate(DBG_TX_QUEUE_SIZE, sizeof(DebugMsg_t));
		Debug_RxSymbolQueue = xQueueCreate(DBG_RX_SYM_Q_SIZE, sizeof(char));
	}

	if (tasks) {
		if (!Debug_SendHandle) {
			xTaskCreate(vTask_DebugSend_Process, "debug-process", DEBUG_SEND_TASK_STACK, NULL,
						DEBUG_SEND_TASK_PRIORITY, &Debug_SendHandle);
		}
	}
}

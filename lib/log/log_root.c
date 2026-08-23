#include "log_root.h"

#include "platform.h"
#include "ring_buff.h"

#if LOG_ENABLE

/* ============================================================================
 * Config
 * ============================================================================ */
#define LOG_TX_BUFF_SIZE  256
#define LOG_RX_BUFF_SIZE  256
#define LOG_TX_TIMEOUT_MS DELAY_1_SECOND
#define LOG_TX_QUEUE_SIZE 64
#define LOG_RX_SYM_Q_SIZE 64

/* ============================================================================
 * Static data
 * ============================================================================ */
static u8 Log_TxBuff[LOG_TX_BUFF_SIZE];
static u8 Log_RxBuff[LOG_RX_BUFF_SIZE];

/* UART / DMA */
static RingBuff_t Log_RxRing;
static u32 Log_DmaRxPos;
static SemaphoreHandle_t Log_TxSem;
static SemaphoreHandle_t Log_TxMtx;

/* State */
static bool Log_IsInit;

/* RX — notifies shell task on new UART data */
static SemaphoreHandle_t Log_RxSem;

/* RX — programmatic character injection (Log_SendChar / Log_SendString) */
static QueueHandle_t LogRxSymbol_Queue;

/* TX — async send queue + drain task handle */
static QueueHandle_t LogSend_Queue;
static TaskHandle_t LogSend_Handle;

/* Count of log messages dropped (alloc failure or full send queue). */
static volatile u32 LogOut_DropCnt;

/* ============================================================================
 * ISR callbacks
 * ============================================================================ */

/* Called from DMA TC ISR (priority 5, maskable) */
static void Log_TxClbk(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(Log_TxSem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Called from DMA HT/TC and USART IDLE ISRs (priority 5, maskable).
 * Calculates how many new bytes DMA wrote since last call, forwards them
 * to the ring buffer, and unblocks the shell task. */
static void Log_RxClbk(void) {
	u32 curPos	= (u32)(LOG_RX_BUFF_SIZE - Pl_Debug_GetDataLengthRx());
	u32 prevPos = Log_DmaRxPos;
	u16 newBytes;

	if (curPos >= prevPos) {
		newBytes = (u16)(curPos - prevPos);
	} else {
		newBytes = (u16)(LOG_RX_BUFF_SIZE - prevPos + curPos);
	}

	Log_DmaRxPos = curPos;

	if (newBytes > 0) {
		RingBuff_InterruptCallback(&Log_RxRing, newBytes);

		if (Log_RxSem) {
			BaseType_t xWoken = pdFALSE;
			xSemaphoreGiveFromISR(Log_RxSem, &xWoken);
			portYIELD_FROM_ISR(xWoken);
		}
	}
}

/* ============================================================================
 * Serial transport (registered into the RTOS-free log core)
 * ============================================================================ */

/* Hardware only, no RTOS objects, no Delay calls — reached from Log_Init(). */
static bool Transport_Init(void) {
	Log_DmaRxPos = 0;

	RingBuff_Init(&Log_RxRing, Log_RxBuff, LOG_RX_BUFF_SIZE, NULL);

	Pl_Debug_Init(Log_TxBuff, LOG_TX_BUFF_SIZE, Log_TxClbk, Log_RxBuff, LOG_RX_BUFF_SIZE,
				  Log_RxClbk);

	Pl_Debug_SetDataLengthRx(LOG_RX_BUFF_SIZE);
	Pl_Debug_Enable_Rx();

	Log_IsInit = true;

	return Log_IsInit;
}

static bool Transport_DeInit(void) {
	Pl_Debug_Disable_Tx();
	Pl_Debug_Disable_Rx();

	Log_IsInit = false;

	return true;
}

static bool Transport_IsReady(void) {
	return Log_IsInit;
}

/* One DMA transfer, at most LOG_TX_BUFF_SIZE bytes. Before the scheduler runs
 * (or before the RTOS objects exist) it degrades to a polling transfer. */
static RET_STATE_t Transport_TxChunk(const u8* pBuff, u16 size) {
	if (!Log_TxSem || !Log_TxMtx || !SYS_OS_IS_RUNNING()) {
		return Pl_Debug_TxData((u8*)pBuff, size);
	}

	if (xSemaphoreTake(Log_TxMtx, LOG_TX_TIMEOUT_MS) != pdTRUE) {
		return RET_STATE_ERR_BUSY;
	}

	memcpy(Log_TxBuff, pBuff, size);

	Pl_Debug_Disable_Tx();
	Pl_Debug_SetDataLengthTx(size);
	Pl_Debug_Enable_Tx();

	if (xSemaphoreTake(Log_TxSem, LOG_TX_TIMEOUT_MS) != pdTRUE) {
		Pl_Debug_Disable_Tx();
		xSemaphoreGive(Log_TxMtx);
		return RET_STATE_ERR_TIMEOUT;
	}

	xSemaphoreGive(Log_TxMtx);
	return RET_STATE_SUCCESS;
}

/* Chunking lives here so every caller — the drain task, LOG_RAW_DIRECT, the
 * pre-scheduler fallback — can hand over a buffer of any length. */
static bool Transport_Transmit(const u8* pBuff, u32 size) {
	if (!pBuff || !size) {
		return false;
	}

	while (size > 0) {
		u16 chunk = (size > (u32)LOG_TX_BUFF_SIZE) ? (u16)LOG_TX_BUFF_SIZE : (u16)size;

		if (Transport_TxChunk(pBuff, chunk) != RET_STATE_SUCCESS) {
			return false;
		}

		pBuff += chunk;
		size -= chunk;
	}

	return true;
}

static const Log_Transport_t Log_Transport_Serial = {
	.init	  = Transport_Init,
	.deinit	  = Transport_DeInit,
	.transmit = Transport_Transmit,
	.is_ready = Transport_IsReady,
};

/* ============================================================================
 * `_write` output sink: copy stdout bytes into the send queue so logging returns
 * immediately; the drain task flushes them to the transport when it can.
 * ============================================================================ */
static void LogOut_Sink(const char* pBuff, u32 len) {
	/* Pre-RTOS or queue not yet created — transmit synchronously instead */
	if (!LogSend_Queue || !SYS_OS_IS_RUNNING()) {
		Log_TransmitBuff((char*)pBuff, len);
		return;
	}

	char* pMem = pvPortMalloc((size_t)len);
	if (!pMem) {
		/* Silent drop — cannot PANIC inside the printf/libc chain */
		LogOut_DropCnt++;
		return;
	}

	memcpy((void*)pMem, (const void*)pBuff, len);
	LogMsg_t msg = {
		.Ptr = pMem,
		.Len = len,
	};
	if (xQueueSend(LogSend_Queue, (void*)&msg, 0) != pdPASS) {
		vPortFree(pMem);
		LogOut_DropCnt++;
	}
}

u32 Log_GetDroppedMsgCount(void) {
	return LogOut_DropCnt;
}

TaskHandle_t LogSend_GetTaskHandle(void) {
	return LogSend_Handle;
}

/* ============================================================================
 * RX path (feeds the shell)
 *
 * Receive one character with timeout. Checks the DMA ring buffer first (real
 * UART data), then the injection queue, then blocks on the RX semaphore and
 * retries. Returns 0 on timeout.
 * ============================================================================ */
char Log_ReceiveSymbol(u32 delay) {
	for (;;) {
		/* 1. Real UART data from DMA ring buffer */
		if (RingBuff_GetCnt(&Log_RxRing) > 0) {
			u8 byte		= 0;
			u16 readCnt = 0;
			RingBuff_Ring2Line_Copy(&Log_RxRing, &byte, 1, &readCnt);
			if (readCnt > 0) {
				return (char)byte;
			}
		}

		/* 2. Programmatically injected character (Log_SendChar / Log_SendString) */
		if (LogRxSymbol_Queue) {
			char ch = 0;
			if (xQueueReceive(LogRxSymbol_Queue, &ch, 0) == pdTRUE) {
				return ch;
			}
		}

		/* 3. Block until UART ISR or injection wakes us, then retry */
		if (!Log_RxSem) {
			return 0;
		}
		if (xSemaphoreTake(Log_RxSem, delay) != pdTRUE) {
			return 0;
		}
	}
}

bool Log_SendChar(char ch, u32 waitTmo) {
	if (!LogRxSymbol_Queue) {
		return false;
	}

	bool ret = (bool)(xQueueSend(LogRxSymbol_Queue, &ch, waitTmo) == pdTRUE);
	if (ret && Log_RxSem) {
		xSemaphoreGive(Log_RxSem);
	}

	return ret;
}

bool Log_SendCharFromISR(char ch, BaseType_t* pWoken) {
	if (!LogRxSymbol_Queue) {
		if (pWoken) {
			*pWoken = pdFALSE;
		}
		return false;
	}

	bool ret = (bool)(xQueueSendFromISR(LogRxSymbol_Queue, &ch, pWoken) == pdTRUE);
	if (ret && Log_RxSem) {
		xSemaphoreGiveFromISR(Log_RxSem, pWoken);
	}

	return ret;
}

bool Log_SendString(char* pStr, u32 waitTmo) {
	ASSERT_CHECK(pStr);

	if (!pStr || !LogRxSymbol_Queue) {
		return false;
	}

	u32 strLen = strlen(pStr);
	for (u32 i = 0; i < strLen; i++) {
		if (xQueueSend(LogRxSymbol_Queue, &pStr[i], waitTmo) != pdTRUE) {
			return false;
		}
	}

	if (Log_RxSem) {
		xSemaphoreGive(Log_RxSem);
	}

	return true;
}

/* ============================================================================
 * Device / system banner
 * ============================================================================ */

void Log_PrintMainInfo(void) {
	LOG_RAW(TERM_END_LINE TERM_END_LINE TERM_COLOR_RED
			"+++++++++ Device started +++++++++" TERM_RESET_STYLE TERM_END_LINE);

	LOG_RAW(TERM_END_LINE "Main info: " TERM_END_LINE);
	LOG_RAW(" * Project: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_PROJECT);
	LOG_RAW(" * Fw/type: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_TARGET);
	LOG_RAW(" * Fw/version: " TERM_COLOR_CYAN "%d.%d.%d" TERM_RESET_STYLE TERM_END_LINE,
			FW_VER_MAJOR, FW_VER_MINOR, FW_VER_BUILD);
	LOG_RAW(" * Fw/tag: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_TAG);
	LOG_RAW(" * Hw/platform: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_PLATFORM);
	LOG_RAW(" * Fw/BSP: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_BSP);
	LOG_RAW(" * GIT Hash: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_GIT_HASH);
	LOG_RAW(" * GIT Branch: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, FW_GIT_BRANCH);
	LOG_RAW(" * Build date/time: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE,
			(__DATE__ " " __TIME__));

	LOG_RAW(TERM_RESET_STYLE);
}

void Log_PrintSysInfo(void) {
	char tmpBuff[32];

	LOG_RAW(TERM_END_LINE "Sys info: " TERM_END_LINE);
	Pl_UID_GetStrAndPtr(tmpBuff);
	LOG_RAW(" * MCU UID: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, tmpBuff);
	Pl_CPU_GetStrAndPtr(tmpBuff);
	LOG_RAW(" * MCU RevID/DevID: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE, tmpBuff);
	LOG_RAW(" * MCU Flash (KBytes): " TERM_COLOR_CYAN "%d" TERM_RESET_STYLE TERM_END_LINE,
			Pl_MCU_GetFlashSize());
	LOG_RAW(" * MCU Reset Flag: " TERM_COLOR_CYAN "%s" TERM_RESET_STYLE TERM_END_LINE,
			Pl_GetRstFlagStr());
	LOG_RAW(" * MCU Clocks (Hz): " TERM_END_LINE);
	LOG_RAW(TERM_COLOR_CYAN "   SYSCLK: %u, HCLK: %u" TERM_END_LINE, Pl_SysClk.SYSCLK,
			Pl_SysClk.HCLK);
	LOG_RAW(TERM_COLOR_CYAN "   APB1: %u, APB2: %u" TERM_END_LINE, Pl_SysClk.APB1, Pl_SysClk.APB2);
	// LOG_RAW("    PLLP: %u, PLLQ: %u, PLLR: %u" TERM_END_LINE, Pl_SysClk.PLL.P, Pl_SysClk.PLL.Q,
	// 		Pl_SysClk.PLL.R);
	// LOG_RAW(TERM_COLOR_CYAN "    ADC: %u" TERM_END_LINE, Pl_SysClk.ADC);

	LOG_RAW(TERM_RESET_STYLE);
}

/* ============================================================================
 * Output drain task — flushes the TX queue over DMA
 * ============================================================================ */

static void vTask_LogSend_Process(void* pvParameters) {
	DISCARD_UNUSED(pvParameters);

	vTaskDelay(DELAY_1_SECOND);

	while (!Log_HardwareIsInit())
		vTaskDelay(pdMS_TO_TICKS(100));

	Log_PrintMainInfo();
	Log_PrintSysInfo();

	LogMsg_t msg;

	for (;;) {
		xQueueReceive(LogSend_Queue, &msg, portMAX_DELAY);

		for (;;) {
			Log_TransmitBuff(msg.Ptr, msg.Len);
			vPortFree(msg.Ptr);

			/* Drain any pending messages without blocking */
			if (xQueueReceive(LogSend_Queue, &msg, 0) != pdTRUE) {
				break;
			}
		}

/* Brief yield in production so the send task doesn't starve motor tasks
 * when there is a high volume of log output. Skipped in debug builds. */
#if !DEBUG_ENABLE
		vTaskDelay(5);
#endif /* !DEBUG_ENABLE */
	}
}

/* ============================================================================
 * FreeRTOS integration
 * ============================================================================ */

void FreeRTOS_LogSend_InitComponents(bool resources, bool tasks) {
	if (resources) {
		Log_TxSem		  = xSemaphoreCreateBinary();
		Log_TxMtx		  = xSemaphoreCreateMutex();
		Log_RxSem		  = xSemaphoreCreateBinary();
		LogSend_Queue	  = xQueueCreate(LOG_TX_QUEUE_SIZE, sizeof(LogMsg_t));
		LogRxSymbol_Queue = xQueueCreate(LOG_RX_SYM_Q_SIZE, sizeof(char));

		Log_TransportRegister(&Log_Transport_Serial);
		Log_SetOutputSink(LogOut_Sink);
	}

	if (tasks) {
		if (!LogSend_Handle) {
			xTaskCreate(vTask_LogSend_Process, "log-out-driver", LOG_SEND_TASK_STACK, NULL,
						LOG_SEND_TASK_PRIORITY, &LogSend_Handle);
		}
	}
}

#else /* LOG_ENABLE */

/* Module disabled: keep the host API linkable as no-ops. */
void Log_PrintMainInfo(void) {
}

void Log_PrintSysInfo(void) {
}

void FreeRTOS_LogSend_InitComponents(bool resources, bool tasks) {
	DISCARD_UNUSED(resources);
	DISCARD_UNUSED(tasks);
}

TaskHandle_t LogSend_GetTaskHandle(void) {
	return NULL;
}

u32 Log_GetDroppedMsgCount(void) {
	return 0;
}

bool Log_SendChar(char ch, u32 waitTmo) {
	DISCARD_UNUSED(ch);
	DISCARD_UNUSED(waitTmo);
	return false;
}

bool Log_SendCharFromISR(char ch, BaseType_t* pWoken) {
	DISCARD_UNUSED(ch);
	if (pWoken)
		*pWoken = pdFALSE;
	return false;
}

bool Log_SendString(char* pStr, u32 waitTmo) {
	DISCARD_UNUSED(pStr);
	DISCARD_UNUSED(waitTmo);
	return false;
}

char Log_ReceiveSymbol(u32 delay) {
	DISCARD_UNUSED(delay);
	return 0;
}

#endif /* LOG_ENABLE */

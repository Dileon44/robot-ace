#include "log.h"

#if LOG_ENABLE

/*
 * RTOS-free I/O core. The physical transport (UART/DMA) and the asynchronous
 * output path (queue + task) are host concerns:
 *   - the transport is injected as a Log_Transport_t vtable;
 *   - the async path is injected as a Log_OutputSink_t. When a sink is set,
 *     `_write` hands stdout bytes to it (the host copies them into a queue and a
 *     task drains it later); otherwise bytes are transmitted synchronously.
 */
static const Log_Transport_t* Log_Transport = NULL;
static Log_OutputSink_t Log_OutputSink		= NULL;

void Log_TransportRegister(const Log_Transport_t* pcTransport) {
	Log_Transport = pcTransport;
}

void Log_SetOutputSink(Log_OutputSink_t sink) {
	Log_OutputSink = sink;
}

/*
 * Wrapper for printf() function native using via selected interface.
 * Overrides the __WEAK stub in shared/syscalls.c.
 */
int _write(int fd, char* ptr, int len) {
	DISCARD_UNUSED(fd);

	if (len <= 0)
		return len;

	if (Log_OutputSink)
		Log_OutputSink(ptr, (u32)len);
	else
		Log_TransmitBuff(ptr, (u32)len);

	return len;
}

/*
 * No setvbuf() here on purpose: _isatty() already reports a terminal, so newlib
 * line-buffers stdout by itself, and every LOG_RAW ends with fflush(stdout)
 * anyway. stdin is unused (the shell reads through Log_ReceiveSymbol) and
 * nothing writes to stderr — the three calls cost 360 B of flash and buy
 * nothing. Add them back only if a caller starts relying on scanf/fgets.
 */
bool Log_Init(void) {
	if (Log_Transport && Log_Transport->init)
		return Log_Transport->init();

	return false;
}

bool Log_DeInit(void) {
	if (Log_Transport && Log_Transport->deinit)
		return Log_Transport->deinit();

	return false;
}

bool Log_HardwareIsInit(void) {
	if (Log_Transport && Log_Transport->is_ready)
		return Log_Transport->is_ready();

	return false;
}

bool Log_TransmitBuff(char* pBuff, u32 size) {
	if (Log_Transport && Log_Transport->transmit)
		return Log_Transport->transmit((const u8*)pBuff, size);

	return false;
}

#else /* LOG_ENABLE */

/*
 * Module disabled: keep the transport/IO API linkable as no-ops. `_write` is
 * intentionally NOT defined here — the __WEAK fallback (shared/syscalls.c) is
 * used, so stray printf output is quietly dropped without a transport.
 */
void Log_TransportRegister(const Log_Transport_t* pcTransport) {
	DISCARD_UNUSED(pcTransport);
}

void Log_SetOutputSink(Log_OutputSink_t sink) {
	DISCARD_UNUSED(sink);
}

bool Log_Init(void) {
	return false;
}

bool Log_DeInit(void) {
	return false;
}

bool Log_HardwareIsInit(void) {
	return false;
}

bool Log_TransmitBuff(char* pBuff, u32 size) {
	DISCARD_UNUSED(pBuff);
	DISCARD_UNUSED(size);
	return false;
}

#endif /* LOG_ENABLE */

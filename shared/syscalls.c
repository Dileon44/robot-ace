/**
 * @file stdio.c
 * @brief Implementation of newlib syscall
 */

#include "main.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#undef errno
extern int errno;
extern int _end;

// __attribute__ ((used))
// caddr_t _sbrk(int incr)
// {
// 	static unsigned char *heap = NULL;
// 	unsigned char *prev_heap;

// 	if (heap == NULL)
// 		heap = (unsigned char *)&_end;

// 	prev_heap = heap;
// 	heap += incr;
// 	return (caddr_t) prev_heap;
// }

// __attribute__ ((used))
// int link(char *old, char *new)
// {
// 	return -1;
// }

__attribute__((used)) int _close(int file) {
	PANIC();
	return -1;
}

__attribute__((used)) int _fstat(int file, struct stat* st) {
	st->st_mode = S_IFCHR;
	return 0;
}

__attribute__((used)) int _isatty(int file) {
	return 1;
}

__attribute__((used)) int _lseek(int file, int ptr, int dir) {
	PANIC();
	return 0;
}

__attribute__((used)) int _read(int file, char* ptr, int len) {
	PANIC();
	return 0;
}

/**
 * Wrapper for printf() function native using via
 * selected interface in debug.c
 */
// int _write(int fd, char* ptr, int len)
// {
// // 	Debug_TransmitBuff(ptr, len);
// 	return len;
// }
#include "platform.h"
int _write(int fd, char* ptr, int len) {
	Pl_USART_Debug_TxData((u8*)ptr, (u16)len);
	return len;
}

// __attribute__ ((used))
// void abort(void)
// {
// 	PANIC();
// 	/* Abort called */
// 	while(1);
// }

int _getpid(void) {
	PANIC();
	return 0;
}

int _kill(int pid, int sig) {
	PANIC();
	return -1;
}
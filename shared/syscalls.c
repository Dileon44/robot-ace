/**
 * @file stdio.c
 * @brief Implementation of newlib syscall
 */

#include "main.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#undef errno
extern int errno;
extern int _end;

__attribute__((used)) caddr_t _sbrk(int incr) {
	static unsigned char* heap = NULL;
	unsigned char* prev_heap;

	if (heap == NULL)
		heap = (unsigned char*)&_end;

	// Guard: heap must not grow into the stack
	if ((heap + incr) > (unsigned char*)__get_MSP()) {
		errno = ENOMEM;
		return (caddr_t)-1;
	}

	prev_heap = heap;
	heap += incr;
	return (caddr_t)prev_heap;
}

__attribute__((used)) int link(char* old, char* new) {
	PANIC();
	return -1;
}

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
 * Fallback for printf(): overridden by lib/log/log_io.c when the log module is
 * enabled, so without a log transport stdout output is quietly dropped.
 */
__WEAK int _write(int fd, char* ptr, int len) {
	return len;
}

int _getpid(void) {
	PANIC();
	return 0;
}

int _kill(int pid, int sig) {
	PANIC();
	return -1;
}
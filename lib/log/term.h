/**
 * @file term.h
 * @brief Terminal primitives: ANSI/VT100 escape sequences for styled output +
 *        terminal input key symbols.
 *
 * Console/terminal primitives used across the project (log header, shell,
 * drivers) completely independent of any logging. Header-only, no dependencies.
 */

#ifndef __TERM_H
#define __TERM_H

// clang-format off

/* -- Input key symbols ------------------------------------------------------ */

#define TERM_SYM_BACKSPACE			'\b'
#define TERM_SYM_DELETE				0x7f
#define TERM_SYM_TAB				'\t'
#define TERM_SYM_EXIT				0x03 //ctrl + c

#define TERM_END_LINE				"\r\n"

/* -- Cursor / screen -------------------------------------------------------- */

#define TERM_SAVE_CURSOR			"\e7"
#define TERM_RESTORE_CURSOR			"\e8"
#define TERM_CLEAR_RIGHT_FROM_CURS	"\e[0K"
#define TERM_RIGHT_ARROW			"\e[C"
#define TERM_LEFT_ARROW				"\e[D"
#define TERM_CLR_SCREEN				"\e[1;1H\e[2J"

/* -- Arrow decode helpers --------------------------------------------------- */

#define TERM_PRE_ARROW_SYM			'['
#define TERM_ARROW_UP_SYM			'A'
#define TERM_ARROW_DOWN_SYM			'B'
#define TERM_ARROW_RIGHT_SYM		'C'
#define TERM_ARROW_LEFT_SYM			'D'

/* -- Colors ----------------------------------------------------------------- */

#define TERM_COLOR_RED				"\e[31m"
#define TERM_COLOR_GREEN			"\e[32m"
#define TERM_COLOR_YELLOW			"\e[33m"
#define TERM_COLOR_BLUE				"\e[34m"
#define TERM_COLOR_MAGENTA			"\e[35m"
#define TERM_COLOR_CYAN				"\e[36m"
#define TERM_COLOR_WHITE			"\e[37m"
#define TERM_RESET_STYLE			"\e[0m"

/* -- Text modes ------------------------------------------------------------- */

#define TERM_SET_MODE_BOLD			"\e[1m"
#define TERM_RESET_MODE_BOLD		"\e[22m"
#define TERM_SET_MODE_ITALIC		"\e[3m"
#define TERM_RESET_MODE_ITALIC		"\e[23m"

/* -- Colored-string helpers ------------------------------------------------- */

#define TERM_COLOR_STR(color, str)	color str TERM_RESET_STYLE
#define TERM_RED_STR(str)			TERM_COLOR_RED str TERM_RESET_STYLE
#define TERM_GREEN_STR(str)			TERM_COLOR_GREEN str TERM_RESET_STYLE
#define TERM_YELLOW_STR(str)		TERM_COLOR_YELLOW str TERM_RESET_STYLE
#define TERM_BLUE_STR(str)			TERM_COLOR_BLUE str TERM_RESET_STYLE
#define TERM_MAGENTA_STR(str)		TERM_COLOR_MAGENTA str TERM_RESET_STYLE
#define TERM_CYAN_STR(str)			TERM_COLOR_CYAN str TERM_RESET_STYLE
#define TERM_WHITE_STR(str)			TERM_COLOR_WHITE str TERM_RESET_STYLE

// clang-format on

#endif /* __TERM_H */

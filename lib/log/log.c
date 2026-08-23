#include "log.h"

#if LOG_ENABLE

#define X_ENTRY(lvl, lvl_str, lvl_color, lvl_emoji) \
	{                                               \
		.Str   = lvl_str,                           \
		.Color = lvl_color,                         \
		.Emoji = lvl_emoji,                         \
	},
static const LogLvl_t LogLvl[] = {LOG_LVL_TABLE()};
#undef X_ENTRY

#define LOG_LVL_TABLE_SIZE NUM_ELEMENTS(LogLvl)

static LOG_LVL_t CurrLogLvl = LOG_DEF_LVL;

void Log_Lvl_Set(LOG_LVL_t lvl) {
	if ((u32)lvl >= LOG_LVL_TABLE_SIZE)
		return;

	CurrLogLvl = lvl;
}

LOG_LVL_t Log_Lvl_Get(void) {
	return CurrLogLvl;
}

char Log_Lvl_GetChar(LOG_LVL_t lvl) {
	return Log_Lvl_GetStr(lvl)[0];
}

const char* Log_Lvl_GetStr(LOG_LVL_t lvl) {
	if ((u32)lvl >= LOG_LVL_TABLE_SIZE)
		return "  ?????";

	return LogLvl[lvl].Str;
}

const char* Log_Lvl_GetColor(LOG_LVL_t lvl) {
	if ((u32)lvl >= LOG_LVL_TABLE_SIZE)
		return TERM_RESET_STYLE;

	return LogLvl[lvl].Color;
}

const char* Log_Lvl_GetEmoji(LOG_LVL_t lvl) {
	if ((u32)lvl >= LOG_LVL_TABLE_SIZE)
		return "";

	return LogLvl[lvl].Emoji;
}

/*
 * Both separators are handled so a path produced on either host style resolves
 * to the bare file name.
 */
const char* Log_CutFilePath(const char* pPath) {
	if (!pPath)
		return "";

	const char* pName = pPath;

	for (const char* pCh = pPath; *pCh; pCh++) {
		if (*pCh == '/' || *pCh == '\\')
			pName = pCh + 1;
	}

	return pName;
}

#else /* LOG_ENABLE */

/* Module disabled: keep the log-level API linkable as no-ops. */
void Log_Lvl_Set(LOG_LVL_t lvl) {
	DISCARD_UNUSED(lvl);
}

LOG_LVL_t Log_Lvl_Get(void) {
	return LOG_LVL_DISABLE;
}

char Log_Lvl_GetChar(LOG_LVL_t lvl) {
	DISCARD_UNUSED(lvl);
	return ' ';
}

const char* Log_Lvl_GetStr(LOG_LVL_t lvl) {
	DISCARD_UNUSED(lvl);
	return "";
}

const char* Log_Lvl_GetColor(LOG_LVL_t lvl) {
	DISCARD_UNUSED(lvl);
	return "";
}

const char* Log_Lvl_GetEmoji(LOG_LVL_t lvl) {
	DISCARD_UNUSED(lvl);
	return "";
}

const char* Log_CutFilePath(const char* pPath) {
	return pPath ? pPath : "";
}

#endif /* LOG_ENABLE */

// Two languages on one panel: Chinese (the original) and English.
//
// T() takes the Chinese text as the key, so the page builders read the same as
// before and every string has exactly one English counterpart in i18n.cpp.
// Everything user-visible on the panel or on the WiFi setup page goes through
// T(); printf format specifiers are preserved by the table, so
// snprintf(buf, n, T("%d 天额度"), num) keeps working in both languages.
#pragma once
#include <stdint.h>

#define LANG_ZH 0
#define LANG_EN 1

uint8_t lang_get(void);
void lang_set(uint8_t lang);

// Look up the English text for a Chinese source string. Pass a string literal
// (that is what the table is keyed by). Read-only and re-entrant: it may be
// called from the UI task and from the fetch task at the same time.
// Missing entries return the Chinese, so a half-translated build is readable
// rather than blank.
const char *T(const char *zh);

// One-off pair for the rare case where the same Chinese word needs two
// different English words in two places (the table is keyed by the Chinese).
static inline const char *L(const char *zh, const char *en) {
    return lang_get() == LANG_EN ? en : zh;
}

// English catchphrase for an agent id (nullptr in Chinese mode / unknown id).
const char *say_line(const char *id, int idx);

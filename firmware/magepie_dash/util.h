// Helpers for formatting numbers/times the way the design does.
#pragma once
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// 1090000000 -> "1.09B", 634000 -> "634K", 1520 -> "1.5K"
static inline const char *fmt_count(uint64_t v, char *buf, size_t n) {
    if (v >= 1000000000ULL)
        snprintf(buf, n, "%.2fB", v / 1e9);
    else if (v >= 10000000ULL)
        snprintf(buf, n, "%.0fM", v / 1e6);
    else if (v >= 1000000ULL)
        snprintf(buf, n, "%.1fM", v / 1e6);
    else if (v >= 1000ULL)
        snprintf(buf, n, "%.0fK", v / 1e3);
    else
        snprintf(buf, n, "%llu", (unsigned long long)v);
    return buf;
}

// 57800 -> "57.8k"
static inline const char *fmt_k(uint64_t v, char *buf, size_t n) {
    if (v >= 1000000ULL)
        snprintf(buf, n, "%.2fM", v / 1e6);
    else if (v >= 1000ULL)
        snprintf(buf, n, "%.1fk", v / 1e3);
    else
        snprintf(buf, n, "%llu", (unsigned long long)v);
    return buf;
}

// 4800 ms -> "4.8 秒"
static inline const char *fmt_secs(uint32_t ms, char *buf, size_t n) {
    if (ms >= 60000)
        snprintf(buf, n, "%u分%u秒", (unsigned)(ms / 60000), (unsigned)((ms / 1000) % 60));
    else if (ms >= 1000)
        snprintf(buf, n, "%.1f 秒", ms / 1000.0);
    else
        snprintf(buf, n, "%ums", (unsigned)ms);
    return buf;
}

// ISO timestamp -> local epoch seconds (the gateway offset is ignored; good
// enough for a countdown).
static inline time_t iso_to_epoch(const char *iso) {
    if (!iso || strlen(iso) < 19) return 0;
    struct tm t = {};
    if (sscanf(iso, "%d-%d-%dT%d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min,
               &t.tm_sec) != 6)
        return 0;
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    t.tm_isdst = -1;
    return mktime(&t);
}

// "3 天 21 小时后重置"
static inline void fmt_countdown(time_t when, char *buf, size_t n) {
    buf[0] = 0;
    time_t now = time(nullptr);
    if (!when || now < 1700000000) return;
    long d = when - now;
    if (d <= 0) {
        snprintf(buf, n, "将重置");
        return;
    }
    long days = d / 86400, hours = (d % 86400) / 3600;
    if (days > 0)
        snprintf(buf, n, "%ld 天 %ld 小时后重置", days, hours);
    else if (hours > 0)
        snprintf(buf, n, "%ld 小时后重置", hours);
    else
        snprintf(buf, n, "%ld 分钟后重置", (d % 3600) / 60);
}

// "2026-10-19T18:59:00-07:00" -> "10月19日 18:59" (Chinese) or "10/19 18:59"
static inline void fmt_reset(const char *iso, char *buf, size_t n) {
    buf[0] = 0;
    if (!iso || strlen(iso) < 16) return;
    int mon = 0, day = 0, hh = 0, mm = 0;
    const char *p = strchr(iso, '-');
    if (!p) return;
    // second dash is in the date: YYYY-MM-DD
    const char *p2 = strchr(p + 1, '-');
    if (!p2) return;
    mon = atoi(p + 1);
    day = atoi(p2 + 1);
    const char *t = strchr(iso, 'T');
    if (t) {
        hh = atoi(t + 1);
        const char *c = strchr(t, ':');
        if (c) mm = atoi(c + 1);
    }
    snprintf(buf, n, "%d月%d日 %02d:%02d", mon, day, hh, mm);
}

// br***@gmail.com — keeps the design's masked style.
static inline void mask_account(const char *in, char *out, size_t n) {
    out[0] = 0;
    if (!in || !*in) return;
    const char *at = strchr(in, '@');
    if (!at) {
        strncpy(out, in, n - 1);
        out[n - 1] = 0;
        return;
    }
    size_t keep = (at - in) >= 2 ? 2 : 1;
    snprintf(out, n, "%.*s***%s", (int)keep, in, at);
}

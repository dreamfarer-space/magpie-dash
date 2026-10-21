/**
 * Minimal lv_conf.h for the magpie dash firmware (LVGL 8.3).
 * Everything not listed here falls back to lv_conf_internal.h defaults.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* colour depth: 16 bit RGB565, no byte swap (NV3041A QSPI wants native order) */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/* The object tree of nine pages does not fit in internal RAM (96 KB overflowed
   while building the usage page), so LVGL's heap lives in PSRAM. Keep the number
   of objects low: every extra object is a PSRAM allocation AND a PSRAM access
   on every redraw. (Tried internal-SRAM-first instead: a full page still took
   147 ms vs 157 ms, and it starved the Wi-Fi driver of internal buffers, so the
   tree stays in PSRAM.) */
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_INCLUDE <esp_heap_caps.h>
#define LV_MEM_CUSTOM_ALLOC(n) heap_caps_malloc((n), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define LV_MEM_CUSTOM_FREE(p) heap_caps_free(p)
#define LV_MEM_CUSTOM_REALLOC(p, n) heap_caps_realloc((p), (n), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define LV_MEM_ADR 0

/* tick from Arduino millis() */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

/* logging (serial) */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

#define LV_DPI_DEF 100

/* Latency, not throughput: the defaults (30 ms) mean a tap can wait 30 ms
   before LVGL even reads the panel and another 30 ms before the repaint timer
   fires. 15 ms halves both waits; idle cost stays negligible because nothing
   is invalidated when the screen is still. */
#define LV_DISP_DEF_REFR_PERIOD 15
#define LV_INDEV_DEF_READ_PERIOD 15

/* fonts: keep Montserrat 14 for the theme, our pixel fonts come from
   fonts/ark*.c and are referenced explicitly. */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_CUSTOM_DECLARE \
    LV_FONT_DECLARE(ark10)      \
    LV_FONT_DECLARE(ark12)      \
    LV_FONT_DECLARE(ark24)

/* theme */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1

/* screenshot support (used by the serial "shot" command) */
#define LV_USE_SNAPSHOT 1

/* widgets we use */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_CHART 1
#define LV_USE_LABEL 1
#define LV_USE_IMG 1
#define LV_USE_LINE 1
#define LV_USE_BTN 1

/* demos off */
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_BENCHMARK 0
#define LV_USE_DEMO_STRESS 0
#define LV_USE_DEMO_MUSIC 0

/* assert */
#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0

#endif /*LV_CONF_H*/

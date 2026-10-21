#include "touch_calibration.h"
#include <Arduino.h>
#include "board.h"
#include "display_port.h"

// Only this overlay is touched. Normal dashboard positions/styles are untouched.
static lv_obj_t *s_prompt = nullptr, *s_live = nullptr;
static lv_obj_t *s_targets[5] = {}, *s_rows[5] = {};
static const int16_t tx[5] = {14, LCD_W - 14, 14, LCD_W - 14, LCD_W / 2};
static const int16_t ty[5] = {14, 14, LCD_H - 14, LCD_H - 14, LCD_H / 2};
static const char *names[5] = {"1 TL", "2 TR", "3 BL", "4 BR", "5 CENTER"};
static float rx[5], ry[5];
static int s_step = 0;
static uint32_t s_count = 0, s_start_ms = 0, s_last_ms = 0, s_live_ms = 0;
static int32_t s_sum_x = 0, s_sum_y = 0;
static int16_t s_min_x, s_max_x, s_min_y, s_max_y;
static bool s_wait_release = true; // never use a finger already down when CAL opens

static lv_obj_t *cal_label(lv_obj_t *p, int x, int y, const char *text) {
    lv_obj_t *l = lv_label_create(p);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(0xEEEEF6), 0);
    lv_obj_set_pos(l, x, y);
    lv_label_set_text(l, text);
    return l;
}

static void reset_sample() {
    s_count = 0;
    s_sum_x = s_sum_y = 0;
}

static void show_step() {
    for (int i = 0; i < 5; ++i)
        lv_obj_set_style_border_color(s_targets[i], lv_color_hex(i == s_step ? 0x39DE83 : 0x7E7C89), 0);
    if (s_step < 5)
        lv_label_set_text_fmt(s_prompt, "%s (%d,%d): hold still, then lift", names[s_step], tx[s_step], ty[s_step]);
}

static void finish_fit() {
    const auto xf = touch_fit_axis(rx, tx);
    const auto yf = touch_fit_axis(ry, ty);
    if (!xf.valid || !yf.valid) {
        lv_label_set_text(s_prompt, "FAIL: insufficient axis span. Send x, then c.");
        Serial.println("[cal] FAIL: degenerate raw span; old transform retained");
        return;
    }
    TouchTransform candidate = {xf.a, xf.b, yf.a, yf.b};
    float worst_x = 0, worst_y = 0;
    for (int i = 0; i < 5; ++i) {
        float ex = touch_abs(touch_axis(rx[i], xf.a, xf.b) - tx[i]);
        float ey = touch_abs(touch_axis(ry[i], yf.a, yf.b) - ty[i]);
        if (ex > worst_x) worst_x = ex;
        if (ey > worst_y) worst_y = ey;
        Serial.printf("[cal] check %s error=(%.2f,%.2f)px%s\n", names[i], ex, ey,
                      i == 4 ? " HOLDOUT (not fitted)" : "");
    }
    if (worst_x > TOUCH_CAL_MAX_ERROR_PX || worst_y > TOUCH_CAL_MAX_ERROR_PX ||
        !touch_apply_transform(candidate)) {
        lv_label_set_text_fmt(s_prompt, "FAIL: residual %.1f/%.1fpx. x, then c to retry.", worst_x, worst_y);
        Serial.println("[cal] FAIL: inconsistent samples/axis map; old transform retained");
        return;
    }
    lv_label_set_text(s_prompt, "PASS: map applied in RAM. Send x to test tabs.");
    Serial.printf("[cal] PASS max_error=(%.2f,%.2f)px; applied in RAM ONLY\n", worst_x, worst_y);
    Serial.println("[cal] Copy these into board.h and rebuild for persistence:");
    Serial.printf("#define TOUCH_X_A %.8ff\n#define TOUCH_X_B %.8ff\n", xf.a, xf.b);
    Serial.printf("#define TOUCH_Y_A %.8ff\n#define TOUCH_Y_B %.8ff\n", yf.a, yf.b);
    Serial.println("[cal] No LCD offset inferred. A shifted/clipped frame requires display-layer repair first.");
}

static void observe_touch(TouchFrame event, int16_t raw_x, int16_t raw_y,
                          int32_t mapped_x, int32_t mapped_y) {
    if (!s_prompt) return;
    const uint32_t now = millis();
    if (event == TouchFrame::Pressed) {
        if (now - s_live_ms >= 150) {
            lv_label_set_text_fmt(s_live, "raw=(%d,%d)  map=(%ld,%ld)", raw_x, raw_y,
                                  (long)mapped_x, (long)mapped_y);
            s_live_ms = now;
        }
        if (s_step >= 5 || s_wait_release) return;
        if (!s_count) {
            s_start_ms = now;
            s_min_x = s_max_x = raw_x;
            s_min_y = s_max_y = raw_y;
        }
        s_last_ms = now;
        if (raw_x < s_min_x) s_min_x = raw_x;
        if (raw_x > s_max_x) s_max_x = raw_x;
        if (raw_y < s_min_y) s_min_y = raw_y;
        if (raw_y > s_max_y) s_max_y = raw_y;
        if (s_count < 4096) { s_sum_x += raw_x; s_sum_y += raw_y; ++s_count; }
        return;
    }
    if (event == TouchFrame::Cancelled) {
        if (s_count) {
            Serial.println("[cal] sample cancelled (multitouch/stale). Lift and retry SAME target.");
            reset_sample();
        }
        // An idle no-frame timeout also reaches here; do not block the next tap.
        return;
    }
    if (event != TouchFrame::Released) return;
    s_wait_release = false;
    if (!s_count || s_step >= 5) return;
    if (s_count < 3 || s_last_ms - s_start_ms < TOUCH_CAL_MIN_HOLD_MS ||
        s_max_x - s_min_x > TOUCH_CAL_MAX_SPREAD || s_max_y - s_min_y > TOUCH_CAL_MAX_SPREAD) {
        Serial.printf("[cal] retry %s: n=%lu hold=%lu ms spread=(%d,%d)\n", names[s_step],
                      (unsigned long)s_count, (unsigned long)(s_last_ms - s_start_ms),
                      s_max_x - s_min_x, s_max_y - s_min_y);
        lv_label_set_text(s_live, "Too short/moving: hold still >=120ms and lift.");
        reset_sample();
        return;
    }
    rx[s_step] = float(s_sum_x) / s_count;
    ry[s_step] = float(s_sum_y) / s_count;
    Serial.printf("[cal] %s screen=(%d,%d) raw=(%.2f,%.2f) samples=%lu\n", names[s_step],
                  tx[s_step], ty[s_step], rx[s_step], ry[s_step], (unsigned long)s_count);
    lv_label_set_text_fmt(s_rows[s_step], "%s: raw %.1f,%.1f -> %d,%d", names[s_step],
                          rx[s_step], ry[s_step], tx[s_step], ty[s_step]);
    ++s_step;
    reset_sample();
    show_step();
    if (s_step == 5) finish_fit();
}

void touch_calibration_begin(lv_obj_t *overlay) {
    s_step = 0;
    reset_sample();
    // Waiting for an up frame prevents CAL being entered mid-gesture. If the
    // controller sends no idle release frame, an explicit first tap is ignored.
    s_wait_release = touch_pressed();
    s_prompt = cal_label(overlay, 26, 54, "");
    s_live = cal_label(overlay, 26, 78, "Check all 4 white borders BEFORE tapping.");
    for (int i = 0; i < 5; ++i) {
        s_targets[i] = lv_obj_create(overlay);
        lv_obj_remove_style_all(s_targets[i]);
        lv_obj_set_pos(s_targets[i], tx[i] - 9, ty[i] - 9);
        lv_obj_set_size(s_targets[i], 19, 19);
        lv_obj_clear_flag(s_targets[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(s_targets[i], 2, 0);
        // One-pixel crosses mark the EXACT target centers, not a box corner.
        for (int axis = 0; axis < 2; ++axis) {
            lv_obj_t *line = lv_obj_create(overlay);
            lv_obj_remove_style_all(line);
            lv_obj_set_pos(line, tx[i] - (axis == 0 ? 4 : 0), ty[i] - (axis == 1 ? 4 : 0));
            lv_obj_set_size(line, axis == 0 ? 9 : 1, axis == 1 ? 9 : 1);
            lv_obj_set_style_bg_color(line, lv_color_hex(0x39DE83), 0);
            lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
            lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        }
        s_rows[i] = cal_label(overlay, 80, 164 + 17 * i, "");
    }
    show_step();
    touch_set_observer(observe_touch);
    Serial.println("[cal] First check physical frame: all four white borders at LCD edges, no clipping/wrap.");
    Serial.println("[cal] If frame is shifted STOP: do not fit touch to hide a display-window error.");
    Serial.println("[cal] Hold/lift: 1 TL, 2 TR, 3 BL, 4 BR, 5 CENTER. Use GREEN cross centers.");
    Serial.println("[cal] Do NOT send t during calibration; it consumes frames without updating CAL.");
}

void touch_calibration_end() {
    touch_set_observer(nullptr);
    s_prompt = s_live = nullptr;
    reset_sample();
}

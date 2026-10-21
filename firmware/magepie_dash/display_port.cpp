#include "display_port.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <math.h>

#include "board.h"
#include <Preferences.h>
#include "ui.h"

static Arduino_DataBus *bus =
    new Arduino_ESP32QSPI(LCD_CS, LCD_SCK, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
// Offsets belong to the controller RAM window, not to LVGL layouts.
static Arduino_GFX *panel = new Arduino_NV3041A(
    bus, GFX_NOT_DEFINED, 0 /*rotation*/, true /*IPS*/, LCD_W, LCD_H,
    LCD_COL_OFFSET, LCD_ROW_OFFSET, LCD_COL_OFFSET, LCD_ROW_OFFSET);

// One full-frame draw buffer in PSRAM (480*272*2 = 261 KB). With 20-row bands
// LVGL walked the object tree 14 times per repaint; a full-height buffer means a
// single pass, and flush sends the screen in one QSPI burst instead of 14.
static lv_color_t *draw_buf1 = nullptr;
static lv_color_t *draw_buf2 = nullptr;
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

static uint8_t gt_addr = 0x5D;
static bool gt_ok = false;
bool dbg_verbose = false;
static bool finger_down = false;
static uint16_t *s_capture = nullptr;
static uint32_t s_touch_started_ms = 0;
static TouchTransform s_transform = {TOUCH_X_A, TOUCH_X_B, TOUCH_Y_A, TOUCH_Y_B};
static TouchObserver s_observer = nullptr;

void touch_set_observer(TouchObserver observer) { s_observer = observer; }

// ---- quick-calibration plumbing -------------------------------------------
static bool s_cap_armed = false, s_cap_ready = false;
static int16_t s_cap_x = 0, s_cap_y = 0;

void touch_capture_next() {
    s_cap_ready = false;
    s_cap_armed = true;
}

bool touch_capture_take(int16_t *x, int16_t *y) {
    if (!s_cap_ready) return false;
    *x = s_cap_x;
    *y = s_cap_y;
    s_cap_ready = false;
    return true;
}

static bool s_map_stored = false;
bool touch_map_present() { return s_map_stored; }

bool touch_transform_save() {
    Preferences p;
    if (!p.begin("touchmap", false)) return false;
    p.putFloat("ax", s_transform.ax);
    p.putFloat("bx", s_transform.bx);
    p.putFloat("ay", s_transform.ay);
    p.putFloat("by", s_transform.by);
    p.putBool("ok", true);
    p.end();
    s_map_stored = true;
    return true;
}

static void touch_transform_load() {
    Preferences p;
    if (p.begin("touchmap", false)) {
        p.clear();  // Clear any corrupted calibration from earlier sessions
        p.end();
    }
    s_transform = {TOUCH_X_A, TOUCH_X_B, TOUCH_Y_A, TOUCH_Y_B};
    s_map_stored = true;
    Serial.printf("[touch] active map 1:1 direct: ax=%.4f bx=%.2f ay=%.4f by=%.2f\n",
                  s_transform.ax, s_transform.bx, s_transform.ay, s_transform.by);
}

bool touch_map_reset_to_default() {
    Preferences p;
    if (p.begin("touchmap", false)) {
        p.clear();
        p.end();
    }
    TouchTransform t{TOUCH_X_A, TOUCH_X_B, TOUCH_Y_A, TOUCH_Y_B};
    const bool ok = touch_apply_transform(t);
    s_map_stored = false;
    Serial.printf("[touch] map reset: ax=%.3f bx=%.1f ay=%.3f by=%.1f (stored map cleared)\n", t.ax,
                  (double)t.bx, (double)t.ay, (double)t.by);
    return ok;
}

void touch_map_point(int16_t raw_x, int16_t raw_y, int32_t *x, int32_t *y) {
    *x = touch_round(touch_axis(raw_x, s_transform.ax, s_transform.bx));
    *y = touch_round(touch_axis(raw_y, s_transform.ay, s_transform.by));
}
bool touch_apply_transform(const TouchTransform &t) {
    if (!isfinite(t.ax) || !isfinite(t.bx) || !isfinite(t.ay) || !isfinite(t.by) ||
        touch_abs(t.ax) < 0.05f || touch_abs(t.ax) > 10.0f ||
        touch_abs(t.ay) < 0.05f || touch_abs(t.ay) > 10.0f ||
        touch_abs(t.bx) > 65535.0f || touch_abs(t.by) > 65535.0f) return false;
    s_transform = t; // RAM only; board.h remains the boot-time source of truth.
    return true;
}

// ------------------------------------------------------------------ GT911
static bool gt_write(uint16_t reg, uint8_t val) {
    Wire.beginTransmission(gt_addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

static bool gt_read(uint16_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(gt_addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(gt_addr, len) != len) return false;
    for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
    return true;
}

static bool gt_probe(uint8_t addr) {
    gt_addr = addr;
    uint8_t id[4] = {0};
    if (!gt_read(0x8140, id, 4)) return false;
    // "911" + variant char
    return id[0] == '9' && id[1] == '1' && id[2] == '1';
}

static bool gt911_begin() {
    pinMode(TOUCH_RES, OUTPUT);
    pinMode(TOUCH_INT, OUTPUT);

    // Address select: INT low during reset -> 0x5D, INT high -> 0x14.
    for (uint8_t attempt = 0; attempt < 2; attempt++) {
        // The previous attempt released INT to INPUT; re-enable each time.
        pinMode(TOUCH_INT, OUTPUT);
        digitalWrite(TOUCH_RES, LOW);
        digitalWrite(TOUCH_INT, attempt == 0 ? LOW : HIGH);
        delay(12);
        digitalWrite(TOUCH_RES, HIGH);
        delay(6);
        digitalWrite(TOUCH_INT, LOW); // INT sync after address selection
        delay(50);
        pinMode(TOUCH_INT, INPUT);
        if (gt_probe(attempt == 0 ? 0x5D : 0x14)) {
            if (!gt_write(0x8040, 0) || !gt_write(0x814E, 0)) return false;
            // Read ONLY: do not rewrite controller config/checksum blindly.
            uint8_t cfg[7] = {0};
            if (gt_read(0x8047, cfg, sizeof(cfg))) {
                uint16_t xmax = cfg[1] | (uint16_t(cfg[2]) << 8);
                uint16_t ymax = cfg[3] | (uint16_t(cfg[4]) << 8);
                Serial.printf("[touch] config ver=0x%02X x_max=%u y_max=%u switch1=0x%02X\n",
                              cfg[0], xmax, ymax, cfg[6]);
                if (xmax != LCD_W || ymax != LCD_H)
                    Serial.println("[touch] range differs from LCD; use CAL, not raw<LCD bounds");
            } else Serial.println("[touch] config read failed; range UNKNOWN");
            return true;
        }
    }
    return false;
}

static uint8_t s_last_npts = 0;
static uint16_t s_last_size = 0;

static TouchFrame gt_read_frame(int16_t *x, int16_t *y) {
    uint8_t st = 0;
    if (!gt_read(0x814E, &st, 1)) return TouchFrame::Error;
    if (!(st & 0x80)) return TouchFrame::NoData; // NOT an up event!
    uint8_t npts = st & 0x0F;
    TouchFrame result = npts == 0 ? TouchFrame::Released : TouchFrame::Cancelled;
    if (npts >= 1 && npts <= 5) {
        uint8_t p[6] = {0};
        s_last_npts = npts;
        // 0x814F is track ID; X starts at 0x8150, little endian.
        if (!gt_read(0x8150, p, sizeof(p))) return TouchFrame::Error;
        *x = (int16_t)((uint16_t(p[1]) << 8) | p[0]);
        *y = (int16_t)((uint16_t(p[3]) << 8) | p[2]);
        s_last_size = (uint16_t(p[5]) << 8) | p[4];
        result = TouchFrame::Pressed;
    }
    if (!gt_write(0x814E, 0)) return TouchFrame::Error;
    return result;
}

// I2C is now touched by two tasks (the UI task and the flush task, the latter to
// keep the panel from going deaf during a repaint). Arduino's Wire API is not
// thread safe, so every GT911 transaction goes through this lock.
static SemaphoreHandle_t s_i2c_mtx = nullptr;

struct TouchLock {
    bool held;
    TouchLock() : held(false) {
        if (s_i2c_mtx) held = xSemaphoreTake(s_i2c_mtx, pdMS_TO_TICKS(25)) == pdTRUE;
    }
    ~TouchLock() {
        if (held) xSemaphoreGive(s_i2c_mtx);
    }
};

static void touch_poll(bool allow_ui = true);  // defined with the touch code below

// ------------------------------------------------------------------ LVGL
// ---- perf counters (serial 'q'): how much of a frame is pixel transport -----
static uint32_t s_flush_n = 0, s_flush_us = 0, s_flush_us_max = 0;

void display_perf_reset() { s_flush_n = 0; s_flush_us = 0; s_flush_us_max = 0; }

void display_perf(uint32_t *frames, uint32_t *us_total, uint32_t *us_max) {
    *frames = s_flush_n;
    *us_total = s_flush_us;
    *us_max = s_flush_us_max;
}

// ---- asynchronous flush -----------------------------------------------------
// LVGL gets two buffers, so it can render band N+1 while a task streams band N
// to the panel. Before this, a page repaint was render + transport (~50-65 ms);
// overlapped it is ~max(render, transport) plus the last band.
//
// The task does the blocking Arduino_GFX transfer (DMA under the hood) and only
// then releases the buffer with lv_disp_flush_ready(), which LVGL documents as
// safe to call from an interrupt/task context.
struct FlushReq {
    lv_area_t a;
    lv_color_t *px;
    lv_disp_drv_t *drv;
};

static QueueHandle_t s_flush_q = nullptr;

static void flush_task(void *arg) {
    (void)arg;
    for (;;) {
        FlushReq r;
        if (xQueueReceive(s_flush_q, &r, portMAX_DELAY) != pdTRUE) continue;
        const uint32_t t0 = micros();
        panel->draw16bitRGBBitmap(r.a.x1, r.a.y1, (uint16_t *)r.px,
                                  (int16_t)(r.a.x2 - r.a.x1 + 1),
                                  (int16_t)(r.a.y2 - r.a.y1 + 1));
        const uint32_t dt = micros() - t0;
        s_flush_n++;
        s_flush_us += dt;
        if (dt > s_flush_us_max) s_flush_us_max = dt;
        // Never touch I2C from the flush task: Wire is not thread-safe and
        // calling touch_poll here deadlocks Core 0 and Core 1.
        lv_disp_flush_ready(r.drv);
    }
}

static void flush_cb(lv_disp_drv_t *d, const lv_area_t *a, lv_color_t *px) {
    uint32_t w = (a->x2 - a->x1) + 1;
    uint32_t h = (a->y2 - a->y1) + 1;
    if (s_capture) {
        // screenshot mode: mirror into a full-frame buffer instead of the panel
        const uint32_t t0 = micros();
        for (uint32_t y = 0; y < h; y++) {
            memcpy(&s_capture[(a->y1 + y) * LCD_W + a->x1], px + y * w, w * sizeof(uint16_t));
        }
        const uint32_t dt = micros() - t0;
        s_flush_n++;
        s_flush_us += dt;
        if (dt > s_flush_us_max) s_flush_us_max = dt;
        lv_disp_flush_ready(d);
        return;
    }
    FlushReq r;
    r.a = *a;
    r.px = px;
    r.drv = d;
    // Queue depth == number of buffers: this only blocks if both buffers are
    // still in flight, i.e. the panel is the bottleneck anyway.
    xQueueSend(s_flush_q, &r, portMAX_DELAY);
}

void display_capture(uint16_t *buf) { s_capture = buf; }

// ---- touch: hardware poll is decoupled from LVGL's read callback -----------
// A repaint blocks the loop for 40-60 ms. Polling the GT911 only from the LVGL
// read callback meant a short tap that fell inside that window was never seen
// (press and release both missed, or the release missed and the click only fired
// on the 300 ms stale timeout) — reported as "taps do nothing, then it jumps".
// So poll the controller from display_tick() and latch the edges; LVGL consumes
// them one per read, in order, however late it gets to run.
static int16_t s_pt_x = LCD_W / 2, s_pt_y = LCD_H / 2;  // last mapped point
static bool s_latch_press = false, s_latch_release = false;
static uint32_t s_last_frame_ms = 0, s_last_log_ms = 0, s_last_edge_ms = 0;
static bool s_touch_started = false;

static uint32_t s_poll_ms = 0;

static void touch_poll(bool allow_ui) {
    if (!gt_ok) return;
    if (!s_touch_started) return;
    const uint32_t now = millis();
    // ~80 Hz is plenty: a human tap lasts 50-100 ms and the edges are latched, so
    // the only thing a shorter interval buys is Wi-Fi/CPU time wasted on I2C
    // (250 Hz cost 5-7 % idle, 12 ms costs ~3 %).
    if (now - s_poll_ms < 12) return;
    s_poll_ms = now;
    if (now - s_touch_started_ms < TOUCH_WARMUP_MS) {
        // Drain/ack reset noise instead of leaving a phantom pending.
        int16_t dummy_x = 0, dummy_y = 0;
        gt_read_frame(&dummy_x, &dummy_y);
        finger_down = false;
        return;
    }
    int16_t raw_x = 0, raw_y = 0;
    TouchFrame frame;
    {
        TouchLock lk;
        if (!lk.held) return;  // another task is mid-transaction; retry next poll
        frame = gt_read_frame(&raw_x, &raw_y);
    }
    const bool was_down = finger_down;
    const bool stale = now - s_last_frame_ms >= TOUCH_STALE_MS;
    if (frame == TouchFrame::Pressed && (raw_x < 0 || raw_y < 0 || (raw_x == 0 && raw_y == 0)))
        frame = TouchFrame::Error;
    if (frame == TouchFrame::Pressed) {
        if (s_cap_armed) {
            s_cap_x = raw_x;
            s_cap_y = raw_y;
            s_cap_ready = true;
            s_cap_armed = false;
        }
        int32_t x, y;
        touch_map_point(raw_x, raw_y, &x, &y);  // original raw -> affine exactly once
        s_last_frame_ms = now;
        if (dbg_verbose && (!was_down || now - s_last_log_ms >= 150)) {
            Serial.printf("[touch] %u raw=(%d,%d) map=(%ld,%ld) n=%u sz=%u%s\n", (unsigned)now,
                          raw_x, raw_y, (long)x, (long)y, (unsigned)s_last_npts,
                          (unsigned)s_last_size, was_down ? "" : " down");
            s_last_log_ms = now;
        }
        if (allow_ui) {
            ui_cal_dot_update((int)x, (int)y);
            // Calibration sees raw points BEFORE bounds checking, even if X>479.
            if (s_observer) s_observer(frame, raw_x, raw_y, x, y);
        }
        if (touch_in_margin(x, LCD_W, TOUCH_EDGE_MARGIN_PX) &&
            touch_in_margin(y, LCD_H, TOUCH_EDGE_MARGIN_PX)) {
            s_pt_x = touch_clip(x, LCD_W);
            s_pt_y = touch_clip(y, LCD_H);
            finger_down = touch_next_pressed(was_down, frame, false);
            if (!was_down && finger_down) { s_latch_press = true; s_last_edge_ms = now; }
        } else {
            finger_down = false;  // never clamp arbitrary corrupt/out-of-map points
        }
    } else {
        finger_down = touch_next_pressed(was_down, frame, stale);
        if (allow_ui && s_observer &&
            (frame == TouchFrame::Released || frame == TouchFrame::Cancelled))
            s_observer(frame, 0, 0, s_pt_x, s_pt_y);
        else if (allow_ui && s_observer && stale)
            s_observer(TouchFrame::Cancelled, 0, 0, s_pt_x, s_pt_y);
    }
    if (was_down && !finger_down) {
        s_latch_release = true;
        s_last_edge_ms = now;
        if (dbg_verbose) Serial.printf("[touch] %u up (%s) after %u ms\n", (unsigned)now,
                      frame == TouchFrame::Released ? "frame" :
                      frame == TouchFrame::Cancelled ? "multitouch" :
                      frame == TouchFrame::Pressed ? "outside map" : "stale timeout",
                      (unsigned)(now - s_last_frame_ms));
    }
}

static lv_indev_t *s_pointer = nullptr;
static bool s_test_input = false;
static lv_indev_data_t s_test_data{};

static void touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;
    if (s_test_input) {
        *data = s_test_data;
        return;
    }
    data->point.x = s_pt_x;
    data->point.y = s_pt_y;
    // One edge per read so a press+release that happened during a repaint is
    // still delivered as a complete click instead of being collapsed.
    if (s_latch_press) {
        s_latch_press = false;
        data->state = LV_INDEV_STATE_PRESSED;
        if (s_latch_release) data->continue_reading = true;
    } else if (s_latch_release) {
        s_latch_release = false;
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = finger_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    }
    static lv_indev_state_t last_delivered = LV_INDEV_STATE_RELEASED;
    if (dbg_verbose && data->state != last_delivered) {
        lv_point_t point = data->point;
        lv_obj_t *hit = lv_indev_search_obj(lv_scr_act(), &point);
        lv_area_t bounds{};
        if (hit) lv_obj_get_coords(hit, &bounds);
        Serial.printf("[agenttrace] %u indev=%s xy=%d,%d hit=%p box=%d,%d..%d,%d page=%d\n",
                      (unsigned)millis(), data->state == LV_INDEV_STATE_PRESSED ? "down" : "up",
                      point.x, point.y, hit, bounds.x1, bounds.y1, bounds.x2, bounds.y2, ui_cur_page());
    }
    last_delivered = data->state;
}

bool touch_debug_read(int16_t *x, int16_t *y) {
    TouchLock lk;
    if (!lk.held) return false;
    return gt_read_frame(x, y) == TouchFrame::Pressed;
}

bool display_begin() {
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    // 40 MHz, not the datasheet-max 32 MHz: the S3 derives SPI clocks from the
    // 80 MHz APB, where 32 MHz request rounds down to a 20 MHz bus. 80/2 = 40 MHz
    // is exact and measured ~2x faster pixel transport. Revert to begin() if the
    // glass ever shows corrupted/offset pixels.
    if (!panel->begin(40000000)) {
        Serial.println("[lcd] NV3041A init failed");
        return false;
    }
    Serial.printf("[lcd] %dx%d rotation=0 window_offset=(%d,%d)\n",
                  panel->width(), panel->height(), LCD_COL_OFFSET, LCD_ROW_OFFSET);
    // Colour order: settled with the 'T' test pattern (TL must be red). With the
    // BGR bit set (0xC8) the glass showed blue there, i.e. red and blue were
    // exchanged; the driver's default MADCTL = MX|MY|RGB (0xC0) is correct here.
    Serial.println("[lcd] colour order = RGB (MADCTL 0xC0, driver default)");
    panel->fillScreen(0x0000);

    Wire.begin(TOUCH_SDA, TOUCH_SCL, 400000);
    gt_ok = gt911_begin();
    s_touch_started_ms = millis();
    s_touch_started = true;
    Serial.printf("[touch] GT911 %s (addr 0x%02X)\n", gt_ok ? "ok" : "not found", gt_addr);
    Serial.printf("[touch] affine X=(%.6f,%.6f) Y=(%.6f,%.6f)\n",
                  s_transform.ax, s_transform.bx, s_transform.ay, s_transform.by);
    touch_transform_load();

    lv_init();
    // Two DMA-capable bands in internal RAM so the flush task can stream one
    // while LVGL renders the other. 72 rows => 4 bands per screen (the tree walk
    // scales with the band count, so this stays cheap) and 2x69 KB.
    const int rows = 72;
    draw_buf1 = (lv_color_t *)heap_caps_malloc(LCD_W * rows * sizeof(lv_color_t),
                                               MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    draw_buf2 = (lv_color_t *)heap_caps_malloc(LCD_W * rows * sizeof(lv_color_t),
                                               MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (draw_buf1 && draw_buf2) {
        lv_disp_draw_buf_init(&draw_buf, draw_buf1, draw_buf2, LCD_W * rows);
        Serial.printf("[lcd] double draw buffer %d rows x2 in internal RAM (async flush)\n", rows);
    } else {
        // No second buffer: fall back to a single full frame in PSRAM. Flush stays
        // asynchronous, it just cannot overlap with rendering as well.
        if (draw_buf2) { heap_caps_free(draw_buf2); draw_buf2 = nullptr; }
        if (draw_buf1) { heap_caps_free(draw_buf1); draw_buf1 = nullptr; }
        draw_buf1 = (lv_color_t *)heap_caps_malloc(LCD_W * LCD_H * sizeof(lv_color_t),
                                                   MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        lv_disp_draw_buf_init(&draw_buf, draw_buf1, nullptr, LCD_W * LCD_H);
        Serial.println("[lcd] single PSRAM draw buffer (no overlap)");
    }
    s_i2c_mtx = xSemaphoreCreateMutex();
    s_flush_q = xQueueCreate(2, sizeof(FlushReq));
    xTaskCreatePinnedToCore(flush_task, "lcdflush", 4096, nullptr, 3, nullptr, 0);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_W;
    disp_drv.ver_res = LCD_H;
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    if (gt_ok) {
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touch_cb;
        s_pointer = lv_indev_drv_register(&indev_drv);
    }
    return true;
}

// Poll the touch controller independently of LVGL: a repaint can block
// lv_timer_handler() for tens of milliseconds and a short tap would be lost.
void display_tick() {
    touch_poll();
    lv_timer_handler();
}

bool display_wait_idle(uint32_t timeout_ms) {
    const uint32_t t0 = millis();
    while (draw_buf.flushing) {
        if (millis() - t0 >= timeout_ms) return false;
        delay(1);
    }
    return true;
}

// Press and release are separate so a benchmark can time "finger down ->
// page on the glass" (tabs switch on press) without the hold in between.
void display_test_press(int x, int y, bool down) {
    if (!s_pointer) { Serial.println("[taptest] no pointer driver"); return; }
    s_test_input = true;
    if (down) {
        s_test_data = {};
        s_test_data.point.x = x;
        s_test_data.point.y = y;
        s_test_data.state = LV_INDEV_STATE_RELEASED;
        lv_indev_reset(s_pointer, nullptr);
        lv_indev_read_timer_cb(s_pointer->driver->read_timer);
        s_test_data.state = LV_INDEV_STATE_PRESSED;
    } else {
        s_test_data.state = LV_INDEV_STATE_RELEASED;
    }
    lv_indev_read_timer_cb(s_pointer->driver->read_timer);
    s_test_input = down;  // while held, the GT911 must not report a release
}

void display_test_tap(int x, int y) {
    display_test_press(x, y, true);
    delay(30);
    display_test_press(x, y, false);
}

bool touch_pressed() { return finger_down; }

bool touch_recent(uint32_t ms) {
    return finger_down || (millis() - s_last_edge_ms) < ms;
}

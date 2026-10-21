// Display + touch port for the JC4827W543 (NV3041A QSPI panel, GT911 touch).
#pragma once
#include <lvgl.h>
#include "touch_math.h"

// Observer receives ORIGINAL GT911 raw coordinates, even if outside the current
// map. Calibration therefore does not depend on correctly hitting an LVGL box.
using TouchObserver = void (*)(TouchFrame event, int16_t raw_x, int16_t raw_y,
                               int32_t mapped_x, int32_t mapped_y);
void touch_set_observer(TouchObserver observer);
void touch_map_point(int16_t raw_x, int16_t raw_y, int32_t *x, int32_t *y);
void touch_capture_next();
bool touch_capture_take(int16_t *x, int16_t *y);
bool touch_transform_save();
// Serial 'K': forget the stored map and fall back to the compiled defaults.
bool touch_map_reset_to_default();
bool touch_map_present();
bool touch_apply_transform(const TouchTransform &transform);

// Initialise panel, touch, LVGL. Returns false if the panel bus failed.
bool display_begin();

// LVGL housekeeping (call from loop()).
void display_tick();
bool display_wait_idle(uint32_t timeout_ms);  // diagnostic: final transfer complete
// Opt-in regression seam: a real LVGL press/release, bypassing only the GT911.
void display_test_tap(int x, int y);
void display_test_press(int x, int y, bool down);  // half a tap
void run_touch_regression();

// True while a finger is on the glass.
bool touch_pressed();

// True while a finger is down, or within `ms` of the last press/release edge.
// Used to keep a data-driven repaint from landing on top of a tap.
bool touch_recent(uint32_t ms);

// Per-event serial logging (taps, page switches, per-request gateway lines).
// Off by default: a USB-CDC write with no host reading can stall the UI thread.
extern bool dbg_verbose;

// Redirect frame flushes into buf (480*272 uint16_t RGB565) instead of the
// panel; pass nullptr to resume normal output. Used by the serial "shot"
// command so the layout can be inspected on a PC.
void display_capture(uint16_t *buf);

// Raw GT911 read for the serial touch diagnostic.
bool touch_debug_read(int16_t *x, int16_t *y);

// Flush statistics (serial 'q'): frame count, total and worst flush duration.
void display_perf(uint32_t *frames, uint32_t *us_total, uint32_t *us_max);
void display_perf_reset();

// Runtime touch mapping: screen = a * raw + b, per axis.
// A 2-point quick calibration fills these in, so mirrors / offsets / scale
// errors on the GT911 are measured instead of guessed. Stored in NVS.
#pragma once

#include <stdint.h>

extern float g_tmap_ax, g_tmap_bx, g_tmap_ay, g_tmap_by;
extern bool  g_tmap_valid;

// Next Pressed frame is captured instead of being forwarded to the mapping.
extern bool    g_cap_armed;
extern bool    g_cap_ready;
extern int16_t g_cap_x, g_cap_y;

void touch_map_point(int16_t raw_x, int16_t raw_y, int32_t *x, int32_t *y);
void touch_map_set(float ax, float bx, float ay, float by, bool persist);
bool touch_map_load();          // false when nothing stored yet
void touch_cap_arm();
bool touch_cap_take(int16_t *x, int16_t *y);   // true once a point arrived

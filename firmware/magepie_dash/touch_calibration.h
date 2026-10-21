#pragma once
#include <lvgl.h>

// Owns only children of the existing CAL overlay. Does not alter any page.
void touch_calibration_begin(lv_obj_t *overlay);
void touch_calibration_end();

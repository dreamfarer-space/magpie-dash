// Ten-page dashboard UI (LVGL 8), terminal style: a prompt bar, outlined panes
// on a monospaced grid, and an htop-like tab bar shared by every page.
#pragma once
#include <lvgl.h>

#include "model.h"

void ui_init();
void ui_update(const DashData &d);  // refresh labels after a fetch
void ui_tick();                     // clock + animations, call every loop
void ui_goto(int page);

// Interface language (see i18n.h): the tap only flags the change, the main loop
// persists it and calls ui_rebuild(), which recreates every page in the new
// language.
void ui_set_language(uint8_t lang);
bool ui_lang_take_dirty();
void ui_rebuild();
void ui_page_settle();  // paint a coalesced page switch, call every loop
void ui_tap_page(int page);  // debug: same path as a tab tap (coalesces)
void ui_next_page();
int ui_cur_page();
void ui_focus_next();          // debug: preview the next agent's avatar
void ui_poke();                // debug: poke the avatar in view
void ui_sheet_benchmark();
void ui_open_sheet(int row);   // debug: open an agent sheet
lv_obj_t *ui_page_obj(int page);
void ui_show_calibration(bool on);
void ui_test_toggle();  // debug: colour + touch-axis test pattern
void ui_cal_dot_update(int x, int y);
void ui_quickcal_begin();
void ui_quickcal_end();
void app_request_refresh();  // debug: root object of a page

// Re-apply persisted preferences to the widgets already built (name, quota
// wording, privacy mask); called after the settings page changes something.
void ui_settings_refresh();

// Serial 'Z': taps the settings rows/keyboard for real and reports PASS/FAIL.
void ui_settings_test();

// Centre of tab i (0=主页 1=Agent .. 5=设置); used by the hit-test regression.
int ui_tab_center_x(int index);
int ui_tab_center_y();

// Debug: step the usage period (今天 -> 7 天 -> 30 天 -> 全部).
void ui_cycle_period();

// Debug: show/hide the connection panel.
void ui_link_panel(bool on);

// Debug: scroll the route detail list (serial G / H).
void ui_routes_scroll(int dy);

// Settings changed on the UI task: the main loop writes NVS after the tap.
bool ui_cfg_take_dirty();

// Debug: show/hide the name keyboard without touching anything else.
void ui_settings_keyboard(bool on);

#define UI_PAGES 10
enum {
    PG_HOME = 0,
    PG_STATE_THINK = 1,
    PG_STATE_CODE = 2,
    PG_STATE_CONFIRM = 3,
    PG_AGENTS = 4,
    PG_QUOTA = 5,
    PG_USAGE = 6,
    PG_ROUTES = 7,
    PG_ROUTES_LIST = 8,
    PG_SETTINGS = 9,
};

// Opt-in on-device regression: inject press/release through the registered
// LVGL input driver, NOT navigation callbacks, so real hit-testing runs.
#include <Arduino.h>
#include "ui.h"
#include "display_port.h"

static void check_tap(const char *name, int from, int x, int y, int want, int &total, int &failed) {
    ui_goto(from);
    lv_obj_update_layout(lv_scr_act());
    lv_point_t point{(lv_coord_t)x, (lv_coord_t)y};
    lv_obj_t *hit = lv_indev_search_obj(lv_scr_act(), &point);
    lv_area_t area{};
    if (hit) lv_obj_get_coords(hit, &area);
    display_test_tap(x, y);
    // Allow the production deferred navigation path to settle, if used.
    for (uint32_t t = millis(); millis() - t < 180;) {
        display_tick();
        ui_page_settle();
        delay(5);
    }
    const int got = ui_cur_page();
    total++;
    if (got != want) failed++;
    Serial.printf("[taptest] %s %s from=%d xy=%d,%d want=%d got=%d hit=%p box=%d,%d..%d,%d clickable=%d bubble=%d\n",
        got == want ? "PASS" : "FAIL", name, from, x, y, want, got, hit,
        area.x1, area.y1, area.x2, area.y2,
        hit ? lv_obj_has_flag(hit, LV_OBJ_FLAG_CLICKABLE) : 0,
        hit ? lv_obj_has_flag(hit, LV_OBJ_FLAG_EVENT_BUBBLE) : 0);
}

void run_touch_regression() {
    int total = 0, failed = 0;
    Serial.println("[taptest] BEGIN input-driver -> LVGL hit-test -> click -> page");
    // home: the right pane (magpie) opens the agent list from anywhere on it,
    // labels and meters included; the left pane (agent) opens its state pages
    check_tap("home-right-background", 0, 450, 150, 4, total, failed);
    check_tap("home-right-big-number", 0, 270, 40, 4, total, failed);
    check_tap("home-right-chart", 0, 350, 210, 4, total, failed);
    check_tap("home-right-top-border", 0, 350, 27, 4, total, failed);
    check_tap("home-left-background", 0, 215, 150, 1, total, failed);
    check_tap("home-left-meter", 0, 100, 136, 1, total, failed);
    check_tap("home-left-stats", 0, 60, 194, 1, total, failed);
    static const char *tname[6] = {"tab-home", "tab-agent", "tab-quota", "tab-usage", "tab-route", "tab-settings"};
    static const int tpage[6] = {0, 4, 5, 6, 7, 9};
    for (int page = 0; page <= 9; page++) {
        for (int i = 0; i < 6; i++)
            if (tpage[i] != page)  // tapping the tab you are already on proves nothing
                check_tap(tname[i], page, ui_tab_center_x(i), ui_tab_center_y(), tpage[i], total, failed);
    }
    // the edge of a tab, not just its centre: the key digit and the bar's last row
    check_tap("tab-edge-key", 4, ui_tab_center_x(2) - 32, ui_tab_center_y(), 5, total, failed);
    check_tap("tab-edge-bottom", 4, ui_tab_center_x(3), 270, 6, total, failed);
    // state page pips and the route log toggle
    check_tap("state-pip-code", 1, 96, 18 + 158, 2, total, failed);
    check_tap("route-expand", 7, 240, 18 + 12, 8, total, failed);
    check_tap("route-collapse", 8, 240, 18 + 12, 7, total, failed);
    ui_goto(0);
    Serial.printf("[taptest] END total=%d failed=%d\n", total, failed);
}

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
    check_tap("home-right-background", 0, 450, 150, 4, total, failed);
    check_tap("home-right-status-dot", 0, 300, 210, 4, total, failed);
    check_tap("home-right-progress", 0, 350, 243, 4, total, failed);
    check_tap("home-right-top-accent", 0, 350, 35, 4, total, failed);
    check_tap("home-left-background", 0, 215, 150, 1, total, failed);
    check_tap("home-left-progress", 0, 100, 215, 1, total, failed);
    static const char *tname[5] = {"tab-agent", "tab-quota", "tab-usage", "tab-route", "tab-settings"};
    static const int tpage[5] = {4, 5, 6, 7, 9};
    for (int page = 4; page <= 9; page++) {
        for (int i = 0; i < 5; i++)
            if (tpage[i] != page)  // tapping the tab you are already on proves nothing
                check_tap(tname[i], page, ui_tab_center_x(i), 20, tpage[i], total, failed);
        if (page != 0) check_tap("home-button", page, 22, 20, 0, total, failed);
    }
    ui_goto(0);
    Serial.printf("[taptest] END total=%d failed=%d\n", total, failed);
}

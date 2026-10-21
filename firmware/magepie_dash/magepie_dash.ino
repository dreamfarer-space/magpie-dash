// magpie dash — 480x272 dashboard for the JC4827W543 (ESP32-S3).
//
// Pages: home / claude states x3 / agents / quota / usage / routes x2
// Data: magpie gateway over WiFi (/v1/magpie/quotas, /usage, /usage/requests) + relay /dash/agents

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <string.h>

#include "board.h"
#include "appcfg.h"
#include "config.h"
#include "display_port.h"
#include "gateway.h"
#include "i18n.h"
#include "usb_link.h"
#include "model.h"
#include "netcfg.h"
#include "ui.h"

// Data is pulled by a background task: doing 5 HTTP requests inside loop()
// froze the UI for the whole transfer (that was the "laggy" feeling).
static DashData g_data;
static SemaphoreHandle_t g_mtx = nullptr;
static volatile bool g_dirty = false;
static volatile bool g_force = false;
static volatile bool g_mock = false;  // serial 'm': demo data, live fetches stop overwriting it
static uint32_t g_last_clock = 0;

void app_request_refresh() { g_force = true; }

static void publish(DashData &src) {
    if (!g_mock && xSemaphoreTake(g_mtx, pdMS_TO_TICKS(300)) == pdTRUE) {
        g_data = src;
        g_dirty = true;
        xSemaphoreGive(g_mtx);
    }
}

static void fetch_task(void *arg) {
    static DashData local;
    uint32_t cycle = 0;
    for (;;) {
        if (g_cfg.link_mode ? netcfg_ready() : usb_link_connected()) {
            // Quotas and the agent catalogue move slowly: fetch them every third
            // cycle so the common path is ~half a second instead of ~1.7 s.
            const bool slow = (cycle++ % 3) == 0;
            const bool light = gw_take_light() || !slow;
            uint32_t t0 = millis();
            const bool have_head = gw_fetch_head(local, light);
            const uint32_t head_ms = millis() - t0;
            // Rows are replaced atomically by the tail (fetch_requests only
            // touches them after a successful fetch). Clearing them here used to
            // leave an empty list whenever the tail could not run.
            (void)have_head;
            publish(local);              // KPI cards are on screen after this
            const uint32_t t1 = millis();
            gw_fetch_tail(local, light);
            const uint32_t tail_ms = millis() - t1;
            publish(local);              // rows follow a moment later
            if (Serial.availableForWrite() >= 120)
                Serial.printf("[gw] ok=%d prov=%d calls=%u today=%u tok=%.0f rows=%d agents=%d\n",
                              (int)local.gw, local.nprov, (unsigned)local.calls,
                              (unsigned)local.today_calls, local.tok_total(), local.nrows,
                              local.nagents);
            if (Serial.availableForWrite() >= 60)
                Serial.printf("[gw] cycle head=%u tail=%u light=%d\n", (unsigned)head_ms,
                              (unsigned)tail_ms, (int)light);
        } else if (Serial.availableForWrite() >= 48) {
            // No fetch for 15 s: say why, once per sleep window.
            Serial.printf("[gw] idle usb=%d netcfg=%d mode=%u\n", (int)usb_link_connected(),
                          (int)netcfg_ready(), (unsigned)g_cfg.link_mode);
        }
        // sleep in slices so a manual refresh (serial 'r') is picked up quickly
        const int slices = (int)g_cfg.refresh_s * 10;  // settings row: 15/30/60 s
        for (int i = 0; i < slices; i++) {
            if (g_force) break;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        g_force = false;
    }
}

// Serial command "shot": render every page into a PSRAM frame buffer and dump
// it as hex RGB565, so the layout can be inspected on a PC.
static uint16_t *g_shot_buf = nullptr;

// all pages (serial 's'), or only the page in view as it is now (serial 'S':
// open sheets, speech bubbles, a running animation frame)
static void shoot_all(bool only_current) {
    if (!g_shot_buf) {
        g_shot_buf = (uint16_t *)heap_caps_malloc(LCD_W * LCD_H * 2, MALLOC_CAP_SPIRAM);
        if (!g_shot_buf) {
            Serial.println("SHOTS: no psram buffer");
            return;
        }
    }
    // The dump is 261 KB per page: normal logging drops bytes when the host is
    // slow (non-blocking TX), so pace this path explicitly and restore after.
    Serial.setTxTimeoutMs(2000);
    Serial.println("SHOTS_BEGIN");
    static const char *hexd = "0123456789abcdef";
    // "OOOOO:<128 hex>*SS": byte offset + payload + byte-sum. The host places
    // every line by its offset and drops one that fails the sum, so a lost byte
    // or another task's log line in the middle can no longer shift the rest of
    // the frame (that used to show up as colour garbage), and it re-asks.
    char line[8 + 128 + 4];
    const int cur = ui_cur_page();
    for (int i = only_current ? cur : 0; i < (only_current ? cur + 1 : UI_PAGES); i++) {
        if (!only_current) ui_goto(i);
        memset(g_shot_buf, 0, LCD_W * LCD_H * 2);
        display_capture(g_shot_buf);
        lv_obj_invalidate(lv_scr_act());
        lv_refr_now(NULL);
        display_capture(nullptr);
        Serial.printf("SHOT %d %d %d %u\n", i, LCD_W, LCD_H, (unsigned)(LCD_W * LCD_H * 2));
        const uint8_t *px = (const uint8_t *)g_shot_buf;
        const uint32_t n = LCD_W * LCD_H * 2;
        for (uint32_t k = 0; k < n; k += 64) {
            int col = snprintf(line, sizeof(line), "%05x:", (unsigned)k);
            uint8_t sum = 0;
            for (uint32_t j = k; j < k + 64 && j < n; j++) {
                line[col++] = hexd[px[j] >> 4];
                line[col++] = hexd[px[j] & 0xF];
                sum += px[j];
            }
            line[col++] = '*';
            line[col++] = hexd[sum >> 4];
            line[col++] = hexd[sum & 0xF];
            line[col] = 0;
            while (Serial.availableForWrite() < 160) delay(1);
            Serial.println(line);
        }
        Serial.println("END");
    }
    Serial.setTxTimeoutMs(0);
    if (!only_current) ui_goto(0);
    Serial.println("SHOTS_DONE");
}


static void dump_tree() {
    Serial.println("DUMP_BEGIN");
    for (int i = 0; i < UI_PAGES; i++) {
        lv_obj_t *p = ui_page_obj(i);
        Serial.printf("TREE %d obj=%p kids=%u\n", i, (void *)p,
                      p ? (unsigned)lv_obj_get_child_cnt(p) : 0u);
        if (!p) continue;
        uint32_t n = lv_obj_get_child_cnt(p);
        for (uint32_t k = 0; k < n; k++) {
            lv_obj_t *c = lv_obj_get_child(p, k);
            lv_area_t a;
            lv_obj_get_coords(c, &a);
            uint32_t bg = (uint32_t)lv_color_to32(lv_obj_get_style_bg_color(c, 0)) & 0xFFFFFF;
            Serial.printf("  #%u %d,%d %dx%d bg=%06x opa=%d\n", (unsigned)k, (int)a.x1,
                          (int)a.y1, (int)(a.x2 - a.x1 + 1), (int)(a.y2 - a.y1 + 1), bg,
                          (int)lv_obj_get_style_bg_opa(c, 0));
        }
    }
    Serial.println("DUMP_END");
}

static void touch_dump() {
    Serial.println("TOUCH_BEGIN");
    uint32_t t0 = millis();
    uint16_t n = 0;
    while (millis() - t0 < 15000) {
        int16_t x = 0, y = 0;
        if (touch_debug_read(&x, &y)) {
            int32_t mx, my;
            touch_map_point(x, y, &mx, &my);
            Serial.printf("pt raw=(%d,%d) map=(%ld,%ld)\n", x, y, (long)mx, (long)my);
            n++;
        }
        delay(30);
    }
    Serial.printf("TOUCH_END %u samples\n", (unsigned)n);
}

// Serial 'l': how busy LVGL is while each page just sits there (animations,
// blinking labels). Anything far above a few % makes touch feel sluggish.
static uint32_t g_busy_us = 0;

static void page_load() {
    Serial.println("LOAD_BEGIN");
    for (int i = 0; i < UI_PAGES; i++) {
        ui_goto(i);
        for (uint32_t t = millis(); millis() - t < 400;) display_tick();  // let the page paint once
        g_busy_us = 0;
        const uint32_t t0 = millis();
        while (millis() - t0 < 2000) {
            const uint32_t a = micros();
            display_tick();
            g_busy_us += micros() - a;
            delay(5);
        }
        Serial.printf("LOAD page %d busy %u%%\n", i, (unsigned)(g_busy_us / 20000));
    }
    ui_goto(0);
    Serial.println("LOAD_END");
}

static void page_timing() {
    Serial.println("PERF_BEGIN");
    for (int i = 0; i < UI_PAGES; i++) {
        uint32_t t0 = millis();
        ui_goto(i);
        lv_obj_invalidate(lv_scr_act());
        display_perf_reset();
        lv_refr_now(NULL);
        uint32_t dt = millis() - t0;
        uint32_t frames, flush_us, flush_us_max;
        display_perf(&frames, &flush_us, &flush_us_max);
        Serial.printf("PERF page %d redraw %u ms | flush %u bands %u ms, render ~%u ms\n",
                      i, (unsigned)dt, (unsigned)frames, (unsigned)(flush_us / 1000),
                      (unsigned)(dt > flush_us / 1000 ? dt - flush_us / 1000 : 0));
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    ui_goto(0);
    Serial.println("PERF_END");
}

// Serial 'P': press the tabs through the input driver and time touch-down ->
// last pixel on the panel, i.e. the latency a finger sees on a page switch
// (tabs switch on press, so the hold before the release is not part of it).
static void tap_latency() {
    static const int order[12] = {1, 2, 3, 4, 5, 0, 3, 1, 5, 2, 4, 0};
    Serial.println("TAPLAT_BEGIN");
    ui_goto(0);
    display_wait_idle(1000);
    uint32_t sum = 0, worst = 0;
    for (int k = 0; k < 12; k++) {
        vTaskDelay(pdMS_TO_TICKS(250));  // separate taps: no burst coalescing
        const uint32_t t0 = millis();
        display_test_press(ui_tab_center_x(order[k]), ui_tab_center_y(), true);
        ui_page_settle();
        display_wait_idle(1000);
        const uint32_t dt = millis() - t0;
        display_test_press(ui_tab_center_x(order[k]), ui_tab_center_y(), false);
        sum += dt;
        if (dt > worst) worst = dt;
        Serial.printf("TAPLAT tab %d -> page %d %u ms\n", order[k], ui_cur_page(), (unsigned)dt);
    }
    Serial.printf("TAPLAT avg %u ms worst %u ms\n", (unsigned)(sum / 12), (unsigned)worst);
    Serial.println("TAPLAT_END");
}

// Demo data shaped like the design, for screenshots without a gateway.
static void mock_data(DashData &d) {
    memset(&d, 0, sizeof(d));  // no DashData() temporary: it is several KB, too big for the loop stack
    strcpy(d.period, "30d");
    d.wifi = d.gw = true;
    d.nprov = 3;
    QuotaProvider &c = d.prov[0];
    strcpy(c.provider, "codex"); strcpy(c.name, "Codex"); strcpy(c.plan, "plus");
    strcpy(c.user, "alice@example.com");
    c.nwin = 2;
    strcpy(c.win[0].name, "5 hours"); c.win[0].remaining = 100;
    strcpy(c.win[0].resetIso, "2026-10-19T18:59:00+08:00");  // raw, like the gateway sends it
    strcpy(c.win[1].name, "7 days"); c.win[1].remaining = 77;
    strcpy(c.win[1].resetIso, "2026-10-22T09:00:00+08:00");
    c.win[1].reset_at = time(nullptr) + 3 * 86400 + 21 * 3600;
    QuotaProvider &g = d.prov[1];
    strcpy(g.provider, "antigravity"); strcpy(g.name, "Antigravity"); strcpy(g.plan, "Google AI Pro");
    strcpy(g.user, "devteam@example.com");
    g.nwin = 4;
    const char *gn[4] = {"Gemini · 7 days", "Gemini · 5 hours", "Claude & GPT · 7 days", "Claude & GPT · 5 hours"};
    const float gr[4] = {89, 100, 42, 10};
    for (int i = 0; i < 4; i++) { strcpy(g.win[i].name, gn[i]); g.win[i].remaining = gr[i]; }
    strcpy(g.win[0].resetIso, "2026-10-24T23:52:00+08:00");
    QuotaProvider &w = d.prov[2];
    strcpy(w.provider, "workbuddy"); strcpy(w.name, "WorkBuddy"); strcpy(w.plan, "Free");
    strcpy(w.user, "择梦舟");
    w.nwin = 1;
    strcpy(w.win[0].name, "Credits"); w.win[0].remaining = 77;
    strcpy(w.win[0].reset, "205.26 / 900");  // label, no timestamp: stays as sent

    d.calls = 9410; d.errors = 14;
    d.tok_in = 29000000; d.tok_out = 5300000; d.tok_cache = 1060000000ULL;
    d.cost = 820;
    d.nseries = 30;
    for (int i = 0; i < 30; i++) {
        snprintf(d.series[i].label, sizeof(d.series[i].label), "%s %d", i < 11 ? "Sep" : "Oct",
                 i < 11 ? 20 + i : i - 10);
        d.series[i].tokens = (float)(((i * 37) % 11) * 9e6 + (i > 20 ? 120e6 : 4e6));
    }
    d.today_calls = 51; d.today_tokens = 634000;
    struct R { const char *a, *an, *pid, *pv, *acct, *m, *stop; uint32_t in, out, cr, ms; uint16_t st; };
    const R rows[4] = {
        {"claude", "Claude Code", "codex", "Codex", "al***@example.com", "codex/gpt-6-luna", "tool_calls", 1200, 600, 56000, 4800, 200},
        {"hermes", "Hermes Agent", "workbuddy", "WorkBuddy", "择梦舟", "workbuddy/deepseek-v4.1-flash", "stop", 2100, 800, 54800, 9100, 200},
        {"codex", "Codex", "codex", "Codex", "al***@example.com", "codex/gpt-6.1-sol", "stop", 900, 300, 55400, 7300, 200},
        {"grok", "Grok Build", "antigravity", "Antigravity", "de***@example.com", "grok/grok-4.6", "stop", 40, 0, 0, 900, 429},
    };
    d.nrows = 4;
    for (int i = 0; i < 4; i++) {
        RouteRow &r = d.rows[i];
        snprintf(r.time, sizeof(r.time), "00:27:%02d", 54 - i * 10);
        strcpy(r.agent, rows[i].a); strcpy(r.agentName, rows[i].an); strcpy(r.provId, rows[i].pid);
        strcpy(r.provider, rows[i].pv); strcpy(r.account, rows[i].acct); strcpy(r.model, rows[i].m);
        strcpy(r.stop, rows[i].stop);
        r.tin = rows[i].in; r.tout = rows[i].out; r.tcache = rows[i].cr; r.ms = rows[i].ms;
        r.status = rows[i].st;
    }
    d.sessions = 2;
    struct A { const char *id, *name, *model; uint32_t calls; };
    const A ag[13] = {
        {"claude", "Claude Code", "opus · effort high", 3120}, {"codex", "Codex", "gpt-6.1-sol", 2410},
        {"hermes", "Hermes Agent", "deepseek-v4.1-flash", 1604}, {"grok", "Grok Build", "grok/grok-4.6", 820},
        {"dsh", "DeepSeek Harness", "group/smart-split", 4}, {"pi", "", "grok/grok-4.6", 2},
        {"claude-desktop", "", "", 0}, {"agy", "", "", 0}, {"codebuddy", "", "", 0},
        {"commandcode", "", "", 0}, {"cursor", "", "", 0}, {"mimocode", "", "", 0}, {"opencode", "", "", 0},
    };
    d.nagents = 13;
    d.agents_full = true;
    for (int i = 0; i < 13; i++) {
        strcpy(d.agents[i].id, ag[i].id); strcpy(d.agents[i].name, ag[i].name);
        strcpy(d.agents[i].model, ag[i].model); d.agents[i].calls = ag[i].calls;
        d.agents[i].today = ag[i].calls / 18;
        d.agents[i].tokens = ag[i].calls * 41000.0f;
        d.agents[i].cost = ag[i].calls * 0.06f;
        d.agents[i].last = ag[i].calls ? (uint32_t)time(nullptr) - (i ? 900u * i : 30u) : 0;
    }
}

// Serial 'q': 2-second timing window. Separates the three things that can make
// the UI feel slow: a stalled loop (WiFi/gateway work), LVGL's own render time,
// and the pixel transport inside flush_cb.
static bool g_perf_on = false;
static uint32_t g_stat_last_ms = 0, g_prev_loop_ms = 0;
static uint32_t g_gap_max = 0, g_tick_max = 0, g_tick_sum = 0, g_tick_n = 0;
static uint32_t g_upd_max = 0, g_upd_sum = 0, g_upd_n = 0;

static void handle_command(uint8_t c) {
        if (dbg_verbose) Serial.printf("cmd=%d '%c'\n", c, (c >= 32 && c < 127) ? c : '?');
        if (c == 'V') {  // verbose per-event logging: taps, page switches, gateway calls
            dbg_verbose = !dbg_verbose;
            Serial.printf("[dbg] verbose %s\n", dbg_verbose ? "on" : "off");
        }
        if (c == 'q') {  // toggle the 2-second timing report ('q' again to stop)
            g_perf_on = !g_perf_on;
            g_stat_last_ms = millis();
            display_perf_reset();
            Serial.printf("[perf] report %s\n", g_perf_on ? "on" : "off");
        }
        if (c == 's') shoot_all(false);
        if (c == 'S') shoot_all(true);
        if (c == 'd') {
            Serial.println("dump start");
            dump_tree();
        }
        if (c == 't') touch_dump();
        if (c == 'p') page_timing();
        if (c == 'P') tap_latency();
        if (c == 'l') page_load();
        if (c == 'f') ui_focus_next();
        if (c == 'k') ui_poke();
        if (c == 'o') ui_open_sheet(0);
        if (c >= '0' && c <= '9') ui_goto(c - '0');
        if (c == 'b') {  // debug: four tab taps in one burst (coalescing test)
            for (int i = 5; i <= 8; i++) ui_tap_page(i);
            Serial.println("[dbg] burst taps 5,6,7,8");
        }
        if (c == 'B') ui_sheet_benchmark();
        if (c == 'A') run_touch_regression();
        if (c == 'Z') ui_settings_test();
        if (c == 'F') ui_cycle_period();
        if (c == 'L') ui_link_panel(true);
        if (c == 'G') { ui_goto(8); ui_routes_scroll(-200); }
        if (c == 'H') { ui_goto(8); ui_routes_scroll(200); }
        if (c == 'l') { }  // 'l' is page_load; keep 'L' for the panel
        if (c == 'X') ui_link_panel(false);
        if (c == 'y') ui_settings_keyboard(true);
        if (c == 'Y') ui_settings_keyboard(false);
        if (c == 'U') {  // debug: flip the interface language (same path as the settings row)
            ui_set_language(lang_get() == LANG_EN ? LANG_ZH : LANG_EN);
            Serial.printf("[ui] language -> %s\n", lang_get() == LANG_EN ? "en" : "zh");
        }
        if (c == 'D') {  // reset the settings page to shipped defaults
            cfg_defaults();
            cfg_save();
            ui_settings_refresh();
            // The shipped default language is English: rebuilding through the
            // normal switch keeps the screen and the stored value in step,
            // instead of flipping to English only after the next boot.
            ui_set_language(g_cfg.lang);
            Serial.println("[cfg] reset to defaults");
        }
        if (c == 'T') ui_test_toggle();
        if (c == 'K') touch_map_reset_to_default();
        if (c == 'c') ui_show_calibration(true);
        if (c == 'x') ui_show_calibration(false);
        if (c == 'r') {  // ask the fetch task to run now (and leave demo mode)
            g_mock = false;
            g_force = true;
        }
        if (c == 'm' && xSemaphoreTake(g_mtx, pdMS_TO_TICKS(500)) == pdTRUE) {
            g_mock = true;
            mock_data(g_data);
            g_dirty = true;
            xSemaphoreGive(g_mtx);
            Serial.println("[mock] demo data loaded ('r' returns to live data)");
        }
}

static void poll_serial() {
    usb_link_tick();
    // Commands arrive either as raw bytes (no host bridge) or inside a
    // CRC-checked frame, so a stray byte can never fire a debug action.
    char framed[64];
    while (usb_link_take_command(framed, sizeof(framed))) {
        int tx, ty;
        // "@x,y": a real tap through the LVGL input driver (demo recordings,
        // scripted checks); anything else is a string of one-key commands
        if (framed[0] == '@' && sscanf(framed + 1, "%d,%d", &tx, &ty) == 2) {
            display_test_tap(tx, ty);
            Serial.printf("[tap] %d,%d -> page %d\n", tx, ty, ui_cur_page());
            continue;
        }
        for (char *p = framed; *p; ++p) handle_command((uint8_t)*p);
    }
    // Limit each UI iteration's serial work so a data frame cannot starve touch.
    int budget = 2048;
    while (Serial.available() && budget-- > 0) {
        int c = Serial.read();
        if (usb_link_receive((uint8_t)c)) continue;
        if (usb_link_connected()) {
            static uint32_t stray_n = 0;
            if (++stray_n % 16 == 1) Serial.printf("[usb] stray byte 0x%02x (n=%u)\n", (unsigned)(c & 0xff), (unsigned)stray_n);
            continue;
        }
        handle_command((uint8_t)c);
    }
}

void setup() {
    Serial.setRxBufferSize(16384);
    Serial.begin(921600);
    // Non-blocking TX: with a host that stops reading (port closed, bridge
    // stopped, USB suspended) the CDC TX ring fills and write() blocks the
    // calling task forever - which froze the whole UI. Drop instead of block.
    Serial.setTxTimeoutMs(0);
    delay(300);
    Serial.println("\n[magpie-dash] boot");
    // NOTE: GPIO0 is wired to the LCD TE signal on this panel, so it is not
    // usable as a button. Navigation is touch-only; long-press the home page
    // header to clear the WiFi settings.

    configTzTime(TZ_STRING, NTP_SERVER);

    cfg_load();  // the header and quota wording come from NVS before the UI builds

    if (!display_begin()) {
        Serial.println("[boot] display init failed, continuing anyway");
    }
    ui_init();
    ui_update(g_data);  // paints the "--" placeholders

    usb_link_begin();
    netcfg_begin();  // no-op with the radio off when the link mode is USB

    g_mtx = xSemaphoreCreateMutex();
    Serial.printf("[boot] DashData %u bytes\n", (unsigned)sizeof(DashData));
    Serial.printf("[boot] internal free=%u largest=%u psram free=%u\n",
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                  (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    xTaskCreatePinnedToCore(fetch_task, "gwfetch", 8192, nullptr, 1, nullptr, 0);
    Serial.printf("[boot] reset_reason=%d (1=poweron 3=sw 4=panic 12=usb 13=wdt)\n", (int)esp_reset_reason());
    Serial.println("[boot] ready");
}

// Hourly-ish heap trace: a slow leak or a dying subsystem shows up here before
// it turns into a silent freeze that no log line explains.
static void heap_heartbeat() {
    static uint32_t last = 0, last_i = 0, last_p = 0;
    uint32_t now = millis();
    if (last && now - last < 30000) return;
    if (Serial.availableForWrite() < 64) return;
    uint32_t ii = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    uint32_t pp = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    Serial.printf("[hb] up=%us int=%u(%+d) psram=%u(%+d)\n", (unsigned)(now / 1000),
                  (unsigned)ii, (int)ii - (int)last_i, (unsigned)pp, (int)pp - (int)last_p);
    last = now; last_i = ii; last_p = pp;
}

// One-line stall report, printed only for a new worst case: if the display still
// feels sluggish while the serial port is otherwise silent, this says whether the
// loop was blocked and by what (LVGL tick vs. the data update).
static uint32_t s_worst_gap = 0, s_last_tick_us = 0, s_last_upd_us = 0;

void loop() {
    const uint32_t now = millis();
    const uint32_t gap = g_prev_loop_ms ? now - g_prev_loop_ms : 0;
    if (gap > g_gap_max) g_gap_max = gap;
    g_prev_loop_ms = now;
    if (gap > 120 && gap > s_worst_gap + 40) {
        s_worst_gap = gap;
        Serial.printf("[stall] loop gap %u ms (LVGL tick %u ms, data update %u ms)\n",
                      (unsigned)gap, (unsigned)(s_last_tick_us / 1000),
                      (unsigned)(s_last_upd_us / 1000));
    }

    poll_serial();
    if (ui_cfg_take_dirty()) cfg_save();  // off the tap path: NVS writes are ~10 ms
    if (ui_lang_take_dirty()) {
        // Language switch: rebuilds every page. Take g_mtx because ui_rebuild()
        // repaints from g_data, which the fetch task writes under this mutex.
        if (xSemaphoreTake(g_mtx, pdMS_TO_TICKS(2000)) == pdTRUE) {
            ui_rebuild();
            xSemaphoreGive(g_mtx);
        }
    }
    heap_heartbeat();
    {
        static uint32_t s_set_ms = 0;
        if (now - s_set_ms > 1000) {  // set_txt only redraws when the text changes
            s_set_ms = now;
            if (ui_cur_page() == PG_SETTINGS) ui_settings_refresh();
        }
    }
#if !USB_LINK_ONLY
    netcfg_loop();
#endif

    uint32_t t0 = micros();
    display_tick();
    ui_page_settle();  // one paint for a burst of taps, never one per tap
    uint32_t dt = micros() - t0;
    s_last_tick_us = dt;
    if (dt > g_tick_max) g_tick_max = dt;
    g_tick_sum += dt;
    g_tick_n++;

    static uint32_t s_last_clock = 0;
    if (now - s_last_clock > 1000) {
        s_last_clock = now;
        ui_tick();
    }
    // A data refresh repaints the visible page (~120-160 ms). Doing that on top
    // of a tap is what "sometimes a tap does nothing" felt like, so hold the
    // update for a moment while the finger is still busy - but never longer than
    // 3 s, or the live data would visibly freeze.
    static uint32_t s_dirty_since = 0;
    if (g_dirty && !s_dirty_since) s_dirty_since = now ? now : 1;
    if (g_dirty && (!touch_recent(400) || now - s_dirty_since > 3000)) {
        s_dirty_since = 0;
        if (xSemaphoreTake(g_mtx, 0) == pdTRUE) {
            t0 = micros();
            ui_update(g_data);
            dt = micros() - t0;
            s_last_upd_us = dt;
            if (dt > g_upd_max) g_upd_max = dt;
            g_upd_sum += dt;
            g_upd_n++;
            g_dirty = false;
            xSemaphoreGive(g_mtx);
        }
    }

    if (g_perf_on && now - g_stat_last_ms >= 2000) {
        uint32_t frames, flush_us, flush_us_max;
        display_perf(&frames, &flush_us, &flush_us_max);
        Serial.printf(
            "[perf] gap=%u tick_max=%u tick_avg=%u upd_max=%u upd_n=%u flush_n=%u "
            "flush_max=%u flush_avg=%u us\n",
            (unsigned)g_gap_max, (unsigned)(g_tick_max / 1000),
            (unsigned)(g_tick_n ? g_tick_sum / g_tick_n / 1000 : 0), (unsigned)(g_upd_max / 1000),
            (unsigned)g_upd_n, (unsigned)frames, (unsigned)(flush_us_max / 1000),
            (unsigned)(frames ? flush_us / frames / 1000 : 0));
        g_gap_max = g_tick_max = g_tick_sum = g_tick_n = 0;
        g_upd_max = g_upd_sum = g_upd_n = 0;
        g_stat_last_ms = now;
        display_perf_reset();
    }
    delay(5);
}

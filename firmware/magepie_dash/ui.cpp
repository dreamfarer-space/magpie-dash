#include "ui.h"
#include "touch_calibration.h"
#include "display_port.h"

#include <Arduino.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#include "agent_sprites.h"
#include "appcfg.h"
#include "assets_sprites.h"
#include "board.h"
#include "config.h"
#include "gateway.h"
#include "netcfg.h"
#include "usb_link.h"
#include "util.h"

LV_FONT_DECLARE(ark10);
LV_FONT_DECLARE(ark12);
LV_FONT_DECLARE(ark24);
LV_FONT_DECLARE(ark16);

// Tap-driven page switch (coalesces bursts of taps); defined with the page
// machinery further down. Programmatic switches use ui_goto().
static void page_request(int page);

// ------------------------------------------------------------------ palette
#define C_BG 0x080C10
#define C_CARD 0x15161D
#define C_LINE 0x383749
#define C_TEXT 0xEEEEF6
#define C_MUTED 0x7E7C89
#define C_DIM 0x2A2B38
#define C_SHADOW 0x24252F
#define C_FAINT 0x4A4A58
#define C_ORANGE 0xDE7552
#define C_GREEN 0x39DE83
#define C_PURPLE 0x7B6DFF
#define C_RED 0xE5484D
#define C_AMBER 0xF2B33D

#define AG_ROWS AGENT_MAX
#define RT_ROWS 12  // full request list: the detail page scrolls

static lv_obj_t *pg[UI_PAGES];
static int s_page = 0;
static const DashData *s_data = nullptr;

// Agent the state pages and the home card are about. Empty = follow whichever
// agent made the latest request; tapping a row on the agent page pins one.
static char s_focus[16] = "";
static uint32_t s_focus_ms = 0;
#define FOCUS_HOLD_MS (10UL * 60UL * 1000UL)  // a pinned agent falls back after 10 min

static void ui_clicks(lv_obj_t *o);
static void link_panel_close();
static void kb_close();
static void link_panel_refresh();
static void close_overlays();

// ------------------------------------------------------------------ refs
static struct {
    // home
    lv_obj_t *date, *clock, *hm_handle;
    // settings page
    lv_obj_t *set_title, *set_val[6];
    // connection panel (settings -> 连接 magpie)
    lv_obj_t *lk, *lk_state, *lk_addr, *lk_hint, *lk_btn[2];
    lv_obj_t *hm_accent, *hm_name, *hm_state, *hm_dot, *hm_pct, *hm_bar, *hm_qlbl, *hm_stats,
        *hm_mag, *hm_tick;
    // state pages (think / code / confirm)
    lv_obj_t *st_agent[3], *st_star[3], *st_pip[3][3];
    lv_obj_t *st_sess[3], *st_title[3], *st_sub[3], *st_l5[3], *st_l7[3], *st_p5[3], *st_p7[3],
        *st_bar5[3], *st_bar7[3], *st_reset[3], *st_conn[3], *st_src[3];
    // agents
    lv_obj_t *ag_list, *ag_row[AG_ROWS], *ag_icon[AG_ROWS], *ag_name[AG_ROWS],
        *ag_detail[AG_ROWS], *ag_calls[AG_ROWS], *ag_live[AG_ROWS], *ag_state[AG_ROWS],
        *ag_tab[UI_PAGES];
    char ag_id[AG_ROWS][16];
    // agent detail sheet (agents page)
    lv_obj_t *sh, *sh_name, *sh_model, *sh_num[4], *sh_share, *sh_bar, *sh_last, *sh_follow;
    char sh_id[16];
    // quota
    lv_obj_t *q_blk[QUOTA_MAX_PROV], *q_dot[QUOTA_MAX_PROV], *q_name[QUOTA_MAX_PROV], *q_plan[QUOTA_MAX_PROV],
        *q_user[QUOTA_MAX_PROV], *q_reset[QUOTA_MAX_PROV], *q_empty;
    lv_obj_t *q_ring[QUOTA_MAX_PROV][QUOTA_MAX_WIN], *q_pct[QUOTA_MAX_PROV][QUOTA_MAX_WIN],
        *q_win[QUOTA_MAX_PROV][QUOTA_MAX_WIN], *q_word[QUOTA_MAX_PROV];
    // usage
    lv_obj_t *u_tok, *u_in, *u_out, *u_cache, *u_calls, *u_err, *u_cost, *u_price, *u_hit, *u_hit2;
    lv_obj_t *u_bars[SERIES_MAX], *u_lbl[5], *u_ymax, *u_empty;
    // routes
    lv_obj_t *r_sum[2], *r_empty[2];
    lv_obj_t *r_row[2][RT_ROWS], *r_icon[2][RT_ROWS], *r_time[2][RT_ROWS], *r_agent[2][RT_ROWS],
        *r_model[2][RT_ROWS], *r_tag[2][RT_ROWS], *r_flow[2][RT_ROWS], *r_use[2][RT_ROWS],
        *r_scroll[2];
    lv_obj_t *r_top_icon, *r_top_agent, *r_top_to;
} U;


// ------------------------------------------------------------------ helpers
static lv_obj_t *obj(lv_obj_t *p, int x, int y, int w, int h) {
    lv_obj_t *o = lv_obj_create(p);
    if (!o) {
        Serial.println("[ui] LVGL out of memory");
        return p;  // keep the UI drawable instead of dereferencing NULL
    }
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    // Containers and decorative bars must not intercept their parent's taps.
    // lv_obj_create() defaults to CLICKABLE even after remove_style_all().
    // Real buttons, avatars, rows and scroll areas explicitly opt in below.
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return o;
}

static lv_obj_t *txt(lv_obj_t *p, const char *s, int x, int y, const lv_font_t *f, uint32_t col) {
    lv_obj_t *l = lv_label_create(p);
    lv_label_set_text(l, s);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(col), 0);
    return l;
}

// label clipped with "..." at a fixed width
static lv_obj_t *txt_clip(lv_obj_t *p, const char *s, int x, int y, int w, const lv_font_t *f,
                          uint32_t col) {
    lv_obj_t *l = txt(p, s, x, y, f, col);
    lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
    lv_obj_set_width(l, w);
    return l;
}

static lv_obj_t *card(lv_obj_t *p, int x, int y, int w, int h) {
    lv_obj_t *c = obj(p, x, y, w, h);
    lv_obj_set_style_bg_color(c, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, 3, 0);
    lv_obj_set_style_border_width(c, 1, 0);
    lv_obj_set_style_border_color(c, lv_color_hex(C_LINE), 0);
    return c;
}

static lv_obj_t *dot(lv_obj_t *p, int x, int y, int s, uint32_t col) {
    lv_obj_t *d = obj(p, x, y, s, s);
    lv_obj_set_style_bg_color(d, lv_color_hex(col), 0);
    lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(d, 0, 0);
    return d;
}

static lv_obj_t *img(lv_obj_t *p, const void *src, int x, int y) {
    lv_obj_t *i = lv_img_create(p);
    lv_img_set_src(i, src);
    lv_obj_set_pos(i, x, y);
    return i;
}

// small bordered tag ("codex/gpt-6-luna", "提示词建议"); sized by its text
static lv_obj_t *chip(lv_obj_t *p, const char *s, uint32_t border, uint32_t col) {
    lv_obj_t *l = txt(p, s, 0, 0, &ark12, col);
    lv_obj_set_style_bg_color(l, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(l, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(l, 1, 0);
    lv_obj_set_style_border_color(l, lv_color_hex(border), 0);
    lv_obj_set_style_radius(l, 2, 0);
    lv_obj_set_style_pad_hor(l, 4, 0);
    lv_obj_set_style_pad_ver(l, 1, 0);
    return l;
}

// row container that lays its children out left to right, vertically centred
static lv_obj_t *hrow(lv_obj_t *p, int x, int y, int w, int h, int gap, lv_flex_align_t main) {
    lv_obj_t *r = obj(p, x, y, w, h);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, main, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(r, gap, 0);
    lv_obj_add_flag(r, LV_OBJ_FLAG_EVENT_BUBBLE);
    return r;
}

// segmented progress bar; segments are row-flexed children
static lv_obj_t *segbar(lv_obj_t *p, int x, int y, int w, int h, int n, uint32_t on) {
    lv_obj_t *c = obj(p, x, y, w, h);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(c, 2, 0);
    int segw = (w - (n - 1) * 2) / n;
    if (segw < 1) segw = 1;
    for (int i = 0; i < n; i++) {
        lv_obj_t *s = obj(c, 0, 0, segw, h);
        lv_obj_set_style_bg_color(s, lv_color_hex(C_DIM), 0);
        lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(s, 1, 0);
        lv_obj_add_flag(s, LV_OBJ_FLAG_EVENT_BUBBLE);
    }
    lv_obj_set_user_data(c, (void *)(uintptr_t)on);
    return c;
}

static void segbar_set(lv_obj_t *bar, float pct, uint32_t on) {
    if (!bar) return;
    uint32_t n = lv_obj_get_child_cnt(bar);
    int lit = (int)(pct * n / 100.0f + 0.5f);
    if (lit < 0) lit = 0;
    if (lit > (int)n) lit = (int)n;
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *s = lv_obj_get_child(bar, i);
        // Only write when the segment actually changes: an unconditional
        // set_style invalidates the segment (and repaints the bar) on every
        // 15 s data refresh even when nothing moved.
        const uint32_t col = i < (uint32_t)lit ? on : C_DIM;
        if (lv_color_to32(lv_obj_get_style_bg_color(s, 0)) != lv_color_to32(lv_color_hex(col)))
            lv_obj_set_style_bg_color(s, lv_color_hex(col), 0);
    }
}

static void segbar_set(lv_obj_t *bar, float pct) {
    if (bar) segbar_set(bar, pct, (uint32_t)(uintptr_t)lv_obj_get_user_data(bar));
}

static lv_obj_t *ring(lv_obj_t *p, int x, int y, int size, uint32_t col) {
    lv_obj_t *a = lv_arc_create(p);
    lv_obj_set_size(a, size, size);
    lv_obj_set_pos(a, x, y);
    lv_arc_set_rotation(a, 270);
    lv_arc_set_bg_angles(a, 0, 360);
    lv_arc_set_range(a, 0, 100);
    lv_arc_set_value(a, 0);
    lv_obj_remove_style(a, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(a, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_width(a, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(a, false, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(a, lv_color_hex(C_DIM), LV_PART_MAIN);
    lv_obj_set_style_arc_color(a, lv_color_hex(col), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(a, 0, LV_PART_MAIN);
    return a;
}

static void hline(lv_obj_t *p, int x, int y, int w, uint32_t col) {
    lv_obj_t *l = obj(p, x, y, w, 1);
    lv_obj_set_style_bg_color(l, lv_color_hex(col), 0);
    lv_obj_set_style_bg_opa(l, LV_OPA_COVER, 0);
}

// Dashed rule between card sections. ONE lv_line whose points form the dashes:
// a 2x2 object per dot meant ~1000 extra objects across the nine pages, which
// ate the heap and slowed every redraw down.
static void dline(lv_obj_t *p, int x, int y, int w) {
    int n = (w / 4 + 1) * 2;
    lv_point_t *pts = (lv_point_t *)lv_mem_alloc(sizeof(lv_point_t) * n);
    if (!pts) return;
    int k = 0;
    for (int i = 0; i + 3 <= w && k + 1 < n; i += 4) {
        pts[k].x = (lv_coord_t)(x + i);
        pts[k].y = (lv_coord_t)y;
        pts[k + 1].x = (lv_coord_t)(x + i + 2);
        pts[k + 1].y = (lv_coord_t)y;
        k += 2;
    }
    if (k < 2) {
        lv_mem_free(pts);
        return;
    }
    lv_obj_t *l = lv_line_create(p);
    lv_line_set_points(l, pts, k);  // lv_line keeps the pointer, so it must be owned
    lv_obj_set_style_line_width(l, 1, 0);
    lv_obj_set_style_line_color(l, lv_color_hex(C_LINE), 0);
}

static void show(lv_obj_t *o, bool on) {
    if (!o) return;
    if (on == !lv_obj_has_flag(o, LV_OBJ_FLAG_HIDDEN)) return;  // no-op: skip the invalidation
    if (on)
        lv_obj_clear_flag(o, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
}

static void set_txt(lv_obj_t *o, const char *s) {
    if (o && strcmp(lv_label_get_text(o), s)) lv_label_set_text(o, s);
}

static void set_txtf(lv_obj_t *o, const char *fmt, ...) {
    if (!o) return;
    char buf[96];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    set_txt(o, buf);
}

// ark16 is the Latin-only 16px cut; anything with CJK drops to ark12
static void set_txt_fit(lv_obj_t *o, const char *s) {
    if (!o) return;
    bool cjk = false;
    for (const char *c = s; *c && !cjk; c++) cjk = (unsigned char)*c >= 0x80;
    lv_obj_set_style_text_font(o, cjk ? &ark12 : &ark16, 0);
    set_txt(o, s);
}

static void set_color(lv_obj_t *o, uint32_t col) {
    if (!o) return;
    if (lv_color_to32(lv_obj_get_style_text_color(o, 0)) == lv_color_to32(lv_color_hex(col))) return;
    lv_obj_set_style_text_color(o, lv_color_hex(col), 0);
}

// ------------------------------------------------------------------ agents
static const AgentSkin *skin_for(const char *id) {
    for (int i = 0; i < AGENT_SKIN_COUNT; i++)
        if (id && !strcmp(AGENT_SKINS[i].id, id)) return &AGENT_SKINS[i];
    return &AGENT_SKINS[AGENT_SKIN_COUNT - 1];  // "generic"
}

static bool skin_known(const char *id) { return skin_for(id) != &AGENT_SKINS[AGENT_SKIN_COUNT - 1]; }

// display name: what magpie calls it, else our catalog name, else the raw id
static const char *agent_label(const char *id, const char *name) {
    if (name && *name) return name;
    if (skin_known(id)) return skin_for(id)->name;
    return (id && *id) ? id : "Agent";
}

// ------------------------------------------------------------------ logos
// Agent logos are pre-rendered at 20 / 56 / 88 px (tools/gen_agents.py), so an
// lv_img shows them 1:1: no zoom, no pivot.
static lv_obj_t *logo(lv_obj_t *p) {
    lv_obj_t *i = lv_img_create(p);
    lv_obj_clear_flag(i, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(i, LV_OBJ_FLAG_EVENT_BUBBLE);
    return i;
}

static void logo_set(lv_obj_t *i, const lv_img_dsc_t *src) {
    if (i && lv_img_get_src(i) != src) lv_img_set_src(i, src);
}

// ------------------------------------------------------------------ avatars
// An agent's avatar is its logo as is, straight on the card: no tile, no
// ring, so the brand colours stay true. Tapping it "pokes" the agent: it hops
// and says one of its lines in a speech bubble (poke it five times quickly and
// it complains). What it is doing shows as three typing dots (thinking / at
// work), a ">_" badge (writing code) or an amber "!" (waiting for you).
//
// Nothing here animates continuously: an endless lv_anim around an 88 px
// alpha image kept LVGL busy 40-70% of the time and made touch sluggish. The
// dots are three 6 px squares recoloured by one slow timer.
enum { AV_IDLE, AV_THINK, AV_CODE, AV_CONFIRM };

struct Avatar {
    lv_obj_t *box, *logo, *badge, *badge_lbl, *bubble, *bubble_lbl, *dots[3];
    const AgentSkin *sk;
    bool big;  // 88 px logo (state pages) or 56 px
    int mode;
    uint8_t pokes, say;
    uint32_t poke_ms;
    uint32_t breathe_ms;  // 0 = not breathing
};

static Avatar s_av_home, s_av_state[3], s_av_sheet;

static void anim_translate_y(void *o, int32_t v) { lv_obj_set_style_translate_y((lv_obj_t *)o, v, 0); }
static void anim_nop(void *, int32_t) {}
static void anim_hide(lv_anim_t *a) { lv_obj_add_flag((lv_obj_t *)a->var, LV_OBJ_FLAG_HIDDEN); }

static void avatar_hop(Avatar &a) {
    if (!g_cfg.anim) return;                            // settings: animations off
    if (!a.box || !lv_obj_is_visible(a.box)) return;  // nobody would see it
    lv_anim_del(a.box, anim_translate_y);
    lv_anim_t an;
    lv_anim_init(&an);
    lv_anim_set_var(&an, a.box);
    lv_anim_set_exec_cb(&an, anim_translate_y);
    lv_anim_set_values(&an, -14, 0);
    lv_anim_set_time(&an, 450);
    lv_anim_set_path_cb(&an, lv_anim_path_bounce);
    lv_anim_start(&an);
    a.breathe_ms = 0;  // the loop was cancelled above; the next sync re-arms it
}

// Idle motion: a slow 3 px rise-and-fall loop, faster while the agent is busy.
// It uses the same translate_y executor as the hop, so anything that wants to
// hop must stop the loop first (lv_anim_del does that).
static void avatar_breathe(Avatar &a, bool on, uint32_t period) {
    if (!a.box) return;
    if (!g_cfg.anim || !on) {
        if (a.breathe_ms) {
            a.breathe_ms = 0;
            lv_anim_del(a.box, anim_translate_y);
            lv_obj_set_style_translate_y(a.box, 0, 0);
        }
        return;
    }
    if (a.breathe_ms == period) return;  // already running at this speed
    a.breathe_ms = period;
    lv_anim_del(a.box, anim_translate_y);
    lv_anim_t an;
    lv_anim_init(&an);
    lv_anim_set_var(&an, a.box);
    lv_anim_set_exec_cb(&an, anim_translate_y);
    lv_anim_set_values(&an, 0, -3);
    lv_anim_set_time(&an, period);
    lv_anim_set_playback_time(&an, period);
    lv_anim_set_repeat_count(&an, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&an, lv_anim_path_ease_in_out);
    lv_anim_start(&an);
}

// Which avatars are on screen right now, and how fast they should breathe.
static bool s_live_state = false;

static void avatars_sync_motion(bool busy) {
    const uint32_t period = busy ? 800 : 1900;
    avatar_breathe(s_av_home, s_page == PG_HOME, period);
    for (int i = 0; i < 3; i++)
        avatar_breathe(s_av_state[i], s_page == PG_STATE_THINK + i, period);
    avatar_breathe(s_av_sheet, U.sh && !lv_obj_has_flag(U.sh, LV_OBJ_FLAG_HIDDEN), period);
}

static void avatar_say(Avatar &a, const char *line, uint32_t ms) {
    if (!a.bubble) return;
    lv_label_set_text(a.bubble_lbl, line);
    lv_obj_clear_flag(a.bubble, LV_OBJ_FLAG_HIDDEN);
    lv_anim_del(a.bubble, anim_nop);
    lv_anim_t an;  // hide again after a while
    lv_anim_init(&an);
    lv_anim_set_var(&an, a.bubble);
    lv_anim_set_exec_cb(&an, anim_nop);
    lv_anim_set_values(&an, 0, 1);
    lv_anim_set_time(&an, 1);
    lv_anim_set_delay(&an, ms);
    lv_anim_set_ready_cb(&an, anim_hide);
    lv_anim_start(&an);
}

static void avatar_poke(Avatar &a) {
    if (!a.sk) return;
    const uint32_t now = lv_tick_get();
    a.pokes = now - a.poke_ms < 1500 ? a.pokes + 1 : 1;
    a.poke_ms = now;
    avatar_say(a, a.pokes >= 5 ? "别戳了 >_<" : a.sk->say[a.say++ & 1], 2200);
    avatar_hop(a);
    Serial.printf("[ui] poke %s x%d\n", a.sk->id, a.pokes);
}

static void avatar_poke_cb(lv_event_t *e) { avatar_poke(*(Avatar *)lv_event_get_user_data(e)); }

static void avatar_badge(Avatar &a, const char *s, uint32_t col) {
    show(a.badge, s != nullptr);
    if (!s) return;
    lv_obj_set_style_bg_color(a.badge, lv_color_hex(col), 0);
    set_txt(a.badge_lbl, s);
}

static void avatar_mode(Avatar &a, int mode) {
    if (!a.box || a.mode == mode) return;
    a.mode = mode;
    for (int k = 0; k < 3; k++) show(a.dots[k], mode == AV_THINK);
    if (mode == AV_CODE)
        avatar_badge(a, ">_", C_PURPLE);
    else if (mode == AV_CONFIRM)
        avatar_badge(a, "!", C_AMBER);
    else
        avatar_badge(a, nullptr, 0);
}

// typing dots: one lit dot walks along (called by a 350 ms timer)
static void avatar_step(Avatar &a, int phase) {
    if (!a.box || a.mode != AV_THINK || !lv_obj_is_visible(a.box)) return;
    const uint32_t on = a.sk ? a.sk->accent : C_TEXT;
    for (int k = 0; k < 3; k++) lv_obj_set_style_bg_color(a.dots[k], lv_color_hex(k == phase ? on : C_FAINT), 0);
}

// logo centred on cx/cy; the speech bubble floats at bubble_y
static void avatar_create(Avatar &a, lv_obj_t *p, int cx, int cy, bool big, int bubble_y) {
    const int sz = big ? 88 : 56, bs = sz + 8;
    a.box = obj(p, cx - bs / 2, cy - bs / 2, bs, bs);
    lv_obj_add_flag(a.box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(a.box, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_event_cb(a.box, avatar_poke_cb, LV_EVENT_CLICKED, &a);
    a.logo = logo(a.box);
    lv_obj_align(a.logo, LV_ALIGN_CENTER, 0, 0);

    // corner badge: ">_" while coding, "!" while waiting
    a.badge = obj(a.box, bs - 18, -8, 26, 18);
    lv_obj_set_style_radius(a.badge, 9, 0);
    lv_obj_set_style_bg_opa(a.badge, LV_OPA_COVER, 0);
    a.badge_lbl = txt(a.badge, "", 0, 0, &ark12, C_BG);
    lv_obj_align(a.badge_lbl, LV_ALIGN_CENTER, 0, 0);
    show(a.badge, false);

    for (int k = 0; k < 3; k++) {
        a.dots[k] = dot(p, cx - 13 + k * 10, cy + bs / 2 + 6, 6, C_FAINT);
        lv_obj_set_style_radius(a.dots[k], 3, 0);
        show(a.dots[k], false);
    }

    a.bubble = obj(p, 0, 0, LV_SIZE_CONTENT, 22);
    lv_obj_update_layout(p);  // a fresh parent reports width 0 until laid out
    lv_obj_align(a.bubble, LV_ALIGN_TOP_MID, cx - lv_obj_get_content_width(p) / 2, bubble_y);
    lv_obj_set_style_bg_color(a.bubble, lv_color_hex(C_TEXT), 0);
    lv_obj_set_style_bg_opa(a.bubble, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(a.bubble, 6, 0);
    lv_obj_set_style_pad_hor(a.bubble, 9, 0);
    lv_obj_add_flag(a.bubble, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    a.bubble_lbl = txt(a.bubble, "", 0, 0, &ark12, C_BG);
    lv_obj_align(a.bubble_lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *tail = obj(a.bubble, 0, 0, 8, 5);  // points down at the avatar
    lv_obj_align(tail, LV_ALIGN_BOTTOM_MID, 0, 5);
    lv_obj_set_style_bg_color(tail, lv_color_hex(C_TEXT), 0);
    lv_obj_set_style_bg_opa(tail, LV_OPA_COVER, 0);
    show(a.bubble, false);

    a.big = big;
    a.sk = nullptr;
    a.mode = -1;
}

static void avatar_set(Avatar &a, const AgentSkin *sk) {
    if (!a.box || a.sk == sk) return;
    a.sk = sk;
    logo_set(a.logo, a.big ? sk->l : sk->m);
}

// "5 hours" -> 5h, "Gemini · 7 days" -> 7d (+ "Gemini" as group), "Credits" -> Credits
static void win_parse(const char *name, int *num, char *unit, char *group, size_t gn) {
    *num = 0;
    *unit = 0;
    group[0] = 0;
    const char *dot = strstr(name, " · ");
    if (dot) {
        size_t n = (size_t)(dot - name) < gn - 1 ? (size_t)(dot - name) : gn - 1;
        memcpy(group, name, n);
        group[n] = 0;
    }
    for (const char *s = name; *s; s++) {
        if (isdigit((unsigned char)*s)) {
            *num = atoi(s);
            while (isdigit((unsigned char)*s)) s++;
            while (*s == ' ') s++;
            *unit = (char)tolower((unsigned char)*s);
            return;
        }
    }
    if (!dot) {
        strncpy(group, name, gn - 1);
        group[gn - 1] = 0;
    }
}

static void win_short(const char *name, char *out, size_t n, bool with_group = true) {
    int num;
    char unit, group[12];
    win_parse(name, &num, &unit, group, sizeof(group));
    if (num && group[0] && with_group)
        snprintf(out, n, "%.6s %d%c", group, num, unit);
    else if (num)
        snprintf(out, n, "%d%c", num, unit);
    else
        snprintf(out, n, "%.8s", group);
}

// "5 小时额度" / "7 天额度" / "Credits"
static void win_long(const char *name, char *out, size_t n) {
    int num;
    char unit, group[12];
    win_parse(name, &num, &unit, group, sizeof(group));
    if (num && unit == 'h')
        snprintf(out, n, "%d 小时额度", num);
    else if (num && unit == 'd')
        snprintf(out, n, "%d 天额度", num);
    else if (num && unit == 'w')
        snprintf(out, n, "%d 周额度", num);
    else
        snprintf(out, n, "%s", group[0] ? group : name);
}

static uint32_t remain_col(float rem) {
    return rem >= 50 ? C_GREEN : (rem >= 20 ? C_AMBER : C_RED);
}

// ------------------------------------------------------------------ chrome
static lv_obj_t *make_page() {
    lv_obj_t *p = obj(lv_scr_act(), 0, 0, LCD_W, LCD_H);
    lv_obj_set_style_bg_color(p, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    return p;
}

static void home_btn(lv_obj_t *p) {
    lv_obj_t *b = obj(p, 4, 4, 36, 32);  // a little larger than the icon: easier to hit
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(b, 4);
    lv_obj_add_event_cb(b, [](lv_event_t *e) { page_request(PG_HOME); }, LV_EVENT_CLICKED, NULL);
    img(b, &home_icon, 2, 2);
}

static void tab_cb(lv_event_t *e) {
    page_request((int)(intptr_t)lv_event_get_user_data(e));
}

// header of pages 4..8: home, magpie mark, then Agent / 额度 / 用量 / 路由
// spread over the rest of the width like the design (and wide enough to hit)
// Tab geometry lives here only: the hit-test regression asks for the centres, so
// a layout change can never leave the test tapping yesterday's coordinates.
static const int TAB_X0 = 82, TAB_W = 76, TAB_GAP = 4;
int ui_tab_center_x(int index) { return TAB_X0 + index * (TAB_W + TAB_GAP) + TAB_W / 2; }

static void tabs(lv_obj_t *p, int active, int page_id) {
    static const char *names[5] = {"Agent", "额度", "用量", "路由", "设置"};
    static const int pages[5] = {PG_AGENTS, PG_QUOTA, PG_USAGE, PG_ROUTES, PG_SETTINGS};
    lv_obj_t *bm = img(p, &bird, 46, 10);
    lv_img_set_pivot(bm, 0, 0);
    lv_img_set_zoom(bm, 72);  // 98x76 -> 27x21 header mark
    const int x0 = TAB_X0, w = TAB_W, gap = TAB_GAP;
    for (int i = 0; i < 5; i++) {
        lv_obj_t *b = obj(p, x0 + i * (w + gap), 6, w, 28);
        lv_obj_set_style_radius(b, 3, 0);
        lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
        // 2 px of slack around the pill; the gap is 6 px, so neighbours cannot
        // steal it while taps a couple of pixels off still land.
        lv_obj_set_ext_click_area(b, 2);
        lv_obj_set_style_bg_color(b, lv_color_hex(C_PURPLE), 0);
        lv_obj_set_style_bg_opa(b, i == active ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        // visible feedback on press, so a tap never feels ignored
        lv_obj_set_style_bg_color(b, lv_color_hex(C_DIM), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_t *l = txt(b, names[i], 0, 0, &ark12, i == active ? C_TEXT : C_MUTED);
        lv_obj_center(l);
        if (i == 0) U.ag_tab[page_id] = l;  // "Agent N" carries a live count
        lv_obj_add_event_cb(b, tab_cb, LV_EVENT_CLICKED, (void *)(intptr_t)pages[i]);
    }
    hline(p, 0, 40, LCD_W, C_DIM);
}

// ------------------------------------------------------------------ page 1
static void build_home() {
    lv_obj_t *p = pg[PG_HOME] = make_page();

    U.hm_handle = txt(p, "@Dreamfarer", 10, 7, &ark12, C_MUTED);
    lv_obj_set_width(U.hm_handle, 190);
    lv_label_set_long_mode(U.hm_handle, LV_LABEL_LONG_DOT);
    // tapping the name is the second way into settings (the tab bar is the first)
    lv_obj_add_flag(U.hm_handle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(U.hm_handle, 6);
    lv_obj_add_event_cb(U.hm_handle, [](lv_event_t *e) { page_request(PG_SETTINGS); },
                        LV_EVENT_CLICKED, NULL);
    U.date = txt(p, "--/-- ---", 0, 9, &ark12, C_MUTED);
    lv_obj_align(U.date, LV_ALIGN_TOP_RIGHT, -86, 9);
    U.clock = txt(p, "-- --", 0, 2, &ark24, C_TEXT);
    lv_obj_align(U.clock, LV_ALIGN_TOP_RIGHT, -10, 2);

    // Touch-only gestures (this panel has no usable BOOT button - GPIO0 is the
    // LCD TE line). Long-press the header strip to open the calibration screen;
    // it carries its own "RESET WIFI" button, so no secret long-press is needed
    // for networking.
    lv_obj_t *hdr = obj(p, 0, 0, LCD_W, 28);
    lv_obj_add_flag(hdr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(
        hdr,
        [](lv_event_t *e) {
            Serial.println("[ui] header long-press -> quick calibration");
            ui_quickcal_begin();
        },
        LV_EVENT_LONG_PRESSED, NULL);

    // ---- left card: the agent at work, as its avatar (tap it to poke it,
    // tap anywhere else on the card for its status pages)
    lv_obj_t *cl = card(p, 8, 34, 228, 230);
    U.hm_accent = obj(cl, 0, 0, 228, 3);
    lv_obj_set_style_bg_color(U.hm_accent, lv_color_hex(C_ORANGE), 0);
    lv_obj_set_style_bg_opa(U.hm_accent, LV_OPA_COVER, 0);
    avatar_create(s_av_home, cl, 114, 64, false, 8);
    U.hm_name = txt(cl, "Claude Code", 0, 0, &ark16, C_TEXT);
    lv_obj_align(U.hm_name, LV_ALIGN_TOP_MID, 0, 122);
    lv_obj_t *st = hrow(cl, 0, 143, 226, 16, 6, LV_FLEX_ALIGN_CENTER);
    U.hm_dot = dot(st, 0, 0, 6, C_ORANGE);
    lv_obj_set_style_radius(U.hm_dot, 3, 0);
    U.hm_state = txt(st, "等待网关", 0, 0, &ark12, C_MUTED);
    dline(cl, 10, 166, 206);
    U.hm_qlbl = txt(cl, "5h", 10, 176, &ark12, C_MUTED);
    U.hm_bar = segbar(cl, 38, 176, 140, 12, 12, C_GREEN);
    U.hm_pct = txt(cl, "--", 0, 0, &ark12, C_GREEN);
    lv_obj_align(U.hm_pct, LV_ALIGN_TOP_RIGHT, -10, 176);
    U.hm_stats = txt_clip(cl, "", 10, 200, 206, &ark12, C_MUTED);
    ui_clicks(cl);
    lv_obj_set_style_border_color(cl, lv_color_hex(C_MUTED), LV_STATE_PRESSED);
    lv_obj_add_event_cb(cl, [](lv_event_t *e) { page_request(PG_STATE_THINK); }, LV_EVENT_CLICKED, NULL);

    // ---- right card: magpie
    lv_obj_t *cr = card(p, 244, 34, 228, 230);
    lv_obj_t *accent2 = obj(cr, 0, 0, 228, 3);
    lv_obj_set_style_bg_color(accent2, lv_color_hex(C_PURPLE), 0);
    lv_obj_set_style_bg_opa(accent2, LV_OPA_COVER, 0);
    img(cr, &bird, 65, 40);
    lv_obj_t *t2 = txt(cr, "magpie", 0, 0, &ark16, C_TEXT);
    lv_obj_align(t2, LV_ALIGN_TOP_MID, 0, 146);
    lv_obj_t *mr = hrow(cr, 0, 168, 226, 16, 6, LV_FLEX_ALIGN_CENTER);
    dot(mr, 0, 0, 6, C_GREEN);
    U.hm_mag = txt(mr, "今天 -- 次 · --", 0, 0, &ark12, C_MUTED);
    dline(cr, 10, 192, 206);
    txt(cr, "今天", 10, 203, &ark12, C_MUTED);
    U.hm_tick = segbar(cr, 44, 206, 172, 6, 24, C_PURPLE);
    ui_clicks(cr);
    lv_obj_set_style_border_color(cr, lv_color_hex(C_MUTED), LV_STATE_PRESSED);
    lv_obj_add_event_cb(cr, [](lv_event_t *e) { page_request(PG_AGENTS); }, LV_EVENT_CLICKED, NULL);
}

// ------------------------------------------------------------------ state pages
static void build_state(int idx, int page_id, bool confirm) {
    lv_obj_t *p = pg[page_id] = make_page();
    if (confirm) {
        lv_obj_set_style_border_width(p, 2, 0);
        lv_obj_set_style_border_color(p, lv_color_hex(C_AMBER), 0);
    }
    home_btn(p);
    avatar_create(s_av_state[idx], p, 118, 128, true, 54);
    avatar_mode(s_av_state[idx], idx == 0 ? AV_THINK : (idx == 1 ? AV_CODE : AV_CONFIRM));

    // three pips under the avatar: which state page this is, tap one to switch
    for (int k = 0; k < 3; k++) {
        lv_obj_t *hit = obj(p, 118 - 66 + k * 44, 222, 44, 36);
        lv_obj_add_flag(hit, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(hit, tab_cb, LV_EVENT_CLICKED, (void *)(intptr_t)(PG_STATE_THINK + k));
        lv_obj_t *pip = U.st_pip[idx][k] = obj(hit, k == idx ? 6 : 15, 15, k == idx ? 32 : 14, 6);
        lv_obj_set_style_radius(pip, 3, 0);
        lv_obj_set_style_bg_color(pip, lv_color_hex(C_DIM), 0);
        lv_obj_set_style_bg_opa(pip, LV_OPA_COVER, 0);
    }

    card(p, 234, 8, 238, 256);
    U.st_agent[idx] = txt_clip(p, "CLAUDE CODE", 244, 15, 150, &ark12, C_ORANGE);
    U.st_sess[idx] = txt(p, "-- 个会话", 0, 0, &ark12, C_MUTED);
    lv_obj_align(U.st_sess[idx], LV_ALIGN_TOP_RIGHT, -18, 15);

    lv_obj_t *tr = hrow(p, 244, 32, 220, 28, 4, LV_FLEX_ALIGN_START);
    U.st_star[idx] = txt(tr, "★", 0, 0, &ark24, C_ORANGE);
    show(U.st_star[idx], !confirm);
    U.st_title[idx] = txt(tr, "斟酌中", 0, 0, &ark24, C_ORANGE);
    U.st_sub[idx] = txt_clip(p, "", 244, 64, 218, &ark12, C_MUTED);
    dline(p, 244, 84, 218);

    U.st_l5[idx] = txt(p, "5 小时额度", 244, 98, &ark12, C_TEXT);
    U.st_p5[idx] = txt(p, "--", 0, 0, &ark24, C_GREEN);
    lv_obj_align(U.st_p5[idx], LV_ALIGN_TOP_RIGHT, -18, 92);
    U.st_bar5[idx] = segbar(p, 244, 120, 218, 12, 20, C_GREEN);

    U.st_l7[idx] = txt(p, "7 天额度", 244, 146, &ark12, C_TEXT);
    U.st_p7[idx] = txt(p, "--", 0, 0, &ark24, C_GREEN);
    lv_obj_align(U.st_p7[idx], LV_ALIGN_TOP_RIGHT, -18, 140);
    U.st_bar7[idx] = segbar(p, 244, 168, 218, 12, 20, C_GREEN);

    U.st_reset[idx] = txt(p, "", 244, 188, &ark12, C_MUTED);
    U.st_src[idx] = txt_clip(p, "", 244, 206, 218, &ark12, C_MUTED);
    lv_obj_t *cn = hrow(p, 244, 228, 200, 16, 6, LV_FLEX_ALIGN_START);
    dot(cn, 0, 0, 6, C_GREEN);
    U.st_conn[idx] = txt(cn, "未连接", 0, 0, &ark12, C_MUTED);

}

// ------------------------------------------------------------------ agents
// Tapping a row opens the agent's sheet: its avatar (pokeable), what it ran
// in the last 30 days, and a button to follow it on the home card and the
// state pages.
static void sheet_fill();

static void sheet_open(const char *id) {
    const uint32_t start_ms = millis();
    if (dbg_verbose) Serial.printf("[agenttrace] %u sheet-open-begin id=%s\n", (unsigned)start_ms, id);
    strncpy(U.sh_id, id, sizeof(U.sh_id) - 1);
    U.sh_id[sizeof(U.sh_id) - 1] = 0;
    sheet_fill();
    show(U.sh, true);
    // Opening details should be stable immediately. Keep the hop as an explicit
    // avatar-poke interaction, not a 450ms animation on every list selection.
    lv_anim_del(s_av_sheet.box, anim_translate_y);
    lv_obj_set_style_translate_y(s_av_sheet.box, 0, 0);
    avatar_say(s_av_sheet, s_av_sheet.sk->say[0], 1800);
    Serial.printf("[ui] sheet %s\n", U.sh_id);
    if (dbg_verbose) Serial.printf("[agenttrace] %u sheet-shown callback=%u ms\n",
                                   (unsigned)millis(), (unsigned)(millis() - start_ms));
}

static void sheet_close() {
    show(U.sh, false);
    U.sh_id[0] = 0;
}

static void agent_input_trace(lv_event_t *e) {
    if (!dbg_verbose) return;
    const lv_event_code_t code = lv_event_get_code(e);
    const char *name = nullptr;
    if (code == LV_EVENT_PRESSED) name = "pressed";
    else if (code == LV_EVENT_RELEASED) name = "released";
    else if (code == LV_EVENT_PRESS_LOST) name = "press-lost";
    else if (code == LV_EVENT_CLICKED) name = "clicked";
    else if (code == LV_EVENT_SCROLL_BEGIN) name = "scroll-begin";
    else if (code == LV_EVENT_SCROLL_END) name = "scroll-end";
    if (!name) return;
    lv_point_t pt{};
    lv_indev_t *input = lv_indev_get_act();
    if (input) lv_indev_get_point(input, &pt);
    Serial.printf("[agenttrace] %u row=%d event=%s xy=%d,%d scroll_y=%d\n",
                  (unsigned)millis(), (int)(intptr_t)lv_event_get_user_data(e), name,
                  pt.x, pt.y, U.ag_list ? (int)lv_obj_get_scroll_y(U.ag_list) : 0);
}

static void agent_row_cb(lv_event_t *e) {
    const int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (i < 0 || i >= AG_ROWS || !U.ag_id[i][0]) return;
    sheet_open(U.ag_id[i]);
}

static void sheet_follow_cb(lv_event_t *e) {
    if (!U.sh_id[0]) return;
    const bool same = !strcmp(s_focus, U.sh_id);
    if (same) {  // following it already: the button lets go again
        s_focus[0] = 0;
    } else {
        strcpy(s_focus, U.sh_id);
        s_focus_ms = millis();
    }
    Serial.printf("[ui] focus agent %s\n", s_focus[0] ? s_focus : "(latest)");
    sheet_close();
    if (s_data) ui_update(*s_data);
    if (!same) page_request(PG_STATE_THINK);
}

static lv_obj_t *button(lv_obj_t *p, int x, int y, int w, const char *s, uint32_t bg, uint32_t col) {
    lv_obj_t *b = obj(p, x, y, w, 30);
    lv_obj_set_style_radius(b, 4, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(b, bg == C_CARD ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(bg == C_CARD ? C_LINE : bg), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_DIM), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *l = txt(b, s, 0, 0, &ark12, col);
    lv_obj_align(l, LV_ALIGN_CENTER, 0, 0);
    return b;
}

static void build_agents() {
    lv_obj_t *p = pg[PG_AGENTS] = make_page();
    home_btn(p);
    tabs(p, 0, PG_AGENTS);

    // every agent magpie manages: more than fit, so the list scrolls
    lv_obj_t *c = card(p, 6, 46, 468, 220);
    lv_obj_t *sc = U.ag_list = obj(c, 0, 2, 466, 214);
    lv_obj_add_flag(sc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(sc, agent_input_trace, LV_EVENT_ALL, (void *)(intptr_t)-1);
    lv_obj_set_scroll_dir(sc, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sc, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_style_width(sc, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(sc, lv_color_hex(C_MUTED), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(sc, LV_OPA_60, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(sc, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < AG_ROWS; i++) {
        lv_obj_t *r = U.ag_row[i] = obj(sc, 0, 0, 466, 34);
        lv_obj_add_flag(r, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(r, lv_color_hex(C_DIM), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(r, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_add_event_cb(r, agent_input_trace, LV_EVENT_ALL, (void *)(intptr_t)i);
        lv_obj_add_event_cb(r, agent_row_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        if (i) dline(r, 10, 0, 446);
        U.ag_icon[i] = logo(r);
        lv_obj_set_pos(U.ag_icon[i], 12, 7);
        U.ag_live[i] = dot(r, 29, 24, 7, C_GREEN);  // working right now
        lv_obj_set_style_radius(U.ag_live[i], 4, 0);
        lv_obj_set_style_border_width(U.ag_live[i], 1, 0);
        lv_obj_set_style_border_color(U.ag_live[i], lv_color_hex(C_CARD), 0);
        U.ag_name[i] = txt(r, "--", 44, 9, &ark16, C_TEXT);
        lv_obj_t *rt = hrow(r, 200, 0, 254, 34, 8, LV_FLEX_ALIGN_END);
        U.ag_detail[i] = txt(rt, "", 0, 0, &ark12, C_MUTED);
        lv_label_set_long_mode(U.ag_detail[i], LV_LABEL_LONG_DOT);
        lv_obj_set_style_max_width(U.ag_detail[i], 180, 0);
        U.ag_state[i] = txt(rt, "", 0, 0, &ark12, C_MUTED);
        U.ag_calls[i] = txt(rt, "", 0, 0, &ark12, C_GREEN);
        show(r, false);
    }

    // ---- the sheet, over the list
    lv_obj_t *s = U.sh = card(p, 6, 46, 468, 220);
    lv_obj_add_flag(s, LV_OBJ_FLAG_CLICKABLE);  // swallow taps meant for rows below
    avatar_create(s_av_sheet, s, 66, 96, false, 38);
    avatar_mode(s_av_sheet, AV_IDLE);
    lv_obj_t *hint = txt(s, "轻触头像", 0, 0, &ark12, C_FAINT);
    lv_obj_align(hint, LV_ALIGN_TOP_LEFT, 66 - 24, 152);

    U.sh_name = txt_clip(s, "--", 140, 12, 220, &ark16, C_TEXT);
    U.sh_model = txt_clip(s, "", 140, 34, 316, &ark12, C_MUTED);
    static const char *labels[4] = {"30 天调用", "今天", "Token", "费用"};
    for (int k = 0; k < 4; k++) {
        const int x = 140 + (k % 2) * 160, y = 58 + (k / 2) * 44;
        lv_obj_t *cell = obj(s, x, y, 150, 38);
        lv_obj_set_style_radius(cell, 4, 0);
        lv_obj_set_style_bg_color(cell, lv_color_hex(C_BG), 0);
        lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
        U.sh_num[k] = txt(cell, "--", 8, 3, &ark16, C_TEXT);
        txt(cell, labels[k], 8, 22, &ark12, C_MUTED);
    }
    U.sh_share = txt(s, "", 140, 152, &ark12, C_MUTED);
    U.sh_bar = segbar(s, 140, 170, 316, 6, 24, C_PURPLE);
    U.sh_last = txt_clip(s, "", 140, 190, 140, &ark12, C_MUTED);
    lv_obj_t *cb = button(s, 290, 182, 70, "关闭", C_CARD, C_MUTED);
    lv_obj_add_event_cb(cb, [](lv_event_t *e) { sheet_close(); }, LV_EVENT_CLICKED, NULL);
    U.sh_follow = button(s, 368, 182, 90, "跟随", C_PURPLE, C_TEXT);
    lv_obj_add_event_cb(U.sh_follow, sheet_follow_cb, LV_EVENT_CLICKED, NULL);
    show(s, false);
}

void ui_sheet_benchmark() {
    Serial.println("[sheettest] BEGIN real row taps, first-frame completion and next 500ms");
    int cases = 0, failed = 0;
    ui_goto(PG_AGENTS);
    for (int trial = 0; trial < 6; trial++) {
        sheet_close();
        lv_obj_scroll_to_y(U.ag_list, 0, LV_ANIM_OFF);
        lv_refr_now(nullptr);
        lv_obj_update_layout(lv_scr_act());
        const int row = trial % 3;
        if (!U.ag_id[row][0]) continue;
        lv_area_t area;
        lv_obj_get_coords(U.ag_row[row], &area);
        const uint32_t t0 = millis();
        display_test_tap(area.x1 + 100, (area.y1 + area.y2) / 2);
        lv_refr_now(nullptr);
        bool drained = display_wait_idle(2000);
        const uint32_t first = millis() - t0;
        const bool opened = !lv_obj_has_flag(U.sh, LV_OBJ_FLAG_HIDDEN) &&
                            !strcmp(U.sh_id, U.ag_id[row]);
        const bool auto_hop = lv_anim_get(s_av_sheet.box, anim_translate_y) != nullptr;
        uint32_t busy_us = 0, tick_max_us = 0;
        const uint32_t observe_start = millis();
        while (millis() - observe_start < 500) {
            const uint32_t a = micros();
            display_tick();
            const uint32_t dt = micros() - a;
            busy_us += dt;
            if (dt > tick_max_us) tick_max_us = dt;
            delay(5);
        }
        lv_area_t close_area;
        // button() creates the close button immediately before follow.
        lv_obj_t *close_button = lv_obj_get_child(U.sh, lv_obj_get_child_cnt(U.sh) - 2);
        lv_obj_get_coords(close_button, &close_area);
        display_test_tap((close_area.x1 + close_area.x2) / 2,
                         (close_area.y1 + close_area.y2) / 2);
        const bool closed = lv_obj_has_flag(U.sh, LV_OBJ_FLAG_HIDDEN);
        cases++;
        // Opening details should show data, not launch a blocking ornamental hop.
        const bool pass = opened && closed && drained && !auto_hop;
        if (!pass) failed++;
        Serial.printf("[sheettest] %s trial=%d row=%d opened=%d closed=%d first_incl_hold=%u ms auto_hop=%d busy_500ms=%u ms max_tick=%u ms\n",
                      pass ? "PASS" : "FAIL", trial, row, opened, closed,
                      (unsigned)first, auto_hop, (unsigned)(busy_us / 1000),
                      (unsigned)(tick_max_us / 1000));
    }
    sheet_close();
    ui_goto(PG_AGENTS);
    Serial.printf("[sheettest] END total=%d failed=%d\n", cases, failed);
}

// ------------------------------------------------------------------ quota
static void build_quota() {
    lv_obj_t *p = pg[PG_QUOTA] = make_page();
    home_btn(p);
    tabs(p, 1, PG_QUOTA);

    // one block per provider (header line + card); more than two do not fit,
    // so they scroll under the pinned tabs
    lv_obj_t *sc = obj(p, 0, 42, LCD_W, LCD_H - 42);
    lv_obj_add_flag(sc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(sc, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sc, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_style_width(sc, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(sc, lv_color_hex(C_MUTED), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(sc, LV_OPA_60, LV_PART_SCROLLBAR);
    lv_obj_set_style_pad_bottom(sc, 6, 0);
    U.q_empty = txt(sc, "暂无额度数据", 0, 0, &ark12, C_MUTED);
    lv_obj_align(U.q_empty, LV_ALIGN_TOP_MID, 0, 90);

    for (int i = 0; i < QUOTA_MAX_PROV; i++) {
        lv_obj_t *b = U.q_blk[i] = obj(sc, 0, i * 104, LCD_W, 102);
        lv_obj_add_flag(b, LV_OBJ_FLAG_EVENT_BUBBLE);
        U.q_dot[i] = dot(b, 10, 10, 6, C_ORANGE);
        U.q_name[i] = txt(b, "--", 22, 5, &ark12, C_MUTED);
        lv_obj_t *left = txt(b, "剩余", 0, 0, &ark12, C_TEXT);
        U.q_word[i] = left;
        lv_obj_set_style_bg_color(left, lv_color_hex(C_DIM), 0);
        lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(left, 2, 0);
        lv_obj_set_style_pad_hor(left, 4, 0);
        lv_obj_align(left, LV_ALIGN_TOP_RIGHT, -8, 4);
        U.q_plan[i] = txt(b, "", 0, 0, &ark12, C_MUTED);
        lv_obj_align(U.q_plan[i], LV_ALIGN_TOP_RIGHT, -48, 5);

        lv_obj_t *c = card(b, 6, 24, 468, 74);
        lv_obj_add_flag(c, LV_OBJ_FLAG_EVENT_BUBBLE);
        U.q_user[i] = txt_clip(c, "--", 12, 12, 220, &ark16, C_TEXT);
        U.q_reset[i] = txt_clip(c, "", 12, 44, 220, &ark12, C_MUTED);
        for (int j = 0; j < QUOTA_MAX_WIN; j++) {
            const int x = 466 - 10 - (QUOTA_MAX_WIN - j) * 54 + 6;
            U.q_ring[i][j] = ring(c, x, 6, 44, C_GREEN);
            // child of the arc + style alignment: stays centred as the text changes
            U.q_pct[i][j] = txt(U.q_ring[i][j], "", 0, 0, &ark10, C_TEXT);
            lv_obj_center(U.q_pct[i][j]);
            U.q_win[i][j] = txt(c, "", x - 5, 52, &ark10, C_MUTED);
            lv_obj_set_width(U.q_win[i][j], 54);
            lv_obj_set_style_text_align(U.q_win[i][j], LV_TEXT_ALIGN_CENTER, 0);
        }
        show(b, false);
    }
}

// ------------------------------------------------------------------ usage
static lv_obj_t *u_filter[4] = {nullptr, nullptr, nullptr, nullptr};

static int s_filter = 2;  // 今天/7 天/30 天/全部, mirrors gw period

static void filter_apply(int idx) {
    static const char *periods[4] = {"today", "7d", "30d", "all"};
    if (idx < 0 || idx > 3) return;
    gw_set_period(periods[idx]);
    s_filter = idx;
    for (int i = 0; i < 4; i++) {
        if (!u_filter[i]) continue;
        const bool sel = i == idx;
        lv_obj_set_style_bg_opa(u_filter[i], sel ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(u_filter[i], lv_color_hex(sel ? C_PURPLE : C_LINE), 0);
        lv_obj_t *l = lv_obj_get_child(u_filter[i], 0);
        if (l) lv_obj_set_style_text_color(l, lv_color_hex(sel ? C_TEXT : C_MUTED), 0);
    }
    app_request_refresh();
}

void ui_cycle_period() {
    filter_apply((s_filter + 1) % 4);
    static const char *names[4] = {"today", "7d", "30d", "all"};
    Serial.printf("[ui] usage period -> %s\n", names[s_filter]);
}

static void filter_cb(lv_event_t *e) {
    const int idx = (int)(intptr_t)lv_event_get_user_data(e);
    static const char *periods[4] = {"today", "7d", "30d", "all"};
    filter_apply(idx);
}

#define CH_X0 44
#define CH_W 420
#define CH_BASE 246
#define CH_H 44

static void build_usage() {
    lv_obj_t *p = pg[PG_USAGE] = make_page();
    home_btn(p);
    tabs(p, 2, PG_USAGE);

    static const char *fname[4] = {"今天", "7 天", "30 天", "全部"};
    int x = 6;
    for (int i = 0; i < 4; i++) {
        const int w = 52;
        lv_obj_t *b = obj(p, x, 47, w, 24);
        lv_obj_set_style_radius(b, 3, 0);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_style_border_color(b, lv_color_hex(i == 2 ? C_PURPLE : C_LINE), 0);
        lv_obj_set_style_bg_color(b, lv_color_hex(C_PURPLE), 0);
        lv_obj_set_style_bg_opa(b, i == 2 ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_t *l = txt(b, fname[i], 0, 0, &ark12, i == 2 ? C_TEXT : C_MUTED);
        lv_obj_center(l);
        lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(b, filter_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        u_filter[i] = b;
        x += w + 6;
    }
    lv_obj_t *src = txt(p, "网关 + 会话日志", 0, 0, &ark12, C_MUTED);
    lv_obj_align(src, LV_ALIGN_TOP_RIGHT, -8, 53);

    // 4 KPI blocks
    static const char *titles[4] = {"Token", "请求", "费用", "缓存命中率"};
    const uint32_t vals_col[4] = {C_PURPLE, C_TEXT, C_GREEN, C_TEXT};
    lv_obj_t *v[4], *c[4];
    for (int i = 0; i < 4; i++) {
        c[i] = card(p, 6 + i * 118, 78, 112, 104);
        txt(c[i], titles[i], 8, 6, &ark12, C_MUTED);
        v[i] = txt(c[i], "--", 8, 24, &ark24, vals_col[i]);
    }
    U.u_tok = v[0];
    U.u_calls = v[1];
    U.u_cost = v[2];
    U.u_hit = v[3];
    U.u_in = txt(c[0], "", 8, 54, &ark12, C_MUTED);
    U.u_out = txt(c[0], "", 8, 67, &ark12, C_MUTED);
    U.u_cache = txt(c[0], "", 8, 80, &ark12, C_MUTED);
    U.u_err = txt(c[1], "", 8, 60, &ark12, C_RED);
    U.u_price = txt(c[2], "", 8, 60, &ark12, C_MUTED);  // filled with the average cost
    U.u_hit2 = txt(c[3], "占提示词", 8, 60, &ark12, C_MUTED);

    // bar chart: hand-drawn bars (the lv_chart widget drew outside its own area
    // on this panel and could not be clipped reliably)
    card(p, 6, 188, 468, 78);
    U.u_ymax = txt(p, "--", 12, 194, &ark10, C_MUTED);
    txt(p, "0", 12, CH_BASE - 8, &ark10, C_MUTED);
    for (int i = 0; i < SERIES_MAX; i++) {
        U.u_bars[i] = obj(p, CH_X0, CH_BASE - 2, 8, 2);
        lv_obj_set_style_bg_color(U.u_bars[i], lv_color_hex(C_PURPLE), 0);
        lv_obj_set_style_bg_opa(U.u_bars[i], LV_OPA_COVER, 0);
        show(U.u_bars[i], false);
    }
    for (int i = 0; i < 5; i++) {
        U.u_lbl[i] = txt(p, "", 0, CH_BASE + 4, &ark10, C_MUTED);
        lv_obj_set_width(U.u_lbl[i], 40);
        lv_obj_set_style_text_align(U.u_lbl[i], LV_TEXT_ALIGN_CENTER, 0);
    }
    U.u_empty = txt(p, "暂无数据", 0, 0, &ark12, C_MUTED);
    lv_obj_align(U.u_empty, LV_ALIGN_TOP_MID, 0, 216);
}

// ------------------------------------------------------------------ routes
static void build_routes(int page_id, int rows) {
    lv_obj_t *p = pg[page_id] = make_page();
    home_btn(p);
    tabs(p, 3, page_id);

    const int idx = (page_id == PG_ROUTES) ? 0 : 1;
    U.r_sum[idx] = txt(p, "今天 -- 次调用 · -- token", 10, 49, &ark12, C_TEXT);

    int y0;
    if (page_id == PG_ROUTES) {
        // topology strip: agent -> magpie -> upstream account
        card(p, 6, 70, 468, 40);
        lv_obj_t *a = hrow(p, 14, 78, 130, 24, 6, LV_FLEX_ALIGN_START);
        lv_obj_set_style_border_width(a, 1, 0);
        lv_obj_set_style_border_color(a, lv_color_hex(C_PURPLE), 0);
        lv_obj_set_style_radius(a, 3, 0);
        lv_obj_set_style_pad_hor(a, 6, 0);
        U.r_top_icon = logo(a);
        U.r_top_agent = txt_clip(a, "--", 0, 0, 92, &ark12, C_TEXT);
        dline(p, 148, 90, 68);
        lv_obj_t *g = obj(p, 222, 74, 36, 32);
        lv_obj_set_style_radius(g, 3, 0);
        lv_obj_set_style_bg_color(g, lv_color_hex(C_TEXT), 0);
        lv_obj_set_style_bg_opa(g, LV_OPA_COVER, 0);
        lv_obj_t *gm = img(g, &bird, 4, 4);
        lv_img_set_pivot(gm, 0, 0);
        lv_img_set_zoom(gm, 75);
        lv_obj_set_style_img_recolor(gm, lv_color_hex(C_BG), 0);
        lv_obj_set_style_img_recolor_opa(gm, LV_OPA_COVER, 0);
        dline(p, 262, 90, 26);
        lv_obj_t *b = hrow(p, 292, 78, 174, 24, 6, LV_FLEX_ALIGN_START);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_style_border_color(b, lv_color_hex(C_LINE), 0);
        lv_obj_set_style_radius(b, 3, 0);
        lv_obj_set_style_pad_hor(b, 6, 0);
        dot(b, 0, 0, 6, C_GREEN);
        U.r_top_to = txt_clip(b, "--", 0, 0, 148, &ark12, C_MUTED);
        y0 = 116;
    } else {
        y0 = 70;
    }

    // one card holding every row, separated by dashed rules (like the design).
    // More rows than fit => the rows live in a scroll area (agents list pattern).
    const int step = 46;
    lv_obj_t *c;
    lv_obj_t *host;
    if (rows > 2) {
        c = card(p, 6, y0, 468, LCD_H - y0 - 6);
        host = obj(c, 0, 2, 466, LCD_H - y0 - 12);
        // Dragging needs CLICKABLE, and the bar must be always-on: with ACTIVE
        // mode it stayed invisible until dragged, so nobody knew there were more
        // rows than the four that fit.
        lv_obj_add_flag(host, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_scroll_dir(host, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(host, LV_SCROLLBAR_MODE_ON);
        lv_obj_set_style_width(host, 4, LV_PART_SCROLLBAR);
        lv_obj_set_style_bg_color(host, lv_color_hex(C_DIM), LV_PART_SCROLLBAR);
        lv_obj_set_style_bg_opa(host, LV_OPA_COVER, LV_PART_SCROLLBAR);
        U.r_scroll[idx] = host;
    } else {
        c = card(p, 6, y0, 468, rows * step + 4);
        host = c;
    }
    U.r_empty[idx] = txt(host, "今日暂无调用", 0, 0, &ark12, C_MUTED);
    lv_obj_center(U.r_empty[idx]);
    for (int i = 0; i < RT_ROWS; i++) {
        U.r_row[idx][i] = nullptr;
        if (i >= rows) continue;
        lv_obj_t *r = U.r_row[idx][i] = obj(host, 0, 2 + i * step, 466, step);
        if (i) dline(r, 10, 0, 446);
        lv_obj_t *l1 = hrow(r, 10, 5, 380, 20, 6, LV_FLEX_ALIGN_START);
        U.r_icon[idx][i] = logo(l1);
        U.r_agent[idx][i] = txt(l1, "--", 0, 0, &ark16, C_TEXT);
        U.r_model[idx][i] = chip(l1, "--", C_LINE, C_TEXT);
        lv_label_set_long_mode(U.r_model[idx][i], LV_LABEL_LONG_DOT);
        lv_obj_set_style_max_width(U.r_model[idx][i], 170, 0);
        U.r_tag[idx][i] = chip(l1, "", C_LINE, C_MUTED);
        U.r_time[idx][i] = txt(r, "--:--:--", 0, 0, &ark12, C_MUTED);
        lv_obj_align(U.r_time[idx][i], LV_ALIGN_TOP_RIGHT, -10, 8);
        U.r_flow[idx][i] = txt_clip(r, "", 30, 29, 250, &ark12, C_MUTED);
        U.r_use[idx][i] = txt(r, "", 0, 0, &ark12, C_MUTED);
        lv_obj_align(U.r_use[idx][i], LV_ALIGN_TOP_RIGHT, -10, 29);
        show(r, false);
    }
}

// ------------------------------------------------------------------ clicks
static void ui_clicks(lv_obj_t *o) { lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE); }


// ---- calibration overlay: white frame + centre cross + corner blocks.
// Used to check whether the panel shows the frame buffer 1:1.
static lv_obj_t *s_cal = nullptr;
static lv_obj_t *s_cal_dot = nullptr;


static lv_obj_t *s_test = nullptr;
static lv_obj_t *s_test_dot = nullptr;

// Serial 'T': four fixed colour quadrants plus a dot that follows the mapped
// touch point. One look answers both open questions at once — which way the
// panel maps red/blue, and whether touch X/Y is mirrored.
void ui_test_toggle() {
    if (s_test) {
        lv_obj_del(s_test);
        s_test = nullptr;
        s_test_dot = nullptr;
        ui_goto(ui_cur_page());
        Serial.println("[test] pattern off");
        return;
    }
    s_test = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_test);
    lv_obj_set_pos(s_test, 0, 0);
    lv_obj_set_size(s_test, LCD_W, LCD_H);
    lv_obj_clear_flag(s_test, LV_OBJ_FLAG_SCROLLABLE);
    static const uint32_t col[4] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF};
    static const char *nm[4] = {"TL RED", "TR GREEN", "BL BLUE", "BR WHITE"};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *r = lv_obj_create(s_test);
        lv_obj_remove_style_all(r);
        lv_obj_set_pos(r, (i % 2) * (LCD_W / 2), (i / 2) * (LCD_H / 2));
        lv_obj_set_size(r, LCD_W / 2, LCD_H / 2);
        lv_obj_set_style_bg_color(r, lv_color_hex(col[i]), 0);
        lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
        lv_obj_t *t = lv_label_create(r);
        lv_label_set_text(t, nm[i]);
        lv_obj_set_style_text_font(t, &ark24, 0);
        lv_obj_set_style_text_color(t, lv_color_hex(0x000000), 0);
        lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);
    }
    s_test_dot = lv_obj_create(s_test);
    lv_obj_remove_style_all(s_test_dot);
    lv_obj_set_size(s_test_dot, 12, 12);
    lv_obj_set_style_radius(s_test_dot, 6, 0);
    lv_obj_set_style_bg_color(s_test_dot, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_bg_opa(s_test_dot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_test_dot, 2, 0);
    lv_obj_set_style_border_color(s_test_dot, lv_color_hex(0x000000), 0);
    lv_obj_add_flag(s_test_dot, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    Serial.println("[test] TL=RED TR=GREEN BL=BLUE BR=WHITE; yellow dot follows the touch point");
}

// A red dot that follows the finger on the calibration screen: lets the user
// (and the serial log) see immediately whether raw touch tracks the screen.
void ui_cal_dot_update(int x, int y) {
    if (s_test && s_test_dot) {  // test pattern: yellow dot follows the finger
        if (x >= 0 && y >= 0 && x < LCD_W && y < LCD_H) {
            lv_obj_set_pos(s_test_dot, x - 6, y - 6);
            lv_obj_clear_flag(s_test_dot, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }
    if (!s_cal || !s_cal_dot) return;
    if (x < 0 || y < 0 || x >= LCD_W || y >= LCD_H) return;
    lv_obj_set_pos(s_cal_dot, x - 5, y - 5);
    lv_obj_clear_flag(s_cal_dot, LV_OBJ_FLAG_HIDDEN);
}

void ui_show_calibration(bool on) {
    if (!on) {
        touch_calibration_end();
        if (s_cal) {
            lv_obj_del(s_cal);
            s_cal = nullptr;
            s_cal_dot = nullptr;
        }
        return;
    }
    if (s_cal) return;
    s_cal = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_cal);
    lv_obj_set_pos(s_cal, 0, 0);
    lv_obj_set_size(s_cal, LCD_W, LCD_H);
    lv_obj_clear_flag(s_cal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_cal, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(s_cal, LV_OPA_COVER, 0);
    // 2 px white frame
    lv_obj_t *f = lv_obj_create(s_cal);
    lv_obj_remove_style_all(f);
    lv_obj_set_pos(f, 0, 0);
    lv_obj_set_size(f, LCD_W, LCD_H);
    lv_obj_set_style_bg_opa(f, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(f, 2, 0);
    lv_obj_set_style_border_color(f, lv_color_hex(0xFFFFFF), 0);
    // centre cross
    lv_obj_t *h = lv_obj_create(s_cal);
    lv_obj_remove_style_all(h);
    lv_obj_set_pos(h, 0, LCD_H / 2 - 1);
    lv_obj_set_size(h, LCD_W, 2);
    lv_obj_set_style_bg_color(h, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(h, LV_OPA_COVER, 0);
    lv_obj_t *v = lv_obj_create(s_cal);
    lv_obj_remove_style_all(v);
    lv_obj_set_pos(v, LCD_W / 2 - 1, 0);
    lv_obj_set_size(v, 2, LCD_H);
    lv_obj_set_style_bg_color(v, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(v, LV_OPA_COVER, 0);
    // corner blocks + labels
    const int cx[4] = {2, LCD_W - 26, 2, LCD_W - 26};
    const int cy[4] = {2, 2, LCD_H - 26, LCD_H - 26};
    const char *nm[4] = {"1 TL", "2 TR", "3 BL", "4 BR"};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *b = lv_obj_create(s_cal);
        lv_obj_remove_style_all(b);
        lv_obj_set_pos(b, cx[i], cy[i]);
        lv_obj_set_size(b, 24, 24);
        lv_obj_set_style_bg_color(b, lv_color_hex(C_ORANGE), 0);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
        lv_obj_t *t = lv_label_create(s_cal);
        lv_label_set_text(t, nm[i]);
        lv_obj_set_style_text_font(t, &ark12, 0);
        lv_obj_set_style_text_color(t, lv_color_hex(C_TEXT), 0);
        lv_obj_set_pos(t, cx[i] < LCD_W / 2 ? cx[i] + 30 : cx[i] - 46, cy[i] < LCD_H / 2 ? cy[i] : cy[i] - 18);
    }
    lv_obj_t *c = lv_label_create(s_cal);
    lv_label_set_text(c, "CAL");
    lv_obj_set_style_text_font(c, &ark24, 0);
    lv_obj_set_style_text_color(c, lv_color_hex(C_PURPLE), 0);
    lv_obj_align(c, LV_ALIGN_CENTER, 0, -20);
    s_cal_dot = lv_obj_create(s_cal);
    lv_obj_remove_style_all(s_cal_dot);
    lv_obj_set_size(s_cal_dot, 10, 10);
    lv_obj_set_style_radius(s_cal_dot, 5, 0);
    lv_obj_set_style_bg_color(s_cal_dot, lv_color_hex(0xFF3030), 0);
    lv_obj_set_style_bg_opa(s_cal_dot, LV_OPA_COVER, 0);
    lv_obj_add_flag(s_cal_dot, LV_OBJ_FLAG_HIDDEN);
    touch_calibration_begin(s_cal);
}

// ------------------------------------------------------------- settings page
// Everything here persists through appcfg (NVS "cfg"), so a power cycle keeps
// the choices. Rows are 464x32 with the label left and the current value right;
// tapping cycles the value, except the name row which opens a keyboard and the
// link row which forces a refresh.
static void s_settings_rows_rebuild();

static bool s_cfg_dirty = false;      // pending NVS write, handled off the tap path
static lv_obj_t *s_kb = nullptr;      // keyboard overlay (nullptr when closed)
static char s_kb_buf[20];
static lv_obj_t *s_kb_preview = nullptr;
static lv_obj_t *s_kb_row_handle = nullptr;

static void kb_redraw_preview() {
    if (!s_kb_preview) return;
    char line[28];
    snprintf(line, sizeof(line), "@%s", s_kb_buf);
    lv_label_set_text(s_kb_preview, line);
}

static void kb_close() {
    if (s_kb) {
        lv_obj_del(s_kb);
        s_kb = nullptr;
        s_kb_preview = nullptr;
    }
}

static void kb_key_cb(lv_event_t *e) {
    const char c = (char)(intptr_t)lv_event_get_user_data(e);
    size_t n = strlen(s_kb_buf);
    if (c == '\b') {
        if (n) s_kb_buf[n - 1] = 0;
    } else if (c != 0 && n + 1 < sizeof(s_kb_buf)) {
        s_kb_buf[n] = c;
        s_kb_buf[n + 1] = 0;
    }
    kb_redraw_preview();
}

static void kb_key(lv_obj_t *p, int x, int y, int w, int h, const char *label, char code) {
    lv_obj_t *b = obj(p, x, y, w, h);
    lv_obj_set_style_radius(b, 3, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(C_LINE), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_PURPLE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(b, 2);
    lv_obj_t *l = txt(b, label, 0, 0, &ark12, C_TEXT);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, kb_key_cb, LV_EVENT_CLICKED, (void *)(intptr_t)code);
}

static void kb_apply() {
    if (!s_kb_buf[0]) return;  // never save an empty handle
    snprintf(g_cfg.handle, sizeof(g_cfg.handle), "%s", s_kb_buf);
    s_cfg_dirty = true;  // persisted by the main loop, not inside the tap
    kb_close();
    ui_settings_refresh();
}

static void kb_open() {
    if (s_kb) return;
    snprintf(s_kb_buf, sizeof(s_kb_buf), "%s", g_cfg.handle);
    s_kb = obj(lv_layer_top(), 0, 96, LCD_W, LCD_H - 96);
    lv_obj_set_style_bg_color(s_kb, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(s_kb, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_kb, 1, 0);
    lv_obj_set_style_border_color(s_kb, lv_color_hex(C_PURPLE), 0);
    lv_obj_add_flag(s_kb, LV_OBJ_FLAG_CLICKABLE);  // swallow taps behind it

    s_kb_preview = txt(s_kb, "", 10, 4, &ark16, C_TEXT);
    kb_redraw_preview();
    lv_obj_t *ok = button(s_kb, LCD_W - 96, 2, 46, "保存", C_PURPLE, C_TEXT);
    lv_obj_set_ext_click_area(ok, 2);
    lv_obj_add_event_cb(ok, [](lv_event_t *e) { kb_apply(); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *no = button(s_kb, LCD_W - 46, 2, 40, "取消", C_CARD, C_MUTED);
    lv_obj_set_ext_click_area(no, 2);
    lv_obj_add_event_cb(no, [](lv_event_t *e) { kb_close(); }, LV_EVENT_CLICKED, NULL);

    static const char *rows[3] = {"1234567890", "qwertyuiop", "asdfghjkl"};
    // 10 letter keys must stay clear of the tall 删/清空 column on the right.
    const int kw = 40, gap = 2, x0 = 4, tall_x = x0 + 10 * (kw + gap), tall_w = 46;
    for (int r = 0; r < 3; r++) {
        for (int i = 0; rows[r][i]; i++) {
            char lab[2] = {rows[r][i], 0};
            kb_key(s_kb, x0 + i * (kw + gap), 32 + r * 36, kw, 32, lab, rows[r][i]);
        }
    }
    // row 4: z x c v b n m - _ backspace
    const char *r4 = "zxcvbnm-_";
    for (int i = 0; r4[i]; i++) {
        char lab[2] = {r4[i], 0};
        kb_key(s_kb, x0 + i * (kw + gap), 140, kw, 32, lab, r4[i]);
    }
    kb_key(s_kb, tall_x, 32, tall_w, 68, "删", '');  // "⌫" is not in the subset font
    kb_key(s_kb, tall_x, 104, tall_w, 68, "清空", 0);
    lv_obj_t *clr = lv_obj_get_child(s_kb, lv_obj_get_child_cnt(s_kb) - 1);
    lv_obj_add_event_cb(clr, [](lv_event_t *e) {
        s_kb_buf[0] = 0;
        kb_redraw_preview();
    }, LV_EVENT_CLICKED, NULL);
}

static void row_cycle_cb(lv_event_t *e) {
    const int which = (int)(intptr_t)lv_event_get_user_data(e);
    switch (which) {
        case 1: g_cfg.quota_mode = !g_cfg.quota_mode; break;
        case 2: g_cfg.refresh_s = cfg_next_refresh(g_cfg.refresh_s); break;
        case 3: g_cfg.anim = !g_cfg.anim; break;
        case 4: g_cfg.privacy = !g_cfg.privacy; break;
        default: return;
    }
    s_cfg_dirty = true;
    ui_settings_refresh();
}

static void row_name_cb(lv_event_t *e) { kb_open(); }


// ------------------------------------------------------- connection panel
// The link is an explicit choice instead of a hidden default, because this
// firmware is meant to be handed to other people: either one USB cable to a PC
// running tools/usb_bridge.py (nothing to configure), or plain WiFi with the
// gateway host typed into the on-device setup page.
static lv_obj_t *s_link = nullptr;

static void link_panel_refresh() {
    if (!s_link) return;
    const bool usb = g_cfg.link_mode == 0;
    char b[80];
    const char *live = gw_active_link();
    if (!strcmp(live, "-"))
        set_txt(U.lk_state, usb ? "USB · 等待电脑端桥接" : "WiFi · 未连接");
    else
        set_txtf(U.lk_state, "%s · 已连接", live);
    set_color(U.lk_state, strcmp(live, "-") ? C_GREEN : C_AMBER);

    if (usb)
        set_txt(U.lk_addr, "电脑端跑 tools/usb_bridge.py");
    else {
        snprintf(b, sizeof(b), "网关 %s:%u", netcfg_host(), (unsigned)netcfg_port());
        set_txt(U.lk_addr, b);
    }
    if (usb)
        set_txt(U.lk_hint, "插上 USB 线就能用，不用路由器");
    else
        set_txt(U.lk_hint, "手机连热点 magpie-dash-setup 配网");

    for (int i = 0; i < 2; i++) {
        if (!U.lk_btn[i]) continue;
        const bool sel = (i == 0) == usb;
        lv_obj_set_style_bg_opa(U.lk_btn[i], sel ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(U.lk_btn[i], lv_color_hex(sel ? C_PURPLE : C_LINE), 0);
        lv_obj_t *l = lv_obj_get_child(U.lk_btn[i], 0);
        if (l) lv_obj_set_style_text_color(l, lv_color_hex(sel ? C_TEXT : C_MUTED), 0);
    }
}

static void link_panel_close() {
    if (s_link) {
        lv_obj_del(s_link);
        s_link = nullptr;
        U.lk_state = U.lk_addr = U.lk_hint = nullptr;
        U.lk_btn[0] = U.lk_btn[1] = nullptr;
    }
}

static void link_mode_cb(lv_event_t *e) {
    netcfg_set_mode((uint8_t)(intptr_t)lv_event_get_user_data(e));
    s_cfg_dirty = true;
    app_request_refresh();
    link_panel_refresh();
    ui_settings_refresh();
}

static void link_mode_btn(lv_obj_t *p, int x, int y, int w, const char *label, int mode) {
    lv_obj_t *b = obj(p, x, y, w, 30);
    lv_obj_set_style_radius(b, 4, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_PURPLE), 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(C_LINE), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_DIM), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(b, 3);
    lv_obj_t *l = txt(b, label, 0, 0, &ark12, C_MUTED);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, link_mode_cb, LV_EVENT_CLICKED, (void *)(intptr_t)mode);
    U.lk_btn[mode] = b;
}

static void link_panel_open() {
    if (s_link) return;
    s_link = obj(lv_layer_top(), 0, 46, LCD_W, LCD_H - 46);
    lv_obj_set_style_bg_color(s_link, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(s_link, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_link, 1, 0);
    lv_obj_set_style_border_color(s_link, lv_color_hex(C_PURPLE), 0);
    lv_obj_add_flag(s_link, LV_OBJ_FLAG_CLICKABLE);

    txt(s_link, "连接 magpie", 10, 6, &ark12, C_TEXT);
    lv_obj_t *x = button(s_link, LCD_W - 42, 2, 36, "关闭", C_CARD, C_MUTED);
    lv_obj_set_ext_click_area(x, 2);
    lv_obj_add_event_cb(x, [](lv_event_t *e) { link_panel_close(); }, LV_EVENT_CLICKED, NULL);

    link_mode_btn(s_link, 10, 30, 220, "USB 一根线", 0);
    link_mode_btn(s_link, 240, 30, 220, "WiFi 局域网", 1);

    U.lk_state = txt(s_link, "--", 10, 70, &ark12, C_TEXT);
    U.lk_addr = txt(s_link, "--", 10, 92, &ark12, C_MUTED);
    U.lk_hint = txt(s_link, "--", 10, 110, &ark12, C_FAINT);

    lv_obj_t *retry = button(s_link, 10, 136, 104, "重新连接", C_CARD, C_TEXT);
    lv_obj_set_ext_click_area(retry, 2);
    lv_obj_add_event_cb(retry, [](lv_event_t *e) {
        if (g_cfg.link_mode == 0)
            app_request_refresh();  // ask the host bridge for data now
        else if (!netcfg_retry())
            Serial.println("[ui] no WiFi credentials yet -> portal");
        link_panel_refresh();
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *portal = button(s_link, 122, 136, 110, "WiFi 配网", C_CARD, C_TEXT);
    lv_obj_set_ext_click_area(portal, 2);
    lv_obj_add_event_cb(portal, [](lv_event_t *e) {
        netcfg_set_mode(1);
        netcfg_open_portal();
        s_cfg_dirty = true;
        link_panel_refresh();
        ui_settings_refresh();
    }, LV_EVENT_CLICKED, NULL);

    link_panel_refresh();
}

static void build_settings() {
    lv_obj_t *p = pg[PG_SETTINGS] = make_page();
    home_btn(p);
    tabs(p, 4, PG_SETTINGS);  // same chrome as every other page

    // Same card + divider rhythm as the agent list, so the page does not look
    // like a different app: six 36 px rows inside one 468x220 card.
    lv_obj_t *c = card(p, 6, 46, 468, 220);
    static const char *labels[5] = {"显示名称", "用量显示", "刷新间隔", "头像动效", "隐藏金额"};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *r = obj(c, 0, i * 36, 466, 36);
        lv_obj_set_style_border_width(r, 1, 0);
        lv_obj_set_style_border_side(r, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(r, lv_color_hex(C_DIM), 0);
        lv_obj_add_flag(r, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(r, 0);  // the row already spans the full width
        const char *name = (i == 5) ? "连接 magpie" : labels[i];
        txt(r, name, 10, 10, &ark12, C_MUTED);
        lv_obj_t *v = txt(r, "--", 0, 9, &ark12, C_TEXT);
        lv_obj_align(v, LV_ALIGN_TOP_RIGHT, -12, 9);
        U.set_val[i] = v;
        if (i == 0)
            lv_obj_add_event_cb(r, row_name_cb, LV_EVENT_CLICKED, NULL);
        else if (i == 5)
            lv_obj_add_event_cb(r, [](lv_event_t *e) { link_panel_open(); }, LV_EVENT_CLICKED, NULL);
        else
            lv_obj_add_event_cb(r, row_cycle_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }
    ui_settings_refresh();
}

bool ui_cfg_take_dirty() {
    const bool d = s_cfg_dirty;
    s_cfg_dirty = false;
    return d;
}

void ui_settings_refresh() {
    if (!U.set_val[0]) return;
    char b[40];
    snprintf(b, sizeof(b), "@%s", g_cfg.handle);
    set_txt(U.set_val[0], b);
    if (U.hm_handle) set_txt(U.hm_handle, b);
    set_txt(U.set_val[1], g_cfg.quota_mode ? "已用" : "剩余");
    snprintf(b, sizeof(b), "%u 秒", (unsigned)g_cfg.refresh_s);
    set_txt(U.set_val[2], b);
    set_txt(U.set_val[3], g_cfg.anim ? "开" : "关");
    set_txt(U.set_val[4], g_cfg.privacy ? "开" : "关");
    if (U.set_val[5]) {
        // Show the pipe that is really carrying data, not just the preference:
        // with the fallback the two can differ.
        const char *live = gw_active_link();
        if (!strcmp(live, "USB")) set_txt(U.set_val[5], "USB · 已连接");
        else if (!strcmp(live, "WiFi")) set_txt(U.set_val[5], "WiFi · 已连接");
        else set_txt(U.set_val[5], g_cfg.link_mode ? "WiFi · 未连接" : "USB · 未连接");
    }
    link_panel_refresh();
    // quota wording follows the setting
    for (int i = 0; i < QUOTA_MAX_PROV; i++)
        if (U.q_word[i]) set_txt(U.q_word[i], g_cfg.quota_mode ? "已用" : "剩余");
}


void ui_link_panel(bool on) {
    if (on) {
        ui_goto(PG_SETTINGS);
        for (int i = 0; i < 4; i++) { lv_timer_handler(); delay(20); }
        link_panel_open();
    } else {
        link_panel_close();
    }
    for (int i = 0; i < 4; i++) { lv_timer_handler(); delay(20); }
}

void ui_settings_keyboard(bool on) {
    if (on) {
        ui_goto(PG_SETTINGS);
        for (int i = 0; i < 4; i++) { lv_timer_handler(); delay(20); }
        kb_open();
    } else {
        kb_close();
    }
    for (int i = 0; i < 4; i++) { lv_timer_handler(); delay(20); }
}

// ------------------------------------------------------------------ settings self-test
// Real indev taps (not direct calls), so a hit-area or layering mistake shows up
// the same way it would under a finger. Serial 'Z'.
void ui_settings_test() {
    char before[20];
    snprintf(before, sizeof(before), "%s", g_cfg.handle);
    const uint8_t q0 = g_cfg.quota_mode, r0 = g_cfg.refresh_s;
    int cases = 0, failed = 0;

    ui_goto(PG_SETTINGS);
    for (int i = 0; i < 6; i++) { lv_timer_handler(); delay(20); }

    display_test_tap(200, 63);  // row 0: 显示名称
    for (int i = 0; i < 6; i++) { lv_timer_handler(); delay(20); }
    cases++;
    bool pass = s_kb != nullptr;
    if (!pass) failed++;
    Serial.printf("[settest] %s keyboard_open=%d\n", pass ? "PASS" : "FAIL", (int)pass);

    display_test_tap(234, 216);  // 'h' (row 3 of the keyboard)
    display_test_tap(318, 180);  // 'i'
    for (int i = 0; i < 4; i++) { lv_timer_handler(); delay(20); }
    cases++;
    pass = strstr(s_kb_buf, "hi") != nullptr;
    if (!pass) failed++;
    Serial.printf("[settest] %s typed=%s\n", pass ? "PASS" : "FAIL", s_kb_buf);

    display_test_tap(LCD_W - 73, 113);  // 保存
    for (int i = 0; i < 8; i++) { lv_timer_handler(); delay(20); }
    cases++;
    pass = (s_kb == nullptr) && strlen(g_cfg.handle) == strlen(before) + 2 &&
           !strcmp(g_cfg.handle + strlen(g_cfg.handle) - 2, "hi");
    if (!pass) failed++;
    Serial.printf("[settest] %s saved=@%s closed=%d\n", pass ? "PASS" : "FAIL", g_cfg.handle,
                  (int)(s_kb == nullptr));

    display_test_tap(200, 99);   // 用量显示
    display_test_tap(200, 135);  // 刷新间隔
    for (int i = 0; i < 6; i++) { lv_timer_handler(); delay(20); }
    cases++;
    pass = g_cfg.quota_mode != q0 && g_cfg.refresh_s != r0;
    if (!pass) failed++;
    Serial.printf("[settest] %s quota=%u refresh=%u\n", pass ? "PASS" : "FAIL",
                  (unsigned)g_cfg.quota_mode, (unsigned)g_cfg.refresh_s);

    cases++;
    pass = U.hm_handle && strstr(lv_label_get_text(U.hm_handle), g_cfg.handle) != nullptr;
    if (!pass) failed++;
    Serial.printf("[settest] %s header=%s\n", pass ? "PASS" : "FAIL",
                  U.hm_handle ? lv_label_get_text(U.hm_handle) : "-");

    // the link row must open the connection panel, not just fire a refresh
    display_test_tap(200, 244);  // row 5: 连接 magpie
    for (int i = 0; i < 8; i++) { lv_timer_handler(); delay(20); }
    cases++;
    pass = s_link != nullptr;
    if (!pass) failed++;
    Serial.printf("[settest] %s link_panel_open=%d\n", pass ? "PASS" : "FAIL", (int)pass);
    link_panel_close();

    // restore the previous look so the test is repeatable
    snprintf(g_cfg.handle, sizeof(g_cfg.handle), "%s", before);
    g_cfg.quota_mode = q0;
    g_cfg.refresh_s = r0;
    cfg_save();
    ui_settings_refresh();
    Serial.printf("[settest] END total=%d failed=%d\n", cases, failed);
}

// debug (serial 'G'): scroll the route list, to prove the list really scrolls
void ui_routes_scroll(int dy) {
    lv_obj_t *sc = U.r_scroll[1];
    if (!sc) return;
    lv_obj_scroll_by(sc, 0, dy, LV_ANIM_OFF);
    Serial.printf("[ui] routes scroll y=%d\n", (int)lv_obj_get_scroll_y(sc));
}

// ------------------------------------------------------------------ public
static bool s_ui_ready = false;  // false during the initial page build

void ui_init() {
    assets_init();
    lv_disp_t *d = lv_disp_get_default();
    lv_theme_t *th = lv_theme_default_init(d, lv_palette_main(LV_PALETTE_BLUE),
                                           lv_palette_main(LV_PALETTE_RED), false, &ark12);
    lv_disp_set_theme(d, th);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(C_BG), 0);

    build_home();
    build_state(0, PG_STATE_THINK, false);
    build_state(1, PG_STATE_CODE, false);
    build_state(2, PG_STATE_CONFIRM, true);
    build_agents();
    build_quota();
    build_usage();
    build_routes(PG_ROUTES, 2);
    build_routes(PG_ROUTES_LIST, RT_ROWS);
    build_settings();

    // typing dots of whichever avatar is on screen
    lv_timer_create(
        [](lv_timer_t *t) {
            static int phase = 0;
            phase = (phase + 1) % 3;
            avatar_step(s_av_home, phase);
            for (int i = 0; i < 3; i++) avatar_step(s_av_state[i], phase);
            avatar_step(s_av_sheet, phase);
        },
        350, NULL);

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    Serial.printf("[ui] lvgl mem: %u used, %u free, %u%% frag, %u max_used\n",
                  (unsigned)(mon.total_size - mon.free_size), (unsigned)mon.free_size,
                  (unsigned)mon.frag_pct, (unsigned)mon.max_used);

    ui_goto(0);
    s_ui_ready = true;
}

static uint32_t s_last_page_ms = 0;
static int s_pending_page = -1;
static uint32_t s_pending_ms = 0;

// Show/hide the pages and paint once. This is the expensive step (50-65 ms on
// this panel), so it must not run once per tap in a burst of taps.
static void apply_page(int page) {
    if (page < 0) page = 0;
    s_pending_page = -1;  // this paint supersedes anything queued: without this a
                          // stale target could jump the UI back later
    const int prev = s_page;
    s_page = page % UI_PAGES;
    if (s_page != prev) close_overlays();  // overlays live on the top layer
    avatars_sync_motion(s_live_state);
    if (U.sh && s_page != PG_AGENTS) sheet_close();
    for (int i = 0; i < UI_PAGES; i++) {
        if (!pg[i]) continue;
        if (i == s_page)
            lv_obj_clear_flag(pg[i], LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(pg[i], LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui_ready && prev != s_page) {
        const uint32_t t0 = millis();
        lv_obj_invalidate(lv_scr_act());
        lv_refr_now(NULL);
        if (dbg_verbose) Serial.printf("[ui] page %d painted %u ms\n", s_page, (unsigned)(millis() - t0));
    }
    s_last_page_ms = millis();
    if (dbg_verbose) Serial.printf("[ui] page %d at %u\n", s_page, (unsigned)s_last_page_ms);
}

// Tap path. A lone tap paints at once (feels immediate); taps that arrive within
// PAGE_COALESCE_MS of the previous paint only record the newest target, so four
// quick tab taps paint the fourth page once instead of painting four pages
// 50-65 ms apart and falling further behind the finger with every tap.
#define PAGE_COALESCE_MS 150

static void page_request(int page) {
    if (page < 0) page = 0;
    page %= UI_PAGES;
    if (!s_ui_ready) {
        apply_page(page);
        return;
    }
    if (page == s_page) {
        s_pending_page = -1;
        return;
    }
    if (millis() - s_last_page_ms > PAGE_COALESCE_MS) {
        apply_page(page);
    } else {
        s_pending_page = page;
        s_pending_ms = millis();
    }
}

// Called from loop(): paint the coalesced target once the burst stops.
void ui_page_settle() {
    if (s_pending_page < 0) return;
    if (millis() - s_pending_ms < 20) return;
    const int p = s_pending_page;
    s_pending_page = -1;
    if (p != s_page) apply_page(p);
}

void ui_tap_page(int page) { page_request(page); }

// Programmatic switches (serial 0..8, init, debug) stay synchronous so a
// following lv_refr_now()/capture always sees the requested page.
static void close_overlays() {
    link_panel_close();
    kb_close();
}

void ui_goto(int page) {
    s_pending_page = -1;
    close_overlays();  // a keyboard / link panel must not float over another page
    apply_page(page);
}

void ui_next_page() { ui_goto(s_page + 1); }

int ui_cur_page() { return s_page; }

// debug (serial 'f'): pin the next agent in the catalog, to preview its avatar
void ui_focus_next() {
    int k = 0;
    while (k < AGENT_SKIN_COUNT - 1 && strcmp(AGENT_SKINS[k].id, s_focus)) k++;
    k = (k + 1) % (AGENT_SKIN_COUNT - 1);
    strcpy(s_focus, AGENT_SKINS[k].id);
    s_focus_ms = millis();
    Serial.printf("[ui] focus agent %s\n", s_focus);
    if (s_data) ui_update(*s_data);
}

lv_obj_t *ui_page_obj(int page) {
    if (page < 0 || page >= UI_PAGES) return nullptr;
    return pg[page];
}

void ui_tick() {
    // clock
    time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    static const char *wd[7] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
    if (tmv.tm_year > 120) {
        set_txtf(U.date, "%d/%d %s", tmv.tm_mon + 1, tmv.tm_mday, wd[tmv.tm_wday]);
        set_txtf(U.clock, "%02d %02d", tmv.tm_hour, tmv.tm_min);
    }
    // the live dot on the home card blinks while the agent is at work
    static bool blink = false;
    blink = !blink;
    for (int i = 0; i < AG_ROWS; i++) {
        if (!U.ag_live[i] || lv_obj_has_flag(U.ag_live[i], LV_OBJ_FLAG_HIDDEN)) continue;
        lv_obj_set_style_bg_opa(U.ag_live[i], blink ? LV_OPA_COVER : LV_OPA_40, 0);
    }
    if (U.hm_dot && s_av_home.mode == AV_THINK)
        lv_obj_set_style_bg_opa(U.hm_dot, blink ? LV_OPA_COVER : LV_OPA_30, 0);
    else if (U.hm_dot)
        lv_obj_set_style_bg_opa(U.hm_dot, LV_OPA_COVER, 0);
}

// debug (serial 'k'): poke the avatar on the page in view
void ui_poke() {
    if (s_page == PG_HOME) avatar_poke(s_av_home);
    if (s_page >= PG_STATE_THINK && s_page <= PG_STATE_CONFIRM) avatar_poke(s_av_state[s_page - PG_STATE_THINK]);
    if (s_page == PG_AGENTS && U.sh_id[0]) avatar_poke(s_av_sheet);
}

// debug (serial 'o'): open the sheet of the first agent in the list
void ui_open_sheet(int row) {
    ui_goto(PG_AGENTS);
    if (row >= 0 && row < AG_ROWS && U.ag_id[row][0]) sheet_open(U.ag_id[row]);
}

// ---- which agent the buddy pages are about
static const char *focus_id(const DashData &d) {
    if (s_focus[0] && millis() - s_focus_ms < FOCUS_HOLD_MS) return s_focus;
    s_focus[0] = 0;
    if (d.nrows && d.rows[0].agent[0]) return d.rows[0].agent;
    for (int i = 0; i < d.nagents; i++)
        if (d.agents[i].calls) return d.agents[i].id;
    return "claude";
}

static const RouteRow *focus_row(const DashData &d, const char *id) {
    for (int i = 0; i < d.nrows; i++)
        if (!strcmp(d.rows[i].agent, id)) return &d.rows[i];
    return nullptr;
}

static const char *focus_name(const DashData &d, const char *id, const RouteRow *row) {
    if (row && row->agentName[0]) return row->agentName;
    for (int i = 0; i < d.nagents; i++)
        if (!strcmp(d.agents[i].id, id)) return agent_label(id, d.agents[i].name);
    return agent_label(id, "");
}

// quota shown next to the buddy: the provider that served this agent last,
// else the first one with time windows
static const QuotaProvider *focus_quota(const DashData &d, const RouteRow *row) {
    if (row) {
        const QuotaProvider *q = d.find(row->provId);
        if (q && q->nwin) return q;
    }
    for (int i = 0; i < d.nprov; i++)
        if (d.prov[i].nwin >= 2) return &d.prov[i];
    return d.nprov ? &d.prov[0] : nullptr;
}

// ---- agent list: what magpie manages (relay), or usage + our catalog
struct AgentView {
    const char *id, *name, *model;
    uint32_t calls;
};

static int agent_views(const DashData &d, AgentView *out, int max) {
    int n = 0;
    for (int i = 0; i < d.nagents && n < max; i++)
        out[n++] = {d.agents[i].id, d.agents[i].name, d.agents[i].model, d.agents[i].calls};
    if (!d.agents_full) {
        // older relay: only agents with calls came back, so list the rest of
        // the agents magpie can wire up too
        for (int k = 0; k < AGENT_SKIN_COUNT - 1 && n < max; k++) {
            bool dup = false;
            for (int i = 0; i < n && !dup; i++) dup = !strcmp(out[i].id, AGENT_SKINS[k].id);
            if (!dup) out[n++] = {AGENT_SKINS[k].id, "", "", 0};
        }
    }
    // without a configured model, show the one it used last
    for (int i = 0; i < n; i++) {
        if (out[i].model[0]) continue;
        const RouteRow *r = focus_row(d, out[i].id);
        if (r) out[i].model = r->model;
    }
    return n;
}

static const AgentRow *agent_stat(const DashData &d, const char *id) {
    for (int i = 0; i < d.nagents; i++)
        if (!strcmp(d.agents[i].id, id)) return &d.agents[i];
    return nullptr;
}

// "刚刚" / "5 分钟前" / "3 小时前" / "2 天前"; "" when unknown
static const char *fmt_ago(uint32_t t, char *buf, size_t n) {
    const time_t now = time(nullptr);
    buf[0] = 0;
    if (!t || now < 1700000000) return buf;
    const long s = (long)(now - (time_t)t);
    if (s < 60)
        snprintf(buf, n, "刚刚");
    else if (s < 3600)
        snprintf(buf, n, "%ld 分钟前", s / 60);
    else if (s < 86400)
        snprintf(buf, n, "%ld 小时前", s / 3600);
    else
        snprintf(buf, n, "%ld 天前", s / 86400);
    return buf;
}

// at work: its latest call is under two minutes old
static bool agent_live(const AgentRow *a) {
    const time_t now = time(nullptr);
    return a && a->last && now > 1700000000 && now - (time_t)a->last < 120;
}

static void sheet_fill() {
    if (!U.sh || !U.sh_id[0]) return;
    const char *id = U.sh_id;
    const AgentSkin *sk = skin_for(id);
    avatar_set(s_av_sheet, sk);
    const AgentRow *a = s_data ? agent_stat(*s_data, id) : nullptr;
    const RouteRow *r = s_data ? focus_row(*s_data, id) : nullptr;
    char b[32];
    set_txt_fit(U.sh_name, agent_label(id, a ? a->name : ""));
    const char *model = a && a->model[0] ? a->model : (r ? r->model : "");
    set_txtf(U.sh_model, "%s  ·  %s", id, model[0] ? model : "默认模型");
    set_txt(U.sh_num[0], a ? fmt_count(a->calls, b, sizeof(b)) : "0");
    set_txt(U.sh_num[1], a ? fmt_count(a->today, b, sizeof(b)) : "0");
    set_txt(U.sh_num[2], a ? fmt_count((uint64_t)a->tokens, b, sizeof(b)) : "0");
    if (a && a->cost >= 10)
        set_txtf(U.sh_num[3], "$%.0f", a->cost);
    else
        if (g_cfg.privacy) set_txt(U.sh_num[3], "$--");
        else set_txtf(U.sh_num[3], "$%.2f", a ? a->cost : 0.0f);
    uint32_t total = 0;
    for (int i = 0; s_data && i < s_data->nagents; i++) total += s_data->agents[i].calls;
    const float share = total && a ? 100.0f * a->calls / total : 0;
    set_txtf(U.sh_share, "占全部调用 %.0f%%", share);
    segbar_set(U.sh_bar, share, sk->accent);
    const bool live = agent_live(a);
    avatar_mode(s_av_sheet, live ? AV_THINK : AV_IDLE);
    if (live)
        set_txt(U.sh_last, "工作中");
    else if (a && a->last)
        set_txtf(U.sh_last, "最近 %s", fmt_ago(a->last, b, sizeof(b)));
    else
        set_txt(U.sh_last, a && a->calls ? "" : "还没用过");
    set_color(U.sh_last, live ? C_GREEN : C_MUTED);
    const bool following = s_focus[0] && !strcmp(s_focus, id);
    set_txt(lv_obj_get_child(U.sh_follow, 0), following ? "取消跟随" : "跟随");
    lv_obj_set_style_bg_color(U.sh_follow, lv_color_hex(following ? C_DIM : sk->accent), 0);
    lv_obj_set_style_border_color(U.sh_follow, lv_color_hex(following ? C_LINE : sk->accent), 0);
    // light accents (Grok, Cursor, Pi...) need dark text on the button
    const uint32_t ac = sk->accent;
    const int lum = ((ac >> 16) & 0xFF) * 3 + ((ac >> 8) & 0xFF) * 6 + (ac & 0xFF);
    set_color(lv_obj_get_child(U.sh_follow, 0), !following && lum > 1800 ? C_BG : C_TEXT);
}

static void tag_for(const RouteRow &r, char *buf, size_t n, uint32_t *col) {
    if (r.status && r.status != 200) {
        snprintf(buf, n, "错误 %u", (unsigned)r.status);
        *col = C_RED;
    } else if (!strcmp(r.stop, "tool_calls") || !strcmp(r.stop, "tool_use")) {
        snprintf(buf, n, "工具调用");
        *col = C_PURPLE;
    } else {
        snprintf(buf, n, "回复");
        *col = C_MUTED;
    }
}

// "Sep 20" -> "9/20", "00" (hourly) -> "00:00"; ark10 has no CJK
static void chart_label(const char *in, char *out, size_t n) {
    static const char *mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int m = 0; m < 12; m++) {
        if (!strncmp(in, mon[m], 3)) {
            snprintf(out, n, "%d/%d", m + 1, atoi(in + 3));
            return;
        }
    }
    if (isdigit((unsigned char)in[0]) && strlen(in) <= 2)
        snprintf(out, n, "%02d:00", atoi(in));
    else
        snprintf(out, n, "%.6s", in);
}

// Percent a quota window should show: remaining (default) or used, per settings.
static inline float q_show(const QuotaWindow *w) {
    if (!w) return 0;
    float v = g_cfg.quota_mode ? (100.0f - w->remaining) : w->remaining;
    return v < 0 ? 0 : (v > 100 ? 100 : v);
}

void ui_update(const DashData &d) {
    s_data = &d;
    char nb[24], nb2[24], lb[40];

    const char *fid = focus_id(d);
    const AgentSkin *sk = skin_for(fid);
    const RouteRow *frow = focus_row(d, fid);
    const char *fname = focus_name(d, fid, frow);
    const QuotaProvider *fq = focus_quota(d, frow);
    const QuotaWindow *w0 = fq && fq->nwin > 0 ? &fq->win[0] : nullptr;
    const QuotaWindow *w1 = fq && fq->nwin > 1 ? &fq->win[1] : nullptr;
    char upper[24];
    snprintf(upper, sizeof(upper), "%s", fname);
    for (char *c = upper; *c; c++) *c = (char)toupper((unsigned char)*c);

    // a new agent took over: everyone hops and it says hello
    static char last_fid[16] = "";
    const bool switched = last_fid[0] && strcmp(last_fid, fid);
    strncpy(last_fid, fid, sizeof(last_fid) - 1);
    const AgentRow *fst = agent_stat(d, fid);
    const bool live = agent_live(fst);

    // -------------------------------------------------- home
    lv_obj_set_style_bg_color(U.hm_accent, lv_color_hex(sk->accent), 0);
    lv_obj_set_style_bg_color(U.hm_dot, lv_color_hex(live ? C_GREEN : sk->accent), 0);
    avatar_set(s_av_home, sk);
    avatar_mode(s_av_home, live ? AV_THINK : AV_IDLE);
    static bool prev_live = false;
    if (prev_live && !live && d.gw) {  // work just finished
        avatar_hop(s_av_home);
        avatar_say(s_av_home, "做完了", 1800);
    }
    prev_live = live;
    s_live_state = live;
    if (switched) {
        avatar_hop(s_av_home);
        avatar_say(s_av_home, sk->say[0], 2500);
    }
    set_txt_fit(U.hm_name, fname);
    const bool tool = frow && (!strcmp(frow->stop, "tool_calls") || !strcmp(frow->stop, "tool_use"));
    if (!d.gw)
        set_txt(U.hm_state, "等待网关");
    else if (live)
        set_txt(U.hm_state, tool ? "运行命令" : "工作中");
    else if (fst && fst->last)
        set_txtf(U.hm_state, "最近 %s", fmt_ago(fst->last, nb2, sizeof(nb2)));
    else if (frow)
        set_txtf(U.hm_state, "最近 %s", frow->time);
    else
        set_txt(U.hm_state, "空闲");
    if (fst && fst->calls)
        set_txtf(U.hm_stats, "今天 %s 次 · 30 天 %s 次", fmt_count(fst->today, nb, sizeof(nb)),
                 fmt_count(fst->calls, nb2, sizeof(nb2)));
    else
        set_txt(U.hm_stats, "最近 30 天未使用");
    if (w0) {
        win_short(w0->name, lb, sizeof(lb), false);  // no room for "Gemini" before the bar
        set_txt(U.hm_qlbl, lb);
        set_txtf(U.hm_pct, "%.0f%%", q_show(w0));
        set_color(U.hm_pct, remain_col(w0->remaining));
        segbar_set(U.hm_bar, q_show(w0), remain_col(w0->remaining));
    } else {
        set_txt(U.hm_pct, "--");
        segbar_set(U.hm_bar, 0);
    }
    set_txtf(U.hm_mag, "今天 %u 次 · %s", (unsigned)d.today_calls,
             fmt_count((uint64_t)d.today_tokens, nb, sizeof(nb)));
    // hour tick strip: how far the day has gone (design's "today" timeline)
    time_t now2 = time(nullptr);
    struct tm tmv2;
    localtime_r(&now2, &tmv2);
    float daypct = tmv2.tm_year > 120 ? (tmv2.tm_hour * 60 + tmv2.tm_min) / 14.4f : 0.0f;
    segbar_set(U.hm_tick, daypct);

    // -------------------------------------------------- state pages
    for (int i = 0; i < 3; i++) {
        avatar_set(s_av_state[i], sk);
        if (switched) avatar_hop(s_av_state[i]);
        for (int k = 0; k < 3; k++)
            lv_obj_set_style_bg_color(U.st_pip[i][k], lv_color_hex(k == i ? sk->accent : C_DIM), 0);

        set_txt(U.st_agent[i], upper);
        set_color(U.st_agent[i], sk->accent);
        set_color(U.st_star[i], sk->accent);
        set_color(U.st_title[i], sk->accent);
        set_txt(U.st_title[i], i == 0 ? "斟酌中" : (i == 1 ? "编写代码" : "等你确认"));
        set_txtf(U.st_sess[i], "%d 个会话", d.sessions);
        if (frow) {
            if (i == 0)
                set_txtf(U.st_sub[i], "已用时 %s", fmt_secs(frow->ms, nb2, sizeof(nb2)));
            else if (i == 1)
                set_txt(U.st_sub[i], frow->model);
            else
                set_txtf(U.st_sub[i], "运行命令 : %s", frow->model);
        } else {
            set_txt(U.st_sub[i], "暂无调用");
        }

        if (w0) {
            win_long(w0->name, lb, sizeof(lb));
            set_txt(U.st_l5[i], lb);
            set_txtf(U.st_p5[i], "%.0f%%", q_show(w0));
            set_color(U.st_p5[i], remain_col(w0->remaining));
            segbar_set(U.st_bar5[i], q_show(w0), remain_col(w0->remaining));
        } else {
            set_txt(U.st_p5[i], "--");
            segbar_set(U.st_bar5[i], 0);
        }
        show(U.st_l7[i], w1 != nullptr);
        show(U.st_p7[i], w1 != nullptr);
        show(U.st_bar7[i], w1 != nullptr);
        if (w1) {
            win_long(w1->name, lb, sizeof(lb));
            set_txt(U.st_l7[i], lb);
            set_txtf(U.st_p7[i], "%.0f%%", q_show(w1));
            set_color(U.st_p7[i], remain_col(w1->remaining));
            segbar_set(U.st_bar7[i], q_show(w1), remain_col(w1->remaining));
        }
        const QuotaWindow *wr = w1 ? w1 : w0;
        if (wr) {
            char cd[40];
            fmt_countdown((time_t)wr->reset_at, cd, sizeof(cd));
            set_txt(U.st_reset[i], cd[0] ? cd : wr->reset);
        } else {
            set_txt(U.st_reset[i], "");
        }
        if (fq)
            set_txtf(U.st_src[i], "额度来源 %s · %s", fq->name, fq->plan);
        else
            set_txt(U.st_src[i], "");
        set_txt(U.st_conn[i], d.gw ? "已连接" : "未连接");
    }

    // -------------------------------------------------- agents
    static AgentView views[AG_ROWS];
    const int nv = agent_views(d, views, AG_ROWS);
    for (int pgid = 0; pgid < UI_PAGES; pgid++)
        if (U.ag_tab[pgid]) set_txt(U.ag_tab[pgid], "Agent");  // count lives in the list
    for (int i = 0; i < AG_ROWS; i++) {
        show(U.ag_row[i], i < nv);
        if (i >= nv) {
            U.ag_id[i][0] = 0;
            continue;
        }
        const AgentView &v = views[i];
        strncpy(U.ag_id[i], v.id, sizeof(U.ag_id[i]) - 1);
        U.ag_id[i][sizeof(U.ag_id[i]) - 1] = 0;
        logo_set(U.ag_icon[i], skin_for(v.id)->s);
        const AgentRow *ast = agent_stat(d, v.id);
        const bool alives = agent_live(ast);
        show(U.ag_live[i], alives);
        // working / finished / idle, spelled out: a bare dot was not readable
        if (alives) {
            set_txt(U.ag_state[i], "工作中");
            set_color(U.ag_state[i], C_AMBER);
        } else if (ast && ast->today) {
            set_txt(U.ag_state[i], "已完成");
            set_color(U.ag_state[i], C_GREEN);
        } else if (ast && (ast->last || ast->calls)) {
            set_txt(U.ag_state[i], "空闲");
            set_color(U.ag_state[i], C_MUTED);
        } else {
            set_txt(U.ag_state[i], "未使用");
            set_color(U.ag_state[i], C_FAINT);
        }
        set_txt_fit(U.ag_name[i], agent_label(v.id, v.name));
        set_color(U.ag_name[i], v.calls ? C_TEXT : C_MUTED);
        set_txt(U.ag_detail[i], v.model[0] ? v.model : "默认模型");
        if (v.calls) {
            set_txtf(U.ag_calls[i], "%s 次", fmt_count(v.calls, nb, sizeof(nb)));
            set_color(U.ag_calls[i], C_GREEN);
        } else {
            set_txt(U.ag_calls[i], "未使用");
            set_color(U.ag_calls[i], C_FAINT);
        }
        const bool pinned = s_focus[0] && !strcmp(s_focus, v.id);
        lv_obj_set_style_bg_color(U.ag_row[i], lv_color_hex(C_DIM), 0);
        lv_obj_set_style_bg_opa(U.ag_row[i], pinned ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }

    sheet_fill();

    // -------------------------------------------------- quota list
    show(U.q_empty, d.nprov == 0);
    for (int i = 0; i < QUOTA_MAX_PROV; i++) {
        const QuotaProvider *q = i < d.nprov ? &d.prov[i] : nullptr;
        show(U.q_blk[i], q != nullptr);
        if (!q) continue;
        snprintf(lb, sizeof(lb), "%s", q->name);
        for (char *c = lb; *c; c++) *c = (char)toupper((unsigned char)*c);
        set_txt(U.q_name[i], lb);
        const char *pid = !strcmp(q->provider, "antigravity") ? "agy" : q->provider;
        lv_obj_set_style_bg_color(U.q_dot[i], lv_color_hex(skin_for(pid)->accent), 0);
        set_txt(U.q_plan[i], q->plan);
        char acct[48];
        mask_account(q->user, acct, sizeof(acct));
        set_txt_fit(U.q_user[i], acct[0] ? acct : q->name);
        const char *rs = q->nwin ? q->win[0].reset : "";
        if (*rs)
            set_txtf(U.q_reset[i], "↻ %s", rs);
        else if (q->lastServed[0])
            set_txtf(U.q_reset[i], "最近 %s", q->lastServed);
        else
            set_txt(U.q_reset[i], "");
        for (int j = 0; j < QUOTA_MAX_WIN; j++) {
            const bool has = j < q->nwin;
            show(U.q_ring[i][j], has);
            show(U.q_win[i][j], has);
            if (!has) continue;
            // right-aligned: the last window sits in the rightmost slot
            const int slot = QUOTA_MAX_WIN - q->nwin + j;
            const int x = 466 - 10 - (QUOTA_MAX_WIN - slot) * 54 + 6;
            lv_obj_set_x(U.q_ring[i][j], x);
            lv_obj_set_x(U.q_win[i][j], x - 5);
            const float rem = q->win[j].remaining;
            const float shown = q_show(&q->win[j]);
            lv_arc_set_value(U.q_ring[i][j], (int)(shown + 0.5f));
            lv_obj_set_style_arc_color(U.q_ring[i][j], lv_color_hex(remain_col(rem)),
                                       LV_PART_INDICATOR);
            set_txtf(U.q_pct[i][j], "%.0f%%", shown);
            win_short(q->win[j].name, lb, sizeof(lb));
            set_txt(U.q_win[i][j], lb);
        }
    }

    // -------------------------------------------------- usage
    char b1[24], b2[24], b3[24];
    set_txt(U.u_tok, fmt_count(d.tok_in + d.tok_out + d.tok_cache, b1, sizeof(b1)));
    set_txtf(U.u_in, "输入 %s", fmt_count(d.tok_in, b1, sizeof(b1)));
    set_txtf(U.u_out, "输出 %s", fmt_count(d.tok_out, b2, sizeof(b2)));
    set_txtf(U.u_cache, "缓存 %s", fmt_count(d.tok_cache, b3, sizeof(b3)));
    set_txtf(U.u_calls, "%u", (unsigned)d.calls);
    set_txtf(U.u_err, "%u 次失败", (unsigned)d.errors);
    set_color(U.u_err, d.errors ? C_RED : C_MUTED);
    // the cost card's subtitle was a dangling label: show what a call costs on
    // average for the selected window instead.
    if (g_cfg.privacy || d.calls == 0)
        set_txt(U.u_price, g_cfg.privacy ? "" : "");
    else
        set_txtf(U.u_price, "均价 $%.4f/次", d.cost / (float)d.calls);
    if (g_cfg.privacy)
        set_txt(U.u_cost, "$--");
    else if (d.cost < 10)
        set_txtf(U.u_cost, "$%.2f", d.cost);
    else
        set_txtf(U.u_cost, "$%.0f", d.cost);
    set_txtf(U.u_hit, "%.0f%%", d.hit_rate());
    {
        const int n = d.nseries;
        float mx = 0;
        for (int i = 0; i < n; i++)
            if (d.series[i].tokens > mx) mx = d.series[i].tokens;
        show(U.u_empty, mx <= 0);
        set_txt(U.u_ymax, mx > 0 ? fmt_count((uint64_t)mx, b1, sizeof(b1)) : "");
        const int slot = n ? CH_W / n : CH_W;
        const int bw = slot > 4 ? slot - 2 : (slot > 1 ? slot - 1 : 1);
        for (int i = 0; i < SERIES_MAX; i++) {
            // Empty buckets must not draw a 2 px stub: a row of stubs reads as a
            // broken dashed baseline instead of "no usage".
            show(U.u_bars[i], i < n && mx > 0 && d.series[i].tokens > 0);
            if (i >= n || mx <= 0 || d.series[i].tokens <= 0) continue;
            int h = (int)(d.series[i].tokens / mx * CH_H + 0.5f);
            if (h < 2) h = 2;
            lv_obj_set_pos(U.u_bars[i], CH_X0 + i * slot, CH_BASE - h);
            lv_obj_set_size(U.u_bars[i], bw, h);
            // the latest bucket stands out, older ones step back
            lv_obj_set_style_bg_opa(U.u_bars[i], i == n - 1 ? LV_OPA_COVER : LV_OPA_80, 0);
        }
        for (int k = 0; k < 5; k++) {
            show(U.u_lbl[k], n > 0);
            if (!n) continue;
            const int src = n == 1 ? 0 : k * (n - 1) / 4;
            chart_label(d.series[src].label, lb, sizeof(lb));
            set_txt(U.u_lbl[k], lb);
            int cx = CH_X0 + src * slot + bw / 2 - 20;
            if (cx > LCD_W - 50) cx = LCD_W - 50;
            lv_obj_set_x(U.u_lbl[k], cx);
        }
    }

    // -------------------------------------------------- routes
    for (int pg_i = 0; pg_i < 2; pg_i++) {
        const int maxrows = pg_i == 0 ? 2 : RT_ROWS;  // detail page scrolls through all
        show(U.r_empty[pg_i], d.nrows == 0);
        for (int i = 0; i < maxrows; i++) {
            const bool has = i < d.nrows;
            show(U.r_row[pg_i][i], has);
            if (!has) continue;
            const RouteRow &r = d.rows[i];
            logo_set(U.r_icon[pg_i][i], skin_for(r.agent)->s);
            set_txt_fit(U.r_agent[pg_i][i], agent_label(r.agent, r.agentName));
            set_txt(U.r_model[pg_i][i], r.model[0] ? r.model : "--");
            uint32_t tc;
            tag_for(r, lb, sizeof(lb), &tc);
            set_txt(U.r_tag[pg_i][i], lb);
            set_color(U.r_tag[pg_i][i], tc);
            set_txt(U.r_time[pg_i][i], r.time);
            const char *prov = !strncmp(r.provider, "session-", 8) ? "会话" : r.provider;
            if (r.account[0])
                set_txtf(U.r_flow[pg_i][i], "→ %s · %s", prov, r.account);
            else
                set_txtf(U.r_flow[pg_i][i], "→ %s", prov);
            set_txtf(U.r_use[pg_i][i], "%s token · %s", fmt_k(r.tokens(), b1, sizeof(b1)),
                     fmt_secs(r.ms, b2, sizeof(b2)));
        }
    }
    if (d.nrows) {
        const RouteRow &r0 = d.rows[0];
        logo_set(U.r_top_icon, skin_for(r0.agent)->s);
        set_txt(U.r_top_agent, agent_label(r0.agent, r0.agentName));
        if (r0.account[0])
            set_txtf(U.r_top_to, "%s · %s", r0.provider, r0.account);
        else
            set_txt(U.r_top_to, r0.provider);
    } else {
        set_txt(U.r_top_agent, "--");
        set_txt(U.r_top_to, "--");
    }
    for (int k = 0; k < 2; k++)
        set_txtf(U.r_sum[k], "今天 %u 次调用 · %s token", (unsigned)d.today_calls,
                 fmt_count((uint64_t)d.today_tokens, b1, sizeof(b1)));

    avatars_sync_motion(live);
}

// ---------------------------------------------------------------------------
// 2-point quick calibration. The GT911 on this panel has been seen reporting a
// mirrored / offset cloud, so nothing is assumed: the user taps two big boxes,
// the affine per axis is solved from those two raw points, and a third tap
// verifies the centre. Result is stored in NVS.
//   step 0 -> tap TL box, step 1 -> tap BR box, step 2 -> tap centre
// ---------------------------------------------------------------------------
#define QC_P1X 36
#define QC_P1Y 36
#define QC_P2X 444
#define QC_P2Y 236

static lv_obj_t *s_qc = nullptr;
static lv_obj_t *s_qc_title = nullptr;
static lv_obj_t *s_qc_box = nullptr;
static lv_timer_t *s_qc_timer = nullptr;
static int s_qc_step = -1;
static bool s_qc_wait_release = false;
static int16_t s_qc_lastx = 0, s_qc_lasty = 0;
static int16_t s_qc_x1 = 0, s_qc_y1 = 0, s_qc_x2 = 0, s_qc_y2 = 0;

static void qc_close() {
    if (s_qc_timer) {
        lv_timer_del(s_qc_timer);
        s_qc_timer = nullptr;
    }
    if (s_qc) {
        lv_obj_del(s_qc);
        s_qc = nullptr;
        s_qc_title = nullptr;
        s_qc_box = nullptr;
        s_cal_dot = nullptr;
    }
    s_qc_step = -1;
}

static void qc_layout(int step) {
    s_qc_step = step;
    const char *msg = "";
    int bx = 0, by = 0, bw = 72, bh = 72;
    if (step == 0) {
        msg = "1/3  TAP TOP LEFT";
        bx = 0; by = 0;
    } else if (step == 1) {
        msg = "2/3  TAP BOTTOM RIGHT";
        bx = LCD_W - bw; by = LCD_H - bh;
    } else if (step == 2) {
        msg = "3/3  TAP CENTER";
        bw = 72; bh = 72;
        bx = LCD_W / 2 - bw / 2; by = LCD_H / 2 - bh / 2;
    } else {
        msg = "CAL OK";
    }
    if (!s_qc) return;
    lv_label_set_text(s_qc_title, msg);
    if (step >= 0 && step <= 2) {
        lv_obj_clear_flag(s_qc_box, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(s_qc_box, bx, by);
        lv_obj_set_size(s_qc_box, bw, bh);
    } else {
        lv_obj_add_flag(s_qc_box, LV_OBJ_FLAG_HIDDEN);
    }
    if (step >= 0 && step <= 2) s_qc_wait_release = true;
}

static void qc_timer_cb(lv_timer_t *t) {
    (void)t;
    if (!s_qc || s_qc_step < 0 || s_qc_step > 2) return;

    // Never capture while a finger is still down: the tap that answered the
    // previous step would otherwise be captured again for this one (that is
    // exactly the "p2 == p1 / too close" loop seen on the bench).
    if (s_qc_wait_release) {
        if (touch_pressed()) return;
        touch_capture_next();
        s_qc_wait_release = false;
        return;
    }

    int16_t rx = 0, ry = 0;
    if (!touch_capture_take(&rx, &ry)) return;

    // A leaked repeat of the same physical tap is never a new answer.
    if (s_qc_step > 0 && abs(rx - s_qc_lastx) < 15 && abs(ry - s_qc_lasty) < 15) {
        Serial.printf("[qc] duplicate (%d,%d), ignoring\n", rx, ry);
        touch_capture_next();
        return;
    }

    if (s_qc_step == 0) {
        s_qc_x1 = rx; s_qc_y1 = ry;
        s_qc_lastx = rx; s_qc_lasty = ry;
        Serial.printf("[qc] p1 raw=(%d,%d)\n", rx, ry);
        qc_layout(1);
    } else if (s_qc_step == 1) {
        s_qc_x2 = rx; s_qc_y2 = ry;
        s_qc_lastx = rx; s_qc_lasty = ry;
        Serial.printf("[qc] p2 raw=(%d,%d)\n", rx, ry);
        const int dxr = (int)s_qc_x2 - (int)s_qc_x1;
        const int dyr = (int)s_qc_y2 - (int)s_qc_y1;
        if (abs(dxr) < 80 || abs(dyr) < 50) {
            Serial.println("[qc] too close, restart");
            qc_layout(0);
            return;
        }
        TouchTransform t2;
        t2.ax = (float)(QC_P2X - QC_P1X) / (float)dxr;
        t2.bx = (float)QC_P1X - t2.ax * (float)s_qc_x1;
        t2.ay = (float)(QC_P2Y - QC_P1Y) / (float)dyr;
        t2.by = (float)QC_P1Y - t2.ay * (float)s_qc_y1;
        if (!touch_apply_transform(t2)) {
            Serial.println("[qc] transform rejected, restart");
            qc_layout(0);
            return;
        }
        touch_transform_save();
        Serial.printf("[qc] MAP ax=%.4f bx=%.2f ay=%.4f by=%.2f saved\n", t2.ax, t2.bx, t2.ay,
                      t2.by);
        qc_layout(2);
    } else {
        int32_t mx = 0, my = 0;
        touch_map_point(rx, ry, &mx, &my);
        const int ex = (int)mx - LCD_W / 2, ey = (int)my - LCD_H / 2;
        Serial.printf("[qc] verify raw=(%d,%d) map=(%d,%d) err=(%d,%d)\n", rx, ry, (int)mx,
                      (int)my, ex, ey);
        // The 2-point fit is exact for offset+scale+mirror, so a sloppy verify
        // tap must not throw the calibration away - it is already stored.
        if (abs(ex) < 90 && abs(ey) < 70) {
            qc_layout(3);
        } else {
            Serial.println("[qc] verify off, keeping stored map anyway");
            qc_layout(3);
        }
    }
}

void ui_quickcal_begin() {
    if (s_qc) return;
    s_qc = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_qc);
    lv_obj_set_pos(s_qc, 0, 0);
    lv_obj_set_size(s_qc, LCD_W, LCD_H);
    lv_obj_clear_flag(s_qc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_qc, lv_color_hex(0x101018), 0);
    lv_obj_set_style_bg_opa(s_qc, LV_OPA_COVER, 0);

    s_qc_box = lv_obj_create(s_qc);
    lv_obj_remove_style_all(s_qc_box);
    lv_obj_set_style_bg_color(s_qc_box, lv_color_hex(C_ORANGE), 0);
    lv_obj_set_style_bg_opa(s_qc_box, LV_OPA_70, 0);
    lv_obj_set_style_radius(s_qc_box, 10, 0);
    lv_obj_set_style_border_width(s_qc_box, 3, 0);
    lv_obj_set_style_border_color(s_qc_box, lv_color_hex(0xFFFFFF), 0);

    s_qc_title = lv_label_create(s_qc);
    lv_label_set_text(s_qc_title, "");
    lv_obj_set_style_text_font(s_qc_title, &ark24, 0);
    lv_obj_set_style_text_color(s_qc_title, lv_color_hex(C_TEXT), 0);
    lv_obj_align(s_qc_title, LV_ALIGN_CENTER, 0, 0);

    s_cal_dot = lv_obj_create(s_qc);
    lv_obj_remove_style_all(s_cal_dot);
    lv_obj_set_size(s_cal_dot, 12, 12);
    lv_obj_set_style_radius(s_cal_dot, 6, 0);
    lv_obj_set_style_bg_color(s_cal_dot, lv_color_hex(0xFF3030), 0);
    lv_obj_set_style_bg_opa(s_cal_dot, LV_OPA_COVER, 0);
    lv_obj_add_flag(s_cal_dot, LV_OBJ_FLAG_HIDDEN);

    s_qc_timer = lv_timer_create(qc_timer_cb, 40, nullptr);
    qc_layout(0);
}

void ui_quickcal_end() { qc_close(); }

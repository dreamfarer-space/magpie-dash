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
#include "i18n.h"
#include "netcfg.h"
#include "usb_link.h"
#include "util.h"

// Ark Pixel *monospaced*: ark10 / ark12 are 5 / 6 px per half-width cell (CJK
// takes two cells), ark24 is the 12 px cut drawn at 2x. Every column below is a
// multiple of the 6 px cell, which is what keeps the pages aligned.
LV_FONT_DECLARE(ark10);
LV_FONT_DECLARE(ark12);
LV_FONT_DECLARE(ark24);

// Tap-driven page switch (coalesces bursts of taps); defined with the page
// machinery further down. Programmatic switches use ui_goto().
static void page_request(int page);

// ------------------------------------------------------------------ palette
// One dark terminal theme: panes are outlined, never filled, and colour is
// reserved for state (green ok / amber busy / red error) and the agent brand.
#define C_BG 0x0B0D10
#define C_BAR 0x161A20     // top bar
#define C_LINE 0x2C333C    // pane borders, rules
#define C_DIM 0x1D232B     // unlit bar cells, inactive tabs, pressed rows
#define C_TEXT 0xD8DEE6
#define C_MUTED 0x7D8794
#define C_FAINT 0x4B535E
#define C_GREEN 0x4ADE80
#define C_GREEN_DIM 0x2C8A50
#define C_CYAN 0x5FC9E6
#define C_AMBER 0xF2B33D
#define C_RED 0xF0605A
#define C_PURPLE 0xA78BFA  // magpie

// screen chrome: a 18 px prompt bar on top, a 28 px tab bar at the bottom, the
// pages live in between and only that band repaints on a page switch
#define TOP_H 18
#define TAB_Y 244
#define TAB_H (LCD_H - TAB_Y)
#define PAGE_Y TOP_H
#define PAGE_H (TAB_Y - TOP_H)

#define AG_ROWS AGENT_MAX
#define RT_ROWS 12  // every request the bridge sends; the log scrolls

static lv_obj_t *pg[UI_PAGES];
static int s_page = 0;
static const DashData *s_data = nullptr;

// Agent the state pages and the home card are about. Empty = follow whichever
// agent made the latest request; tapping a row on the agent page pins one.
static char s_focus[16] = "";
static uint32_t s_focus_ms = 0;
#define FOCUS_HOLD_MS (10UL * 60UL * 1000UL)  // a pinned agent falls back after 10 min

static void link_panel_close();
static void kb_close();
static void qc_close();  // quick 2-point touch calibration overlay
static void link_panel_refresh();
static void close_overlays();

// ------------------------------------------------------------------ refs
static struct {
    // chrome
    lv_obj_t *tb_prompt, *tb_cursor, *tb_right, *tab_box[6], *tab_lbl[6];
    // settings page
    lv_obj_t *set_row[7], *set_val[7];
    // connection panel (settings -> 连接 magpie)
    lv_obj_t *lk, *lk_state, *lk_addr, *lk_hint, *lk_btn[2];
    // home
    lv_obj_t *hm_name, *hm_sub, *hm_state, *hm_dot, *hm_qk[2], *hm_qbar[2], *hm_qpct[2],
        *hm_today, *hm_30d, *hm_reset;
    lv_obj_t *mg_state, *mg_calls, *mg_tok, *mg_costk, *mg_cost, *mg_sess, *mg_last, *mg_cap,
        *mg_max, *mg_bars[SERIES_MAX], *mg_empty;
    // state pages (think / code / confirm)
    lv_obj_t *st_title_l[3], *st_sess[3], *st_title[3], *st_cursor[3], *st_sub[3], *st_name[3],
        *st_pip[3][3];
    lv_obj_t *st_l5[3], *st_l7[3], *st_p5[3], *st_p7[3], *st_bar5[3], *st_bar7[3], *st_reset[3],
        *st_conn[3], *st_conn_dot[3], *st_src[3];
    // agents
    lv_obj_t *ag_count, *ag_list, *ag_row[AG_ROWS], *ag_icon[AG_ROWS], *ag_name[AG_ROWS],
        *ag_detail[AG_ROWS], *ag_calls[AG_ROWS], *ag_live[AG_ROWS], *ag_state[AG_ROWS];
    char ag_id[AG_ROWS][16];
    // agent detail sheet (agents page)
    lv_obj_t *sh, *sh_name, *sh_model, *sh_num[4], *sh_share, *sh_bar, *sh_last, *sh_follow, *sh_close;
    char sh_id[16];
    // quota
    lv_obj_t *q_blk[QUOTA_MAX_PROV], *q_pane[QUOTA_MAX_PROV], *q_name[QUOTA_MAX_PROV],
        *q_user[QUOTA_MAX_PROV], *q_empty;
    lv_obj_t *q_key[QUOTA_MAX_PROV][QUOTA_MAX_WIN], *q_bar[QUOTA_MAX_PROV][QUOTA_MAX_WIN],
        *q_pct[QUOTA_MAX_PROV][QUOTA_MAX_WIN], *q_rst[QUOTA_MAX_PROV][QUOTA_MAX_WIN];
    // usage
    lv_obj_t *u_tok, *u_in, *u_out, *u_cache, *u_calls, *u_err, *u_cost, *u_price, *u_hit, *u_hit2;
    lv_obj_t *u_bars[SERIES_MAX], *u_lbl[5], *u_ymax, *u_empty;
    // routes
    lv_obj_t *r_sum[2], *r_empty[2], *r_count[2];
    lv_obj_t *r_row[2][RT_ROWS], *r_icon[2][RT_ROWS], *r_time[2][RT_ROWS], *r_agent[2][RT_ROWS],
        *r_model[2][RT_ROWS], *r_tag[2][RT_ROWS], *r_flow[2][RT_ROWS], *r_use[2][RT_ROWS],
        *r_scroll[2];
    lv_obj_t *r_top_icon, *r_top_agent, *r_top_to, *r_top_dot;
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

// right-aligned column: the text ends at xr whatever its length
static lv_obj_t *txt_r(lv_obj_t *p, const char *s, int xr, int y, int w, const lv_font_t *f,
                       uint32_t col) {
    lv_obj_t *l = txt_clip(p, s, xr - w, y, w, f, col);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
    return l;
}

static void fill(lv_obj_t *o, uint32_t col) {
    lv_obj_set_style_bg_color(o, lv_color_hex(col), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
}

// Outlined pane: a 1 px box, no fill (cheaper to paint, and it reads as a
// terminal window). Content goes inside as children, in pane coordinates.
static lv_obj_t *pane(lv_obj_t *p, int x, int y, int w, int h, uint32_t border = C_LINE) {
    lv_obj_t *c = obj(p, x, y, w, h);
    lv_obj_set_style_border_width(c, 1, 0);
    lv_obj_set_style_border_color(c, lv_color_hex(border), 0);
    return c;
}

// Title cut into a pane's top border, "┌─ title ───┐" style. It sits on the
// pane's parent (a child would be clipped by the pane), with a background
// patch that hides the border behind it. right=true hangs it off the far end;
// that one is aligned (not placed), so it stays flush when its text changes.
static lv_obj_t *pane_title(lv_obj_t *pane, const char *s, uint32_t col, bool right = false) {
    lv_obj_t *p = lv_obj_get_parent(pane);
    lv_obj_update_layout(pane);  // a fresh object reports 0,0 until laid out
    const int x = lv_obj_get_x(pane), y = lv_obj_get_y(pane) - 6;
    lv_obj_t *l = txt(p, s, x + 8, y, &ark12, col);
    fill(l, C_BG);
    lv_obj_set_style_pad_hor(l, 3, 0);
    if (right)
        lv_obj_align(l, LV_ALIGN_TOP_RIGHT, -(lv_obj_get_width(p) - x - lv_obj_get_width(pane) + 8), y);
    return l;
}

static lv_obj_t *dot(lv_obj_t *p, int x, int y, int s, uint32_t col) {
    lv_obj_t *d = obj(p, x, y, s, s);
    fill(d, col);
    return d;
}

static void hline(lv_obj_t *p, int x, int y, int w, uint32_t col) {
    lv_obj_t *l = obj(p, x, y, w, 1);
    fill(l, col);
}

// Rule between pane sections: one 1 px rectangle. It used to be an lv_line
// through ~110 collinear points (meant as dashes, but a polyline joins them, so
// it drew solid anyway); every segment was a separate line draw with its own
// mask, and with one rule per log row that was the slowest thing on the route
// page.
static void dline(lv_obj_t *p, int x, int y, int w) { hline(p, x, y, w, C_LINE); }

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

static void set_color(lv_obj_t *o, uint32_t col) {
    if (!o) return;
    if (lv_color_to32(lv_obj_get_style_text_color(o, 0)) == lv_color_to32(lv_color_hex(col))) return;
    lv_obj_set_style_text_color(o, lv_color_hex(col), 0);
}

static void set_bg(lv_obj_t *o, uint32_t col) {
    if (!o) return;
    if (lv_color_to32(lv_obj_get_style_bg_color(o, 0)) == lv_color_to32(lv_color_hex(col))) return;
    lv_obj_set_style_bg_color(o, lv_color_hex(col), 0);
}

// ------------------------------------------------------------------ bars
// A btop-style meter: one recoloured label of "■" cells (12 px each, with a
// gap built into the glyph). One object instead of one per segment, and the
// label only re-renders when the lit count or colour really changes.
static lv_obj_t *tbar(lv_obj_t *p, int x, int y, int cells) {
    lv_obj_t *l = txt(p, "", x, y, &ark12, C_DIM);
    lv_label_set_recolor(l, true);
    lv_obj_set_user_data(l, (void *)(intptr_t)cells);
    return l;
}

static void tbar_set(lv_obj_t *l, float pct, uint32_t col) {
    if (!l) return;
    const int cells = (int)(intptr_t)lv_obj_get_user_data(l);
    int lit = (int)(pct * cells / 100.0f + 0.5f);
    if (pct > 0 && lit == 0) lit = 1;  // anything left at all keeps one cell lit
    if (lit < 0) lit = 0;
    if (lit > cells) lit = cells;
    char b[32 * 3 + 40];
    int n = 0;
    if (lit) {
        n += snprintf(b + n, sizeof(b) - n, "#%06X ", (unsigned)col);
        for (int i = 0; i < lit; i++) n += snprintf(b + n, sizeof(b) - n, "■");
        n += snprintf(b + n, sizeof(b) - n, "#");
    }
    if (cells - lit) {
        n += snprintf(b + n, sizeof(b) - n, "#%06X ", (unsigned)C_DIM);
        for (int i = lit; i < cells; i++) n += snprintf(b + n, sizeof(b) - n, "■");
        n += snprintf(b + n, sizeof(b) - n, "#");
    }
    set_txt(l, b);
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
// An agent's avatar is its logo as is, straight on the pane: no tile, no
// ring, so the brand colours stay true. Tapping it "pokes" the agent: it hops
// and says one of its lines in a speech bubble (poke it five times quickly and
// it complains). What it is doing shows as three typing dots (thinking / at
// work), a ">_" badge (writing code) or an amber "!" (waiting for you).
enum { AV_IDLE, AV_THINK, AV_CODE, AV_CONFIRM };

struct Avatar {
    lv_obj_t *box, *logo, *badge, *badge_lbl, *bubble, *bubble_lbl, *tail, *dots[3];
    const AgentSkin *sk;
    bool big;     // 88 px logo (state pages) or 56 px
    bool below;   // bubble hangs under the avatar (tail points up)
    int cx;       // avatar centre, parent coordinates
    int mode;
    uint8_t pokes, say;
    uint32_t poke_ms;
    uint32_t breathe_ms;  // 0 = not breathing
};

static Avatar s_av_home, s_av_state[3], s_av_sheet;

// English catchphrases come from i18n.cpp (keyed by agent id); the shipped
// table in agent_sprites.h stays Chinese and is the fallback.
static const char *say_for(const AgentSkin *sk, int idx) {
    if (!sk) return "";
    const char *en = say_line(sk->id, idx);
    return en ? en : sk->say[idx & 1];
}

// The breathing loop asks for the same 0..-3 px offsets ~60 times a second;
// writing an unchanged style still invalidates the avatar, so skip those.
static void anim_translate_y(void *o, int32_t v) {
    lv_obj_t *obj = (lv_obj_t *)o;
    if (lv_obj_get_style_translate_y(obj, 0) != v) lv_obj_set_style_translate_y(obj, v, 0);
}
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
    // Keep the whole bubble inside its pane: a long line centred on an avatar
    // near the edge used to be cut off. The tail still points at the avatar.
    lv_obj_t *p = lv_obj_get_parent(a.bubble);
    lv_obj_update_layout(a.bubble);
    const int w = lv_obj_get_width(a.bubble), pw = lv_obj_get_width(p);
    int x = a.cx - w / 2;
    if (x > pw - w - 4) x = pw - w - 4;
    if (x < 4) x = 4;
    lv_obj_set_x(a.bubble, x);
    lv_obj_set_x(a.tail, a.cx - x - 4 - 6);  // child x starts after the 6 px padding
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
    avatar_say(a, a.pokes >= 5 ? T("别戳了 >_<") : say_for(a.sk, a.say++), 2200);
    avatar_hop(a);
    Serial.printf("[ui] poke %s x%d\n", a.sk->id, a.pokes);
}

static void avatar_poke_cb(lv_event_t *e) { avatar_poke(*(Avatar *)lv_event_get_user_data(e)); }

static void avatar_badge(Avatar &a, const char *s, uint32_t col) {
    show(a.badge, s != nullptr);
    if (!s) return;
    set_bg(a.badge, col);
    set_txt(a.badge_lbl, s);
}

static void avatar_mode(Avatar &a, int mode) {
    if (!a.box || a.mode == mode) return;
    a.mode = mode;
    for (int k = 0; k < 3; k++) show(a.dots[k], mode == AV_THINK);
    if (mode == AV_CODE)
        avatar_badge(a, ">_", C_CYAN);
    else if (mode == AV_CONFIRM)
        avatar_badge(a, "!", C_AMBER);
    else
        avatar_badge(a, nullptr, 0);
}

// typing dots: one lit dot walks along (called by a 350 ms timer)
static void avatar_step(Avatar &a, int phase) {
    if (!a.box || a.mode != AV_THINK || !lv_obj_is_visible(a.box)) return;
    const uint32_t on = a.sk ? a.sk->accent : C_TEXT;
    for (int k = 0; k < 3; k++) set_bg(a.dots[k], k == phase ? on : C_FAINT);
}

// logo centred on cx/cy; the speech bubble's top edge sits at bubble_y (below
// the avatar when bubble_y > cy)
static void avatar_create(Avatar &a, lv_obj_t *p, int cx, int cy, bool big, int bubble_y) {
    const int sz = big ? 88 : 56, bs = sz + 8;
    a.box = obj(p, cx - bs / 2, cy - bs / 2, bs, bs);
    lv_obj_add_flag(a.box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(a.box, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_event_cb(a.box, avatar_poke_cb, LV_EVENT_CLICKED, &a);
    a.logo = logo(a.box);
    lv_obj_align(a.logo, LV_ALIGN_CENTER, 0, 0);

    // corner badge: ">_" while coding, "!" while waiting
    a.badge = obj(a.box, bs - 18, -6, 24, 16);
    fill(a.badge, C_CYAN);
    a.badge_lbl = txt(a.badge, "", 0, 0, &ark12, C_BG);
    lv_obj_align(a.badge_lbl, LV_ALIGN_CENTER, 0, 0);
    show(a.badge, false);

    for (int k = 0; k < 3; k++) {
        a.dots[k] = dot(p, cx - 13 + k * 10, cy + bs / 2 + 4, 6, C_FAINT);
        show(a.dots[k], false);
    }

    a.below = bubble_y > cy;
    a.cx = cx;
    a.bubble = obj(p, 0, bubble_y, LV_SIZE_CONTENT, 20);
    fill(a.bubble, C_TEXT);
    lv_obj_set_style_pad_hor(a.bubble, 6, 0);
    lv_obj_add_flag(a.bubble, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    a.bubble_lbl = txt(a.bubble, "", 0, 0, &ark12, C_BG);
    lv_obj_align(a.bubble_lbl, LV_ALIGN_LEFT_MID, 0, 0);
    // tail points at the avatar; x is set per line in avatar_say()
    a.tail = obj(a.bubble, 0, a.below ? -4 : 20, 8, 4);
    lv_obj_add_flag(a.tail, LV_OBJ_FLAG_FLOATING);  // must not widen the bubble
    fill(a.tail, C_TEXT);
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
        snprintf(out, n, T("%d 小时额度"), num);
    else if (num && unit == 'd')
        snprintf(out, n, T("%d 天额度"), num);
    else if (num && unit == 'w')
        snprintf(out, n, T("%d 周额度"), num);
    else
        snprintf(out, n, "%s", group[0] ? group : name);
}

// When a quota window resets. The raw stamp is kept in the model and formatted
// here, so switching the interface language re-renders the date instead of
// leaving it in the language the fetch ran in; a provider that only sends a
// label ("205.26 / 900") keeps that label.
static void reset_text(const QuotaWindow &w, char *out, size_t n) {
    out[0] = 0;
    if (w.resetIso[0]) fmt_reset(w.resetIso, out, n);
    if (!out[0]) snprintf(out, n, "%s", w.reset);
}

static uint32_t remain_col(float rem) {
    return rem >= 50 ? C_GREEN : (rem >= 20 ? C_AMBER : C_RED);
}

// ------------------------------------------------------------------ chrome
// The prompt bar and the tab bar are built once and shared by every page, so a
// page switch repaints only the band between them (480x226 instead of the
// full 480x272) and the navigation never moves from page to page.
static const char *const PAGE_PATH[UI_PAGES] = {
    "~", "~/agent/thinking", "~/agent/coding", "~/agent/confirm", "~/agents",
    "~/quota", "~/usage", "~/routes", "~/routes/log", "~/config",
};
static const int PAGE_TAB[UI_PAGES] = {0, 0, 0, 0, 1, 2, 3, 4, 4, 5};
static const int TAB_PAGE[6] = {PG_HOME, PG_AGENTS, PG_QUOTA, PG_USAGE, PG_ROUTES, PG_SETTINGS};
static const int TAB_W = LCD_W / 6;

// Tab geometry lives here only: the hit-test regression asks for the centres, so
// a layout change can never leave the test tapping yesterday's coordinates.
int ui_tab_center_x(int index) { return index * TAB_W + TAB_W / 2; }
int ui_tab_center_y() { return TAB_Y + TAB_H / 2; }

static lv_obj_t *make_page() {
    lv_obj_t *p = obj(lv_scr_act(), 0, PAGE_Y, LCD_W, PAGE_H);
    fill(p, C_BG);
    return p;
}

static void tab_cb(lv_event_t *e) {
    page_request((int)(intptr_t)lv_event_get_user_data(e));
}

static void chrome_prompt() {
    if (!U.tb_prompt) return;
    char b[80];
    snprintf(b, sizeof(b), "#%06X %s@magpie#:#%06X %s#$", (unsigned)C_GREEN, g_cfg.handle,
             (unsigned)C_CYAN, PAGE_PATH[s_page]);
    if (strcmp(lv_label_get_text(U.tb_prompt), b)) {
        lv_label_set_text(U.tb_prompt, b);
        lv_obj_align_to(U.tb_cursor, U.tb_prompt, LV_ALIGN_OUT_RIGHT_MID, 3, 0);
    }
}

static void chrome_tabs() {
    const int on = PAGE_TAB[s_page];
    for (int i = 0; i < 6; i++) {
        set_bg(U.tab_box[i], i == on ? C_GREEN : C_DIM);
        set_color(U.tab_lbl[i], i == on ? C_BG : C_MUTED);
    }
}

static void build_chrome() {
    lv_obj_t *scr = lv_scr_act();
    // ---- prompt bar: user@magpie:~/page$ _              ● usb  10/20 周二 22:47
    lv_obj_t *top = obj(scr, 0, 0, LCD_W, TOP_H);
    fill(top, C_BAR);
    // Touch-only gestures (this panel has no usable BOOT button - GPIO0 is the
    // LCD TE line): long-press the bar for the quick touch calibration.
    lv_obj_add_flag(top, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(
        top,
        [](lv_event_t *e) {
            Serial.println("[ui] top bar long-press -> quick calibration");
            ui_quickcal_begin();
        },
        LV_EVENT_LONG_PRESSED, NULL);
    U.tb_prompt = txt(top, "", 6, 3, &ark12, C_TEXT);
    lv_label_set_recolor(U.tb_prompt, true);
    // tapping the prompt (it carries the handle) is the second way into settings
    lv_obj_add_flag(U.tb_prompt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(U.tb_prompt, 4);
    lv_obj_add_event_cb(U.tb_prompt, [](lv_event_t *e) { page_request(PG_SETTINGS); },
                        LV_EVENT_CLICKED, NULL);
    U.tb_cursor = obj(top, 0, 0, 6, 11);
    fill(U.tb_cursor, C_TEXT);
    U.tb_right = txt_r(top, "", LCD_W - 6, 3, 220, &ark12, C_MUTED);
    lv_label_set_recolor(U.tb_right, true);

    // ---- tab bar, htop style: "1[主页 ] 2[Agent] ..." with the page lit
    const char *names[6] = {T("主页"), "Agent", T("额度"), T("用量"), T("路由"), T("设置")};
    lv_obj_t *bar = obj(scr, 0, TAB_Y, LCD_W, TAB_H);
    fill(bar, C_BG);
    hline(bar, 0, 0, LCD_W, C_LINE);
    for (int i = 0; i < 6; i++) {
        lv_obj_t *b = obj(bar, i * TAB_W, 1, TAB_W, TAB_H - 1);
        char k[2] = {(char)('1' + i), 0};
        txt(b, k, 6, 8, &ark12, C_TEXT);
        // The label box is the button; 8 px of slack reaches over the key digit
        // and the bar edges while leaving a 4 px dead gap between neighbours.
        lv_obj_t *box = U.tab_box[i] = obj(b, 16, 4, TAB_W - 20, TAB_H - 9);
        fill(box, C_DIM);
        lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(box, 8);
        // Tabs switch on touch-down, not on release: a finger rests 80-150 ms
        // before it lifts, and waiting for that was most of the felt latency.
        // Safe here because the tab bar never scrolls, so a press is a tap.
        lv_obj_add_event_cb(box, tab_cb, LV_EVENT_PRESSED, (void *)(intptr_t)TAB_PAGE[i]);
        U.tab_lbl[i] = txt(box, names[i], 0, 0, &ark12, C_MUTED);
        lv_obj_center(U.tab_lbl[i]);
    }
}

// key / value line: muted key at x, value right-aligned at xr
static lv_obj_t *kv(lv_obj_t *p, const char *k, int x, int xr, int y, int vw) {
    txt(p, k, x, y, &ark12, C_MUTED);
    return txt_r(p, "--", xr, y, vw, &ark12, C_TEXT);
}

// hand-drawn bar chart (lv_chart drew outside its own area on this panel)
static void bars_create(lv_obj_t *p, lv_obj_t **bars, int x0, int base) {
    for (int i = 0; i < SERIES_MAX; i++) {
        bars[i] = obj(p, x0, base - 2, 4, 2);
        fill(bars[i], C_GREEN_DIM);
        show(bars[i], false);
    }
}

// lays the series out over [x0, x0+w), bars grow up from base; returns the
// slot width (0 when there is nothing to draw)
static int bars_fill(lv_obj_t **bars, const DashData &d, int x0, int w, int base, int h) {
    const int n = d.nseries;
    float mx = 0;
    for (int i = 0; i < n; i++)
        if (d.series[i].tokens > mx) mx = d.series[i].tokens;
    const int slot = n ? w / n : w;
    const int bw = slot > 4 ? slot - 2 : (slot > 1 ? slot - 1 : 1);
    for (int i = 0; i < SERIES_MAX; i++) {
        // Empty buckets must not draw a 2 px stub: a row of stubs reads as a
        // broken dashed baseline instead of "no usage".
        const bool on = i < n && mx > 0 && d.series[i].tokens > 0;
        show(bars[i], on);
        if (!on) continue;
        int bh = (int)(d.series[i].tokens / mx * h + 0.5f);
        if (bh < 2) bh = 2;
        lv_obj_set_pos(bars[i], x0 + i * slot, base - bh);
        lv_obj_set_size(bars[i], bw, bh);
        set_bg(bars[i], i == n - 1 ? C_GREEN : C_GREEN_DIM);  // the latest bucket stands out
    }
    return mx > 0 ? slot : 0;
}

// ------------------------------------------------------------------ page 1
static void build_home() {
    lv_obj_t *p = pg[PG_HOME] = make_page();

    // ---- left pane: the agent at work, as its avatar (tap it to poke it,
    // tap anywhere else on the pane for its status pages)
    lv_obj_t *cl = pane(p, 6, 8, 230, 210);
    pane_title(cl, "agent", C_MUTED);
    avatar_create(s_av_home, cl, 42, 44, false, 88);
    U.hm_name = txt_clip(cl, "--", 84, 14, 138, &ark12, C_TEXT);
    U.hm_sub = txt_clip(cl, "", 84, 30, 138, &ark12, C_MUTED);
    U.hm_dot = dot(cl, 84, 52, 6, C_FAINT);
    U.hm_state = txt_clip(cl, T("等待网关"), 96, 48, 138, &ark12, C_MUTED);
    dline(cl, 8, 104, 214);
    for (int k = 0; k < 2; k++) {
        U.hm_qk[k] = txt(cl, k ? "7d" : "5h", 8, 114 + k * 18, &ark12, C_MUTED);
        U.hm_qbar[k] = tbar(cl, 42, 114 + k * 18, 12);
        U.hm_qpct[k] = txt_r(cl, "--", 222, 114 + k * 18, 36, &ark12, C_TEXT);
    }
    U.hm_today = kv(cl, T("今天调用"), 8, 222, 158, 100);
    U.hm_30d = kv(cl, T("30 天调用"), 8, 222, 174, 100);
    U.hm_reset = txt_clip(cl, "", 8, 190, 214, &ark12, C_FAINT);
    lv_obj_add_flag(cl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_color(cl, lv_color_hex(C_MUTED), LV_STATE_PRESSED);
    lv_obj_add_event_cb(cl, [](lv_event_t *e) { page_request(PG_STATE_THINK); }, LV_EVENT_CLICKED, NULL);

    // ---- right pane: magpie, today at a glance
    lv_obj_t *cr = pane(p, 244, 8, 230, 210);
    pane_title(cr, "magpie", C_PURPLE);
    U.mg_state = pane_title(cr, "offline", C_FAINT, true);
    U.mg_calls = txt(cr, "--", 10, 12, &ark24, C_TEXT);
    txt(cr, T("今天调用"), 10, 40, &ark12, C_MUTED);
    U.mg_tok = kv(cr, "token", 10, 220, 62, 120);
    U.mg_cost = kv(cr, T("费用"), 10, 220, 78, 120);
    U.mg_costk = lv_obj_get_child(cr, lv_obj_get_child_cnt(cr) - 2);  // "费用 30d" key
    U.mg_sess = kv(cr, T("会话"), 10, 220, 94, 120);
    U.mg_last = kv(cr, T("最近"), 10, 220, 110, 120);
    dline(cr, 8, 130, 214);
    U.mg_cap = txt(cr, "token / 30d", 10, 138, &ark12, C_MUTED);
    U.mg_max = txt_r(cr, "", 220, 138, 80, &ark12, C_FAINT);
    bars_create(cr, U.mg_bars, 10, 200);
    U.mg_empty = txt(cr, T("暂无数据"), 0, 0, &ark12, C_FAINT);
    lv_obj_align(U.mg_empty, LV_ALIGN_TOP_MID, 0, 170);
    lv_obj_add_flag(cr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_color(cr, lv_color_hex(C_MUTED), LV_STATE_PRESSED);
    lv_obj_add_event_cb(cr, [](lv_event_t *e) { page_request(PG_AGENTS); }, LV_EVENT_CLICKED, NULL);
}

// ------------------------------------------------------------------ state pages
static void build_state(int idx, int page_id, bool confirm) {
    lv_obj_t *p = pg[page_id] = make_page();
    avatar_create(s_av_state[idx], p, 96, 82, true, 8);
    avatar_mode(s_av_state[idx], idx == 0 ? AV_THINK : (idx == 1 ? AV_CODE : AV_CONFIRM));

    // three pips under the avatar: which state page this is, tap one to switch
    for (int k = 0; k < 3; k++) {
        lv_obj_t *hit = obj(p, 96 - 66 + k * 44, 142, 44, 32);
        lv_obj_add_flag(hit, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(hit, tab_cb, LV_EVENT_CLICKED, (void *)(intptr_t)(PG_STATE_THINK + k));
        lv_obj_t *pip = U.st_pip[idx][k] = obj(hit, k == idx ? 6 : 15, 13, k == idx ? 32 : 14, 6);
        fill(pip, C_DIM);
    }
    U.st_name[idx] = txt_clip(p, "--", 6, 186, 180, &ark12, C_TEXT);
    lv_obj_set_style_text_align(U.st_name[idx], LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *c = pane(p, 192, 8, 282, 210, confirm ? C_AMBER : C_LINE);
    U.st_title_l[idx] = pane_title(c, "claude", C_MUTED);
    U.st_sess[idx] = pane_title(c, T("-- 个会话"), C_MUTED, true);

    U.st_title[idx] = txt(c, confirm ? T("! 等你确认") : (idx == 0 ? T("> 斟酌中") : T("> 编写代码")), 10, 8,
                          &ark24, confirm ? C_AMBER : C_TEXT);
    U.st_cursor[idx] = obj(c, 0, 0, 12, 20);
    fill(U.st_cursor[idx], confirm ? C_AMBER : C_TEXT);
    lv_obj_align_to(U.st_cursor[idx], U.st_title[idx], LV_ALIGN_OUT_RIGHT_MID, 4, 0);
    U.st_sub[idx] = txt_clip(c, "", 10, 38, 262, &ark12, C_MUTED);
    dline(c, 8, 58, 266);

    U.st_l5[idx] = txt(c, T("5 小时额度"), 10, 68, &ark12, C_TEXT);
    U.st_p5[idx] = txt_r(c, "--", 270, 68, 60, &ark12, C_TEXT);
    U.st_bar5[idx] = tbar(c, 10, 84, 21);
    U.st_l7[idx] = txt(c, T("7 天额度"), 10, 104, &ark12, C_TEXT);
    U.st_p7[idx] = txt_r(c, "--", 270, 104, 60, &ark12, C_TEXT);
    U.st_bar7[idx] = tbar(c, 10, 120, 21);

    U.st_reset[idx] = txt_clip(c, "", 10, 142, 262, &ark12, C_MUTED);
    U.st_src[idx] = txt_clip(c, "", 10, 158, 262, &ark12, C_MUTED);
    U.st_conn_dot[idx] = dot(c, 10, 190, 6, C_FAINT);
    U.st_conn[idx] = txt(c, T("未连接"), 22, 186, &ark12, C_MUTED);
}

// ------------------------------------------------------------------ agents
// Tapping a row opens the agent's sheet: its avatar (pokeable), what it ran
// in the last 30 days, and a button to follow it on the home pane and the
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
    avatar_say(s_av_sheet, say_for(s_av_sheet.sk, 0), 1800);
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

// outlined terminal button; filled when it is the primary action
static lv_obj_t *button(lv_obj_t *p, int x, int y, int w, const char *s, uint32_t bg, uint32_t col) {
    lv_obj_t *b = obj(p, x, y, w, 28);
    lv_obj_set_style_bg_color(b, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(b, bg == C_BG ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(bg == C_BG ? C_LINE : bg), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_LINE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *l = txt(b, s, 0, 0, &ark12, col);
    lv_obj_align(l, LV_ALIGN_CENTER, 0, 0);
    return b;
}

// fixed columns shared by the header and every row
#define AGC_NAME 34
#define AGC_MODEL 190
#define AGC_STATE 352
#define AGC_CALLS 456

static void build_agents() {
    lv_obj_t *p = pg[PG_AGENTS] = make_page();

    // every agent magpie manages: more than fit, so the list scrolls
    lv_obj_t *c = pane(p, 6, 8, 468, 210);
    pane_title(c, "agents", C_MUTED);
    U.ag_count = pane_title(c, "--", C_MUTED, true);
    txt(c, "NAME", AGC_NAME, 6, &ark10, C_FAINT);
    txt(c, "MODEL", AGC_MODEL, 6, &ark10, C_FAINT);
    txt(c, "STATE", AGC_STATE, 6, &ark10, C_FAINT);
    txt_r(c, "30D", AGC_CALLS, 6, 40, &ark10, C_FAINT);
    hline(c, 8, 19, 452, C_DIM);

    lv_obj_t *sc = U.ag_list = obj(c, 1, 21, 466, 188);
    lv_obj_add_flag(sc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(sc, agent_input_trace, LV_EVENT_ALL, (void *)(intptr_t)-1);
    lv_obj_set_scroll_dir(sc, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sc, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_style_width(sc, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(sc, lv_color_hex(C_MUTED), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(sc, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(sc, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < AG_ROWS; i++) {
        lv_obj_t *r = U.ag_row[i] = obj(sc, 0, 0, 466, 30);
        lv_obj_add_flag(r, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(r, lv_color_hex(C_DIM), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(r, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_add_event_cb(r, agent_input_trace, LV_EVENT_ALL, (void *)(intptr_t)i);
        lv_obj_add_event_cb(r, agent_row_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        U.ag_icon[i] = logo(r);
        lv_obj_set_pos(U.ag_icon[i], 7, 5);
        U.ag_live[i] = dot(r, 23, 19, 6, C_GREEN);  // working right now
        lv_obj_set_style_border_width(U.ag_live[i], 1, 0);
        lv_obj_set_style_border_color(U.ag_live[i], lv_color_hex(C_BG), 0);
        U.ag_name[i] = txt_clip(r, "--", AGC_NAME - 1, 9, 150, &ark12, C_TEXT);
        U.ag_detail[i] = txt_clip(r, "", AGC_MODEL - 1, 9, 156, &ark12, C_MUTED);
        U.ag_state[i] = txt(r, "", AGC_STATE - 1, 9, &ark12, C_MUTED);
        U.ag_calls[i] = txt_r(r, "", AGC_CALLS - 1, 9, 50, &ark12, C_GREEN);
        show(r, false);
    }

    // ---- the sheet, over the list: a full-page backdrop (it also hides the
    // list's own pane titles) holding a pane of its own
    U.sh = obj(p, 0, 0, LCD_W, PAGE_H);
    fill(U.sh, C_BG);
    lv_obj_add_flag(U.sh, LV_OBJ_FLAG_CLICKABLE);  // swallow taps meant for rows below
    lv_obj_t *s = pane(U.sh, 6, 8, 468, 210, C_CYAN);
    lv_obj_add_flag(s, LV_OBJ_FLAG_EVENT_BUBBLE);
    pane_title(s, "agent", C_CYAN);
    avatar_create(s_av_sheet, s, 56, 74, false, 8);
    avatar_mode(s_av_sheet, AV_IDLE);
    txt(s, T("轻触头像"), 32, 128, &ark12, C_FAINT);

    U.sh_name = txt_clip(s, "--", 120, 10, 336, &ark12, C_TEXT);
    U.sh_model = txt_clip(s, "", 120, 26, 336, &ark12, C_MUTED);
    dline(s, 120, 46, 336);
    const char *labels[4] = {T("30 天调用"), T("今天"), "Token", T("费用")};
    for (int k = 0; k < 4; k++) {
        const int x = 120 + (k % 2) * 176, y = 56 + (k / 2) * 18;
        U.sh_num[k] = kv(s, labels[k], x, x + 160, y, 80);
    }
    U.sh_share = txt(s, "", 120, 98, &ark12, C_MUTED);
    U.sh_bar = tbar(s, 120, 114, 28);
    U.sh_last = txt_clip(s, "", 120, 136, 336, &ark12, C_MUTED);
    lv_obj_t *cb = U.sh_close = button(s, 288, 170, 76, T("关闭"), C_BG, C_MUTED);
    lv_obj_add_event_cb(cb, [](lv_event_t *e) { sheet_close(); }, LV_EVENT_CLICKED, NULL);
    U.sh_follow = button(s, 372, 170, 86, T("跟随"), C_CYAN, C_BG);
    lv_obj_add_event_cb(U.sh_follow, sheet_follow_cb, LV_EVENT_CLICKED, NULL);
    show(U.sh, false);
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
        lv_obj_get_coords(U.sh_close, &close_area);
        display_test_tap((close_area.x1 + close_area.x2) / 2,
                         (close_area.y1 + close_area.y2) / 2);
        const bool closed = lv_obj_has_flag(U.sh, LV_OBJ_FLAG_HIDDEN);
        cases++;
        // Opening details should show data, not launch a blocking ornamental hop.
        // The idle breathing loop is fine; only a hop (it starts at -14) is not.
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
// One pane per provider, one meter line per quota window. Panes stack in a
// flex column, so a provider with four windows simply takes more room and the
// rest scroll under it.
#define QL_STEP 18

static void build_quota() {
    lv_obj_t *p = pg[PG_QUOTA] = make_page();

    lv_obj_t *sc = obj(p, 0, 0, LCD_W, PAGE_H);
    lv_obj_add_flag(sc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(sc, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sc, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_style_width(sc, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(sc, lv_color_hex(C_MUTED), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(sc, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(sc, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(sc, 2, 0);
    lv_obj_set_style_pad_row(sc, 4, 0);
    lv_obj_set_style_pad_bottom(sc, 8, 0);
    U.q_empty = txt(sc, T("暂无额度数据"), 0, 0, &ark12, C_MUTED);
    lv_obj_add_flag(U.q_empty, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(U.q_empty, LV_ALIGN_TOP_MID, 0, 100);

    for (int i = 0; i < QUOTA_MAX_PROV; i++) {
        lv_obj_t *b = U.q_blk[i] = obj(sc, 0, 0, LCD_W, 6 + 12 + QUOTA_MAX_WIN * QL_STEP);
        lv_obj_add_flag(b, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_t *c = U.q_pane[i] = pane(b, 6, 6, 468, 12 + QUOTA_MAX_WIN * QL_STEP);
        lv_obj_add_flag(c, LV_OBJ_FLAG_EVENT_BUBBLE);
        U.q_name[i] = pane_title(c, "--", C_TEXT);
        U.q_user[i] = pane_title(c, "", C_MUTED, true);
        for (int j = 0; j < QUOTA_MAX_WIN; j++) {
            const int y = 9 + j * QL_STEP;
            U.q_key[i][j] = txt_clip(c, "", 10, y, 96, &ark12, C_TEXT);
            U.q_bar[i][j] = tbar(c, 110, y, 16);
            U.q_pct[i][j] = txt_r(c, "", 344, y, 40, &ark12, C_TEXT);
            U.q_rst[i][j] = txt_clip(c, "", 352, y, 108, &ark12, C_FAINT);
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
        set_bg(u_filter[i], sel ? C_GREEN : C_DIM);
        set_color(lv_obj_get_child(u_filter[i], 0), sel ? C_BG : C_MUTED);
    }
    app_request_refresh();
}

void ui_cycle_period() {
    filter_apply((s_filter + 1) % 4);
    static const char *names[4] = {"today", "7d", "30d", "all"};
    Serial.printf("[ui] usage period -> %s\n", names[s_filter]);
}

static void filter_cb(lv_event_t *e) { filter_apply((int)(intptr_t)lv_event_get_user_data(e)); }

#define CH_X0 46
#define CH_W 420
#define CH_BASE 202
#define CH_H 36

static void build_usage() {
    lv_obj_t *p = pg[PG_USAGE] = make_page();

    const char *fname[4] = {T("今天"), T("7 天"), T("30 天"), T("全部")};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *b = obj(p, 6 + i * 58, 4, 54, 22);
        fill(b, i == s_filter ? C_GREEN : C_DIM);
        lv_obj_t *l = txt(b, fname[i], 0, 0, &ark12, i == s_filter ? C_BG : C_MUTED);
        lv_obj_center(l);
        lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(b, 2);
        lv_obj_add_event_cb(b, filter_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        u_filter[i] = b;
    }
    txt_r(p, T("src: 网关 + 会话日志"), LCD_W - 6, 9, 200, &ark12, C_FAINT);

    // 4 KPI panes
    const char *titles[4] = {"token", T("请求"), T("费用"), T("缓存命中")};
    const uint32_t vals_col[4] = {C_GREEN, C_TEXT, C_AMBER, C_CYAN};
    lv_obj_t *v[4], *c[4];
    for (int i = 0; i < 4; i++) {
        c[i] = pane(p, 6 + i * 119, 40, 111, 92);
        pane_title(c[i], titles[i], C_MUTED);
        v[i] = txt(c[i], "--", 8, 10, &ark24, vals_col[i]);
    }
    U.u_tok = v[0];
    U.u_calls = v[1];
    U.u_cost = v[2];
    U.u_hit = v[3];
    U.u_in = txt(c[0], "", 8, 42, &ark12, C_MUTED);
    U.u_out = txt(c[0], "", 8, 56, &ark12, C_MUTED);
    U.u_cache = txt(c[0], "", 8, 70, &ark12, C_MUTED);
    U.u_err = txt(c[1], "", 8, 42, &ark12, C_RED);
    U.u_price = txt_clip(c[2], "", 8, 42, 96, &ark12, C_MUTED);  // average cost per call
    U.u_hit2 = txt(c[3], T("占提示词"), 8, 42, &ark12, C_MUTED);

    lv_obj_t *ch = pane(p, 6, 144, 468, 74);
    pane_title(ch, T("token / 时段"), C_MUTED);
    U.u_ymax = txt_r(p, "--", CH_X0 - 6, CH_BASE - CH_H - 4, 36, &ark10, C_FAINT);
    txt_r(p, "0", CH_X0 - 6, CH_BASE - 9, 36, &ark10, C_FAINT);
    hline(p, CH_X0, CH_BASE, CH_W, C_LINE);
    bars_create(p, U.u_bars, CH_X0, CH_BASE);
    for (int i = 0; i < 5; i++) {
        U.u_lbl[i] = txt(p, "", 0, CH_BASE + 3, &ark10, C_FAINT);
        lv_obj_set_width(U.u_lbl[i], 40);
        lv_obj_set_style_text_align(U.u_lbl[i], LV_TEXT_ALIGN_CENTER, 0);
    }
    U.u_empty = txt(p, T("暂无数据"), 0, 0, &ark12, C_FAINT);
    lv_obj_align(U.u_empty, LV_ALIGN_TOP_MID, 0, 176);
}

// ------------------------------------------------------------------ routes
// fixed columns of the request log
#define RC_TIME 8
#define RC_ICON 62
#define RC_AGENT 86
#define RC_MODEL 196
#define RC_RIGHT 456

static void build_routes(int page_id) {
    lv_obj_t *p = pg[page_id] = make_page();
    const int idx = (page_id == PG_ROUTES) ? 0 : 1;

    // summary line; tapping it expands to the log-only page and back
    lv_obj_t *sum = obj(p, 0, 0, LCD_W, 24);
    lv_obj_add_flag(sum, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(sum, tab_cb, LV_EVENT_CLICKED,
                        (void *)(intptr_t)(idx ? PG_ROUTES : PG_ROUTES_LIST));
    U.r_sum[idx] = txt(sum, T("今天 -- 次调用 · -- token"), 6, 6, &ark12, C_TEXT);
    txt_r(sum, idx ? T("收起 «") : T("展开 »"), LCD_W - 6, 6, 80, &ark12, C_MUTED);

    int y0;
    if (idx == 0) {
        // topology: agent ──▶ magpie ──▶ upstream account
        lv_obj_t *t = pane(p, 6, 30, 468, 32);
        pane_title(t, "route", C_MUTED);
        U.r_top_icon = logo(t);
        lv_obj_set_pos(U.r_top_icon, 8, 5);
        U.r_top_agent = txt_clip(t, "--", 34, 9, 110, &ark12, C_TEXT);
        txt(t, "──▶", 148, 9, &ark12, C_FAINT);
        txt(t, "magpie", 188, 9, &ark12, C_PURPLE);
        txt(t, "──▶", 230, 9, &ark12, C_FAINT);
        U.r_top_dot = dot(t, 272, 13, 6, C_GREEN);
        U.r_top_to = txt_clip(t, "--", 284, 9, 176, &ark12, C_MUTED);
        y0 = 74;
    } else {
        y0 = 32;
    }

    lv_obj_t *c = pane(p, 6, y0, 468, 218 - y0);
    pane_title(c, "log", C_MUTED);
    U.r_count[idx] = pane_title(c, "", C_MUTED, true);
    lv_obj_t *host = U.r_scroll[idx] = obj(c, 1, 5, 466, 218 - y0 - 8);
    // Dragging needs CLICKABLE, and the bar is always on: with ACTIVE mode it
    // stayed invisible until dragged, so nobody knew there were more rows.
    lv_obj_add_flag(host, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(host, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(host, LV_SCROLLBAR_MODE_ON);
    lv_obj_set_style_width(host, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(host, lv_color_hex(C_LINE), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(host, LV_OPA_COVER, LV_PART_SCROLLBAR);
    U.r_empty[idx] = txt(host, T("暂无调用"), 0, 0, &ark12, C_MUTED);
    lv_obj_center(U.r_empty[idx]);

    const int step = 34;
    for (int i = 0; i < RT_ROWS; i++) {
        lv_obj_t *r = U.r_row[idx][i] = obj(host, 0, i * step, 460, step);
        if (i) dline(r, 8, 0, 448);
        U.r_time[idx][i] = txt(r, "--:--:--", RC_TIME, 4, &ark12, C_FAINT);
        U.r_icon[idx][i] = logo(r);
        lv_obj_set_pos(U.r_icon[idx][i], RC_ICON, 6);
        U.r_agent[idx][i] = txt_clip(r, "--", RC_AGENT, 4, 104, &ark12, C_TEXT);
        U.r_model[idx][i] = txt_clip(r, "--", RC_MODEL, 4, 170, &ark12, C_CYAN);
        U.r_tag[idx][i] = txt_r(r, "", RC_RIGHT, 4, 84, &ark12, C_MUTED);
        U.r_flow[idx][i] = txt_clip(r, "", RC_AGENT, 19, 220, &ark12, C_MUTED);
        U.r_use[idx][i] = txt_r(r, "", RC_RIGHT, 19, 150, &ark12, C_MUTED);
        show(r, false);
    }
}


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
        lv_obj_set_style_bg_color(b, lv_color_hex(C_AMBER), 0);
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
    lv_obj_set_style_text_color(c, lv_color_hex(C_GREEN), 0);
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
// the choices. Rows are 466x34 with the label left and the current value right;
// tapping cycles the value, except the name row which opens a keyboard and the
// link row which opens the connection panel.
static bool s_cfg_dirty = false;      // pending NVS write, handled off the tap path
static lv_obj_t *s_kb = nullptr;      // keyboard overlay (nullptr when closed)
static char s_kb_buf[20];
static lv_obj_t *s_kb_preview = nullptr;

static void kb_redraw_preview() {
    if (!s_kb_preview) return;
    char line[48];
    snprintf(line, sizeof(line), "#%06X name>#  @%s", (unsigned)C_GREEN, s_kb_buf);
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
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(C_LINE), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_GREEN), LV_STATE_PRESSED);
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
    fill(s_kb, C_BG);
    lv_obj_set_style_border_width(s_kb, 1, 0);
    lv_obj_set_style_border_color(s_kb, lv_color_hex(C_GREEN), 0);
    lv_obj_add_flag(s_kb, LV_OBJ_FLAG_CLICKABLE);  // swallow taps behind it

    s_kb_preview = txt(s_kb, "", 8, 8, &ark12, C_TEXT);
    lv_label_set_recolor(s_kb_preview, true);
    kb_redraw_preview();
    lv_obj_t *ok = button(s_kb, LCD_W - 96, 2, 46, T("保存"), C_GREEN, C_BG);
    lv_obj_set_ext_click_area(ok, 2);
    lv_obj_add_event_cb(ok, [](lv_event_t *e) { kb_apply(); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *no = button(s_kb, LCD_W - 46, 2, 40, T("取消"), C_BG, C_MUTED);
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
    kb_key(s_kb, tall_x, 32, tall_w, 68, T("删"), '\b');  // "⌫" is not in the subset font
    kb_key(s_kb, tall_x, 104, tall_w, 68, T("清空"), 0);
    lv_obj_t *clr = lv_obj_get_child(s_kb, lv_obj_get_child_cnt(s_kb) - 1);
    lv_obj_add_event_cb(clr, [](lv_event_t *e) {
        s_kb_buf[0] = 0;
        kb_redraw_preview();
    }, LV_EVENT_CLICKED, NULL);
}

// Row order of the config page. The language row joined late: seven rows now
// share the same 210 px pane, so the pitch is 29 px instead of 34.
#define SET_ROWS 7
#define SET_PITCH 29
enum { ROW_NAME = 0, ROW_QUOTA, ROW_REFRESH, ROW_ANIM, ROW_PRIVACY, ROW_LANG, ROW_LINK };

static void row_cycle_cb(lv_event_t *e) {
    const int which = (int)(intptr_t)lv_event_get_user_data(e);
    switch (which) {
        case ROW_QUOTA: g_cfg.quota_mode = !g_cfg.quota_mode; break;
        case ROW_REFRESH: g_cfg.refresh_s = cfg_next_refresh(g_cfg.refresh_s); break;
        case ROW_ANIM: g_cfg.anim = !g_cfg.anim; break;
        case ROW_PRIVACY: g_cfg.privacy = !g_cfg.privacy; break;
        case ROW_LANG:
            // Rebuilds the whole tree, so it cannot run inside this tap handler.
            ui_set_language(lang_get() == LANG_EN ? LANG_ZH : LANG_EN);
            return;
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
        set_txt(U.lk_state, usb ? T("● USB · 等待电脑端桥接") : T("● WiFi · 未连接"));
    else
        set_txtf(U.lk_state, T("● %s · 已连接"), live);
    set_color(U.lk_state, strcmp(live, "-") ? C_GREEN : C_AMBER);

    if (usb)
        set_txt(U.lk_addr, "$ py tools/usb_bridge.py");
    else {
        snprintf(b, sizeof(b), T("网关 %s:%u"), netcfg_host(), (unsigned)netcfg_port());
        set_txt(U.lk_addr, b);
    }
    if (usb)
        set_txt(U.lk_hint, T("插上 USB 线就能用，不用路由器"));
    else
        set_txt(U.lk_hint, T("手机连 WiFi magpie-dash-setup 配网"));

    for (int i = 0; i < 2; i++) {
        if (!U.lk_btn[i]) continue;
        const bool sel = (i == 0) == usb;
        set_bg(U.lk_btn[i], sel ? C_GREEN : C_DIM);
        set_color(lv_obj_get_child(U.lk_btn[i], 0), sel ? C_BG : C_MUTED);
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
    lv_obj_t *b = obj(p, x, y, w, 28);
    fill(b, C_DIM);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_LINE), LV_STATE_PRESSED);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(b, 3);
    lv_obj_t *l = txt(b, label, 0, 0, &ark12, C_MUTED);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, link_mode_cb, LV_EVENT_CLICKED, (void *)(intptr_t)mode);
    U.lk_btn[mode] = b;
}

static void link_panel_open() {
    if (s_link) return;
    s_link = obj(lv_layer_top(), 0, PAGE_Y, LCD_W, PAGE_H);
    fill(s_link, C_BG);
    lv_obj_add_flag(s_link, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *c = pane(s_link, 6, 8, 468, 210, C_CYAN);
    lv_obj_add_flag(c, LV_OBJ_FLAG_EVENT_BUBBLE);
    pane_title(c, T("连接 magpie"), C_CYAN);

    lv_obj_t *x = button(c, 468 - 66, 8, 56, T("关闭"), C_BG, C_MUTED);
    lv_obj_set_ext_click_area(x, 2);
    lv_obj_add_event_cb(x, [](lv_event_t *e) { link_panel_close(); }, LV_EVENT_CLICKED, NULL);

    link_mode_btn(c, 10, 8, 160, T("USB 一根线"), 0);
    link_mode_btn(c, 176, 8, 160, T("WiFi 局域网"), 1);

    U.lk_state = txt(c, "--", 10, 52, &ark12, C_TEXT);
    U.lk_addr = txt(c, "--", 10, 72, &ark12, C_TEXT);
    U.lk_hint = txt(c, "--", 10, 88, &ark12, C_FAINT);
    dline(c, 8, 112, 450);

    lv_obj_t *retry = button(c, 10, 124, 104, T("重新连接"), C_BG, C_TEXT);
    lv_obj_set_ext_click_area(retry, 2);
    lv_obj_add_event_cb(retry, [](lv_event_t *e) {
        if (g_cfg.link_mode == 0)
            app_request_refresh();  // ask the host bridge for data now
        else if (!netcfg_retry())
            Serial.println("[ui] no WiFi credentials yet -> portal");
        link_panel_refresh();
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *portal = button(c, 122, 124, 110, T("WiFi 配网"), C_BG, C_TEXT);
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

    lv_obj_t *c = pane(p, 6, 8, 468, 210);
    pane_title(c, "config", C_MUTED);
    const char *labels[SET_ROWS] = {T("显示名称"), T("用量显示"), T("刷新间隔"), T("头像动效"),
                                    T("隐藏金额"), T("语言"), T("连接 magpie")};
    for (int i = 0; i < SET_ROWS; i++) {
        lv_obj_t *r = U.set_row[i] = obj(c, 1, 3 + i * SET_PITCH, 466, SET_PITCH);
        if (i < SET_ROWS - 1) {
            lv_obj_set_style_border_width(r, 1, 0);
            lv_obj_set_style_border_side(r, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_color(r, lv_color_hex(C_DIM), 0);
        }
        lv_obj_set_style_bg_color(r, lv_color_hex(C_DIM), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(r, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_add_flag(r, LV_OBJ_FLAG_CLICKABLE);
        txt(r, labels[i], 10, 8, &ark12, C_MUTED);
        U.set_val[i] = txt_r(r, "--", 440, 8, 260, &ark12, C_TEXT);
        txt(r, "»", 448, 8, &ark12, C_FAINT);
        if (i == ROW_NAME)
            lv_obj_add_event_cb(r, row_name_cb, LV_EVENT_CLICKED, NULL);
        else if (i == ROW_LINK)
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

// the top bar's link indicator: the pipe that is really carrying data
static void chrome_status() {
    if (!U.tb_right) return;
    const char *live = gw_active_link();
    const bool up = strcmp(live, "-") != 0;
    char clock[24] = "";
    const time_t now = time(nullptr);
    struct tm tmv;
    localtime_r(&now, &tmv);
    const char *wd[7] = {T("周日"), T("周一"), T("周二"), T("周三"), T("周四"), T("周五"), T("周六")};
    if (tmv.tm_year > 120)
        snprintf(clock, sizeof(clock), "%d/%d %s %02d:%02d", tmv.tm_mon + 1, tmv.tm_mday,
                 wd[tmv.tm_wday], tmv.tm_hour, tmv.tm_min);
    else
        snprintf(clock, sizeof(clock), "--:--");
    set_txtf(U.tb_right, "#%06X ●# %s   #%06X %s#", (unsigned)(up ? C_GREEN : C_AMBER),
             up ? (strcmp(live, "USB") ? "wifi" : "usb") : "offline", (unsigned)C_TEXT, clock);
}

void ui_settings_refresh() {
    if (!U.set_val[0]) return;
    char b[40];
    snprintf(b, sizeof(b), "@%s", g_cfg.handle);
    set_txt(U.set_val[0], b);
    set_txt(U.set_val[ROW_QUOTA], g_cfg.quota_mode ? T("已用") : T("剩余"));
    snprintf(b, sizeof(b), T("%u 秒"), (unsigned)g_cfg.refresh_s);
    set_txt(U.set_val[ROW_REFRESH], b);
    set_txt(U.set_val[ROW_ANIM], g_cfg.anim ? T("开") : T("关"));
    set_txt(U.set_val[ROW_PRIVACY], g_cfg.privacy ? T("开") : T("关"));
    // the row names the language in use, written in that language
    set_txt(U.set_val[ROW_LANG], lang_get() == LANG_EN ? "English" : "中文");
    if (U.set_val[ROW_LINK]) {
        // Show the pipe that is really carrying data, not just the preference:
        // with the fallback the two can differ.
        const char *live = gw_active_link();
        if (!strcmp(live, "USB")) set_txt(U.set_val[ROW_LINK], T("USB · 已连接"));
        else if (!strcmp(live, "WiFi")) set_txt(U.set_val[ROW_LINK], T("WiFi · 已连接"));
        else set_txt(U.set_val[ROW_LINK], g_cfg.link_mode ? T("WiFi · 未连接") : T("USB · 未连接"));
    }
    chrome_prompt();  // the prompt carries the handle
    chrome_status();
    link_panel_refresh();
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

// centre of an object in screen coordinates (taps follow the layout)
static void obj_center(lv_obj_t *o, int *x, int *y) {
    lv_obj_update_layout(lv_scr_act());
    lv_area_t a;
    lv_obj_get_coords(o, &a);
    *x = (a.x1 + a.x2) / 2;
    *y = (a.y1 + a.y2) / 2;
}

static void tap_obj(lv_obj_t *o) {
    int x, y;
    obj_center(o, &x, &y);
    display_test_tap(x, y);
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

    tap_obj(U.set_row[0]);  // 显示名称
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

    tap_obj(U.set_row[1]);  // 用量显示
    tap_obj(U.set_row[2]);  // 刷新间隔
    for (int i = 0; i < 6; i++) { lv_timer_handler(); delay(20); }
    cases++;
    pass = g_cfg.quota_mode != q0 && g_cfg.refresh_s != r0;
    if (!pass) failed++;
    Serial.printf("[settest] %s quota=%u refresh=%u\n", pass ? "PASS" : "FAIL",
                  (unsigned)g_cfg.quota_mode, (unsigned)g_cfg.refresh_s);

    cases++;
    pass = U.tb_prompt && strstr(lv_label_get_text(U.tb_prompt), g_cfg.handle) != nullptr;
    if (!pass) failed++;
    Serial.printf("[settest] %s header=%s\n", pass ? "PASS" : "FAIL",
                  U.tb_prompt ? lv_label_get_text(U.tb_prompt) : "-");

    // the link row must open the connection panel, not just fire a refresh
    tap_obj(U.set_row[ROW_LINK]);  // 连接 magpie
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

// Every page is built once, with its final text: LVGL labels are cheap to update
// but the layout (titles cut into pane borders, fixed columns) depends on the
// wording, so a language change rebuilds the tree instead of patching strings.
static void build_pages() {
    build_home();
    build_state(0, PG_STATE_THINK, false);
    build_state(1, PG_STATE_CODE, false);
    build_state(2, PG_STATE_CONFIRM, true);
    build_agents();
    build_quota();
    build_usage();
    build_routes(PG_ROUTES);
    build_routes(PG_ROUTES_LIST);
    build_chrome();  // after the pages: drawn on top of them
    build_settings();
}

void ui_init() {
    assets_init();
    lv_disp_t *d = lv_disp_get_default();
    lv_theme_t *th = lv_theme_default_init(d, lv_palette_main(LV_PALETTE_GREEN),
                                           lv_palette_main(LV_PALETTE_RED), true, &ark12);
    lv_disp_set_theme(d, th);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(C_BG), 0);

    build_pages();

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

// Show/hide the pages and paint once. Only the page band (480x226) and the two
// chrome strips that change are repainted, never the whole panel.
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
    chrome_prompt();
    chrome_tabs();
    if (s_ui_ready && prev != s_page) {
        const uint32_t t0 = millis();
        lv_refr_now(NULL);
        if (dbg_verbose) Serial.printf("[ui] page %d painted %u ms\n", s_page, (unsigned)(millis() - t0));
    }
    s_last_page_ms = millis();
    if (dbg_verbose) Serial.printf("[ui] page %d at %u\n", s_page, (unsigned)s_last_page_ms);
}

// Tap path. A lone tap paints at once (feels immediate); taps that arrive within
// PAGE_COALESCE_MS of the previous paint only record the newest target, so four
// quick tab taps paint the fourth page once instead of painting four pages
// one after another and falling further behind the finger with every tap.
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

// Programmatic switches (serial 0..9, init, debug) stay synchronous so a
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

// ---- interface language ------------------------------------------------
// Switching rebuilds the object tree. It must not run inside a tap handler (the
// row that was tapped is about to be deleted), so a tap only raises a flag and
// loop() calls ui_rebuild() right after it persists the new preference.
static bool s_lang_dirty = false;

void ui_set_language(uint8_t lang) {
    if (lang_get() == lang) return;
    lang_set(lang);
    g_cfg.lang = lang;
    s_lang_dirty = true;
    s_cfg_dirty = true;  // NVS write happens in the main loop, like every other row
}

bool ui_lang_take_dirty() {
    const bool d = s_lang_dirty;
    s_lang_dirty = false;
    return d;
}

void ui_rebuild() {
    const uint32_t t0 = millis();
    close_overlays();          // keyboard + connection panel live on the top layer
    ui_show_calibration(false);  // these three sit directly on the screen, and
    if (s_test) ui_test_toggle();  // lv_obj_clean() would free them behind our back
    qc_close();
    // Animations keep raw object pointers (the breathing loop and the delayed
    // bubble hide), so stop them before their objects go away.
    lv_anim_del(s_av_home.box, NULL);
    lv_anim_del(s_av_home.bubble, NULL);
    for (int i = 0; i < 3; i++) {
        lv_anim_del(s_av_state[i].box, NULL);
        lv_anim_del(s_av_state[i].bubble, NULL);
    }
    lv_anim_del(s_av_sheet.box, NULL);
    lv_anim_del(s_av_sheet.bubble, NULL);

    lv_obj_clean(lv_scr_act());
    memset(&U, 0, sizeof(U));
    memset(&s_av_home, 0, sizeof(s_av_home));
    memset(s_av_state, 0, sizeof(s_av_state));
    memset(&s_av_sheet, 0, sizeof(s_av_sheet));
    for (int i = 0; i < 4; i++) u_filter[i] = nullptr;

    build_pages();
    if (s_data) ui_update(*s_data);  // repaint with the data already on hand
    apply_page(s_page);
    Serial.printf("[ui] rebuilt for %s: %u ms\n", lang_get() == LANG_EN ? "en" : "zh",
                  (unsigned)(millis() - t0));
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
    chrome_status();
    // 1 Hz blink: the prompt cursor, the state page cursor, and the live dots
    static bool blink = false;
    blink = !blink;
    lv_obj_set_style_bg_opa(U.tb_cursor, blink ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    if (s_page >= PG_STATE_THINK && s_page <= PG_STATE_CONFIRM) {
        lv_obj_t *c = U.st_cursor[s_page - PG_STATE_THINK];
        lv_obj_set_style_bg_opa(c, blink ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }
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
        snprintf(buf, n, T("刚刚"));
    else if (s < 3600)
        snprintf(buf, n, T("%ld 分钟前"), s / 60);
    else if (s < 86400)
        snprintf(buf, n, T("%ld 小时前"), s / 3600);
    else
        snprintf(buf, n, T("%ld 天前"), s / 86400);
    return buf;
}

// at work: its latest call is under two minutes old
static bool agent_live(const AgentRow *a) {
    const time_t now = time(nullptr);
    return a && a->last && now > 1700000000 && now - (time_t)a->last < 120;
}

static void fmt_cost(lv_obj_t *l, float cost) {
    if (g_cfg.privacy)
        set_txt(l, "$••");
    else if (cost < 10)
        set_txtf(l, "$%.2f", cost);
    else
        set_txtf(l, "$%.0f", cost);
}

static void sheet_fill() {
    if (!U.sh || !U.sh_id[0]) return;
    const char *id = U.sh_id;
    const AgentSkin *sk = skin_for(id);
    avatar_set(s_av_sheet, sk);
    const AgentRow *a = s_data ? agent_stat(*s_data, id) : nullptr;
    const RouteRow *r = s_data ? focus_row(*s_data, id) : nullptr;
    char b[32];
    set_txt(U.sh_name, agent_label(id, a ? a->name : ""));
    const char *model = a && a->model[0] ? a->model : (r ? r->model : "");
    set_txtf(U.sh_model, "%s · %s", id, model[0] ? model : T("默认模型"));
    set_txt(U.sh_num[0], a ? fmt_count(a->calls, b, sizeof(b)) : "0");
    set_txt(U.sh_num[1], a ? fmt_count(a->today, b, sizeof(b)) : "0");
    set_txt(U.sh_num[2], a ? fmt_count((uint64_t)a->tokens, b, sizeof(b)) : "0");
    fmt_cost(U.sh_num[3], a ? a->cost : 0.0f);
    uint32_t total = 0;
    for (int i = 0; s_data && i < s_data->nagents; i++) total += s_data->agents[i].calls;
    const float share = total && a ? 100.0f * a->calls / total : 0;
    set_txtf(U.sh_share, T("占全部调用 %.0f%%"), share);
    tbar_set(U.sh_bar, share, sk->accent);
    const bool live = agent_live(a);
    avatar_mode(s_av_sheet, live ? AV_THINK : AV_IDLE);
    if (live)
        set_txt(U.sh_last, T("● 工作中"));
    else if (a && a->last)
        set_txtf(U.sh_last, T("最近 %s"), fmt_ago(a->last, b, sizeof(b)));
    else
        set_txt(U.sh_last, a && a->calls ? "" : T("还没用过"));
    set_color(U.sh_last, live ? C_GREEN : C_MUTED);
    const bool following = s_focus[0] && !strcmp(s_focus, id);
    set_txt(lv_obj_get_child(U.sh_follow, 0), following ? T("取消跟随") : T("跟随"));
    set_bg(U.sh_follow, following ? C_DIM : C_CYAN);
    lv_obj_set_style_border_color(U.sh_follow, lv_color_hex(following ? C_LINE : C_CYAN), 0);
    set_color(lv_obj_get_child(U.sh_follow, 0), following ? C_TEXT : C_BG);
}

static void tag_for(const RouteRow &r, char *buf, size_t n, uint32_t *col) {
    if (r.status && r.status != 200) {
        snprintf(buf, n, T("错误 %u"), (unsigned)r.status);
        *col = C_RED;
    } else if (!strcmp(r.stop, "tool_calls") || !strcmp(r.stop, "tool_use")) {
        snprintf(buf, n, T("工具调用"));
        *col = C_AMBER;
    } else {
        snprintf(buf, n, T("回复"));
        *col = C_GREEN;
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
    char nb[24], nb2[24], lb[48];

    const char *fid = focus_id(d);
    const AgentSkin *sk = skin_for(fid);
    const RouteRow *frow = focus_row(d, fid);
    const char *fname = focus_name(d, fid, frow);
    const QuotaProvider *fq = focus_quota(d, frow);
    const QuotaWindow *w0 = fq && fq->nwin > 0 ? &fq->win[0] : nullptr;
    const QuotaWindow *w1 = fq && fq->nwin > 1 ? &fq->win[1] : nullptr;
    const QuotaWindow *wr = w1 ? w1 : w0;  // the longer window decides the reset line
    char reset[40] = "";
    if (wr) {
        fmt_countdown((time_t)wr->reset_at, reset, sizeof(reset));
        if (!reset[0]) reset_text(*wr, reset, sizeof(reset));
    }

    // a new agent took over: everyone hops and it says hello
    static char last_fid[16] = "";
    const bool switched = last_fid[0] && strcmp(last_fid, fid);
    strncpy(last_fid, fid, sizeof(last_fid) - 1);
    const AgentRow *fst = agent_stat(d, fid);
    const bool live = agent_live(fst);
    const bool tool = frow && (!strcmp(frow->stop, "tool_calls") || !strcmp(frow->stop, "tool_use"));

    // -------------------------------------------------- chrome
    chrome_status();

    // -------------------------------------------------- home
    set_bg(U.hm_dot, !d.gw ? C_FAINT : (live ? C_GREEN : C_MUTED));
    avatar_set(s_av_home, sk);
    avatar_mode(s_av_home, live ? AV_THINK : AV_IDLE);
    static bool prev_live = false;
    if (prev_live && !live && d.gw) {  // work just finished
        avatar_hop(s_av_home);
        avatar_say(s_av_home, T("做完了"), 1800);
    }
    prev_live = live;
    s_live_state = live;
    if (switched) {
        avatar_hop(s_av_home);
        avatar_say(s_av_home, say_for(sk, 0), 2500);
    }
    set_txt(U.hm_name, fname);
    {
        const char *model = fst && fst->model[0] ? fst->model : (frow ? frow->model : "");
        set_txtf(U.hm_sub, "%s%s%s", fid, model[0] ? " · " : "", model);
    }
    if (!d.gw)
        set_txt(U.hm_state, T("等待网关"));
    else if (live)
        set_txt(U.hm_state, tool ? T("运行命令") : T("工作中"));
    else if (fst && fst->last)
        set_txtf(U.hm_state, T("最近 %s"), fmt_ago(fst->last, nb2, sizeof(nb2)));
    else if (frow)
        set_txtf(U.hm_state, T("最近 %s"), frow->time);
    else
        set_txt(U.hm_state, T("空闲"));
    set_color(U.hm_state, live ? C_GREEN : C_MUTED);
    const QuotaWindow *hw[2] = {w0, w1};
    for (int k = 0; k < 2; k++) {
        show(U.hm_qk[k], hw[k] || k == 0);
        show(U.hm_qbar[k], hw[k] || k == 0);
        show(U.hm_qpct[k], hw[k] || k == 0);
        if (!hw[k]) {
            set_txt(U.hm_qpct[k], "--");
            tbar_set(U.hm_qbar[k], 0, C_GREEN);
            continue;
        }
        win_short(hw[k]->name, lb, sizeof(lb), false);  // no room for "Gemini" before the bar
        set_txt(U.hm_qk[k], lb);
        set_txtf(U.hm_qpct[k], "%.0f%%", q_show(hw[k]));
        set_color(U.hm_qpct[k], remain_col(hw[k]->remaining));
        tbar_set(U.hm_qbar[k], q_show(hw[k]), remain_col(hw[k]->remaining));
    }
    set_txtf(U.hm_today, T("%s 次"), fst ? fmt_count(fst->today, nb, sizeof(nb)) : "0");
    set_txtf(U.hm_30d, T("%s 次"), fst ? fmt_count(fst->calls, nb, sizeof(nb)) : "0");
    set_txt(U.hm_reset, reset);

    set_txt(U.mg_state, d.gw ? "● online" : "● offline");
    set_color(U.mg_state, d.gw ? C_GREEN : C_FAINT);
    set_txtf(U.mg_calls, "%u", (unsigned)d.today_calls);
    set_txt(U.mg_tok, fmt_count((uint64_t)d.today_tokens, nb, sizeof(nb)));
    set_txtf(U.mg_costk, T("费用 %s"), d.period);
    fmt_cost(U.mg_cost, d.cost);
    set_txtf(U.mg_sess, "%d", d.sessions);
    set_txt(U.mg_last, d.nrows ? d.rows[0].time : "--");
    set_txtf(U.mg_cap, "token / %s", d.period);
    {
        const int slot = bars_fill(U.mg_bars, d, 10, 210, 200, 50);
        float mx = 0;
        for (int i = 0; i < d.nseries; i++)
            if (d.series[i].tokens > mx) mx = d.series[i].tokens;
        set_txtf(U.mg_max, "%s", mx > 0 ? fmt_count((uint64_t)mx, nb, sizeof(nb)) : "");
        show(U.mg_empty, slot == 0);
    }

    // -------------------------------------------------- state pages
    for (int i = 0; i < 3; i++) {
        avatar_set(s_av_state[i], sk);
        if (switched) avatar_hop(s_av_state[i]);
        for (int k = 0; k < 3; k++) set_bg(U.st_pip[i][k], k == i ? sk->accent : C_DIM);
        set_txt(U.st_name[i], fname);
        set_txt(U.st_title_l[i], fid);
        set_txtf(U.st_sess[i], T("%d 个会话"), d.sessions);
        if (frow) {
            if (i == 0)
                set_txtf(U.st_sub[i], T("已用时 %s"), fmt_secs(frow->ms, nb2, sizeof(nb2)));
            else if (i == 1)
                set_txt(U.st_sub[i], frow->model);
            else
                set_txtf(U.st_sub[i], T("运行命令 : %s"), frow->model);
        } else {
            set_txt(U.st_sub[i], T("暂无调用"));
        }

        if (w0) {
            win_long(w0->name, lb, sizeof(lb));
            set_txt(U.st_l5[i], lb);
            set_txtf(U.st_p5[i], "%.0f%%", q_show(w0));
            set_color(U.st_p5[i], remain_col(w0->remaining));
            tbar_set(U.st_bar5[i], q_show(w0), remain_col(w0->remaining));
        } else {
            set_txt(U.st_p5[i], "--");
            tbar_set(U.st_bar5[i], 0, C_GREEN);
        }
        show(U.st_l7[i], w1 != nullptr);
        show(U.st_p7[i], w1 != nullptr);
        show(U.st_bar7[i], w1 != nullptr);
        if (w1) {
            win_long(w1->name, lb, sizeof(lb));
            set_txt(U.st_l7[i], lb);
            set_txtf(U.st_p7[i], "%.0f%%", q_show(w1));
            set_color(U.st_p7[i], remain_col(w1->remaining));
            tbar_set(U.st_bar7[i], q_show(w1), remain_col(w1->remaining));
        }
        set_txt(U.st_reset[i], reset[0] ? reset : "");
        if (fq)
            set_txtf(U.st_src[i], T("额度来源 %s · %s"), fq->name, fq->plan);
        else
            set_txt(U.st_src[i], "");
        set_txt(U.st_conn[i], d.gw ? T("已连接") : T("未连接"));
        set_color(U.st_conn[i], d.gw ? C_GREEN : C_MUTED);
        set_bg(U.st_conn_dot[i], d.gw ? C_GREEN : C_FAINT);
    }

    // -------------------------------------------------- agents
    static AgentView views[AG_ROWS];
    const int nv = agent_views(d, views, AG_ROWS);
    set_txtf(U.ag_count, T("%d 个"), nv);
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
            set_txt(U.ag_state[i], T("工作中"));
            set_color(U.ag_state[i], C_AMBER);
        } else if (ast && ast->today) {
            set_txt(U.ag_state[i], T("已完成"));
            set_color(U.ag_state[i], C_GREEN);
        } else if (ast && (ast->last || ast->calls)) {
            set_txt(U.ag_state[i], T("空闲"));
            set_color(U.ag_state[i], C_MUTED);
        } else {
            set_txt(U.ag_state[i], T("未使用"));
            set_color(U.ag_state[i], C_FAINT);
        }
        set_txt(U.ag_name[i], agent_label(v.id, v.name));
        set_color(U.ag_name[i], v.calls ? C_TEXT : C_MUTED);
        set_txt(U.ag_detail[i], v.model[0] ? v.model : T("默认模型"));
        if (v.calls) {
            set_txt(U.ag_calls[i], fmt_count(v.calls, nb, sizeof(nb)));
            set_color(U.ag_calls[i], C_GREEN);
        } else {
            set_txt(U.ag_calls[i], "-");
            set_color(U.ag_calls[i], C_FAINT);
        }
        const bool pinned = s_focus[0] && !strcmp(s_focus, v.id);
        set_bg(U.ag_row[i], C_DIM);
        lv_obj_set_style_bg_opa(U.ag_row[i], pinned ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }

    sheet_fill();

    // -------------------------------------------------- quota list
    show(U.q_empty, d.nprov == 0);
    for (int i = 0; i < QUOTA_MAX_PROV; i++) {
        const QuotaProvider *q = i < d.nprov ? &d.prov[i] : nullptr;
        show(U.q_blk[i], q != nullptr);
        if (!q) continue;
        const int lines = q->nwin ? q->nwin : 1;
        lv_obj_set_height(U.q_pane[i], 12 + lines * QL_STEP);
        lv_obj_set_height(U.q_blk[i], 6 + 12 + lines * QL_STEP);
        snprintf(lb, sizeof(lb), "%s · %s", q->name, q->plan);
        set_txt(U.q_name[i], q->plan[0] ? lb : q->name);
        char acct[48];
        mask_account(q->user, acct, sizeof(acct));
        set_txt(U.q_user[i], acct);
        show(U.q_user[i], acct[0] != 0);  // an empty title would still cut the border
        for (int j = 0; j < QUOTA_MAX_WIN; j++) {
            const bool has = j < q->nwin;
            show(U.q_key[i][j], has || (j == 0 && !q->nwin));
            show(U.q_bar[i][j], has);
            show(U.q_pct[i][j], has);
            show(U.q_rst[i][j], has);
            if (!has) {
                if (j == 0 && !q->nwin) {
                    set_txtf(U.q_key[i][j], T("最近 %s"), q->lastServed[0] ? q->lastServed : "--");
                    lv_obj_set_width(U.q_key[i][j], 440);
                }
                continue;
            }
            lv_obj_set_width(U.q_key[i][j], 96);
            const float rem = q->win[j].remaining;
            const float shown = q_show(&q->win[j]);
            win_short(q->win[j].name, lb, sizeof(lb));
            set_txt(U.q_key[i][j], lb);
            tbar_set(U.q_bar[i][j], shown, remain_col(rem));
            set_txtf(U.q_pct[i][j], "%.0f%%", shown);
            set_color(U.q_pct[i][j], remain_col(rem));
            char rb[20];
            reset_text(q->win[j], rb, sizeof(rb));
            set_txt(U.q_rst[i][j], rb);
        }
    }

    // -------------------------------------------------- usage
    char b1[24], b2[24], b3[24];
    set_txt(U.u_tok, fmt_count(d.tok_in + d.tok_out + d.tok_cache, b1, sizeof(b1)));
    set_txtf(U.u_in, "in    %s", fmt_count(d.tok_in, b1, sizeof(b1)));
    set_txtf(U.u_out, "out   %s", fmt_count(d.tok_out, b2, sizeof(b2)));
    set_txtf(U.u_cache, "cache %s", fmt_count(d.tok_cache, b3, sizeof(b3)));
    set_txtf(U.u_calls, "%u", (unsigned)d.calls);
    set_txtf(U.u_err, T("%u 次失败"), (unsigned)d.errors);
    set_color(U.u_err, d.errors ? C_RED : C_MUTED);
    // the cost pane's subtitle: what a call costs on average for this window
    if (g_cfg.privacy || d.calls == 0)
        set_txt(U.u_price, "");
    else
        set_txtf(U.u_price, T("均价 $%.4f"), d.cost / (float)d.calls);
    fmt_cost(U.u_cost, d.cost);
    set_txtf(U.u_hit, "%.0f%%", d.hit_rate());
    {
        const int n = d.nseries;
        const int slot = bars_fill(U.u_bars, d, CH_X0, CH_W, CH_BASE, CH_H);
        float mx = 0;
        for (int i = 0; i < n; i++)
            if (d.series[i].tokens > mx) mx = d.series[i].tokens;
        show(U.u_empty, slot == 0);
        set_txt(U.u_ymax, mx > 0 ? fmt_count((uint64_t)mx, b1, sizeof(b1)) : "");
        const int bw = slot > 4 ? slot - 2 : (slot > 1 ? slot - 1 : 1);
        for (int k = 0; k < 5; k++) {
            show(U.u_lbl[k], n > 0 && slot > 0);
            if (!n || !slot) continue;
            const int src = n == 1 ? 0 : k * (n - 1) / 4;
            chart_label(d.series[src].label, lb, sizeof(lb));
            set_txt(U.u_lbl[k], lb);
            int cx = CH_X0 + src * slot + bw / 2 - 20;
            if (cx > LCD_W - 52) cx = LCD_W - 52;
            lv_obj_set_x(U.u_lbl[k], cx);
        }
    }

    // -------------------------------------------------- routes
    for (int pg_i = 0; pg_i < 2; pg_i++) {
        show(U.r_empty[pg_i], d.nrows == 0);
        set_txtf(U.r_count[pg_i], T("%d 条"), d.nrows);
        for (int i = 0; i < RT_ROWS; i++) {
            const bool has = i < d.nrows;
            show(U.r_row[pg_i][i], has);
            if (!has) continue;
            const RouteRow &r = d.rows[i];
            logo_set(U.r_icon[pg_i][i], skin_for(r.agent)->s);
            set_txt(U.r_agent[pg_i][i], agent_label(r.agent, r.agentName));
            set_txt(U.r_model[pg_i][i], r.model[0] ? r.model : "--");
            uint32_t tc;
            tag_for(r, lb, sizeof(lb), &tc);
            set_txt(U.r_tag[pg_i][i], lb);
            set_color(U.r_tag[pg_i][i], tc);
            set_txt(U.r_time[pg_i][i], r.time);
            // same Chinese word as the home pane's "sessions", but singular here
            const char *prov = !strncmp(r.provider, "session-", 8) ? L("会话", "session") : r.provider;
            if (r.account[0])
                set_txtf(U.r_flow[pg_i][i], "→ %s · %s", prov, r.account);
            else
                set_txtf(U.r_flow[pg_i][i], "→ %s", prov);
            set_txtf(U.r_use[pg_i][i], "%s tok · %s", fmt_k(r.tokens(), b1, sizeof(b1)),
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
        set_bg(U.r_top_dot, r0.ok() ? C_GREEN : C_RED);
    } else {
        set_txt(U.r_top_agent, "--");
        set_txt(U.r_top_to, "--");
        set_bg(U.r_top_dot, C_FAINT);
    }
    for (int k = 0; k < 2; k++)
        set_txtf(U.r_sum[k], T("今天 %u 次调用 · %s token"), (unsigned)d.today_calls,
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
    lv_obj_set_style_bg_color(s_qc, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(s_qc, LV_OPA_COVER, 0);

    s_qc_box = lv_obj_create(s_qc);
    lv_obj_remove_style_all(s_qc_box);
    lv_obj_set_style_bg_color(s_qc_box, lv_color_hex(C_GREEN), 0);
    lv_obj_set_style_bg_opa(s_qc_box, LV_OPA_70, 0);
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

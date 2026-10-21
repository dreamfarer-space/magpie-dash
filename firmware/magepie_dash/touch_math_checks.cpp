// These execute as static_asserts in the REAL Arduino build, with no RAM cost.
#include "board.h"
#include "touch_math.h"

static_assert(touch_axis(251.0f, -1.0f, LCD_H - 1.0f) == 20.0f, "measured Y inversion");
static_assert(touch_axis(0.0f, -1.0f, 271.0f) == 271.0f, "bottom Y");
static_assert(touch_axis(271.0f, -1.0f, 271.0f) == 0.0f, "top Y");
static_assert(touch_axis(449.0f, 1.0f, 0.0f) == 449.0f, "do not invent X offset");
static_assert(touch_round(-0.6f) == -1 && touch_round(0.6f) == 1, "signed rounding");
static_assert(touch_clip(-2, 480) == 0 && touch_clip(482, 480) == 479, "edge clamp");
static_assert(touch_in_margin(-12, 480, 12) && !touch_in_margin(-13, 480, 12), "left margin");
static_assert(touch_in_margin(491, 480, 12) && !touch_in_margin(492, 480, 12), "right margin");
static_assert(touch_next_pressed(true, TouchFrame::NoData, false), "empty poll must preserve press");
static_assert(touch_next_pressed(true, TouchFrame::Error, false), "short I2C fault must preserve press");
static_assert(!touch_next_pressed(true, TouchFrame::Released, false), "explicit release");
static_assert(!touch_next_pressed(true, TouchFrame::NoData, true), "stale timeout release");
static_assert(!touch_next_pressed(false, TouchFrame::NoData, false), "idle stays idle");
static_assert(touch_next_pressed(false, TouchFrame::Pressed, true), "fresh sample wins over old timeout");
static_assert(!touch_next_pressed(true, TouchFrame::Cancelled, false), "multitouch cancels");

constexpr int16_t sx[4] = {14, 466, 14, 466};
constexpr int16_t sy[4] = {14, 14, 258, 258};
// Synthetic regression fixtures ONLY, not measurements of this board.
constexpr float rx[4] = {59, 511, 59, 511};
constexpr float ry[4] = {257, 257, 13, 13};
constexpr float flat[4] = {20, 20, 20, 20};
constexpr float noisy[4] = {58, 512, 60, 510};
constexpr auto xf = touch_fit_axis(rx, sx);
constexpr auto yf = touch_fit_axis(ry, sy);
constexpr auto nf = touch_fit_axis(noisy, sx);
static_assert(xf.valid && xf.a == 1.0f && xf.b == -45.0f, "offset fixture fit");
static_assert(yf.valid && yf.a == -1.0f && yf.b == 271.0f, "mirrored fit");
static_assert(!touch_fit_axis(flat, sx).valid, "reject zero span");
static_assert(touch_abs(touch_axis(285, xf.a, xf.b) - 240) < 0.01f, "independent center");
static_assert(nf.valid && touch_abs(touch_axis(285, nf.a, nf.b) - 240) < 0.01f, "noisy LS fit");
constexpr float scaled[4] = {28, 932, 28, 932};
constexpr auto sf = touch_fit_axis(scaled, sx);
static_assert(sf.valid && sf.a == 0.5f && sf.b == 0.0f, "gain fixture fit");

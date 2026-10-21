// Pure coordinate math shared by the firmware and compile-time regression tests.
#pragma once
#include <stdint.h>

struct TouchTransform {
    float ax, bx, ay, by;
};

constexpr float touch_abs(float v) { return v < 0.0f ? -v : v; }
constexpr float touch_axis(float raw, float a, float b) { return a * raw + b; }
constexpr int32_t touch_round(float v) {
    return static_cast<int32_t>(v >= 0.0f ? v + 0.5f : v - 0.5f);
}
constexpr int16_t touch_clip(int32_t v, int16_t size) {
    return v < 0 ? 0 : (v >= size ? size - 1 : static_cast<int16_t>(v));
}
constexpr bool touch_in_margin(int32_t v, int16_t size, int16_t margin) {
    return v >= -margin && v < size + margin;
}

// Four-point least squares for ONE axis. No XY swap/cross-term is hidden here.
struct TouchAxisFit { bool valid; float a, b; };
constexpr TouchAxisFit touch_fit_axis(const float *raw, const int16_t *screen) {
    float rm = 0.0f, sm = 0.0f;
    for (int i = 0; i < 4; ++i) { rm += raw[i] * 0.25f; sm += screen[i] * 0.25f; }
    float variance = 0.0f, covariance = 0.0f;
    for (int i = 0; i < 4; ++i) {
        variance += (raw[i] - rm) * (raw[i] - rm);
        covariance += (raw[i] - rm) * (screen[i] - sm);
    }
    if (variance < 1.0f) return {false, 0.0f, 0.0f};
    const float a = covariance / variance;
    return {true, a, sm - a * rm};
}

// GT911 ready=0 means NO NEW FRAME, not finger-up. Keep state until a real
// zero-contact frame or a bounded stale timeout. Errors follow the same rule.
enum class TouchFrame { NoData, Pressed, Released, Error, Cancelled };
constexpr bool touch_next_pressed(bool previous, TouchFrame frame, bool stale) {
    return frame == TouchFrame::Pressed ? true :
           (frame == TouchFrame::Released || frame == TouchFrame::Cancelled || stale)
               ? false : previous;
}

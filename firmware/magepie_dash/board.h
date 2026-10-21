// JC4827W543 (Guition 4.3" 480x272, ESP32-S3-WROOM-1 N4R8)
// Panel: NV3041A over QSPI, GT911 capacitive touch.
#pragma once

#define LCD_W 480
#define LCD_H 272

// NV3041A RAM window offsets, NOT LVGL layout offsets. Leave at zero unless
// the physical CAL frame is clipped/shifted. This Arduino_GFX constructor
// accepts unsigned offsets only (0..255); never guess a -45 px UI shift.
#define LCD_COL_OFFSET 0
#define LCD_ROW_OFFSET 0

// Independent affine maps from ORIGINAL GT911 coordinates to LVGL pixels:
// screen_x = TOUCH_X_A * raw_x + TOUCH_X_B (same for Y).
// Measured with the 'T' colour/touch test pattern: raw GT911 coordinates
// already share the panel's frame, so both axes are 1:1 (no mirroring).
// A 180-degree mapping (ax=-1, ay=-1) made taps land diagonally opposite.
#define TOUCH_X_A 1.0f
#define TOUCH_X_B 0.0f
#define TOUCH_Y_A 1.0f
#define TOUCH_Y_B 0.0f

// Field calibration (921600 baud): first send 'c' and check ALL four white
// borders touch the physical edges, with no clipping/wrap. If not, fix the
// DISPLAY window first. Then hold each indicated cross >=120 ms and lift:
// TL(14,14), TR(466,14), BL(14,258), BR(466,258), CENTER(240,136).
// CAL fits the first four points and validates the independent center point;
// on success it applies the fit in RAM and prints these four #defines. Copy
// them here and rebuild to retain it after reboot. 'x' closes the overlay.
// Manual two-point formula per axis: a=(s2-s1)/(r2-r1), b=s1-a*r1;
// use ORIGINAL raw Y, not an already mirrored value (no second Y flip).
#define TOUCH_WARMUP_MS 800U
#define TOUCH_STALE_MS 80U
#define TOUCH_EDGE_MARGIN_PX 12
#define TOUCH_CAL_MIN_HOLD_MS 120U
#define TOUCH_CAL_MAX_SPREAD 20
#define TOUCH_CAL_MAX_ERROR_PX 12.0f

// QSPI LCD
#define LCD_CS 45
#define LCD_SCK 47
#define LCD_D0 21
#define LCD_D1 48
#define LCD_D2 40
#define LCD_D3 39
#define LCD_BL 1

// GT911 touch (I2C)
#define TOUCH_SCL 4
#define TOUCH_SDA 8
#define TOUCH_INT 3
#define TOUCH_RES 38

// physical buttons
#define BTN_BOOT 0

// microSD (unused, kept for reference)
#define TF_CS 10
#define TF_MISO 11
#define TF_CLK 12
#define TF_MOSI 13

// audio codec (unused)
#define SPK_LRCLK 2
#define SPK_BCLK 42
#define SPK_DIN 41

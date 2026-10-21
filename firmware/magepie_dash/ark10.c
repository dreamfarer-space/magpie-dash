/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --font D:\esp32\tools\fonts\ark-pixel-10px-monospaced-zh_hans.ttf --size 10 --bpp 1 --format lvgl --lv-include lvgl.h --no-compress --force-fast-kern-format -r 0x20-0x7e -o D:\esp32\magepie-esp32\firmware\magepie_dash\ark10.c -r 0xab -r 0xb7 -r 0xbb -r 0x2014 -r 0x2022 -r 0x2026 -r 0x2192 -r 0x21bb -r 0x2500 -r 0x250c -r 0x2510 -r 0x25a0 -r 0x25b6 -r 0x25cf -r 0x2605 -r 0x4e00 -r 0x4e01 -r 0x4e07 -r 0x4e09 -r 0x4e0a -r 0x4e0b -r 0x4e0d -r 0x4e2a -r 0x4e2d -r 0x4e3b -r 0x4e86 -r 0x4e8c -r 0x4ea4 -r 0x4ef6 -r 0x4f1a -r 0x4f4d -r 0x4f4f -r 0x4f59 -r 0x4f60 -r 0x4fdd -r 0x50cf -r 0x513f -r 0x5165 -r 0x5168 -r 0x5173 -r 0x5176 -r 0x5177 -r 0x5185 -r 0x5199 -r 0x51fa -r 0x51fb -r 0x521a -r 0x5230 -r 0x5237 -r 0x529b -r 0x53e3 -r 0x53ea -r 0x540d -r 0x540e -r 0x547d -r 0x55b5 -r 0x5668 -r 0x56db -r 0x56de -r 0x5728 -r 0x5730 -r 0x578b -r 0x5929 -r 0x5934 -r 0x5b87 -r 0x5b8c -r 0x5b99 -r 0x5b9a -r 0x5c0f -r 0x5c31 -r 0x5c40 -r 0x5de5 -r 0x5df2 -r 0x5e74 -r 0x5ea6 -r 0x5f00 -r 0x5f0f -r 0x606f -r 0x6210 -r 0x6211 -r 0x624b -r 0x652f -r 0x6587 -r 0x65e0 -r 0x65e5 -r 0x662f -r 0x663e -r 0x6700 -r 0x6708 -r 0x672a -r 0x672c -r 0x6765 -r 0x683c -r 0x6848 -r 0x684c -r 0x6b63 -r 0x6d77 -r 0x6e90 -r 0x70b9 -r 0x751f -r 0x7528 -r 0x7531 -r 0x7535 -r 0x7684 -r 0x7801 -r 0x79f0 -r 0x7a7a -r 0x7b49 -r 0x7c73 -r 0x7f51 -r 0x7f6e -r 0x821f -r 0x884c -r 0x8981 -r 0x8a00 -r 0x8ba4 -r 0x8bed -r 0x8bf7 -r 0x8bfb -r 0x8fc7 -r 0x8fd9 -r 0x8fde -r 0x914d -r 0x91cd -r 0x91cf -r 0x91d1 -r 0x95ed -r 0x95f2 -r 0x95f4 -r 0x968f -r 0x9762 -r 0x9875 -r 0x9ed8 -r 0xff01 -r 0xff0c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef ARK10
#define ARK10 1
#endif

#if ARK10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfa,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x5f, 0x59, 0xaf, 0xa0,

    /* U+0024 "$" */
    0x27, 0xaa, 0x63, 0x3e, 0x20,

    /* U+0025 "%" */
    0x91, 0x22, 0x48, 0x90,

    /* U+0026 "&" */
    0x4a, 0xa4, 0xba, 0x50,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x29, 0x49, 0x12, 0x20,

    /* U+0029 ")" */
    0x89, 0x12, 0x52, 0x80,

    /* U+002A "*" */
    0xab, 0xaa,

    /* U+002B "+" */
    0x22, 0xf2, 0x20,

    /* U+002C "," */
    0x60,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x11, 0x22, 0x44, 0x48, 0x80,

    /* U+0030 "0" */
    0x69, 0xbd, 0x99, 0x60,

    /* U+0031 "1" */
    0x59, 0x24, 0xb8,

    /* U+0032 "2" */
    0x69, 0x12, 0x48, 0xf0,

    /* U+0033 "3" */
    0x69, 0x12, 0x19, 0x60,

    /* U+0034 "4" */
    0x35, 0x59, 0x9f, 0x10,

    /* U+0035 "5" */
    0xf8, 0x8e, 0x11, 0xe0,

    /* U+0036 "6" */
    0x69, 0x8e, 0x99, 0x60,

    /* U+0037 "7" */
    0xf1, 0x12, 0x24, 0x40,

    /* U+0038 "8" */
    0x69, 0x96, 0x99, 0x60,

    /* U+0039 "9" */
    0x69, 0x97, 0x19, 0x60,

    /* U+003A ":" */
    0x88,

    /* U+003B ";" */
    0x40, 0x60,

    /* U+003C "<" */
    0x2a, 0x22,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0x69, 0x12, 0x20, 0x20,

    /* U+0040 "@" */
    0x69, 0xbb, 0xb8, 0x60,

    /* U+0041 "A" */
    0x69, 0x99, 0xf9, 0x90,

    /* U+0042 "B" */
    0xe9, 0x9e, 0x99, 0xe0,

    /* U+0043 "C" */
    0x69, 0x88, 0x89, 0x60,

    /* U+0044 "D" */
    0xe9, 0x99, 0x99, 0xe0,

    /* U+0045 "E" */
    0xf8, 0x8e, 0x88, 0xf0,

    /* U+0046 "F" */
    0xf8, 0x8e, 0x88, 0x80,

    /* U+0047 "G" */
    0x69, 0x88, 0xb9, 0x70,

    /* U+0048 "H" */
    0x99, 0x9f, 0x99, 0x90,

    /* U+0049 "I" */
    0xf2, 0x22, 0x22, 0xf0,

    /* U+004A "J" */
    0x11, 0x11, 0x99, 0x60,

    /* U+004B "K" */
    0x99, 0xac, 0xa9, 0x90,

    /* U+004C "L" */
    0x88, 0x88, 0x88, 0xf0,

    /* U+004D "M" */
    0x9f, 0xf9, 0x99, 0x90,

    /* U+004E "N" */
    0x9d, 0xdb, 0xb9, 0x90,

    /* U+004F "O" */
    0x69, 0x99, 0x99, 0x60,

    /* U+0050 "P" */
    0xe9, 0x9e, 0x88, 0x80,

    /* U+0051 "Q" */
    0x69, 0x99, 0x9a, 0x50,

    /* U+0052 "R" */
    0xe9, 0x9e, 0xa9, 0x90,

    /* U+0053 "S" */
    0x69, 0x86, 0x19, 0x60,

    /* U+0054 "T" */
    0xf2, 0x22, 0x22, 0x20,

    /* U+0055 "U" */
    0x99, 0x99, 0x99, 0x60,

    /* U+0056 "V" */
    0x99, 0x95, 0x55, 0x20,

    /* U+0057 "W" */
    0x99, 0x99, 0xff, 0x90,

    /* U+0058 "X" */
    0x99, 0x96, 0x99, 0x90,

    /* U+0059 "Y" */
    0x99, 0x55, 0x22, 0x20,

    /* U+005A "Z" */
    0xf1, 0x24, 0x48, 0xf0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0xe0,

    /* U+005C "\\" */
    0x88, 0x44, 0x22, 0x21, 0x10,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0xe0,

    /* U+005E "^" */
    0x54,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x61, 0x79, 0x70,

    /* U+0062 "b" */
    0x88, 0xe9, 0x99, 0xe0,

    /* U+0063 "c" */
    0x69, 0x89, 0x60,

    /* U+0064 "d" */
    0x11, 0x79, 0x99, 0x70,

    /* U+0065 "e" */
    0x69, 0xf8, 0x60,

    /* U+0066 "f" */
    0x34, 0xf4, 0x44, 0x40,

    /* U+0067 "g" */
    0x79, 0x97, 0x16,

    /* U+0068 "h" */
    0x88, 0xe9, 0x99, 0x90,

    /* U+0069 "i" */
    0x20, 0xe2, 0x22, 0xf0,

    /* U+006A "j" */
    0x23, 0x92, 0x4e,

    /* U+006B "k" */
    0x88, 0x9a, 0xca, 0x90,

    /* U+006C "l" */
    0xc4, 0x44, 0x44, 0x30,

    /* U+006D "m" */
    0x9f, 0xf9, 0x90,

    /* U+006E "n" */
    0xe9, 0x99, 0x90,

    /* U+006F "o" */
    0x69, 0x99, 0x60,

    /* U+0070 "p" */
    0xe9, 0x9e, 0x88,

    /* U+0071 "q" */
    0x79, 0x97, 0x11,

    /* U+0072 "r" */
    0xbc, 0x88, 0x80,

    /* U+0073 "s" */
    0x78, 0x61, 0xe0,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x30,

    /* U+0075 "u" */
    0x99, 0x99, 0x70,

    /* U+0076 "v" */
    0x99, 0x55, 0x20,

    /* U+0077 "w" */
    0x99, 0xff, 0x90,

    /* U+0078 "x" */
    0x99, 0x69, 0x90,

    /* U+0079 "y" */
    0x99, 0x55, 0x2c,

    /* U+007A "z" */
    0xf2, 0x48, 0xf0,

    /* U+007B "{" */
    0x34, 0x44, 0x84, 0x44, 0x30,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0xc2, 0x22, 0x12, 0x22, 0xc0,

    /* U+007E "~" */
    0x5a,

    /* U+00AB "«" */
    0x5a, 0x50,

    /* U+00B7 "·" */
    0xf0,

    /* U+00BB "»" */
    0xa5, 0xa0,

    /* U+2014 "—" */
    0xff, 0x80,

    /* U+2022 "•" */
    0x6f, 0xf6,

    /* U+2026 "…" */
    0xdb, 0xdb,

    /* U+2192 "→" */
    0x2, 0x0, 0xbf, 0xe0, 0x20, 0x20,

    /* U+21BB "↻" */
    0x70, 0x61, 0x54, 0x18, 0x30, 0x51, 0x1c,

    /* U+2500 "─" */
    0xff, 0xc0,

    /* U+250C "┌" */
    0xfe, 0x8, 0x20, 0x80,

    /* U+2510 "┐" */
    0xf8, 0x42, 0x10, 0x80,

    /* U+25A0 "■" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x80,

    /* U+25B6 "▶" */
    0xc0, 0x78, 0x3f, 0x1f, 0xef, 0xff, 0xfb, 0xf1,
    0xe0, 0xc0, 0x0,

    /* U+25CF "●" */
    0x3e, 0x3f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xfe, 0x3e, 0x0,

    /* U+2605 "★" */
    0x8, 0x4, 0x7, 0x1f, 0xf3, 0xe0, 0xe0, 0xd8,
    0x44, 0x41, 0x0,

    /* U+4E00 "一" */
    0xff, 0x80,

    /* U+4E01 "丁" */
    0xff, 0x84, 0x2, 0x1, 0x0, 0x80, 0x40, 0x20,
    0x10, 0x38, 0x0,

    /* U+4E07 "万" */
    0xff, 0x88, 0x4, 0x3, 0xe1, 0x11, 0x8, 0x84,
    0x82, 0x86, 0x0,

    /* U+4E09 "三" */
    0x7f, 0x0, 0x0, 0x0, 0x3, 0xe0, 0x0, 0x0,
    0x0, 0xff, 0x80,

    /* U+4E0A "上" */
    0x8, 0x4, 0x2, 0x1, 0xe0, 0x80, 0x40, 0x20,
    0x10, 0xff, 0x80,

    /* U+4E0B "下" */
    0xff, 0x84, 0x2, 0x1, 0x80, 0xa0, 0x48, 0x20,
    0x10, 0x8, 0x0,

    /* U+4E0D "不" */
    0xff, 0x82, 0x2, 0x3, 0x42, 0x96, 0x44, 0x20,
    0x10, 0x8, 0x0,

    /* U+4E2A "个" */
    0x8, 0xa, 0x8, 0x99, 0x30, 0x80, 0x40, 0x20,
    0x10, 0x8, 0x0,

    /* U+4E2D "中" */
    0x8, 0x4, 0x3f, 0xf1, 0x18, 0x8f, 0xfc, 0x20,
    0x10, 0x8, 0x0,

    /* U+4E3B "主" */
    0x10, 0x4, 0x3f, 0xe1, 0x0, 0x83, 0xf8, 0x20,
    0x10, 0xff, 0x80,

    /* U+4E86 "了" */
    0xff, 0x1, 0x2, 0xc, 0x8, 0x8, 0x8, 0x8,
    0x38,

    /* U+4E8C "二" */
    0x7f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xfe,

    /* U+4EA4 "交" */
    0x8, 0x7f, 0xc8, 0x88, 0x2a, 0x28, 0xa0, 0x20,
    0x6c, 0xc1, 0x80,

    /* U+4EF6 "件" */
    0x2a, 0x15, 0x13, 0xfa, 0x44, 0x22, 0xfd, 0x8,
    0x84, 0x42, 0x0,

    /* U+4F1A "会" */
    0x8, 0xa, 0x8, 0x9b, 0xb0, 0x3, 0xf8, 0x40,
    0x44, 0x7d, 0x0,

    /* U+4F4D "位" */
    0x24, 0x11, 0x17, 0xf8, 0x4, 0x92, 0x49, 0x24,
    0x84, 0x5f, 0x80,

    /* U+4F4F "住" */
    0x24, 0x11, 0x17, 0xf8, 0x44, 0x22, 0x7d, 0x8,
    0x84, 0x5f, 0x80,

    /* U+4F59 "余" */
    0x8, 0x1b, 0x30, 0x67, 0xc0, 0x83, 0xf8, 0x20,
    0x92, 0x98, 0x80,

    /* U+4F60 "你" */
    0x28, 0x17, 0xd2, 0x3a, 0x44, 0x22, 0x59, 0x2a,
    0xa5, 0x46, 0x0,

    /* U+4FDD "保" */
    0x2f, 0x94, 0x52, 0x39, 0xf4, 0x22, 0xfd, 0x1c,
    0x95, 0x52, 0x0,

    /* U+50CF "像" */
    0x2f, 0x18, 0x97, 0xfa, 0x95, 0xfa, 0x55, 0x5c,
    0x95, 0x56, 0x80,

    /* U+513F "儿" */
    0x24, 0x12, 0x9, 0x4, 0x82, 0x41, 0x21, 0x12,
    0x89, 0x87, 0x80,

    /* U+5165 "入" */
    0x20, 0x8, 0x2, 0x1, 0x1, 0x40, 0xa0, 0x88,
    0x82, 0x80, 0x80,

    /* U+5168 "全" */
    0x1c, 0x11, 0x10, 0x57, 0xd0, 0x83, 0xf8, 0x20,
    0x10, 0xff, 0x80,

    /* U+5173 "关" */
    0x41, 0x11, 0x1f, 0xc1, 0x0, 0x87, 0xfc, 0x50,
    0x44, 0xc1, 0x80,

    /* U+5176 "其" */
    0x22, 0x7f, 0xc8, 0x87, 0xc2, 0x27, 0xfc, 0x0,
    0x44, 0xc1, 0x80,

    /* U+5177 "具" */
    0x7f, 0x20, 0x9f, 0xc8, 0x27, 0xf2, 0xb, 0xfe,
    0x44, 0xc1, 0x80,

    /* U+5185 "内" */
    0x8, 0x4, 0x3f, 0xf1, 0x19, 0x4d, 0x16, 0x3,
    0x1, 0x81, 0x80,

    /* U+5199 "写" */
    0xff, 0xd0, 0x4f, 0xc4, 0x3, 0xf0, 0xb, 0xf4,
    0x2, 0xe, 0x0,

    /* U+51FA "出" */
    0x8, 0x24, 0x92, 0x4f, 0xe0, 0x84, 0x46, 0x23,
    0x11, 0xff, 0x80,

    /* U+51FB "击" */
    0x8, 0x4, 0x1f, 0xc1, 0xf, 0xf8, 0x41, 0x24,
    0x92, 0x7f, 0x0,

    /* U+521A "刚" */
    0xf8, 0xc5, 0x62, 0xbb, 0x5a, 0xad, 0x57, 0x6b,
    0x11, 0x99, 0x80,

    /* U+5230 "到" */
    0xf8, 0x91, 0x52, 0xbf, 0x52, 0x2f, 0xd4, 0x8a,
    0x71, 0xc1, 0x80,

    /* U+5237 "刷" */
    0xf8, 0xc5, 0x7e, 0xb4, 0x5f, 0xad, 0x56, 0xab,
    0x71, 0xa1, 0x80,

    /* U+529B "力" */
    0x8, 0x4, 0x3f, 0xe1, 0x10, 0x88, 0x84, 0x42,
    0x41, 0xc3, 0x0,

    /* U+53E3 "口" */
    0xff, 0x6, 0xc, 0x18, 0x30, 0x7f, 0x80,

    /* U+53EA "只" */
    0x7f, 0x20, 0x90, 0x48, 0x27, 0xf0, 0x0, 0x88,
    0x82, 0x80, 0x80,

    /* U+540D "名" */
    0x10, 0x1f, 0x98, 0x52, 0x40, 0xc1, 0xff, 0x42,
    0x21, 0x1f, 0x80,

    /* U+540E "后" */
    0x7, 0x3c, 0x10, 0xf, 0xf4, 0x2, 0xfd, 0x42,
    0xa1, 0x9f, 0x80,

    /* U+547D "命" */
    0x1c, 0x11, 0x37, 0x60, 0xf, 0x7c, 0xa6, 0x53,
    0xeb, 0x4, 0x0,

    /* U+55B5 "喵" */
    0x5, 0x77, 0xe9, 0x55, 0xfa, 0xad, 0x7f, 0xaa,
    0x15, 0xf, 0x80,

    /* U+5668 "器" */
    0x77, 0x2a, 0x9d, 0xc1, 0x4f, 0xf9, 0x13, 0xde,
    0xaa, 0x77, 0x0,

    /* U+56DB "四" */
    0xff, 0xca, 0x65, 0x32, 0x99, 0x4d, 0x1f, 0x3,
    0x1, 0xff, 0x80,

    /* U+56DE "回" */
    0xff, 0xc0, 0x6f, 0xb4, 0x5a, 0x2d, 0x16, 0xfb,
    0x1, 0xff, 0x80,

    /* U+5728 "在" */
    0x10, 0x7f, 0xc8, 0x4, 0x84, 0x46, 0xf9, 0x10,
    0x88, 0x5f, 0x80,

    /* U+5730 "地" */
    0x4a, 0x25, 0x12, 0xfd, 0xd5, 0xaa, 0x55, 0xa1,
    0x91, 0xf, 0x80,

    /* U+578B "型" */
    0xfa, 0xa9, 0x7e, 0xaa, 0x19, 0x18, 0x41, 0xfc,
    0x10, 0xff, 0x80,

    /* U+5929 "天" */
    0x7f, 0x4, 0x2, 0x1f, 0xf0, 0x80, 0x40, 0x50,
    0x44, 0xc1, 0x80,

    /* U+5934 "头" */
    0x24, 0xa, 0x11, 0x4, 0x80, 0x87, 0xfc, 0x48,
    0x42, 0xc0, 0x80,

    /* U+5B87 "宇" */
    0x8, 0x7f, 0xe0, 0x27, 0xc0, 0x87, 0xfc, 0x20,
    0x10, 0x38, 0x0,

    /* U+5B8C "完" */
    0x8, 0x7f, 0xe0, 0x37, 0xd0, 0x7, 0xfc, 0x50,
    0x49, 0xc7, 0x80,

    /* U+5B99 "宙" */
    0x8, 0x7f, 0xe2, 0x2f, 0xe4, 0x93, 0xf9, 0x24,
    0x92, 0x7f, 0x0,

    /* U+5B9A "定" */
    0x8, 0x7f, 0xe0, 0x2f, 0xe0, 0x81, 0x78, 0xa0,
    0xb0, 0x87, 0x80,

    /* U+5C0F "小" */
    0x8, 0x4, 0x12, 0x49, 0x24, 0x8c, 0x46, 0x22,
    0x10, 0x18, 0x0,

    /* U+5C31 "就" */
    0x22, 0xf9, 0x3, 0xfe, 0x49, 0x57, 0xa8, 0x95,
    0x6a, 0x69, 0x80,

    /* U+5C40 "局" */
    0x7f, 0x20, 0x9f, 0xc8, 0x7, 0xfa, 0x5, 0x72,
    0xa9, 0x9d, 0x80,

    /* U+5DE5 "工" */
    0x7f, 0x4, 0x2, 0x1, 0x0, 0x80, 0x40, 0x20,
    0x10, 0xff, 0x80,

    /* U+5DF2 "已" */
    0xff, 0x0, 0xa0, 0x50, 0x2f, 0xf4, 0x2, 0x1,
    0x1, 0x7f, 0x80,

    /* U+5E74 "年" */
    0x20, 0x1f, 0xd1, 0x17, 0xe2, 0x41, 0x23, 0xfe,
    0x8, 0x4, 0x0,

    /* U+5EA6 "度" */
    0x4, 0x3f, 0xd2, 0x8f, 0xf4, 0xa2, 0xf9, 0x44,
    0x9c, 0xb1, 0x80,

    /* U+5F00 "开" */
    0xff, 0x91, 0x8, 0x84, 0x4f, 0xf9, 0x11, 0x8,
    0x84, 0x82, 0x0,

    /* U+5F0F "式" */
    0x5, 0x2, 0x7f, 0xe0, 0x8f, 0xc1, 0x10, 0x8a,
    0x73, 0xc0, 0x80,

    /* U+606F "息" */
    0x10, 0x3f, 0x90, 0x4f, 0xe4, 0x13, 0xf8, 0x21,
    0x4a, 0xbe, 0x80,

    /* U+6210 "成" */
    0x5, 0x3f, 0xd1, 0xe, 0xa5, 0x52, 0xa9, 0x48,
    0xe5, 0x8d, 0x80,

    /* U+6211 "我" */
    0x15, 0x72, 0x49, 0x1f, 0xf2, 0x41, 0xab, 0x88,
    0x45, 0x6d, 0x80,

    /* U+624B "手" */
    0x7, 0x3c, 0x2, 0xf, 0xe0, 0x87, 0xfc, 0x20,
    0x10, 0x38, 0x0,

    /* U+652F "支" */
    0x8, 0x7f, 0xc2, 0x1, 0x7, 0xf2, 0x8, 0x88,
    0x38, 0xe3, 0x80,

    /* U+6587 "文" */
    0x10, 0x4, 0x3f, 0xe4, 0x42, 0x20, 0xa0, 0x20,
    0x28, 0xe3, 0x80,

    /* U+65E0 "无" */
    0x7f, 0x4, 0x2, 0x1f, 0xf0, 0x80, 0xa0, 0x52,
    0x49, 0xc7, 0x80,

    /* U+65E5 "日" */
    0xff, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0xfe,

    /* U+662F "是" */
    0x7f, 0x20, 0x9f, 0xc8, 0x2f, 0xf8, 0x41, 0x3c,
    0xd0, 0x9f, 0x80,

    /* U+663E "显" */
    0x7f, 0x20, 0x9f, 0xc8, 0x27, 0xf4, 0xa5, 0x54,
    0x28, 0xff, 0x80,

    /* U+6700 "最" */
    0x7f, 0x20, 0x9f, 0xc8, 0x2f, 0xfa, 0x55, 0xea,
    0x92, 0xfa, 0x80,

    /* U+6708 "月" */
    0x3f, 0x90, 0x4f, 0xe4, 0x12, 0x9, 0xfc, 0x82,
    0x81, 0x81, 0x80,

    /* U+672A "未" */
    0x8, 0x3f, 0x82, 0x1, 0xf, 0xf8, 0xe0, 0xa9,
    0x93, 0x8, 0x0,

    /* U+672C "本" */
    0x8, 0x4, 0x3f, 0xe3, 0x82, 0xa1, 0x51, 0x25,
    0x7d, 0x8, 0x0,

    /* U+6765 "来" */
    0x8, 0x3f, 0x82, 0x5, 0x4f, 0xf8, 0xe0, 0xa8,
    0x92, 0x88, 0x80,

    /* U+683C "格" */
    0x48, 0x27, 0xbc, 0x49, 0x2e, 0x66, 0xcf, 0x3c,
    0x92, 0x4f, 0x0,

    /* U+6848 "案" */
    0x8, 0x7f, 0xe4, 0x2f, 0xe2, 0x20, 0xe3, 0xfe,
    0x54, 0xc9, 0x80,

    /* U+684C "桌" */
    0xf, 0x84, 0x1f, 0xc8, 0x27, 0xf2, 0xb, 0xfe,
    0x54, 0xc9, 0x80,

    /* U+6B63 "正" */
    0xff, 0x84, 0x2, 0x9, 0x4, 0xf2, 0x41, 0x20,
    0x90, 0xff, 0x80,

    /* U+6D77 "海" */
    0x90, 0x2f, 0xc8, 0x13, 0xe5, 0x51, 0xfd, 0x54,
    0xbf, 0x81, 0x0,

    /* U+6E90 "源" */
    0x9f, 0xa9, 0x5, 0xf2, 0x95, 0x78, 0xa5, 0x5e,
    0xa4, 0xaa, 0x80,

    /* U+70B9 "点" */
    0x8, 0x7, 0xc2, 0xf, 0xe4, 0x13, 0xf8, 0x0,
    0xaa, 0x8a, 0x80,

    /* U+751F "生" */
    0x48, 0x24, 0x1f, 0xd1, 0x0, 0x83, 0xf8, 0x20,
    0x10, 0xff, 0x80,

    /* U+7528 "用" */
    0x7f, 0xa4, 0x5f, 0xe9, 0x14, 0x8b, 0xfd, 0x22,
    0x91, 0x89, 0x80,

    /* U+7531 "由" */
    0x8, 0x4, 0x3f, 0xf1, 0x18, 0x8f, 0xfe, 0x23,
    0x11, 0xff, 0x80,

    /* U+7535 "电" */
    0x10, 0x7f, 0xa4, 0x5f, 0xe9, 0x14, 0x8b, 0xfc,
    0x21, 0xf, 0x80,

    /* U+7684 "的" */
    0x24, 0x23, 0xfe, 0x32, 0x19, 0x4f, 0x96, 0x43,
    0x21, 0xf3, 0x0,

    /* U+7801 "码" */
    0xef, 0x20, 0xa2, 0x5d, 0x2a, 0xfd, 0x6, 0xfb,
    0xc1, 0x3, 0x0,

    /* U+79F0 "称" */
    0x14, 0x73, 0xca, 0x3e, 0x42, 0x23, 0x59, 0xab,
    0x65, 0x26, 0x0,

    /* U+7A7A "空" */
    0x8, 0x7f, 0xe0, 0x24, 0x4c, 0x1b, 0xf8, 0x20,
    0x10, 0xff, 0x80,

    /* U+7B49 "等" */
    0x44, 0x3b, 0xea, 0x41, 0x7, 0xf0, 0x13, 0xfe,
    0x44, 0x16, 0x0,

    /* U+7C73 "米" */
    0x8, 0x24, 0x8a, 0x81, 0xf, 0xf8, 0xe0, 0xa9,
    0x93, 0x8, 0x0,

    /* U+7F51 "网" */
    0xff, 0xc0, 0x75, 0x74, 0x5a, 0x2e, 0xaf, 0x57,
    0x1, 0x81, 0x80,

    /* U+7F6E "置" */
    0x7f, 0x2a, 0xbf, 0xe1, 0x7, 0xf2, 0x9, 0xfc,
    0x82, 0xff, 0x80,

    /* U+821F "舟" */
    0x8, 0x3f, 0x90, 0x4a, 0x24, 0x97, 0xfd, 0x44,
    0x92, 0x83, 0x0,

    /* U+884C "行" */
    0x4f, 0xc0, 0x8, 0xb, 0xfc, 0x12, 0x9, 0x4,
    0x82, 0x47, 0x0,

    /* U+8981 "要" */
    0xff, 0x8a, 0x1f, 0xca, 0xa7, 0xf0, 0x83, 0xfe,
    0x24, 0x6f, 0x0,

    /* U+8A00 "言" */
    0x10, 0x4, 0x3f, 0xe0, 0x7, 0xf0, 0x1, 0xfc,
    0x82, 0x7f, 0x0,

    /* U+8BA4 "认" */
    0x84, 0x22, 0x1, 0x18, 0x84, 0x42, 0x51, 0xa8,
    0xa2, 0x60, 0x80,

    /* U+8BED "语" */
    0x9f, 0xa2, 0x7, 0xd9, 0x25, 0xfa, 0x1, 0x7e,
    0xe1, 0x5f, 0x80,

    /* U+8BF7 "请" */
    0x82, 0x27, 0xc0, 0x9b, 0xf4, 0x8a, 0x7d, 0x22,
    0xdf, 0x48, 0x80,

    /* U+8BFB "读" */
    0x82, 0x27, 0xc0, 0x9b, 0xf4, 0x4a, 0x51, 0x7e,
    0xca, 0x58, 0x80,

    /* U+8FC7 "过" */
    0x81, 0x20, 0x87, 0xe1, 0x2c, 0x52, 0x9, 0xd,
    0x40, 0x9f, 0x80,

    /* U+8FD9 "这" */
    0x84, 0x2f, 0xc0, 0x41, 0x2c, 0x92, 0x31, 0x66,
    0x80, 0xbf, 0x80,

    /* U+8FDE "连" */
    0x82, 0x2f, 0xc1, 0x1, 0x4c, 0xfa, 0x11, 0x7f,
    0x44, 0x9f, 0x80,

    /* U+914D "配" */
    0xf7, 0xa0, 0x7c, 0x3a, 0xfb, 0x44, 0xa3, 0xd3,
    0x29, 0xf7, 0x80,

    /* U+91CD "重" */
    0x7f, 0x4, 0x3f, 0xe9, 0x27, 0xf2, 0x49, 0xfc,
    0x10, 0xff, 0x80,

    /* U+91CF "量" */
    0x7f, 0x20, 0xbf, 0xe9, 0x27, 0xf2, 0x49, 0xfc,
    0x10, 0xff, 0x80,

    /* U+91D1 "金" */
    0x1c, 0x11, 0x10, 0x57, 0xd0, 0x83, 0xf8, 0x20,
    0x54, 0xff, 0x80,

    /* U+95ED "闭" */
    0x9f, 0xa0, 0x41, 0x30, 0x9b, 0xec, 0x66, 0x53,
    0x49, 0x8d, 0x80,

    /* U+95F2 "闲" */
    0x9f, 0xa0, 0x42, 0x37, 0xd8, 0x8c, 0xe6, 0xab,
    0x11, 0x81, 0x80,

    /* U+95F4 "间" */
    0x9f, 0xa0, 0x4f, 0xb4, 0x5b, 0xed, 0x16, 0x8b,
    0x7d, 0x81, 0x80,

    /* U+968F "随" */
    0xe1, 0x5b, 0xe8, 0x9b, 0x7a, 0xed, 0x5f, 0xab,
    0x10, 0x97, 0x80,

    /* U+9762 "面" */
    0xff, 0x84, 0x3f, 0xf4, 0x5b, 0xed, 0x16, 0xfb,
    0x45, 0xff, 0x80,

    /* U+9875 "页" */
    0xff, 0x84, 0x1f, 0xc8, 0x24, 0x92, 0x49, 0x54,
    0x44, 0xc1, 0x80,

    /* U+9ED8 "默" */
    0xfa, 0xd5, 0x7e, 0x95, 0xff, 0xa1, 0x13, 0xf4,
    0xa, 0xa8, 0x80,

    /* U+FF01 "！" */
    0xfe, 0x80,

    /* U+FF0C "，" */
    0xf6
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 80, .box_w = 1, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 3, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 12, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 80, .box_w = 1, .box_h = 2, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 21, .adv_w = 80, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 25, .adv_w = 80, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 29, .adv_w = 80, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 80, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 35, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 36, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 42, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 80, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 53, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 80, .box_w = 1, .box_h = 5, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 80, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 84, .adv_w = 80, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 88, .adv_w = 80, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 80, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 206, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 211, .adv_w = 80, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 215, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 216, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 217, .adv_w = 80, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 218, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 242, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 80, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 253, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 273, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 276, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 301, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 309, .adv_w = 80, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 311, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 316, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 317, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 319, .adv_w = 160, .box_w = 2, .box_h = 2, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 320, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 322, .adv_w = 160, .box_w = 9, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 324, .adv_w = 160, .box_w = 4, .box_h = 4, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 326, .adv_w = 160, .box_w = 8, .box_h = 2, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 328, .adv_w = 160, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 334, .adv_w = 160, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 160, .box_w = 10, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 343, .adv_w = 160, .box_w = 6, .box_h = 5, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 347, .adv_w = 160, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 351, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 362, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 373, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 384, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 395, .adv_w = 160, .box_w = 9, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 397, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 408, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 419, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 430, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 441, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 452, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 463, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 474, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 485, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 496, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 505, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 513, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 524, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 535, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 546, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 557, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 568, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 579, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 590, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 601, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 612, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 623, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 634, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 645, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 656, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 667, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 678, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 689, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 700, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 711, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 722, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 733, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 744, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 755, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 766, .adv_w = 160, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 773, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 784, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 795, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 806, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 817, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 828, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 839, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 850, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 861, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 872, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 883, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 894, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 905, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 916, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 927, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 938, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 949, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 960, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 971, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 982, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 993, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1004, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1015, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1026, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1037, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1048, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1059, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1070, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1081, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1092, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1103, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1114, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1125, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1136, .adv_w = 160, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1144, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1155, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1166, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1177, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1188, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1199, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1210, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1221, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1232, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1243, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1254, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1265, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1276, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1287, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1298, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1309, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1320, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1331, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1342, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1353, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1364, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1375, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1386, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1397, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1408, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1419, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1430, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1441, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1452, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1463, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1474, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1485, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1496, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1507, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1518, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1529, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1540, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1551, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1562, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1573, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1584, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1595, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1606, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1617, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1628, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1639, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1650, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1661, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1672, .adv_w = 160, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1674, .adv_w = 160, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0xc, 0x10, 0x1f69, 0x1f77, 0x1f7b, 0x20e7, 0x2110,
    0x2455, 0x2461, 0x2465, 0x24f5, 0x250b, 0x2524, 0x255a, 0x4d55,
    0x4d56, 0x4d5c, 0x4d5e, 0x4d5f, 0x4d60, 0x4d62, 0x4d7f, 0x4d82,
    0x4d90, 0x4ddb, 0x4de1, 0x4df9, 0x4e4b, 0x4e6f, 0x4ea2, 0x4ea4,
    0x4eae, 0x4eb5, 0x4f32, 0x5024, 0x5094, 0x50ba, 0x50bd, 0x50c8,
    0x50cb, 0x50cc, 0x50da, 0x50ee, 0x514f, 0x5150, 0x516f, 0x5185,
    0x518c, 0x51f0, 0x5338, 0x533f, 0x5362, 0x5363, 0x53d2, 0x550a,
    0x55bd, 0x5630, 0x5633, 0x567d, 0x5685, 0x56e0, 0x587e, 0x5889,
    0x5adc, 0x5ae1, 0x5aee, 0x5aef, 0x5b64, 0x5b86, 0x5b95, 0x5d3a,
    0x5d47, 0x5dc9, 0x5dfb, 0x5e55, 0x5e64, 0x5fc4, 0x6165, 0x6166,
    0x61a0, 0x6484, 0x64dc, 0x6535, 0x653a, 0x6584, 0x6593, 0x6655,
    0x665d, 0x667f, 0x6681, 0x66ba, 0x6791, 0x679d, 0x67a1, 0x6ab8,
    0x6ccc, 0x6de5, 0x700e, 0x7474, 0x747d, 0x7486, 0x748a, 0x75d9,
    0x7756, 0x7945, 0x79cf, 0x7a9e, 0x7bc8, 0x7ea6, 0x7ec3, 0x8174,
    0x87a1, 0x88d6, 0x8955, 0x8af9, 0x8b42, 0x8b4c, 0x8b50, 0x8f1c,
    0x8f2e, 0x8f33, 0x90a2, 0x9122, 0x9124, 0x9126, 0x9542, 0x9547,
    0x9549, 0x95e4, 0x96b7, 0x97ca, 0x9e2d, 0xfe56, 0xfe61
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 171, .range_length = 65122, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 135, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t ark10 = {
#else
lv_font_t ark10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if ARK10*/


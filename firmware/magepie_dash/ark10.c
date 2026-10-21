/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --font D:\esp32\tools\fonts\ark-pixel-10px-proportional-zh_hans.ttf --size 10 --bpp 1 --format lvgl --lv-include lvgl.h --no-compress --force-fast-kern-format -r 0x20-0x7e -o D:\esp32\magepie-esp32\firmware\magepie_dash\ark10.c -r 0xb7 -r 0x2014 -r 0x2192 -r 0x21bb -r 0x25a0 -r 0x25cf -r 0x2605 -r 0x4e00 -r 0x4e01 -r 0x4e07 -r 0x4e09 -r 0x4e0a -r 0x4e0b -r 0x4e0d -r 0x4e2a -r 0x4e2d -r 0x4e86 -r 0x4e8c -r 0x4ea4 -r 0x4ef6 -r 0x4f1a -r 0x4f4d -r 0x4f4f -r 0x4f59 -r 0x4f60 -r 0x4fdd -r 0x50cf -r 0x513f -r 0x5165 -r 0x5168 -r 0x5173 -r 0x5176 -r 0x5177 -r 0x5185 -r 0x5199 -r 0x51fa -r 0x51fb -r 0x521a -r 0x5230 -r 0x5237 -r 0x529b -r 0x53e3 -r 0x53ea -r 0x540d -r 0x540e -r 0x547d -r 0x55b5 -r 0x5668 -r 0x56db -r 0x56de -r 0x5728 -r 0x5730 -r 0x578b -r 0x5929 -r 0x5934 -r 0x5b87 -r 0x5b8c -r 0x5b99 -r 0x5b9a -r 0x5c0f -r 0x5c31 -r 0x5c40 -r 0x5de5 -r 0x5df2 -r 0x5e74 -r 0x5ea6 -r 0x5f00 -r 0x5f0f -r 0x606f -r 0x6210 -r 0x6211 -r 0x624b -r 0x652f -r 0x6587 -r 0x65e0 -r 0x65e5 -r 0x662f -r 0x663e -r 0x6700 -r 0x6708 -r 0x672a -r 0x672c -r 0x6765 -r 0x683c -r 0x6848 -r 0x684c -r 0x6b63 -r 0x6d77 -r 0x6e90 -r 0x70b9 -r 0x751f -r 0x7528 -r 0x7531 -r 0x7535 -r 0x7684 -r 0x7801 -r 0x79f0 -r 0x7a7a -r 0x7b49 -r 0x7c73 -r 0x7f51 -r 0x7f6e -r 0x821f -r 0x884c -r 0x8981 -r 0x8ba4 -r 0x8bf7 -r 0x8bfb -r 0x8fc7 -r 0x8fd9 -r 0x8fde -r 0x914d -r 0x91cd -r 0x91cf -r 0x91d1 -r 0x95ed -r 0x95f2 -r 0x95f4 -r 0x968f -r 0x9762 -r 0x9ed8 -r 0xff01 -r 0xff0c
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
    0x57, 0xd4, 0xa5, 0x7d, 0x40,

    /* U+0024 "$" */
    0x23, 0xab, 0x47, 0x16, 0xae, 0x20,

    /* U+0025 "%" */
    0x45, 0x51, 0x20, 0x82, 0x45, 0x51, 0x0,

    /* U+0026 "&" */
    0x21, 0x45, 0x19, 0x96, 0x27, 0x40,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x29, 0x49, 0x12, 0x20,

    /* U+0029 ")" */
    0x89, 0x12, 0x52, 0x80,

    /* U+002A "*" */
    0x25, 0x5d, 0x52, 0x0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xf8,

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
    0x40, 0x58,

    /* U+003C "<" */
    0x2a, 0x22,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0x74, 0x42, 0x22, 0x0, 0x80,

    /* U+0040 "@" */
    0x74, 0x67, 0x5a, 0xde, 0xe,

    /* U+0041 "A" */
    0x21, 0x14, 0xa7, 0x46, 0x20,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+0043 "C" */
    0x74, 0x61, 0x8, 0x45, 0xc0,

    /* U+0044 "D" */
    0xf4, 0x63, 0x18, 0xc7, 0xc0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x43, 0xe0,

    /* U+0046 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x0,

    /* U+0047 "G" */
    0x74, 0x61, 0x9, 0xc5, 0xe0,

    /* U+0048 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0049 "I" */
    0xe9, 0x24, 0xb8,

    /* U+004A "J" */
    0x11, 0x11, 0x99, 0x60,

    /* U+004B "K" */
    0x8c, 0xa9, 0x8a, 0x4a, 0x20,

    /* U+004C "L" */
    0x88, 0x88, 0x88, 0xf0,

    /* U+004D "M" */
    0x83, 0x8f, 0x1d, 0x5a, 0xb2, 0x64, 0x80,

    /* U+004E "N" */
    0x8e, 0x73, 0x59, 0xce, 0x20,

    /* U+004F "O" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0050 "P" */
    0xf4, 0x63, 0xe8, 0x42, 0x0,

    /* U+0051 "Q" */
    0x74, 0x63, 0x1a, 0xc9, 0xa0,

    /* U+0052 "R" */
    0xf4, 0x63, 0xe9, 0x46, 0x20,

    /* U+0053 "S" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0056 "V" */
    0x8c, 0x54, 0xa5, 0x10, 0x80,

    /* U+0057 "W" */
    0x93, 0x26, 0x4d, 0x5a, 0xa8, 0x91, 0x0,

    /* U+0058 "X" */
    0x8c, 0x54, 0x45, 0x46, 0x20,

    /* U+0059 "Y" */
    0x8c, 0x54, 0xa2, 0x10, 0x80,

    /* U+005A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

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
    0x79, 0x99, 0x71, 0x60,

    /* U+0068 "h" */
    0x88, 0xe9, 0x99, 0x90,

    /* U+0069 "i" */
    0x43, 0x24, 0xb8,

    /* U+006A "j" */
    0x23, 0x92, 0x49, 0xc0,

    /* U+006B "k" */
    0x88, 0x9a, 0xca, 0x90,

    /* U+006C "l" */
    0xc9, 0x24, 0x98,

    /* U+006D "m" */
    0xd5, 0x6b, 0x5a, 0x80,

    /* U+006E "n" */
    0xe9, 0x99, 0x90,

    /* U+006F "o" */
    0x69, 0x99, 0x60,

    /* U+0070 "p" */
    0xe9, 0x99, 0xe8, 0x80,

    /* U+0071 "q" */
    0x79, 0x99, 0x71, 0x10,

    /* U+0072 "r" */
    0xba, 0x48,

    /* U+0073 "s" */
    0x78, 0x61, 0xe0,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x30,

    /* U+0075 "u" */
    0x99, 0x99, 0x70,

    /* U+0076 "v" */
    0x8c, 0x54, 0xa2, 0x0,

    /* U+0077 "w" */
    0xad, 0x6a, 0xa5, 0x0,

    /* U+0078 "x" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+0079 "y" */
    0x8c, 0x54, 0xa2, 0x13, 0x0,

    /* U+007A "z" */
    0xf2, 0x48, 0xf0,

    /* U+007B "{" */
    0x34, 0x44, 0x84, 0x44, 0x30,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0xc2, 0x22, 0x12, 0x22, 0xc0,

    /* U+007E "~" */
    0x45, 0x44,

    /* U+00B7 "·" */
    0xf0,

    /* U+2014 "—" */
    0xff, 0x80,

    /* U+2192 "→" */
    0x2, 0x0, 0xbf, 0xe0, 0x20, 0x20,

    /* U+21BB "↻" */
    0x70, 0x61, 0x54, 0x18, 0x30, 0x51, 0x1c,

    /* U+25A0 "■" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x80,

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

    /* U+8BA4 "认" */
    0x84, 0x22, 0x1, 0x18, 0x84, 0x42, 0x51, 0xa8,
    0xa2, 0x60, 0x80,

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
    {.bitmap_index = 1, .adv_w = 64, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 96, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 8, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 14, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 64, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 28, .adv_w = 96, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 32, .adv_w = 96, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 36, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 44, .adv_w = 64, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 45, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 46, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 52, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 80, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 64, .box_w = 1, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 64, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 94, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 98, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 110, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 120, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 96, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 244, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 249, .adv_w = 96, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 253, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 254, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 255, .adv_w = 80, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 256, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 281, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 292, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 313, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 317, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 346, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 112, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 354, .adv_w = 64, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 356, .adv_w = 112, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 361, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 363, .adv_w = 160, .box_w = 2, .box_h = 2, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 364, .adv_w = 160, .box_w = 9, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 366, .adv_w = 160, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 372, .adv_w = 160, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 390, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 401, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 412, .adv_w = 160, .box_w = 9, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 414, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 425, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 436, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 447, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 458, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 469, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 480, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 491, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 502, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 511, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 530, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 541, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 552, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 563, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 574, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 585, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 596, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 607, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 618, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 629, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 640, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 651, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 662, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 673, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 684, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 695, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 706, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 717, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 728, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 739, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 750, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 761, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 772, .adv_w = 160, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 779, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 790, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 801, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 812, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 823, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 834, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 845, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 856, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 867, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 878, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 889, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 900, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 911, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 922, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 933, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 944, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 955, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 966, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 977, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 988, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 999, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1010, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1021, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1032, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1043, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1054, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1065, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1076, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1087, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1098, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1109, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1120, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1131, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1142, .adv_w = 160, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1150, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1161, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1172, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1183, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1194, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1205, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1216, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1227, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1238, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1249, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1260, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1271, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1282, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1293, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1304, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1315, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1326, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1337, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1348, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1359, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1370, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1381, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1392, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1403, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1414, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1425, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1436, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1447, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1458, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1469, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1480, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1491, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1502, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1513, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1524, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1535, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1546, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1557, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1568, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1579, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1590, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1601, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1612, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1623, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1634, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1645, .adv_w = 160, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1647, .adv_w = 160, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x1f5d, 0x20db, 0x2104, 0x24e9, 0x2518, 0x254e, 0x4d49,
    0x4d4a, 0x4d50, 0x4d52, 0x4d53, 0x4d54, 0x4d56, 0x4d73, 0x4d76,
    0x4dcf, 0x4dd5, 0x4ded, 0x4e3f, 0x4e63, 0x4e96, 0x4e98, 0x4ea2,
    0x4ea9, 0x4f26, 0x5018, 0x5088, 0x50ae, 0x50b1, 0x50bc, 0x50bf,
    0x50c0, 0x50ce, 0x50e2, 0x5143, 0x5144, 0x5163, 0x5179, 0x5180,
    0x51e4, 0x532c, 0x5333, 0x5356, 0x5357, 0x53c6, 0x54fe, 0x55b1,
    0x5624, 0x5627, 0x5671, 0x5679, 0x56d4, 0x5872, 0x587d, 0x5ad0,
    0x5ad5, 0x5ae2, 0x5ae3, 0x5b58, 0x5b7a, 0x5b89, 0x5d2e, 0x5d3b,
    0x5dbd, 0x5def, 0x5e49, 0x5e58, 0x5fb8, 0x6159, 0x615a, 0x6194,
    0x6478, 0x64d0, 0x6529, 0x652e, 0x6578, 0x6587, 0x6649, 0x6651,
    0x6673, 0x6675, 0x66ae, 0x6785, 0x6791, 0x6795, 0x6aac, 0x6cc0,
    0x6dd9, 0x7002, 0x7468, 0x7471, 0x747a, 0x747e, 0x75cd, 0x774a,
    0x7939, 0x79c3, 0x7a92, 0x7bbc, 0x7e9a, 0x7eb7, 0x8168, 0x8795,
    0x88ca, 0x8aed, 0x8b40, 0x8b44, 0x8f10, 0x8f22, 0x8f27, 0x9096,
    0x9116, 0x9118, 0x911a, 0x9536, 0x953b, 0x953d, 0x95d8, 0x96ab,
    0x9e21, 0xfe4a, 0xfe55
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 183, .range_length = 65110, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 123, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 3,
    0, 0, 0, 0, 0, 4, 0, 0,
    0, 5, 0, 0, 0, 6, 0, 7,
    8, 0, 7, 0, 0, 9, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 10,
    0, 0, 11, 0, 0, 11, 0, 0,
    0, 0, 0, 10, 0, 0, 0, 8,
    8, 0, 8, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 4, 0, 4,
    5, 0, 4, 0, 0, 6, 0, 0,
    0, 0, 7, 0, 8, 8, 8, 9,
    10, 0, 11, 12, 0, 4, 11, 11,
    8, 11, 8, 11, 8, 9, 11, 9,
    13, 11, 13, 11, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, -16, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -16, 0, 0, 0, 0, -16, 0, 0,
    -16, 0, -16, 0, -16, 0, 0, 0,
    -16, -16, -16, -16, -16, -16, -16, 0,
    0, 0, -16, -16, 0, 0, 0, -16,
    0, 0, -16, -16, -16, 0, -16, 0,
    0, 0, -16, 0, 0, 0, 0, 0,
    0, -16, 0, -16, 0, 0, 0, -16,
    -16, -16, 0, -16, -16, -16, -16, 0,
    -16, 0, 0, 0, -16, -16, 0, -16,
    0, 0, 0, -16, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -16, 0, 0,
    0, 0, 0, 0, 0, -16, 0, 0,
    0, 0, 0, -16, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -16, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 11,
    .right_class_cnt     = 13,
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
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 1,
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
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
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


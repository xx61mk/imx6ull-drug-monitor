/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --font C:/Users/徐翔/SquareLine/assets/STXINGKA.TTF -o C:/Users/徐翔/SquareLine/assets\ui_font_Font4.c --format lvgl -r 0x20-0x7f --symbols 你好啊 --no-compress --no-prefilter
 ******************************************************************************/

#include "../ui.h"

#ifndef UI_FONT_FONT4
#define UI_FONT_FONT4 1
#endif

#if UI_FONT_FONT4

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x8, 0xc4, 0x62, 0x31, 0x8, 0x4, 0x20,

    /* U+0022 "\"" */
    0xb6, 0xd0,

    /* U+0023 "#" */
    0x11, 0x8, 0x84, 0x5f, 0xf2, 0x21, 0x10, 0x89,
    0xff, 0x46, 0x22, 0x11, 0x0,

    /* U+0024 "$" */
    0x0, 0x80, 0x10, 0x3f, 0xd, 0x22, 0x38, 0x48,
    0xf, 0x0, 0x70, 0xb, 0x2, 0x10, 0x4c, 0x7f,
    0x8f, 0xc0, 0x80, 0x10, 0x0,

    /* U+0025 "%" */
    0x30, 0x51, 0x92, 0xa4, 0xc8, 0x10, 0x36, 0x6a,
    0xd2, 0x94, 0x98,

    /* U+0026 "&" */
    0x7, 0x9, 0x13, 0x1c, 0x18, 0x31, 0x5b, 0x8e,
    0x9c, 0xfe, 0x70,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x12, 0x24, 0xc8, 0x88, 0x88, 0x40,

    /* U+0029 ")" */
    0x11, 0x11, 0x13, 0x22, 0x64, 0x80,

    /* U+002A "*" */
    0x25, 0x7c, 0xf8, 0x80,

    /* U+002B "+" */
    0x8, 0x4, 0x2, 0x1, 0xf, 0xf8, 0x40, 0x20,
    0x10, 0x8, 0x0,

    /* U+002C "," */
    0x28,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xe0,

    /* U+002F "/" */
    0x11, 0x11, 0x22, 0x26, 0x44, 0x4c, 0x80,

    /* U+0030 "0" */
    0x6, 0x19, 0x31, 0x61, 0x43, 0xc2, 0x86, 0x8c,
    0x98, 0x70,

    /* U+0031 "1" */
    0x1, 0x94, 0x62, 0x31, 0x18, 0xc4, 0x20,

    /* U+0032 "2" */
    0xe, 0x32, 0x22, 0x76, 0x1c, 0x38, 0x60, 0xc4,
    0xfc, 0xf8,

    /* U+0033 "3" */
    0xf, 0x86, 0x10, 0xc, 0xe, 0x1c, 0x7, 0x80,
    0x10, 0x4, 0x46, 0x1e, 0x0,

    /* U+0034 "4" */
    0x3, 0xd, 0x1a, 0x32, 0x64, 0xcc, 0x8e, 0xfc,
    0x10, 0x10, 0x10,

    /* U+0035 "5" */
    0x7, 0x83, 0xe1, 0x0, 0xf8, 0x11, 0x0, 0x40,
    0x31, 0x38, 0x7c, 0x1e, 0x0,

    /* U+0036 "6" */
    0x6, 0x38, 0xc3, 0x7, 0xdc, 0xb1, 0x44, 0x98,
    0xe0,

    /* U+0037 "7" */
    0x7e, 0xfe, 0x10, 0x41, 0x4, 0x18, 0x20, 0xc1,
    0x80,

    /* U+0038 "8" */
    0xf, 0x3f, 0x32, 0x24, 0x28, 0x10, 0x38, 0x48,
    0xc8, 0x98, 0xf0,

    /* U+0039 "9" */
    0xc, 0x7d, 0xc7, 0x1c, 0x53, 0x9a, 0xc, 0x10,
    0x41, 0x0,

    /* U+003A ":" */
    0x2, 0x0, 0x88,

    /* U+003B ";" */
    0x2, 0x20, 0x8, 0x80,

    /* U+003C "<" */
    0x3, 0x87, 0xe, 0x1c, 0xe, 0x1, 0xc0, 0x1c,
    0x3,

    /* U+003D "=" */
    0xff, 0x80, 0x0, 0x0, 0xf, 0xf8,

    /* U+003E ">" */
    0xc0, 0x38, 0x3, 0x80, 0x70, 0x70, 0xe1, 0xc1,
    0x80,

    /* U+003F "?" */
    0x3f, 0x10, 0x43, 0x18, 0xc6, 0x10, 0x42, 0x8,
    0x0,

    /* U+0040 "@" */
    0x7, 0xc0, 0x60, 0xc2, 0x1, 0x91, 0xda, 0x4c,
    0xc6, 0x23, 0x19, 0x4, 0x64, 0x11, 0x90, 0xc6,
    0x43, 0x29, 0x99, 0x13, 0xb8, 0x60, 0xc, 0x60,
    0x60, 0xfe, 0x0,

    /* U+0041 "A" */
    0x0, 0x10, 0x3, 0x80, 0x2c, 0x2, 0x40, 0x26,
    0xf, 0xf0, 0x7f, 0x82, 0x10, 0x21, 0x82, 0xe,
    0x20, 0x60,

    /* U+0042 "B" */
    0xf, 0x8c, 0x16, 0x44, 0x32, 0xf, 0x1c, 0x3,
    0xe0, 0x8c, 0x61, 0x18, 0x87, 0xc0,

    /* U+0043 "C" */
    0x1, 0xc0, 0xd0, 0xc4, 0x63, 0x18, 0x8c, 0xc6,
    0x71, 0x98, 0xc0, 0x30, 0x2c, 0x13, 0x18, 0x78,
    0x0,

    /* U+0044 "D" */
    0xf, 0x8f, 0xfb, 0xf1, 0xe4, 0x18, 0x82, 0x20,
    0x44, 0x9, 0x2, 0x20, 0x88, 0x21, 0x18, 0x3c,
    0x0,

    /* U+0045 "E" */
    0x7, 0xc7, 0xf1, 0x88, 0xc0, 0x20, 0x7, 0xc0,
    0x70, 0x60, 0x60, 0x20, 0x68, 0x73, 0xfc, 0x78,
    0x0,

    /* U+0046 "F" */
    0x7, 0xdf, 0xf7, 0x8f, 0x2, 0xc0, 0x80, 0x40,
    0x20, 0xfc, 0x4, 0x2, 0x1, 0x1, 0x80, 0x40,
    0x0,

    /* U+0047 "G" */
    0x1, 0xc1, 0xf1, 0xe8, 0xe0, 0x70, 0x98, 0x6c,
    0x72, 0x28, 0x92, 0x99, 0x40, 0xe0, 0x20, 0x30,
    0x18, 0x6, 0x0,

    /* U+0048 "H" */
    0x7, 0x83, 0xc9, 0x60, 0x9e, 0x12, 0x82, 0x40,
    0x64, 0x7, 0xe3, 0xfc, 0x31, 0x2, 0x30, 0x22,
    0x0,

    /* U+0049 "I" */
    0xe, 0x65, 0x8a, 0x38, 0x58, 0x82, 0x4, 0x10,
    0x60, 0x80,

    /* U+004A "J" */
    0x3, 0x83, 0x42, 0x23, 0x21, 0x11, 0x8, 0x88,
    0x64, 0x27, 0x2, 0x6, 0x6, 0x7, 0x3, 0x1,
    0x0,

    /* U+004B "K" */
    0x9e, 0x2f, 0x26, 0xc5, 0x80, 0x60, 0x18, 0x1,
    0x0, 0x3c, 0x2, 0xe0, 0x43, 0x88, 0x8, 0x0,
    0x0,

    /* U+004C "L" */
    0x0, 0x40, 0x18, 0x5, 0x1, 0x60, 0x28, 0xb,
    0x2, 0x40, 0x40, 0x10, 0x1f, 0x83, 0xfe, 0x3,
    0x80,

    /* U+004D "M" */
    0xc, 0x1, 0xf3, 0x1f, 0x5c, 0xf1, 0xa7, 0x8a,
    0xa8, 0x37, 0x41, 0x9a, 0x4, 0xc8, 0x36, 0x40,
    0x99, 0x16, 0x44, 0x80, 0x1c,

    /* U+004E "N" */
    0x6, 0x3, 0xe3, 0x7a, 0x76, 0x4b, 0x5, 0x60,
    0xac, 0xc, 0xc0, 0x98, 0x19, 0x3, 0x32, 0x23,
    0xc0, 0x30,

    /* U+004F "O" */
    0x6, 0x1e, 0x39, 0x31, 0x61, 0xc3, 0xc2, 0x86,
    0x8c, 0x98, 0x70,

    /* U+0050 "P" */
    0x1f, 0x9f, 0xfc, 0x8c, 0x21, 0x10, 0x44, 0x26,
    0x11, 0xf8, 0x40, 0x10, 0x8, 0x2, 0x0,

    /* U+0051 "Q" */
    0x3, 0x83, 0x93, 0x84, 0xc2, 0x31, 0x80, 0xc0,
    0x40, 0x20, 0x78, 0x3f, 0xe0, 0xf0,

    /* U+0052 "R" */
    0x1f, 0xc7, 0xfd, 0xd0, 0xb2, 0x10, 0x84, 0x27,
    0x1f, 0x3, 0xc0, 0x5e, 0x8, 0xf2, 0x6, 0x0,
    0x0,

    /* U+0053 "S" */
    0x7, 0xc3, 0xf8, 0xe2, 0x10, 0x3, 0x0, 0x3c,
    0x0, 0xc0, 0x4, 0x43, 0x9f, 0xe3, 0xf0, 0x0,

    /* U+0054 "T" */
    0x3f, 0x9f, 0xee, 0xa, 0x2, 0x1, 0x0, 0xc0,
    0x20, 0x18, 0xc, 0x3, 0x0, 0x80, 0x20,

    /* U+0055 "U" */
    0xf, 0x1, 0xf8, 0x9e, 0x8c, 0xcc, 0x64, 0x46,
    0x4, 0x60, 0x66, 0x6, 0x50, 0x35, 0x11, 0xc9,
    0xc, 0x70,

    /* U+0056 "V" */
    0x7, 0x3, 0xe0, 0xe9, 0x3b, 0x24, 0x45, 0x99,
    0x2, 0x60, 0xc8, 0x12, 0x2, 0xc0, 0x70, 0x4,
    0x0,

    /* U+0057 "W" */
    0x1e, 0x1, 0xf8, 0x97, 0x26, 0x51, 0x99, 0x4,
    0xc8, 0x27, 0x21, 0xb9, 0xd, 0x64, 0x29, 0x20,
    0xc5, 0x0, 0x8, 0x0,

    /* U+0058 "X" */
    0x1c, 0x9d, 0x2c, 0x53, 0x18, 0x4, 0x3, 0x0,
    0xc0, 0x50, 0x24, 0x11, 0xcc, 0x60,

    /* U+0059 "Y" */
    0x6, 0x7, 0x83, 0xc5, 0x93, 0x49, 0xa2, 0x61,
    0x30, 0x54, 0x3a, 0xe, 0x93, 0x38, 0x1c, 0x6,
    0x1, 0x0, 0x40,

    /* U+005A "Z" */
    0xe, 0x3f, 0x71, 0xe3, 0xc6, 0x18, 0x3c, 0x63,
    0x4, 0x1c, 0x24, 0x48, 0xb0, 0xc0,

    /* U+005B "[" */
    0x1, 0xc0, 0xe0, 0x20, 0x18, 0x4, 0x3, 0x0,
    0x80, 0x40, 0x30, 0x8, 0x6, 0x1, 0x0, 0xc0,
    0x3c, 0xf, 0x0,

    /* U+005C "\\" */
    0x88, 0x44, 0x46, 0x22, 0x23, 0x11, 0x10,

    /* U+005D "]" */
    0x3, 0xc0, 0xf0, 0xc, 0x2, 0x1, 0x80, 0x40,
    0x30, 0x8, 0x4, 0x3, 0x0, 0x80, 0x60, 0x10,
    0x18, 0xe, 0x0,

    /* U+005E "^" */
    0x10, 0x30, 0xe1, 0x66, 0x48, 0x91, 0x80,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xb4,

    /* U+0061 "a" */
    0x1c, 0x71, 0xa4, 0xcb, 0x49, 0x0,

    /* U+0062 "b" */
    0x4, 0x31, 0x8a, 0x51, 0x8d, 0x24, 0x9a, 0x8c,
    0x0,

    /* U+0063 "c" */
    0x11, 0x9d, 0x88, 0x49, 0x80,

    /* U+0064 "d" */
    0x1, 0x3, 0x2, 0x2, 0x1c, 0x3c, 0x68, 0xc8,
    0x7a, 0x4c,

    /* U+0065 "e" */
    0x1b, 0x75, 0xc9, 0xb0,

    /* U+0066 "f" */
    0x8, 0x20, 0x84, 0x10, 0x82, 0x18, 0x7a, 0x49,
    0x28, 0xa3, 0x0,

    /* U+0067 "g" */
    0x18, 0xc4, 0xa6, 0xaf, 0x62, 0x18, 0xc2, 0x0,

    /* U+0068 "h" */
    0x4, 0x18, 0x20, 0xc3, 0x4, 0x13, 0x6c, 0xb1,
    0xaa, 0x60,

    /* U+0069 "i" */
    0x11, 0x26, 0x48, 0x96,

    /* U+006A "j" */
    0x0, 0x10, 0x6, 0x79, 0x41, 0x6, 0x30, 0x84,
    0x20, 0x80,

    /* U+006B "k" */
    0x4, 0x18, 0x61, 0x82, 0xce, 0x9a, 0x68, 0xf5,
    0x30,

    /* U+006C "l" */
    0x8, 0xca, 0xe6, 0x73, 0x10, 0x97, 0x0,

    /* U+006D "m" */
    0x4, 0x36, 0xb6, 0xdd, 0xcd, 0xbc, 0x90,

    /* U+006E "n" */
    0x9, 0x6f, 0xac, 0xda, 0xc0,

    /* U+006F "o" */
    0x32, 0xa5, 0x3a, 0x20,

    /* U+0070 "p" */
    0x4, 0x8, 0x1e, 0x1a, 0x74, 0x25, 0x3a, 0x7c,
    0xc0, 0x80, 0x80,

    /* U+0071 "q" */
    0xc, 0x69, 0xb4, 0xcb, 0x2b, 0x8a, 0x14, 0x50,
    0xc1, 0x0,

    /* U+0072 "r" */
    0x77, 0xa9, 0x42, 0x98,

    /* U+0073 "s" */
    0x0, 0x9d, 0x2a, 0x7d, 0x80,

    /* U+0074 "t" */
    0x0, 0xdf, 0xe4, 0x23, 0x1e, 0xc0,

    /* U+0075 "u" */
    0x25, 0xbc, 0xa6, 0xb7, 0x60,

    /* U+0076 "v" */
    0x3, 0x77, 0x2b, 0xe0,

    /* U+0077 "w" */
    0x20, 0x6a, 0xda, 0xd2, 0xb7, 0x48,

    /* U+0078 "x" */
    0x29, 0xe2, 0x8, 0x7a, 0xc0,

    /* U+0079 "y" */
    0x21, 0x9e, 0xb6, 0xb7, 0x63, 0xc, 0x61, 0x0,

    /* U+007A "z" */
    0x38, 0xf5, 0x18, 0x3c, 0x43, 0x18, 0xc2, 0x0,

    /* U+007B "{" */
    0x19, 0x8, 0x42, 0x11, 0x98, 0x61, 0x8, 0x42,
    0x10, 0x60,

    /* U+007C "|" */
    0xff, 0xfe,

    /* U+007D "}" */
    0xc1, 0x8, 0x42, 0x10, 0xc3, 0x31, 0x8, 0x42,
    0x13, 0x0,

    /* U+007E "~" */
    0x70, 0x4c, 0xa1, 0xc0,

    /* U+007F "" */
    0x0,

    /* U+4F60 "你" */
    0x6, 0x2, 0x30, 0x23, 0x3, 0x2e, 0x77, 0xe6,
    0x76, 0xce, 0x8c, 0x30, 0x61, 0xe7, 0xf3, 0x7f,
    0x12, 0x30, 0x1, 0x0,

    /* U+554A "啊" */
    0x2, 0x0, 0x8, 0x70, 0x7f, 0x8f, 0xec, 0xbf,
    0x51, 0xdf, 0xc6, 0x57, 0x11, 0x44, 0x4, 0x10,
    0x10, 0x40, 0x1, 0x0, 0x4, 0x0, 0x20,

    /* U+597D "好" */
    0x10, 0x0, 0x98, 0x9, 0x48, 0xac, 0x59, 0x8d,
    0x18, 0x71, 0xe7, 0x3f, 0xb6, 0xeb, 0x84, 0x68,
    0x42, 0x4c, 0x3, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 62, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 88, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 8, .adv_w = 77, .box_w = 3, .box_h = 4, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 10, .adv_w = 156, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 23, .adv_w = 188, .box_w = 11, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 44, .adv_w = 176, .box_w = 8, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 55, .adv_w = 125, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 56, .box_w = 1, .box_h = 4, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 67, .adv_w = 85, .box_w = 4, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 85, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 83, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 83, .adv_w = 171, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 94, .adv_w = 85, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 95, .adv_w = 131, .box_w = 5, .box_h = 1, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 96, .adv_w = 85, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 72, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 104, .adv_w = 112, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 69, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 112, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 131, .adv_w = 131, .box_w = 10, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 109, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 155, .adv_w = 109, .box_w = 10, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 107, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 112, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 115, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 117, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 85, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 214, .adv_w = 171, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 223, .adv_w = 171, .box_w = 9, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 229, .adv_w = 171, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 238, .adv_w = 83, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 259, .box_w = 14, .box_h = 15, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 274, .adv_w = 163, .box_w = 13, .box_h = 11, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 292, .adv_w = 141, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 155, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 323, .adv_w = 165, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 340, .adv_w = 157, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 357, .adv_w = 144, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 374, .adv_w = 144, .box_w = 10, .box_h = 15, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 393, .adv_w = 165, .box_w = 12, .box_h = 11, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 112, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 96, .box_w = 9, .box_h = 15, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 437, .adv_w = 168, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 454, .adv_w = 152, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 471, .adv_w = 213, .box_w = 14, .box_h = 12, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 492, .adv_w = 147, .box_w = 12, .box_h = 12, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 510, .adv_w = 147, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 160, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 536, .adv_w = 165, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 550, .adv_w = 168, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 567, .adv_w = 165, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 583, .adv_w = 147, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 598, .adv_w = 160, .box_w = 13, .box_h = 11, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 109, .box_w = 11, .box_h = 12, .ofs_x = -2, .ofs_y = -1},
    {.bitmap_index = 633, .adv_w = 179, .box_w = 14, .box_h = 11, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 653, .adv_w = 155, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 667, .adv_w = 141, .box_w = 10, .box_h = 15, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 686, .adv_w = 139, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 700, .adv_w = 75, .box_w = 10, .box_h = 15, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 719, .adv_w = 72, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 726, .adv_w = 93, .box_w = 10, .box_h = 15, .ofs_x = -3, .ofs_y = -4},
    {.bitmap_index = 745, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 752, .adv_w = 128, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 753, .adv_w = 85, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 754, .adv_w = 104, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 760, .adv_w = 83, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 769, .adv_w = 75, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 774, .adv_w = 107, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 784, .adv_w = 75, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 788, .adv_w = 69, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 799, .adv_w = 99, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 807, .adv_w = 93, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 817, .adv_w = 59, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 821, .adv_w = 72, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 831, .adv_w = 93, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 840, .adv_w = 59, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 847, .adv_w = 141, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 854, .adv_w = 93, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 859, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 863, .adv_w = 104, .box_w = 8, .box_h = 11, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 874, .adv_w = 96, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -5},
    {.bitmap_index = 884, .adv_w = 75, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 888, .adv_w = 80, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 893, .adv_w = 64, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 899, .adv_w = 99, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 904, .adv_w = 83, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 908, .adv_w = 125, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 914, .adv_w = 75, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 927, .adv_w = 77, .box_w = 6, .box_h = 10, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 935, .adv_w = 107, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 945, .adv_w = 139, .box_w = 1, .box_h = 15, .ofs_x = 4, .ofs_y = -4},
    {.bitmap_index = 947, .adv_w = 107, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 957, .adv_w = 171, .box_w = 9, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 961, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 962, .adv_w = 256, .box_w = 12, .box_h = 13, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 982, .adv_w = 256, .box_w = 14, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1005, .adv_w = 256, .box_w = 12, .box_h = 13, .ofs_x = 2, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x5ea, 0xa1d
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 20320, .range_length = 2590, .glyph_id_start = 97,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
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
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_Font4 = {
#else
lv_font_t ui_font_Font4 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 5,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_FONT4*/


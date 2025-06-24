#ifndef FONT_8X8_H
#define FONT_8X8_H

#include <stdint.h>

#ifdef BOARD_HAS_SSD1306

// 8x8像素字体数据 (ASCII 32-126)
extern const uint8_t font_8x8[95][8];

#endif // BOARD_HAS_SSD1306

#endif // FONT_8X8_H

#ifdef BOARD_HAS_SSD1306

#include "ssd1306_drv.h"
#include "ssd1306_comm.h"
#include "font_8x8.h"
#include "nrf_delay.h"
#include <string.h>
#include <stdlib.h> 

static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static bool display_enabled = false;

// fix some wrong cmd not good with this screen (commited by Dylan)
bool ssd1306_init(void) {
    if (!ssd1306_comm_init()) {
        display_enabled = false;
        return false;
    }
    
    display_enabled = true;

    ssd1306_write_command(0xAE);
    ssd1306_write_command(0xD5); 
    ssd1306_write_command(0xF0);
    ssd1306_write_command(0xA8);
    ssd1306_write_command(0x3F);
    ssd1306_write_command(0xD3);
    ssd1306_write_command(0x00);
    ssd1306_write_command(0x40 | 0x0);
    ssd1306_write_command(0x8D);
    ssd1306_write_command(0x14);
    ssd1306_write_command(0x20);
    ssd1306_write_command(0x00);
    ssd1306_write_command(0xA1);
    ssd1306_write_command(0xC8);
    ssd1306_write_command(0xDA);
    ssd1306_write_command(0x12);
    ssd1306_write_command(0x81);
    ssd1306_write_command(0xCF); 
    ssd1306_write_command(0xD9);
    ssd1306_write_command(0xF1);
    ssd1306_write_command(0xDB);
    ssd1306_write_command(0x40);
    ssd1306_write_command(0xA4);
    ssd1306_write_command(0xA6);
    ssd1306_write_command(0x2E);

    ssd1306_clear();
    ssd1306_display();
    ssd1306_write_command(0xAF); 
    return true;
}

void ssd1306_uninit(void) {
    if (display_enabled) {
        ssd1306_write_command(0xAE); // Display off
        ssd1306_comm_uninit();
        display_enabled = false;
    }
}

void ssd1306_clear(void) {
    if (!display_enabled) return;
    
    memset(ssd1306_buffer, 0, sizeof(ssd1306_buffer));
    ssd1306_display();
}

void ssd1306_display(void) {
    if (!display_enabled) return;

    for (uint8_t i = 0; i < 8; i++) {
        ssd1306_write_command(0xB0 + i);

        ssd1306_write_command(0x00);  // maybe 0x02
        ssd1306_write_command(0x10);
        ssd1306_write_data(&ssd1306_buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
    }
}

void ssd1306_set_pixel(uint8_t x, uint8_t y, bool on) {
    if (!display_enabled || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    uint16_t index = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit = y % 8;

    if (on) {
        ssd1306_buffer[index] |= (1 << bit);
    } else {
        ssd1306_buffer[index] &= ~(1 << bit);
    }
}

// add some test code which can easily removed (commited by dylan)
void ssd1306_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;) {
        ssd1306_set_pixel(x0, y0, true);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill) {
    if (!display_enabled || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if (x + width > SSD1306_WIDTH) width = SSD1306_WIDTH - x;
    if (y + height > SSD1306_HEIGHT) height = SSD1306_HEIGHT - y;

    if (fill) {
        for (uint8_t i = x; i < x + width; i++) {
            for (uint8_t j = y; j < y + height; j++) {
                ssd1306_set_pixel(i, j, true);
            }
        }
    } else {
        ssd1306_draw_line(x, y, x + width - 1, y);
        ssd1306_draw_line(x, y, x, y + height - 1);
        ssd1306_draw_line(x + width - 1, y, x + width - 1, y + height - 1);
        ssd1306_draw_line(x, y + height - 1, x + width - 1, y + height - 1);
    }
}

void ssd1306_draw_char(uint8_t x, uint8_t y, char c) {
    if (!display_enabled || c < 32 || c > 126) return;
    if (x + 8 > SSD1306_WIDTH || y + 8 > SSD1306_HEIGHT) return;
    
    const uint8_t* char_data = font_8x8[c - 32];
    
    // 正确的字体绘制：横向排列，高位在左
    for (int row = 0; row < 8; row++) {
        uint8_t byte = char_data[row];
        for (int col = 0; col < 8; col++) {
            if (byte & (0x80 >> col)) {  // 从高位开始，0x80右移col位
                ssd1306_set_pixel(x + col, y + row, true);
            }
        }
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char* str) {
    if (!display_enabled || !str) return;
    
    uint8_t cur_x = x;
    while (*str && cur_x + 8 <= SSD1306_WIDTH) {
        ssd1306_draw_char(cur_x, y, *str);
        cur_x += 8;
        str++;
    }
}

void ssd1306_draw_string_centered(uint8_t y, const char* str) {
    if (!display_enabled || !str) return;
    
    uint8_t len = strlen(str);
    if (len == 0) return;
    
    uint8_t total_width = len * 8;
    if (total_width > SSD1306_WIDTH) {
        ssd1306_draw_string(0, y, str);
    } else {
        uint8_t x = (SSD1306_WIDTH - total_width) / 2;
        ssd1306_draw_string(x, y, str);
    }
}

bool ssd1306_is_enabled(void) {
    return display_enabled;
}

#endif // BOARD_HAS_SSD1306

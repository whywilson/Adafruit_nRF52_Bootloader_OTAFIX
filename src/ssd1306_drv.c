#ifdef BOARD_HAS_SSD1306

#include "ssd1306_drv.h"
#include "ssd1306_comm.h"
#include "font_8x8.h"
#include "nrf_delay.h"
#include <string.h>

static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static bool display_enabled = false;

bool ssd1306_init(void) {
    // 初始化I2C通信
    if (!ssd1306_comm_init()) {
        display_enabled = false;
        return false;
    }
    
    // SSD1306初始化序列
    ssd1306_write_command(0xAE); // Display off
    ssd1306_write_command(0xD5); // Set display clock divide ratio
    ssd1306_write_command(0x80); // Default ratio
    ssd1306_write_command(0xA8); // Set multiplex ratio
    ssd1306_write_command(0x3F); // 64 lines
    ssd1306_write_command(0xD3); // Set display offset
    ssd1306_write_command(0x00); // No offset
    ssd1306_write_command(0x40); // Set start line address
    ssd1306_write_command(0x8D); // Charge pump setting
    ssd1306_write_command(0x14); // Enable charge pump
    ssd1306_write_command(0x20); // Memory addressing mode
    ssd1306_write_command(0x00); // Horizontal addressing mode
    ssd1306_write_command(0xA1); // Set segment remap (A0/A1)
    ssd1306_write_command(0xC8); // Set COM output scan direction
    ssd1306_write_command(0xDA); // Set COM pins hardware configuration
    ssd1306_write_command(0x12); // Alternative COM pin config
    ssd1306_write_command(0x81); // Set contrast control
    ssd1306_write_command(0xCF); // High contrast
    ssd1306_write_command(0xD9); // Set pre-charge period
    ssd1306_write_command(0xF1); // Phase 1: 1 DCLK, Phase 2: 15 DCLK
    ssd1306_write_command(0xDB); // Set VCOMH deselect level
    ssd1306_write_command(0x40); // 0.77 * VCC
    ssd1306_write_command(0xA4); // Entire display ON (resume to RAM content)
    ssd1306_write_command(0xA6); // Set normal display (not inverted)
    
    // 清屏
    ssd1306_clear();
    
    // 开启显示
    ssd1306_write_command(0xAF); // Display on
    
    display_enabled = true;
    
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
    
    // 设置列地址范围
    ssd1306_write_command(0x21); // Column address
    ssd1306_write_command(0x00); // Start column
    ssd1306_write_command(0x7F); // End column (127)
    
    // 设置页地址范围
    ssd1306_write_command(0x22); // Page address
    ssd1306_write_command(0x00); // Start page
    ssd1306_write_command(0x07); // End page (7)
    
    // 发送显示数据
    ssd1306_write_data(ssd1306_buffer, sizeof(ssd1306_buffer));
}

void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill) {
    if (!display_enabled || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    
    // 确保矩形在屏幕范围内
    if (x + width > SSD1306_WIDTH) width = SSD1306_WIDTH - x;
    if (y + height > SSD1306_HEIGHT) height = SSD1306_HEIGHT - y;
    
    for (uint8_t i = 0; i < width; i++) {
        for (uint8_t j = 0; j < height; j++) {
            ssd1306_set_pixel(x + i, y + j, fill);
        }
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
        // 字符串太长，从左边开始显示
        ssd1306_draw_string(0, y, str);
    } else {
        // 居中显示
        uint8_t x = (SSD1306_WIDTH - total_width) / 2;
        ssd1306_draw_string(x, y, str);
    }
}

bool ssd1306_is_enabled(void) {
    return display_enabled;
}

#endif // BOARD_HAS_SSD1306

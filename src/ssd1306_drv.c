/*
 * @brief: 已修正的SSD1306驱动逻辑。
 * @author: Dylan 
 * @details : 记得把注释删了
 * 主要变更:
 * - 在 ssd1306_display 函数中:重写了此函数，使其采用"页寻址模式”
 * - 逻辑进行屏幕刷新。放弃了水平寻址模式下的整块数据发送方式。
 */
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

    // --- SSD1306 标准初始化序列 ---
    ssd1306_write_command(0xAE); // 关闭显示

    ssd1306_write_command(0xD5); // 设置显示时钟分频比/振荡器频率
    ssd1306_write_command(0xF0); // 设置分频比

    ssd1306_write_command(0xA8); // 设置多路复用率
    ssd1306_write_command(0x3F); // 128x64屏幕对应64 MUX

    ssd1306_write_command(0xD3); // 设置显示偏移
    ssd1306_write_command(0x00); // 无偏移

    ssd1306_write_command(0x40 | 0x0); // 设置显示起始行 (0)

    ssd1306_write_command(0x8D); // 电荷泵设置
    ssd1306_write_command(0x14); // 使能电荷泵

    ssd1306_write_command(0x20); // 设置内存寻址模式
    ssd1306_write_command(0x00); // 水平寻址模式

    ssd1306_write_command(0xA0); // 设置段重映射为正常模式
    
    ssd1306_write_command(0xC0); // 设置COM输出扫描方向为正向 (C0)

    ssd1306_write_command(0xDA); // 设置COM引脚硬件配置
    ssd1306_write_command(0x12); // 适用于128x64的交替COM引脚配置

    ssd1306_write_command(0x81); // 设置对比度控制
    ssd1306_write_command(0xCF); // 设置对比度值

    ssd1306_write_command(0xD9); // 设置预充电周期
    ssd1306_write_command(0xF1); // 设置预充电值

    ssd1306_write_command(0xDB); // 设置VCOMH反压电平
    ssd1306_write_command(0x40); // VCOMH = ~0.77xVCC

    ssd1306_write_command(0xA4); // 全局显示开启 (恢复到RAM内容)
    ssd1306_write_command(0xA6); // 设置为正常显示 (非反色)
    ssd1306_write_command(0x2E); // 停用滚动

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

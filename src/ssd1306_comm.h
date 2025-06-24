#ifndef SSD1306_COMM_H
#define SSD1306_COMM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef BOARD_HAS_SSD1306

// I2C通信初始化和清理
bool ssd1306_comm_init(void);
void ssd1306_comm_uninit(void);

// SSD1306通信函数
void ssd1306_write_command(uint8_t cmd);
void ssd1306_write_data(const uint8_t* data, uint16_t len);

#endif // BOARD_HAS_SSD1306

#endif // SSD1306_COMM_H

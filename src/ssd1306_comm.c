#ifdef BOARD_HAS_SSD1306

#include "ssd1306_comm.h"
#include "boards.h"
#include "nrfx_twi.h"
#include "nrf_delay.h"
#include <string.h>

#define SSD1306_I2C_ADDR 0x3C

static const nrfx_twi_t twi = NRFX_TWI_INSTANCE(0);
static bool twi_initialized = false;

bool ssd1306_comm_init(void) {
    if (twi_initialized) return true;
    
    nrfx_twi_config_t twi_config = NRFX_TWI_DEFAULT_CONFIG(I2C_SCL_PIN, I2C_SDA_PIN);
    twi_config.frequency = NRF_TWI_FREQ_100K;
    
    nrfx_err_t err = nrfx_twi_init(&twi, &twi_config, NULL, NULL);
    if (err != NRFX_SUCCESS) {
        return false;
    }
    
    nrfx_twi_enable(&twi);
    twi_initialized = true;
    
    // 延时让设备稳定
    nrf_delay_ms(50);
    
    // 测试I2C通信
    uint8_t test_data[2] = {0x00, 0xAE}; // Display off command
    nrfx_twi_xfer_desc_t xfer = NRFX_TWI_XFER_DESC_TX(SSD1306_I2C_ADDR, test_data, 2);
    err = nrfx_twi_xfer(&twi, &xfer, 0);
    
    if (err != NRFX_SUCCESS) {
        // 尝试备用地址
        xfer.address = 0x3D;
        err = nrfx_twi_xfer(&twi, &xfer, 0);
    }
    
    return (err == NRFX_SUCCESS);
}

void ssd1306_comm_uninit(void) {
    if (twi_initialized) {
        nrfx_twi_disable(&twi);
        nrfx_twi_uninit(&twi);
        twi_initialized = false;
    }
}

void ssd1306_write_command(uint8_t cmd) {
    if (!twi_initialized) return;
    
    uint8_t data[2] = {0x00, cmd}; // Control byte (Co=0, D/C=0) + command
    nrfx_twi_xfer_desc_t xfer = NRFX_TWI_XFER_DESC_TX(SSD1306_I2C_ADDR, data, 2);
    nrfx_twi_xfer(&twi, &xfer, 0);
    
    // 短延时确保命令执行
    nrf_delay_ms(1);
}

void ssd1306_write_data(const uint8_t* data, uint16_t len) {
    if (!twi_initialized || !data || len == 0) return;
    
    // 分批发送数据，每次最多31字节（1字节控制+30字节数据）
    const uint16_t chunk_size = 30;
    uint16_t sent = 0;
    
    while (sent < len) {
        uint16_t to_send = (len - sent > chunk_size) ? chunk_size : (len - sent);
        uint8_t buffer[31]; // 1字节控制 + 30字节数据
        
        buffer[0] = 0x40; // Data control byte (Co=0, D/C=1)
        memcpy(&buffer[1], &data[sent], to_send);
        
        nrfx_twi_xfer_desc_t xfer = NRFX_TWI_XFER_DESC_TX(SSD1306_I2C_ADDR, buffer, to_send + 1);
        nrfx_twi_xfer(&twi, &xfer, 0);
        
        sent += to_send;
        
        // 小延时避免I2C总线过载
        if (sent < len) {
            nrf_delay_ms(1);
        }
    }
}

#endif // BOARD_HAS_SSD1306

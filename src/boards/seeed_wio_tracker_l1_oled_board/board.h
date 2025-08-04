#ifndef _SEEED_WIO_TRACKER_L1_OLED_BOARD_H
#define _SEEED_WIO_TRACKER_L1_OLED_BOARD_H

#define _PINNUM(port, pin)    ((port)*32 + (pin))

/*------------------------------------------------------------------*/
/* LED
 *------------------------------------------------------------------*/
#define LEDS_NUMBER           2
#define LED_PRIMARY_PIN       _PINNUM(1, 15)  // P1.15 - RGB LED Red
#define LED_SECONDARY_PIN     _PINNUM(1, 16)  // P1.16 - RGB LED Green  
#define LED_STATE_ON          1               // Active HIGH

/*------------------------------------------------------------------*/
/* BUTTON
 *------------------------------------------------------------------*/
#define BUTTONS_NUMBER        2
#define BUTTON_1              _PINNUM(0, 8)   // P0.08 - User Button
#define BUTTON_2              _PINNUM(0, 28)  // P0.28 - Boot Button
#define BUTTON_PULL           NRF_GPIO_PIN_PULLUP

//--------------------------------------------------------------------+
// I2C for SSD1306 OLED Display
//--------------------------------------------------------------------+
#define I2C_SCL_PIN           _PINNUM(0, 5)   // P0.05 - I2C SCL
#define I2C_SDA_PIN           _PINNUM(0, 6)   // P0.06 - I2C SDA
#define SSD1306_I2C_ADDR      0x3D            // SSD1306 OLED Display I2C Address

// SSD1306 display configuration
#define SSD1306_COM_PINS      0x02            // Sequential COM pin configuration
#define SSD1306_SEGMENT_REMAP 0xA0            // No segment remap 
#define SSD1306_COM_SCAN_DIR  0xC0            // Normal COM scan direction

//--------------------------------------------------------------------+
// BLE OTA
//--------------------------------------------------------------------+
#define BLEDIS_MANUFACTURER   "Seeed"
#define BLEDIS_MODEL          "Wio Tracker L1"

//--------------------------------------------------------------------+
// USB
//--------------------------------------------------------------------+
#define USB_DESC_VID           0x2886
#define USB_DESC_UF2_PID       0x0044
#define USB_DESC_CDC_ONLY_PID  0x0044

//------------- UF2 -------------//
#define UF2_PRODUCT_NAME      "Seeed TRACKER-L1 001"
#define UF2_VOLUME_LABEL      "TRACKER-L1"
#define UF2_BOARD_ID          "TRACKER-L1"
#define UF2_INDEX_URL         "https://www.seeedstudio.com/"

#define BRAND_NAME "Seeed-Studio"

#endif // _SEEED_WIO_TRACKER_L1_OLED_BOARD_H

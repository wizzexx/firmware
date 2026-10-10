#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define USB_VID 0x303a
#define USB_PID 0x1001

// Аппаратный UART для ESP32-S3
static const uint8_t TX = 43;
static const uint8_t RX = 44;

// I2C
static const uint8_t SDA = 2;
static const uint8_t SCL = 1;

// Аппаратный SPI под твой экран и тачскрин
static const uint8_t SS   = 10;
static const uint8_t MOSI = 13;
static const uint8_t MISO = 14;
static const uint8_t SCK  = 12;

// АЦП пины для ESP32-S3
static const uint8_t A0 = 1;
static const uint8_t A1 = 2;
static const uint8_t A2 = 3;
static const uint8_t A3 = 4;
static const uint8_t A4 = 5;
static const uint8_t A5 = 6;
static const uint8_t A6 = 7;
static const uint8_t A7 = 8;
static const uint8_t A8 = 9;
static const uint8_t A9 = 10;

// Grove / периферия
#define GROVE_SDA 2
#define GROVE_SCL 1

// Инфракрасный порт и радиомодули
#define RXLED 1
#define TXLED 2

// Выход из глубокого сна по кнопке Boot (GPIO 0)
#define DEEPSLEEP_WAKEUP_PIN 0
#define DEEPSLEEP_PIN_ACT LOW

#endif /* Pins_Arduino_h */

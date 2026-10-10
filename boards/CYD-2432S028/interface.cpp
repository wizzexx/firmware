#include "core/bus_HAL.h"
#include "core/powerSave.h"
#include "core/utils.h"
#include <Arduino.h>
#include <interface.h>

#if defined(HAS_CAPACITIVE_TOUCH)
#if defined(TOUCH_GT911_I2C)
#include "TouchDrvGT911.hpp"
TouchDrvGT911 touch;
struct TouchPointPro {
    int16_t x = 0;
    int16_t y = 0;
};
#else
#include "CYD28_TouchscreenC.h"
#define CYD28_DISPLAY_HOR_RES_MAX 240
#define CYD28_DISPLAY_VER_RES_MAX 320
CYD28_TouchC touch(CYD28_DISPLAY_HOR_RES_MAX, CYD28_DISPLAY_VER_RES_MAX);
#endif
#elif defined(USE_TFT_eSPI_TOUCH)
#define XPT2046_CS TOUCH_CS
#else
#include "CYD28_TouchscreenR.h"
#define CYD28_DISPLAY_HOR_RES_MAX 320
#define CYD28_DISPLAY_VER_RES_MAX 240
CYD28_TouchR touch(CYD28_DISPLAY_HOR_RES_MAX, CYD28_DISPLAY_VER_RES_MAX);
#if defined(TOUCH_XPT2046_SPI)
#define XPT2046_CS XPT2046_SPI_CONFIG_CS_GPIO_NUM
#else
#define XPT2046_CS 33
#endif
#endif

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
SPIClass touchSPI;
void _setup_gpio() {
#ifndef HAS_CAPACITIVE_TOUCH
    #if defined(XPT2046_CS) && (XPT2046_CS >= 0)
    pinMode(XPT2046_CS, OUTPUT);
    digitalWrite(XPT2046_CS, HIGH);
    #endif
#endif

#if defined(HAS_CAPACITIVE_TOUCH)
    setSysI2CBus(&Wire1);
#if defined(TOUCH_GT911_I2C)
    bruceConfigPins.sys_i2c.sda = (gpio_num_t)SYS_I2C_SDA;
    bruceConfigPins.sys_i2c.scl = (gpio_num_t)SYS_I2C_SCL;
#else
    bruceConfigPins.sys_i2c.sda = (gpio_num_t)CYD28_TouchC_SDA;
    bruceConfigPins.sys_i2c.scl = (gpio_num_t)CYD28_TouchC_SCL;
#endif
#endif

#if defined(TOUCH_GT911_I2C)
    pinMode(BOARD_TOUCH_INT, INPUT);
    touch.setPins(-1, BOARD_TOUCH_INT);
    if (!touch.begin(Wire1, GT911_SLAVE_ADDRESS_L, SYS_I2C_SDA, SYS_I2C_SCL)) {
        Serial.println("Failed to find GT911 - check your wiring!");
    }
#else
#if !defined(USE_TFT_eSPI_TOUCH)
    if (!touch.begin()) {
        Serial.println("Touch IC not Started");
        log_i("Touch IC not Started");
    } else log_i("Touch IC Started");
#endif
#endif

    bruceConfig.colorInverted = 0;
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup
***************************************************************************************/
void _post_setup_gpio() {
#if defined(USE_TFT_eSPI_TOUCH)
    #if defined(TOUCH_CS) && (TOUCH_CS >= 0)
    pinMode(TOUCH_CS, OUTPUT);
    #endif
    // Базовая калибровка XPT2046 для дисплея 2.8" ILI9341
    uint16_t calData[5] = { 275, 3494, 361, 3528, 4 };
    tft.setTouch(calData);
#endif

#if defined(TFT_BL) && (TFT_BL >= 0)
    pinMode(TFT_BL, OUTPUT);
    ledcAttach(TFT_BL, TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits);
    ledcWrite(TFT_BL, 255);
#endif

    // Принудительное отключение инверсии для правильных цветов (чёрный фон)
    bruceConfig.colorInverted = 0;
    tft.invertDisplay(0);

    bruceConfigPins.gps_bus.rx = (gpio_num_t)GPS_SERIAL_RX;
    bruceConfigPins.gps_bus.tx = (gpio_num_t)GPS_SERIAL_TX;
    bruceConfigPins.gpsBaudrate = 9600;

    bruceConfigPins.rfTx = 2;
    bruceConfigPins.rfRx = 1;
    bruceConfigPins.irTx = 2;
    bruceConfigPins.irRx = 1;
}

/*********************************************************************
** Function: setBrightness
**********************************************************************/
void _setBrightness(uint8_t brightval) {
#if defined(TFT_BL) && (TFT_BL >= 0)
    int dutyCycle;
    if (brightval == 100) dutyCycle = 255;
    else if (brightval == 75) dutyCycle = 130;
    else if (brightval == 50) dutyCycle = 70;
    else if (brightval == 25) dutyCycle = 20;
    else if (brightval == 0) dutyCycle = 0;
    else dutyCycle = ((brightval * 255) / 100);

    ledcWrite(TFT_BL, dutyCycle);
#endif
}

/*********************************************************************
** Function: InputHandler
**********************************************************************/
void InputHandler(void) {
    static long d_tmp = 0;
    if (millis() - d_tmp > 200 || LongPress) {
#if defined(USE_TFT_eSPI_TOUCH)
        TouchPoint t;
        checkPowerSaveTime();
        uint16_t tx = 0, ty = 0;
        bool _IH_touched = tft.getTouch(&tx, &ty);
        if (_IH_touched) {
            t.x = tx;
            t.y = ty;
            NextPress = false;
            PrevPress = false;
            UpPress = false;
            DownPress = false;
            SelPress = false;
            EscPress = false;
            AnyKeyPress = false;
            NextPagePress = false;
            PrevPagePress = false;
            touchPoint.pressed = false;
            _IH_touched = false;
#elif defined(TOUCH_GT911_I2C)
        static unsigned long tm = millis();
        TouchPointPro t;
        uint8_t touched = 0;
        uint8_t rot = 5;

        if (rot != bruceConfigPins.rotation) {
            if (bruceConfigPins.rotation == 1) {
                touch.setMaxCoordinates(TFT_HEIGHT, TFT_WIDTH);
                touch.setSwapXY(true);
                touch.setMirrorXY(false, true);
            }
            if (bruceConfigPins.rotation == 3) {
                touch.setMaxCoordinates(TFT_HEIGHT, TFT_WIDTH);
                touch.setSwapXY(true);
                touch.setMirrorXY(true, false);
            }
            if (bruceConfigPins.rotation == 0) {
                touch.setMaxCoordinates(TFT_WIDTH, TFT_HEIGHT);
                touch.setSwapXY(false);
                touch.setMirrorXY(false, false);
            }
            if (bruceConfigPins.rotation == 2) {
                touch.setMaxCoordinates(TFT_WIDTH, TFT_HEIGHT);
                touch.setSwapXY(false);
                touch.setMirrorXY(true, true);
            }
            rot = bruceConfigPins.rotation;
        }

        static bool lastTouchState = false;
        static unsigned long lastTouchTime = 0;

        touched = touch.getPoint(&t.x, &t.y);
        bool currentTouchState = touched > 0;

        if (currentTouchState && !lastTouchState && (millis() - lastTouchTime) > 100) {
            lastTouchTime = millis();
        } else if (!currentTouchState || lastTouchState) {
            touched = 0;
        }
        lastTouchState = currentTouchState;
        if (((millis() - tm) > 190 || LongPress) && touched) {
            tm = millis();
#else
        if (touch.touched()) {
            auto t = touch.getPointScaled();
#endif
#if !defined(TOUCH_GT911_I2C)
            if (bruceConfigPins.rotation == 3) {
                t.y = (tftHeight + 20) - t.y;
                t.x = tftWidth - t.x;
            }
            if (bruceConfigPins.rotation == 0) {
                int tmp = t.x;
                t.x = tftWidth - t.y;
                t.y = tmp;
            }
            if (bruceConfigPins.rotation == 2) {
                int tmp = t.x;
                t.x = t.y;
                t.y = (tftHeight + 20) - tmp;
            }
#endif

            if (!wakeUpScreen()) AnyKeyPress = true;
            else goto END;

            touchPoint.x = t.x;
            touchPoint.y = t.y;
            touchPoint.pressed = true;
            touchHeatMap(touchPoint);
        END:
            d_tmp = millis();
        }
    }
}

void powerOff() {
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
    esp_deep_sleep_start();
}

void checkReboot() {}

#pragma once

#include "OLEDDisplay.h"
#include "driver/gpio.h"

/** Driver for 128x64 OLED display on heltec board, based on ESP-IDF library */
class SSD1306 : public OLEDDisplay {
public:
    SSD1306(gpio_num_t sda_gpio, gpio_num_t scl_gpio, gpio_num_t reset_gpio);

    virtual bool connect();

    virtual void display();

private:
    virtual void sendCommand(uint8_t command_id);
    gpio_num_t _sda_gpio;
    gpio_num_t _scl_gpio;
    gpio_num_t _reset_gpio;
};

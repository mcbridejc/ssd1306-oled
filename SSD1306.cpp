#include "SSD1306.h"

#include <algorithm>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"

using std::min;
using std::max;

// This can be different, depending on how the address select pin is wired
// 0x3C is the address for the heltec board, but the IDF i2c excludes the R/W
// bit, so the argument must be shifted by 1
#define SSD1306_I2C_ADDR (0x3c << 1)

SSD1306::SSD1306(gpio_num_t sda_gpio, gpio_num_t scl_gpio, gpio_num_t reset_gpio) : OLEDDisplay() {
    _sda_gpio = sda_gpio;
    _scl_gpio = scl_gpio;
    _reset_gpio = reset_gpio;
}

bool SSD1306::connect() {
    // Configure I2C
    i2c_config_t cfg;

    printf("Connecting to display\n");
    cfg.mode = I2C_MODE_MASTER;
    cfg.sda_io_num = _sda_gpio;
    cfg.sda_pullup_en = GPIO_PULLUP_DISABLE;
    cfg.scl_io_num = _scl_gpio;
    cfg.scl_pullup_en = GPIO_PULLUP_DISABLE;
    cfg.master.clk_speed = 400000;

    ESP_ERROR_CHECK( i2c_param_config(I2C_NUM_0, &cfg) );
    ESP_ERROR_CHECK( i2c_driver_install(
        I2C_NUM_0,
        I2C_MODE_MASTER,
        16, // Rx buffer length
        16, // Tx buffer length
        ESP_INTR_FLAG_IRAM
    ));

    // Reset display controller
    ESP_ERROR_CHECK( gpio_set_direction(_reset_gpio, GPIO_MODE_OUTPUT) );
    ESP_ERROR_CHECK( gpio_set_level(_reset_gpio, 0));
    vTaskDelay(1);
    ESP_ERROR_CHECK( gpio_set_level(_reset_gpio, 1));
    return true;
}

void SSD1306::sendCommand(uint8_t command_id) {
    const TickType_t ticks_to_wait = 10;
    i2c_cmd_handle_t handle;
    handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, SSD1306_I2C_ADDR, true);
    i2c_master_write_byte(handle, 0x80, true);
    i2c_master_write_byte(handle, command_id, true);
    i2c_master_stop(handle);
    ESP_ERROR_CHECK( i2c_master_cmd_begin(I2C_NUM_0, handle, ticks_to_wait) );
    i2c_cmd_link_delete(handle);
}

void SSD1306::display() {
    const int x_offset = (128 - this->width()) / 2;
    const TickType_t ticks_to_wait = 10;
    i2c_cmd_handle_t handle;

#ifdef OLEDDISPLAY_DOUBLE_BUFFER
    uint8_t minBoundY = UINT8_MAX;
    uint8_t maxBoundY = 0;

    uint8_t minBoundX = UINT8_MAX;
    uint8_t maxBoundX = 0;
    uint8_t x, y;


    // Calculate the Y bounding box of changes
    // and copy buffer[pos] to buffer_back[pos];
    for (y = 0; y < (this->height() / 8); y++) {
        for (x = 0; x < this->width(); x++) {
        uint16_t pos = x + y * this->width();
        if (buffer[pos] != buffer_back[pos]) {
            minBoundY = min(minBoundY, y);
            maxBoundY = max(maxBoundY, y);
            minBoundX = min(minBoundX, x);
            maxBoundX = max(maxBoundX, x);
        }
        buffer_back[pos] = buffer[pos];
        }
    }

    // If the minBoundY wasn't updated
    // we can savely assume that buffer_back[pos] == buffer[pos]
    // holdes true for all values of pos

    if (minBoundY == UINT8_MAX) return;

    sendCommand(COLUMNADDR);
    sendCommand(x_offset + minBoundX);
    sendCommand(x_offset + maxBoundX);

    sendCommand(PAGEADDR);
    sendCommand(minBoundY);
    sendCommand(maxBoundY);

    uint8_t k = 0;
    for (y = minBoundY; y <= maxBoundY; y++) {
        for (x = minBoundX; x <= maxBoundX; x++) {
        if (k == 0) {
            handle = i2c_cmd_link_create();
            i2c_master_start(handle);
            i2c_master_write_byte(handle, SSD1306_I2C_ADDR, true);
            i2c_master_write_byte(handle, 0x40, true);
        }

        i2c_master_write_byte(handle, buffer[x + y * this->width()], true);
        k++;
        if (k == 16)  {
            i2c_master_stop(handle);
            i2c_master_cmd_begin(I2C_NUM_0, handle, ticks_to_wait);
            free(handle);
            k = 0;
        }
        }
    }

    if (k != 0) {
        i2c_master_stop(handle);
        i2c_master_cmd_begin(I2C_NUM_0, handle, ticks_to_wait);
        free(handle);
    }
    
#else

    sendCommand(COLUMNADDR);
    sendCommand(x_offset);
    sendCommand(x_offset + (this->width() - 1));
    printf("Printing to display width %d height %d\n", this->width(), this->height());

    sendCommand(PAGEADDR);
    sendCommand(0x0);
    sendCommand((this->height() / 8) - 1);


    if (geometry == GEOMETRY_128_64) {
        sendCommand(0x7);
    } else if (geometry == GEOMETRY_128_32) {
        sendCommand(0x3);
    }

    for (uint16_t i=0; i < displayBufferSize; i++) {
        i2c_master_start(handle);
        i2c_master_write_byte(handle, SSD1306_I2C_ADDR, true);
        i2c_master_write_byte(handle, 0x40, true);
        for (uint8_t x = 0; x < 16; x++) {
            i2c_master_write_byte(handle, buffer[i], true);
            i++;
        }
        i--;
        i2c_master_stop(handle);
        i2c_master_cmd_begin(I2C_NUM_0, handle, ticks_to_wait);
    }
#endif
}
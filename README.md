# ssd1306-oled

A C++ ESP-IDK component for controlling a SSD1306 OLED display. This is based on the [arduino driver provided by Heltec](https://github.com/HelTecAutomation/Heltec_ESP32), which appears to be based on drivers created by [Thing Pulse](https://thingpulse.com), whose products you should check out, if for no other reason, because they appear to have done most of the heavy lifting for this display driver and released it open source.

## API Documentation

See [this thing pulse github page](https://github.com/ThingPulse/esp8266-oled-ssd1306) for better documenation of the API. THis driver is essentially the same, just adapted to work with the ESP-IDK libraries instead of Arduino.

## Example

A simple example:

```C++
#include "SSD1306.h"

extern "C" {

void app_main()
{
    SSD1306 display(GPIO_NUM_4, GPIO_NUM_15, GPIO_NUM_16);
    display.init();
    display.drawString(0, 0, "Hi Universe");
}
```

## Project Setup

I add this to my projects as a git submodule in the `components` directory. However, you may also simply create a subdirectory in your project and simply copy the contents of this repo there. 

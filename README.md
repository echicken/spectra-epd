# spectra-epd
Arduino library for using Pervasive Displays Spectra 4.2" E2417ES053

### ESP-32 example

```c++
#include <Arduino.h>
#include "Spectra.h"

#define PIN_SCL 14
#define PIN_SDA 16
#define PIN_CS 12
#define PIN_DC 13
#define PIN_RESET 4
#define PIN_BUSY 5
#define PIN_BS 21

Spectra spectra(PIN_SCL, PIN_SDA, PIN_CS, PIN_DC, PIN_RESET, PIN_BUSY, PIN_BS);

void stripes() {
    for (int n = 0; n < 15000; n++) {
        spectra.buffer[n] = ((n + 1) % 2 == 0 ? 255 : 0);
    }
    for (int n = 15000; n < 30000; n++) {
        spectra.buffer[n] = (n % 2 == 0 ? 255 : 0);
    }
}

void draw(void* pvParameters) {
    spectra.draw();
    vTaskDelete(NULL); // self
}

void setup() {
    spectra_init();
    stripes();
}

void loop() {
    // If WiFi and BLE tasks run on core 0, we should run our task on core 1
    // I am not sure how this will perform when other stuff gets added
    // I have not done any math on the stack size (1024) as yet
    // I have not played with the priority number (20) much yet
    xTaskCreatePinnedToCore(draw, "spectra", 1024, NULL, 20, NULL, 1);
    while (1) {}
}
```

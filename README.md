# spectra-epd
Arduino library for using Pervasive Displays Spectra 4.2" E2417ES053

### ESP-32 example

SPI_DELAY_US is an inter-bit delay value in microseconds.  On the ESP-32 a
value of 4 seems to slow things down enough to get a reliable transfer.  Other
values result in garbage, image slant, offsetting between the red and black pages.

portDISABLE_INTERRUPTS and portENABLE_INTERRUPTS are here to stop background
tasks from messing with our timing.  I have not yet tested this while using wifi.

The delay and interrupt disabling both seem hacky and I'd like to find some
better way to deal with the problems that crop up without them.

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
#define SPI_DELAY_US 4

uint8_t image_buffer[30000];
Spectra spectra(PIN_SCL, PIN_SDA, PIN_CS, PIN_DC, PIN_RESET, PIN_BUSY, PIN_BS, SPI_DELAY_US);

void stripes() {
    for (int n = 0; n < 15000; n++) {
        image_buffer[n] = ((n + 1) % 2 == 0 ? 255 : 0);
    }
    for (int n = 15000; n < 30000; n++) {
        image_buffer[n] = (n % 2 == 0 ? 255 : 0);
    }
}

void setup() {
    spectra_init();
    stripes();
    portDISABLE_INTERRUPTS();
    spectra_draw(image_buffer);
    portENABLE_INTERRUPTS();
}

void loop() {

}
```

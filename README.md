# spectra-epd
Arduino library for using Pervasive Displays Spectra 4.2" E2417ES053

![spectra-epd](https://bbs.electronicchicken.com/images/maya-on-epd-764-400.png)

* [ESP-32 example](#esp-32-example)
* [Constructor](#constructor)
* [Properties](#properties)
* [Methods](#methods)
* [Notes](#notes)
    * [Order of operations](#order-of-operations)
    * [Static images](#static-images)
    * [Image Conversion](#image-conversion)

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

## Constructor

```
Spectra spectra(PIN_SCL, PIN_SDA, PIN_CS, PIN_DC, PIN_RESET, PIN_BUSY, PIN_BS);
```

All pins are digital outputs, except for PIN_BUSY which is a digital input.
Assign whatever pins work for you for these purposes:

* PIN_SCL
    * SPI clock
    * Pin 7 on J5 of _EPD Extension Board Gen2 (EXT2)-01_
* PIN_SDA
    * SPI MOSI
    * Pin 15 on J5 of _EPD Extension Board Gen2 (EXT2)-01_
* PIN_CS
    * SPI Chip Select
    * Pin 19 on J5 of _EPD Extension Board Gen2 (EXT2)-01_
* PIN_DC
    * SPI Data/Command
    * Pin 9 on J5 of _EPD Extension Board Gen2 (EXT2)-01_
* PIN_RESET
    * EPD reset pin
    * Pin 10 on J5 of _EPD Extension Board Gen2 (EXT2)-01_
* PIN_BUSY
    * EPD busy state
    * Pin 8 on J5 of _EPD Extension Board Gen2 (EXT2)-01_
* PIN_BS
    * EPD enable / panel On pin
    * Pin 11 on J5 of _EPD Extension Board Gen2 (EXT2)-01_

## Properties

* WHITE
    * Use this as the 'colour' parameter for 'white' when calling a drawing method
* BLACK
    * Use this as the 'colour' parameter for 'black' when calling a drawing method
* RED
    * Use this as the 'colour' parameter for 'red' when calling a drawing method
* buffer
    * This is the image buffer that will be sent to the EPD when Spectra.draw() is called
    * uint8_t[30000]
    * Every byte represents eight pixels on the EPD
    * The first 15000 bytes are the black frame
    * The last 15000 bytes are the red frame
    * Each byte is MSB first.  Bit 7 of Byte 0 is the first pixel on the EPD, and so on.

## Methods

All of these methods return void.  Except for Spectra.draw(), none of these have
any immediate effect on the EPD.  You must call Spectra.draw() to send data to
and update the display.

* set_pixel(int x, int y, int colour)
    * Set pixel at zero-based coordinates x, y to 'colour'
    * The first pixel is 0:0, the last pixel is 399:299
* blank()
    * Clear the contents of the image buffer (ie. set every pixel to white)
* line(int x0, int y0, int x1, int y1, int colour)
    * Draw a line from x0, y0 to x1, y1 in a given colour
* rectangle(int x0, int y0, int x1, int y1, int colour, bool fill)
    * Draw a rectangle from top left x0, y0 to bottom right x1, y1 in a given colour, filled or not
* circle(int x, int y, int r, int colour, bool fill);
    * Draw a circle with centre point x, y and radius r, of a given colour, filled or not
* draw()
    * Send the current image buffer to the EPD (ie. apply changes and update the physical display)

## Notes

### Order of operations

EPD image data is sent in two 'frames'.  Each frame consists of 15000 bytes, or
one bit for each of the 120000 pixels on the display.  The first frame contains
information on which pixels should be black.  The second frame contains data on
which pixels should be red.  A pixel left unset in both frames remains white. A
pixel set in both frames will be drawn in red.

The order in which any of the above drawing methods are called will affect how
the display actually gets drawn in the end.  This would result in a big red
circle:

```
spectra.circle(199, 149, 100, spectra.BLACK, true);
spectra.circle(199, 149, 150, spectra.RED, true);
spectra.draw();
```

While this would result in a black circle nested inside a red circle:

```
spectra.circle(199, 149, 150, spectra.RED, true);
spectra.circle(199, 149, 100, spectra.BLACK, true);
spectra.draw();
```

Attempts to set pixels outside of the valid range of x,y coordinates will be
ignored silently.  The valid range of 'x' coordinates is 0-399.  The valid
range of 'y' coordinates is 0-299.

It's okay to use a drawing method that may attempt to set pixels at invalid
coordinates.  Only valid areas of the image buffer will be adjusted. For example
this will produce a series of concentric circles:

```
spectra.circle(199, 149, 250, spectra.RED, true);
spectra.circle(199, 149, 200, spectra.BLACK, true);
spectra.circle(199, 149, 150, spectra.RED, true);
spectra.circle(199, 149, 100, spectra.BLACK, true);
spectra.circle(199, 149, 50, spectra.RED, true);
spectra.draw();
```

Areas of the two outermost circles fall outside of the matrix and are ignored,
however any valid portions of them are drawn.

### Static images

The following assumes that you've flashed a SPIFFS image to an ESP-32. I had to
do some digging to find out how to do this with [mkspiffs](https://github.com/igrr/mkspiffs)
and [esptool.py](https://github.com/espressif/esptool). (PlatformIO used to do
this for me with the esp8266.  You may have an easier time if you're using the
Arduino IDE.)

If you have a directory called _data_ in your CWD, the following should turn its
contents into a compatible image and then upload it to the proper area of flash
memory on the ESP-32.  (Disclaimer: it works for me, but I make no promises of
safety or success.)

```sh
/path/to/mkspiffs -d 5 -c data -a -s 1503232 -b 4096 -p 256 fs.img
esptool.py write_flash 0x291000 fs.img
```

In this example, there would have been a file called _image.bin_ sitting inside
of the _data_ directory mentioned above. _image.bin_ should be 30000 bytes long,
with the first 15000 bytes comprising the black frame, and the second 15000
bytes comprising the red frame.  Bit 7 of byte 0 is the top-leftmost pixel of
the black frame.  Bit 0 of byte 14999 is the bottom-rightmost pixel of the black
frame.  Bit 7 of byte 15000 is the top-leftmost pixel of the red frame.  Bit 0
of byte 29999 is the bottom-rightmost pixel of the red frame.

```c++
#include <Arduino.h>
#include "Spectra.h"
#include "FS.h"
#include "SPIFFS.h"

#define PIN_SCL 14
#define PIN_SDA 16
#define PIN_CS 12
#define PIN_DC 13
#define PIN_RESET 4
#define PIN_BUSY 5
#define PIN_BS 21

Spectra spectra(PIN_SCL, PIN_SDA, PIN_CS, PIN_DC, PIN_RESET, PIN_BUSY, PIN_BS);

void draw(void* pvParameters) {
    spectra.draw();
    vTaskDelete(NULL); // self
}

void setup() {
    spectra_init();
    SPIFFS.begin(false, "", 10);
}

void loop() {
    File f = SPIFFS.open("/image.bin");
    f.read(spectra.buffer, 30000);
    f.close();
    xTaskCreatePinnedToCore(draw, "spectra", 1024, NULL, 20, NULL, 1);
    while (1) {}
}
```

### Image Conversion

I host an [image conversion utility](https://bbs.electronicchicken.com/?page=More/006-spectra-image.xjs)
that you can use to produce files for use with this library (as in the above
example).  Also available is the option to produce code to paste into your project
and store the image in program memory rather than on a filesystem.

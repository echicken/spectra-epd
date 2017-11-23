#include <Arduino.h>
#include "Spectra.h"

Spectra::Spectra(int SCL, int SDA, int CS, int DC, int RESET, int BUSY, int BS) {
    PIN_SCL = SCL;
    PIN_SDA = SDA;
    PIN_CS = CS;
    PIN_DC = DC;
    PIN_RESET = RESET;
    PIN_BUSY = BUSY;
    PIN_BS = BS;
    WHITE = 0;
    BLACK = 1;
    RED = 2;
    uint8_t buffer[30000];
}

void Spectra::delay_ms(uint32_t ms) {
    while (ms) {
        delayMicroseconds(1000);
        ms--;
    }
}

void Spectra::send_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        delayMicroseconds(1);
        digitalWrite(PIN_SDA, ((data>>(7-i))&1) == 1 ? HIGH : LOW);
        delayMicroseconds(1);
        digitalWrite(PIN_SCL, HIGH);
        delayMicroseconds(1);
        digitalWrite(PIN_SCL, LOW);
    }
}

void Spectra::send_data(uint8_t index, const uint8_t* data, uint16_t len, uint16_t offset) {
    digitalWrite(PIN_DC, LOW);
    digitalWrite(PIN_CS, LOW);
    send_byte(index);
    digitalWrite(PIN_CS, HIGH);
    digitalWrite(PIN_DC, HIGH);
    digitalWrite(PIN_CS, LOW);
    for (int i = 0; i < len; i++) {
        send_byte(data[offset + i]);
    }
    digitalWrite( PIN_CS, HIGH );
}

void Spectra::busy_wait() {
    while (digitalRead(PIN_BUSY) != HIGH) {
    }
}

void Spectra::init() {
    pinMode(PIN_SCL, OUTPUT);
    pinMode(PIN_SDA, OUTPUT);
    pinMode(PIN_CS, OUTPUT);
    pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_BUSY, INPUT);
    pinMode(PIN_BS, OUTPUT);
    delay_ms(5);
}

void Spectra::set_pixel(int x, int y, int colour) {
    if (x >= 0 && x < EPD_WIDTH && y >= 0 && y < EPD_HEIGHT) {
        int i = ((y * EPD_WIDTH) + x);
        int byte_index = i / 8;
        int bit_index = 7 - (i % 8);
        if (colour == WHITE) {
            buffer[byte_index] &= ~(1<<bit_index);
            buffer[15000 + byte_index] &= ~(1<<bit_index);
        } else if (colour == BLACK) {
            buffer[byte_index] |= (1<<bit_index);
            buffer[15000 + byte_index] &= ~(1<<bit_index);
        } else if (colour == RED) {
            buffer[byte_index] &= ~(1<<bit_index);
            buffer[15000 + byte_index] |= (1<<bit_index);
        }
    }
}

void Spectra::line(int x0, int y0, int x1, int y1, int colour) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;
    for(;;) {
        set_pixel(x0, y0, colour);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void Spectra::rectangle(int x0, int y0, int x1, int y1, int colour, bool fill) {
    line(x0, y0, x1, y0, colour);
    if (!fill) {
        line(x0, y0, x1, y1, colour);
        line(x1, y0, x1, y1, colour);
    } else {
        for (int y = y0 + 1; y < y1; y++) {
            line (x0, y, x1, y, colour);
        }
    }
    line(x0, y1, x1, y1, colour);
}

void Spectra::draw_circle(int x, int y, int xx, int yy, int colour, bool fill) {
    set_pixel(x + xx, y + yy, colour);
    set_pixel(x - xx, y + yy, colour);
    set_pixel(x + xx, y - yy, colour);
    set_pixel(x - xx, y - yy, colour);
    set_pixel(x + yy, y + xx, colour);
    set_pixel(x - yy, y + xx, colour);
    set_pixel(x + yy, y - xx, colour);
    set_pixel(x - yy, y - xx, colour);
    if (fill) {
        for (int xxx = x - xx + 1; xxx < x + xx; xxx++) {
            set_pixel(xxx, y + yy, colour);
            set_pixel(xxx, y - yy, colour);
        }
        for (int xxx = x - yy + 1; xxx < x + yy; xxx++) {
            set_pixel(xxx, y + xx, colour);
            set_pixel(xxx, y - xx, colour);
        }
    }
}

void Spectra::circle(int x, int y, int r, int colour, bool fill) {
    int xx = 0;
    int yy = r;
    int d = 3 - (2 * r);
    while (xx < yy) {
        draw_circle(x, y, xx, yy, colour, fill);
        xx++;
        if (d < 0) {
            d = d + (4 * xx) + 6;
        } else {
            yy--;
            d = d + (4 * (xx - yy)) + 10;
        }
        draw_circle(x, y, xx, yy, colour, fill);
    }
}

void Spectra::draw_rect(const uint8_t* data, int x, int y, int w, int h, bool transparent, int colour) {
    if (transparent || colour == WHITE) {

    } else {
        int idx = ((y * EPD_WIDTH) + x) / 8; // Byte index to start adding to Spectra::buffer
        if (colour == RED) idx = idx + 15000;
        for (int i = 0; i < ((w * h) / 8); i++) { // For each byte in 'data'
            buffer[idx] = data[i];
            if ((i + 1) % (w / 8) == 0) {
                idx = idx + (EPD_WIDTH / 8);
            }
        }
    }
}

void Spectra::blank() {
    for (int n = 0; n < 30000; n++) {
        buffer[n] = 0;
    }
}

void Spectra::draw() {

    delay_ms(5);
    digitalWrite(PIN_BS, HIGH);

    delay_ms(5);
    digitalWrite(PIN_RESET, HIGH);

    delay_ms(5);
    digitalWrite(PIN_RESET, LOW);

    delay_ms(10);
    digitalWrite(PIN_RESET, HIGH);

    delay_ms(5);
    digitalWrite(PIN_CS, HIGH);
    delayMicroseconds(1);

    uint8_t data1[] = { 0x0E };
    send_data(0x00, data1, 1, 0);    // Panel Settings

    uint8_t data2[] = { 0x17, 0x17, 0x27 };
    send_data(0x06, data2, 3, 0);    // Booster Soft Start Settings

    uint8_t data3[] = { 0x01, 0x90, 0x01, 0x2c };
    send_data(0x61, data3, 4, 0);    // Resolution Settings

    uint8_t data4[] = { 0x87 };
    send_data(0x50, data4, 1, 0);    // Vcom and data interval setting

    uint8_t data5[] = { 0x88 };
    send_data(0xe3, data5, 1, 0);    // Power Saving

    uint8_t data6[] = { 0x02 };
    send_data(0xE0, data6, 1, 0);    // Active Temperature

    uint8_t data7[] = { 0x1F };
    send_data(0xE5, data7, 1, 0);    // Input Temperature: 31C, I think

    send_data(0x10, buffer, 15000, 0);
    send_data(0x13, buffer, 15000, 15000);
    delay_ms(50);

    uint8_t data8[] = { 0x00 };
    send_data(0x04, data8, 1, 0);    // Power on
    delay_ms(5);
    busy_wait();

    send_data(0x12, data8, 1, 0);    // Display Refresh
    delay_ms(5);
    busy_wait();

    send_data(0x02, data8, 1, 0);    // Turn off DC/DC
    busy_wait();

    digitalWrite(PIN_DC, LOW);
    digitalWrite(PIN_CS, LOW);
    digitalWrite(PIN_SDA, LOW);
    digitalWrite(PIN_SCL, LOW);
    digitalWrite(PIN_RESET, LOW);
    digitalWrite(PIN_BS, LOW);

}

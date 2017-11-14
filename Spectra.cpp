#include <Arduino.h>
#include "Spectra.h"

Spectra::Spectra(int SCL, int SDA, int CS, int DC, int RESET, int BUSY) {
    PIN_SCL = SCL;
    PIN_SDA = SDA;
    PIN_CS = CS;
    PIN_DC = DC;
    PIN_RESET = RESET;
    PIN_BUSY = BUSY;
}

void Spectra::send_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_SDA, ((data>>(7-i))&1) == 1 ? HIGH : LOW);
        digitalWrite(PIN_SCL, HIGH);
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
}

void Spectra::draw(const uint8_t buffer[30000]) {

    delay(5);
    digitalWrite(PIN_RESET, HIGH);

    delay(5);
    digitalWrite(PIN_RESET, LOW);

    delay(10);
    digitalWrite(PIN_RESET, HIGH);

    delay(5);
    digitalWrite(PIN_CS, HIGH);

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
    send_data(0xE5, data7, 1, 0);    // Input Temperature: 25C

    send_data(0x10, buffer, 15000, 0);
    send_data(0x13, buffer, 15000, 15000);
    delay(50);

    uint8_t data8[] = { 0x00 };
    send_data(0x04, data8, 1, 0);    // Power on
    delay(5);
    busy_wait();

    send_data(0x12, data8, 1, 0);    // Display Refresh
    delay(5);
    busy_wait();

    send_data(0x02, data8, 1, 0);    // Turn off DC/DC
    busy_wait();

    digitalWrite(PIN_DC, LOW);
    digitalWrite(PIN_CS, LOW);
    digitalWrite(PIN_SDA, LOW);
    digitalWrite(PIN_SCL, LOW);
    digitalWrite(PIN_RESET, LOW);

}

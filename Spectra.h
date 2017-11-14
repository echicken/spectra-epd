#ifndef Spectra_h
#define Spectra_h

#include <Arduino.h>

class Spectra
{
    public:
        Spectra(int SCL, int SDA, int CS, int DC, int RESET, int BUSY);
        void init();
        void draw(const uint8_t buffer[30000]);
    private:
        int PIN_SCL;
        int PIN_SDA;
        int PIN_CS;
        int PIN_DC;
        int PIN_RESET;
        int PIN_BUSY;
        void send_byte(uint8_t data);
        void send_data(uint8_t index, const uint8_t* data, uint16_t len, uint16_t offset);
        void busy_wait();
};

#endif

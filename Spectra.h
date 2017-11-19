#ifndef Spectra_h
#define Spectra_h

#define FRAME_WHT 0
#define FRAME_BLK 1
#define FRAME_RED 2
#define EPD_WIDTH 400

#include <Arduino.h>

class Spectra
{
    public:
        Spectra(int SCL, int SDA, int CS, int DC, int RESET, int BUSY, int BS);
        uint8_t buffer[30000];
        void init();
        void set_pixel(int x, int y, int frame);
        void line(int x0, int y0, int x1, int y1, int frame);
        void rectangle(int x0, int y0, int x1, int y1, int frame, bool fill);
        void circle(int x, int y, int r, int frame, bool fill);
        void blank();
        void draw();
    private:
        int PIN_SCL;
        int PIN_SDA;
        int PIN_CS;
        int PIN_DC;
        int PIN_RESET;
        int PIN_BUSY;
        int PIN_BS;
        void draw_circle(int x, int y, int xx, int yy, int frame, bool fill);
        void delay_ms(uint32_t ms);
        void send_byte(uint8_t data);
        void send_data(uint8_t index, const uint8_t* data, uint16_t len, uint16_t offset);
        void busy_wait();
};

#endif

#ifndef Spectra_h
#define Spectra_h

#define EPD_WIDTH 400
#define EPD_HEIGHT 300
#define EPD_BYTE_WIDTH 50
#define EPD_BUFFER 30000
#define EPD_FRAME 15000

#include <Arduino.h>

class Spectra
{
    public:
        Spectra(int SCL, int SDA, int CS, int DC, int RESET, int BUSY, int BS);
        int WHITE;
        int BLACK;
        int RED;
        uint8_t buffer[EPD_BUFFER];
        void init();
        void set_pixel(int x, int y, int colour);
        void line(int x0, int y0, int x1, int y1, int colour);
        void rectangle(int x0, int y0, int x1, int y1, int colour, bool fill);
        void circle(int x, int y, int r, int colour, bool fill);
        void draw_rect(const uint8_t* data, int x, int y, int w, int h, int colour, bool transparent = false, int scale = 1);
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
        void draw_circle(int x, int y, int xx, int yy, int colour, bool fill);
        void delay_ms(uint32_t ms);
        void send_byte(uint8_t data);
        void send_data(uint8_t index, const uint8_t* data, uint16_t len, uint16_t offset);
        void busy_wait();
};

#endif

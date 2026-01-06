#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuration for 128x32 display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C 

class LcdDriver {
private:
    Adafruit_SSD1306 display;
    int sdaPin;
    int sclPin;

public:
    // UPDATED: Constructor now requires the pin numbers
    LcdDriver(int sda, int scl);

    bool begin();
    void clear();
    void update(); 
    void drawImage(int x, int y, const unsigned char* bitmap, int w, int h);
    void printText(int x, int y, const char* text);
};

#endif
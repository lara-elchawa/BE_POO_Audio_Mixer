#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 0: Configuration:
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C 
#define VOLUME_BAR_WIDTH 5
#define VOLUME_BAR_HEIGHT_MAX 30

// I: Creating 3 'virtual' zones: 
// - text       : top right (9x128 px)
// - volume bar : left      (23x10 px)
// - image      : the rest of the screen
enum LCDzone {
    TEXT_ZONE  = 0,
    VOLUM_ZONE = 1,
    IMAGE_ZONE = 2
};

// II: LCD class
class LcdDriver {
private:
    Adafruit_SSD1306 display;
    int sdaPin;
    int sclPin;

    void getZoneOffset(LCDzone zone, int *xOff, int *yOff);  // 

public:
    LcdDriver(int sda, int scl);    // construtor

    bool begin();   // init
    void clear();   // clear the screen 
    void update();  // print all change(s) done 

    void drawImage(int x, int y, const unsigned char* bitmap, int w, int h);    // basic function
    void printText(int x, int y, const char* text);                             // basic function

    void drawVolumeBar(LCDzone zone, int percentage);                                       // print the frame for the volume bar
    void printTextInZone(LCDzone zone, int x, int y, const char* text);                     // print text  in the textual zone  
    void drawImageInZone(LCDzone zone, int x, int y, const unsigned char* bitmap, int width, int height);   // print image in the image   zone
};

#endif
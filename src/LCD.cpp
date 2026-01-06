#include "LCD.h"

// UPDATED: Constructor initializes pins and the display object
LcdDriver::LcdDriver(int sda, int scl) 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
    this->sdaPin = sda;
    this->sclPin = scl;
}

bool LcdDriver::begin() {
    // Start I2C with the specific pins requested
    Wire.begin(sdaPin, sclPin);

    // SSD1306_SWITCHCAPVCC generates display voltage from 3.3V internal
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        return false;
    }
    
    display.clearDisplay();
    display.display();
    return true;
}

void LcdDriver::clear() {
    display.clearDisplay();
}

void LcdDriver::update() {
    display.display();
}

void LcdDriver::drawImage(int x, int y, const unsigned char* bitmap, int w, int h) {
    display.drawBitmap(x, y, bitmap, w, h, SSD1306_WHITE);
}

void LcdDriver::printText(int x, int y, const char* text) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.println(text);
}
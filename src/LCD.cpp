#include "LCD.h"

LcdDriver::LcdDriver(int sda, int scl) 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
    sdaPin = sda;
    sclPin = scl;
}

bool LcdDriver::begin() {
    Wire.begin(sdaPin, sclPin);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
        return false;
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

void LcdDriver::getZoneOffset(LCDzone zone, int *xOff, int *yOff) {
    switch(zone) {
        case TEXT_ZONE:
            *xOff = 10; // at the left, the 10 first px are occupied by the volume bar
            *yOff = 0;
            break;
        case VOLUM_ZONE:
            *xOff = 0; 
            *yOff = 0; 
            break;
        case IMAGE_ZONE:
            *xOff = 10; // the 10 first px, at the left, are occupied by the voume zone
            *yOff = 9;  // the 9  first px, at the top , are occupied by the text  zone
            break;
    }
}

void LcdDriver::printTextInZone(LCDzone zone, int localX, int localY, const char* text) {
    int offXset, offsetY;                       // will be used to store the offsets
    getZoneOffset(zone, &offXset, &offsetY);    // Calculate the offset to apply
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(offXset + localX, offsetY + localY);
    display.println(text);
}

void LcdDriver::drawImageInZone(LCDzone zone, int localX, int localY, const unsigned char* bitmap, int width, int height) {
    int offXset, offsetY;                       // will be used to store the offsets
    getZoneOffset(zone, &offXset, &offsetY);    // Calculate the offset to apply
    
    display.drawBitmap(offXset + localX, offsetY + localY, bitmap, width, height, SSD1306_WHITE);    // draw in the image zone
}

void LcdDriver::drawVolumeBar(LCDzone zone, int percentage) {
    int offXset, offsetY;                       // will be used to store the offsets
    getZoneOffset(zone, &offXset, &offsetY);    // Calculate the offset to apply
    
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    int barHeight = map(percentage, 0, 100, 0, VOLUME_BAR_HEIGHT_MAX-2);   // transform percentage in px, according to the max (set in LCD.h)
    int topMargin = VOLUME_BAR_HEIGHT_MAX - barHeight;

    display.fillRect(offXset + 2, offsetY + topMargin, 5/*width of the bar*/, barHeight/*height of the bar*/, SSD1306_WHITE);
}
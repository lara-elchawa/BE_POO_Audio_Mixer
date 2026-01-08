#include "LCD.h"

/** 
 * @brief create the object
 * @param sda (int) the SDA pin
 * @param scl (int) the SCL pin
 **/
LcdDriver::LcdDriver(int sda, int scl) 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
    sdaPin = sda;
    sclPin = scl;
}

/**
 * @brief initialise the LCD
 */
bool LcdDriver::begin() {
    Wire.begin(sdaPin, sclPin);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
        return false;
    display.clearDisplay();
    display.display();
    return true;
}

/**
 * @brief clear all the LCD display
 */
void LcdDriver::clear() {
    display.clearDisplay();
}

/**
 * @brief dran the given image by "bitmap" on the LCD (don't care about the LCD decomposition in 3 zones)
 * @param x (int) the x value of the beginning of the image  
 * @param y (int) the y value of the beginning of the image
 * @param bitmap (const unsigned char*) an array which contains the bitmap to display (see LCD.h)
 * @param w (int) the width  of the image
 * @param h (int) the height of the image
 */
void LcdDriver::update() {
    display.display();
}

/**
 * @brief draw the given image by "bitmap" on the LCD (don't care about the LCD decomposition in 3 zones)
 * @param x (int) the x value of the beginning of the image  
 * @param y (int) the y value of the beginning of the image
 * @param bitmap (const unsigned char*) the 
 * @param w (int) the width  of the image
 * @param h (int) the height of the image
 */
void LcdDriver::drawImage(int x, int y, const unsigned char* bitmap, int w, int h) {
    display.drawBitmap(x, y, bitmap, w, h, SSD1306_WHITE);
}

/**
 * @brief print the given text on the LCD (don't care about the LCD decomposition in 3 zones)
 * @param x (int) the x value of the beginning of the text  
 * @param y (int) the y value of the beginning of the text
 * @param text (const char*) the text to print
 */
void LcdDriver::printText(int x, int y, const char* text) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.println(text);
}

/**
 * @brief compute the offset in x and y of a given zone
 * @param zone TEXT_ZONE, VOLUM_ZONE or IMAGE_ZONE
 * @param xOff (int) the x value to add to correspond in the selected zone
 * @param yOff (int) the y value to add to correspond in the selected zone
 */
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

/**
 * @brief print a given text in the image zone
 * NB: used only for debugging, watch out to the lenght: line break in the volume bar
 * @param zone TEXT_ZONE, VOLUM_ZONE or IMAGE_ZONE
 * @param localX (int) start x value of the text, in the zone (don't care about the LCD decomposition in 3 zones)
 * @param localY (int) start y value of the image, in the zone (don't care about the LCD decomposition in 3 zones)
 * @param bitmap (const unsigned char*) an array which contains the bitmap to display (see LCD.h)
 * @param width  (int) the width  of the bitmap to display
 * @param height (int) the height of the bitmap to display
 */
void LcdDriver::printTextInZone(LCDzone zone, int localX, int localY, const char* text) {
    int offXset, offsetY;                       // will be used to store the offsets
    getZoneOffset(zone, &offXset, &offsetY);    // Calculate the offset to apply
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(offXset + localX, offsetY + localY);
    display.println(text);
}

/**
 * @brief draw the image send as "bitmap" in the image zone (maximum 118x23 px)
 * @param zone TEXT_ZONE, VOLUM_ZONE or IMAGE_ZONE
 * @param localX (int) start x value of the image, in the zone (don't care about the LCD decomposition in 3 zones)
 * @param localY (int) start y value of the image, in the zone (don't care about the LCD decomposition in 3 zones)
 * @param bitmap (const unsigned char*) an array which contains the bitmap to display (see LCD.h)
 * @param width  (int) the width  of the bitmap to display
 * @param height (int) the height of the bitmap to display
 */
void LcdDriver::drawImageInZone(LCDzone zone, int localX, int localY, const unsigned char* bitmap, int width, int height) {
    int offXset, offsetY;                       // will be used to store the offsets
    getZoneOffset(zone, &offXset, &offsetY);    // Calculate the offset to apply
    
    display.drawBitmap(offXset + localX, offsetY + localY, bitmap, width, height, SSD1306_WHITE);    // draw in the image zone
}

/**
 * @brief draw the volume bar in this zone with the specified percentage
 * @param zone TEXT_ZONE, VOLUM_ZONE or IMAGE_ZONE
 * @param percentage (int) from 0 to 100 
 */
void LcdDriver::drawVolumeBar(LCDzone zone, int percentage) {
    int offXset, offsetY;                       // will be used to store the offsets
    getZoneOffset(zone, &offXset, &offsetY);    // Calculate the offset to apply
    
    if (percentage > 100) 
        percentage = 100;
    if (percentage < 0) 
        percentage = 0;
    int barHeight = map(percentage, 0, 100, 0, VOLUME_BAR_HEIGHT_MAX-2);   // transform percentage in px, according to the max (set in LCD.h)
    int topMargin = VOLUME_BAR_HEIGHT_MAX - barHeight;

    display.fillRect(offXset + 2, offsetY + topMargin, 5/*width of the bar*/, barHeight/*height of the bar*/, SSD1306_WHITE);
}


/**
 * @brief clear the zone (text, volume or image) selected
 * @param zone TEXT_ZONE, VOLUM_ZONE or IMAGE_ZONE
 */
void LcdDriver::clearZone(LCDzone zone) {
    int x;
    int y;
    int width;
    int height;
    getZoneOffset(zone, &x, &y);
    switch(zone) {
        case TEXT_ZONE:
            width  = 128;
            height = 9;
            break;
        case VOLUM_ZONE:
            width  = 10;
            height = 23;
            break;
        case IMAGE_ZONE:
            width  = 118;
            height = 23;
            break;
    }
    display.fillRect(x, y, width, height, SSD1306_BLACK);
}
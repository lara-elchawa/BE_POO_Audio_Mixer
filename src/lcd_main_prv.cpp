#include <Arduino.h>
#include "LCD.h"  

// Configuration of I2C pins:
#define I2C_SDA 21
#define I2C_SCL 22

// create the object:
LcdDriver myLcd(I2C_SDA, I2C_SCL);

// Heart Bitmap:
const unsigned char heartIcon [] PROGMEM = {
  0x00, 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18
};

void setup() {
  Serial.begin(115200); 

  if (!myLcd.begin()) {
    Serial.println("LCD Init Failed!");
    while(1); 
  }

  // test 1: print 2 texts + a custom heart
  // myLcd.clear();
  // myLcd.printText(0, 0, "ESP32 Ready!");
  // myLcd.printText(0, 10, "SDA:21 SCL:22"); 
  // myLcd.drawImage(100, 10, heartIcon, 8, 8);
  // myLcd.update();

  // test 2: with 3 zones:
  myLcd.clear();
  myLcd.printTextInZone(TEXT_ZONE, 2, 0, "50%");                 // text zone
  myLcd.drawVolumeBar(VOLUM_ZONE, 100);                           // volume bar zone
  myLcd.printTextInZone(IMAGE_ZONE, 5, 5, "Bruno M-Lazy song");  // image zone
  myLcd.drawImageInZone(IMAGE_ZONE, 55, 15, heartIcon, 8, 8);
  myLcd.update();
}

void loop() {


  
}
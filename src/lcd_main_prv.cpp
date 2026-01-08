#include <Arduino.h>
#include "LCD.h"  

// Configuration of I2C pins:
#define I2C_SDA 21
#define I2C_SCL 22

// create the object:
LcdDriver myLcd(I2C_SDA, I2C_SCL);



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
  // myLcd.clear();
  // myLcd.printTextInZone(TEXT_ZONE, 2, 0, "50%");                // text zone
  // myLcd.drawVolumeBar(VOLUM_ZONE, 50);                         // volume bar zone
  // myLcd.printTextInZone(IMAGE_ZONE, 5, 5, "Bruno M-Lazy song"); // image zone
  // myLcd.drawImageInZone(IMAGE_ZONE, 55, 15, heartIcon, 8, 8);
  // myLcd.update();
}

void loop() {
  // test 3: 3 zones (improved)
  myLcd.clear();

  for (int i=100 ; i>=0 ; i=i-20)  {
    String msgToPrint = String(i) + "% - Spotify"; 

    myLcd.clear();
    myLcd.printTextInZone(TEXT_ZONE, 10, 0, msgToPrint.c_str());  // text zone
    myLcd.drawVolumeBar(VOLUM_ZONE, i);                           // volume bar zone
    myLcd.drawImageInZone(IMAGE_ZONE, 0, 0, SpotifyIcon, 8, 8);   // image zone
  
    myLcd.update();
    delay(100);
  }
}
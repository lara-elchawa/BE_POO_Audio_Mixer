#include <Arduino.h>
#include "LCD.h"  

// --- HARDWARE CONFIGURATION ---
#define I2C_SDA 21
#define I2C_SCL 22

// Instantiate the driver with the correct pins
LcdDriver myLcd(I2C_SDA, I2C_SCL);

// Example 8x8 bitmap (Heart)
const unsigned char heartIcon [] PROGMEM = {
  0x00, 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18
};

void setup() {
  Serial.begin(115200); 

  if (!myLcd.begin()) {
    Serial.println("LCD Init Failed!");
    while(1); 
  }

  myLcd.clear();
  myLcd.printText(0, 0, "ESP32 Ready!");
  myLcd.printText(0, 10, "SDA:21 SCL:22"); 
  myLcd.drawImage(100, 10, heartIcon, 8, 8);
  myLcd.update();
}

void loop() {
  // Your code here
}
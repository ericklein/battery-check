/*
  Project Name:   battery_check
  Description:    Read and display voltage, percentage charge, and temperature for LiPo batteries

  See README.md for target information and revision history
*/

#include "Adafruit_LC709203F.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

//#define DEBUG

// Current version of Adafruit LC709203F library sets battery size (APA value) 
// using a fixed, limited set of defined values enumerated in LC709203F.h.  
// Select the one closest to the battery size being used

// #define BATTERYSIZE LC709203F_APA_100MAH // 0x08
// #define BATTERYSIZE LC709203F_APA_200MAH // 0x0B
// #define BATTERYSIZE LC709203F_APA_500MAH  // 0x10
// #define BATTERYSIZE LC709203F_APA_1000MAH // 0x19
#define BATTERYSIZE LC709203F_APA_2000MAH // 0x2D
// #define BATTERYSIZE LC709203F_APA_3000MAH // 0x36

Adafruit_LC709203F lc;
Adafruit_SH110X display = Adafruit_SH110X(64, 128, &Wire);

void setup() 
{
  #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial)
    {
    }
  #endif
  debugMessage("battery check started");

  if (!lc.begin())
  {
    debugMessage("Couldnt find Adafruit LC709203F? Make sure a battery is plugged in!");
    while (1) delay(10);
  }

  debugMessage("Found LC709203F");
  //debugMessage("Version: 0x");
  //Serial.println(lc.getICversion(), HEX);

  // lc.setThermistorB(3950);
  // Serial.print("Thermistor B = "); Serial.println(lc.getThermistorB());

  lc.setPackSize(BATTERYSIZE);

  //lc.setAlarmVoltage(3.8);

  //setup display
  display.begin(0x3C, true); // Address 0x3C default
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setRotation(1);
}

void loop() 
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Bat:");
  display.print(lc.cellVoltage(), 2);
  display.println("V");
  display.print("Chg:");
  display.print(lc.cellPercent(),1);
  display.println("%");
  display.display(); // actually display all of the above
  //Serial.print("Batt Voltage: "); Serial.println(lc.cellVoltage(), 3);
  //Serial.print("Batt Percent: "); Serial.println(lc.cellPercent(), 1);
  // Serial.print("Batt Temp: "); Serial.println(lc.getCellTemperature(), 1);

  delay(2000);  // dont query too often!
}

void debugMessage(String messageText)
// wraps Serial.println as #define conditional
{
  #ifdef DEBUG
    Serial.println(messageText);
  #endif
}
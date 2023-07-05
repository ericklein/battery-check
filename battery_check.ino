/*
  Project Name:   battery_check
  Description:    Read and display voltage, percentage charge, and temperature for LiPo batteries

  See README.md for target information and revision history
*/

#include <Adafruit_LC709203F.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Configuration Step 1: Set debug message output
// comment out to turn off; 1 = summary, 2 = verbose
// #define DEBUG 1

// Configuration Step 2: Set battery parameters, if applicable

// If LC709203F detected on i2c, define battery pack based on settings curve from datasheet
// #define BATTERY_APA 0x08 // 100mAH
// #define BATTERY_APA 0x0B // 200mAH
#define BATTERY_APA 0x10 // 500mAH
// #define BATTERY_APA 0x19 // 1000mAH
// #define BATTERY_APA 0x1D // 1200mAH
// #define BATTERY_APA 0x2D // 2000mAH
// #define BATTERY_APA 0x32 // 2500mAH
// #define BATTERY_APA 0x36 // 3000mAH

// used for reading battery voltage from analog PIN on applicable devices
const float batteryMaxVoltage = 4.2;  // maximum battery voltage
const float batteryMinVoltage = 3.2;  // what we regard as an empty battery
// battery pin for Adafruit Feather M0 Express (part#3403)

#if defined (ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  #define VBATPIN A13
#else
  #define VBATPIN A7 // (e.g., D9 on Adafruit Feather M0 Express)
#endif

// hardware status data
typedef struct
{
  float batteryPercent;
  float batteryVoltage;
  float batteryTemp;
} hdweData;
hdweData hardwareData;

// initialize hardware 
Adafruit_LC709203F lc;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

void setup() 
{
  #ifdef DEBUG
    Serial.begin(115200);
    // wait for serial port connection
    while (!Serial);
  #endif 
  debugMessage("battery voltage level testing",1);
  debugMessage("-----------------------------",1);

  hardwareData.batteryVoltage = 0;  // 0 = no battery attached

  //lc.setAlarmVoltage(3.8);

  //setup display
  display.begin(0x3C, true); // Address 0x3C default
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setRotation(1);
}

void loop() 
{
  display.clearDisplay();

  // LC709203F data
  batteryReadVoltage_i2c();
  display.setCursor(0,0);
  display.println("LC709203F");
  display.print(hardwareData.batteryVoltage, 2);
  display.print("V, ");
  display.print(hardwareData.batteryPercent,1);
  display.print("%, ");
  display.print(hardwareData.batteryTemp,1);
  display.println("F");

  // Adafruit recommended read of analog pin
  batteryReadVoltage_adafruit();
  display.println();
  display.println("Adafruit optimized");
  display.print(hardwareData.batteryVoltage, 2);
  display.print("V, ");
  display.print(hardwareData.batteryPercent,1);
  display.println("%");

  // ESP32 optimized software read of analog pin
  batteryReadVoltage_esp32();
  display.println();
  display.println("ESP32 optimized");
  display.print(hardwareData.batteryVoltage, 2);
  display.print("V, ");
  display.print(hardwareData.batteryPercent,1);
  display.println("%");

  display.display();
  delay(1000*60);  // LC709203F needs >=2 seconds between samples
}

void batteryReadVoltage_i2c() 
{
  if (lc.begin())
  // Check battery monitoring status
  {
    debugMessage(String("Version: 0x")+lc.getICversion(),2);
    lc.setPackAPA(BATTERY_APA);
    lc.setThermistorB(3950);
    debugMessage(String("Thermistor B = ")+lc.getThermistorB(),2);

    hardwareData.batteryPercent = lc.cellPercent();
    hardwareData.batteryVoltage = lc.cellVoltage();
    hardwareData.batteryTemp = 32 + (1.8* lc.getCellTemperature());
  } 
  if (hardwareData.batteryVoltage!=0) 
  {
    debugMessage(String("LC709203F battery voltage=") + hardwareData.batteryVoltage + "v, percent=" + hardwareData.batteryPercent + "%, temp=" + hardwareData.batteryTemp +"F",1);    
  }
}

void batteryReadVoltage_esp32() 
{
  pinMode(VBATPIN,INPUT);
  // assumes default ESP32 analogReadResolution (4095)
  // the 1.05 is a fudge factor original author used to align reading with multimeter
  hardwareData.batteryVoltage = ((float)analogRead(VBATPIN) / 4095) * 3.3 * 2 * 1.05;
  hardwareData.batteryPercent = (uint8_t)(((hardwareData.batteryVoltage - batteryMinVoltage) / (batteryMaxVoltage - batteryMinVoltage)) * 100);
  hardwareData.batteryTemp = 0;
  if (hardwareData.batteryVoltage!=0) 
  {
    debugMessage(String("Battery voltage from ESP32 routines: ") + hardwareData.batteryVoltage + "v, percent: " + hardwareData.batteryPercent + "%",1);
  }
}

void batteryReadVoltage_adafruit()
{
  #if defined (ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
    // Adafruit power management guide for Adafruit ESP32V2
    hardwareData.batteryVoltage = analogReadMilliVolts(VBATPIN);
    hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
    hardwareData.batteryVoltage /= 1000; // convert to volts!
    hardwareData.batteryPercent = (uint8_t)(((hardwareData.batteryVoltage - batteryMinVoltage) / (batteryMaxVoltage - batteryMinVoltage)) * 100);
    hardwareData.batteryTemp = 0;
  #else
    // Adafruit power management guide for Adafruit Feather M0 Express ()
    hardwareData.batteryVoltage = analogRead(VBATPIN);
    hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
    hardwareData.batteryVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
    hardwareData.batteryVoltage /= 1024; // convert to voltage
    hardwareData.batteryPercent = (uint8_t)(((hardwareData.batteryVoltage - batteryMinVoltage) / (batteryMaxVoltage - batteryMinVoltage)) * 100);
  #endif
  if (hardwareData.batteryVoltage!=0) 
  {
    debugMessage(String("Battery voltage from adafruit routine: ") + hardwareData.batteryVoltage + "v, percent: " + hardwareData.batteryPercent + "%",1);
  }
}


void debugMessage(String messageText, int messageLevel)
// wraps Serial.println as #define conditional
{
  #ifdef DEBUG
    if (messageLevel <= DEBUG)
    {
      Serial.println(messageText);
      Serial.flush();  // Make sure the message gets output (before any sleeping...)
    }
  #endif
}
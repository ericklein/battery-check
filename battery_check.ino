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
const int   batteryReads = 5;

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
  float batteryTemperature;
} hdweData;
hdweData hardwareData;

// initialize hardware 
Adafruit_LC709203F lc;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

//screen assist constants
const int xMargins = 10;
const int yMargins = 2;
const int batteryBarWidth = 28;
const int batteryBarHeight = 10;

// battery charge level lookup table
const float vs[101] = {
  3.200,  3.250,  3.300,  3.350,  3.400,  3.450,
  3.500,  3.550,  3.600,  3.650,  3.700,  3.703,
  3.706,  3.710,  3.713,  3.716,  3.719,  3.723,
  3.726,  3.729,  3.732,  3.735,  3.739,  3.742,
  3.745,  3.748,  3.752,  3.755,  3.758,  3.761,
  3.765,  3.768,  3.771,  3.774,  3.777,  3.781,
  3.784,  3.787,  3.790,  3.794,  3.797,  3.800,
  3.805,  3.811,  3.816,  3.821,  3.826,  3.832,
  3.837,  3.842,  3.847,  3.853,  3.858,  3.863,
  3.868,  3.874,  3.879,  3.884,  3.889,  3.895,
  3.900,  3.906,  3.911,  3.917,  3.922,  3.928,
  3.933,  3.939,  3.944,  3.950,  3.956,  3.961,
  3.967,  3.972,  3.978,  3.983,  3.989,  3.994,
  4.000,  4.008,  4.015,  4.023,  4.031,  4.038,
  4.046,  4.054,  4.062,  4.069,  4.077,  4.085,
  4.092,  4.100,  4.111,  4.122,  4.133,  4.144,
  4.156,  4.167,  4.178,  4.189,  4.200};

void setup() 
{
  #ifdef DEBUG
    Serial.begin(115200);
    // wait for serial port connection
    while (!Serial);
    debugMessage("battery voltage level testing",1);
    debugMessage("-----------------------------",1);
  #endif 

  hardwareData.batteryVoltage = 0;  // 0 = no battery attached
  hardwareData.batteryPercent = 0;
  hardwareData.batteryTemperature = 0;

  //display setup
  display.begin(0x3C, true); // Default i2c address is 0x3C
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setRotation(1);
}

void loop() 
{
  display.clearDisplay();

  // LC709203F data
  batteryRead_LC709();
  display.setCursor(0,0);
  display.println("LC709203F");
  display.print(hardwareData.batteryVoltage, 3);
  display.print("V,");
  display.print(hardwareData.batteryPercent,2);
  display.print("%,");
  display.print(hardwareData.batteryTemperature,1);
  display.println("F");

  // Visualization uses most accurate data from hardware monitor
  screenHelperBatteryStatus((display.width()-xMargins-batteryBarWidth-3),(display.height()-yMargins-batteryBarHeight),batteryBarWidth, batteryBarHeight);

  hardwareData.batteryTemperature = 0; // no other read routines capture battery temperature

  // Adafruit recommended read of analog pin
  batteryReadVoltage_Adafruit(batteryReads);
  hardwareData.batteryPercent = batteryGetChargeLevel(hardwareData.batteryVoltage);
  display.println();
  display.println("Adafruit");
  display.print(hardwareData.batteryVoltage, 3);
  display.print("V, ");
  display.print(hardwareData.batteryPercent);
  display.println("%");

  #if defined (ARDUINO_ADAFRUIT_FEATHER_ESP32_V2) // ESP32 optimized software read of analog pin
    batteryReadVoltage_esp32(batteryReads);
    hardwareData.batteryPercent = batteryGetChargeLevel(hardwareData.batteryVoltage);
    display.println();
    display.println("ESP32");
    display.print(hardwareData.batteryVoltage, 3);
    display.print("V,");
    display.print(hardwareData.batteryPercent);
    display.println("%");
  #endif

  display.display();
  delay(1000*60);  // LC709203F needs >=2 seconds between samples
}

void batteryRead_LC709() 
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
    hardwareData.batteryTemperature = 32 + (1.8* lc.getCellTemperature());
  }
  else {
    debugMessage("Couldn't find LC709203F. Make sure hardware and battery are plugged in",1);
  }
  if (hardwareData.batteryVoltage!=0) 
  {
    debugMessage(String("LC709203F battery voltage=") + hardwareData.batteryVoltage + "v, percent=" + hardwareData.batteryPercent + "%, temp=" + hardwareData.batteryTemperature +"F",1);    
  }
}

void batteryReadVoltage_esp32(int reads) 
{
  pinMode(VBATPIN,INPUT);
  float accumulatedVoltage = 0;
    for (int loop = 0; loop < reads; loop++)
    {
      accumulatedVoltage += analogRead(VBATPIN);
    }
  hardwareData.batteryVoltage = accumulatedVoltage/reads; // we now have the average reading
  hardwareData.batteryVoltage *= 2;     // we divided by 2, so multiply back
  hardwareData.batteryVoltage *= 3.3;   // Multiply by 3.3V, our reference voltage
  hardwareData.batteryVoltage *= 1.05;  // the 1.05 is a fudge factor original author used to align reading with multimeter
  hardwareData.batteryVoltage /= 4095;  // assumes default ESP32 analogReadResolution (4095)
  if (hardwareData.batteryVoltage!=0) 
  {
    debugMessage(String("Battery voltage from ESP32 optimized routine: ") + hardwareData.batteryVoltage + "v, percent: " + hardwareData.batteryPercent + "%",1);
  }
}

void batteryReadVoltage_Adafruit(int reads)
{
  float accumulatedVoltage = 0;
  #if defined (ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
    // modified from the Adafruit power management guide for Adafruit ESP32V2
    for (int loop = 0; loop < reads; loop++)
      {
        accumulatedVoltage += analogReadMilliVolts(VBATPIN);
      }
    hardwareData.batteryVoltage = accumulatedVoltage/reads; // we now have the average reading
    // convert into volts  
    hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
    hardwareData.batteryVoltage /= 1000; // convert to volts!
  #else
    // modified from the Adafruit power management guide for Adafruit Feather M0 Express
    for (int loop = 0; loop < reads; loop++)
      {
        accumulatedVoltage += analogRead(VBATPIN);
      }
    hardwareData.batteryVoltage = accumulatedVoltage/reads; // we now have the average reading
    hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
    hardwareData.batteryVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
    hardwareData.batteryVoltage /= 1024; // convert to voltage
  #endif
  if (hardwareData.batteryVoltage!=0) 
  {
    debugMessage(String("Battery voltage from adafruit routine: ") + hardwareData.batteryVoltage + "v",1);
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

void screenHelperBatteryStatus(int initialX, int initialY, int barWidth, int barHeight)
// helper function for screenXXX() routines that draws battery charge %
{
  // IMPROVEMENT : Screen dimension boundary checks for function parameters
  if (hardwareData.batteryVoltage>0) 
  {
    // battery nub; width = 3pix, height = 60% of barHeight
    display.fillRect((initialX+barWidth),(initialY+(int(barHeight/5))),3,(int(barHeight*3/5)),SH110X_WHITE);
    // battery border
    display.drawRect(initialX,initialY,barWidth,barHeight,SH110X_WHITE);
    //battery percentage as rectangle fill, 1 pixel inset from the battery border
    display.fillRect((initialX + 2),(initialY + 2),(int((hardwareData.batteryPercent/100)*barWidth) - 4),(barHeight - 4),SH110X_WHITE);
    debugMessage(String("battery status drawn to screen as ") + hardwareData.batteryPercent + "%",2);
  }
}

int batteryGetChargeLevel(float volts)
{
  int idx = 50;
  int prev = 0;
  int half = 0;
  if (volts >= 4.2){
    return 100;
  }
  if (volts <= 3.2){
    return 0;
  }
  while(true){
    half = abs(idx - prev) / 2;
    prev = idx;
    if(volts >= vs[idx]){
      idx = idx + half;
    }else{
      idx = idx - half;
    }
    if (prev == idx){
      break;
    }
  }
  debugMessage(String("Battery percentage as int is ")+idx+"%",1);
  return idx;
}
/*
  Project Name:   battery_check
  Description:    Read and display voltage, percentage charge, and temperature for LiPo batteries

  See README.md for target information and revision history
*/

#include "config.h"
#include <Adafruit_LC709203F.h>
#include <Adafruit_SH110X.h>

// hardware status data
typedef struct
{
  float batteryPercent;
  float batteryVoltage;
  float batteryTemperatureF;
} hdweData;
hdweData hardwareData;

// initialize hardware 
Adafruit_LC709203F lc;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

//screen assist constants
const uint8_t xMargins = 10;
const uint8_t yMargins = 2;
const uint8_t batteryBarWidth = 28;
const uint8_t batteryBarHeight = 14;

void setup()
{
  #ifdef DEBUG
    Serial.begin(115200);
    // wait for serial port connection
    while (!Serial);
  #endif

  debugMessage("Battery Voltage Tester",1);

  hardwareData.batteryVoltage = 0.0;  // 0 = no battery attached
  hardwareData.batteryPercent = 0.0;
  hardwareData.batteryTemperatureF = 0.0;

  //display setup
  if(!display.begin(0x3C, true)) // Default i2c address is 0x3C
  {
    debugMessage("Can't initialize display",1);
    while(1); // stop the app
  }
  display.setTextColor(SH110X_WHITE);
  display.setRotation(1);
  display.setTextWrap(false);
}

void loop() 
{
  display.clearDisplay();

  // LC709203F data
  batteryRead_LC709();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print(hardwareData.batteryVoltage, 3);
  display.println("v");
  display.print(hardwareData.batteryPercent,1);
  display.println("%");
  display.print(hardwareData.batteryTemperatureF,1);
  display.println("F");

  // Visualization uses hardwareData.batteryVoltage just set via LC709203F
  screenHelperBatteryStatus((display.width()-xMargins-batteryBarWidth-3),(display.height()-yMargins-batteryBarHeight), batteryBarWidth, batteryBarHeight);

  hardwareData.batteryTemperatureF = 0; // no other read routines capture battery temperature

  // Adafruit recommended read of analog pin
  display.setTextSize(1);
  display.setCursor(90,0);
  batteryReadVoltage_Adafruit(pinReadsPerSample);
  hardwareData.batteryPercent = batteryGetChargeLevel(hardwareData.batteryVoltage);
  display.print(hardwareData.batteryVoltage, 3);
  display.print("v");
  display.setCursor(90,20);
  display.print(hardwareData.batteryPercent,0);
  display.println("%");

  display.display();
  delay(1000*batterySampleInterval);  // LC709203F needs >=2 seconds between samples
}

void batteryRead_LC709() 
{
  if (lc.begin())
  // Check battery monitoring status
  {
    debugMessage(String("Version: 0x")+lc.getICversion(),2);
    lc.setPackAPA(BATTERY_APA);
    lc.setThermistorB(3950);

    hardwareData.batteryPercent = lc.cellPercent();
    hardwareData.batteryVoltage = lc.cellVoltage();
    hardwareData.batteryTemperatureF = 32 + (1.8* lc.getCellTemperature());
  }
  else
  {
    debugMessage("Couldn't find LC709203F. Make sure hardware and battery are plugged in",1);
    hardwareData.batteryVoltage = 0.0;  // 0 = no battery attached
    hardwareData.batteryPercent = 0.0;
    hardwareData.batteryTemperatureF = 0.0;
  }
  debugMessage(String("LC709203F voltage=") + hardwareData.batteryVoltage + "v, percent=" + hardwareData.batteryPercent + "%, temp=" + hardwareData.batteryTemperatureF +"F",1);    
}

void batteryReadVoltage_Adafruit(uint8_t reads)
{
  float accumulatedVoltage = 0.0;
  #if defined (ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
    // modified from the Adafruit power management guide for Adafruit ESP32V2
    for (uint8_t loop = 0; loop < reads; loop++)
      {
        accumulatedVoltage += analogReadMilliVolts(BATTERY_VOLTAGE_PIN);
      }
    hardwareData.batteryVoltage = accumulatedVoltage/reads; // we now have the average reading
    // convert into volts  
    hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
    hardwareData.batteryVoltage /= 1000; // convert to volts!
  #else
    // modified from the Adafruit power management guide for Adafruit Feather M0 Express
    for (uint8_t loop = 0; loop < reads; loop++)
      {
        accumulatedVoltage += analogRead(BATTERY_VOLTAGE_PIN);
      }
    hardwareData.batteryVoltage = accumulatedVoltage/reads; // we now have the average reading
    hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
    hardwareData.batteryVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
    hardwareData.batteryVoltage /= 1024; // convert to voltage
  #endif
  debugMessage(String("Adafruit algo battery voltage=") + hardwareData.batteryVoltage + "v",1);
}

float usbPinGetVoltage(uint8_t reads)
{
  float accumulatedVoltage = 0.0;
  float usbVoltage = 0.0;

  #if defined (ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
    // modified from the Adafruit power management guide for Adafruit ESP32V2
    for (uint8_t loop = 0; loop < reads; loop++)
      {
        accumulatedVoltage += analogReadMilliVolts(USB_VOLTAGE_PIN);
      }
    usbVoltage = accumulatedVoltage/reads; // we now have the average reading
    usbVoltage /= 1000; // convert to volts
  #else
    // modified from the Adafruit power management guide for Adafruit Feather M0 Express
    for (uint8_t loop = 0; loop < reads; loop++)
      {
        accumulatedVoltage += analogRead(USB_VOLTAGE_PIN);
        debugMessage(String("Accumulated units=") + accumulatedVoltage,2);
      }
    usbVoltage = accumulatedVoltage/reads; // we now have the average reading
    usbVoltage *= 2;    // we divided by 2, so multiply back
    usbVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
    usbVoltage /= 1024; // convert to voltage
  #endif
  debugMessage(String("USB voltage=") + usbVoltage + "v",1);
  return usbVoltage;
}

void debugMessage(String messageText, uint8_t messageLevel)
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

void screenHelperBatteryStatus(uint16_t initialX, uint16_t initialY, uint8_t barWidth, uint8_t barHeight)
// helper function for screenXXX() routines that draws battery charge %
// 
{
  // IMPROVEMENT : Screen dimension boundary checks for passed parameters
  // IMPROVEMENT : Check for offscreen drawing based on passed parameters
  if (hardwareData.batteryVoltage>batteryVoltageTable[0]) 
  {
    // battery nub; width = 3pix, height = 60% of barHeight
    display.fillRect((initialX+barWidth),(initialY+(int(barHeight/5))),3,(int(barHeight*3/5)),SH110X_WHITE);
    // battery border
    display.drawRect(initialX,initialY,barWidth,barHeight,SH110X_WHITE);
    // display battery remaining battery % as rectangle fill, 1 pixel inset from the battery border
    display.fillRect((initialX + 2),(initialY + 2),int(0.5+(hardwareData.batteryPercent*((barWidth-4)/100.0))),(barHeight - 4),SH110X_WHITE);
    debugMessage(String("Battery percent displayed=") + hardwareData.batteryPercent + "%, " + int(0.5+(hardwareData.batteryPercent*((barWidth-4)/100.0))) + " pixels of " + (barWidth-4) + " max",1);
    if (usbPinGetVoltage(pinReadsPerSample) > 4.5)
    {
      // USB power = battery is charging, display battery charging indicator
      display.fillTriangle(initialX+4,initialY+6,initialX+15,initialY+6,initialX+15,initialY+10,SH110X_BLACK);
      display.fillTriangle(initialX+13,initialY+4,initialX+24,initialY+8,initialX+13,initialY+8,SH110X_BLACK);
    }
  }
  else
    debugMessage("Battery is not charged",1);
}

int batteryGetChargeLevel(float volts)
{
  uint8_t idx = 50;
  uint8_t prev = 0;
  uint8_t half = 0;
  if (volts >= 4.2){
    return 100;
  }
  if (volts <= 3.2){
    return 0;
  }
  while(true){
    half = abs(idx - prev) / 2;
    prev = idx;
    if(volts >= batteryVoltageTable[idx]){
      idx = idx + half;
    }else{
      idx = idx - half;
    }
    if (prev == idx){
      break;
    }
  }
  debugMessage(String("Software battery percentage=")+idx+"%",1);
  return idx;
}
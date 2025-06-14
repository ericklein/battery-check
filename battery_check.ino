/*
  Project Name:   battery_check
  Description:    Read and display voltage, percentage charge, and temperature for LiPo batteries

  See README.md for target information and revision history
*/

// hardware and internet configuration parameters
#include "config.h"

// hardware support
#include <Adafruit_SH110X.h>
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

#ifndef HARDWARE_SIMULATE
  // battery voltage sensor
  #include <Adafruit_LC709203F.h>
  Adafruit_LC709203F lc;
#endif

// button support
#include <ezButton.h>
ezButton buttonOne(buttonAPin);
ezButton buttonTwo(buttonBPin);

// hardware status data
typedef struct
{
  float batteryPercent;
  float batteryVoltage;
  float batteryTemperatureF;
} hdweData;
hdweData hardwareData;

uint32_t timeLastBatterySample;
uint8_t screenCurrent = 0; // tracks which screen is displayed

void setup()
{
  #ifdef DEBUG
    Serial.begin(115200);
    // wait for serial port connection
    while (!Serial)
      debugMessage(String("Starting battery_check with ") + batterySampleInterval + " second sample interval",1);
  #endif

  //display setup
  if(!display.begin(0x3C, true)) // Default i2c address is 0x3C
  {
    debugMessage("Can't initialize display",1);
    while(1); // stop the app
  }
  display.setTextColor(SH110X_WHITE);
  display.setRotation(1);
  display.setTextWrap(false);

  buttonOne.setDebounceTime(buttonDebounceDelay);
  buttonTwo.setDebounceTime(buttonDebounceDelay);

  //First time draw
  screenUpdate();
}

void loop() 
{
  buttonOne.loop();
  if (buttonOne.isReleased())
    screenStats();
  buttonTwo.loop();
  if (buttonTwo.isReleased())
    screenBatteryIcon();

  // is it time to update the current screen?
  if((millis() - timeLastBatterySample) >= (batterySampleInterval * 1000)) // converting sensorSampleInterval into milliseconds
  {
    screenUpdate();
    timeLastBatterySample = millis();
  }
}

void screenUpdate()
// Description: Display requested screen
// Parameters: NA (global)
// Output: NA (void)
// Improvement: ?
{
  switch(screenCurrent) {
    case 0:
      screenStats();
      break;
    case 1:
      screenBatteryIcon();
      break;
    default:
      // handle out of range screenCurrent
      screenStats();
      debugMessage("Error: Unexpected screen ID",1);
      break;
  }
}

void screenStats()
{
  display.clearDisplay();

  // Labels
  display.setCursor(xHardwareMargin,yLabel);
  display.println("Hdwe");
  display.setCursor(xSoftwareMargin,yLabel);
  display.println("GPIO");

  display.setCursor(xLabelMargin,yBatteryVoltage);
  display.print("Volt");
  display.setCursor(xLabelMargin,yBatteryPercent);
  display.print("%");
  display.setCursor(xLabelMargin,yBatteryTempF);
  display.print("TempF");

  batteryReadLC709();
  display.setCursor(xHardwareMargin,yBatteryVoltage);
  display.print(hardwareData.batteryVoltage, 3);
  display.setCursor(xHardwareMargin,yBatteryPercent);
  display.print(hardwareData.batteryPercent,1);
  display.setCursor(xHardwareMargin, yBatteryTempF);
  display.print(hardwareData.batteryTemperatureF,1);

  // Software battery data display
  hardwareData.batteryTemperatureF = 0; // zero as not set by GPIO read
  batteryReadVoltageSoftware(batteryReadsPerSample);
  display.setCursor(xSoftwareMargin,yMargins);
  display.setCursor(xSoftwareMargin,yBatteryVoltage);  
  display.print(hardwareData.batteryVoltage, 3);
  display.setCursor(xSoftwareMargin,yBatteryPercent);
  display.print(hardwareData.batteryPercent,0);

  display.display();
}

void screenBatteryIcon()
{
  // Hardware battery data display
  // Display battery level icon, set via LC709203F
  screenHelperBatteryStatus(xHardwareMargin,(display.height()-yMargins-batteryBarHeight), batteryBarWidth, batteryBarHeight);
}

bool batteryReadLC709()
// Description: sets global battery values from i2c battery monitor
// Parameters: integer that sets how many times the analog pin is sampled
// Output: NA (globals)
// Improvement: MAX17084 support
{
  #ifdef HARDWARE_SIMULATE
    batterySimulate();
    return true;
  #else
    if (lc.begin())
    {
      lc.setPackAPA(BATTERY_APA);
      lc.setThermistorB(3950);

      hardwareData.batteryPercent = lc.cellPercent();
      hardwareData.batteryVoltage = lc.cellVoltage();
      hardwareData.batteryTemperatureF = 32 + (1.8* lc.getCellTemperature());
      debugMessage(String("LC709203F voltage=") + hardwareData.batteryVoltage + "v, percent=" + hardwareData.batteryPercent + "%, temp=" + hardwareData.batteryTemperatureF +"F",1);
      return true;
    }
    else
    {
      hardwareData.batteryVoltage = 0.0;  // 0 = no battery attached
      hardwareData.batteryPercent = 0.0;
      hardwareData.batteryTemperatureF = 0.0;
      debugMessage("Couldn't find LC709203F",1);
      return false;
    }
  #endif
}

bool batteryReadVoltageSoftware(uint8_t reads)
// Description: sets global battery values from analog pin value (on supported boards)
// Parameters: integer that sets how many times the analog pin is sampled
// Output: NA (globals)
// Improvement:
{
  // read gpio pin on supported boards for battery level
  debugMessage("batteryRead using GPIO pin",2);
  float accumulatedVoltage = 0.0f;
  for (uint8_t loop = 0; loop < reads; loop++)
  {
    //ESP32
    // accumulatedVoltage += analogReadMilliVolts(BATTERY_VOLTAGE_PIN);
    //debugMessage(String("Battery read ") + loop + " is " + analogReadMilliVolts(BATTERY_VOLTAGE_PIN) + "mV",2);
    // ARM
    accumulatedVoltage += analogRead(BATTERY_VOLTAGE_PIN);
    debugMessage(String("Battery read ") + loop + " is " + analogRead(BATTERY_VOLTAGE_PIN) + "??",2);
  }
  hardwareData.batteryVoltage = accumulatedVoltage/reads; // we now have the average reading
  // convert into volts
  // ESP32
  // hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
  // hardwareData.batteryVoltage /= 1000; // convert to volts!
  // ARM
  hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
  hardwareData.batteryVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
  hardwareData.batteryVoltage /= 1024; // convert to voltage
  hardwareData.batteryPercent = batteryGetChargeLevel(hardwareData.batteryVoltage);

  if (hardwareData.batteryVoltage>batteryVoltageTable[0]) {
    debugMessage(String("Battery voltage: ") + hardwareData.batteryVoltage + "v, percent: " + hardwareData.batteryPercent + "%", 1);
    return true;
  }
  else
    return false;
}

float usbPinGetVoltage(uint8_t reads)
{
  // read gpio pin on supported boards for voltage level
  debugMessage("USB pin voltage using GPIO pin",2);
  float accumulatedVoltage = 0.0f;
  float usbVoltage = 0.0f;

  for (uint8_t loop = 0; loop < reads; loop++)
  {
    // ESP32
    // accumulatedVoltage += analogReadMilliVolts(USB_VOLTAGE_PIN);
    // debugMessage(String("USB voltage read ") + loop + " is " + analogReadMilliVolts(USB_VOLTAGE_PIN) + "mV",2);
    // ARM
    accumulatedVoltage += analogRead(USB_VOLTAGE_PIN);
    debugMessage(String("USB voltage read ") + loop + " is " + analogRead(USB_VOLTAGE_PIN) + "??",2);
  }
  usbVoltage = accumulatedVoltage/reads; // we now have the average reading
  // ESP32
  // usbVoltage /= 1000; // convert to volts
  // hardwareData.batteryVoltage *= 2;    // we divided by 2, so multiply back
  // ARM
  usbVoltage *= 2;    // we divided by 2, so multiply back
  usbVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
  usbVoltage /= 1024; // convert to voltage
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
// Description: helper function for screenXXX() routines, displaying battery charge % in a battery icon
// Parameters:
// Output: NA (void)
// IMPROVEMENT : Screen dimension boundary checks for passed parameters
// IMPROVEMENT : Check for offscreen drawing based on passed parameters
// IMPROVEMENT: batteryNub isn't placed properly when barHeight is changed from (default) 10 pixels
{
  if (batteryReadLC709())
  {
    // battery nub; width = 3pix, height = 60% of barHeight
    display.fillRect((initialX+barWidth),(initialY+(int(barHeight/5))),3,(int(barHeight*3/5)),SH110X_WHITE);
    // battery border
    display.drawRect(initialX,initialY,barWidth,barHeight,SH110X_WHITE);
    // display battery remaining battery % as rectangle fill, 1 pixel inset from the battery border
    display.fillRect((initialX + 2),(initialY + 2),int(0.5+(hardwareData.batteryPercent*((barWidth-4)/100.0))),(barHeight - 4),SH110X_WHITE);
    debugMessage(String("Battery percent displayed=") + hardwareData.batteryPercent + "%, " + int(0.5+(hardwareData.batteryPercent*((barWidth-4)/100.0))) + " pixels of " + (barWidth-4) + " max",1);
    // DEBUG USE ONLY, display voltage level in the icon, requires changing bar height to 14 pixels
    // display.setFont(); // switch to generic small font
    // display.setCursor(initialX +2 ,initialY + 2);
    // display.print(hardwareData.batteryVoltage);
    // if the device on USB power, display battery charging indicator
    if (usbPinGetVoltage(batteryReadsPerSample) > 4.5)
    {
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

#ifdef HARDWARE_SIMULATE
  void batterySimulate()
  // Simulate battery data
  // IMPROVEMENT: Simulate battery below SCD40 required level
  {
    hardwareData.batteryVoltage = random(batterySimVoltageMin, batterySimVoltageMax) / 100.00;
    hardwareData.batteryPercent = batteryGetChargeLevel(hardwareData.batteryVoltage);
    debugMessage(String("SIMULATED Battery voltage: ") + hardwareData.batteryVoltage + "v, percent: " + hardwareData.batteryPercent + "%", 1);  
  }
#endif
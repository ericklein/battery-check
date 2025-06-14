/*
  Project Name:   battery_check
  Description:    public (non-secret) configuration data

  See README.md for target information and revision history
*/

// Configuration Step 1: Set debug message output
// comment out to turn off; 1 = summary, 2 = verbose
#define DEBUG 2

// Configuration Step 2: Set delay between samples in seconds
#ifdef DEBUG
	const uint8_t batterySampleInterval = 5;
#else
	const uint8_t batterySampleInterval = 30;
#endif

// Configuration Step 3: simulate battery,
// returning random but plausible values
// comment out to turn off
// #define HARDWARE_SIMULATE

// Configuration Step 4: Set battery parameters
// If LC709203F detected on i2c, define battery pack based on settings curve from datasheet
// #define BATTERY_APA 0x08 // 100mAH
// #define BATTERY_APA 0x0B // 200mAH
 #define BATTERY_APA 0x10 // 500mAH
// #define BATTERY_APA 0x19 // 1000mAH
// #define BATTERY_APA 0x1D // 1200mAH
// #define BATTERY_APA 0x32 // 2500mAH
// #define BATTERY_APA 0x36 // 3000mAH

// Configuration variables unlikely to require changes

//screen assist constants
const uint8_t xLabelMargin        = 5;
const uint8_t xHardwareMargin     = 40;
const uint8_t xSoftwareMargin     = 90; 
const uint8_t yLabel              = 10;
const uint8_t yBatteryVoltage     = 22;
const uint8_t yBatteryPercent     = 35;
const uint8_t yBatteryTempF       = 48;

const uint8_t yMargins = 2;
const uint8_t batteryBarWidth     = 28;
const uint8_t batteryBarHeight    = 14;

const uint8_t screenCount = 2;

// Buttons
const uint8_t buttonCPin = 5;
const uint8_t buttonBPin = 6;
const uint16_t buttonDebounceDelay = 50; // time in milliseconds to debounce button

// Battery 
// analog pin used to reading battery voltage
// #define BATTERY_VOLTAGE_PIN A13 // Adafruit Feather ESP32V2
#define BATTERY_VOLTAGE_PIN A7 // Adafruit Feather M0 Express, pin 9
// #define BATTERY_VOLTAGE_PIN 04 // Adafruit MagTag

#define USB_VOLTAGE_PIN A0 // Adafruit Feather M0 Express, pin 14

// number of analog pin reads sampled to average battery voltage
const uint8_t   batteryReadsPerSample = 5;

// battery charge level lookup table
const float batteryVoltageTable[101] = {
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
  4.156,  4.167,  4.178,  4.189,  4.200 };
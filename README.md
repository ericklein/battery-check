# Battery Check

### Purpose
Read and display voltage, percentage charge for LiPo batteries. Battery temperature if available, is displayed as well.
The solution uses both dedicated voltage monitoring hardware and calculations based on a battery monitoring pin included in the Adafruit hardware.

### Features

### Target configuration
- set configuration parameters in config.h

### Bill of Materials (BOM)
- MCU
	- [Adafruit Feather M0 Express](https://www.adafruit.com/product/3403)
	- [Adafruit Feather ESP32v2](https://www.adafruit.com/product/5400)
- battery monitor
	- [Adafruit LC709203F battery voltage monitor](https://www.adafruit.com/product/4712)
	- 10K thermistor
- screen
	- [Adafruit FeatherWing OLED - 128x64 OLED](https://www.adafruit.com/product/4650)
- battery
	- [Adafruit batteries](https://www.adafruit.com/category/889)

### Pinouts
- Stemma QT cable between MCU board and LC709203F board
- Battery connected to LC709203F board
- Power connector between LC709203F board and MCU board
- Feather screen connection to MCU board
- 10K thermistor between thermistor pin and ground pin on LC709203F board (required to measure battery temperature)

### Information Sources
[Adafruit Feather M0 Express power management](https://learn.adafruit.com/adafruit-feather-m0-express-designed-for-circuit-python-circuitpython/power-management-2)
[Adafruit LC709203F board overview](https://learn.adafruit.com/adafruit-lc709203f-lipo-lipoly-battery-monitor?view=all#overview)

### Issues and Feature Requests
- See GitHub Issues for project

### .plan (big ticket items)
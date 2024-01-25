# esphome-parkside-plem-50-c3
Parkside PLEM 50 C3 laser range finder I2C Integration for ESPHome

Work in progress, this is not a functional module.

## Description
Parkside PLEM 50 C3 is a cheap (around €30) laser finder. I find this one very useful for monitoring water levels in water tanks and drainages.

The device consist of laser sensor module and keyboard PCB + display module. Laser module is connected to keyboard with 20-conductor 0.5mm FFC cable.  
Unlike the C2 model, whole logic is in laser module. Laser module continuously send the display state to keyboard/display PCB over I2C. Laser module is master in this communication.  
Keyboard PCB address is 3F, I2C frequency is 400 kHz.

For ESPHome integration we need the laser sensor module only.

Pinout of the FFC (left to right):
1. VCC (3.3V)
2. VCC (3.3V)
3. VCC (3.3V)
4. GND
5. GND
6. GND
7.
8.
9.
10. I2C SDA
11. I2C SCL
12. display backlight
13. Tx (not used)
14. Rx (not used)
15. 
16. Signal for buzzer
17.
18. Keyboard
19. Power button - when pressed, set to 0.33V
20. Needs to be set to 1

## Usage
[See here for how to use external components](https://esphome.io/components/external_components.html).
See the example configuration file.


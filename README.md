# esphome-parkside-plem-50-c3
Parkside PLEM 50 C3 laser range finder I2C Integration for ESPHome

Work in progress, this is not a functional module.

## Description
Parkside PLEM 50 C3 is a cheap (around €30) laser finder. I find this one very useful for monitoring water levels in water tanks and drainages.

The device consist of laser sensor module and keyboard PCB + display module. Laser module is connected to keyboard with 20-conductor 0.5mm FFC cable.  
Unlike the C2 model, whole logic is in laser module. Laser module continuously send the display state to keyboard/display PCB over I2C. Laser module is master in this communication.  
Keyboard PCB address is 3F, I2C frequency is 400 kHz.

For ESPHome integration we need the laser sensor module only.

This module does NOT work with ESP8266 - it's unable to work as I2C slave.  The configuration was developed with ESP32CAM module.

## Usage
[See here for how to use external components](https://esphome.io/components/external_components.html).
See the example configuration file.

To work properly, the range finder must be set to metric system (using a Unit key). Processing of feet/inc is not implemented.  

## Connection
The laser module cannot be powered all the time. If we do that, it just won't respond.  
However, it cannot be powered by GPIO directly - the current is too high. So we need to switch it somehow. I have used the 2301 P-MOSFET transistor, which is on the keyboard PCB. But you can use whichever suitable switch/relay module you like.  

We need to connect the following wires:
1-3 - laser module power. Connect it to the transistor Drain.
Connect transistor Source to 3.3V.  
Connect transistor Gate to GPIO, parameter pin_laser_power.  

4-6 GND  
10 connect to GPIO, parameter pin_sda.  
11 connect to GPIO, parameter pin_scl.  
18 connect to GPIO, parameter pin_keyboard.  
19 connect to GPIO, parameter pin_power_button.  

## Developer notes
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
19. Power button - when pressed, set to 0.33V. Set to 0V works, too.
20. Reference voltage for the keyboard?

Keyboard (at reference 3.38V):  
0V Measure  
1.12V Functíons (volumes, Pythagoras...)  
2.67V Beep on/off                  
3.02V Reference button  
1.52V Continuous measurement         
2.56V Memory                         
2.77V Unit                           
2.35V Clear                          
1.69V +/-  


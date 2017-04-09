# wifi-flash-trigger
ESP8266 + Arduino Wifi Flash trigger for high speed photography.  Uses esp-link as the wifi link to arduino

## Download esp-link
It’s here: https://github.com/jeelabs/esp-link/releases.


## Firmware versions:
esp-link: This seems to only work with esp-link v3.0.alpha0 - 2016-10-23, d0377bd.  https://github.com/jeelabs/esp-link
el-client: https://github.com/jeelabs/el-client (d559214ada405739fa235b13c7e2b42175ddbe95 tested)

## Flash the ESP8266
Flash to the esp8266: as per these instructions: https://github.com/jeelabs/esp-link/blob/master/FLASHING.md

esptool.py --port /dev/ttyUSB0 --baud 230400 write_flash -fs 32m -ff 80m 0x00000 boot_v1.6.bin 0x1000 user1.bin 0x3FC000 esp_init_data_default.bin 0x3FE000 blank.bin


## Warning
The esp-link in the webserver_controls.ino has debugging enabled

```ELClient esp(&Serial, &Serial);```

This totally breaks things. Be sure to have debugging disabled if you wish for things to work
```ELClient esp(&Serial);```




## Cofigure the esp-link
Connect to 192.168.4.1 and set the settings to this:


|RESET     | gpio12   |
|----------|----------|
|ISP/Flash | disabled |
|Conn LED  | disabled |
|Serial LED| GPIO14   |
|UART Pins | swapped  |

![flash-trigger_bb](https://cloud.githubusercontent.com/assets/286973/24841090/76d5db74-1d30-11e7-9619-d6fc9f7f678d.png)

## connect up the arduino:
5 wires:  BE SURE TO USE A 3.3V ARDUINO, or use a level shifter

|NAME    |NODEMCU PIN | ESP8266 pin | ARDUINO pin |      |
|--------|--------|---------|---------|------|
|        |GND     | G       |         | GND  |
|        |VCC     | 3V      |         | VCC  |
|        |RESET   | D6      | GPIO12  | RST  |
|        |ESPRX   | D7      | GPIO13  | TX   |
|        |ESPTX   | D8      | GPIO15  | RX   |

## Flash the arduino
Be sure that nothing else (like a web browser) is accessing 192.168.4.1, or avrdude will break.

```
$ avrdude -p m328p -c stk500v1 -e -U flash:w:.pioenvs/pro8MHzatmega328/firmware.hex -P net:192.168.4.1:23

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.01s

avrdude: Device signature = 0x1e950f (probably m328p)
avrdude: erasing chip
avrdude: reading input file ".pioenvs/pro8MHzatmega328/firmware.hex"
avrdude: input file .pioenvs/pro8MHzatmega328/firmware.hex auto detected as Intel Hex
avrdude: writing flash (1592 bytes):

Writing | ################################################## | 100% 0.48s

avrdude: 1592 bytes of flash written
avrdude: verifying flash memory against .pioenvs/pro8MHzatmega328/firmware.hex:
avrdude: load data flash data from input file .pioenvs/pro8MHzatmega328/firmware.hex:
avrdude: input file .pioenvs/pro8MHzatmega328/firmware.hex auto detected as Intel Hex
avrdude: input file .pioenvs/pro8MHzatmega328/firmware.hex contains 1592 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.93s

avrdude: verifying ...
avrdude: 1592 bytes of flash verified

avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.

```


Sometimes when it fails with
'not in sync'

```
ioctl("TIOCMGET"): Inappropriate ioctl for device
ioctl("TIOCMGET"): Inappropriate ioctl for device
avrdude: stk500_getsync() attempt 1 of 10: not in sync: resp=0x45
avrdude: stk500_getsync() attempt 2 of 10: not in sync: resp=0x4c
avrdude: stk500_getsync() attempt 3 of 10: not in sync: resp=0x2d
avrdude: stk500_getsync() attempt 4 of 10: not in sync: resp=0x43
avrdude: stk500_getsync() attempt 5 of 10: not in sync: resp=0x6c
avrdude: stk500_getsync() attempt 6 of 10: not in sync: resp=0x69
avrdude: stk500_getsync() attempt 7 of 10: not in sync: resp=0x65
avrdude: stk500_getsync() attempt 8 of 10: not in sync: resp=0x6e
avrdude: stk500_getsync() attempt 9 of 10: not in sync: resp=0x74
avrdude: stk500_getsync() attempt 10 of 10: not in sync: resp=0x20
```

this can help:

```../../esp-link-v3.0.14-g963ffbb/avrflash  192.168.4.1 .pioenvs/pro8MHzatmega328/firmware.hex ```


## Troubleshooting
* Before flashing the AVR, be sure to set the uC console speed in the web page to 57600
* After flashing the AVR, set console speed to 38400 (for 3.3v AVR)
* When flashing the AVR, be sure to close all web pages accessing the esp8266
* When using a 3.3V AVR, be sure that your serial port speed is set to 38400 or lower.
* After flashing the AVR power cycle the ESP8266.
* Then you should be able to see the uC RESET connect to the ESP in the console window.

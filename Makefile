SHELL := /bin/bash
esp_ip = 192.168.4.1

normal_baud = 38400
flash_baud  = 57600

.PHONY: arduino_code arduino_flash esp_flash all flash

all:

flash: arduino_flash

arduino_code:
	cd platformio && platformio run

arduino_flash:  arduino_code
	curl "http://$(esp_ip)/console/baud?rate=$(flash_baud)" -o /dev/null # Switch esp-link to 57600
	echo "Close all http connections(i.e. web pages) that connect to $(esp_ip)"
	@read -n 1 -p "Press enter when all windows are closed: "
	avrdude -DV -p m328p -c arduino -e -U flash:w:platformio/.pioenvs/pro8MHzatmega328/firmware.hex -P net:$(esp_ip):23
	curl "http://$(esp_ip)/console/baud?rate=$(normal_baud)" -o /dev/null # Switch esp-link back
	@read -n 1 -p "Hopefully that reprogramming finished.  Power cycle the ESP8266 and press enter to continue : "

esp_flash:
	echo "Flashing the ESP8266"
	cd esp-link-v3.0.alpha0 && esptool.py --port /dev/ttyUSB0 --baud 230400 write_flash -fs 32m -ff 80m 0x00000 boot_v1.6.bin 0x1000 user1.bin 0x3FC000 esp_init_data_default.bin 0x3FE000 blank.bin

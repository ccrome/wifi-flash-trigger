Flash trigger program

compile with:

platformio run

flash with
  $ avrdude -p m328p -c stk500v1 -e -U flash:w:.pioenvs/pro8MHzatmega328/firmware.hex -P net:192.168.4.1:23

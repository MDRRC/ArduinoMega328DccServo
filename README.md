# ArduinoMega328DccServo

A DCC decoder for 8 servo's . Servo address and positions can be changed using CV's.
It uses an Atmel Mega328P processor running at 16MHz. 
The processor itself must be programmed with a Arduino bootloader.

![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/servodecoder.PNG) 
 

## Programmning
 * Make sure the Atmel processor has the Arduino bootloader in it.
 * Connect a USB serial to TTL converter to  
 * Press the reset button and keep it pressed.
 * Press uoload in the IDE
 * Release the reset button
 * Hex file is uploaded.
 
 At first time power on the default CV value are set. The le dflashes fast, after CV's are written a reset will be performed. 

## License
NONE! Feel free to use / change the source code, schematic and PCB data.
PCB's are NOT sold, so if you want a PCB just order it. 
If you use this decoder it's on OWN risk!!! 

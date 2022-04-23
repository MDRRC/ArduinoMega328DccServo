# ArduinoMega328DccServo

A DCC decoder for 8 servo's. Servo address and positions can be changed using CV's.
It uses an Atmel Mega328P processor running at 16MHz. 
The processor itself must be programmed with a Arduino bootloader.

![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/servodecoder.PNG) 
 
The decoder uses the [BMRA DCC Library](https://www.arduino.cc/reference/en/libraries/nmradcc/) and the design is based on the examples. 

## Programmning
 * Make sure the Atmel processor has the Arduino bootloader in it.
 * Connect a USB serial to TTL converter to  
 * Press the reset button and keep it pressed.
 * Press uoload in the IDE
 * Release the reset button
 * Hex file is uploaded.
 * Change CV's according the CV numbers defined in FactoryDefaultCVs[] 
 
 At first time power on the default CV value are set. The led flashes fast, after CV's are written a reset will be performed. 
 When wrtiign CV value 8 a all CV valuesare writtne to there default values.  

## License
NONE! Feel free to use / change the source code, schematic and PCB data.
PCB's are NOT sold, so if you want a PCB just order the PCB's yourself. 
If you use this decoder it's on OWN risk!!! 

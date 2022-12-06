# ArduinoMega328DccServo

A(nother) Arduino based DCC decoder for 8 servo's. Servo address and positions can be changed using CV's or by a terminal program.
It uses an Atmel Mega328P processor running at 16MHz. 
The processor itself must be programmed with a Arduino bootloader and the fuses must be set to external xtal.

![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/servodecoder.PNG) 
 
The decoder uses the [NMRA DCC Library](https://www.arduino.cc/reference/en/libraries/nmradcc/) and the design is based on the examples.
Adress range is limited to 255.  

## Programming
 * Make sure the Atmel processor has the Arduino bootloader in it.
 * Connect a USB serial to TTL converter to X1.
 * Press the reset button and keep it pressed.
 * Press upload in the IDE.
 * Release the reset button.
 * Hex file is uploaded.
 * Change CV's according the CV numbers defined in FactoryDefaultCVs[] 
 
At first time power on the default CV values are set. The led flashes fast, after CV's are written a reset will be performed. 
When writing CV value 8 all CV values are written to there default values.  

## Connecting 
 * Supply voltage 9V to X2.
 * DCC signal to X20.
 * Servo's to Sv1 .. SV7 (servo connection is signal, supply voltage, ground).

## Simple console
To list or chamge CV values 
 * Connect a serial terminal and set baudrate to 57600
 * Pull pin 7 (CONS in schematic)m to ground using jumper JP1
 * To change a CV value enter S xx yy <ENTER> where xx the CV nummer is and yy the value.

![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/cvset.PNG)

 * If the entered command is not ok message CV command entry not ok!! will appear.
 
![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/cvnok.PNG)
 
 * To list the CV values press L <ENTER> 

![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/cvlist.PNG)

 * Press H <ENTER> for minimal help screen.

![](https://github.com/MDRRC/ArduinoMega328DccServo/blob/main/Hardware/help.PNG)

## CV programming (programming track)
 * Connect the decoder to a programming track.
 * Enter CV number and CV value.
 * The servo will move to the new minimal or maximum positions when changed. 

## POM programming 
Test only with [MDRRC-II](https://robertdotevers.wordpress.com/134-2/) Menu TO -> POM when MDRRC-II is on.
 * Connect the decoder to the main track (no need for programming track). 
   * Enter the address of the servo to be changed. 
   * Enter CV number 1 to change the address.
   * Enter CV Number 2 to change the minimum angle of the servo
   * Enter CV number 3 to change the maximum angle of the servo.
 * Enter the value 

Yes, you do not need to enter the CV values as shown in an image above. Just 1, 2, or 3.
 
## License
NONE! Feel free to use / change the source code, schematic and PCB data.
PCB's are NOT sold, so if you want a PCB just order the PCB's yourself. 
If you use this decoder it's on OWN risk!!! 

#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2022-04-17 20:55:31

#include "Arduino.h"
#define NUMSERVOS 8
#define SERVOSPEED 50
#define SERVO_DETACH_CNT 1000 / SERVOSPEED
#define RUN_LED_TIME 1000
#define RUN_LED_PIN 4
#include <DCC_Decoder.h>
#include <Servo.h>

void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data) ;
void RunLed() ;
void setup() ;
void loop() ;

#include "DccServoMega328P.ino"


#endif

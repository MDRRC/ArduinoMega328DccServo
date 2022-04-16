#define NUMSERVOS   8 // Enter the number of servos here
#define SERVOSPEED  50 // [ms] between servo updates, lower is faster
#define SERVO_DETACH_CNT  1000 / SERVOSPEED

// GO TO setup() TO CONFIGURE DCC ADDRESSES, PIN NUMBERS, SERVO ANGLES

#include <DCC_Decoder.h>
#include <Servo.h>

unsigned long timetomove;

typedef struct  {
  int   address;   // User Configurable DCC address
  byte  dccstate;  // Internal use DCC state of accessory: 1=on, 0=off
  byte  angle;     // Internal use current angle of servo
  byte  setpoint;  // Internal use destination angle of servo
  byte  offangle;  // User Configurable servo angle for DCC state = 0
  byte  onangle;   // User Configurable servo angle for DCC state = 1
  byte  servoPin;
  byte  detachcnt;
  Servo servo;
} DCCAccessoryData;

DCCAccessoryData servo[NUMSERVOS];

void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data) {
  address -= 1;
  address *= 4;
  address += 1;
  address += (data & 0x06) >> 1;
  // address = address - 4 // uncomment this line for Roco Maus or Z21
  boolean enable = (data & 0x01) ? 1 : 0;
  for (int i = 0; i < NUMSERVOS; i++) {
    if (address == servo[i].address) {
      if (servo[1].servo.attached() == false)
      {
        servo[i].servo.attach(servo[i].servoPin);
      }
      if (enable)
      { 
        servo[i].dccstate = 1;
servo[i].setpoint = servo[i].onangle;
      }
      else
      { 
                servo[i].dccstate = 0;
        servo[i].setpoint = servo[i].offangle;

      }

      servo[i].detachcnt = 0;
    }
  }
}

void setup() {
  // CONFIGURATION OF SERVOS
  // Copy & Paste as many times as you have servos
  // The amount must be same as NUMSERVOS
  // Don't forget to increment the array index

  //SV1
  servo[0].address   =   1; //DCC address
  servo[0].servoPin  =   8  ;
  servo[0].servo.attach( 8 ); // Arduino servo pin
  servo[0].offangle  =  90 ; // Servo angle for DCC state = 0
  servo[0].onangle   = 200 ; // Servo angle for DCC state = 1
  servo[0].detachcnt = 0;

  //SV2
  servo[1].address   =   2 ; // This is an accessory without servo
  servo[1].servoPin  =   9;
  servo[1].servo.attach( 9); // Arduino servo pin
  servo[1].offangle  =  70 ; // Servo angle for DCC state = 0
  servo[1].onangle   = 210 ; // Servo angle for DCC state = 1
  servo[1].detachcnt = 0;

  //SV3 
  servo[2].address   =   3;
  servo[2].servoPin  =   10;
  servo[2].servo.attach(10); // Arduino servo pin
  servo[2].offangle  =  13 ; // Servo angle for DCC state = 0
  servo[2].onangle   = 0 ; // Servo angle for DCC state = 1
  servo[2].detachcnt = 0;

  //SV4
  servo[3].address   =  4 ;
  servo[3].servoPin  =   11 ;
  servo[3].servo.attach( 11); // Arduino servo pin
  servo[3].offangle  =  14 ; // Servo angle for DCC state = 0
  servo[3].onangle   = 0 ; // Servo angle for DCC state = 1
  servo[3].detachcnt = 0;

  //SV5
  servo[4].address   =   5 ;
  servo[4].servoPin  =  12 ;
  servo[4].servo.attach(12); // Arduino servo pin
  servo[4].offangle  =  5 ; // Servo angle for DCC state = 0
  servo[4].onangle   = 25 ; // Servo angle for DCC state = 1
  servo[4].detachcnt = 0;

  //SV6
  servo[5].address   =   6 ;
  servo[5].servoPin  =   13 ;
  servo[5].servo.attach(13); // Arduino servo pin
  servo[5].offangle  = 33; // Servo angle for DCC state = 0
  servo[5].onangle   = 12; // Servo angle for DCC state = 1
  servo[5].detachcnt = 0;

  //SV7
  servo[4].address   =   7 ;
  servo[4].servoPin  =  18 ;
  servo[4].servo.attach(18); // Arduino servo pin
  servo[4].offangle  =  5 ; // Servo angle for DCC state = 0
  servo[4].onangle   = 25 ; // Servo angle for DCC state = 1
  servo[4].detachcnt = 0;

  //SV8
  servo[5].address   =   8 ;
  servo[5].servoPin  =   19 ;
  servo[5].servo.attach(19); // Arduino servo pin
  servo[5].offangle  = 33; // Servo angle for DCC state = 0
  servo[5].onangle   = 12; // Servo angle for DCC state = 1
  servo[5].detachcnt = 0;

  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC.SetupDecoder( 0x00, 0x00, 0 );

  for (byte i = 0; i < NUMSERVOS; i++) {
    servo[i].angle = servo[i].offangle;
    servo[i].servo.write(servo[i].angle);
    servo[i].dccstate = 0;
    servo[i].setpoint = servo[i].angle;
  }
}

void loop() {

  DCC.loop(); // Call to library function that reads the DCC data

  // Move the servos when it is timetomove
  if (millis() > timetomove) {
    timetomove = millis() + (unsigned long)SERVOSPEED;
    for (byte i = 0; i < NUMSERVOS; i++) {
      // Move servo to position if not in position.
      if (servo[i].angle < servo[i].setpoint)
      {
        servo[i].angle++;
      }
      else if (servo[i].angle > servo[i].setpoint)
      {
        servo[i].angle--;
      }

      if (servo[i].angle != servo[i].setpoint)
      {
        servo[i].servo.write(servo[i].angle);
      }
      else
      {
        // Servo in position, switch off.
        servo[i].detachcnt++;
        if (servo[i].detachcnt >= SERVO_DETACH_CNT)
        {
          servo[i].detachcnt = SERVO_DETACH_CNT;
          if (servo[i].servo.attached())
          {

            servo[i].servo.detach();
            digitalWrite(servo[i].servoPin, HIGH);
          }
        }
      }
    }
  }
}

#define NUMSERVOS   6 // Enter the number of servos here
#define SERVOSPEED  100 // [ms] between servo updates, lower is faster

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
      { servo[i].dccstate = 1;
      }
      else
      { servo[i].dccstate = 0;
      }
    }
  }
}

void setup() {
  // CONFIGURATION OF SERVOS
  // Copy & Paste as many times as you have servos
  // The amount must be same as NUMSERVOS
  // Don't forget to increment the array index

  //SV6
  servo[0].address   =   6 ; // DCC address
  servo[0].servoPin  =   11 ;
  servo[0].servo.attach( 11); // Arduino servo pin
  servo[0].offangle  =  90 ; // Servo angle for DCC state = 0
  servo[0].onangle   = 200 ; // Servo angle for DCC state = 1

  //SV5
  servo[1].address   =   5 ; // This is an accessory without servo
  servo[1].servoPin  =   7  ;
  servo[1].servo.attach( 7 ); // Arduino servo pin
  servo[1].offangle  =  70 ; // Servo angle for DCC state = 0
  servo[1].onangle   = 210 ; // Servo angle for DCC state = 1

  //Sv4
  servo[2].address   =  4 ;
  servo[2].servoPin  =   6 ;
  servo[2].servo.attach( 6); // Arduino servo pin
  servo[2].offangle  =  60 ; // Servo angle for DCC state = 0
  servo[2].onangle   = 200 ; // Servo angle for DCC state = 1

  //SV3
  servo[3].address   =  3 ;
  servo[3].servoPin  =   5 ;
  servo[3].servo.attach( 5); // Arduino servo pin
  servo[3].offangle  =  70 ; // Servo angle for DCC state = 0
  servo[3].onangle   = 200 ; // Servo angle for DCC state = 1

  //SV2
  servo[4].address   =   2 ;
  servo[4].servoPin  =  4 ;
  servo[4].servo.attach( 4); // Arduino servo pin
  servo[4].offangle  =  90 ; // Servo angle for DCC state = 0
  servo[4].onangle   = 225 ; // Servo angle for DCC state = 1

  //SV1
  servo[5].address   =   1 ;
  servo[5].servoPin  =   3 ;
  servo[5].servo.attach(3); // Arduino servo pin
  servo[5].offangle  =  80 ; // Servo angle for DCC state = 0
  servo[5].onangle   = 120 ; // Servo angle for DCC state = 1


  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC.SetupDecoder( 0x00, 0x00, 0 );

  pinMode(4, OUTPUT); 
  digitalWrite(4, LOW);

  for (byte i = 0; i < NUMSERVOS; i++) {
    servo[i].angle = servo[i].offangle;
    servo[i].servo.write(servo[i].angle);
    servo[i].dccstate = 0;
    servo[i].setpoint = servo[i].angle;
  }

   digitalWrite(4, HIGH);

   Serial.begin(57600);
   Serial.println("DCC Servo");
}

void loop() {

  DCC.loop(); // Call to library function that reads the DCC data

  for (byte i = 0; i < NUMSERVOS; i++) {
    if (servo[i].dccstate == 1) {
      servo[i].setpoint = servo[i].onangle;
    }
    else {
      servo[i].setpoint = servo[i].offangle;
    }
  }

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

                Serial.print("Angle : ");
          Serial.print(i);
          Serial.print(" ");
          Serial.print(servo[i].angle);
          Serial.print(" ");

      if (servo[i].angle != servo[i].setpoint)
      {
        servo[i].servo.write(servo[i].angle);
      }
      else
      {
          /*Serial.print("SP : ");
          Serial.print(i);
          Serial.print(" ");
          Serial.print(servo[i].setpoint);
          Serial.print(" ");
          Serial.print(servo[i].angle);
          Serial.print(" ");*/

        if (servo[i].setpoint == servo[i].onangle)
        {
          servo[i].setpoint = servo[i].offangle;
          Serial.print("Set : ");
          Serial.println(servo[i].setpoint);

          /*Serial.print("Off : ");
          Serial.print(i);
          Serial.print(" ");
          Serial.println(servo[i].setpoint);*/
        }
        else
        {
          servo[i].setpoint = servo[i].onangle;
          Serial.print("Set : ");
          Serial.println(servo[i].setpoint);

          /*Serial.print("On : ");
          Serial.print(i);
          Serial.print(" ");
          Serial.println(servo[i].setpoint);*/
        }
        // Servo in position, switch off.
        /*if (servo[i].servo.attached())
        {
          servo[i].servo.detach();
          digitalWrite(servo[i].servoPin, HIGH);
        }*/
      }
    }
  }
}

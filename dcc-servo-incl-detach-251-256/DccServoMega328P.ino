#define NUMSERVOS 8    // Enter the number of servos here
#define SERVOSPEED 50  // [ms] between servo updates, lower is faster
#define SERVO_DETACH_CNT 1000 / SERVOSPEED
#define RUN_LED_TIME 1000
#define RUN_LED_PIN 4

// GO TO setup() TO CONFIGURE DCC ADDRESSES, PIN NUMBERS, SERVO ANGLES

#include <DCC_Decoder.h>
#include <Servo.h>

unsigned long timetomove;

typedef struct {
  int address;    // User Configurable DCC address
  byte angle;     // Internal use current angle of servo
  byte setpoint;  // Internal use destination angle of servo
  byte offangle;  // User Configurable servo angle for DCC state = 0
  byte onangle;   // User Configurable servo angle for DCC state = 1
  byte servoPin;
  byte detachcnt;
  Servo servo;
} DCCAccessoryData;

typedef struct {
  int address;  // User Configurable DCC address
  byte servoPin;
  byte offangle;  // User Configurable servo angle for DCC state = 0
  byte onangle;   // User Configurable servo angle for DCC state = 1
} DCCAccessoryDataStorage;

DCCAccessoryDataStorage DccAccData[NUMSERVOS] = {
    {1, 8, 90, 200}, {2, 9, 60, 210}, {3, 10, 13, 0}, {4, 11, 14, 0},
    {5, 12, 5, 25},  {6, 14, 33, 12}, {7, 18, 5, 25}, {8, 19, 33, 12}};

DCCAccessoryData servo[NUMSERVOS];
uint32_t RunLedTimer;
bool RunLedLevel;

void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data) {
  address -= 1;
  address *= 4;
  address += 1;
  address += (data & 0x06) >> 1;
  // address = address - 4 // uncomment this line for Roco Maus or Z21
  boolean enable = (data & 0x01) ? 1 : 0;
  for (int i = 0; i < NUMSERVOS; i++) {
    if (address == servo[i].address) {
      if (servo[1].servo.attached() == false) {
        servo[i].servo.attach(servo[i].servoPin);
      }
      if (enable) {
        servo[i].setpoint = servo[i].onangle;
      } else {
        servo[i].setpoint = servo[i].offangle;
      }

      servo[i].detachcnt = 0;
    }
  }
}

void RunLed() {
  if (millis() >= (RunLedTimer + RUN_LED_TIME)) {
    RunLedTimer = millis();

    if (RunLedLevel == false) {
      RunLedLevel = true;
      digitalWrite(RUN_LED_PIN, HIGH);
    } else {
      RunLedLevel = false;
      digitalWrite(RUN_LED_PIN, LOW);
    }
  }
}

void setup() {
  // CONFIGURATION OF SERVOS
  // Copy & Paste as many times as you have servos
  // The amount must be same as NUMSERVOS
  // Don't forget to increment the array index

  RunLedLevel = true;
  RunLedTimer = millis();
  pinMode(RUN_LED_PIN, OUTPUT);
  digitalWrite(RUN_LED_PIN, HIGH);

  for (byte i = 0; i < NUMSERVOS; i++) {
    servo[i].address = DccAccData[i].address;
    servo[i].servoPin = DccAccData[i].servoPin;
    servo[i].angle = DccAccData[i].offangle;
    servo[i].servo.write(servo[i].angle);
    servo[i].setpoint = servo[i].angle;
    servo[i].onangle = DccAccData[i].onangle;
    servo[i].offangle = DccAccData[i].offangle;

    servo[0].servo.attach(DccAccData[i].servoPin);
  }

  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler,
                                            true);
  DCC.SetupDecoder(0x00, 0x00, 0);
}

void loop() {
  // Call to library function that reads the DCC data
  DCC.loop();
  RunLed();

  // Move the servos when it is timetomove
  if (millis() > timetomove) {
    timetomove = millis() + (unsigned long)SERVOSPEED;
    for (byte i = 0; i < NUMSERVOS; i++) {
      DCC.loop();

      // Move servo to position if not in position.
      if (servo[i].angle < servo[i].setpoint) {
        servo[i].angle++;
      } else if (servo[i].angle > servo[i].setpoint) {
        servo[i].angle--;
      }

      if (servo[i].angle != servo[i].setpoint) {
        servo[i].servo.write(servo[i].angle);
      } else {
        // Servo in position, switch off.
        servo[i].detachcnt++;
        if (servo[i].detachcnt >= SERVO_DETACH_CNT) {
          servo[i].detachcnt = SERVO_DETACH_CNT;
          if (servo[i].servo.attached()) {
            servo[i].servo.detach();
            digitalWrite(servo[i].servoPin, HIGH);
          }
        }
      }
    }
  }
}

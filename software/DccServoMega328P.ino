/***********************************************************************************************************************
   I N C L U D E S
 **********************************************************************************************************************/
#include <NmraDcc.h>
#include <Servo.h>

/***********************************************************************************************************************
   D E F I N E S
 **********************************************************************************************************************/
#define NUMSERVOS 8   // Enter the number of servos here
#define SERVOSPEED 50 // [ms] between servo updates, lower is faster
#define SERVO_DETACH_CNT 500 / SERVOSPEED
#define RUN_LED_TIME 1000
#define RUN_LED_PIN 4

#define CV_SERVO_DATA_START 30
#define CV_DECODER_MASTER_RESET 120

#define ENABLE_DEBUG 0

/**
 * Typedef struct for servo data.
 */

typedef struct
{
    uint16_t address; // User Configurable DCC address
    byte angle;       // Internal use current angle of servo
    byte setpoint;    // Internal use destination angle of servo
    byte minangle;    // User Configurable servo angle for DCC state = 0
    byte maxangle;    // User Configurable servo angle for DCC state = 1
    byte servoPin;
    byte detachcnt;
    Servo servo;
} DCCAccessoryData;

/**
 * Typedef struct for CV data storage.
 */
struct CVPair
{
    uint16_t CV;
    uint8_t Value;
};

/***********************************************************************************************************************
   D A T A   D E C L A R A T I O N S (exported, local)
 **********************************************************************************************************************/

// clang-format off
CVPair FactoryDefaultCVs [] =
{
  {CV_ACCESSORY_DECODER_ADDRESS_LSB, 1},
  {CV_ACCESSORY_DECODER_ADDRESS_MSB, 0},
  {CV_DECODER_MASTER_RESET, 0},
  {CV_SERVO_DATA_START,  1},  	// Acc decoder number 1
  {31,  2}, 	// Servo 1 position minimum.
  {32,120}, 	// Servo 1 position maximum.
  {33,  8}, 	// Servo 1 pin.

  {34,	2},  	// Acc decoder number 2
  {35,	2}, 	// Servo 2 position minimum.
  {36,120}, 	// Servo 2 position maximum.
  {37,  9}, 	// Servo 2 pin.

  {38,	3},  	// Acc decoder number 3
  {39,	2}, 	// Servo 3 position minimum.
  {40,120}, 	// Servo 3 position maximum.
  {41, 10}, 	// Servo 3 pin.

  {42,	4},  	// Acc decoder number 4
  {43,	2}, 	// Servo 4 position minimum.
  {44,120}, 	// Servo 4 position maximum.
  {45, 11}, 	// Servo 4 pin.

  {46,	5},  	// Acc decoder number 5
  {47,	2}, 	// Servo 5 position minimum.
  {48,120}, 	// Servo 5 position maximum.
  {49, 12}, 	// Servo 5 pin.

  {50,	6},  	// Acc decoder number 6
  {51,	2}, 	// Servo 6 position minimum.
  {52,120}, 	// Servo 6 position maximum.
  {53, 14}, 	// Servo 6 pin.

  {54,	7},  	// Acc decoder number 7
  {55,	2}, 	// Servo 7 position minimum.
  {56,120}, 	// Servo 7 position maximum.
  {57, 18}, 	// Servo 7 pin.

  {58,	8},  	// Acc decoder number 8
  {59,	2}, 	// Servo 8 position minimum.
  {60,120}, 	// Servo 8 position maximum.
  {61, 19}, 	// Servo 8 pin.
};
// clang-format on

bool RunLedLevel                = true;
bool PerformResetFactoryDefault = false;
uint32_t RunLedTimer            = millis();
unsigned long timetomove        = 0;
uint8_t FactoryDefaultCVIndex   = 0;

DCCAccessoryData servo[NUMSERVOS];
NmraDcc Dcc;
DCC_MSG Packet;

void (*ResetFunc)(void) = 0;

/***********************************************************************************************************************
 * Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset to flag to the loop() function that a
 * reset to Factory Defaults needs to be done
 */
void notifyCVResetFactoryDefault()
{
    FactoryDefaultCVIndex      = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
    PerformResetFactoryDefault = true;
}

/***********************************************************************************************************************
 */
void notifyDccAccState(uint16_t Addr, uint16_t BoardAddr, uint8_t OutputAddr, uint8_t State)
{
    State          = State;
    BoardAddr      = BoardAddr;
    boolean Enable = false;
    boolean Found  = false;
    uint8_t Index  = 0;

    switch (OutputAddr)
    {
    case 1:
    case 3:
    case 5:
    case 7: Enable = true; break;
    default: break;
    }

    while ((Found == false) && (Index < NUMSERVOS))
    {
        if (Addr == servo[Index].address)
        {
            if (servo[Index].servo.attached() == false)
            {
                servo[Index].servo.attach(servo[Index].servoPin);
            }
            if (Enable)
            {
                servo[Index].setpoint = servo[Index].maxangle;
            }
            else
            {
                servo[Index].setpoint = servo[Index].minangle;
            }
            Found                  = true;
            servo[Index].detachcnt = 0;
        }
        else
        {
            Index++;
        }
    }
}

/***********************************************************************************************************************
 */
void RunLed()
{
    if (millis() >= (RunLedTimer + RUN_LED_TIME))
    {
        RunLedTimer = millis();

        if (RunLedLevel == false)
        {
            RunLedLevel = true;
            digitalWrite(RUN_LED_PIN, HIGH);
        }
        else
        {
            RunLedLevel = false;
            digitalWrite(RUN_LED_PIN, LOW);
        }
    }
}

/***********************************************************************************************************************
 * If a Cv value changed update data and when the min or max position is changed start moving servo to the new position.
 */
void CheckForCvChanges()
{
    uint8_t Index;

    for (Index = 0; Index < NUMSERVOS; Index++)
    {
        // Address changed?
        if (servo[Index].address != Dcc.getCV(CV_SERVO_DATA_START + (Index * 4)))
        {
            servo[Index].address = Dcc.getCV(CV_SERVO_DATA_START + (Index * 4));
        }

        // Min position changed? If so move to new position.
        if (servo[Index].minangle != Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 1)))
        {
            servo[Index].minangle = Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 1));
            servo[Index].setpoint = servo[Index].minangle;
            servo[Index].servo.attach(servo[Index].servoPin);
            servo[Index].detachcnt = 0;
        }

        // Max position changed? If so move to new position.
        if (servo[Index].maxangle != Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 2)))
        {
            servo[Index].maxangle = Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 2));
            servo[Index].setpoint = servo[Index].maxangle;
            servo[Index].servo.attach(servo[Index].servoPin);
            servo[Index].detachcnt = 0;
        }
    }
}

/***********************************************************************************************************************
 */
void setup()
{
    uint8_t Index;

    RunLedTimer = millis();
    pinMode(RUN_LED_PIN, OUTPUT);
    digitalWrite(RUN_LED_PIN, HIGH);

    for (Index = 0; Index < NUMSERVOS; Index++)
    {
        servo[Index].address  = Dcc.getCV(CV_SERVO_DATA_START + (Index * 4));
        servo[Index].minangle = Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 1));
        servo[Index].maxangle = Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 2));
        servo[Index].servoPin = Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 3));
        servo[Index].angle    = servo[Index].maxangle;
        servo[Index].setpoint = servo[Index].angle;
        servo[Index].servo.write(servo[Index].angle);
        servo[Index].servo.attach(servo[Index].servoPin);

        // After initial power on EEPROM contains 255, write CV values into EEPROM.
        // This prohibits the use of address 255.
        if (servo[Index].address == 255)
        {
            notifyCVResetFactoryDefault();
        }
    }

    // Init DCC module.
    Dcc.pin(0, 2, 1);
    Dcc.init(MAN_ID_DIY, 1, FLAGS_OUTPUT_ADDRESS_MODE | FLAGS_DCC_ACCESSORY_DECODER, 0);
}

/***********************************************************************************************************************
 */
void loop()
{
    // Call to library function that reads the DCC data
    Dcc.process();

    if (FactoryDefaultCVIndex && Dcc.isSetCVReady())
    {
        FactoryDefaultCVIndex--; // Decrement first as initially it is the size of the array
        Dcc.setCV(FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);

        if (RunLedLevel == false)
        {
            RunLedLevel = true;
            digitalWrite(RUN_LED_PIN, HIGH);
        }
        else
        {
            RunLedLevel = false;
            digitalWrite(RUN_LED_PIN, LOW);
        }

        // Small delay so led blinks fast indicating cv reset is going on.
        delay(100);
    }
    else
    {
        if (PerformResetFactoryDefault == true)
        {
            // Reset to apply default Cv values.
            ResetFunc();
        }
        else
        {
            RunLed();
            CheckForCvChanges();
        }
    }

    // Move the servos when it is time to move
    if (millis() > timetomove)
    {
        timetomove = millis() + (unsigned long)SERVOSPEED;
        for (byte i = 0; i < NUMSERVOS; i++)
        {
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
                // Servo in position, switch off after some time.
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

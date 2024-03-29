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
#define ENABLE_CONSOLE_PIN 7
#define CONSOLE_BUFFER_RX_SIZE 30

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
  {32, 11}, 	// Servo 1 position maximum.
  {33,  8}, 	// Servo 1 pin.

  {34,	2},  	// Acc decoder number 2
  {35,	2}, 	// Servo 2 position minimum.
  {36, 10}, 	// Servo 2 position maximum.
  {37,  9}, 	// Servo 2 pin.

  {38,	3},  	// Acc decoder number 3
  {39,	2}, 	// Servo 3 position minimum.
  {40, 10}, 	// Servo 3 position maximum.
  {41, 10}, 	// Servo 3 pin.

  {42,	4},  	// Acc decoder number 4
  {43,	2}, 	// Servo 4 position minimum.
  {44, 10}, 	// Servo 4 position maximum.
  {45, 11}, 	// Servo 4 pin.

  {46,	5},  	// Acc decoder number 5
  {47,	2}, 	// Servo 5 position minimum.
  {48, 11}, 	// Servo 5 position maximum.
  {49, 12}, 	// Servo 5 pin.

  {50,	6},  	// Acc decoder number 6
  {51,	2}, 	// Servo 6 position minimum.
  {52, 11}, 	// Servo 6 position maximum.
  {53, 14}, 	// Servo 6 pin.

  {54,	7},  	// Acc decoder number 7
  {55,	2}, 	// Servo 7 position minimum.
  {56, 11}, 	// Servo 7 position maximum.
  {57, 18}, 	// Servo 7 pin.

  {58,	8},  	// Acc decoder number 8
  {59,	2}, 	// Servo 8 position minimum.
  {60, 11}, 	// Servo 8 position maximum.
  {61, 19}, 	// Servo 8 pin.
};
// clang-format on

bool RunLedLevel                = true;
bool PerformResetFactoryDefault = false;
bool ConsoleMode                = false;
uint32_t RunLedTimer            = millis();
unsigned long timetomove        = 0;
uint8_t FactoryDefaultCVIndex   = 0;
uint8_t RxBufferIndex           = 0;

NmraDcc Dcc;
DCC_MSG Packet;
DCCAccessoryData servo[NUMSERVOS];
char RxBufferSerial[CONSOLE_BUFFER_RX_SIZE];

/***********************************************************************************************************************
   F U N C T I O N S
 **********************************************************************************************************************/

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
 * Check if a POM (Programming On Main) message for acc. decoders is present.
 * 10AAAAAA 0 1AAACDDD 0 (1110CCVV 0 VVVVVVVV 0 DDDDDDDD) 0 EEEEEEEE 1
 * ONLY tested with MDRRC-II.
 */
void notifyDccMsg(DCC_MSG* Msg)
{
    uint16_t Address  = 0;
    uint16_t CvNumber = 0;
    uint8_t CvValue   = 0;
    uint8_t Temp      = 0;
    bool Found        = false;
    uint8_t Index     = 0;

    // Check if bytes contain POM command data.
    if (((Msg->Data[0] & 0b11000000) == 0b10000000) && ((Msg->Data[1] & 0b10000000) == 0b10000000))
    {
        if ((Msg->Data[2] & 0b11110000) == 0b11100000)
        {
            // Yep, get data.
            Address = (Msg->Data[0]) & 0b00111111;
            Temp    = Msg->Data[1] & 0b01110000;
            Temp ^= 0b01110000;
            Address |= (uint16_t(Temp << 2));

            Temp     = Msg->Data[2] & 0b00000011;
            CvNumber = (uint16_t)(Temp) << 8;
            CvNumber |= Msg->Data[3];
            CvNumber++;

            CvValue = Msg->Data[4];

            // Check if address is known, if so update CV value.
            while ((Found == false) && (Index < NUMSERVOS))
            {
                if (Address == servo[Index].address)
                {
                    switch (CvNumber)
                    {
                    case 1:
                        // Address
                        Serial.print(Address);
                        Dcc.setCV(CV_SERVO_DATA_START + (Index * 4), CvValue);
                        break;
                    case 2:
                        // Min angle
                        Dcc.setCV(CV_SERVO_DATA_START + ((Index * 4) + 1), CvValue);
                        break;
                    case 3:
                        // Max angle
                        Dcc.setCV(CV_SERVO_DATA_START + ((Index * 4) + 2), CvValue);
                        break;
                    }
                    Found = true;
                }
                else
                {
                    Index++;
                }
            }
        }
    }
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
static void ConsoleHelp()
{
    Serial.println("H       : Help");
    Serial.println("L       : List CV values");
    Serial.println("S xx yy : Set Cv xx to value yy");
}

/***********************************************************************************************************************
 */
static void ConsoleWriteWithSpaces(int Value)
{
    String text;
    int Length;
    uint8_t Index;

    text   = String(Value);
    Length = text.length();
    for (Index = 0; Index < (4 - Length); Index++)
    {
        Serial.print(" ");
    }
    Serial.print(text);
}

/***********************************************************************************************************************
 */
static void ConsoleListCvValues()
{
    uint8_t Index;

    Serial.println("CV Address  CV  Min   CV   Max  CV   Pin");
    Serial.println("----------------------------------------");

    for (Index = 0; Index < NUMSERVOS; Index++)
    {
        ConsoleWriteWithSpaces(CV_SERVO_DATA_START + (Index * 4));
        Serial.print(" ");
        ConsoleWriteWithSpaces(Dcc.getCV(CV_SERVO_DATA_START + (Index * 4)));
        Serial.print(" ");
        ConsoleWriteWithSpaces(CV_SERVO_DATA_START + ((Index * 4) + 1));
        Serial.print(" ");
        ConsoleWriteWithSpaces(Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 1)));
        Serial.print(" ");
        ConsoleWriteWithSpaces(CV_SERVO_DATA_START + ((Index * 4) + 2));
        Serial.print(" ");
        ConsoleWriteWithSpaces(Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 2)));
        Serial.print(" ");
        ConsoleWriteWithSpaces(CV_SERVO_DATA_START + ((Index * 4) + 3));
        Serial.print(" ");
        ConsoleWriteWithSpaces(Dcc.getCV(CV_SERVO_DATA_START + ((Index * 4) + 3)));
        Serial.println();
    }
}

/***********************************************************************************************************************
 */
static void ConsoleSetCvValue(char* DataPtr)
{
    int Number;
    int Value;
    char* SpacePtr;
    bool Succes = false;

    // Locate space
    SpacePtr = strchr(DataPtr, 32);
    if (SpacePtr != NULL)
    {
        // Get CV number
        SpacePtr++;
        Number = atoi(SpacePtr);

        // Get CV value
        SpacePtr = strchr(SpacePtr, 32);
        if (SpacePtr != NULL)
        {
            SpacePtr++;
            Value = atoi(SpacePtr);

            Dcc.setCV((uint16_t)(Number), (uint8_t)(Value));

            Serial.print("CV Number : ");
            Serial.print(Number);
            Serial.print(" set to : ");
            Serial.println(Value);

            Succes = true;
        }
    }

    if (Succes == false)
    {
        Serial.println("CV command entry not ok!!");
    }
}

/***********************************************************************************************************************
 */
static void Console(char ByteRx)
{
    // Bounce data
    Serial.print(ByteRx);

    // When enter not pressed fill buffer and check for overrun buffer.
    if (ByteRx != 0x0D)
    {
        RxBufferSerial[RxBufferIndex] = toUpperCase(ByteRx);
        RxBufferIndex++;

        if (RxBufferIndex >= CONSOLE_BUFFER_RX_SIZE)
        {
            RxBufferIndex = 0;
            memset(RxBufferSerial, 0, CONSOLE_BUFFER_RX_SIZE);
        }
    }
    else
    {
        // Enter pressed, check first entry in buffer.
        switch (RxBufferSerial[0])
        {
        case 'H':
            // List help screen.
            ConsoleHelp();
            break;
        case 'L':
            // List CV data.
            ConsoleListCvValues();
            break;
        case 'S':
            // Set CV value.
            ConsoleSetCvValue(RxBufferSerial);
            break;
        default: Serial.println("Unknown command..."); break;
        }

        RxBufferIndex = 0;
        memset(RxBufferSerial, 0, CONSOLE_BUFFER_RX_SIZE);
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

    // Init serial port and console related items.
    Serial.begin(57600);
    Serial.println("DccServoMega328P");

    pinMode(ENABLE_CONSOLE_PIN, INPUT);
    digitalWrite(ENABLE_CONSOLE_PIN, HIGH);

    RxBufferIndex = 0;
    memset(RxBufferSerial, 0, CONSOLE_BUFFER_RX_SIZE);
}

/***********************************************************************************************************************
 */
void loop()
{
    int ByteRx = 0;

    // Call to library function that reads the DCC data
    Dcc.process();

    if (digitalRead(ENABLE_CONSOLE_PIN) == LOW)
    {
        // Console active...
        ByteRx = Serial.read();
        if (ByteRx > 0)
        {
            // read the incoming byte:
            Console((char)(ByteRx));
        }
    }

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

    Dcc.process();

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

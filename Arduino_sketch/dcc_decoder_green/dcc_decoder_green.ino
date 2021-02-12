// for ATMEGA328P
#include <NmraDcc.h>

// Uncomment to force CV Reset to Factory Defaults
//#define RESET_FACTORY_DEFAULTS

// Uncomment any of the lines below to enable debug messages for different parts of the code
//#define DEBUG_FUNCTIONS
//#define DEBUG_SPEED
//#define DEBUG_PWM
//#define DEBUG_DCC_ACK
//#define DEBUG_DCC_MSG
//#define DEBUG_STATE

#if defined(DEBUG_FUNCTIONS) or defined(DEBUG_SPEED) or defined(DEBUG_PWM) or defined(DEBUG_DCC_ACK) or defined(DEBUG_DCC_MSG) or defined(DEBUG_STATE)
#define DEBUG_PRINT
#endif

#define DECODER_ADDRESS 3
#define LONG_ADDRESS 123

#define CV_VSTART 2
#define START_VOLTAGE 10 // 255 is full rectified voltage

#define CV_ACC_RATE 3
#define ACC_RATE 3 // value*0.896/speed_steps sec/step 3->2.688 sec to full speed

#define CV_DEC_RATE 4
#define DEC_RATE 3 // value*0.896/speed_steps sec/step 3->2.688 sec from full speed

#define CV_VHIGH 5
#define HIGH_VOLTAGE 255 // 255 is full rectified voltage

#define CV_VMID 6
#define MID_VOLTAGE 127 // 255 is full rectified voltage

#define VERSION 1           // CV 7

#define CV_PWM_PERIOD 9
#define PWM_PERIOD 1

#define CV_PACKET_TIMEOUT 11
#define PACKET_TIMEOUT 100 // in 10 ms
#define CV_UNLOCK_NUMBER 15
#define CV_LOCKING_NUMBER 16
#define CV_AUTO_STOP_CONFIG 27
#define CV_OUTPUT_LOCATION_F0f 33
#define CV_OUTPUT_LOCATION_F0r 34
#define CV_OUTPUT_LOCATION_F1 35
#define CV_OUTPUT_LOCATION_F2 36
#define CV_OUTPUT_LOCATION_F3 37
#define CV_OUTPUT_LOCATION_F4 38
#define CV_OUTPUT_LOCATION_F5 39
#define CV_OUTPUT_LOCATION_F6 40
#define CV_OUTPUT_LOCATION_F7 41
#define CV_OUTPUT_LOCATION_F8 42
#define CV_OUTPUT_LOCATION_F9 43
#define CV_OUTPUT_LOCATION_F10 44
#define CV_OUTPUT_LOCATION_F11 45
#define CV_OUTPUT_LOCATION_F12 46
#define CV_PRODUCT_ID 47  //Product ID: 47-50 (CV47<<24 + CV48<<16 + CV49<<8 + CV50)
#define CV_KICK_START 65
#define CV_FWD_TRIM 66
#define CV_SPEED_MAP 67 //67-94
#define CV_REV_TRIM 95
#define CV_USER_ID1 105
#define CV_USER_ID2 106

//todo
// function effects 57-61, dimming, inverting, 0-speed activate/deactivate
// SW_IN function mapping, minimum time
// overload amplitude, time

#define DCC_PIN       2
#define MOTOR_IN1_PIN 9
#define MOTOR_IN2_PIN 10
#define MOTOR_nFAULT_PIN 8
#define LED_F0f_PIN   5
#define LED_F0r_PIN   6
#define LED_AUX1_PIN  3
#define LED_AUX2_PIN  11
#define LED_AUX3_PIN  A0
#define LED_AUX4_PIN  A1
#define LED_AUX5_PIN  A2
#define LED_AUX6_PIN  A3
#define LED_AUX7_PIN  A4
#define LED_AUX8_PIN  A5
#define LED_GREEN_PIN 12
#define LED_YELLOW_PIN 13
#define LED_RED_PIN    7
#define LED_BLUE_PIN   4
#define DCC1_PK_PIN   A6
#define DCC2_PK_PIN   A7

NmraDcc dcc;

struct CVPair{
  uint16_t CV;
  uint8_t  Value;
};

CVPair factoryDefaultCVs [] =
{
  {CV_MULTIFUNCTION_PRIMARY_ADDRESS, DECODER_ADDRESS&0x7F },//CV1
  {CV_VSTART, START_VOLTAGE },//CV2
  {CV_ACC_RATE, ACC_RATE },//CV3
  {CV_DEC_RATE, DEC_RATE },//CV4
  {CV_VHIGH, HIGH_VOLTAGE },//CV5
  {CV_VMID, MID_VOLTAGE },//CV6, (CV7, CV8 implicit in dcc.init())
  {CV_PWM_PERIOD, PWM_PERIOD},//CV9
  {CV_PACKET_TIMEOUT, PACKET_TIMEOUT},//CV11
  {CV_UNLOCK_NUMBER, 0},
  {CV_LOCKING_NUMBER, 0},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_MSB, highByte(LONG_ADDRESS) + 0xC0},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_LSB, lowByte(LONG_ADDRESS)},
  {CV_AUTO_STOP_CONFIG, 0b00110011},//7,6: reserved, 5: DC+, 4: DC-, 3: unused, 2: Signal, 1: Asym left, 0: Asym right
  {CV_29_CONFIG, 0x00
  //| CV29_LOCO_DIR            // 0: normal direction, 1: reversed direction
  | CV29_F0_LOCATION           // 0: 14 speed steps, 1:28/128 speed steps
  //| CV29_APS                 // 0: only DCC, 1: analogue enabled
  //| CV29_ADV_ACK             // 0: Railcom disabled, 1: Railcom enabled
  //| CV29_SPEED_TABLE_ENABLE  // 0: simple speed curve (CV2, CV5, CV6)
#ifdef LONG_ADDRESS
  | CV29_EXT_ADDRESSING        // 0: one byte addressing, 1: two byte addressing
#endif
  //| CV29_OUTPUT_ADDRESS_MODE // 0: Decoder Address Mode, 1: Output Address Mode
  //| CV29_ACCESSORY_DECODER   // 0: Multi-Function Decoder Mode, 1: Accessory Decoder Mode
  },
  {CV_OUTPUT_LOCATION_F0f, 1},// 1:F0f, 2:F0r, 4:AUX1, 8:AUX2, 16:AUX3, 32:AUX4, 64:AUX5, 128:AUX6
  {CV_OUTPUT_LOCATION_F0r, 2},// 1:F0f, 2:F0r, 4:AUX1, 8:AUX2, 16:AUX3, 32:AUX4, 64:AUX5, 128:AUX6
  {CV_OUTPUT_LOCATION_F1, 4}, // 1:F0f, 2:F0r, 4:AUX1, 8:AUX2, 16:AUX3, 32:AUX4, 64:AUX5, 128:AUX6
  {CV_OUTPUT_LOCATION_F2, 8}, // 1:F0f, 2:F0r, 4:AUX1, 8:AUX2, 16:AUX3, 32:AUX4, 64:AUX5, 128:AUX6
  {CV_OUTPUT_LOCATION_F3, 16},// 1:F0f, 2:F0r, 4:AUX1, 8:AUX2, 16:AUX3, 32:AUX4, 64:AUX5, 128:AUX6
  {CV_OUTPUT_LOCATION_F4, 4}, // 1:AUX2, 2:AUX3, 4:AUX4, 8:AUX5, 16:AUX6, 32:AUX7, 64:AUX8
  {CV_OUTPUT_LOCATION_F5, 8}, // 1:AUX2, 2:AUX3, 4:AUX4, 8:AUX5, 16:AUX6, 32:AUX7, 64:AUX8
  {CV_OUTPUT_LOCATION_F6, 16},// 1:AUX2, 2:AUX3, 4:AUX4, 8:AUX5, 16:AUX6, 32:AUX7, 64:AUX8
  {CV_OUTPUT_LOCATION_F7, 32},// 1:AUX2, 2:AUX3, 4:AUX4, 8:AUX5, 16:AUX6, 32:AUX7, 64:AUX8
  {CV_OUTPUT_LOCATION_F8, 64},// 1:AUX2, 2:AUX3, 4:AUX4, 8:AUX5, 16:AUX6, 32:AUX7, 64:AUX8
  {CV_OUTPUT_LOCATION_F9, 0}, // 1:AUX5, 2:AUX6, 4:AUX7, 8:AUX8
  {CV_OUTPUT_LOCATION_F10, 0},// 1:AUX5, 2:AUX6, 4:AUX7, 8:AUX8
  {CV_OUTPUT_LOCATION_F11, 0},// 1:AUX5, 2:AUX6, 4:AUX7, 8:AUX8
  {CV_OUTPUT_LOCATION_F12, 0},// 1:AUX5, 2:AUX6, 4:AUX7, 8:AUX8
  {CV_PRODUCT_ID+0, 0},  // product ID highest byte
  {CV_PRODUCT_ID+1, 0},  // product ID high byte
  {CV_PRODUCT_ID+2, 1},  // product ID low byte
  {CV_PRODUCT_ID+3, 2},  // product ID lowest byte
  {CV_KICK_START, 100},
  {CV_FWD_TRIM, 128},
  {CV_SPEED_MAP+0, 43},
  {CV_SPEED_MAP+1, 46},
  {CV_SPEED_MAP+2, 50},
  {CV_SPEED_MAP+3, 54},
  {CV_SPEED_MAP+4, 58},
  {CV_SPEED_MAP+5, 62},
  {CV_SPEED_MAP+6, 66},
  {CV_SPEED_MAP+7, 71},
  {CV_SPEED_MAP+8, 76},
  {CV_SPEED_MAP+9, 81},
  {CV_SPEED_MAP+10, 87},
  {CV_SPEED_MAP+11, 93},
  {CV_SPEED_MAP+12, 99},
  {CV_SPEED_MAP+13, 106},
  {CV_SPEED_MAP+14, 113},
  {CV_SPEED_MAP+15, 121},
  {CV_SPEED_MAP+16, 129},
  {CV_SPEED_MAP+17, 137},
  {CV_SPEED_MAP+18, 146},
  {CV_SPEED_MAP+19, 155},
  {CV_SPEED_MAP+20, 165},
  {CV_SPEED_MAP+21, 176},
  {CV_SPEED_MAP+22, 187},
  {CV_SPEED_MAP+23, 199},
  {CV_SPEED_MAP+24, 212},
  {CV_SPEED_MAP+25, 226},
  {CV_SPEED_MAP+26, 240},
  {CV_SPEED_MAP+27, 255},
  {CV_REV_TRIM, 128},
  {CV_USER_ID1, 42},
  {CV_USER_ID2, 42},
};

uint8_t factoryDefaultCVIndex = 0;
// state variables
uint8_t numSpeedSteps = SPEED_STEP_128;
uint32_t lastFunctionState;
uint32_t newFunctionState;
bool lastDirection;   // DCC_DIR_FWD(1): (a+/b-), DCC_DIR_REV(0): rev (a-/b+)
bool newDirection;    // DCC_DIR_FWD(1): (a+/b-), DCC_DIR_REV(0): rev (a-/b+)
bool actualDirection; // DCC_DIR_FWD(1): (a+/b-), DCC_DIR_REV(0): rev (a-/b+)
uint8_t lastSpeed;    // 0-127, 0:EMCY stop, 1: normal stop, 2-127: drive
uint8_t newSpeed;     // 0-127, 0:EMCY stop, 1: normal stop, 2-127: drive
uint8_t actualSpeed;
uint8_t targetSpeed;
uint8_t savedSpeed;
uint16_t maxOCR;
// timing for acceleration / deceleration
uint32_t lastSpeedStep = 0;
uint32_t newSpeedStep = 0;
uint32_t packetTimeout;
uint32_t lastPacket;
// configuration variables
uint8_t vStart;
uint8_t vMid;
uint8_t vHigh;
uint8_t accRate;
uint8_t decRate;
uint8_t pwmPeriod;
uint8_t functionMapping[14];
uint8_t lastOutputState = 0;
uint8_t newOutputState = 0;

void notifyCVResetFactoryDefault(){
/*+
 *  notifyCVResetFactoryDefault() Called when CVs must be reset.
 *                                This is called when CVs must be reset
 *                                to their factory defaults. This callback
 *                                should write the factory default value of
 *                                relevent CVs using the setCV() method.
 *                                setCV() must not block whens this is called.
 *                                Test with isSetCVReady() prior to calling setCV()
 *
 *  Inputs:
 *    None
 *                                                                                                        *
 *  Returns:
 *    None
 */
// Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset 
// to flag to the loop() function that a reset to Factory Defaults needs to be done
  factoryDefaultCVIndex = sizeof(factoryDefaultCVs)/sizeof(CVPair);
};

void notifyDccReset(uint8_t hardReset){
/*+
 *  notifyDccReset(uint8_t hardReset) Callback for a DCC reset command.
 *
 *  Inputs:
 *    hardReset - 0 normal reset command.
 *                1 hard reset command.
 *
 *  Returns:
 *    None
 */
  ;
}

void notifyDccIdle(void){
/*+
 *  notifyDccIdle() Callback for a DCC idle command.
 *
 *  Inputs:
 *    None
 *
 *  Returns:
 *    None
 */
  ;
}

void notifyCVChange( uint16_t CV, uint8_t Value){
/*+
 *  notifyCVChange()  Called when a CV value is changed.
 *                    This is called whenever a CV's value is changed.
 *  notifyDccCVChange()  Called only when a CV value is changed by a Dcc packet or a internal lib function.
 *                    it is NOT called if the CV is changed by means of the setCV() method.
 *                    Note: It is not called if notifyCVWrite() is defined
 *                    or if the value in the EEPROM is the same as the value
 *                    in the write command. 
 *
 *  Inputs:
 *    CV        - CV number.
 *    Value     - Value of the CV.
 *
 *  Returns:
 *    None
 */
  switch(CV){
    case CV_VSTART:
      vStart = Value;
      break;
    case CV_ACC_RATE:
      accRate = Value;
      break;
    case CV_DEC_RATE:
      decRate = Value;
      break;
    case CV_VMID:
      vMid = Value;
      break;
    case CV_VHIGH:
      vHigh = Value;
      break;
    case CV_PWM_PERIOD:
      pwmPeriod = Value;
      break;
    case CV_OUTPUT_LOCATION_F0f:
    case CV_OUTPUT_LOCATION_F0r:
    case CV_OUTPUT_LOCATION_F1:
    case CV_OUTPUT_LOCATION_F2:
    case CV_OUTPUT_LOCATION_F3:
    case CV_OUTPUT_LOCATION_F4:
    case CV_OUTPUT_LOCATION_F5:
    case CV_OUTPUT_LOCATION_F6:
    case CV_OUTPUT_LOCATION_F7:
    case CV_OUTPUT_LOCATION_F8:
    case CV_OUTPUT_LOCATION_F9:
    case CV_OUTPUT_LOCATION_F10:
    case CV_OUTPUT_LOCATION_F11:
    case CV_OUTPUT_LOCATION_F12:
      functionMapping[CV - CV_OUTPUT_LOCATION_F0f] = Value;
      break;
  }
}

void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps){
/*+
 *  notifyDccSpeed() Callback for a multifunction decoder speed command.
 *                   The received speed and direction are unpacked to separate values.
 *
 *  Inputs:
 *    Addr        - Active decoder address.
 *    AddrType    - DCC_ADDR_SHORT or DCC_ADDR_LONG.
 *    Speed       - Decoder speed. 0               = Emergency stop
 *                                 1               = Regular stop
 *                                 2 to SpeedSteps = Speed step 1 to max.
 *    Dir         - DCC_DIR_REV or DCC_DIR_FWD
 *    SpeedSteps  - Highest speed, SPEED_STEP_14   =  15
 *                                 SPEED_STEP_28   =  29
 *                                 SPEED_STEP_128  = 127
 *
 *  Returns:
 *    None
 */
#ifdef DEBUG_SPEED
  Serial.print("notifyDccSpeed: Addr: ");
  Serial.print(Addr,DEC);
  Serial.print( (AddrType == DCC_ADDR_SHORT) ? "-S" : "-L" );
  Serial.print(" Speed: ");
  Serial.print(Speed,DEC);
  Serial.print(" Steps: ");
  Serial.print(SpeedSteps,DEC);
  Serial.print(" Dir: ");
  Serial.println( (Dir == DCC_DIR_FWD) ? "Forward" : "Reverse" );
#endif
  newDirection = Dir;
  newSpeed = Speed;
  numSpeedSteps = SpeedSteps;
}

void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState){
/*+
 *  notifyDccFunc() Callback for a multifunction decoder function command.
 *
 *  Inputs:
 *    Addr        - Active decoder address.
 *    AddrType    - DCC_ADDR_SHORT or DCC_ADDR_LONG.
 *    FuncGrp     - Function group. FN_0      - 14 speed step headlight function.
 *                                                                  Mask FN_BIT_00.
 *                                  FN_0_4    - Functions  0 to  4. Mask FN_BIT_00 - FN_BIT_04
 *                                  FN_5_8    - Functions  5 to  8. Mask FN_BIT_05 - FN_BIT_08
 *                                  FN_9_12   - Functions  9 to 12. Mask FN_BIT_09 - FN_BIT_12
 *                                  FN_13_20  - Functions 13 to 20. Mask FN_BIT_13 - FN_BIT_20 
 *                                  FN_21_28  - Functions 21 to 28. Mask FN_BIT_21 - FN_BIT_28
 *    FuncState   - Function state. Bitmask where active functions have a 1 at that bit.
 *                                  You must & FuncState with the appropriate
 *                                  FN_BIT_nn value to isolate a given bit.
 *
 *  Returns:
 *    None
 */
#ifdef DEBUG_FUNCTIONS
  Serial.print("notifyDccFunc: Addr: ");
  Serial.print(Addr,DEC);
  Serial.print( (AddrType == DCC_ADDR_SHORT) ? 'S' : 'L' );
  Serial.print("  Function Group: ");
  Serial.print(FuncGrp,DEC);
#endif

  switch(FuncGrp){
    case FN_0_4:
      bitWrite(newFunctionState, 0, FuncState & FN_BIT_00);
      bitWrite(newFunctionState, 1, FuncState & FN_BIT_01);
      bitWrite(newFunctionState, 2, FuncState & FN_BIT_02);
      bitWrite(newFunctionState, 3, FuncState & FN_BIT_03);
      bitWrite(newFunctionState, 4, FuncState & FN_BIT_04);
#ifdef DEBUG_FUNCTIONS
      Serial.print(" FN 0-4: ");
      Serial.print(newFunctionState & 0x1F);
#endif
      break;
    case FN_5_8:
      bitWrite(newFunctionState, 5, FuncState & FN_BIT_05);
      bitWrite(newFunctionState, 6, FuncState & FN_BIT_06);
      bitWrite(newFunctionState, 7, FuncState & FN_BIT_07);
      bitWrite(newFunctionState, 8, FuncState & FN_BIT_08);
#ifdef DEBUG_FUNCTIONS
      Serial.print(" FN 5-8: ");
      Serial.print((newFunctionState >> 5) & 0xF);
#endif
      break;
    case FN_9_12:
      bitWrite(newFunctionState, 9, FuncState & FN_BIT_09);
      bitWrite(newFunctionState, 10, FuncState & FN_BIT_10);
      bitWrite(newFunctionState, 11, FuncState & FN_BIT_11);
      bitWrite(newFunctionState, 12, FuncState & FN_BIT_12);
#ifdef DEBUG_FUNCTIONS
      Serial.print(" FN 9-12: ");
      Serial.print((newFunctionState >> 9) & 0xF);
#endif
      break;
    case FN_13_20:
      bitWrite(newFunctionState, 13, FuncState & FN_BIT_13);
      bitWrite(newFunctionState, 14, FuncState & FN_BIT_14);
      bitWrite(newFunctionState, 15, FuncState & FN_BIT_15);
      bitWrite(newFunctionState, 16, FuncState & FN_BIT_16);
      bitWrite(newFunctionState, 17, FuncState & FN_BIT_17);
      bitWrite(newFunctionState, 18, FuncState & FN_BIT_18);
      bitWrite(newFunctionState, 19, FuncState & FN_BIT_19);
      bitWrite(newFunctionState, 20, FuncState & FN_BIT_20);
#ifdef DEBUG_FUNCTIONS
      Serial.print(" FN 13-20: ");
      Serial.print((newFunctionState >> 13) & 0xFF);
#endif
      break;
    case FN_21_28:
      bitWrite(newFunctionState, 21, FuncState & FN_BIT_21);
      bitWrite(newFunctionState, 22, FuncState & FN_BIT_22);
      bitWrite(newFunctionState, 23, FuncState & FN_BIT_23);
      bitWrite(newFunctionState, 24, FuncState & FN_BIT_24);
      bitWrite(newFunctionState, 25, FuncState & FN_BIT_25);
      bitWrite(newFunctionState, 26, FuncState & FN_BIT_26);
      bitWrite(newFunctionState, 27, FuncState & FN_BIT_27);
      bitWrite(newFunctionState, 28, FuncState & FN_BIT_28);
#ifdef DEBUG_FUNCTIONS
      Serial.print(" FN 21-28: ");
      Serial.print((newFunctionState >> 21) & 0xFF);
#endif
      break;
  }
#ifdef DEBUG_FUNCTIONS
  Serial.println();
#endif
}

#ifdef  DEBUG_DCC_MSG
void notifyDccMsg( DCC_MSG * Msg){
/*+
 *  notifyDccMsg() Raw DCC packet callback.
 *                 Called with raw DCC packet bytes.
 *
 *  Inputs:
 *    Msg        - Pointer to DCC_MSG structure. The values are:
 *                 Msg->Size          - Number of Data bytes in the packet.
 *                 Msg->PreambleBits  - Number of preamble bits in the packet.
 *                 Msg->Data[]        - Array of data bytes in the packet.
 *
 *  Returns:
 *    None
 */
  Serial.print("notifyDccMsg: ") ;
  for(uint8_t i = 0; i < Msg->Size; i++){
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' ');
  }
  Serial.println();
}
#endif

void notifyCVAck(void){
/*+
 *  notifyCVAck() Called when a CV write must be acknowledged.
 *                This callback must increase the current drawn by this
 *                decoder by at least 60mA for 6ms +/- 1ms.
 *
 *  Inputs:
 *    None
 *                                                                                                        *
 *  Returns:
 *    None
 */
  static bool ackDir = true;
  #ifdef DEBUG_DCC_ACK
  Serial.println("notifyCVAck") ;
  #endif
  
  digitalWrite(MOTOR_IN1_PIN, ackDir);
  digitalWrite(MOTOR_IN2_PIN, !ackDir);
  delay(6);
  digitalWrite(MOTOR_IN1_PIN, 1);
  digitalWrite(MOTOR_IN2_PIN, 1);
  ackDir = !ackDir;
}

void setup(){
#ifdef DEBUG_PRINT
  Serial.begin(115200);
  Serial.println("NMRA Dcc Multifunction Motor Decoder Demo");
#endif

  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  pinMode(LED_F0f_PIN, OUTPUT);
  pinMode(LED_F0r_PIN, OUTPUT);
  pinMode(LED_AUX1_PIN, OUTPUT);
  pinMode(LED_AUX2_PIN, OUTPUT);
  pinMode(LED_AUX3_PIN, OUTPUT);
  pinMode(LED_AUX4_PIN, OUTPUT);
  pinMode(LED_AUX5_PIN, OUTPUT);
  pinMode(LED_AUX6_PIN, OUTPUT);
  pinMode(LED_AUX7_PIN, OUTPUT);
  pinMode(LED_AUX8_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(MOTOR_nFAULT_PIN, INPUT_PULLUP);
  
  newDirection = DCC_DIR_FWD;
  actualDirection = DCC_DIR_FWD;
  digitalWrite(MOTOR_IN1_PIN, 1);
  digitalWrite(MOTOR_IN2_PIN, 1);
  newSpeed = lastSpeed = 0;
  targetSpeed = actualSpeed = 0;

  dcc.pin(DCC_PIN, false); // interrupt pin 2, no pullup
  // only process DCC Packets with My Address
  // Call notifyCVResetFactoryDefault() if CV 7 & 8 == 255
  dcc.init(MAN_ID_DIY, VERSION, FLAGS_MY_ADDRESS_ONLY | FLAGS_AUTO_FACTORY_DEFAULT, 0); 

#ifdef RESET_FACTORY_DEFAULTS
  notifyCVResetFactoryDefault();
#endif
  // Read the current CV values for vStart and vHigh 
  vStart = dcc.getCV(CV_VSTART);
  vMid = dcc.getCV(CV_VMID);
  vHigh = dcc.getCV(CV_VHIGH);
  accRate = dcc.getCV(CV_ACC_RATE);
  decRate = dcc.getCV(CV_DEC_RATE);
  packetTimeout = 10*dcc.getCV(CV_PACKET_TIMEOUT);
  pwmPeriod = dcc.getCV(CV_PWM_PERIOD);
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0);   // TIMER1 A and B invert PWM
  TCCR1B = _BV(WGM13) | _BV(CS10);      // TIMER1 prescaler=1, phase and frequency correct mode PWM, TOP=ICR1
  switch(pwmPeriod){ //  f = 8 MHz / TOP
    case 0:   // 40 kHz
      maxOCR = 200;
      break;
    case 1:   // 35 kHz
      maxOCR = 229;
      break;
    case 2:   // 30 kHz
      maxOCR = 267;
      break;
    case 3:   // 25 kHz
      maxOCR = 320;
      break;
    case 4:   // 20 kHz
      maxOCR = 400;
      break;
  }
  ICR1 = maxOCR;
  
  for(byte i=0;i<14;i++){
    functionMapping[i] = dcc.getCV(CV_OUTPUT_LOCATION_F0f + i);
  }
}

void loop(){
  if(dcc.process()){        // update last valid packet reception time
    lastPacket = millis();
  }
  if((millis()-lastPacket) > packetTimeout){  // timeout since last valid packet
    newSpeed = 0;
  }
  switch(dcc.getDccPolarity()){
    case RISING:
      digitalWrite(LED_GREEN_PIN, LOW);
      digitalWrite(LED_BLUE_PIN, HIGH);
      break;
    case FALLING:
      digitalWrite(LED_GREEN_PIN, HIGH);
      digitalWrite(LED_BLUE_PIN, LOW);
      break;
  }
  // handle direction changes
  if(newDirection != actualDirection){
    if(lastDirection != newDirection){
      savedSpeed = targetSpeed;
      lastDirection = newDirection;
    }
    targetSpeed = 0;
    if(actualSpeed == 0){
      actualDirection = newDirection;
      targetSpeed = savedSpeed;
    }
  }else{
    targetSpeed = savedSpeed;
  }
  digitalWrite(LED_YELLOW_PIN, (actualDirection == DCC_DIR_FWD) || (newDirection == DCC_DIR_FWD));
  digitalWrite(LED_RED_PIN, (actualDirection == DCC_DIR_REV) || (newDirection == DCC_DIR_REV));
  // handle speed change
  if(lastSpeed != newSpeed){
    lastSpeed = newSpeed;
    if(newSpeed == 0){                    // Stop if speed = 0
      targetSpeed = actualSpeed = 0;
      OCR1A = 0;
      OCR1B = 0;
#ifdef DEBUG_PWM
      Serial.println("Emergency stop");
#endif
    }else if(newSpeed == 1){              // Stop if speed = 1
      targetSpeed = 0;
#ifdef DEBUG_PWM
      Serial.println("Regular stop");
#endif
    }else{                                // Calculate PWM value in the range 1..255   
      uint8_t vScF1;                      // start to mid
      uint8_t vScF2;                      // mid to high
      uint8_t modSpeed = newSpeed - 1;    // 1-126
      uint8_t modSteps = numSpeedSteps - 1; // 14/28/126
      
      if((vHigh > 1) && (vHigh > vStart)){  // vHigh is valid
        if((vMid > 1) && (vHigh > vMid)){     // vMid is valid too, using vStart, vMid and vHigh
          vScF1 = vMid - vStart;
          vScF2 = vHigh - vMid;
        }else{                              // vMid invalid, using vStart and vHigh
          vScF2 = vScF1 = (vHigh - vStart)/2;
        }
      }else{                                // vHigh invalid, using only vStart
        vScF2 = vScF1 = (255 - vStart)/2;
      }

      if(modSpeed<modSteps/2){
        targetSpeed = (int16_t) vStart + 2*modSpeed*vScF1/modSteps;
      }else{
        targetSpeed = (int16_t) vStart + 2*modSpeed*vScF2/modSteps + vScF1 - vScF2;
      }
    }
    savedSpeed = targetSpeed;
  }
  
  // ramp up/down PWM
  if(targetSpeed != actualSpeed){
    newSpeedStep = millis();
    if(targetSpeed > actualSpeed){   // accelerating
      if((newSpeedStep - lastSpeedStep) > accRate*896/(numSpeedSteps-1)){
        lastSpeedStep = newSpeedStep;
        actualSpeed++;
      }
    }else{                  // decelerating
      if((newSpeedStep - lastSpeedStep) > decRate*896/(numSpeedSteps-1)){
        lastSpeedStep = newSpeedStep;
        actualSpeed--;
      }
    }
    if(actualDirection == DCC_DIR_FWD){    // DCC FWD
      OCR1B = 0;
      OCR1A = map(actualSpeed, 0, 255, 0, maxOCR);
    }else{                // DCC REV
      OCR1A = 0;
      OCR1B = map(actualSpeed, 0, 255, 0, maxOCR);
    }
    
  }
  
  if((newFunctionState != lastFunctionState)||(actualSpeed == 0)){  // handle function change, map to outputs
    lastFunctionState = newFunctionState;
    newOutputState = 0;
    if(actualDirection){        // F0f
      newOutputState |= (bitRead(newFunctionState, 0) ? functionMapping[0] : 0);
    }else{                    // F0r
      newOutputState |= (bitRead(newFunctionState, 0) ? functionMapping[1] : 0);
    }
    for(byte i=1;i<=12;i++){  //F1-F12
      newOutputState |= (bitRead(newFunctionState, i) ? functionMapping[i+1] : 0);
    }
  }

  if(newOutputState != lastOutputState){  // actualise outputs
    lastOutputState = newOutputState;
    digitalWrite(LED_F0f_PIN, bitRead(newOutputState, 0));
    digitalWrite(LED_F0r_PIN, bitRead(newOutputState, 1));
    digitalWrite(LED_AUX1_PIN, bitRead(newOutputState, 2));
    digitalWrite(LED_AUX2_PIN, bitRead(newOutputState, 3));
    digitalWrite(LED_AUX3_PIN, bitRead(newOutputState, 4));
  }
  
  
  if( factoryDefaultCVIndex && dcc.isSetCVReady()){ // handle factory default
    factoryDefaultCVIndex--; // Decrement first as initially it is the size of the array 
    dcc.setCV( factoryDefaultCVs[factoryDefaultCVIndex].CV, factoryDefaultCVs[factoryDefaultCVIndex].Value);
  }
}

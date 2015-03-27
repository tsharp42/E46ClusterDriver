#include <PCF8574.h>
#include <SimpleTimer.h>
#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <Wire.h>
#include <CmdMessenger.h>

// Pin to output the speed signal
// Connect to cluster with an open colelctor buffer (2N2222 and a 1K resistor will do)
#define SPEED_PIN 9
#define HEARTBEAT_PIN A0
// Address for the PC5874(A) used to drive extra channels
#define IOADDRESS 0x3C
// Address of the second Arduino handling the KBUS interface
#define KBUSADDRESS 0x80
// 20" Wheel = 17
// 24" Wheel = 14
// 30" Wheel = 11
#define RPMPerMPH 11.00f
// Used to map the incoming RPM percent to the cluster's guage.
// E46 = 25 per 1000 RPM, Thus a cluster with a max reading of 7k yields a max value of 175.
#define RPMMaxVal 175
// Which pin to use for the CAN chip select.
#define CAN_CS 10
// To prevent jitter on the speedo, this defines a deadzone below which the speedo will not move.
#define SPEED_DEADZONE 5

CmdMessenger cmdMessenger = CmdMessenger(Serial);
MCP_CAN CAN(CAN_CS);   
SimpleTimer timer;
PCF8574 expander;

// Debug
bool heartbeat = false;

float RPM = 0.00f;
float MaxRPM = 5000.00f;
float RPMPercent = 0.00f;
int RPMSendVal = 0;
float Speed = 0.00f;
bool LeftIndicatorLightActive = false;
bool RightIndicatorLightActive = false;
bool HighBeam = false;
bool CruiseControl = false;
bool ParkingBrake = false; // PCF8574 PIN 0

// KBUS
byte LightByte1 = 0x00;

enum
{
  Setup,
  DriveTrainHS,
  Lights,
  General,
  LowPriority
};

void OnSetup()
{
  float newMaxRPM = cmdMessenger.readInt16Arg();
  
  if(newMaxRPM > 0){
    MaxRPM = newMaxRPM;
  }
}

void OnGeneral()
{
  CruiseControl = cmdMessenger.readBoolArg();
  ParkingBrake = cmdMessenger.readBoolArg();
}

void OnLowPriority()
{
  //DoKbus();
}

void OnDriveTrainHS()
{
  // To prevent a divide by zero, simply add 1 to the value
  RPM = cmdMessenger.readFloatArg() + 1.00f;
  Speed = cmdMessenger.readFloatArg();
  
  // Clamp the RPM to the defined maximum
  if(RPM > MaxRPM) { RPM = MaxRPM; }
  // Calculate RPM Percentage
  RPMPercent = (RPM / MaxRPM) * 100.00f;
  // Map the RPM to a value for the cluster
  RPMSendVal = map(RPMPercent, 0, 100, 0, RPMMaxVal);
  
  
  // Reverse will yield a negative speed, invert.
  if(Speed < 0)
  {
    Speed = -Speed;
  }
  // To prevent jitter at low speeds, this is a deadzone.
  if(Speed < SPEED_DEADZONE)
  {
     Speed = 0; 
  }
  
}

void OnLights()
{
  LeftIndicatorLightActive = cmdMessenger.readBoolArg();
  RightIndicatorLightActive = cmdMessenger.readBoolArg();
  HighBeam = cmdMessenger.readBoolArg();
  
  LightByte1 = 0x00;
  if(RightIndicatorLightActive)
    LightByte1 = LightByte1 | 0x40;
    
  if(LeftIndicatorLightActive)
    LightByte1 = LightByte1 | 0x20;
    
  if(HighBeam)
    LightByte1 = LightByte1 | 0x04;
    
    
  DoKbus();
}

void setup() {
  
  Serial.begin(115200);
  
  cmdMessenger.attach(Setup, OnSetup);
  cmdMessenger.attach(DriveTrainHS, OnDriveTrainHS);
  cmdMessenger.attach(Lights, OnLights);
  cmdMessenger.attach(General, OnGeneral);
  cmdMessenger.attach(LowPriority, OnLowPriority);
  
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(HEARTBEAT_PIN, OUTPUT);
  
  expander.begin(IOADDRESS);
  expander.pinMode(0, OUTPUT);
  expander.pinMode(1, OUTPUT);
  expander.pinMode(2, OUTPUT);
  expander.pinMode(3, OUTPUT);
  expander.digitalWrite(0, HIGH);
  expander.digitalWrite(1, HIGH);
  expander.digitalWrite(2, HIGH);
  expander.digitalWrite(3, HIGH);
  
  
  RPMSendVal = 125;
  
  Serial.write("Can init");
  
  /* INIT CAN */
    START_INIT:

    if(CAN_OK == CAN.begin(CAN_500KBPS))
    {
       Serial.write("OK!");
    }
    else
    {
        Serial.write("Can could not init");
        delay(1000);
        goto START_INIT;
    }
  /* END CAN */
  
  Serial.write("OK!");
  
  timer.setInterval(20, DoCAN);
  timer.setInterval(500, HeartBeat);
}

void loop() {

 
  if(Speed == 0)
  {
    noTone(SPEED_PIN); 
  }else{
    tone(SPEED_PIN, RPMPerMPH * Speed);
  }
  
  cmdMessenger.feedinSerialData();
  
  timer.run();
  
  expander.digitalWrite(0, ParkingBrake);

}

void DoCAN() {
  
  // RPM
  //                                   LSB   MSB
  unsigned char stmp[8] = {0x05, 0x62, 0xFF, RPMSendVal, 0x65, 0x12, 0, 62};
  CAN.sendMsgBuf(0x316, 0, 8, stmp);

  // DME 4
  // L1: Lights. Bits: | ? | ? | ? | EML | Cruise | ? | Check Engine | ? |
  // L2: Lights. Bits: | ? | ? | ? | ? | O/H Light | ? | Oil Level | ? |
  //                         L1   <  MPG   >   L2   oTemp  Chg  -   -
  byte L1 = 0x00;
  if(CruiseControl)
    L1 = 0x08;
  
  unsigned char stmp2[8] = {L1, 0x00, 0x00, 0x00, 0xAB, 0x00, 0x00, 0x00};
  CAN.sendMsgBuf(0x545, 0, 8, stmp2);
  
  // DME2
  //                          ?   cTemp   -    -     -    Throt Kick    -
  unsigned char stmp3[8] = {0x00, 0xAB, 0x8c, 0x08, 0x00, 0xFE, 0x00, 0x00};
  CAN.sendMsgBuf(0x329, 0, 8, stmp3);
  
  // Seems to clear the (!) and Traction control lights?
  unsigned char stmp4[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  CAN.sendMsgBuf(0x153, 0, 8, stmp4);
  
}

void DoKbus(){
 // Send Kbus Controller Commands
  // BM = 	| Double Rate | Rind | Lind | RearFogs | FrontFogs | FullBeam | ? | ? |
  // BM2 = 	| CarImage | ? | LsideLight | RSideLight | Rr-L-Sidelight | Rr-R-Sidelight | ? | ? |
  Wire.beginTransmission(KBUSADDRESS);
  Wire.write(LightByte1); // First Byte of Light Command
  Wire.write(0x00); // Second Byte Of Light command
  Wire.endTransmission(); 
}

void HeartBeat()
{
    heartbeat = !heartbeat;
    digitalWrite(HEARTBEAT_PIN, heartbeat);
}

#include <Wire.h>

#define kbus Serial

/*
// Lights B1
bool Lights_DoubleRate      = false;
bool Lights_LeftIndicator   = false;
bool Lights_RightIndicator  = false;
bool Lights_RearFogs        = false;
bool Lights_FrontFogs       = false;
bool Lights_FullBeam        = false;

// Lights B2
bool Car_Image = true;
*/

#define I2C_ADDRESS 0x80

byte LightByte1 = 0x00;
byte LightByte2 = 0x00;

void setup() {
  // KBus is 9600 8E1
  kbus.begin(9600, SERIAL_8E1);

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int howMany)
{
  // Lights
  byte mes1[] = {0xD0, 0x08, 0xBF, 0x5B, Wire.read(), 0x00, 0x00, Wire.read(), 0x00, 0x58, 0x00};
  sendKbus(mes1);
  
  delay(500);
  
  // Time
  //byte mes2[] = {0x3B, 0x05, 0x80, 0x40, 0x01, 0x0C, 0x3B, 0x00};
  
  byte mes2[] = {0x3B, 0x05, 0x80, 0x40, 0x01, Wire.read(), Wire.read(), 0x00};
  sendKbus(mes2);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Light Messages
  // LIGHTS:
  // BM = 	| Double Rate | Rind | Left | RearFogs | FrontFogs | FullBeam | ? | ? |
  // BM2 = 	| CarImage | ? | LsideLight | RSideLight | Rr-L-Sidelight | Rr-R-Sidelight | ? | ? |
  // D0 08 BF 5B _BM_ 00 00 _BM2_ 00 58 <CHK>
  /*
  byte LightByte1 = 0x00;
  byte LightByte2 = 0x00;
  
  // Byte 1
  if(Lights_DoubleRate)
    LightByte1 = LightByte1 | 0x80;
  
  if(Lights_RightIndicator)
    LightByte1 = LightByte1 | 0x40;
   
  if(Lights_LeftIndicator)
    LightByte1 = LightByte1 | 0x20;
    
  if(Lights_RearFogs)
    LightByte1 = LightByte1 | 0x10;
    
  if(Lights_FrontFogs)
    LightByte1 = LightByte1 | 0x08;
    
  if(Lights_FullBeam)
    LightByte1 = LightByte1 | 0x04; 
    
  // Byte 2
  if(Car_Image)
    LightByte2 = LightByte2 | 0x80; 
    
  byte mes1[] = {0xD0, 0x08, 0xBF, 0x5B, LightByte1, 0x00, 0x00, LightByte2, 0x00, 0x58, 0x00};
  sendKbus(mes1);
  
  
  delay(5000);
  */
}


void sendKbus(byte *data)
{
  int end_i = data[1]+2 ;
  data[end_i-1] = iso_checksum(data, end_i-1);
  kbus.write(data, end_i + 1);
}


byte iso_checksum(byte *data, byte len)//len is the number of bytes (not the # of last byte)
{
  byte crc=0;
  for(byte i=0; i<len; i++)
  {    
    crc=crc^data[i];
  }
  return crc;
}

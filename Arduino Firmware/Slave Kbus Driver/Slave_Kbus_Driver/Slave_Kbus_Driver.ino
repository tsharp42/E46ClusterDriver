#include <Wire.h>
#include <SimpleTimer.h>

#define kbus Serial

#define I2C_ADDRESS 0x80
#define HEARTBEAT_PIN A0

byte LightByte1 = 0x00;
byte LightByte2 = 0x00;

bool sendFrame = false;
bool heartbeat = false;

SimpleTimer timer;

void setup() {
  // KBus is 9600 8E1
  kbus.begin(9600, SERIAL_8E1);

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  
  pinMode(HEARTBEAT_PIN, OUTPUT);
  timer.setInterval(500, HeartBeat);
}

void receiveEvent(int howMany)
{
  // Lights
  LightByte1 = Wire.read();
  LightByte2 = Wire.read();
  sendFrame = true;
  
  HeartBeat();
}

void loop() {
  
  timer.run();
  
  if(sendFrame)
  {
    byte mes1[] = {0xD0, 0x08, 0xBF, 0x5B, LightByte1, 0x00, 0x00, LightByte2, 0x00, 0x58, 0x00};
    sendKbus(mes1);
    sendFrame = false; 
  }
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

void HeartBeat()
{
    heartbeat = !heartbeat;
    digitalWrite(HEARTBEAT_PIN, heartbeat);
}

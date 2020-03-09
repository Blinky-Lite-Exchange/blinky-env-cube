#include <OneWire.h>
const int blueLEDPin = 2;
boolean blueLED = true;

const int oneWirePin1 = 10;
const int oneWirePowerPin1 = 11;
byte oneWireChipType1;
byte oneWireAddress1[8];
float celsius1;

const int oneWirePin2 = 4;
const int oneWirePowerPin2 = 5;
byte oneWireChipType2;
byte oneWireAddress2[8];
float celsius2;

OneWire  oneWire1(oneWirePin1);
OneWire  oneWire2(oneWirePin2);

void setup(void) 
{
  pinMode(blueLEDPin, OUTPUT);
  pinMode(A0, INPUT);

  oneWireChipType1 = initOneWireTemperatureProbe(oneWirePowerPin1, oneWireAddress1, &oneWire1);
  oneWireChipType2 = initOneWireTemperatureProbe(oneWirePowerPin2, oneWireAddress2, &oneWire2);

  Serial1.begin(9600);
  
}

void loop(void) 
{
  delay(1000);
  digitalWrite(blueLEDPin, blueLED);
  blueLED = !blueLED;
  Serial1.println("Booger2");
  
  celsius1 = getOneWireTemperature(&oneWire1, oneWireAddress1, oneWireChipType1);
  celsius2 = getOneWireTemperature(&oneWire2, oneWireAddress2, oneWireChipType2);

  Serial1.print(celsius1);
  Serial1.print(" ");
  Serial1.println(celsius2);
  Serial1.println(analogRead(A0));
}

float getOneWireTemperature(OneWire* ow, byte* addr, byte chipType)
{
  byte i;
  byte data[12];
  float celsius;
  ow->reset();
  ow->select(addr);
  ow->write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(750);     // maybe 750ms is enough, maybe not
  
  ow->reset();
  ow->select(addr);    
  ow->write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) data[i] = ow->read();
  int16_t raw = (data[1] << 8) | data[0];
  if (chipType) 
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10)  raw = (raw & 0xFFF0) + 12 - data[6];
  }
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0;
  return celsius;
  
}

byte initOneWireTemperatureProbe(int powerPin, byte* addr, OneWire* ow)
{
  byte type_s = 0;
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);    
  if ( !ow->search(addr)) 
  {
    ow->reset_search();
    delay(250);
    return 0;
  }
   
  // the first ROM byte indicates which chip
  switch (addr[0]) 
  {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      return 0;
  } 
  return type_s;
}

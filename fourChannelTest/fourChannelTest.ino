#include <OneWire.h>
#include "dht11.h"
const int blueLEDPin = 2;
boolean blueLED = true;

const int dht11Pin =  3;
const int oneWirePin1 = 10;
const int oneWirePowerPin1 = 11;
const int oneWirePin2 = 4;
const int oneWirePowerPin2 = 5;
const int moisturePin = A0;

float temperature1;
float humidity;

byte oneWireChipType1;
byte oneWireAddress1[8];
float temperature2;

byte oneWireChipType2;
byte oneWireAddress2[8];
float temperature3;

float moisture = 0;

OneWire  oneWire1(oneWirePin1);
OneWire  oneWire2(oneWirePin2);
dht11 DHT11;

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
  
  temperature2 = getOneWireTemperature(&oneWire1, oneWireAddress1, oneWireChipType1);
  temperature3 = getOneWireTemperature(&oneWire2, oneWireAddress2, oneWireChipType2);
  DHT11.read(dht11Pin);
  temperature1 = (float)DHT11.temperature;
  humidity = (float)DHT11.humidity;

  moisture = (float) analogRead(moisturePin);
  moisture = 100.0 * (1024 - moisture) / 1024.0;

  Serial1.println(" ");
  Serial1.print("Temperature1 (°C): ");
  Serial1.println(temperature1);
  Serial1.print("Temperature2 (°C): ");
  Serial1.println(temperature2);
  Serial1.print("Temperature3 (°C): ");
  Serial1.println(temperature3);
  Serial1.print("Humidity (%): ");
  Serial1.println(humidity);
  Serial1.print("Moisture (%): ");
  Serial1.println(moisture);
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

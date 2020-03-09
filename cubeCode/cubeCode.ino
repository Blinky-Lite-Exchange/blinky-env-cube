#define BAUD_RATE 115200
#include <OneWire.h>
#include "dht11.h"

const int led13Pin = 13;
const int blueLEDPin = 2;
const int dht11Pin =  3;
const int oneWirePin1 = 10;
const int oneWirePowerPin1 = 11;
const int oneWirePin2 = 4;
const int oneWirePowerPin2 = 5;
const int moisturePin = A0;

boolean blueLED = false;
byte oneWireChipType1;
byte oneWireAddress1[8];

byte oneWireChipType2;
byte oneWireAddress2[8];

OneWire  oneWire1(oneWirePin1);
OneWire  oneWire2(oneWirePin2);
dht11 DHT11;


struct Readings
{
  float temperature1;
  float temperature2;
  float temperature3;
  float humidity;
  float moisture;
};
Readings readings;

struct Settings
{
  int readFlag;
};
Settings settings;

void setup()
{
  pinMode(led13Pin, OUTPUT);    
  pinMode(blueLEDPin, OUTPUT);
  pinMode(moisturePin, INPUT);

  oneWireChipType1 = initOneWireTemperatureProbe(oneWirePowerPin1, oneWireAddress1, &oneWire1);
  oneWireChipType2 = initOneWireTemperatureProbe(oneWirePowerPin2, oneWireAddress2, &oneWire2);
    
  Serial1.begin(BAUD_RATE);
  delay(2000);
  putSettings();
  getReadings();
 }

void loop()
{
  if(Serial1.available() > 0)
  {
    Serial1.readBytes((uint8_t*)&settings, 4);
    putSettings();
    getReadings();
    Serial1.write((uint8_t*)&readings, 20);
  }
}
void putSettings()
{
  blueLED = !blueLED;
  digitalWrite(blueLEDPin, blueLED);
  digitalWrite(led13Pin, blueLED);
}
void getReadings()
{
  readings.temperature2 = getOneWireTemperature(&oneWire1, oneWireAddress1, oneWireChipType1);
  readings.temperature3 = getOneWireTemperature(&oneWire2, oneWireAddress2, oneWireChipType2);
  DHT11.read(dht11Pin);
  readings.temperature1 = (float)DHT11.temperature;
  readings.humidity = (float)DHT11.humidity;

  readings.moisture = (float) analogRead(moisturePin);
  readings.moisture = 100.0 * (1024 - readings.moisture) / 1024.0;

  Serial1.println(" ");
  Serial1.print("Temperature1 (°C): ");
  Serial1.println(readings.temperature1);
  Serial1.print("Temperature2 (°C): ");
  Serial1.println(readings.temperature2);
  Serial1.print("Temperature3 (°C): ");
  Serial1.println(readings.temperature3);
  Serial1.print("Humidity (%): ");
  Serial1.println(readings.humidity);
  Serial1.print("Moisture (%): ");
  Serial1.println(readings.moisture);

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

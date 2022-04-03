#include "BlinkyBus.h"
#include <OneWire.h>
#include "dht11.h"
#define BAUD_RATE  19200
#define commLEDPin    2
#define measurementInterval   2000

#define BLINKYBUSBUFSIZE  6
union BlinkyBusUnion
{
  struct
  {
    int16_t state;
    int16_t i16_temperature1;
    int16_t i16_temperature2;
    int16_t i16_temperature3;
    int16_t i16_humidity;
    int16_t i16_moisture;
  };
  int16_t buffer[BLINKYBUSBUFSIZE];
} bb;
BlinkyBus blinkyBus(bb.buffer, BLINKYBUSBUFSIZE, Serial1, commLEDPin);

struct DS18B20
{
  int signalPin;
  int powerPin;
  byte chipType;
  byte address[8];
  OneWire oneWire;
  float temp = 0.0;
};

DS18B20 dS18B20_A;
DS18B20 dS18B20_B;

const int moisturePin = A0;
const int dht11Pin =  3;
dht11 DHT11;

unsigned long now;
unsigned long lastMeasurement = 0;

void setup() 
{
  dS18B20_A.signalPin = 10;
  dS18B20_A.powerPin = 11;
  pinMode(dS18B20_A.powerPin, OUTPUT);
  digitalWrite(dS18B20_A.powerPin, HIGH);    
  dS18B20_A.oneWire = OneWire(dS18B20_A.signalPin);
  dS18B20_A.chipType = initDS18B20(dS18B20_A.address, &dS18B20_A.oneWire);

  dS18B20_B.signalPin = 4;
  dS18B20_B.powerPin = 5;
  pinMode(dS18B20_B.powerPin, OUTPUT);
  digitalWrite(dS18B20_B.powerPin, HIGH);    
  dS18B20_B.oneWire = OneWire(dS18B20_B.signalPin);
  dS18B20_B.chipType = initDS18B20(dS18B20_B.address, &dS18B20_B.oneWire);

  pinMode(moisturePin, INPUT);

  Serial1.begin(BAUD_RATE);
  bb.state = 1; //init
  blinkyBus.start();
  lastMeasurement = millis(); 
  
//  Serial.begin(9600);

}

void loop() 
{
  now = millis();
  if ((now - lastMeasurement) > measurementInterval)
  {
    lastMeasurement = now;
    dS18B20_A.temp = getDS18B20Temperature(&dS18B20_A.oneWire, dS18B20_A.address, dS18B20_A.chipType);
    dS18B20_B.temp = getDS18B20Temperature(&dS18B20_B.oneWire, dS18B20_B.address, dS18B20_B.chipType);

    bb.i16_temperature2  = (int16_t) (dS18B20_A.temp * 10.0);
    bb.i16_temperature3  = (int16_t) (dS18B20_B.temp * 10.0);

    bb.i16_moisture = (int16_t) analogRead(moisturePin);
    
    DHT11.read(dht11Pin);
    bb.i16_temperature1 =  (int16_t) DHT11.temperature;
    bb.i16_humidity = (int16_t) DHT11.humidity;

/*
    Serial.print(bb.i16_temperature1);
    Serial.print(" ");
    Serial.print(bb.i16_temperature2);
    Serial.print(" ");
    Serial.print(bb.i16_temperature3);
    Serial.print(" ");
    Serial.print(bb.i16_humidity);
    Serial.print(" ");
    Serial.print(bb.i16_moisture);
    Serial.println(" ");
*/   
  }
  blinkyBus.poll();
}

byte initDS18B20(byte* addr, OneWire* ow)
{
  byte type_s = 0;
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
float getDS18B20Temperature(OneWire* ow, byte* addr, byte chipType)
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

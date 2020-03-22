#define BAUD_RATE 115200
#include <OneWire.h>
#include "dht11.h"

const int dht11Pin =  3;
const int oneWirePin1 = 10;
const int oneWirePowerPin1 = 11;
const int oneWirePin2 = 4;
const int oneWirePowerPin2 = 5;
const int moisturePin = A0;
byte oneWireChipType1;
byte oneWireAddress1[8];
byte oneWireChipType2;
byte oneWireAddress2[8];
OneWire  oneWire1(oneWirePin1);
OneWire  oneWire2(oneWirePin2);
dht11 DHT11;

struct TransmitData
{
  float temperature1;
  float temperature2;
  float temperature3;
  float humidity;
  float moisture;
};
struct ReceiveData
{
  int loopDelay = 2000;
};

void setupPins()
{
  pinMode(A0, INPUT);

  oneWireChipType1 = initOneWireTemperatureProbe(oneWirePowerPin1, oneWireAddress1, &oneWire1);
  oneWireChipType2 = initOneWireTemperatureProbe(oneWirePowerPin2, oneWireAddress2, &oneWire2);
}
void processNewSetting(TransmitData* tData, ReceiveData* rData, ReceiveData* newData)
{
  rData->loopDelay = newData->loopDelay;

}
boolean processData(TransmitData* tData, ReceiveData* rData)
{
  tData->temperature2 = getOneWireTemperature(&oneWire1, oneWireAddress1, oneWireChipType1);
  tData->temperature3 = getOneWireTemperature(&oneWire2, oneWireAddress2, oneWireChipType2);
  DHT11.read(dht11Pin);
  tData->temperature1 = (float)DHT11.temperature;
  tData->humidity = (float)DHT11.humidity;

  tData->moisture = (float) analogRead(moisturePin);
  tData->moisture = 100.0 * (1024 - tData->moisture) / 1024.0;

  delay(rData->loopDelay);
  return true;
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
const int microLEDPin = 13;
const int commLEDPin = 2;
boolean commLED = true;

struct TXinfo
{
  int cubeInit = 1;
  int newSettingDone = 0;
};
struct RXinfo
{
  int newSetting = 0;
};

struct TX
{
  TXinfo txInfo;
  TransmitData txData;
};
struct RX
{
  RXinfo rxInfo;
  ReceiveData rxData;
};
TX tx;
RX rx;
ReceiveData settingsStorage;

int sizeOfTx = 0;
int sizeOfRx = 0;

void setup()
{
  setupPins();
  pinMode(microLEDPin, OUTPUT);    
  pinMode(commLEDPin, OUTPUT);  
  digitalWrite(commLEDPin, commLED);
//  digitalWrite(microLEDPin, commLED);

  sizeOfTx = sizeof(tx);
  sizeOfRx = sizeof(rx);
  Serial1.begin(BAUD_RATE);
  delay(1000);
}
void loop()
{
  boolean goodData = false;
  goodData = processData(&(tx.txData), &settingsStorage);
  if (goodData)
  {
    tx.txInfo.newSettingDone = 0;
    if(Serial1.available() > 0)
    { 
      commLED = !commLED;
      digitalWrite(commLEDPin, commLED);
      Serial1.readBytes((uint8_t*)&rx, sizeOfRx);
      
      if (rx.rxInfo.newSetting > 0)
      {
        processNewSetting(&(tx.txData), &settingsStorage, &(rx.rxData));
        tx.txInfo.newSettingDone = 1;
        tx.txInfo.cubeInit = 0;
      }
    }
    Serial1.write((uint8_t*)&tx, sizeOfTx);
  }
  
}

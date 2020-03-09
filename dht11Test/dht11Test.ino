#include "dht11.h"

dht11 DHT11;

#define DHT11PIN 3

const int blueLEDPin = 2;
boolean blueLED = true;

void setup() 
{
  pinMode(blueLEDPin, OUTPUT);
  digitalWrite(blueLEDPin, blueLED);
  Serial1.begin(9600);
  Serial1.println("DHT11 TEST PROGRAM ");
  Serial1.print("LIBRARY VERSION: ");
  Serial1.println(DHT11LIB_VERSION);
  Serial1.println();

}

void loop() 
{
  digitalWrite(blueLEDPin, blueLED);
  blueLED = !blueLED;
  Serial1.println("Booger1\n");

  int chk = DHT11.read(DHT11PIN);

  Serial1.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
    Serial1.println("OK"); 
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    Serial1.println("Checksum error"); 
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    Serial1.println("Time out error"); 
    break;
    default: 
    Serial1.println("Unknown error"); 
    break;
  }

  Serial1.print("Humidity (%): ");
  Serial1.println((float)DHT11.humidity, 2);

  Serial1.print("Temperature (°C): ");
  Serial1.println((float)DHT11.temperature, 2);
  delay(2000);
}

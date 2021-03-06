/* Sketch for using the AQE shield with Arduino Uno
   Copied from: https://github.com/jmsaavedra/Air-Quality-Egg/tree/master/libraries/EggBus
   Adapted by Michael van den Bossche
   Version 5.1 - Used RTC to write time stamp for the air quality measurements.
   Version 5.0 - included RTC to keep time.
   Version 4.0 - updated the SD card library to use fat32 for cards > 4GB. Library made by Bill Greiman
   Version 3.0 - changed the library for the DHT22 - now using the library by Rob Tillaart: https://github.com/RobTillaart/Arduino
   Version 2.0 - included writing to SD card using the Fat16 library.
*/

#include <stdint.h>
#include <dht.h>
#include "Wire.h"
#include "EggBus.h"
#include <SdFat.h>          // include the SdFat library
#include <DS3231.h>         // real-time clock

EggBus eggBus;

//define filename
char name [13] = "AQE001.CSV";
String outputString;

//From dht22_test (Tillaart) 
dht DHT;
#define DHT22_PIN A3 //analog pin 3
//end portion from dht22_test

//Create the RTC object
DS3231 RTC; 
DateTime dt(__DATE__,__TIME__);

SdFat sd;                   // Start the SdFat object, call it 'sd' (could be called anything).
SdFile myFile;              // start the SdFile object, call it 'myFile'.

// store error strings in flash to save RAM
// from Stalkerv21_DataLogger_5min example
#define error(s) error_P(PSTR(s))

void setup(){
  Serial.begin(9600);
  
  Wire.begin();
  RTC.begin();
  // RTC.adjust(dt);  
  
  Serial.println(F("Sensor Type, Sensor Value, Sensor Units, Sensor Resistance"));
  Serial.println(F("----------------------------------------------------------"));  

  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  // ON SEEEDUINO STALKER V2.3, THIS IS PIN 10. SdFat handles setting SS
  if (!sd.begin(10, SPI_HALF_SPEED)) sd.initErrorHalt();

  // open the file for write at end ('Append').
  if (!myFile.open(name, O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt();

  
  // logging header
  myFile.println("date,time,reltime,NO2 [ppb],CO [ppb],O3 [ppb],PM,RH [%RH],T [oC]");
    
  myFile.close();
}

//allows us to read the sensors every second, regardless of how much time we've spent doing other things
DateTime lastSensorRead;
boolean OneSecondHasPassed(void)
{
  DateTime rightNow = RTC.now();
  if(rightNow.get() >= lastSensorRead.get() + 10)  // do sensor measurements every 10 seconds
  {
    lastSensorRead = rightNow;
    return true;
  }
  else return false;
}

void loop(){
  if (OneSecondHasPassed())
     {  
    uint8_t   egg_bus_address;
    uint32_t r0 = 0;
    float sensor_resistance;
  
    eggBus.init();

    outputString = makePrettyTime();
	Serial.println(outputString);
    outputString += ',';

    while((egg_bus_address = eggBus.next())){
      uint8_t numSensors = eggBus.getNumSensors();
      for(uint8_t ii = 0; ii < numSensors; ii++){
        
        Serial.print(eggBus.getSensorType(ii));
        Serial.print(F(", "));
      
        r0 = eggBus.getSensorR0(ii);
        sensor_resistance = eggBus.getSensorResistance(ii);
      
        Serial.print(eggBus.getSensorValue(ii), 8);          
        Serial.print(F(", "));
        outputString += Float2String(eggBus.getSensorValue(ii));
        outputString += ','; 

        Serial.print(eggBus.getSensorUnits(ii));            
        Serial.print(F(", "));      
      
        Serial.print(sensor_resistance, 3);
      
        Serial.println();
      }
    }
    uint32_t start = micros();
    int chk = DHT.read22(DHT22_PIN);
    uint32_t stop = micros();
  
    Serial.print(millis(), DEC);
    Serial.print(F(", "));      
  
    Serial.print("Humidity");
    Serial.print(F(", "));
    float currHumidity = DHT.humidity;
    Serial.print(currHumidity, 8);
    Serial.print(F(", "));      

    Serial.println(F("%, n/a"));
    delay(2500);

    outputString += Float2String(currHumidity);
    outputString += ',';

    Serial.print(millis(), DEC);
    Serial.print(F(", "));      
  
    Serial.print("Temperature");
    Serial.print(F(", "));
    float currentTemperature = DHT.temperature;
    Serial.print(currentTemperature, 8);
    Serial.print(F(", "));      

    Serial.println(F("degrees C, n/a"));
    delay(2500);

    outputString += Float2String(currentTemperature);
    Serial.println();

    if (!sd.begin(10, SPI_HALF_SPEED)) sd.initErrorHalt();
    if (!myFile.open(name, O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt();
    myFile.println(outputString);
  
    myFile.close(); 
  }
}

String Float2String(const float& ff)
{
  //ugh...float to String is a pain in Arduino World...
  char tempStr[32];
  dtostrf(ff, 4, 2, tempStr);

  return String(tempStr);
}

String makePrettyTime(void)
{
  DateTime now = RTC.now(); //get the current date-time
  char timestamp[32];
  sprintf(timestamp, "%04d-%02d-%02d,%02d:%02d:%02d,%i", now.year(), now.month(), now.date(), now.hour(), now.minute(), now.second(), now.get());
  return String(timestamp);
}  


/* Sketch for using the AQE shield with Arduino Uno
   Copied from: https://github.com/jmsaavedra/Air-Quality-Egg/tree/master/libraries/EggBus
   Adapted by Michael van den Bossche
   Version 2.0 - included writing to SD card using the Fat16 library.
*/

#include <stdint.h>
#include <DHT.h>
#include "Wire.h"
#include "EggBus.h"
#include <Fat16.h>       // for SD disk
#include <Fat16util.h>   // for SD disk

EggBus eggBus;

//define filename
char name [13] = "AQE001.CSV";
String outputString;

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

SdCard card;         
Fat16 file; 

// store error strings in flash to save RAM
// from Stalkerv21_DataLogger_5min example
#define error(s) error_P(PSTR(s))

void error_P(const char* str) {
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode) {
    PgmPrint("SD error: ");
    Serial.println(card.errorCode, HEX);
  }
  while(1);
}

void setup(){
  Serial.begin(9600);
  Serial.println(F("Air Quality Egg - EPA Serial Build v1.0"));
  Serial.println(F("======================================================================\r\n"));
  Serial.println(F("Timestamp, Sensor Type, Sensor Value, Sensor Units, Sensor Resistance"));
  Serial.println(F("----------------------------------------------------------------------"));  

  // from Stalkerv21_DataLogger_5min example
  // initialize the SD card
  if (!card.init()) error("card.init");
   
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) error("Fat16::init");
  
  // clear write error
  file.writeError = false;
  
  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!file.open(name, O_CREAT | O_APPEND | O_WRITE))
      error("error opening file");
  
  // logging header
  file.println("timestamp [ms?],[NO2] [ppb],[CO] [ppb],[O3] [ppb],PM,RH [%RH],T [oC]");
    
  if (!file.close()) 
      error("error closing file");

}

void loop(){
  uint8_t   egg_bus_address;
  uint32_t r0 = 0;
  float sensor_resistance;
  
  eggBus.init();

  outputString = Float2String(millis());
  outputString += ',';
  
  while((egg_bus_address = eggBus.next())){
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++){
      Serial.print(millis(), DEC);
      Serial.print(F(", "));      
      
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

  Serial.print(millis(), DEC);
  Serial.print(F(", "));      
  
  Serial.print("Humidity");
  Serial.print(F(", "));
  float currHumidity = getHumidity();
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
  float currentTemperature = getTemperature();
  Serial.print(currentTemperature, 8);
  Serial.print(F(", "));      

  Serial.println(F("degrees C, n/a"));
  delay(2500);

  outputString += Float2String(currentTemperature);
  Serial.println();

  // clear write error
  file.writeError = false;
  
  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!file.open(name, O_CREAT | O_APPEND | O_WRITE))
    error("error opening file");  

  file.println(outputString);
  
  if (!file.close()) 
    error("error closing file");
 
}

void printAddress(uint8_t * address){
  for(uint8_t jj = 0; jj < 6; jj++){
    if(address[jj] < 16) Serial.print("0");
    Serial.print(address[jj], HEX);
    if(jj != 5 ) Serial.print(":");
  }
  Serial.println();
}

//--------- DHT22 humidity ---------
float getHumidity(){
  float h = dht.readHumidity();
  if (isnan(h)){
    //failed to get reading from DHT    
    delay(2500);
    h = dht.readHumidity();
    if(isnan(h)){
      return -1; 
    }
  } 
  else return h;
}

//--------- DHT22 temperature ---------
float getTemperature(){
  float t = dht.readTemperature();
  if (isnan(t)){
    //failed to get reading from DHT
    delay(2500);
    t = dht.readTemperature();
    if(isnan(t)){
      return -1; 
    }
  } 
  return t;
}

String Float2String(const float& ff)
{
  //ugh...float to String is a pain in Arduino World...
  char tempStr[32];
  dtostrf(ff, 4, 2, tempStr);

  return String(tempStr);
}


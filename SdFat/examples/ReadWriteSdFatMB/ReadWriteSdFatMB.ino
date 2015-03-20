/*Based on Bill Greiman's ReadWriteSDFat example in the SDFat library
  This program creates the SD object, creates and opens a file, writes to file, and closes the file.
  Written by: Michael van den Bossche
  Date: 2015.01.31
*/

#include <SdFat.h>          // include the SdFat library
SdFat sd;                   // Start the SdFat object, call it 'sd' (could be called anything).
SdFile myFile;              // start the SdFile object, call it 'myFile'.

void setup() {
  Serial.begin(9600);

  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  // ON SEEEDUINO STALKER V2.3, THIS IS PIN 10. SdFat handles setting SS
  if (!sd.begin(10, SPI_HALF_SPEED)) sd.initErrorHalt();

  // open the file for write at end ('Append').
  // to save with a different file name, change "test.txt" 
  if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt();

  // write to file:
  Serial.print("Writing ...");    //prints a message to the serial monitor
  myFile.println("Test 1 2 3");   //uses the myFile println() function.

  // close the file:
  myFile.close();
  Serial.println("done.");

}

void loop() {
  // nothing happens after setup
}



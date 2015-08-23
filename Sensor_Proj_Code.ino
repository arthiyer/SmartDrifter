//The library for the temperatre Sensor
#include <OneWire.h>

//The library that is used to keep track of time
#include <Time.h>

//The library that is used to write to the SD card
#include <SPI.h>
#include <SD.h>

//The object that refers to the file being written to on the SD card
File myFile;

//i is used to make sure that "TEMP, PH, TIME, DATE" is only printed once
int i = 0;

//PH sensor info
char Sensor[]="pH Sensor";
float Intercept = 13.720;
float Slope = -3.838;
int TimeBetweenReadings = 500;
int ReadingNumber=0;

//Temperature sensor info
int DS18S20_Pin = 2;
OneWire ds(DS18S20_Pin);

void setup() 
{
  Serial.begin(9600);

  //SD CS pin
  pinMode(8, OUTPUT);
  
  if (!SD.begin(8)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  //Setup for the time
  setTime(1439942400);
  
}
void loop() 
{
  writeToFile();
  delay(120000);

}

/*
 * Retrieves the PH in numerical form from the sensor. 
 * This method uses linear algebra to find the point 
 * for the PH from a graph provided by Vernier
 */
float getpH() {
  float Count = analogRead(A0);
  float Voltage = Count / 1023 * 5.0;// convert from count to raw voltage
  float SensorReading= Intercept + Voltage * Slope;
  delay(TimeBetweenReadings);// delay in between reads for stability
  ReadingNumber++;
  Serial.println(SensorReading);
  return SensorReading;
}

/*
 * Acts a very important method to this class. This method calls all
 * the necessary methods to collect data and populate the SD card with
 * that data.
 */
void writeToFile(){
  myFile = SD.open("data.txt", FILE_WRITE); //creates a file in the SD card
  if (myFile) {
    if (i == 0) {
      myFile.println("TEMP, PH, TIME, DATE");
    }
    i++;
   
    float temp = getTemp();//retrieves the temperature data
    float pH = getpH();// retrieves the pH data

    /*
     * prints the temp and pH data to the SD card
     */
    myFile.print(temp);
    myFile.print(", ");
    myFile.print(pH);
    myFile.print(", ");
    myFile.print(hour());
    printDigits(minute());
    printDigits(second());
    myFile.print(", ");
    myFile.print(day());
    myFile.print(" ");
    myFile.print(month());
    myFile.print(" ");
    myFile.print(year()); 
    myFile.println();
    myFile.close();
  } else {
    Serial.println("error opening data.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("data.txt");
  if (myFile) {
    Serial.println("data.txt:");

    while (myFile.available()) {
        Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    Serial.println("error opening data.txt");
  }
}

/*
 * used as a template for printing the time 
 */
void printDigits(int digits){
  myFile.print(":");
  if(digits < 10)
    myFile.print('0');
  myFile.print(digits);
}

/*
 * retrieves the temperature data fro the sensor
 */
float getTemp(){

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); 

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);

  
  for (int i = 0; i < 9; i++) { 
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB);
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}


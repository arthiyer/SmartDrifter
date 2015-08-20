#include <OneWire.h>
#include <Time.h>
#include <SPI.h>
#include <SD.h>

File myFile;

int i = 0;
char Sensor[]="pH Sensor";
float Intercept = 13.720;
float Slope = -3.838;
int TimeBetweenReadings = 500;
int ReadingNumber=0;

int DS18S20_Pin = 2;
OneWire ds(DS18S20_Pin);

void setup() 
{
  Serial.begin(9600);

  pinMode(8, OUTPUT);
  
  if (!SD.begin(8)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  setTime(1439482720);  

  Serial.println ("");
  
}
void loop() 
{
  writeToFile();
  delay(120000);
//  delay(5000);

}

float getpH() {
  float Count = analogRead(A0);
  float Voltage = Count / 1023 * 5.0;// convert from count to raw voltage
  float SensorReading= Intercept + Voltage * Slope;
  delay(TimeBetweenReadings);// delay in between reads for stability
  ReadingNumber++;
  Serial.println(SensorReading);
  return SensorReading;
}

void writeToFile(){
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    if (i == 0) {
      myFile.println("TEMP, PH, TIME, DATE");
    }
    i++;
   
    float temp = getTemp();
    float pH = getpH();
    
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
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    while (myFile.available()) {
        Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    Serial.println("error opening test.txt");
  }
}

void printDigits(int digits){
  myFile.print(":");
  if(digits < 10)
    myFile.print('0');
  myFile.print(digits);
}

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


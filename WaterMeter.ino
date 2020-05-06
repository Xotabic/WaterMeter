#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

//RTC_DS1307 rtc;
RTC_Millis rtc;
File myFile;

const int chipSelect = 8; //chip select for SD card
long int posCounter = 0;
String dataString = "start writing";

int sensor2Pin = A6;
int sensor1Pin = A7;
int ledPin = 6;  

int sensor1Val = 0;
int sensor2Val = 0;
char currentDay = 0;
char prevDay = 0;
//int initVal = 800;
//int treshould = 5; //treshould for detecting rising values
//int Sensor1falling=0;

//int s1Prev = initVal;
//int s2Prev = initVal;
char risingCounterS1 = 0;
char risingCounterS2 = 0;
char s1_passed = 0;
char s2_passed = 0;
char s1UnderMiddle =0;
char s2UnderMiddle =0;

char SendDebugData = 0;
unsigned char send_counter = 0;
char inByte[2];
unsigned char bytecounter = 0;

int liters = 23;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  digitalWrite(ledPin,HIGH);
  Wire.begin();
  //rtc.begin();
  rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
    //sensorValuePrev = sensorValue;
  DateTime now = rtc.now();
  
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    currentDay = now.day();
    Serial.write(currentDay);
  
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
   // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataString = "";
    dataString += String(now.year(),DEC);
    dataString += ".";
    dataString += String(now.month(),DEC);
    dataString += ".";
    dataString += String(now.day(),DEC); 
    dataString += " "; 
    dataString += String(now.hour(),DEC); 
    dataString += ":";    
    dataString += String(now.minute(),DEC);    
    dataString += ":";    
    dataString += String(now.second(),DEC);    

    
    myFile.println(dataString);
    
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 

//rtc.adjust(DateTime(2018, 8, 3, 13, 33, 0));
}


void CheckSensors2(int sensor1Val, int sensor2Val)
{
   if((sensor1Val<620) && (s1UnderMiddle ==0))
   {
     s1UnderMiddle = 1;
   }
   else if ((sensor1Val>620) && (s1UnderMiddle ==1))
   {
     s1UnderMiddle = 0;
     s1_passed = 1;
   }
   
   if((sensor2Val<620) && (s2UnderMiddle ==0))
   {
     s2UnderMiddle = 1;
   }
   else if ((sensor2Val>620) && (s2UnderMiddle ==1))
   {
     s2UnderMiddle = 0;
     s2_passed = 1;
   }
   
//   if((s2_passed == 1)&&(s1_passed == 1))
//    {
//      liters++;
//      posCounter++;
//      s1_passed = 0;
//      s2_passed = 0;
//    }  
}

void WriteDataToSDCard ()
{
  if((s2_passed == 1)&&(s1_passed == 1))
  {
    liters++;
    posCounter++;
    s1_passed = 0;
    s2_passed = 0;
    DateTime now = rtc.now();
    dataString = String(posCounter);
    myFile = SD.open("testo.txt", FILE_WRITE);
    
    dataString = String(posCounter);
    dataString += " ; ";
    dataString += String(now.year(),DEC);
    dataString += ".";
    dataString += String(now.month(),DEC);
    dataString += ".";
    dataString += String(now.day(),DEC); 
    dataString += " "; 
    dataString += String(now.hour(),DEC); 
    dataString += ":";    
    dataString += String(now.minute(),DEC);    
    dataString += ":";    
    dataString += String(now.second(),DEC);    
    dataString += " ; ";
    dataString += String(liters);
    
    myFile.println(dataString);
    myFile.close();
    Serial.println(dataString);

    Serial.print("@");
    Serial.print(liters);
    Serial.println("#");
  }
}

void IsNewDay ()
{
  DateTime now = rtc.now();
  if(currentDay != now.day())
  {
    currentDay = now.day();
    liters = 0;
    
  }
  
}


void loop() {

  sensor1Val = analogRead(sensor1Pin);
  sensor2Val = analogRead(sensor2Pin);

  CheckSensors2(sensor1Val,sensor2Val);
  WriteDataToSDCard();


  if (Serial.available()>0) 
  {
    inByte[bytecounter] = Serial.read();
    bytecounter++;
    if(bytecounter>=2)
    { 
      bytecounter=0;
//      Serial.println(inByte[0],DEC);
//      Serial.println(inByte[1],DEC);
      if(inByte[0] == 68)               // pressed "D"
      {
        SendDebugData = 1;
        Serial.println("Start sending debug data");
      }
      else if(inByte[0] == 83)          // pressed "S"
      {
        SendDebugData = 0;
        Serial.println("Stop debug data");
      }
      else if(inByte[0] == 76)          // pressed "L"
      {
        Serial.print("@");
        Serial.print(liters);
        Serial.println("#");
      }
    }
  }

    
  if(SendDebugData==1)
  {
    Serial.print(send_counter++);
    Serial.print(" s1 ");
    Serial.print(sensor1Val);
    Serial.print(" s2 ");
    Serial.println(sensor2Val);
  }


  IsNewDay();
  
  delay(100);
//    Serial.print("s1 ");
//    Serial.print(sensor1Val);
//    Serial.print(" s2 ");
//    Serial.print(sensor2Val);
//    Serial.print(" L ");
//    Serial.println(liters);


  
}

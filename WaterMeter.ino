// now in GitHub

#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "SoftwareSerial.h"

#define START_BYTE  35
#define STOP_BYTE   37
#define MESSAGE_LENGTH  14

#define ADR_WIFI_BOARD  0x10
#define ADR_WATER_METER 0x20
#define ADR_SERVO_BOARD 0x30

#define CMD_GET_DATA    0x01
#define CMD_SEND_DATA   0x02
#define CDM_GET_LITERS  0x03
#define CMD_SET_ANGLE   0x04


RTC_DS1307 rtc;
//RTC_Millis rtc;
File myFile;
SoftwareSerial SWSerial(6, 5); // RX, TX

bool NewMessage = false;
bool DataReady = false;
bool DataToSend = false;
byte ByteCounter = 0;
byte InBuf[MESSAGE_LENGTH];
byte OutBuf[10];
unsigned long int LastTime = 0;

int Counter1s = 0;
int Task1s = 10;

const int chipSelect = 8; //chip select for SD card
long int posCounter = 0;
String dataString = "start writing";

int sensor2Pin = A6;
int sensor1Pin = A7;
int ledPin = 7;


int sensor1Val = 0;
int sensor2Val = 0;
static int currentDay = 0;
char prevDay = 0;

char risingCounterS1 = 0;
char risingCounterS2 = 0;
char s1_passed = 0;
char s2_passed = 0;
char s1UnderMiddle = 0;
char s2UnderMiddle = 0;

char SendDebugData = 0;
unsigned char send_counter = 0;
char inByte[2];
unsigned char bytecounter = 0;

int liters = 5;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  SWSerial.begin(9600);
  digitalWrite(ledPin, HIGH);
  Wire.begin();
  rtc.begin();

  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc.adjust(DateTime(2019, 8, 12, 23, 29, 0));

  if (! rtc.isrunning())
  {
    Serial.println("RTC is NOT running!");
  }

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
  Serial.print("Current day ");
  Serial.println(currentDay);
  
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
    //File dataFile = SD.open("datalog.txt", FILE_WRITE);
    myFile = SD.open("testo.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (myFile) {
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
    myFile.close();

    //dataFile.println(dataString);
    //dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
    Serial.println("error opening datalog.txt");
    }
 

}


void CheckSensors2(int sensor1Val, int sensor2Val)
{
  if ((sensor1Val < 610) && (s1UnderMiddle == 0))
  {
    s1UnderMiddle = 1;
  }
  else if ((sensor1Val > 630) && (s1UnderMiddle == 1))
  {
    s1UnderMiddle = 0;
    s1_passed = 1;
  }

  if ((sensor2Val < 610) && (s2UnderMiddle == 0))
  {
    s2UnderMiddle = 1;
  }
  else if ((sensor2Val > 630) && (s2UnderMiddle == 1))
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
  if ((s2_passed == 1) && (s1_passed == 1))
  {
    liters++;
    posCounter++;
    s1_passed = 0;
    s2_passed = 0;
    DateTime now = rtc.now();

    dataString = String(posCounter);
    //myFile = SD.open("testo.txt", FILE_WRITE);

    dataString = String(posCounter);
    dataString += " ; ";
    dataString += String(now.year(), DEC);
    dataString += ".";
    dataString += String(now.month(), DEC);
    dataString += ".";
    dataString += String(now.day(), DEC);
    dataString += " ";
    dataString += String(now.hour(), DEC);
    dataString += ":";
    dataString += String(now.minute(), DEC);
    dataString += ":";
    dataString += String(now.second(), DEC);
    dataString += " ; ";
    dataString += String(liters);

    //myFile.println(dataString);
    //myFile.close();

    Serial.println(dataString);

    Serial.print("@");
    Serial.print(liters);
    Serial.println("#");
  }
}

void IsNewDay ()
{
  DateTime now = rtc.now();
  if (currentDay != now.day())
  {
    Serial.print("Old day ");
    Serial.println(currentDay, DEC);
    currentDay = now.day();
    Serial.print("new day ");
    Serial.println(currentDay, DEC);
    
      dataString = String(now.year(),DEC);
      dataString += ".";
      dataString += String(now.month(),DEC);
      dataString += ".";
      dataString += String(now.day(),DEC);
      dataString += "; ";
      dataString += String(liters);

      myFile = SD.open("DAY.txt", FILE_WRITE);
      myFile.println(dataString);
      myFile.close();
    
    liters = 0;
    posCounter = 0;
    Serial.println("New day!");
    Serial.print("@");
    Serial.print(liters);
    Serial.println("#");

  }

}


void loop() {



  if ((millis() - LastTime) > 100)
  {
    LastTime = millis();
    Counter1s++;
    sensor1Val = analogRead(sensor1Pin);
    sensor2Val = analogRead(sensor2Pin);

    CheckSensors2(sensor1Val, sensor2Val);
    WriteDataToSDCard();
  }

  if (Counter1s >= Task1s)
  {
    Counter1s = 0;
    IsNewDay();
  }

  CheckUartBus();

  if (DataReady == true)
  {
    GetData();
    DataReady = false;
  }

  if (DataToSend == true)
  {
    SendData();
    DataToSend = false;
  }




  //

  //delay(100);
  //    Serial.print("s1 ");
  //    Serial.print(sensor1Val);
  //    Serial.print(" s2 ");
  //    Serial.println(sensor2Val);
  //    Serial.print(" L ");
  //    Serial.println(liters);

}

void GetData()
{
  if (InBuf[1] == 15)
  {
        OutBuf[0] = liters >> 8;
        OutBuf[1] = liters;
        DataToSend = true;
  }
}


void SendData()
{
  SWSerial.write(11);
  SWSerial.write(OutBuf[0]);
  SWSerial.write(OutBuf[1]);
  SWSerial.write(16);
}

void CheckUartBus(void)
{
  byte Start = 10;
  byte Stop = 15;
  byte inByte = 0;
  byte i = 0;

  if (SWSerial.available() > 0)
  {
    inByte = SWSerial.read();
    if (inByte == Start)
    {
      ByteCounter = 0;
      Serial.println("BC 0");
    }

    InBuf[ByteCounter] = inByte;
    
    //Serial.println(inByte,DEC);
    if ((ByteCounter ==1))
    {
      if ((InBuf[0] == Start) && (InBuf[1] == Stop))
      {
        Serial.println("got request");
        ByteCounter = 0;
        DataReady = true;
      }
    }
    else
    {
      ByteCounter++;
    }
  }
}

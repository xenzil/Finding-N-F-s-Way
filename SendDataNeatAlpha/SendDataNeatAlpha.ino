//FreeRtos bitch
#include <Arduino_FreeRTOS.h>
#include <croutine.h>
#include <event_groups.h>
#include <FreeRTOSConfig.h>
#include <FreeRTOSVariant.h>
#include <list.h>
#include <mpu_wrappers.h>
#include <portable.h>
#include <portmacro.h>
#include <projdefs.h>
#include <queue.h>
#include <semphr.h>
#include <StackMacros.h>
#include <task.h>
#include <timers.h>
SemaphoreHandle_t xSemaphore = NULL;

//CUSTOM LIBRARIES
#include <UltrasoundLibrary.h>
#include <CompassLibrary.h>
#include <HandshakeLibrary.h>

//GENERAL LIBRARIES
#include <string.h>
#include <Wire.h>
#include <L3G.h>
#include <Keypad.h>
#include <LSM303.h>

//Infared
#define IRpin A0

//all settings are now in UltrasoundLibrary.cpp
UltrasoundLibrary ULB;
long distanceCenter, distanceBottom, distanceRight, distanceLeft;
//Need to change library definitions too
#define echoPin4 22 // Echo Pin
#define trigPin4 23 // Trigger Pin
#define echoPin5 24 // Echo Pin
#define trigPin5 25 // Trigger Pin
#define echoPin6 26 // Echo Pin
#define trigPin6 27 // Trigger Pin
#define echoPin7 28 // Echo Pin
#define trigPin7 29 // Trigger Pin
//end of ultra

//Handshake
HandshakeLibrary HLB;


//compass
CompassLibrary CLB;
float headings;
//end of compass

//serialise
unsigned len;
char buffer[50];
//end of serialise

//infared
float calibrated = 0;
float sensorValue = 0;
//end of infared

//pedo
L3G gyro;
LSM303 compass1;
int steps, flag = 0;
#define PEDOSIZE 5

float xaccl[PEDOSIZE] = {0};
float yaccl[PEDOSIZE] = {0};
float zaccl[PEDOSIZE] = {0};

float xval[PEDOSIZE] = {0};
float yval[PEDOSIZE] = {0};
float zval[PEDOSIZE] = {0};

float xavg;
float yavg;
float zavg;

float compassBefore;
float compassAfter;
float changeInCompass;

//end of pedo

//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int locationInfo[6];
//End of keypad

typedef struct conLocation //double can be changed to int for Optimization
{
  unsigned char startingBuildingNo;
  double data1;

  unsigned char startingLvlNo;
  double data2;

  unsigned char startingNodeNo;
  double data3;

  unsigned char endingBuildingNo;
  double data4;

  unsigned char endingLevelNo;
  double data5;

  unsigned char endingNodeNo;
  double data6;

} TLocationPacket;

void sendLocationConfig()
{
  TLocationPacket location;

  location.startingBuildingNo = 'U';
  location.data1 = locationInfo[0];

  location.startingLvlNo = 'V';
  location.data2 = locationInfo[1];

  location.startingNodeNo = 'W';
  location.data3 = locationInfo[2];

  location.endingBuildingNo = 'X';
  location.data4 = locationInfo[3];

  location.endingLevelNo = 'Y';
  location.data5 = locationInfo[4];

  location.endingNodeNo = 'Z';
  location.data6 = locationInfo[5];
  len = serialize(buffer, &location, sizeof(location));
  Serial.println("next is LOCATION len");
  Serial.println(len);
}

typedef struct conDevice
{
  //Pedo
  unsigned char devCode1;
  double data1;
  //Compass
  unsigned char devCode2;
  double data2;
  //Sonar1
  unsigned char devCode4;
  double data4;
  //Sonar2
  unsigned char devCode5;
  double data5;
  //Sonar3
  unsigned char devCode6;
  double data6;
  //Sonar4
  unsigned char devCode7;
  double data7;
  //Infra
  unsigned char devCode8;
  double data8;
  //Keypad
  unsigned char devCode9;
  double data9;
} TConfigPacket;

void sendConfig()
{
  TConfigPacket cfg;
  cfg.devCode1 = 'A';
  cfg.data1 = steps;
  cfg.devCode2 = 'B';
  cfg.data2 = headings;
  cfg.devCode4 = 'D';
  cfg.data4 = distanceCenter;
  cfg.devCode5 = 'E';
  cfg.data5 = distanceBottom;
  cfg.devCode6 = 'F';
  cfg.data6 = distanceRight;
  cfg.devCode7 = 'G';
  cfg.data7 = distanceLeft;
  cfg.devCode8 = 'H';
  cfg.data8 = calibrated;

  len = serialize(buffer, &cfg, sizeof(cfg));
  Serial.println("next is len");
  Serial.println(len);
}

//serialize % add checksum
unsigned int serialize(char *buf, void *p, size_t size)
{
  char checksum = 0;
  buf[0] = size;
  memcpy(buf + 1, p , size);
  for (int i = 1; i <= size; i++)
  {
    checksum ^= buf[i];
  }
  Serial.println((int)checksum);
  Serial.println("abv value is checksum");
  buf[size + 1] = checksum;
  return size + 2;
}

void sendSerialData(char *buffer, int len)
{
  for (int i = 0; i < len; i++) {
    //Serial.println(buffer[i]);
    Serial1.write(buffer[i]);
  }
}

void rawDataInfared() {
  sensorValue = 1023 - analogRead(IRpin);
  calibrated = sensorValue * 0.26369 - 120;
}

void calibratePedo() {
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;
  for (int i = 0; i < PEDOSIZE; i++) {
    gyro.read();
    xval[i] = gyro.g.x;
    yval[i] = gyro.g.y;
    zval[i] = gyro.g.z;

    sumX = xval[i] + sumX;
    sumY = yval[i] + sumY;
    sumZ = zval[i] + sumZ;
  }

  delay(200);
  xavg = sumX / 5;
  yavg = sumY / 5;
  zavg = sumZ / 5;
}


void formatInfoToSend() {
  //Serial.println("Reading Infrared");
  //rawDataInfared();
  sendConfig();
  sendSerialData(buffer, len);
  Serial.print("Steps(A): ");
  Serial.println(steps);
  Serial.print("Comp(B): ");
  Serial.println(headings);
  Serial.print("Ultra(C): "); //D
  Serial.println(distanceCenter);
  Serial.print("Ultra(B): "); //E
  Serial.println(distanceBottom);
  Serial.print("Ultra(R): "); //F
  Serial.println(distanceRight);
  Serial.print("Ultra(L): "); //G
  Serial.println(distanceLeft);
  //Serial.print("Infrared(H): ");
  //Serial.println(calibrated);
}


void taskTransmit(void *pvParameters);
void taskReadPedo(void *pvParameters);
void taskReadCompass(void *pvParameters);
void taskReadUltrasound(void *pvParameters);
//void taskReadKeypad(void *pvParameters);

void setup() {

  Serial1.begin(9600);
  Serial.begin(9600);

  Serial.println("Initialising");
  //HANDSHAKE
  HLB.executeHandshake();
  //Initialise various sensors
  Wire.begin();
  //ultra
  ULB.ultrasoundInit();
  //compass
  CLB.compassInit();
  //pedo
  gyro.init();
  gyro.enableDefault();
  calibratePedo();
  Serial.println("Creating tasks");
  //TASKS
  xSemaphore = xSemaphoreCreateMutex();
  xTaskCreate(
    taskTransmit,
    (const portCHAR *) "Transmit",
    256,
    NULL,
    2,
    NULL
  );
  xTaskCreate(
    taskReadPedo,
    (const portCHAR *) "Read Pedo",
    256,
    NULL,
    1,
    NULL
  );
  xTaskCreate(
    taskReadCompass,
    (const portCHAR *) "Read Compass",
    128,
    NULL,
    1,
    NULL
  );
  xTaskCreate(
    taskReadUltrasound,
    (const portCHAR *) "Read Ultrasound",
    128,
    NULL,
    1,
    NULL
  );
  Serial.println("input coordinates");
  readKeypad();
  Serial.println("start tasks");
}

void readKeypad() {

  bool sendFlag = true;
  bool dataIncomplete = true; //flag
  int index = 0;

  String input = "";
  //0 is starting building no
  //1 is starting lvl no
  //2 is starting node no
  //3 is ending building no
  //4 is ending level no
  //5 is ending node no

  while (dataIncomplete) {
    char key = keypad.getKey();

    if (key == '*') { // * acts as backspace
      Serial.println("backspace");
      input = "";
    }

    else if (key == '#') { // * acts as enter
      Serial.println("enter");
      locationInfo[index] = input.toInt();
      Serial.println(locationInfo[index]);
      index = index + 1;
      input = "";

      if (index > 5) {
        dataIncomplete = false; //gets out of while loop since input is done
      }
    }

    else if (key) {
      //Serial.println(key);
      input += key ;
    }
  }

  Serial.println("data input complete, sending to PI");
  while (sendFlag) {
    sendLocationConfig();
    sendSerialData(buffer, len);
    sendFlag = false;
    //Serial.print("Keypad: ");
  }

}

void loop() {

  vTaskStartScheduler();
  //while(1);
}

void taskTransmit(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE ) {
      TickType_t xLastWakeTime2 = xTaskGetTickCount();
      Serial.println("task reached");
      formatInfoToSend();
      if (Serial1.available()) {
        int ack = Serial1.read();
        if (ack == '1') {
          Serial.println("Transmission Successful");
        } else if (ack == '0') {
          while (ack == '0') {
            ack = Serial1.read();
            Serial.println("Transmission Unsuccessful");
            Serial.println("Retransmitting..");
            sendSerialData(buffer, len);
          }
        }
        xSemaphoreGive( xSemaphore );
        vTaskDelayUntil(&xLastWakeTime2, 60);
      }
    }
  }
}
void taskReadPedo(void* pvParameters) {
  (void)pvParameters;

  //CHANGE THIS TO INCREASE OR DECREASE SENSITIVITY
  //If high threshhold, less sensitivity.
  //And vice versa
  float threshhold = 2000;
  float lastVect;
  float Pi = 3.14159;
  float totvect[PEDOSIZE] = {0};
  //
  //  CLB.readCompass();
  //
  //  compassBefore = CLB.headings;
  int index = 0;
  while (1) {
    if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE ) {
      //Serial.println("I'm in pedo!");
      //unsigned long timeStart = millis();
      //Serial.print("Start: ");
      //Serial.println(timeStart);
      int flag = 0;

      TickType_t xLastWakeTime1 = xTaskGetTickCount();
      gyro.read();
      xaccl[index] = float(gyro.g.x);
      yaccl[index] = float(gyro.g.y);
      zaccl[index] = float(gyro.g.z);

      totvect[index] = sqrt( pow((xval[index] - xavg), 2) + pow((yaccl[index] - yavg), 2));

      index = index + 1;
      float totave[PEDOSIZE] = {0};
      //After PEDOSIZE samples, detect step
      if (index >= PEDOSIZE - 1) {
        for (int i = 1; i < PEDOSIZE; i++) {
          totave[i] = (totvect[i] + totvect[i - 1]) / 2 ;
          //Serial.print("Vector: ");
          //Serial.println(totave[i]);
          if (totave[i] > threshhold) {
            flag = 1;
            break;
          }
        }
        index = 0;
      }

      //unsigned long timeEnd = millis();
      //Serial.print("End: ");
      //Serial.println(timeEnd);

      if (flag == 1) {
        steps = steps + 1;
        flag = 0;
        xSemaphoreGive( xSemaphore );
        vTaskDelayUntil(&xLastWakeTime1, 100);
      }
      else if (flag == 0) {
        xSemaphoreGive( xSemaphore );
        vTaskDelayUntil(&xLastWakeTime1, 10);
      }
    }
  }
  //  flag = 0;
  //  CLB.readCompass();
  //  compassAfter = CLB.headings;
  //
  //  changeInCompass = compassBefore - compassAfter;
  //  changeInCompass = abs(changeInCompass);
  //
  //  if (changeInCompass >= 180) {
  //    changeInCompass = 360 - changeInCompass;
  //  }
}
void taskReadCompass(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE ) {
      TickType_t xLastWakeTime2 = xTaskGetTickCount();

      //Serial.println("I'm in Compass!");
      CLB.readCompass();
      headings = CLB.headings;

      xSemaphoreGive( xSemaphore );
      vTaskDelayUntil(&xLastWakeTime2, 5);
    }
  }
}
void taskReadUltrasound(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE ) {
      TickType_t xLastWakeTime2 = xTaskGetTickCount();
      //Serial.println("I'm in Ultrasound!");
      distanceCenter = ULB.readUltrasoundCenter(trigPin4, echoPin4);
      distanceBottom = ULB.readUltrasoundBottom(trigPin5, echoPin5);
      distanceRight = ULB.readUltrasoundRight(trigPin6, echoPin6);
      distanceLeft = ULB.readUltrasoundLeft(trigPin7, echoPin7);

      xSemaphoreGive( xSemaphore );
      vTaskDelayUntil(&xLastWakeTime2, 5);
    }
  }
}


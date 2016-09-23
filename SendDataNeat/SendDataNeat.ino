//CUSTOM LIBRARIES
#include <UltrasoundLibrary.h>
#include <CompassLibrary.h>
#include <HandshakeLibrary.h>

//GENERAL LIBRARIES
#include <smartTimer.h>
#include <string.h>
#include <Wire.h>
#include <L3G.h>

//Infared
#define IRpin A0

//all settings are now in UltrasoundLibrary.cpp
UltrasoundLibrary ULB;
long distance4,distance5,distance6,distance7; 

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

//timer
CSmartTimer *timer;
//end of timer

//compass
CompassLibrary CLB;
float headings;
//end of compass

//serialise
unsigned len;
char buffer[512];
//end of serialise

//infared
float calibrated = 0;
float sensorValue = 0;
//end of infared

//pedo
L3G gyro;
int steps, flag = 0;

float xval[100] = {0};
float yval[100] = {0};
float zval[100] = {0};

float xavg;
float yavg;
float zavg;
//end of pedo

typedef struct con
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

} TConfigPacket;

void sendConfig()
{
  TConfigPacket cfg;

  cfg.devCode1 = 'A';
  cfg.data1 = steps;

  cfg.devCode2 = 'B';
  cfg.data2 = headings;

  cfg.devCode4 = 'D';
  cfg.data4 = distance4;

  cfg.devCode5 = 'E';
  cfg.data5 = distance5;

  cfg.devCode6 = 'F';
  cfg.data6 = distance6;

  cfg.devCode7 = 'G';
  cfg.data7 = distance7;

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
  for (int i = 0; i < 100; i++) {
    gyro.read();
    xval[i] = gyro.g.x;
    yval[i] = gyro.g.y;
    zval[i] = gyro.g.z;

    sumX = xval[i] + sumX;
    sumY = yval[i] + sumY;
    sumZ = zval[i] + sumZ;
  }

  // delay(100);
  xavg = sumX / 100.0;
  yavg = sumY / 100.0;
  zavg = sumZ / 100.0;
}

void rawDataPedo() {
  int acc = 0;

  //CHANGE THIS TO INCREASE OR DECREASE SENSITIVITY
  //If high threshhold, less sensitivity.
  //And vice versa
  float threshhold = 6000.0;


  float totvect[100] = {0};
  float totave[100] = {0};
  float xaccl[100] = {0};
  float yaccl[100] = {0};
  float zaccl[100] = {0};
  for (int i = 0; i < 100; i++) {
    gyro.read();
    xaccl[i] = float(gyro.g.x);
    yaccl[i] = float(gyro.g.y);
    zaccl[i] = float(gyro.g.z);

    totvect[i] = sqrt(((xaccl[i] - xavg) * (xaccl[i] - xavg)) + ((yaccl[i] - yavg) * (yaccl[i] - yavg)) + ((zval[i] - zavg) * (zval[i] - zavg)));
    totave[i] = (totvect[i] + totvect[i - 1]) / 2 ;
    //Serial.println(totave[i]);
    //delay(200);

    if (totave[i] > threshhold && flag == 0)
    {
      steps = steps + 1;
      flag = 1;

    }
    else if (totave[i] > threshhold && flag == 1)
    {
      //do nothing
    }
    if (totave[i] < threshhold  && flag == 1)
    {
      //do nothing
    }
  }
  //CHANGE THIS TO CALCULATE FASTER EG WALKING FASTER
  //If delay too fast, prone to calculate even shakes.
  //If delay to slow, cannot calculate speed fast enough
  delay(1300);
  flag = 0;

}

void formatInfoToSend() {

  rawDataPedo();

  //READ COMPASS @ CompassLibrary.cpp
  CLB.readCompass();
  headings = CLB.headings;
  Serial.print("compass new:");
  Serial.println(headings);

  distance4 = ULB.readUltrasound(trigPin4, echoPin4);
  distance5 = ULB.readUltrasound(trigPin5, echoPin5);
  distance6 = ULB.readUltrasound(trigPin6, echoPin6);
  distance7 = ULB.readUltrasound(trigPin7, echoPin7);
  
  rawDataInfared();
  sendConfig();
  sendSerialData(buffer, len);
  Serial.print("Steps(A): ");
  Serial.println(steps);
  Serial.print("Compass(B): ");
  Serial.println(headings);
  Serial.print("Ultrasound 1(D): ");
  Serial.println(distance4);
  Serial.print("Ultrasound 2(E): ");
  Serial.println(distance5);
  Serial.print("Ultrasound 3(F): ");
  Serial.println(distance6);
  Serial.print("Ultrasound 4(G): ");
  Serial.println(distance7);
  Serial.print("Infrared(H): ");
  Serial.println(calibrated);
}

void setup() {

  Serial1.begin(9600);
  Serial.begin(9600);
  Wire.begin();
  //ultra
  ULB.ultrasoundInit();
  //compass
  CLB.compassInit();

  //pedo
  gyro.init();
  gyro.enableDefault();
  calibratePedo();

  //timer
  //  timer = new CSmartTimer(STIMER1);
  //  timer->attachCallback(formatInfoToSend, 3000); //100ms
  //
  //  timer->startTimer();
}

void loop() {

  HLB.executeHandshake();
  while (1) {
    formatInfoToSend();
    if (Serial1.available()) {
      int ack = Serial1.read();
      if(ack == '1'){
        Serial.println("Transmission Successful");
      }else if(ack == '0'){
        while(ack == '0')
        Serial.println("Transmission Unsuccessful");
        Serial.println("Retransmitting..");
        sendSerialData(buffer, len);
      }
    }
    delay(2000);
    //while(1);
  }
}

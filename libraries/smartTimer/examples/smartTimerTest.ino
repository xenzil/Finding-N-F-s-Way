#include <smartTimer.h>
#include <string.h>

CSmartTimer *timer;

void fun1()
{
  Serial.println("1");
}

void fun2()
{
  #ifdef RTC
  uint8_t day, month, hour, minute, sec;
  uint16_t year;
  uint64_t epoch;
  
  char buffer[512];

  timer->RTCGetLocalTime(day, month, year, hour, minute, sec);

  sprintf(buffer, "The date is %d/%d/%d and the time is %d:%d:%d", day, month, year, hour, minute, sec, epoch);
  Serial.println(buffer);
  #else
  Serial.println("I am awesome!");
  #endif
  
}

void fun3()
{
  static int flag=0;
  
  if(flag)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);
    
  flag=!flag;
}

void setup()
{

  pinMode(13, OUTPUT);
  
  digitalWrite(13, HIGH);
  
  Serial.begin(9600);  
  timer = new CSmartTimer(STIMER1);
  timer->attachCallback(fun1, 100);
  timer->attachCallback(fun2, 1000);
  timer->attachCallback(fun3, 500);

#ifdef RTC  
  // Test RTC
  timer->RTCSetUTCOffset(8, 0);
  timer->RTCSetLocalTime(14, 6, 2015, 16, 1, 0);
#endif
  timer->startTimer();
}

void loop()
{
}



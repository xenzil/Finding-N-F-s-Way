#ifndef UltrasoundLibrary_h
#define UltrasoundLibrary_h

#include <Arduino.h> 

class UltrasoundLibrary
{
  public:
	long readUltrasound(int trigPin, int echoPin);
	void ultrasoundInit();
    
};

#endif

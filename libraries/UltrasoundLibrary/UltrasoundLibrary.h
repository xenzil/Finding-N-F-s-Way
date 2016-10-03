#ifndef UltrasoundLibrary_h
#define UltrasoundLibrary_h

#include <Arduino.h> 

class UltrasoundLibrary
{
  public:
	long readUltrasoundLeft(int trigPin, int echoPin);
	long readUltrasoundRight(int trigPin, int echoPin);
	long readUltrasoundCenter(int trigPin, int echoPin);
	long readUltrasoundBottom(int trigPin, int echoPin);
	void ultrasoundInit();
    
};

#endif

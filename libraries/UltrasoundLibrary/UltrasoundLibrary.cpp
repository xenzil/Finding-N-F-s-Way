#include <Arduino.h>
#include <UltrasoundLibrary.h>

#define echoPin4 22 // Echo Pin
#define trigPin4 23 // Trigger Pin
#define echoPin5 24 // Echo Pin
#define trigPin5 25 // Trigger Pin
#define echoPin6 26 // Echo Pin
#define trigPin6 27 // Trigger Pin
#define echoPin7 28 // Echo Pin
#define trigPin7 29 // Trigger Pin

int maximumRange = 200, minimumRange = 3;
long duration, distance;

long UltrasoundLibrary::readUltrasound(int trigPin, int echoPin)
{
  /* The following trigPin/echoPin cycle is used to determine the
    distance of the nearest object by bouncing soundwaves off of it. */
	
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;

  if (distance >= maximumRange || distance <= minimumRange) {
  
  /* Send a negative number to computer and Turn LED ON
    to indicate "out of range" */
	
  distance = -1;
  }
  return distance;
}

void UltrasoundLibrary::ultrasoundInit(){
  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);
  pinMode(trigPin5, OUTPUT);
  pinMode(echoPin5, INPUT);
  pinMode(trigPin6, OUTPUT);
  pinMode(echoPin6, INPUT);
  pinMode(trigPin7, OUTPUT);
  pinMode(echoPin7, INPUT);
}
#include <LSM303.h>
#include <Arduino.h>
#include <CompassLibrary.h>

LSM303 compass;
//float headings;

void CompassLibrary::readCompass()
{
  compass.read();
  float Pi = 3.14159;

  // Calculate the angle of the vector y,x
  headings = (atan2(compass.m.y, compass.m.x) * 180) / Pi;

  // Normalize to 0-360
  if (headings < 0)
  {
    headings = 360 + headings;
  }
}

void CompassLibrary::compassInit(){
	compass.init();
	compass.enableDefault();
}
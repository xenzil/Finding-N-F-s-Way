#ifndef CompassLibrary_h
#define CompassLibrary_h

#include <Arduino.h> 

class CompassLibrary
{
  public:
	void readCompass();
	void compassInit();
	float headings;
    
};

#endif

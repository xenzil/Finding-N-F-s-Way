#include <Arduino.h>
#include <HandshakeLibrary.h>


char letter = ' ';
char ack1 = '1';
char ack2 = '2';
char ack3 = '3';


void HandshakeLibrary::executeHandshake()
{

  //wait for pi's '1'
  while(1){
    if(Serial1.available()>0){
      letter = Serial1.read();
    }
    if(ack1 == letter){
      //Serial.println(letter);
      break;
    }
  }
  
  letter = ' ';

	//delay(1);
	Serial1.write(ack2);
  while(1){
	  

    if(Serial1.available()>0){
      letter = Serial1.read();
    }
    if(letter == ack3){
      //Serial.println(letter);
      break;
    }
  }
  
  Serial.println("Handshake Complete");
}
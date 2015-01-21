/* Arduino Code for JMP, version 0.8
   @author Marien Wolthuis
   date created 21/1/2015			*/

#include <ExtensionSensor.h>
#include <Math.h>

#define ESFRAC_MIN 0.1

ExtensionSensor eSens;
volatile float eSpeed;

unsigned long lR;
void setup(){
	attachInterrupt(0, ISR_eSens, FALLING);
	
	Serial.begin(9600);
}

void ISR_eSens(){
	eSpeed = eSens.read();
}


void loop(){
	unsigned long temp = eSens.lastReading();
	/*if(temp != lR){
		Serial.println(temp);
		lR = temp;
	} */
	Serial.println(eSpeed);
}

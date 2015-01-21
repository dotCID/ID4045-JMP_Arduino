/* Arduino Code for JMP, version 0.8
   @author Marien Wolthuis
   date created 21/1/2015			*/

#include <ExtensionSensor2.h>
#include <RotationSensor2.h>
#include <Math.h>

#define ESFRAC_MIN 0.1

ExtensionSensor eSens;
RotationSensor rSens(A0);
volatile float eSpeed;
volatile float rSpeed;
int timer1_counter;

unsigned long lR;
void setup(){
	attachInterrupt(0, ISR_eSens, FALLING);
	
	noInterrupts();           // disable all interrupts
  	TCCR1A = 0;
  	TCCR1B = 0;
 	timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
   	TCNT1 = timer1_counter;   // preload timer
  	TCCR1B |= (1 << CS12);    // 256 prescaler 
  	TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  	interrupts();             // enable all interrupts
	
	Serial.begin(9600);
}

void ISR_eSens(){
	eSpeed = eSens.read();
		//Serial.println(eSpeed);
}


ISR(TIMER1_OVF_vect){
	interrupts();				// enable interrupts because other interrupt has priority
	TCNT1 = timer1_counter;
	rSpeed = rSens.read();
}

void loop(){
//	unsigned long temp = eSens.lastReading();
	Serial.print("rSpeed = ");Serial.println(rSpeed);
	Serial.print("eSpeed = ");Serial.println(eSpeed);
	//Serial.println(analogRead(A0));
}

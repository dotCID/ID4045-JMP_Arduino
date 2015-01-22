/* Unit test for the Extension motor and sensor combination
   @author Marien Wolthuis
   date created 21/1/2015			*/
   
#include <RotationMotor.h>
#include <ExtensionMotor.h>
#include <RotationSensor2.h>
#include <ExtensionSensor2.h>
#include <Math.h>

#define RPOS_MAX 70
#define RPOS_MIN 0

#define RSFRAC_MIN 0.1

bool rotation = true;
bool extension = false;

String inputString = "";
bool stringComplete = false;

volatile float rSpeed; // SI units
volatile float eSpeed;

float rPos, rDes;
float ePos, eDes;
int rDir;
int eDir;
int timer1_counter;

RotationMotor rMot(10, 11);
RotationSensor rSens(A0);

ExtensionMotor eMot(5,7,6);
ExtensionSensor eSens;

void setup(){
	pinMode(5, OUTPUT); //fwd
	digitalWrite(5, LOW);
	
	pinMode(6, OUTPUT); //dis
	digitalWrite(6, LOW);
	
	pinMode(7, OUTPUT); //rev
	digitalWrite(7, LOW);
	
	pinMode(10, OUTPUT); //up
	digitalWrite(10, LOW);
	pinMode(11, OUTPUT); //down
	digitalWrite(11, LOW);
	
	noInterrupts();           
  	TCCR1A = 0;
  	TCCR1B = 0;
 	timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
   	TCNT1 = timer1_counter;   // preload timer
  	TCCR1B |= (1 << CS12);    // 256 prescaler 
  	TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  	interrupts();             

	attachInterrupt(0, ISR_eSens, FALLING);

	
	Serial.begin(9600);
}

void ISR_eSens(){
	eSpeed = eSens.read();
}

ISR(TIMER1_OVF_vect){
	interrupts();				// enable interrupts because other interrupt has priority
	TCNT1 = timer1_counter;
	rSpeed = rSens.read();
}

void processSerial(){
	while (Serial.available()) {
		char inChar = (char)Serial.read(); 
		inputString += inChar;
		if (inChar == '\n') {
		  stringComplete = true;
		} 
	}
	
	if(stringComplete) {
		if(inputString == "up\n"){
			Serial.println("Up");
			if(rotation){
				rMot.setSpeed(0.5);
				rMot.run(1);
				delay(3000);
				rMot.stop();
			}
		}else if(inputString == "down\n"){
			Serial.println("Down");
			if(rotation){
				rMot.setSpeed(0.5);
				rMot.run(-1);
				delay(3000);
				rMot.stop();
			}
		}else if(inputString == "extend\n"){
			Serial.println("extending");
			if(extension){/*
				eMot.setSpeed(1);
				eMot.run(1);
				delay(1000);
				eMot.stop();*/
				
				digitalWrite(5, LOW);
				digitalWrite(7, HIGH);
				digitalWrite(6, HIGH);
				
				delay(1000);
				
				digitalWrite(7, LOW);
				digitalWrite(6, LOW);
				
			}
		}else if(inputString == "retract\n"){
			Serial.println("retracting");
			if(extension){
				digitalWrite(5, HIGH);
				digitalWrite(7, LOW);
				digitalWrite(6, HIGH);
				
				delay(1000);
				
				digitalWrite(5, LOW);
				digitalWrite(6, LOW);
			}
		}else if(inputString == "stop\n"){
			Serial.println("Stop");
			eDes = ePos;
			eMot.stop();
			rDes = rPos;
			rMot.stop();
		}else if(inputString.startsWith("eDes")){
			String val = inputString.substring(5,10);
			rDes = val.toFloat();
			//Serial.print("rDes is now ");Serial.println(rDes);
		}else if(inputString.startsWith("rDes")){
			String val = inputString.substring(5,10);
			rDes = val.toFloat();
			//Serial.print("rDes is now ");Serial.println(rDes);
		}
		
		inputString = "";
		stringComplete = false;
	}
}

void calcDir(){
	rDir = rDes>rPos?1:-1;
	eDir = eDes>ePos?1:-1;
}

void loop(){
	
	rMot.setSpeed(1);
	eMot.setSpeed(0.5);
	
	processSerial();

	calcDir();
		
	rPos = rSens.getLocation();
	
	if(!rotation || (rDir == 1 && rPos >= rDes - rDes * 0.05) || (rDir == -1 && rPos <= rDes + rDes*0.05) ){
		rMot.stop();
	}else rMot.run(rDir);
	
	Serial.print("rPos = "); Serial.println(rPos);
	Serial.print("rDes = "); Serial.println(rDes);
	Serial.print("rSpeed = "); Serial.println(rSpeed);
	Serial.println();
	
	Serial.print("ePos = "); Serial.println(ePos);
	Serial.print("eDes = "); Serial.println(eDes);
	Serial.print("eSpeed = "); Serial.println(eSpeed);
	Serial.println();
	Serial.println("************************************************");
	Serial.println();

	delay(500);
}

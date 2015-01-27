/* Unit test for the Extension motor and sensor combination
   @author Marien Wolthuis
   date created 21/1/2015			*/
   
#include <RotationMotor.h>
#include <ExtensionMotor.h>
#include <RotationSensor2.h>
#include <ExtensionSensor2.h>
#include <Math.h>

#define FWD_PIN 7
#define REV_PIN 6
#define ENA_PIN 5

#define UP_PIN 10
#define DOWN_PIN 11

#define RPOS_MAX 70
#define RPOS_MIN 0

#define RSFRAC_MIN 0.1
#define ESFRAC_MIN 0.5

bool rotation = true;
bool extension = true;
bool delayed = true;

String inputString = "";
bool stringComplete = false;

volatile float rSpeed; // SI units
volatile float eSpeed;

float rPos, rDes;
float ePos, eDes;
int rDir;
int eDir;
int timer1_counter;

RotationMotor rMot(UP_PIN, DOWN_PIN);
RotationSensor rSens(A0);

ExtensionMotor eMot(FWD_PIN,REV_PIN,ENA_PIN);
ExtensionSensor eSens;

void setup(){
	pinMode(FWD_PIN, OUTPUT); //fwd
	digitalWrite(FWD_PIN, LOW);
	
	pinMode(ENA_PIN, OUTPUT); //dis
	digitalWrite(ENA_PIN, LOW);
	
	pinMode(REV_PIN, OUTPUT); //rev
	digitalWrite(REV_PIN, LOW);
	
	eMot.setSpeed(ESFRAC_MIN);
	
	pinMode(UP_PIN, OUTPUT); //up
	digitalWrite(UP_PIN, LOW);
	
	pinMode(DOWN_PIN, OUTPUT); //down
	digitalWrite(DOWN_PIN, LOW);
	
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
	eSens.setDirection(1);
	eSpeed = eSens.read();
	//Serial.println(eSens.getLocation());
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
				//rMot.setSpeed(1);
				//rMot.run(1);
				//if(delayed) delay(3000);
				//rMot.stop();
				digitalWrite(UP_PIN, HIGH);
				digitalWrite(DOWN_PIN, LOW);
				if(delayed) delay(3000);
				digitalWrite(UP_PIN, LOW);
			}
		}else if(inputString == "down\n"){
			Serial.println("Down");
			if(rotation){
				//rMot.setSpeed(1);
				//rMot.run(-1);
				//if(delayed) delay(3000);
				//rMot.stop();
				
				
				digitalWrite(UP_PIN, LOW);
				digitalWrite(DOWN_PIN, HIGH);
				if(delayed) delay(3000);
				digitalWrite(DOWN_PIN, LOW);
			}
		}else if(inputString == "extend\n"){
			if(extension){
				Serial.println("extending");
				
				eMot.setSpeed(1);
				eMot.run(1);
				if(delayed) delay(300);
				eMot.stop();
				eMot.setSpeed(ESFRAC_MIN);
				eMot.run(1);
				if(delayed) delay(1000);
				eMot.stop();
				
				/*
				digitalWrite(REV_PIN, LOW);
				digitalWrite(FWD_PIN, HIGH); 
				analogWrite(ENA_PIN, 160);
				delay(350); // get past flip point
				analogWrite(ENA_PIN, 80);
				
				delay(2000);
				
				digitalWrite(FWD_PIN, LOW);
				digitalWrite(ENA_PIN, LOW); */
				
				Serial.println("done");
			}
		}else if(inputString == "retract\n"){
			if(extension){
				Serial.println("retracting");
				
				eMot.setSpeed(ESFRAC_MIN);
				eMot.run(-1);
				if(delayed) delay(1000);
				eMot.stop();
				
				
				/*digitalWrite(FWD_PIN, LOW);
				digitalWrite(ENA_PIN, HIGH);
				digitalWrite(REV_PIN, HIGH);
				//analogWrite(ENA_PIN, 180);
				delay(2000);
				
				digitalWrite(REV_PIN, LOW);
				digitalWrite(ENA_PIN, LOW);*/
				
				Serial.println("done");
			}
		}else if(inputString == "stop\n"){
			Serial.println("Stop");
			eDes = ePos;
			eMot.stop();
			rDes = rPos;
			rMot.stop();
			digitalWrite(FWD_PIN, LOW);
			digitalWrite(REV_PIN, LOW);
			digitalWrite(ENA_PIN, LOW);
		}else if(inputString.startsWith("eDes")){
			String val = inputString.substring(5,10);
			rDes = val.toFloat();
			rDes = floor(rDes/9.0)*9.0; // convert to multiples of 9 mm, which is what we're working with
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
	
	//rMot.setSpeed(1);
	//eMot.setSpeed(0.5);
	
	processSerial();

	calcDir();
		
	//rPos = rSens.getLocation();
	ePos = eSens.getLocation();
	
	/*if(!rotation || (rDir == 1 && rPos >= rDes - rDes * 0.05) || (rDir == -1 && rPos <= rDes + rDes*0.05) ){
		rMot.stop();
	}else rMot.run(rDir);
	*/
	
	//if(!extension || (eDir == 1 && ePos >= eDes - eDes * 0.05) || (eDir == -1 && ePos <= eDes + eDes*0.05) ){
		//rMot.stop();
	//}else// rMot.run(rDir);
	/*
	
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

	delay(500);*/
}

/* Test code to find the fractional speed value on which the motor will run
	Serial commands: "rsfrac <0.0 - 1.0>" and "esfrac <0.0 - 1.0>" and "home"(returns it to home position, then sets location to 0)
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


bool rotation = true;
bool extension = true;
bool delayed = true;

String inputString = "";
bool stringComplete = false;

volatile float rSpeed; // SI units
volatile float eSpeed;

float rPos, rDes;
float ePos, eDes;
float eSFrac, rSFrac;
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
	Serial.println(eSens.getLocation()/9.0);
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
		if(inputString.startsWith("esfrac")){
			String val = inputString.substring(7,inputString.length());
			eSFrac = val.toFloat();
			Serial.print("Attempting eSFrac ");Serial.println(eSFrac);
			inputString = "";
			stringComplete = false;
			
			eMot.setSpeed(eSFrac);
			eMot.run(1);
			delay(1000);
			eMot.stop();
			Serial.println("Done.");
		}else if(inputString.startsWith("rsfrac")){ // not fully implemented yet
			Serial.print("Attempting rSFrac ");Serial.println(rSFrac);
			String val = inputString.substring(7,inputString.length());
			rSFrac = val.toFloat();
			rMot.setSpeed(rSFrac);
			rMot.run(1);
			delay(1000);
			rMot.stop();
			Serial.println("Done.");
		}else if(inputString == "home\n"){
			goHome();
		}
		
		inputString = "";
		stringComplete = false;
	}
}

void loop(){
	processSerial();
	
}


void goHome(){
	Serial.print("Homing");
	eMot.setSpeed(0.8);
	eMot.run(-1);
	for(int i=0;i<10;i++){
		delay(125);
		Serial.print(".");
	}Serial.println();
	eMot.stop();
	eSens.setLocation(0.0);
	Serial.print("Should be home now (ePos = ");Serial.print(ePos);Serial.println(").");
}
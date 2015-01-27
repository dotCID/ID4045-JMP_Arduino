/* Test sketch for the motors and sensors
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

#define EPOS_MAX 525
#define EPOS_MIN 0

#define RSFRAC_MIN 0.1
#define ESFRAC_MIN 0.5

#define MOVEMENT_TIMEOUT 2000

#define TILT_POINT 72
#define TILT_FRAC 0.55

int loopCount;
int offlinePrint = true;
bool test = true; // controls output of prints

bool rotation = false;
bool extension = true;

String inputString = "";
bool stringComplete = false;

volatile float rSpeed; // SI units

volatile float eSpeed;
float eSpeedDes = 0.01; // m/s

float eSFrac = ESFRAC_MIN;

float eSpeed_P_accel = 0.1; 			// P controllers
float eSpeed_P_brake = 0.3;

bool eBrake, eRunning, rBrake, rRunning;

float rPos, rDes, rDis;
float ePos, eDes, eDis;
int rDir;
int eDir;
int timer1_counter;

unsigned long movementTime;
bool movementActive = false;
bool beforeTilt = true;

RotationMotor rMot(UP_PIN, DOWN_PIN);
RotationSensor rSens(A0);

ExtensionMotor eMot(FWD_PIN,REV_PIN,ENA_PIN);
ExtensionSensor eSens;

void setup(){
	pinMode(FWD_PIN, OUTPUT); //fwd
	digitalWrite(FWD_PIN, LOW);
	
	pinMode(ENA_PIN, OUTPUT); //enable
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
			}
		}else if(inputString == "down\n"){
			Serial.println("Down");
			if(rotation){
				//rMot.setSpeed(1);
				//rMot.run(-1);
				//if(delayed) delay(3000);
				//rMot.stop();
			}
		}else if(inputString == "extend\n"){
			if(extension){
				Serial.println("extending");
				eDes = EPOS_MAX;
				eRunning = true;
				
				/* Option 2: control the motor class hardcoded
				eMot.setSpeed(1);
				eMot.run(1);
				if(delayed) delay(300);
				eMot.stop();
				eMot.setSpeed(ESFRAC_MIN);
				eMot.run(1);
				if(delayed) delay(1000);
				eMot.stop();
				*/
				/* Option 3: Direct pin control
				digitalWrite(REV_PIN, LOW);
				digitalWrite(FWD_PIN, HIGH); 
				analogWrite(ENA_PIN, 160);
				delay(350); // get past flip point
				analogWrite(ENA_PIN, 80);
				
				delay(2000);
				
				digitalWrite(FWD_PIN, LOW);
				digitalWrite(ENA_PIN, LOW); */
				
				//Serial.println("done");
			}
		}else if(inputString == "retract\n"){
			if(extension){
				Serial.println("retracting");
				eDes = EPOS_MIN;
				eRunning = true;
				
				/* Option 2: control the motor class hardcoded
				eMot.setSpeed(ESFRAC_MIN);
				eMot.run(-1);
				if(delayed) delay(1000);
				eMot.stop();
				*/
				
				/* Option 3: Direct pin control
				digitalWrite(FWD_PIN, LOW);
				digitalWrite(ENA_PIN, HIGH);
				digitalWrite(REV_PIN, HIGH);
				//analogWrite(ENA_PIN, 180);
				delay(2000);
				
				digitalWrite(REV_PIN, LOW);
				digitalWrite(ENA_PIN, LOW);*/
				
				//Serial.println("done");
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
		}else if(inputString == "home\n"){
			goHome();
		}else if(inputString.startsWith("aExt")){
			eRunning = true;
			String val = inputString.substring(5,inputString.length());
			eDes = val.toFloat();
			eDes = floor(eDes/9.0)*9.0; // convert to intervals of 9mm
			Serial.print("eDes is now ");Serial.println(eDes);
		}else if(inputString.startsWith("aRot")){
			String val = inputString.substring(5,inputString.length());
			rDes = val.toFloat();
			//Serial.print("rDes is now ");Serial.println(rDes);
		}
		
		inputString = "";
		stringComplete = false;
	}
}

int speedPID_ext(){
	if(!eRunning) return 0; // quit if we're supposed to stand still
	
	if(!movementActive){
		movementTime = millis();
		movementActive = true;
		if(ePos > eDes) eDis = ePos - eDes;
		else eDis = eDes - ePos;
	}else{
		if(millis()-movementTime>MOVEMENT_TIMEOUT){
			Serial.println("****  TIMEOUT! ****");
			eRunning  = false;
			movementActive = false;
			eMot.stop();
			eSFrac = 0;
			return -1;
		}
	}
	
	if(eDir == 1 && ePos > 0.60 * eDis){
		eBrake = true;
	}else if(eDir == -1 && ePos < 0.60 * eDis){
		eBrake = true;
	}else eBrake = false;
	
	if(!eBrake){
		if(ePos < TILT_POINT){
			eSFrac = TILT_FRAC;
		}else{
			if(beforeTilt){
				eSFrac = ESFRAC_MIN;
				beforeTilt = false;
			}
		}
		
		if(eSFrac < ESFRAC_MIN){
			eSFrac = ESFRAC_MIN;
		}else if(eSpeed < eSpeedDes){
			eSFrac += eSpeed_P_accel * ((eSpeedDes/4.0) - eSpeed);
			if(eSFrac > 1) eSFrac = 1;
		}else if(eSpeed > eSpeedDes){
			eSFrac -= eSpeed_P_accel * (eSpeed - (eSpeedDes/4.0));
			if(eSFrac < ESFRAC_MIN) eSFrac = ESFRAC_MIN;
		}
		
	}else{
		
		if(eSFrac < ESFRAC_MIN){
			eSFrac = ESFRAC_MIN;
		}else if(eSFrac > ESFRAC_MIN){
			eSFrac -= eSpeed_P_brake  * (eSFrac - ESFRAC_MIN);
		}
	}
	return 1;
}


void calcDir(){
	rDir = rDes>rPos?1:-1;
	eDir = eDes>ePos?1:-1;
}

void loop(){
	processSerial();
	
	calcDir();
	eSens.setDirection(eDir);	
	if(eRunning) {
		ePos = eSens.getLocation(); 
	}else{
		eSens.setLocation(ePos); // if it's not under power, assume any changes to be not happening
	}
	
	if(!eRunning){
		if(offlinePrint) { Serial.println("**** OFFLINE ****"); offlinePrint = false;
		Serial.print("ePos = ");Serial.println(ePos); }
		eMot.stop();
		eSFrac = 0;
		movementActive = false;
	}else if((eDir == 1 && ePos >= eDes-1) || (eDir == -1 && ePos <=eDes+1) ){ // if we're close, run free
		offlinePrint = true;
		Serial.println("**** CLOSE CALL! ****");
		Serial.print("ePos = ");Serial.println(ePos);
		eMot.free();
		eSFrac = ESFRAC_MIN;
		eRunning = false;
		movementActive = false;
		
	}else if((eDir == 1 && ePos >= eDes) || (eDir == -1 && ePos <=eDes) ){ // in case of overshoot, stop
		offlinePrint = true;
		Serial.println("**** OVERSHOT! ****");
		Serial.print("ePos = ");Serial.println(ePos);
		eMot.stop();
		eSFrac = 0;
		eRunning = false;
		movementActive = false;
		
	}else{
		offlinePrint = true;
		speedPID_ext();
		
		eMot.setSpeed(eSFrac);
		eMot.run(eDir);
		eSens.setDirection(eDir);
		
	}
	
	loopCount++;
	if(loopCount > 250){
		if(eRunning && test){ Serial.print("ePos = "); Serial.println(ePos); }
		if(eRunning && test){ Serial.print("eDes = "); Serial.println(eDes); }
		if(eRunning && test){ Serial.print("eSFrac = "); Serial.println(eSFrac); }
		if(eRunning && test){ Serial.print("eSpeed = "); Serial.println(eSpeed); }
		if(eRunning && test){ Serial.print("eRunning = "); Serial.println(eRunning); }
		if(eRunning && test) Serial.println();
		loopCount = 0;
	}
}

void goHome(){
	Serial.print("Homing");
	offlinePrint = true;
	eMot.setSpeed(0.7);
	eMot.run(-1);
	for(int i=0;i<20;i++){
		delay(125);
		Serial.print(".");
	}Serial.println();
	eMot.stop();
	eSens.setLocation(0.0);
	ePos = eSens.getLocation();
	eRunning = false;
	beforeTilt = true;
	Serial.println("Should be home now");
}
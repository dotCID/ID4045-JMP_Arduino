/* Sensor Unit test for ID4045 JMP
   @author Marien Wolthuis
   date created 21/1/2015			*/

#include <ExtensionMotor.h>
#include <ExtensionSensor2.h>
#include <Math.h>

#define FWD_PIN 7
#define REV_PIN 6
#define ENA_PIN 5

#define EPOS_MAX 525
#define EPOS_MIN 0

#define ESFRAC_MIN 0.4

#define MOVEMENT_TIMEOUT 2000

#define TILT_POINT 72
#define TILT_FRAC 0.6

int loopCount;
int offlinePrint = true;
bool test = true; // controls output of prints

String inputString = "";
bool stringComplete = false;

volatile float eSpeed; // SI units
float eSpeedDes = 0.01; // m/s

float eSFrac = ESFRAC_MIN;

float eSpeed_P_accel = 0.1; 			// P controllers
float eSpeed_P_brake = 0.8;

bool eBrake, eRunning;
float ePos, eDes;
int eDir;

unsigned long movementTime;
bool movementActive = false;
bool beforeTilt = true;
bool home = false;

ExtensionMotor eMot(FWD_PIN,REV_PIN,ENA_PIN);
ExtensionSensor eSens;

void setup(){
	pinMode(FWD_PIN, OUTPUT); // fwd
	digitalWrite(FWD_PIN, LOW);
	pinMode(REV_PIN, OUTPUT); // rev
	digitalWrite(REV_PIN, LOW);
	pinMode(ENA_PIN, OUTPUT); // enable
	digitalWrite(ENA_PIN, LOW);
	
	attachInterrupt(0, ISR_eSens, FALLING);
	
	Serial.begin(9600);
	
	goHome();
}

void ISR_eSens(){
	//eSens.setDirection(1);
	eSpeed = eSens.read();
	//Serial.println(eSens.getLocation());
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
		if(inputString == "extend\n"){
			eDes = EPOS_MAX;
			eRunning = true;
		}else if(inputString == "retract\n"){
			eDes = EPOS_MIN;
			eRunning = true;
		}else 
		if(inputString == "zeroPosition\n"){
			goHome();
		}else if(inputString.startsWith("aExt")){
			eRunning = true;
			String val = inputString.substring(5,10);
			eDes = val.toFloat();
			eDes = floor(eDes/9.0)*9.0; // convert to intervals of 9mm
			Serial.print("eDes is now ");Serial.println(eDes);
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
	
	if(eDir == 1 && ePos > 0.5 * eDes){
		eBrake = true;
	}else if(eDir == -1 && ePos < 0.5 * eDes){
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
	eDir = eDes>ePos?1:-1;
}

void loop(){
	processSerial();
	
	calcDir();
	eSens.setDirection(eDir);	
	//if(eRunning) {
		ePos = eSens.getLocation(); 
	//}else{
	//	eSens.setLocation(ePos); // if it's not under power, assume any changes to be non relevant
	//}
	
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
	eMot.setSpeed(ESFRAC_MIN);
	eMot.run(-1);
	for(int i=0;i<10;i++){
		delay(125);
		Serial.print(".");
	}Serial.println();
	eMot.stop();
	eSens.setLocation(0.0);
	eRunning = false;
	beforeTilt = true;
	home = true;
	Serial.println("Should be home now");
}
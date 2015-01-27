/* Complete control class for the Arduino
   Can work with the remote control GUI
   @author Marien Wolthuis
   date created 27/1/2015			*/

#include <RotationMotor2.h>
#include <RotationSensor2.h>
#include <ExtensionMotor.h>
#include <ExtensionSensor2.h>
#include <Math.h>

#define UP_PIN 10
#define DOWN_PIN 11

#define RPOS_MAX 1.5
#define RPOS_MIN 0.1

#define RSFRAC_MIN 1

#define FWD_PIN 7
#define REV_PIN 6
#define ENA_PIN 5

#define EPOS_MAX 525
#define EPOS_MIN 0

#define ESFRAC_MIN 0.5

#define E_MOVEMENT_TIMEOUT 2000
#define R_MOVEMENT_TIMEOUT 20000

#define TILT_POINT 72
#define TILT_FRAC 0.55

int loopCount;
int offlinePrint = true;
bool test = true; // controls output of prints

String inputString = "";
bool stringComplete = false;

volatile float rSpeed; // SI units
float rSpeedDes = 0.1; // rad/s
float rSFrac = ESFRAC_MIN;
float rSpeed_P_accel = 0.1; 			// P controllers
float rSpeed_P_brake = 0.3;
bool rBrake, rRunning;
float rPos, rDes, rDis;
int rDir;
bool rMovementActive = false;
unsigned long rMovementTime;
int timer1_counter;

volatile float eSpeed; // SI units
float eSpeedDes = 0.0025; // m/s
float eSFrac = ESFRAC_MIN;
float eSpeed_P_accel = 0.1; 			// P controllers
float eSpeed_P_brake = 0.3;
bool eBrake, eRunning;
float ePos, eDes, eDis;
int eDir;
bool eMovementActive = false;
unsigned long eMovementTime;

bool beforeTilt = true;

float hDes, vDes;
float hPos, vPos;

RotationMotor rMot(UP_PIN, DOWN_PIN);
RotationSensor rSens(A0);

ExtensionMotor eMot(FWD_PIN,REV_PIN,ENA_PIN);
ExtensionSensor eSens;

void setup(){
	pinMode(13,OUTPUT);
	
	pinMode(UP_PIN, OUTPUT); // up
	digitalWrite(UP_PIN, LOW);
	pinMode(DOWN_PIN, OUTPUT); // down
	digitalWrite(DOWN_PIN, LOW);
	pinMode(A0, INPUT);
	
	
	pinMode(FWD_PIN, OUTPUT); // fwd
	digitalWrite(FWD_PIN, LOW);
	pinMode(REV_PIN, OUTPUT); // rev
	digitalWrite(REV_PIN, LOW);
	pinMode(ENA_PIN, OUTPUT); // enable
	digitalWrite(ENA_PIN, LOW);

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

ISR(TIMER1_OVF_vect){
	interrupts();				// enable interrupts because other interrupt has priority
	TCNT1 = timer1_counter;
	rSpeed = rSens.read();
}

void ISR_eSens(){
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
		if(inputString == "stop\n"){
			rRunning = false;
			eRunning = false;
			eMot.stop();
			rMot.stop();
			rDes = rPos;
			eDes = ePos;
			offlinePrint = true;
		}else if(inputString == "blink\n"){
			Serial.println("Match!");
			for(int i=0;i<10;i++){
				digitalWrite(13, HIGH);
				delay(250);
				digitalWrite(13,LOW);
				delay(250);
		 	}
		}else if(inputString == "on\n"){
		    digitalWrite(13,HIGH);
		}else if(inputString == "off\n"){
		    digitalWrite(13,LOW);
		}else if(inputString == "up\n"){
			rDes = RPOS_MAX;
			rRunning = true;
		}else if(inputString == "down\n"){
			rDes = RPOS_MIN;
			rRunning = true;
		}else if(inputString == "extend\n"){
			eDes = EPOS_MAX;
			eRunning = true;
		}else if(inputString == "retract\n"){
			eDes = EPOS_MIN;
			eRunning = true;
		}else if(inputString.startsWith("aExt")){
			eRunning = true;
			String val = inputString.substring(5,inputString.length());
			eDes = val.toFloat();
			eDes = floor(eDes/9.0)*9.0; // convert to intervals of 9mm
			if(eDes<EPOS_MIN) eDes = EPOS_MIN;
			if(eDes>EPOS_MAX) eDes = EPOS_MAX;
			Serial.print("eDes is now ");Serial.println(eDes);
		}else if(inputString.startsWith("aRot")){
			rRunning = true;
			String val = inputString.substring(5,inputString.length());
			rDes = val.toFloat();
			if(rDes<RPOS_MIN) rDes = RPOS_MIN;
			if(rDes>RPOS_MAX) rDes = RPOS_MAX;
			Serial.print("rDes is now ");Serial.println(rDes);
		}else if(inputString.startsWith("hPos")){
			String val = inputString.substring(5,inputString.length());
			hDes = val.toFloat();
			calcERfromHV();
		}else if(inputString.startsWith("vPos")){
			String val = inputString.substring(5,inputString.length());
			vDes = val.toFloat();
			calcERfromHV();
		}else if(inputString.startsWith("getPos")){
			calcHVfromER();
			
			Serial.print("aExt(");
			Serial.print(ePos);
			Serial.print(") aRot(");
			Serial.print(rPos);
			Serial.print(") hPos(");
			Serial.print(hPos);
			Serial.print(") vPos(");
			Serial.print(vPos);
			Serial.println(")");
		}else if(inputString == "home\n"){
			goHome();
		}
		
		inputString = "";
		stringComplete = false;
	}
}

int speedPID_ext(){
	if(!eRunning) return 0; // quit if we're supposed to stand still
	
	if(!eMovementActive){
		eMovementTime = millis();
		eMovementActive = true;
		if(ePos > eDes) eDis = ePos - eDes;
		else eDis = eDes - ePos;
	}else{
		if(millis()-eMovementTime>E_MOVEMENT_TIMEOUT){
			Serial.println("****  TIMEOUT! ****");
			eRunning  = false;
			eMovementActive = false;
			eMot.stop();
			eSFrac = 0;
			return -1;
		}
	}
	
	if(eDir == 1 && (eDes - ePos) > 0.60 * eDis){
		eBrake = true;
	}else if(eDir == -1 && (ePos - eDes) < 0.60 * eDis){
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
			eSFrac += eSpeed_P_accel * (eSpeedDes - eSpeed);
			if(eSFrac > 1) eSFrac = 1;
		}else if(eSpeed > eSpeedDes){
			eSFrac -= eSpeed_P_accel * (eSpeed - eSpeedDes);
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


int speedPID_rot(){
	if(!rRunning) return 0; // quit if we're supposed to stand still
	
	if(!rMovementActive){
		rMovementTime = millis();
		rMovementActive = true;
		if(rPos > rDes) rDis = rPos - rDes;
		else rDis = rDes - rPos;
		
		Serial.print("///// rDis: ");Serial.println(rDis);
	}else{
		if(millis()-rMovementTime>R_MOVEMENT_TIMEOUT){
			Serial.println("****  TIMEOUT! ****");
			rRunning  = false;
			rMovementActive = false;
			rMot.stop();
			rSFrac = 0;
			return -1;
		}
	}
	
	if(rDir == 1 && (rDes - rPos) > 0.95 * rDis){
		rBrake = true;
	}else if(rDir == -1 && (rPos - rDes) < 0.95 * rDis){
		rBrake = true;
	}else rBrake = false;
	
	if(!rBrake){
		if(rSFrac < RSFRAC_MIN){
			rSFrac = RSFRAC_MIN;
		}else if(rSpeed < rSpeedDes){
			rSFrac += rSpeed_P_accel * (rSpeedDes - rSpeed);
			if(rSFrac > 1) rSFrac = 1;
		}else if(rSpeed > rSpeedDes){
			rSFrac -= rSpeed_P_accel * (rSpeed - rSpeedDes);
			if(rSFrac < RSFRAC_MIN) rSFrac = RSFRAC_MIN;
		}
		
	}else{
		
		if(rSFrac < RSFRAC_MIN){
			rSFrac = RSFRAC_MIN;
		}else if(rSFrac > RSFRAC_MIN){
			rSFrac -= rSpeed_P_brake  * (rSFrac - RSFRAC_MIN);
		}
	}
	return 1;
}


void calcDir(){
	eDir = eDes>ePos?1:-1;
	rDir = rDes>rPos?1:-1;
}

void calcERfromHV(){
	rDes = atan2(vDes, hDes);
	if(rDes>RPOS_MAX) rDes = RPOS_MAX;
	if(rDes<RPOS_MIN) rDes = RPOS_MIN;
	
	eDes = vDes / sin(rDes);
	if(eDes>EPOS_MAX) eDes = EPOS_MAX;
	if(eDes<EPOS_MIN) eDes = EPOS_MIN;
}

void calcHVfromER(){
	hPos = cos(rPos) * ePos;
	vPos = sin(rPos) * ePos;
}

void loop(){
	processSerial();
	calcDir();
		
	if(eRunning) {
		ePos = eSens.getLocation(); 
	}else{
		eSens.setLocation(ePos); // if it's not under power, assume any changes to be non relevant
	}
	
	rPos = rSens.getLocation();
	
	eSens.setDirection(eDir);	
	moveToDestination();
	
	loopCount++;
	if(loopCount > 250){
		if(eRunning && test){ Serial.print("ePos = "); Serial.println(ePos); }
		if(eRunning && test){ Serial.print("eDes = "); Serial.println(eDes); }
		if(eRunning && test){ Serial.print("eSFrac = "); Serial.println(eSFrac); }
		if(eRunning && test){ Serial.print("eSpeed = "); Serial.println(eSpeed); }
		if(eRunning && test){ Serial.print("eRunning = "); Serial.println(eRunning); }
		if(eRunning && test) Serial.println();
		
		
		if(rRunning && test){ Serial.print("rPos = "); Serial.println(rPos); }
		if(rRunning && test){ Serial.print("rDes = "); Serial.println(rDes); }
		if(rRunning && test){ Serial.print("rSFrac = "); Serial.println(rSFrac); }
		if(rRunning && test){ Serial.print("rSpeed = "); Serial.println(rSpeed); }
		if(rRunning && test){ Serial.print("rRunning = "); Serial.println(rRunning); }
		if(rRunning && test) Serial.println();
		loopCount = 0;
	}
}

void moveToDestination(){
	if(!eRunning && !rRunning){
		if(offlinePrint) { 
			Serial.println("**** OFFLINE ****"); offlinePrint = false;
			Serial.print("ePos = ");Serial.println(ePos); 
			Serial.print("rPos = ");Serial.println(rPos); 
		}
		rMot.stop();
		rSFrac = 0;
		rMovementActive = false;
		
		eMot.stop();
		eSFrac = 0;
		eMovementActive = false;
		return;
	}
	
	// Extension destination checking and moving
	if(eRunning && ((eDir == 1 && ePos >= eDes-1) || (eDir == -1 && ePos <=eDes+1))){ // if we're close stop
		offlinePrint = true;
		Serial.println("**** EXTENSION: CLOSE CALL! ****");
		Serial.print("ePos = ");Serial.println(ePos);
		Serial.print("eDes = ");Serial.println(eDes);
		eMot.stop();
		eSFrac = ESFRAC_MIN;
		eRunning = false;
		eMovementActive = false;
		
	}else if(eRunning && ((eDir == 1 && ePos >= eDes) || (eDir == -1 && ePos <=eDes)) ){ // in case of overshoot, stop
		offlinePrint = true;
		Serial.println("**** EXTENSION: OVERSHOT! ****");
		Serial.print("ePos = ");Serial.println(ePos);
		Serial.print("eDes = ");Serial.println(eDes);
		eMot.stop();
		eSFrac = 0;
		eRunning = false;
		eMovementActive = false;
		
	}else{
		offlinePrint = true;
		speedPID_ext();
		
		eMot.setSpeed(eSFrac);
		eMot.run(eDir);
		eSens.setDirection(eDir);
		
	}
	
	// same for rotation
	if(rRunning && ((rDir == 1 && rPos >= rDes-0.01) || (rDir == -1 && rPos <=rDes+0.01))){ // if we're close
		offlinePrint = true;
		Serial.println("**** ROTATION: CLOSE CALL! ****");
		Serial.print("rPos = ");Serial.println(rPos);
		Serial.print("rDes = ");Serial.println(rDes);
		rMot.stop();
		//rSFrac = RSFRAC_MIN;
		rRunning = false;
		rMovementActive = false;
		
	}else if(rRunning && ((rDir == 1 && rPos >= rDes) || (rDir == -1 && rPos <=rDes) )){ // in case of overshoot, stop
		offlinePrint = true;
		Serial.println("**** ROTATION: OVERSHOT! ****");
		Serial.print("rPos = ");Serial.println(rPos);
		Serial.print("rDes = ");Serial.println(rDes);
		rMot.stop();
		//rSFrac = 0;
		rRunning = false;
		rMovementActive = false;
		
	}else{
		offlinePrint = true;
		//speedPID_rot();
		rMot.run(rDir);	
	}
}

void goHome(){
	Serial.print("Homing");
	offlinePrint = true;
	rDes = RPOS_MIN;
	rRunning = true;
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
	Serial.println("Extension should be home now");
}
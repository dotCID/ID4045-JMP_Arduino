/* Arduino Code for JMP, version 0.8
   @author Marien Wolthuis
   date created 16/1/2015			*/
   
#include <ExtensionMotor.h>
#include <RotationMotor.h>
#include <ExtensionSensor.h>
#include <RotationSensor2.h> // experimental one
#include <Math.h>

#define EPOS_MAX 70
#define EPOS_MIN 0

#define ESFRAC_MIN 0.1
#define RSFRAC_MIN 0.1

/**************************
 *	  Testing booleans	  *
 **************************/
bool test = true; 
bool PIDtest = true;
float old_SFrac;
/**************************/

String inputString = "";
bool stringComplete = false;

float hPos, hDes, vPos, vDes;
float eSpeed, rSpeed; // SI units
float eSpeedDes = 0.5; // m/s
float rSpeedDes = 1.0; // d/s
float eSFrac = ESFRAC_MIN;
float rSFrac = RSFRAC_MIN;
float eSpeed_P_accel = 0.1; // P controllers
float eSpeed_P_brake = 0.4;
float rSpeed_P = 0.1;
bool eBrake, rBrake, eRunning, rRunning;
float ePos, eDes, rPos, rDes;
int eDir, rDir;
int timer1_counter;

void setup(){
	Serial.begin(9600);
}

int speedPID_ext(){
	if(!eRunning) return 0; // quit if we're supposed to stand still
	
	if(eDir == 1 && ePos > 0.75 * eDes){
		eBrake = true;
	}else if(eDir == -1 && ePos < 0.75 * eDes){
		eBrake = true;
	}else eBrake = false;
	
	if(!eBrake){
		if(eSpeed < eSpeedDes){
			eSFrac += eSFrac * eSpeed_P_accel;
			if(eSFrac > 1) eSFrac = 1;
		}else if(eSpeed > eSpeedDes){
			eSFrac -= eSFrac * eSpeed_P_accel;
			if(eSFrac < ESFRAC_MIN) eSFrac = ESFRAC_MIN;
		}
		
	}else{
		
		if(eSFrac < ESFRAC_MIN){
			if(eSFrac < ESFRAC_MIN) eSFrac = ESFRAC_MIN;
		}else if(eSFrac > ESFRAC_MIN){
			eSFrac -= eSFrac * eSpeed_P_brake;
		}
	}
}

void calcDir(){
	eDir = eDes>ePos?1:-1;
	rDir = rDes>rPos?1:-1;
}

void loop(){	
	processSerial();

	calcDir();
	
	if((eDir == 1 && ePos >= eDes-1) || (eDir == -1 && ePos <=eDes+1) ){ // if we're close..
		//eMot.free();
		eRunning = false;
		
		if(PIDtest) eSpeed = 0;
	}else if((eDir == 1 && ePos >= eDes) || (eDir == -1 && ePos <=eDes) ){ // in case of overshoot, stop
		//eMot.stop();
		eRunning = false;
		
		if(PIDtest) eSpeed = 0;
	}else{
		if(PIDtest) {if(eSpeed == 0){ eSpeed = 0.1;} old_SFrac = rSFrac;}
		
		eRunning = true;
		speedPID_ext();
		
		if(PIDtest) { eSpeed += (eSpeed * (eSFrac - old_SFrac))*eDir; };

		//eMot.setSpeed(eSFrac);
		//eMot.run(eDir);
		
		if(PIDtest) ePos += eSpeed; 
	}
	


if(test) Serial.print("eDir = "); Serial.println(eDir);

if(test) Serial.print("ePos = "); Serial.println(ePos);
if(test) Serial.print("eDes = "); Serial.println(eDes);
if(test) Serial.print("eSFrac = "); Serial.println(eSFrac);
if(test) Serial.print("eSpeed = "); Serial.println(eSpeed);
if(test) Serial.println();


if(test) delay(1000);
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
		if(inputString == "blink\n"){
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
		}else if(inputString == "extend\n"){
			eDes = EPOS_MAX;
		}else if(inputString == "retract\n"){
			eDes = EPOS_MIN;
		}else if(inputString.startsWith("aExt")){
			String val = inputString.substring(5,10);
			eDes = val.toFloat();
		}else if(inputString.startsWith("aRot")){
			String val = inputString.substring(5,10);
			rDes = val.toFloat();
		}else if(inputString.startsWith("hPos")){
			String val = inputString.substring(5,10);
			hDes = val.toFloat();
		}else if(inputString.startsWith("vPos")){
			String val = inputString.substring(5,10);
			vDes = val.toFloat();
		}else if(inputString.startsWith("getPos")){
			Serial.print("aExt(");
			Serial.print(ePos);
			Serial.print(") aRot(");
			Serial.print(rPos);
			Serial.print(") hPos(");
			Serial.print(hPos);
			Serial.print(") vPos(");
			Serial.print(vPos);
			Serial.println(")");
		}
		inputString = "";
		stringComplete = false;
	}
}
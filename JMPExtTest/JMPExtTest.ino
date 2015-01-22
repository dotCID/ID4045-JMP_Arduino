/* Unit test for the Extension motor and sensor combination
   @author Marien Wolthuis
   date created 21/1/2015			*/
   
#include <ExtensionMotor.h>
#include <ExtensionSensor2.h>
#include <Math.h>

#define EPOS_MAX 70
#define EPOS_MIN 0

#define ESFRAC_MIN 0.1

/**************************
 *	  Testing values	  *
 **************************/
bool test = false; 
/**************************/
						
String inputString = "";
bool stringComplete = false;

float hPos, hDes, vPos, vDes;
volatile float eSpeed; // SI units
float eSpeedDes = 0.5; // m/s

float eSFrac = ESFRAC_MIN;

float eSpeed_P_accel = 0.3; 			// P controllers
float eSpeed_P_brake = 0.4;

bool eBrake, eRunning;
float ePos, eDes;
int eDir;
int timer1_counter;

ExtensionMotor eMot(5, 6, 7);
ExtensionSensor eSens;

void setup(){
	pinMode(5, OUTPUT); //fwd
	pinMode(7, OUTPUT); //rev
	pinMode(6, OUTPUT); //dis
	
	attachInterrupt(0, ISR_eSens, RISING);
	
	Serial.begin(9600);
}

void ISR_eSens(){
	eSpeed = eSens.read();
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
		}else if(inputString == "retract\n"){
			eDes = EPOS_MIN;
		}else if(inputString.startsWith("aExt")){
			String val = inputString.substring(5,10);
			eDes = val.toFloat();
		}
		
		inputString = "";
		stringComplete = false;
	}
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

void calcDir(){
	eDir = eDes>ePos?1:-1;
}

void loop(){	
	processSerial();

	calcDir();
		
	ePos = eSens.getLocation();
	
	if((eDir == 1 && ePos >= eDes-1) || (eDir == -1 && ePos <=eDes+1) ){ // if we're close, run free
		eMot.free();
		eRunning = false;
		
	}else if((eDir == 1 && ePos >= eDes) || (eDir == -1 && ePos <=eDes) ){ // in case of overshoot, stop
		eMot.stop();
		eRunning = false;
		
	}else{
		
		eRunning = true;
		speedPID_ext();
		
		eMot.setSpeed(eSFrac);
		eMot.run(eDir);
		eSens.setDirection(eDir);
		
	}
	
	if(test) Serial.print("ePos = "); Serial.println(ePos);
	if(test) Serial.print("eDes = "); Serial.println(eDes);
	if(test) Serial.print("eSFrac = "); Serial.println(eSFrac);
	if(test) Serial.print("eSpeed = "); Serial.println(eSpeed);
	if(test) Serial.println();


	if(test) delay(1000);
}

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
/**************************/
 

int dPinModes[14][3] = {{0,  INPUT,  LOW},	
						{1,  INPUT,  LOW},
						{2,  INPUT,  LOW},		// ISR 0
						{3,  INPUT,  LOW},		// ~ ISR 1
						{4,  INPUT,  LOW},
						{5,  OUTPUT, HIGH},		// ~ 
						{6,  OUTPUT, HIGH},		// ~ eMot.enable
						{7,  OUTPUT, LOW},		// eMot.fwd
						{8,  OUTPUT, LOW},		// eMot.bck
						{9,  INPUT,  LOW},
						{10, INPUT,  LOW},		// ~ rMot.pin1
						{11, INPUT,  LOW},		// ~ rMot.pin2
						{12, INPUT,  LOW},
						{13, OUTPUT, LOW}};		// LED for testing
						
int aPinModes[6][2] = {{A0, INPUT},
					   {A1, INPUT},
					   {A2, INPUT},
					   {A3, INPUT},
					   {A4, INPUT},
					   {A5, INPUT}};

						
String inputString = "";
bool stringComplete = false;

float hPos, hDes, vPos, vDes;
float eSpeed, rSpeed; // SI units
float eSpeedDes = 0.5; // m/s
float rSpeedDes = 1.0; // d/s
float eSpeedMin = 0.1;
float rSpeedMin = 0.1;
float eSFrac = ESFRAC_MIN;
float rSFrac = RSFRAC_MIN;
float eSpeed_P = 0.1; // P controllers
float rSpeed_P = 0.1;
bool eBrake, rBrake;
float ePos, eDes, rPos, rDes;
int eDir, rDir;
int timer1_counter;

ExtensionMotor eMot(7, 8, 6);
RotationMotor rMot(10, 11);
ExtensionSensor eSens(3);
RotationSensor rSens(A1);

void setup(){
	for(int i=0;i<14;i++){
		pinMode(dPinModes[i][0],dPinModes[i][1]);
		digitalWrite(dPinModes[i][0],dPinModes[i][2]);
	}
	
	for(int i=0;i<6;i++){
		pinMode(aPinModes[i][0],aPinModes[i][1]);
		digitalWrite(aPinModes[i][0],aPinModes[i][2]);
	}
	
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
if(!test)	eSpeed = eSens.read();
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

void speedPID_ext(){
	if(eDir == 1 && ePos > 0.75 * eDes){
		eBrake = true;
	}else if(eDir == -1 && ePos < 0.75 * eDes){
		eBrake = true;
	}
	
	if(!eBrake){
		if(eSpeed < eSpeedDes){
			eSFrac += eSFrac * eSpeed_P;
			if(eSFrac > 1) eSFrac = 1;
		}else if(eSpeed > eSpeedDes){
			eSFrac -= eSFrac * eSpeed_P;
			if(eSFrac < ESFRAC_MIN) eSFrac = ESFRAC_MIN;
		}
	}else{
		if(eSpeed < eSpeedMin){
			eSFrac += eSFrac * eSpeed_P;
			if(eSFrac > 1) eSFrac = 1;
		}else if(eSpeed > eSpeedMin){
			eSFrac -= eSFrac * eSpeed_P;
			if(eSFrac < ESFRAC_MIN) eSFrac = ESFRAC_MIN;
		}
	}
}

void speedPID_rot(){
	if(rDir == 1 && rPos > 0.75 * rDes){
		rBrake = true;
	}else if(rDir == -1 && rPos < 0.75 * rDes){
		rBrake = true;
	}

	if(!rBrake){
		if(rSpeed < rSpeedDes){
			rSFrac += rSFrac * rSpeed_P;
			if(rSFrac > 1) rSFrac = 1;
		}else if(rSpeed > rSpeedDes){
			rSFrac -= rSFrac * rSpeed_P;
			if(rSFrac < RSFRAC_MIN) rSFrac = RSFRAC_MIN;
		}
	}else{
		if(rSpeed < rSpeedMin){
			rSFrac += rSFrac * rSpeed_P;
			if(rSFrac > 1) rSFrac = 1;
		}else if(rSpeed > rSpeedMin){
			rSFrac -= rSFrac * rSpeed_P;
			if(rSFrac < RSFRAC_MIN) rSFrac = RSFRAC_MIN;
		}
	}
}

void calcDir(){
	eDir = eDes>ePos?1:-1;
	rDir = rDes>rPos?1:-1;
}

void calcERfromHV(){
	rDes = atan2(vDes, hDes);
	eDes = vDes / sin(rDes);
}

void calcHVfromER(){
	hPos = cos(rPos) * ePos;
	vPos = sin(rPos) * ePos;
}

void loop(){
	float old_SFrac = 0;
	
	processSerial();

	calcDir();
	
	//calcERfromHV();
	
	//ePos = eSens.getLocation();
	//rPos = rSens.getLocation();
	//calcHVfromER();
	
	if(ePos >= eDes){
		eMot.stop();
if(PIDtest) eSpeed = 0;
		eBrake = false;
	}else{

if(PIDtest) {if(eSpeed == 0) eSpeed = 0.1;  old_SFrac = rSFrac;}

		speedPID_ext();
		
if(PIDtest) { eSpeed += eSpeed * (eSFrac - old_SFrac); };

		eMot.setSpeed(eSFrac);
		eMot.run(eDir);
		
if(PIDtest) ePos += eSpeed; 
	}
	
	if(rPos >= rDes){
		rMot.stop();
		rBrake = false;
	}else{
if(PIDtest) { if(rSpeed == 0) rSpeed = 0.1; old_SFrac = rSFrac;}

		speedPID_rot();

if(PIDtest) { rSpeed += rSpeed * (rSFrac - old_SFrac); };

		rMot.setSpeed(rSFrac);
		rMot.run(rDir);

if(PIDtest) rPos += rSpeed; 
	}
if(test) Serial.print("ePos = "); Serial.println(ePos);
if(test) Serial.print("eDes = "); Serial.println(eDes);
if(test) Serial.print("eSFrac = "); Serial.println(eSFrac);
if(test) Serial.print("eSpeed = "); Serial.println(eSpeed);
if(test) Serial.print("rSFrac = "); Serial.println(rSFrac);
if(test) Serial.print("rSpeed = "); Serial.println(rSpeed);
if(test) Serial.println();


if(test) delay(1000);
}

ISR(TIMER1_OVF_vect){
	interrupts();				// enable interrupts because other interrupt has priority
	TCNT1 = timer1_counter;
	rSpeed = rSens.read();
}

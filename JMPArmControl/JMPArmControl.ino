/* Arduino Code for JMP, version 0.3
   @author Marien Wolthuis
   date created 16/1/2015			*/
   
#include <ExtensionMotor.h>
#include <RotationMotor.h>
#include <ExtensionSensor.h>
#include <RotationSensor2.h> // experimental one
#include <Math.h>

#define EPOS_MAX 70
#define EPOS_MIN 0

int dPinModes[14][3] = {{0,  INPUT,  LOW},	
						{1,  INPUT,  LOW},
						{2,  INPUT,  LOW},		// ISR 0
						{3,  INPUT,  LOW},		// ~ ISR 1
						{4,  INPUT,  LOW},
						{5,  OUTPUT, HIGH},		// ~ eMot.pchan1, P-channels determine speed
						{6,  OUTPUT, HIGH},		// ~ eMot.pchan2, P-channels determine speed
						{7,  OUTPUT, LOW},		// eMot.nchan1
						{8,  OUTPUT, LOW},		// eMot.nchan2
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
float eSFrac, rSFrac;
float eSpeed_P = 0.1; // P controllers
float rSpeed_P = 0.1;
bool eBrake, rBrake;
float ePos, eDes, rPos, rDes;
int eDir, rDir;
int timer1_counter;

ExtensionMotor eMot(5, 5, 7, 8);
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
	eSpeed = eSens.read();
}

void processSerial(){
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

void speedPID(){
	if(eDir == 1 && ePos < 0.75 * eDes){ // if(eDir==1)
		eBrake = true;
	}else if(eDir == -1 && ePos > 0.75 * eDes){
		eBrake = true;
	}
	
	if(!eBrake){
		if(eSpeed < eSpeedDes){
			eSFrac += eSFrac * eSpeed_P;
		}else if(eSpeed > eSpeedDes){
			eSFrac -= eSFrac * eSpeed_P;
		}
	}else{
		if(eSpeed < eSpeedMin){
			eSFrac += eSFrac * eSpeed_P;
		}else if(eSpeed > eSpeedMin){
			eSFrac -= eSFrac * eSpeed_P;
		}
	}
	
	
	if(rDir == 1 && rPos < 0.75 * rDes){ // if(eDir==1)
		rBrake = true;
	}else if(rDir == -1 && rPos > 0.75 * rDes){
		rBrake = true;
	}

	if(!rBrake){
		if(rSpeed < rSpeedDes){
			rSFrac += rSFrac * rSpeed_P;
		}else if(rSpeed > rSpeedDes){
			rSFrac -= rSFrac * rSpeed_P;
		}
	}else{
		if(rSpeed < rSpeedMin){
			rSFrac += rSFrac * rSpeed_P;
		}else if(rSpeed > rSpeedMin){
			rSFrac -= rSFrac * rSpeed_P;
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
	processSerial();
	
	speedPID();
	eMot.setSpeed(eSFrac);
	rMot.setSpeed(rSFrac);
	calcDir();
	
	calcERfromHV();
	
	ePos = eSens.getLocation();
	rPos = rSens.getLocation();
	calcHVfromER();
	
	if(ePos == eDes) 
		eMot.stop();
	else
		eMot.run(eDir);
	
	if(rPos == rDes)
		rMot.stop();
	else
		rMot.run(rDir);
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}


ISR(TIMER1_OVF_vect){
	interrupts();				// enable interrupts because other interrupt has priority
	TCNT1 = timer1_counter;
	rSpeed = rSens.read();
}

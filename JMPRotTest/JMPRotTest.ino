/* Unit test for the Extension motor and sensor combination
   @author Marien Wolthuis
   date created 21/1/2015			*/
   
#include <RotationMotor.h>
#include <RotationSensor2.h>
#include <Math.h>

#define RPOS_MAX 70
#define RPOS_MIN 0

#define RSFRAC_MIN 0.1

/**************************
 *	  Testing values	  *
 **************************/
bool test = false; 
/**************************/
						
String inputString = "";
bool stringComplete = false;

volatile float rSpeed; // SI units
float rSpeedDes = 0.5; // rad/s

float rSFrac = RSFRAC_MIN;

float rSpeed_P_accel = 0.3; 			// P controllers
float rSpeed_P_brake = 0.4;

bool rBrake, rRunning;
float rPos, rDes;
int rDir;
int timer1_counter;

RotationMotor rMot(10, 11);
RotationSensor rSens(A0);

void setup(){
	pinMode(10, OUTPUT); //fwd
	pinMode(11, OUTPUT); //rev
	
	noInterrupts();           
  	TCCR1A = 0;
  	TCCR1B = 0;
 	timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
   	TCNT1 = timer1_counter;   // preload timer
  	TCCR1B |= (1 << CS12);    // 256 prescaler 
  	TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  	interrupts();             
	
	Serial.begin(9600);
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
			rDes = RPOS_MAX;
		}else if(inputString == "down\n"){
			rDes = RPOS_MIN;
		}else if(inputString.startsWith("aRot")){
			String val = inputString.substring(5,10);
			rDes = val.toFloat();
		}
		
		inputString = "";
		stringComplete = false;
	}
}

int speedPID_rot(){
	if(!rRunning) return 0; // quit if we're supposed to stand still
	
	if(rDir == 1 && rPos > 0.75 * rDes){
		rBrake = true;
	}else if(rDir == -1 && rPos < 0.75 * rDes){
		rBrake = true;
	}else rBrake = false;
	
	if(!rBrake){
		if(rSpeed < rSpeedDes){
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
	rDir = rDes>rPos?1:-1;
}

void loop(){	
	processSerial();

	calcDir();
		
	rPos = rSens.getLocation();
	
	if((rDir == 1 && rPos >= rDes-1) || (rDir == -1 && rPos <=rDes+1) ){ // if we're close, run free
		rMot.free();
		rRunning = false;
		
	}else if((rDir == 1 && rPos >= rDes) || (rDir == -1 && rPos <=rDes) ){ // in case of overshoot, stop
		rMot.stop();
		rRunning = false;
		
	}else{
		
		rRunning = true;
		speedPID_rot();
		
		rMot.setSpeed(rSFrac);
		rMot.run(rDir);
		rSens.setDirection(rDir);
		
	}
	
	if(test) Serial.print("rPos = "); Serial.println(rPos);
	if(test) Serial.print("rDes = "); Serial.println(rDes);
	if(test) Serial.print("rSFrac = "); Serial.println(rSFrac);
	if(test) Serial.print("rSpeed = "); Serial.println(rSpeed);
	if(test) Serial.println();


	if(test) delay(1000);
}

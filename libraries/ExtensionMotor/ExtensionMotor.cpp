/* Arduino Code for JMP, version 0.1
   Motor controller class
   @author Marien Wolthuis
   date created 16/1/2015			*/
 
#include <Arduino.h>
#include <ExtensionMotor.h>

static int _fwd[4] = {0,1,0,1};
static int _bck[4] = {1,0,1,0};
static int _stop[4] ={0,0,0,0};
static int _free[4] ={1,1,0,0};

ExtensionMotor::ExtensionMotor(int pchan1, int pchan2, int nchan1, int nchan2){
	_pins[0] = pchan1;
	_pins[1] = pchan2;
	_pins[2] = nchan1;
	_pins[3] = nchan2;
	
	ExtensionMotor::stop();
}

bool ExtensionMotor::run(int direction){

	return false; 
}

bool ExtensionMotor::stop(){
	for(int i=0;i<4;i++)
		digitalWrite(_pins[i],_stop[i]);
	
	return true;
}

bool ExtensionMotor::free(){
	for(int i=0;i<4;i++)
		digitalWrite(_pins[i],_free[i]);
	
	return true;
}

float ExtensionMotor::getSpeed(){
	return (_speed / 255);
}

void ExtensionMotor::setSpeed(float speed){
	_speed = (int) (speed * 255);
}

int ExtensionMotor::getDirection(){
	return _direction;
}

bool ExtensionMotor::isActive(){
	return _active;
}


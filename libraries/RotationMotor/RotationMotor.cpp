/* Arduino Code for JMP, version 0.1
   Motor controller class
   @author Marien Wolthuis
   date created 20/1/2015			*/

#include <Arduino.h>
#include <RotationMotor.h>

#define FORWARD 1
#define BACKWARD -1

static int _fwd[2]  = {1,0};
static int _bck[2]  = {0,1};
static int _stop[2] = {1,1};
static int _free[2] = {0,0};

RotationMotor::RotationMotor(int pin1, int pin2){
	_pins[0] = pin1;
	_pins[1] = pin2;
	
	RotationMotor::stop();
}

bool RotationMotor::run(int direction){
	_direction = direction;
	if(direction == FORWARD){
		for(int i=0;i<2;i++)
			analogWrite(_pins[i], (_speed * _fwd[i]));
		return true;
	}else if(direction == BACKWARD){
		for(int i=0;i<2;i++)
			analogWrite(_pins[i], (_speed * _bck[i]));
		return true;
	}
	
	return false;
}

bool RotationMotor::stop(){
	for(int i=0;i<2;i++)
		digitalWrite(_pins[i],_stop[i]);
	return true;
}

bool RotationMotor::free(){
	for(int i=0;i<2;i++)
		digitalWrite(_pins[i],_free[i]);
}

float RotationMotor::getSpeed(){
	return (_speed / 255);
}

void RotationMotor::setSpeed(float speed){
	_speed = (int) (speed * 255);
}

int RotationMotor::getDirection(){
	return _direction;
}

bool RotationMotor::isActive(){
	return _active;
}


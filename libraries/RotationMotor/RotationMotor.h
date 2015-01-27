/* Arduino Code for JMP, version 0.1
   Motor controller class
   @author Marien Wolthuis
   date created 16/1/2015			*/
   
#ifndef RotationMotor_h
#define RotationMotor_h

#include <Arduino.h>

class RotationMotor
{
public:
	RotationMotor(int pin1, int pin2);
	bool run(int direction);
	bool stop();
	
	float getSpeed();
	void setSpeed(float speed);
	
	int getDirection();
	
	bool isActive();
	
private:
	int _pins[2];
	int _speed;
	int _direction;
	bool _active;
};

#endif

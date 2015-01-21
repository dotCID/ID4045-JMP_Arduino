/* Arduino Code for JMP, version 0.1
   Motor controller class
   @author Marien Wolthuis
   date created 20/1/2015			*/
   
#ifndef ExtensionMotor_h
#define ExtensionMotor_h

#include <Arduino.h>

class ExtensionMotor
{
public:
	ExtensionMotor(int pchan1, int pchan2, int nchan1, int nchan2);
	bool run(int direction);
	bool stop();
	bool free();
	
	float getSpeed();
	void setSpeed(float speed);
	
	int getDirection();
	
	bool isActive();
	
private:
	int _pins[4];
	int _speed;
	int _direction;
	bool _active;
	int _fwd[4];
	int _bck[4];
	int _stop[4];
	int _free[4];
};

#endif

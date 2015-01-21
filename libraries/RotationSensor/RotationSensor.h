/* Arduino Code for JMP, version 0.1
   Sensor controller class
   @author Marien Wolthuis
   date created 20/1/2015			*/
   
#ifndef RotationSensor_h
#define RotationSensor_h

#include <Arduino.h>

class RotationSensor
{
public:
	RotationSensor(int pin);
	float read();
	
	float lastReading();
	float currentReading();
	void setDirection(int direction);
	void setLocation(float location);
	float getLocation();
	
private:
	int _pin;
	int _lastReading;
	int _currentReading;
	unsigned long _lastTime;
	unsigned long _curTime;
	int _direction;
	float _location;
	float _speed;
	float _resolution;
};

#endif

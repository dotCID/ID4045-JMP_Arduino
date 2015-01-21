/* Arduino Code for JMP, version 0.1
   Sensor controller class
   @author Marien Wolthuis
   date created 20/1/2015			*/
   
#ifndef RotationSensor2_h
#define RotationSensor2_h

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
	float _readings[3];
	int _t_0, _t_1, _t_2, _read_index; // tracks last write index
	unsigned long _readTimes[3];
	int _direction;
	float _speed;
	float _multiplier;
};

#endif

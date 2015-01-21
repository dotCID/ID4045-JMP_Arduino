/* Arduino Code for JMP, version 0.1
   Sensor controller class
   @author Marien Wolthuis
   date created 20/1/2015			*/
   
#ifndef ExtensionSensor_h
#define ExtensionSensor_h

#include <Arduino.h>

class ExtensionSensor
{
public:
	ExtensionSensor(int pin);
	float read();
	
	unsigned long lastReading();
	unsigned long currentReading();
	void setDirection(int direction);
	void setLocation(float location);
	float getLocation();
	
private:
	int _pin;
	unsigned long _lastTime;
	unsigned long _currentTime;
	int _direction;
	float _step;
	float _location;
	float _speed;
};

#endif

/* Arduino Code for JMP, version 0.1
   Sensor controller class
   @author Marien Wolthuis
   date created 20/1/2015			*/
   
#ifndef ExtensionSensor2_h
#define ExtensionSensor2_h

#include <Arduino.h>

class ExtensionSensor
{
public:
	ExtensionSensor();
	float read();
	
	unsigned long lastReading();
	unsigned long currentReading();
	void setDirection(int direction);
	void setLocation(float location);
	float getLocation();
	
private:
	unsigned long _readings[3];
	int _t_0, _t_1, _t_2, _read_index; // tracks last write index
	int _direction;
	float _step;
	float _location;
	float _speed;
};

#endif

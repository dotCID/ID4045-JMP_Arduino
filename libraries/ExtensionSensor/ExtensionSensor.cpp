#include <Arduino.h>
#include <ExtensionSensor.h>


ExtensionSensor::ExtensionSensor(){
	_step = 9; // 9mm?
}

float ExtensionSensor::read(){
	_lastTime = _currentTime;
	_currentTime = millis();
	_speed = _step / (_currentTime - _lastTime); // m / s
//	_location+= _step*_direction;
	return _speed;
}
	
unsigned long ExtensionSensor::lastReading(){
	return _lastTime;
}

unsigned long ExtensionSensor::currentReading(){
	return _currentTime;
}

void ExtensionSensor::setLocation(float location){
	_location = location;
}

float ExtensionSensor::getLocation(){
	return _location;
}

void ExtensionSensor::setDirection(int direction){
	if(direction == -1 || direction == 1)
		_direction = direction;
}


#include <Arduino.h>
#include <RotationSensor.h>


RotationSensor::RotationSensor(int pin){
	_resolution = 280/1024 * M_PI / 180; // conversion value from degrees/resolution -> radians/measured_value
	_pin = pin;
}

float RotationSensor::read(){
	_lastTime = _curTime;
	_curTime = millis();
	_lastReading = _currentReading;
	_currentReading = analogRead(_pin);
	float lastLoc = _location;
	_location = _currentReading * _resolution;
	_speed = (_location - lastLoc) / ((_curTime - _lastTime)*1000); // rad/s
	return _speed;
}
	
float RotationSensor::lastReading(){
	return _lastReading;
}

float RotationSensor::currentReading(){
	return _currentReading;
}

void RotationSensor::setDirection(int direction){
	_direction = direction;
}

void RotationSensor::setLocation(float location){
	_location = location;
}

float RotationSensor::getLocation(){
	return _location;
}

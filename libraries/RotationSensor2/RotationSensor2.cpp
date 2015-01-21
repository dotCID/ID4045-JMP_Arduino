#include <Arduino.h>
#include <RotationSensor2.h>


RotationSensor::RotationSensor(int pin){
	_resolution = 280/1024 * M_PI / 180; // conversion value from degrees/resolution -> radians/measured_value
	_pin = pin;
}

float RotationSensor::read(){
	_t_0 = _read_index;							// reading(t)
	_t_1 = _read_index==0?2:_read_index-1;		// reading(t-1)
	_t_2 = _read_index==1?2:_read_index==0?1:0;	// reading(t-2)
	
	_readings[_t_0] = analogRead(_pin) * _resolution; // rad
	_readTimes[_t_0] = millis();
	_read_index = _read_index==2?0:_read_index+1;
	
	_speed = (_readings[_t_2] - 4*_readings[_t_1] - 3*_readings[_t_0])/(2 * (_readTimes[_t_2] - _readTimes[_t_0]));
	return _speed;
}
	
float RotationSensor::lastReading(){
	return _readings[_t_1] / _resolution;
}

float RotationSensor::currentReading(){
	return _readings[_t_0] / _resolution;
}

void RotationSensor::setDirection(int direction){
	_direction = direction;
}

void RotationSensor::setLocation(float location){
	_readings[_t_0] = location;
}

float RotationSensor::getLocation(){
	return _readings[_t_0];
}

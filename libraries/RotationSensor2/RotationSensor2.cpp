#include <Arduino.h>
#include <RotationSensor2.h>


RotationSensor::RotationSensor(int pin){
	_resolution = 90.0 / 1024.0 * M_PI / 180.0; // conversion value from degrees/resolution -> radians/measured_value
	_pin = pin;
}

float RotationSensor::read(){
	_t_0 = _read_index;							// reading(t)
	_t_1 = _read_index==0?2:_read_index-1;		// reading(t-1)
	_t_2 = _read_index==1?2:_read_index==0?1:0;	// reading(t-2)
	_readings[_t_0] = map(analogRead(_pin),76.0,1023.0,90.0,0.0);
	// _readings[_t_0] = (analogRead(_pin)-76) * _resolution; // rad
	_readTimes[_t_0] = millis();
	_read_index = _read_index==2?0:_read_index+1;

	Serial.print("t-2 ");Serial.println(_readings[_t_2]);
	Serial.print("t-1 ");Serial.println(_readings[_t_1]);
	Serial.print("t   ");Serial.println(_readings[_t_0]);
	Serial.println();
	
	_speed = (_readings[_t_2] - 4*_readings[_t_1] + 3*_readings[_t_0])/(2 * (_readTimes[_t_2] - _readTimes[_t_0]));
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

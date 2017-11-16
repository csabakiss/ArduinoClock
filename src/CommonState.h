#pragma once
#include <TimeLib.h>
#include <Arduino.h>

class CommonState
{
public:
	float humidity;
	float temperature;
	time_t time;
	byte alarmHour;
	byte alarmMin;
	bool isAlarmOn;
};


#include "AlarmModule.h"

#define DIGIT(c) c - '0';

AlarmModule::AlarmModule()
{
}

AlarmModule::~AlarmModule()
{
}

void AlarmModule::init(CommonState* state)
{
	Serial.println(F("alarm.init"));
	pinMode(LIGHT_PIN, OUTPUT);
	analogWrite(LIGHT_PIN, 0);
	ModuleBase::init(state);
}

void AlarmModule::loop()
{
	bool hasOutput = false;
	if (state->isAlarmOn)
	{
		long secondsToAlarm = getSecondsToAlarm();
		if (secondsToAlarm < LIGHT_DURATION_SECONDS)
		{
			if (secondsToAlarm > 0) {
				int output = (int)((float)(LIGHT_DURATION_SECONDS - secondsToAlarm) / (float)LIGHT_DURATION_SECONDS * 128.0);								
				analogWrite(LIGHT_PIN, output);
				hasOutput = true;
			}
			else if (secondsToAlarm > -900) {
				analogWrite(LIGHT_PIN, 255);
				hasOutput = true;
			}
		}
	}

	if (!hasOutput) {
		analogWrite(LIGHT_PIN, 0);
	}	
}

long AlarmModule::getSecondsToAlarm()
{
	int hourDiff = state->alarmHour - hour(state->time);
	int minDiff = state->alarmMin - minute(state->time);
	return (long)hourDiff * 3600L + (long)minDiff * 60L - second(state->time);
}
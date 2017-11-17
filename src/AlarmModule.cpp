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
	ModuleBase::init(state);
}

void AlarmModule::loop()
{
	if (state->isAlarmOn && hour(state->time) == state->alarmHour && minute(state->time) == state->alarmMin)
	{
		// TODO beep beep beep
	}
}
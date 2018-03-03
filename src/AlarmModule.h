#pragma once

#include <TimeLib.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>

#include "ModuleBase.h"

// The number of seconds to turn the lights on before the alarm goes off
#define LIGHT_DURATION_SECONDS 600
#define LIGHT_PIN 11

class AlarmModule :
	public ModuleBase
{
public:
	AlarmModule();
	~AlarmModule();
	
	void init(CommonState * state);
	void loop();

private:
	long getSecondsToAlarm();
};


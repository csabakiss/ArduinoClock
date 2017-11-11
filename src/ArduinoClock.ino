#include <Wire.h>

#include "Config.h"
#include "CommonState.h"

#ifndef Serial1
	#include <SoftwareSerial.h>
	SoftwareSerial Serial1(4, 5); // RX, TX
#endif

#include "DHTModule.h"
#include "DisplayModule.h"
#include "EmonCmsLogger.h"
#include "TimeModule.h"

CommonState state;

ModuleBase* modules[] = { new DisplayModule(), new TimeModule(), new DHTModule(), new EmonCmsLogger() };
byte nModules = 4;

void setup()
{
	Serial.begin(57600);	
	Serial1.begin(57600);

	WiFi.init(&Serial1);

	for (byte i = 0; i < nModules; i++)
	{
		modules[i] -> init(&state);
	}
}

void loop()
{
	for (byte i = 0; i < nModules; i++)
	{
		modules[i] -> loop();
	}	

	delay(400);
}

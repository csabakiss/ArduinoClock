#include <Arduino.h>
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
#include "AlarmModule.h"
#include "AlarmWebSettingModule.h"

//#define DEBUG

CommonState state;

ModuleBase* modules[] = { new DisplayModule(), new TimeModule(), new DHTModule(), new EmonCmsLogger(), new AlarmModule(), new AlarmWebSettingModule() };
byte nModules = 6;

void setup()
{
	Serial.begin(115200);	
	Serial1.begin(19200);

	WiFi.init(&Serial1);

	for (byte i = 0; i < nModules; i++)
	{
		modules[i]->init(&state);
	}
}

void loop()
{
	for (byte i = 0; i < nModules; i++)
	{
		modules[i]->loop();
	}	

#ifdef DEBUG
	Serial.print(F("free ram:"));
	Serial.println(freeRam());
#endif // DEBUG


	delay(250);
}

#ifdef DEBUG
int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
#endif

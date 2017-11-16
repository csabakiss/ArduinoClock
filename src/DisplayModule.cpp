#include "DisplayModule.h"

DisplayModule::DisplayModule()
{
}


DisplayModule::~DisplayModule()
{
}

void DisplayModule::init(CommonState* state)
{
	Serial.println(F("display.init"));
	ModuleBase::init(state);

	display.begin();
	display.clearDisplay();
	display.setFont(u8x8_font_pxplusibmcgathin_f);

	while (WiFi.status() == WL_NO_SHIELD) {
		// Don't continue if there is no wifi shield
		display.draw2x2String(2, 2, "NYISTA");
		display.draw2x2String(0, 4, "INTERNET");
		delay(5000);
		display.clearDisplay();
	}
}

void DisplayModule::loop()
{
	displayAlarm(0, 0);
	displayTime(0, 2);
	displayDate(0, 5);
	displayTemp(1, 7);
	displayHumidity(11, 7);
}

void DisplayModule::displayAlarm(int x, int y)
{
	char buffer[13] = "Alarm: off  ";
	if (state->isAlarmOn)
	{
		sprintf_P(buffer, (const char*)F("Alarm: %.2d:%.2d"), state->alarmHour, state->alarmMin);
	}

	display.drawString(x, y, buffer);
}

void DisplayModule::displayDate(int x, int y)
{
	char buffer[17];
	sprintf_P(buffer, (const char*)F("%.4d. %.2d.%.2d. %s"), year(state->time), month(state->time), day(state->time), dayShortStr(weekday(state->time)));
	display.drawString(x, y, buffer);
}

void DisplayModule::displayTime(int x, int y)
{
	char buffer[9];
	sprintf_P(buffer, (const char*)F("%.2d:%.2d:%.2d"), hour(state->time), minute(state->time), second(state->time));
	display.draw2x2String(x, y, (const char*)buffer);
}

void DisplayModule::displayTemp(int x, int y)
{
	if (!isnan(state->temperature))
	{
		char buffer[8];
		sprintf_P(buffer, (const char*)F("%.2d °C"), (int)state->temperature);
		display.drawUTF8(x, y, buffer);
	}
}

void DisplayModule::displayHumidity(int x, int y)
{
	if (!isnan(state->humidity))
	{
		char buffer[5];
		sprintf_P(buffer, (const char*)F("%.2d %%"), (int)state->humidity);
		display.drawString(x, y, buffer);
	}
}

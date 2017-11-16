#pragma once

#include <U8g2lib.h>
#include <WiFiEsp.h>

#include "ModuleBase.h"

class DisplayModule :
	public ModuleBase
{
public:
	DisplayModule();
	~DisplayModule();

	void init(CommonState * state);
	void loop();

private:
	U8X8_SH1106_128X64_NONAME_HW_I2C display = U8X8_SH1106_128X64_NONAME_HW_I2C(U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);

	void displayAlarm(int x, int y);
	void displayDate(int x, int y);
	void displayTime(int x, int y);
	void displayTemp(int x, int y);
	void displayHumidity(int x, int y);
};


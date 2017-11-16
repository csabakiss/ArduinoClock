#pragma once

#include <TimeLib.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>

#include "ModuleBase.h"

class AlarmModule :
	public ModuleBase
{
public:
	AlarmModule();
	~AlarmModule();
	
	void init(CommonState * state);
	void loop();

private:
	time_t alarm;
	bool isAlarming;
	WiFiEspServer server = WiFiEspServer(80);

	void handleWebServerConnections();
	byte getNexStatus(char c, byte status);
	void returnHttpError(WiFiEspClient * client);
	void returnHttpOk(WiFiEspClient * client);
	void printPage(WiFiEspClient * client);
};


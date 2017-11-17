#pragma once
#include <TimeLib.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>

#include "ModuleBase.h"

class AlarmWebSettingModule :
	public ModuleBase
{
public:
	AlarmWebSettingModule();
	~AlarmWebSettingModule();
	void init(CommonState * state);
	void loop();

private:
	WiFiEspServer server = WiFiEspServer(80);

	void handleWebServerConnections();
	byte getNexStatus(char c, byte status);
	void returnHttpError(WiFiEspClient * client);
	void returnHttpOk(WiFiEspClient * client);
	void printPage(WiFiEspClient * client);
};


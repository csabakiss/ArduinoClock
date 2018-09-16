#include "AlarmWebSettingModule.h"
#include <eeprom.h>

#define DIGIT(c) c - '0'
#define ALARM_OFF 255

AlarmWebSettingModule::AlarmWebSettingModule()
{
}


AlarmWebSettingModule::~AlarmWebSettingModule()
{
}

void AlarmWebSettingModule::init(CommonState* state)
{
	Serial.println(F("alarm_setting.init"));
	ModuleBase::init(state);

	byte savedHour = EEPROM.read(0);
	byte savedMinute = EEPROM.read(1);

	if (savedHour != ALARM_OFF && savedMinute != ALARM_OFF)
	{
		state->isAlarmOn = true;
		state->alarmHour = savedHour;
		state->alarmMin = savedMinute;
		Serial.print(savedHour);
		Serial.print(':');
		Serial.println(savedMinute);
	}
	else
	{
		state->isAlarmOn = false;
	}

	server.begin();
}

void AlarmWebSettingModule::loop()
{
	handleWebServerConnections();
}

void AlarmWebSettingModule::handleWebServerConnections()
{
	WiFiEspClient client = server.available();
	if (client)
	{		
		byte attempts = 0;

		/*
		Use numbers to represent a simple state machine for processing the request in one iteration.
		- 0: uninteresing char
		- 20: first \n
		- 21: second \n
		- 30-31-32-33: h=HH
		- 40-41-42-43: m=MM
		- 50-51-52: GET
		- 60-61-62-63: POST
		- 70-71-72: off
		*/
		byte status = 0;

		byte hour = ALARM_OFF;
		byte minute = ALARM_OFF;
		bool isGet = false;
		bool isPost = false;
		bool isTurnedOff = false;
		while (client.connected() && attempts++ < 100)
		{
			if (client.available())
			{
				attempts = 0;
				char c = client.read();
				Serial.print(c);

				if (c == -1 || c == '\r')
				{
					continue;
				}

				status = getNextStatus(c, status);
				if (status == 32) { hour = DIGIT(c); }
				else if (status == 33) { hour = hour * 10 + DIGIT(c); }
				else if (status == 42) { minute = DIGIT(c); }
				else if (status == 43) { minute = minute * 10 + DIGIT(c); }
				else if (status == 52) { isGet = true; }
				else if (status == 63) { isPost = true; }
				else if (status == 72) { isTurnedOff = true; }
				else if (status == 21) { break; }
			}
		}

#ifdef DEBUG
			Serial.print(hour);
			Serial.print(':');
			Serial.println(minute);
#endif // DEBUG

		if (!(isPost^isGet))
		{
			returnHttpError(&client);
		}
		else if (isPost)
		{
			bool isValidAlarmTime = hour >= 0 && hour < 24 && minute >= 0 && minute < 60;
			if (!(isValidAlarmTime^isTurnedOff))
			{
				returnHttpError(&client);
			}
			else if (isValidAlarmTime)
			{
				EEPROM.update(0, hour);
				EEPROM.update(1, minute);

				state->isAlarmOn = true;
				state->alarmHour = hour;
				state->alarmMin = minute;				
			}
			else if (state->isAlarmOn)
			{
				// isTurnedOff must be true here												
				EEPROM.update(0, ALARM_OFF);
				EEPROM.update(1, ALARM_OFF);
				state->isAlarmOn = false;
			}
			returnHttpOk(&client);
		}
		else
		{
			// isGet must be true here
			// print the page for all GET requests
			printPage(&client);
		}

		client.flush();
		client.stop();
		Serial.println(F("Client disconnected"));
	}
}

byte AlarmWebSettingModule::getNextStatus(char c, byte status)
{
	bool isDigit = c >= '0' && c <= '9';

	if (c == '\n') { status = status == 20 ? 21 : 20; }
	else if (c == 'h') { status = 30; }
	else if (c == '=' && status == 30) { status = 31; }
	else if (c == 'm') { status = 40; }
	else if (c == '=' && status == 40) { status = 41; }
	else if (c == 'G' && status == 0) { status = 50; }
	else if (c == 'E' && status == 50) { status = 51; }
	else if (c == 'T' && status == 51) { status = 52; }
	else if (c == 'P' && status == 0) { status = 60; }
	else if (c == 'O' && status == 60) { status = 61; }
	else if (c == 'S' && status == 61) { status = 62; }
	else if (c == 'T' && status == 62) { status = 63; }
	else if (c == 'o') { status = 70; }
	else if (c == 'f' && (status == 70 || status == 71)) { status++; }
	else if (isDigit && (status == 41 || status == 42 || status == 31 || status == 32))
	{
		status++;
	}
	else { status = 0; }

	return status;
}

void AlarmWebSettingModule::returnHttpError(WiFiEspClient* client)
{
	client->print(F("HTTP/1.1 500 Internal Server Error\r\n\r\n"));
}

void AlarmWebSettingModule::returnHttpOk(WiFiEspClient* client)
{
	client->print(F("HTTP/1.1 200 OK\r\n\r\n"));
}

void AlarmWebSettingModule::printPage(WiFiEspClient* client)
{
	client->print(F(
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Connection: close\r\n"
		"\r\n"));

	client->print(F(
		"<!DOCTYPE HTML>"
		"<html>"
		"<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">"
		"<style>"
		"input, button { font-size: 8vw; top: 40vh; width: 23vw; position: absolute; }"
		"</style>"
		"<body>"
		"<input id=\"hour\" type=\"number\" value=\""
	));

	if (state->isAlarmOn)
	{
		client->print(state->alarmHour);
	}

	client->print(F(
		"\" />"
		"<input id=\"min\" style=\"left:25vw\" type=\"number\" value=\""
	));

	if (state->isAlarmOn)
	{
		client->print(state->alarmMin);
	}

	client->print(F(
		"\" />"
		"<button id=\"ok\" style=\"left:50vw\" type=\"submit\">Ok</button>"
		"<button id=\"off\" style=\"left:75vw\" type=\"submit\">Off</button>"
		"<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>"
		"<script type=\"text/javascript\">"
		"$('#ok').click(function () {"
			"setButtonState(true);"
			"$.post('/h=' + $('#hour').val() + '&m=' + $('#min').val())"
				".always(function() {setButtonState(false);})"
				".fail(function() {alert('Could not set alarm');});"
		"});"
		"$('#off').click(function () {"
			"setButtonState(true);"
			"$.post('/off', function() {$('#hour, #min').val('');})"				
				".always(function() {setButtonState(false);});"
		"});"
		"function setButtonState(isDisabled) { $('#ok, #off').attr('disabled', isDisabled); }"
		"</script>"
		"</body>"
		"</html>"
	));
}

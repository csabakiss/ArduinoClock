#pragma once

#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>

#include "ModuleBase.h"
#include "Config.h"

#define TIME_SERVER "time.nist.gov"
#define NTP_REFRESH_TIME_INTERVAL 3600
#define NTP_PACKET_SIZE 48 // NTP timestamp is in the first 48 bytes of the message

class TimeModule :
	public ModuleBase
{
public:
	TimeModule();
	~TimeModule();
	void init(CommonState * state);
	void loop();	

private:	
	const int NTP_UDP_TIMEOUT = 2000;    // timeout in miliseconds to wait for an UDP packet to arrive	
	const unsigned int localPort = 2390;        // local port to listen for UDP packets

	byte packetBuffer[NTP_PACKET_SIZE];
	unsigned long lastWifiConnectionAttempt = 0;
	time_t nextSync;
	WiFiEspUDP udp = WiFiEspUDP();

	
	time_t getCurrentLocalTime();
	time_t getCurrentUtcTime();
	bool sendNTPpacket(const char * ntpSrv);
	time_t provideLocalTimeUsingWiFi();
};


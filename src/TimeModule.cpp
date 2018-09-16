#include "TimeModule.h"

TimeModule::TimeModule()
{
}

TimeModule::~TimeModule()
{
}

void TimeModule::init(CommonState* state)
{
	Serial.println(F("time.init"));
	ModuleBase::init(state);	
	nextSync = now();
}

void TimeModule::loop()
{
	if (now() > nextSync)
	{
		Serial.println(F("[time] sync.."));
		time_t currentTime = provideLocalTimeUsingWiFi();		
		if (currentTime != 0)
		{
			setTime(currentTime);
			nextSync = now() + NTP_REFRESH_TIME_INTERVAL;
		}
		else {
			nextSync = now() + 30UL;
		}
	}

	state->time = now();
}

time_t TimeModule::provideLocalTimeUsingWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		if (WiFi.begin(WifiSsid, WifiPass) == WL_CONNECT_FAILED)
		{
			return 0;
		}
	}

	time_t currentLocalTime = 0;
	if (udp.begin(localPort) == 1)
	{
		currentLocalTime = getCurrentLocalTime();
		udp.stop();
	}

	return currentLocalTime;
}

time_t TimeModule::getCurrentLocalTime() {
	time_t currentTimeUTC = getCurrentUtcTime();
	if (currentTimeUTC > 0)
	{
		const unsigned long timeZoneOffset = 7UL * 3600UL;
		return currentTimeUTC - timeZoneOffset;
	}

	return 0;
}

time_t TimeModule::getCurrentUtcTime()
{
	if (!sendNTPpacket(TIME_SERVER))
	{
		return 0;
	}

	// wait for a reply for UDP_TIMEOUT miliseconds
	unsigned long startMs = millis();
	while (!udp.available() && (millis() - startMs) < NTP_UDP_TIMEOUT) {}

	if (udp.parsePacket()) {
		Serial.println(F("[NTP] packet received"));
		// We've received a packet, read the data from it into the buffer
		if (udp.read((char*)packetBuffer, NTP_PACKET_SIZE) == 0)
		{
			return 0;
		}

		// the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:

		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		// now convert NTP time into everyday time:
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;

		// subtract seventy years:
		unsigned long unixTime = secsSince1900 - seventyYears;
		return unixTime;
	}

	return 0;
}

// send an NTP request to the time server at the given address
bool TimeModule::sendNTPpacket(const char *ntpSrv)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)

	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	// NTP requests are to port 123
	if (udp.beginPacket(ntpSrv, 123) == 1)
	{
		udp.write(packetBuffer, NTP_PACKET_SIZE);
		if (udp.endPacket() == 1)
		{
			return true;
		}
	}

	return false;
}

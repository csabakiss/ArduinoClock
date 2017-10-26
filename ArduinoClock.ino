#include <DHT.h>
#include <TimeLib.h>
#include <Wire.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <U8g2lib.h>

// Secrets are stored in Secrets.h which is not under source control management
#if __has_include("Secrets.h")
	#include "Secrets.h"
#else
	#define EmonCmsApiKey "your api key"
	#define WifiSsid "cool wifi name"
	#define WifiPass "secret password"
#endif

#define DHT_PIN 2
#define DHTTYPE DHT11

#ifndef Serial1
	#include <SoftwareSerial.h>
	SoftwareSerial Serial1(4, 5); // RX, TX
#endif

const int internetTimeRefreshInterval = 3600;
const size_t NTP_PACKET_SIZE = 48;  // NTP timestamp is in the first 48 bytes of the message
const int NTP_UDP_TIMEOUT = 2000;    // timeout in miliseconds to wait for an UDP packet to arrive
const char timeServer[] = "time.nist.gov";
const unsigned int localPort = 2390;        // local port to listen for UDP packets

byte packetBuffer[NTP_PACKET_SIZE];
unsigned long lastWifiConnectionAttempt = 0;
int dataLogCycle = 0;
WiFiEspUDP Udp;
DHT dht(DHT_PIN, DHTTYPE);
U8X8_SH1106_128X64_NONAME_HW_I2C display(U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);

void setup()
{
	Serial.begin(57600);

	// DISPLAY
	display.begin();
	display.clearDisplay();
	display.setFont(u8x8_font_pxplusibmcga_f);

	Serial1.begin(57600);
	WiFi.init(&Serial1);

	while (WiFi.status() == WL_NO_SHIELD) {
		// Don't continue if there is no wifi shield
		display.draw2x2String(2, 2, "NYISTA");
		display.draw2x2String(0, 4, "INTERNET");
		delay(5000);
		display.clearDisplay();
	}

	setSyncInterval(internetTimeRefreshInterval);
	setSyncProvider(provideLocalTimeUsingWiFi);

	// DHT
	dht.begin();
}

void loop()
{
	float temperature = dht.readTemperature();
	float humidity = dht.readHumidity();
	
	if (dataLogCycle++ > 1000)
	{
		dataLogCycle = 0;
		logDataToEmoncms(temperature, humidity);
	}

	time_t currentTime = now();
	displayTime(0, 2, currentTime);
	displayDate(0, 5, currentTime);
	
	displayTemp(1, 7, temperature);
	displayHumidity(11, 7, humidity);

	delay(750);
}

void displayDate(int x, int y, time_t time)
{
	char buffer[17];
	sprintf_P(buffer, (const char*)F("%.4d. %.2d.%.2d. %s"), year(time), month(time), day(time), dayShortStr(weekday(time)));
	display.drawString(x, y, buffer);
}

void displayTime(int x, int y, time_t time)
{
	char buffer[9];
	sprintf_P(buffer, (const char*)F("%.2d:%.2d:%.2d"), hour(time), minute(time), second(time));
	display.draw2x2String(x, y, (const char*)buffer);
}

void displayTemp(int x, int y, float temperature)
{	
	if (!isnan(temperature))
	{
		char buffer[8];
		sprintf_P(buffer, (const char*)F("%.2d °C"), (int)temperature);
		display.drawUTF8(x, y, buffer);
	}
}

void displayHumidity(int x, int y, float humidity)
{		
	if (!isnan(humidity))
	{
		char buffer[5];
		sprintf_P(buffer, (const char*)F("%.2d %%"), (int)humidity);
		display.drawString(x, y, buffer);
	}
}

void logDataToEmoncms(float temperature, float humidity)
{
	if (WiFi.status() == WL_CONNECTED)
	{
		WiFiEspClient client;
		if (client.connect("emoncms.org", 80)) {
			// Make a HTTP request
			char buffer[160];
			sprintf_P(
				buffer,
				(const char*)F("GET /input/post?node=bedroom&apikey=%s&fulljson={\"temp\":%.2d,\"humd\":%.2d} HTTP/1.1"),				
				EmonCmsApiKey,
				(int)temperature,
				(int)humidity);
			client.println(buffer);
			client.println(F("Host: emoncms.org"));
			client.println(F("Connection: close"));
			client.println();
			client.flush();
			client.stop();
		}
	}
}

time_t provideLocalTimeUsingWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		if (lastWifiConnectionAttempt == 0 || lastWifiConnectionAttempt + 30000UL < millis())
		{
			lastWifiConnectionAttempt = millis();
			if (WiFi.begin(WifiSsid, WifiPass) == WL_CONNECT_FAILED)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	time_t currentLocalTime = 0;
	if (Udp.begin(localPort) == 1)
	{
		currentLocalTime = getCurrentLocalTime();
		Udp.stop();
	}

	return currentLocalTime;
}

time_t getCurrentLocalTime() {
	time_t currentTimeUTC = getCurrentUtcTime();
	if (currentTimeUTC > 0)
	{
		const unsigned long timeZoneOffset = 7UL * 3600UL;
		return currentTimeUTC - timeZoneOffset;
	}

	return 0;
}

time_t getCurrentUtcTime()
{
	if (!sendNTPpacket(timeServer))
	{
		return 0;
	}

	// wait for a reply for UDP_TIMEOUT miliseconds
	unsigned long startMs = millis();
	while (!Udp.available() && (millis() - startMs) < NTP_UDP_TIMEOUT) {}

	Serial.println(Udp.parsePacket());
	if (Udp.parsePacket()) {
		Serial.println("packet received");
		// We've received a packet, read the data from it into the buffer
		if (Udp.read((char*)packetBuffer, NTP_PACKET_SIZE) == 0)
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
bool sendNTPpacket(const char *ntpSrv)
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
	//NTP requests are to port 123
	if (Udp.beginPacket(ntpSrv, 123) == 1)
	{
		Udp.write(packetBuffer, NTP_PACKET_SIZE);
		if (Udp.endPacket() == 1)
		{
			return true;
		}
	}

	return false;
}

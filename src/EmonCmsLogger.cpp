#include "EmonCmsLogger.h"

EmonCmsLogger::EmonCmsLogger()
{
}

EmonCmsLogger::~EmonCmsLogger()
{
}

void EmonCmsLogger::init(CommonState* state)
{
	Serial.println(F("cmslogger.init"));
	ModuleBase::init(state);
}

void EmonCmsLogger::loop()
{
	if (dataLogCycle == 0 || dataLogCycle++ > DATA_LOG_CYCLE_INTERVAL)
	{
		dataLogCycle = 1;
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
					(int)state->temperature,
					(int)state->humidity);
				client.println(buffer);
				client.println(F("Host: emoncms.org"));
				client.println(F("Connection: close"));
				client.println();
				client.flush();
				client.stop();
			}
		}
	}
}

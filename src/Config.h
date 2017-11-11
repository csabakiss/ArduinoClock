#pragma once

// Secrets are stored in Secrets.h which is not under source control management
#if __has_include("Secrets.h")
#include "Secrets.h"
#else
	#define EmonCmsApiKey "your api key"
	#define WifiSsid "cool wifi name"
	#define WifiPass "secret password"
#endif

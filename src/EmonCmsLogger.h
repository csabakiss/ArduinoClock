#pragma once
#include <WiFiEsp.h>
#include "Config.h"
#include "ModuleBase.h"

#define DATA_LOG_CYCLE_INTERVAL 1000

class EmonCmsLogger :
	public ModuleBase
{
public:
	EmonCmsLogger();
	~EmonCmsLogger();
	void init(CommonState * state);
	void loop();

private:
	int dataLogCycle = 0;
};


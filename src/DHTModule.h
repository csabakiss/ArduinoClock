#pragma once

#include <DHT.h>

#include "ModuleBase.h"

#define DHT_PIN 2
#define DHTTYPE DHT11

class DHTModule :
	public ModuleBase
{
public:
	DHTModule();
	~DHTModule();
	
	virtual void init(CommonState* state);

	virtual void loop();

private:
	DHT dht = DHT(DHT_PIN, DHTTYPE);
};


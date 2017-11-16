#include "DHTModule.h"

DHTModule::DHTModule()
{
}


DHTModule::~DHTModule()
{
}

void DHTModule::init(CommonState* state)
{
	Serial.println(F("dht.init"));
	ModuleBase::init(state);
	dht.begin();
}

void DHTModule::loop()
{	
	state->temperature = dht.readTemperature();
	state->humidity = dht.readHumidity();
}

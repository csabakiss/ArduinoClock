#include "ModuleBase.h"

ModuleBase::ModuleBase()
{
}

ModuleBase::~ModuleBase()
{
}

void ModuleBase::init(CommonState* state)
{
	Serial.println(F("mb.init"));
	this->state = state;
}

void ModuleBase::loop()
{
	Serial.println(F("mb.loop"));
}

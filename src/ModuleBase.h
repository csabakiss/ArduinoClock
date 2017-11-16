#pragma once
#include "CommonState.h"

class ModuleBase
{
public:
	ModuleBase();
	~ModuleBase();

	virtual void init(CommonState* state);
	virtual void loop();

protected:
	CommonState* state;
};


#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>

#include "PTAInputChangedHandler.h"
#include "PTACommandEventHandler.h"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

class PTAInstanceHandler
{
public:
	PTAInstanceHandler();

	PTAInputChangedHandler inputChangedHandler;
	PTACommandEventHandler commandEventHandler;
};

extern PTAInstanceHandler PTAInstanceHandler_;

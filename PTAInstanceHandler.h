#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>

#include "PTAInputChangedHandler.h"
#include "PTACommandExecutedHandler.h"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

class PTAInstanceHandler
{
public:
	PTAInstanceHandler();

	PTAInputChangedHandler inputChangedHandler;
	PTACommandExecutedHandler commandExecutedHandler;
};

extern PTAInstanceHandler PTAInstanceHandler_;

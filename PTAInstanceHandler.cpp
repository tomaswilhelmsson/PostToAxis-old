#include "PTAInstanceHandler.h"

PTAInstanceHandler PTAInstanceHandler_;

PTAInstanceHandler::PTAInstanceHandler()
{
	Application::get()->userInterface()->messageBox("PTAInstanceHandler constructor");
}

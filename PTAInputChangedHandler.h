#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

class PTAInputChangedHandler : public InputChangedEventHandler
{
public:
	PTAInputChangedHandler();
	void notify(const Ptr<InputChangedEventArgs> &eventArgs) override;

	void hasFile(bool value);
	bool hasFile();

	void filePath(const std::string filePath);
	const std::string filePath();
private:
	Ptr<Application> _app;
	Ptr<UserInterface> _ui;
	Ptr<CAM> _cam;

	bool _hasFile = false;
	std::string _filePath;
};


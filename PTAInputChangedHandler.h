#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>
#include <map>
#include <list>

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

	void addList(int index);
	void clearLists();

	std::map<std::string, std::vector<std::vector<Ptr<OperationBase>>>> _operationList;
private:
	Ptr<Application> _app;
	Ptr<UserInterface> _ui;
	Ptr<CAM> _cam;

	bool _hasFile = false;
	std::string _filePath;

//	std::vector<int> _selectedIndexes;

	std::map<std::string, std::map<int, std::map<int, bool>>> _selectedIndexes;

	// A more sensible operationlist
};


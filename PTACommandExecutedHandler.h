#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>
#include "IniFile.h"
#include "MD5.h"
#include "PTASocket.h"
#include "PTAInputChangedHandler.h"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

class PTACommandExecutedHandler : public CommandEventHandler
{
public:
	PTACommandExecutedHandler();
	void notify(const Ptr<CommandEventArgs> &eventArgs) override;

	bool sendFile(const std::string filePath, const std::string ipAddr, const std::string port, const std::string workSheetPath = "");

	std::string postProcess(Ptr<ObjectCollection> opsToPost);
	std::string generateWorksheet(Ptr<ObjectCollection> opsToPost);
	bool generateToolpath(Ptr<Operation> op, bool askConfirmation);

	std::string getMD5(const std::string filePath);

	char *getFileData(const std::string filePath, size_t *size);
private:
	Ptr<Application> _app;
	Ptr<UserInterface> _ui;
	Ptr<CAM> _cam;

	PTAInputChangedHandler *_inputChangedHandler;
};


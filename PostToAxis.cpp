
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>
#include "PTASocket.h"


#include "IniFile.h"
#include "MD5.h"
#include "PTAInputChangedHandler.h"
#include "PTACommandExecutedHandler.h"
#include "PTAInstanceHandler.h"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

Ptr<Application> app;
Ptr<UserInterface> ui;

class PTACommandCreatedHandler : public CommandCreatedEventHandler
{
public:
	~PTACommandCreatedHandler() {
	}
	void notify(const Ptr<CommandCreatedEventArgs> &eventArgs) override
	{
	
		if (eventArgs)
		{
			Ptr<Command> command = eventArgs->command();
			if (!command)
				return;

			Ptr<CommandEvent> exec = command->execute();
			if (!exec)
				return;

			PTAInputChangedHandler *inputChangedHandler = &PTAInstanceHandler_.inputChangedHandler;

			exec->add(&PTAInstanceHandler_.commandExecutedHandler);
			command->inputChanged()->add(&PTAInstanceHandler_.inputChangedHandler);

			Ptr<CommandInputs> commandInputs = command->commandInputs();

			if (!commandInputs)
				return;

			std::string path = IniFile::getLocalAppData();

			IniFile::setFile(path + "\\PostToAxis", "Settings.ini");

			std::string ipAddr = IniFile::getString("Settings", "IP");
			std::string port = IniFile::getString("Settings", "PORT");

			commandInputs->addTextBoxCommandInput("testText", "testText", "Test text!", 1, true);
			commandInputs->addStringValueInput("ipInput", "IP Address: ", ipAddr.c_str());
			commandInputs->addStringValueInput("portInput", "Port", port.c_str());
			commandInputs->addBoolValueInput("genSetupSheet", "Generate setup sheet?", true);

			commandInputs->addBoolValueInput("buttonClick", "Open file", false);
			Ptr<TextBoxCommandInput> filePathText = commandInputs->addTextBoxCommandInput("filePathText", "File Path:", "", 1, true);
			if (inputChangedHandler->hasFile())
				filePathText->text(inputChangedHandler->filePath());
			else
				filePathText->isVisible(false);

			Ptr<BoolValueCommandInput> removeFile = commandInputs->addBoolValueInput("removeFile", "Remove file", false);
			if (inputChangedHandler->hasFile())
				removeFile->isVisible(true);
			else
				removeFile->isVisible(false);

			Ptr<DropDownCommandInput> setupDropDown = commandInputs->addDropDownCommandInput("setupSelect", "Setup: ", DropDownStyles::LabeledIconDropDownStyle);
			Ptr<DropDownCommandInput> opDropDown = commandInputs->addDropDownCommandInput("operationSelect", "Operation: ", DropDownStyles::CheckBoxDropDownStyle);
			
			if (inputChangedHandler->hasFile())
			{
				setupDropDown->isEnabled(false);
				opDropDown->isEnabled(false);
				setupDropDown->isVisible(false);
				opDropDown->isVisible(false);
			}

			Ptr<Product> product = app->activeDocument()->products()->itemByProductType("CAMProductType");

			Ptr<CAM> cam = product->cast<CAM>();

			for (Ptr<Setup> setup : cam->setups())
				setupDropDown->listItems()->add(setup->name(), false);

			for (Ptr<Operation> op : cam->allOperations())
				opDropDown->listItems()->add(op->name(), false);

			command->setDialogInitialSize(500, 300);
			command->setDialogMinimumSize(500, 300);
			command->okButtonText("Send");
		}
	}
private:
} onCommandCreated_;



/* Removes a command definition by name */
void removeCommandDefinition(std::string definitionIdentifier)
{
	Ptr<CommandDefinitions> definitions = ui->commandDefinitions();

	if (!definitions)
		return;

	if (definitions->itemById(definitionIdentifier))
	{
		definitions->itemById(definitionIdentifier)->deleteMe();
	}
}

/* Removes a toolbar control button */
void removeToolbarControl(std::string toolbarIdentifier, std::string toolbarControlIdentifier)
{
	Ptr<ToolbarPanel> panel = ui->allToolbarPanels()->itemById(toolbarIdentifier);

	if (!panel)
		return;

	Ptr<ToolbarControls> control = panel->controls();

	if (!control)
		return;

	if (control->itemById(toolbarControlIdentifier))
		control->itemById(toolbarControlIdentifier)->deleteMe();
}

extern "C" XI_EXPORT bool run(const char* context)
{
	app = Application::get();
	if (!app)
		return false;

	ui = app->userInterface();
	if (!ui)
		return false;

	Ptr<CommandDefinitions> definitions = ui->commandDefinitions();
	if (!definitions)
		return false;

	removeCommandDefinition("PostToAxis");

	Ptr<CommandDefinition> cmdPostToAxisButton = definitions->addButtonDefinition("PostToAxis", "Post to Axis", "Post to axis", "./resources/");
	
	if (!cmdPostToAxisButton)
		return false;

	Ptr<CommandCreatedEvent> postToAxisCreatedEvent = cmdPostToAxisButton->commandCreated();

	if (!postToAxisCreatedEvent)
		return false;

	postToAxisCreatedEvent->add(&onCommandCreated_);
	
	removeToolbarControl("CAMActionPanel", "PostToAxis");

	Ptr<ToolbarControls> camToolbarControl = ui->allToolbarPanels()->itemById("CAMActionPanel")->controls();

	if (!camToolbarControl)
		return false;

	Ptr<CommandControl> buttonControl = camToolbarControl->addCommand(definitions->itemById("PostToAxis"), "", false);
	
	if (!buttonControl)
		return false;

	buttonControl->isPromotedByDefault(true);
	buttonControl->isPromoted(true);

	ui->messageBox("Hello addin");

	return true;
}

extern "C" XI_EXPORT bool stop(const char* context)
{
	if (ui)
	{
		removeCommandDefinition("PostToAxis");
		removeToolbarControl("CAMActionPanel", "PostToAxis");
		ui->messageBox("Stop addin");
		ui = nullptr;
	}

	return true;
}


#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif // XI_WIN

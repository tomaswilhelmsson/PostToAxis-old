#include "PTAInputChangedHandler.h"

PTAInputChangedHandler::PTAInputChangedHandler() : InputChangedEventHandler()
{
	_app = Application::get();
	_ui = _app->userInterface();

	Ptr<Product> product = _app->activeDocument()->products()->itemByProductType("CAMProductType");

	if (product)
		_cam = product->cast<CAM>();
}

void PTAInputChangedHandler::notify(const Ptr<InputChangedEventArgs> &eventArgs)
{
	if (eventArgs->input()->id() == "buttonClick")
	{
		Ptr<BoolValueCommandInput> cmdInput = eventArgs->input();
		Ptr<FileDialog> fileDialog = _ui->createFileDialog();
		fileDialog->isMultiSelectEnabled(false);
		fileDialog->title("Select a ngc file");
		fileDialog->filter("LinuxCNC files (*.ngc)");
		fileDialog->filterIndex(0);
		DialogResults result = fileDialog->showOpen();
		if (result == DialogResults::DialogOK) {
			hasFile(true);
			filePath(fileDialog->filename());
			//_ui->messageBox("File selected: " + fileDialog->filename());
			Ptr<TextBoxCommandInput> fileTextBox = eventArgs->inputs()->itemById("filePathText");
			Ptr<BoolValueCommandInput> removeFile = eventArgs->inputs()->itemById("removeFile");
			Ptr<DropDownCommandInput> setupSelect = eventArgs->inputs()->itemById("setupSelect");
			Ptr<DropDownCommandInput> opSelect = eventArgs->inputs()->itemById("operationSelect");
			setupSelect->isEnabled(false);
			setupSelect->isVisible(false);
			opSelect->isEnabled(false);
			opSelect->isVisible(false);
			removeFile->isVisible(true);
			fileTextBox->text(fileDialog->filename());
			fileTextBox->isVisible(true);
		}
	}
	else if (eventArgs->input()->id() == "removeFile")
	{
		hasFile(false);
		Ptr<TextBoxCommandInput> fileTextBox = eventArgs->inputs()->itemById("filePathText");
		fileTextBox->text("");
		fileTextBox->isVisible(false);
		filePath("");
		Ptr<BoolValueCommandInput> removeFile = eventArgs->inputs()->itemById("removeFile");
		Ptr<DropDownCommandInput> setupSelect = eventArgs->inputs()->itemById("setupSelect");
		Ptr<DropDownCommandInput> opSelect = eventArgs->inputs()->itemById("operationSelect");
		setupSelect->isEnabled(true);
		setupSelect->isVisible(true);
		opSelect->isEnabled(true);
		opSelect->isVisible(true);
		removeFile->isVisible(false);


	}
	else if (eventArgs->input()->id() == "setupSelect")
	{
		Ptr<CommandInputs> inputs = eventArgs->inputs();
		Ptr<DropDownCommandInput> cmdInput = eventArgs->input();

		Ptr<DropDownCommandInput> opDropDownMenu = inputs->itemById("operationSelect");
		opDropDownMenu->listItems()->clear();

		if (_cam)
		{
			for (Ptr<Operation> op : _cam->allOperations())
			{
/*
				for (Ptr<ListItem> item : cmdInput->listItems())
				{
					if (item->isSelected())
						if (op->parentSetup()->name() == item->name())
							opDropDownMenu->listItems()->add(op->name(), false);
				}
*/				
				if (op->parentSetup()->name() == cmdInput->selectedItem()->name())
					opDropDownMenu->listItems()->add(op->name(), false);
			}
		}
		else {
			_ui->messageBox("No CAM Object");
		}
	}
}

void PTAInputChangedHandler::hasFile(bool value)
{
	_hasFile = value;
}

bool PTAInputChangedHandler::hasFile()
{
	return _hasFile;
}

void PTAInputChangedHandler::filePath(const std::string filePath) {
	_filePath = filePath;
}
const std::string PTAInputChangedHandler::filePath() {
	return _filePath;
}
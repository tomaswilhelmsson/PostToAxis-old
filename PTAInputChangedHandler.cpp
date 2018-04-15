#include "PTAInputChangedHandler.h"

PTAInputChangedHandler::PTAInputChangedHandler() : InputChangedEventHandler()
{
	_app = Application::get();
	_ui = _app->userInterface();

	Ptr<Product> product = _app->activeDocument()->products()->itemByProductType("CAMProductType");

	if (!product)
		_ui->messageBox("Could not find CAMProductTYpe");
	else
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
			int setupIndex = cmdInput->selectedItem()->index();
			Ptr<ChildOperationList> childList = _cam->setups()->item(cmdInput->selectedItem()->index())->children();
//			Ptr<ObjectCollection> operations = _cam->setups()->itemByName(cmdInput->selectedItem()->name())->allOperations();

			for (int i = 0; i < childList->count(); i++)
			{
				std::string objectType = childList->item(i)->objectType();

				Ptr<OperationBase> child = childList->item(i);

				if (objectType == "adsk::cam::Operation")
				{
					opDropDownMenu->listItems()->add(child->name(), false);
					_operationList[_app->activeDocument()->name()][setupIndex].push_back(child);
				}
				else if (objectType == "adsk::cam::CAMFolder") {
					Ptr<CAMFolder> folder = child->cast<CAMFolder>();

					opDropDownMenu->listItems()->add("Folder: " + folder->name(), false);

					_operationList[_app->activeDocument()->name()][setupIndex].push_back(folder);

					for (int i = 0; i < folder->allOperations()->count(); i++)
					{
						std::string objectType = folder->children()->item(i)->objectType();

						if (objectType == "adsk::cam::Operation") {
							Ptr<Operation> op = folder->children()->item(i)->cast<Operation>();
							_operationList[_app->activeDocument()->name()][setupIndex].push_back(op);
							opDropDownMenu->listItems()->add("    " + op->name(), false);
						}
					}
				}
				else if (objectType == "adsk::cam::CAMPattern") {
					Ptr<CAMPattern> pattern = child->cast<CAMPattern>();

					opDropDownMenu->listItems()->add("Pattern: " + pattern->name(), false);

					_operationList[_app->activeDocument()->name()][setupIndex].push_back(pattern);

					for (int i = 0; i < pattern->allOperations()->count(); i++)
					{
						std::string objectType = pattern->children()->item(i)->objectType();

						if (objectType == "adsk::cam::Operation") {
							Ptr<Operation> op = pattern->children()->item(i)->cast<Operation>();
							_operationList[_app->activeDocument()->name()][setupIndex].push_back(op);
							opDropDownMenu->listItems()->add("    " + op->name(), false);
						}
					}

				}
				//_ui->messageBox("ObjectType " + objectType);
			}


			for (std::pair<int, bool> pair : _selectedIndexes[_app->activeDocument()->name()][cmdInput->selectedItem()->index()])
			{
				opDropDownMenu->listItems()->item(pair.first)->isSelected(pair.second);
			}
			return;
		}
			/*
		if (_cam)
		{
			for (Ptr<Operation> op : _cam->allOperations())
			{
				if (op->parentSetup()->name() == cmdInput->selectedItem()->name())
					opDropDownMenu->listItems()->add(op->name(), false);
			}
		}
*/
		else {
			_ui->messageBox("No CAM Object");
		}
	}
	else if (eventArgs->input()->id() == "operationSelect")
	{
		Ptr<DropDownCommandInput> setupDropDownMenu = eventArgs->inputs()->itemById("setupSelect");
		Ptr<DropDownCommandInput> opDropDownMenu = eventArgs->input();

		// To keep track of all the selected stuff we select the list index
		int setupIndex = setupDropDownMenu->selectedItem()->index();

		int selectedIndex = -1;

		// Index unselected
		if (selectedIndex == -1) {
			for (int i = 0; i < opDropDownMenu->listItems()->count(); i++) {
				if (_selectedIndexes[_app->activeDocument()->name()][setupIndex][i] != opDropDownMenu->listItems()->item(i)->isSelected())
				{
					selectedIndex = i;
					_selectedIndexes[_app->activeDocument()->name()][setupIndex][i] = opDropDownMenu->listItems()->item(i)->isSelected();
					break;
				}
			}
		}

		/* If a folder or a pattern is selected select/deselect all operations associated with that */
		std::string objectType = _operationList[_app->activeDocument()->name()][setupIndex][selectedIndex]->objectType();

		if (objectType == "adsk::cam::CAMFolder")
		{
			Ptr<CAMFolder> folder =_operationList[_app->activeDocument()->name()][setupIndex][selectedIndex]->cast<CAMFolder>();

			for (int i = selectedIndex + 1; i < (selectedIndex + 1) + folder->allOperations()->count(); i++) {
				opDropDownMenu->listItems()->item(i)->isSelected(_selectedIndexes[_app->activeDocument()->name()][setupIndex][selectedIndex]);
				_selectedIndexes[_app->activeDocument()->name()][setupIndex][i] = _selectedIndexes[_app->activeDocument()->name()][setupIndex][selectedIndex];
			}
		}
		else if (objectType == "adsk::cam::CAMPattern") {
			Ptr<CAMPattern> pattern = _operationList[_app->activeDocument()->name()][setupIndex][selectedIndex]->cast<CAMPattern>();

			for (int i = selectedIndex + 1; i < (selectedIndex + 1) + pattern->allOperations()->count(); i++) {
				opDropDownMenu->listItems()->item(i)->isSelected(_selectedIndexes[_app->activeDocument()->name()][setupIndex][selectedIndex]);
				_selectedIndexes[_app->activeDocument()->name()][setupIndex][i] = _selectedIndexes[_app->activeDocument()->name()][setupIndex][selectedIndex];
			}
		}
		/*
		Ptr<ObjectCollection> opList = _cam->setups()->item(setupIndex)->allOperations();
		
		for (Ptr<OperationBase> test : _cam->setups()->item(setupIndex)->allOperations())
			_ui->messageBox("ObjectType " + std::string(test->));
		
		std::string objectType = opList->item(selectedIndex)->objectType();
		if (objectType == "adsk::cam::CAMFolder") {
			Ptr<CAMFolder> folder = opList->item(selectedIndex)->cast<CAMFolder>();

			if (folder) {
				for (int i = selectedIndex + 1; i < (selectedIndex + 1) + folder->children()->count(); i++)
				{
					if (opDropDownMenu->listItems()->item(selectedIndex)->isSelected())
					{
						_selectedIndexes[setupIndex][i] = true;
						opDropDownMenu->listItems()->item(i)->isSelected(true);
					}
					else {
						_selectedIndexes[setupIndex][i] = false;
						opDropDownMenu->listItems()->item(i)->isSelected(false);
					}
				}
			}
		}
		*/
	}
	else {
		_ui->messageBox("PTAInputChangedHandler::notify\nUnhandled event: ", eventArgs->input()->id());
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

void PTAInputChangedHandler::addList(int index) {
	// Add if list does not exist
	if (_selectedIndexes[_app->activeDocument()->name()].find(index) == _selectedIndexes[_app->activeDocument()->name()].end()) {
		_selectedIndexes[_app->activeDocument()->name()][index] = std::map<int, bool>();
		_operationList[_app->activeDocument()->name()].push_back(std::vector<Ptr<OperationBase>>());
	}
}

void PTAInputChangedHandler::clearLists() {
	_selectedIndexes[_app->activeDocument()->name()].clear();
	_operationList[_app->activeDocument()->name()].clear();
}
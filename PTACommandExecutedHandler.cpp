#include "PTACommandExecutedHandler.h"
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <io.h>
#include "PTAInstanceHandler.h"
#include "IniFile.h"

PTACommandExecutedHandler::PTACommandExecutedHandler() : CommandEventHandler()
{
	_app = Application::get();
	_ui = _app->userInterface();

}

void PTACommandExecutedHandler::notify(const Ptr<CommandEventArgs> &eventArgs)
{
	_inputChangedHandler = &PTAInstanceHandler_.inputChangedHandler;

	Ptr<Command> cmd = eventArgs->command();
	Ptr<CommandInputs> cmdInputs = cmd->commandInputs();
	int progValue = 0;
	Ptr<ProgressDialog> progDialog = _ui->createProgressDialog();

	Ptr<StringValueCommandInput> ipInput = cmdInputs->itemById("ipInput");
	Ptr<StringValueCommandInput> portInput = cmdInputs->itemById("portInput");

	if (!ipInput || !portInput)
	{
		_ui->messageBox("Enter ip address or port");
		return;
	}

	Ptr<Products> products = _app->activeDocument()->products();

	if (!products)
		return;

	_cam = Application::get()->activeDocument()->products()->itemByProductType("CAMProductType");

	if (progDialog)
		progDialog->isBackgroundTranslucent(false);

	progDialog->show("Sending data to Axis", "Percentage %p, Current value: %v, Steps: %m", 0, 3, 1);

	// Write config

	Ptr<DropDownCommandInput> setupInput = cmdInputs->itemById("setupSelect");
	Ptr<DropDownCommandInput> opInput = cmdInputs->itemById("operationSelect");

	if (IniFile::isOk())
	{
		progDialog->message("Writing config values");
		IniFile::setString("Settings", "IP", ipInput->value());
		IniFile::setString("Settings", "PORT", portInput->value());
		progDialog->progressValue(++progValue);
	}

	if (!setupInput || !opInput)
	{
		_ui->messageBox("fail setupInput || opInput");
		return;
	}

	if (!_inputChangedHandler->hasFile())
	{
		Ptr<ObjectCollection> opsToPost = ObjectCollection::create();

		Ptr<ListItem> setupSelected = setupInput->selectedItem();
		Ptr<ListItem> opSelected = opInput->selectedItem();

		if (!setupSelected)
		{
			_ui->messageBox("No setup selected");
			return;
		}

		progDialog->message("Gathering operations...");

		if(opSelected)
		{
			for (Ptr<Setup> setup : _cam->setups())
			{
				if (setup->name() == setupSelected->name())
				{
					for (Ptr<ListItem> opSelected : opInput->listItems())
					{
						if (opSelected->isSelected())
						{
							for (Ptr<Operation> op : _cam->allOperations())
							{
								if (op->name() == opSelected->name() && op->parentSetup()->name() == setupSelected->name())
								{
									if (op->hasToolpath())
										opsToPost->add(op);
									else
										_ui->messageBox("The toolpath \"" + op->name() + "\" has no toolpaths, skipping");

									break;
								}
							}
						}
					}
				}
			}

			progDialog->progressValue(++progValue);
		}
		else {
			for (Ptr<Setup> setup : _cam->setups())
			{
				if (setup->name() == setupSelected->name())
				{
					bool isValid = true;

					for (Ptr<Operation> op : setup->allOperations())
					{
						if (!op->hasToolpath()) {
							isValid = false;
							break;
						}
					}

					if (isValid)
						opsToPost->add(setup);
				}
			}
			progDialog->progressValue(++progValue);
		}
		progDialog->message("Post processing...");
		std::string ngcFilename = postProcess(opsToPost);

		std::string wsFilename = "";
//		Sleep(3000);
		progDialog->progressValue(++progValue);

		Ptr<BoolValueCommandInput> genSetupSheet = cmdInputs->itemById("genSetupSheet");

		if (genSetupSheet->value())
		{
			wsFilename = generateWorksheet(opsToPost);
			_ui->messageBox("Generated Setup Sheet: " + wsFilename);
		}

		if (!ngcFilename.empty())
		{
			if (sendFile(ngcFilename, ipInput->value(), portInput->value(), wsFilename))
				_ui->messageBox("Send successfull");
			else
			{
				_ui->messageBox("Send failed");
				goto done;
			}
		}
		else {
			_ui->messageBox("Post process failed..");
		}

	}
	else {
		if (sendFile(_inputChangedHandler->filePath(), ipInput->value(), portInput->value()))
			_ui->messageBox("Send successfull");
		else
			_ui->messageBox("Send failed");
	}
done:

	progDialog->hide();
}

bool PTACommandExecutedHandler::sendFile(const std::string filePath, const std::string ipAddr ,const std::string port, const std::string workSheetPath)
{
	PTASocket socket;
	if (!socket.connectTo(ipAddr, port)) {
		_ui->messageBox("Failed to connect to server..");
		return false;
	}
//	_ui->messageBox("Connected to: " + ipAddr + ':' + port);

	std::string md5 = getMD5(filePath);

	if (md5.empty()) return false;

	size_t fileSize;
	char *fileData = getFileData(filePath, &fileSize);

	if (fileData == nullptr) return false;

	PTAPacket packet(PTA_PACKET_NGC, md5.c_str(), fileData, fileSize);

	delete[] fileData;
	socket.sendPacket(&packet);
	socket.disconnect();
	socket.connectTo(ipAddr, port);

	fileData = getFileData(workSheetPath, &fileSize);

	PTAPacket ssPacket(PTA_PACKET_SS, getMD5(workSheetPath).c_str(), fileData, fileSize);
	socket.sendPacket(&ssPacket);
	/*
	int result = socket.send(md5.c_str(), md5.length());

	if (result < 0)
		return false;

	int len;
	char *r = socket.receiveAll(&len);

	if (r == nullptr) return false;

	if (strncmp(r, "ok", 2) != 0) {
		delete [] r;
		return false;
	}
	//	_ui->messageBox("Server responded: " + std::string(r));
	delete[] r;

	socket.sendFile(filePath.c_str());

	r = socket.receiveAll(&len);
	if (r == nullptr) return false;

	if (strncmp(r, "ok", 2) != 0) {
		delete [] r;
		return false;
	}
//	_ui->messageBox("Server responded: " + std::string(r));

	delete [] r;
	*/
	socket.disconnect();

	return true;
}

#include <stdio.h>
std::string PTACommandExecutedHandler::postProcess(Ptr<ObjectCollection> opsToPost)
{
	char tmpFilename[1024];
	int errno = tmpnam_s((char *)&tmpFilename, 128);
	std::string tmpFilePath(tmpFilename);

	std::string outputPath = tmpFilePath.substr(0, tmpFilePath.find_last_of("\\/"));
	std::string outputFilename = tmpFilePath.substr(tmpFilePath.find_last_of("\\/") + 1);

	std::string postConfig = _cam->genericPostFolder() + '/' + "linuxcnc.cps";

	PostOutputUnitOptions postUnits = PostOutputUnitOptions::MillimetersOutput;

	Ptr<PostProcessInput> postInput = PostProcessInput::create(outputFilename, postConfig, outputPath, postUnits);

	if (postInput == nullptr)
		_ui->messageBox("Failed to create post processor input");

	postInput->isOpenInEditor(false);

	if (!_cam->checkAllToolpaths())
	{
		_ui->messageBox("Some toolpaths are not valid");
		return std::string();
	}

	if (!_cam->postProcess(opsToPost, postInput))
	{
		std::string errMsg;
		int errorCode =_app->getLastError(&errMsg);
		if(errorCode != GenericErrors::Ok)
			_ui->messageBox("Failed to post mine: " + errMsg);

		return std::string();
	}

//	_ui->messageBox("post done");

//	_cam->generateSetupSheet(opsToPost, SetupSheetFormats::HTMLFormat, outputPath);
	return outputPath + "\\" + outputFilename + ".ngc";
}

std::string PTACommandExecutedHandler::generateWorksheet(Ptr<ObjectCollection> opsToPost)
{
	char tmpFilename[1024];
	int errno = tmpnam_s((char *)&tmpFilename, 128);
	std::string tmpFilePath(tmpFilename);

	_cam->generateSetupSheet(opsToPost, SetupSheetFormats::HTMLFormat, tmpFilePath, false);

	return tmpFilePath + "\\" + _app->activeDocument()->name() + ".html";
}

char *PTACommandExecutedHandler::getFileData(const std::string filePath, size_t *size)
{
	char *outputBuffer = nullptr;
	std::ifstream fileToSend(filePath, std::ifstream::binary | std::ifstream::ate);

	if (fileToSend.is_open())
	{
		*size = fileToSend.tellg();
		fileToSend.seekg(fileToSend.beg);

		outputBuffer = new char[*size];

		fileToSend.read(outputBuffer, *size);
		fileToSend.close();
	}

	return outputBuffer;
}

#include <algorithm>
std::string PTACommandExecutedHandler::getMD5(const std::string filePath)
{
	int tries = 5;
	try {
		while (tries > 0)
		{
			std::ifstream file(filePath.c_str(), std::ifstream::binary | std::ifstream::ate);

			if (file) {
				int fileSize = file.tellg();
				file.seekg(file.beg);

				char *buffer = new char[fileSize];
				file.read(buffer, fileSize);


				std::string md5string = md5(std::string(buffer, fileSize));
				file.close();
				delete[] buffer;
				return md5string;
			}

			Sleep(500);
		}
	}
	catch (const std::ifstream::failure &e)
	{
		_ui->messageBox("Fail: " + std::string(e.what()));
	}
	
	return "";
}
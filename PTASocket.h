#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <fstream>

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

#define BUFFER_SIZE 4096

enum : uint32_t {
	PTA_PACKET_NGC = 2,
	PTA_PACKET_SS = 4
};

struct PTAPacket
{
	PTAPacket(uint32_t type, const char *md5, const char *buffer, size_t bufferLen = 0) {
		packetType = type;

		memcpy(&this->md5, md5, 32);

		data = new char[bufferLen];
		memcpy(data, buffer, bufferLen);
		dataLen = bufferLen;

		size = sizeof(PTAPacket) + bufferLen;
	};

	~PTAPacket() {
		if (data != nullptr)
			delete[] data;
	};

	const char identifier[4] = "PTA";
	size_t size;
	uint32_t packetType;
	char md5[32];

	size_t dataLen;
	char *data;
};

class PTASocket
{
public:
	PTASocket();
	~PTASocket();

	bool connectTo(const std::string ipAddr, const std::string port);
	void disconnect();

	int sendPacket(PTAPacket *packet);
	int send(const char *buffer, int len);
	int sendFile(const char *path);

	char *receive(int len);
	char *receiveAll(int *len);
private:
	WSAData _wsaData;
	SOCKET _socket = INVALID_SOCKET;
	sockaddr_in _addr;

	Ptr<UserInterface> _ui;
};

#pragma once
#pragma comment(lib,"ws2_32")
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<thread>
#include<vector>
#include<string>
#include<mutex>
#include<deque>
#include<queue>

#define MAX_SOCKBUF 1024
#define MAX_WORKERTHREAD 4 //쓰레드풀에 넣을 쓰레드의 수

#define H_ECHO 8282
#define H_COORDINATE 3142
#define H_GETNEWBI 4910
#define H_CONNECTION 1000

using namespace std;

const int SERVER_PORT = 8080;
const int MAX_CLIENT = 2000;

enum class IOOperation { //작업 동작의 종류
	RECV, SEND
};

struct OverlappedEx { //WSAOVERLAPPED 구조체를 확장해서 필요한 정보를 더 넣음
	WSAOVERLAPPED m_Overlapped; // Overlapped IO 구조체
	SOCKET m_cliSocket;
	WSABUF m_wsaBuf;
	IOOperation m_Operation;
};

struct ClientInfo {
	SOCKET cliSocket;
	OverlappedEx RecvOverlappedEx;

	char RecvBuf[MAX_SOCKBUF];
	int x = 0;
	int y = 0;

	ClientInfo() {
		ZeroMemory(&RecvOverlappedEx, sizeof(OverlappedEx));
		cliSocket = INVALID_SOCKET;
	}
};

struct PacketData
{
	ClientInfo* clientinfo = nullptr;
	int SessionNumber = 0;
	int DataSize = 0;
	char* pPacketData = nullptr;

	void Set(PacketData& vlaue)
	{
		clientinfo = vlaue.clientinfo;
		DataSize = vlaue.DataSize;
		SessionNumber = vlaue.SessionNumber;

		pPacketData = new char[vlaue.DataSize];
		CopyMemory(pPacketData, vlaue.pPacketData, vlaue.DataSize);
	}

	void Set(ClientInfo* clientinfo_, int sessionNumber, int dataSize_, const void* pData)
	{
		clientinfo = clientinfo_;
		SessionNumber = sessionNumber;
		DataSize = dataSize_;

		pPacketData = new char[dataSize_];
		CopyMemory(pPacketData, pData, dataSize_);
	}

	void Release()
	{
		delete pPacketData;
	}
};
#pragma once
#pragma comment(lib,"ws2_32")
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<thread>
#include<vector>
#include<string>

#define MAX_SOCKBUF 1024
#define MAX_WORKERTHREAD 4 //쓰레드풀에 넣을 쓰레드의 수

#define H_ECHO 8282
#define H_COORDINATE 3142
#define H_GETNEWBI 4910
#define H_CONNECTION 1000

using namespace std;

const int SERVER_PORT = 8080;
const int MAX_CLIENT = 100;

enum class IOOperation { //작업 동작의 종류
	RECV, SEND
};

struct OverlappedEx { //WSAOVERLAPPED 구조체를 확장해서 필요한 정보를 더 넣음
	WSAOVERLAPPED m_Overlapped; // Overlapped IO 구조체
	SOCKET m_cliSocket;
	WSABUF m_wsaBuf;
	char m_Buf[MAX_SOCKBUF]; //data buffer
	IOOperation m_Operation;
};

struct ClientInfo {
	SOCKET cliSocket;
	OverlappedEx RecvOverlappedEx;
	OverlappedEx SendOverlappedEx;

	int x = 0;
	int y = 0;

	ClientInfo() {
		ZeroMemory(&RecvOverlappedEx, sizeof(OverlappedEx));
		ZeroMemory(&SendOverlappedEx, sizeof(OverlappedEx));
		cliSocket = INVALID_SOCKET;
	}
};
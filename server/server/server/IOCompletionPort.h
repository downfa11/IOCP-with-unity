#pragma once
#include"base.h"

class IOCompletionPort {

public:

	vector<ClientInfo> ClientInfos;
	vector<pair<int, string>> chatlog;
	SOCKET ListenSocket = INVALID_SOCKET;
	int ClientCnt = 0;

	vector<thread> IOWorkerThreads;
	thread mAccepterThread;
	thread mProcessThread;

	HANDLE IOCPHandle = INVALID_HANDLE_VALUE; // CompletionPort 객체 handle

	bool isWorkerRun = true; // 작업 쓰레드 동작 flag
	bool isAccepterRun = true; // 접속 쓰레드 동작 flag
	bool IsRunProcessThread = false;

	char SocketBuf[1024] = { 0, }; //socket buffer

	deque<PacketData> PacketDataQueue;
	queue<OverlappedEx*> SendDataQueue;
	mutex mLock;
	mutex sendLock;

	IOCompletionPort(void) {}

	~IOCompletionPort(void) {
		WSACleanup(); //winsock의 사용을 끝낸다.
	}

	bool InitSocket() {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			cout << "WSAStartup() fail. : " << WSAGetLastError();
			return false;
		}

		ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (ListenSocket == INVALID_SOCKET)
		{
			cout << "socket() fail. : " << WSAGetLastError();
			return false;
		}

		return true;
	}

	bool BindandListen(int BindPort) {
		SOCKADDR_IN ServerAddr;
		ServerAddr.sin_family = AF_INET;
		ServerAddr.sin_port = htons(BindPort);
		ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(ListenSocket, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR_IN)) != 0) {
			cout << "bind() fail. : " << WSAGetLastError();
			return false;
		}

		// IOCompletionPort socket을 등록하고 접속 대기 큐를 5개로 설정한다.
		if (listen(ListenSocket, 5) != 0) {
			cout << "listen() fail. : " << WSAGetLastError();
			return false;
		}

		return true;
	}

	bool StartServer(const int maxCount) {

		IsRunProcessThread = true;
		mProcessThread = thread([this]() { ProcessPacket(); });


		CreateClient(maxCount);
		cout << "start" << endl;
		IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (IOCPHandle == NULL) {
			cout << "CreateIoCompletionPort() fail. : " << GetLastError();
			return false;
		}

		bool ret = CreateWorkerThread();
		if (ret == false)
			return false;

		ret = CreateAccepterThread();
		if (ret == false)
			return false;

		cout << "server start." << endl;
		return true;
	}

	void DestroyThread() {

		IsRunProcessThread = false;

		if (mProcessThread.joinable())
		{
			mProcessThread.join();
		}

		isWorkerRun = false;
		CloseHandle(IOCPHandle);
		for (auto& th : IOWorkerThreads) {
			if (th.joinable())
				th.join();
		}

		isAccepterRun = false;
		closesocket(ListenSocket);

		if (mAccepterThread.joinable())
			mAccepterThread.join();
	}


private:

	void CreateClient(const int maxClientCount) {
		for (int i = 0; i < maxClientCount; i++)
			ClientInfos.emplace_back();
	}

	//WaitingThread Queue에서 대기할 쓰레드들을 생성
	bool CreateWorkerThread() {
		unsigned int threadId = 0;
		//대기상태로 넣을 쓰레드를 생성하길 권장하는 수 = cpu*2+1
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
			IOWorkerThreads.emplace_back([this]() { WorkerThread(); });

		return true;
	}

	bool CreateAccepterThread() {
		mAccepterThread = thread([this]() {AccepterThread(); });
		return true;
	}

	ClientInfo* GetEmptyClientInfo() {
		for (auto& client : ClientInfos) {
			if (client.cliSocket == INVALID_SOCKET)
				return &client;
		}

		return nullptr;
	}

	bool BindIOCompletionPort(ClientInfo* clientInfo) {
		//Completion Port 객체와 소켓, CompletionKey를 연결시키는 역할
		auto hIOCP = CreateIoCompletionPort((HANDLE)clientInfo->cliSocket, IOCPHandle, (ULONG_PTR)(clientInfo), 0);
		if (hIOCP == NULL || IOCPHandle != hIOCP) {
			cout << "CreateIoCompletionPort() fail. : " << GetLastError();
			return false;
		}

		return true;

	}

	bool BindRecv(ClientInfo* clientinfo) {
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		clientinfo->RecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		clientinfo->RecvOverlappedEx.m_wsaBuf.buf = clientinfo->RecvBuf;
		clientinfo->RecvOverlappedEx.m_Operation = IOOperation::RECV;

		//socket_error면 client socket이 끊어진걸로 처리한다
		if (WSARecv(clientinfo->cliSocket, &(clientinfo->RecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag,
			(LPWSAOVERLAPPED) & (clientinfo->RecvOverlappedEx), NULL) == SOCKET_ERROR
			&& (WSAGetLastError() != ERROR_IO_PENDING)) {
			cout << "WSARecv() fail : " << WSAGetLastError();
			return false;
		} //overlapped 이라서 PENDING 오류 떠야함. 접수는 됐는데 아직 안끝난거(중첩중)

		return true;

	}

	bool Send(ClientInfo* clientinfo, const void* message, int len, int number) {

		auto sendOverlappedEx = new OverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(OverlappedEx));
		sendOverlappedEx->m_wsaBuf.len = sizeof(int) * 2 + len;
		sendOverlappedEx->m_cliSocket = clientinfo->cliSocket;
		sendOverlappedEx->m_wsaBuf.buf = new char[sendOverlappedEx->m_wsaBuf.len];

		char* bufferPtr = sendOverlappedEx->m_wsaBuf.buf;

		memcpy(bufferPtr, &len, sizeof(int));
		bufferPtr += sizeof(int);

		memcpy(bufferPtr, &number, sizeof(int));
		bufferPtr += sizeof(int);

		memcpy(bufferPtr, message, len);

		sendOverlappedEx->m_Operation = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;

		lock_guard<mutex> guard(sendLock);

		//if(number!= H_COORDINATE)
//cout << clientinfo->cliSocket << " : [" << number << "]" << endl;

		SendDataQueue.push(sendOverlappedEx);
		if (SendDataQueue.size() == 1)
		{
			SendIO();
		}

		return true;
	}

	void SendCompleted(ClientInfo* clientinfo, const int dataSize) {
		lock_guard<mutex> gaurd(sendLock);
		cout << "send completed";
		if (!SendDataQueue.empty() && SendDataQueue.front()->m_wsaBuf.len == dataSize) {
			delete[] SendDataQueue.front()->m_wsaBuf.buf;
			delete SendDataQueue.front();
			SendDataQueue.pop();
			cout << " delete!" << endl;
			cout << "SendDataQueue size : " << SendDataQueue.size() << endl;
		}
		if (!SendDataQueue.empty())	SendIO();
		
	}

	bool SendIO() {

		auto sendOverlappedEx = SendDataQueue.front();
		DWORD dwRecvNumBytes = 0;
		int ret = WSASend(sendOverlappedEx->m_cliSocket, &sendOverlappedEx->m_wsaBuf, 1, &dwRecvNumBytes, 0,
			(LPWSAOVERLAPPED)sendOverlappedEx, NULL);

		if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			cout << "WSASend() fail: " << WSAGetLastError() << endl;
			return false;
		}
		return true;
	}
	// Overlapped IO 작업에 대한 완료 통보를 받아서 그 처리를 하는 함수
	void WorkerThread() {
		ClientInfo* clientinfo = NULL; //CompletionKey를 받을 포인터
		bool success;
		DWORD dwIoSize = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		while (isWorkerRun)
		{
			// 쓰레드들은 WaitingThread Queue에 대기상태로 들어가게 된다.
			// 완료된 IO 작업이 발생하면 IOCP Queue에서 완료된 작업을 가져와서 작업을 수행.
			// PostQueueCompletionStatus() 함수에 의해 사용자 메세지가 도착하면 쓰레드 종료.



			success = GetQueuedCompletionStatus(IOCPHandle, &dwIoSize, (PULONG_PTR)&clientinfo, &lpOverlapped, INFINITE);
			// PULONG_PTR : CompletonKey, LPOVERLLAPED : Overlapped IO 객체

			if (success && dwIoSize == 0 && lpOverlapped == NULL) //사용자 쓰레드 종료 메세지 처리
			{
				isWorkerRun = false;
				continue;
			}

			if (lpOverlapped == NULL)
				continue;

			if (!success || (dwIoSize == 0 && success)) {
				cout << endl;
				cout << "socket " << (int)clientinfo->cliSocket << " disconnected." << endl;
				CloseSocket(clientinfo);
				continue;
			}

			OverlappedEx* overlappedEx = (OverlappedEx*)lpOverlapped;
			if (overlappedEx->m_Operation == IOOperation::RECV) // recv 작업 결과 뒷처리
			{
				clientinfo->RecvBuf[dwIoSize] = '\0';

				int packetNumber;
				memcpy(&packetNumber, clientinfo->RecvBuf + sizeof(int), sizeof(int));

				int messageLength = dwIoSize - sizeof(int);
				char* messageData = clientinfo->RecvBuf + 2 * sizeof(int);

				switch (packetNumber) {

				case H_ECHO:
					cout << "Client " << (int)clientinfo->cliSocket << " (bytes : " << messageLength << ") : " << messageData << endl;
					chatlog.push_back(make_pair((int)clientinfo->cliSocket, messageData));
					for (auto& inst : ClientInfos) {
						if (inst.cliSocket != INVALID_SOCKET)
							Send(&inst, messageData, messageLength, H_ECHO);
					}
					break;

				case H_COORDINATE:
					RecevePosition(*clientinfo, messageData);
					syncPosition(*clientinfo, messageData, messageLength);

					break;


				default:
					cout << "패킷 번호 : " << packetNumber << ", 길이 : " << messageLength << ", 받은 내용 : " << clientinfo->RecvBuf << endl;
					break;
				}




				BindRecv(clientinfo);
			}
			else if (overlappedEx->m_Operation == IOOperation::SEND) {
				SendCompleted(clientinfo, dwIoSize);
				//cout << "[send] "<< ":: ClientID:" + (int)clientinfo->cliSocket << ", bytes : " << dwIoSize << ", message : " << overlappedEx->m_Buf << endl;
			}
			else cout << "[예외] : " << (int)clientinfo->cliSocket << "에서 발생함." << endl;
		}
	}

	void AccepterThread() {
		SOCKADDR_IN clientAddr;
		int addrlen = sizeof(SOCKADDR_IN);

		while (isAccepterRun) {
			ClientInfo* clientinfo = GetEmptyClientInfo();
			if (clientinfo == NULL) {
				cout << "client full" << endl;
				return;
			}

			//접속 요청이 들어올떄까지 기다린다.
			clientinfo->cliSocket = accept(ListenSocket, (SOCKADDR*)&clientAddr, &addrlen);
			if (clientinfo->cliSocket == INVALID_SOCKET)
				continue;

			//IO Completion Port 객체와 소켓을 연결
			if (!BindIOCompletionPort(clientinfo))
				return;

			//recv Overlapped IO 작업을 요청
			if (!BindRecv(clientinfo))
				return;

			char clientIP[32] = { 0, };
			int socket = (int)clientinfo->cliSocket;
			inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1);
			cout << endl;
			cout << socket << " client connect." << endl;
			//cout << "IP : " << clientIP << " socket : " << socket << endl;
			ClientCnt++;
			Send(clientinfo, &socket, sizeof(int), H_CONNECTION);


			for (auto& inst : ClientInfos) {
				if (inst.cliSocket != INVALID_SOCKET)
				{
					int clisocket = (int)inst.cliSocket;
					Send(clientinfo, &clisocket, sizeof(int), H_GETNEWBI);
				}
			}

			for (auto& inst : ClientInfos) {
				if (inst.cliSocket != INVALID_SOCKET) {
					Send(&inst, &socket, sizeof(int), H_GETNEWBI);
				}
			}
		

		}
	}

	void CloseSocket(ClientInfo* clientinfo, bool isForce = false) {
		struct linger stLinger = { 0,0 }; //SO_DONTLINGER 로 설정
		if (isForce)
			stLinger.l_onoff = 1;

		shutdown(clientinfo->cliSocket, SD_BOTH); //송수신 모두 우아한 연결중단

		setsockopt(clientinfo->cliSocket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
		closesocket(clientinfo->cliSocket);
		ClientCnt--;
		clientinfo->cliSocket = INVALID_SOCKET;
	}

	void RecevePosition(ClientInfo& client, const string& message) {
		size_t commaPos = message.find(',');
		if (commaPos != string::npos) {
			client.x = stod(message.substr(0, commaPos));
			client.y = stod(message.substr(commaPos + 1));
		}
	}

	void syncPosition(ClientInfo& client, char* messageData, int messageLength) {

		string socketString = to_string(client.cliSocket);
		string fullMessage = socketString + "," + messageData;

		for (auto& inst : ClientInfos) {
			if (inst.cliSocket != INVALID_SOCKET)
				Send(&inst, fullMessage.c_str(), fullMessage.size(), H_COORDINATE);
		}
	}

	void ProcessPacket()
	{
		while (IsRunProcessThread)
		{
			auto packetData = DequePacketData();
			if (packetData.DataSize != 0)
				Send(packetData.clientinfo, packetData.pPacketData, packetData.DataSize, packetData.SessionNumber);

			else
				this_thread::sleep_for(chrono::milliseconds(1));
		}
	}

	PacketData DequePacketData()
	{
		PacketData packetData;

		lock_guard<std::mutex> guard(mLock);
		if (PacketDataQueue.empty())
		{
			return PacketData();
		}

		packetData.Set(PacketDataQueue.front());

		PacketDataQueue.front().Release();
		PacketDataQueue.pop_front();

		return packetData;
	}

}; 
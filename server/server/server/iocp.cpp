#include "IOCompletionPort.h"
#include<list>


int main() {

    IOCompletionPort ioCompletionPort;

    ioCompletionPort.InitSocket();

    ioCompletionPort.BindandListen(SERVER_PORT);

    ioCompletionPort.StartServer(MAX_CLIENT);

    while (1) {
        /* for (ClientInfo& client : ClientInfos) {
             if (client.cliSocket != INVALID_SOCKET) {
                 UpdateClientInfo(client);
             }
         }*/
        cout << "$ ";

        bool what = false;
        string command;
        getline(cin, command);

        //cout << command << endl;
        if (command == "clientCount")
            cout << "Connected Client : " << ioCompletionPort.ClientCnt << "s" << endl;
        else if (command == "status") {
            for (auto& cli : ioCompletionPort.ClientInfos) {
                if (cli.cliSocket == INVALID_SOCKET)
                    continue;
                cout << cli.cliSocket << "'s Position : " << cli.x << "," << cli.y << endl;
            }
            // 서버 상태 관련 명령 처리
            what = true;
        }
        else if (command == "exit")
        {
            ioCompletionPort.DestroyThread();
            what = true;

            break;
        }
        else if (command == "help") {
            cout << "clientCount : 접속한 클라이언트의 수를 표시합니다." << endl;
            cout << "status : 접속한 클라이언트 전체의 좌표를 표시합니다." << endl;
            cout << "detail {socket} : 접속한 클라이언트의 상세정보를 표시합니다." << endl;
            cout << "chatlog : 접속한 클라이언트들의 채팅 내역을 표시합니다." << endl;
            cout << "exit : 서버를 종료합니다." << endl;
            what = true;
        }

        else if (command == "detail") {

            int client_socket;
            cin >> client_socket;
            cin.ignore();

            for (auto inst : ioCompletionPort.ClientInfos)
            {
                if ((int)inst.cliSocket == client_socket)
                {
                    cout << "Client socket " << (int)inst.cliSocket << endl;
                    cout << "       x " << inst.x << endl;
                    cout << "       y " << inst.y << endl;
                }
            }
            what = true;
        }

        else if (command == "chatlog") {

            auto chatLog = ioCompletionPort.chatlog;
            cout << "Chat log " << endl;
            cout << "size : " << chatLog.size() << endl;
            for (auto chat : chatLog)
                cout << "- " << chat.first<<" : "<<chat.second.c_str() << endl;

            what = true;
        }

        else if (what == false)
            cout << "You can enter 'help' to know commands." << endl;
    }


	return 0;
}
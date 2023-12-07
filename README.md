# IOCP-with-unity

IOCP Socket Network Server with Unity3d **[**Assignment term project**]**

- Client was implemented in unity3d(C#)

[careful] No Timeout, Graceful shutdown


컴퓨터 네트워크 과제로 제출한 IOCP 서버와 Unity 클라이언트 프로젝트
IP 127.0.0.1, Port 8080

dummy는 원하는 수의 클라이언트가 서버에 접속해서 초당 랜덤 좌표로 이동합니다. (x,y : -1,1)

유니티 클라이언트에서는 해당 dummy들의 좌표 이동을 시각화해서 볼 수 있고 채팅이 가능합니다. 

[서버의 명령어]
- help : 도움말을 표시합니다.
- clientCount : 접속한 클라이언트의 수를 표시합니다.
- status : 접속한 클라이언트 전체의 좌표를 표시합니다.;
- detail {socket} : 접속한 클라이언트의 상세정보를 표시합니다.    
- chatlog : 접속한 클라이언트들의 채팅 내역을 표시합니다.
- exit : 서버를 종료합니다.
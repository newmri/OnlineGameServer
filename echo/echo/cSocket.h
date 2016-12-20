#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32")
class cSocket
{
public:
	cSocket(void);
	~cSocket(void);
	// 서버 클라 공통 함수
	// 소켓 초기화
	bool InitSocket();
	// 소켓 연결 종료
	void closeSocket(SOCKET socketClose, bool blsForce = false);
	// 서버용
	// bind and listen
	bool BindandListen(int nBindPort);
	// 요청 수락
	bool StartServer();
	// 클라용
	// 접속
	bool Connect(char* pszIP, int nPort);
private:
	// 서버 연결 소켓
	// 리슨 소켓
	SOCKET m_socket;
	// 수락후 연결 소켓
	SOCKET m_socketConnect;
	// 소켓 버퍼
	char m_szSocketBuf[1024];
};

#include <iostream>
using namespace std;
#include "cSocket.h"
cSocket::cSocket(void)
{
	m_socket = INVALID_SOCKET;
	m_socketConnect = INVALID_SOCKET;
	ZeroMemory(m_szSocketBuf, 1024);
}
cSocket::~cSocket(void)
{
	WSACleanup();
}

bool cSocket::InitSocket()
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet)
	{
		cout << "[에러] 위치 : cSocket::InitSocket(), 이유: WSAStartup() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_socket)
	{
		cout << "[에러] 위치 : cSocket::InitSocket(), 이유: socket() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	cout << "소켓 초기화 성공.." << endl;
	return true;
}

void cSocket::closeSocket(SOCKET socketClose, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };
	// 강제 종료
	if (true == bIsForce)
		stLinger.l_onoff = 1;
	// 송수신 중단
	shutdown(socketClose, SD_BOTH);
	// 옵션 설정
	setsockopt(socketClose, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// 연결 종료
	closesocket(socketClose);
	socketClose = INVALID_SOCKET;
}

bool cSocket::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;

	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int nRet = bind(m_socket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet)
	{
		cout << "[에러] 위치 : cSocket::BindandListen, 이유: bind() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	nRet = listen(m_socket, 5);
	if (0 != nRet)
	{
		cout << "[에러] 위치 : cSocket::BindandListen, 이유: listen() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	cout << "서버 등록 성공.." << endl;
	return true;
}

bool cSocket::StartServer()
{
	char szOutStr[1024];
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	cout << "서버 시작.." << endl;

	// 대기
	m_socketConnect = accept(m_socket, (SOCKADDR*)&stClientAddr, &nAddrLen);
	if (INVALID_SOCKET == m_socketConnect)
	{
		cout << "[에러] 위치 : cSocket::StartServer, 이유: accept() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	sprintf(szOutStr, "클라이언트 접속: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), m_socketConnect);
	cout << szOutStr << endl;

	// echo
	while (true)
	{
		int nRecvLen = recv(m_socketConnect, m_szSocketBuf, 1024, 0);
		if (0 == nRecvLen)
		{
			cout << " 연결 종료" << endl;
			closeSocket(m_socketConnect);

			// 다시 서버 시작
			StartServer();
			return false;
		}

		else if (-1 == nRecvLen)
		{
			cout << "[에러] 위치 : cSocket::StartServer, 이유: recv() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
			closeSocket(m_socketConnect);

			// 다시 서버 시작
			StartServer();
			return false;
		}
		m_szSocketBuf[nRecvLen] = NULL;
		cout << "메세지 수신: 수신 bytes[" << nRecvLen << "], 내용:[" << m_szSocketBuf << "]" << endl;

		int nSendLen = send(m_socketConnect, m_szSocketBuf, nRecvLen, 0);
		if (-1 == nSendLen)
		{
			cout << "[에러] 위치 : cSocket::StartServer, 이유: send() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
			closeSocket(m_socketConnect);
			// 다시 서버 시작
			StartServer();
			return false;
		}
		cout << "메세지 송신: 송신 bytes[" << nSendLen << "], 내용:[" << m_szSocketBuf << "]" << endl;
	}

	// 클라 연결 종료
	closeSocket(m_socketConnect);
	// 리슨 연결 종료
	closeSocket(m_socket);

	cout << "서버 정상 종료.." << endl;
	return true;
}

bool cSocket::Connect(char* szIP, int nPort)
{

	SOCKADDR_IN stServerAddr;
	char szOutMsg[1024];
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(nPort);
	stServerAddr.sin_addr.s_addr = inet_addr(szIP);

	int nRet = connect(m_socket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (SOCKET_ERROR == nRet)
	{
		cout << "[에러] 위치 : cSocket::Connect, 이유: connect() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	cout << "접속 성공.." << endl;
	while (true)
	{
		cout << ">>";
		cin >> szOutMsg;
		if (0 == strcmpi(szOutMsg, "quit"))
			break;

		int nSendLen = send(m_socket, szOutMsg, strlen(szOutMsg), 0);
		if (-1 == nSendLen)
		{
			cout << "[에러] 위치 : cSocket::Connect, 이유: send() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
			return false;
		}
		cout << "메세지 송신: 송신 bytes[" << nSendLen << "], 내용:[" << m_szSocketBuf << "]" << endl;

		int nRecvLen = recv(m_socket, m_szSocketBuf, 1024, 0);
		if (0 == nRecvLen)
		{
			cout << "클라 연결 종료" << endl;
			closeSocket(m_socket);
			return false;
		}
		else if (-1 == nRecvLen)
		{
			cout << "[에러] 위치 : cSocket::Connect, 이유: recv() 함수 실패, ErrorCode:" << WSAGetLastError() << endl;
			closeSocket(m_socket);
			return false;
		}
		m_szSocketBuf[nRecvLen] = NULL;
		cout << "메세지 수신: 수신 bytes[" << nRecvLen << "], 내용:[" << m_szSocketBuf << "]" << endl;
	}
	closeSocket(m_socket);
	cout << "클라 정상 종료.." << endl;
	return true;
}
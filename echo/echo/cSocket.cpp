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
		cout << "[����] ��ġ : cSocket::InitSocket(), ����: WSAStartup() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_socket)
	{
		cout << "[����] ��ġ : cSocket::InitSocket(), ����: socket() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	cout << "���� �ʱ�ȭ ����.." << endl;
	return true;
}

void cSocket::closeSocket(SOCKET socketClose, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };
	// ���� ����
	if (true == bIsForce)
		stLinger.l_onoff = 1;
	// �ۼ��� �ߴ�
	shutdown(socketClose, SD_BOTH);
	// �ɼ� ����
	setsockopt(socketClose, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// ���� ����
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
		cout << "[����] ��ġ : cSocket::BindandListen, ����: bind() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	nRet = listen(m_socket, 5);
	if (0 != nRet)
	{
		cout << "[����] ��ġ : cSocket::BindandListen, ����: listen() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	cout << "���� ��� ����.." << endl;
	return true;
}

bool cSocket::StartServer()
{
	char szOutStr[1024];
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	cout << "���� ����.." << endl;

	// ���
	m_socketConnect = accept(m_socket, (SOCKADDR*)&stClientAddr, &nAddrLen);
	if (INVALID_SOCKET == m_socketConnect)
	{
		cout << "[����] ��ġ : cSocket::StartServer, ����: accept() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	sprintf(szOutStr, "Ŭ���̾�Ʈ ����: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), m_socketConnect);
	cout << szOutStr << endl;

	// echo
	while (true)
	{
		int nRecvLen = recv(m_socketConnect, m_szSocketBuf, 1024, 0);
		if (0 == nRecvLen)
		{
			cout << " ���� ����" << endl;
			closeSocket(m_socketConnect);

			// �ٽ� ���� ����
			StartServer();
			return false;
		}

		else if (-1 == nRecvLen)
		{
			cout << "[����] ��ġ : cSocket::StartServer, ����: recv() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
			closeSocket(m_socketConnect);

			// �ٽ� ���� ����
			StartServer();
			return false;
		}
		m_szSocketBuf[nRecvLen] = NULL;
		cout << "�޼��� ����: ���� bytes[" << nRecvLen << "], ����:[" << m_szSocketBuf << "]" << endl;

		int nSendLen = send(m_socketConnect, m_szSocketBuf, nRecvLen, 0);
		if (-1 == nSendLen)
		{
			cout << "[����] ��ġ : cSocket::StartServer, ����: send() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
			closeSocket(m_socketConnect);
			// �ٽ� ���� ����
			StartServer();
			return false;
		}
		cout << "�޼��� �۽�: �۽� bytes[" << nSendLen << "], ����:[" << m_szSocketBuf << "]" << endl;
	}

	// Ŭ�� ���� ����
	closeSocket(m_socketConnect);
	// ���� ���� ����
	closeSocket(m_socket);

	cout << "���� ���� ����.." << endl;
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
		cout << "[����] ��ġ : cSocket::Connect, ����: connect() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
		return false;
	}
	cout << "���� ����.." << endl;
	while (true)
	{
		cout << ">>";
		cin >> szOutMsg;
		if (0 == strcmpi(szOutMsg, "quit"))
			break;

		int nSendLen = send(m_socket, szOutMsg, strlen(szOutMsg), 0);
		if (-1 == nSendLen)
		{
			cout << "[����] ��ġ : cSocket::Connect, ����: send() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
			return false;
		}
		cout << "�޼��� �۽�: �۽� bytes[" << nSendLen << "], ����:[" << m_szSocketBuf << "]" << endl;

		int nRecvLen = recv(m_socket, m_szSocketBuf, 1024, 0);
		if (0 == nRecvLen)
		{
			cout << "Ŭ�� ���� ����" << endl;
			closeSocket(m_socket);
			return false;
		}
		else if (-1 == nRecvLen)
		{
			cout << "[����] ��ġ : cSocket::Connect, ����: recv() �Լ� ����, ErrorCode:" << WSAGetLastError() << endl;
			closeSocket(m_socket);
			return false;
		}
		m_szSocketBuf[nRecvLen] = NULL;
		cout << "�޼��� ����: ���� bytes[" << nRecvLen << "], ����:[" << m_szSocketBuf << "]" << endl;
	}
	closeSocket(m_socket);
	cout << "Ŭ�� ���� ����.." << endl;
	return true;
}
#include "cAsyncSocket.h"
cAsyncSocket::cAsyncSocket(void)
{
	m_socketListen = INVALID_SOCKET;
	m_hWnd = NULL;
	m_pMainDlg = NULL;
	ZeroMemory(m_szSocketBuf, 1024);
}

cAsyncSocket::~cAsyncSocket(void)
{
	// ���� ��� ����
	WSACleanup();
}

// ���� �ʱ�ȭ �Լ�
bool cAsyncSocket::InitSocket(HWND hWnd)
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
	//	m_pMainDlg->OutputMsg("[����] WSAStartup()�Լ� ����: %d", WSAGetLastError());
		return false;
	}
	// ���� ������ TCP ���� ����
	m_socketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_socketListen) {
	//	m_pMainDlg->OutputMsg("[����] socket()�Լ� ����: %d", WSAGetLastError());
		return false;
	}

	// ������ �ڵ� ����
	m_hWnd = hWnd;
	//m_pMainDlg->OutputMsg("���� �ʱ�ȭ ����");
	return true;

}

// �ش� ���� ����
void cAsyncSocket::CloseSocket(SOCKET socketClose, bool bIsForce)
{
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER�� ����
	// bIsForce�� true�̸� SO_LINGER, timeout=0���� �����Ͽ� ���� ����
	if (true == bIsForce)
		stLinger.l_onoff = 1;

	// ������ �ۼ��� �ߴ�
	shutdown(socketClose, SD_BOTH);
	// ���� �ɼ� ����
	setsockopt(socketClose, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// ���� ����
	closesocket(socketClose);

	socketClose = INVALID_SOCKET;
}

// ������ �ּ� ������ ����, ���� ��û ��� �Լ�
bool cAsyncSocket::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	// ������Ʈ ����
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_socketListen, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		//m_pMainDlg->OutputMsg("[���� bind()�Լ� ����: %d", WSAGetLastError());
		return false;
	}

	// ���� ��� ť 5
	nRet = listen(m_socketListen, 5);
	if (0 != nRet) {
		//m_pMainDlg->OutputMsg("[����] listen()�Լ� ����: %d", WSAGetLastError());
		return false;
	}
	//m_pMainDlg->OutputMsg("���� ��� ����..");
	return true;
}

bool cAsyncSocket::StartServer()
{
	// �̺�Ʈ ���
	int nRet = WSAAsyncSelect(m_socketListen, m_hWnd, WM_SOCKETMSG, FD_ACCEPT | FD_CLOSE);
	if (SOCKET_ERROR == nRet) {
		//m_pMainDlg->OutputMsg("[����] WSAAsyncSelect() �Լ� ����: %d", WSAGetLastError());
		return false;
	}
	return true;


}
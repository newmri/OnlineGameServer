#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32")
class cSocket
{
public:
	cSocket(void);
	~cSocket(void);
	// ���� Ŭ�� ���� �Լ�
	// ���� �ʱ�ȭ
	bool InitSocket();
	// ���� ���� ����
	void closeSocket(SOCKET socketClose, bool blsForce = false);
	// ������
	// bind and listen
	bool BindandListen(int nBindPort);
	// ��û ����
	bool StartServer();
	// Ŭ���
	// ����
	bool Connect(char* pszIP, int nPort);
private:
	// ���� ���� ����
	// ���� ����
	SOCKET m_socket;
	// ������ ���� ����
	SOCKET m_socketConnect;
	// ���� ����
	char m_szSocketBuf[1024];
};

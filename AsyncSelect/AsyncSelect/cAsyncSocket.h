#pragma once
#include"stdafx.h"
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32")
#define WM_SOCKETMSG WM_USER+1
class CAsyncSelectDlg;
class cAsyncSocket
{
public:
	cAsyncSocket(void);
	~cAsyncSocket(void);

	/* ���� Ŭ�� �����Լ� */
	// ���� �ʱ�ȭ �Լ�
	bool InitSocket(HWND hWnd);
	// ���� ������ ����
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);

	/* ������ �Լ� */
	// ������ �ּ������� ���ϰ� ����, ���� ��û�� �ޱ� ���� ������ ���
	bool BindandListen(int nBindPort);
	// ���� ��û�� �����ϰ� �޼����� �޾Ƽ� ó���ϴ� �Լ�
	bool StartServer();
	void setMainDlg(CAsyncSelectDlg* pMainDlg) { m_pMainDlg = pMainDlg; }
private:
	// Ŭ���̾�Ʈ ���ӿ�û ��� ����
	SOCKET m_socketListen;
	// ��Ʈ��ũ �̺�Ʈ�� �߻��� �޼����� ���� ������ �ڵ�
	HWND m_hWnd;
	// ���� ������ ������
	CAsyncSelectDlg* m_pMainDlg;
	// ���� ����
	char m_szSocketBuf[1024];
};


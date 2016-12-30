
// OverlappedCallback.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
class COverlappedCallbackDlg;
#define MAX_SOCKBUF 1024

// COverlappedCallbackApp:
// See OverlappedCallback.cpp for the implementation of this class

// Overlapped I/O �۾� ���� ����
enum enumOperation { OP_RECV, OP_SEND };
// WSAOVERLAPPED ����ü Ȯ��
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped; // I/O ����ü
	SOCKET m_socketClient;
	WSABUF m_wsaBuf; // I/O �۾� ����
	char m_szBuf[MAX_SOCKBUF]; // ������ ����
	enumOperation m_eOperation; // �۾� ���� ����
	void* m_pOverlappedCallback; // �ݹ� �Լ� ������
};

class cOverlappedCallback
{
public:
	cOverlappedCallback(void);
	~cOverlappedCallback(void);
	// -- ���� Ŭ�� ���� �Լ� -- //
	// ���� �ʱ�ȭ �Լ�
	bool InitSocket();
	// ���� ����
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);

	// -- ������ �Լ� -- //
	bool BindandListen(int nBindPort);
	// ���� ��û ���� �� �޼��� ó��
	bool StartServer();
	// accept ��û ó�� ������ ����
	bool CreateAccepterThread();
	// WSARecv Overlapped I/O �۾�
	bool BindRecv(SOCKET socket);
	// WSASend Overlapped I/O �۾�
	bool SendMsg(SOCKET socket, char* pMsg, int nLen);
	// ����� ���� �޴� ������
	void AccepterThread();

	void SetMainDlg(COverlappedCallbackDlg* pMainDlg) { m_pMainDlg = pMainDlg; }
	COverlappedCallbackDlg* GetMainDlg() { return m_pMainDlg; }

	// ������ ������ �ı�
	void DestoryThread();

private:
	// ���ӵ� Ŭ�� ��
	int m_nClientCnt;
	// ���� ������ ������
	COverlappedCallbackDlg* m_pMainDlg;
	// ���� ������ �ڵ�
	HANDLE m_hAccepterThread;
	// ���� ������ ���� �÷���
	bool m_bAccepterRun;
	SOCKET m_sockListen;
	// ���� ����
	char m_szSocketBuf[1024];
};
class COverlappedCallbackApp : public CWinApp
{
public:
	COverlappedCallbackApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern COverlappedCallbackApp theApp;
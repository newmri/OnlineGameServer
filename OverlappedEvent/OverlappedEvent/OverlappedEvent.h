
// OverlappedEvent.h : main header file for the PROJECT_NAME application
//

#pragma once
class COverlappedEventDlg;
#define MAX_SOCKBUF 1024
// Overlapped I/O�۾� ���� ����
enum enumOperation { OP_RECV, OP_SEND };
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// WSAOVERLAPPED����ü�� Ȯ�� ���� �ʿ� ������ ����
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped; // IO ����ü
	int m_nIdx; //stClientInfo ����ü �迭 �ε���
	WSABUF m_wsaBuf; // OVerlapped I/O �۾� ����
	char m_szBuf[MAX_SOCKBUF]; // ������ ����
	enumOperation m_eOperation; // Overlapped IO �۾� ���� ����
};

// Ŭ���̾�Ʈ ������ ������� ����ü���� 0��° �迭���� ���̷� ������ ������ ���ο� ���ӿ� ����
// �̺�Ʈ ������ ���� WSAWaitForMultipleEvents�� �ٽ� �ɾ��ش�.
struct stClientInfo {
	SOCKET m_socketClient[WSA_MAXIMUM_WAIT_EVENTS]; // Client�� ����Ǵ� ����
	WSAEVENT m_eventHandle[WSA_MAXIMUM_WAIT_EVENTS]; // �̺�Ʈ ������ ���� �̺�Ʈ ��ü
	stOverlappedEx m_stOverlappedEx[WSA_MAXIMUM_WAIT_EVENTS];
};

class cOverlappedEvent
{
public:
	cOverlappedEvent(void);
	~cOverlappedEvent(void);

	// ----���� Ŭ�� ���� �Լ� ---- //
	// ���� �ʱ�ȭ �Լ�
	bool InitSocket();
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);
	// ---- ������ �Լ� ---- //
	bool BindandListen(int nBindPort);
	bool StartServer();

	// Overlapeed I/O �۾��� ���� �����带 ����
	bool CreateWokerThread();
	// acceppt ��û ó�� ������
	bool CreateAccepterThread();
	// ��� ���� ���� index ��ȯ
	int GetEmptyIndex();
	// WSARecv Overlapped I/O �۾��� ��Ŵ
	bool BindRecv(int nIdx);
	// WSASend Overlapped I/O �۾��� ��Ŵ
	bool SendMsg(int nIdx, char* pMsg, int nLen);

	// Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� ó��
	void WokerThread();
	// ���� ���� 
	void AccepterThread();
	// I/O �Ϸῡ ���� ��� ó��
	void OverlappedResult(int nIdx);

	void SetMainDlg(COverlappedEventDlg* pMainDlg) { m_pMainDlg = pMainDlg; }

	// ���� �� ������ �ı�
	void DestroyThread();

private:
	// Ŭ�� ���� ���� ����ü
	stClientInfo m_stClientInfo;

	// ���� �� Ŭ�� ��
	int m_nClientCnt;
	// ���� ������ ������
	COverlappedEventDlg* m_pMainDlg;
	// �۾� ������ �ڵ�
	HANDLE m_hWorkerThread;
	// ���� ������ �ڵ�
	HANDLE m_hAcceptThread;
	// �۾� ������ ���� �÷���
	bool m_bWorkerRun;
	// ���� ������ ���� �÷���
	bool m_bAccepterRun;
	// ���� ����
	char m_szSocketBuf[1024];
};

// COverlappedEventApp:
// See OverlappedEvent.cpp for the implementation of this class
//

class COverlappedEventApp : public CWinApp
{
public:
	COverlappedEventApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern COverlappedEventApp theApp;
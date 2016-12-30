
// OverlappedCallback.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OverlappedCallback.h"
#include "OverlappedCallbackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COverlappedCallbackApp

BEGIN_MESSAGE_MAP(COverlappedCallbackApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COverlappedCallbackApp construction

COverlappedCallbackApp::COverlappedCallbackApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only COverlappedCallbackApp object

COverlappedCallbackApp theApp;

// ������ �ޱ� ���� ��� �ϴ� ������
unsigned int WINAPI CallAccepterThread(LPVOID p)
{
	cOverlappedCallback* pOverlappedCallback = (cOverlappedCallback*)p;
	pOverlappedCallback->AccepterThread();
	return 0;
}
// Overlapped I/O �۾� �Ϸ� �뺸�� �޴� �ݹ� �Լ�
void CALLBACK CompletionRoutine(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED Overlapped, DWORD dwFlags);

cOverlappedCallback::cOverlappedCallback(void)
{
	m_pMainDlg = NULL;
	m_bAccepterRun = true;
	m_nClientCnt = 0;
	m_hAccepterThread = NULL;
	m_sockListen = INVALID_SOCKET;
	ZeroMemory(m_szSocketBuf, 1024);
}

cOverlappedCallback::~cOverlappedCallback(void)
{
	// ���� ��� ����
	WSACleanup();

	closesocket(m_sockListen);
	m_bAccepterRun = false;
	// ������ ���� ���
	WaitForSingleObject(m_hAccepterThread, INFINITE);

}

// ���� �ʱ�ȭ �Լ�
bool cOverlappedCallback::InitSocket()
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[����] WSAStartup()�Լ� ����: %d", WSAGetLastError());
		return false;
	}

	// TCP Overlapped I/O ���� ����
	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_sockListen) {
		m_pMainDlg->OutputMsg("[����] socket()�Լ� ����: %d", WSAGetLastError());
		return false;
	}

	// ������ �ڵ� ����
	m_pMainDlg->OutputMsg("���� �ʱ�ȭ ����");
	return true;

}
// �ش� ���� ����
void cOverlappedCallback::CloseSocket(SOCKET socketClose, bool bIsForce)
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
// ���� �ּ� ������ ���ϰ� ����, ���� ��û ���� ���� ��� �Լ�
bool cOverlappedCallback::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	// ������Ʈ ����
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_sockListen, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[���� bind()�Լ� ����: %d", WSAGetLastError());
		return false;
	}

	// ���� ��� ť 5
	nRet = listen(m_sockListen, 5);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[����] listen()�Լ� ����: %d", WSAGetLastError());
		return false;
	}
	m_pMainDlg->OutputMsg("���� ��� ����..");
	return true;
}

bool cOverlappedCallback::CreateAccepterThread()
{
	unsigned int uiThreadId = 0;
	m_hAccepterThread = (HANDLE)_beginthreadex(NULL, 0, &CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hAccepterThread == NULL) {
		m_pMainDlg->OutputMsg("AccepterThread Creation Failed: %u", GetLastError());
		return false;
	}
	ResumeThread(m_hAccepterThread);
	m_pMainDlg->OutputMsg("AccepterThread����..");
	return true;

}
bool cOverlappedCallback::StartServer()
{
	// ���ӵ� Ŭ�� �ּ� ���� ���� ����ü
	bool bRet = CreateAccepterThread();
	if (false == bRet) return false;

	m_pMainDlg->OutputMsg("���� ����");
	return true;

}
bool cOverlappedCallback::BindRecv(SOCKET socket)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// I/O�� ���� ����ü ����
	stOverlappedEx* pOverlappedEx = new stOverlappedEx;
	// ����ü �ʱ�ȭ
	ZeroMemory(pOverlappedEx, sizeof(WSAOVERLAPPED));

	// Overlapped I/O�� ���� �� ������ ����
	pOverlappedEx->m_wsaBuf.len = MAX_SOCKBUF;
	pOverlappedEx->m_wsaBuf.buf = &pOverlappedEx->m_szBuf[0];
	pOverlappedEx->m_socketClient = socket;
	pOverlappedEx->m_eOperation = OP_RECV;

	pOverlappedEx->m_pOverlappedCallback = this;

	int nRet = WSARecv(socket, &(pOverlappedEx->m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED)(pOverlappedEx), CompletionRoutine);

	// socket_eeror �̸� socket�� ������ �ɷ� ó��
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[����] WSARecv()�Լ� ����: %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool cOverlappedCallback::SendMsg(SOCKET socket, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// I/O�� ���� ����ü ����
	stOverlappedEx* pOverlappedEx = new stOverlappedEx;
	// ����ü �ʱ�ȭ
	ZeroMemory(pOverlappedEx, sizeof(WSAOVERLAPPED));
	// ���۵� �޼����� ����
	CopyMemory(pOverlappedEx->m_szBuf, pMsg, nLen);

	// Overlapped I/O�� ���� �� ������ ����
	pOverlappedEx->m_wsaBuf.buf = pOverlappedEx->m_szBuf;
	pOverlappedEx->m_wsaBuf.len = nLen;
	pOverlappedEx->m_socketClient = socket;
	pOverlappedEx->m_eOperation = OP_SEND;

	pOverlappedEx->m_pOverlappedCallback = this;

	int nRet = WSASend(socket, &(pOverlappedEx->m_wsaBuf), 1, &dwRecvNumBytes,0, (LPWSAOVERLAPPED)(pOverlappedEx), CompletionRoutine);

	// socket_eeror �̸� socket�� ������ �ɷ� ó��
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[����] WSASend()�Լ� ����: %d", WSAGetLastError());
		return false;
	}
	return true;
}
// ����� ���� �޴� ������
void cOverlappedCallback::AccepterThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun) {
		
		// Ŭ�� ���� ��û ���
		SOCKET sockAccept= accept(m_sockListen, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == sockAccept) continue;

		bool bRet = BindRecv(sockAccept);
		if (false == bRet) return;
		m_pMainDlg->OutputMsg("Ŭ���̾�Ʈ ����: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), sockAccept);
	}
}
void CALLBACK CompletionRoutine(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFalgs)
{
	stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

	// class�� �����͸� �޴´�
	cOverlappedCallback* pOverlappedCallback = (cOverlappedCallback*)pOverlappedEx->m_pOverlappedCallback;
	COverlappedCallbackDlg* pMainDlg = pOverlappedCallback->GetMainDlg();

	// ���� ����
	if (0 == dwTransferred) {
		pMainDlg->OutputMsg("[���� ����] socket: %d", pOverlappedEx->m_socketClient);
		pOverlappedCallback->CloseSocket(pOverlappedEx->m_socketClient);
		goto end;
	}

	if (0 != dwError) {
		pMainDlg->OutputMsg("[����] CompletionRoutine ����: %d", WSAGetLastError());
		goto end;
	}

	switch (pOverlappedEx->m_eOperation)
	{
		// WSARecv�� Overlapped I/O�� �Ϸ� �Ǿ���.
	case OP_RECV:
	{
		pOverlappedEx->m_szBuf[dwTransferred] = NULL;
		pMainDlg->OutputMsg("[����] bytes : %d, msg: %s", dwTransferred, pOverlappedEx->m_szBuf);
		// �޽��� ����
		pOverlappedCallback->SendMsg(pOverlappedEx->m_socketClient, pOverlappedEx->m_szBuf, dwTransferred);
	}
	break;
	// WSASend�� Overlapped I/O�� �Ϸ� �Ǿ���.
	case OP_SEND:
	{
		pOverlappedEx->m_szBuf[dwTransferred] = NULL;
		pMainDlg->OutputMsg("[�۽�] bytes : %d, msg: %s", dwTransferred, pOverlappedEx->m_szBuf);
		// �ٽ� WSASend�� Overlapped I/O�� �ɾ���
		pOverlappedCallback->BindRecv(pOverlappedEx->m_socketClient);
	}
	break;
	default:
	{
		pMainDlg->OutputMsg("���ǵ��� ���� Operation");
	}
	break;
	}
end: delete pOverlappedEx;
}

void cOverlappedCallback::DestoryThread()
{
	closesocket(m_sockListen);
	m_bAccepterRun = false;
	// ������ ���Ḧ ��ٸ�
	WaitForSingleObject(m_hAccepterThread, INFINITE);
}
// COverlappedCallbackApp initialization

BOOL COverlappedCallbackApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	COverlappedCallbackDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


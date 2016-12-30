
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

// 접속을 받기 위해 대기 하는 쓰레드
unsigned int WINAPI CallAccepterThread(LPVOID p)
{
	cOverlappedCallback* pOverlappedCallback = (cOverlappedCallback*)p;
	pOverlappedCallback->AccepterThread();
	return 0;
}
// Overlapped I/O 작업 완료 통보를 받는 콜백 함수
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
	// 윈속 사용 종료
	WSACleanup();

	closesocket(m_sockListen);
	m_bAccepterRun = false;
	// 쓰레드 종료 대기
	WaitForSingleObject(m_hAccepterThread, INFINITE);

}

// 소켓 초기화 함수
bool cOverlappedCallback::InitSocket()
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러] WSAStartup()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// TCP Overlapped I/O 소켓 생성
	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_sockListen) {
		m_pMainDlg->OutputMsg("[에러] socket()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 윈도우 핸들 저장
	m_pMainDlg->OutputMsg("소켓 초기화 성공");
	return true;

}
// 해당 소켓 종료
void cOverlappedCallback::CloseSocket(SOCKET socketClose, bool bIsForce)
{
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER로 설정
									  // bIsForce가 true이면 SO_LINGER, timeout=0으로 설정하여 강제 종료
	if (true == bIsForce)
		stLinger.l_onoff = 1;

	// 데이터 송수신 중단
	shutdown(socketClose, SD_BOTH);
	// 소켓 옵션 설정
	setsockopt(socketClose, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// 소켓 종료
	closesocket(socketClose);

	socketClose = INVALID_SOCKET;

}
// 서버 주소 정보를 소켓과 연결, 접속 요청 위해 소켓 등록 함수
bool cOverlappedCallback::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	// 서버포트 설정
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_sockListen, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러 bind()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 접속 대기 큐 5
	nRet = listen(m_sockListen, 5);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러] listen()함수 실패: %d", WSAGetLastError());
		return false;
	}
	m_pMainDlg->OutputMsg("서버 등록 성공..");
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
	m_pMainDlg->OutputMsg("AccepterThread시작..");
	return true;

}
bool cOverlappedCallback::StartServer()
{
	// 접속된 클라 주소 정보 저장 구조체
	bool bRet = CreateAccepterThread();
	if (false == bRet) return false;

	m_pMainDlg->OutputMsg("서버 시작");
	return true;

}
bool cOverlappedCallback::BindRecv(SOCKET socket)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// I/O를 위한 구조체 생성
	stOverlappedEx* pOverlappedEx = new stOverlappedEx;
	// 구조체 초기화
	ZeroMemory(pOverlappedEx, sizeof(WSAOVERLAPPED));

	// Overlapped I/O를 위해 각 정보를 세팅
	pOverlappedEx->m_wsaBuf.len = MAX_SOCKBUF;
	pOverlappedEx->m_wsaBuf.buf = &pOverlappedEx->m_szBuf[0];
	pOverlappedEx->m_socketClient = socket;
	pOverlappedEx->m_eOperation = OP_RECV;

	pOverlappedEx->m_pOverlappedCallback = this;

	int nRet = WSARecv(socket, &(pOverlappedEx->m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED)(pOverlappedEx), CompletionRoutine);

	// socket_eeror 이면 socket이 끊어진 걸로 처리
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[에러] WSARecv()함수 실패: %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool cOverlappedCallback::SendMsg(SOCKET socket, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// I/O를 위한 구조체 생성
	stOverlappedEx* pOverlappedEx = new stOverlappedEx;
	// 구조체 초기화
	ZeroMemory(pOverlappedEx, sizeof(WSAOVERLAPPED));
	// 전송될 메세지를 복사
	CopyMemory(pOverlappedEx->m_szBuf, pMsg, nLen);

	// Overlapped I/O를 위해 각 정보를 세팅
	pOverlappedEx->m_wsaBuf.buf = pOverlappedEx->m_szBuf;
	pOverlappedEx->m_wsaBuf.len = nLen;
	pOverlappedEx->m_socketClient = socket;
	pOverlappedEx->m_eOperation = OP_SEND;

	pOverlappedEx->m_pOverlappedCallback = this;

	int nRet = WSASend(socket, &(pOverlappedEx->m_wsaBuf), 1, &dwRecvNumBytes,0, (LPWSAOVERLAPPED)(pOverlappedEx), CompletionRoutine);

	// socket_eeror 이면 socket이 끊어진 걸로 처리
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[에러] WSASend()함수 실패: %d", WSAGetLastError());
		return false;
	}
	return true;
}
// 사용자 접속 받는 쓰레드
void cOverlappedCallback::AccepterThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun) {
		
		// 클라 접속 요청 대기
		SOCKET sockAccept= accept(m_sockListen, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == sockAccept) continue;

		bool bRet = BindRecv(sockAccept);
		if (false == bRet) return;
		m_pMainDlg->OutputMsg("클라이언트 접속: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), sockAccept);
	}
}
void CALLBACK CompletionRoutine(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFalgs)
{
	stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

	// class의 포인터를 받는다
	cOverlappedCallback* pOverlappedCallback = (cOverlappedCallback*)pOverlappedEx->m_pOverlappedCallback;
	COverlappedCallbackDlg* pMainDlg = pOverlappedCallback->GetMainDlg();

	// 접속 끊김
	if (0 == dwTransferred) {
		pMainDlg->OutputMsg("[접속 끊김] socket: %d", pOverlappedEx->m_socketClient);
		pOverlappedCallback->CloseSocket(pOverlappedEx->m_socketClient);
		goto end;
	}

	if (0 != dwError) {
		pMainDlg->OutputMsg("[에러] CompletionRoutine 실패: %d", WSAGetLastError());
		goto end;
	}

	switch (pOverlappedEx->m_eOperation)
	{
		// WSARecv로 Overlapped I/O가 완료 되었다.
	case OP_RECV:
	{
		pOverlappedEx->m_szBuf[dwTransferred] = NULL;
		pMainDlg->OutputMsg("[수신] bytes : %d, msg: %s", dwTransferred, pOverlappedEx->m_szBuf);
		// 메시지 에코
		pOverlappedCallback->SendMsg(pOverlappedEx->m_socketClient, pOverlappedEx->m_szBuf, dwTransferred);
	}
	break;
	// WSASend로 Overlapped I/O가 완료 되었다.
	case OP_SEND:
	{
		pOverlappedEx->m_szBuf[dwTransferred] = NULL;
		pMainDlg->OutputMsg("[송신] bytes : %d, msg: %s", dwTransferred, pOverlappedEx->m_szBuf);
		// 다시 WSASend로 Overlapped I/O를 걸어줌
		pOverlappedCallback->BindRecv(pOverlappedEx->m_socketClient);
	}
	break;
	default:
	{
		pMainDlg->OutputMsg("정의되지 않은 Operation");
	}
	break;
	}
end: delete pOverlappedEx;
}

void cOverlappedCallback::DestoryThread()
{
	closesocket(m_sockListen);
	m_bAccepterRun = false;
	// 쓰레드 종료를 기다림
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


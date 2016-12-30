
// OverlappedEvent.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OverlappedEvent.h"
#include "OverlappedEventDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COverlappedEventApp

BEGIN_MESSAGE_MAP(COverlappedEventApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COverlappedEventApp construction

COverlappedEventApp::COverlappedEventApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only COverlappedEventApp object

COverlappedEventApp theApp;

// WSARecv와 WSASend의 Overlapped I/O 작업 처리
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	cOverlappedEvent* pOverlappedEvent = (cOverlappedEvent*)p;
	pOverlappedEvent->WokerThread();
	return 0;
}
unsigned int WINAPI CallAccepterThread(LPVOID p)
{
	cOverlappedEvent* pOverlappedEvent = (cOverlappedEvent*)p;
	pOverlappedEvent->AccepterThread();
	return 0;
}

cOverlappedEvent::cOverlappedEvent(void)
{
	// 변수 초기화
	m_pMainDlg = NULL;
	m_bWorkerRun = true;
	m_bAccepterRun = true;
	m_nClientCnt = 0;
	m_hWorkerThread = NULL;
	m_nClientCnt = 0;
	m_hWorkerThread = NULL;
	m_hAccepterThread= NULL;
	ZeroMemory(m_szSocketBuf, 1024);

	// 클라 구조체 초기화
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		m_stClientInfo.m_socketClient[i] = INVALID_SOCKET;
		m_stClientInfo.m_eventHandle[i] = WSACreateEvent();
		ZeroMemory(&m_stClientInfo.m_stOverlappedEx[i], sizeof(WSAOVERLAPPED));
	}
}
cOverlappedEvent::~cOverlappedEvent(void)
{
	// 윈속 사용을 끝낸다
	WSACleanup();

	// listen소켓 닫음
	closesocket(m_stClientInfo.m_socketClient[0]);
	SetEvent(m_stClientInfo.m_eventHandle[0]);
	m_bWorkerRun = false;
	m_bAccepterRun = false;
	// 쓰레드 종료 기다림
	WaitForSingleObject(m_hWorkerThread, INFINITE);
	WaitForSingleObject(m_hAccepterThread, INFINITE);
}

// 소켓 초기화 함수
bool cOverlappedEvent::InitSocket()
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러] WSAStartup()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// TCP Overlapped I/O 소켓 생성
	m_stClientInfo.m_socketClient[0] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_stClientInfo.m_socketClient[0]) {
		m_pMainDlg->OutputMsg("[에러] socket()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 윈도우 핸들 저장
	m_pMainDlg->OutputMsg("소켓 초기화 성공");
	return true;
}

// 해당 소켓 종료
void cOverlappedEvent::CloseSocket(SOCKET socketClose, bool bIsForce)
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

bool cOverlappedEvent::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	// 서버포트 설정
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_stClientInfo.m_socketClient[0], (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러 bind()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 접속 대기 큐 5
	nRet = listen(m_stClientInfo.m_socketClient[0], 5);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러] listen()함수 실패: %d", WSAGetLastError());
		return false;
	}
	m_pMainDlg->OutputMsg("서버 등록 성공..");
	return true;

}

bool cOverlappedEvent::CreateWokerThread()
{
	unsigned int uiThreadId = 0;
	m_hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hWorkerThread == NULL) {
		 m_pMainDlg->OutputMsg("WorkerThread Creation Failed: %u",GetLastError());
		return false;
	}
	ResumeThread(m_hWorkerThread);
	m_pMainDlg->OutputMsg("WorkerThread시작..");
	return true;
}

bool cOverlappedEvent::CreateAccepterThread()
{
	unsigned int uiThreadId = 0;
	m_hAccepterThread = (HANDLE)_beginthreadex(NULL, 0, &CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hAccepterThread == NULL) {
		m_pMainDlg->OutputMsg("AccepterThread Creation Failed: %u",GetLastError());
		return false;
	}
	ResumeThread(m_hAccepterThread);
	 m_pMainDlg->OutputMsg("AccepterThread시작..");
	return true;
}

// 사용 되지 않은 Index 반환
int cOverlappedEvent::GetEmptyIndex()
{
	// 0번째 배열은 정보 갱신용 이벤트
	for (int i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		if (INVALID_SOCKET == m_stClientInfo.m_socketClient[i])
			return i;
	}
	return -1;
}

bool cOverlappedEvent::StartServer()
{
	// 접속된 클라 주소 정보 저장 구조체
	bool bRet = CreateWokerThread();
	if (false == bRet) return false;
	bRet = CreateAccepterThread();
	if (false == bRet) return false;

	// 정보 갱신 이벤트 생성
	m_stClientInfo.m_eventHandle[0] = WSACreateEvent();
	m_pMainDlg->OutputMsg("서버 시작");
	return true;

}
bool cOverlappedEvent::BindRecv(int nIdx)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	m_stClientInfo.m_eventHandle[nIdx] = WSACreateEvent();
	// Overlapped I/O를 위해 각 정보를 셋팅
	m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaOverlapped.hEvent = m_stClientInfo.m_eventHandle[nIdx];
	m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaBuf.len = MAX_SOCKBUF;
	m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaBuf.buf = m_stClientInfo.m_stOverlappedEx[nIdx].m_szBuf;
	m_stClientInfo.m_stOverlappedEx[nIdx].m_nIdx = nIdx;
	m_stClientInfo.m_stOverlappedEx[nIdx].m_eOperation = OP_RECV;
	int nRet = WSARecv(m_stClientInfo.m_socketClient[nIdx], &(m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag,
		(LPWSAOVERLAPPED)&(m_stClientInfo.m_stOverlappedEx[nIdx]), NULL);
	
	// socket_error이면 client socket이 끊어진걸로 처리
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		 m_pMainDlg->OutputMsg("[에러] WSARecv()함수 실패: %d",WSAGetLastError());
		return false;
	}
	return true;

}
bool cOverlappedEvent::SendMsg(int nIdx, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// 전송될 메세지를 복사
	CopyMemory(m_stClientInfo.m_stOverlappedEx[nIdx].m_szBuf, pMsg, nLen);

	// Overlapped I/O를 위해 각 정보를 셋팅
	m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaOverlapped.hEvent = m_stClientInfo.m_eventHandle[nIdx];
	m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaBuf.len = MAX_SOCKBUF;
	m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaBuf.buf = m_stClientInfo.m_stOverlappedEx[nIdx].m_szBuf;
	m_stClientInfo.m_stOverlappedEx[nIdx].m_nIdx = nIdx;
	m_stClientInfo.m_stOverlappedEx[nIdx].m_eOperation = OP_SEND;

	int nRet = WSARecv(m_stClientInfo.m_socketClient[nIdx], &(m_stClientInfo.m_stOverlappedEx[nIdx].m_wsaBuf), 1, &dwRecvNumBytes,0,
		(LPWSAOVERLAPPED)&(m_stClientInfo.m_stOverlappedEx[nIdx]), NULL);

	// socket_error이면 client socket이 끊어진걸로 처리
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[에러] WSASend()함수 실패: %d",WSAGetLastError());
		return false;
	}
	return true;

}

// 사용자 접속 받는 쓰레드
void cOverlappedEvent::AccepterThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun) {
		// 접속 받을 구조체 인덱스 얻어옴
		int nIdx = GetEmptyIndex();
		if (-1 == nIdx) {
			m_pMainDlg->OutputMsg("[에러] client Full");
			return;
		}

		// 클라 접속 요청 대기
		m_stClientInfo.m_socketClient[nIdx] = accept(m_stClientInfo.m_socketClient[0], (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == m_stClientInfo.m_socketClient[nIdx])
			return;
		bool bRet = BindRecv(nIdx);
		if (false == bRet) return;
		m_pMainDlg->OutputMsg("클라이언트 접속: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr),m_stClientInfo.m_socketClient[nIdx]);

		// 클라 갯수 증가
		m_nClientCnt++;
		// 클라이언트가 접속 되었으므로 WorkerThread로 정보 갱신
		WSASetEvent(m_stClientInfo.m_eventHandle[0]);
	}
}

void cOverlappedEvent::WokerThread()
{
	while (m_bWorkerRun) {
		// 요청 Overlapped I/O  작업이 완료 됬는지 이벤트를 기다림
		DWORD dwObjIdx = WSAWaitForMultipleEvents(WSA_MAXIMUM_WAIT_EVENTS, m_stClientInfo.m_eventHandle,FALSE, INFINITE, FALSE);
		// 에러 발생
		if (WSA_WAIT_FAILED == dwObjIdx) {
			m_pMainDlg->OutputMsg("[에러] WSAWaitFOrMultiEvents 실패: %d", WSAGetLastError());
			break;
		}
		// 이벤트 리셋
		WSAResetEvent(m_stClientInfo.m_eventHandle[dwObjIdx]);
		// 접속 들어옴
		if (WSA_WAIT_EVENT_0 == dwObjIdx)
			continue;
		
		// Overlapped I/O 결과 처리
		OverlappedResult(dwObjIdx);
	}


}
void cOverlappedEvent::OverlappedResult(int nIdx)
{
	DWORD dwTransfer = 0;
	DWORD dwFlags = 0;
	BOOL bRet = WSAGetOverlappedResult(m_stClientInfo.m_socketClient[nIdx], (LPWSAOVERLAPPED)&m_stClientInfo.m_stOverlappedEx[nIdx],
		&dwTransfer,
		FALSE,
		&dwFlags);
	if (TRUE == bRet && 0 == dwTransfer) {
		m_pMainDlg->OutputMsg("[에러] WSAGetOverlappedResult 실패: %d", WSAGetLastError());
		return;
	}

	// 접속 끊김
	if (0 == dwTransfer) {
		m_pMainDlg->OutputMsg("[접속 끊김] socket: %d", m_stClientInfo.m_socketClient[nIdx]);
		CloseSocket(m_stClientInfo.m_socketClient[nIdx]);
		m_nClientCnt--;
		return;
	}
	
	stOverlappedEx* pOverlappedEx = &m_stClientInfo.m_stOverlappedEx[nIdx];
	switch(pOverlappedEx->m_eOperation)
	{
		// WSARecv로 완료
	case OP_RECV:
	{
		m_pMainDlg->OutputMsg("[수신] bytes : %d, msg: %s", dwTransfer, pOverlappedEx->m_szBuf);
		pOverlappedEx->m_szBuf[dwTransfer] = NULL;

		// 메시지 에코
		SendMsg(nIdx, pOverlappedEx->m_szBuf, dwTransfer);
	}
	case OP_SEND:
	{
		m_pMainDlg->OutputMsg("[송신] bytes : %d, msg: %s", dwTransfer, pOverlappedEx->m_szBuf);
		pOverlappedEx->m_szBuf[dwTransfer] = NULL;

		// 다시 Recv Overlapped I/O를 걸어줌
		BindRecv(nIdx);
	}
	break;
	default:
	{
		m_pMainDlg->OutputMsg("정의 되지 않은 Operation");
	}
	break;


	}
}

void cOverlappedEvent::DestroyThread()
{
	closesocket(m_stClientInfo.m_socketClient[0]);
	SetEvent(m_stClientInfo.m_eventHandle[0]);
	m_bWorkerRun = false;
	m_bAccepterRun = false;
	// 쓰레드 종료 기다림
	WaitForSingleObject(m_hWorkerThread, INFINITE);
	WaitForSingleObject(m_hAccepterThread, INFINITE);
}
	// COverlappedEventApp initialization

BOOL COverlappedEventApp::InitInstance()
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

	COverlappedEventDlg dlg;
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


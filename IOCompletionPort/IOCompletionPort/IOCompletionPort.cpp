
// IOCompletionPort.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IOCompletionPort.h"
#include "IOCompletionPortDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIOCompletionPortApp

BEGIN_MESSAGE_MAP(CIOCompletionPortApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CIOCompletionPortApp construction

CIOCompletionPortApp::CIOCompletionPortApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CIOCompletionPortApp object

CIOCompletionPortApp theApp;

// WSARecv와 WSASend의 Overlapped I/O 작업 처리를 위한 쓰레드
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	cIOCompletionPort* pOverlappedEvent = (cIOCompletionPort*)p;
	pOverlappedEvent->WokerThread();
	return 0;
}
// Client의 접속을 받기 위한 쓰레드
unsigned int WINAPI CallAccepterThread(LPVOID p)
{
	cIOCompletionPort* pOverlappedEvent = (cIOCompletionPort*)p;
	pOverlappedEvent->AccepterThread();
	return 0;
}
cIOCompletionPort::cIOCompletionPort(void)
{
	// 모든 멤버 변수들 초기화
	m_pMainDlg = NULL;
	m_bWorkerRun = true;
	m_bAccepterRun = true;
	m_nClientCnt = 0;
	m_hAccepterThread = NULL;
	m_hIOCP = NULL;
	m_socketListen = INVALID_SOCKET;
	ZeroMemory(m_szSocketBuf, 1024);
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
		m_hWorkerThread[i] = NULL;

	// client 정보를 저장한 구조체를 생성
	m_pClientInfo = new stClientInfo[MAX_CLIENT];

}
cIOCompletionPort::~cIOCompletionPort(void)
{
	// 윈속 사용을 끝낸다
	WSACleanup();
	// 사용한 객체를 삭제
	if (m_pClientInfo) {
		delete[] m_pClientInfo;
		m_pClientInfo = NULL;
	}
}
// 소켓 초기화 함수
bool cIOCompletionPort::InitSocket()
{
	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러] WSAStartup()함수 실패: %d", WSAGetLastError());
		return false;
	}
	// TCP Overlapped I/O 소켓 생성
	m_socketListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_socketListen) {
		m_pMainDlg->OutputMsg("[에러] socket()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 윈도우 핸들 저장
	m_pMainDlg->OutputMsg("소켓 초기화 성공");
	return true;
}
// 해당 소켓 종료
void cIOCompletionPort::CloseSocket(stClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER로 설정
									  // bIsForce가 true이면 SO_LINGER, timeout=0으로 설정하여 강제 종료
	if (true == bIsForce)
		stLinger.l_onoff = 1;

	// 데이터 송수신 중단
	shutdown(pClientInfo->m_socketClient, SD_BOTH);
	// 소켓 옵션 설정
	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// 소켓 종료
	closesocket(pClientInfo->m_socketClient);

	pClientInfo->m_socketClient = INVALID_SOCKET;
}
// 서버 주소정보를 소켓과 연결, 소켓 등록
bool cIOCompletionPort::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	// 서버포트 설정
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_socketListen, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러 bind()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 접속 대기 큐 5
	nRet = listen(m_socketListen, 5);
	if (0 != nRet) {
		m_pMainDlg->OutputMsg("[에러] listen()함수 실패: %d", WSAGetLastError());
		return false;
	}
	m_pMainDlg->OutputMsg("서버 등록 성공..");
	return true;




}
bool cIOCompletionPort::CreateWokerThread()
{
	unsigned int uiThreadId = 0;

	// WaitingThread Queue에 대기 상태로 넣을 쓰레드들 생성
	// 권장되는 개수: (cpu개수 *2)+1
	for (int i = 0; i < MAX_WORKERTHREAD; i++) {
		m_hWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId);
		if (m_hWorkerThread == NULL) {
			m_pMainDlg->OutputMsg("WorkerThread Creation Failed: %u", GetLastError());
			return false;
		}
		ResumeThread(m_hWorkerThread[i]);
	}
	m_pMainDlg->OutputMsg("WorkerThread시작..");
	return true;
}
bool cIOCompletionPort::CreateAccepterThread()
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
bool cIOCompletionPort::BindIOCompletionPort(stClientInfo* pClientInfo)
{
	HANDLE hIOCP;
	// socket과 pClientInfo를 COmpletionPort객체와 연결시킨다.
	hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, m_hIOCP, reinterpret_cast<DWORD>(pClientInfo), 0);
	if (NULL == hIOCP || m_hIOCP != hIOCP) {
		m_pMainDlg->OutputMsg("[에러] CreateIoCompletionPort() 함수 실패", GetLastError());
		return false;
	}
	return true;
}
bool cIOCompletionPort::StartServer()
{

	bool bRet = CreateWokerThread();
	if (false == bRet)return false;
	bRet = CreateAccepterThread();
	if (false == bRet)return false;

	// CompletionPort객체 생성 요청
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == m_hIOCP) {
		m_pMainDlg->OutputMsg("[에러] CreateIoCompletionPort() 함수 실패", GetLastError());
		return false;
	}
	m_pMainDlg->OutputMsg("서버 시작");
	return false;
}
bool cIOCompletionPort::BindRecv(stClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// Overlapped I/O를 위해 각 정보를 셋팅
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stRecvOverlappedEx.m_szBuf;
	pClientInfo->m_stRecvOverlappedEx.m_eOperation = OP_RECV;

	int nRet = WSARecv(pClientInfo->m_socketClient, &(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED)&(pClientInfo->m_stRecvOverlappedEx), NULL);

	// socket_error이면 클라이언트가 끊어진걸로 처리
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[에러] WSARecv()함수 실패: %d", WSAGetLastError());
		return false;
	}
	return true;
}
bool cIOCompletionPort::SendMsg(stClientInfo* pClientInfo, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;
	
	// 전송될 메시지 복사
	CopyMemory(pClientInfo->m_stSendOverlappedEx.m_szBuf, pMsg, nLen);

	// Overlapped I/O를 위해 각 정보를 셋팅
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = nLen;
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stSendOverlappedEx.m_szBuf;
	pClientInfo->m_stRecvOverlappedEx.m_eOperation = OP_SEND;

	int nRet = WSASend(pClientInfo->m_socketClient, &(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes,0, (LPWSAOVERLAPPED)&(pClientInfo->m_stRecvOverlappedEx), NULL);

	// socket_error이면 클라이언트가 끊어진걸로 처리
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		m_pMainDlg->OutputMsg("[에러] WSARecv()함수 실패: %d", WSAGetLastError());
		return false;
	}
	return true;

}
stClientInfo* cIOCompletionPort::GetEmptyClientInfo()
{
	for (int i = 0; i < MAX_CLIENT; i++) {
		if (INVALID_SOCKET == m_pClientInfo[i].m_socketClient)return &m_pClientInfo[i];
	}
	return NULL;
}
void cIOCompletionPort::AccepterThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun) {

		// 접속 받을 구조체의 인덱스를 얻어옴
		stClientInfo* pClientInfo = GetEmptyClientInfo();
		if (NULL == pClientInfo) {m_pMainDlg->OutputMsg("[에러] Client Full"); return;}
		// 클라 접속 요청 대기
		pClientInfo->m_socketClient = accept(m_socketListen, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == pClientInfo->m_socketClient) continue;

		// I/O Completion Port 객체와 소켓 연결
		bool bRet = BindIOCompletionPort(pClientInfo);
		if (false == bRet) return;
		// Recv Overlapped I/O 작업을 요청해 놓는다
		bRet = BindRecv(pClientInfo);
		if (false == bRet) return;
		m_pMainDlg->OutputMsg("클라이언트 접속: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), pClientInfo->m_socketClient);
		// 클라이언트 갯수 증가
		m_nClientCnt++;
	}

}
void cIOCompletionPort::WokerThread()
{
	// CompletionKey를 받을 포인터 변수
	stClientInfo* pClientInfo = NULL;
	// 함수 호출 성공 여부
	BOOL bSuccess = TRUE;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD dwIoSize = 0;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
	LPOVERLAPPED lpOverlapped = NULL;

	while (m_bWorkerRun) {
		// 이 함수로 인해 쓰레드들은 WaitingThread Queue에 대기 상태로 들어감
		// 완료된 Overlapped I/O 작업이 발생하면 ICPI Queue에서 완료된 작업을 가져와 뒤 처리를 함
		// 그리고 PostQueuedCompletionStatus() 함수에 의해 사용자 메세지가 도착되면 쓰레드 종료
		bSuccess = GetQueuedCompletionStatus(m_hIOCP,
			&dwIoSize, // 실제 전송된 바이트
			(LPDWORD)&pClientInfo, // CompletionKey
			&lpOverlapped, // Overlapped IO 객체
			INFINITE); // 대기할 시간
		if (FALSE == bSuccess && 64 != GetLastError()) {
			m_pMainDlg->OutputMsg("[에러] GetQueuedCompletionStatus()함수 실패: %d", WSAGetLastError());
		}
		// client가 접속을 끊었을때
		if (FALSE == bSuccess && 0 == dwIoSize) {
			m_pMainDlg->OutputMsg("socket(%d) 접속 끊김", pClientInfo->m_socketClient);
			CloseSocket(pClientInfo);
			continue;
		}

		// 사용자 쓰레드 종료 메세지 처리
		if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
			m_bWorkerRun = false;
			continue;
		}
		if (NULL == lpOverlapped)continue;

		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;
		// Overlapped I/O Recv 작업 결과 뒤 처리
		if (OP_RECV == pOverlappedEx->m_eOperation) {
			pOverlappedEx->m_szBuf[dwIoSize] = NULL;
			m_pMainDlg->OutputMsg("[수신] bytes: %d, msg: %s", dwIoSize, pOverlappedEx->m_szBuf);
			// 메세지 에코
			BindRecv(pClientInfo);
		}
		else if (OP_SEND == pOverlappedEx->m_eOperation) {
			m_pMainDlg->OutputMsg("[송신] bytes: %d, msg: %s", dwIoSize, pOverlappedEx->m_szBuf);
		}
		// 예외
		else {
			m_pMainDlg->OutputMsg("socket(%d)에서의 예외상황", pClientInfo->m_socketClient);
		}
	}
}
void cIOCompletionPort::DestroyThread()
{
	for (int i = 0; i < MAX_WORKERTHREAD; i++) {
		// WaitingThread Queue에서 대기중인 쓰레드에 사용자 종료 메세지를 보냄
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
	}
	for (int i = 0; i < MAX_WORKERTHREAD; i++) {
		// 쓰레드 핸들을 닫고 쓰레드가 종료될 때까지 대기
		CloseHandle(m_hWorkerThread[i]);
		WaitForSingleObject(m_hWorkerThread[i], INFINITE);
	}
	m_bAccepterRun = false;
	// Accepter 쓰레드를 종료
	closesocket(m_socketListen);
	// 쓰레드 종료 대기
	WaitForSingleObject(m_hAccepterThread, INFINITE);


}

// CIOCompletionPortApp initialization

BOOL CIOCompletionPortApp::InitInstance()
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

	CIOCompletionPortDlg dlg;
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


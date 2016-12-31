
// IOCompletionPort.h : main header file for the PROJECT_NAME application
//

#pragma once
class CIOCompletionPortDlg;
#define MAX_SOCKBUF 1024
#define MAX_CLIENT 100
#define MAX_WORKERTHREAD 4 // 쓰레드 폴에 넣을 쓰레드 수
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIOCompletionPortApp:
// See IOCompletionPort.cpp for the implementation of this class
//
enum enumOperation { OP_RECV, OP_SEND };
// WSAOVERLAPPED 구조체 확장
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped; // Overlapped I/O 구조체
	SOCKET m_socketClient; // 클라 소켓
	WSABUF m_wsaBuf; // Overlapped I/O 작업 버퍼
	char m_szBuf[MAX_SOCKBUF]; // 데이터 버퍼
	enumOperation m_eOperation; // 작업 동작 종류
};

// 클라 정보 구조체
struct stClientInfo {
	SOCKET m_socketClient; // Client와 연결되는 소켓
	stOverlappedEx m_stRecvOverlappedEx; // RECV Overlapped I/O 작업을 위한 변수
	stOverlappedEx m_stSendOverlappedEx; // SEND Overlapped I/O 작업을 위한 변수
	// 생성자에서 멤버 변수들을 초기화
	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
		m_socketClient = INVALID_SOCKET;
	}
};

class cIOCompletionPort
{
public:
	cIOCompletionPort(void);
	~cIOCompletionPort(void);

	// -- 서버 클라 공통 함수 -- //
	bool InitSocket();
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false);

	// -- 서버용 함수 -- //
	bool BindandListen(int nBindPort);
	bool StartServer();
	// WaitingThread Queue에서 대기할 쓰레드들을 생성
	bool CreateWokerThread();
	// accept처리 쓰레드 생성
	bool CreateAccepterThread();
	// 사용하지 않는 클라 정보 구조체 반환
	stClientInfo* GetEmptyClientInfo();

	// CompletionPort 객체와 소켓과 CompletionKey를 연결
	bool BindIOCompletionPort(stClientInfo* pClientInfo);
	// WSARecv Overlapped I/O 작업을 시킨다
	bool BindRecv(stClientInfo* pClientInfo);
	// WSASend Overlapped I/O 작업을 시킨다
	bool SendMsg(stClientInfo* pClientInfo, char* pMsg, int nLen);
	// I/O 작업 완료 통보를 받아 처리하는 함수
	void WokerThread();
	// 접속 받는 쓰레드
	void AccepterThread();

	// 메인 윈도우의 포인터를 저장
	void SetMainDlg(CIOCompletionPortDlg* pMainDlg) { m_pMainDlg = pMainDlg; }
	CIOCompletionPortDlg* GetMainDlg() { return m_pMainDlg; }

	// 생성된 쓰레드를 파괴
	void DestroyThread();
private:
	// 클라 정보 저장 구조체
	stClientInfo* m_pClientInfo;
	// 리슨 소켓
	SOCKET m_socketListen;
	// 접속된 클라 수
	int m_nClientCnt;
	// 메인 윈도우 포인터
	CIOCompletionPortDlg* m_pMainDlg;
	// 작업쓰레드 핸들 -- WaitingThread Queue에 들어갈 쓰레드
	HANDLE m_hWorkerThread[MAX_WORKERTHREAD];
	// 접속 쓰레드 핸들
	HANDLE m_hAccepterThread;
	// CompletionPort 객체 핸들
	HANDLE m_hIOCP;
	// 작업 쓰레드 동작 플래그
	bool m_bWorkerRun;
	// 접속 쓰레드 동작 플래그
	bool m_bAccepterRun;
	// 소켓 버퍼
	char m_szSocketBuf[1024];
};
class CIOCompletionPortApp : public CWinApp
{
public:
	CIOCompletionPortApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIOCompletionPortApp theApp;
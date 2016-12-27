
// OverlappedEvent.h : main header file for the PROJECT_NAME application
//

#pragma once
class COverlappedEventDlg;
#define MAX_SOCKBUF 1024
// Overlapped I/O작업 동작 종류
enum enumOperation { OP_RECV, OP_SEND };
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// WSAOVERLAPPED구조체를 확장 시켜 필요 정보를 넣음
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped; // IO 구조체
	int m_nIdx; //stClientInfo 구조체 배열 인덱스
	WSABUF m_wsaBuf; // OVerlapped I/O 작업 버퍼
	char m_szBuf[MAX_SOCKBUF]; // 데이터 버퍼
	enumOperation m_eOperation; // Overlapped IO 작업 동작 종류
};

// 클라이언트 정보를 담기위한 구조체지만 0번째 배열에는 더미로 접속이 들어오면 새로운 접속에 대한
// 이벤트 감지를 위해 WSAWaitForMultipleEvents를 다시 걸어준다.
struct stClientInfo {
	SOCKET m_socketClient[WSA_MAXIMUM_WAIT_EVENTS]; // Client와 연결되는 소켓
	WSAEVENT m_eventHandle[WSA_MAXIMUM_WAIT_EVENTS]; // 이벤트 감지를 위한 이벤트 객체
	stOverlappedEx m_stOverlappedEx[WSA_MAXIMUM_WAIT_EVENTS];
};

class cOverlappedEvent
{
public:
	cOverlappedEvent(void);
	~cOverlappedEvent(void);

	// ----서버 클라 공통 함수 ---- //
	// 소켓 초기화 함수
	bool InitSocket();
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);
	// ---- 서버용 함수 ---- //
	bool BindandListen(int nBindPort);
	bool StartServer();

	// Overlapeed I/O 작업을 위한 쓰레드를 생성
	bool CreateWokerThread();
	// acceppt 요청 처리 쓰레드
	bool CreateAccepterThread();
	// 사용 되지 않은 index 반환
	int GetEmptyIndex();
	// WSARecv Overlapped I/O 작업을 시킴
	bool BindRecv(int nIdx);
	// WSASend Overlapped I/O 작업을 시킴
	bool SendMsg(int nIdx, char* pMsg, int nLen);

	// Overlapped I/O작업에 대한 완료 통보를 받아 처리
	void WokerThread();
	// 접속 수락 
	void AccepterThread();
	// I/O 완료에 따른 결과 처리
	void OverlappedResult(int nIdx);

	void SetMainDlg(COverlappedEventDlg* pMainDlg) { m_pMainDlg = pMainDlg; }

	// 생성 된 쓰레드 파괴
	void DestroyThread();

private:
	// 클라 정보 저장 구조체
	stClientInfo m_stClientInfo;

	// 접속 된 클라 수
	int m_nClientCnt;
	// 메인 윈도우 포인터
	COverlappedEventDlg* m_pMainDlg;
	// 작업 쓰레드 핸들
	HANDLE m_hWorkerThread;
	// 접속 쓰레드 핸들
	HANDLE m_hAcceptThread;
	// 작업 쓰레드 동작 플래그
	bool m_bWorkerRun;
	// 접속 쓰레드 동작 플래그
	bool m_bAccepterRun;
	// 소켓 버퍼
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
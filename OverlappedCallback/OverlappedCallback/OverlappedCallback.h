
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

// Overlapped I/O 작업 동작 종류
enum enumOperation { OP_RECV, OP_SEND };
// WSAOVERLAPPED 구조체 확장
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped; // I/O 구조체
	SOCKET m_socketClient;
	WSABUF m_wsaBuf; // I/O 작업 버퍼
	char m_szBuf[MAX_SOCKBUF]; // 데이터 버퍼
	enumOperation m_eOperation; // 작업 동작 종류
	void* m_pOverlappedCallback; // 콜백 함수 포인터
};

class cOverlappedCallback
{
public:
	cOverlappedCallback(void);
	~cOverlappedCallback(void);
	// -- 서버 클라 공통 함수 -- //
	// 소켓 초기화 함수
	bool InitSocket();
	// 연결 종료
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);

	// -- 서버용 함수 -- //
	bool BindandListen(int nBindPort);
	// 접속 요청 수락 및 메세지 처리
	bool StartServer();
	// accept 요청 처리 쓰레드 생성
	bool CreateAccepterThread();
	// WSARecv Overlapped I/O 작업
	bool BindRecv(SOCKET socket);
	// WSASend Overlapped I/O 작업
	bool SendMsg(SOCKET socket, char* pMsg, int nLen);
	// 사용자 접속 받는 스레드
	void AccepterThread();

	void SetMainDlg(COverlappedCallbackDlg* pMainDlg) { m_pMainDlg = pMainDlg; }
	COverlappedCallbackDlg* GetMainDlg() { return m_pMainDlg; }

	// 생성된 쓰레드 파괴
	void DestoryThread();

private:
	// 접속된 클라 수
	int m_nClientCnt;
	// 메인 윈도우 포인터
	COverlappedCallbackDlg* m_pMainDlg;
	// 접속 쓰레드 핸들
	HANDLE m_hAccepterThread;
	// 접속 쓰레드 동작 플래그
	bool m_bAccepterRun;
	SOCKET m_sockListen;
	// 소켓 버퍼
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
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

	/* 서버 클라 공통함수 */
	// 소켓 초기화 함수
	bool InitSocket(HWND hWnd);
	// 소켓 연결을 종료
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);

	/* 서버용 함수 */
	// 서버의 주소정보를 소켓과 연결, 접속 요청을 받기 위해 소켓을 등록
	bool BindandListen(int nBindPort);
	// 접속 요청을 수락하고 메세지를 받아서 처리하는 함수
	bool StartServer();
	void setMainDlg(CAsyncSelectDlg* pMainDlg) { m_pMainDlg = pMainDlg; }
private:
	// 클라이언트 접속요청 대기 소켓
	SOCKET m_socketListen;
	// 네트워크 이벤트가 발생시 메세지를 보낼 윈도우 핸들
	HWND m_hWnd;
	// 메인 윈도우 포인터
	CAsyncSelectDlg* m_pMainDlg;
	// 소켓 버퍼
	char m_szSocketBuf[1024];
};


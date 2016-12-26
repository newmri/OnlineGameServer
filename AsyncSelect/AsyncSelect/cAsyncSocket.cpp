#include "cAsyncSocket.h"
cAsyncSocket::cAsyncSocket(void)
{
	m_socketListen = INVALID_SOCKET;
	m_hWnd = NULL;
	m_pMainDlg = NULL;
	ZeroMemory(m_szSocketBuf, 1024);
}

cAsyncSocket::~cAsyncSocket(void)
{
	// 윈속 사용 종료
	WSACleanup();
}

// 소켓 초기화 함수
bool cAsyncSocket::InitSocket(HWND hWnd)
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
	//	m_pMainDlg->OutputMsg("[에러] WSAStartup()함수 실패: %d", WSAGetLastError());
		return false;
	}
	// 연결 지향형 TCP 소켓 생성
	m_socketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_socketListen) {
	//	m_pMainDlg->OutputMsg("[에러] socket()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 윈도우 핸들 저장
	m_hWnd = hWnd;
	//m_pMainDlg->OutputMsg("소켓 초기화 성공");
	return true;

}

// 해당 소켓 종료
void cAsyncSocket::CloseSocket(SOCKET socketClose, bool bIsForce)
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

// 서버의 주소 정보를 연결, 접속 요청 대기 함수
bool cAsyncSocket::BindandListen(int nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	// 서버포트 설정
	stServerAddr.sin_port = htons(nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_socketListen, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		//m_pMainDlg->OutputMsg("[에러 bind()함수 실패: %d", WSAGetLastError());
		return false;
	}

	// 접속 대기 큐 5
	nRet = listen(m_socketListen, 5);
	if (0 != nRet) {
		//m_pMainDlg->OutputMsg("[에러] listen()함수 실패: %d", WSAGetLastError());
		return false;
	}
	//m_pMainDlg->OutputMsg("서버 등록 성공..");
	return true;
}

bool cAsyncSocket::StartServer()
{
	// 이벤트 등록
	int nRet = WSAAsyncSelect(m_socketListen, m_hWnd, WM_SOCKETMSG, FD_ACCEPT | FD_CLOSE);
	if (SOCKET_ERROR == nRet) {
		//m_pMainDlg->OutputMsg("[에러] WSAAsyncSelect() 함수 실패: %d", WSAGetLastError());
		return false;
	}
	return true;


}
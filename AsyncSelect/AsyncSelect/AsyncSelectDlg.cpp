
// AsyncSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AsyncSelect.h"
#include "AsyncSelectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAsyncSelectDlg dialog



CAsyncSelectDlg::CAsyncSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ASYNCSELECT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAsyncSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctOutput);
}

BEGIN_MESSAGE_MAP(CAsyncSelectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKETMSG, OnSocketMsg)
	// AFX_MSG_MAP
	ON_BN_CLICKED(ID_STARTSERVER,OnBnClickedStartserver)
END_MESSAGE_MAP()

LRESULT CAsyncSelectDlg::OnSocketMsg(WPARAM wParam, LPARAM lParam)
{
	SOCKET sock = (SOCKET)wParam;
	// ���� üũ
	int nError = WSAGETSELECTERROR(lParam);
	if (0 != nError) {
		OutputMsg("[����] WSAGETSELECTERROR: %d", nError);
		m_AsyncSocket.CloseSocket(sock);
		return false;
	}
	// �̺�Ʈ üũ
	int nEvent = WSAGETSELECTEVENT(lParam);
	switch (nEvent) {
	case FD_READ:
	{
		int nRecvLen = recv(sock, m_szSocketBuf, MAX_SOCKBUF, 0);
		if (0 == nRecvLen) {
			OutputMsg("Ŭ���̾�Ʈ�� ������ ���� �Ǿ����ϴ�.");
			m_AsyncSocket.CloseSocket(sock);
			return false;
		}
		else if (-1 == nRecvLen) {
			OutputMsg("[����] recv ���� : %d", WSAGetLastError());
			m_AsyncSocket.CloseSocket(sock);
		}
		m_szSocketBuf[nRecvLen] = NULL;
		OutputMsg("socket[%d], �޼��� ����: %d bytes, ����: %s", sock, nRecvLen, m_szSocketBuf);
		int nSendLen = send(sock, m_szSocketBuf, nRecvLen, 0);
		if (-1 == nSendLen) {
			OutputMsg("[����] send ����: %d", WSAGetLastError());
			m_AsyncSocket.CloseSocket(sock);
			return false;
		}
		OutputMsg("socket[%d], �޽��� �۽�: %d bytes, ����: %s", sock, nSendLen, m_szSocketBuf);
		break;
	}
	case FD_ACCEPT:
	{
		// ���ӵ� Ŭ���̾�Ʈ �ּ� ������ ������ ����ü
		SOCKADDR_IN stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);
		SOCKET sockClient = accept(sock, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == sockClient) {
			OutputMsg("[����] accept()�Լ� ����: %d", WSAGetLastError());
			return false;
		}
		// ���ϰ� ��Ʈ��ũ �̺�Ʈ�� ����Ѵ�
		int nRet = WSAAsyncSelect(sockClient, m_hWnd, WM_SOCKETMSG, FD_READ | FD_CLOSE);
		if (SOCKET_ERROR == nRet) {
			OutputMsg("[����] WSAAsuncSelect()�Լ� ����: %d", WSAGetLastError());
			return false;
		}
		OutputMsg("Ŭ���̾�Ʈ ����: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), sockClient);
	}
	break;
	case FD_CLOSE:
	{
		OutputMsg("Ŭ���̾�Ʈ ���� ���� : SOCKET(%d)", sock);
		m_AsyncSocket.CloseSocket(sock);
	}
	break;
	}
	return true;


}

// ��� �޼���
void CAsyncSelectDlg::OutputMsg(char* szOutputString, ...)
{
	char szOutStr[1024];
	va_list argptr;
	va_start(argptr, szOutputString);
	vsprintf(szOutStr, szOutputString, argptr);
	va_end(argptr);
	m_ctOutput.SetCurSel(m_ctOutput.AddString(szOutStr));


}
void CAsyncSelectDlg::OnBnClickedStartserver()
{
	// ���� �޼����� �����쿡 �Ѹ��� ���� ���� ���̾�α� �����͸� �Ѱ��ش�
	m_AsyncSocket.setMainDlg(this);
	// ������ �ʱ�ȭ
	m_AsyncSocket.InitSocket(m_hWnd);
	// ���ϰ� �ּҸ� ����, ���
	m_AsyncSocket.BindandListen(8000);
	// ��Ʈ��ũ �̺�Ʈ�� �����츦 ��� ��Ű�� ���� ����
	m_AsyncSocket.StartServer();



}
// CAsyncSelectDlg message handlers

BOOL CAsyncSelectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAsyncSelectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAsyncSelectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAsyncSelectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


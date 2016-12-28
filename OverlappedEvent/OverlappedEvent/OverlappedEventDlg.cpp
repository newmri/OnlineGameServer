
// OverlappedEventDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OverlappedEvent.h"
#include "OverlappedEventDlg.h"
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


// COverlappedEventDlg dialog



COverlappedEventDlg::COverlappedEventDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_OVERLAPPEDEVENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COverlappedEventDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctOutput);
}

BEGIN_MESSAGE_MAP(COverlappedEventDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_MESSAGE(WM_SOCKETMSG, OnSocketMsg)
	// AFX_MSG_MAP
	ON_BN_CLICKED(ID_STARTSERVER, OnBnClickedStartserver)
END_MESSAGE_MAP()

// 출력 메세지
void COverlappedEventDlg::OutputMsg(char* szOutputString, ...)
{
	char szOutStr[1024];
	va_list argptr;
	va_start(argptr, szOutputString);
	vsprintf(szOutStr, szOutputString, argptr);
	va_end(argptr);
	m_ctOutput.SetCurSel(m_ctOutput.AddString(szOutStr));


}
void COverlappedEventDlg::OnBnClickedStartserver()
{
	// 소켓 메세지를 윈도우에 뿌리기 위해 현재 다이얼로그 포인터를 넘겨준다
	m_OverlappedEvent.SetMainDlg(this);
	// 소켓을 초기화
	m_OverlappedEvent.InitSocket();
	// 소켓과 주소를 연결, 등록
	m_OverlappedEvent.BindandListen(8000);
	//  서버 시작
	m_OverlappedEvent.StartServer();



}
// COverlappedEventDlg message handlers

BOOL COverlappedEventDlg::OnInitDialog()
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

void COverlappedEventDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void COverlappedEventDlg::OnPaint()
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
HCURSOR COverlappedEventDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


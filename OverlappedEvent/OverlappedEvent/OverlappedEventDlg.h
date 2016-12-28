
// OverlappedEventDlg.h : header file
//

#pragma once
#include "afxwin.h"


// COverlappedEventDlg dialog
class COverlappedEventDlg : public CDialogEx
{
// Construction
public:
	COverlappedEventDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OVERLAPPEDEVENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// 이벤트가 발생하여 WM_SOCKETMSG를 받으면 이 함수가 실행.
	// WParam이 이벤트가 발생한 소켓이고 lParam은 이벤트 종류와 에러코드 보유
	//LRESULT OnSocketMsg(WPARAM wParam, LPARAM lParam);
public:
	// 출력 메세지
	void OutputMsg(char* szOutputString, ...);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 메시지를 출력하기 위해 만든 리스트 박스 컨트롤
	CListBox m_ctOutput;
	// 소켓 버퍼
	char m_szSocketBuf[1024];
	// WSAAsuncSelect모델을 적용 시킨 클래스 변수
	cOverlappedEvent m_OverlappedEvent;
	afx_msg void OnBnClickedStartserver();
};

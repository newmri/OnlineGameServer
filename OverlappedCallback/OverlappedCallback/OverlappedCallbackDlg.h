
// OverlappedCallbackDlg.h : header file
//

#pragma once
#include "afxwin.h"


// COverlappedCallbackDlg dialog
class COverlappedCallbackDlg : public CDialogEx
{
// Construction
public:
	COverlappedCallbackDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OVERLAPPEDCALLBACK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	// ��� �޼���
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
	// ���� ����
	char m_szSocketBuf[1024];
	// WSAAsuncEvent���� ���� ��Ų Ŭ���� ����
	cOverlappedCallback m_OverlappedCallback;
	afx_msg void OnBnClickedStartserver();
	CListBox m_ctOutput;
};

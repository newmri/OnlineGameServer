
// IOCompletionPortDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CIOCompletionPortDlg dialog
class CIOCompletionPortDlg : public CDialogEx
{
// Construction
public:
	CIOCompletionPortDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IOCOMPLETIONPORT_DIALOG };
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

	// WSAAsuncEvent���� ���� ��Ų Ŭ���� ����
	cIOCompletionPort m_IOCompletionPort;
	afx_msg void OnBnClickedStartserver();
	CListBox m_ctOutput;
};

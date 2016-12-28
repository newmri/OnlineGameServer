
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
	// �̺�Ʈ�� �߻��Ͽ� WM_SOCKETMSG�� ������ �� �Լ��� ����.
	// WParam�� �̺�Ʈ�� �߻��� �����̰� lParam�� �̺�Ʈ ������ �����ڵ� ����
	//LRESULT OnSocketMsg(WPARAM wParam, LPARAM lParam);
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
	// �޽����� ����ϱ� ���� ���� ����Ʈ �ڽ� ��Ʈ��
	CListBox m_ctOutput;
	// ���� ����
	char m_szSocketBuf[1024];
	// WSAAsuncSelect���� ���� ��Ų Ŭ���� ����
	cOverlappedEvent m_OverlappedEvent;
	afx_msg void OnBnClickedStartserver();
};

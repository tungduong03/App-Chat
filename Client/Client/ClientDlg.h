
// ClientDlg.h : header file
//

#pragma once


// CClientDlg dialog
class CClientDlg : public CDialogEx
{
// Construction
public:
	CClientDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void OnEnChangeMsg();
	DECLARE_MESSAGE_MAP()
public:
	CEdit Msg;
	CButton btn_send;
	CButton btn_create_channel;
	CEdit channel;
	CButton btn_login;
	CEdit name;
	CListBox list_msg;
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedButton3();
	static UINT ReceiveThread(LPVOID pParam);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};


// ClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"
#include "afxsock.h"
#include "string"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CAsyncSocket clientSocket; 

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


// CClientDlg dialog



CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSG, Msg);
	DDX_Control(pDX, IDC_SEND, btn_send);
	DDX_Control(pDX, IDC_BUTTON3, btn_create_channel);
	DDX_Control(pDX, IDC_Channel, channel);
	DDX_Control(pDX, IDC_LOGIN, btn_login);
	DDX_Control(pDX, IDC_NAME, name);
	DDX_Control(pDX, IDC_LIST1, list_msg);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOGIN, &CClientDlg::OnBnClickedLogin)
	ON_BN_CLICKED(IDC_SEND, &CClientDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_BUTTON3, &CClientDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	if (!AfxSocketInit())
	{
		AfxMessageBox(_T("Failed to initialize Winsock"));
		return FALSE;
	}

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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
	btn_send.EnableWindow(FALSE);
	btn_create_channel.EnableWindow(FALSE);
	channel.EnableWindow(FALSE);
	Msg.EnableWindow(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CClientDlg::ReceiveThread(LPVOID pParam)
{
	CClientDlg* pDlg = (CClientDlg*)pParam;
	char buffer[1024];
	int bytesReceived;

	while (true)
	{
		bytesReceived = clientSocket.Receive(buffer, sizeof(buffer));

		if (bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0';
			CString receivedData = CString(buffer);
			// Hiển thị tin nhắn trả về lên danh sách list_msg
			pDlg->list_msg.AddString(receivedData);
			//pDlg->UpdateData(true);
		}
	}

	return 0;
}

void CClientDlg::OnBnClickedLogin()
{
	// TODO: Add your control notification handler code here
	UpdateData(false);
	int create = clientSocket.Create();
	if (create == -1) {
		list_msg.AddString(_T("Failed to create!"));
	}
	int connect = clientSocket.Connect(_T("127.0.0.1"), 8000);
	if (connect == -1) {
		list_msg.AddString(_T("Failed to connect!"));
	}
	list_msg.AddString(_T("Connect success!"));

	// Tạo và khởi chạy luồng nhận tin nhắn từ máy chủ
	AfxBeginThread(ReceiveThread, this);

	CString name_client;
	name.GetWindowTextW(name_client);

	CString nick_name;
	nick_name = _T("/NICK ") + name_client + '\0';

	// Lấy con trỏ tới dữ liệu của chuỗi nick_name
	LPTSTR szNickName = nick_name.GetBuffer(0);

	// Sao chép chuỗi nick_name vào biến nick
	char nick[128]; // Đảm bảo nick đủ lớn để chứa chuỗi nick_name
	memset(nick, 0, sizeof(nick));
	strcpy_s(nick, CT2A(szNickName));
	
	clientSocket.Send(nick, strlen(nick), 0);
	nick_name.ReleaseBuffer();


	//khóa 
	name.EnableWindow(FALSE);
	btn_login.EnableWindow(FALSE);
	btn_send.EnableWindow(true);
	btn_create_channel.EnableWindow(true); 
	channel.EnableWindow(true);
	Msg.EnableWindow(true);

}


void CClientDlg::OnBnClickedSend()
{
	// TODO: Add your control notification handler code here
	UpdateData(false);
	CString message;
	Msg.GetWindowTextW(message);
	message = message + '\0';

	// Lấy con trỏ tới dữ liệu của chuỗi
	LPTSTR MSG = message.GetBuffer(0);
	// Sao chép chuỗi 
	char msg[256]; 
	memset(msg, 0, sizeof(msg));
	strcpy_s(msg, CT2A(MSG));
	list_msg.AddString(message); 

	char mess[128];
	strncpy_s(mess, msg, strlen(msg));
	// Chuyển đổi mảng ký tự sang chữ hoa
	for (int i = 0; mess[i] != '\0'; ++i) {
		mess[i] = toupper(mess[i]);
	}
	char* help = strstr(mess, "/HELP");
	if (help == mess) {
		list_msg.AddString(_T("/NICK <username> to change username"));
		list_msg.AddString(_T("/JOIN <channel> to join a channel"));
		list_msg.AddString(_T("/PART <channel> to part a channel"));
		list_msg.AddString(_T("/LIST to list all channels"));
		list_msg.AddString(_T("/PRIVMSG <username> <message> to send a private message"));
		list_msg.AddString(_T("/WHO <username> to get info a user"));
		list_msg.AddString(_T("/QUIT to quit"));
	}
	else {
		clientSocket.Send(msg, strlen(msg), 0); 
		char* quit = strstr(mess, "/QUIT");
		if (quit == mess) EndDialog(0);
	}
	
	
	Msg.SetWindowTextW(_T(""));
	UpdateData(true);
}


void CClientDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	UpdateData(false);
	CString text;
	channel.GetWindowTextW(text);
	CString channel_;
	channel_ = _T("/CHANNEL ") + text + '\0';
	
	// Lấy con trỏ tới dữ liệu của chuỗi nick_name
	LPTSTR szChannel = channel_.GetBuffer(0);
	
	// Sao chép chuỗi nick_name vào biến nick
	char channel__[128]; // Đảm bảo nick đủ lớn để chứa chuỗi nick_name
	memset(channel__, 0, sizeof(channel__));
	strcpy_s(channel__, CT2A(szChannel));
	list_msg.AddString(channel_);
	channel.SetWindowTextW(_T(""));
	clientSocket.Send(channel__, strlen(channel__), 0); 
}

// Csv2Bi5ConverterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Csv2Bi5Converter.h"
#include "Csv2Bi5ConverterDlg.h"
#include "afxdialogex.h"
#include "Config.h"

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


// CCsv2Bi5ConverterDlg dialog



CCsv2Bi5ConverterDlg::CCsv2Bi5ConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CSV2BI5CONVERTER_DIALOG, pParent)
	, m_strSourcePath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCsv2Bi5ConverterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnConvert);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	DDX_Control(pDX, IDC_EDIT1, m_editSourcePath);
	DDX_Control(pDX, IDC_EDIT2, m_editDestPath);
}

BEGIN_MESSAGE_MAP(CCsv2Bi5ConverterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCsv2Bi5ConverterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCsv2Bi5ConverterDlg::OnBnClickedCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CCsv2Bi5ConverterDlg message handlers

BOOL CCsv2Bi5ConverterDlg::OnInitDialog()
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
	m_ctrlProgress.SetRange(0, 100);
	Config config;
	::SetWindowTextA(m_editSourcePath.GetSafeHwnd(), config.GetSourcePath());
	::SetWindowTextA(m_editDestPath.GetSafeHwnd(), config.GetDestPath());

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCsv2Bi5ConverterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCsv2Bi5ConverterDlg::OnPaint()
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
HCURSOR CCsv2Bi5ConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCsv2Bi5ConverterDlg::OnBnClickedOk()
{
	if (m_converter.GetStatus() == Csv2Bi5Engine::Status::None || m_converter.GetStatus() == Csv2Bi5Engine::Status::Stopped)
	{
		Config config;
		char str[MAX_PATH] = "";
		::GetWindowTextA(m_editSourcePath.GetSafeHwnd(), str, MAX_PATH);
		if (strcmp(str, "") == 0)
		{
			AfxMessageBox(_T("Please input source path"));
			m_editSourcePath.SetFocus();
			return;
		}
		config.SetSourcePath(str);
		::GetWindowTextA(m_editDestPath.GetSafeHwnd(), str, MAX_PATH);
		if (strcmp(str, "") == 0)
		{
			AfxMessageBox(_T("Please input dest path"));
			m_editDestPath.SetFocus();
			return;
		}
		config.SetDestPath(str);
		config.SaveConfig();
		m_converter.StartConvert(config.GetSourcePath(), config.GetDestPath());
		SetTimer(WM_USER+1, 1000, NULL);
		m_btnConvert.EnableWindow(false);
	}
}


void CCsv2Bi5ConverterDlg::OnBnClickedCancel()
{
	if (m_converter.GetStatus() == Csv2Bi5Engine::Status::None || m_converter.GetStatus() == Csv2Bi5Engine::Status::Stopped)
	{
		CDialog::OnCancel();
	}
	else
	{
		if (AfxMessageBox(_T("Do you want to stop convert?"), MB_OKCANCEL) == IDOK)
		{
			m_converter.StopConvert();
		}
	}
}


void CCsv2Bi5ConverterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == (WM_USER + 1))
	{
		if (m_converter.GetStatus() == Csv2Bi5Engine::Status::None || m_converter.GetStatus() == Csv2Bi5Engine::Status::Stopped)
		{
			KillTimer(WM_USER + 1);
			AfxMessageBox(_T("Finished!"));

			m_btnConvert.EnableWindow(true);
		}
		else
		{
			m_ctrlProgress.SetPos((int)m_converter.GetProgress());
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

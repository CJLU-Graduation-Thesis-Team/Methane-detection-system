
// CMSDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CMS.h"
#include "CMSDlg.h"
#include "afxdialogex.h"

#include "IOCPModel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCMSDlg 对话框



CCMSDlg::CCMSDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CMS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCMSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCMSDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_EXIT, &CCMSDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_START, &CCMSDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CCMSDlg::OnBnClickedBtnStop)
END_MESSAGE_MAP()


// CCMSDlg 消息处理程序

BOOL CCMSDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//初始化界面信息
	this->Init();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCMSDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCMSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//////////////////////////////////////////////////////////////////////
// 初始化Socket库以及界面信息
void CCMSDlg::Init()
{
	// 初始化Socket库
	if (false == m_IOCP.LoadSocketLib())
	{
		AfxMessageBox(_T("加载Winsock 2.2失败，服务器端无法运行！"));
		PostQuitMessage(0);
	}

	// 设置本机IP地址
	SetDlgItemText(IDC_STATIC_IP, m_IOCP.GetLocalIP());
	// 设置默认端口
	SetDlgItemInt(IDC_EDIT_PORT, DEFAULT_PORT);
	// 初始化列表
	this->InitListCtrl();
	// 绑定主界面指针(为了方便在界面中显示信息 )
	m_IOCP.SetMainDlg(this);
}

///////////////////////////////////////////////////////////////////////
//	初始化List Control
void CCMSDlg::InitListCtrl()
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
	pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	pList->InsertColumn(0, "INFORMATION", LVCFMT_LEFT, 500);
}


void CCMSDlg::OnBnClickedBtnExit()
{
	// 停止监听
	m_IOCP.Stop();

	CDialog::OnCancel();
}


void CCMSDlg::OnBnClickedBtnStart()
{
	if (false == m_IOCP.Start())
	{
		AfxMessageBox(_T("服务器启动失败！"));
		return;
	}

	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
}


void CCMSDlg::OnBnClickedBtnStop()
{
	m_IOCP.Stop();

	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
}


///////////////////////////////////////////////////////////////////////
//	系统退出的时候，为确保资源释放，停止监听，清空Socket类库
void CCMSDlg::OnDestroy()
{
	OnBnClickedBtnExit();

	CDialog::OnDestroy();
}

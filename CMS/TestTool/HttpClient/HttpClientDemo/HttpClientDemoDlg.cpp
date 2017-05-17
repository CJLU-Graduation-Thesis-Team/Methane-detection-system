
// HttpClientDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HttpClientDemo.h"
#include "HttpClientDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI SendThread(LPVOID lpparentet);


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CHttpClientDemoDlg �Ի���



CHttpClientDemoDlg::CHttpClientDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_HTTPCLIENTDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHttpClientDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	//��ʼ���ٽ���
	InitializeCriticalSection(&m_csSend);
	CDialogEx::DoDataExchange(pDX);
}

CHttpClientDemoDlg::~CHttpClientDemoDlg()
{
	//��ʼ���ٽ���
	DeleteCriticalSection(&m_csSend);
}

BEGIN_MESSAGE_MAP(CHttpClientDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_REG, &CHttpClientDemoDlg::OnBnClickedBtnReg)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CHttpClientDemoDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_BTN_ADD_DEV, &CHttpClientDemoDlg::OnBnClickedBtnAddDev)
	ON_BN_CLICKED(IDC_BTN_REQ_DEV_LIST, &CHttpClientDemoDlg::OnBnClickedBtnReqDevList)
	ON_BN_CLICKED(IDC_BTN_REQ_DEV_STATUS, &CHttpClientDemoDlg::OnBnClickedBtnReqDevStatus)
	ON_BN_CLICKED(IDC_BTN_SET_THOR, &CHttpClientDemoDlg::OnBnClickedBtnSetThor)
END_MESSAGE_MAP()


// CHttpClientDemoDlg ��Ϣ�������

BOOL CHttpClientDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	CString cstrCloudIp = _T("120.25.214.105");
	((CIPAddressCtrl*)GetDlgItem(IDC_CLOND_IP))->SetWindowTextW(cstrCloudIp);
	

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CHttpClientDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CHttpClientDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CHttpClientDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI SendThread(LPVOID lpparentet)
{

	CHttpClientDemoDlg* p_Dlg = (CHttpClientDemoDlg*)lpparentet;
	if ( NULL != p_Dlg)
	{
		EnterCriticalSection(&(p_Dlg->m_csSend));

		CString cstrCloudIp;
		((CIPAddressCtrl*)p_Dlg->GetDlgItem(IDC_CLOND_IP))->GetWindowTextW(cstrCloudIp);
		p_Dlg->m_tcp.SetServerIp(WtoA(cstrCloudIp.GetBuffer()));

		p_Dlg->m_tcp.Send( p_Dlg->strReqHttpMsg );

		LeaveCriticalSection(&(p_Dlg->m_csSend));

	}
	return 0;
}


void CHttpClientDemoDlg::OnBnClickedBtnReg()
{
	CString cStrUserName;
	CString cStrPassWd;
	GetDlgItem(IDC_REG_NAME)->GetWindowText(cStrUserName);
	GetDlgItem(IDC_REG_PASSWD)->GetWindowText(cStrPassWd);

	std::string strReqTmp;

	/*GET / Main/AddUser   HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8
	
	UserName : xxxx &PassWd : xxxx */

	//strReqTmp = "POST /Main/AddUser HTTP/1.0";
	//strReqTmp += "\r\n Content-Type: text/html; charset=UTF-8";
	//strReqTmp += "\r\n\r\n";
	//strReqTmp += "UserName:" + WtoA(cStrUserName.GetBuffer()) + "&PassWd:" + WtoA(cStrPassWd.GetBuffer());


	/*POST / Main/AddUser?UserName : xxxx &PassWd : xxxx    HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8 */

	strReqTmp = "POST /Main/AddUser?UserName" + WtoA(cStrUserName.GetBuffer()) + "&PassWd:" + WtoA(cStrPassWd.GetBuffer()) + " HTTP/1.0";
	//strReqTmp += "UserName:" + WtoA(cStrUserName.GetBuffer()) + "&PassWd:" + WtoA(cStrPassWd.GetBuffer());
	strReqTmp += "\r\n Content-Type: text/html; charset=UTF-8";
	strReqTmp += "\r\n\r\n";
	//strReqTmp += "UserName:" + WtoA(cStrUserName.GetBuffer()) + "&PassWd:" + WtoA(cStrPassWd.GetBuffer());


	strReqHttpMsg = strReqTmp;

	CreateThread(NULL, 0, SendThread, this, 0, NULL);
}


void CHttpClientDemoDlg::OnBnClickedBtnLogin()
{
	CString cStrUserName;
	CString cStrPassWd;
	GetDlgItem(IDC_LOGIN_NAME)->GetWindowText(cStrUserName);
	GetDlgItem(IDC_LOGIN_PASSWD)->GetWindowText(cStrPassWd);

	std::string strReqTmp;

	/*GET /Login ? UserName = xxxx &PassWd = xxxx   HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8*/

	strReqTmp = "GET /Login?UserName=" + WtoA(cStrUserName.GetBuffer()) + "&PassWd=" + WtoA(cStrPassWd.GetBuffer()) + " HTTP/1.0";
	strReqTmp += "\r\n Content-Type: text/html; charset=UTF-8";
	strReqTmp += "\r\n\r\n";

	strReqHttpMsg = strReqTmp;

	CreateThread(NULL, 0, SendThread, this, 0, NULL);
}


void CHttpClientDemoDlg::OnBnClickedBtnAddDev()
{
	CString cStrUserName;
	CString cStrDevSn;

	GetDlgItem(IDC_SET1_NAME)->GetWindowText(cStrUserName);
	GetDlgItem(IDC_SET1_SN)->GetWindowText(cStrDevSn);

	std::string strReqTmp;

	/*POST /Main/AddDevice   HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8
	
	UserName:xxxx&DevSn:xxxx*/

	

	strReqTmp = "POST /Main/AddDevice?";
	strReqTmp += "UserName:" + WtoA(cStrUserName.GetBuffer()) + "&DevSn:" + WtoA(cStrDevSn.GetBuffer()) + "&NickName:xxxxx HTTP/1.1";
	strReqTmp += " \r\n Content-Type: text/html; charset=UTF-8";
	strReqTmp += "\r\n\r\n";


	strReqHttpMsg = strReqTmp;

	CreateThread(NULL, 0, SendThread, this, 0, NULL);
}


void CHttpClientDemoDlg::OnBnClickedBtnReqDevList()
{
	CString cStrUserName;
	GetDlgItem(IDC_REQ1_NAME)->GetWindowText(cStrUserName);

	std::string strReqTmp;

	/*GET  /Main/GetDeviceList ? UserName= xxxx  HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8*/

	strReqTmp = "GET /Main/GetDeviceList?UserName=" + WtoA(cStrUserName.GetBuffer()) + " HTTP/1.0";
	strReqTmp += "\r\n Content-Type: text/html; charset=UTF-8";
	strReqTmp += "\r\n\r\n";

	strReqHttpMsg = strReqTmp;

	CreateThread(NULL, 0, SendThread, this, 0, NULL);
}


void CHttpClientDemoDlg::OnBnClickedBtnReqDevStatus()
{
	CString cStrDevSn;
	GetDlgItem(IDC_REQ2_SN)->GetWindowText(cStrDevSn);

	std::string strReqTmp;

	/*GET /Main/GetDeviceStatus ? DevSn= xxxx  HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8*/

	strReqTmp = "GET /Main/GetDeviceStatus?DevSn=" + WtoA(cStrDevSn.GetBuffer()) + " HTTP/1.0";
	strReqTmp += "\r\n Content-Type: text/html; charset=UTF-8";
	strReqTmp += "\r\n\r\n";

	strReqHttpMsg = strReqTmp;

	CreateThread(NULL, 0, SendThread, this, 0, NULL);
}


void CHttpClientDemoDlg::OnBnClickedBtnSetThor()
{
	CString cStrThro;
	CString cStrDevSn;

	GetDlgItem(IDC_SET2_SN)->GetWindowText(cStrDevSn);
	GetDlgItem(IDC_SET2_THRO)->GetWindowText(cStrThro);

	std::string strReqTmp;

	/*POST  /Main/SetDeviceThreshold   HTTP / 1.0

	Content - Type : text / html; charset = UTF - 8

	Threshold:123.0&DevSn:12345*/

	strReqTmp = "POST /Main/SetDeviceThreshold HTTP/1.0";
	strReqTmp += "\r\n Content-Type: text/html; charset=UTF-8";
	strReqTmp += "\r\n\r\n";
	strReqTmp += "Threshold:" + WtoA(cStrThro.GetBuffer()) + "&DevSn:" + WtoA(cStrDevSn.GetBuffer());


	strReqHttpMsg = strReqTmp;

	CreateThread(NULL, 0, SendThread, this, 0, NULL);
}

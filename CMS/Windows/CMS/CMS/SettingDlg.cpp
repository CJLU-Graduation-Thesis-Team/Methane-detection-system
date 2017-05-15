// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CMS.h"
#include "SettingDlg.h"

// CSettingDlg dialog

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
	, m_bAutoLaunch(FALSE)
	, m_bRetJson(TRUE)
	, m_bAutoRun(FALSE)
	, m_strRoot(_T(""))
	, m_strDSN(_T("MySql"))
	, m_strServer(_T("localhost"))
	, m_strSrouce(_T("methane-detection-system"))
	, m_strUserName(_T("root"))
	, m_strPassWd(_T("cjlu12345+"))
	, m_nPort(80)
	, m_nDevPort(8808)
	, m_bListFile(TRUE)
	, m_nMaxConn(5000)
	, m_nSessionTimeout(0)
	, m_nDeadConnectionTimeout(0)
	, m_nMaxClientConn(0)
	, m_fMaxSpeed(0)
	, m_bDisableScreenLog(FALSE)
	, m_bDisableFileLog(FALSE)
	, m_strDefName(_T("index.html,index.htm,default.html,default.htm"))
{

}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK2, m_bAutoLaunch);
	DDX_Check(pDX, IDC_CHECK1, m_bAutoRun);
	DDX_Check(pDX, IDC_CHECK6, m_bRetJson);
	DDX_Text(pDX, IDC_EDIT1, m_strRoot);
	DDX_Text(pDX, IDC_EDIT10, m_strDSN);
	DDX_Text(pDX, IDC_EDIT11, m_strServer);
	DDX_Text(pDX, IDC_EDIT12, m_strSrouce);
	DDX_Text(pDX, IDC_EDIT13, m_strUserName);
	DDX_Text(pDX, IDC_EDIT14, m_strPassWd);
	DDX_Text(pDX, IDC_EDIT2, m_nPort);
	DDX_Text(pDX,IDC_EDIT_DEEV_PORT,m_nDevPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
	DDV_MinMaxInt(pDX, m_nDevPort, 1, 65535);
	DDX_Check(pDX, IDC_CHECK3, m_bListFile);
	DDX_Text(pDX, IDC_EDIT3, m_nMaxConn);
	DDV_MinMaxInt(pDX, m_nMaxConn, 1, 100000);
	DDX_Text(pDX, IDC_EDIT4, m_nSessionTimeout);
	DDV_MinMaxInt(pDX, m_nSessionTimeout, 0, 99999);
	DDX_Text(pDX, IDC_EDIT5, m_nDeadConnectionTimeout);
	DDV_MinMaxInt(pDX, m_nDeadConnectionTimeout, 0, 9999);
	DDX_Text(pDX, IDC_EDIT6, m_nMaxClientConn);
	DDV_MinMaxInt(pDX, m_nMaxClientConn, 0, 99999);
	DDX_Text(pDX, IDC_EDIT7, m_fMaxSpeed);
	DDV_MinMaxFloat(pDX, m_fMaxSpeed, 0, 999999);
	DDX_Check(pDX, IDC_CHECK4, m_bDisableScreenLog);
	DDX_Check(pDX, IDC_CHECK5, m_bDisableFileLog);
	DDX_Text(pDX, IDC_EDIT8, m_strDefName);
	DDX_Control(pDX, IDC_CHECK5, m_chkDisableLog);
	DDX_Control(pDX, IDC_CHECK4, m_chkDisableScreenLog);
	DDX_Control(pDX, IDC_CHECK3, m_chkListDir);
	DDX_Control(pDX, IDC_EDIT8, m_edtDftFileName);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSettingDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK3, &CSettingDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK5, &CSettingDlg::OnBnClickedCheck5)
	ON_EN_CHANGE(IDC_EDIT5, &CSettingDlg::OnEnChangeEdit5)
END_MESSAGE_MAP()


// CSettingDlg message handlers

void CSettingDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	AutoLaunch(m_bAutoLaunch);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("AutoRun"), m_bAutoRun);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("DisableScreenLog"), m_bDisableScreenLog);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("DisableFileLog"), m_bDisableFileLog);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("ListFile"), m_bListFile);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("DefaultFileName"), m_strDefName);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("Root"), m_strRoot);



	AfxGetApp()->WriteProfileString(INI_SESSION, _T("DSN"), m_strDSN);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("Server"), m_strServer);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("Srouce"), m_strSrouce);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("UserName"), m_strUserName);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("PassWd"), m_strPassWd);


	
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("RetJson"), m_bRetJson);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("Port"), m_nPort);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("DevPort"), m_nDevPort);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("MaxConnections"), m_nMaxConn);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("DeadConnectionTimeout"), m_nDeadConnectionTimeout);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("SessionTimeout"), m_nSessionTimeout);
	AfxGetApp()->WriteProfileInt(INI_SESSION, _T("MaxClientConnections"), m_nMaxClientConn);
	CString strSpeed(_T(""));
	strSpeed.Format(_T("%.2f"), m_fMaxSpeed);
	AfxGetApp()->WriteProfileString(INI_SESSION, _T("MaxSpeed"), strSpeed);

	OnOK();
}

void CSettingDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	TCHAR szFilePath[520] = {0};
	if( 0 == GetModuleFileName(NULL, szFilePath, 512))
	{
	}
	else
	{
		TCHAR* pEnd = _tcsrchr(szFilePath, _T('\\'));
		if(pEnd == NULL)
		{
		}
		else
		{
			pEnd[0] = 0;
		}
	}
	
	m_bAutoLaunch = IsAutoLaunch();
	m_bAutoRun = AfxGetApp()->GetProfileInt(INI_SESSION, _T("AutoRun"), 0);
	m_bRetJson = AfxGetApp()->GetProfileInt(INI_SESSION, _T("RetJson"), 1);

	m_bDisableScreenLog = AfxGetApp()->GetProfileInt(INI_SESSION, _T("DisableScreenLog"), 0);
	m_bDisableFileLog = AfxGetApp()->GetProfileInt(INI_SESSION, _T("DisableFileLog"), 0);
	m_bListFile = AfxGetApp()->GetProfileInt(INI_SESSION, _T("ListFile"), 1);
	m_strDefName = AfxGetApp()->GetProfileString(INI_SESSION, _T("DefaultFileName"), _T("index.html,index.htm,default.html,default.htm"));
	m_strRoot = AfxGetApp()->GetProfileString(INI_SESSION, _T("Root"), szFilePath);
	m_strDSN = AfxGetApp()->GetProfileString(INI_SESSION, _T("DSN"), _T("MySql"));
	m_strServer = AfxGetApp()->GetProfileString(INI_SESSION, _T("Server"), _T("localhost"));
	m_strSrouce = AfxGetApp()->GetProfileString(INI_SESSION, _T("Srouce"), _T("methane-detection-system"));
	m_strUserName = AfxGetApp()->GetProfileString(INI_SESSION, _T("UserName"), _T("root"));
	m_strPassWd = AfxGetApp()->GetProfileString(INI_SESSION, _T("PassWd"), _T("cjlu12345+"));
	m_nPort = AfxGetApp()->GetProfileInt(INI_SESSION, _T("Port"), 80);
	m_nDevPort = AfxGetApp()->GetProfileInt(INI_SESSION, _T("DevPort"), 8808);
	m_nMaxConn = AfxGetApp()->GetProfileInt(INI_SESSION, _T("MaxConnections"), 5000);
	m_nDeadConnectionTimeout = AfxGetApp()->GetProfileInt(INI_SESSION, _T("DeadConnectionTimeout"), 30);
	m_nSessionTimeout = AfxGetApp()->GetProfileInt(INI_SESSION, _T("SessionTimeout"), 0);
	m_nMaxClientConn = AfxGetApp()->GetProfileInt(INI_SESSION, _T("MaxClientConnections"), 0);
	CString strSpeed = AfxGetApp()->GetProfileString(INI_SESSION, _T("MaxSpeed"), _T("0"));
	m_fMaxSpeed = (float)_ttof((LPCTSTR)strSpeed);
	
	

	m_edtDftFileName.SetReadOnly(m_bListFile);
	//if(m_bDisableFileLog) m_bDisableScreenLog = TRUE;
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingDlg::OnBnClickedCheck3()
{
	// 点击了"允许浏览目录"复选框
	m_edtDftFileName.SetReadOnly(BST_CHECKED == m_chkListDir.GetCheck());
}


void CSettingDlg::OnBnClickedCheck5()
{
	// 点击了"禁止输出日志"复选框
	/*
	if( BST_CHECKED == m_chkDisableLog.GetCheck() )
	{
		m_chkDisableScreenLog.SetCheck(BST_CHECKED);
	}
	*/
}


void CSettingDlg::OnEnChangeEdit5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

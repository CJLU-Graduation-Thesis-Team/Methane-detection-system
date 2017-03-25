// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "CMS.h"
#include "TcpIocp.h"

#include "MainFrm.h"
#include "SettingDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_STATUS_CONNECTIONS 10001
#define ID_STATUS_SPEED_UP 10002
#define ID_STATUS_SPEED_DOWN 10003

#define IDT_UPDATE_SPEED 10005 /*ˢ�´�������*/
#define CHECKTIME_SPEED 2000 /*ÿ�����ٺ���ˢ��һ�δ���*/

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_START, &CMainFrame::OnStart)
	ON_UPDATE_COMMAND_UI(ID_START, &CMainFrame::OnUpdateStart)
	ON_COMMAND(ID_STOP, &CMainFrame::OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, &CMainFrame::OnUpdateStop)
	ON_COMMAND(ID_SETTING, &CMainFrame::OnSetting)
	ON_UPDATE_COMMAND_UI(ID_SETTING, &CMainFrame::OnUpdateSetting)
	ON_COMMAND(ID_LOG, &CMainFrame::OnLog)
	ON_UPDATE_COMMAND_UI(ID_LOG, &CMainFrame::OnUpdateLog)
	ON_COMMAND(ID_PAUSE, &CMainFrame::OnPause)
	ON_UPDATE_COMMAND_UI(ID_PAUSE, &CMainFrame::OnUpdatePause)
	ON_COMMAND(ID_APP_EXIT, &CMainFrame::OnAppExit)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_NOTIFY_ICON, &CMainFrame::OnTrayIcon)
	ON_MESSAGE(WM_LOGMESSAGE, &CMainFrame::OnLogMessage)
	ON_COMMAND(ID_CLEAR_LOG, &CMainFrame::OnClearLog)
	ON_UPDATE_COMMAND_UI(ID_CLEAR_LOG, &CMainFrame::OnUpdateClearLog)
	ON_COMMAND(ID_ENABLE_LOG, &CMainFrame::OnEnableLog)
	ON_UPDATE_COMMAND_UI(ID_ENABLE_LOG, &CMainFrame::OnUpdateEnableLog)
	ON_WM_TIMER()
	ON_MESSAGE(WM_CONNECTION_NUMBER, &CMainFrame::OnConnectionNumber)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ״̬��ָʾ��
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};


// CMainFrame ����/����

CMainFrame::CMainFrame() : m_ll(LL_ALL), m_nWindowLogger(0), m_nFileLogger(0)
{
	// TODO: �ڴ���ӳ�Ա��ʼ������
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_uTimer = 0;
	InitializeCriticalSection(&m_cs);
}

CMainFrame::~CMainFrame()
{
	DeleteObject(m_hIcon);
	DeleteCriticalSection(&m_cs);
}

void CMainFrame::lock()
{
	EnterCriticalSection(&m_cs);
}

void CMainFrame::unlock()
{
	LeaveCriticalSection(&m_cs);
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// ����һ����ͼ��ռ�ÿ�ܵĹ�����
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("δ�ܴ�����ͼ����\n");
		return -1;
	}
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("δ�ܴ���������\n");
		return -1;      // δ�ܴ���
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("δ�ܴ���״̬��\n");
		return -1;      // δ�ܴ���
	}
	else
	{
		m_wndStatusBar.SetPaneInfo(1, ID_STATUS_CONNECTIONS, SBPS_NORMAL, 110);
		m_wndStatusBar.SetPaneInfo(2, ID_STATUS_SPEED_UP, SBPS_NORMAL, 100);
		m_wndStatusBar.SetPaneInfo(3, ID_STATUS_SPEED_DOWN, SBPS_NORMAL, 100);
	}

	// ����ͼ��
	SetIcon(m_hIcon, TRUE);

	// TODO: �������Ҫ��ͣ������������ɾ��������
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// ��ʼ����־ģ��
	theLogger.open(true, LL_NONE);
	
	// �������ͼ��.
	memset(&m_IconData, 0, sizeof(m_IconData));
	m_IconData.cbSize = sizeof(m_IconData);
	m_IconData.hWnd = m_hWnd;
	m_IconData.uID = 100;
	m_IconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_IconData.uCallbackMessage = WM_NOTIFY_ICON;
	m_IconData.uTimeout = 500;
	_tcscpy(m_IconData.szTip, _T("��������ƽ̨ - by Guangle's C++ Studio"));
	m_IconData.hIcon = m_hIcon;
	Shell_NotifyIcon(NIM_ADD, &m_IconData);

	// ��ʼ��״̬��
	ResetStatus();

	//  �Ƿ������־
	if(!AfxGetApp()->GetProfileInt(INI_SESSION, _T("DisableScreenLog"), 0))
	{
		m_nWindowLogger = theLogger.addHwndAppender(m_hWnd, WM_LOGMESSAGE);
	}
	if( !AfxGetApp()->GetProfileInt(INI_SESSION, _T("DisableFileLog"), 0))
	{
		m_nFileLogger = theLogger.addFileAppender(getLogFileName());
	}
	if(m_nWindow != 0 || m_nFileLogger != 0)
	{
		theLogger.setLogLevel(LL_ALL);
	}

	// ����
	if(AfxGetApp()->GetProfileInt(INI_SESSION, _T("AutoRun"), 0))
	{
		LOGGER_CINFO(theLogger, _T("��������ƽ̨ ������...\r\n"));
		OnStart();
	}
	else
	{
		LOGGER_CINFO(theLogger, _T("��������ƽ̨ ׼������.\r\n"));
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame ��Ϣ�������

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// ������ǰ�Ƶ���ͼ����
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// ����ͼ��һ�γ��Ը�����
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// ����ִ��Ĭ�ϴ���
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

std::wstring CMainFrame::getLogFileName()
{
	TCHAR szFilePath[MAX_PATH] = {0};
	if( 0 == GetModuleFileName(NULL, szFilePath, MAX_PATH))
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
			pEnd++;
			_tcscpy(pEnd, _T("HTTPServerLOG.TXT"));
		}
	}

	return szFilePath;
}


void CMainFrame::OnStart()
{
	HTTPSTARTDESC startDesc;
	memset(&startDesc, 0, sizeof(HTTPSTARTDESC));

	TCHAR szFilePath[MAX_PATH] = {0};
	if( 0 == GetModuleFileName(NULL, szFilePath, MAX_PATH))
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
	
	CString strRoot = AfxGetApp()->GetProfileString(INI_SESSION, _T("Root"), szFilePath);
	_tcsncpy(startDesc.szRootDir, (LPCTSTR)strRoot, MAX_PATH);
	startDesc.nPort = AfxGetApp()->GetProfileInt(INI_SESSION, _T("Port"), 80);
	startDesc.nMaxConnection =  AfxGetApp()->GetProfileInt(INI_SESSION, _T("MaxConnections"), 5000);
	startDesc.bNavDir = AfxGetApp()->GetProfileInt(INI_SESSION, _T("ListFile"), TRUE);
	startDesc.dwDeadConnectionTimeout = AfxGetApp()->GetProfileInt(INI_SESSION, _T("DeadConnectionTimeout"), 30) * 1000;
	startDesc.dwSessionTimeout = AfxGetApp()->GetProfileInt(INI_SESSION, _T("SessionTimeout"), 0) * 1000;
	if(startDesc.dwDeadConnectionTimeout < 0) startDesc.dwDeadConnectionTimeout = 0;
	if(startDesc.dwSessionTimeout < 0) startDesc.dwSessionTimeout = 0;
	startDesc.pStatusHandler = this;
	CString strSpeed = AfxGetApp()->GetProfileString(INI_SESSION, _T("MaxSpeed"), _T("0"));
	double dSpeed = _ttof((LPCTSTR)strSpeed);
	startDesc.llMaxSpeed = (__int64)(dSpeed * 1024);
	startDesc.nMaxClientConn = AfxGetApp()->GetProfileInt(INI_SESSION, _T("MaxClientConnections"), 0);
	CString strDefName = AfxGetApp()->GetProfileString(INI_SESSION, _T("DefaultFileName"), _T("index.html,index.htm,default.html,default.htm"));
	_tcsncpy(startDesc.szDefaultFileName, (LPCTSTR)strDefName, MAX_PATH);


	// ��ʼ��Socket��
	if (false == m_Tcp.LoadSocketLib())
	{
		LOGGER_CINFO(theLogger, _T("����Winsock 2.2ʧ�ܣ����������޷����У�..\r\n"));
	}
	if (true == m_Tcp.Start())
	{
		LOGGER_CINFO(theLogger, _T("Tcp Iocp �����ɹ�..\r\n"));
	}
	else
	{
		LOGGER_CINFO(theLogger, _T("Tcp Iocp ����ʧ��..\r\n"));
	}


	if( SE_SUCCESS == m_Server.Run(&startDesc))
	{
		ResetStatus();
		m_uTimer = SetTimer(IDT_UPDATE_SPEED, CHECKTIME_SPEED, NULL);

		LOGGER_CINFO(theLogger, _T("HTTP Server �����ɹ�,��Ŀ¼[%s],�˿�[%d],���������[%d].\r\n"), strRoot, startDesc.nPort, startDesc.nMaxConnection);
	}
	else
	{
		LOGGER_CINFO(theLogger, _T("HTTP Server ����ʧ��,�Ѿ������л��߶˿�[%d]��ռ��..\r\n"), startDesc.nPort);
	}


}

void CMainFrame::ResetStatus()
{
	SetConnectionsNumber(0);
	m_lBytesSent = 0;
	m_lBytesRecv = 0;
	m_dwLastTick = 0;

	if(m_uTimer != 0)
	{
		KillTimer(m_uTimer);
		m_uTimer = 0;
	}

	m_wndStatusBar.SetPaneText(2, _T("����: 0.00 B/s"), TRUE);
	m_wndStatusBar.SetPaneText(3, _T("����: 0.00 B/s"), TRUE);
}

void CMainFrame::OnUpdateStart(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_Server.IsRuning());
}

void CMainFrame::OnStop()
{
	//ֹͣDev Tcp ����
	m_Tcp.Stop();

	HTTP_SERVER_ERROR_TYPE hm = m_Server.Stop();
	if(hm == SE_SUCCESS)
	{
		ResetStatus();
		LOGGER_CINFO(theLogger, _T("HTTP Server ֹͣ����.\r\n"));
	}
	else
	{
		LOGGER_CINFO(theLogger, _T("��ʱ�޷�ֹͣ����,��鿴��־.\r\n"));
	}
}

void CMainFrame::OnUpdateStop(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Server.IsRuning());
}

void CMainFrame::OnSetting()
{
	// TODO: Add your command handler code here
	do_setting();
}

void CMainFrame::OnUpdateSetting(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CFrameWnd::PreTranslateMessage(pMsg);
}

#define REMOVE_LINE_COUNT 100
void CMainFrame::do_log(const TCHAR* strLog)
{
	if (m_wndView.m_edt.m_hWnd == NULL)
	{
	}
	else
	{
		int nLmt = m_wndView.m_edt.GetLimitText();
		int nTextLength = m_wndView.m_edt.GetWindowTextLength();
		if( (int)(nTextLength + _tcslen(strLog)) >= nLmt )
		{
			int nPos = m_wndView.m_edt.LineIndex(REMOVE_LINE_COUNT);
			m_wndView.m_edt.SetSel(0, nPos);
			m_wndView.m_edt.ReplaceSel(_T(""), FALSE);  // ������ Clear(), ���� Undo() ��ԭ��, �� Clear() ��û������ɾ������.
		}

		m_wndView.m_edt.SetSel(nTextLength, nTextLength);
		m_wndView.m_edt.ReplaceSel(strLog);
	}
}

void CMainFrame::OnLog()
{
}

LRESULT CMainFrame::OnLogMessage(WPARAM w, LPARAM l)
{
	LPCTSTR log_msg = reinterpret_cast<LPCTSTR>(l);
	do_log(log_msg);
	delete []log_msg;
	return 0;
}

void CMainFrame::OnUpdateLog(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CMainFrame::OnPause()
{
	// TODO: Add your command handler code here
}

void CMainFrame::OnUpdatePause(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CMainFrame::OnAppExit()
{
	// TODO: Add your command handler code here
	if(m_Server.IsRuning()) m_Server.Stop();
	Shell_NotifyIcon(NIM_DELETE, &m_IconData);
	//
	CFrameWnd::OnClose();
}

void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
}

void CMainFrame::do_setting()
{
	CSettingDlg dlg;
	if( IDOK == dlg.DoModal())
	{
		// Ӧ���µ�����(������־��),����������Ҫ����������

		if(dlg.m_bDisableScreenLog)
		{
			if(m_nWindowLogger != 0) theLogger.removeAppender(m_nWindowLogger);
			m_nWindowLogger = 0;
		}
		else
		{
			if(m_nWindowLogger == 0) m_nWindowLogger = theLogger.addHwndAppender(m_hWnd, WM_LOGMESSAGE);
		}

		if(dlg.m_bDisableFileLog)
		{
			if(m_nFileLogger != 0) theLogger.removeAppender(m_nFileLogger);
			m_nFileLogger = 0;
		}
		else
		{
			if(m_nFileLogger == 0) m_nFileLogger = theLogger.addFileAppender(getLogFileName());
		}

		if(m_nWindowLogger == 0 && m_nFileLogger == 0)
		{
			theLogger.setLogLevel(LL_NONE);
		}
		else
		{
			theLogger.setLogLevel(LL_ALL);
		}
	}
}

LRESULT CMainFrame::OnTrayIcon(WPARAM w, LPARAM l)
{
	if( l == WM_LBUTTONUP || l == WM_RBUTTONUP)
	{
		ShowWindow(SW_SHOW);
	}
	return 0;
}

void CMainFrame::OnClearLog()
{
	m_wndView.m_edt.SetSel(0, -1);
	m_wndView.m_edt.ReplaceSel(_T(""), FALSE);
}


void CMainFrame::OnUpdateClearLog(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
}


void CMainFrame::OnEnableLog()
{
	// TODO: �ڴ���������������
	if(m_nWindowLogger == 0)
	{
		m_nWindowLogger = theLogger.addHwndAppender(m_hWnd, WM_LOGMESSAGE);
		theLogger.setLogLevel(LL_ALL);
	}
	else
	{
		theLogger.removeAppender(m_nWindowLogger);
		m_nWindowLogger = 0;
		if(m_nFileLogger == 0) theLogger.setLogLevel(LL_NONE);
	}
}


void CMainFrame::OnUpdateEnableLog(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	if(m_nWindowLogger == 0)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::SetConnectionsNumber(int nTotalConnections)
{
	m_nTotalConnections = nTotalConnections;
	/*wchar_t szConnectionsText[200] = {L'\0'};
	wsprintf(szConnectionsText, _T("Ŀǰ��: %d������"), m_nTotalConnections);
	m_wndStatusBar.SetPaneText(1, szConnectionsText, TRUE);*/

	// ֻ������Ϣ�ķ�ʽˢ���ı�,��Ϊһ��������һ���߳��ڵ���SetConnectionsNumber()����
	// �������һ���̵߳���, IsWindow()��ʧ��.
	PostMessage(WM_CONNECTION_NUMBER, m_nTotalConnections, 0);
}

LRESULT CMainFrame::OnConnectionNumber(WPARAM w, LPARAM l)
{
	int nConnectionNumber = (int)w;
	wchar_t szConnectionsText[200] = {L'\0'};
	wsprintf(szConnectionsText, _T("Ŀǰ��: %d������"), nConnectionNumber);
	m_wndStatusBar.SetPaneText(1, szConnectionsText, TRUE);
	return 0;
}

void CMainFrame::OnNewConnection(const wchar_t *pszIP, unsigned int nPort, BOOL bRefused, BOOL bKicked)
{
	// ��Ҫͬ��.
	if(bKicked)
	{
		LOGGER_CINFO(theLogger, _T("[%s:%d] - �ͻ����ӱ��ܾ�,������IP������������������.\r\n"), pszIP, nPort);
	}
	else
	{
		if(bRefused)
		{
			LOGGER_CINFO(theLogger, _T("[%s:%d] - ������æ,���ӱ�����.\r\n"), pszIP, nPort);
		}
		else
		{
			LOGGER_CINFO(theLogger, _T("[%s:%d] - �����ӱ�����.\r\n"), pszIP, nPort);

			lock();
			SetConnectionsNumber(m_nTotalConnections + 1);
			unlock();
		}
	}
}

void CMainFrame::OnConnectionClosed(const wchar_t *pszIP, unsigned int nPort, REMOVE_REASON rr, __int64 nByteSent, unsigned int nTimeUsed)
{
	// ��Ҫͬ��
	lock();
	SetConnectionsNumber(m_nTotalConnections - 1);
	unlock();

	std::wstring strReason(L"");
	switch(rr)
	{
	case RR_CLIENTCLOSED: { strReason = _T("�ͻ��˹ر�������"); break; }
	case RR_SENDCOMPLETE: { strReason  = _T("���ݷ������"); break; }
	case RR_RECV_FAILED: { strReason = _T("����ʧ��"); break; }
	case  RR_SEND_FAILED: { strReason = _T("����ʧ��"); break; }
	case RR_DEAD: { strReason = _T("����Ծ����"); break; }
	case RR_SESSION_TIMEOUT: { strReason= _T("�Ự��ʱ"); break; }
	default: { strReason = _T("δ֪"); break; }
	}

	// �����Է������ݵĳ���
	CString strBytes(_T(""));
	if(nByteSent >= G_BYTES)
	{
		strBytes.Format(_T("%.2fGB"), nByteSent * 1.0 / G_BYTES);
	}
	else if(nByteSent >= M_BYTES)
	{
		strBytes.Format(_T("%.2fMB"), nByteSent * 1.0 / M_BYTES);
	}
	else if(nByteSent >= K_BYTES)
	{
		strBytes.Format(_T("%.2fKB"), nByteSent * 1.0 / K_BYTES);
	}
	else
	{
		strBytes.Format(_T("%lldBytes"), nByteSent);
	}

	// ����ƽ������
	CString strSpeed(_T(""));
	if(nTimeUsed <= 0)
	{
		strSpeed = _T("---");
	}
	else
	{
		double llSpeed = nByteSent * 1.0 / nTimeUsed * 1000;
		if(llSpeed >= G_BYTES)
		{
			strSpeed.Format(_T("%.2fGB/s"), llSpeed * 1.0 / G_BYTES);
		}
		else if(llSpeed >= M_BYTES)
		{
			strSpeed.Format(_T("%.2fMB/s"), llSpeed * 1.0 / M_BYTES);
		}
		else if(llSpeed >= K_BYTES)
		{
			strSpeed.Format(_T("%.2fKB/s"), llSpeed * 1.0 / K_BYTES);
		}
		else
		{
			strSpeed.Format(_T("%.2fB/s"), llSpeed);
		}
	}

	LOGGER_CINFO(theLogger, _T("[%s:%d] - ���ӱ��ر�[%s],�ܼƷ�������[%s],��ʱ[%.3fs],ƽ���ٶ�[%s].\r\n"), 
		pszIP, nPort, strReason.c_str(), (LPCTSTR)strBytes, nTimeUsed * 1.0 / 1000, (LPCTSTR)strSpeed);
}

void CMainFrame::OnDataSent(const wchar_t *pszIP, unsigned int nPort, unsigned int nBytesSent)
{
	// ��Ҫ��ͬ�� InterLockedExchangeAdd
	// InterlockedExchangeAdd64(&m_lBytesSent, nBytesSent);

	lock(); 
	m_lBytesSent += nBytesSent;
	unlock();
}

void CMainFrame::OnDataReceived(const wchar_t *pszIP, unsigned int nPort, unsigned int nBytesReceived)
{
	// ��Ҫ��ͬ��

	// Windows Vista/7 �����������dll
	// InterlockedExchangeAdd64(&m_lBytesRecv, nBytesReceived); 

	// ����ϵͳ���ٽ��
	lock();
	m_lBytesRecv += nBytesReceived;
	unlock();
}

void CMainFrame::OnRequested(const wchar_t *pszIP, unsigned int nPort, const wchar_t *pszUrl, HTTP_METHOD hm, SERVER_CODE sc)
{
	// ֻдһ����־�Ϳ�����.
	LOGGER_CINFO(theLogger, _T("[%s:%d] - ������Դ[%s],��Ӧ��: %d.\r\n"), pszIP, nPort, pszUrl, sc);  
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if( nIDEvent == m_uTimer )
	{
		// ��¼����,�����ټ���

		lock();
		__int64 lBytesSent = m_lBytesSent;
		__int64 lBytesRecv = m_lBytesRecv;
		DWORD dwLastTickCount = m_dwLastTick;
		DWORD dwTickCount = GetTickCount();
		
		m_lBytesRecv = 0;
		m_lBytesSent = 0;
		m_dwLastTick = dwTickCount;
		unlock();


		/*
		__int64 lBytesSent = m_lBytesSent;
		__int64 lBytesRecv = m_lBytesRecv;
		DWORD dwLastTickCount = m_dwLastTick;
		DWORD dwTickCount = GetTickCount();

		InterlockedExchange64(&m_lBytesRecv, 0);
		InterlockedExchange64(&m_lBytesSent, 0);
		m_dwLastTick = dwTickCount;
		*/

		// �����ı���ʾ,û��ͬ��Ҫ��
		if(dwLastTickCount == 0 || dwTickCount <= dwLastTickCount)
		{
			// ��һ�β�������
		}
		else
		{
			CString strSpeedText(_T(""));

			// �������д��� Sent
			double llSpeed = lBytesSent * 1.0 / (dwTickCount - dwLastTickCount) * 1000;
			if(llSpeed >= G_BYTES)
			{
				strSpeedText.Format(_T("����: %.2f GB/s"), llSpeed * 1.0 / G_BYTES);
			}
			else if(llSpeed >= M_BYTES)
			{
				strSpeedText.Format(_T("����: %.2f MB/s"), llSpeed * 1.0 / M_BYTES);
			}
			else if(llSpeed >= K_BYTES)
			{
				strSpeedText.Format(_T("����: %.2f KB/s"), llSpeed * 1.0 / K_BYTES);
			}
			else
			{
				strSpeedText.Format(_T("����: %.2f B/s"), llSpeed);
			}
			m_wndStatusBar.SetPaneText(2, strSpeedText, TRUE);
			

			// �������д��� Received
			llSpeed = lBytesRecv * 1.0 / (dwTickCount - dwLastTickCount) * 1000;
			if(llSpeed >= G_BYTES)
			{
				strSpeedText.Format(_T("����: %.2f GB/s"), llSpeed * 1.0 / G_BYTES);
			}
			else if(llSpeed >= M_BYTES)
			{
				strSpeedText.Format(_T("����: %.2f MB/s"), llSpeed * 1.0 / M_BYTES);
			}
			else if(llSpeed >= K_BYTES)
			{
				strSpeedText.Format(_T("����: %.2f KB/s"), llSpeed * 1.0 / K_BYTES);
			}
			else
			{
				strSpeedText.Format(_T("����: %.2f B/s"), llSpeed);
			}
			m_wndStatusBar.SetPaneText(3, strSpeedText, TRUE);
		}
	}
	else
	{
		ASSERT(0);
	}

	__super::OnTimer(nIDEvent);
}

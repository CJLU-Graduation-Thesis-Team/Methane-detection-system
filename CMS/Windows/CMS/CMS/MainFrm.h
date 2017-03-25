// MainFrm.h : CMainFrame ��Ľӿ�
//


#pragma once

#include "ChildView.h"
#include "HttpServer.h"
#include "TcpIocp.h"

#define WM_NOTIFY_ICON (WM_USER + 107)
#define WM_LOGMESSAGE (WM_USER + 108)
#define WM_CONNECTION_NUMBER (WM_USER + 109) // ���µ�������Ŀ

class CMainFrame : public CFrameWnd, public IHTTPServerStatusHandler
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// ����
public:
	CHTTPServer m_Server;
	CIOCPModel  m_Tcp;                     

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	// IHTTPServerStatusHandler ��ʵ��
	virtual void OnNewConnection(const wchar_t *pszIP, unsigned int nPort, BOOL bRefused, BOOL bKicked);
	virtual void OnConnectionClosed(const wchar_t *pszIP, unsigned int nPort, REMOVE_REASON rr, __int64 nByteSent, unsigned int nTimeUsed);
	virtual void OnDataSent(const wchar_t *pszIP, unsigned int nPort, unsigned int nBytesSent);
	virtual void OnDataReceived(const wchar_t *pszIP, unsigned int nPort, unsigned int nBytesReceived);
	virtual void OnRequested(const wchar_t *pszIP, unsigned int nPort, const wchar_t *pszUrl, HTTP_METHOD hm, SERVER_CODE sc);

// ʵ��
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // �ؼ���Ƕ���Ա
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CChildView    m_wndView;
	LOGGER_LEVEL m_ll;
	int m_nWindowLogger;
	int m_nFileLogger;
	void do_log(const TCHAR* strLog);
	void do_setting();

	NOTIFYICONDATA m_IconData;
	HICON m_hIcon;

	// ������״̬��صı���
	int m_nTotalConnections;
	__int64 m_lBytesSent;
	__int64 m_lBytesRecv;
	DWORD m_dwLastTick; // �ϴ�ͳ�ƴ���ʱ��ϵͳʱ��.(��ΪTIMER��һ��׼ʱ)
	UINT m_uTimer; // ˢ�´������ݵĶ�ʱ��,1��һ��.
	CRITICAL_SECTION m_cs;
protected:
	void SetConnectionsNumber(int nTotalConnections);
	void ResetStatus();
	void lock();
	void unlock();
	std::wstring getLogFileName();

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnStart();
	afx_msg void OnUpdateStart(CCmdUI *pCmdUI);
	afx_msg void OnStop();
	afx_msg void OnUpdateStop(CCmdUI *pCmdUI);
	afx_msg void OnSetting();
	afx_msg void OnUpdateSetting(CCmdUI *pCmdUI);
	afx_msg void OnLog();
	afx_msg void OnUpdateLog(CCmdUI *pCmdUI);
	afx_msg void OnPause();
	afx_msg void OnUpdatePause(CCmdUI *pCmdUI);
	afx_msg void OnAppExit();
	afx_msg void OnClose();
	afx_msg LRESULT OnTrayIcon(WPARAM w, LPARAM l);
	afx_msg LRESULT OnLogMessage(WPARAM w, LPARAM l);
	afx_msg void OnClearLog();
	afx_msg void OnUpdateClearLog(CCmdUI *pCmdUI);
	afx_msg void OnEnableLog();
	afx_msg void OnUpdateEnableLog(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnConnectionNumber(WPARAM w, LPARAM l);
};



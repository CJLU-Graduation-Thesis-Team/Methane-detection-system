#include "stdafx.h"
#include "CMS.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebServerApp

BEGIN_MESSAGE_MAP(CCMSApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CCMSApp::OnAppAbout)
END_MESSAGE_MAP()


// CWebServerApp ����

CCMSApp::CCMSApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CWebServerApp ����

CCMSApp theApp;


// CWebServerApp ��ʼ��

BOOL CCMSApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��

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
			_tcscpy(pEnd, _T("\\settings.ini"));
			m_pszProfileName = _tcsdup(szFilePath);
		}
	}



	// ��Ҫ���������ڣ��˴��뽫�����µĿ�ܴ���
	// ����Ȼ��������ΪӦ�ó���������ڶ���
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame) return FALSE;
	m_pMainWnd = pFrame;

	// ���������ؿ�ܼ�����Դ
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

	if ( _tcsicmp(this->m_lpCmdLine, _T("hide")) == 0)
	{
		pFrame->ShowWindow(SW_HIDE);
		
	}
	else
	{
		pFrame->ShowWindow(this->m_nCmdShow);
	}
	
	pFrame->UpdateWindow();

	return TRUE;

}


// CWebServerApp ��Ϣ�������




// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CCMSApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CWebServerApp ��Ϣ�������


#define MY_REGKEY _T("httpserver")
BOOL AutoLaunch(BOOL bRun /* = TRUE */)
{	
	// ��ȡ·��
	TCHAR szFileName[MAX_PATH + 10] = {0};
	szFileName[0] = _T('\"');
	if (0 == GetModuleFileName(NULL, szFileName + 1, MAX_PATH) ) return FALSE;
	_tcscat(szFileName, _T("\" hide")); // ��С������

	BOOL bRet = FALSE;
	HKEY hKey;
	LPCTSTR szKeyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion");		
	long ret = RegOpenKeyEx(HKEY_CURRENT_USER, szKeyPath, 0, KEY_WRITE, &hKey);
	if(ret != ERROR_SUCCESS)
	{
		TRACE("�޷���ȡע���.");
	}
	else
	{
		HKEY hRunKey;
		ret = RegCreateKeyEx(hKey, _T("Run"), 0, NULL, 0, KEY_WRITE, NULL, &hRunKey, NULL);
		if(ERROR_SUCCESS == ret)
		{
			if(bRun)
			{
				bRet = (ERROR_SUCCESS == ::RegSetValueEx(hRunKey, MY_REGKEY, 0, REG_SZ, (BYTE*)szFileName, (_tcslen(szFileName) + 1) * sizeof(TCHAR)));
			}
			else
			{
				ret = RegDeleteValue(hRunKey, MY_REGKEY);
				bRet = (ret == ERROR_SUCCESS);
			}

			RegCloseKey(hRunKey);
		}
		else
		{
			TRACE("�޷�дע���.");
		}
		RegCloseKey(hKey);
	}
	return bRet;
}


BOOL IsAutoLaunch()
{
	BOOL bRet = FALSE;
	HKEY hKey;
	TCHAR szKeyPath[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");		
	long ret = RegOpenKeyEx(HKEY_CURRENT_USER, szKeyPath, 0, KEY_READ, &hKey);
	if(ret != ERROR_SUCCESS)
	{
		TRACE("�޷���ȡע���.");
	}
	else
	{
		TCHAR szValue[MAX_PATH + 1] = {0};
		DWORD dwType = REG_SZ;
		DWORD dwLen = MAX_PATH * sizeof(TCHAR);

		LONG lRet = ::RegQueryValueEx(hKey, MY_REGKEY, NULL, &dwType, (LPBYTE)szValue, &dwLen);
		if(ERROR_SUCCESS != lRet)
		{
		}
		else
		{
			TCHAR szFileName[MAX_PATH + 10] = {0};
			if (0 == GetModuleFileName(NULL, szFileName + 1, MAX_PATH) )
			{
				TRACE("�޷���ѯ��ȡ��ǰ���̵��ļ���.");
			}
			else
			{
				szFileName[0] = _T('\"');
				_tcscat(szFileName, _T("\" hide"));
				return _tcsicmp(szFileName, szValue) == 0;
			}

		}
		RegCloseKey(hKey);
	}

	return bRet;
}

// �����ַ���,�������������ĩβ���� "..."
LPTSTR omi_tcscpy(LPTSTR lpszDest, LPCTSTR lpszSrc, int nMaxDestSize)
{
	int nSrcSize = _tcslen(lpszSrc);
	if( nSrcSize >= nMaxDestSize) 
	{
		_tcsncpy(lpszDest, lpszSrc, nMaxDestSize - 4); // 3�� . ��һ�� \0
		lpszDest[nMaxDestSize - 1] = 0;
		lpszDest[nMaxDestSize - 2] = _T('.');
		lpszDest[nMaxDestSize - 3] = _T('.');
		lpszDest[nMaxDestSize - 4] = _T('.');
	}
	else
	{
		_tcscpy(lpszDest, lpszSrc);
	}
	return lpszDest;
}

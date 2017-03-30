/* Copyright (C) 2011  
 *
 *
 *    
*/

#include "stdafx.h"
#include "HttpServer.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPContent.h"
#include "HTTPWork.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define EXTHREAD 1		/*��CPU��Ŀ�������߳���*/

// HTTP1.1ѡ���
#define OPT_KEEPALIVE 0x00000001
#define _OUTPUT_DETAIL

//////////////////////////////////////////////////////////////////////////
// �ṹ����
typedef struct tagListenParam				// �����̵߳Ĳ���
{
	CHTTPServer *pHttpserver;				// ��ʵ��ָ��
	int nPort;
	SOCKET hSock;	// �����׽���
}PARAM_LISTENTHREAD;

typedef struct tagSvrParam					// �����̵߳Ĳ���
{
	CHTTPServer			*pHttpserver;
	int					nThreadIndex;				// �߳����(ÿ�����������������߳�)
	HANDLE				hComplitionPort;			// ��ɶ˿ھ��
}PARAM_SVRTHREAD;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CHTTPServer::CHTTPServer(/*HWND hNotifyWnd / * = NULL * /, UINT uMessageID / * = WM_HTTPFILESERVER * /*/)
{
	m_bRuning = FALSE;
	m_bNavDir = TRUE;
	
	m_nListenPort = 80;					// �����˿�
	m_nAcceptBufLen = (sizeof(sockaddr_in) + 16) * 2;  // �����16���ֽ��� AcceptEx()��Ҫ��,��MSDN.
	m_pAcceptBuf = new char[m_nAcceptBufLen + 1];
	ASSERT(m_pAcceptBuf);
	
	m_pServiceInf = NULL;				// ��������
	m_nMaxConnections = 2000;			// ÿ����ɶ˿�(����)���ܵ����������

	m_nMaxClientConn = 0;
	m_llMaxSpeed = 0;

	m_sockListen = INVALID_SOCKET;
	m_sockNewClient = INVALID_SOCKET;
	m_pStatusHandler = NULL;

	m_dwSessionTimeout = 0;		// 0��ʾ����鳬ʱ.
	m_dwDeadConnectionTimeout = 0;	// 0��ʾ����鳬ʱ.

	m_hDCTimerQueue = NULL;				// �����Ӷ�ʱ������.
	m_hSessionTimerQueue = NULL;		// �Ự��ʱ��ʱ������
	m_hSpeedTimerQueue = NULL;			// �ٶ����ƶ�ʱ������.

	// ��ʼ��ͬ������
	InitializeCriticalSection(&m_cs);
}

CHTTPServer::~CHTTPServer()
{
	ASSERT(m_sockListen == INVALID_SOCKET);
	ASSERT(m_sockNewClient == INVALID_SOCKET);
	ASSERT(m_pServiceInf == NULL);

	delete []m_pAcceptBuf;
	DeleteCriticalSection(&m_cs);
}

BOOL CHTTPServer::InitWinsock(WORD nMainVer, WORD nSubVer)
{
	WORD wVer;
	WSADATA ws;
	wVer = MAKEWORD(nMainVer, nSubVer);
	return WSAStartup(wVer, &ws) == 0;
}

BOOL CHTTPServer::CleanWinsock()
{
	return WSACleanup() == 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

PCLIENTINF CHTTPServer::newClientDesc(SOCKET hSocket, const std::wstring &strIP, unsigned int nPort)
{
	PCLIENTINF pClientInf = new CLIENTINF;
	ASSERT(pClientInf);
	if( NULL == pClientInf ) return NULL;

	pClientInf->pInstant = (void *)this;
	pClientInf->nSended = 0;
	pClientInf->dwLastSent = 0;
	pClientInf->dwRecved = 0;
	pClientInf->hSocket = hSocket;
	pClientInf->nOprationType = opp_none;
	//pClientInf->pBuff = new char[MAX_SOCKBUFF + 1]; ASSERT(pClientInf->pBuff);
	//memset(pClientInf->pBuff, 0, MAX_SOCKBUFF + 1);
	pClientInf->WSABuf.buf = new char[MAX_SOCKBUFF + 1];
	memset(pClientInf->WSABuf.buf, 0, MAX_SOCKBUFF + 1);
	pClientInf->WSABuf.len = MAX_SOCKBUFF;
	memset(&pClientInf->Overlapped, 0, sizeof(WSAOVERLAPPED));
	
	pClientInf->pRequest = new CHTTPRequest(strIP.c_str(), nPort); // ����HTTP Request ����.
	pClientInf->pResponse = new CHTTPResponse(strIP.c_str(), nPort); // ����HTTP Response ����

	pClientInf->dwStartTime = GetTickCount(); // ��¼ʱ��.
	pClientInf->dwEndTime = pClientInf->dwStartTime;
	pClientInf->dwLastActiveTime = pClientInf->dwStartTime;

	ASSERT(strIP.size() <= MAX_IP_LENGTH);
	wcscpy(pClientInf->pszIP, strIP.c_str()); // ��¼Զ��IP�Ͷ˿�.
	pClientInf->nPort = nPort;

	// ���ó�ʱ������
	pClientInf->hDeadConnectionTimeout = NULL;
	pClientInf->hSessionTimeout = NULL;
	pClientInf->hSpeedtimeout = NULL;

	return pClientInf;
}

void CHTTPServer::deleteClientDesc(PCLIENTINF pClientInf, BOOL bSkeepTimer)
{
	if(NULL == pClientInf) 
	{
		ASSERT(0);
		return;
	}

	// ������Դ.һ��Ҫ��ֹͣ��ʱ��,�ٻ�����Դ.
	if(!bSkeepTimer)
	{
		if(pClientInf->hDeadConnectionTimeout != NULL)
		{
			// INVALID_HANDLE_VALUE ��ȴ�OnTimeout()��������,�����ʱOnTimeout()�������еĻ�
			if(!DeleteTimerQueueTimer(m_hDCTimerQueue, pClientInf->hDeadConnectionTimeout, INVALID_HANDLE_VALUE))
			{
				LOGGER_CERROR(theLogger, _T("�޷�ɾ�������Ӷ�ʱ��,������[%d].\r\n"), GetLastError());
			}
		}
		if(pClientInf->hSessionTimeout != NULL)
		{
			if(!DeleteTimerQueueTimer(m_hSessionTimerQueue, pClientInf->hSessionTimeout, INVALID_HANDLE_VALUE))
			{
				LOGGER_CERROR(theLogger, _T("�޷�ɾ���Ự��ʱ��,������[%d].\r\n"), GetLastError());
			}
		}
		if(pClientInf->hSpeedtimeout != NULL)
		{
			if(!DeleteTimerQueueTimer(m_hSpeedTimerQueue, pClientInf->hSpeedtimeout, INVALID_HANDLE_VALUE))
			{
				LOGGER_CERROR(theLogger, _T("�޷�ɾ���ٶ����ƶ�ʱ��,������[%d].\r\n"), GetLastError());
			}
		}
	}
	else
	{
		// doStop() ǿ��ɾ��ʱ���е�����.
		// ���еĶ�ʱ���Ѿ��ȱ�ɾ����.
	}

	if(pClientInf->pRequest) delete pClientInf->pRequest;
	if(pClientInf->pResponse) delete pClientInf->pResponse;
	if(pClientInf->hSocket != INVALID_SOCKET && pClientInf->nOprationType != opp_dead && pClientInf->nOprationType != opp_session_timeout)
	{
		// �����ӻ��߻Ự��ʱ�����ӵ��׽����Ѿ��ر�
		shutdown(pClientInf->hSocket, SD_BOTH);
		closesocket(pClientInf->hSocket);
	}
	//if(pClientInf->pBuff) delete []pClientInf->pBuff;
	if(pClientInf->WSABuf.buf) delete []pClientInf->WSABuf.buf;
	delete pClientInf;
}

// ׼�������׽���
HTTP_SERVER_ERROR_TYPE CHTTPServer::prepareListenSocket(int nPort, SOCKET& hListenSock)
{
	SOCKET hSock = INVALID_SOCKET;

	// �����׽���
	if( (hSock = socket(PF_INET, SOCK_STREAM, /*IPPROTO_TCP*/0 )) == INVALID_SOCKET )
	{
		return SE_CREATESOCK_FAILED;
	}

	// ����Ϊ������ģʽ
	u_long nonblock = 1;
	ioctlsocket(hSock, FIONBIO, &nonblock);
	
	// �󶨶˿�
	sockaddr_in addr;
	addr.sin_family			= AF_INET;
	addr.sin_port			= htons(nPort);
	addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	if( 0 != bind(hSock, (sockaddr *)&addr, sizeof(sockaddr_in)) )
	{
		closesocket(hSock);
		return SE_BIND_FAILED;
	}

	// ����
	if( 0 != listen(hSock, 10))
	{
		closesocket(hSock);
		return SE_LISTEN_FAILED;
	}
	else
	{
		hListenSock = hSock;
		return SE_SUCCESS;
	}
}

HTTP_SERVER_ERROR_TYPE CHTTPServer::Run(PHTTPSTARTDESC pStartDesc)
{
	ASSERT(pStartDesc);
	if(m_bRuning) return SE_RUNING;

	// ������ʱ���Ķ�ʱ��
	BOOL bTimersReady = FALSE;
	do 
	{
		if(pStartDesc->dwSessionTimeout > 0)
		{
			if(NULL == (m_hSessionTimerQueue = CreateTimerQueue()))
			{
				ASSERT(0);
				break;
			}
		}
		if(pStartDesc->dwDeadConnectionTimeout > 0)
		{
			if( NULL == (m_hDCTimerQueue = CreateTimerQueue()) )
			{
				ASSERT(0);
				break;
			}
		}
		if(pStartDesc->llMaxSpeed > 0)
		{
			if(NULL == (m_hSpeedTimerQueue = CreateTimerQueue()))
			{
				ASSERT(0);
				break;
			}
		}
		bTimersReady = TRUE;
	} while (false);
	if(!bTimersReady)
	{
		doStop();
		return SE_CREATETIMER_FAILED;
	}

	// ���������׽���
	HTTP_SERVER_ERROR_TYPE ret = prepareListenSocket(pStartDesc->nPort, m_sockListen);
	if(SE_SUCCESS != ret)
	{
		doStop();
		return ret;
	}

	// ��ȡCPU����
	int i = 0, j = 0;
	SYSTEM_INFO sysInfo;
	DWORD dwThreadId = 0;
	GetSystemInfo(&sysInfo);
	int nThreadCount = sysInfo.dwNumberOfProcessors + EXTHREAD;
	if( nThreadCount > 64 ) nThreadCount = 64; // һ����ɶ˿����������64���߳�.

	// ��¼״̬
	m_bRuning = TRUE;
	m_strWorkDir = pStartDesc->szRootDir;
	m_nListenPort = pStartDesc->nPort;
	m_nMaxConnections = pStartDesc->nMaxConnection;
	m_nMaxClientConn = pStartDesc->nMaxClientConn;
	m_bNavDir = pStartDesc->bNavDir;
	m_pStatusHandler = pStartDesc->pStatusHandler; // ����״̬֪ͨ�Ľӿ�
	m_dwDeadConnectionTimeout = pStartDesc->dwDeadConnectionTimeout;
	m_dwSessionTimeout = pStartDesc->dwSessionTimeout;
	m_llMaxSpeed = pStartDesc->llMaxSpeed;

	m_SockInfMap.clear();
	m_ClientIPMap.clear();
	m_vecDeftFileNames.clear();

	// ����Ĭ���ļ����б�
	std::wstring strDftFileNames(pStartDesc->szDefaultFileName);
	strDftFileNames.push_back(L','); // ����һ������,����ѭ������.
	std::wstring::size_type st = 0;
	std::wstring::size_type stNext = 0;
	while( (stNext = strDftFileNames.find(L',', st)) != std::wstring::npos )
	{
		if(stNext > st)
		{
			std::wstring strDftFileName =  strDftFileNames.substr(st, stNext - st);
			m_vecDeftFileNames.push_back(strDftFileName);
		}

		// ������һ���ļ�
		st = stNext + 1;
	}

	// �������������,��������Ӧ�ķ����߳�
	m_pServiceInf = new SVRINF;
	ASSERT(m_pServiceInf);
	m_pServiceInf->nSockCount = 0;
	m_pServiceInf->nThreadCount = nThreadCount; // ��¼���̵߳ĸ���.
	m_pServiceInf->hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, nThreadCount); // ������ɶ˿�.
	m_pServiceInf->pThread = new CWinThread* [m_pServiceInf->nThreadCount];
	for(int j = 0; j < m_pServiceInf->nThreadCount; ++j)
	{
		// ׼���̲߳���
		PARAM_SVRTHREAD *pThreadInf = new PARAM_SVRTHREAD;
		pThreadInf->hComplitionPort = m_pServiceInf->hCompletionPort;
		pThreadInf->nThreadIndex	= j;
		pThreadInf->pHttpserver		= this;

		// �����߳�.
		m_pServiceInf->pThread[j] = AfxBeginThread(ServiceProc, (LPVOID)pThreadInf, 0, 0, CREATE_SUSPENDED); //CreateThread(NULL, 0, ServiceProc, pThreadInf, 0, &dwThreadId);
		ASSERT(m_pServiceInf->pThread[j]);
		m_pServiceInf->pThread[j]->m_bAutoDelete = FALSE;
		m_pServiceInf->pThread[j]->ResumeThread();
	}

	// ���������׽��ֵ���ɶ˿�,����ִ�е�һ��accept()
	if(m_pServiceInf->hCompletionPort != CreateIoCompletionPort((HANDLE)m_sockListen, m_pServiceInf->hCompletionPort, (DWORD)m_sockListen, 0))
	{
		LOGGER_CERROR(theLogger, _T("�޷����������׽��ֵ���ɶ˿�,�������޷�����,������[%d].\r\n"), GetLastError());
		doStop();
		return SE_CREATE_IOCP_FAILED;
	}
	else
	{
		doAccept();
	}

	// ��־
	//LOGGER_CTRACE(theLogger, _T("�����߳̿�ʼ����(%d�������߳�,CPU����:%d)...\r\n"), nThreadCount, sysInfo.dwNumberOfProcessors);
	return SE_SUCCESS;
}

void CHTTPServer::doStop()
{
	int nThreadCount = 0;
	DWORD dwRet = 0;
	HANDLE *pThreadArr = NULL;

	// ����Ϣ��ÿ�������߳�,Ȼ��ȴ�ÿһ�������߳�ֹͣ
	// �����߳�ֹͣ��,���е������¼������ٴ���.
	if(m_pServiceInf)
	{
		nThreadCount = m_pServiceInf->nThreadCount;
		pThreadArr = new HANDLE[nThreadCount];
		for( int ii = 0; ii < nThreadCount; ++ii)
		{
			pThreadArr[ii] = m_pServiceInf->pThread[ii]->m_hThread;
			PostQueuedCompletionStatus(m_pServiceInf->hCompletionPort, 0, 0, NULL);		// 3��������Ϊ0ʱ,��ʾ�˳�. ÿ���̴߳���һ����
		}

		// MAXIMUM_WAIT_OBJECTS = 64; ����,ÿ�������������е��߳����� 64��.
		dwRet = WaitForMultipleObjects(nThreadCount, pThreadArr, TRUE, INFINITE);     // �����˳���־��,�ȴ������߳��˳�
		if(dwRet == WAIT_TIMEOUT)
		{
			// ��ʱ,�����л�δ�˳����߳�ǿ�ƽ���.
		}
		for( int ii = 0; ii < nThreadCount; ++ii)
		{
			if(m_pServiceInf->pThread[ii] != NULL)
			{
				delete m_pServiceInf->pThread[ii];
				m_pServiceInf->pThread[ii] = NULL;
			}
		}
		delete[] pThreadArr;
	}

	// �رն�ʱ������,ȡ�����еĶ�ʱ��,��ȷ���ص��������ٱ�����.
	// Ȼ�����ͷſͻ������ľ��ǰ�ȫ��.
	if(m_hDCTimerQueue != NULL)
	{
		if( !DeleteTimerQueueEx(m_hDCTimerQueue, INVALID_HANDLE_VALUE) )
		{
			LOGGER_CERROR(theLogger, _T("�޷�ɾ�������Ӷ�ʱ������,������[%d].\r\n"), GetLastError());
		}
		m_hDCTimerQueue = NULL;
	}
	if(m_hSessionTimerQueue != NULL)
	{
		if( !DeleteTimerQueueEx(m_hSessionTimerQueue, INVALID_HANDLE_VALUE) )
		{
			LOGGER_CERROR(theLogger, _T("�޷�ɾ���Ự��ʱ������,������[%d].\r\n"), GetLastError());
		}
		m_hSessionTimerQueue = NULL;
	}
	if(m_hSpeedTimerQueue != NULL)
	{
		if( !DeleteTimerQueueEx(m_hSpeedTimerQueue, INVALID_HANDLE_VALUE) )
		{
			LOGGER_CERROR(theLogger, _T("�޷�ɾ���ٶ����ƶ�ʱ������,������[%d].\r\n"), GetLastError());
		}
		m_hSpeedTimerQueue = NULL;
	}

	// �ر������׽��ֺ�Ϊ���������Ӷ�׼�����׽���
	if(INVALID_SOCKET != m_sockListen)
	{
		closesocket(m_sockListen);
		m_sockListen = INVALID_SOCKET;
	}
	if(INVALID_SOCKET != m_sockNewClient)
	{
		shutdown(m_sockNewClient, SD_BOTH);
		closesocket(m_sockNewClient);
		m_sockNewClient = INVALID_SOCKET;
	}

	// ��������߳�������
	// �ر���ɶ˿�,ʹ����I/O����ȡ���Ա㰲ȫɾ�� LPWSAOVERLAPPED ָ��.
	if(m_pServiceInf)
	{
		delete[] m_pServiceInf->pThread;
		m_pServiceInf->pThread = NULL;
		m_pServiceInf->nThreadCount = 0;
		CloseHandle(m_pServiceInf->hCompletionPort); 
		m_pServiceInf->hCompletionPort = NULL;
		delete m_pServiceInf;
		m_pServiceInf = NULL;
	}

	// �������ͷ�ʣ�µ��׽��ֺ͹�������,��տͻ�IP��
	// ����Ҫ����,���е�����,���ж�ʱ��,�̶߳��Ѿ�ֹͣ����,���ٻ�����Դ����.
	if(m_SockInfMap.size() > 0)
	{
		LOGGER_CWARNING(theLogger, _T("�˳�ʱ����:[%d]������,��ǿ�ƹر�.\r\n"), m_SockInfMap.size());
	}
	for(sockinf_iter iter = m_SockInfMap.begin(); iter != m_SockInfMap.end(); ++iter)
	{
		// ���е�Timer�Ѿ��� DeleteTimerQueueEx()�б�ɾ��,���ﲻ����ɾ��Timer��
		deleteClientDesc(iter->second, TRUE);
	}
	m_SockInfMap.clear();
	m_ClientIPMap.clear();

	// ���״̬�ӿں�Ĭ���ļ����б�
	m_pStatusHandler = NULL;
	m_vecDeftFileNames.clear();
}

HTTP_SERVER_ERROR_TYPE CHTTPServer::Stop()
{
	if(!m_bRuning) return SE_STOPPED;

	doStop();

	// �����־
	m_bRuning = FALSE;
	return SE_SUCCESS;
}

/*
int CHTTPServer::OnAccept()
{
	// ���� accept()
	sockaddr_in clientAddr;
	int nLen = sizeof(sockaddr_in);
	SOCKET hNewSocket = accept(m_sockListen, (sockaddr*)&clientAddr, &nLen);
	if( INVALID_SOCKET == hNewSocket )
	{
		// accept()ʧ��,��������DOWN���������׽��ֱ��ر�.
		LOGGER_CERROR(theLogger, _T("�޷������µ�����,���������������׽��ֱ��ر�,����������ϵͳ����.������[%d]\r\n"), WSAGetLastError());
		return ON_ACCEPT_CLOSED;
	}

	// ���Զ��IP�Ͷ˿�
	std::wstring strClientIP = AtoW(inet_ntoa(clientAddr.sin_addr)).c_str();
	unsigned int nClientPort = ntohs(clientAddr.sin_port);

	//int nRet = 4; // ��ʼ״̬: 0 ��ʾ���ӱ�����; 1��ʾ������ɶ˿�ʧ��; 2��ʾWSARecvʧ��; 3��ʾ��ʱ������ʧ��; 4��ʾ������æ;
	//DWORD dwRecvBytes = 0,dwFlags = 0, dwErr = 0; 
	PCLIENTINF pClientInf = NULL;
	BOOL bAccepted = FALSE;
	
	// ��ӵ��б���.
	lock();
	do 
	{
		if(m_pServiceInf->nSockCount < m_nMaxConnections)
		{
			// ��������ɶ˿�.
			HANDLE hRet = CreateIoCompletionPort((HANDLE)hNewSocket, m_pServiceInf->hCompletionPort, (DWORD)hNewSocket, 0);
			if(hRet != m_pServiceInf->hCompletionPort)
			{
				LOGGER_CERROR(theLogger, _T("��������ɶ˿�ʧ��.[%s][%d],�׽��ֽ����ر�.\r\n"), strClientIP.c_str(), nClientPort);
				break;
			}
			else
			{
				// �����ɹ�, ׼���ͻ�����������
				pClientInf = newClientDesc(hNewSocket, strClientIP, nClientPort);

				// ���ó�ʱ������
				if(m_dwDeadConnectionTimeout > 0)
				{
					ASSERT(m_hTimerQueue);
					if(!CreateTimerQueueTimer(&(pClientInf->hDeadConnectionTimeout), m_hTimerQueue, &(CHTTPServer::DeadConnectionTimeoutCallback), (PVOID)pClientInf, m_dwDeadConnectionTimeout, 0, 0))
					{
						LOGGER_CERROR(theLogger, _T("�޷����������ӳ�ʱ������,������[%d].\r\n"), GetLastError());
						break;
					}
				}
				
				if(m_dwSessionTimeout > 0)
				{
					ASSERT(m_hTimerQueue);
					if(!CreateTimerQueueTimer(&(pClientInf->hSessionTimeout), m_hTimerQueue, &(CHTTPServer::SessionTimeoutCallback), (PVOID)pClientInf, m_dwSessionTimeout, 0, 0))
					{
						LOGGER_CERROR(theLogger, _T("�޷������Ự��ʱ������,������[%d].\r\n"), GetLastError());
						break;
					}
				}

				// �����ɹ���, ִ��һ�ν��ն���.
				// ������ LPWSAOVERLAPPED �ṹ, WSARecv() ����������,�������׽����� ����ģʽ���Ƿ�����ģʽ.
				pClientInf->WSABuf.len = MAX_SOCKBUFF;
				if(!doRecv(pClientInf))
				{
					LOGGER_CERROR(theLogger, _T("�޷����տͻ�������,����ֵ[%d], �׽���[%s][%d]�����ر�.\r\n"), WSAGetLastError(), strClientIP.c_str(), nClientPort);
					break;
				}
				
				// ��һ�����ݰ����ղ����ɹ���,��ʽ�����б�.
				m_pServiceInf->nSockCount++;
				m_SockInfMap.insert(std::make_pair(hNewSocket, pClientInf));
				bAccepted = TRUE;
			}
		}
	} while(false);
	unlock();

	if(bAccepted)
	{
		// ���ӱ�����,�������Դָ���Ѿ���������.
	}
	else
	{
		// ����û�б�����,������Դ.
		if( pClientInf != NULL ) deleteClientDesc(pClientInf);
	}

	if(m_pStatusHandler)
	{
		m_pStatusHandler->OnNewConnection(strClientIP.c_str(), nClientPort, !bAccepted);
	}

	return bAccepted ? ON_ACCEPT_SUCCESS : ON_ACCEPT_BUSY;
}
*/

BOOL CHTTPServer::OnAcceptEx(BOOL bSkeep)
{
	// �����ʧ�ܵ� AcceptEx()���÷���,ֻ���Զ�������������
	if(bSkeep)
	{
		LOGGER_CWARNING(theLogger, _T("��׽��һ��ʧ�ܵ�AcceptEx����,������,������[%d].\r\n"), WSAGetLastError());
		shutdown(m_sockNewClient, SD_BOTH);
		closesocket(m_sockNewClient);
		m_sockNewClient = INVALID_SOCKET;
		doAccept();
		return TRUE;
	}

	// ��ȡ�ͻ�IP����Ϣ
	// �����׽�����Ϣ,ʹ getsockname() �� getpeername() ����.
	if( 0 != setsockopt( m_sockNewClient, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&m_sockListen, sizeof(m_sockListen)) )
	{
		LOGGER_CERROR(theLogger, _T("�޷������׽�����Ϣ,������[%d].\r\n"), WSAGetLastError());
	}
	sockaddr_in clientAddr;
	int nAddrLen = sizeof(sockaddr_in);
	if( 0 != getpeername(m_sockNewClient, (sockaddr *)&clientAddr, &nAddrLen) )
	{
		LOGGER_CERROR(theLogger, _T("�޷���ȡ�ͻ��˵�ַ�Ͷ˿�,������[%d].\r\n"), WSAGetLastError());
	}

	// ���Զ��IP�Ͷ˿�
	std::wstring strClientIP = AtoW(inet_ntoa(clientAddr.sin_addr)).c_str();
	unsigned int nClientPort = ntohs(clientAddr.sin_port);

	// ��ӵ��б���.
	PCLIENTINF pClientInf = NULL;
	BOOL bAccepted = FALSE;
	SOCKET hNewSocket = m_sockNewClient;
	BOOL bKicked = FALSE;

	lock();
	do 
	{
		// �鿴�Ƿ��Ѿ��ﵽ�������.
		if(m_pServiceInf->nSockCount < m_nMaxConnections)
		{
			// �鿴��IP�Ƿ��Ѿ��ﵽ���������
			string_int_map::iterator iterClientIP = m_ClientIPMap.end();
			if(m_nMaxClientConn > 0)
			{
				iterClientIP = m_ClientIPMap.find(strClientIP);
				if(iterClientIP != m_ClientIPMap.end() && iterClientIP->second >= m_nMaxClientConn)
				{
					bKicked = TRUE;
					break; // �����������.
				}
			}

			// ��������ɶ˿�.
			HANDLE hRet = CreateIoCompletionPort((HANDLE)hNewSocket, m_pServiceInf->hCompletionPort, (DWORD)hNewSocket, 0);
			if(hRet != m_pServiceInf->hCompletionPort)
			{
				LOGGER_CERROR(theLogger, _T("[%s:%d] - ��������ɶ˿�ʧ��,�׽��ֽ����ر�.\r\n"), strClientIP.c_str(), nClientPort);
				break;
			}
			else
			{
				// �����ɹ�, ׼���ͻ�����������
				pClientInf = newClientDesc(hNewSocket, strClientIP, nClientPort);

				// ���ó�ʱ������
				if(m_dwDeadConnectionTimeout > 0)
				{
					ASSERT(m_hDCTimerQueue);
					if(!CreateTimerQueueTimer(&(pClientInf->hDeadConnectionTimeout), m_hDCTimerQueue, &(CHTTPServer::DeadConnectionTimeoutCallback), (PVOID)pClientInf, m_dwDeadConnectionTimeout, 0, 0))
					{
						LOGGER_CERROR(theLogger, _T("�޷����������ӳ�ʱ������,������[%d].\r\n"), GetLastError());
						break;
					}
				}

				if(m_dwSessionTimeout > 0)
				{
					ASSERT(m_hSessionTimerQueue);
					if(!CreateTimerQueueTimer(&(pClientInf->hSessionTimeout), m_hSessionTimerQueue, &(CHTTPServer::SessionTimeoutCallback), (PVOID)pClientInf, m_dwSessionTimeout, 0, 0))
					{
						LOGGER_CERROR(theLogger, _T("�޷������Ự��ʱ������,������[%d].\r\n"), GetLastError());
						break;
					}
				}

				// �����ɹ���, ִ��һ�ν��ն���.
				pClientInf->WSABuf.len = MAX_SOCKBUFF; // ��ʼ����
				if(!doRecv(pClientInf))
				{
					LOGGER_CERROR(theLogger, _T("[%s:%d] - �޷����տͻ�������,������[%d],�׽��ֽ����ر�.\r\n"), strClientIP.c_str(), nClientPort, WSAGetLastError());
					break;
				}

				// ��һ�����ݰ����ղ����ɹ���,��ʽ�����б�.
				m_pServiceInf->nSockCount++;
				m_SockInfMap.insert(std::make_pair(hNewSocket, pClientInf));
				if(m_nMaxClientConn > 0)
				{
					// ��¼��ǰ�ͻ�һ���Ѿ��ж��ٸ�������.
					if(iterClientIP != m_ClientIPMap.end()) iterClientIP->second++;
					else m_ClientIPMap.insert(std::make_pair(strClientIP, 1));
				}
				bAccepted = TRUE;
			}
		}
	} while(false);
	unlock();

	// �鿴���,��������Դ,��Ҫ������¿ͻ���û�б�����,��ôӦ�ùر�����׽���.
	if(bAccepted)
	{
		// ���ӱ�����,�������Դָ���Ѿ���������.
	}
	else
	{
		// ����û�б�����,������Դ.
		if( pClientInf != NULL )
		{
			deleteClientDesc(pClientInf);
		}
		else
		{
			shutdown(hNewSocket, SD_BOTH);
			closesocket(hNewSocket);
		}
	}
	
	// ׼��������һ������
	doAccept();

	// ����״̬
	if(m_pStatusHandler)
	{
		m_pStatusHandler->OnNewConnection(strClientIP.c_str(), nClientPort, !bAccepted, bKicked);
	}
	return bAccepted;
}

int CHTTPServer::OnClose(PCLIENTINF pClientInf, REMOVE_REASON reason)
{
	PCLIENTINF pSockInf = pClientInf;

	// ��¼��Ҫ������.
	__int64 nSended = pClientInf->nSended;
	DWORD dwStartTime = pClientInf->dwStartTime;
	DWORD dwEndTime = GetTickCount();
	std::wstring strClientIP(pClientInf->pszIP);
	unsigned int nClientPort = pClientInf->nPort;

	// �Ӷ��������,�������IP��
	lock();
	m_SockInfMap.erase(pClientInf->hSocket);
	m_pServiceInf->nSockCount--;

	if(m_nMaxClientConn > 0)
	{
		string_int_map::iterator iter = m_ClientIPMap.find(strClientIP);
		if(iter != m_ClientIPMap.end())
		{
			if( --(iter->second) <= 0) m_ClientIPMap.erase(iter);
		}
	}
	unlock();

	// ������Դ
	deleteClientDesc(pClientInf);
	
	// ״̬����
	if(m_pStatusHandler)
	{
		m_pStatusHandler->OnConnectionClosed(strClientIP.c_str(), nClientPort, reason, nSended, (dwEndTime - dwStartTime));
	}

	return 0;
}

//HRESULT CHTTPServer::Notify(HTTP_MESSAGE_TYPE msgType, PHTTP_MSG pMsg)
//{
//	if(m_hNotifyWnd != NULL)
//	{
//		return SendMessage(m_hNotifyWnd, m_uMessageID, (WPARAM)msgType, (LPARAM)pMsg);
//
//		PHTTP_MSG pNewMsg = new HTTP_MSG;
//		pNewMsg->nClientPort = pMsg->nClientPort;
//		pNewMsg->nValue = pMsg->nValue;
//		pNewMsg->strClientIP = pMsg->strClientIP;
//		pNewMsg->strValue = pMsg->strValue;
//		return SendNotifyMessage(m_hNotifyWnd, m_uMessageID, (WPARAM)msgType, (LPARAM)pNewMsg);
//	}
//	return 0;
//}

//UINT __cdecl CHTTPServer::ListenProc(LPVOID pParam)
//{
//	// ���Ʋ���
//	PARAM_LISTENTHREAD ListenInf;
//	memcpy(&ListenInf, pParam, sizeof(PARAM_LISTENTHREAD));
//	delete (PARAM_LISTENTHREAD *)pParam;
//
//	SOCKET sockArr[10];
//	WSAEVENT eventArr[10];
//	DWORD nIndex = 0;
//	BOOL bStop = FALSE;
//	WSANETWORKEVENTS netevent;
//	sockaddr_in addrNew;
//	int nSize;
//	int nSockRet = 0;
//	HTTP_ERROR err;
//
//	// �����׽���, ��, ����
//	sockArr[0] = INVALID_SOCKET;
//	sockArr[0] = socket(PF_INET, SOCK_STREAM, /*IPPROTO_TCP*/0);
//	if(sockArr[0] == INVALID_SOCKET)
//	{
//		err.bSuccess = FALSE;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_CREATESOCKET, &err);
//		return 1;
//	}
//	else
//	{
//		err.bSuccess = TRUE;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_CREATESOCKET, &err);
//	}
//
//	sockaddr_in addr;
//	addr.sin_family			= AF_INET;
//	addr.sin_port			= htons(ListenInf.nPort);
//	addr.sin_addr.s_addr	= htonl(INADDR_ANY);
//	nSockRet = bind(sockArr[0], (sockaddr *)&addr, sizeof(sockaddr_in));
//	if(nSockRet)
//	{
//		err.bSuccess = FALSE;
//		err.nValue = nSockRet;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_BIND, &err);
//		return 1;
//	}
//	else
//	{
//		err.bSuccess = TRUE;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_BIND, &err);
//	}
//
//	nSockRet = listen(sockArr[0], 10);
//	if(nSockRet)
//	{
//		err.bSuccess = FALSE;
//		err.nValue = nSockRet;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_LISTEN, &err);
//		return 1;
//	}
//	else
//	{
//		err.bSuccess = TRUE;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_LISTEN, &err);
//	}
//
//	// �� WAS EventIOģ��, ���������׽���, ��select() Ҳ��ȫ����. 
//	eventArr[0] = WSACreateEvent();
//	if( WSA_INVALID_EVENT == eventArr[0] )
//	{
//		err.bSuccess = FALSE;
//		err.nValue = WSAGetLastError();
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_CREATEEVENT, &err);
//		return 1;
//	}
//	else
//	{
//		err.bSuccess = TRUE;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_CREATEEVENT, &err);
//	}
//	
//	if( SOCKET_ERROR == WSAEventSelect(sockArr[0], eventArr[0], FD_ACCEPT) )
//	{
//		err.bSuccess = FALSE;
//		err.nValue = WSAGetLastError();
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_SELECT, &err);
//		return 1;
//	}
//	else
//	{
//		err.bSuccess = TRUE;
//		ListenInf.pHttpserver->Notify(SE_LISTENPROC_SELECT, &err);
//	}
//
//	while(1)
//	{
//		nIndex = WSAWaitForMultipleEvents(1, eventArr, TRUE, ListenInf.dwTimeOut, TRUE);
//
//		// �˳���־
//		if ( ListenInf.pHttpserver->IsStopping() )
//		{
//			closesocket(sockArr[0]);
//			WSACloseEvent(eventArr[0]);
//			return 0;
//		}
//
//		// ��������
//		else if(nIndex == WSA_WAIT_EVENT_0)
//		{
//			if( SOCKET_ERROR == WSAEnumNetworkEvents(sockArr[0], eventArr[0], &netevent))
//			{
//				// ���Դ���,���� WSAWaitForMultipleEvents() Ҳ�᷵��ͬ���Ĵ���.
//				TRACE("WSAEnumNetworkEvents()����ʧ��[%d]\r\n", WSAGetLastError());
//			}
//			else
//			{
//				if(netevent.lNetworkEvents & FD_ACCEPT)
//				{
//					if(netevent.iErrorCode[FD_ACCEPT_BIT] != 0)
//					{
//					}
//					else
//					{
//						nSize = sizeof(sockaddr_in);
//						SOCKET sockNew = accept(sockArr[0], (sockaddr *)&addrNew, &nSize);
//						if(sockNew == INVALID_SOCKET)
//						{
//							err.bSuccess = FALSE;
//							err.bSuccess = WSAGetLastError();
//							ListenInf.pHttpserver->Notify(SE_LISTENPROC_ACCEPT, &err);
//						}
//						else
//						{
//							ListenInf.pHttpserver->OnAccept(sockNew);
//						}
//					}
//				}
//			}
//		}
//
//		// ��������ʧ��
//		else if( WSA_WAIT_FAILED == nIndex)
//		{
//			err.bSuccess = FALSE;
//			err.nValue = WSAGetLastError();
//			if( WSAENETDOWN == err.nValue )
//			{
//				// ����Ƕ����Ļ�, 5������¼��.
//				ListenInf.pHttpserver->Notify(SE_LISTENPROC_WAIT, &err);
//				Sleep(5 * 1000);
//			}
//			else
//			{
//				// �������������,�˳�.
//				ListenInf.pHttpserver->Notify(SE_LISTENPROC_WAIT, &err);
//				closesocket(sockArr[0]);
//				WSACloseEvent(eventArr[0]);
//				return 1;
//			}
//		}
//
//		// �ȴ���ʱ.
//		else
//		{
//			// nIndex == WSA_WAIT_TIMEOUT;
//		}
//
//		// �����¼�
//		WSAResetEvent(eventArr[0]);
//	}
//
//	// ��Ӧ�ô������˳�.
//	ASSERT(FALSE);
//	return 0;
//}

/*
UINT __cdecl CHTTPServer::ListenProc(LPVOID pParam)
{
	LOGGER_CTRACE(theLogger, _T("�����߳̿�ʼ����.\r\n"));

	// ���Ʋ���
	PARAM_LISTENTHREAD ListenInf;
	memcpy(&ListenInf, pParam, sizeof(PARAM_LISTENTHREAD));
	delete (PARAM_LISTENTHREAD *)pParam;

	fd_set readfds;
	
	//timeval timout;
	//timout.tv_sec = ListenInf.dwTimeOut / 1000;
	//timout.tv_usec = 0;
	//LOGGER_CTRACE(theLogger, _T("�����߳̿�ʼ����...\r\n"));

	while(TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(ListenInf.hSock, &readfds);

		int nSockRet = select(0, &readfds, NULL, NULL, NULL);

		if(nSockRet == SOCKET_ERROR) // �رջ�����ʲô���ⷢ��.
		{
			LOGGER_CERROR(theLogger, _T("�����̷߳������˳�,������[%d].\r\n"), WSAGetLastError());
			return 1;
			//break;
		}
		else if(nSockRet == 0) // time out
		{
			ASSERT(0); // ��Ӧ�ó�ʱ.
		}
		else // accpet() �������Ե�����.������listen�׽��ֱ��ر�/����.
		{
			ASSERT(FD_ISSET(ListenInf.hSock, &readfds));
			if( ON_ACCEPT_CLOSED == ListenInf.pHttpserver->OnAccept())
			{
				LOGGER_CTRACE(theLogger, _T("�����߳��˳�.\r\n"));
				return 0;
			}
		}
	}

	return 0;
}
*/
UINT __cdecl CHTTPServer::ServiceProc(LPVOID pParam)
{
	// ���Ʋ���
	PARAM_SVRTHREAD ServiceInf;
	memcpy(&ServiceInf, pParam, sizeof(PARAM_SVRTHREAD));
	delete (PARAM_SVRTHREAD *)pParam;
	TRACE("�߳�[%d]��ʼ����...\r\n", ServiceInf.nThreadIndex);
		
	while(true)
	{
		DWORD dwBytesTransferred = 0;
		SOCKET hSocket = INVALID_SOCKET;
		PCLIENTINF pSockInf = NULL;
		if(!GetQueuedCompletionStatus(ServiceInf.hComplitionPort, &dwBytesTransferred, (DWORD *)&hSocket, (LPOVERLAPPED*)&pSockInf, INFINITE))
		{
			if(pSockInf == NULL)
			{
				// ������δ֪����,������Ҫ�����������ܽ��
				LOGGER_CERROR(theLogger, _T("�����߳�[%d]������ֹͣ,��Ҫ���������������޸�,������[%d].\r\n"), ServiceInf.nThreadIndex, GetLastError());
				break;
			}
			else
			{
				if(hSocket == ServiceInf.pHttpserver->m_sockListen)
				{
					// �����׽���,IOʧ��,����.
					ServiceInf.pHttpserver->OnAcceptEx(TRUE);
				}
				else
				{
					// �����׽��� IO����ʧ��. 
					// 1.Server���ڳ�ʱ��ԭ��,�����ر����׽��ֵ��µ�ǰ����ִ�е�IOʧ��.
					// 2.�ͻ��˹ر����׽���.
					REMOVE_REASON rr = RR_UNKNOWN;
					if(pSockInf->nOprationType == opp_dead)
					{
						rr = RR_DEAD;
					}
					else if(pSockInf->nOprationType == opp_session_timeout)
					{
						rr = RR_SESSION_TIMEOUT;
					}
					else
					{
						rr = RR_CLIENTCLOSED;
					}
					ServiceInf.pHttpserver->OnClose(pSockInf, rr);
				}
			}
		}
		else
		{
			// ����Լ���õ��˳���־
			if(dwBytesTransferred == 0 && hSocket == 0 && pSockInf == NULL)
			{
				break;
			}

			if(hSocket == ServiceInf.pHttpserver->m_sockListen)
			{
				// �����׽��ֿɶ�
				ServiceInf.pHttpserver->OnAcceptEx(FALSE);
			}
			else
			{
				// �����׽���,��������
				// ����һ���׽���,�κ�ʱ��ֻ��һ����������ڽ���.������߼���֤����һ��,����,���Է��ĵ�ʹ��pSockInfָ��.
				ASSERT(pSockInf);
				if(dwBytesTransferred == 0)
				{
					// һ�����û������������ɹ�ȴû������.
					ASSERT(0);
					ServiceInf.pHttpserver->OnClose(pSockInf, RR_UNKNOWN);
				}
				else
				{
					ASSERT(pSockInf->hSocket == hSocket);
					if(pSockInf->nOprationType == opp_recv)
					{
						// ���ղ���.
						ServiceInf.pHttpserver->OnRecv(pSockInf, dwBytesTransferred);
					}
					else if(pSockInf->nOprationType == opp_send)
					{
						// ���Ͳ������
						ServiceInf.pHttpserver->OnSend(pSockInf, dwBytesTransferred);
					}
					else
					{
						// ������ͻ��߽��ղ����Ѿ�ִ����һ����,������Ϊ��ʱ�������׽��ֱ������������ر�
						REMOVE_REASON rr = RR_UNKNOWN;
						if(pSockInf->nOprationType == opp_dead)
						{
							rr = RR_DEAD;
						}
						else if(pSockInf->nOprationType == opp_session_timeout)
						{
							rr = RR_SESSION_TIMEOUT;
						}
						else
						{
							rr = RR_CLIENTCLOSED;
							ASSERT(0);
						}
						ServiceInf.pHttpserver->OnClose(pSockInf, rr);
					}
				} // dwBytesTransferred != 0
			} // �Ƿ��������׽���
		} // GetQueuedCompletionStatus() return

		//TRACE("�����߳�[%d]������һ��I/O�¼�.\r\n", ServiceInf.nThreadIndex);
	} // while

	TRACE("����[%d]�߳��˳�.\r\n", ServiceInf.nThreadIndex);
	return 0;
}

VOID CALLBACK CHTTPServer::DeadConnectionTimeoutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	// ֻҪ��ʱ������������,��ô pClientInf ָ��һ������Ч��.
	PCLIENTINF pClientInf = (PCLIENTINF)lpParameter;
	CHTTPServer *pServer = (CHTTPServer *)pClientInf->pInstant;
	pServer->OnDeadConnectionDetected(pClientInf);
}

VOID CALLBACK CHTTPServer::SessionTimeoutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	// ֻҪ��ʱ������������,��ô pClientInf ָ��һ������Ч��.
	PCLIENTINF pClientInf = (PCLIENTINF)lpParameter;
	CHTTPServer *pServer = (CHTTPServer *)pClientInf->pInstant;
	pServer->OnSessionTimeout(pClientInf);
}

VOID CALLBACK CHTTPServer::SpeedTimeoutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	// ֻҪ��ʱ������������,��ô pClientInf ָ��һ������Ч��.
	PCLIENTINF pClientInf = (PCLIENTINF)lpParameter;
	CHTTPServer *pServer = (CHTTPServer *)pClientInf->pInstant;
	pServer->OnDelaySend(pClientInf);
}

BOOL CHTTPServer::doSend(PCLIENTINF pClientInf)
{
	pClientInf->nOprationType = opp_send;
	DWORD dwTransfered = 0;
	DWORD dwFlags = 0;
	if(SOCKET_ERROR == WSASend(pClientInf->hSocket, &pClientInf->WSABuf, 1, &dwTransfered, dwFlags, (WSAOVERLAPPED *)pClientInf, NULL))
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			TRACE("WSASend Error:%d.\r\n",  WSAGetLastError());
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CHTTPServer::doRecv(PCLIENTINF pClientInf)
{
	pClientInf->nOprationType = opp_recv;
	DWORD dwTransfered = 0;
	DWORD dwFlags = 0;
	if(SOCKET_ERROR == WSARecv(pClientInf->hSocket, &(pClientInf->WSABuf), 1, &dwTransfered, &dwFlags, (LPWSAOVERLAPPED)pClientInf, NULL))
	{
		DWORD dwErr = WSAGetLastError();//WSAEFAULT
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			TRACE("WSARecv Error:%d.\r\n",  WSAGetLastError());
			return FALSE;
		}
	}
	return TRUE;
}

// WSARecv()�������
int CHTTPServer::OnRecv(PCLIENTINF pClientInf, DWORD dwBytesTransferred)
{
	ASSERT(pClientInf->nOprationType & opp_recv);

	//��¼�ܹ����յ��ֽ���
	DWORD dwLastActiveTimeBak = pClientInf->dwLastActiveTime;
	pClientInf->dwRecved += dwBytesTransferred;
	pClientInf->dwLastActiveTime = GetTickCount(); // ��¼���һ�λ�Ծʱ��.

	// ȡ�������Ӷ�ʱ��,�����ʱ��ʱ������������Ҳ��Ҫ��,�׽��ֱ��ر�.�����WSASend()WSARecv()��ʧ��.
	// ����Ǳ�������,�ٶȷǳ���,�п��ܳ��� dwLastActiveTimeBak == pClientInf->dwLastActiveTime �����.
	if(pClientInf->hDeadConnectionTimeout != NULL && dwLastActiveTimeBak != pClientInf->dwLastActiveTime)
	{
		ASSERT(m_hDCTimerQueue);
		if(!ChangeTimerQueueTimer(m_hDCTimerQueue, pClientInf->hDeadConnectionTimeout, m_dwDeadConnectionTimeout, 0))
		{
			LOGGER_CERROR(theLogger, _T("�޷����������Ӷ�ʱ��,������[%d].\r\n"), GetLastError());
			ASSERT(0);
		}
	}

	// ֪ͨ
	if(m_pStatusHandler)
	{
		m_pStatusHandler->OnDataReceived(pClientInf->pszIP, pClientInf->nPort, dwBytesTransferred);
	}

	// �ѽ��յ������ݴ�ѹ��Request����.
	if(dwBytesTransferred != pClientInf->pRequest->PushData(pClientInf->WSABuf.buf, dwBytesTransferred) || pClientInf->pRequest->IsEnd())
	{
		// ѹ������ʧ��(˵���������) ���� ѹ�����ݳɹ�,���Ѿ����յ�����������ͷ
		OnRequest(pClientInf);

		// �κ�һ������,������ͻ���һ����Ӧ. ��ʼ��������.
		pClientInf->WSABuf.len = 0;
		pClientInf->nSended = 0;

		// ���ͻ�Ӧ���ĵ�һ������
		DWORD dwSendTransfered = 0;
		int nPopSize = pClientInf->pResponse->PopData(pClientInf->WSABuf.buf, MAX_SOCKBUFF); ASSERT(nPopSize > 0);
		pClientInf->WSABuf.len = nPopSize; 
		if( !doSend(pClientInf) )
		{
			OnClose(pClientInf, RR_SEND_FAILED);
		}
	}
	else
	{
		// ����ѹ��ɹ�,������������
		pClientInf->WSABuf.len = MAX_SOCKBUFF;
		if(!doRecv(pClientInf))
		{
			OnClose(pClientInf, RR_RECV_FAILED);
		}
	}
	return 0;
}

int CHTTPServer::OnSend(PCLIENTINF pClientInf, DWORD dwBytesTransferred)
{
	ASSERT(pClientInf->nOprationType & opp_send);

	//// �ϸ�������ɵ���β��������,Ȼ����׼���¸��� ////
	// ��¼�ܹ����͵��ֽ���
	DWORD dwLastActiveTimeBak = pClientInf->dwLastActiveTime;
	pClientInf->nSended += dwBytesTransferred;
	pClientInf->dwLastActiveTime = GetTickCount();
	pClientInf->dwLastSent = dwBytesTransferred;

	// ֪ͨ״̬���սӿ�(�ӿ�ʵ��Ӧ�÷ǳ����ٵĴ�OnDataSent()����,������ܻᵼ�������ӳ�ʱ����.
	if(m_pStatusHandler)
	{
		m_pStatusHandler->OnDataSent(pClientInf->pszIP, pClientInf->nPort, dwBytesTransferred);
	}

	// ���һ���ڻ��������Ƿ�������
	if(dwBytesTransferred < pClientInf->WSABuf.len)
	{
		// ����һ��û�з���������Ƶ��������Ŀ�ͷ
		memmove(pClientInf->WSABuf.buf, pClientInf->WSABuf.buf + dwBytesTransferred, pClientInf->WSABuf.len - dwBytesTransferred);
		pClientInf->WSABuf.len -= dwBytesTransferred;
	}
	else
	{
		pClientInf->WSABuf.len = 0;
	}

	// ���������Ϊ��,����Response�����β,˵��ȫ�����ݶ��Ѿ��������, �Ͽ��ͻ��˵�����.
	if(pClientInf->WSABuf.len <= 0 && pClientInf->pResponse->IsEOF())
	{
		OnClose(pClientInf, RR_SENDCOMPLETE);
		return 0; 
	}
	
	// ����û�з�����,��������
	// ����Ƿ񳬳����ٶ�����
	unsigned long lMaxLength = MAX_SOCKBUFF; // ��������͵İ�����.
	DWORD dwDelay = 0; // ��ʱ���͵�ʱ��.
	if( m_llMaxSpeed > 0 )
	{
		// ��������ٶ�����,���� nSended�������Ӧ�����������ʱ������
		DWORD dwExpectTime = (DWORD)(pClientInf->nSended * 1.0 / m_llMaxSpeed * 1000 + pClientInf->dwStartTime);
		if(dwExpectTime > pClientInf->dwLastActiveTime) // ��ɵ�ʱ�����ǰ,˵���ٶȳ���
		{
			// �������ʱ��������ʱ��
			dwDelay = dwExpectTime - pClientInf->dwLastActiveTime;
			if(dwDelay > MAX_WAITTIME_ONSPEEDLIMITED)
			{
				// ������ȴ�ʱ��,��һ�η���һ����С��.
				dwDelay = MAX_WAITTIME_ONSPEEDLIMITED;
				lMaxLength = MIN_SIZE_ONSPEEDLIMITED;
			}
			else
			{
				// ��һ�ο��Է���һ������.
				lMaxLength = MAX_SOCKBUFF;
			}
		}
	}

	// ���������Ӷ�ʱ��
	// �����Ҫ��ʱ����,�������Ӷ�ʱ��Ӧ���Ӻ�.(�������ȴ���ʱ�䲻��������ʱ��.)
	if(pClientInf->hDeadConnectionTimeout != NULL && pClientInf->dwLastActiveTime != dwLastActiveTimeBak)
	{
		DWORD dwDCTime = (dwDelay == 0) ? m_dwDeadConnectionTimeout : (m_dwDeadConnectionTimeout + dwDelay + DCTIMEOUT_DELAY);
		ASSERT(m_hDCTimerQueue);
		if(!ChangeTimerQueueTimer(m_hDCTimerQueue, pClientInf->hDeadConnectionTimeout, dwDCTime, 0))
		{
			LOGGER_CERROR(theLogger, _T("�޷����������Ӷ�ʱ��,������[%d].\r\n"), GetLastError());
			ASSERT(0);
		}
	}

	// �� Response �����ж�ȡ���� ��仺����
	if( lMaxLength > pClientInf->WSABuf.len )
	{
		// ������������.
		int nPopSize = pClientInf->pResponse->PopData(pClientInf->WSABuf.buf + pClientInf->WSABuf.len, lMaxLength - pClientInf->WSABuf.len);
		pClientInf->WSABuf.len += nPopSize;
	}
	ASSERT(pClientInf->WSABuf.len > 0);

	// ִ�з���
	ASSERT(pClientInf->hSpeedtimeout == NULL);
	if(dwDelay == 0 || pClientInf->hSpeedtimeout != NULL)
	{
		if(!doSend(pClientInf))
		{
			// ������������ʱ��,�ͻ��˹ر�������,��ᵼ��WSASend()ʧ��.
			OnClose(pClientInf, RR_SEND_FAILED);
		}
	}
	else
	{
		// ��ʱ����,������һ�η��͵Ķ�ʱ��
		if( !CreateTimerQueueTimer(&pClientInf->hSpeedtimeout, m_hSpeedTimerQueue, &CHTTPServer::SpeedTimeoutCallback, pClientInf,  dwDelay, 0, 0))
		{
			ASSERT(0);
			LOGGER_CERROR(theLogger, _T("�޷������ٶ����Ƴ�ʱ������,������[%d].\r\n"), GetLastError());
			OnClose(pClientInf, RR_UNKNOWN);
		}
	}
	return 0;
}

int CHTTPServer::OnDelaySend(PCLIENTINF pClientInf)
{
	// ɾ����ʱ��
	DeleteTimerQueueTimer(m_hSpeedTimerQueue, pClientInf->hSpeedtimeout, NULL);
	pClientInf->hSpeedtimeout = NULL;

	// ��ʱ���� pClientInf ����Ϣ�Ѿ�׼������.
	if(!doSend(pClientInf))
	{
		// ������������ʱ��,�ͻ��˹ر�������,��ᵼ��WSASend()ʧ��.
		OnClose(pClientInf, RR_SEND_FAILED);
	}
	return 0;
}

// �Ѿ��������յ���һ������ͷ,����֮
// Ŀ��: ͨ������Request����,׼����Response����,���ڿ��ܵ����������һ��Content���󲢹�����Response������.
void CHTTPServer::OnRequest(PCLIENTINF pSockInf)
{
	ASSERT(pSockInf); 
	ASSERT(pSockInf->pRequest); 
	ASSERT(pSockInf->pResponse);
	std::wstring strUrlObject(L"");
	std::wstring strServerFilePath(L"");

	//���������Ƿ���ȷ�����־λ
	bool bReqDone = false;
	//��ȡ����Ĳ�������
	std::vector<std::string> vecUrlData;
	//Wrok��
	CHTTPWork cHttpWork;
	//Ret XMl
	std::string strRetXml;


	// �Ƿ�����Ч������ͷ
	if(!pSockInf->pRequest->Verify())
	{
		// ����ͷ��ʽ����ȷ,����HTTP 400��һ�ι���400��Ԥ����˵���ı�
		pSockInf->pResponse->SetServerCode(SC_BADREQUEST); 
		CHTTPContent *pContent = new CHTTPContent;
		pContent->OpenText(g_HTTP_Bad_Request, strlen(g_HTTP_Bad_Request));
		pSockInf->pResponse->AttachContent(pContent);
		goto exit;
	}
	
	// ����ķ����Ƿ��� GET ���� HEAD
	HTTP_METHOD method = pSockInf->pRequest->GetMethod();
	pSockInf->pResponse->SetMethod(method);
	if (method != METHOD_GET && method != METHOD_POST)
	{
		// Ŀǰֻ֧������HTTP����
		pSockInf->pResponse->SetServerCode(SC_BADMETHOD);
		CHTTPContent *pContent = new CHTTPContent;
		pContent->OpenText(g_HTTP_Bad_Method, strlen(g_HTTP_Bad_Method));
		pSockInf->pResponse->AttachContent(pContent);
		goto exit;
	}
	
	// ��ȡ�ͻ�������Ķ���
	strUrlObject = pSockInf->pRequest->GetUrlObject();
	if(strUrlObject.size() <= 0)
	{
		// URL Object Ϊ��,˵���ͻ��˵�����������.
		pSockInf->pResponse->SetServerCode(SC_BADREQUEST); // ����ͷ��ʽ����
		CHTTPContent *pContent = new CHTTPContent;
		pContent->OpenText(g_HTTP_Bad_Request, strlen(g_HTTP_Bad_Request));
		pSockInf->pResponse->AttachContent(pContent);
		goto exit;
	}

	//��ȡ�ͻ�������Ĳ���
	pSockInf->pRequest->GetUrlData(method , strUrlObject , vecUrlData);
	if ( vecUrlData.empty() )
	{
		// URL vecUrlData Ϊ��,˵���ͻ��˵�����������.
		pSockInf->pResponse->SetServerCode(SC_BADREQUEST); // ����ͷ��ʽ����
		CHTTPContent *pContent = new CHTTPContent;
		pContent->OpenText(g_HTTP_Bad_Request, strlen(g_HTTP_Bad_Request));
		pSockInf->pResponse->AttachContent(pContent);
		goto exit;
	}


	//�����󷽷����д���
	switch (method)
	{
				case METHOD_GET:
				{
					if (!strUrlObject.compare(_T("/Login")))
					{
						cHttpWork.Login(vecUrlData.at(0), vecUrlData.at(1), strRetXml);
						bReqDone = true;
					}
					else if (!strUrlObject.compare(_T("/Main/GetDeviceList")))
					{
						cHttpWork.GetDeviceList(vecUrlData.at(0), strRetXml);
						bReqDone = true;

					}
					else if (!strUrlObject.compare(_T("/Main/GetDeviceStatus")))
					{
						cHttpWork.GetDeviceStatus(vecUrlData.at(0) , strRetXml);
						bReqDone = true;

					}
					else  //Url  ����
					{
						strRetXml = "����Url���� ";
					}


				}
				break;
				case METHOD_POST:
				{
					if (!strUrlObject.compare(_T("/Main/AddUser")))
					{
						cHttpWork.AddUser(vecUrlData.at(0), vecUrlData.at(1), strRetXml);
						bReqDone = true;

					}
					else if (!strUrlObject.compare(_T("/Main/AddDevice")))
					{
						cHttpWork.AddDevice(vecUrlData.at(0), vecUrlData.at(1), strRetXml);
						bReqDone = true;
					}
					else if (!strUrlObject.compare(_T("/Main/SetDeviceThreshold")))
					{
						cHttpWork.SetDeviceThreshold(vecUrlData.at(0), vecUrlData.at(1), strRetXml);
						bReqDone = true;

					}
					else  //Url  ����
					{
						strRetXml = "����Url���� ";
					}
				}
				break;
	}
	



exit:
	if (bReqDone)  //   ������ȷ����
	{
		pSockInf->pResponse->SetServerCode(SC_OK);

		CHTTPContent *pContent = new CHTTPContent;
		pContent->WriteString(strRetXml.c_str());
		pSockInf->pResponse->AttachContent(pContent);
		pSockInf->pResponse->CookResponseWithXMl(strRetXml);
	}
	else  //���ִ���
	{
		// ׼����Ӧͷ
		pSockInf->pResponse->CookResponse();
	}

	// ֪ͨ״̬���սӿ�
	if(m_pStatusHandler)
	{
		m_pStatusHandler->OnRequested(pSockInf->pszIP, pSockInf->nPort, strUrlObject.c_str(), 
			pSockInf->pRequest->GetMethod(), pSockInf->pResponse->GetServerCode());
	}
	return;
}



BOOL CHTTPServer::mapServerFile(const std::wstring &strUrl, std::wstring &strFilePath)
{
	// ��ø�Ŀ¼
	strFilePath = m_strWorkDir;
	ASSERT(strFilePath.size() > 0);
	if(strFilePath.back() == L'\\') strFilePath.erase(strFilePath.end()--);

	// �������·��
	strFilePath += strUrl;
	for(std::wstring::iterator iter = strFilePath.begin(); iter != strFilePath.end(); ++iter)
	{
		if( *iter == L'/' ) *iter = L'\\'; // URL����б���滻Ϊ��б��.
	}

	return TRUE;
}

void CHTTPServer::lock()
{
	EnterCriticalSection(&m_cs);
}

void CHTTPServer::unlock()
{
	LeaveCriticalSection(&m_cs);
}

int CHTTPServer::OnDeadConnectionDetected(PCLIENTINF pClientInf)
{
	// ��⵽�����Ӻ�,���׽��ֹر�,����,����ִ�е�IO������ʧ��,�� GetQueuedCompletionStatus() ��������.
	// Ȼ����ִ�� OnClose() �������׽��ֶ�Ӧ����Դȫ������.

	//LOGGER_CWARNING(theLogger, _T("��⵽һ��������[%s:%d].\r\n"), pClientInf->pszIP, pClientInf->nPort);
	pClientInf->nOprationType = opp_dead;
	shutdown(pClientInf->hSocket, SD_BOTH);
	closesocket(pClientInf->hSocket);
	return 0;
}

int CHTTPServer::OnSessionTimeout(PCLIENTINF pClientInf)
{
	//LOGGER_CWARNING(theLogger, _T("����[%s:%d]�Ự��ʱ.\r\n"), pClientInf->pszIP, pClientInf->nPort);
	pClientInf->nOprationType = opp_session_timeout;
	shutdown(pClientInf->hSocket, SD_BOTH);
	closesocket(pClientInf->hSocket);

	// �˴����ܰ� pClientInf->hSocket ����Ϊ INVALID_SOCKET, ��Ϊ�ͷ���Դ����Ҫ��������ΪKEY.
	return 0;
}

BOOL CHTTPServer::doAccept()
{
	if(m_sockListen == INVALID_SOCKET) return FALSE;

	// ����һ���µ��׽���.
	m_sockNewClient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if( INVALID_SOCKET == m_sockNewClient )
	{
		ASSERT(0);
		LOGGER_CERROR(theLogger, _T("�޷�Ϊ�����Ӵ����׽���,������[%d].\r\n"), WSAGetLastError());
		return FALSE;
	}

	// ����AcceptEx()����
	DWORD dwBytesReceived = 0;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(m_sockListen, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),&lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL) )
	{
		ASSERT(0);
		LOGGER_CERROR(theLogger, _T("�޷����AcceptEx()����ָ��,�������޷����յ��ͻ�������,������[%d].\r\n"), WSAGetLastError());
		return FALSE;
	}

	memset(&m_AcceptOL, 0, sizeof(m_AcceptOL));
	if( !lpfnAcceptEx(m_sockListen, m_sockNewClient, m_pAcceptBuf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytesReceived, &m_AcceptOL) )
	{
		if( WSA_IO_PENDING != WSAGetLastError())
		{
			ASSERT(0);
			LOGGER_CERROR(theLogger, _T("�޷����ܿͻ�������,������[%d].\r\n"), WSAGetLastError());
			return FALSE;
		}
		else
		{
			// �������,��ʱ�޿ͻ�������.
		}
	}
	else
	{
		// �����õ�һ���µĿͻ�����,˵����ʱ�кܶ��������ŶӵȺ������.
		ASSERT(0); // ���������׽����Ѿ�������һ����ɶ˿�,��Ӧ�����е�����.
		OnAcceptEx(FALSE);
	}
	return TRUE;
}

//BOOL CHTTPServer::RegisterXSPHander(LPCTSTR lpszType, IXSPHandler *pHander)
//{
//	//if(NULL == lpszType || NULL == pHander) return FALSE;
//
//	//hander_iter iter;
//	//iter = m_HanderMap.find(std::string(lpszType));
//	//if(iter == m_HanderMap.end())
//	//{
//	//	m_HanderMap.insert(std::make_pair(std::string(lpszType), pHander));
//	//}
//	//else
//	//{
//	//	iter->second = pHander;
//	//}
//	return TRUE;
//}
//
//BOOL CHTTPServer::UnRegisterXSPHander(LPCTSTR lpszType)
//{
//	//if(NULL == lpszType) return FALSE;
//	//
//	//hander_iter iter;
//	//iter = m_HanderMap.find(std::string(lpszType));
//	//if(iter == m_HanderMap.end())
//	//{
//	//	return FALSE;
//	//}
//	//else
//	//{
//	//	m_HanderMap.erase(iter);
//	//}
//	return TRUE;
//}

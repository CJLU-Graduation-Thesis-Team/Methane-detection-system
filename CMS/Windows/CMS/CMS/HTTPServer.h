/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#if !defined(AFX_HTTPFILESERVER1_H__2E98529A_F7A8_4D40_AD95_457ADF4FD0B8__INCLUDED_)
#define AFX_HTTPFILESERVER1_H__2E98529A_F7A8_4D40_AD95_457ADF4FD0B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/*
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")
*/

/*
HTTP Server������/������
Ŀ��: ����һ��HTTP Server���ڴ��еĴ���,������Ҫ�����߳�,��������ʹ��������������.
1. ��������������߳�,Ϊ��ɶ˿ڷ���,���������¼�����ʱ,����OnAcceptEx(), OnSend(), OnRecv(),OnClose()�ص�������������������.
2. ������ά�ֶ�ʱ������,���г�ʱ�¼�����ʱ,����OnDelaySend(),OnDeadConnectionDetected(),OnSessionTimeout()�ص�����.

*/
#pragma warning(disable : 4786)
#include "HTTPDef.h"

class CHTTPServer
{
protected:
	std::wstring m_strWorkDir;			// ����·��
	BOOL m_bRuning;						// �Ƿ���������
	BOOL m_bNavDir;						// �Ƿ��������Ŀ¼
	string_vector m_vecDeftFileNames;	// Ĭ���ļ���

	int m_nListenPort;					// �����˿�
	SOCKET m_sockListen;				// �����׽���.
	SOCKET m_sockNewClient;				// AccpetEx()�õ��¿ͻ��׽���.
	char *m_pAcceptBuf;					// AccpetEx()�û���
	int m_nAcceptBufLen;				// ..
	OVERLAPPED m_AcceptOL;				// ..

	SVRINF* m_pServiceInf;				// ��ɶ˿���Ϣ.
	int m_nMaxConnections;				// ������ܵ����������

	SOCKINFMAP m_SockInfMap;			// �ͻ���Ϣ�б�,ÿ������(�ͻ�)��Ӧһ����¼(PCLIENTINF)ָ��.
	CRITICAL_SECTION m_cs;				// ͬ������

	int m_nMaxClientConn;				// һ���ͻ���(IP)�����������.0��ʾ������.
	__int64 m_llMaxSpeed;				// ÿ����������ٶ�����,��λ B/s. 0��ʾ������.
	string_int_map m_ClientIPMap;		// �ͻ���IP��ַ��(ÿIP��Ӧһ����¼,��������ÿ�ͻ������������)

	DWORD m_dwDeadConnectionTimeout;	// ����,�����ӳ�ʱ,���һ��������ָ��������û�з��ͻ��߽��յ��κ�����,����Ϊ��������,�����Ƴ�
	DWORD m_dwSessionTimeout;			// ����,�Ự��ʱ.һ���������ֻά��ָ��ʱ�䳤.
	HANDLE m_hDCTimerQueue;				// �����Ӷ�ʱ������.
	HANDLE m_hSessionTimerQueue;		// �Ự��ʱ��ʱ������
	HANDLE m_hSpeedTimerQueue;			// �ٶ����ƶ�ʱ������.
	
	IHTTPServerStatusHandler *m_pStatusHandler; // ״̬�ص��ӿ�,ʵ�������ڿ��Ի������״̬.
public:

	CHTTPServer();
	virtual ~CHTTPServer();

	// ����������� WS2 ��, �� AfxSocketInit() Ҳ����.
	static BOOL InitWinsock(WORD nMainVer, WORD nSubVer);
	static BOOL CleanWinsock();

	// ��������ֹͣ������
	HTTP_SERVER_ERROR_TYPE Run(PHTTPSTARTDESC pStartDesc);
	HTTP_SERVER_ERROR_TYPE Stop();
	BOOL IsRuning() { return m_bRuning; }
	
protected:
	CHTTPServer(const CHTTPServer& other);
	const CHTTPServer& operator = (const CHTTPServer& other);

	PCLIENTINF newClientDesc(SOCKET hSocket, const std::wstring &strIP, unsigned int nPort); // ��ʼ��һ���ͻ�������.
	void deleteClientDesc(PCLIENTINF pClientInf, BOOL bSkeepTimer = FALSE);	// ���տͻ�������.
	HTTP_SERVER_ERROR_TYPE prepareListenSocket(int nPort, SOCKET& hListenSock);
	void doStop();	// ���շ�������Դ,��Run()����ʧ�ܵ�����µ���.
	BOOL doAccept(); // ����һ�� AcceptEx()����
	BOOL doRecv(PCLIENTINF pClientInf);	// WSARecv()��װ����,�������
	BOOL doSend(PCLIENTINF pClientInf); // WSASend()��װ����,�������
	BOOL mapServerFile(const std::wstring &strUrl, std::wstring &strFilePath); //��URLӳ��Ϊ�������ϵ������ļ���.
	void lock();
	void unlock();

protected:
	static UINT __cdecl ServiceProc(LPVOID pParam); // �����߳�, �첽�����׽���.
	static VOID CALLBACK DeadConnectionTimeoutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
	static VOID CALLBACK SessionTimeoutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
	static VOID CALLBACK SpeedTimeoutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

	// �ص����� ////////////////////////////////////////////////////////////////////////
	BOOL OnAcceptEx(BOOL bSkeep); // ����accept�¼�
	void OnRequest(PCLIENTINF pSockInf);					// ����һ������ͷ
	int OnClose(PCLIENTINF pClientInf, REMOVE_REASON reason); // ��⵽�����ӱ��رջ������ݷ��������Ҫ�ر�
	int OnRecv(PCLIENTINF pClientInf, DWORD dwTransfered); // �������ݵ����������ɼ�WSARecv()����
	int OnSend(PCLIENTINF pClientInf, DWORD dwTransfered); // �������ݵ����������ɼ�WSASend()����
	int OnDelaySend(PCLIENTINF pClient); // �ٶȿ���,��ʱ����
	int OnDeadConnectionDetected(PCLIENTINF pClientInf); // ��⵽������
	int OnSessionTimeout(PCLIENTINF pClientInf); // ��⵽�Ự��ʱ
};

#endif // !defined(AFX_HTTPFILESERVER1_H__2E98529A_F7A8_4D40_AD95_457ADF4FD0B8__INCLUDED_)

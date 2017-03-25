/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#pragma once
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ⲿ������Ҫ�Ķ���
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��ϢID, HttpServer ����� SendMessage ����������Ϣ֪ͨ������.
// ������Ҫ������Ϣ��Ӧ����,��������δ֪�Ľ��.
#define WM_HTTP_SERVER (WM_USER + 1009) 

// ����ֵ����(�������Ͷ���)
enum HTTP_SERVER_ERROR_TYPE
{
	SE_SUCCESS = 0,
	
	SE_RUNING = 1, // ��������
	SE_STOPPED = 2, // �Ѿ�ֹͣ

	SE_CREATESOCK_FAILED = 100, // �׽��ִ���ʧ��
	SE_BIND_FAILED = 101, // �󶨶˿�ʧ��
	SE_LISTEN_FAILED = 102, // listen() ��������ʧ��.
	SE_CREATETIMER_FAILED = 103, // �޷�������ʱ��
	SE_CREATE_IOCP_FAILED = 104,


	SE_UNKNOWN = 1000
};

// �������Ƴ��׽��ֵ�ԭ��
enum REMOVE_REASON
{
	RR_CLIENTCLOSED = 0, // �ͻ��˹ر�������
	RR_SENDCOMPLETE, // �������
	RR_RECV_FAILED, // ����ʧ��.
	RR_SEND_FAILED, // ����ʧ��.

	RR_DEAD,
	RR_SESSION_TIMEOUT,

	RR_UNKNOWN // δ֪.	
};


// ��ϢWM_HTTP_SERVER������
enum HTTP_MESSAGE_TYPE
{
	MT_CONNECTION_NEW = 100,
	MT_CONNECTION_CLOSED, // ���ӹر�
	MT_REQUEST_URL,	// �ͻ������URL
	MT_CONNECTION_REFUSED,

	MT_UNKNOWN
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// �ڲ�ʵ��ʹ�� /////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �궨��
#define MAX_SOCKET 64
#define MAX_SOCKBUFF 8192 // �����ջ�����.
#define MAX_METHODSIZE 100 // ���ڱ���HTTP�����ַ����ĳ���,���˳��� 200
#define MAX_REQUESTHEADERSIZE 1024 * 10
#define MAX_RESPONSEHEADERSIZE 1024 * 10
#define MAX_IP_LENGTH 50 // IP��ַ����󳤶�
#define MIN_SIZE_ONSPEEDLIMITED 512 // �ﵽ�ٶ�����ʱ,���͵���С���ֽ���.
#define MAX_WAITTIME_ONSPEEDLIMITED 2000 // �ﵽ�ٶ�����ʱ,���ȴ����ٺ��뷢��һ����.���ֵ������õù���,�п��ܵ��¿ͻ������������û��Ӧ
#define DCTIMEOUT_DELAY 50 // ����������ٶ����ƶ�ʱ��,��ô�����Ӷ�ʱ��Ҫ��Ӧ�Ӻ�,��Ϊ�����ǿͻ���û��Ӧ,���Ƿ���������ʱ����,���Բ�����������.

#define UNIT_BUFFER_SIZE 1024 * 10 // HTTP Content ���ڴ������ٶ�
#define MAX_BUFFER_SIZE 1024 * 1024 * 5 // HTTP Content ����ڴ� 5M

#define ETAG_BYTE_SIZE 5 // �����ڴ�����,����ETagʱ��ȡ���ֽ���.

#define G_BYTES (1024 * 1024 * 1024) // 1GB
#define M_BYTES (1024 * 1024)		 // 1MB
#define K_BYTES 1024				 // 1KB

// ��������Ӧ��
enum SERVER_CODE
{
	SC_AUTO = 0,
	SC_OK = 200,
	SC_NOCONTENT = 204,
	SC_PARTIAL = 206,
	SC_BADREQUEST = 400,
	SC_FORBIDDEN = 403,
	SC_NOTFOUND = 404,
	SC_BADMETHOD = 405,
	SC_SERVERERROR = 500,

	SC_UNKNOWN = 600
};


// �ص�����������
enum opptype		
{
	opp_none = 0,		// δ����
	opp_recv = 0x01,			// ����
	opp_send = 0x02,			// ����
	opp_accept = 0x04,			// ����������
	opp_close = 0x08,			// �ر�
	opp_dead = 0x10,			// ������
	opp_session_timeout = 0x20,	// �Ự��ʱ

	opp_all = 0xFF
};

// HTTP ����
enum HTTP_METHOD
{
	METHOD_UNDEFINE = 0,
	METHOD_GET = 1,
	METHOD_POST,
	METHOD_PUT,
	METHOD_HEAD, // ֻ������Ӧͷ
	METHOD_DELETE, // ɾ��
	METHOD_TRACE,
	METHOD_CONNECT,

	METHOD_UNKNOWN
};

// �ṹ����
// һ���ͻ��׽��ֵ���������(�ͻ�������) WSAOVERLAPPED �����ǵ�һ����Ա.
class CHTTPRequest;
class CHTTPResponse;

typedef struct tagClientInf			
{
	WSAOVERLAPPED	Overlapped;				// �ص��ṹ
	WSABUF			WSABuf;					// socket���ݻ���

	SOCKET			hSocket;				// �׽��־��
	int				nOprationType;			// �ص��������� enum opptype

	CHTTPRequest*	pRequest;				// HTTP����������
	CHTTPResponse*  pResponse;				// HTTP��Ӧ������
	DWORD			dwLastSent;				// ���һ�η��ͳɹ�ʱ���͵��ֽ���(��ʱ����ʱ������ֶ�)
	__int64			nSended;				// һ�����͵��ַ���
	DWORD			dwRecved;				// һ�����յ����ַ���
	DWORD			dwStartTime;			// ���ӿ�ʼʱ��ʱ�� GetTickcount()����ֵ/���ÿ���GetTickcount ���,ֻ����ʾ�������.
	DWORD			dwEndTime;				// ���ӽ���ʱ��ʱ��
	wchar_t			pszIP[MAX_IP_LENGTH + 1]; // �ͻ�������IP��ַ,����������hSocketȡ��,������Ҫ�õ��ĵط�̫����,���Ᵽ��һ�����Ч��.
	unsigned int	nPort;					// �ͻ������Ӷ˿�

	DWORD			dwLastActiveTime;		// ��¼���һ�η��ͻ��߽������ݵ�ʱ��,������Ϊ�ж��Ƿ�ʱ���ڵ�����.(Ҫ����GetTickcount()ѭ�������).
	HANDLE			hDeadConnectionTimeout; // �����ӳ�ʱ������
	HANDLE			hSessionTimeout;		// �Ự��ʱ������
	HANDLE			hSpeedtimeout;			// ���ٶ�ʱ��
	void*			pInstant;				// ������¼ CHTTPServer ʵ��ָ��
}CLIENTINF, *PCLIENTINF;

// һ������˿ڵ���������(����������)
typedef struct tagServiceInf		
{
	HANDLE hCompletionPort;				// ��ɶ˿ھ��
	CWinThread* *pThread;				// �߳�����
	int	nThreadCount;					// ÿ���˿ڶ�Ӧ���߳���,(CPU���� + 1)
	int nSockCount;						// ��ǰ�׽��ֵ�����
}SVRINF;


typedef std::map<SOCKET, CLIENTINF*> SOCKINFMAP;
typedef SOCKINFMAP::iterator sockinf_iter;
typedef SOCKINFMAP::const_iterator sockinf_citer;
typedef std::map<std::wstring, int> string_int_map;
typedef std::vector<std::wstring> string_vector;

// �ⲿ������ַ���
extern const char* g_HTTP_Content_NotFound;
extern const char* g_HTTP_Bad_Request;
extern const char* g_HTTP_Bad_Method;
extern const char* g_HTTP_Server_Error;
extern const char* g_HTTP_Forbidden;

// ��һ��ʱ���ʽ��Ϊ HTTP Ҫ���ʱ���ʽ(GMT).
std::string FormatHTTPDate(__int64* ltime);
std::string ToHex(const unsigned char* pData, int nSize);

// OnAccept() ����ֵ
#define ON_ACCEPT_SUCCESS 0
#define ON_ACCEPT_BUSY 1
#define ON_ACCEPT_CLOSED 2

// ������״̬��Ϣ�ӽӿ�
// HTTP�������������ڼ���������ӿڵķ�����ʹ�ýӿڵ�ʵ�ֿ��Ի�ȡʱʱ��HTTP������
class IHTTPServerStatusHandler
{
public:
	// ��������ʱ����,�ɲ��� bRefused ��ʶ�Ƿ񱻷������ܾ�.
	virtual void OnNewConnection(const wchar_t *pszIP, unsigned int nPort, BOOL bRefused, BOOL bKicked) = 0;

	// ���Ӷ˿�ʱ����
	virtual void OnConnectionClosed(const wchar_t *pszIP, unsigned int nPort, REMOVE_REASON rr, __int64 nByteSent, unsigned int nTimeUsed) = 0;

	// ���ݷ������ʱ����
	virtual void OnDataSent(const wchar_t *pszIP, unsigned int nPort, unsigned int nBytesSent) = 0;

	// ���յ�����ʱ����
	virtual void OnDataReceived(const wchar_t *pszIP, unsigned int nPort, unsigned int nBytesReceived) = 0;

	// ������ͻ�������,�ڷ��ͻ�Ӧ���ͻ���ǰ����.
	virtual void OnRequested(const wchar_t *pszIP, unsigned int nPort, const wchar_t *pszUrl, HTTP_METHOD hm, SERVER_CODE sc) = 0;
};

// HTTP ��������
typedef struct tagHTTPServerStartDesc
{
	//Http Info
	TCHAR szRootDir[MAX_PATH + 1]; //
	TCHAR szDefaultFileName[MAX_PATH + 1]; // Ĭ���ļ���
	BOOL bNavDir; // = TRUE, 
	int nPort;// = 80, 
	int nMaxConnection;// = 2000, 
	int nMaxClientConn;  // = 0,
	__int64 llMaxSpeed;	// ÿ����������ٶ� B/s
	DWORD dwSessionTimeout; // = 600000, 
	DWORD dwDeadConnectionTimeout; // = 30000,
	IHTTPServerStatusHandler *pStatusHandler; // = NULL

	//Dev Info
	int nDevPort;
}HTTPSTARTDESC, *PHTTPSTARTDESC;


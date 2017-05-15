/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#include "StdAfx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "HTTPResponse.h"
#include "HTTPContent.h"

CHTTPResponse::CHTTPResponse(const wchar_t* pszIP, unsigned int nPort) :
	m_strIP(pszIP), m_nPort(nPort)
{
	m_pData = new char[MAX_RESPONSEHEADERSIZE + 1]; ASSERT(m_pData);
	ZeroMemory(m_pData, MAX_RESPONSEHEADERSIZE + 1);
	m_nReadPos = 0;
	m_nServerCode = SC_AUTO;

	m_nHeaderSize = 0;

	m_Method = METHOD_UNDEFINE;

	m_pContent = NULL;
}

CHTTPResponse::~CHTTPResponse(void)
{
	if(m_pData != NULL) delete[] m_pData;
	if(m_pContent != NULL)
	{
		m_pContent->Close();
		delete m_pContent;
	}
}

void CHTTPResponse::AttachContent(CHTTPContent *pContent)
{
	ASSERT(pContent);
	if(m_pContent != NULL)
	{
		m_pContent->Close();
		delete m_pContent;
		ASSERT(0);
	}
	m_pContent = pContent;
}


char* CHTTPResponse::getDataPtr(CHTTPContent *pContent)
{
	ASSERT(pContent);
	if (m_pContent != NULL)
	{
		m_pContent->Close();
		delete m_pContent;
		ASSERT(0);
	}
	return m_pData;
}

HTTP_METHOD CHTTPResponse::SetMethod(HTTP_METHOD mh)
{
	HTTP_METHOD mh_old = m_Method;
	m_Method = mh;
	return mh_old;
}

SERVER_CODE CHTTPResponse::SetServerCode(SERVER_CODE nNewCode)
{
	SERVER_CODE nOldCode = m_nServerCode;
	m_nServerCode = nNewCode;
	return nOldCode;
}

std::string CHTTPResponse::getFirstLine(SERVER_CODE nServerCode)
{
	std::string str = "HTTP/1.1 ";
	switch (nServerCode)
	{
	case SC_OK:
		{
			str += "200 OK";
			break;
		};
	case SC_NOCONTENT:
		{
			str += "204 No Content";
			break;
		};
	case SC_PARTIAL:
		{
			str += "206 Partial Content";
			break;
		};
	case SC_BADREQUEST:
		{
			str += "400 Bad Request";
			break;
		};
	case SC_NOTFOUND:
		{
			str += "404 Not Found";
			break;
		}
	case SC_BADMETHOD:
		{
			str += "405 Method Not Allowed";
			break;
		};
	default:
		str += "500 Internal Server Error";
	};

	str += "\r\n";
	return str;
}

BOOL CHTTPResponse::CookResponseWithXMl(std::string strRetXMl)
{
	if (m_pContent != NULL)
	{

	}
	else
	{
		ASSERT(0);
		CHTTPContent *pContent = new CHTTPContent;
		AttachContent(pContent);
	}

	// ����ͻ���Ҫ�� Body, ������Ӧ��Ϊ 204
	if (m_Method == METHOD_HEAD)
	{
		SetServerCode(SC_NOCONTENT);
	}

	// ��һ��
	strcat(m_pData, getFirstLine(GetServerCode()).c_str());

	// Date
	strcat(m_pData, "Date: ");
	strcat(m_pData, FormatHTTPDate(NULL).c_str());
	strcat(m_pData, "\r\n");

	// ���¼���ֻ���������ݵ�ʱ������.
	if ( !strRetXMl.empty() )
	{
		//// Last-Modified
		//strcat(m_pData, "Last-Modified: ");
		//strcat(m_pData, m_pContent->GetLastModified().c_str());
		//strcat(m_pData, "\r\n");

		//// ETag
		//strcat(m_pData, "ETag: ");
		//strcat(m_pData, m_pContent->GetETag().c_str());
		//strcat(m_pData, "\r\n");

		// Content-Type
		strcat(m_pData, "Content-Type: ");
		strcat(m_pData, "application/json" );
		strcat(m_pData, "\r\n");

		// Content-Length
	/*	char szLen[200] = { 0 };
		__int64 lLen = strRetXMl.length();
		strcat(m_pData, "Content-Length: ");
		strcat(m_pData, _i64toa(lLen, szLen, 10));
		strcat(m_pData, "\r\n");*/

		//// Content-Range: bytes %d-%d/%d\r\n"
		//if (SC_PARTIAL == GetServerCode())
		//{
		//	strcat(m_pData, "Content-Range: ");
		//	strcat(m_pData, m_pContent->GetContentRange().c_str());
		//	strcat(m_pData, "\r\n");
		//}
	}
	else
	{
		// Content-Length 
		strcat(m_pData, "Content-Length: 0\r\n");
	}

	// "Accept-Ranges: bytes" ֧�ֶϵ�����.
	//strcat(m_pData, "Accept-Ranges: bytes\r\n");

	// ֻ֧�� GET �� POST ����
	if (GetServerCode() == SC_BADMETHOD)
	{
		strcat(m_pData, "Allow: GET, POST\r\n");
	}

	// XServer
	strcat(m_pData, "Server: Chenguangle HTTP Server/1.5\r\n");

	// ��blank line��
	strcat(m_pData, "\r\n");

	// connection,������
	//strcat(m_pData, "Connection: close\r\n\r\n");

	//// �ͻ����Ƿ�ֻҪ����Ӧͷ.
	//if (m_Method == METHOD_HEAD)
	//{
	//	m_pContent->Close();
	//}

	// ������Ӧͷ�ĳ���
	m_nHeaderSize = strlen(m_pData);

	//������Ӧ�峤��
	//m_nReadPos = strRetXMl.length();

	return TRUE;
}

BOOL CHTTPResponse::CookResponse()
{
	if(m_pContent != NULL)
	{
		
	}
	else
	{
		ASSERT(0);
		CHTTPContent *pContent = new CHTTPContent;
		AttachContent(pContent);
	}
	

	//// ׼������
	//if( SC_AUTO == GetServerCode())
	//{
	//	// ����Ϊһ���ļ�
	//	if(m_strFileName.size() > 0)
	//	{
	//		if(m_pContent->OpenFile(m_strFileName.c_str(), m_lFrom, m_lTo))
	//		{
	//			if(m_lFrom == 0 && m_lTo == -1)
	//			{
	//				SetServerCode(SC_OK);
	//			}
	//			else
	//			{
	//				SetServerCode(SC_PARTIAL);
	//			}
	//		}
	//		else
	//		{
	//			SetServerCode(SC_NOTFOUND);
	//			
	//			long lFrom = 0;
	//			long lTo = 0;
	//			m_pContent->OpenHtml(g_HTTP_Content_NotFound, strlen(g_HTTP_Content_NotFound));
	//		}
	//	}
	//	else
	//	{
	//		// ����ΪĿ¼���ļ��б�
	//		SetServerCode(SC_OK);
	//		m_pContent->OpenDir(m_strRoot.c_str(), m_strDir.c_str());
	//	}
	//}
	//else if(SC_BADMETHOD == GetServerCode())
	//{
	//	m_pContent->OpenText(g_HTTP_Bad_Method, strlen(g_HTTP_Bad_Method));
	//}
	//else if(SC_BADREQUEST == GetServerCode())
	//{
	//	m_pContent->OpenText(g_HTTP_Bad_Request, strlen(g_HTTP_Bad_Request));
	//}
	//else
	//{
	//	// ������Զ��������Ĵ���������, �����������Щ����.
	//}

	// ����ͻ���Ҫ�� Body, ������Ӧ��Ϊ 204
	if(m_Method == METHOD_HEAD)
	{
		SetServerCode(SC_NOCONTENT);
	}

	// ��һ��
	strcat(m_pData, getFirstLine(GetServerCode()).c_str());
	
	// Date
	strcat(m_pData, "Date: ");
	strcat(m_pData, FormatHTTPDate(NULL).c_str());
	strcat(m_pData, "\r\n");
	
	// ���¼���ֻ���������ݵ�ʱ������.
	if(m_pContent->IsOpen())
	{
		// Last-Modified
		strcat(m_pData, "Last-Modified: ");
		strcat(m_pData, m_pContent->GetLastModified().c_str());
		strcat(m_pData, "\r\n");

		// ETag
		strcat(m_pData, "ETag: ");
		strcat(m_pData, m_pContent->GetETag().c_str());
		strcat(m_pData, "\r\n");

		// Content-Type
		strcat(m_pData, "Content-Type: ");
		strcat(m_pData, m_pContent->GetContentType().c_str());
		strcat(m_pData, "\r\n");

		// Content-Length
		char szLen[200] = {0};
		__int64 lLen = m_pContent->GetContentLength();
		strcat(m_pData, "Content-Length: ");
		strcat(m_pData, _i64toa(lLen, szLen, 10));
		strcat(m_pData, "\r\n");

		// Content-Range: bytes %d-%d/%d\r\n"
		if(SC_PARTIAL == GetServerCode())
		{
			strcat(m_pData, "Content-Range: ");
			strcat(m_pData, m_pContent->GetContentRange().c_str());
			strcat(m_pData, "\r\n");
		}
	}
	else
	{
		// Content-Length 
		strcat(m_pData, "Content-Length: 0\r\n");
	}

	// "Accept-Ranges: bytes" ֧�ֶϵ�����.
	strcat(m_pData, "Accept-Ranges: bytes\r\n");

	//// ֻ֧�� GET �� HEAD ����
	//if(GetServerCode() == SC_BADMETHOD)
	//{
	//	strcat(m_pData, "Allow: GET, HEAD\r\n");
	//}
	// ֻ֧�� GET �� POST ����
	if (GetServerCode() == SC_BADMETHOD)
	{
		strcat(m_pData, "Allow: GET, POST\r\n");
	}

	// XServer
	strcat(m_pData, "Server: Chenguangle HTTP Server/1.5\r\n");

	// connection,������
	strcat(m_pData, "Connection: close\r\n\r\n");

	// �ͻ����Ƿ�ֻҪ����Ӧͷ.
	if(m_Method == METHOD_HEAD)
	{
		m_pContent->Close();
	}

	// ������Ӧͷ�ĳ���
	m_nHeaderSize = strlen(m_pData);

	return TRUE;
}

int CHTTPResponse::PopData(void* pData, int nSize)
{	
	if(IsEOF()) return 0;

	int nReaded = m_nHeaderSize - m_nReadPos;

	// �������ͷ�л��������ȴ� pData �ж�ȡ����ͷ
	if(nReaded > 0)
	{
		if(nReaded > nSize) nReaded = nSize;
		memcpy(pData, m_pData + m_nReadPos, nReaded);
		m_nReadPos += nReaded;
	}
	
	// ���������ͷ�ж�ȡ�������ݲ���,���Content�ж�ȡ
	if(nReaded < nSize && m_pContent->IsOpen())
	{
		nReaded += m_pContent->Read((char*)pData + nReaded, nSize - nReaded);
	}
	
	return nReaded;
}

BOOL CHTTPResponse::IsEOF()
{
	return m_nReadPos == m_nHeaderSize && ( m_pContent == NULL || m_pContent->IsOpen() && m_pContent->IsEOF());
}
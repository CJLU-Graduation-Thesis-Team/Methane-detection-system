/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#include "StdAfx.h"
#include "HTTPRequest.h"

CHTTPRequest::CHTTPRequest(const wchar_t* pszIP, unsigned int nPort) :
	m_strIP(pszIP), m_nPort(nPort)
{
	m_pData = new char[MAX_REQUESTHEADERSIZE + 1]; // ������
	ASSERT(m_pData);
	ZeroMemory(m_pData, MAX_REQUESTHEADERSIZE + 1);
	m_nPos = 0; // �������α�.
}

CHTTPRequest::~CHTTPRequest(void)
{
	if(m_pData != NULL) delete[] m_pData;
}

// nSize, ���� 0 - ���������
int CHTTPRequest::PushData(const char* pData, int nSize)
{
	int nBufferLeft = MAX_REQUESTHEADERSIZE - m_nPos;
	if( nSize > nBufferLeft ) return 0;

	memcpy(m_pData + m_nPos, pData, nSize);
	m_nPos += nSize;
	return nSize;
}

BOOL CHTTPRequest::IsEnd()
{
	return strstr(m_pData, "\r\n\r\n") != NULL;
}

BOOL CHTTPRequest::Verify()
{
	// ���������Ļ���Ӧ��������ͷ�����
	char* pszEndTag = strstr(m_pData, "\r\n\r\n");
	if(pszEndTag == NULL)
	{
		return FALSE;
	}

	if( (pszEndTag + 4) != (m_pData + m_nPos) )
	{
		return FALSE;
	}

	return TRUE;
}

HTTP_METHOD CHTTPRequest::GetMethod()
{
	// ȡ�� HTTP ����
	char szMethod[MAX_METHODSIZE] = {0};
	int nMethodIndex = 0;
	for(int i = 0; i < MAX_METHODSIZE && i < m_nPos; ++i)
	{
		if(m_pData[i] != ' ')
		{
			szMethod[nMethodIndex++] = m_pData[i];
		}
		else
		{
			break;
		}
	}

	// ����
	if( strcmp(szMethod, "GET") == 0 ) return METHOD_GET;
	if( strcmp(szMethod, "PUT") == 0 ) return METHOD_PUT;
	if( strcmp(szMethod, "POST") == 0 ) return METHOD_POST;
	if( strcmp(szMethod, "HEAD") == 0 ) return METHOD_HEAD;
	if( strcmp(szMethod, "DELETE") == 0 ) return METHOD_DELETE; // ɾ��
	if( strcmp(szMethod, "TRACE") == 0 ) return METHOD_TRACE;
	if( strcmp(szMethod, "CONNECT") == 0 ) return METHOD_CONNECT;

	return METHOD_UNKNOWN;
}

// ���ؿͻ����������, ������ؿ��ַ���,˵���ͻ��������ʽ����.
std::wstring CHTTPRequest::GetUrlObject()
{
	std::wstring strObject(L"");
	const char* lpszRequest = m_pData;
	const char *pStart = NULL, *pEnd = NULL;

	// ��һ�еĵ�һ���ո����һ���ַ���ʼ��������ļ�����ʼ.
	for(int i = 0; i < m_nPos; ++i)
	{
		if(lpszRequest[i] == ' ')
		{
			pStart = lpszRequest + i + 1; 
			break;
		}
		if(lpszRequest[i] == '\n') break;
	}
	if(pStart == NULL) return strObject;

	// �ӵ�һ�е�ĩβ������ҵ�һ���ո�,ʵ��: GET / HTTP/1.1
	pEnd = strstr(lpszRequest, "\r\n"); ASSERT(pEnd);
	if(pEnd == NULL || pEnd < pStart) return strObject;
	while(pEnd >= pStart)
	{
		if(pEnd[0] == ' ')
		{
			pEnd--;
			break;
		}
		pEnd--;
	}

	if(pEnd == NULL || pEnd < pStart) return strObject;

	// �Ѿ�ȡ���˿�ʼ�ͽ�����λ��.
	int nObjectLen = pEnd - pStart + 1;
	char *pszObject = new char[nObjectLen + 1]; ASSERT(pszObject);
	ZeroMemory(pszObject, nObjectLen + 1);
	
	// UTF8����
	int nNewPos = 0;
	BOOL bUTF8 = FALSE;
	for( int i = 0; i < nObjectLen; ++i)
	{
		if(pStart[i] == '?')
		{
			// '?' �����ǲ���,����.
			break;
		}

		if(pStart[i] == '%' && i + 1 < nObjectLen && i + 2 < nObjectLen)
		{
			char szValue[5] = {0}, *pStopPos = NULL;
			szValue[0] = '0';
			szValue[1] = 'x';
			szValue[2] = pStart[i + 1];
			szValue[3] = pStart[i + 2];
			szValue[4] = 0;
			pszObject[nNewPos++] = (char) strtol(szValue, &pStopPos, 16);
			
			i += 2;
			
			// �ո�ᱻ����Ϊ %20
			if(pszObject[nNewPos - 1] != ' ')
			{
				bUTF8 = TRUE;
			}
		}
		else
		{
			// ��ȡ����URL,��Ӧ���滻��б��
			//if(pStart[i] == '/') pszObject[nNewPos++] = '\\';
			//else pszObject[nNewPos++] = pStart[i];
			pszObject[nNewPos++] = pStart[i];
		}
	}

	if(bUTF8)
	{
		strObject = UTF8toW(pszObject);
		//std::wstring wURL = UTF8toW(pszObject);
		//strObject = WtoA(wURL.c_str());
	}
	else
	{
		strObject = AtoW(pszObject);
	}

	delete[] pszObject;
	return strObject;
}

// ���ؿͻ����������ֵ
bool CHTTPRequest::GetUrlData(std::wstring wstrUrlObject, std::vector<std::string>& vecRetData)
{
	std::wstring strObject(L"");
	const char* lpszRequest = m_pData;
	const char *pStart = NULL, *pEnd = NULL;

	// ��һ�еĵ�һ���ո����һ���ַ���ʼ��������ļ�����ʼ.
	for (int i = 0; i < m_nPos; ++i)
	{
		if (lpszRequest[i] == ' ')
		{
			pStart = lpszRequest + i + 1;
			break;
		}
		if (lpszRequest[i] == '\n') break;
	}
	if (pStart == NULL)
	{
		return false;
	}

	// �ӵ�һ�е�ĩβ������ҵ�һ���ո�,ʵ��: GET / HTTP/1.1
	pEnd = strstr(lpszRequest, "\r\n"); ASSERT(pEnd);
	if (pEnd == NULL || pEnd < pStart)
	{
		return false;
	}

	while (pEnd >= pStart)
	{
		if (pEnd[0] == ' ')
		{
			pEnd--;
			break;
		}
		pEnd--;
	}

	if (pEnd == NULL || pEnd < pStart)
	{
		return false;
	}

	// �Ѿ�ȡ���˿�ʼ�ͽ�����λ��
	int nObjectLen = pEnd - pStart + 1;
	char *pszObject = new char[nObjectLen + 1]; ASSERT(pszObject);
	ZeroMemory(pszObject, nObjectLen + 1);

	// UTF8����
	//��ȡ�����Data����
	std::string strDataTmp;
	for (int i = 0; i < nObjectLen; ++i)
	{
		if (pStart[i] == '?')
		{
			// '?' �����ǲ���,����.
			//
			int nDataLen = nObjectLen - i - 1;  //�������� ��
			char *pszUrlData = new char[nDataLen + 1]; ASSERT(pszUrlData);
			ZeroMemory(pszUrlData, nDataLen + 1);
			memcpy(pszUrlData, pStart+ i +1, nDataLen );
			strDataTmp = pszUrlData;
			delete[] pszUrlData;
			break;
		}
	}

	if ( !wstrUrlObject.compare(_T("/Login")) )
	{
		int nPosName , nPosPassWd , nPosAnd;
		nPosName = strDataTmp.find("UserName") + sizeof("UserName");  //�ַ���ĩβ�С�\0�� ֱ����� �൱�����=
		nPosPassWd = strDataTmp.find("PassWd") + sizeof("PassWd") ;
		nPosAnd = strDataTmp.find('&');

		std::string strUserName, strPassWd;
		strUserName = strDataTmp.substr(nPosName , (nPosAnd - nPosName));
		strPassWd = strDataTmp.substr(nPosPassWd, strDataTmp.length() - nPosPassWd );

		vecRetData.push_back(strUserName);
		vecRetData.push_back(strPassWd);

	}
	else if ( !wstrUrlObject.compare(_T("/Main/GetDeviceList")) )
	{
		int i = 0;
	}



	return true;

}

std::string CHTTPRequest::GetField(const char* pszKey)
{
	// ������ֶ�Ӧ���Ǵӻ��п�ʼ,: ����.
	std::string strKey("\r\n");
	strKey += pszKey; strKey += "; "; //ð�ź����и��ո�.
	std::string strValue("");

	// �ҵ��ֶεĿ�ʼ
	const char* pszStart = strstr(m_pData, strKey.c_str());
	if(pszStart == NULL) return strValue;
	pszStart += strKey.size();

	// �ҵ��ֶν���
	const char* pszEnd = strstr(pszStart, "\r\n");
	if(pszEnd == NULL) return strValue;

	strValue.assign(pszStart, pszEnd - pszStart);
	return strValue;
}

BOOL CHTTPRequest::GetRange(__int64 &lFrom, __int64 &lTo)
{
	__int64 nFrom = 0;
	__int64 nTo = -1; // -1 ��ʾ�����һ���ֽ�.

	const char* lpszRequest = m_pData;
	const char* pRange = strstr(lpszRequest, "\r\nRange: bytes=");
	if(pRange)
	{
		/*
		The first 500 bytes (byte offsets 0-499, inclusive):
		bytes=0-499
		The second 500 bytes (byte offsets 500-999, inclusive):
		bytes=500-999
		The final 500 bytes (byte offsets 9500-9999, inclusive):
		bytes=-500
		bytes=9500-
		The first and last bytes only (bytes 0 and 9999):
		bytes=0-0,-1
		Several legal but not canonical specifications of the second 500 bytes (byte offsets 500-999, inclusive):
		bytes=500-600,601-999
		bytes=500-700,601-999
		*/

		pRange += strlen("\r\nRange: bytes=");
		const char *pMinus = strchr(pRange, '-');
		if(pMinus)
		{
			char szFrom[200], szTo[200];
			memset(szFrom, 0, 200);
			memset(szTo, 0, 200);
			memcpy(szFrom, pRange, pMinus - pRange);
			nFrom = _atoi64(szFrom);

			pMinus++;
			pRange = strstr(pMinus, "\r\n");
			if(pMinus + 1 == pRange)
			{
				nTo = -1;
			}
			else
			{
				memcpy(szTo, pMinus, pRange - pMinus);
				nTo = _atoi64(szTo);
				if(nTo <= 0) nTo = -1;
			}

			lFrom = nFrom;
			lTo = nTo;

			return TRUE;
		}
		else
		{
		}
	}
	else
	{
	}
	return FALSE;
}
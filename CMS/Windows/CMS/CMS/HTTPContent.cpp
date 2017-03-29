/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#include "StdAfx.h"
#include "HTTPContent.h"
#include "HTTPDef.h"
#include <io.h>

#define OPEN_NONE 0
#define OPEN_FILE 1
#define OPEN_TEXT 2
#define OPEN_BINARY 3
#define OPEN_HTML 4
#define OPEN_DIR 5

CHTTPContent::CHTTPContent(void)
{
	m_strContentType = "";
	m_strFileName = L"";

	m_pFile = NULL;
	m_nBufferSize = 0;
	m_pData = NULL;
	m_nReadPos = 0;
	m_nWritePos = 0;
	m_nOpenType = OPEN_NONE;
}

CHTTPContent::~CHTTPContent(void)
{
	Close();
}

BOOL CHTTPContent::OpenFile(const wchar_t* pszFileName)
{
	__int64 lFrom = 0;
	__int64 lTo = -1;
	return OpenFile(pszFileName, lFrom, lTo);
}

BOOL CHTTPContent::OpenFile(const wchar_t* pszFileName, __int64 &lFrom, __int64 &lTo)
{
	if(OPEN_NONE != m_nOpenType) return FALSE;
	std::string strFileName = WtoA(pszFileName);

	ASSERT(m_pFile == NULL);
	m_pFile = fopen(strFileName.c_str(), "rb");
	if(NULL == m_pFile)
	{
	}
	else
	{
		if( 0 == _stat32i64(strFileName.c_str(), &m_FileInf)) // 32 bit time, 64 bit size
		{
			m_nOpenType = OPEN_FILE;
			m_strFileName = pszFileName;
			
			m_lFrom = lFrom;
			m_lTo = lTo;
			
			// lFrom Ӧ�ô���0��С���ļ��ĳ���
			if(m_lFrom > 0 && m_lFrom < m_FileInf.st_size) _fseeki64(m_pFile, m_lFrom, SEEK_SET);
			else m_lFrom = 0;
			
			// lTo Ӧ�ô��ڵ��� lFrom��С���ļ��ĳ���.
			if(m_lTo >= m_lFrom && m_lTo <  m_FileInf.st_size)
			{
			}
			else
			{
				m_lTo = m_FileInf.st_size - 1;
			}

			// ����, -1��ʾ�ļ���β
			lFrom = m_lFrom;
			if(m_lTo == m_FileInf.st_size - 1) lTo = -1;
			else lTo = m_lTo;
		}
		else
		{
			// _stat ����ʧ��, û������,��Ϊ fopen() ����.
			fclose(m_pFile);
			m_pFile = NULL;
			ASSERT(0);
		}
	}

	return OPEN_NONE != m_nOpenType;
}

BOOL CHTTPContent::OpenText(const char* pszText, int nSize)
{
	if(OPEN_NONE != m_nOpenType) return FALSE;

	ASSERT(m_pData == NULL);
	if( nSize == Write( (void*)pszText, nSize) )
	{
		m_nOpenType = OPEN_TEXT;
	}

	return OPEN_NONE != m_nOpenType;
}

BOOL CHTTPContent::OpenHtml(const char* pszHtml, int nSize)
{
	if(OPEN_NONE != m_nOpenType) return FALSE;

	ASSERT(m_pData == NULL);
	if( nSize == Write( (void*)pszHtml, nSize) )
	{
		m_nOpenType = OPEN_HTML;
	}

	return OPEN_NONE != m_nOpenType;
}

BOOL CHTTPContent::OpenDir(const std::wstring &strUrl, const std::wstring &strFilePath)
{
	if(OPEN_NONE != m_nOpenType) return FALSE;
	ASSERT(m_pData == NULL);

	wchar_t buffer[_MAX_PATH + 100] = {0};
	char sizeBuf[_MAX_PATH + 100] = {0};

	// ����һ��UTF8 HTML�ı���,�������ļ��б�.
	
	// 1. ���HTMLͷ,��ָ��UTF-8�����ʽ
	WriteString("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/></head>");
	WriteString("<body>");

	// 2. ���·��
	//(1). �����һ�� ��Ŀ¼
	WriteString("<A href=\"/\">/</A>");

	//(2). ����Ŀ¼
	std::wstring::size_type st = 1;
	std::wstring::size_type stNext = 1;
	while( (stNext = strUrl.find(L'/', st)) != std::wstring::npos)
	{
		std::wstring strDirName =  strUrl.substr(st, stNext - st + 1);
		std::wstring strSubUrl = strUrl.substr(0, stNext + 1);

		WriteString("&nbsp;|&nbsp;");

		WriteString("<A href=\"");
		WriteString(WtoUTF8(strSubUrl.c_str()).c_str());
		WriteString("\">");
		WriteString(WtoUTF8(strDirName.c_str()).c_str());
		WriteString("</A>");
		
		// ��һ��Ŀ¼
		st = stNext + 1;
	}
	WriteString("<br /><hr />");

	// 3. �г���ǰĿ¼�µ������ļ�
	std::wstring strFullName;
	strFullName = strFilePath;
	if(strFullName.back() != L'\\') strFullName += L'\\'; // ���������'\\'��β���ļ�·��,����. ע��һ��ԭ��:URL����б�ָܷ�;�ļ����Է�б�ָܷ�
	strFullName += L"*";

	std::string strFiles(""); // ��ͨ�ļ�д������ַ�����.

	_wfinddata64_t fileinfo;
	intptr_t findRet = _wfindfirst64(strFullName.c_str(), &fileinfo);
	if( -1 != findRet )
	{
		do 
		{
			// ���� . �ļ�
			if( _tcsicmp(fileinfo.name, L".") == 0 || 0 == _tcsicmp(fileinfo.name, L"..") )
			{
				continue;
			}

			// ����ϵͳ�ļ��������ļ�
			if( fileinfo.attrib & _A_SYSTEM || fileinfo.attrib & _A_HIDDEN )
			{
				continue;
			}

			// �����Ŀ¼����
			if( fileinfo.attrib & _A_SUBDIR )
			{
				// �������Ŀ¼,ֱ��д��

				// ����޸�ʱ��
				_wctime64_s( buffer, _countof(buffer), &fileinfo.time_write );
				WriteString(WtoUTF8(buffer).c_str());

				// Ŀ¼����Ҫת��ΪUTF8����
				wsprintf(buffer, L"%s/", fileinfo.name);
				std::string fileurl = WtoUTF8(strUrl.c_str());
				std::string filename = WtoUTF8(buffer);

				WriteString("&nbsp;&nbsp;");
				WriteString("<A href=\"");
				WriteString(fileurl.c_str());
				WriteString(filename.c_str());
				WriteString("\">");
				WriteString(filename.c_str());
				WriteString("</A>");

				// д��Ŀ¼��־
				WriteString("&nbsp;&nbsp;[DIR]");

				// ����
				WriteString("<br />");
			}
			else
			{
				// ��ͨ�ļ�,д�뵽һ��������ַ���string������,ѭ�����ٺϲ�.����,���е�Ŀ¼����ǰ��,�ļ��ں���
				_wctime64_s( buffer, _countof(buffer), &fileinfo.time_write );
				strFiles += WtoUTF8(buffer);

				// �ļ���ת��ΪUTF8������д��
				std::string filename = WtoUTF8(fileinfo.name);
				std::string fileurl = WtoUTF8(strUrl.c_str());

				strFiles += "&nbsp;&nbsp;";
				strFiles += "<A href=\"";
				strFiles += fileurl;
				strFiles += filename;
				strFiles += "\">";
				strFiles += filename;
				strFiles += "</A>";

				// �ļ���С
				// ע: ����Windows�� wsprintf ��֧�� %f ����,����ֻ���� sprintf ��
				double filesize = 0;
				if( fileinfo.size >= G_BYTES)
				{
					filesize = (fileinfo.size * 1.0) / G_BYTES;
					sprintf(sizeBuf, "%.2f&nbsp;GB", filesize);
				}
				else if( fileinfo.size >= M_BYTES ) // MB
				{
					filesize = (fileinfo.size * 1.0) / M_BYTES;
					sprintf(sizeBuf, "%.2f&nbsp;MB", filesize);
				}
				else if( fileinfo.size >= K_BYTES ) //KB
				{
					filesize = (fileinfo.size * 1.0) / K_BYTES;
					sprintf(sizeBuf, "%.2f&nbsp;KB", filesize);
				}
				else // Bytes
				{
					sprintf(sizeBuf, "%lld&nbsp;Bytes", fileinfo.size);
				}
			
				strFiles += "&nbsp;&nbsp;";
				strFiles += sizeBuf;

				// ����
				strFiles += "<br />";
			}
		} while ( -1 != _wfindnext64(findRet, &fileinfo));
		
		_findclose(findRet);
	}

	// ���ļ��ַ���д�뵽 Content ��.
	if(strFiles.size() > 0)
	{
		WriteString(strFiles.c_str());
	}

	// 4. ���������־.
	WriteString("</body></html>");

	m_nOpenType = OPEN_DIR;
	return TRUE;
}

BOOL CHTTPContent::OpenData(const char* pData, int nSize)
{
	if(OPEN_NONE != m_nOpenType) return FALSE;

	ASSERT(m_pData == NULL);
	if( nSize == Write( (void*)pData, nSize) )
	{
		m_nOpenType = OPEN_BINARY;
	}

	return OPEN_NONE != m_nOpenType;
}

void CHTTPContent::Close()
{
	//ASSERT( OPEN_NONE != m_nOpenType );

	m_strContentType = "";
	m_strFileName = L"";

	if(m_pFile != NULL) fclose(m_pFile);
	m_pFile = NULL;
	if(m_pData != NULL) delete[] m_pData;
	m_pData = NULL;
	m_nBufferSize = 0;

	m_nReadPos = 0;
	m_nWritePos = 0;
	m_nOpenType = OPEN_NONE;
}

BOOL CHTTPContent::IsOpen()
{
	return  OPEN_NONE != m_nOpenType;
}

std::string CHTTPContent::getContentTypeFromFileName(const char* pszFileName)
{
	std::string strType = "application/octet-stream";

	const char *pExt = strrchr(pszFileName, '.');
	if(pExt && strlen(pExt) < 19)
	{
		char szExt[20];
		strcpy(szExt, pExt + 1);

		if(stricmp(szExt, "jpg") == 0)
		{
			strType =  "image/jpeg";
		}
		else if(stricmp(szExt, "txt") == 0)
		{
			strType = "text/plain";
		}
		else if(stricmp(szExt, "htm") == 0)
		{
			strType = "text/html";
		}
		else if(stricmp(szExt, "html") == 0)
		{
			strType = "text/html";
		}
		else if(stricmp(szExt, "gif") == 0)
		{
			strType = "image/gif";
		}
		else if(stricmp(szExt, "png") == 0)
		{
			strType = "image/png";
		}
		else if(stricmp(szExt, "bmp") == 0)
		{
			strType = "image/x-xbitmap";
		}
		else
		{
		}
	}

	return strType;
}

int CHTTPContent::WriteString(const char* pszString)
{
	return Write((void *)pszString, strlen(pszString));
}

int CHTTPContent::Write(void* pData, int nSize) // Ϊ����֧�� PUT, POST ������׼��.
{
	ASSERT(pData && nSize > 0);

	m_nOpenType = OPEN_TEXT;

	int nLeft = m_nBufferSize - m_nWritePos;
	
	// �ռ䲻��,��������.
	if(nLeft < nSize)
	{
		int nNewIncSize = UNIT_BUFFER_SIZE;
		while( nNewIncSize < nSize) { nNewIncSize += UNIT_BUFFER_SIZE; }

		if( m_nBufferSize + nNewIncSize > MAX_BUFFER_SIZE)
		{
			// ������󳤶�.
			ASSERT(0);
			return 0;
		}
		
		char* pTmp = m_pData;
		m_pData = new char[m_nBufferSize + nNewIncSize];
		if(m_pData != NULL)
		{
			memcpy(m_pData, pTmp, m_nWritePos);
			delete[] pTmp;
			m_nBufferSize += nNewIncSize;
		}
		else
		{
			// �޷����䵽�ڴ�
			ASSERT(0);
			m_pData = pTmp;
			return 0;
		}
	}

	// �ռ�������֮��,��������
	memcpy(m_pData + m_nWritePos, pData, nSize);
	m_nWritePos += nSize;
	return nSize;
}


std::string CHTTPContent::GetContentType()
{
	std::string strType("application/octet-stream");

	if(m_nOpenType == OPEN_FILE)
	{
		std::string strFileName = WtoA(m_strFileName);
		strType = getContentTypeFromFileName(strFileName.c_str());
	}
	else if(m_nOpenType == OPEN_TEXT)
	{
		strType = "text/plain";
	}
	else if(m_nOpenType == OPEN_HTML)
	{
		strType = "text/html";
	}
	else if(m_nOpenType == OPEN_DIR)
	{
		strType = "text/html";
	}
	else
	{
	}

	return strType;
}


__int64 CHTTPContent::GetContentLength()
{
	__int64 nLength = 0;

	if(m_nOpenType == OPEN_FILE)
	{
		nLength = m_lTo - m_lFrom + 1;
	}
	else
	{
		nLength = m_nWritePos;
	}

	return nLength;
}

std::string CHTTPContent::GetLastModified()
{
	__int64 ltime;

	if(m_nOpenType == OPEN_FILE)
	{
		ltime = m_FileInf.st_mtime;
	}
	else
	{
		_time64( &ltime );
	}

	return FormatHTTPDate(&ltime);
}

std::string CHTTPContent::GetContentRange()
{
	char szRanges[300] = {0};
	if(m_nOpenType == OPEN_FILE)
	{
		sprintf(szRanges, "bytes %lld-%lld/%lld", m_lFrom, m_lTo, m_FileInf.st_size);
	}
	else
	{
		
	}
	return szRanges;
}


BOOL CHTTPContent::IsEOF()
{
	if(m_pFile)
	{
		if(feof(m_pFile))
		{
			return TRUE;
		}
		else
		{
			return _ftelli64(m_pFile) >= m_lTo;
		}
	}
	else
	{
		return m_nReadPos >= m_nWritePos;
	}
}

int CHTTPContent::Read(void* pData, int nSize)
{
	ASSERT(nSize);

	if(m_pFile)
	{
		int nRet = 0;
		__int64 lCurPos = _ftelli64(m_pFile);  // ftell()���ص��ǵ�ǰָ���λ��,ָ���һ��δ�����ֽ�
		__int64 lLeftSize = m_lTo - lCurPos + 1; // �ļ���ʣ����ֽ���

		if(nSize > lLeftSize)
		{
			nSize = (int)lLeftSize;// �˴��ǰ�ȫ��
		}
		return fread(pData, 1, nSize, m_pFile); 
	}
	else
	{
		int nReadSize = m_nWritePos - m_nReadPos;
		ASSERT(nReadSize);
		if(nReadSize > nSize) 
		{
			nReadSize = nSize;
		}

		memcpy(pData, m_pData + m_nReadPos, nReadSize);
		m_nReadPos += nReadSize;
		return nReadSize;
	}
}

std::string CHTTPContent::GetETag()
{
	std::string strETag("");
	if(OPEN_FILE == m_nOpenType)
	{
		char szLength[201] = {0};
		//_ltoa_s((m_lTo - m_lFrom + 1), szLength, 200, 10);
		_i64toa(m_FileInf.st_size, szLength, 10);

		// ������ļ�, �����ļ���С���޸����ڴ���. [ETag: ec5ee54c00000000:754998030] �޸�ʱ���HEXֵ:�ļ�����
		// ȷ��ͬһ����Դ�� ETag ��ͬһ��ֵ.
		// ��ʹ�ͻ��������ֻ�������Դ��һ����.
		// �ϵ������ͻ��˸��� ETag ��ֵȷ�����صļ��������ǲ���ͬһ���ļ�.
		strETag = ToHex((const unsigned char*)(&m_FileInf.st_mtime), sizeof(m_FileInf.st_mtime));
		strETag += ":";
		strETag += szLength;
	}
	else
	{
		char szLength[201] = {0};
		 _ltoa_s(m_nWritePos, szLength, 200, 10); // �ڴ�����û��Ҫ�� __int64

		// ������ڴ�����, ���ݴ�С��ȡ���ɸ��ֽڵ�16�����ַ�����.
		unsigned char szValue[ETAG_BYTE_SIZE + 1];

		for(int i = 0; i < ETAG_BYTE_SIZE; ++i)
		{
			int nUnit = m_nWritePos / ETAG_BYTE_SIZE;
			szValue[i] = m_pData[nUnit * i];
		}

		strETag = ToHex(szValue, ETAG_BYTE_SIZE);
		strETag += ":";
		strETag += szLength;
	}

	return strETag;
}

/*
int CHTTPContent::Seek(int nOffset);
*/
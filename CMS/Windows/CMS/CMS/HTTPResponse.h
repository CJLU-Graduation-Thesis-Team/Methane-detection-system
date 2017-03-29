/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#pragma once
#include "HTTPDef.h"

class CHTTPContent;
class CHTTPResponse
{
public:
	CHTTPResponse(const wchar_t* pszIP, unsigned int nPort);
	virtual ~CHTTPResponse(void);

protected:
	HTTP_METHOD m_Method;
	std::wstring m_strIP;
	unsigned int m_nPort;
	SERVER_CODE m_nServerCode;

	char* m_pData; // ��Ӧͷ����
	int m_nReadPos; // ��ȡλ��
	int m_nHeaderSize; // ��Ӧͷ�ĳ���
	
	CHTTPContent* m_pContent;

	std::string getFirstLine(SERVER_CODE nServerCode); // �����ƶ��� Servercode ���ض�Ӧ�� HTTP��Ӧͷ�ĵ�һ��,�������з�.
	std::string getContentType(const char* pszFileName); 

public:
	SERVER_CODE SetServerCode(SERVER_CODE nNewCode); // ����HTTP�������Ӧ��.
	HTTP_METHOD SetMethod(HTTP_METHOD mh);
	SERVER_CODE GetServerCode() { return m_nServerCode;}

	BOOL CookResponseWithXMl(std::string strRetXMl);  //��XM��������Ӧ
	BOOL CookResponse(); // ������Ӧ
	int PopData(void* pData, int nSize); // ��������
	BOOL IsEOF(); // �ж��Ƿ񵽽�β

	void AttachContent(CHTTPContent *pContent);
};

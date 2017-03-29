/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#pragma once

#include "HTTPDef.h"


class CHTTPRequest
{
public:
	CHTTPRequest(const wchar_t* pszIP, unsigned int nPort);
	virtual ~CHTTPRequest(void);

protected:
	std::wstring m_strIP; // �ͻ������IP��ַ
	unsigned int m_nPort; // �˿�

	char* m_pData; // ������
	int m_nPos; // �������α�.

public:
	int PushData(const char* pData, int nSize); // �׽����յ����ݺ�,���뵽 HTTP Request ʵ����.

	BOOL IsEnd(); // ����ͷ�Ƿ����.
	BOOL Verify(); // ��֤����ͷ����Ч��.

	HTTP_METHOD GetMethod(); // ����HTTP ����
	std::wstring GetUrlObject(); // ���ؿͻ�������Ķ���(�Ѿ�����UTF8����,���Է��ؿ��ַ���)
	bool CHTTPRequest::GetUrlData(std::wstring wstrUrlObject , std::vector<std::string>& vecRetData);  //���ؿͻ��������Ǹ���������  (HTTPͷ��ֻ��ANSI�ַ�,���Է���string������).
	std::string GetField(const char* pszKey); // ��������ͷ�е�һ���ֶ�(HTTPͷ��ֻ��ANSI�ַ�,���Է���string).
	BOOL GetRange(__int64 &lFrom, __int64 &lTo);

	const std::wstring& GetIP() { return m_strIP; }
	unsigned int GetPort() { return m_nPort; }
};

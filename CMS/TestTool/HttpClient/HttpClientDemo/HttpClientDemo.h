
// HttpClientDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CHttpClientDemoApp: 
// �йش����ʵ�֣������ HttpClientDemo.cpp
//

class CHttpClientDemoApp : public CWinApp
{
public:
	CHttpClientDemoApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CHttpClientDemoApp theApp;
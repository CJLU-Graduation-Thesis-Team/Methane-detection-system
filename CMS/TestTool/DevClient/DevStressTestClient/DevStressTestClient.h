// DevStressTestClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif


// CDevStressTestClientApp:
// �йش����ʵ�֣������ DevStressTestClient.cpp
//

class CDevStressTestClientApp : public CWinApp
{
public:
	CDevStressTestClientApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDevStressTestClientApp theApp;
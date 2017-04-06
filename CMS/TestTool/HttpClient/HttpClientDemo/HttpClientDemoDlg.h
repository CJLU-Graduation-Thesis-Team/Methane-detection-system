
// HttpClientDemoDlg.h : ͷ�ļ�
//

#pragma once

#include "TCP.h"


// CHttpClientDemoDlg �Ի���
class CHttpClientDemoDlg : public CDialogEx
{
// ����
public:
	CHttpClientDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

	~CHttpClientDemoDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HTTPCLIENTDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	CTCP m_tcp;
	//��
	CRITICAL_SECTION m_csSend;
	//SendMsg
	std::string strReqHttpMsg;




// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnReg();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnAddDev();
	afx_msg void OnBnClickedBtnReqDevList();
	afx_msg void OnBnClickedBtnReqDevStatus();
	afx_msg void OnBnClickedBtnSetThor();
};

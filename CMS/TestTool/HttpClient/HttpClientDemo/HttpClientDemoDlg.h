
// HttpClientDemoDlg.h : 头文件
//

#pragma once

#include "TCP.h"


// CHttpClientDemoDlg 对话框
class CHttpClientDemoDlg : public CDialogEx
{
// 构造
public:
	CHttpClientDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

	~CHttpClientDemoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HTTPCLIENTDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	CTCP m_tcp;
	//锁
	CRITICAL_SECTION m_csSend;
	//SendMsg
	std::string strReqHttpMsg;




// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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

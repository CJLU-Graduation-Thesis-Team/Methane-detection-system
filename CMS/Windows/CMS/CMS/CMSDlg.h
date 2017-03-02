
// CMSDlg.h : 头文件
//

#include "IOCPModel.h"

#pragma once


// CCMSDlg 对话框
class CCMSDlg : public CDialogEx
{
// 构造
public:
	CCMSDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CMS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	// 初始化Socket库以及界面信息
	void Init();

	// 初始化List控件
	void InitListCtrl();


public:
	// 当前客户端有新消息到来的时候，在主界面中显示新到来的信息(在类CIOCPModel中调用)
	// 为了减少界面代码对效率的影响，此处使用了内联
	inline void AddInformation(const CString strInfo)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
		pList->InsertItem(0, strInfo);
	}

private:
	CIOCPModel m_IOCP;                         // 主要对象，完成端口模型

public:
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
};

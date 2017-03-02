
// CMSDlg.h : ͷ�ļ�
//

#include "IOCPModel.h"

#pragma once


// CCMSDlg �Ի���
class CCMSDlg : public CDialogEx
{
// ����
public:
	CCMSDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CMS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	// ��ʼ��Socket���Լ�������Ϣ
	void Init();

	// ��ʼ��List�ؼ�
	void InitListCtrl();


public:
	// ��ǰ�ͻ���������Ϣ������ʱ��������������ʾ�µ�������Ϣ(����CIOCPModel�е���)
	// Ϊ�˼��ٽ�������Ч�ʵ�Ӱ�죬�˴�ʹ��������
	inline void AddInformation(const CString strInfo)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
		pList->InsertItem(0, strInfo);
	}

private:
	CIOCPModel m_IOCP;                         // ��Ҫ������ɶ˿�ģ��

public:
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
};

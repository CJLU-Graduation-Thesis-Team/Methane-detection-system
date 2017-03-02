
// CMSDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CMS.h"
#include "CMSDlg.h"
#include "afxdialogex.h"

#include "IOCPModel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCMSDlg �Ի���



CCMSDlg::CCMSDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CMS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCMSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCMSDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_EXIT, &CCMSDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_START, &CCMSDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CCMSDlg::OnBnClickedBtnStop)
END_MESSAGE_MAP()


// CCMSDlg ��Ϣ�������

BOOL CCMSDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//��ʼ��������Ϣ
	this->Init();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCMSDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CCMSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//////////////////////////////////////////////////////////////////////
// ��ʼ��Socket���Լ�������Ϣ
void CCMSDlg::Init()
{
	// ��ʼ��Socket��
	if (false == m_IOCP.LoadSocketLib())
	{
		AfxMessageBox(_T("����Winsock 2.2ʧ�ܣ����������޷����У�"));
		PostQuitMessage(0);
	}

	// ���ñ���IP��ַ
	SetDlgItemText(IDC_STATIC_IP, m_IOCP.GetLocalIP());
	// ����Ĭ�϶˿�
	SetDlgItemInt(IDC_EDIT_PORT, DEFAULT_PORT);
	// ��ʼ���б�
	this->InitListCtrl();
	// ��������ָ��(Ϊ�˷����ڽ�������ʾ��Ϣ )
	m_IOCP.SetMainDlg(this);
}

///////////////////////////////////////////////////////////////////////
//	��ʼ��List Control
void CCMSDlg::InitListCtrl()
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
	pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	pList->InsertColumn(0, "INFORMATION", LVCFMT_LEFT, 500);
}


void CCMSDlg::OnBnClickedBtnExit()
{
	// ֹͣ����
	m_IOCP.Stop();

	CDialog::OnCancel();
}


void CCMSDlg::OnBnClickedBtnStart()
{
	if (false == m_IOCP.Start())
	{
		AfxMessageBox(_T("����������ʧ�ܣ�"));
		return;
	}

	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
}


void CCMSDlg::OnBnClickedBtnStop()
{
	m_IOCP.Stop();

	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
}


///////////////////////////////////////////////////////////////////////
//	ϵͳ�˳���ʱ��Ϊȷ����Դ�ͷţ�ֹͣ���������Socket���
void CCMSDlg::OnDestroy()
{
	OnBnClickedBtnExit();

	CDialog::OnDestroy();
}

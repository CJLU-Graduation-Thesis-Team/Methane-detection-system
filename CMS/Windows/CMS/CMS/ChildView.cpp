// ChildView.cpp : CChildView ���ʵ��
//

#include "stdafx.h"
#include "CMS.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_EDIT 10001


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CChildView ��Ϣ�������

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // ���ڻ��Ƶ��豸������
	
	// TODO: �ڴ˴������Ϣ����������
	
	// ��ҪΪ������Ϣ������ CWnd::OnPaint()
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rc(0, 0, lpCreateStruct->cx, lpCreateStruct->cy);
	if ( !m_edt.Create(ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL , rc, this, IDC_EDIT) ) return -1;

	/*LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);*/
	m_font.CreateStockObject(DEFAULT_GUI_FONT);
	m_edt.SetFont(&m_font, FALSE);
	m_edt.SetReadOnly(TRUE);

	return 0;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_edt.MoveWindow(0, 0, cx, cy);
}

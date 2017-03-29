#include "stdafx.h"
#include "HTTPWork.h"


CHTTPWork::CHTTPWork()
{
}


CHTTPWork::~CHTTPWork()
{
}

//Get
bool CHTTPWork::Login(std::string strName, std::string strPwd, std::string strRetXml)
{
	DB db_manger;
	CString cstrSql;
	CMarkup Xml;
	_variant_t  vid;

	db_manger.OnInitADOConn();
	//�Ȳ����û��Ƿ��Ѵ���
	_RecordsetPtr m_pRecordset;
	cstrSql.Format(_T("SELECT id FROM user WHERE strUserName = '%s'"), AtoW(strName.c_str()).c_str());
	m_pRecordset = db_manger.GetRecordSet(cstrSql.GetBuffer());

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vid = m_pRecordset->GetCollect(_T("Id"));
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}

	if (vid.vt == VT_EMPTY || vid.vt == VT_NULL)//�û�������
	{
		//���ɷ��ذ�����
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>")); \
			Xml.AddElem(_T("Dec"), _T("�û�������") );
		Xml.AddElem(_T("Ret"), 201);
		strRetXml = WtoA(Xml.GetDoc());

	}
	else //�û��Ѿ�����
	{
		_RecordsetPtr m_pRecordset;
		cstrSql.Format(_T("SELECT id FROM user WHERE strUserName = '%s' AND strPassWd= '%s'" ), AtoW(strName.c_str()).c_str() , AtoW(strPwd.c_str()).c_str());
		m_pRecordset = db_manger.GetRecordSet(cstrSql.GetBuffer());

		try
		{
			while (!m_pRecordset->adoEOF)
			{
				vid = m_pRecordset->GetCollect(_T("Id"));
				m_pRecordset->MoveNext();
			}
		}
		catch (_com_error e)
		{
			// ��ʾ������Ϣ
			AfxMessageBox(e.Description());
		}

		if (vid.vt == VT_EMPTY || vid.vt == VT_NULL)//�������
		{
			Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));
			Xml.AddElem(_T("Dec"), _T("�������"));
			Xml.AddElem(_T("Ret"), 201);
			strRetXml = WtoA(Xml.GetDoc());
		}
		else
		{
			Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));
			Xml.AddElem(_T("Dec"), _T("��½�ɹ�"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
		}

	}
	db_manger.ExitConnect();
	return true;
}

bool CHTTPWork::GetDeviceList(std::string strName, std::string strRetXml)
{

	return true;
}

bool CHTTPWork::GetDeviceStatus(std::string strName, int nDeviceSn, std::string strRetXml)
{
	return true;
}

//Post
bool CHTTPWork::SetDeviceThreshold(std::string strName, int nDeviceSn, double dSetData)
{

	return true;
}

bool CHTTPWork::AddDevice(std::string strName, int nDeviceSn)
{

	return true;
}

bool CHTTPWork::AddUser(std::string strName, std::string strPwd,std::string& strRetXml)
{
	DB db_manger;
	CString cstrSql;
	CMarkup Xml;
	_variant_t  vid;

	db_manger.OnInitADOConn();
	//�Ȳ����û��Ƿ��Ѵ���
	_RecordsetPtr m_pRecordset;
	cstrSql.Format(_T("SELECT id FROM user WHERE strUserName = '%s'"), AtoW(strName.c_str()).c_str() );
	m_pRecordset = db_manger.GetRecordSet(cstrSql.GetBuffer());

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vid = m_pRecordset->GetCollect(_T("Id"));
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}

	if (vid.vt == VT_EMPTY || vid.vt == VT_NULL)//�û�������
	{
		//����û�
		cstrSql.Format(_T("INSERT INTO user  (strUserName , strPassWd)  VALUES ('%s'  , '%s')"), AtoW(strName.c_str()).c_str(), AtoW(strPwd.c_str()).c_str());
		if (!db_manger.ExecuteSQL(cstrSql.GetBuffer()))
		{//����ִ��ʧ��
			db_manger.ExitConnect();
			LOGGER_CERROR(theLogger, _T("ע���û�ʧ��.\r\n"), GetLastError());
			Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>")); \
			Xml.AddElem(_T("Dec"), _T("ע��ɹ�"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
			return false;
		}
		//���ɷ��ذ�����
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));\
		Xml.AddElem(_T("Dec"), _T("ע��ɹ�"));
		Xml.AddElem(_T("Ret"), 200);
		strRetXml = WtoA(Xml.GetDoc());

	}
	else //�û��Ѿ�����
	{
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));
		Xml.AddElem(_T("Dec"),_T("�˺��Ѿ�����"));
		Xml.AddElem(_T("Ret"), 200);
		strRetXml = WtoA(Xml.GetDoc());
	}
	db_manger.ExitConnect();
	return true;
}
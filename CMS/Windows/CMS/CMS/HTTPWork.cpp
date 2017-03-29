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
	//先查找用户是否已存在
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
		// 显示错误信息
		AfxMessageBox(e.Description());
	}

	if (vid.vt == VT_EMPTY || vid.vt == VT_NULL)//用户不存在
	{
		//生成返回包数据
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>")); \
			Xml.AddElem(_T("Dec"), _T("用户不存在") );
		Xml.AddElem(_T("Ret"), 201);
		strRetXml = WtoA(Xml.GetDoc());

	}
	else //用户已经存在
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
			// 显示错误信息
			AfxMessageBox(e.Description());
		}

		if (vid.vt == VT_EMPTY || vid.vt == VT_NULL)//密码错误
		{
			Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));
			Xml.AddElem(_T("Dec"), _T("密码错误"));
			Xml.AddElem(_T("Ret"), 201);
			strRetXml = WtoA(Xml.GetDoc());
		}
		else
		{
			Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));
			Xml.AddElem(_T("Dec"), _T("登陆成功"));
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
	//先查找用户是否已存在
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
		// 显示错误信息
		AfxMessageBox(e.Description());
	}

	if (vid.vt == VT_EMPTY || vid.vt == VT_NULL)//用户不存在
	{
		//添加用户
		cstrSql.Format(_T("INSERT INTO user  (strUserName , strPassWd)  VALUES ('%s'  , '%s')"), AtoW(strName.c_str()).c_str(), AtoW(strPwd.c_str()).c_str());
		if (!db_manger.ExecuteSQL(cstrSql.GetBuffer()))
		{//操作执行失败
			db_manger.ExitConnect();
			LOGGER_CERROR(theLogger, _T("注册用户失败.\r\n"), GetLastError());
			Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>")); \
			Xml.AddElem(_T("Dec"), _T("注册成功"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
			return false;
		}
		//生成返回包数据
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));\
		Xml.AddElem(_T("Dec"), _T("注册成功"));
		Xml.AddElem(_T("Ret"), 200);
		strRetXml = WtoA(Xml.GetDoc());

	}
	else //用户已经存在
	{
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?>"));
		Xml.AddElem(_T("Dec"),_T("账号已经存在"));
		Xml.AddElem(_T("Ret"), 200);
		strRetXml = WtoA(Xml.GetDoc());
	}
	db_manger.ExitConnect();
	return true;
}
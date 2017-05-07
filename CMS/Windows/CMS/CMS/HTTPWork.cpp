#include "stdafx.h"
#include "HTTPWork.h"


CHTTPWork::CHTTPWork()
{
	m_DBManger.OnInitADOConn();
}


CHTTPWork::~CHTTPWork()
{
	m_DBManger.ExitConnect();
}

//Get
bool CHTTPWork::Login(std::string strName, std::string strPwd, std::string&  strRetXml)
{
	CMarkup Xml;
	_variant_t  vSql_ID;

	//�Ȳ����û��Ƿ��Ѵ���
	m_cstrSql.Format(_T("SELECT id FROM user WHERE strUserName = '%s'"), AtoW(strName.c_str()).c_str());
	m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vSql_ID = m_pRecordset->GetCollect(_T("Id"));
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}

	if (vSql_ID.vt == VT_EMPTY || vSql_ID.vt == VT_NULL)//�û�������
	{
		//���ɷ��ذ�����
		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r")); \
			Xml.AddElem(_T("Dec"), _T("UserName is not Exist") );
		Xml.AddElem(_T("Ret"), 201);
		strRetXml = WtoA(Xml.GetDoc());
	}
	else //�û��Ѿ�����
	{
		m_cstrSql.Format(_T("SELECT id FROM user WHERE strUserName = '%s' AND strPassWd= '%s'" ), AtoW(strName.c_str()).c_str() , AtoW(strPwd.c_str()).c_str());
		m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());

		_variant_t  vSql_PassID;


		try
		{
			while (!m_pRecordset->adoEOF)
			{
				vSql_PassID = m_pRecordset->GetCollect(_T("Id"));
				m_pRecordset->MoveNext();
			}
		}
		catch (_com_error e)
		{
			// ��ʾ������Ϣ
			AfxMessageBox(e.Description());
		}


		Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
		Xml.AddElem(_T("Root"));
		Xml.IntoElem();


		if (vSql_PassID.vt == VT_EMPTY || vSql_PassID.vt == VT_NULL)//�������
		{
			Xml.AddElem(_T("Dec"), _T("Error Pwd"));
			Xml.AddElem(_T("Ret"), 201);
			strRetXml = WtoA(Xml.GetDoc());
		}
		else
		{

			Xml.AddElem(_T("Dec"), _T("Login Ok"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
		}
	}

	return true;
}

bool CHTTPWork::GetDeviceList(std::string strName, std::string& strRetXml)
{
	CMarkup Xml;
	_variant_t  vSql_Data;
	std::vector<int> vecDev;
	std::vector<std::string> vecDevSn;

	m_cstrSql.Format(_T("SELECT dev.id  , dev.strSN FROM device dev LEFT  JOIN user usr ON dev.nUserId = usr.Id WHERE usr.strUserName = '%s' ORDER BY dev.id"), AtoW(strName.c_str()).c_str());
	m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());

	_RecordsetPtr m_pDevDataRecordset;  //���ؽ��ָ��

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vSql_Data = m_pRecordset->GetCollect(_T("Id"));
			vecDev.push_back(vSql_Data); 

			_variant_t  vSql_strData;
			vSql_strData = m_pRecordset->GetCollect(_T("strSN"));
			std::string  strSN = (_bstr_t)vSql_strData;
			vecDevSn.push_back(strSN);
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}


	Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
	Xml.AddElem(_T("Root"));
	Xml.IntoElem();

	if ( !vecDev.empty() )    //�����豸
	{
		Xml.AddElem(_T("Ret"), 200);

		Xml.AddElem(_T("DevList"));
		Xml.IntoElem();

		int nDevNum = vecDev.size();
		for (int i = 0; i < nDevNum; i++)
		{
			m_cstrSql.Format(_T("SELECT\
				dat.dRealData, dev.dThresholdValue, dat.dRealData - dev.dThresholdValue as BoolValue\
				FROM\
				DATA dat\
				LEFT JOIN device dev ON dev.id = dat.nDevId\
				WHERE\
				dat.nDevId = '%d'\
				ORDER BY dat.id DESC LIMIT 1"), vecDev.at(i));
			m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());
			if (m_pRecordset->adoEOF)  //û�в��ҵ���¼
			{
				Xml.AddElem(_T("Dev"));
				Xml.AddAttrib(_T("Id"), i);
				Xml.AddAttrib(_T("SN"), AtoW(vecDevSn.at(i)).c_str());
				Xml.AddChildElem(_T("RealData"));
			}
			else
			{
				_variant_t  vSql_RealData;
				vSql_RealData = m_pRecordset->GetCollect(_T("dRealData"));

				_variant_t  vSql_ThresholdData;
				vSql_ThresholdData = m_pRecordset->GetCollect(_T("dThresholdValue"));

				_variant_t  vSql_BOOLData;
				vSql_BOOLData = m_pRecordset->GetCollect(_T("BoolValue"));
				BOOL bBoolBeyond;
				bBoolBeyond = (BOOL)vSql_BOOLData;
				if (bBoolBeyond > 0)
				{
					bBoolBeyond = 1;
				}
				else if (bBoolBeyond < 0 )
				{
					bBoolBeyond = -1;
				}
				else
				{

				}

				Xml.AddElem(_T("Dev"));
				Xml.AddAttrib(_T("Id"), i);
				Xml.AddAttrib(_T("SN"), AtoW(vecDevSn.at(i)).c_str());
				Xml.AddChildElem(_T("RealData"), vSql_RealData);
				Xml.AddChildElem(_T("Threshold"), vSql_ThresholdData);

				Xml.AddChildElem(_T("BoolBeyond"), bBoolBeyond);

			}

		}

	}
	else  //�û���û���豸
	{
		Xml.AddElem(_T("Ret"), 202);

	}

	strRetXml = WtoA(Xml.GetDoc());

	return true;
}

bool CHTTPWork::GetDeviceStatus(std::string  strDeviceSn, std::string&  strRetXml)
{
	CMarkup Xml;
	_variant_t  vSql_Data;
	std::vector<int> vecDev;
	std::vector<std::string> vecDevSn;

	m_cstrSql.Format(_T("SELECT\
		dat.dRealData, dev.dThresholdValue, dat.dRealData - dev.dThresholdValue as BoolValue\
		FROM\
		DATA dat\
		LEFT JOIN device dev ON dev.id = dat.nDevId\
		WHERE\
		dev.strSN = '%s'\
		ORDER BY\
		dat.id DESC\
		LIMIT 1"), AtoW(strDeviceSn.c_str()).c_str());

	m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());


	Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
	Xml.AddElem(_T("Root"));
	Xml.IntoElem();

	if (m_pRecordset->adoEOF)  //û�в��ҵ���¼
	{
		Xml.AddElem(_T("Dev"));
		Xml.AddAttrib(_T("SN"), AtoW(strDeviceSn).c_str());
		Xml.AddChildElem(_T("RealData"));
	}
	else
	{

		_variant_t  vSql_RealData;
		vSql_RealData = m_pRecordset->GetCollect(_T("dRealData"));

		_variant_t  vSql_ThresholdData;
		vSql_ThresholdData = m_pRecordset->GetCollect(_T("dThresholdValue"));

		_variant_t  vSql_BOOLData;
		vSql_BOOLData = m_pRecordset->GetCollect(_T("BoolValue"));
		BOOL bBoolBeyond;
		bBoolBeyond = (BOOL)vSql_BOOLData;
		if (bBoolBeyond > 0)
		{
			bBoolBeyond = 1;
		}
		else if (bBoolBeyond < 0)
		{
			bBoolBeyond = -1;
		}
		else
		{

		}


		Xml.AddElem(_T("Dev"));
		Xml.AddAttrib(_T("SN"), AtoW(strDeviceSn).c_str());
		Xml.AddChildElem(_T("RealData"), vSql_RealData);
		Xml.AddChildElem(_T("Threshold"), vSql_ThresholdData);

		Xml.AddChildElem(_T("BoolBeyond"), bBoolBeyond);

	}


	strRetXml = WtoA(Xml.GetDoc());

	return true;
}

//Post
bool CHTTPWork::SetDeviceThreshold(std::string   strDeviceSn, std::string  strdSetData, std::string&  strRetXml)
{
	
	CMarkup Xml;
	_variant_t  vSql_ID;

	//�Ȳ����豸�Ƿ������
	m_cstrSql.Format(_T("SELECT device.id FROM device WHERE  device.strSN = '%s'"),AtoW(strDeviceSn.c_str()).c_str());
	m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vSql_ID = m_pRecordset->GetCollect(_T("Id"));
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}

	Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
	Xml.AddElem(_T("Root"));
	Xml.IntoElem();


	if (vSql_ID.vt == VT_EMPTY || vSql_ID.vt == VT_NULL)//�豸��δ���
	{
		//���ɷ��ذ�����
		Xml.AddElem(_T("Dec"), _T("Device  is Not Exist"));
		Xml.AddElem(_T("Ret"), 201);
	}
	else //�豸�Ѿ����
	{
		//����û�
		m_cstrSql.Format(_T("UPDATE device SET device.dThresholdValue = '%f' WHERE   device.strSN = '%s' "),  atof(strdSetData.c_str())  ,  AtoW(strDeviceSn.c_str()).c_str());
		if (!m_DBManger.ExecuteSQL(m_cstrSql.GetBuffer()))
		{//����ִ��ʧ��
			LOGGER_CERROR(theLogger, _T("������ֵʧ��.\r\n"), GetLastError());
			Xml.AddElem(_T("Dec"), _T("Set Threshold  Failled"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
			return false;
		}
		Xml.AddElem(_T("Dec"), _T("Set Threshold   Ok"));
		Xml.AddElem(_T("Ret"), 200);
	}
	strRetXml = WtoA(Xml.GetDoc());

	return true;
}

bool CHTTPWork::AddDevice(std::string strName, std::string  strDeviceSn, std::string&  strRetXml)
{
	CMarkup Xml;
	_variant_t  vSql_ID;

	//�Ȳ����豸�Ƿ������
	m_cstrSql.Format(_T("	SELECT device.id FROM device LEFT JOIN user ON device.nUserId = user.id WHERE user.strUserName = '%s' AND device.strSN = '%s'"), AtoW(strName.c_str()).c_str() ,  AtoW(strDeviceSn.c_str()).c_str() );
	m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vSql_ID = m_pRecordset->GetCollect(_T("Id"));
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}

	Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
	Xml.AddElem(_T("Root"));
	Xml.IntoElem();


	if (vSql_ID.vt == VT_EMPTY || vSql_ID.vt == VT_NULL)//�豸��δ���
	{
		//����û�
		m_cstrSql.Format(_T("INSERT INTO device  (strSN , nUserId)  VALUES ('%s'  , (SELECT user.id FROM user WHERE user.strUserName = '%s'))"),  AtoW(strDeviceSn.c_str()).c_str()  , AtoW(strName.c_str()).c_str());
		if (!m_DBManger.ExecuteSQL(m_cstrSql.GetBuffer()))
		{//����ִ��ʧ��
			LOGGER_CERROR(theLogger, _T("����豸ʧ��.\r\n"), GetLastError());
			Xml.AddElem(_T("Dec"), _T("Add Device Failled"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
			return false;
		}
		//���ɷ��ذ�����
		Xml.AddElem(_T("Dec"), _T("Add Device  Ok"));
		Xml.AddElem(_T("Ret"), 201);
	}
	else //�豸�Ѿ����
	{
		Xml.AddElem(_T("Dec"), _T("Device is Exist"));
		Xml.AddElem(_T("Ret"), 200);
	}
	strRetXml = WtoA(Xml.GetDoc());
	return true;
}

bool CHTTPWork::AddUser(std::string strName, std::string strPwd,std::string& strRetXml)
{
	CMarkup Xml;
	_variant_t  vSql_ID;

	//�Ȳ����û��Ƿ��Ѵ���
	m_cstrSql.Format(_T("SELECT id FROM user WHERE strUserName = '%s'"), AtoW(strName.c_str()).c_str());
	m_pRecordset = m_DBManger.GetRecordSet(m_cstrSql.GetBuffer());

	try
	{
		while (!m_pRecordset->adoEOF)
		{
			vSql_ID = m_pRecordset->GetCollect(_T("Id"));
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}

	Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
	Xml.AddElem(_T("Root"));
	Xml.IntoElem();


	if (vSql_ID.vt == VT_EMPTY || vSql_ID.vt == VT_NULL)//�û�������
	{
		//����û�
		m_cstrSql.Format(_T("INSERT INTO user  (strUserName , strPassWd)  VALUES ('%s'  , '%s')"), AtoW(strName.c_str()).c_str(), AtoW(strPwd.c_str()).c_str());
		if (!m_DBManger.ExecuteSQL(m_cstrSql.GetBuffer()))
		{//����ִ��ʧ��
			LOGGER_CERROR(theLogger, _T("ע���û�ʧ��.\r\n"), GetLastError());
			Xml.AddElem(_T("Dec"), _T("Register Failled"));
			Xml.AddElem(_T("Ret"), 200);
			strRetXml = WtoA(Xml.GetDoc());
			return false;
		}
		//���ɷ��ذ�����
		Xml.AddElem(_T("Dec"), _T("Register Ok"));
		Xml.AddElem(_T("Ret"), 201);
	}
	else //�û��Ѿ�����
	{
		Xml.AddElem(_T("Dec"), _T("UserName is Exist"));
		Xml.AddElem(_T("Ret"), 200);
	}
	strRetXml = WtoA(Xml.GetDoc());

	return true;
}

bool CHTTPWork::AddDevData(std::string strDevSn, std::string strAdcData , double  dRealData, std::string&  strRetXml )
{
	CMarkup Xml;


	Xml.SetDoc(_T("<?xml version='1.0' encoding='UTF-8'?> \n\r"));
	Xml.AddElem(_T("Root"));
	Xml.IntoElem();

	m_cstrSql.Format(_T("INSERT  INTO data(nDevId, nAdcData, dRealData) VALUES((SELECT ID FROM device WHERE strSN = '%s'),  %d,  %f)"), AtoW(strDevSn.c_str()).c_str(), atoi(strAdcData.c_str()) , dRealData);

	if (!m_DBManger.ExecuteSQL(m_cstrSql.GetBuffer()))
	{//����ִ��ʧ��
		LOGGER_CERROR(theLogger, _T("�����豸�������ʧ��.\r\n"), GetLastError());
		Xml.AddElem(_T("Dec"), _T("Insert Test Data Failled"));
		Xml.AddElem(_T("Ret"), 200);
		strRetXml = WtoA(Xml.GetDoc());
		return false;
	}
	//���ɷ��ذ�����
	Xml.AddElem(_T("Dec"), _T("Insert Test Data  Ok"));
	Xml.AddElem(_T("Ret"), 201);


	//if (vSql_ID.vt == VT_EMPTY || vSql_ID.vt == VT_NULL)//�û�������
	//{
	//	//����û�
	//	m_cstrSql.Format(_T("INSERT INTO user  (strUserName , strPassWd)  VALUES ('%s'  , '%s')"), AtoW(strName.c_str()).c_str(), AtoW(strPwd.c_str()).c_str());
	//	if (!m_DBManger.ExecuteSQL(m_cstrSql.GetBuffer()))
	//	{//����ִ��ʧ��
	//		LOGGER_CERROR(theLogger, _T("ע���û�ʧ��.\r\n"), GetLastError());
	//		Xml.AddElem(_T("Dec"), _T("Register Failled"));
	//		Xml.AddElem(_T("Ret"), 200);
	//		strRetXml = WtoA(Xml.GetDoc());
	//		return false;
	//	}
	//	//���ɷ��ذ�����
	//	Xml.AddElem(_T("Dec"), _T("Register Ok"));
	//	Xml.AddElem(_T("Ret"), 201);
	//}
	//else //�û��Ѿ�����
	//{
	//	Xml.AddElem(_T("Dec"), _T("UserName is Exist"));
	//	Xml.AddElem(_T("Ret"), 200);
	//}


	strRetXml = WtoA(Xml.GetDoc());


	return true;
}

#include "stdafx.h"
#include "DB.h"

DBConfig DbTmp;
DBConfig DB::m_dbConfig = DbTmp;

DB::DB()
{
	InitializeCriticalSection(&m_csDB);
}


DB::~DB()
{
	DeleteCriticalSection(&m_csDB);
}

void DB::Init()
{

}

bool DB::Start(void)
{

	return true;
}

bool DB::Stop(void)
{

	return true;
}

void DB::Fini(void)
{

}

bool DB::SetDBConfig(DBConfig strDBInfo)
{
	m_dbConfig = strDBInfo;
	return true;
}


void  DB::OnInitADOConn()
{
	// ��ʼ��OLE/COM�⻷�� 
	::CoInitialize(NULL);
	HRESULT hr;

	try
	{
		// ����Connection����,����ͨ�������ļ���ȡ������Ϣ
		hr = m_pConnection.CreateInstance("ADODB.Connection");
		if (SUCCEEDED(hr))
		{
			m_pConnection->ConnectionTimeout = 600;//�������ӳ�ʱʱ��
			m_pConnection->CommandTimeout = 120;//����ִ�����ʱʱ��

			std::string strCon = "DSN=" + m_dbConfig.strDSN + ";Server=" + m_dbConfig.strServer + ";Database=" + m_dbConfig.strDataBase;
			m_pConnection->Open(strCon.c_str(), m_dbConfig.strUserName.c_str() , m_dbConfig.strPassWd.c_str() , adModeUnknown);
		}
	}
	// ��׽�쳣
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}
}


_RecordsetPtr&  DB::GetRecordSet(_bstr_t bstrSQL)
{
	try
	{
		// �������ݿ⣬���Connection����Ϊ�գ��������������ݿ�
		if (m_pConnection == NULL)
			OnInitADOConn();
		// ������¼������
		m_pRecordset.CreateInstance(__uuidof(Recordset));
		// ȡ�ñ��еļ�¼
		m_pRecordset->Open(bstrSQL, m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);
	}
	// ��׽�쳣
	catch (_com_error e)
	{
		// ��ʾ������Ϣ
		AfxMessageBox(e.Description());
	}
	// ���ؼ�¼��
	return m_pRecordset;
}


BOOL DB::ExecuteSQL(_bstr_t bstrSQL)
{
	// _variant_t RecordsAffected;
	try
	{
		// �Ƿ��Ѿ��������ݿ�
		if (m_pConnection == NULL)
			OnInitADOConn();
		// Connection�����Execute����:(_bstr_t CommandText, 
		// VARIANT * RecordsAffected, long Options ) 
		// ����CommandText�������ִ���ͨ����SQL���
		// ����RecordsAffected�ǲ�����ɺ���Ӱ�������, 
		// ����Options��ʾCommandText�����ͣ�adCmdText-�ı����adCmdTable-����
		// adCmdProc-�洢���̣�adCmdUnknown-δ֪
		m_pConnection->Execute(bstrSQL, NULL, adCmdText);
		return true;
	}
	catch (_com_error e)
	{
		AfxMessageBox(e.Description());
		return false;
	}
}


void DB::ExitConnect()
{
	// �رռ�¼��������
	if (m_pRecordset != NULL)
	{
		m_pRecordset->Close();
		//m_pRecordset->Release();
	}
	m_pConnection->Close();
	//m_pConnection->Release();
	// �ͷŻ���
	::CoUninitialize();
}

#include "stdafx.h"
#include "DB.h"


DB::DB()
{
}


DB::~DB()
{
}


void  DB::OnInitADOConn()
{
	// 初始化OLE/COM库环境 
	::CoInitialize(NULL);
	HRESULT hr;

	try
	{
		// 创建Connection对象,可以通过配置文件获取连接信息
		hr = m_pConnection.CreateInstance("ADODB.Connection");
		if (SUCCEEDED(hr))
		{
			m_pConnection->ConnectionTimeout = 600;//设置连接超时时间
			m_pConnection->CommandTimeout = 120;//设置执行命令超时时间

			m_pConnection->Open("DSN=MySql;Server=localhost;Database=methane-detection-system", "root", "chen77..", adModeUnknown);
		}
	}
	// 捕捉异常
	catch (_com_error e)
	{
		// 显示错误信息
		AfxMessageBox(e.Description());
	}
}


_RecordsetPtr&  DB::GetRecordSet(_bstr_t bstrSQL)
{
	try
	{
		// 连接数据库，如果Connection对象为空，则重新连接数据库
		if (m_pConnection == NULL)
			OnInitADOConn();
		// 创建记录集对象
		m_pRecordset.CreateInstance(__uuidof(Recordset));
		// 取得表中的记录
		m_pRecordset->Open(bstrSQL, m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);
	}
	// 捕捉异常
	catch (_com_error e)
	{
		// 显示错误信息
		AfxMessageBox(e.Description());
	}
	// 返回记录集
	return m_pRecordset;
}


BOOL DB::ExecuteSQL(_bstr_t bstrSQL)
{
	// _variant_t RecordsAffected;
	try
	{
		// 是否已经连接数据库
		if (m_pConnection == NULL)
			OnInitADOConn();
		// Connection对象的Execute方法:(_bstr_t CommandText, 
		// VARIANT * RecordsAffected, long Options ) 
		// 其中CommandText是命令字串，通常是SQL命令。
		// 参数RecordsAffected是操作完成后所影响的行数, 
		// 参数Options表示CommandText的类型：adCmdText-文本命令；adCmdTable-表名
		// adCmdProc-存储过程；adCmdUnknown-未知
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
	// 关闭记录集和连接
	if (m_pRecordset != NULL)
	{
		m_pRecordset->Close();
		//m_pRecordset->Release();
	}
	m_pConnection->Close();
	//m_pConnection->Release();
	// 释放环境
	::CoUninitialize();
}


bool DB::ConnectDB()
{
	CoInitialize(NULL);

	_ConnectionPtr pConn;
	_RecordsetPtr pRs("ADODB.Recordset");

	pConn.CreateInstance(__uuidof(Connection));

	try 
	{
		HRESULT hr = pConn->Open("DSN=MySql;Server=localhost;Database=methane-detection-system", "root", "chen77..", adModeUnknown);
	}
	catch (_com_error e)
	{
		AfxMessageBox(e.Description());
	}




	pRs->Open("Select * From Data", _variant_t(pConn, true), adOpenStatic, adLockOptimistic, adCmdText);

	CString strID = (LPCTSTR)_bstr_t(pRs->GetCollect("FlightNO"));

	return true;
}
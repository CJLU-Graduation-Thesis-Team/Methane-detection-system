#pragma once

#include "HTTPDef.h"

#include "DB.h"

#include "Logger.h"

#include "Markup.h"



class CHTTPWork
{
public:
	CHTTPWork();
	~CHTTPWork();

public:
	//Get
	bool Login(std::string strName , std::string strPwd , std::string&  strRetXml);

	bool GetDeviceList(std::string strName , std::string&  strRetXml);

	bool GetDeviceStatus(std::string  strDeviceSn , std::string&  strRetXml);

	//Post
	bool SetDeviceThreshold(std::string   strDeviceSn, std::string  strdSetData, std::string&  strRetXml);

	bool AddDevice(std::string strName, std::string strDeviceSn, std::string strNickName, std::string&  strRetXml);

	bool AddUser(std::string strName, std::string strPwd, std::string&  strRetXml);

	//Dev
	bool AddDevData(std::string strDevSn ,std::string strAdcData , double  dRealData, std::string&  strRetXml);

private:
	DB m_DBManger;  //数据库操作类
	_RecordsetPtr m_pRecordset;  //返回结果指针
	CString  m_cstrSql;  //查询语句

};


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
	bool Login(std::string strName , std::string strPwd , std::string strRetXml);

	bool GetDeviceList(std::string strName , std::string strRetXml);

	bool GetDeviceStatus(std::string strName,int nDeviceSn , std::string strRetXml);

	//Post
	bool SetDeviceThreshold(std::string strName, int nDeviceSn, double dSetData);

	bool AddDevice(std::string strName, int nDeviceSn );

	bool AddUser(std::string strName, std::string strPwd, std::string&  strRetXml);

};


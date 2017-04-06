#pragma once

#include <string>
#include <stdio.h>  
#include <winsock2.h>  
#include<WS2tcpip.h>

#include "ATW.h"

#pragma comment(lib,"ws2_32.lib")  


#define RET_MAX_SIZE 1024

class CTCP
{
public:
	CTCP();
	~CTCP();

	bool SetServerIp(std::string strIp);
	bool Send(std::string strHttpMsg);


private:
	std::string strServerIp;
	int  nPort;

	//windows
	WSADATA wsa;


};


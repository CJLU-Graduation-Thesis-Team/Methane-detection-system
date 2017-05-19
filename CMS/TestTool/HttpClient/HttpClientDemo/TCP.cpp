#include "stdafx.h"
#include "TCP.h"



CTCP::CTCP()
:strServerIp("120.25.214.105"),
nPort(88)
{
	WSAStartup(MAKEWORD(1, 1), &wsa); //initial Ws2_32.dll by a process  
}


CTCP::~CTCP()
{
	WSACleanup(); //clean up Ws2_32.dll   
}


bool CTCP::SetServerIp(std::string strIp)
{
	strServerIp = strIp;
	return true;
}

bool CTCP::Send(std::string strHttpMsg)
{
	SOCKET soc;
	SOCKADDR_IN serveraddr;
	//SOCKADDR_IN clientaddr;

	if ((soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)   //create a tcp socket  
	{
		AfxMessageBox(_T("���ӷ���ʧ��"),MB_OK);
		printf("Create socket fail!\n");
		return false;
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(nPort);

	inet_pton(AF_INET, strServerIp.c_str(), (void*)&serveraddr.sin_addr.S_un.S_addr);
	//serveraddr.sin_addr.S_un.S_addr = inet_addr( strServerIp.c_str() );


	//���ó�ʱʱ��
	int timeout = 3000; //3s
	int ret = setsockopt(soc, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
	ret = setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	//connect to server  
	printf("Try to connect...\n");
	if (connect(soc, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Connect fail!\n");
		return false;
	}
	printf("Connected\n");

	if(send(soc, strHttpMsg.c_str(), strHttpMsg.length() , 0) < 0)
	{
		AfxMessageBox(_T("����ʧ��\n(3s��û�з���)"), MB_OK);
		return false;
	}

	char szRetBuf[RET_MAX_SIZE];
	memset(szRetBuf , 0 , RET_MAX_SIZE);
	if (recv(soc, szRetBuf, RET_MAX_SIZE, 0) <= 0)
	{
		AfxMessageBox(_T(" ����ƽ̨������Ϣʧ��\n(3s��û���յ�)"), MB_OK);

		closesocket(soc);
		return false;
	}

	closesocket(soc);
	CString cstrRet = AtoW(szRetBuf).c_str();
	AfxMessageBox( cstrRet , MB_OK);

	return true;
}
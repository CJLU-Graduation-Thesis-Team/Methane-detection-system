/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#include "stdafx.h"
#include <string>
#include <time.h>

static char month[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char week[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

const char* g_HTTP_Content_NotFound = "<html><head><title>Guangle's HTTP Server - by Guangle's C++ Studio</title></head><body>404 Not Found</body></html>";
const char* g_HTTP_Bad_Request = "400 Bad Request - Guangle's HTTP Server";
const char* g_HTTP_Bad_Method = "405 Method Not Allowed - Guangle's HTTP Server";
const char* g_HTTP_Server_Error = "500 Opps, server error! - Guangle's HTTP Server";
const char* g_HTTP_Forbidden = "403 Forbidden - Guangle's HTTP Server";

std::string FormatHTTPDate(__int64* ltime)
{
	struct tm t;
	if(ltime != NULL)
	{
		_gmtime64_s(&t, ltime);
	}
	else
	{
		//  �����ָ��,��ȡ��ǰʱ��.
		__int64 ltime_cur;
		_time64( &ltime_cur );
		_gmtime64_s(&t, &ltime_cur);
	}

	char szTime[100] = {0};

	// ��ʽ���ʼ�ʱ�� - Sun, 24 Aug 2008 22:43:45 GMT
	sprintf(szTime, "%s, %d %s %d %d:%d:%d GMT", 
		week[t.tm_wday], t.tm_mday, month[t.tm_mon], 
		t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);

	return szTime;
}

std::string ToHex(const unsigned char* pData, int nSize)
{
	int nStrSize = nSize * 2 + 1;
	char* pStr = new char[nStrSize];
	memset(pStr, 0, nStrSize);

	int nPos = 0;
	for(int i = 0; i < nSize; ++i)
	{
		pStr[nPos] = hex[pData[i] >> 4];
		pStr[nPos + 1] = hex[pData[i] & 0x0F];
		nPos += 2;
	}

	std::string str(pStr);
	delete[] pStr;

	return str;
}
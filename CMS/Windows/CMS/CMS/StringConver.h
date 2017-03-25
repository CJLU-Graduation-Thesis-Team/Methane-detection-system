#pragma once

#include <winnls.h>  
#include <malloc.h> 

class StringConver
{
public:
	StringConver();
	~StringConver();

public:
	//LPCWSTR AnsiToUnicode(LPCSTR Ansi);
	LPCSTR AnsiToUtf8(LPCSTR Ansi);
	LPCSTR WcharToUtf8(LPCWSTR szUnicode);
	LPCSTR Utf8toAnsi(LPCSTR utf8);
	LPCWSTR Utf8toWchar(LPCSTR utf8);
};


namespace StringConVer
{
	inline std::wstring AnsiToUnicode(LPCSTR Ansi)
	{
		int WLength = MultiByteToWideChar(CP_ACP, 0, Ansi, -1, NULL, 0);
		LPWSTR pszW = (LPWSTR)_alloca((WLength + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_UTF8, 0, Ansi, -1, pszW, WLength);
		pszW[WLength] = 0;
		std::wstring wstr = pszW;
		return wstr;
	}

	//inline LPCWSTR AnsiToUnicode(LPCSTR Ansi);
	//LPCSTR AnsiToUtf8(LPCSTR Ansi);
	//LPCSTR WcharToUtf8(LPCWSTR szUnicode);
	//LPCSTR Utf8toAnsi(LPCSTR utf8);
	//LPCWSTR Utf8toWchar(LPCSTR utf8);
};
#include "stdafx.h"
#include "StringConver.h"


StringConver::StringConver()
{
}


StringConver::~StringConver()
{
}

//
//namespace StringConVer
//{
//	inline LPCWSTR AnsiToUnicode(LPCSTR Ansi)
//	{
//		int WLength = MultiByteToWideChar(CP_ACP, 0, Ansi, -1, NULL, 0);
//		LPWSTR pszW = (LPWSTR)_alloca((WLength + 1) * sizeof(WCHAR));
//		MultiByteToWideChar(CP_UTF8, 0, Ansi, -1, pszW, WLength);
//		pszW[WLength] = 0;
//		return pszW;
//	}
//
//};

//inline LPCWSTR StringConVer::AnsiToUnicode(LPCSTR Ansi)
//{
//	int WLength = MultiByteToWideChar(CP_ACP, 0, Ansi, -1, NULL, 0);
//	LPWSTR pszW = (LPWSTR)_alloca((WLength + 1) * sizeof(WCHAR));
//	MultiByteToWideChar(CP_UTF8, 0, Ansi, -1, pszW, WLength);
//	pszW[WLength] = 0;
//	return pszW;
//}



LPCSTR StringConver::AnsiToUtf8(LPCSTR Ansi)
{
	int WLength = MultiByteToWideChar(CP_ACP, 0, Ansi, -1, NULL, 0);
	LPWSTR pszW = (LPWSTR)_alloca((WLength + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, Ansi, -1, pszW, WLength);

	int ALength = WideCharToMultiByte(CP_UTF8, 0, pszW, -1, NULL, 0, NULL, NULL);
	LPSTR pszA = (LPSTR)_alloca(ALength + 1);
	WideCharToMultiByte(CP_UTF8, 0, pszW, -1, pszA, ALength, NULL, NULL);
	pszA[ALength] = 0;

	return pszA;
}

LPCSTR StringConver::WcharToUtf8(LPCWSTR szUnicode)
{
	int ALength = WideCharToMultiByte(CP_UTF8, 0, szUnicode, -1, NULL, 0, NULL, NULL);
	LPSTR pszA = (LPSTR)_alloca(ALength + 1);
	WideCharToMultiByte(CP_UTF8, 0, szUnicode, -1, pszA, ALength, NULL, NULL);
	pszA[ALength] = 0;

	return pszA;
}

LPCSTR StringConver::Utf8toAnsi(LPCSTR utf8)
{
	int WLength = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, NULL);
	LPWSTR pszW = (LPWSTR)_alloca((WLength + 1) *sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, pszW, WLength);
	pszW[WLength] = 0;

	int ALength = WideCharToMultiByte(CP_ACP, 0, pszW, -1, NULL, 0, NULL, NULL);
	LPSTR pszA = (LPSTR)_alloca(ALength + 1);
	WideCharToMultiByte(CP_ACP, 0, pszW, -1, pszA, ALength, NULL, NULL);
	pszA[ALength] = 0;

	return pszA;
}

LPCWSTR StringConver::Utf8toWchar(LPCSTR utf8)
{
	int WLength = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, NULL);
	LPWSTR pszW = (LPWSTR)_alloca((WLength + 1) *sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, pszW, WLength);
	pszW[WLength] = 0;
	return pszW;
}
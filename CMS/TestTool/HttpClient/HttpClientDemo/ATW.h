/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

// ATW.h: interface for the CBase64 class.
// by Chen.Guangle - Guangle's C++ Studio
// 2017-03-08
// ת���ַ�����

/*

1. ʵ��USC��ѹ��,Unicode <-> UTF-8.
2. ʵ�ֶ��ֽ��� <-> Unicode

����Unicode�ַ���,�� wstring �洢.
���ڷ�Unicode�ַ����� string �洢. ANSI, GB2312 ��һ��
����UTF-8��,Ҳ�� string �洢, UTF-8�ı��봮�в�����null����, ����û����.
*/

/*
 ��Ҫʹ��ATL �е� USES_CONVERSION; A2W, A2T, W2A �ȵĺ�, ������Щ�궼���� alloca() �����ں���ջ�з����ڴ�.
 ��Ȼ�ǳ�����,�������غ��Զ�����, �����������Σ��, ����ջֻ�� 1M �Ĵ�С.

 ���µĺ���ʹ�õĿռ䶼���ڶ��з����,�Ƚϰ�ȫ.
*/

#pragma once
#include <string>

#ifdef _UNICODE
#define TtoA WtoA
#define AtoT AtoW
//#else
//#define TtoA(a) a
//#define AtoT(w) w
#endif

std::string WtoA(const wchar_t* pwszSrc);
std::string WtoA(const std::wstring &strSrc);

std::wstring AtoW(const char* pszSrc);
std::wstring AtoW(const std::string &strSrc);

std::string WtoUTF8(const wchar_t* pwszSrc);
std::string WtoUTF8(const std::wstring &strSrc);

std::wstring UTF8toW(const char* pszSrc);
std::wstring UTF8toW(const std::string &strSr);

// ���һ���� null ��β���ַ����Ƿ���UTF-8, �������0, Ҳֻ��ʾ������պ÷���UTF8�ı������.
// ����ֵ˵��: 
// 1 -> �����ַ�������UTF-8�������
// -1 -> ��⵽�Ƿ���UTF-8�������ֽ�
// -2 -> ��⵽�Ƿ���UTF-8�ֽڱ���ĺ����ֽ�.
int IsTextUTF8(const char* pszSrc); 
/* Copyright (C) 2011  
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 *    
*/

#ifndef _H_FILEHEADER_SLOGGER_
#define _H_FILEHEADER_SLOGGER_

/* Simple logger - slogger

����־ϵͳ

��С��ģ�Ĵ���ʵ�ִ����������ʵ�C++��־ϵͳ:
1. �ּ���.
2. ���Զ�λ�������ͬ���ն�/��,���ļ�,Windows����,������,����̨��.
3. �̰߳�ȫ��.
4. �ܷ���Ŀ������߹ر�ĳ���������־��Ϣ(�ú궨��ʵ��)


*/

/* 

���

1. �ο��� log4cplus, �и����Ӵ�Ĵ����ģ(����һ����־ϵͳ��˵), ������дһ�� slogger ���뷨.
   ��ʵ��,��ʱ��(����Ϊ�Ǵ���������)Ҫ��ܼ�:�����������־�Ա���Ի����˽���������״��,�������ʽ,��
   ����,LOGGER�������Ȳ�ûҪ��, ��� log4cplus ��Ȼ���ڱ���.

2. �����м���"�첽��־"���뷨,���ǵ���־�����þ���Ҫ׼ȷ,��ʱ�ķ�ӳ�������е�״̬,��"�첽"��˱�������,�����.


*/

/*

��Ȩ˵��

��������, �����޸�,ʹ��.

*/

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <locale>

//
// �궨��
// ��������Ԥ����Ŀ��Ժܷ������ӻ���ɾ����־���,���Ч��
// 

// log level define
#define LOGGER_LEVEL slogger::LogLevel
#define LL_NONE slogger::ll_none
#define LL_FATAL slogger::ll_fatal
#define LL_ERROR slogger::ll_error
#define LL_WARNING slogger::ll_warning
#define LL_INFO slogger::ll_info
#define LL_DEBUG slogger::ll_debug
#define LL_TRACE slogger::ll_trace
#define LL_ALL slogger::ll_all

// logger disable all
#ifndef LOGGER_DISABLE
#define LOGGER_DECLARE(logger) slogger::Logger logger
#define LOGGER_USING(logger) extern slogger::Logger logger
#define LOGGER_CLOG(logger, ll, fmt, ...) (logger).log((ll), (fmt), __VA_ARGS__)
#else
#define LOGGER_DEFINE(logger) 
#define LOGGER_USING(logger) 
#define LOGGER_CLOG(logger, ll, fmt, args) 
#endif

// logger disable ll_fatal
#if !defined(LOGGER_DISABLE_FATAL) && !defined(LOGGER_DISABLE)
#define LOGGER_FATAL(logger, msg) (logger)<<slogger::ll_fatal<<##msg##<<(logger)
#define LOGGER_CFATAL(logger, fmt, ...) LOGGER_CLOG((logger), (LL_FATAL), (fmt), __VA_ARGS__)
#else
#define LOGGER_FATAL(logger, msg)
#define LOGGER_CFATAL(logger, fmt, ...)
#endif

// logger disable ll_error
#if !defined(LOGGER_DISABLE_ERROR) && !defined(LOGGER_DISABLE)
#define LOGGER_ERROR(logger, msg) (logger)<<slogger::ll_error<<##msg##<<(logger)
#define LOGGER_CERROR(logger, fmt, ...) LOGGER_CLOG((logger), (LL_ERROR), (fmt), __VA_ARGS__)
#else
#define LOGGER_ERROR(logger, msg)
#define LOGGER_CERROR(logger, fmt, ...)
#endif

// logger disable ll_info
#if !defined(LOGGER_DISABLE_INFO) && !defined(LOGGER_DISABLE)
#define LOGGER_INFO(logger, msg) (logger)<<slogger::ll_info<<##msg##<<(logger)
#define LOGGER_CINFO(logger, fmt, ...) LOGGER_CLOG((logger), (LL_INFO), (fmt), __VA_ARGS__)
#else
#define LOGGER_INFO(logger, msg)
#define LOGGER_CINFO(logger, fmt, ...)
#endif

// logger disable ll_warning
#if !defined(LOGGER_DISABLE_WARNING) && !defined(LOGGER_DISABLE)
#define LOGGER_WARNING(logger, msg) (logger)<<slogger::ll_warning<<##msg##<<(logger)
#define LOGGER_CWARNING(logger, fmt, ...) LOGGER_CLOG((logger), (LL_WARNING), (fmt), __VA_ARGS__)
#else
#define LOGGER_WARNING(logger, msg)
#define LOGGER_CWARNING(logger, fmt, ...)
#endif

// logger disable ll_debug
#if !defined(LOGGER_DISABLE_DEBUG) && !defined(LOGGER_DISABLE)
#define LOGGER_DEBUG(logger, msg) (logger)<<slogger::ll_debug<<##msg##<<(logger)
#define LOGGER_CDEBUG(logger, fmt, ...) LOGGER_CLOG((logger), (LL_DEBUG), (fmt), __VA_ARGS__)
#else
#define LOGGER_DEBUG(logger, msg)
#define LOGGER_CDEBUG(logger, fmt, ...)
#endif

// logger disable ll_trace
#if !defined(LOGGER_DISABLE_TRACE) && !defined(LOGGER_DISABLE)
#define LOGGER_TRACE(logger, msg) (logger)<<slogger::ll_trace<<##msg##<<(logger)
#define LOGGER_CTRACE(logger, fmt, ...) LOGGER_CLOG((logger), (LL_TRACE), (fmt), __VA_ARGS__)
#else
#define LOGGER_TRACE(logger, msg)
#define LOGGER_CTRACE(logger, fmt, ...)
#endif

//
// simple logger for C++ code
//

namespace slogger /* simple logger*/
{

#if defined(_UNICODE) || defined(UNICODE)
	typedef std::wstring tstring;
	typedef std::wostringstream tstringstream;
	typedef std::wostream tostream;
	typedef wchar_t tchar;
	typedef std::wofstream tofstream;
	#define tcout std::wcout
#else
	typedef std::string tstring;
	typedef std::ostringstream tstringstream;
	typedef std::ostream tostream;
	typedef char tchar;
	typedef std::ofstream tofstream;
	#define tcout std::cout
#endif

	// log level
	enum LogLevel
	{
		ll_none = 0,
		ll_fatal,
		ll_error,
		ll_warning,
		ll_info,
		ll_debug,
		ll_trace,

		ll_all
	};

	// ����Ļ���
	class Appender
	{
	public:
		Appender() {};
		virtual ~Appender() {};
		virtual int append(const tstring &logMsg) = 0;
		virtual bool open() = 0;
		virtual void close() = 0;
	};
	typedef std::vector<Appender*> AppenderArr;

	// Loggerʵ��
	class Logger
	{
	private:
		tstring _timeFmt; // time format.
		LogLevel _ll; // log level
		LogLevel _llTmp; // for cpp style output
		tstringstream _buf; // for cpp style output
		AppenderArr _appenders; // appender list
		bool _mt; // multiple thread mode
		bool _isOpen; // is open
#ifdef _WIN32
		CRITICAL_SECTION _locker; //multi-thread lock
#else
		pthread_mutex_t _locker;
#endif

	private:
		Logger(const Logger &other); // ���� appender ָ���б�, ��ֹ����
		Logger& operator = (const Logger &other);

		int _doLog(const tchar *msg); // format time string and call appenders to write log.
		int _flush();	// flush() internal buffer to appenders, for cpp style output.

		void _lock(); // multi thread lock
		void _unlock();

	public:
		Logger();
		virtual ~Logger(void);

		// ��ʼ��������
		bool open(bool mt = true, LogLevel ll = ll_all, const tstring &timeFmt = _T("%m/%d/%Y-%H:%M:%S"));
		bool close();
		inline bool isOpen() { return _isOpen; }

		// ��־����
		LogLevel getLogLevel() { return _ll; }
		LogLevel setLogLevel(LogLevel ll);

		// ������
		int addFileAppender(const tstring &fileName, unsigned long maxBytes = 5 * 1024 * 1024, bool multiFile = true); // Ԥ����� �ļ������
		int addConsoleAppender(tostream &os = tcout); // Ԥ����ı�׼�������, ������ cout/wcout, cerr/wcerr, clog/wclog
#ifdef _WIN32
		int addHwndAppender(HWND hWnd, unsigned int msgId); // Ԥ�����Windows���������,��������Ϣ��Ӧ������ɾ����־�ַ���.
#endif
		int addDebugAppender(); // Ԥ����ĵ����������.
		bool removeAppender(int appenderHandle);

		// �����־
		// cpp style ���ø�ʽ: theLogger << ll_info << 1234 << _T("Hello World!) << 0.12 << theLogger;
		// ���������� << theLogger �Ļ�,�ڶ��̻߳����»���������. ��ÿ����ñ���������, �������᲻�÷���, C++�������̫����.
		// �������ú�,�������ǵ��� << theLogger
		tstringstream& operator << (const LogLevel ll); // ��ʼ���� C++������־. set tmp log level
		friend Logger& operator << (tostream &ss, Logger &logger); // // ��������,����ջ����еĴ��������־, ����������������ɶԵ���

		// c style ���ø�ʽ,�� sprintf һ�� log(ll_info, _T("ip address:%s, port:%d\n"), ipAddress, port);
		int log(LogLevel ll, const tchar *fmt, ...);
	};
}

#endif
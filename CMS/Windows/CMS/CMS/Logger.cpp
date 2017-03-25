/* Copyright (C) 2011  
 *
 *
 *    
*/

#include "StdAfx.h"
#include "Logger.h"
#include <fstream>
#include <assert.h>
#include "stdarg.h"

using namespace slogger;
#define TIME_BUF_SZIE 128

//////////////////////////////////////////////////

// ���������
class Win32DebugAppender : public Appender
{
public:
	int append(const tstring &logMsg);
	bool open() { return true; }
	void close() {}
};

// ����̨���
class ConsoleAppender : public Appender
{
public:
	tostream& _os;

	ConsoleAppender(tostream &os);
	int append(const tstring &logMsg);
	bool open() { return true; }
	void close() {}
};

// Windowsϵͳ - �������
#ifdef _WIN32
class HwndAppender : public Appender
{
public:
	HWND _hwnd;
	unsigned int _msgId;

	HwndAppender(HWND hWnd, unsigned int msgId);
	int append(const tstring &logMsg);
	bool open() { return _hwnd != NULL; }
	void close() {}
};


HwndAppender::HwndAppender(HWND hWnd, unsigned int msgId)
	: _hwnd(hWnd), _msgId(msgId)
{
}

/*
����:
�����A���ڵĴ��ں����е�������־�������,�ͻ�����.
����,��A���ڵ�OnButton1Click()�е�������־�������,��ô����־��������л�����������SendMessage()����,��SendMessage()��Ҫ�ȴ�
���ڹ��̴������,����ʱA���ڵĴ���������������OnButton1Click()��,���Ի�����.
������Ϊ��־��,Ӧ�ÿ����������������,��ν����?

1. ʹ��PostMessage���Խ��,�����ַ������ڴ���δ���?
2. ���⽨��һ���߳�,ʹ����Ϣ����.

����ĸ�Դ:
SendMessage ��һ���Ƚ��ر�ĺ���.
ǰ��:�߳�A�д����˴���W
������߳�A����W��SendMessage����������һ����Ϣ,��ֱ�ӵ�����Ϣ������,�������߳�A����Ϣ����,������ν��"�����"�����.
����,��ͬ�߳��еĴ��ں����е���SendMessage���ᵼ������.
������߳�B����W��SendMessage������Ϣ,������Ϣ�����߳�A����Ϣ����,���ȴ������Ϣ��������ɺ�ŷ���.��ν��"���"���.
*/
int HwndAppender::append(const tstring &logMsg)
{
	int len = logMsg.size();
	tchar *msg_ptr = new tchar[len + 1];
	assert(msg_ptr);
	_tcscpy(msg_ptr, logMsg.c_str());

	//SendMessage(_hwnd, _msgId, len, reinterpret_cast<LPARAM>(msg_ptr));
	//delete[] msg_ptr;

	PostMessage(_hwnd, _msgId, len, reinterpret_cast<LPARAM>(msg_ptr));
	return 0;
}

#endif

// �ļ����
class FileAppender : public Appender
{
public:
	std::locale _loc;
	tstring _fileName;
	tofstream _fs;
	unsigned long _maxSize;
	bool _multiFiles;
	unsigned long _curSize;
	int _fileIndex;

	FileAppender(const tstring &fileName, unsigned long maxSize, bool multiFile);
	~FileAppender();
	int append(const tstring &logMsg);
	bool open();
	void close();
};

int Win32DebugAppender::append(const tstring &logMsg)
{
	OutputDebugString(logMsg.c_str());
	return 0;
}

ConsoleAppender::ConsoleAppender(tostream &os)
: _os(os)
{
}

int ConsoleAppender::append(const tstring &logMsg)
{
	_os << logMsg << std::flush;
	return 0;
}

FileAppender::FileAppender(const tstring &fileName, unsigned long maxSize, bool multiFiles)
: _fileName(fileName), _loc(""), _maxSize(maxSize), _multiFiles(multiFiles), _curSize(0), _fileIndex(0)
{
	//if(_fs.is_open())
	//{
	//	_fs.imbue(_loc);
	//	_fs.seekp(0, std::ios_base::end);

	//	//std::ios::pos_type pos = _fs.tellp();
	//	//TRACE("log file postion:%d\r\n", static_cast<size_t>(pos));
	//}
}

bool FileAppender::open()
{
	// wofstream ���ʱ,����wchar_t�ַ���, ���ʱ���ݻ����ѿ��ַ���ת������д�ļ�. 
	// ���û������, ��ô��C�ֳ�, �޷��������.
	// ���Ա��� imbue ��ǰϵͳ�Ļ���.

	_fs.open(_fileName.c_str(), std::ios::app | std::ios::out);
	if(_fs.is_open())
	{
		_fs.imbue(_loc);
		_fs.seekp(0, std::ios_base::end);
		_curSize = static_cast<unsigned long>(_fs.tellp());
		return true;
	}
	else
	{
		return false;
	}
}

void FileAppender::close()
{
	if(_fs.is_open()) _fs.close();
}

FileAppender::~FileAppender()
{
	close();
}

int FileAppender::append(const tstring &logMsg)
{
	if(_fs.is_open())
	{
		// need to create a new file?
		if( _multiFiles && _curSize > _maxSize )
		{
			// close current file
			_fs.close();
			ASSERT(!_fs.is_open());

			// make a new log file name: oldfile{time}_index.xx, eg. "mylog_20101229_235959_1.txt"
			__time64_t t;
			_time64( &t );
			struct tm *tt = _localtime64(&t);
			tchar tmString[TIME_BUF_SZIE + 1] = {0};
			_stprintf(tmString, _T("_%04d%02d%02d_%02d%02d%02d_%d"), tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec, ++_fileIndex);
			//tstringstream tos;
			//tos << _T("_") << tt->tm_year + 1900 << tt->tm_mon << tt->tm_mday << _T("_") << tt->tm_hour << tt->tm_min << tt->tm_sec;

			tstring newFileName(_fileName);
			tstring::size_type indexDot = _fileName.find_last_of(_T('.'));
			if(tstring::npos == indexDot)
			{
				//newFileName += tos.str();
				newFileName += tmString;
			}
			else
			{
				// newFileName.insert(indexDot, tos.str());
				newFileName.insert(indexDot, tmString);
			}

			// open new log file
			_fs.open(newFileName.c_str(), std::ios::out);
			if(_fs.is_open())
			{
				_fs.imbue(_loc);
				_curSize = 0;
			}
			else
			{
				ASSERT(0);
			}
		}

		// write log message to file
		if(_fs.is_open()) 
		{
			_fs << logMsg << std::flush;
			_curSize = static_cast<unsigned long>(_fs.tellp());
		}
	}
	return 0;
}

Logger::Logger()
: _ll(ll_none), _timeFmt(_T("")), _mt(false), _isOpen(false)
{
}

Logger::~Logger(void)
{
	close();
}

tstringstream& Logger::operator << (LogLevel ll)
{
	_lock();

	_llTmp = ll;
	return _buf;
}

Logger& slogger::operator << (tostream &ss, Logger &logger)
{
	logger._flush();

	logger._unlock();
	return logger;
}

int Logger::addDebugAppender()
{
	Win32DebugAppender *appender = new Win32DebugAppender();
	if(!appender->open())
	{
		delete appender;
		return 0;
	}
	else
	{
		_lock();
		_appenders.push_back(appender);
		_unlock();

		return reinterpret_cast<int>(appender);
	}
}

int Logger::addFileAppender(const tstring &fileName, unsigned long maxBytes, bool multiFile)
{
	FileAppender *appender = new FileAppender(fileName, maxBytes, multiFile);
	if(appender->open())
	{
		_lock();
		_appenders.push_back(appender);
		_unlock();

		return reinterpret_cast<int>(appender);
	}
	else
	{
		delete appender;
		return 0;
	}
}

int Logger::addHwndAppender(HWND hWnd, unsigned int msgId)
{
	HwndAppender *appender = new HwndAppender(hWnd, msgId);
	if(!appender->open())
	{
		delete appender;
		return 0;
	}
	else
	{

		_lock();
		_appenders.push_back(appender);
		_unlock();

		return reinterpret_cast<int>(appender);
	}
}

int Logger::addConsoleAppender(tostream &os)
{
	ConsoleAppender *appender = new ConsoleAppender(os);
	if(!appender->open())
	{
		delete appender;
		return 0;
	}
	else
	{

		_lock();
		_appenders.push_back(appender);
		_unlock();

		return reinterpret_cast<int>(appender);
	}
}

bool Logger::removeAppender(int appender)
{
	bool ret = false;
	_lock();
	for(AppenderArr::iterator iter = _appenders.begin(); iter != _appenders.end(); ++iter)
	{
		if( appender == reinterpret_cast<int>(*iter) )
		{
			(*iter)->close();
			delete (*iter);
			_appenders.erase(iter);
			ret = true;
			break;
		}
	}
	_unlock();
	return ret;
}

LogLevel Logger::setLogLevel(LogLevel ll)
{
	LogLevel tmp = _ll;
	_lock();
	_ll = ll;
	_unlock();
	return tmp;
}

void Logger::_lock()
{
	if(_mt) 
	{
#ifdef _WIN32
		EnterCriticalSection(&_locker);
#else
		pthread_mutex_lock(*_locker);
#endif
	}
}

void Logger::_unlock()
{
	if(_mt) 
	{
#ifdef _WIN32
		LeaveCriticalSection(&_locker);
#else
		pthread_mutex_unlock(&_locker);
#endif
	}
}

int Logger::_flush()
{
	int ret = 0;
	if(_llTmp <= _ll)
	{
		ret = _doLog(_buf.str().c_str());
	}
	_llTmp = _ll;
	_buf.str(_T(""));
	return ret;
}

int Logger::_doLog(const tchar *msg)
{
	// get time string
	tchar tmString[TIME_BUF_SZIE + 1] = {0};
	__time64_t t;
	_time64( &t );
	_tcsftime(tmString, TIME_BUF_SZIE, _timeFmt.c_str(), _localtime64( &t));

	// format time and log message
	// ���Ҫ������Ӹ��ӵĸ�ʽ, ����Ϊÿ�� appender ����һ��  layout .
	tstringstream o;
	o << tmString << _T(" - ") << msg;

	// call all appenders to write log
	for(AppenderArr::iterator iter = _appenders.begin(); iter != _appenders.end(); ++iter)
	{
		(*iter)->append(o.str());
	}
	return 0;
}

bool Logger::open(bool mt, LogLevel ll /*= ll_all*/, const tstring &timeFmt /*= _T("%m/%d/%Y-%H:%M:%S")*/)
{
	if(isOpen()) return false;

	_mt = mt;
	_ll = ll;
	_timeFmt = timeFmt;

	// initialize sync object
	if(_mt) 
	{
#ifdef _WIN32
		InitializeCriticalSection(&_locker);
#else
		pthread_mutex_init(&_locker, NULL);
#endif
	}

	_isOpen = true;
	return true;
}

bool Logger::close()
{
	if(!isOpen()) return false;

	_lock();
	for(AppenderArr::iterator iter = _appenders.begin(); iter != _appenders.end(); ++iter)
	{
		(*iter)->close();
		delete (*iter);
	}
	_appenders.clear();
	_unlock();

	if(_mt)
	{
#ifdef _WIN32
		DeleteCriticalSection(&_locker);
#else
		pthread_mutex_destroy(&_locker);
#endif
	}

	_mt = false;
	_isOpen = false;
	return true;
}

int Logger::log(LogLevel ll, const tchar *fmt, ...)
{

	if( ll <= _ll)
	{   
		// calculate required buffer size
		va_list args;
		va_start(args, fmt);
		int count = _vsctprintf(fmt, args);
		va_end(args);

		// alloc memory
		tchar* logBuf = (tchar*) malloc( (count + 1) * sizeof(tchar) );
		if(logBuf == NULL) return 0;

		// output to buffer
		va_start(args, fmt);
		int realCount = _vstprintf(logBuf, fmt, args); 
		ASSERT(count == realCount);
		va_end(args);
		logBuf[realCount] = 0;

		// write to log
		_lock();
		_doLog(logBuf);
		_unlock();

		// free memory
		free(logBuf);
		return realCount;
	}
	else
	{
		return 0;
	}
}
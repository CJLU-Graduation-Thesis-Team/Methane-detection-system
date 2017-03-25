#pragma once

#ifndef _MEMFILE_HEADER_PROTECT_
#define _MEMFILE_HEADER_PROTECT_

/*
memfile.h

�ڴ滺��,���Զ�д.

by ������ - Que's C++ Studio
2010-12-16

*/

class memfile
{
private:
	memfile(const memfile &other);
	memfile& operator = (const memfile &other);

public:
	memfile(int memInc = 1024, int maxSize = INT_MAX);
	~memfile();

	int write(const void *buf, int len); // ����д����ֽ���
	int puts(const char* buf); // ����д����ֽ���,����������0
	int putc(char ch);
	int seekp(int offset, int origin); // ����0 ��ʾ�ɹ�.
	int tellp() const;
	int settle();

	int read(void *buf, int size); // ���ض�ȡ�ֽ���
	char getc();
	int getline(char *buf, int size); // ���ض�ȡ���ֽ���,�������з�
	int seekg(int offset, int origin);
	int tellg() const;

	const void* data() const;
	int size() const;
	int reset();
	bool eof(bool readEof = true) const;

private:
	char *_data;
	int _dataSize;
	int _readPos;
	int _writePos;
	int _fileSize;
	int _maxSize;
	int _memInc;

	int _seek(int &pos, int offset, int origin);
};

#endif
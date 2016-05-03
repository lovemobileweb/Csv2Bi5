#include "stdafx.h"
#include "Log.h"

#include <iostream>
#include <fstream>

using namespace std;

Log::Log()
{
}


Log::~Log()
{
}

void Log::Delete()
{
	remove("error.txt");
}

void Log::WriteLine(char const * const _Format, ...)
{
	char buf[1024];
	va_list ap;

	ofstream log("error.txt", ios::out | ios::app | ios::binary);
	_strtime_s(buf);
	log.write(buf, strlen(buf));
	log.write("\t", 1);
	va_start(ap, _Format);
	vsprintf_s(buf, _Format, ap);
	va_end(ap);
	log.write(buf, strlen(buf));
	log.write("\r\n", 2);
	log.close();
}

#pragma once



class Log
{
public:
	Log();
	~Log();

public:
	static void WriteLine(char const* const _Format, ...);
	static void Delete();
};


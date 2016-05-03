#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

class Config
{
private:
	string m_sourcePath;
	string m_destPath;

public:
	Config();
	~Config();

	void LoadConfig();
	void SaveConfig();

	const char * GetSourcePath();
	void SetSourcePath(const char *path);
	const char * GetDestPath();
	void SetDestPath(const char *path);
};

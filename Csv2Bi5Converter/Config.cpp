#include "stdafx.h"
#include "Config.h"

Config::Config()
{
	LoadConfig();
}


Config::~Config()
{
}

string trim(string& str, char ch)
{
	size_t first = str.find_first_not_of(ch);
	size_t last = str.find_last_not_of(ch);
	return str.substr(first, (last - first + 1));
}

void Config::LoadConfig()
{
	ifstream config("config.txt");
	if (!config.is_open())
	{
		m_sourcePath = "C:\\CSV_data";
		m_destPath = "C:\\BI5_data";
	}
	else
	{
		string line;
		while (getline(config, line))
		{
			string key, value;
			stringstream sline(line);
			getline(sline, key, '=');
			trim(key, ' ');
			trim(key, '\t');
			trim(key, '\r');
			trim(key, '\n');
			getline(sline, value);
			trim(value, ' ');
			trim(value, '\t');
			trim(value, '\r');
			trim(value, '\n');
			if (key == "source")
			{
				m_sourcePath = value;
			}
			else if(key == "dest")
			{
				m_destPath = value;
			}
		}
		config.close();
	}
}

void Config::SaveConfig()
{
	ofstream config("config.txt", ios::trunc);
	if (!config.is_open())
	{
		return;
	}
	char buf[1024];
	sprintf_s(buf, "source=%s\ndest=%s", m_sourcePath.c_str(), m_destPath.c_str());
	config.write(buf, strlen(buf));
	config.close();
}

const char * Config::GetSourcePath()
{
	return m_sourcePath.c_str();
}

void Config::SetSourcePath(const char * path)
{
	m_sourcePath = path;
}

const char * Config::GetDestPath()
{
	return m_destPath.c_str();
}

void Config::SetDestPath(const char * path)
{
	m_destPath = path;
}

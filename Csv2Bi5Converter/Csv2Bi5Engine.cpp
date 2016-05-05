#include "stdafx.h"
#include "Csv2Bi5Engine.h"
#include "Log.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <experimental/filesystem>
#include "LzmaCompress.h"

namespace fs = std::experimental::filesystem;
using namespace std;

template<typename T>
void ByteSwap(T* p)
{
	for (int i = 0; i < sizeof(T) / 2; ++i)
		std::swap(((char *)p)[i], ((char *)p)[sizeof(T) - 1 - i]);
}

Csv2Bi5Engine::Csv2Bi5Engine()
{
	SetStatus(Status::None);
	InitProcessed();
}


Csv2Bi5Engine::~Csv2Bi5Engine()
{
}

float Csv2Bi5Engine::GetProgress()
{
	return m_workLoad.QuadPart == 0 ? 0 : (float)(m_curLoad.QuadPart * 100.0 / m_workLoad.QuadPart);
}

void Csv2Bi5Engine::StartConvert(const char *sourcePath, const char *destPath)
{
	//Log::WriteLine("Start Convert");
	if (m_status == Status::None || m_status == Status::Stopped)
	{
		strcpy_s(m_sourcePath, sourcePath);
		strcpy_s(m_destPath, destPath);
		_beginthread(Csv2Bi5Engine::Run, 0, this);
	}
	else
	{
		Log::WriteLine("Failed to start! (%d)", m_status);
		return;
	}
}

void Csv2Bi5Engine::StopConvert()
{
	if (m_status == Status::Starting || m_status == Status::Started)
		SetStatus(Status::Stopping);
}

Csv2Bi5Engine::Status Csv2Bi5Engine::GetStatus()
{
	return m_status;
}

void Csv2Bi5Engine::SetStatus(Status status)
{
	m_status = status;
}

const char * Csv2Bi5Engine::GetSourcePath()
{
	return m_sourcePath;
}

const char * Csv2Bi5Engine::GetDestPath()
{
	return m_destPath;
}

void Csv2Bi5Engine::Run(void * arg)
{
	Csv2Bi5Engine *pObj = (Csv2Bi5Engine *)arg;
	pObj->Process();
}

int StringToInt(string str)
{
	/*int dotIdx = str.find_first_of('.');
	string ret = str.substr(0, dotIdx);
	int iRet = 0;
	int k = str.length();
	if (k > dotIdx + 5)
	{
	ret += str.substr(dotIdx + 1, 5);
	iRet = stoi(ret);
	}
	else
	{
	ret += str.substr(dotIdx + 1);
	iRet = stoi(ret);
	iRet *= pow(10, 6 - (k - dotIdx));
	}
	return iRet;*/
	int ret = 0;
	int dotIdx = str.find_first_of('.');
	//int nIdx = dotIdx + 5;
	int nIdx;
	bool negative = (str[0] == '-');
	int i = 0;
	if (negative == true)
		i = 1;
	nIdx = i;
	int k = str.length();
	for (; i < 6; i++, nIdx++)
	{
		if (nIdx < k)
		{
			if (i == dotIdx)
				nIdx++;
			ret = ret * 10 + (int)(str[nIdx] - '0');
		}
		else
		{
			ret *= 10;
		}
	}
	return negative ? -ret : ret;
}

void Csv2Bi5Engine::Process()
{
	char buf[CSV2BI5_MAX_PATH];
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	m_workLoad.QuadPart = 0;
	InitProcessed();
	SetStatus(Status::Started);
	Log::Delete();
	//Log::WriteLine("Started Converting");
	
	//Log::WriteLine("Init csv file list");
	m_csvFiles.clear();
	m_bi5Files.clear();
	strcpy_s(buf, GetSourcePath());
	strcat_s(buf, "\\*.csv");

	hFind = FindFirstFileA(buf, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		Log::WriteLine("Source Path is invalid or access denied!");
		SetStatus(Status::Stopped);
		return;
	}
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		}
		else
		{
			CsvFile csvFile;
			strcpy_s(csvFile.path, ffd.cFileName);
			LARGE_INTEGER filesize;
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			m_workLoad.QuadPart += filesize.QuadPart;
			csvFile.line = 0;
			m_csvFiles.push_back(csvFile);
		}
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);

	ofstream bi5tmp;
	char curBi5Path[CSV2BI5_MAX_PATH] = "";

	for (list<CsvFile>::iterator it = m_csvFiles.begin(); GetStatus() == Status::Started && it != m_csvFiles.end(); ++it)
	{
		IncreaseProcessed();
		CsvFile &csvFile = *it;
		strcpy_s(buf, GetSourcePath());
		strcat_s(buf, "\\");
		strcat_s(buf, csvFile.path);
		//Log::WriteLine("Converting %s ...", csvFile.path);

		// get product name
		string productName;
		stringstream sfilename(csvFile.path);
		if (!getline(sfilename, productName, '_'))
		{
			Log::WriteLine("Failed to get productName string of %s", csvFile.path);
			SetStatus(Status::Stopped);
			continue;
		}

		ifstream csv(buf);
		if (!csv.is_open())
		{
			Log::WriteLine("Failed to open csv file %s", csvFile.path);
			continue;
		}
		string line;
		string year, month, day, hour, minute, second, millisecond, ask, bid, askVolume, bidVolume;
		streampos curPos = 0;
		while (GetStatus() == Status::Started && getline(csv, line))
		{
			streampos newPos = csv.tellg();
			m_curLoad.QuadPart += newPos - curPos;
			curPos = newPos;
			csvFile.line++;
			stringstream sline(line);

			/* 
			* parse csv line
			*/

			// get year string
			if (!getline(sline, year, '.'))
			{
				Log::WriteLine("Failed to get year string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get month string
			if (!getline(sline, month, '.'))
			{
				Log::WriteLine("Failed to get month string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get day string
			if (!getline(sline, day, ' '))
			{
				Log::WriteLine("Failed to get day string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get hour string
			if (!getline(sline, hour, ':'))
			{
				Log::WriteLine("Failed to get hour string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get minute string
			if (!getline(sline, minute, ':'))
			{
				Log::WriteLine("Failed to get minute string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get second string
			if (!getline(sline, second, '.'))
			{
				Log::WriteLine("Failed to get second string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get millisecond string
			if (!getline(sline, millisecond, ','))
			{
				Log::WriteLine("Failed to get millisecond string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get ask string
			if (!getline(sline, ask, ','))
			{
				Log::WriteLine("Failed to get ask string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get bid string
			if (!getline(sline, bid, ','))
			{
				Log::WriteLine("Failed to get bid string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get ask_volume string
			if (!getline(sline, askVolume, ','))
			{
				Log::WriteLine("Failed to get ask_volume string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}
			// get bid_volume string
			if (!getline(sline, bidVolume, ','))
			{
				Log::WriteLine("Failed to get bid_volume string at line %d of %s", csvFile.line, csvFile.path);
				SetStatus(Status::Stopped);
				break;
			}

			/*
			* save to bi5 record
			*/
			sprintf_s(buf, "%s\\%s\\%s\\%02d\\%s\\%02dh_ticks", GetDestPath(), productName.c_str(), year.c_str(), stoi(month) - 1, day.c_str(), stoi(hour)/* - 1*/);
			if (strcmp(curBi5Path, buf) != 0)
			{
				// try to create output directory
				char dirPath[CSV2BI5_MAX_PATH];
				sprintf_s(dirPath, "%s\\%s\\%s\\%02d\\%s", GetDestPath(), productName.c_str(), year.c_str(), stoi(month) - 1, day.c_str());
				fs::create_directories(dirPath);
				// open bi5 tmp file
				bi5tmp.close();
				bi5tmp.open(buf, ios::out | ios::app | ios::binary);
				if (!bi5tmp.is_open())
				{
					Log::WriteLine("Failed to open bi5 file %s", curBi5Path);
					continue;
				}
				Bi5File bi5File;
				strcpy_s(bi5File.path, buf);
				m_bi5Files.push_back(bi5File);
				m_workLoad.QuadPart += 100;
				strcpy_s(curBi5Path, buf);
			}
			// write bi5 record
			int intMillisecond = (stoi(minute) * 60 + stoi(second)) * 1000 + stoi(millisecond);
			/*int intAsk = (int)(stof(ask) * 100000);
			int intBid = (int)(stof(bid) * 100000);
			static int i = 0;
			float f;
			if (++i == 14)
			{
				f = (float)stof(ask);
				f = (float)stof(bid);
				f = (float)atof(ask.c_str());
				f = (float)atof(bid.c_str());
				sscanf(ask.c_str(), "%f", &f);
				sscanf(bid.c_str(), "%f", &f);
			}*/
			int intAsk = StringToInt(ask);
			int intBid = StringToInt(bid);
			float floatAskVolume = stof(askVolume);
			float floatBidVolume = stof(bidVolume);
			ByteSwap(&intMillisecond);
			ByteSwap(&intAsk);
			ByteSwap(&intBid);
			ByteSwap(&floatAskVolume);
			ByteSwap(&floatBidVolume);
			bi5tmp.write((char*)(&intMillisecond), sizeof(int));
			bi5tmp.write((char*)(&intBid), sizeof(int));
			bi5tmp.write((char*)(&intAsk), sizeof(int));
			bi5tmp.write((char*)(&floatBidVolume), sizeof(float));
			bi5tmp.write((char*)(&floatAskVolume), sizeof(float));
		}
	}

	bi5tmp.close();

	for (list<Bi5File>::iterator it = m_bi5Files.begin(); it != m_bi5Files.end(); ++it)
	{
		Bi5File &bi5File = *it;
		strcpy_s(buf, bi5File.path);
		strcat_s(buf, ".bi5");
		//Log::WriteLine("Compressing to %s", buf);
		simpleCompress(elzma_file_format::ELZMA_lzma, bi5File.path, buf);
		m_curLoad.QuadPart += 100;
	}

	//Log::WriteLine("Finished to convert");
	SetStatus(Status::Stopped);
}

void Csv2Bi5Engine::IncreaseProcessed()
{
	m_processingPos++;
}

void Csv2Bi5Engine::InitProcessed()
{
	m_curLoad.QuadPart = 0;
	m_workLoad.QuadPart = 0;
	m_processingPos = -1;
}

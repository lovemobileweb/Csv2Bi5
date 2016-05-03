#pragma once

#include <list>

#define CSV2BI5_MAX_PATH 256

class Csv2Bi5Engine
{
public:
	struct CsvFile
	{
		char path[CSV2BI5_MAX_PATH];
		int line;
	};

	struct Bi5File
	{
		char path[CSV2BI5_MAX_PATH];
	};

	enum Status
	{
		None,
		Starting,
		Started,
		Stopping,
		Stopped
	};

private:
	char m_sourcePath[CSV2BI5_MAX_PATH];
	char m_destPath[CSV2BI5_MAX_PATH];
	std::list<CsvFile> m_csvFiles;
	std::list<Bi5File> m_bi5Files;
	int m_processingPos;
	LARGE_INTEGER m_curLoad;
	LARGE_INTEGER m_workLoad;
	Status m_status;

public:
	Csv2Bi5Engine();
	~Csv2Bi5Engine();
	float GetProgress();
	void StartConvert(const char* sourcePath, const char* destPath);
	void StopConvert();
	Status GetStatus();
	void SetStatus(Status status);
	const char* GetSourcePath();
	const char* GetDestPath();

private:
	static void Run(void *arg);
	void Process();
	void IncreaseProcessed();
	void InitProcessed();
};


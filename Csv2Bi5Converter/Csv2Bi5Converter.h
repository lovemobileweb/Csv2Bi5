
// Csv2Bi5Converter.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCsv2Bi5ConverterApp:
// See Csv2Bi5Converter.cpp for the implementation of this class
//

class CCsv2Bi5ConverterApp : public CWinApp
{
public:
	CCsv2Bi5ConverterApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCsv2Bi5ConverterApp theApp;
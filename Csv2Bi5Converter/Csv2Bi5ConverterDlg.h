
// Csv2Bi5ConverterDlg.h : header file
//

#pragma once

#include "Csv2Bi5Engine.h"
#include "afxwin.h"
#include "afxcmn.h"

// CCsv2Bi5ConverterDlg dialog
class CCsv2Bi5ConverterDlg : public CDialogEx
{
// Construction
public:
	CCsv2Bi5ConverterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CSV2BI5CONVERTER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	Csv2Bi5Engine m_converter;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_strSourcePath;
	CButton m_btnConvert;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CProgressCtrl m_ctrlProgress;
	CEdit m_editSourcePath;
	CEdit m_editDestPath;
};

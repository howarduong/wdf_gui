// wdf_gui2Dlg.h : Dialog
//

#pragma once

#include "wdf_unpacker.h"
#include "DictionaryManager.h"

// Cwdf_gui2Dlg dialog
class Cwdf_gui2Dlg : public CDialogEx
{
// 
public:
	Cwdf_gui2Dlg(CWnd* pParent = NULL);	// Default constructor

// Dialog
	enum { IDD = IDD_WDF_GUI2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// New: Control variable
	CEdit m_editWdf;
	CEdit m_editLst;
	CEdit m_editLogWdf;
	CEdit m_editLogLst;
	CEdit m_editLogB;
	CProgressCtrl m_progress;
	WDFUnpacker m_unpacker;
	DictionaryManager m_dictMgr;

	// Message map
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTabLog(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBrowseWdf();
	afx_msg void OnBnClickedBrowseLst();
	afx_msg void OnBnClickedUnpack();
	afx_msg void OnBnClickedDictCollect();
	afx_msg void OnBnClickedDictParse();
};

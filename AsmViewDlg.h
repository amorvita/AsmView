/*--

	Copyright (c) 2015 YoungJin Shin <codewiz@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>

	Abstrct:

		AsmView
		x86 Byte Code Disassembler

--*/

#pragma once

#include "DiStorm3\distorm.h"
#include "afxwin.h"

// CAsmViewDlg dialog
class CAsmViewDlg : public CDialogEx
{
// Construction
public:
	CAsmViewDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ASMVIEW_DIALOG };
	CFont CodeFont;
	CPoint MinTrack;
	int MaxWidth;
	CString ConfigPath;

	CRect BytesMargin;
	CRect CodeMargin;
	BOOL MarginInitComplete;
	int CodeBytesSize;


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnDisasm();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnAbout();
	afx_msg void OnSetFont();
	CEdit StartAddressEditor;
	CComboBox DecodeType;
	CEdit BytesEdit;
	CEdit CodeEdit;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnExit();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnSetCodeBytesSize(UINT id);

	void CheckCodeBytesSizeMenuItem(UINT id);
};

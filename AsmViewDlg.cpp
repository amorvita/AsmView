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

#include "stdafx.h"
#include "AsmView.h"
#include "AsmViewDlg.h"
#include "afxdialogex.h"
#include <strsafe.h>
#pragma comment(lib, "version.lib")

BOOL GetFixedVersionInfo(LPCTSTR path, VS_FIXEDFILEINFO *fi)
{
	PBYTE			data = NULL;	
	DWORD			size;
	TCHAR			shortPath[MAX_PATH];
	PVOID			fixedVersion;
	BOOL			ret = FALSE;
	UINT			len;

	if(GetShortPathName(path, shortPath, MAX_PATH) == 0)
		goto $cleanup;

	size = GetFileVersionInfoSize(shortPath, NULL);
	if(size == 0)
		goto $cleanup;

	data = new BYTE[size];

	if(!GetFileVersionInfo(shortPath, 0, size, data))
		goto $cleanup;

	if(!VerQueryValue(data, TEXT("\\"), &fixedVersion, &len))
		goto $cleanup;

	memcpy(fi, fixedVersion, sizeof(*fi));
	ret = TRUE;

$cleanup:
	if(data)
		delete [] data;

	return ret;	
}

BOOL WINAPI
VerToStr(DWORD hi, DWORD lo, LPTSTR buf, SIZE_T size)
{
	DWORD	v1 = hi >> 16;
	DWORD	v2 = hi & 0xffff;
	DWORD	v3 = lo >> 16;
	DWORD	v4 = lo & 0xffff;

	return SUCCEEDED(StringCbPrintf(buf, size, TEXT("%d.%d.%d.%d"), v1, v2, v3, v4));
}

BOOL GetVersion(LPCTSTR path, LPTSTR version, SIZE_T size)
{
	VS_FIXEDFILEINFO fi;
	if(!GetFixedVersionInfo(path, &fi))
		return FALSE;

	VerToStr(fi.dwFileVersionMS, fi.dwFileVersionLS, version, size);
	return TRUE;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace CodeBytesSizeConv
{
	static int SizeList[] = { 0, 16, 20, 24, 32 };

	int Id2Size(UINT id)
	{
		int Pos = id - ID_CODEBYTESSIZE_0BYTES;
		return SizeList[Pos];
	}

	UINT Size2Id(int Len)
	{
		for(int i=0; i<ARRAYSIZE(SizeList); ++i)
			if(SizeList[i] == Len)
				return ID_CODEBYTESSIZE_0BYTES + i;

		return ID_CODEBYTESSIZE_16BYTES;
	}
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()

END_MESSAGE_MAP()


CAsmViewDlg::CAsmViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAsmViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	MinTrack.x = GetSystemMetrics(SM_CXMINTRACK);
	MinTrack.y = GetSystemMetrics(SM_CYMINTRACK);
	MaxWidth = GetSystemMetrics(SM_CXMAXTRACK);

	MarginInitComplete = FALSE;
}

void CAsmViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_START, StartAddressEditor);
	DDX_Control(pDX, IDC_CMB_DECODE_TYPE, DecodeType);
	DDX_Control(pDX, IDC_EDT_HEX, BytesEdit);
	DDX_Control(pDX, IDC_EDT_CODE, CodeEdit);
}

BEGIN_MESSAGE_MAP(CAsmViewDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_DISASM, &CAsmViewDlg::OnBnClickedBtnDisasm)
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(IDM_ABOUT, &CAsmViewDlg::OnAbout)
	ON_COMMAND(IDM_SET_FONT, &CAsmViewDlg::OnSetFont)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CAsmViewDlg::OnBnClickedBtnClear)
	ON_COMMAND(IDM_EXIT, &CAsmViewDlg::OnExit)
	ON_WM_HELPINFO()
	ON_COMMAND_RANGE(ID_CODEBYTESSIZE_0BYTES, ID_CODEBYTESSIZE_32BYTES, OnSetCodeBytesSize)
END_MESSAGE_MAP()


BOOL CAsmViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	CRect AsmViewRect;
	CRect BytesRect;
	CRect CodeRect;

	GetWindowRect(&AsmViewRect);
	CodeEdit.GetWindowRect(&CodeRect);
	BytesEdit.GetWindowRect(&BytesRect);

	MinTrack.x = AsmViewRect.Width();
	MinTrack.y = AsmViewRect.Height();

	MaxWidth = AsmViewRect.Width();


	BytesMargin.SetRect(BytesRect.left - AsmViewRect.left
						, BytesRect.top - AsmViewRect.top
						, AsmViewRect.right - BytesRect.right
						, AsmViewRect.bottom - BytesRect.bottom);

	CodeMargin.SetRect(CodeRect.left - AsmViewRect.left
						, CodeRect.top - AsmViewRect.top
						, AsmViewRect.right - CodeRect.right
						, AsmViewRect.bottom - CodeRect.bottom);

	MarginInitComplete = TRUE;

	TCHAR Path[MAX_PATH];
	GetModuleFileName(NULL, Path, ARRAYSIZE(Path));
	TCHAR *Sep = _tcsrchr(Path, _T('.'));
	if(*Sep)
		*Sep = _T('\0');
	StringCchCopy(Path, ARRAYSIZE(Path), _T(".ini"));
	ConfigPath = Path;

	LOGFONT lf;

	if(GetPrivateProfileStruct(_T("AsmView"), _T("Font"), &lf, sizeof(lf), Path))
	{
		CodeFont.CreateFontIndirect(&lf);
	}
	else
	{
		CodeFont.CreateStockObject(DEFAULT_GUI_FONT);
	}

	CodeEdit.SetFont(&CodeFont);

	DecodeType.SetCurSel(GetPrivateProfileInt(_T("AsmView"), _T("DecodeType"), 1, Path));

	TCHAR StartAddress[MAX_PATH];
	GetPrivateProfileString(_T("AsmView"), _T("StartAddress"), _T("0"), StartAddress, ARRAYSIZE(StartAddress), Path);
	StartAddressEditor.SetWindowText(StartAddress);

	CodeBytesSize = GetPrivateProfileInt(_T("AsmView"), _T("CodeBytesSize"), 16, Path);
	UINT id = CodeBytesSizeConv::Size2Id(CodeBytesSize);
	CodeBytesSize = CodeBytesSizeConv::Id2Size(id);
	CheckCodeBytesSizeMenuItem(id);

	WINDOWPLACEMENT Place;
	if(GetPrivateProfileStruct(_T("AsmView"), _T("Placement"), &Place, sizeof(Place), Path))
		SetWindowPlacement(&Place);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAsmViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAsmViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAsmViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UCHAR GetBinaryValue(char Ch)
{
	switch(Ch)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return (Ch - '0');

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		return (Ch - 'a') + 10;

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		return (Ch - 'A') + 10;
	}

	return 0xff;
}

void CAsmViewDlg::OnBnClickedBtnDisasm()
{
	// TODO: Add your control notification handler code here

	UINT Decoded;
	_DecodedInst Results[128];
	PUCHAR Binary;
	ULONG BinarySize, Pos = 0;

	ULONG Len = BytesEdit.GetWindowTextLength();
	Binary = new UCHAR[Len];

	CString Str;
	TCHAR Ch;
	
	BytesEdit.GetWindowText(Str);
	
	for(int i=0; i<Len; )
	{
		Ch = Str.GetAt(i);
		if(GetBinaryValue(Ch) == 0xff || GetBinaryValue(Str.GetAt(i+1)) == 0xff)
		{
			++i;
			continue;
		}
		else if(Ch == _T('0') && Str.GetAt(i+1) == _T('x'))
		{
			i += 2;
			continue;
		}
		
		Binary[Pos] = (GetBinaryValue(Str.GetAt(i)) << 4) + GetBinaryValue(Str.GetAt(i+1));
		
		i += 2;
		++Pos;
	}

	BinarySize = Pos;
	
	CodeEdit.SetWindowText(_T(""));

	unsigned __int64 Address = 0;

	CString StartAddress;
	StartAddressEditor.GetWindowText(StartAddress);

	for(int i=0; i<StartAddress.GetLength(); ++i)
	{
		Address = 16 * Address + GetBinaryValue(StartAddress.GetAt(i));
	}
	

	CString R;
	CString tmp;
	
	PUCHAR CodeBytes = Binary;
	ULONG Remain = BinarySize;

	_DecodeType Type;
	int SelItem = DecodeType.GetCurSel();
	switch(SelItem)
	{
	case 0:
		Type = Decode16Bits;
		break;

	case -1:
	case 1:
		Type = Decode32Bits;
		break;

	case 2:
		Type = Decode64Bits;
		break;
	}

	for(;;)
	{
		distorm_decode64(Address, CodeBytes, Remain, Type, Results, ARRAYSIZE(Results), &Decoded);
		for(UINT i=0; i<Decoded; ++i)
		{
			if(CodeBytesSize)
			{
				if(Type == Decode64Bits)
					tmp.Format(_T("%08x %08x  %*S  %-8S %S\r\n")
								, (ULONG)(Address >> 32)
								, (ULONG)(Address & 0xffffffff)
								, -CodeBytesSize
								, Results[i].instructionHex.p
								, Results[i].mnemonic.p
								, Results[i].operands.p);
				else
					tmp.Format(_T("%08x  %*S  %-8S %S\r\n")
								, (ULONG)(Address & 0xffffffff)
								, -CodeBytesSize
								, Results[i].instructionHex.p
								, Results[i].mnemonic.p
								, Results[i].operands.p);
			}
			else
			{
				if(Type == Decode64Bits)
					tmp.Format(_T("%08x %08x  %-8S %S\r\n")
								, (ULONG)(Address >> 32)
								, (ULONG)(Address & 0xffffffff)
								, Results[i].mnemonic.p
								, Results[i].operands.p);
				else
					tmp.Format(_T("%08x  %-8S %S\r\n")
								, (ULONG)(Address & 0xffffffff)
								, Results[i].mnemonic.p
								, Results[i].operands.p);
			}
			
			Address += Results[i].size;
			CodeBytes += Results[i].size;
			Remain -= Results[i].size;
			R += tmp;	
		}

		if(Decoded != 0)
		{
			CodeEdit.SetSel(CodeEdit.GetWindowTextLength(), CodeEdit.GetWindowTextLength());
			CodeEdit.ReplaceSel(R);
		}
		else if(Remain == 0 || Decoded == 0)
		{
			break;
		}
	}

	CodeEdit.SetSel(0, 0);

	delete [] Binary;

}


void CAsmViewDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	//lpMMI->ptMaxTrackSize.x = MaxWidth;
	lpMMI->ptMinTrackSize = MinTrack;
	

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CAboutDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages

	CRect rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, RGB(255,255,255));
}


HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH) GetStockObject(WHITE_BRUSH);
}




BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	CString str, tmp;
	str.LoadString(IDS_TITLE);
	GetDlgItem(IDC_TITLE)->SetWindowText(str);

	TCHAR exe[MAX_PATH];
	GetModuleFileName(NULL, exe, sizeof(exe));

	TCHAR buf[MAX_PATH];
	GetVersion(exe, buf, sizeof(buf));
	str.Format(_T("%s"), buf);
	GetDlgItem(IDC_VERSION)->SetWindowText(str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAsmViewDlg::OnAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CAsmViewDlg::OnSetFont()
{
	// TODO: Add your command handler code here

	CHOOSEFONT cf;
	LOGFONT lf;

	CodeFont.GetLogFont(&lf);

	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = GetSafeHwnd();
	cf.lpLogFont = &lf;
	cf.rgbColors = RGB(0,0,0);
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_ANSIONLY | CF_FIXEDPITCHONLY | CF_INITTOLOGFONTSTRUCT;

	if(ChooseFont(&cf) == TRUE)
	{
		CodeFont.DeleteObject();
		CodeFont.CreateFontIndirect(cf.lpLogFont);

		CodeEdit.SetFont(&CodeFont);

		//UpdateWindow();
	}
}


void CAsmViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if(MarginInitComplete)
	{
		CRect Main, CodeRect, BytesRect;
		GetWindowRect(&Main);
		CodeEdit.GetWindowRect(&CodeRect);
		BytesEdit.GetWindowRect(&BytesRect);

		CodeEdit.SetWindowPos(&wndNoTopMost, 0, 0
								, Main.right - CodeMargin.right - (Main.left + CodeMargin.left)
								, Main.bottom - CodeMargin.bottom - (Main.top + CodeMargin.top)
								, SWP_NOZORDER | SWP_NOMOVE);

		BytesEdit.SetWindowPos(&wndNoTopMost, 0, 0
								, Main.right - BytesMargin.right - (Main.left + BytesMargin.left)
								, BytesRect.Height()
								, SWP_NOZORDER | SWP_NOMOVE);
	}
}


BOOL CAsmViewDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return TRUE;

	return CDialogEx::OnEraseBkgnd(pDC);
}


void CAsmViewDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here

	LOGFONT lf;
	CodeFont.GetLogFont(&lf);
	WritePrivateProfileStruct(_T("AsmView"), _T("Font"), &lf, sizeof(lf), ConfigPath);

	CString StartAddress;
	StartAddressEditor.GetWindowText(StartAddress);
	WritePrivateProfileString(_T("AsmView"), _T("StartAddress"), StartAddress, ConfigPath);

	CString Type;
	Type.Format(_T("%d"), DecodeType.GetCurSel());
	WritePrivateProfileString(_T("AsmView"), _T("DecodeType"), Type, ConfigPath);

	CString Size;
	Size.Format(_T("%d"), CodeBytesSize);
	WritePrivateProfileString(_T("AsmView"), _T("CodeBytesSize"), Size, ConfigPath);

	WINDOWPLACEMENT Place;
	GetWindowPlacement(&Place);
	WritePrivateProfileStruct(_T("AsmView"), _T("Placement"), &Place, sizeof(Place), ConfigPath);

}


void CAsmViewDlg::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here
	CodeEdit.SetWindowText(_T(""));
}


void CAsmViewDlg::OnExit()
{
	// TODO: Add your command handler code here
	EndDialog(IDOK);
}


BOOL CAsmViewDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	// TODO: Add your message handler code here and/or call default

	//return CDialogEx::OnHelpInfo(pHelpInfo);
	OnAbout();
	return TRUE;
}

void CAsmViewDlg::OnSetCodeBytesSize(UINT id)
{
	CodeBytesSize = CodeBytesSizeConv::Id2Size(id);
	CheckCodeBytesSizeMenuItem(id);
}

void CAsmViewDlg::CheckCodeBytesSizeMenuItem(UINT id)
{
	CMenu *Menu = GetMenu();
	CMenu *SubMenu = Menu->GetSubMenu(1);
	CMenu *CMenu = SubMenu->GetSubMenu(0);
	CMenu->CheckMenuRadioItem(ID_CODEBYTESSIZE_0BYTES, ID_CODEBYTESSIZE_32BYTES, id, MF_BYCOMMAND);
}

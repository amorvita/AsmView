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

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAsmViewApp:
// See AsmView.cpp for the implementation of this class
//

class CAsmViewApp : public CWinApp
{
public:
	CAsmViewApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	HACCEL Accel;

	DECLARE_MESSAGE_MAP()
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
};

extern CAsmViewApp theApp;
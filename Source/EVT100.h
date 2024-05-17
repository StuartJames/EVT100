/*
 *        Copyright (c) 2020-2021 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or   
 *  modify it under the terms of the GNU General Public License as  
 *  published by the Free Software Foundation; either version 2 of  
 *  the License, or (at your option) any later version.             
 *                                                                  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of  
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   
 *  GNU General Public License for more details.                    
 *                                                                  
 *
 *  Based on a design by Michael Haardt
 *
 * Edit Date/Ver   Edit Description
 * ==============  ===================================================
 * SJ   19/08/2020  Original
 *
 */

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

extern const TCHAR szEVT100Class[];

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////

class CEVT100App : public CWinApp
{
public:
									CEVT100App();

	virtual BOOL		InitInstance();
	virtual int			ExitInstance();
	virtual BOOL		PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

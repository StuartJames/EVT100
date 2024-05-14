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

#include "stdafx.h"
#include "EVT100Defs.h"
#include "EVT100.h"
#include "mainfrm.h"
#include "EVT100Doc.h"
#include "EVT100View.h"
#include "EVT100Dlgs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#pragma comment(linker,"/manifestdependency:\"type='win32' name='microsoft.windows.Common-Controls' "\
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const TCHAR szHydraSystems[] = _T("HydraSystems");
const TCHAR szEmaClass[] = _T("EmaClass");

/////////////////////////////////////////////////////////////////////////////
// CEVT100App

BEGIN_MESSAGE_MAP(CEVT100App, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
//	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEVT100App construction

CEVT100App::CEVT100App()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CEVT100App object

CEVT100App theApp;

/////////////////////////////////////////////////////////////////////////////
// CEVT100App initialization

BOOL CEVT100App::InitInstance()
{
  //HWND hwnd = ::FindWindow (szEmaClass,NULL);
  //if(hwnd){
  //  if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
  //  SetForegroundWindow (hwnd);
  //  return FALSE;
  //}
  SetRegistryKey(szHydraSystems);
  LoadStdProfileSettings();  // Load standard INI file options (including MRU)
  CSingleDocTemplate* pDocTemplate;
  pDocTemplate = new CSingleDocTemplate(IDR_MAINFRAME,RUNTIME_CLASS(CEVT100Doc),RUNTIME_CLASS(CMainFrame),RUNTIME_CLASS(CEVT100View));
  AddDocTemplate(pDocTemplate);
//  OnFileNew();
  CCommandLineInfo cmdInfo; 
	ParseCommandLine(cmdInfo);
  if(!ProcessShellCommand(cmdInfo)) return FALSE;
  m_nCmdShow = SW_SHOWNORMAL;
  m_pMainWnd->UpdateWindow();
  m_pERecentFileList = m_pRecentFileList;
  ((CMainFrame*)m_pMainWnd)->Initialize();        // Show any previouse views
  return TRUE;
}


int CEVT100App::ExitInstance() 
{
  return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////

void CEVT100App::OnAppAbout()
{
CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}



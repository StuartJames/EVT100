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
#include "SplashWnd.h"
#include "EVT100.h"
#include "mainfrm.h"
#include "EVT100Doc.h"
#include "EVT100View.h"
#include "EVT100Dlgs.h"
#include "globals.h"
#include <dwmapi.h>
#include "EVT100VisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#pragma comment(linker,"/manifestdependency:\"type='win32' name='microsoft.windows.Common-Controls' "\
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

TXTREGN Regions[4] = {
  {60,  30, 35, 20, _T("Arial Black"),{-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true},
  {90,  65, 35, 20, _T("Lucida"),{-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true},
  {160, 15, 10, 20, _T("Tahoma"),{-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true},
  {25,  30, 35, 20, _T("Tahoma"),{-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true}
};

const TCHAR szHydraSystems[] = _T("HydraSystems");
const TCHAR szEVT100Class[] = _T("EVT100Class");

/////////////////////////////////////////////////////////////////////////////
// CEVT100App

BEGIN_MESSAGE_MAP(CEVT100App, CWinAppEx)
//	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
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

BOOL CEVT100App::InitInstance()
{
  SetRegistryKey(szHydraSystems);
  LoadStdProfileSettings();  // Load standard INI file options (including MRU)
  CSplashWnd::InitialiseRegions(Regions, true);
  CSingleDocTemplate* pDocTemplate;
  INITCOMMONCONTROLSEX CommonControls;
  CommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
  CommonControls.dwICC = ICC_STANDARD_CLASSES;  // I could not see any effect of the specific value here
  InitCommonControlsEx(&CommonControls);
  InitContextMenuManager();
  InitKeyboardManager();
  InitTooltipManager();
  CMFCToolTipInfo ttParams;
  ttParams.m_bVislManagerTheme = TRUE;
  theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);
  pDocTemplate = new CSingleDocTemplate(IDR_MAINFRAME,RUNTIME_CLASS(CEVT100Doc),RUNTIME_CLASS(CMainFrame),RUNTIME_CLASS(CEVT100View));
  AddDocTemplate(pDocTemplate);
  CCommandLineInfo cmdInfo; 
	ParseCommandLine(cmdInfo);
  if(!ProcessShellCommand(cmdInfo)) return FALSE;
  CSplashWnd::ShowSplashScreen(SPLASH_TIME, IDB_SPLASH, m_pMainWnd);// show main frame before this statement so that correct screen is used
  SetFrameColours(); 
  m_pMainWnd->UpdateWindow();
  ((CMainFrame*)m_pMainWnd)->Initialize();        // Show any previouse views
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

int CEVT100App::ExitInstance() 
{
  return CWinAppEx::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100App::SetFrameColours() 
{
BOOL value = TRUE;
COLORREF CapCol = CLR_CAPTION_COLOR;
COLORREF BrdCol = CLR_BOARDER_COLOR;

  ::DwmSetWindowAttribute(m_pMainWnd->m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
  ::DwmSetWindowAttribute(m_pMainWnd->m_hWnd, DWMWA_CAPTION_COLOR, &CapCol, sizeof(CapCol));
  ::DwmSetWindowAttribute(m_pMainWnd->m_hWnd, DWMWA_BORDER_COLOR, &BrdCol, sizeof(BrdCol));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100App::PreTranslateMessage(MSG* pMsg)
{
	if(CSplashWnd::PreTranslateAppMessage(pMsg)) return TRUE;
  return CWinAppEx::PreTranslateMessage(pMsg);
}

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
#include "Globals.h"
#include "EVT100Defs.h"
#include "SplashWnd.h"
#include "EVT100.h"
#include "EVT100Doc.h"
#include "MainFrm.h"
#include "EVT100VisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars
	
// toolbar buttons - IDs are command buttons
static UINT buttons[] =
{
	ID_FILE_CONNECT,
	ID_SEPARATOR,
	IDM_VIEW_PAUSE,
	ID_SEPARATOR,
	IDM_VIEW_CLEAR,
};

static UINT indicators[] =
{
	ID_SEPARATOR,       
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// Helpers for saving/restoring window state and variables

static char BASED_CODE szSettings[] = "Settings";
static char BASED_CODE szWindowPos[] = "WindowPos";
static char szWFormat[] = "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d";

/////////////////////////////////////////////////////////////////////////////

static BOOL PASCAL NEAR ReadWindowPlacement(LPWINDOWPLACEMENT pwp)
{

  CString strBuffer = AfxGetApp()->GetProfileString(szSettings, szWindowPos);
  if (strBuffer.IsEmpty()) return FALSE;
  WINDOWPLACEMENT wp;
	int nRead = sscanf_s(strBuffer, szWFormat,
    &wp.flags, &wp.showCmd,
    &wp.ptMinPosition.x, &wp.ptMinPosition.y,
    &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
    &wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
    &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);
  if (nRead != 10) return FALSE;
  wp.length = sizeof wp;
  *pwp = wp;
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

static void PASCAL NEAR WriteWindowPlacement(LPWINDOWPLACEMENT pwp)
{
char szBuffer[sizeof("-32767") * 8 + sizeof("65535") * 2];

  sprintf_s(szBuffer, sizeof(szBuffer), szWFormat,
    pwp->flags, pwp->showCmd,
    pwp->ptMinPosition.x, pwp->ptMinPosition.y,
    pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
    pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
    pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
  AfxGetApp()->WriteProfileString(szSettings, szWindowPos, szBuffer);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
  ON_WM_CREATE()
  ON_WM_CLOSE()
  ON_WM_TIMER()
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
  ON_COMMAND(ID_FILE_CONNECT, OnConnect)
  ON_COMMAND(ID_FILE_DISCONNECT, OnConnect)
  ON_UPDATE_COMMAND_UI(ID_FILE_CONNECT, OnUpdateConnect)
  ON_UPDATE_COMMAND_UI(ID_FILE_DISCONNECT, OnUpdateConnect)
  ON_MESSAGE(WM_CTLCOLORSCROLLBAR, OnCtlColorScrollBar)
  ON_MESSAGE(WM_CTLCOLORDLG, OnCtlColorScrollBar)
END_MESSAGE_MAP()

LRESULT CMainFrame::OnCtlColorScrollBar(WPARAM wParam, LPARAM lParam)
{
  CDC *pDC = CDC::FromHandle((HDC)wParam);
  return (LRESULT)(HBRUSH)m_BkGndBrush;
}

/////////////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame()
{
  m_BkGndBrush.CreateSolidBrush(RGB(20,20,20)); 
}

/////////////////////////////////////////////////////////////////////////////

CMainFrame::~CMainFrame()
{
  m_BkGndBrush.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
WNDCLASS wndcls;

	BOOL bRes = CFrameWndEx::PreCreateWindow(cs);
  HINSTANCE hInst = AfxGetInstanceHandle();
  if (!::GetClassInfo(hInst, szEVT100Class, &wndcls)){	// see if the class already exists
    ::GetClassInfo(hInst, cs.lpszClass, &wndcls);	// get default stuff
    wndcls.style &= ~(CS_HREDRAW | CS_VREDRAW);
    wndcls.lpszClassName = szEVT100Class;              // register a new class
    wndcls.hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(IDR_MAINFRAME));
    ASSERT(wndcls.hIcon != NULL);
    if (!AfxRegisterClass(&wndcls)) AfxThrowResourceException();
  }
  cs.lpszClass = szEVT100Class;
  return bRes;
}

/////////////////////////////////////////////////////////////////////////////

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
WINDOWPLACEMENT wp;

  if(CFrameWndEx::OnCreate(lpCreateStruct) == -1) return -1;
  CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CVT100VisualManager));
  if(ReadWindowPlacement(&wp)) SetWindowPlacement(&wp);
  if(!m_wndMenuBar.Create(this)){
    TRACE0("Failed to create menubar\n");
    return -1; 
  }
  m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
  CMFCPopupMenu::SetForceMenuFocus(FALSE);     // prevent the menu bar from taking the focus on activation     
  if(!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER |	CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
    !m_wndToolBar.LoadToolBar(IDR_DISCONNECTED)){
    TRACE0("Failed to create toolbar\n");
    return -1;   
  }
  m_wndToolBar.AutoGrayInactiveImages(FALSE, 0, TRUE);
  if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT))){
    TRACE0("Failed to create status bar\n");
    return -1;   
  }
  m_wndStatusBar.PostMessage(SB_SETTEXT, 1 | SBT_OWNERDRAW, (LPARAM)0);
  m_wndStatusBar.SetPaneWidth(0, 400);
  m_wndMenuBar.EnableDocking(CBRS_ALIGN_TOP);
  m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
  EnableDocking(CBRS_ALIGN_ANY);
  DockPane(&m_wndMenuBar);
  DockPane(&m_wndToolBar);
  CDockingManager::SetDockingMode(DT_SMART);
  EnableAutoHidePanes(CBRS_ALIGN_TOP);
//  EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);
  CRect rc(0,0,0,0);
  VERIFY(m_wndHScroll.Create(WS_VISIBLE | WS_CHILD | SBS_HORZ, rc, this, AFX_IDW_HSCROLL_FIRST));
  VERIFY(m_wndVScroll.Create(WS_VISIBLE | WS_CHILD | SBS_VERT, rc, this, AFX_IDW_HSCROLL_FIRST + 1));
  SetWindowText("EVT100");  // set the name of the main window
  return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnClose() 
{
WINDOWPLACEMENT wp;
 
  wp.length = sizeof wp;
  if(GetWindowPlacement(&wp)){
    wp.flags = 0;
    if(IsZoomed()) wp.flags |= WPF_RESTORETOMAXIMIZED;
    WriteWindowPlacement(&wp);
  }
  CFrameWndEx::OnClose();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
  CFrameWndEx::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

CEVT100Doc* CMainFrame::GetDocument() 
{  
  CEVT100Doc* pDoc=(CEVT100Doc*) GetActiveDocument();
  if(pDoc != NULL) return pDoc;
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{                                   
  if(nIDEvent == IDT_BREAKTIMEOUT){
    KillTimer(IDT_BREAKTIMEOUT);
		ClearCommBreak(GetDocument()->m_idComDev);
  }
}

/////////////////////////////////////////////////////////////////////

void CMainFrame::OnAppAbout()
{
	CSplashWnd::ShowSplashScreen(0, IDB_SPLASH, this);
}
/////////////////////////////////////////////////////////////////////////////

void CMainFrame::Initialize() 
{
	OnConnect();
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnConnect() 
{
  CEVT100Doc* pDoc = (CEVT100Doc*)GetDocument();
	if(pDoc == NULL) return;
  if(pDoc->m_IsConnected) pDoc->CloseConnection();
  else if(!pDoc->OpenConnection()) AfxMessageBox(IDS_CONNECTION_FAILED, MB_OK | MB_ICONEXCLAMATION);
  CMFCToolBar::ResetAllImages();
  if(!pDoc->m_IsConnected) m_wndToolBar.LoadBitmap(IDR_DISCONNECTED);
  else m_wndToolBar.LoadBitmap(IDR_CONNECTED);
  m_wndToolBar.Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateConnect(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(GetDocument()->m_IsConnected == TRUE);
  pCmdUI->Enable(TRUE);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
  CEVT100Doc* pDoc = (CEVT100Doc*)GetDocument();
  return CFrameWndEx::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::SetWindowTitle(CString Title) 
{
CString cstr;

  if(Title.IsEmpty()) cstr.Format(_T("EVT100"), (LPCSTR)Title);
  else cstr.Format(_T("EVT100 - %s"), (LPCSTR)Title);
  SetWindowText(cstr);  // set the name of the main window
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::RecalcLayout(BOOL bNotify)
{
  CFrameWndEx::RecalcLayout(bNotify);
  CView* pView = GetActiveView();
  if(pView != NULL) {
    CRect rc;
    pView->GetWindowRect(&rc);
    ScreenToClient(&rc);
    int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
    int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
    rc.right -= cxVScroll;
    rc.bottom -= cyHScroll;
    pView->MoveWindow(rc);
    rc.left = rc.right;
    rc.right += cxVScroll;
    m_wndVScroll.MoveWindow(rc);
    rc.left = 0;
    rc.right -= cxVScroll;
    rc.top = rc.bottom;
    rc.bottom+= cyHScroll;
    m_wndHScroll.MoveWindow(rc);
  }
}


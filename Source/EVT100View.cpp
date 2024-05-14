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
#include "EVT100Doc.h"
#include "EVT100View.h"
#include "EVTColorPalette.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CEVT100View, CScrollView)

BEGIN_MESSAGE_MAP(CEVT100View, CScrollView)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_COMMNOTIFY, OnCommNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CEVT100View::CEVT100View()
{
  m_bHasCaret = FALSE;
  m_pFont = NULL;
  m_InBlock = new BYTE[MAXBLOCK + 1];
}

/////////////////////////////////////////////////////////////////////////////

CEVT100View::~CEVT100View()
{
  if(m_bHasCaret){
    HideCaret();
    DestroyCaret();
    m_bHasCaret = FALSE;
  }
  if(m_pFont != NULL){
    delete m_pFont;
    m_pFont = NULL;
  }
  if(m_InBlock) delete m_InBlock;
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CEVT100View::AssertValid() const
{
  CScrollView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::Dump(CDumpContext& dc) const
{
  CScrollView::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////

CEVT100Doc* CEVT100View::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEVT100Doc)));
  return (CEVT100Doc*)m_pDocument;
}
#endif 

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnInitialUpdate() 
{
  CScrollView::OnInitialUpdate();
  SetSizes();		// Reset scrollbars
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
char cChar;
bool IsActioned = false;

  CEVT100Doc *pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  cChar = (char)nChar;
  if(pDoc->m_IsConnected){
		if(nChar < 128){	// Write character to port; if local echo is set, echo it
		  if(pDoc->m_bLocalEcho) pDoc->FormatScreenData("%c", cChar);
			UpdateWindow();
	    pDoc->SendHostByte(cChar);
			IsActioned = true;
		}
  }
  if(!IsActioned) CScrollView::OnChar(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
CEVT100Doc *pDoc = GetDocument();
bool IsActioned = false;

  switch(nChar){
    case VK_LEFT:
      pDoc->SendHostMessage("\x1b[D");
      IsActioned = true;
      break;
    case VK_RIGHT:
      pDoc->SendHostMessage("\x1b[C");
      IsActioned = true;
      break;
    case VK_UP:
      pDoc->SendHostMessage("\x1b[A");
      IsActioned = true;
      break;
    case VK_DOWN:
      pDoc->SendHostMessage("\x1b[B");
      IsActioned = true;
      break;
    case VK_PRIOR:
      pDoc->SendHostMessage("\x1b[5~");
      IsActioned = true;
      break;
    case VK_NEXT:
      pDoc->SendHostMessage("\x1b[6~");
      IsActioned = true;
      break;
    case VK_HOME:
      pDoc->SendHostMessage("\x1b[H");
      IsActioned = true;
      break;
    case VK_END:
      pDoc->SendHostMessage("\x1b[F");
      IsActioned = true;
      break;
    case VK_DELETE:
      pDoc->SendHostMessage("\x1b[~3");
      IsActioned = true;
      break;
  }
  if(!IsActioned) CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnKillFocus(CWnd* pNewWnd) 
{
  CScrollView::OnKillFocus(pNewWnd);
  if (m_bHasCaret){ 	// Destroy text cursor if it is currently displayed
    ::HideCaret(m_hWnd);
    ::DestroyCaret();
    m_bHasCaret = FALSE;
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnSetFocus(CWnd* pOldWnd) 
{
  CEVT100Doc *pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  CScrollView::OnSetFocus(pOldWnd);
  if(pDoc->m_IsConnected){
    ::CreateCaret(m_hWnd, NULL, 1, pDoc->m_CharSize.cy);	// Create and display text cursor
    ::ShowCaret(m_hWnd);
    m_bHasCaret = TRUE;
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnSize(UINT nType, int cx, int cy) 
{
  CScrollView::OnSize(nType, cx, cy);
  SetSizes();		// Reset scrollbars
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CEVT100View::OnCommNotify(WPARAM wParam, LPARAM lParam)
{
int nLength;
MSG msg;

  CEVT100Doc *pDoc = GetDocument();
  if(pDoc == NULL) return NULL;
  if((COM_EVENT & LOWORD(lParam)) == COM_EVENT){
    if(!(pDoc->m_IsConnected)){
      SetEvent(pDoc->m_hPostEvent);
      return NULL;
    }
    switch(pDoc->m_EventType){
      case EV_ERR:
      case EV_BREAK:
        pDoc->FormatScreenData(IDS_COMM_OVF_FRM_ERROR);
      case EV_RXCHAR:
		    do{
			    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) AfxGetApp()->PumpMessage();
				  if(nLength = pDoc->ReadCommBlock()){
					  pDoc->ProcessHostData(nLength, (LPSTR)pDoc->m_InBlock);
	        }
		    }
				while (nLength > 0);
		    SetEvent(pDoc->m_hPostEvent);	// Notify secondary thread the WM_COMMNOTIFY has been processed
			  SetEvent(pDoc->m_hPostEvent);
        break;
    }
  }
  UpdateWindow();
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::OnDraw(CDC* pDC)
{
CEVT100Doc *pDoc = GetDocument();
CPoint ScrollPos;
CRect rect;
int nRow, nEndRow, nVertPos, nHorzPos;
UINT Attr = ATTR_DEFAULT, SegStart, SegEnd;
int SegLength;
TEXTMETRIC tm;

  ASSERT_VALID(pDoc);
  rect = ((CPaintDC *)pDC)->m_ps.rcPaint;		                                    // Rectangle to be painted
	CBrush backBrush(RGB(0, 0, 0));
  CFont *pOldFont = pDC->SelectObject(m_pFont);
  pDC->GetTextMetrics(&tm);
  m_CharWidth=tm.tmAveCharWidth;
  m_CharHeight=tm.tmHeight;
  ScrollPos = GetScrollPosition();
  rect.top += ScrollPos.y;
  rect.bottom += ScrollPos.y;
  nRow = min(MAXROW - 1, max(0, rect.top / m_CharHeight));
  nEndRow = min(MAXROW - 1, (rect.bottom - 1) / m_CharHeight);
  if(m_bHasCaret) ::HideCaret(m_hWnd);
  pDC->SetTextColor(RGBFromAnsi256(2));                                           // default to green text
  pDC->SetBkColor(RGBFromAnsi256(0));
  pDC->SetBkMode(OPAQUE);
  for (; nRow <= nEndRow; nRow++){
    int nLine = (nRow + pDoc->m_nTopRow) % MAXROW;
    nVertPos = nRow * m_CharHeight;
    rect.top = nVertPos;
    rect.bottom = nVertPos + m_CharHeight;
    nHorzPos = SCRN_MARGIN_X;
    UINT StrLength = (int)strlen(pDoc->m_Screen[nLine].m_Str);
    int AttrCount = pDoc->m_Screen[nLine].GetCount();
    if(AttrCount){                                                                // line contains attribute changes
      int i = 0;
      pDoc->m_Screen[nLine].GetAttr(i++, &Attr, &SegStart);                       // get the first attribute
      do{
        pDC->SetTextColor(RGBFromAnsi256((Attr >> ATTR_TEXT_SHIFT) & 0xFF));
        pDC->SetBkColor(RGBFromAnsi256((Attr >> ATTR_BACK_SHIFT) & 0xFF));
        if(i < AttrCount) pDoc->m_Screen[nLine].GetAttr(i++, &Attr, &SegEnd);     // get the next attribute if any
        else SegEnd = StrLength;                                                  // else rest of line with current attribute
        SegLength = SegEnd - SegStart;
        pDC->TextOut(nHorzPos, nVertPos, (LPCSTR)&pDoc->m_Screen[nLine].m_Str[SegStart], SegLength);
        nHorzPos += SegLength * m_CharWidth;
        SegStart =  SegEnd;
      }
      while(SegEnd < StrLength);
    }
    else{                                                                         // no attribute change in line
      pDC->TextOut(nHorzPos, nVertPos, (LPCSTR)pDoc->m_Screen[nLine].m_Str, StrLength);
    }
  }
  if(pOldFont != NULL) pDC->SelectObject(pOldFont);
  if(m_bHasCaret) ::ShowCaret(m_hWnd);
  ScrollToCursor();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::ScrollToCursor(bool CheckScroll /* = false*/)
{
POINT scrollPos;
POINT newPos;
POINT toPos;
bool DoScroll = false;;
RECT clientRect;

  CEVT100Doc *pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  scrollPos = GetScrollPosition();  	// Calculate cursor position on view
  newPos.x = 5 + pDoc->m_CursorPos.x * pDoc->m_CharSize.cx - scrollPos.x;
  newPos.y = ((pDoc->m_CursorPos.y + MAXROW - pDoc->m_nTopRow) % MAXROW) * pDoc->m_CharSize.cy - scrollPos.y;
  if(m_bHasCaret)	SetCaretPos(newPos);
  if(CheckScroll){	// If bScroll is TRUE, scroll view to show cursor
    toPos.x = scrollPos.x;
    toPos.y = scrollPos.y;
    GetClientRect(&clientRect);
    if(newPos.x < 0){
      toPos.x += newPos.x;
      DoScroll = true;
    }
    if(newPos.y < 0){
      toPos.y += newPos.y;
      DoScroll = true;
    }
    if(newPos.x + pDoc->m_CharSize.cx > clientRect.right){
      toPos.x += newPos.x + pDoc->m_CharSize.cx - clientRect.right;
      DoScroll = true;
    }
    if(newPos.y + pDoc->m_CharSize.cy > clientRect.bottom){
      toPos.y += newPos.y + pDoc->m_CharSize.cy - clientRect.bottom;
      DoScroll = true;
    }
    if(pDoc->m_Scrolled > 0){
      toPos.y += pDoc->m_Scrolled * pDoc->m_CharSize.cy;
      if (m_nMapMode != MM_TEXT){
				CWindowDC dc(NULL);
				dc.SetMapMode(m_nMapMode);
				dc.LPtoDP((LPPOINT)&toPos);
      }
      ScrollToDevicePosition(toPos);
      pDoc->m_Scrolled = 0;
    }
    else if(DoScroll) ScrollToPosition(toPos);
    Invalidate();
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::SetFont(LOGFONT *lf)
{
  if (m_pFont){
    delete m_pFont;
    m_pFont = NULL;
  }
  m_pFont = new CFont;
  m_pFont->CreateFontIndirect(lf);
  SetSizes();
  Invalidate();
  ScrollToCursor(TRUE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100View::SetSizes()
{
SIZE sizeTotal;
SIZE sizePage;
SIZE sizeLine;
RECT clientRect;

  CEVT100Doc *pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  	// Calculate scrollbar sizes
  GetClientRect(&clientRect);
  sizeTotal.cx = pDoc->m_CharSize.cx * MAXCOL;
  sizeTotal.cy = pDoc->m_CharSize.cy * ((pDoc->m_CursorPos.y + MAXROW - pDoc->m_nTopRow) % MAXROW + 1);
  sizePage.cx = clientRect.right - clientRect.right % pDoc->m_CharSize.cx;
  sizePage.cy = clientRect.bottom - clientRect.bottom % pDoc->m_CharSize.cy;
  sizeLine.cx = pDoc->m_CharSize.cx;
  sizeLine.cy = pDoc->m_CharSize.cy;
  SetScrollSizes(MM_TEXT, sizeTotal, sizePage, sizeLine);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100View::OnEraseBkgnd(CDC* pDC) 
{
  CBrush backBrush(RGB(0,0,0));
  CBrush* poldBrush = pDC->SelectObject(&backBrush);
  CRect rect;
  pDC->GetClipBox(&rect);
  pDC->PatBlt(rect.left,rect.top,rect.Width(),rect.Height(),PATCOPY);
  pDC->SelectObject(poldBrush);
  return TRUE;
}

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
#include "EVT100ScrollBar.h"
#include "XMemDC.h"
#include "EVT100VisualManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEVT100ScrollBar, CScrollBar)

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEVT100ScrollBar, CScrollBar)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETFOCUS()
	ON_MESSAGE(SBM_SETSCROLLINFO, OnSetScrollInfo)
	ON_MESSAGE(SBM_GETSCROLLINFO, OnGetScrollInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CEVT100ScrollBar::CEVT100ScrollBar()
{
	m_bPreSubclassInit = TRUE;
	ZeroMemory(&m_spi, sizeof(m_spi));
	m_pSBTrack = NULL;
	m_crBack = RGB(30, 30, 30);
}

/////////////////////////////////////////////////////////////////////////////

CEVT100ScrollBar::~CEVT100ScrollBar()
{
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::Init()
{
	m_spi.fVert = GetStyle() & SBS_VERT;
	RefreshMetrics();
	Invalidate(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::PreSubclassWindow()
{
	CScrollBar::PreSubclassWindow();
	if(m_bPreSubclassInit){
		Init();
	}
}

/////////////////////////////////////////////////////////////////////////////

int CEVT100ScrollBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CScrollBar::OnCreate(lpCreateStruct) == -1) return -1;
	Init();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100ScrollBar::PreCreateWindow(CREATESTRUCT &cs)
{
	// When creating controls dynamically Init() must	be called from OnCreate() and not from PreSubclassWindow().
	if(!CScrollBar::PreCreateWindow(cs)) return FALSE;
	m_bPreSubclassInit = FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::SetUseVisualStyle(BOOL bUseVisualStyle/* = TRUE*/)
{
	m_bUseVisualStyle = bUseVisualStyle;
	RefreshMetrics();
	RedrawScrollBar();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::RefreshMetrics()
{
//	RefreshXtremeColors();
	m_cxHScroll = GetSystemMetrics(SM_CXHSCROLL);
	m_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	m_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	m_cyVScroll = GetSystemMetrics(SM_CYVSCROLL);
	m_crBack = CLR_BAR_BACKGROUND;
	m_crBackHilite = CLR_MENU_HIGHLIGHT;
	m_crBorderHilite = CLR_MENU_HIGHLIGHT;
	m_crBorder = CLR_BAR_BACKGROUND;
	m_crBackPushed = CLR_BAR_BACKGROUND;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::OnSetFocus(CWnd * /*pOldWnd*/)
{
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::CalcScrollBarInfo(LPRECT lprc, SCROLLBARPOSINFO *pSBInfo, SCROLLINFO *pSI)
{
int cpx;
DWORD dwRange;
int denom;
BOOL fVert = pSBInfo->fVert;

	pSBInfo->rc = *lprc;
	pSBInfo->pxPopup = 0;
	if(fVert){
		pSBInfo->pxTop = lprc->top;
		pSBInfo->pxBottom = lprc->bottom;
		pSBInfo->pxLeft = lprc->left;
		pSBInfo->pxRight = lprc->right;
		pSBInfo->cpxThumb = m_cyVScroll;
	}
	else{
		pSBInfo->pxTop = lprc->left;
		pSBInfo->pxBottom = lprc->right;
		pSBInfo->pxLeft = lprc->top;
		pSBInfo->pxRight = lprc->bottom;
		pSBInfo->cpxThumb = m_cxHScroll;
	}
	pSBInfo->pos = pSI->nPos;
	pSBInfo->page = pSI->nPage;
	pSBInfo->posMin = pSI->nMin;
	pSBInfo->posMax = pSI->nMax;
	dwRange = ((DWORD)(pSBInfo->posMax - pSBInfo->posMin)) + 1;
	cpx = min((pSBInfo->pxBottom - pSBInfo->pxTop) / 2, pSBInfo->cpxThumb);
	pSBInfo->pxUpArrow = pSBInfo->pxTop + cpx;
	pSBInfo->pxDownArrow = pSBInfo->pxBottom - cpx;
	if((pSBInfo->page != 0) && (dwRange != 0)){
		int i = MulDiv(pSBInfo->pxDownArrow - pSBInfo->pxUpArrow,	pSBInfo->page, dwRange);
		pSBInfo->cpxThumb = max(pSBInfo->cpxThumb / 2, i);
	}
	pSBInfo->pxMin = pSBInfo->pxTop + cpx;
	pSBInfo->cpx = pSBInfo->pxBottom - cpx - pSBInfo->cpxThumb - pSBInfo->pxMin;
	denom = dwRange - (pSBInfo->page ? pSBInfo->page : 1);
	if(denom)	pSBInfo->pxThumbTop = MulDiv(pSBInfo->pos - pSBInfo->posMin, pSBInfo->cpx, denom) + pSBInfo->pxMin;
	else pSBInfo->pxThumbTop = pSBInfo->pxMin - 1;
	pSBInfo->pxThumbBottom = pSBInfo->pxThumbTop + pSBInfo->cpxThumb;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::SetupScrollInfo()
{
SCROLLINFO si;

	CRect rc = GetScrollBarRect();
	GetScrollInfo(&si);
	CalcScrollBarInfo(&rc, &m_spi, &si);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CEVT100ScrollBar::OnSetScrollInfo(WPARAM wParam, LPARAM lParam)
{
	LRESULT bResult = DefWindowProc(SBM_SETSCROLLINFO, FALSE, lParam);
	if(wParam){
		Invalidate(FALSE);
		UpdateWindow();
	}
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CEVT100ScrollBar::OnGetScrollInfo(WPARAM wParam, LPARAM lParam)
{
	LRESULT bResult = DefWindowProc(SBM_GETSCROLLINFO, wParam, lParam);
	LPSCROLLINFO lpsi = (LPSCROLLINFO)lParam;
	if(lpsi && (lpsi->fMask & SIF_TRACKPOS) && m_pSBTrack){
		lpsi->nTrackPos = m_pSBTrack->posNew;
	}
	return bResult;
}


/////////////////////////////////////////////////////////////////////////////

int CEVT100ScrollBar::HitTestScrollBar(POINT pt)
{
int px = m_spi.fVert ? pt.y : pt.x;

	if(!::PtInRect(&m_spi.rc, pt)) return HTNOWHERE;
	if(px < m_spi.pxUpArrow) return EVT_HTSCROLLUP;
	if(m_spi.pxPopup > 0 && px > m_spi.pxPopup) return EVT_HTSCROLLPOPUP;
	if(px >= m_spi.pxDownArrow) return EVT_HTSCROLLDOWN;
	if(px < m_spi.pxThumbTop) return EVT_HTSCROLLUPPAGE;
	if(px < m_spi.pxThumbBottom) return EVT_HTSCROLLTHUMB;
	if(px < m_spi.pxDownArrow) return EVT_HTSCROLLDOWNPAGE;
	return HTERROR;
}

/////////////////////////////////////////////////////////////////////////////

CRect CEVT100ScrollBar::GetScrollBarRect()
{
CRect BarRect;

	if(::IsWindow(GetSafeHwnd())) ::GetClientRect(GetSafeHwnd(), BarRect);
	else BarRect.SetRectEmpty();
	return BarRect;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::GetScrollInfo(SCROLLINFO *psi)
{
	psi->cbSize = sizeof(SCROLLINFO);
	psi->fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	::GetScrollInfo(m_hWnd, SB_CTL, psi);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100ScrollBar::IsScrollBarEnabled() const
{
	return IsWindowEnabled();
}

/////////////////////////////////////////////////////////////////////////////

CWnd *CEVT100ScrollBar::GetParentWindow() const
{
	return GetParent();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::RedrawScrollBar()
{
	if(m_hWnd) Invalidate(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::OnPaint()
{
CRect BarRect;

  CPaintDC dcPaint(this); // device context for painting
	GetClientRect(BarRect);
	CXMemDC memDC(&dcPaint, &BarRect);
	if(!m_pSBTrack || !m_pSBTrack->bTrackThumb)	SetupScrollInfo();
	DrawScrollBar(memDC);
}

/////////////////////////////////////////////////////////////////////////////

void DrawArrowGlyph(CDC *pDC, CRect rcArrow, BOOL bHorz, BOOL bUpArrow, COLORREF clr)
{
int nHeight = min(rcArrow.Width(), rcArrow.Height());

	if(nHeight < 6) return;
	int x = rcArrow.left + ((rcArrow.Width() - nHeight) / 2) + 2;
	int y = rcArrow.top + ((rcArrow.Height() - nHeight) / 2) + 2;
	nHeight -= 4;
	HFONT hFont = CreateFont(nHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("MARLETT"));
	HFONT hOldFont = (HFONT)SelectObject(pDC->GetSafeHdc(), hFont);
	SetTextColor(pDC->GetSafeHdc(), clr);
	pDC->SetBkMode(TRANSPARENT);
	TextOut(pDC->GetSafeHdc(), x, y, bHorz ? (bUpArrow ? _T("3") : _T("4")) : (bUpArrow ? _T("5") : _T("6")), 1);
	SelectObject(pDC->GetSafeHdc(), hOldFont);
	DeleteObject(hFont);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::DrawScrollBar(CDC *pDC)
{
CEVT100ScrollBar::SCROLLBARTRACKINFO *pSBTrack = GetScrollBarTrackInfo();
CEVT100ScrollBar::SCROLLBARPOSINFO *pSBInfo = GetScrollBarPosInfo();

	BOOL nPressetHt = pSBTrack ? (pSBTrack->bTrackThumb || pSBTrack->fHitOld ? pSBInfo->ht : -1) : -1;
	BOOL nHotHt = pSBTrack ? -1 : pSBInfo->ht;
	int cWidth = (pSBInfo->pxRight - pSBInfo->pxLeft);
	if(cWidth <= 0)	return;
	BOOL bEnabled = (pSBInfo->posMax - pSBInfo->posMin - pSBInfo->page + 1 > 0) && IsScrollBarEnabled();
	int nBtnTrackSize = pSBInfo->pxThumbBottom - pSBInfo->pxThumbTop;
	int nBtnTrackPos = pSBInfo->pxThumbTop - pSBInfo->pxUpArrow;
	if(!bEnabled || pSBInfo->pxThumbBottom > pSBInfo->pxDownArrow) nBtnTrackPos = nBtnTrackSize = 0;
	if(pSBInfo->fVert){
		CRect rcVScroll(pSBInfo->rc);
		CRect rcArrowUp(rcVScroll.left, rcVScroll.top, rcVScroll.right, pSBInfo->pxUpArrow);
		CRect rcArrowDown(rcVScroll.left, pSBInfo->pxDownArrow, rcVScroll.right, rcVScroll.bottom);
		CRect rcTrack(rcVScroll.left, rcArrowUp.bottom, rcVScroll.right, rcArrowDown.top);
		int BtnTop = rcTrack.top + nBtnTrackPos;
		CRect rcBtnTrack(rcTrack.left + BUTTON_MARGIN, BtnTop, rcTrack.right - BUTTON_MARGIN, BtnTop + nBtnTrackSize);
		pDC->FillSolidRect(rcVScroll, m_crBack);
		pDC->FillSolidRect(rcArrowUp, nHotHt == EVT_HTSCROLLUP ? m_crBackHilite : m_crBack);
		DrawArrowGlyph(pDC, rcArrowUp, FALSE, TRUE, bEnabled ? (nHotHt == EVT_HTSCROLLUP ? RGB(0, 0, 0) : GetSysColor(COLOR_3DSHADOW)) : RGB(0, 0, 0));
		pDC->FillSolidRect(rcArrowDown, nHotHt == EVT_HTSCROLLDOWN ? m_crBackHilite : m_crBack);
		DrawArrowGlyph(pDC, rcArrowDown, FALSE, FALSE, bEnabled ? (nHotHt == EVT_HTSCROLLDOWN ? RGB(0, 0, 0) : GetSysColor(COLOR_3DSHADOW)) : RGB(0, 0, 0));
		pDC->FillSolidRect(rcBtnTrack, m_crBackHilite);
	}
	else{
		CRect rcHScroll(pSBInfo->rc);
		CRect rcArrowLeft(rcHScroll.left, rcHScroll.top, pSBInfo->pxUpArrow, rcHScroll.bottom);
		CRect rcArrowRight(pSBInfo->pxDownArrow, rcHScroll.top, rcHScroll.right, rcHScroll.bottom);
		CRect rcTrack(rcArrowLeft.right, rcHScroll.top, rcArrowRight.left, rcHScroll.bottom);
		int BtnLeft = rcTrack.left + nBtnTrackPos;
//		CRect rcLeftTrack(rcTrack.left, rcTrack.top, BtnRight, rcTrack.bottom);
		CRect rcBtnTrack(BtnLeft, rcTrack.top, BtnLeft + nBtnTrackSize, rcTrack.bottom);
		pDC->FillSolidRect(rcHScroll, m_crBack);
		pDC->FillSolidRect(rcArrowLeft, nHotHt == EVT_HTSCROLLUP ? m_crBackHilite : m_crBack);
		DrawArrowGlyph(pDC, rcArrowLeft, TRUE, TRUE, bEnabled ? (nHotHt == EVT_HTSCROLLUP ? RGB(0, 0, 0) : GetSysColor(COLOR_3DSHADOW)) : RGB(0, 0, 0));
		pDC->FillSolidRect(rcArrowRight, nHotHt == EVT_HTSCROLLDOWN ? m_crBackHilite : m_crBack);
		DrawArrowGlyph(pDC, rcArrowRight, TRUE, FALSE, bEnabled ? (nHotHt == EVT_HTSCROLLDOWN ? RGB(0, 0, 0) : GetSysColor(COLOR_3DSHADOW)) : RGB(0, 0, 0));
		pDC->FillSolidRect(rcBtnTrack, m_crBackHilite);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::OnMouseLeave()
{
	OnMouseMove(0, CPoint(-1, -1));
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	int ht = HitTestScrollBar(point);
	if(ht != m_spi.ht){
		m_spi.ht = ht;
		Invalidate(FALSE);
		if(m_spi.ht != HTNOWHERE){
			TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, HOVER_DEFAULT};
			_TrackMouseEvent(&tme);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	if((GetStyle() & WS_TABSTOP))	SetFocus();
	PerformTrackInit(m_hWnd, point, &m_spi, (GetKeyState(VK_SHIFT) < 0) ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::PerformTrackInit(HWND hWnd, CPoint point, SCROLLBARPOSINFO* pSBInfo, BOOL bDirect)
{
int px = m_spi.fVert ? point.y : point.x;
SCROLLBARTRACKINFO* pSBTrack = new SCROLLBARTRACKINFO;

	memset(pSBTrack, 0, sizeof(SCROLLBARTRACKINFO));
	pSBTrack->cmdSB = (UINT)-1;
	pSBTrack->bTrackThumb = FALSE;
	pSBTrack->pSBInfo = pSBInfo;
	pSBTrack->hWndTrack = hWnd;
	m_pSBTrack = pSBTrack;
	RECT rcSB;
	LPINT pwX = (LPINT)&rcSB;
	LPINT pwY = pwX + 1;
	if(!pSBInfo->fVert)	pwX = pwY--;
	*(pwX + 0) = pSBInfo->pxLeft;
	*(pwY + 0) = pSBInfo->pxTop;
	*(pwX + 2) = pSBInfo->pxRight;
	*(pwY + 2) = pSBInfo->pxBottom;
	if(px < pSBInfo->pxUpArrow){
		pSBInfo->ht = EVT_HTSCROLLUP;
		pSBTrack->cmdSB = SB_LINEUP;
		*(pwY + 2) = pSBInfo->pxUpArrow;
	}
	else if(px >= pSBInfo->pxDownArrow){
		pSBInfo->ht = EVT_HTSCROLLDOWN;
		pSBTrack->cmdSB = SB_LINEDOWN;
		*(pwY + 0) = pSBInfo->pxDownArrow;
	}
	else if(px < pSBInfo->pxThumbTop){
		pSBInfo->ht = EVT_HTSCROLLUPPAGE;
		pSBTrack->cmdSB = SB_PAGEUP;
		*(pwY + 0) = pSBInfo->pxUpArrow;
		*(pwY + 2) = pSBInfo->pxThumbTop;
	}
	else if(px < pSBInfo->pxThumbBottom){
		pSBInfo->ht = EVT_HTSCROLLTHUMB;

DoThumbPos:
		if(pSBInfo->pxDownArrow - pSBInfo->pxUpArrow <= pSBInfo->cpxThumb){
			delete m_pSBTrack;
			m_pSBTrack = NULL;
			return;
		}
		pSBTrack->cmdSB = SB_THUMBPOSITION;
		CalcTrackDragRect(pSBTrack);
		pSBTrack->pxOld = pSBInfo->pxStart = pSBInfo->pxThumbTop;
		pSBTrack->posNew = pSBTrack->posOld = pSBInfo->pos;
		pSBTrack->dpxThumb = pSBInfo->pxStart - px;
		pSBTrack->bTrackThumb = TRUE;
		::SetCapture(hWnd);
		DoScroll(SB_THUMBTRACK, pSBTrack->posOld);
	}
	else if(px < pSBInfo->pxDownArrow){
		pSBInfo->ht = EVT_HTSCROLLDOWNPAGE;
		pSBTrack->cmdSB = SB_PAGEDOWN;
		*(pwY + 0) = pSBInfo->pxThumbBottom;
		*(pwY + 2) = pSBInfo->pxDownArrow;
	}
	if((bDirect && pSBTrack->cmdSB != SB_LINEUP && pSBTrack->cmdSB != SB_LINEDOWN)){
		if(pSBTrack->cmdSB != SB_THUMBPOSITION){
			goto DoThumbPos;
		}
		pSBTrack->dpxThumb = -(pSBInfo->cpxThumb / 2);
	}
	::SetCapture(hWnd);
	if(pSBTrack->cmdSB != SB_THUMBPOSITION){
		CopyRect(&pSBTrack->rcTrack, &rcSB);
	}
	if(!pSBTrack->bTrackThumb){
		TrackBox(WM_LBUTTONDOWN, point);
	}
	else{
		TrackThumb(WM_LBUTTONDOWN, point);

	}
	while(::GetCapture() == hWnd){
		MSG msg;
		if(!::GetMessage(&msg, NULL, 0, 0)){
			AfxPostQuitMessage((int)msg.wParam);
			break;
		}
		if(!IsWindow(hWnd)) break;
		UINT cmd = msg.message;
		if(cmd == WM_TIMER && msg.wParam == IDSYS_SCROLL){
			ContScroll();
		}
		else if (cmd >= WM_MOUSEFIRST && cmd <= WM_MOUSELAST) {
			CPoint ptScreen = msg.pt;
			::ScreenToClient(hWnd, &ptScreen);
			if(!pSBTrack->bTrackThumb){
				TrackBox(cmd, ptScreen);
			}
			else{
				TrackThumb(cmd, ptScreen);
			}
		}
		else{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	if(pSBTrack->hTimerSB != 0){
		::KillTimer(hWnd, pSBTrack->hTimerSB);
	}
	delete m_pSBTrack;
	m_pSBTrack = NULL;
	if (IsWindow(hWnd)) {
		GetCursorPos(&point);
		::ScreenToClient(hWnd, &point);
		m_spi.ht = HitTestScrollBar(point);
		RedrawScrollBar();
	}
}

/////////////////////////////////////////////////////////////////////////////

int CEVT100ScrollBar::SBPosFromPx(CEVT100ScrollBar::SCROLLBARPOSINFO *pSBInfo, int px)
{
	if(px < pSBInfo->pxMin){
		return pSBInfo->posMin;
	}
	if(px >= pSBInfo->pxMin + pSBInfo->cpx){
		return (pSBInfo->posMax - (pSBInfo->page ? pSBInfo->page - 1 : 0));
	}
	if(pSBInfo->cpx) return (pSBInfo->posMin + MulDiv(pSBInfo->posMax - pSBInfo->posMin - (pSBInfo->page ? pSBInfo->page - 1 : 0), px - pSBInfo->pxMin, pSBInfo->cpx));
	else return (pSBInfo->posMin - 1);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::MoveThumb(int px)
{
SCROLLBARTRACKINFO *pSBTrack = m_pSBTrack;

	if((pSBTrack == NULL) || (px == pSBTrack->pxOld))	return;
	SCROLLBARPOSINFO *pSBInfo = m_pSBTrack->pSBInfo;

pxReCalc:
	pSBTrack->posNew = SBPosFromPx(pSBInfo, px);
	if(pSBTrack->posNew != pSBTrack->posOld){
		DoScroll(SB_THUMBTRACK, pSBTrack->posNew);
		pSBTrack->posOld = pSBTrack->posNew;
		if(px >= pSBInfo->pxMin + pSBInfo->cpx){
			px = pSBInfo->pxMin + pSBInfo->cpx;
			goto pxReCalc;
		}
	}
	pSBInfo->pxThumbTop = px;
	pSBInfo->pxThumbBottom = pSBInfo->pxThumbTop + pSBInfo->cpxThumb;
	pSBTrack->pxOld = px;
	RedrawScrollBar();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::TrackThumb(UINT message, CPoint pt)
{
SCROLLBARTRACKINFO *pSBTrack = m_pSBTrack;

	if(!pSBTrack)	return;
	SCROLLBARPOSINFO *pSBInfo = pSBTrack->pSBInfo;
	if(HIBYTE(message) != HIBYTE(WM_MOUSEFIRST)) return;
	if(pSBInfo == NULL)	return;
	int px;
	if(!PtInRect(&pSBTrack->rcTrack, pt))	px = pSBInfo->pxStart;
	else{
		px = (pSBTrack->pSBInfo->fVert ? pt.y : pt.x) + pSBTrack->dpxThumb;
		if(px < pSBInfo->pxMin)	px = pSBInfo->pxMin;
		else if(px >= pSBInfo->pxMin + pSBInfo->cpx) px = pSBInfo->pxMin + pSBInfo->cpx;
	}
	MoveThumb(px);
	if(message == WM_LBUTTONUP || GetKeyState(VK_LBUTTON) >= 0){
		EndScroll(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::TrackBox(UINT message, CPoint point)
{
SCROLLBARTRACKINFO *pSBTrack = m_pSBTrack;

	if(pSBTrack == NULL) return;
	if(message != WM_NULL && HIBYTE(message) != HIBYTE(WM_MOUSEFIRST)) return;
	if((pSBTrack->cmdSB == SB_PAGEUP || pSBTrack->cmdSB == SB_PAGEDOWN)){
		int *pLength = (int *)&pSBTrack->rcTrack;
 		if(pSBTrack->pSBInfo->fVert) pLength++;
		if(pSBTrack->cmdSB == SB_PAGEUP) pLength[2] = pSBTrack->pSBInfo->pxThumbTop;
		else pLength[0] = pSBTrack->pSBInfo->pxThumbBottom;
	}
	BOOL fHit = PtInRect(&pSBTrack->rcTrack, point);
	BOOL fHitChanged = fHit != (BOOL)pSBTrack->fHitOld;
	if(fHitChanged){
		pSBTrack->fHitOld = fHit;
		RedrawScrollBar();
	}
	int cmsTimer = GetDoubleClickTime() / 10;
	switch(message)
		case WM_LBUTTONUP:{
			EndScroll(FALSE);
			break;
		case WM_LBUTTONDOWN:
			pSBTrack->hTimerSB = 0;
			cmsTimer = GetDoubleClickTime() * 4 / 5;
			[[fallthrough]];
		case WM_MOUSEMOVE:
			if(fHit && fHitChanged){
				pSBTrack->hTimerSB = ::SetTimer(m_pSBTrack->hWndTrack, IDSYS_SCROLL, cmsTimer, NULL);
				DoScroll(pSBTrack->cmdSB, 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::ContScroll()
{
SCROLLBARTRACKINFO *pSBTrack = m_pSBTrack;

	ASSERT(pSBTrack);
	if(pSBTrack == NULL)	return;
	CPoint pt;
	GetCursorPos(&pt);
	::ScreenToClient(m_pSBTrack->hWndTrack, &pt);
	TrackBox(WM_NULL, pt);
	if(pSBTrack->fHitOld){
		pSBTrack->hTimerSB = ::SetTimer(m_pSBTrack->hWndTrack, IDSYS_SCROLL, GetDoubleClickTime() / 10, NULL);
		DoScroll(pSBTrack->cmdSB, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::CalcTrackDragRect(SCROLLBARTRACKINFO *pSBTrack) const
{
int     cx;
int     cy;
LPINT   pwX, pwY;
pwX = pwY = (LPINT)&pSBTrack->rcTrack;

	if(pSBTrack->pSBInfo->fVert){
		cy = GetSystemMetrics(SM_CYVTHUMB);
		pwY++;
	}
	else{
		cy = GetSystemMetrics(SM_CXHTHUMB);
		pwX++;
	}
	cx = (pSBTrack->pSBInfo->pxRight - pSBTrack->pSBInfo->pxLeft) * 8;
	cy *= 2;
	*(pwX + 0) = pSBTrack->pSBInfo->pxLeft - cx;
	*(pwY + 0) = pSBTrack->pSBInfo->pxTop - cy;
	*(pwX + 2) = pSBTrack->pSBInfo->pxRight + cx;
	*(pwY + 2) = pSBTrack->pSBInfo->pxBottom + cy;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::DoScroll(int cmd, int pos)
{
	GetParent()->SendMessage((UINT)(m_spi.fVert ? WM_VSCROLL : WM_HSCROLL), MAKELONG(cmd, pos), (LPARAM)m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100ScrollBar::EndScroll(BOOL fCancel)
{
SCROLLBARTRACKINFO *pSBTrack = m_pSBTrack;

	if(pSBTrack){
		pSBTrack->cmdSB = 0;
		ReleaseCapture();
		if(pSBTrack->bTrackThumb){
			if(fCancel){
				pSBTrack->posOld = pSBTrack->pSBInfo->pos;
			}
			DoScroll(SB_THUMBPOSITION, pSBTrack->posOld);
			RedrawScrollBar();
		}
		else{
			if(pSBTrack->hTimerSB != 0){
				::KillTimer(pSBTrack->hWndTrack, pSBTrack->hTimerSB);
				pSBTrack->hTimerSB = 0;
			}
		}
		DoScroll(SB_ENDSCROLL, 0);
	}
}


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
* Edit Date/Ver   Edit Description
* ==============  ===================================================
* SJ   20/10/2024  Original
*
*/

#include "stdafx.h"
#include "XMemDC.h"
#include "EVT100DropDown.h"
#include "EVT100VisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CEVT100DropDown::CEVT100DropDown()
{
	m_bHighlighted = FALSE;
	m_bFocused = FALSE;
	m_bPreSubclassInit = TRUE;
	m_bAutoComp = FALSE;
	m_bDisableAC = FALSE;
	m_crBack = CLR_COMBO_BACKGROUND;
	m_crText = CLR_COMBO_TEXT_NORM;
}

/////////////////////////////////////////////////////////////////////////////

CEVT100DropDown::~CEVT100DropDown()
{
}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEVT100DropDown, CComboBox)

BEGIN_MESSAGE_MAP(CEVT100DropDown, CComboBox)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::RefreshMetrics()
{
	m_clrBorderDisabled =	CLR_COMBO_BORD_GRAY;
	m_clrBorderNormal =	CLR_COMBO_BORD_NORM;
	m_clrBorderHot = CLR_COMBO_BORD_HIGH;
	m_clrButtonNormal = CLR_COMBO_FACE_NORM;
	m_clrButtonHot = CLR_COMBO_FACE_HOT;
	m_clrButtonPressed = CLR_COMBO_FACE_SEL;
	m_clrButtonArrowNormal = CLR_COMBO_BORD_NORM;
	m_clrButtonArrowHot = CLR_COMBO_BORD_HIGH;
	m_clrButtonArrowPressed = CLR_COMBO_TEXT_NORM;
	if(!m_hWnd)	return;
	CClientDC dc(this);
	CFontDC font(&dc, CWnd::GetFont());
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	m_nThumbWidth = max(15L, tm.tmAveCharWidth * 2 + 4);
	m_nThumbWidth = min(m_nThumbWidth, GetSystemMetrics(SM_CXVSCROLL) + 1);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::Init()
{
	RefreshMetrics();
	Invalidate(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::PreSubclassWindow()
{
	CComboBox::PreSubclassWindow();
	if(m_bPreSubclassInit){
		Init();					// Initialize the control.
	}
}

/////////////////////////////////////////////////////////////////////////////

int CEVT100DropDown::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CComboBox::OnCreate(lpCreateStruct) == -1)	return -1;
	Init();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100DropDown::PreCreateWindow(CREATESTRUCT &cs)
{
	if(!CComboBox::PreCreateWindow(cs))	return FALSE;
	m_bPreSubclassInit = FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::OnPaint()
{
	CPaintDC paintDC(this);
	CRect rc;
	GetClientRect(rc);
	CXMemDC dcMem(&paintDC, &rc);
	HBRUSH hBrush = GetClientBrush(&dcMem);
	FillRect(dcMem, rc, hBrush);
	if((GetStyle() & 3) == CBS_SIMPLE){
		CComboBox::DefWindowProc(WM_PAINT, (WPARAM)dcMem.m_hDC, 0);
	}
	else{
		BOOL bPressed = ::SendMessage(m_hWnd, CB_GETDROPPEDSTATE, 0, 0L) != 0;
		BOOL bSelected = m_bFocused || m_bHighlighted;
		BOOL bEnabled = IsWindowEnabled();
		BOOL bRightAlign = GetExStyle() & WS_EX_RIGHT;
		COLORREF clrBorders = bSelected ? m_clrBorderHot : m_clrBorderNormal;
		DefWindowProc(WM_PAINT, (WPARAM)dcMem.GetSafeHdc(), 0);
		dcMem.SelectClipRgn(NULL);
		DrawFrame(dcMem, rc, 3, hBrush);																																											// list box frame
		if(bRightAlign)	FillRect(dcMem, CRect(rc.left, rc.top, rc.left + GetSystemMetrics(SM_CXVSCROLL) + 3, rc.bottom), hBrush);	 // button background
		else FillRect(dcMem, CRect(rc.right - GetSystemMetrics(SM_CXVSCROLL) - 3, rc.top, rc.right, rc.bottom), hBrush);
		CRect rcBtn(rc.right - m_nThumbWidth, rc.top, rc.right, rc.bottom);
		if(bRightAlign)	rcBtn = CRect(rc.left, rc.top, rc.left + m_nThumbWidth, rc.bottom);
		if(!bSelected && !bPressed && bEnabled){
			dcMem.Draw3dRect(rc, m_clrButtonNormal, m_clrButtonNormal);																													// deselected frame
			COLORREF clrButton = m_clrButtonNormal;
			rcBtn.DeflateRect(1, 1);
			dcMem.FillSolidRect(rcBtn, clrButton);																																						  // button 
			DrawFrame(dcMem, rcBtn, 1, hBrush);																																									// button frame
		}
		else{
			dcMem.Draw3dRect(rc, bEnabled ? clrBorders : m_clrBorderDisabled, bEnabled ? clrBorders : m_clrBorderDisabled);		   // selected frame
			COLORREF clrButton = bPressed ? m_clrButtonPressed : bSelected ? m_clrButtonHot : m_clrButtonNormal;
			dcMem.FillSolidRect(rcBtn, clrButton);																																							 // button
			dcMem.Draw3dRect(rcBtn, bEnabled ? clrBorders : m_clrBorderDisabled, bEnabled ? clrBorders : m_clrBorderDisabled);	 // button frame
			rcBtn.DeflateRect(1, 1);
		}
		CPoint pt(rcBtn.CenterPoint());
		int nWidth = max(2, rcBtn.Width() / 2 - 4);
		DrawTriangle(&dcMem, CPoint(pt.x - nWidth, pt.y - nWidth / 2), CPoint(pt.x + nWidth, pt.y - nWidth / 2), CPoint(pt.x, pt.y - nWidth / 2 + nWidth),
			!bEnabled ?m_clrBorderDisabled : bPressed ? m_clrButtonArrowPressed : bSelected ? m_clrButtonArrowHot : m_clrButtonArrowNormal);		 
		if(!bEnabled){
			CPenDC pen(dcMem, m_clrButtonNormal);
			dcMem.MoveTo(rcBtn.left - 1, rcBtn.top);
			dcMem.LineTo(rcBtn.left - 1, rcBtn.bottom);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::DrawTriangle(CDC* pDC, CPoint pt0, CPoint pt1, CPoint pt2, COLORREF clr)
{
	CPenDC pen(*pDC, clr);
	CBrushDC brush(*pDC, clr);
	CPoint pts[] = {pt0, pt1, pt2};
	pDC->Polygon(pts, 3);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::FillSolidRect(HDC hdc, int x, int y, int cx, int cy, HBRUSH hBrush)
{
	::FillRect(hdc, CRect(x, y, x + cx, y + cy), hBrush);
}

/////////////////////////////////////////////////////////////////////////////

void  CEVT100DropDown::DrawFrame(HDC hdc, LPRECT lprc, int nSize, HBRUSH hBrush)
{
	FillSolidRect(hdc, lprc->left, lprc->top, lprc->right - lprc->left - nSize, nSize, hBrush);
	FillSolidRect(hdc, lprc->left, lprc->top, nSize, lprc->bottom - lprc->top - nSize, hBrush);
	FillSolidRect(hdc, lprc->right - nSize, lprc->top, nSize, lprc->bottom - lprc->top, hBrush);
	FillSolidRect(hdc, lprc->left, lprc->bottom - nSize, lprc->right - lprc->left, nSize, hBrush);
}

/////////////////////////////////////////////////////////////////////////////

HBRUSH CEVT100DropDown::GetClientBrush(CDC *pDC)
{
	CWnd *pwndEdit = GetWindow(GW_CHILD);
	BOOL bDisabled = !IsWindowEnabled() || (pwndEdit && pwndEdit->GetStyle() & ES_READONLY);
	HBRUSH hBrush = (HBRUSH)::SendMessage(::GetParent(m_hWnd), bDisabled ? WM_CTLCOLORSTATIC : WM_CTLCOLOREDIT, (WPARAM)pDC->GetSafeHdc(), (LPARAM)m_hWnd);
	if(hBrush) return hBrush;
	return GetSysColorBrush(bDisabled ? COLOR_3DFACE : COLOR_WINDOW);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::OnMouseLeave()
{
	Default();
	SetHighlighted(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::SetHighlighted(BOOL bHot)
{
	if(bHot != m_bHighlighted){
		m_bHighlighted = bHot;
		if(bHot){
			TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, HOVER_DEFAULT};
			_TrackMouseEvent(&tme);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rc;
	GetClientRect(&rc);
	BOOL bHot = rc.PtInRect(point) && !m_bFocused;
	SetHighlighted(bHot);
	CComboBox::OnMouseMove(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::OnKillFocus(CWnd *pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);
	m_bFocused = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::OnSetFocus(CWnd *pOldWnd)
{
	CComboBox::OnSetFocus(pOldWnd);
	m_bFocused = TRUE;
	m_bHighlighted = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100DropDown::OnSysColorChange()
{
	CComboBox::OnSysColorChange();
	RefreshMetrics();
	Invalidate(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100DropDown::PreTranslateMessage(MSG *pMsg)
{
	return CComboBox::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////

HBRUSH CEVT100DropDown::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hBGBrush = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor){
		case CTLCOLOR_LISTBOX :
			pDC->SetBkColor(RGB(200,0,0));
			break;
	}
	pDC->SetTextColor(CLR_COMBO_TEXT_NORM);
	hBGBrush = CreateSolidBrush(CLR_COMBO_BACKGROUND);
	return hBGBrush;
}

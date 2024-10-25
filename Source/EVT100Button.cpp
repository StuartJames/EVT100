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
#include "EVT100Button.h"
#include "EVT100VisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEVT100Btn, CButton)

/////////////////////////////////////////////////////////////////////////////
// CEVT100Btn

CEVT100Btn::CEVT100Btn()
{
	m_bHot = FALSE;
	m_IsPushed = FALSE;
	m_bChecked = FALSE;
	m_nPushButtonStyle = 0;
	m_bShowFocus = FALSE;
	m_nBorderGap = 4;
	m_nBorderStyle = GroupBoxBorder;
	m_crBack = CLR_BUTN_BACKGROUND;
	m_clrFace = CLR_BUTN_FACE_NORM;
	m_clrFaceHighlite = CLR_BUTN_FACE_HIGH;
	m_clrBorder = CLR_BUTN_BORD_NORM;
	m_clrBorderHigh = CLR_BUTN_BORD_HIGH;
	m_clrDisbled = CLR_BUTN_FACE_GRAY;
	m_cxBorder = GetSystemMetrics(SM_CXBORDER);
	m_cyBorder = GetSystemMetrics(SM_CYBORDER);
	m_cxEdge = GetSystemMetrics(SM_CXEDGE);
	m_cyEdge = GetSystemMetrics(SM_CYEDGE);
}

/////////////////////////////////////////////////////////////////////////////

CEVT100Btn::~CEVT100Btn()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEVT100Btn, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(BM_SETSTYLE, OnDefaultAndInvalidate)
	ON_MESSAGE(BM_SETCHECK, OnDefaultAndInvalidate)
	ON_MESSAGE(WM_CAPTURECHANGED, OnDefaultAndInvalidate)
	ON_MESSAGE_VOID(WM_ENABLE, OnInvalidate)
	ON_MESSAGE(BM_SETSTATE, OnSetState)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100Btn::OnEraseBkgnd(CDC * /*pDC*/)
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnMouseMove(UINT nFlags, CPoint point)
{
	if(GetButtonStyle() == BS_GROUPBOX) return;
	CButton::OnMouseMove(nFlags, point);
	CRect rc = GetButtonRect();
	BOOL bHot = rc.PtInRect(point);
	if(bHot != m_bHot){
		m_bHot = bHot;
		RedrawButton();
		if(m_bHot){
			TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, HOVER_DEFAULT};
			_TrackMouseEvent(&tme);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnMouseLeave()
{
	OnMouseMove(0, CPoint(-1, -1));
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnLButtonDown(UINT nFlags, CPoint point)
{
	CButton::OnLButtonDown(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CEVT100Btn::OnSetState(WPARAM wParam, LPARAM /*lParam*/)
{
	BOOL bVisible = GetStyle() & WS_VISIBLE;
	if(bVisible) ModifyStyle(WS_VISIBLE, 0);
	Default();
	if(bVisible) ModifyStyle(0, WS_VISIBLE);
	if(m_IsPushed == 2) return 0;
	m_IsPushed = (wParam != 0);
	RedrawButton();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

CRect CEVT100Btn::GetButtonRect()
{
	CRect rc;
	GetClientRect(rc);
	return rc;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnSetFocus(CWnd *pOldWnd)
{
	CButton::OnSetFocus(pOldWnd);
	RedrawButton(TRUE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnKillFocus(CWnd *pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
	RedrawButton(TRUE);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CEVT100Btn::OnSetText(WPARAM wParam, LPARAM lParam)
{
	BOOL bVisible = GetStyle() & WS_VISIBLE;
	if(bVisible) ModifyStyle(WS_VISIBLE, 0);
	LRESULT lResult = DefWindowProc(WM_SETTEXT, wParam, lParam);
	if(bVisible) ModifyStyle(0, WS_VISIBLE);
	RedrawButton();
	return lResult;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CEVT100Btn::OnDefaultAndInvalidate(WPARAM, LPARAM)
{
	LRESULT lResult = Default();
	RedrawButton(TRUE);
	return lResult;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnInvalidate()
{
	RedrawButton(TRUE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnPaint()
{
	CPaintDC dcPaint(this);
	OnDraw(&dcPaint);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::OnDraw(CDC *pDC)
{
	if((GetButtonStyle() == BS_GROUPBOX)){
		CFont *pOldFont = pDC->SelectObject(CWnd::GetFont());
		pDC->SetBkColor(GetSysColor(COLOR_3DFACE));
		pDC->SetBkMode(TRANSPARENT);
		DrawButton(pDC);
		pDC->SelectObject(pOldFont);
	}
	else{
		CRect BtnRect;
		GetClientRect(BtnRect);
		CXMemDC memDC(pDC, &BtnRect);
		HBRUSH hBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)memDC.GetSafeHdc(), (LPARAM)GetSafeHwnd());
		if(hBrush){
			::FillRect(memDC.GetSafeHdc(), BtnRect, hBrush);
		}
		else{
			memDC.FillSolidRect(BtnRect, CLR_BUTN_BACKGROUND);
		}
		CFont *pOldFont = memDC.SelectObject(CWnd::GetFont());
		memDC.SetBkMode(TRANSPARENT);
		DrawButton(memDC);
		memDC.SelectObject(pOldFont);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawButton(CDC *pDC)
{
	if(IsPushButton()){
		DrawBackground(pDC);
		DrawText(pDC);
		if(::GetFocus() == m_hWnd && m_bShowFocus){
			DrawFocusRect(pDC);
		}
		return;
	}
	switch(GetButtonStyle()){
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			DrawRadioButtonMark(pDC);
			DrawText(pDC);
			break;
		case BS_3STATE:
		case BS_AUTO3STATE:
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
			DrawCheckBoxMark(pDC);
			DrawText(pDC);
			break;
		case BS_GROUPBOX:
			DrawGroupBox(pDC);
			DrawText(pDC);
			break;
		default: TRACE(_T("Warning: unkown style"));
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawBackground(CDC *pDC)
{
const BOOL bPressed = m_IsPushed;
const BOOL bChecked = GetChecked();
const BOOL bSelected = bPressed || bChecked;

	CRect rc(GetButtonRect());
	CBrushDC brush(pDC->GetSafeHdc(), bSelected ? m_clrFaceHighlite : CLR_BUTN_BACKGROUND);
	CPenDC pen(pDC->GetSafeHdc(), bSelected ? m_clrBorderHigh : m_clrBorder);
	pDC->RoundRect(rc, CPoint(10, 10));
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawFocusRect(CDC *pDC)
{
	pDC->SetTextColor(CLR_MENU_TEXT_NORM);
	pDC->SetBkColor(CLR_CAPTION_COLOR);
	CRect rc(GetButtonRect());
	rc.DeflateRect(m_nBorderGap, m_nBorderGap);
	pDC->DrawFocusRect(rc);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawText(CDC *pDC)
{
static const BYTE StyleMap[] = {
	CBR_PUSHBUTTON,
	CBR_PUSHBUTTON,
	CBR_CHECKTEXT,
	CBR_CHECKTEXT,
	CBR_RADIOTEXT,
	CBR_CHECKTEXT,
	CBR_CHECKTEXT,
	CBR_GROUPTEXT,
	CBR_CLIENTRECT,
	CBR_RADIOTEXT,
	CBR_CLIENTRECT,
	CBR_PUSHBUTTON,
};
RECT rc;
int x, y, cx = 0, cy = 0;
CString strText;
UINT dsFlags = DT_SINGLELINE | DT_LEFT | DT_TOP | DT_NOCLIP;

	pDC->SetTextColor(IsWindowEnabled() ? CLR_MENU_TEXT_NORM : CLR_MENU_TEXT_GRAY);
	const BYTE bStyle = GetButtonStyle();
	const DWORD dwStyle = GetStyle();
	const BOOL pbfPush = IsPushButton();
	if(pbfPush)	CalcRect(pDC, &rc, CBR_PUSHBUTTON);
	else CalcRect(pDC, &rc, StyleMap[bStyle]);
	strText = GetButtonText();
	UINT wAlignment = GetTextAlignment();
	if(strText.IsEmpty()) return;
	if(GetStyle() & BS_MULTILINE){
		dsFlags |= DT_WORDBREAK | DT_EDITCONTROL;
		dsFlags &= ~DT_SINGLELINE;
	}
	CRect rcCalc(0, 0, rc.right - rc.left, 0);
	pDC->DrawText(strText, rcCalc, dsFlags | DT_CALCRECT);
	cx = rcCalc.Width();
	cy = rcCalc.Height();
	CRect rcText(rc);
	if(pbfPush && ((GetStyle() & BS_MULTILINE) == 0)) rcText.DeflateRect(m_cxBorder, m_cyBorder);
	switch(wAlignment & BS_HORZMASK){		// Horizontal
		case BS_LEFT: 
			x = rc.left + (pbfPush ? m_cxBorder : 0);
			break;
		case BS_RIGHT:
			x = rc.right - cx - (pbfPush ? m_cxBorder : 0);
			dsFlags |= DT_RIGHT;
			break;
		default:
			x = (rc.left + rc.right - cx) / 2;
			dsFlags |= DT_CENTER;
			break;
	}
	switch(wAlignment & BS_VERTMASK){		// Vertical
		case BS_TOP: 
			y = rc.top + (pbfPush ? m_cyBorder : 0);
			break;
		case BS_BOTTOM:
			y = rc.bottom - cy - (pbfPush ? m_cyBorder : 0);
			dsFlags |= DT_BOTTOM;
			break;
		default:
			y = (rc.top + rc.bottom - cy) / 2;
			dsFlags |= DT_VCENTER;
			break;
	}
	if(GetExStyle() & WS_EX_RTLREADING) dsFlags |= DT_RTLREADING;
	if(dwStyle & BS_MULTILINE) pDC->DrawText(strText, CRect(x, y, x + cx, y + cy), dsFlags | DT_NOCLIP);
	else pDC->DrawText(strText, rcText, dsFlags);
	if(::GetFocus() == m_hWnd && m_bShowFocus){
		if(!pbfPush){
			const RECT rcClient = GetButtonRect();
			if(bStyle == LOBYTE(BS_USERBUTTON)) CopyRect(&rc, &rcClient);
			else{
				rc.top = max(rcClient.top, (long)y - m_cyBorder);
				rc.bottom = min(rcClient.bottom, rc.top + m_cyEdge + cy);
				rc.left = max(rcClient.left, (long)x - m_cxBorder);
				rc.right = min(rcClient.right, rc.left + m_cxEdge + cx);
			}
		}
		else InflateRect(&rc, -m_cxBorder, -m_cyBorder);
		pDC->SetTextColor(CLR_MENU_TEXT_NORM);
		pDC->SetBkColor(CLR_BUTN_BACKGROUND);
		::DrawFocusRect(pDC->GetSafeHdc(), &rc);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawGroupBox(CDC *pDC)
{
CRect rcGroupBox;
CRect rcGroupText;

	CalcRect(pDC, &rcGroupBox, CBR_GROUPFRAME);
	CalcRect(pDC, &rcGroupText, CBR_GROUPTEXT);
	if(GetTextAlignment() & BS_RIGHT) rcGroupText.OffsetRect(+m_cxEdge, 0);
	else rcGroupText.OffsetRect(-m_cxEdge, 0);
	pDC->ExcludeClipRect(&rcGroupText);
	if(m_nBorderStyle == GroupBoxSingleLine){
		pDC->FillSolidRect(rcGroupBox.left, rcGroupBox.top, rcGroupBox.Width(), 1, CLR_BUTN_BORD_NORM);
	}
	else{
		CBrushDC brush(pDC->GetSafeHdc(), CLR_BUTN_BACKGROUND);
		CPenDC pen(pDC->GetSafeHdc(), CLR_BUTN_BORD_NORM);
		pDC->RoundRect(rcGroupBox, CPoint(10, 10));
	}
	pDC->SelectClipRgn(NULL);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawCheckBoxMark(CDC *pDC)
{
CRect rc;
const BOOL bPressed = m_IsPushed;
const BOOL bSelected = IsHighlighted();
const BOOL bEnabled = IsWindowEnabled();
const BOOL bChecked = GetChecked();

	CalcRect(pDC, &rc, CBR_CHECKBOX);
	if(!bEnabled){
		CBrushDC brush(pDC->GetSafeHdc(), m_clrDisbled);
		CPenDC pen(pDC->GetSafeHdc(), m_clrFace);
		pDC->RoundRect(rc, CPoint(3, 3));
	}
	else{
		CBrushDC brush(pDC->GetSafeHdc(), bChecked || bPressed ? m_clrFaceHighlite : m_clrFace);
		CPenDC pen(pDC->GetSafeHdc(), (bChecked || bPressed) ? m_clrFaceHighlite : m_clrFace);
		pDC->RoundRect(rc, CPoint(3, 3));
	}
	if(bChecked){
		const CPoint pt = rc.CenterPoint();
		CPenDC pen(pDC->GetSafeHdc(), (!bEnabled || bChecked == 2) ? CLR_BUTN_TEXT_GRAY : CLR_BUTN_TEXT_NORM);
		for(int i = 0; i < 3; i++){
			pDC->MoveTo(pt.x - 3, pt.y - 1 + i);
			pDC->LineTo(pt.x - 1, pt.y + 1 + i);
			pDC->LineTo(pt.x + 4, pt.y - 4 + i);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::DrawRadioButtonMark(CDC *pDC)
{
const BOOL bPressed = m_IsPushed;
const BOOL bHot = IsHighlighted();
const BOOL bEnabled = IsWindowEnabled();
const BOOL bChecked = GetChecked();
CRect rcRadio;

	CalcRect(pDC, &rcRadio, CBR_RADIOBOX);
	DrawFrameControl(pDC->GetSafeHdc(), rcRadio, DFC_BUTTON, DFCS_BUTTONRADIO | DFCS_FLAT | (bChecked ? DFCS_CHECKED : 0) | (bPressed ? DFCS_PUSHED : 0) | (bEnabled ? 0 : DFCS_INACTIVE));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100Btn::IsPushButton() const
{
DWORD dwStyle = GetStyle();

	switch(GetButtonStyle()){
		case LOBYTE(BS_PUSHBUTTON):
		case LOBYTE(BS_DEFPUSHBUTTON):
		case LOBYTE(BS_OWNERDRAW): return TRUE;
		default:
			if((dwStyle & BS_PUSHLIKE) == BS_PUSHLIKE) return TRUE;
			break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::RefreshMetrics()
{
	if(::IsWindow(m_hWnd)) RedrawButton();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100Btn::GetChecked()
{
	if(IsPushButton() || !m_hWnd)	return m_bChecked;
	return (BOOL)::SendMessage(m_hWnd, BM_GETCHECK, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////

CString CEVT100Btn::GetButtonText()
{
CString str;

	if(m_hWnd) GetWindowText(str);
	return str;
}

/////////////////////////////////////////////////////////////////////////////

int CEVT100Btn::GetTextAlignment() const
{
DWORD bHorz;
DWORD bVert;
DWORD dwStyle = GetStyle();

	bHorz = dwStyle & (BS_LEFT | BS_RIGHT | BS_CENTER);
	bVert = dwStyle & (BS_TOP | BS_BOTTOM | BS_VCENTER);
	if(!bHorz || !bVert){
		if(IsPushButton()){
			if(!bHorz) bHorz = BS_CENTER;
		}
		else{
			if(!bHorz) bHorz = BS_LEFT;
		}
		if(!bVert) bVert = BS_VCENTER;
	}
	return bHorz | bVert;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::CalcRect(CDC *pDC, LPRECT lprc, int code)
{
int dy;
UINT align;
CSize extent;

	*lprc = GetButtonRect();
	align = GetTextAlignment();
	switch(code){
		case CBR_PUSHBUTTON:
			InflateRect(lprc, -m_cxEdge, -m_cyEdge);			// Subtract out raised edge all around
			break;
		case CBR_CHECKBOX:
		case CBR_RADIOBOX:{
				CSize szGlyph(13, 13);
				switch(align & BS_VERTMASK){
					case BS_VCENTER: lprc->top = (lprc->top + lprc->bottom - szGlyph.cy) / 2; break;
					case BS_TOP:
					case BS_BOTTOM:{
						extent = pDC->GetTextExtent(_T(" "), 1);
						dy = extent.cy + extent.cy / 4;
						extent.cx = dy;													// Save vertical extent
						dy = (dy - szGlyph.cy) / 2;							// Get centered amount
						if((align & BS_VERTMASK) == BS_TOP) lprc->top += dy;
						else lprc->top = lprc->bottom - extent.cx + dy;
						break;
					}
				}
				lprc->bottom = lprc->top + szGlyph.cy;
				if(GetStyle() & BS_RIGHTBUTTON) lprc->left = lprc->right - szGlyph.cx;
				else lprc->right = lprc->left + szGlyph.cx;
				break;
			}
		case CBR_CHECKTEXT:
		case CBR_RADIOTEXT:{
				int nOffset = 16;
				if(GetStyle() & BS_RIGHTBUTTON)	lprc->right -= nOffset;
				else lprc->left += nOffset;
				break;
			}
		case CBR_GROUPTEXT:{
				CString strText = GetButtonText();
				StripMnemonics(strText);
				if(strText.IsEmpty()){
					SetRectEmpty(lprc);
					break;
				}
				extent = pDC->GetTextExtent(strText);
				extent.cx += m_cxEdge * 2;
				// Left Align. TODO!
				if(m_nBorderStyle == GroupBoxSingleLine){
					if(GetTextAlignment() & BS_RIGHT)	lprc->left = lprc->right - (int)(extent.cx);
					else lprc->right = lprc->left + (int)(extent.cx);
				}
				else if(GetTextAlignment() & BS_RIGHT){
					lprc->right -= 6 + m_cxBorder;
					lprc->left += lprc->right - (int)(extent.cx);
				}
				else{
					lprc->left += 8 + m_cxBorder;
					lprc->right = lprc->left + (int)(extent.cx);
				}
				lprc->bottom = lprc->top + extent.cy;
				break;
			}
		case CBR_GROUPFRAME:
			extent = pDC->GetTextExtent(_T(" "), 1);
			lprc->top += extent.cy / 2;
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::CalcRects(CDC *pDC, CRect *pButtonText, UINT *pDrawFlags)
{
	CString strText = GetButtonText();
	UINT dsFlags = DT_SINGLELINE | DT_LEFT | DT_TOP;
	if(GetExStyle() & WS_EX_RTLREADING) dsFlags |= DT_RTLREADING;
	if(GetStyle() & BS_MULTILINE){
		dsFlags |= DT_WORDBREAK | DT_EDITCONTROL;
		dsFlags &= ~DT_SINGLELINE;
	}
	CRect rcItem = GetButtonRect();
	rcItem.DeflateRect(m_nBorderGap, m_nBorderGap);
	const int nTextAlign = GetTextAlignment();
	CSize sizeText = CSize(rcItem.Width(), 0);
	if(!strText.IsEmpty()){
		CRect rcTextCalc(0, 0, sizeText.cx, 0);
		pDC->DrawText(strText, rcTextCalc, dsFlags | DT_CALCRECT);
		sizeText = CSize(min(sizeText.cx, (long)rcTextCalc.Width()), rcTextCalc.Height());
	}
	else{
		sizeText = CSize(0, 0);
	}
	CRect rcText(rcItem);
	rcText.left = IsAlignRight(nTextAlign) ? rcText.right - sizeText.cx : IsAlignCenter(nTextAlign) ? (rcText.left + rcText.right - sizeText.cx) / 2 : rcItem.left;
	dsFlags |= IsAlignRight(nTextAlign) ? DT_RIGHT : IsAlignCenter(nTextAlign) ? DT_CENTER : DT_LEFT;
	rcText.top = IsAlignBottom(nTextAlign) ? rcText.bottom - sizeText.cy : IsAlignMiddle(nTextAlign) ? (rcText.top + rcText.bottom - sizeText.cy) / 2 : rcItem.top;
	rcText.right = rcText.left + sizeText.cx;
	rcText.bottom = rcText.top + sizeText.cy;
	if(pButtonText) *pButtonText = rcText;
	if(pDrawFlags) *pDrawFlags = dsFlags;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Btn::StripMnemonics(CString& strClear)
{
	for(int i = 0; i < strClear.GetLength(); i++){
		if(strClear[i] == _T('&')){  // Converts "&&" to "&" and "&&&&" to "&&"
			strClear.Delete(i);
		}
	}
}



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

#pragma once

constexpr auto CLR_COMBO_BACKGROUND			= RGB(40,  40,  40);
constexpr auto CLR_COMBO_FACE_GRAY			= RGB(50, 50, 50);
constexpr auto CLR_COMBO_FACE_NORM			= RGB(80, 80, 80);
constexpr auto CLR_COMBO_FACE_HOT				= RGB(100, 100, 100);
constexpr auto CLR_COMBO_FACE_SEL				= RGB(0, 100, 200);
constexpr auto CLR_COMBO_BORD_GRAY			= RGB(120, 120, 120);
constexpr auto CLR_COMBO_BORD_NORM			= RGB(180, 180, 180);
constexpr auto CLR_COMBO_BORD_HIGH			= RGB(200, 200, 200);
constexpr auto CLR_COMBO_TEXT_NORM			= RGB(230, 230, 230);

///////////////////////////////////////////////////////////////////////////////

class CEVT100DropDown : public CComboBox
{
	DECLARE_DYNAMIC(CEVT100DropDown)

public:
						CEVT100DropDown();
						~CEVT100DropDown();

	void			EnableAutoCompletion(BOOL bEnable = TRUE);
	void			RefreshMetrics();

protected:
	HBRUSH		GetClientBrush(CDC* pDC);
	void			FillSolidRect(HDC hdc, int x, int y, int cx, int cy, HBRUSH hBrush);
	void			DrawFrame(HDC hdc, LPRECT lprc, int nSize, HBRUSH hBrush);
	void			SetHighlighted(BOOL bHot);
	void			DrawTriangle(CDC* pDC, CPoint pt0, CPoint pt1, CPoint pt2, COLORREF clr);



	BOOL			PreCreateWindow(CREATESTRUCT& cs);
	void			PreSubclassWindow();
	void			Init();
	BOOL			PreTranslateMessage(MSG* pMsg);

	void			OnMouseLeave();
	void			OnMouseMove(UINT nFlags, CPoint point);
	void			OnSysColorChange();

	BOOL			m_bPreSubclassInit;
	BOOL			m_bHighlighted;
	BOOL			m_bFocused;
	int				m_nThumbWidth;
	BOOL			m_bAutoComp;
	BOOL			m_bDisableAC;
	COLORREF	m_clrBorderDisabled;
	COLORREF	m_clrBorderHot;
	COLORREF	m_clrBorderNormal;
	COLORREF	m_clrButtonPressed;
	COLORREF	m_clrButtonHot;
	COLORREF	m_clrButtonNormal;
	COLORREF	m_clrButtonArrowPressed;
	COLORREF	m_clrButtonArrowHot;
	COLORREF	m_clrButtonArrowNormal;
	COLORREF	m_crBack;  
	COLORREF	m_crText;  

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

AFX_INLINE void CEVT100DropDown::EnableAutoCompletion(BOOL bEnable/*=TRUE*/) {
	m_bAutoComp = bEnable;
}


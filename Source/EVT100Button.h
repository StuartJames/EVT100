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

#define BS_HORZMASK (BS_LEFT | BS_RIGHT | BS_CENTER)
#define BS_VERTMASK (BS_TOP | BS_BOTTOM | BS_VCENTER)

constexpr auto CLR_BUTN_BACKGROUND		= RGB(40,  40,  40);
constexpr auto CLR_BUTN_FACE_GRAY			= RGB(50, 50, 50);
constexpr auto CLR_BUTN_FACE_NORM			= RGB(80, 80, 80);
constexpr auto CLR_BUTN_FACE_HIGH			= RGB(0, 100, 200);
constexpr auto CLR_BUTN_BORD_NORM			= RGB(120, 120, 120);
constexpr auto CLR_BUTN_BORD_HIGH			= RGB(200, 200, 200);
constexpr auto CLR_BUTN_TEXT_GRAY			= RGB(150, 150, 150);
constexpr auto CLR_BUTN_TEXT_NORM			= RGB(230, 230, 230);
constexpr auto CLR_BUTN_TEXT_HIGH			= RGB(250, 250, 250);

constexpr auto CBR_CLIENTRECT					= 0;
constexpr auto CBR_CHECKBOX						= 1;
constexpr auto CBR_RADIOBOX						= 2;
constexpr auto CBR_CHECKTEXT					= 3;
constexpr auto CBR_RADIOTEXT					= 4;
constexpr auto CBR_GROUPTEXT					= 5;
constexpr auto CBR_GROUPFRAME					= 6;
constexpr auto CBR_PUSHBUTTON					= 7;

enum GroupBoxBorderStyle{
	GroupBoxBorder,        // Complete frame with caption.
	GroupBoxSingleLine,        // Single line with caption.
	GroupBoxNone        // No frame or caption.
};				

class CEVT100Btn : public CButton
{
	DECLARE_DYNAMIC(CEVT100Btn)

public:
									CEVT100Btn();
									~CEVT100Btn();

protected:
	virtual CRect		GetButtonRect();
	virtual void		OnDraw(CDC *pDC);
	virtual void		DrawButton(CDC* pDC);
	BOOL						IsPushButton() const;
	virtual void		RefreshMetrics();
	void						RedrawButton(BOOL bUpdateWindow = FALSE);
	BOOL						IsDropDownStyle() const;
	virtual BOOL		IsHighlighted();
	virtual BOOL		GetChecked();
	CString					GetButtonText();
	int							GetTextAlignment() const;


private:

	UINT		        m_nTypeStyle;		// Button style
	void						CalcRect(CDC *pDC, LPRECT lprc, int code);
	void						CalcRects(CDC *pDC, CRect *pButtonText, UINT *pDrawFlags);
	void						StripMnemonics(CString& strClear);

protected:
	virtual void		DrawBackground(CDC* pDC);
	virtual void		DrawText(CDC* pDC);
	virtual void		DrawFocusRect(CDC *pDC);
	virtual void		DrawGroupBox(CDC *pDC);
	virtual void		DrawCheckBoxMark(CDC *pDC);
	virtual void		DrawRadioButtonMark(CDC *pDC);

	BOOL						m_IsPushed;        // TRUE if button was pressed
	BOOL						m_bHot;        // TRUE if mouse under button
	BOOL						m_bChecked;        // TRUE if checked.
	long						m_nPushButtonStyle;        // The style of the button,
	BOOL						m_bShowFocus;        // TRUE to show the focus rectangle.
	int							m_nBorderGap;
	COLORREF				m_crBack;							// Color of background.
	COLORREF				m_clrBorder;          // button border
	COLORREF				m_clrBorderHigh;			// Border highlight
	COLORREF				m_clrFace;						// button face
	COLORREF				m_clrFaceHighlite;		// button face
	COLORREF				m_clrDisbled;					// Disabled colour

	long						m_nImageAlignment;     // Image Alignment.
	long						m_nTextImageRelation;  // Image and text orientation.
	long						m_nBorderStyle;        // Border style used.
	int							m_cxBorder;
	int							m_cyBorder;
	int							m_cyEdge;
	int							m_cxEdge;

	afx_msg BOOL		OnEraseBkgnd(CDC *pDC);
	afx_msg void		OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void		OnSetFocus(CWnd *pOldWnd);
	afx_msg void		OnKillFocus(CWnd *pNewWnd);
	afx_msg void		OnPaint();
	afx_msg LRESULT OnSetState(WPARAM wParam, LPARAM lParam);
	afx_msg void		OnMouseLeave();
	afx_msg void		OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void		OnInvalidate();
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDefaultAndInvalidate(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

AFX_INLINE void CEVT100Btn::RedrawButton(BOOL bUpdateWindow)
{
	if(m_hWnd) Invalidate(FALSE);
	if(m_hWnd && bUpdateWindow) UpdateWindow();
}

AFX_INLINE BOOL CEVT100Btn::IsHighlighted()
{
	return m_bHot || (::GetCapture() == m_hWnd);
}

AFX_INLINE BOOL IsAlignRight(int nAlign)
{
	return (nAlign & BS_HORZMASK) == BS_RIGHT;
}

AFX_INLINE BOOL IsAlignCenter(int nAlign)
{
	return (nAlign & BS_HORZMASK) == BS_CENTER;
}

AFX_INLINE BOOL IsAlignTop(int nAlign)
{
	return (nAlign & BS_VERTMASK) == BS_TOP;
}

AFX_INLINE BOOL IsAlignBottom(int nAlign)
{
	return (nAlign & BS_VERTMASK) == BS_BOTTOM;
}

AFX_INLINE BOOL IsAlignMiddle(int nAlign)
{
	return (nAlign & BS_VERTMASK) == BS_VCENTER;
}

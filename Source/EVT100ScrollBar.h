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

#define EVT_HTSCROLLUP          60
#define EVT_HTSCROLLDOWN        61
#define EVT_HTSCROLLUPPAGE      62
#define EVT_HTSCROLLDOWNPAGE    63
#define EVT_HTSCROLLTHUMB       64
#define EVT_HTSCROLLPOPUP       65
#define IDSYS_SCROLL						23345
#define BUTTON_MARGIN						3

/////////////////////////////////////////////////////////////////////////////

class CEVT100ScrollBar : public CScrollBar 
{
	DECLARE_DYNAMIC(CEVT100ScrollBar)

											CEVT100ScrollBar();

protected:
	struct SCROLLBARPOSINFO{
		int posMin;        // Minimum position
		int posMax;        // Maximum position
		int page;        // Page size
		int pos;        // Position of thumb

		int pxTop;        // Top bounding rectangle
		int pxBottom;        // Bottom bounding rectangle
		int pxLeft;        // Left bounding rectangle
		int pxRight;        // Right bounding rectangle
		int cpxThumb;        // Size of thumb button
		int pxUpArrow;        // Position of Up arrow
		int pxDownArrow;        // Position of Down arrow
		int pxStart;        // Previous position of thumb button
		int pxThumbBottom;        // Thumb bottom bounding rectangle
		int pxThumbTop;        // Thumb top bounding rectangle
		int pxPopup;        // In-place scroll position
		int cpx;        // position in pixels
		int pxMin;        // Minimum position in pixels

		RECT rc;        // Bounding rectangle
		int ht;        // Hit test
		BOOL fVert;        // TRUE if vertical
	};

	struct SCROLLBARTRACKINFO{
		BOOL fHitOld;        // Previous hittest
		RECT rcTrack;        // Bounding rectangle of available thumb position
		UINT cmdSB;        // Scroll command
		UINT_PTR hTimerSB;        // Timer identifier
		int dpxThumb;        // Delta of thumb button
		int pxOld;        // Previous position in pixels
		int posOld;        // Previous position
		int posNew;        // new position
		HWND hWndTrack;        // Parent tracking window

		BOOL bTrackThumb;        // TRUE if thumb is tracking
		SCROLLBARPOSINFO	*pSBInfo;        // SCROLLBARPOSINFO pointer
	};

	BOOL								m_bUseVisualStyle;        // True to use Visual Styles.
	BOOL								m_bPreSubclassInit;        // TRUE is subclassed.
	SCROLLBARPOSINFO		m_spi;        // ScrollBar position.
	SCROLLBARTRACKINFO	*m_pSBTrack;        // ScrollBar tracking

public:
	virtual							~CEVT100ScrollBar(void);
	void								SetUseVisualStyle(BOOL bUseVisualStyle = TRUE);

	COLORREF						m_crBackPushed;					// Color when pushed.
	COLORREF						m_crBackHilite;					// Color when border.
	COLORREF						m_crBorderHilite;       // Color of border when highlighted.
	COLORREF						m_crBorder;							// Color of border.
	COLORREF						m_crBack;								// Color of background.
	int									m_cxHScroll;						// Width, in pixels, of the arrow bitmap on a horizontal scroll bar
	int									m_cyHScroll;						// Height, in pixels, of a horizontal scroll bar.
	int									m_cyVScroll;						// Height, in pixels, of the arrow bitmap on a vertical scroll bar.
	int									m_cxVScroll;						// Width, in pixels, of a vertical scroll bar;

protected:
	void								Init();
	BOOL								PreCreateWindow(CREATESTRUCT &cs);
	void								PreSubclassWindow();
	void								DrawScrollBar(CDC *pDC);
	CWnd*								GetParentWindow() const;
	CRect								GetScrollBarRect();
	void								GetScrollInfo(SCROLLINFO *psi);
	BOOL								IsScrollBarEnabled() const;
	void								RefreshMetrics();
	void								RedrawScrollBar();
	virtual void				CalcScrollBarInfo(LPRECT lprc, SCROLLBARPOSINFO *pSBInfo, SCROLLINFO *pSI);
	void								CalcTrackDragRect(SCROLLBARTRACKINFO *pSBTrack) const;
	void								SetupScrollInfo();
	int									HitTestScrollBar(POINT pt);
	void								PerformTrackInit(HWND hWnd, CPoint point, SCROLLBARPOSINFO *pSBInfo, BOOL bDirect);
	void								EndScroll(BOOL fCancel);
	void								MoveThumb(int px);
	void								TrackThumb(UINT message, CPoint pt);
	void								TrackBox(UINT message, CPoint point);
	void								DoScroll(int cmd, int pos);
	void								ContScroll();
	int									SBPosFromPx(SCROLLBARPOSINFO *pSBInfo, int px);
	SCROLLBARTRACKINFO	*GetScrollBarTrackInfo();
	SCROLLBARPOSINFO		*GetScrollBarPosInfo();

	afx_msg int					OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void				OnPaint();
	afx_msg void				OnMouseLeave();
	afx_msg void				OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void				OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void				OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void				OnSetFocus(CWnd *pOldWnd);
	afx_msg LRESULT			OnSetScrollInfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT			OnGetScrollInfo(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

AFX_INLINE CEVT100ScrollBar::SCROLLBARTRACKINFO *CEVT100ScrollBar::GetScrollBarTrackInfo()
{
	return m_pSBTrack;
}

AFX_INLINE CEVT100ScrollBar::SCROLLBARPOSINFO *CEVT100ScrollBar::GetScrollBarPosInfo()
{
	return &m_spi;
}

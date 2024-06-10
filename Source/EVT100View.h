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

//constexpr auto WM_COMMNOTIFY		= 0x0444;			
constexpr auto COM_EVENT				= 0x04;
constexpr auto INBLOCKSIZE			= 1024;
constexpr auto SCRN_MARGIN_X		= 5;
constexpr auto SCRN_MARGIN_Y		= 5;

class CEVT100View : public CScrollView
{
protected: // create from serialization only
	CEVT100View();
	DECLARE_DYNCREATE(CEVT100View)

protected:
	bool		m_CaretVisible;	// Set when text cursor is displayed
	CFont*	m_pFont;
	CSize		m_CharSize;
  BYTE*   m_InBlock;
	POINT		m_CaretPos;
	bool		m_BlinkText;
	bool		m_Pause;
	int			m_WrapRows;
	int			m_MaxCol;

public:
	CEVT100Doc* GetDocument();

public:
	void ScrollToCursor(bool CheckScroll = false);
	void SetFont(CDC* pDC, CFont *pOldFont, UINT Attr);
	void SetFont(LOGFONT *lf);
	void SetSizes();
	int  ExTextOut(CDC* pDC, int *pHorz, int *pVert, int Width, LPCSTR Str, int Length, bool ViewWrap);

	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();

public:
	virtual ~CEVT100View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

protected:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnCommNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void	OnPause();
	afx_msg void	OnUpdatePause(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TTYview.cpp
inline CEVT100Doc* CEVT100View::GetDocument()
   { return (CEVT100Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

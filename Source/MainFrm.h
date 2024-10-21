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

#include "EVT100ScrollBar.h"

class CEVT100Doc;

/////////////////////////////////////////////////////////////////////////////

class CMainFrame : public CFrameWndEx
{
protected: 

	DECLARE_DYNCREATE(CMainFrame)

public:
													CMainFrame();
	virtual									~CMainFrame();

  CEVT100Doc*							GetDocument();
	void										Initialize();
	virtual BOOL						PreTranslateMessage(MSG* pMsg);
  void                    SetWindowTitle(CString Title);

#ifdef _DEBUG
	virtual void						AssertValid() const;
	virtual void						Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL						PreCreateWindow(CREATESTRUCT& cs);

	CMFCMenuBar							m_wndMenuBar;
	CMFCToolBar							m_wndToolBar;
	CMFCStatusBar						m_wndStatusBar;
	CEVT100ScrollBar				m_wndHScroll;
	CEVT100ScrollBar				m_wndVScroll;
	CBrush									m_BkGndBrush;

protected:
	afx_msg int							OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void						OnClose();
	afx_msg void						OnTimer(UINT_PTR nIDEvent);
	afx_msg void						OnAppAbout();
	afx_msg void						OnConnect();
	afx_msg void						OnUpdateConnect(CCmdUI* pCmdUI);
	afx_msg LRESULT					OnCtlColorScrollBar(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	virtual void RecalcLayout(BOOL bNotify = TRUE);
};

/////////////////////////////////////////////////////////////////////////////

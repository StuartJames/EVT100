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

class CEVT100Doc;

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only

	DECLARE_DYNCREATE(CMainFrame)

public:
													CMainFrame();

  CEVT100Doc*								GetDocument();
	void										Initialize();
	virtual BOOL						PreTranslateMessage(MSG* pMsg);
  void                    SetWindowTitle(CString Title);

protected:
	virtual BOOL						PreCreateWindow(CREATESTRUCT& cs);

public:
	virtual									~CMainFrame();
#ifdef _DEBUG
	virtual void						AssertValid() const;
	virtual void						Dump(CDumpContext& dc) const;
#endif
	CStatusBar							m_wndStatusBar;

protected:  // control bar embedded members
	CToolBar								m_wndToolBar;

protected:
	afx_msg int							OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void						OnClose();
	afx_msg void						OnTimer(UINT_PTR nIDEvent);
	afx_msg void						OnConnect();
	afx_msg void						OnUpdateConnect(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

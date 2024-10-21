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

#include <afxdrawmanager.h>
#include <afxshowallbutton.h>

// Internal Colours
constexpr auto CLR_RED_TEST						= RGB(240, 0,  0);
constexpr auto CLR_CAPTION_COLOR			= RGB(30, 30, 30);
constexpr auto CLR_BOARDER_COLOR			= RGB(0, 80, 0);
constexpr auto CLR_BAR_BACKGROUND			= RGB(40,  40,  40);
constexpr auto CLR_MENU_HIGHLIGHT			= RGB(110, 110, 110);
constexpr auto CLR_BAR_ITEM_FRAME			= RGB(80,  80,  80);   // highligts around toolbar items
constexpr auto CLR_MENU_TEXT_GRAY			= RGB(150, 150, 150);
constexpr auto CLR_MENU_TEXT_NORM			= RGB(230, 230, 230);
constexpr auto CLR_MENU_TEXT_HIGH			= RGB(250, 250, 250);

class CEVT100Doc;

/////////////////////////////////////////////////////////////////////////////

class CVT100VisualManager : public CMFCVisualManagerOfficeXP
{
protected: 
	DECLARE_DYNCREATE(CVT100VisualManager)

	COLORREF							m_clrGrayedText;
	COLORREF							m_clrTextNormal;
	COLORREF							m_clrTextHilite;

public:
	CVT100VisualManager();
	virtual								~CVT100VisualManager();
	virtual void					OnUpdateSystemColors();	
	virtual void					OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea = FALSE);	
	virtual void					OnDrawPaneBorder(CDC* pDC, CBasePane* pBar, CRect& rect);
	virtual void					OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect);
	virtual void					OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bIsHoriz);
	virtual COLORREF			OnDrawMenuLabel(CDC* pDC, CRect rect);
	virtual void					OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle);
	virtual COLORREF			GetStatusBarPaneTextColor(CMFCStatusBar* pStatusBar, CMFCStatusBarPaneInfo* pPane);
	virtual COLORREF			GetHighlightedMenuItemTextColor(CMFCToolBarMenuButton* pButton);
	virtual COLORREF			GetMenuItemTextColor(CMFCToolBarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);
	virtual COLORREF			GetToolbarButtonTextColor(CMFCToolBarButton* pButton, CMFCVisualManager::AFX_BUTTON_STATE state);
	virtual BOOL					OnNcActivate(CWnd* pWnd, BOOL bActive);
	virtual BOOL					OnNcPaint(CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw);
	virtual void					OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state);
	virtual void					OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited);

protected:

};


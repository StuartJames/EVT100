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
#include "EVT100VisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CVT100VisualManager, CMFCVisualManagerOfficeXP)

/////////////////////////////////////////////////////////////////////////////

CVT100VisualManager::CVT100VisualManager()
{
	m_bMenuFlatLook = TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CVT100VisualManager::~CVT100VisualManager()
{
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnUpdateSystemColors()
{
	CMFCVisualManagerOfficeXP::OnUpdateSystemColors();
	m_clrBarBkgnd = CLR_BAR_BACKGROUND;
	m_clrMenuLight = CLR_MENU_HIGHLIGHT;
	m_clrHighlight = CLR_BAR_ITEM_FRAME;
	m_clrMenuBorder = CLR_BAR_BACKGROUND;          
	m_clrMenuItemBorder = CLR_MENU_HIGHLIGHT;
	m_clrGrayedText = CLR_MENU_TEXT_GRAY;
	m_clrTextNormal = CLR_MENU_TEXT_NORM;
	m_clrTextHilite = CLR_MENU_TEXT_HIGH;
	m_brBarBkgnd.DeleteObject();
	m_brBarBkgnd.CreateSolidBrush(m_clrBarBkgnd);
	m_brMenuLight.DeleteObject();
	m_brMenuLight.CreateSolidBrush(m_clrMenuLight);
	m_brHighlight.DeleteObject();
	m_brHighlight.CreateSolidBrush(m_clrHighlight);
	GetGlobalData()->clrGrayedText = CLR_MENU_TEXT_GRAY;
	GetGlobalData()->clrBtnText = CLR_MENU_TEXT_NORM;
	GetGlobalData()->clrBarShadow = CLR_BAR_BACKGROUND;
	GetGlobalData()->clrBarDkShadow = CLR_BAR_BACKGROUND;
	GetGlobalData()->clrBarLight = CLR_BAR_BACKGROUND;
	GetGlobalData()->clrBarFace	= CLR_BAR_BACKGROUND;
	GetGlobalData()->brBarFace.DeleteObject();
	GetGlobalData()->brBarFace.CreateSolidBrush(GetGlobalData()->clrBarFace);
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea)
{
	CRuntimeClass* pBarClass = pBar->GetRuntimeClass();
	if(pBarClass == NULL || pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCMenuBar))){
		pDC->FillRect(rectClip.IsRectEmpty() ? rectClient : rectClip, &m_brBarBkgnd);
		return;
	}
	else if(pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCToolBar))){
		pDC->FillRect(rectClip.IsRectEmpty() ? rectClient : rectClip, &m_brBarBkgnd);
		return;
	}
	else if(pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCStatusBar))){
		pDC->FillRect(rectClip.IsRectEmpty() ? rectClient : rectClip, &m_brBarBkgnd);
		return;
	}
	else CMFCVisualManagerOfficeXP::OnFillBarBackground(pDC, pBar, rectClient, rectClip, bNCArea);
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnDrawPaneBorder(CDC* pDC, CBasePane* pBar, CRect& rect)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if(!pBar->IsDialogControl()) {
		return;
		DWORD dwStyle = pBar->GetPaneStyle();
		if (!(dwStyle & CBRS_BORDER_ANY)) return;
		if (pBar->IsFloating()) {
			return;
		}
		COLORREF clrBckOld = pDC->GetBkColor(); // FillSolidRect changes it
		if (dwStyle & CBRS_BORDER_LEFT)	pDC->FillSolidRect(0, 0, 1, rect.Height() - 1, m_clrHighlight);
		if (dwStyle & CBRS_BORDER_TOP)	pDC->FillSolidRect(0, 0, rect.Width() - 1, 1, m_clrHighlight);
		if (dwStyle & CBRS_BORDER_RIGHT) pDC->FillSolidRect(rect.right, 0/*RGL~:1*/, -1, rect.Height()/*RGL-: - 1*/, m_clrHighlight);
		if (dwStyle & CBRS_BORDER_BOTTOM)	pDC->FillSolidRect(0, rect.bottom, rect.Width() - 1, -1, m_clrHighlight);
		// if undockable toolbar at top of frame, apply special formatting to mesh properly with frame menu
		if (!pBar->CanFloat()) {
			pDC->FillSolidRect(0, 0, rect.Width(), 1, m_clrHighlight);
			pDC->FillSolidRect(0, 1, rect.Width(), 1, m_clrHighlight);
		}
		if (dwStyle & CBRS_BORDER_LEFT) ++rect.left;
		if (dwStyle & CBRS_BORDER_TOP) ++rect.top;
		if (dwStyle & CBRS_BORDER_RIGHT) --rect.right;
		if (dwStyle & CBRS_BORDER_BOTTOM) --rect.bottom;
		pDC->SetBkColor(clrBckOld);	// Restore Bk color:		*/
	}
	else CMFCVisualManagerOfficeXP::OnDrawPaneBorder(pDC, pBar, rect); 
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bIsHoriz)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (!pBar->IsDialogControl()) {
		CRect rectSeparator = rect;
		if (bIsHoriz) {
			rectSeparator.left += rectSeparator.Width() / 2 - 1;
			rectSeparator.right = rectSeparator.left + 2;
		}
		else {
			rectSeparator.top += rectSeparator.Height() / 2 - 1;
			rectSeparator.bottom = rectSeparator.top + 2;
		}
		pDC->Draw3dRect(rectSeparator, m_clrMenuLight, m_clrMenuLight);
	}
	else CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bIsHoriz); 
}

/////////////////////////////////////////////////////////////////////////////

COLORREF CVT100VisualManager::OnDrawMenuLabel(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brMenuLight);
	CRect rectSeparator = rect;
	rectSeparator.top = rectSeparator.bottom - 2;
	pDC->Draw3dRect(rectSeparator, m_clrMenuBorder, m_clrMenuBorder);
	return GetGlobalData()->clrBtnText;
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnDrawStatusBarPaneBorder(CDC *pDC, CMFCStatusBar * /*pBar*/, CRect rectPane, UINT /*uiID*/, UINT nStyle)
{
  ASSERT_VALID(pDC);
  ASSERT_VALID(this);
 // Do nothing
}

/////////////////////////////////////////////////////////////////////////////

COLORREF CVT100VisualManager::GetStatusBarPaneTextColor(CMFCStatusBar* /*pStatusBar*/, CMFCStatusBarPaneInfo* pPane)
{
	ENSURE(pPane != NULL);
	return(pPane->nStyle & SBPS_DISABLED) ? GetGlobalData()->clrGrayedText : GetGlobalData()->clrBtnText;
}

/////////////////////////////////////////////////////////////////////////////

COLORREF CVT100VisualManager::GetHighlightedMenuItemTextColor(CMFCToolBarMenuButton *pButton)
{
  ASSERT_VALID(pButton);

  if(pButton->m_nStyle & TBBS_DISABLED) return m_clrGrayedText;
  return m_clrTextHilite;
}

/////////////////////////////////////////////////////////////////////////////

COLORREF CVT100VisualManager::GetMenuItemTextColor(CMFCToolBarMenuButton * /*pButton*/, BOOL bHighlighted, BOOL bDisabled)
{
  if(bHighlighted) return bDisabled ? GetGlobalData()->clrBtnFace : GetGlobalData()->clrTextHilite;
  return bDisabled ? m_clrGrayedText : m_clrTextHilite;
}

/////////////////////////////////////////////////////////////////////////////

COLORREF CVT100VisualManager::GetToolbarButtonTextColor(CMFCToolBarButton* pButton, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pButton);

	BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() && !pButton->IsEditable()) || (!CMFCToolBar::IsCustomizeMode() &&(pButton->m_nStyle & TBBS_DISABLED));
	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton))) {
		return CMFCVisualManagerOfficeXP::GetToolbarButtonTextColor(pButton,  state);
	}
	else return (bDisabled ? m_clrGrayedText : (state == ButtonsIsHighlighted) ? m_clrTextHilite : m_clrTextNormal);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CVT100VisualManager::OnNcActivate(CWnd* pWnd, BOOL bActive)
{
	ASSERT_VALID(pWnd);

	pWnd->SendMessage(WM_NCPAINT, 0, 0);
	return TRUE;
//	return CMFCVisualManagerOfficeXP::OnNcActivate(pWnd, bActive);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CVT100VisualManager::OnNcPaint(CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw)
{
	ASSERT_VALID(pWnd);

	return CMFCVisualManagerOfficeXP::OnNcPaint(pWnd, lstSysButtons, rectRedraw);
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCShowAllButton))) {
		if (state == ButtonsIsHighlighted) {
			CDrawingManager dm(*pDC);
			dm.HighlightRect(rect);
		}
		return;
	}
	if (!m_bEnableToolbarButtonFill) {
		BOOL bIsPopupMenu = FALSE;
		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
		if (pMenuButton != NULL) {
			bIsPopupMenu = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));
		}
		if (!bIsPopupMenu) {
			return;
		}
	}
	if (!pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton)) && !CMFCToolBar::IsCustomizeMode() &&	state != ButtonsIsHighlighted && (pButton->m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE))) {
	}				// do nothing
}

/////////////////////////////////////////////////////////////////////////////

void CVT100VisualManager::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited)
{
	CMFCVisualManagerOfficeXP::OnDrawScrollButtons(pDC, rect, nBorderSize, iImage, bHilited);
}

/////////////////////////////////////////////////////////////////////////////



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
#include "Evt100Defs.h"
#include "Evt100.h"
#include "Evt100Dlgs.h"
#include <afxadv.h>
#include "EVT100VisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

void PASCAL DDX_HexText(CDataExchange *pDX, int nIDC, long& value)
{
HWND hWndCtrl =pDX->PrepareEditCtrl(nIDC);
char buf[20];

  if (pDX->m_bSaveAndValidate){
    value = 0L ;
    ::GetWindowText(hWndCtrl, buf, 20 );
    if(strchr(buf,'x')!=NULL) sscanf_s( buf, "0x%lx", &value ) ;
    else if(strchr(buf,'o')!=NULL) sscanf_s( buf, "0o%lo", &value ) ;
    else sscanf_s( buf, "%lu", &value ) ;
  }
  else{
    sprintf_s(buf, 20, "0x%lx", value ) ;
    ::SetWindowText( hWndCtrl, buf ) ;
  }
}

void PASCAL DDV_HexText( CDataExchange *pDX, long& value, BOOL &flag)
{
  if(pDX->m_bSaveAndValidate && ( value > 0xffffffff )) {
    AfxMessageBox( "You entered an invalid value!",MB_ICONEXCLAMATION) ; 
    pDX->Fail();
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAboutDlg::IDD, pParent)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_bitmap.SubclassDlgItem(IDB_ELBIEYE, this);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CAboutBitmap, CButton)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CAboutBitmap::DrawItem(LPDRAWITEMSTRUCT lpD)
{
CBitmap Bitmap;

  CDC* pDC = CDC::FromHandle(lpD->hDC);
  ASSERT(pDC != NULL);
  Bitmap.LoadBitmap(IDB_EMA);
  CDC dcMem;
  if(!dcMem.CreateCompatibleDC(pDC)) return;
  CBitmap* pBitmapOld = dcMem.SelectObject(&Bitmap);
  if(pBitmapOld == NULL) return;
  pDC->BitBlt(lpD->rcItem.left,lpD->rcItem.top,lpD->rcItem.right,lpD->rcItem.bottom,&dcMem,0,0,SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CEVTSettingsDlg::CEVTSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEVTSettingsDlg::IDD, pParent)
{
	m_LineWrap = FALSE;
	m_ViewWrap = FALSE;
	m_Baud = _T("");
	m_DataBits = _T("");
	m_DTRDSR = FALSE;
	m_LocalEcho = FALSE;
	m_NewLine = FALSE;
	m_Parity = -1;
	m_RTSCTS = FALSE;
	m_StopBits = -1;
	m_XONXOFF = FALSE;
	m_SerialPort = _T("");
	memset(&m_LogFont, 0, sizeof(m_LogFont));
	m_LogFont.lfHeight = -9;
	m_LogFont.lfWeight = FW_DONTCARE;
	m_LogFont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	strcpy_s(m_LogFont.lfFaceName, sizeof(m_LogFont.lfFaceName), "FixedSys");
	m_IsConnected = FALSE;
	m_BgColor = CLR_BAR_BACKGROUND;
	m_FgColor = CLR_MENU_TEXT_NORM;
	m_BgBrush.CreateSolidBrush(m_BgColor);

}

/////////////////////////////////////////////////////////////////////////////

void CEVTSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_LINEWRAP, m_LineWrap);
	DDX_Check(pDX, IDC_VIEWWRAP, m_ViewWrap);
	DDX_CBString(pDX, IDC_BAUDCB, m_Baud);
	DDX_CBString(pDX, IDC_DATABITSCB, m_DataBits);
	DDX_Check(pDX, IDC_DTRDSR, m_DTRDSR);
	DDX_Check(pDX, IDC_LOCALECHO, m_LocalEcho);
	DDX_Check(pDX, IDC_NEWLINE, m_NewLine);
	DDX_CBIndex(pDX, IDC_PARITYCB, m_Parity);
	DDX_Check(pDX, IDC_RTSCTS, m_RTSCTS);
	DDX_CBIndex(pDX, IDC_STOPBITSCB, m_StopBits);
	DDX_Check(pDX, IDC_XONXOFF, m_XONXOFF);
	DDX_CBString(pDX, IDC_PORTCB, m_SerialPort);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEVTSettingsDlg, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

BOOL CEVTSettingsDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
//	SetBackgroundColor(m_BgColor);
	((CComboBox *)GetDlgItem(IDC_PORTCB))->EnableWindow(!m_IsConnected);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

HBRUSH CEVTSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

/*	if(nCtlColor == CTLCOLOR_MSGBOX || nCtlColor == CTLCOLOR_DLG) {
		pDC->SetTextColor(m_FgColor);
		pDC->SetBkColor(m_BgColor);
		return (HBRUSH) m_BgBrush.GetSafeHandle();
	}*/
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}


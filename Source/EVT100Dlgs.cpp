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
	: CDialog(CAboutDlg::IDD, pParent)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
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
	: CDialog(CEVTSettingsDlg::IDD, pParent)
{
	m_LineWrap = FALSE;
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
	memset(&m_lfFont, 0, sizeof(m_lfFont));
	m_lfFont.lfHeight = -9;
	m_lfFont.lfWeight = FW_DONTCARE;
	m_lfFont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	strcpy_s(m_lfFont.lfFaceName, sizeof(m_lfFont.lfFaceName), "FixedSys");
	m_IsConnected = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVTSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_AUTOWRAP, m_LineWrap);
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

BEGIN_MESSAGE_MAP(CEVTSettingsDlg, CDialog)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

BOOL CEVTSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	((CComboBox *)GetDlgItem(IDC_PORTCB))->EnableWindow(!m_IsConnected);
	return TRUE;
}


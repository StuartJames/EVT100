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
* SJ   19/10/2020  Original
*
*/

#include "stdafx.h"
#include "Evt100Defs.h"
#include "Evt100.h"
#include "Evt100Dialogs.h"
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
	m_AutoReconnect = FALSE;
	m_SerialPort = _T("");
/*	memset(&m_LogFont, 0, sizeof(m_LogFont));
	m_LogFont.lfHeight = -9;
	m_LogFont.lfWeight = FW_DONTCARE;
	m_LogFont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	strcpy_s(m_LogFont.lfFaceName, sizeof(m_LogFont.lfFaceName), "FixedSys");		*/
	m_IsConnected = FALSE;
	m_BgColor = CLR_BAR_BACKGROUND;
	m_FgColor = CLR_MENU_TEXT_NORM;
	m_BgBrush.CreateSolidBrush(m_BgColor);

}

///////////////////////////////////////////////////////////////////////////////

CEVTSettingsDlg::~CEVTSettingsDlg()
{
	m_BgBrush.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////

void CEVTSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COM_GRP, m_ComGrp);
	DDX_Control(pDX, IDC_SCRN_GRP, m_ScrnGrp);

	DDX_Control(pDX, IDC_DTRDSR, m_BtnDtrDsr);
	DDX_Control(pDX, IDC_RTSCTS, m_BtnRtsCts);
	DDX_Control(pDX, IDC_XONXOFF, m_BtnXonXoff);
	DDX_Control(pDX, IDC_LINEWRAP, m_BtnLineWrap);
	DDX_Control(pDX, IDC_VIEWWRAP, m_BtnViewWrap);
	DDX_Control(pDX, IDC_NEWLINE, m_BtnNewLine);
	DDX_Control(pDX, IDC_LOCALECHO, m_BtnEcho);		
	DDX_Control(pDX, IDC_PORTCB, m_ComboPort);
	DDX_Control(pDX, IDC_BAUDCB, m_ComboBaud);
	DDX_Control(pDX, IDC_DATABITSCB, m_ComboDataBits);
	DDX_Control(pDX, IDC_PARITYCB, m_ComboParity);
	DDX_Control(pDX, IDC_STOPBITSCB, m_ComboStopBits);
	DDX_Control(pDX, IDC_AUTO_RECONNECT, m_BtnAutoRecon);
  DDX_Control(pDX, IDC_SCRIPTS, m_BtnScripts);
  DDX_Control(pDX, IDOK, m_BtnOK);
	DDX_Control(pDX, IDCANCEL, m_BtnCancel);

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
	DDX_Check(pDX, IDC_AUTO_RECONNECT, m_AutoReconnect);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEVTSettingsDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_SCRIPTS, &CEVTSettingsDlg::OnBnClickedScripts)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

BOOL CEVTSettingsDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(m_BgColor);
	((CComboBox *)GetDlgItem(IDC_PORTCB))->EnableWindow(!m_IsConnected);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

HBRUSH CEVTSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	if(nCtlColor == CTLCOLOR_MSGBOX || nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_BTN || nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_EDIT) {
		pDC->SetTextColor(m_FgColor);
		pDC->SetBkColor(m_BgColor);
		return (HBRUSH) m_BgBrush.GetSafeHandle();
	}	
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}

/////////////////////////////////////////////////////////////////////////////

void CEVTSettingsDlg::OnBnClickedScripts()
{
CEVTScriptsDlg ScriptDlg;

  ScriptDlg.m_IsConnected = m_IsConnected;
	ScriptDlg.m_ScriptType = m_ScriptType;
	if(ScriptDlg.DoModal() == IDOK){
    m_ScriptType = ScriptDlg.m_ScriptType;
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CEVTScriptsDlg::CEVTScriptsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEVTScriptsDlg::IDD, pParent)
{
	m_IsConnected = FALSE;
	m_ScriptType = 0;
	m_BgColor = CLR_BAR_BACKGROUND;
	m_FgColor = CLR_MENU_TEXT_NORM;
	m_BgBrush.CreateSolidBrush(m_BgColor);

}

///////////////////////////////////////////////////////////////////////////////

CEVTScriptsDlg::~CEVTScriptsDlg()
{
	m_BgBrush.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////

void CEVTScriptsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCRIPT_GRP, m_ScriptGrp);
	DDX_Control(pDX, IDC_SCRIPT_NONE, m_ScriptNone);
	DDX_Control(pDX, IDC_SCRIPT_ESP32, m_ScriptESP32);
	DDX_Control(pDX, IDC_SCRIPT_USBJTAG, m_ScriptUSBJTAG);
	DDX_Control(pDX, IDC_SCRIPT_CUSTOM, m_ScriptCustom);
	DDX_Control(pDX, IDOK, m_BtnOK);
	DDX_Control(pDX, IDCANCEL, m_BtnCancel);
	DDX_Radio(pDX, IDC_SCRIPT_NONE, m_ScriptType);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEVTScriptsDlg, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CEVTScriptsDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(m_BgColor);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

HBRUSH CEVTScriptsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	if(nCtlColor == CTLCOLOR_MSGBOX || nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_BTN || nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_EDIT) {
		pDC->SetTextColor(m_FgColor);
		pDC->SetBkColor(m_BgColor);
		return (HBRUSH) m_BgBrush.GetSafeHandle();
	}	
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}



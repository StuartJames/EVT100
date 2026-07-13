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

#pragma once

#include "EVT100Button.h"
#include "EVT100DropDown.h"

void PASCAL DDX_HexText(CDataExchange *pDX, int nIDC, long& value);
void PASCAL DDV_HexText(CDataExchange *pDX, long& value, BOOL &flag);

/////////////////////////////////////////////////////////////////////////////
// CAboutBitmap window

class CAboutBitmap : public CButton
{
public:
//	void							SizeToContent();

protected:
	virtual void			DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAboutBox dialog

class CAboutDlg : public CDialogEx
{

public:
										CAboutDlg(CWnd* pParent = NULL);    // standard constructor

	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CAboutBitmap			m_bitmap;

	virtual BOOL			OnInitDialog();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CEVTSettingsDlg dialog

class CEVTSettingsDlg : public CDialogEx
{
public:
										CEVTSettingsDlg(CWnd* pParent = NULL);   // standard constructor

	virtual						~CEVTSettingsDlg();

	BOOL							m_LineWrap;
	BOOL							m_ViewWrap;
	CString						m_Baud;
	CString						m_DataBits;
	BOOL							m_DTRDSR;
	BOOL							m_LocalEcho;
	BOOL							m_NewLine;
	int								m_Parity;
	BOOL							m_RTSCTS;
  int								m_StopBits;
	BOOL							m_XONXOFF;
	BOOL							m_AutoReconnect;
	CString						m_SerialPort;
	LOGFONT						m_LogFont;
	BOOL							m_IsConnected;
  int								m_ScriptType;
	CEVT100Btn				m_ComGrp;
	CEVT100Btn				m_ScrnGrp;
	CEVT100Btn				m_BtnDtrDsr;
	CEVT100Btn				m_BtnRtsCts;
	CEVT100Btn				m_BtnXonXoff;
	CEVT100Btn				m_BtnLineWrap;
	CEVT100Btn				m_BtnViewWrap;
	CEVT100Btn				m_BtnNewLine;
	CEVT100Btn				m_BtnEcho;
	CEVT100Btn				m_BtnBaud;
  CEVT100Btn				m_BtnScripts;
  CEVT100Btn				m_BtnAutoRecon;
  CEVT100Btn				m_BtnOK;
  CEVT100Btn				m_BtnCancel;
  CEVT100DropDown		m_ComboPort;
	CEVT100DropDown		m_ComboBaud;
	CEVT100DropDown		m_ComboParity;
	CEVT100DropDown		m_ComboDataBits;
	CEVT100DropDown		m_ComboStopBits;

	enum { IDD = IDD_SETTINGS };

protected:
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	COLORREF					m_BgColor;
	COLORREF					m_FgColor;
	CBrush						m_BgBrush;

	virtual	BOOL			OnInitDialog();
	virtual	HBRUSH		OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

public:
   afx_msg void			OnBnClickedScripts();
};

/////////////////////////////////////////////////////////////////////////////
// CEVTSettingsDlg dialog

class CEVTScriptsDlg : public CDialogEx {
public:
										CEVTScriptsDlg(CWnd *pParent = NULL);  // standard constructor
 	virtual						~CEVTScriptsDlg();

	CEVT100Btn				m_ScriptGrp;
	CEVT100Btn				m_ScriptNone;
	CEVT100Btn				m_ScriptESP32;
	CEVT100Btn				m_ScriptUSBJTAG;
	CEVT100Btn				m_ScriptCustom;
  CEVT100Btn				m_BtnOK;
  CEVT100Btn				m_BtnCancel;
	BOOL							m_IsConnected;
  int								m_ScriptType;

	enum { IDD = IDD_SCRIPTS };

protected:
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	COLORREF					m_BgColor;
	COLORREF					m_FgColor;
	CBrush						m_BgBrush;

	virtual	BOOL			OnInitDialog();
	virtual	HBRUSH		OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

};
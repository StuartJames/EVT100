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

extern BOOL _bAbort;
void PASCAL DDX_HexText(CDataExchange *pDX, int nIDC, long& value);
void PASCAL DDV_HexText(CDataExchange *pDX, long& value, BOOL &flag);

/////////////////////////////////////////////////////////////////////////////
// CAboutBitmap window

class CAboutBitmap : public CButton
{
public:
	void SizeToContent();

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAboutBox dialog

class CAboutDlg : public CDialog
{
	CAboutBitmap	m_bitmap;

public:
	CAboutDlg(CWnd* pParent = NULL);    // standard constructor

	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CEVTSettingsDlg dialog

class CEVTSettingsDlg : public CDialog
{
public:
	CEVTSettingsDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_SETTINGS };
	BOOL			m_LineWrap;
	BOOL			m_ViewWrap;
	CString		m_Baud;
	CString		m_DataBits;
	BOOL			m_DTRDSR;
	BOOL			m_LocalEcho;
	BOOL			m_NewLine;
	int				m_Parity;
	BOOL			m_RTSCTS;
  int				m_StopBits;
	BOOL			m_XONXOFF;
	CString		m_SerialPort;
	LOGFONT		m_LogFont;
	BOOL			m_IsConnected;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

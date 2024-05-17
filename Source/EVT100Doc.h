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

#include "EVTLineObj.h"

const char CoSeDe[] = ";";				// Control Sequence Delimeter

 // Secondary thread function
UINT CommWatchProc(LPVOID lpParam);

class CEVT100View;

class CEVT100Doc : public CDocument
{
protected: 
	CEVT100Doc();
	DECLARE_DYNCREATE(CEVT100Doc)
 	CString				m_SerialPort;
	long					m_Baud;
	int						m_DataBits;
	BOOL					m_DTRDSR;
	BOOL					m_LocalEcho;
	BOOL					m_LineWrap;
	BOOL					m_NewLine;
	int						m_Parity;
	BOOL					m_RTSCTS;
	int						m_StopBits;
	BOOL					m_XONXOFF;
  DWORD					m_EventType;
  BYTE*					m_InBlock;
	int						m_CurrentAttr;
	char					m_EscapeArgs[MAX_ESC_ARGS + 1];
	int						m_ArgCount;
	bool					m_ShowCodes;
	int						m_EscState;

protected: 
	void					GetSystemVars();
	void					SaveSystemVars();
  void					IncLineIndex(int Inc);
  void					DecLineIndex(int Dec);
	void					ProcessCtrlSequ(LPSTR lpMessage, int *i);

public:
	bool					m_IsConnected;
	HANDLE				m_idComDev;
	CEVT100View		*m_pTermWnd;		// The view window
	HANDLE				m_hPostEvent;	// To sync WM_COMMNOTIFY processing
	CWinThread		*m_pThread;
	OVERLAPPED		m_osWrite, m_osRead;
	LOGFONT				m_lfFont;
	CSize					m_CharSize;
	CLineObj			m_Screen[MAXROW];
	char					*m_pLineBuf;
	CPoint				m_CursorPos;
	CPoint				m_CursorSave;
	int						m_TopRow;
	int						m_Scrolled;
	CString				Title;

public:
	bool					OpenConnection();
	bool					SetupConnection();
	void					CloseConnection();
	int						ReadCommBlock();
	bool					SendHostByte(char cChar);
	bool					SendHostMessage(const char* lpStrFmt, ...);
  bool					FormatScreenData(UINT uID);
  bool					FormatScreenData(const char* lpStrFmt, ...);
  bool					ProcessHostData(int nLength, LPSTR lpBlock);
	void					SetFontSize();

	public:
	virtual BOOL	OnNewDocument();
	virtual void	OnCloseDocument();

public:
	virtual				~CEVT100Doc();
	virtual void	Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void	OnEditSettings();
	afx_msg void	OnViewClear();
	afx_msg void	OnUpdateViewClear(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewSetfont();
};

/////////////////////////////////////////////////////////////////////////////

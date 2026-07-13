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

#include "EVTLineObj.h"

enum ScriptType_e{
  SCRIPT_TYPE_NONE = 0,
  SCRIPT_TYPE_ESP32,		// Classic reset sequence, sets DTR and RTS lines sequentially.
  SCRIPT_TYPE_USBJTAG,  // Required when the device is connecting via its USB-JTAG-Serial peripheral.
  SCRIPT_TYPE_CUSTOM,
};

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
	int						m_Parity;
	BOOL					m_RTSCTS;
	int						m_StopBits;
	BOOL					m_XONXOFF;
  BOOL					m_AutoReconnect;
	BOOL					m_LocalEcho;
	BOOL					m_NewLine;
	Wrap_t				m_UserWrap;
	Wrap_t				m_SoftWrap;
  DWORD					m_EventType;
  BYTE*					m_InBlock;
	int						m_CurrentAttr;
	char					m_EscapeArgs[MAX_ESC_ARGS + 1];
	int						m_ArgCount;
	bool					m_ShowCodes;
	int						m_EscState;
  int						m_ScriptType;

protected: 
	void					GetSystemVars();
	void					SaveSystemVars();
  void					IncLineIndex(int Inc);
  void					DecLineIndex(int Dec);
	void					ProcessCtrlSequ(LPSTR lpMessage, int *i);
	void					ScreenErase(int Mode = 0);
	void					RunConnectScript(void);

public:
	bool					m_IsConnected;
	HANDLE				m_idComDev;
	CEVT100View		*m_pView;		// The view window
	HANDLE				m_hPostEvent;	// To sync WM_COMMNOTIFY processing
	CWinThread		*m_pThread;
	OVERLAPPED		m_osWrite, m_osRead;
	LOGFONT				m_LogFont;
	CLineObj			m_Screen[MAXROW];
	char					*m_pLineBuf;
	CPoint				m_CursorPos;
	CPoint				m_CursorSave;
	int						m_TopRow;
	int						m_Scrolled;
	CString				Title;

	bool					OpenConnection();
	bool					SetupConnection();
	void					CloseConnection();
	void					Reconnect();
	int						ReadCommBlock();
	bool					SendHostByte(char cChar);
	bool					SendHostMessage(const char* lpStrFmt, ...);
  bool					FormatScreenData(UINT uID);
  bool					FormatScreenData(const char* lpStrFmt, ...);
  bool					ProcessHostData(int nLength, LPSTR lpBlock);
	void					SetDTR(bool State);
	void					SetRTS(bool State);

	virtual BOOL	OnNewDocument();
	virtual void	OnCloseDocument();

	virtual				~CEVT100Doc();
	virtual void	Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif

	afx_msg void	OnViewSetfont();

protected:
	afx_msg void	OnEditSettings();
	afx_msg void	OnViewClear();
	afx_msg void	OnUpdateViewClear(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

inline void CEVT100Doc::SetDTR(bool State)
{
  if(m_IsConnected){
		if(State) EscapeCommFunction(m_idComDev, SETDTR);
		else EscapeCommFunction(m_idComDev, CLRDTR);
	}
}

/////////////////////////////////////////////////////////////////////////////

inline void CEVT100Doc::SetRTS(bool State)
{
  if(m_IsConnected){
		if(State) EscapeCommFunction(m_idComDev, SETRTS);
		else EscapeCommFunction(m_idComDev, CLRRTS);
	}
}

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
#include <afxadv.h>
#include "EVT100Defs.h"
#include <fstream>
#include "EVT100.h"
#include "mainfrm.h"
#include "EVT100Doc.h"
#include "EVT100View.h"
#include "EVT100Dlgs.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL _bAbort;
#define WRITE_TIMEOUT     5000     // milliseconds
#define READ_TIMEOUT      500      // milliseconds

static char BASED_CODE szSettings[] = "Settings";
static char BASED_CODE szVariables[] = "Variables";
static char BASED_CODE szFont[] = "Font";
static char BASED_CODE szFormat[] = "%d %d %d %d %d %d %d %d %d %d %d %5s";
static char BASED_CODE szRFFormat[] = "%ld %ld %d %d %d %[ -z]";
static char BASED_CODE szWFFormat[] = "%ld %ld %d %d %d %s";
static char BASED_CODE szSystem[] = "System";
static char BASED_CODE szFileList[] = "Recent File List";
static char BASED_CODE szFileEntry[] = "File%d";

/////////////////////////////////////////////////////////////////////////////

static BOOL GetProfileFont(LOGFONT* plf)
{
CWinApp* pApp = AfxGetApp();

  CString strBuffer = AfxGetApp()->GetProfileString(szSettings, szFont);
  if (strBuffer.IsEmpty()) return FALSE;
  LOGFONT lf;
  ZeroMemory(&lf, sizeof(lf));
  int nRead = sscanf_s(strBuffer, szRFFormat,
    &lf.lfHeight,
    &lf.lfWeight,
    &lf.lfItalic,
    &lf.lfUnderline,
    &lf.lfPitchAndFamily,
    &lf.lfFaceName, LF_FACESIZE);
  if (nRead != 6) return FALSE;
  *plf = lf;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

static void WriteProfileFont(const LOGFONT* plf)
{
CWinApp* pApp = AfxGetApp();
char szBuffer[sizeof("-32767") * 6 + LF_FACESIZE];

  sprintf_s(szBuffer, szWFFormat,
    plf->lfHeight,
    plf->lfWeight,
    plf->lfItalic,
    plf->lfUnderline,
    plf->lfPitchAndFamily,
    plf->lfFaceName, LF_FACESIZE);
  AfxGetApp()->WriteProfileString(szSettings, szFont, szBuffer);
}

/////////////////////////////////////////////////////////////////////////////

UINT CommWatchProc(LPVOID lpParam)
{
CEVT100Doc *pDoc = (CEVT100Doc *)lpParam;
OVERLAPPED os;
DWORD dwEventMask, dwEventType, dwTransfer;
USHORT EventTest;
COMSTAT CommStatus;

  ZeroMemory(&os, sizeof(OVERLAPPED));
  os.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (os.hEvent == NULL){
   	AfxMessageBox(IDS_NOTHREAD, MB_OK | MB_ICONEXCLAMATION);
	  return FALSE;
  }
  dwEventMask = (EV_RXCHAR | EV_ERR | EV_BREAK);
  if(!SetCommMask(pDoc->m_idComDev, dwEventMask)) return FALSE;
  while(pDoc->m_IsConnected){	// Loop until user requests disconnect
    dwEventType = 0;
  	if(!WaitCommEvent(pDoc->m_idComDev, &dwEventType, &os)){
	    if(GetLastError() == ERROR_IO_PENDING){
	 	    GetOverlappedResult(pDoc->m_idComDev, &os, &dwTransfer, TRUE);
  		  os.Offset += dwTransfer;
	    }
  	}
    EventTest = 1;
    while(pDoc->m_IsConnected && (dwEventType & dwEventMask)){ // post a message for each event
      if((dwEventType & EventTest) == EventTest){
        pDoc->m_EventType = EventTest;
        ResetEvent(pDoc->m_hPostEvent);   // notify primary thread that data is waiting
        ((CWnd *)(pDoc->m_pTermWnd))->PostMessage(WM_COMMNOTIFY, (WPARAM)pDoc->m_idComDev, MAKELONG(COM_EVENT, 0));
        WaitForSingleObject(pDoc->m_hPostEvent, INFINITE);  // Wait until WM_COMMNOTIFY is processed by the primary thread
        if(EventTest == EV_ERR) ClearCommError(pDoc->m_idComDev, &dwTransfer, &CommStatus);
      }
      dwEventType &= ~EventTest;
      EventTest += EventTest;
    }
  }
  CloseHandle(os.hEvent);
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CEVT100Doc, CDocument)

BEGIN_MESSAGE_MAP(CEVT100Doc, CDocument)
	ON_COMMAND(IDM_FILE_SETTINGS, OnEditSettings)
	ON_COMMAND(IDM_VIEW_CLEAR, OnViewClear)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_CLEAR, OnUpdateViewClear)
  ON_COMMAND(ID_VIEW_SETFONT, &CEVT100Doc::OnViewSetfont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CEVT100Doc::CEVT100Doc()
{
  m_UserWrap.Line = true;
  m_UserWrap.View = true;
  m_SoftWrap.Line = true;
  m_SoftWrap.View = true;
  m_Baud = 9600;
  m_DataBits = 8;
  m_DTRDSR = FALSE;
  m_LocalEcho = FALSE;
  m_NewLine = FALSE;
  m_Parity = 0;
  m_RTSCTS = TRUE;
  m_StopBits = 0;
  m_XONXOFF = FALSE;
  m_IsConnected = FALSE;
  m_ShowCodes = false;
  m_SerialPort="COM1";
  ZeroMemory(&m_LogFont, sizeof(m_LogFont));
  m_LogFont.lfHeight = -13;
  m_LogFont.lfWeight = FW_DONTCARE;
  m_LogFont.lfItalic = 255;
  m_LogFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
  strcpy_s(m_LogFont.lfFaceName, sizeof(m_LogFont.lfFaceName), "Consolas");
  ZeroMemory(&m_osRead, sizeof(OVERLAPPED));
  ZeroMemory(&m_osWrite, sizeof(OVERLAPPED));
  m_hPostEvent = NULL;
  m_pThread = NULL;
  m_pTermWnd = NULL;
  m_CursorPos.SetPoint(0, 0);
  m_CursorSave.SetPoint(0, 0);
  m_TopRow = 0;
  m_Scrolled = 0;
  m_InBlock = new BYTE[INBLOCKSIZE + 1];
  m_ArgCount = 0;
  m_EscState = ESC_PROC_NORMAL;
  m_CurrentAttr = ATTR_DEFAULT;
  m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;
  GetProfileFont(&m_LogFont);
  GetSystemVars();
}

/////////////////////////////////////////////////////////////////////////////

CEVT100Doc::~CEVT100Doc()
{
  for(int i = 0; i < MAXROW; ++i) m_Screen[i].Clear();
  if(m_InBlock) delete m_InBlock;
  if (m_osRead.hEvent){
  	CloseHandle(m_osRead.hEvent);
  	m_osRead.hEvent = NULL;
  }
  if (m_osWrite.hEvent){
  	CloseHandle(m_osWrite.hEvent);
  	m_osWrite.hEvent = NULL;
  }
  if(m_hPostEvent){
  	CloseHandle(m_hPostEvent);
  	m_hPostEvent = NULL;
  }
  SaveSystemVars();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::GetSystemVars()
{
  char temp[10] = {0};

  CString strBuffer = AfxGetApp()->GetProfileString(szSettings, szVariables);
  if (strBuffer.IsEmpty()) return;
  int nRead = sscanf_s(strBuffer, szFormat,
    &m_XONXOFF, &m_LocalEcho,
    &m_NewLine, &m_UserWrap.Line,
    &m_UserWrap.View, &m_RTSCTS,
    &m_DataBits, &m_DTRDSR, 
    &m_Parity, &m_StopBits,
    &m_Baud, &temp, 10);
  if(strlen(temp) > 3 ) m_SerialPort = temp;
  m_SoftWrap.Line = m_UserWrap.Line;
  m_SoftWrap.View = m_UserWrap.View;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::SaveSystemVars()
{
char szBuffer[100];

  sprintf_s(szBuffer, sizeof(szBuffer), szFormat,
    m_XONXOFF, m_LocalEcho,
    m_NewLine, m_UserWrap.Line,
    m_UserWrap.View, m_RTSCTS,
    m_DataBits, m_DTRDSR,
    m_Parity, m_StopBits,
    m_Baud, (const char *)m_SerialPort);
  AfxGetApp()->WriteProfileString(szSettings, szVariables, szBuffer);
  WriteProfileFont(&m_LogFont);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CEVT100Doc::OnNewDocument()
{
  if (!CDocument::OnNewDocument()) return FALSE;
  if ((m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) return FALSE;
  if ((m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL))	== NULL) return FALSE;
  if ((m_hPostEvent = CreateEvent(NULL, TRUE, TRUE, NULL)) == NULL) return FALSE;
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnCloseDocument() 
{
  if(m_IsConnected) CloseConnection();
	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CEVT100Doc serialization

void CEVT100Doc::Serialize(CArchive& ar)
{
  if (ar.IsStoring()){
  }
  else{
  }
}

/////////////////////////////////////////////////////////////////////////////
// CEVT100Doc diagnostics

#ifdef _DEBUG
void CEVT100Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEVT100Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif 

/////////////////////////////////////////////////////////////////////////////

bool CEVT100Doc::OpenConnection()
{
POSITION firstViewPos;
CEVT100View *pView;
bool fRetVal;
COMMTIMEOUTS CommTimeOuts;
CString cstr;

  if(m_IsConnected) return false;
  CWaitCursor Wait;   // do wait cursor 
  firstViewPos = GetFirstViewPosition();
  pView = (CEVT100View *)GetNextView(firstViewPos);
  pView->SetFont(&m_LogFont);
	cstr.Format(_T("\\\\.\\%s"), m_SerialPort);
	if((m_idComDev = CreateFile(cstr, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE){
		DWORD Error = GetLastError();
		return false;
	}
  m_IsConnected = SetupConnection();
  CommTimeOuts.ReadIntervalTimeout = MAXDWORD;
  CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
  CommTimeOuts.ReadTotalTimeoutConstant = 0;
  CommTimeOuts.WriteTotalTimeoutMultiplier = 20;
  CommTimeOuts.WriteTotalTimeoutConstant = 20;
  if((m_IsConnected = SetCommTimeouts(m_idComDev, &CommTimeOuts)) == FALSE){
    CloseHandle(m_idComDev);
    AfxMessageBox("Failed to start communications thread!");
    return false;
  }
  SetCommMask(m_idComDev, EV_RXCHAR);
  if(m_IsConnected){	// Create secondary thread
 	 	m_pTermWnd = pView;
		if((m_pThread = AfxBeginThread(CommWatchProc, this)) == NULL){
		  m_IsConnected = FALSE;
		  CloseHandle(m_idComDev);
		  fRetVal = FALSE;
		}
		else{
		  m_pThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		  m_pThread->ResumeThread();
//		  EscapeCommFunction(m_idComDev, SETDTR);
		}
  }
  else CloseHandle(m_idComDev);
  if(m_IsConnected){
    m_CurrentAttr = ATTR_DEFAULT;
    pView->SendMessage(WM_SETFOCUS);
    ((CMainFrame*)AfxGetMainWnd())->SetWindowTitle(m_SerialPort);
    m_SoftWrap.Line = m_UserWrap.Line;                            // reset wrap to user settings
    m_SoftWrap.View = m_UserWrap.View;
  }
  return m_IsConnected;
}

/////////////////////////////////////////////////////////////////////////////

bool CEVT100Doc::SetupConnection()
{
DCB dcb;

	ZeroMemory(&dcb,sizeof(dcb));
  dcb.DCBlength = sizeof(DCB);
  if(!GetCommState(m_idComDev, &dcb)) return false;
  dcb.BaudRate = m_Baud;
  dcb.ByteSize = m_DataBits;
  dcb.fParity = TRUE;
  switch (m_Parity){
    case 0:
   		dcb.Parity = NOPARITY;
	    dcb.fParity = FALSE;
  		break;
    case 1: dcb.Parity = EVENPARITY; break;
    case 2: dcb.Parity = ODDPARITY; break;
    case 3: dcb.Parity = MARKPARITY; break;
    case 4: dcb.Parity = SPACEPARITY; break;
    default: ASSERT(FALSE);
  }
  switch (m_StopBits){
  	case 0: dcb.StopBits = ONESTOPBIT; break;
	  case 1: dcb.StopBits = ONE5STOPBITS; break;
  	case 2: dcb.StopBits = TWOSTOPBITS; break;
	  default: ASSERT(FALSE);
  }
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDsrSensitivity = m_DTRDSR;
  dcb.fDtrControl = m_DTRDSR ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_ENABLE;
  dcb.fOutxCtsFlow = m_RTSCTS;
  dcb.fRtsControl = m_RTSCTS ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;
  dcb.fInX = dcb.fOutX = m_XONXOFF;
  dcb.XonChar = ASCII_XON;
  dcb.XoffChar = ASCII_XOFF;
  dcb.XonLim = 100;
  dcb.XoffLim = 100;
  dcb.fBinary = TRUE;
  return SetCommState(m_idComDev, &dcb);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::CloseConnection()
{
POSITION firstViewPos;
CEVT100View *pView;

  if (!m_IsConnected) return;
  firstViewPos = GetFirstViewPosition();
	pView = (CEVT100View *)GetNextView(firstViewPos);
  pView->SendMessage(WM_KILLFOCUS);
  m_IsConnected = FALSE;
  SetCommMask(m_idComDev, 0);
  SetEvent(m_hPostEvent);
  WaitForSingleObject(m_pThread->m_hThread, INFINITE);
  m_pThread = NULL;
  EscapeCommFunction(m_idComDev, CLRDTR);
  PurgeComm(m_idComDev, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
  CloseHandle(m_idComDev);
  ((CMainFrame*)AfxGetMainWnd())->SetWindowTitle("");

}

/////////////////////////////////////////////////////////////////////////////

bool CEVT100Doc::SendHostByte(char cChar)
{
BOOL fWriteStat;
DWORD dwBytesWritten;

  if (!m_IsConnected) return false;
  fWriteStat = WriteFile(m_idComDev, &cChar, 1, &dwBytesWritten, &m_osWrite);
  if(!fWriteStat && (GetLastError() == ERROR_IO_PENDING)){
		WaitForSingleObject(m_osWrite.hEvent, WRITE_TIMEOUT);
		GetOverlappedResult(m_idComDev, &m_osWrite, &dwBytesWritten, false);
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CEVT100Doc::SendHostMessage(const char* lpStrFmt, ...)
{
int i = 0, cnt;
va_list  argptr;		                              // Argument list pointer	      
char strc[50];			                              // Buffer to build sting into   

  va_start(argptr, lpStrFmt);	                    // Initialize va_ functions     
  cnt = vsprintf_s(strc, 50, lpStrFmt, argptr);   // prints string to buffer	    
  va_end(argptr);			                            // Close va_ functions		
  while(i < cnt) if(!SendHostByte(strc[i++])) return false;
  return true;
}

/////////////////////////////////////////////////////////////////////////////

int CEVT100Doc::ReadCommBlock()
{
bool ReadWaiting = false, Error = false;
COMSTAT ComStat;
DWORD dwFlags, dwLength;

  if (!m_IsConnected) return 0;
  ClearCommError(m_idComDev, &dwFlags, &ComStat);
  dwLength = min((DWORD)INBLOCKSIZE, ComStat.cbInQue);
  if(dwLength > 0){
		if(!ReadFile(m_idComDev, m_InBlock, dwLength,	&dwLength, &m_osRead)){
	    if(GetLastError() != ERROR_IO_PENDING) Error = true;
			else ReadWaiting = true;
		}
		else {    
	 	  GetOverlappedResult(m_idComDev, &m_osRead, &dwLength, FALSE); // read return immidiately 
      m_osRead.Offset += dwLength;
    }
		while(ReadWaiting && m_IsConnected) {
			dwFlags = WaitForSingleObject(m_osRead.hEvent, READ_TIMEOUT);
			switch(dwFlags){  // Read completed.
	      case WAIT_OBJECT_0:
          if(!GetOverlappedResult(m_idComDev, &m_osRead, &dwLength, FALSE)){ // Error in communications
						Error = true;
						dwLength = 0;
					}
					else{ // Read completed successfully.
		  	    m_osRead.Offset += dwLength;
	          ReadWaiting = false;    //  Reset flag so that another opertion can be issued.
					}
	        break;
		    case WAIT_TIMEOUT: // Operation isn't complete yet.Loop back around.
          break;                       
			  default:
					Error = true;
          ReadWaiting = false;  // Error in the WaitForSingleObject
					dwLength = 0;
          break;
			}
		}
  }
  if(Error) FormatScreenData("\x1b[1mSlave communications port read error %lu\x1b[0m\n",GetLastError());
  return dwLength;
}

/////////////////////////////////////////////////////////////////////////////

bool CEVT100Doc::FormatScreenData(UINT uID)
{
CString str;

  str.LoadString(uID);
  return FormatScreenData((const char *)str);
}

/////////////////////////////////////////////////////////////////////////////

bool CEVT100Doc::FormatScreenData(const char* lpStrFmt, ...)
{
int i = 0, cnt;
va_list  argptr;		                                // Argument list pointer	      
char strc[132];			                                // Buffer to build sting into   

  va_start(argptr, lpStrFmt);	                      // Initialize va_ functions     
  cnt = vsprintf_s(strc, 132, lpStrFmt, argptr);    // prints string to buffer	    
  va_end(argptr);			                              // Close va_ functions	
  return ProcessHostData((int)strlen(strc), strc);
}

/////////////////////////////////////////////////////////////////////////////

#include "DataProcessing.cpp"

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnEditSettings() 
{
char buf[34];
CEVTSettingsDlg SettingsDlg;

  _itoa_s(m_Baud, buf, 34, 10);
  SettingsDlg.m_Baud = buf;
  _itoa_s(m_DataBits, buf, 34, 10);
  SettingsDlg.m_DataBits = buf;
  SettingsDlg.m_DTRDSR = m_DTRDSR;
  SettingsDlg.m_Parity = m_Parity;
  SettingsDlg.m_SerialPort = m_SerialPort;
  SettingsDlg.m_RTSCTS = m_RTSCTS;
  SettingsDlg.m_StopBits = m_StopBits;
  SettingsDlg.m_XONXOFF = m_XONXOFF;
  SettingsDlg.m_LineWrap = m_UserWrap.Line;
  SettingsDlg.m_ViewWrap = m_UserWrap.View;
  SettingsDlg.m_LocalEcho = m_LocalEcho;
  SettingsDlg.m_NewLine = m_NewLine;
  SettingsDlg.m_LogFont = m_LogFont;
  SettingsDlg.m_IsConnected = m_IsConnected;
  if(SettingsDlg.DoModal() == IDOK){
    m_Baud = atoi(SettingsDlg.m_Baud);
    m_DataBits = atoi(SettingsDlg.m_DataBits);
    m_DTRDSR = SettingsDlg.m_DTRDSR;
    m_Parity = SettingsDlg.m_Parity;
    if(!m_IsConnected) m_SerialPort = SettingsDlg.m_SerialPort;
    m_RTSCTS = SettingsDlg.m_RTSCTS;
    m_StopBits = SettingsDlg.m_StopBits;
    m_XONXOFF = SettingsDlg.m_XONXOFF;
    m_UserWrap.Line = SettingsDlg.m_LineWrap > 0;
    m_SoftWrap.Line = m_UserWrap.Line;
    m_UserWrap.View = SettingsDlg.m_ViewWrap> 0;
    m_SoftWrap.View = m_UserWrap.View;
    m_LocalEcho = SettingsDlg.m_LocalEcho;
    m_NewLine = SettingsDlg.m_NewLine;
    m_LogFont = SettingsDlg.m_LogFont;
    POSITION firstViewPos = GetFirstViewPosition();
    CEVT100View *pView = (CEVT100View *)GetNextView(firstViewPos);
    pView->SetFont(&m_LogFont);
    if(m_IsConnected){
      if(!SetupConnection()){
      	AfxMessageBox(IDS_BADSETUP);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnViewClear() 
{
  if(m_pTermWnd == NULL) return;
	for(int i = 0; i < MAXROW; i++) m_Screen[i].Clear();
  m_CursorPos.SetPoint(0, 0);
  m_TopRow = 0;
  m_Scrolled = 0;
  m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;
  m_pTermWnd->SetSizes();
  m_pTermWnd->RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_ERASE);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnUpdateViewClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(true);
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnViewSetfont()
{
LOGFONT newlf;

	newlf = m_LogFont;
	CFontDialog fontDialog(&newlf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FIXEDPITCHONLY);
	if(fontDialog.DoModal() == IDOK){
		m_LogFont = newlf;
	}
}

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
static char BASED_CODE szFormat[] = "%d %d %d %d %d %d %d %d %d %d %5s";
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
  m_LineWrap = TRUE;
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
  ZeroMemory(&m_lfFont, sizeof(m_lfFont));
  m_lfFont.lfHeight = -13;
  m_lfFont.lfWeight = FW_DONTCARE;
  m_lfFont.lfItalic = 255;
  m_lfFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
  strcpy_s(m_lfFont.lfFaceName, sizeof(m_lfFont.lfFaceName), "Consolas");
  ZeroMemory(&m_osRead, sizeof(OVERLAPPED));
  ZeroMemory(&m_osWrite, sizeof(OVERLAPPED));
  m_hPostEvent = NULL;
  m_pThread = NULL;
  m_pTermWnd = NULL;
  m_CursorPos.SetPoint(0, 0);
  m_CursorSave.SetPoint(0, 0);
  m_TopRow = 0;
  m_CharSize.cx = 8;
  m_CharSize.cy = 15;
  m_Scrolled = 0;
  m_InBlock = new BYTE[MAXBLOCK + 1];
  m_ArgCount = 0;
  m_EscState = ESC_PROC_NORMAL;
  m_CurrentAttr = ATTR_DEFAULT;
  m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;
  GetProfileFont(&m_lfFont);
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
  if (m_hPostEvent){
  	CloseHandle(m_hPostEvent);
  	m_hPostEvent = NULL;
  }
  SaveSystemVars();
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::GetSystemVars()
{
char temp[10];

  CString strBuffer = AfxGetApp()->GetProfileString(szSettings, szVariables);
  if (strBuffer.IsEmpty()) return;
  int nRead = sscanf_s(strBuffer, szFormat,
    &m_XONXOFF, &m_LocalEcho,
    &m_NewLine, &m_LineWrap,
    &m_RTSCTS, &m_DataBits,
    &m_DTRDSR, &m_Parity,
    &m_StopBits, &m_Baud,
    &temp, 10);
  if(strlen(temp) > 3 ) m_SerialPort = temp;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::SaveSystemVars()
{
char szBuffer[100];

  sprintf_s(szBuffer, sizeof(szBuffer), szFormat,
    m_XONXOFF, m_LocalEcho,
    m_NewLine, m_LineWrap,
    m_RTSCTS, m_DataBits,
    m_DTRDSR, m_Parity,
    m_StopBits, m_Baud,
    (const char *)m_SerialPort);
  AfxGetApp()->WriteProfileString(szSettings, szVariables, szBuffer);
  WriteProfileFont(&m_lfFont);
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
		// TODO: add storing code here
  }
  else{
		// TODO: add loading code here
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
#endif //_DEBUG

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
  SetFontSize();
  firstViewPos = GetFirstViewPosition();
  pView = (CEVT100View *)GetNextView(firstViewPos);
  pView->SetFont(&m_lfFont);
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
  dwLength = min((DWORD)MAXBLOCK, ComStat.cbInQue);
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

bool CEVT100Doc::ProcessHostData(int nLength, LPSTR lpMessage)
{
int i = 0;
char *pToken = nullptr, *pNextToken = nullptr;

  if(m_pTermWnd == NULL) return false;
//  lpMessage[nLength] = 0;
//  TRACE(":%s.\n", lpMessage);
	while(i < nLength){
    if(m_EscState == ESC_PROC_NORMAL){
      switch (lpMessage[i]){                              // standard ASCII control codes and normal text
        case ASCII_ESC:
          if(m_ShowCodes) break;
          m_EscState = ESC_PROC_ESCAPE;                   // process ESC codes with next iteration
          break;
        case ASCII_CR:
					m_CursorPos.x = 0;
				  break;
        case ASCII_BEL:
      	  break;
        case ASCII_BS:
				  if(m_CursorPos.x > 0){
					  m_pLineBuf[m_CursorPos.x] = 0;                // truncate current line
					  m_pLineBuf[--m_CursorPos.x] = ' ';            // erase old character
				  }
      	  break;
        case ASCII_TAB:
      	  if((m_CursorPos.x / TABSTOP) < ((MAXCOL - 1) / TABSTOP)){
      	    m_CursorPos.x += TABSTOP - m_CursorPos.x % TABSTOP;
      	  }
          break;
        case ASCII_LF:
      	  m_CursorPos.x = 0;
          IncLineIndex(1);
          m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;       // get a pointer to the character buffer
          m_Screen[m_CursorPos.y].Clear();
          m_Screen[m_CursorPos.y].PushAttr(m_CurrentAttr | ATTR_MARKER, m_CursorPos.x);
      	  m_pTermWnd->SetSizes();
      	  break;
        default:
       	  m_pLineBuf[m_CursorPos.x++] = (lpMessage[i] & 0x7f);
       	  m_pLineBuf[m_CursorPos.x] = 0;
      	  if(m_CursorPos.x >= MAXCOL){
            if(m_LineWrap) FormatScreenData("\n");          // start a new line
            else --m_CursorPos.x;                           // overwrite last character
          }
//			 	  m_pTermWnd->Invalidate(FALSE);
      	  break;
      }
    }
    else{
      switch(m_EscState){
        case ESC_PROC_ESCAPE:{
          switch(lpMessage[i]){
            case '[': 
              m_EscState = ESC_PROC_CSI;
              break;
            case '#':                             // double byte code: we don't service any of these
            case '(':
            case ')':
            case '0':
            case '3':
            case '5':
            case '6':
              m_EscState = ESC_LOSE_NEXT;         // lose the next character
              break;
            case 'P':
              m_EscState = ESC_PROC_STRING;       // String control: do nothing but wait for terminator
              break;
            default:                              // unknown code: go back to normal mode
              m_EscState = ESC_PROC_NORMAL;       
              break;
          }
          break;
        }
        case ESC_PROC_CSI:                        // process Control Sequence codes
          ProcessCtrlSequ(lpMessage, &i);
          break;
        case ESC_PROC_STRING:                     // wait for terminator
           if(lpMessage[i] == '/') m_EscState = ESC_PROC_NORMAL;
          break;
        case ESC_LOSE_NEXT:                       // just lose next character
        default:{
          m_EscState = ESC_PROC_NORMAL;           // go back to display mode
          break;
        }
      }
      if(m_EscState == ESC_PROC_NORMAL) m_ArgCount = 0;
    }
    i++;
  }
  m_pTermWnd->ScrollToCursor(true);
  return true;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::ProcessCtrlSequ(LPSTR lpMessage, int *i)
{
char *pToken = nullptr, *pNextToken = nullptr;
int j;

  switch(lpMessage[*i]){
    case 	'A':{        // Cursor up
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      DecLineIndex(j);
      m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;                         // get a pointer to the character buffer
      m_CurrentAttr = m_Screen[m_CursorPos.y].GetAttrAt(m_CursorPos.x);   // get the attribute at the cursor position
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'B':{       // Cursor down
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      IncLineIndex(j);
      m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;                         // get a pointer to the character buffer
      m_CurrentAttr = m_Screen[m_CursorPos.y].GetAttrAt(m_CursorPos.x);   // get the attribute at the cursor position
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'C':{       // Cursor right
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      m_CursorPos.x += j;
      if(m_CursorPos.x > MAXCOL) m_CursorPos.x = MAXCOL;
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'c':
      SendHostMessage("\x1b[?1;1c");
      m_EscState = ESC_PROC_NORMAL;
      break;
    case 	'D':{       // Cursor left
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      m_CursorPos.x -= j;
      if(m_CursorPos.x < 0) m_CursorPos.x = 0;
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'E':{       // Cursor to next line
      IncLineIndex(1);
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'F':{       // Special graphics
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'f':       // ASCII character set
    case 	'G':{
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'H':{       // Cursor home
      m_CursorPos.x = 0;
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'h':{       
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken);
      if(*pToken == '7') m_LineWrap = TRUE;      // Enable line wrap
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'J':{      
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(*pToken == '0'){       // Erase from cursor to end of screen
      }
      else if(*pToken == '1'){  // Erase from beginning of screen to cursor
      }
      else if(*pToken == '2'){  // Erase entire screen
        OnViewClear();
      }
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'K':{       
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken);
      if((pToken != nullptr) || (*pToken == '0')){       // Erase from cursor to end of line
        ZeroMemory(&m_pLineBuf[m_CursorPos.x], MAXCOL - m_CursorPos.x);
      }
      else if(*pToken == '1'){  // Erase from beginning of line to cursor
        FillMemory(&m_pLineBuf[0], m_CursorPos.x, ' ');
      }
      else if(*pToken == '2'){  // Erase entire cursor line
        ZeroMemory(&m_pLineBuf[m_CursorPos.x], MAXCOL);
      }
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'l':{       
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(*pToken == '7') m_LineWrap = FALSE;      // Enable line wrap
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 	'm':{
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      while(pToken != nullptr){
        j = atoi(pToken);
        if(j == 0) m_CurrentAttr = ATTR_DEFAULT;                            // restore defaults (green text on black background)
        else if(j == 1) m_CurrentAttr |= ATTR_BOLD;
        else if(j == 2) m_CurrentAttr |= ATTR_DIM;
        else if(j == 3) m_CurrentAttr |= ATTR_ITALIC;
        else if(j == 4) m_CurrentAttr |= ATTR_ULINE;
        else if(j == 5) m_CurrentAttr |= ATTR_BLINK;
        else if(j == 7) m_CurrentAttr |= ATTR_REVERSE;
        else if(j >= 30 && j <=37){
          m_CurrentAttr = (m_CurrentAttr & ~ATTR_FPALLET) | ((j - 30) << ATTR_FORE_SHIFT); // set text colour
//              TRACE("m 0x%x | %d.\n", m_CurrentAttr, atoi(pToken));
        }
        else if(j == 38){                                                   // set text colour
  				pToken = strtok_s(nullptr, CoSeDe, &pNextToken);
          int k = atoi(pToken); 
          if(k == 2) break;                                                 // Set text colour to an RGB value. Not serviced
          else if(k == 5){                                                  // Set text colour to index n in a 256-colour palette 
    				pToken = strtok_s(nullptr, CoSeDe, &pNextToken);
            m_CurrentAttr = (m_CurrentAttr & ~ATTR_FPALLET) | ((atoi(pToken) << ATTR_FORE_SHIFT)); 
//                TRACE("m5 0x%x | %d.\n", m_CurrentAttr, atoi(pToken));
          }
          else break;                                                       // error in control arguments
        }
        else if(j >= 40 && j <=47){                                          // set background colour
          m_CurrentAttr = (m_CurrentAttr & ~ATTR_BPALLET) | ((j - 40) << ATTR_BACK_SHIFT);
        }
        else if(j == 48){                                                   // set background colour
  				pToken = strtok_s(nullptr, CoSeDe, &pNextToken);
          int k = atoi(pToken); 
          if(k == 2) break;                                                 // Set background colour to an RGB value. Not serviced
          else if(k == 5){                                                  // Set background colour to index n in a 256-colour palette 
    				pToken = strtok_s(nullptr, CoSeDe, &pNextToken);
            m_CurrentAttr = (m_CurrentAttr & ~ATTR_BPALLET) | (atoi(pToken) << ATTR_BACK_SHIFT); 
          }
          else break;                                                       // error in control arguments
        }
        else if(j >= 90 && j <= 97){                                        // set bright text colour
          m_CurrentAttr = (m_CurrentAttr & ~ATTR_FPALLET) | ((j - 82) << ATTR_FORE_SHIFT); 
        }
        else if(j >= 100 && j <= 107){                                      // set bright background colour
          m_CurrentAttr = (m_CurrentAttr & ~ATTR_BPALLET) | ((j - 92) << ATTR_BACK_SHIFT);
        }
				pToken = strtok_s(nullptr, CoSeDe, &pNextToken);
      }
      m_Screen[m_CursorPos.y].PushAttr(m_CurrentAttr | ATTR_MARKER, m_CursorPos.x);  // save the attribute change points
//          TRACE("m:%d-%d :%d.\n", m_CursorPos.y, m_CursorPos.x, m_CurrentAttr);
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'n':{
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      int k = atoi(pToken);
      if(k == 5){
        SendHostMessage("\x1b[0n");
      }
      else if(k == 6){
        SendHostMessage("\x1b[%d;%dR", m_CursorPos.x, m_CursorPos.y);
      }
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'q':
    case 'S':{
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 's':{
      m_CursorSave = m_CursorPos;
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'T':{
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'u':{
      m_CursorPos = m_CursorSave;
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'y':{
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    case 'Z':{                                        // Identify compatibilities
      SendHostMessage("\x1b[/Z");
      m_EscState = ESC_PROC_NORMAL;
      break;
    }
    default:{                                         // save perameters until we get a control character
      if(m_ArgCount < MAX_ESC_ARGS){
        m_EscapeArgs[m_ArgCount++] = lpMessage[*i];
        m_EscapeArgs[m_ArgCount] = 0;                 // terminate
      }
      break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::IncLineIndex(int Inc)
{
  for(int i = 0; i < Inc; ++i){
    m_CursorPos.y = (m_CursorPos.y + 1) % MAXROW;
    if(m_CursorPos.y == m_TopRow){
      m_TopRow = (m_TopRow + 1) % MAXROW;
      m_Scrolled++;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::DecLineIndex(int Dec)
{
  for(int i = 0; i < Dec; ++i){
    if(m_CursorPos.y == m_TopRow) return;
    m_CursorPos.y = (m_CursorPos.y - 1) % MAXROW;
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnEditSettings() 
{
char buf[34];
CEVTSettingsDlg SettingsDlg;

  SettingsDlg.m_LineWrap = m_LineWrap;
  _itoa_s(m_Baud, buf, 34, 10);
  SettingsDlg.m_Baud = buf;
  _itoa_s(m_DataBits, buf, 34, 10);
  SettingsDlg.m_DataBits = buf;
  SettingsDlg.m_DTRDSR = m_DTRDSR;
  SettingsDlg.m_LocalEcho = m_LocalEcho;
  SettingsDlg.m_NewLine = m_NewLine;
  SettingsDlg.m_Parity = m_Parity;
  SettingsDlg.m_SerialPort = m_SerialPort;
  SettingsDlg.m_RTSCTS = m_RTSCTS;
  SettingsDlg.m_StopBits = m_StopBits;
  SettingsDlg.m_XONXOFF = m_XONXOFF;
  SettingsDlg.m_lfFont = m_lfFont;
  SettingsDlg.m_IsConnected = m_IsConnected;
  if (SettingsDlg.DoModal() == IDOK){
    m_LineWrap = SettingsDlg.m_LineWrap;
    m_Baud = atoi(SettingsDlg.m_Baud);
    m_DataBits = atoi(SettingsDlg.m_DataBits);
    m_DTRDSR = SettingsDlg.m_DTRDSR;
    m_LocalEcho = SettingsDlg.m_LocalEcho;
    m_NewLine = SettingsDlg.m_NewLine;
    m_Parity = SettingsDlg.m_Parity;
    if (!m_IsConnected) m_SerialPort = SettingsDlg.m_SerialPort;
    m_RTSCTS = SettingsDlg.m_RTSCTS;
    m_StopBits = SettingsDlg.m_StopBits;
    m_XONXOFF = SettingsDlg.m_XONXOFF;
     m_lfFont = SettingsDlg.m_lfFont;
    SetFontSize();
    POSITION firstViewPos = GetFirstViewPosition();
    CEVT100View *pView = (CEVT100View *)GetNextView(firstViewPos);
    pView->SetFont(&m_lfFont);
    if (m_IsConnected){
      if (!SetupConnection()){
      	AfxMessageBox(IDS_BADSETUP);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::SetFontSize()
{
CDC *pDC;
TEXTMETRIC tm;
CFont *pOldFont;
CFont font;

  font.CreateFontIndirect(&m_lfFont);
  pDC = AfxGetMainWnd()->GetDC();
  pOldFont = pDC->SelectObject(&font);
  pDC->GetTextMetrics(&tm);
  m_CharSize.cx = tm.tmAveCharWidth;
  m_CharSize.cy = tm.tmHeight + tm.tmExternalLeading;
  if (pOldFont) pDC->SelectObject(pOldFont);
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

	newlf = m_lfFont;
	CFontDialog fontDialog(&newlf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FIXEDPITCHONLY);
	if(fontDialog.DoModal() == IDOK){
		m_lfFont = newlf;
	}
}

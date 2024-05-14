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
	ON_COMMAND(IDM_SETTINGS, OnEditSettings)
	ON_COMMAND(IDM_VIEW_CLEAR, OnViewClear)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_CLEAR, OnUpdateViewClear)
	ON_COMMAND(IDM_VIEW_PAUSE, OnPause)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_PAUSE, OnUpdatePause)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CEVT100Doc::CEVT100Doc()
{
  m_bAutoWrap = TRUE;
  m_nBaud = 9600;
  m_nDataBits = 8;
  m_bDTRDSR = FALSE;
  m_bLocalEcho = FALSE;
  m_bNewLine = FALSE;
  m_nParity = 0;
  m_bRTSCTS = TRUE;
  m_nStopBits = 0;
  m_bXONXOFF = FALSE;
  m_IsConnected = FALSE;
  m_Pause = false;
  m_ShowCodes = false;
  m_sPort="COM1";
  ZeroMemory(&m_lfFont, sizeof(m_lfFont));
  m_lfFont.lfHeight = -9;
  m_lfFont.lfWeight = FW_DONTCARE;
  m_lfFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
  strcpy_s(m_lfFont.lfFaceName, sizeof(m_lfFont.lfFaceName), "FixedSys");
  ZeroMemory(&m_osRead, sizeof(OVERLAPPED));
  ZeroMemory(&m_osWrite, sizeof(OVERLAPPED));
  m_hPostEvent = NULL;
  m_pThread = NULL;
  m_pTermWnd = NULL;
  m_CursorPos.SetPoint(0, 0);
  m_CursorSave.SetPoint(0, 0);
  m_nTopRow = 0;
  m_CharSize.cx = 8;
  m_CharSize.cy = 15;
  m_Scrolled = 0;
  m_InBlock = new BYTE[MAXBLOCK + 1];
  m_Escape = false;
  m_ArgCount = 0;
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
    &m_bXONXOFF, &m_bLocalEcho,
    &m_bNewLine, &m_bAutoWrap,
    &m_bRTSCTS, &m_nDataBits,
    &m_bDTRDSR, &m_nParity,
    &m_nStopBits, &m_nBaud,
    &temp, 10);
  if(strlen(temp) > 3 ) m_sPort = temp;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::SaveSystemVars()
{
char szBuffer[100];

  sprintf_s(szBuffer, sizeof(szBuffer), szFormat,
    m_bXONXOFF, m_bLocalEcho,
    m_bNewLine, m_bAutoWrap,
    m_bRTSCTS, m_nDataBits,
    m_bDTRDSR, m_nParity,
    m_nStopBits, m_nBaud,
    (const char *)m_sPort);
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
	cstr.Format(_T("\\\\.\\%s"), m_sPort);
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
    ((CMainFrame*)AfxGetMainWnd())->SetWindowTitle(m_sPort);
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
  dcb.BaudRate = m_nBaud;
  dcb.ByteSize = m_nDataBits;
  dcb.fParity = TRUE;
  switch (m_nParity){
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
  switch (m_nStopBits){
  	case 0: dcb.StopBits = ONESTOPBIT; break;
	  case 1: dcb.StopBits = ONE5STOPBITS; break;
  	case 2: dcb.StopBits = TWOSTOPBITS; break;
	  default: ASSERT(FALSE);
  }
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDsrSensitivity = m_bDTRDSR;
  dcb.fDtrControl = m_bDTRDSR ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_ENABLE;
  dcb.fOutxCtsFlow = m_bRTSCTS;
  dcb.fRtsControl = m_bRTSCTS ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;
  dcb.fInX = dcb.fOutX = m_bXONXOFF;
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
int i = 0, j;
char *pToken = nullptr, *pNextToken = nullptr;

  if(m_pTermWnd == NULL) return false;
//  lpMessage[nLength] = 0;
//  TRACE(":%s.\n", lpMessage);
	while(i < nLength){
    if(!m_Escape){
      switch (lpMessage[i]){
        case ASCII_ESC:
          if(m_ShowCodes) break;
				  m_Escape = true;
          break;
        case ASCII_CR:
					  m_CursorPos.x = 0;
				  break;
        case ASCII_BEL:
      	  break;
        case ASCII_BS:
				  if(m_CursorPos.x > 0){
					  m_pLineBuf[m_CursorPos.x] = 0; // truncate current line
					  m_pLineBuf[--m_CursorPos.x] = ' '; // erase old character
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
      	  if(m_CursorPos.x >= MAXCOL) FormatScreenData("\n");
//			 	  m_pTermWnd->Invalidate(FALSE);
      	  break;
      }
    }
    else{
      switch(lpMessage[i]){
        case 	'A':{        // Cursor up
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          if(pToken != nullptr){      
            j = atoi(pToken);
            DecLineIndex(j);
            m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;                         // get a pointer to the character buffer
            m_CurrentAttr = m_Screen[m_CursorPos.y].GetAttrAt(m_CursorPos.x);   // get the attribute at the cursor position
          }
          m_Escape = false;
          break;
        }
        case 	'B':{       // Cursor down
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          if(pToken != nullptr){     
            j = atoi(pToken);
            IncLineIndex(j);
            m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;                         // get a pointer to the character buffer
            m_CurrentAttr = m_Screen[m_CursorPos.y].GetAttrAt(m_CursorPos.x);   // get the attribute at the cursor position
          }
          m_Escape = false;
          break;
        }
        case 	'C':{       // Cursor right
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          if(pToken != nullptr){      
            j = atoi(pToken);
            m_CursorPos.x += j;
            if(m_CursorPos.x > MAXCOL) m_CursorPos.x = MAXCOL;
          }
          m_Escape = false;
          break;
        }
        case 	'D':{       // Cursor left
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          if(pToken != nullptr){     
            j = atoi(pToken);
            m_CursorPos.x -= j;
            if(m_CursorPos.x < 0) m_CursorPos.x = 0;
          }
          m_Escape = false;
          break;
        }
        case 	'E':{       // Cursor to next line
          IncLineIndex(1);
          m_Escape = false;
          break;
        }
        case 	'F':{       // Special graphics
          m_Escape = false;
          break;
        }
        case 	'G':
        case 	'f':{       // ASCII character set
          m_Escape = false;
          break;
        }
        case 	'H':{       // Cursor home
          m_CursorPos.x = 0;
          m_Escape = false;
          break;
        }
        case 	'J':{      
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          if(*pToken == '0'){       // Erase from cursor to end of screen
          }
          else if(*pToken == '1'){  // Erase from beginning of screen to cursor
          }
          else if(*pToken == '2'){  // Erase entire screen
            OnViewClear();
          }
          m_Escape = false;
          break;
        }
        case 	'K':{       
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          if(*pToken == '0'){       // Erase from cursor to end of line
         	  ZeroMemory(&m_pLineBuf[m_CursorPos.x], MAXCOL - m_CursorPos.x);
          }
          else if(*pToken == '1'){  // Erase from beginning of line to cursor
         	  FillMemory(&m_pLineBuf[0], m_CursorPos.x, ' ');
          }
          else if(*pToken == '2'){  // Erase entire cursor line
         	  ZeroMemory(&m_pLineBuf[m_CursorPos.x], MAXCOL);
          }
          m_Escape = false;
          break;
        }
        case 	'S':{
          m_Escape = false;
          break;
        }
        case 	'T':{
          m_Escape = false;
          break;
        }
        case 	's':{
          m_CursorSave = m_CursorPos;
          m_Escape = false;
          break;
        }
        case 	'u':{
          m_CursorPos = m_CursorSave;
          m_Escape = false;
          break;
        }
        case 	'm':{
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
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
              m_CurrentAttr = (m_CurrentAttr & ATTR_TPALLET) | ((j - 30) << ATTR_TEXT_SHIFT); // set text colour
            }
            else if(j == 38){                                                   // set text colour
  						pToken = strtok_s(nullptr, "[;", &pNextToken);
              int k = atoi(pToken); 
              if(k == 2) break;                                                 // Set text colour to an RGB value. Not serviced
              else if(k == 5){                                                  // Set text colour to index n in a 256-colour palette 
    						pToken = strtok_s(nullptr, "[;", &pNextToken);
                m_CurrentAttr = (m_CurrentAttr & ATTR_TPALLET) | ((atoi(pToken) << ATTR_TEXT_SHIFT)); 
              }
              else break;                                                       // error in control arguments
            }
            else if(j >= 40 && j <=47){                                          // set background colour
              m_CurrentAttr = (m_CurrentAttr & ATTR_BPALLET) | ((j - 40) << ATTR_BACK_SHIFT);
            }
            else if(j == 48){                                                   // set background colour
  						pToken = strtok_s(nullptr, "[;", &pNextToken);
              int k = atoi(pToken); 
              if(k == 2) break;                                                 // Set background colour to an RGB value. Not serviced
              else if(k == 5){                                                  // Set background colour to index n in a 256-colour palette 
    						pToken = strtok_s(nullptr, "[;", &pNextToken);
                m_CurrentAttr = (m_CurrentAttr & ATTR_BPALLET) | (atoi(pToken) << ATTR_BACK_SHIFT); 
              }
              else break;                                                       // error in control arguments
            }
            else if(j >= 90 && j <= 97){                                        // set text colour
              m_CurrentAttr = (m_CurrentAttr & ATTR_TPALLET) | ((j - 30) << ATTR_TEXT_SHIFT); 
            }
            else if(j >= 100 && j <= 107){                                      // set bright background colour
              m_CurrentAttr = (m_CurrentAttr & ATTR_BPALLET) | ((j - 40) << ATTR_BACK_SHIFT);
            }
						pToken = strtok_s(nullptr, "[;", &pNextToken);
          }
          m_Screen[m_CursorPos.y].PushAttr(m_CurrentAttr | ATTR_MARKER, m_CursorPos.x);  // save the attribute change points
          TRACE("m:%d-%d :%d.\n", m_CursorPos.y, m_CursorPos.x, m_CurrentAttr);
          m_Escape = false;
          break;
        }
        case 'n':{
          pToken = strtok_s(m_EscapeArgs, "[;", &pNextToken); // get a token
          int k = atoi(pToken);
          if(k == 5){
            SendHostMessage("\x1b[0n");
          }
          else if(k == 6){
            SendHostMessage("\x1b[%d;%dR", m_CursorPos.x, m_CursorPos.y);
          }
          m_Escape = false;
          break;
        }
        case 'c':
          SendHostMessage("\x1b[?1;1c");
          m_Escape = false;
          break;
        case 'Z':{                                        // Identify compatibilities
          SendHostMessage("\x1b[/Z");
          m_Escape = false;
          break;
        }
        default:{
          if(m_ArgCount < MAX_ESC_ARGS){
            m_EscapeArgs[m_ArgCount++] = lpMessage[i];
            m_EscapeArgs[m_ArgCount] = 0;                 // terminate
          }
          break;
        }
      }
      if(!m_Escape) m_ArgCount = 0;
    }
    i++;
  }
  m_pTermWnd->ScrollToCursor(true);
  return true;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::IncLineIndex(int Inc)
{
  for(int i = 0; i < Inc; ++i){
    m_CursorPos.y = (m_CursorPos.y + 1) % MAXROW;
    if(m_CursorPos.y == m_nTopRow){
      m_nTopRow = (m_nTopRow + 1) % MAXROW;
      m_Scrolled++;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::DecLineIndex(int Dec)
{
  for(int i = 0; i < Dec; ++i){
    if(m_CursorPos.y == m_nTopRow) return;
    m_CursorPos.y = (m_CursorPos.y - 1) % MAXROW;
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnEditSettings() 
{
char buf[34];
CEVTSettingsDlg SettingsDlg;

  SettingsDlg.m_bAutoWrap = m_bAutoWrap;
  _itoa_s(m_nBaud, buf, 34, 10);
  SettingsDlg.m_sBaud = buf;
  _itoa_s(m_nDataBits, buf, 34, 10);
  SettingsDlg.m_sDataBits = buf;
  SettingsDlg.m_bDTRDSR = m_bDTRDSR;
  SettingsDlg.m_bLocalEcho = m_bLocalEcho;
  SettingsDlg.m_bNewLine = m_bNewLine;
  SettingsDlg.m_nParity = m_nParity;
  SettingsDlg.m_sPort = m_sPort;
  SettingsDlg.m_bRTSCTS = m_bRTSCTS;
  SettingsDlg.m_nStopBits = m_nStopBits;
  SettingsDlg.m_bXONXOFF = m_bXONXOFF;
  SettingsDlg.m_lfFont = m_lfFont;
  SettingsDlg.m_IsConnected = m_IsConnected;
  if (SettingsDlg.DoModal() == IDOK){
    m_bAutoWrap = SettingsDlg.m_bAutoWrap;
    m_nBaud = atoi(SettingsDlg.m_sBaud);
    m_nDataBits = atoi(SettingsDlg.m_sDataBits);
    m_bDTRDSR = SettingsDlg.m_bDTRDSR;
    m_bLocalEcho = SettingsDlg.m_bLocalEcho;
    m_bNewLine = SettingsDlg.m_bNewLine;
    m_nParity = SettingsDlg.m_nParity;
    if (!m_IsConnected) m_sPort = SettingsDlg.m_sPort;
    m_bRTSCTS = SettingsDlg.m_bRTSCTS;
    m_nStopBits = SettingsDlg.m_nStopBits;
    m_bXONXOFF = SettingsDlg.m_bXONXOFF;
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
  m_nTopRow = 0;
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

void CEVT100Doc::OnPause() 
{
  m_Pause = !m_Pause;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::OnUpdatePause(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(m_Pause);
	pCmdUI->Enable(m_IsConnected);
}

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

#include "stdafx.h"                                       // includes to satisfy intelisense
#include "EVT100Defs.h"
#include "EVT100Doc.h"
#include "EVT100View.h"

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
            if(m_SoftWrap.Line) FormatScreenData("\n");     // start a new line
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
int j, ExitState = ESC_PROC_NORMAL;                                       // default exit state

  switch(lpMessage[*i]){
    case 	'A':{                                                           // Cursor up
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      DecLineIndex(j);
      m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;                         // get a pointer to the character buffer
      m_CurrentAttr = m_Screen[m_CursorPos.y].GetAttrAt(m_CursorPos.x);   // get the attribute at the cursor position
      break;
    }
    case 	'B':{                                                           // Cursor down
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      IncLineIndex(j);
      m_pLineBuf = m_Screen[m_CursorPos.y].m_Str;                         // get a pointer to the character buffer
      m_CurrentAttr = m_Screen[m_CursorPos.y].GetAttrAt(m_CursorPos.x);   // get the attribute at the cursor position
      break;
    }
    case 	'C':{                                                           // Cursor right
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      m_CursorPos.x += j;
      if(m_CursorPos.x > MAXCOL) m_CursorPos.x = MAXCOL;
      break;
    }
    case 'c':
      SendHostMessage("\x1b[?1;1c");
      break;
    case 	'D':{                                                           // Cursor left
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(pToken != nullptr) j = atoi(pToken);
      else j = 1;
      m_CursorPos.x -= j;
      if(m_CursorPos.x < 0) m_CursorPos.x = 0;
      break;
    }
    case 	'E':{                                                           // Cursor to next line
      IncLineIndex(1);
      break;
    }
    case 	'F':{                                                           // Special graphics
      break;
    }
    case 	'f':                                                            // ASCII character set
    case 	'G':{
      break;
    }
    case 	'H':{                                                           // Cursor home
      m_CursorPos.x = 0;
      break;
    }
    case 	'h':{       
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken);
      if(*pToken == '7') m_SoftWrap.View = true;                          // Enable line wrap
      break;
    }
    case 	'J':{      
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if((pToken == nullptr) || (*pToken == '0')){                        // Erase from cursor to end of screen
        ScreenErase(EM_FROM_CURSOR);
      }
      else if(*pToken == '1'){                                            // Erase from beginning of screen to cursor
        ScreenErase(EM_TO_CURSOR);
      }
      else if(*pToken == '2'){                                            // Erase entire screen
        ScreenErase(EM_ALL);
      }
      break;
    }
    case 	'K':{       
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken);
      if((pToken == nullptr) || (*pToken == '0')){                        // Erase from cursor to end of line
        ZeroMemory(&m_pLineBuf[m_CursorPos.x], MAXCOL - m_CursorPos.x);
      }
      else if(*pToken == '1'){                                            // Erase from beginning of line to cursor
        FillMemory(&m_pLineBuf[0], m_CursorPos.x, ' ');
      }
      else if(*pToken == '2'){                                            // Erase entire cursor line
        ZeroMemory(&m_pLineBuf[m_CursorPos.x], MAXCOL);
      }
      break;
    }
    case 	'l':{       
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      if(*pToken == '7') m_SoftWrap.Line = false;                         // Enable line wrap
      break;
    }
    case 	'm':{
      pToken = strtok_s(m_EscapeArgs, CoSeDe, &pNextToken); 
      while(pToken != nullptr){
        j = atoi(pToken);
        if(j == 0) m_CurrentAttr = ATTR_DEFAULT;                          // restore defaults (green text on black background)
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
      break;
    }
    case 'q':
    case 'S':{
      break;
    }
    case 's':{
      m_CursorSave = m_CursorPos;
      break;
    }
    case 'T':{
      break;
    }
    case 'u':{
      m_CursorPos = m_CursorSave;
      break;
    }
    case 'y':{
      break;
    }
    case 'Z':{                                        // Identify compatibilities
      SendHostMessage("\x1b[/Z");
      break;
    }
    default:{                                         // save perameters until we get a control character
      if(m_ArgCount < MAX_ESC_ARGS){
        m_EscapeArgs[m_ArgCount++] = lpMessage[*i];
        m_EscapeArgs[m_ArgCount] = 0;                 // terminate the buffer
        ExitState = m_EscState;                       // more parameters
      }
      break;
    }
  }
  m_EscState = ExitState;
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::IncLineIndex(int Inc)
{
  for(int i = 0; i < Inc; ++i){
    m_CursorPos.y = (m_CursorPos.y + 1) % MAXROW;       // buffers arranged as a cylinder
    if(m_CursorPos.y == m_TopRow){                      // scroll if looped
      m_TopRow = (m_TopRow + 1) % MAXROW;
      m_Scrolled++;                                     // view should set scroll bar
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CEVT100Doc::DecLineIndex(int Dec)
{
  for(int i = 0; i < Dec; ++i){
    if(m_CursorPos.y == m_TopRow) return;               // only as far as the top line
    m_CursorPos.y = (m_CursorPos.y - 1) % MAXROW;
  }
}


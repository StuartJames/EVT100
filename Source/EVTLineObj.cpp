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
#include "EVT100Defs.h"
#include "EVTLineObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CLineObj::CLineObj() noexcept
{
  ZeroMemory(&m_Str[0], MAXCOL);
}

/////////////////////////////////////////////////////////////////////////////

CLineObj::~CLineObj()
{
  m_AttrArray.RemoveAll();
  m_PosArray.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////

void CLineObj::Clear(void)
{
  m_AttrArray.RemoveAll();
  m_PosArray.RemoveAll();
  ZeroMemory(&m_Str[0], MAXCOL);
  m_IsUsed = false;
}

/////////////////////////////////////////////////////////////////////////////

void CLineObj::PushAttr(UINT Attr, UINT Index)
{

  for(int i = 0; i < m_PosArray.GetCount(); ++i){
    if(Index == m_PosArray.GetAt(i)){
      m_AttrArray.SetAt(i, Attr);
      return;
    }
  }
  m_AttrArray.Add(Attr);
  m_PosArray.Add(Index);
  m_IsUsed = true;
}

/////////////////////////////////////////////////////////////////////////////

void CLineObj::GetAttr(int Pos, UINT *pAttr, UINT *pIndex)
{
  *pAttr = m_AttrArray.GetAt(Pos);
  *pIndex =  m_PosArray.GetAt(Pos);
}

/////////////////////////////////////////////////////////////////////////////

UINT CLineObj::GetAttrAt(UINT Pos)
{
UINT Attr = ATTR_DEFAULT;

  for(int i = 0; i < m_PosArray.GetCount(); ++i){
    if(m_PosArray.GetAt(i) < Pos) Attr = m_AttrArray.GetAt(i);
    else break;
  }
  return Attr;
}


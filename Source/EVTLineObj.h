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

class CLineObj
{
public:
									CLineObj() noexcept;
	virtual					~CLineObj();
	void						Clear(void);
	void						PushAttr(UINT Attr, UINT Pos);
	void						GetAttr(int Pos, UINT *pAttr, UINT *pIndex);
	int							GetCount(){ return (int)m_AttrArray.GetCount(); };
	bool						IsUsed(){ return m_IsUsed; };
	UINT						GetAttrAt(UINT Pos);

	char						m_Str[MAXCOL];

private:

	CUIntArray			m_AttrArray;
	CUIntArray			m_PosArray;
	bool						m_IsUsed;
};


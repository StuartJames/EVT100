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

#ifndef _EMADEFS_H
#define _EMADEFS_H

#define EM_MAXFILENAMELENGTH	512 // maximum open/save filename buffer length

// Download types
#define DLT_INV		0
#define DLT_BIN		1
#define DLT_HEX		2
#define DLT_EXE		3

#define SPLASH_TIME 5

// Message screen constants
constexpr auto MAXROW                 = 200;
constexpr auto MAXCOL                 = 255;
constexpr auto TABSTOP                = 8;
constexpr auto MAXMRU                 = 8;

constexpr auto ATTR_MARKER		        = 0x00000001;
constexpr auto ATTR_BOLD		          = 0x00000002;
constexpr auto ATTR_DIM			          = 0x00000004;
constexpr auto ATTR_ITALIC	          = 0x00000008;
constexpr auto ATTR_ULINE							= 0x00000010;
constexpr auto ATTR_BLINK							= 0x00000020;
constexpr auto ATTR_REVERSE						= 0x00000040;
constexpr auto ATTR_FCOLOR	          = 0x00000700;
constexpr auto ATTR_BCOLOR	          = 0x00070000;
constexpr auto ATTR_FPALLET           = 0x0000FF00;
constexpr auto ATTR_BPALLET           = 0x00FF0000;
constexpr auto ATTR_DEFAULT           = 0x00000201; // default pale green text on black background
constexpr auto ATTR_FORE_SHIFT        = 8;
constexpr auto ATTR_BACK_SHIFT        = 16;

// ASCII characters that receive special processing
constexpr auto ASCII_BEL              = 0x07;
constexpr auto ASCII_BS               = 0x08;
constexpr auto ASCII_TAB              = 0x09;
constexpr auto ASCII_LF               = 0x0A;
constexpr auto ASCII_VTAB             = 0x0B;
constexpr auto ASCII_CR               = 0x0D;
constexpr auto ASCII_XON              = 0x11;
constexpr auto ASCII_XOFF             = 0x13;
constexpr auto ASCII_ESC	            = 0x1B;

constexpr auto MAX_ESC_ARGS           = 8;


// System Message text colours
#define MSGTEXT_WHITE			'0'
#define MSGTEXT_RED				'1'
#define MSGTEXT_GREEN			'2'
#define MSGTEXT_BLUE			'3'
#define MSGTEXT_YELLOW		'4'
#define MSGTEXT_CYAN		  '5'
#define MSGTEXT_MAUVE	  	'6'

typedef enum EscState_e{
	ESC_PROC_NORMAL,
	ESC_PROC_ESCAPE,
	ESC_PROC_CSI,
	ESC_LOSE_NEXT,
	ESC_PROC_STRING
} EscState_e;

typedef struct Wrap_t{
	bool Line;
  bool View;
} Wrap_t;


#endif
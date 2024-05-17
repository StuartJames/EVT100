/*
 *        Copyright (c) 2011-2020 HydraSystems.
 *
 *  This software is copyrighted by and is the sole property of HydraSystems.
 *  All rights, title, ownership, or other interests in the software
 *  remain the property of HydraSystems.
 *  This software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorised use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  Based on a design by Michael Haardt
 *
 * Edit Date/Ver   Edit Description
 * ==============  ===================================================
 * SJ   19/08/2011  Original
 *
 *
 */


#pragma once

constexpr auto VERSION_BUFF_SIZE  = 50;
constexpr auto HOLD_TIMER         = 1;
constexpr auto HIDE_TIMER         = 2;
constexpr auto SMAX_PATH          = 1050;

/////////////////////////////////////////////////////////////////////////////

typedef struct tagRegions{
	int				VerticalOffset;		// empty space between top 3-D border and product name
	int				VerticalHeight;		// empty space between bottom 3-D border and bottom of product name
	int				LeftMargin;				// distance from left side to place name, company, copyright and version
	int				RightMargin;			// distance from right side to place name, company, copyright and version
	LPCTSTR		FontName;					// name of font for application name
	SIZE			PointSize;				// point size used for the application name	
	COLORREF	TextColor;				// color used for text
	int				TextFlags;
	bool			Enabled;
} TXTREGN;

typedef TXTREGN FAR* LPTXTREGN;

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

class CSplashWnd : public CWnd
{
protected:

	CSplashWnd();
	virtual					~CSplashWnd();

	CBitmap					m_bitmap;         // Splash screen image.

	static CSplashWnd*	m_pSplashWnd;     // Points to the splash screen.

public:

	enum Regions {PRODUCT_REGION, GENERAL_REGION, STATUS_REGION, COMPANY_REGION};

	static void			InitialiseRegion(int Region, int VertOffset, int VertHeight, int LeftMargin, int RightMargin, int Flags, COLORREF TextColor);
	static void			InitialiseRegions(TXTREGN *pRgns, bool KeyHide);
	static BOOL			ShowSplashScreen(UINT HideTimeOut, UINT uBitmapID, CWnd* pParentWnd = nullptr);
	static void			ShowStatus(LPCTSTR statusMessage, BOOL FinalMessage = FALSE);
	static BOOL			PreTranslateAppMessage(MSG* pMsg);
	static void		  GetVersionString(CString& version){ version = m_pVersion;};

private:
	static LPTSTR		m_pProductName;
	static LPTSTR		m_pCompanyName;
	static LPTSTR		m_pVersion;
	static LPTSTR		m_pCopyright;
	static LPTSTR		m_pComments;
	static LPTSTR		m_pStatusMessage;

	static TXTREGN	m_Regions[4];

	static UINT			m_DisplayTimeout;
	static bool			m_KeyHide;

protected:
	void						HideSplashScreen();
	virtual void		PostNcDestroy();
	void						ClearMessageQueue();
	void						DisplayCompanyName(HDC paintDC, int windowWidth, int windowHeight);
	void						DisplayProductName(HDC paintDC, int windowWidth, int windowHeight);
	void						DisplayStatusLine(HDC paintDC, int windowWidth, int windowHeight);
	void						DisplayBody(HDC paintDC, int windowWidth, int windowHeight);
	HFONT						CreatePointFont(int pointSize, LPCTSTR fontName, HDC dc);
	SIZE						FindFontPointSize(HDC paintDC, LPCTSTR fontName, char **stringsToCheck, int numberOfStringsToCheck, SIZE maximumSize);
	void						DrawEmbosedText(HDC paintDC, char *pString, COLORREF Colour, CRect DispRect, int flags);
	void						GetStrings();

	afx_msg void		OnPaint();
	afx_msg void		OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


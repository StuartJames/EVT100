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


#include "stdafx.h"
#include "SplashWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static char BASED_CODE szMainName[] = _T("HydraSystems");
static char BASED_CODE szFullVersion[] = _T("Version %d.%d.%d  build %d.");
static char BASED_CODE szVersionThree[] = _T("Version %d.%d.%d.");
static char BASED_CODE szVersionTwo[] = _T("Version %d.%d.");
static char BASED_CODE szVersionOne[] = _T("Version %d.");

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

CSplashWnd*		CSplashWnd::m_pSplashWnd;
LPTSTR				CSplashWnd::m_pProductName = NULL;
LPTSTR				CSplashWnd::m_pCompanyName = NULL;
LPTSTR				CSplashWnd::m_pVersion = NULL;
LPTSTR				CSplashWnd::m_pCopyright = NULL;
LPTSTR				CSplashWnd::m_pComments = NULL;
LPTSTR				CSplashWnd::m_pStatusMessage = NULL;

// create rectangle that product name has to fit in
TXTREGN				CSplashWnd::m_Regions[] = {
	{100, 20, 40, 20, _T("Arial"), {-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_CENTER | DT_SINGLELINE, true},
	{190, 80, 40, 20, _T("Verdana"), {-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true},
	{290, 10, 10, 10, _T("Verdana"),{-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true},
	{100, 20, 40, 20, _T("Arial"), {-1, -1}, RGB(0, 0, 0), DT_VCENTER | DT_SINGLELINE, true }};

UINT					CSplashWnd::m_DisplayTimeout = 10;			// Ten seconds hide timeout
bool					CSplashWnd::m_KeyHide = true;

CSplashWnd::CSplashWnd()
{
	GetStrings();
}

CSplashWnd::~CSplashWnd()
{
	if(m_pProductName != NULL) delete m_pProductName;
	if(m_pCompanyName != NULL) delete m_pCompanyName;
	if(m_pVersion != NULL) delete m_pVersion;
	if(m_pCopyright != NULL) delete m_pCopyright;
	if(m_pComments != NULL) delete m_pComments;
	if(m_pStatusMessage != NULL) delete m_pStatusMessage;
	ASSERT(m_pSplashWnd == this);		// Clear the static window pointer.
	m_pSplashWnd = NULL;
	m_pProductName = NULL;
	m_pCompanyName = NULL;
	m_pVersion = NULL;
	m_pCopyright = NULL;
	m_pComments = NULL;
	m_pStatusMessage = NULL;
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void CSplashWnd::InitialiseRegion(int Region, int VertOffset, int VertHeight, int LeftMargin, int RightMargin, int Flags, COLORREF TextColor)
{
	ASSERT(Region <= COMPANY_REGION);
	m_Regions[Region].VerticalOffset = VertOffset;
	m_Regions[Region].VerticalHeight = VertHeight;
	m_Regions[Region].LeftMargin = LeftMargin;
	m_Regions[Region].RightMargin = RightMargin;
	m_Regions[Region].TextColor = TextColor;
	m_Regions[Region].TextFlags = Flags;
	if(Region == COMPANY_REGION)	m_KeyHide = false;
}


void CSplashWnd::InitialiseRegions(TXTREGN *pRgns, bool KeyHide)
{
LPTXTREGN	pRegions;

	ASSERT(pRgns != NULL);
	for(int i = 0; i <= COMPANY_REGION; ++i){
		pRegions = (pRgns + i);
		m_Regions[i].VerticalOffset = pRegions->VerticalOffset;
		m_Regions[i].VerticalHeight = pRegions->VerticalHeight;
		m_Regions[i].LeftMargin = pRegions->LeftMargin;
		m_Regions[i].RightMargin = pRegions->RightMargin;
		m_Regions[i].TextColor = pRegions->TextColor;
		m_Regions[i].TextFlags = pRegions->TextFlags;
		m_Regions[i].FontName = pRegions->FontName;
	}
	m_KeyHide = KeyHide;
}

// show main frame before this statement so that correct screen is used
BOOL CSplashWnd::ShowSplashScreen(UINT HideTimeOut, UINT BitmapID, CWnd* pParentWnd /*= NULL*/)
{
BITMAP bm;

	ASSERT(BitmapID);
	if(m_pSplashWnd != NULL) return FALSE;
	m_pSplashWnd = new CSplashWnd;	// Allocate a new splash screen, and create the window.
	if(!m_pSplashWnd->m_bitmap.LoadBitmap(BitmapID)){
		return FALSE;
	}
	m_pSplashWnd->m_bitmap.GetBitmap(&bm);
	CString strWndClass = AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	if(!m_pSplashWnd->CreateEx(0, strWndClass, NULL, WS_POPUP | WS_VISIBLE,	0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL)){
		TRACE0("Failed to create splash screen.\n");
		delete m_pSplashWnd;
		return FALSE;
	}
	m_pSplashWnd->CenterWindow(pParentWnd);	// Center the window.
	m_pSplashWnd->UpdateWindow();
	if(HideTimeOut > 0){
		m_DisplayTimeout = HideTimeOut * 1000;
		m_pSplashWnd->SetTimer(HIDE_TIMER, m_DisplayTimeout, NULL);	// Set a timer to destroy the splash screen.
	}
	return TRUE;
}


void CSplashWnd::ShowStatus(LPCTSTR statusMessage, BOOL FinalMessage)
{
	if(m_pSplashWnd == NULL )	return;
	if(FinalMessage){	// Reset the timer to destroy the splash screen.
		m_pSplashWnd->KillTimer(HIDE_TIMER);
		m_pSplashWnd->SetTimer(HIDE_TIMER, m_DisplayTimeout, NULL);
		m_KeyHide = TRUE;	// enable key input hide
	}
	if(m_pStatusMessage){
		delete [] m_pStatusMessage;
		m_pStatusMessage = NULL;
	}
	if(statusMessage){
		int len = (int)strlen(statusMessage) + 1;
		m_pStatusMessage = new char[len];
		strcpy_s(m_pStatusMessage, len, statusMessage);
	}
	m_pSplashWnd->UpdateWindow();
	m_pSplashWnd->InvalidateRect(NULL, FALSE);
	m_pSplashWnd->ClearMessageQueue();

}


void CSplashWnd::ClearMessageQueue()
{
MSG msg;

  while(PeekMessage(&msg, m_hWnd,  0, 0, PM_REMOVE)){ 
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


BOOL CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if(m_pSplashWnd == NULL)	return FALSE;
	HWND hwnd = m_pSplashWnd->GetSafeHwnd();
	if((m_pSplashWnd->m_KeyHide == true) &&
			(pMsg->message == WM_KEYDOWN ||	// If we get a keyboard or mouse message, hide the splash screen.
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)){
		m_pSplashWnd->HideSplashScreen();
		if(hwnd == pMsg->hwnd) return TRUE;	// was it for us
	}
	return FALSE;	// message not handled
}


void CSplashWnd::PostNcDestroy()
{
	delete this;	// Free the C++ class.
}


void CSplashWnd::OnPaint()
{
CPaintDC paintDC(this);
CDC dcImage;
BITMAP bm;

	if(dcImage.CreateCompatibleDC(&paintDC)){
		m_bitmap.GetBitmap(&bm);
		CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);		// Paint the image.
		paintDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
		dcImage.SelectObject(pOldBitmap);
		DisplayCompanyName(paintDC, bm.bmWidth, bm.bmHeight);
		DisplayProductName(paintDC, bm.bmWidth, bm.bmHeight);
		DisplayBody(paintDC, bm.bmWidth, bm.bmHeight);
		if(m_pStatusMessage) DisplayStatusLine(paintDC, bm.bmWidth, bm.bmHeight);
	}
}


void CSplashWnd::OnTimer(UINT_PTR IDEvent)
{
	switch(IDEvent){
		case HOLD_TIMER:
			m_pSplashWnd->SetTimer(HIDE_TIMER, m_DisplayTimeout, NULL);	// Set a timer to destroy the splash screen.
			break;
		case HIDE_TIMER:
			HideSplashScreen();	// Destroy the splash screen window.
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::HideSplashScreen()
{
	DestroyWindow();	// Destroy the window, and update the mainframe.
	AfxGetMainWnd()->UpdateWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::GetStrings()
{	
DWORD dwHandle;			// ignored 
char *buf;				// pointer to buffer to receive file-version info.
LPVOID lplpBuffer;
UINT Length;
struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslate;
char SubBlock[51];

	char moduleName[SMAX_PATH];
	::GetModuleFileName(NULL, moduleName, SMAX_PATH); 	// get name of executable
	DWORD verSize = GetFileVersionInfoSize(moduleName, &dwHandle);	// Get the size of the version information.
	if(verSize != 0){
		buf = new char[verSize + 1];
		BOOL res = GetFileVersionInfo(moduleName,	NULL, verSize, buf);
		ASSERT(res);
		VerQueryValue(buf, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &Length);  // get the language and code page
		sprintf_s(SubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\ProductName"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
		if(VerQueryValue(buf, SubBlock, &lplpBuffer, &Length)){
			m_pProductName = new TCHAR[Length + 1];
			strcpy_s(m_pProductName, Length + 1, (char*)lplpBuffer);
		}
		sprintf_s(SubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\CompanyName"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
		if(VerQueryValue(buf, SubBlock, &lplpBuffer, &Length)){
			m_pCompanyName = new TCHAR[Length + 1];
			strcpy_s(m_pCompanyName, Length + 1, (char*)lplpBuffer);
		}
		sprintf_s(SubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\LegalCopyright"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
		if(VerQueryValue(buf, SubBlock, &lplpBuffer, &Length)){
			m_pCopyright = new TCHAR[Length + 1];
			strcpy_s(m_pCopyright, Length + 1, (char*)lplpBuffer);
		}
		sprintf_s(SubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\Comments"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
		if(VerQueryValue(buf, SubBlock, &lplpBuffer, &Length)){
			m_pComments = new TCHAR[Length + 1];
			strcpy_s(m_pComments, Length + 1, (char*)lplpBuffer);
		}
		sprintf_s(SubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
		if(VerQueryValue(buf, SubBlock, &lplpBuffer, &Length)){
			char numberString[VERSION_BUFF_SIZE];
      int versionNumbers[4] = {0, 0, 0, 0};
      char* ptr = strchr((char*)lplpBuffer, '.');
      while(ptr != NULL) {
        *ptr = ','; // change dp to comma
        ptr = strchr((char*)lplpBuffer, '.');
      }
      int numberOfValues = sscanf_s((char*)lplpBuffer, "%d,%d,%d,%d", &versionNumbers[0], &versionNumbers[1], &versionNumbers[2], &versionNumbers[3]);
			if(versionNumbers[3] != 0) sprintf_s(numberString, VERSION_BUFF_SIZE, szFullVersion, versionNumbers[0], versionNumbers[1], versionNumbers[2], versionNumbers[3]);
			else if(versionNumbers[2] != 0)	sprintf_s(numberString, VERSION_BUFF_SIZE, szVersionThree, versionNumbers[0], versionNumbers[1], versionNumbers[2]);
  		else if(versionNumbers[1] != 0)	sprintf_s(numberString, VERSION_BUFF_SIZE, szVersionTwo,  versionNumbers[0], versionNumbers[1]);
			else sprintf_s(numberString, VERSION_BUFF_SIZE, szVersionOne, versionNumbers[0]);
			m_pVersion = new char[strnlen(numberString, 50) + 1];
			strcpy_s(m_pVersion, strnlen(numberString, 50) + 1, numberString);
		}
		delete [] buf;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::DisplayCompanyName(HDC paintDC, int windowWidth, int windowHeight)
{
SIZE sectionSize;
CRect DisplayRect;
char *pMainName = &szMainName[0];
LOGFONT lf;

	DisplayRect.left = m_Regions[COMPANY_REGION].LeftMargin;
	DisplayRect.top = m_Regions[COMPANY_REGION].VerticalOffset;
	DisplayRect.right = windowWidth - m_Regions[COMPANY_REGION].RightMargin;
	DisplayRect.bottom = m_Regions[COMPANY_REGION].VerticalOffset + m_Regions[COMPANY_REGION].VerticalHeight;
	sectionSize.cx = DisplayRect.right - m_Regions[COMPANY_REGION].LeftMargin;
	sectionSize.cy = m_Regions[COMPANY_REGION].VerticalHeight;
	if((m_Regions[COMPANY_REGION].PointSize.cx == -1) && (m_Regions[COMPANY_REGION].PointSize.cy == -1)){
		m_Regions[COMPANY_REGION].PointSize = FindFontPointSize(paintDC, m_Regions[COMPANY_REGION].FontName, &pMainName, 1, sectionSize);
	}
	HFONT Font = CreatePointFont(m_Regions[COMPANY_REGION].PointSize.cy, m_Regions[COMPANY_REGION].FontName, paintDC);
	GetObject(Font, sizeof(lf), &lf); // Get the font details
	DeleteObject(Font);								// we can now delete the font
	lf.lfWeight = 800;								// 700 for bold, 400 for regular, 800 for extra bold
	Font = CreateFontIndirect(&lf);		// now create the font again this time as bold
	HFONT originalFont = (HFONT)SelectObject(paintDC, Font);
	DrawEmbosedText(paintDC, pMainName, m_Regions[COMPANY_REGION].TextColor, DisplayRect, m_Regions[COMPANY_REGION].TextFlags);
	SelectObject(paintDC, originalFont);
	DeleteObject(Font);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::DisplayProductName(HDC paintDC, int windowWidth, int windowHeight)
{
SIZE sectionSize;
CRect DisplayRect;

	ASSERT(m_pProductName);	// Check Resources -> version -> ProductName
	DisplayRect.left = m_Regions[PRODUCT_REGION].LeftMargin;
	DisplayRect.top = m_Regions[PRODUCT_REGION].VerticalOffset;
	DisplayRect.right = windowWidth - m_Regions[PRODUCT_REGION].RightMargin;
	DisplayRect.bottom = m_Regions[PRODUCT_REGION].VerticalOffset + m_Regions[PRODUCT_REGION].VerticalHeight;
	sectionSize.cx = DisplayRect.right - m_Regions[PRODUCT_REGION].LeftMargin;
	sectionSize.cy = m_Regions[PRODUCT_REGION].VerticalHeight;
	if((m_Regions[PRODUCT_REGION].PointSize.cx == -1) && (m_Regions[PRODUCT_REGION].PointSize.cy == -1)){
		m_Regions[PRODUCT_REGION].PointSize = FindFontPointSize(paintDC, m_Regions[PRODUCT_REGION].FontName, &m_pProductName, 1, sectionSize);
	}
	HFONT Font = CreatePointFont(m_Regions[PRODUCT_REGION].PointSize.cy, m_Regions[PRODUCT_REGION].FontName, paintDC);
	HFONT originalFont = (HFONT)SelectObject(paintDC, Font);
	DrawEmbosedText(paintDC, m_pProductName, m_Regions[PRODUCT_REGION].TextColor, DisplayRect, m_Regions[PRODUCT_REGION].TextFlags);
	SelectObject(paintDC, originalFont);
	DeleteObject(Font);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::DisplayBody(HDC paintDC, int windowWidth, int windowHeight)
{
SIZE sectionSize;
RECT DisplayRect;
char *stringsToCheck[6] = {0};
int StringCount = 0;

	char **pStringsToCheck = &stringsToCheck[0];
	int RightMargin = windowWidth - m_Regions[GENERAL_REGION].RightMargin;
	sectionSize.cx = RightMargin - m_Regions[GENERAL_REGION].LeftMargin;
	sectionSize.cy = m_Regions[GENERAL_REGION].VerticalHeight;
	if((m_Regions[GENERAL_REGION].PointSize.cx == -1) && (m_Regions[GENERAL_REGION].PointSize.cy == -1)){
		if(m_pCompanyName != NULL){ *pStringsToCheck++ = m_pCompanyName; StringCount++; }
		if(m_pVersion != NULL){ *pStringsToCheck++ = m_pVersion; StringCount++; }
		if(m_pCopyright != NULL){ *pStringsToCheck++ = m_pCopyright; StringCount++; }
		if(m_pComments != NULL){ *pStringsToCheck++ = m_pComments; StringCount++; }
		m_Regions[GENERAL_REGION].PointSize = FindFontPointSize(paintDC, m_Regions[GENERAL_REGION].FontName, stringsToCheck, StringCount, sectionSize);
	}
	HFONT bodyFont = CreatePointFont(m_Regions[GENERAL_REGION].PointSize.cy, m_Regions[GENERAL_REGION].FontName, paintDC); 
	HFONT originalFont = (HFONT)SelectObject( paintDC, bodyFont );
	SetTextColor(paintDC, (m_Regions[GENERAL_REGION].TextColor == -1) ? m_Regions[PRODUCT_REGION].TextColor : m_Regions[GENERAL_REGION].TextColor);
	SetBkMode(paintDC, TRANSPARENT);
	int StringHeight = m_Regions[GENERAL_REGION].PointSize.cx;	
	int topOfText = m_Regions[GENERAL_REGION].VerticalOffset; 
	DisplayRect.left = m_Regions[GENERAL_REGION].LeftMargin;
	DisplayRect.right = RightMargin;
	if(m_pVersion != NULL){
		DisplayRect.top = topOfText;
		DisplayRect.bottom = topOfText + StringHeight;
		topOfText += StringHeight;
		DrawText(paintDC, m_pVersion, (int)strlen(m_pVersion), &DisplayRect, m_Regions[GENERAL_REGION].TextFlags);
	}
	if(m_pCompanyName != NULL){
		DisplayRect.top = topOfText;
		DisplayRect.bottom = topOfText + StringHeight;
		topOfText += StringHeight;
		DrawText(paintDC, m_pCompanyName, (int)strlen(m_pCompanyName), &DisplayRect, m_Regions[GENERAL_REGION].TextFlags);
	}
	if(m_pCopyright != NULL){
		DisplayRect.top = topOfText;
		DisplayRect.bottom = topOfText + StringHeight;
		topOfText += StringHeight;
		DrawText(paintDC, m_pCopyright, (int)strlen(m_pCopyright), &DisplayRect, m_Regions[GENERAL_REGION].TextFlags);
	}
	if(m_pComments != NULL){
		DisplayRect.top = topOfText;
		DisplayRect.bottom = topOfText + StringHeight;
		topOfText += StringHeight;
		DrawText(paintDC, m_pComments, (int)strlen(m_pComments), &DisplayRect, m_Regions[GENERAL_REGION].TextFlags);
	}
	SelectObject(paintDC, originalFont);
	DeleteObject(bodyFont);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::DisplayStatusLine(HDC paintDC, int windowWidth, int windowHeight)
{
SIZE sectionSize;
RECT statusRect;

	sectionSize.cx = (windowWidth - m_Regions[STATUS_REGION].RightMargin) - m_Regions[STATUS_REGION].LeftMargin;
	sectionSize.cy = m_Regions[STATUS_REGION].VerticalHeight;
	statusRect.left = m_Regions[STATUS_REGION].LeftMargin;
	statusRect.top = m_Regions[STATUS_REGION].VerticalOffset;
	statusRect.right = windowWidth - m_Regions[STATUS_REGION].RightMargin;
	statusRect.bottom = m_Regions[STATUS_REGION].VerticalOffset + m_Regions[STATUS_REGION].VerticalHeight;
	if((m_Regions[STATUS_REGION].PointSize.cx == -1) && (m_Regions[STATUS_REGION].PointSize.cy == -1)){
		m_Regions[STATUS_REGION].PointSize = FindFontPointSize(paintDC, m_Regions[STATUS_REGION].FontName, &m_pStatusMessage, 1, sectionSize);
	}
	HFONT statusFont = CreatePointFont(m_Regions[STATUS_REGION].PointSize.cy, m_Regions[STATUS_REGION].FontName, paintDC);
	HFONT originalFont = (HFONT)SelectObject(paintDC, statusFont);
	SetTextColor(paintDC, (m_Regions[GENERAL_REGION].TextColor == -1) ? m_Regions[PRODUCT_REGION].TextColor : m_Regions[STATUS_REGION].TextColor);
	SetBkMode(paintDC, TRANSPARENT);
	DrawText(paintDC, m_pStatusMessage, (int)strlen(m_pStatusMessage), &statusRect, m_Regions[STATUS_REGION].TextFlags);
	SelectObject(paintDC, originalFont);
	DeleteObject(statusFont);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HFONT CSplashWnd::CreatePointFont(int pointSize, LPCTSTR fontName, HDC dc)
{
HFONT font;

	LOGFONT logicalFont;
	memset(&logicalFont, 0, sizeof(LOGFONT));
	strcpy_s(logicalFont.lfFaceName, LF_FACESIZE, fontName);
	logicalFont.lfHeight = -MulDiv(pointSize, GetDeviceCaps(dc, LOGPIXELSY), 72); //pointSize * 10;
	font = CreateFontIndirect( &logicalFont );
	return font;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SIZE CSplashWnd::FindFontPointSize(HDC paintDC, LPCTSTR fontName, char **stringsToCheck, int numberOfStringsToCheck, SIZE maximumSize)
{
HFONT font;
LOGFONT logicalFont;
int pointSize = 8;
SIZE previousLargest;
SIZE largest;

	previousLargest.cx = 0;
	previousLargest.cy = 0;
	largest.cx = 0;
	largest.cy = 0;
	memset(&logicalFont, 0, sizeof(LOGFONT));
	strcpy_s(logicalFont.lfFaceName, LF_FACESIZE, fontName);
	maximumSize.cy /= numberOfStringsToCheck;
	while(TRUE){
		logicalFont.lfHeight = -MulDiv(pointSize, GetDeviceCaps(paintDC, LOGPIXELSY), 72); //pointSize * 10;
		font = CreateFontIndirect(&logicalFont);
		HFONT originalFont = (HFONT)SelectObject(paintDC, font);
		char **stringsPtr = stringsToCheck;
		for(int i = 0; i < numberOfStringsToCheck; i++){ 
			LPCTSTR string = *stringsPtr++;
			int stringLength = (int)strlen(string);
			SIZE szString;
			GetTextExtentPoint32(paintDC, string, stringLength, &szString);
			if(largest.cx < szString.cx){
				previousLargest = largest;
				largest.cx = szString.cx;
				largest.cy = szString.cy;
			}
		}
		SelectObject(paintDC, originalFont);
		DeleteObject(font);
		if(largest.cx > maximumSize.cx) break;
		if(largest.cy > maximumSize.cy) break;
		pointSize += 2;
	}
	pointSize -= 2;
	largest.cx = previousLargest.cy;		// in cx, return actual height of font in device units
	largest.cy = pointSize;
	return largest;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSplashWnd::DrawEmbosedText(HDC paintDC, char *pString, COLORREF Colour, CRect DispRect, int Flags)
{
	SetBkMode(paintDC, TRANSPARENT);
	SetTextColor(paintDC, RGB(200,200,200));
	DrawText(paintDC, pString, (int)strlen(pString), &DispRect, Flags);
	SetTextColor(paintDC, m_Regions[PRODUCT_REGION].TextColor);
	DispRect.OffsetRect(-1,-1);
	DrawText(paintDC, pString, (int)strlen(pString), &DispRect, Flags);
}

#include <locale>

#include "mainForm.h"	// includes wcommon.h

WindowInfo wiMain, wiHist, wiVars;

#ifdef _DEBUG
void DebugMsg(std::wstring ws)
{
	OutputDebugString(ws.c_str());
}
#endif


// DEBUG 
#ifdef _DEBUG
BOOL CALLBACK EnumTypesFunc(HMODULE hModule, LPTSTR lpType, LONG_PTR lParam);
BOOL CALLBACK EnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam);

int DebugEnumResources()
{
	// Load the executable module
	HMODULE hModule = GetModuleHandle(NULL); // NULL to get the handle of the current process

	if (hModule == NULL) {
		DebugMsg(L"Error loading module: " + std::to_wstring(GetLastError()));
		return 1;
	}

	// Enumerate resource types
	if (!EnumResourceTypesW(hModule, EnumTypesFunc, 0)) {
		DebugMsg(L"Error enumerating resource types: " + std::to_wstring(GetLastError()));
		return 1;
	}

	return 0;
}

// Callback function to enumerate resource types
BOOL CALLBACK EnumTypesFunc(HMODULE hModule, LPTSTR lpType, LONG_PTR lParam)
{

	static const std::wstring __names[] = {
			L"",					// 
			L"CURSOR      ",		// 1
			L"BITMAP      ",		// 2
			L"ICON        ",		// 3
			L"MENU        ",		// 4
			L"DIALOG      ",		// 5
			L"STRING      ",		// 6
			L"FONTDIR     ",		// 7
			L"FONT        ",		// 8
			L"ACCELERATOR ",		// 9
			L"RCDATA      ",		// 10
			L"MESSAGETABLE",		// 11
			L"GROUP_CURSOR",		// 12
			L"?",					// 13
			L"GROUP_ICON  ",		// 14
			L"?",					// 15
			L"VERSION     ",		// 16
			L"DLGINCLUDE  ",		// 17
			L"?",					// 18
			L"PLUGPLAY    ",		// 19
			L"VXD         ",		// 20
			L"ANICURSOR   ",		// 21
			L"ANIICON     ",		// 22
			L"HTML        ",		// 23
			L"MANIFEST    "			// 24
	};
	DebugMsg(L"Resource Type: ");

	if (IS_INTRESOURCE(lpType)) {
		DebugMsg(__names[(long)lpType] + L"(" + std::to_wstring((long)lpType) + L") ");
	}
	else {
		DebugMsg(lpType);
	}

	// Enumerate names for the given resource type
	if (!EnumResourceNamesW(hModule, lpType, EnumNamesFunc, 0)) {
		DebugMsg(L"Error enumerating resource names: " + std::to_wstring(GetLastError()));
		return FALSE;
	}

	return TRUE;
}

// Callback function to enumerate resource names
BOOL CALLBACK EnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam)
{
	DebugMsg(L"  Resource Name: ");

	if (IS_INTRESOURCE(lpName)) {
		DebugMsg(std::to_wstring((long)lpName));
	}
	else {
		DebugMsg(lpName);
	}
	DebugMsg(L"\n");

	return TRUE;
}
#endif
// /DEBUG

void MyLoadWindowIcon(nlib::Form* f)
{
	if (!f->IconFromResource(nullptr, MAKEINTRESOURCEW(IDI_MAINICON)))
	{
		DWORD dw = GetLastError();
#if _DEBUG
		DebugMsg(L"IconFromResource returned false, Error code:" + std::to_wstring(dw));
#endif
	}
}

using namespace nlib;


POINT WinDistance(RECT main, RECT other, FalconCalc::WindowSide side)
{
	switch (side)
	{
		case FalconCalc::wsNone:  return { 0,0 };
		case FalconCalc::wsTop:	  return {main.left - other.left, main.top - other.bottom};
		case FalconCalc::wsRight: return {other.left - main.right, main.top - other.top};
		case FalconCalc::wsBottom:return { main.left - other.left, other.top - main.bottom};
		case FalconCalc::wsLeft:  return {main.left - other.right, main.top - other.top};
	}
	return { 0,0 };
}


// TASK: test the iontersection of rectangle 'rect' with rectangle 'r'
// RETURNS: flags in an integer for each corner of 'r' that is inside 'rect'
//			0: no intersection,					
//			1: top	left of r is inside 'rect' or top left of 'rect' is inside 'r'
//          2: top right of r is inside 'rect' or top right of 'rect' is inside 'r' 
//			4: bottom right of r is inside 'rect' or bottom right of 'rect' is inside 'r',
//			8:bottom left of r is inside 'rect'


FalconCalc::WindowSide GetSnapSide(RECT& r, int& dist)	// 'dist' will be set to the distance of corresponding sides
{
	Rect rT;	// area beside the main window
	// area of main  temporarily stored in rT, which is modified below
	rT = wiMain.BareVisibleWindowRect();

	if (rT.DoesIntersect(r))				// windows intersect -> no snap
		return FalconCalc::wsNone;

	Rect rSnapArea = rT;

	// check area above main form
	rSnapArea.bottom = rT.top;
	rSnapArea.top = rT.top - dist;
	if (rSnapArea.DoesIntersect(r))
	{
		dist = rT.top - r.bottom;
		return FalconCalc::wsTop;
	}
	// check area right of main form
	rSnapArea = rT;
	rSnapArea.left = rT.right;
	rSnapArea.right = rT.right + dist;
	if (rSnapArea.DoesIntersect(r))
	{
		dist = r.left - rT.right;
		return FalconCalc::wsRight;
	}
	// check area below main window
	rSnapArea = rT;
	rSnapArea.top = rT.bottom;
	rSnapArea.bottom = rT.bottom + dist;
	if (rSnapArea.DoesIntersect(r))
	{
		dist = r.top - rT.bottom;
		return FalconCalc::wsBottom;
	}
	// check area left of main window
	rSnapArea = rT;
	rSnapArea.left = rT.left - dist;
	rSnapArea.right = rT.left;
	if (rSnapArea.DoesIntersect(r))
	{
		dist = rT.left - r.right;
		return FalconCalc::wsLeft;
	}

	return FalconCalc::wsNone;
}

bool SnapTo(RECT& r, FalconCalc::WindowSide side, POINT dist)	// calculates snapped coordinates into input/output 'r'
{	  
	switch (side)
	{
		default:
		case FalconCalc::wsNone: return false;									// move
		case FalconCalc::wsTop:		OffsetRect(&r, dist.x, dist.y); break;	//		down
		case FalconCalc::wsRight:	OffsetRect(&r, -dist.x, dist.y); break;	//		left
		case FalconCalc::wsBottom:	OffsetRect(&r, dist.x, -dist.y); break;	//		up
		case FalconCalc::wsLeft:	OffsetRect(&r, dist.x, dist.y); break;	//		right
	}
	return true;
}


// ************************ WindowInfo *************************

void WindowInfo::InitInfo(nlib::Form* pForm, std::wstring name)	// only change info but does not delete original _pForn
{
	_name = name;
	_pForm = pForm;
	if (pForm)			// so that we can set _pForm to nullptr
		_GetRects();
}
void WindowInfo::RefreshInfo()
{
	if (_pForm)
		_GetRects(true);
};

#if defined(_DEBUG)
void WindowInfo::_ShowStyles()
{
	std::wstring s;
	struct Styles
	{
		DWORD val;
		std::wstring name;
	};
	static Styles styles[] = {
		{WS_BORDER               ,L"WS_BORDER"},
		{WS_CAPTION              ,L"WS_CAPTION"},
		{WS_CHILD                ,L"WS_CHILD"},
		{WS_CHILDWINDOW          ,L"WS_CHILDWINDOW"},
		{WS_CLIPCHILDREN         ,L"WS_CLIPCHILDREN"},
		{WS_CLIPSIBLINGS         ,L"WS_CLIPSIBLINGS"},
		{WS_DISABLED             ,L"WS_DISABLED"},
		{WS_DLGFRAME             ,L"WS_DLGFRAME"},
		{WS_GROUP                ,L"WS_GROUP"},
		{WS_HSCROLL              ,L"WS_HSCROLL"},
		{WS_ICONIC               ,L"WS_ICONIC"},
		{WS_MAXIMIZE             ,L"WS_MAXIMIZE"},
		{WS_MAXIMIZEBOX          ,L"WS_MAXIMIZEBOX"},
		{WS_MINIMIZE             ,L"WS_MINIMIZE"},
		{WS_MINIMIZEBOX          ,L"WS_MINIMIZEBOX"},
		{WS_OVERLAPPED           ,L"WS_OVERLAPPED"},
		{WS_OVERLAPPEDWINDOW     ,L"WS_OVERLAPPEDWINDOW"},
		{WS_POPUP                ,L"WS_POPUP"},
		{WS_POPUPWINDOW          ,L"WS_POPUPWINDOW"},
		{WS_SIZEBOX              ,L"WS_SIZEBOX"},
		{WS_SYSMENU              ,L"WS_SYSMENU"},
		{WS_TABSTOP              ,L"WS_TABSTOP"},
		{WS_THICKFRAME           ,L"WS_THICKFRAME"},
		{WS_TILED                ,L"WS_TILED"},
		{WS_TILEDWINDOW          ,L"WS_TILEDWINDOW"},
		{WS_VISIBLE              ,L"WS_VISIBLE"},
		{WS_VSCROLL              ,L"WS_VSCROLL"},
	};
	bool b = false;
	for (auto& a : styles)
		if (dwStyle & a.val)
		{
			s = s + (b ? L", " : L"") + a.name;
			b = true;
		}
	s = s + L"\n";
	DebugMsg(std::wstring(L"_GetRects (") + _name + std::wstring(L")\nStyles: ") + s);

	static Styles exStyles[] = {
		{ WS_EX_ACCEPTFILES                   ,L"WS_EX_ACCEPTFILES"},
		{ WS_EX_APPWINDOW                     ,L"WS_EX_APPWINDOW"},
		{ WS_EX_CLIENTEDGE                    ,L"WS_EX_CLIENTEDGE"},
		{ WS_EX_COMPOSITED                    ,L"WS_EX_COMPOSITED"},
		{ WS_EX_CONTEXTHELP                   ,L"WS_EX_CONTEXTHELP"},
		{ WS_EX_CONTROLPARENT                 ,L"WS_EX_CONTROLPARENT"},
		{ WS_EX_DLGMODALFRAME                 ,L"WS_EX_DLGMODALFRAME"},
		{ WS_EX_LAYERED                       ,L"WS_EX_LAYERED"},
		{ WS_EX_LAYOUTRTL                     ,L"WS_EX_LAYOUTRTL"},
		{ WS_EX_LEFT                          ,L"WS_EX_LEFT"},
		{ WS_EX_LEFTSCROLLBAR                 ,L"WS_EX_LEFTSCROLLBAR"},
		{ WS_EX_LTRREADING                    ,L"WS_EX_LTRREADING"},
		{ WS_EX_MDICHILD                      ,L"WS_EX_MDICHILD"},
		{ WS_EX_NOACTIVATE                    ,L"WS_EX_NOACTIVATE"},
		{ WS_EX_NOINHERITLAYOUT               ,L"WS_EX_NOINHERITLAYOUT"},
		{ WS_EX_NOPARENTNOTIFY                ,L"WS_EX_NOPARENTNOTIFY"},
		{ WS_EX_NOREDIRECTIONBITMAP           ,L"WS_EX_NOREDIRECTIONBITMAP"},
		{ WS_EX_OVERLAPPEDWINDOW              ,L"WS_EX_OVERLAPPEDWINDOW"},
		{ WS_EX_PALETTEWINDOW                 ,L"WS_EX_PALETTEWINDOW"},
		{ WS_EX_RIGHT                         ,L"WS_EX_RIGHT"},
		{ WS_EX_RIGHTSCROLLBAR                ,L"WS_EX_RIGHTSCROLLBAR"},
		{ WS_EX_RTLREADING                    ,L"WS_EX_RTLREADING"},
		{ WS_EX_STATICEDGE                    ,L"WS_EX_STATICEDGE"},
		{ WS_EX_TOOLWINDOW                    ,L"WS_EX_TOOLWINDOW"},
		{ WS_EX_TOPMOST                       ,L"WS_EX_TOPMOST"},
		{ WS_EX_TRANSPARENT                   ,L"WS_EX_TRANSPARENT"},
		{ WS_EX_WINDOWEDGE                    ,L"WS_EX_WINDOWEDGE"},
	};
	s = L"	Extended Styles: ";
	b = false;
	for (auto& a : exStyles)
		if (dwExStyle & a.val)
		{
			s = s + (b ? L", " : L"") + a.name;
			b = true;
		}
	s = s + L"\n";
	DebugMsg(s);
}
#endif
void WindowInfo::_GetRects(bool force)
{
	if (_pForm && (force || !_initted))
	{
		cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(_pForm->Handle(), (WINDOWINFO*)this);

		// debug
#if defined(_DEBUG)
		//_ShowStyles();
#endif
		// /debug

		int bwx = GetSystemMetrics(SM_CXFRAME),
			bwy = GetSystemMetrics(SM_CYFRAME);
		if (dwExStyle & WS_EX_LAYERED)
		{
			_noShadowRect.top = rcClient.top + bwy;
			_noShadowRect.right = rcClient.right - bwx;
			_noShadowRect.bottom = rcClient.bottom - bwy;
			_noShadowRect.left = rcClient.left + bwx;
		}
		else
			_noShadowRect = rcWindow;
		if (bwx != cxWindowBorders)	// then part of the border is invisible
		{
			_noBorderRect.top = rcWindow.top + bwy;
			_noBorderRect.right = rcWindow.right - bwx;
			_noBorderRect.bottom = rcWindow.bottom - bwy;
			_noBorderRect.left = rcWindow.left + bwx;
		}
		else
			_noBorderRect = rcWindow;
		_initted = true;
	}
}

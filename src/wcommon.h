#pragma once

#ifndef _WCOMMON_H
#define _WCOMMON_h

#include "stdafx_zoli.h"

#include <windows.h>
#include <locale>
namespace FalconCalc {
	class LittleEngine;

	enum WindowSide { wsNone, wsTop, wsRight, wsBottom, wsLeft };
	constexpr auto STATE_VER_STRING = L"FalconCalc State File V1.0";
}
extern SIZE dropShadowSize;

#ifdef _DEBUG
void DebugMsg(std::wstring ws);
#endif

class WindowInfo : WINDOWINFO
{
public:
	WindowInfo() {}
	WindowInfo(std::wstring name,  nlib::Form* pForm) : _name(name), _pForm(pForm) { _GetRects(); }
	void InitInfo(nlib::Form* pForm, std::wstring name);	// only change info but does not delete original _pForn
	void RefreshInfo(); 
	int Left() { return rcWindow.left; }
	int Top() { return rcWindow.top; }
	int Right() { return rcWindow.right; }
	int Bottom() { return rcWindow.bottom; }
	int Width() { return rcWindow.right - rcWindow.left; }
	int Height() { return rcWindow.bottom - rcWindow.top; }
	SIZE BorderSize()  // w.o. Title bar height
	{
		_GetRects();
		return { (long)cxWindowBorders, (long)cyWindowBorders };
	}
	SIZE DropShadowSize()
	{
		_GetRects();
		return { _noShadowRect.Width() - nlib::Rect(rcClient).Width(), _noShadowRect.Height() - nlib::Rect(rcClient).Height() - TitleBarHeight() };
	}
	int TitleBarHeight()
	{
		return (rcWindow.top - rcClient.top) - (long)cyWindowBorders;
	}
	nlib::Rect WindowRect() { _GetRects(); return rcWindow; }
	nlib::Rect ClientRect() { _GetRects(); return rcClient; }
	nlib::Rect WindowRectNoDropShadow() { _GetRects(); return _noShadowRect; }
	nlib::Rect BareVisibleWindowRect()
	{
		_GetRects();
		return _noBorderRect;
	}
private:
	std::wstring _name;
	nlib::Form* _pForm = nullptr;
	nlib::Rect  _noShadowRect,		// window rectangle w.o. drop shadow (if any)
				_noBorderRect;		// window with no border, but with title bar
	bool _initted = false;

#if defined(_DEBUG)
	void _ShowStyles();
#endif
	void _GetRects(bool force = false);
};

class ConditionFlag
{
	int _cnt = 0;
public:
	int operator++() { return ++_cnt; }
	int operator--() { if (_cnt) --_cnt; return _cnt; }
	operator bool() { return _cnt; }
	void Reset() { _cnt = 0; }
};

extern WindowInfo wiMain, wiHist, wiVars;

POINT WinDistance(RECT main, RECT other, FalconCalc::WindowSide side);
FalconCalc::WindowSide GetSnapSide(RECT& r, int& dist);
bool SnapTo(RECT& r, FalconCalc::WindowSide side, POINT dist);	// calculates snapped coordinates into input/output 'r'

bool IsAlpha(wchar_t ch, std::locale loc);    // needed for names in one locale when working in another localse
bool IsAlnum(wchar_t ch, std::locale loc);    // needed for names in one locale when working in another localse

#endif

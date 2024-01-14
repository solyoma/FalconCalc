#include "stdafx_zoli.h"
#include "application.h"
#include "windowfunctions.h"
#include "generalcontrol.h"


//---------------------------------------------


namespace NLIBNS
{


const WindowStyleSet wsOverlappedWindow = WindowStyleSet() << wsOverlapped << wsCaption << wsSysMenu << wsThickFrame << wsMinimizeBox << wsMaximizeBox;
const WindowStyleSet wsPopupWindow = WindowStyleSet() <<  wsPopup << wsBorder << wsSysMenu;
const WindowStyleSet wsTiledWindow = WindowStyleSet() <<  wsOverlapped << wsCaption << wsSysMenu << wsThickFrame << wsMinimizeBox << wsMaximizeBox;
const WindowStyleSet lbsStandard = WindowStyleSet() << lbsNotify << lbsSort << wsVScroll << wsBorder;

const ExtendedWindowStyleSet wsExOverlappedWindow = ExtendedWindowStyleSet() << wsExWindowEdge << wsExClientEdge;
const ExtendedWindowStyleSet wsExPaletteWindow = ExtendedWindowStyleSet() << wsExWindowEdge << wsExToolWindow << wsExTopmost;

ATOM RegisterWindowClass(const ClassParams &params)
{
    return RegisterWindowClass(params.classname, params.style, params.wndextra, params.icon, params.iconsm, params.brush, params.cursor);
}
ATOM RegisterWindowClass(const std::wstring &classname, UINT style, int wndextra, HICON hIcon, HICON hIconSm, HBRUSH hBrush, HCURSOR hCursor)
{
    WNDCLASSEX classex;
    classex.cbSize = sizeof(WNDCLASSEXW);
    classex.hInstance = hInstance;

    if (GetClassInfoEx(hInstance, classname.c_str(), &classex))
        return 0;

    classex.cbSize = sizeof(WNDCLASSEXW);
    classex.style = style;
    classex.lpfnWndProc = &AppWndProc;
    classex.cbClsExtra = 0;
    classex.cbWndExtra = wndextra;
    classex.hInstance = hInstance;
    classex.hIcon = hIcon;
    classex.hIconSm = hIconSm;
    classex.hCursor = hCursor;
    classex.hbrBackground = hBrush;
    classex.lpszMenuName = NULL;
    classex.lpszClassName = classname.c_str();
    ATOM classatom = RegisterClassEx(&classex);
    if (classatom == 0)
    {
        int error = GetLastError();
        if (error != 0)
            throw L"Couldn't register class.";
    }
    return classatom;
}

HWND CreateWindowHandle(const std::wstring &classname, const WindowParams &params, LPVOID lpParam)
{
    return CreateWindowHandle(classname, params.windowtext, params.extstyle, params.style, params.x, params.y, params.width, params.height, params.parent, params.menu, lpParam);
}
HWND CreateWindowHandle(const std::wstring &classname, const std::wstring &windowtext, DWORD exstyle, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, LPVOID param)
{
    HWND handle = CreateWindowEx(exstyle, classname.c_str(), windowtext.c_str(), style, x, y,  width, height, parent, menu, hInstance, param);
    return handle;
}

WNDPROC ReplaceWndProc(HWND handle, WNDPROC newproc)
{
    WNDPROC oldproc = (WNDPROC)SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)&AppWndProc);
    if (oldproc == NULL)
    {
        //LONG error = GetLastError();
        throw L"Couldn't change window proc!";
    }
    return oldproc;
}

void InitCommonControl(DWORD control)
{
    INITCOMMONCONTROLSEX InitCtrlEx;
    InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrlEx.dwICC  = control;
    InitCommonControlsEx(&InitCtrlEx);
}

void ConstraintSizing(byte fwSide, Rect &lprc, int minwidth, int minheight, int maxwidth, int maxheight)
{
  ConstraintSizing(fwSide,lprc,minwidth,minheight,maxwidth,maxheight,0,0,0,0);
}

void ConstraintSizing(byte fwSide, Rect &lprc, int minwidth, int minheight)
{
  ConstraintSizing(fwSide,lprc,minwidth,minheight,999999,999999,0,0,0,0);
}

void ConstraintSizing(byte fwSide, Rect &lprc, int minwidth, int minheight, int maxwidth, int maxheight, int widthdiff, int modx, int heightdiff, int mody)
{
  int base = min(maxwidth,max(minwidth,(int)(lprc.right-lprc.left))); // the new width
  if (modx) {
    int w = base - widthdiff;
    if (w % modx < modx / 2)
      base -= w % modx;
    else
      base += modx - (w % modx);
  }

  switch(fwSide) {
    case(WMSZ_BOTTOMLEFT): case(WMSZ_TOPLEFT): case(WMSZ_LEFT):
      lprc.left = lprc.right - base;
    break;
    case(WMSZ_BOTTOMRIGHT): case(WMSZ_TOPRIGHT): case(WMSZ_RIGHT):
      lprc.right = lprc.left + base;
    break;
  }

  base = min(maxheight,max(minheight,(int)(lprc.bottom-lprc.top)));

  if (mody) {
    int h = base - heightdiff;
    if (h % mody < mody / 2)
      base -= h % mody;
    else
      base += mody - (h % mody);
  }

  switch(fwSide) {
    case(WMSZ_TOPLEFT): case(WMSZ_TOPRIGHT): case(WMSZ_TOP):
      lprc.top = lprc.bottom - base;
    break;
    case(WMSZ_BOTTOMLEFT): case(WMSZ_BOTTOMRIGHT): case(WMSZ_BOTTOM):
      lprc.bottom = lprc.top + base;
    break;
  }
}

extern ConstValue<Application, DWORD> gen_lcid;
bool ContainsAccelerator(const std::wstring &str, WCHAR key)
{
    size_t p = 0;
    wchar_t w = 0;
    while(w != key)
    {
        p = str.find(L"&", p);
        if (p == std::wstring::npos || p == str.size() - 1)
            return false;
        p++;
        w = str[p];
        LCMapString(gen_lcid,LCMAP_LOWERCASE,&w,1,&w,1);
    }
    return true;
}


//---------------------------------------------


std::wstring GetClassName(HWND handle)
{

    std::wstring result(257, 0);
    result.resize(GetClassName(handle, const_cast<wchar_t*>(result.c_str()), 257));
    return result;
}


//---------------------------------------------

// Helper types for Get****Form functions passed to EnumThreadWindows.
enum GetFormEnumType { gfetNext, gfetPrev, gfetTop, gfetBottom };
struct GetFormEnumData
{
    GetFormEnumType type;
    Form *base;
    bool disabled;
    bool hidden;
    Form *result;

    // Used inside GetFormEnumProc only.
    Form *last;

    GetFormEnumData(GetFormEnumType type, Form *base, bool disabled, bool hidden) : type(type), base(base), disabled(disabled), hidden(hidden), result(NULL), last(NULL) {}
};

// Helper for Get****Form functions passed to EnumThreadWindows.
static BOOL CALLBACK GetFormEnumProc(HWND hwnd, LPARAM lParam)
{
    GetFormEnumData *data = (GetFormEnumData*)(lParam);

    Form *f;
    if (data->base != NULL && hwnd == data->base->Handle())
        f = data->base;
    else
        f = dynamic_cast<Form*>(application->ControlFromHandle(hwnd));

    if (((data->type != gfetNext && data->type != gfetPrev) || f != data->base) && (!f || (!data->hidden && !f->Visible()) || (!data->disabled && !f->Enabled())))
        return TRUE;

    if (data->type == gfetTop)
    {
        data->result = f;
        return FALSE;
    }
    if (data->type == gfetBottom)
    {
        f = dynamic_cast<Form*>(application->ControlFromHandle(hwnd));
        if (f)
            data->result = f;
    }
    else if (data->type == gfetNext)
    {
        if (data->last == data->base)
        {
            data->result = f;
            return FALSE;
        }
    }
    else if (data->type == gfetPrev)
    {
        if (f == data->base)
        {
            data->result = data->last;
            return FALSE;
        }
    }
    data->last = f;
    return TRUE;
}

Form* GetNextForm(Form *current, bool disabled, bool hidden)
{
    GetFormEnumData data(gfetNext, current, disabled, hidden);
    EnumThreadWindows(MainThreadId, &GetFormEnumProc, (LPARAM)&data);
    return data.result;
}

Form* GetPrevForm(Form *current, bool disabled, bool hidden)
{
    GetFormEnumData data(gfetPrev, current, disabled, hidden);
    EnumThreadWindows(MainThreadId, &GetFormEnumProc, (LPARAM)&data);
    return data.result;
}

Form* GetTopForm(bool disabled, bool hidden)
{
    GetFormEnumData data(gfetTop, NULL, disabled, hidden);
    EnumThreadWindows(MainThreadId, &GetFormEnumProc, (LPARAM)&data);
    return data.result;
}

Form* GetBottomForm(bool disabled, bool hidden)
{
    GetFormEnumData data(gfetBottom, NULL, disabled, hidden);
    EnumThreadWindows(MainThreadId, &GetFormEnumProc, (LPARAM)&data);
    return data.result;
}


//---------------------------------------------


}
/* End of NLIBNS */


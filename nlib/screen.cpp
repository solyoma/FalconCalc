#include "stdafx_zoli.h"
#include "screen.h"
#include "controlbase.h"
#include "generalcontrol.h"
#ifdef DESIGNING
#include "designproperties.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING
ValuePair<Cursors> CursorStrings[] = {
    VALUEPAIR(cAppStarting),
    VALUEPAIR(cDefault),
    VALUEPAIR(cIBeam),
    VALUEPAIR(cCross),
    VALUEPAIR(cHand),
    VALUEPAIR(cHelpArrow),
    VALUEPAIR(cNo),
    VALUEPAIR(cNormal),
    VALUEPAIR(cScrollAll),
    VALUEPAIR(cScrollNS),
    VALUEPAIR(cScrollEW),
    VALUEPAIR(cScrollE),
    VALUEPAIR(cScrollN),
    VALUEPAIR(cScrollNE),
    VALUEPAIR(cScrollNW),
    VALUEPAIR(cScrollS),
    VALUEPAIR(cScrollSE),
    VALUEPAIR(cScrollSW),
    VALUEPAIR(cScrollW),
    VALUEPAIR(cSizeAll),
    VALUEPAIR(cSizeColumn),
    VALUEPAIR(cSizeNESW),
    VALUEPAIR(cSizeNS),
    VALUEPAIR(cSizeNWSE),
    VALUEPAIR(cSizeWE),
    VALUEPAIR(cSplitH),
    VALUEPAIR(cSplitV),
    VALUEPAIR(cUp),
    VALUEPAIR(cWait),
};

#endif

ConstValue<Screen, int> LogPixelsX;
ConstValue<Screen, int> LogPixelsY;
ConstValue<Screen, float> Scaling;

std::pair<const wchar_t*, int> ScreenCursor::modulecursors[] = {
                            std::pair<const wchar_t*, int>(L"comctl32", 107),
                            std::pair<const wchar_t*, int>(L"comctl32", 135),
                            std::pair<const wchar_t*, int>(L"comctl32", 106),
                                                          };

ScreenCursor* ScreenCursor::instance = NULL;

/**
 * \return The same value which is in the global [screencursor](\ref ::screencursor) variable while the program is running. 
 */
ScreenCursor* ScreenCursor::GetInstance()
{
    if (!instance)
        instance = new ScreenCursor();

    screencursor = instance;
    return instance;
}

void ScreenCursor::FreeInstance()
{
    delete instance;
    instance = NULL;
    screencursor = NULL;
}

ScreenCursor::ScreenCursor() : control(NULL), nchover(false)
{

}

/**
 * If no control is currently under the mouse, the return value is NULL.
 * \sa HoveredControlByArea()
 */
Control* ScreenCursor::HoveredControl()
{
    return control;
}

/**
 * Depending on the nonclient parameter, the return value might be NULL even if the mouse hovers over a control.
 * Use HoveredOnBorder() to determine whether the cursor is over the client or nonclient area of a window.
 * \param nonclient Whether to return a control when the mouse pointer is over its non-client area or when it is in the client area.
 * \return The current control under the mouse pointer if the pointer is over the specified area, or NULL.
 * \sa HoveredControl()
 */ 
Control* ScreenCursor::HoveredControlByArea(bool nonclient)
{
    return nchover == nonclient ? control : NULL;
}

/**
 * \return The return value is \a true if the mouse pointer is over a control's non-client area. If the mouse
 * is not over a control, the return value is \a false.
 * \sa HoveredControlByArea()
 */
bool ScreenCursor::HoveredOnBorder()
{
    if (!control)
        return false;
    return nchover;
}

bool ScreenCursor::MouseMovedOn(Control *mcontrol, bool nc)
{
    if ((mcontrol == control && nchover == nc) || !mcontrol->HandleCreated())
        return false;

    control = mcontrol;
    nchover = nc;

    TRACKMOUSEEVENT tr;
    tr.cbSize = sizeof(TRACKMOUSEEVENT);
    tr.dwFlags = !nc ? TME_LEAVE : TME_NONCLIENT;
    tr.hwndTrack = control->Handle();
    tr.dwHoverTime = HOVER_DEFAULT;
    _TrackMouseEvent(&tr);

    return true;
}

void ScreenCursor::MouseLeftFrom(Control *mcontrol)
{
    if (control != mcontrol)
        return;

    control = NULL;
}

/**
 * \sa Cursors
 */
void ScreenCursor::Set(Cursors cursor)
{
    if (cursor == cDefault)
    {
        if (control)
            SendMessage(control->Handle(), WM_SETCURSOR, (WPARAM)control->Handle(), MAKELPARAM(nchover ? HTBORDER : HTCLIENT, WM_MOUSEMOVE));
        return;
    }

    if ((int)cursor < 0)
    {
        SetCursor(GetCursor(cursor)/*(HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(-(int)cursor), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED)*/);
        return;
    }
}

/**
 * The function uses @::LoadImage() to get the cursor handle.
 * \return The handle to the cursor or NULL if the cursor was not found.
 */
HCURSOR ScreenCursor::GetCursor(Cursors cursor)
{
    if (cursor < 0)
    {
        int ms = sizeof(modulecursors) / sizeof(std::pair<const wchar_t*, int>);
        if (cursor >= -ms)
            return (HCURSOR)LoadImage(GetModuleHandle(modulecursors[-1 - (int)cursor].first), MAKEINTRESOURCE(modulecursors[-1 - (int)cursor].second), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        else
            return (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(-(int)cursor), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    }
    return NULL;
}

/**
 * The area is correctly measured even if the cursor is animated. The function doesn't cache the returned values,
 * so if the same value must be used several times, it is the responsibility of the application to cache it.
 * \param cursor The value in the Cursors enumeration for the cursor to measure.
 * \param [out] cursorrect Receives the rectangle of the cursor when drawn to the coordinates (0, 0).
 * \param [out] framecount Receives the number of frames of the cursor. If the cursor is not animated, the result is 1.
 * \return Whether the function succeeded in measuring the cursor. If this is false, the values in \a cursorrect and \a framecount are invalid.
 * \sa Cursors, GetCursor()
 */
bool ScreenCursor::CursorSize(Cursors cursor, Rect &cursorrect, int &framecount)
{
    framecount = 0;
    cursorrect = Rect(0, 0, 0, 0);

    HCURSOR c = GetCursor(cursor);
    if (!c)
        return false;

    bool error = false;

    // A bitmap must be created and the cursor drawn on it to determine real cursor rectangle.
    int w = GetSystemMetrics(SM_CXCURSOR),
        h = GetSystemMetrics(SM_CYCURSOR);
    Bitmap bmp(w, h);
    auto bits = bmp.LockBits(glmWriteOnly);
    try
    {
        // Clear the bitmap.
        byte *line = (byte*)bits->Scan0;
        memset(line, 0, 4 * w * h);
        //for (size_t y = 0; y < bits->Height; ++y)
        //    for (size_t x = 0; x < bits->Width; ++x)
        //    {
        //        line[x*4 + 0] = 0; // blue
        //        line[x*4 + 1] = 0; // green
        //        line[x*4 + 2] = 0; // red
        //        line[x*4 + 3] = 0; // alpha
        //    }
    }
    catch(...)
    {
        error = true;
    }
    bmp.UpdateBits();

    if (error)
        return false;

    HDC dc = bmp.GetDC();
    if (dc && !DrawIconEx(dc, 0, 0, c, 0, 0, UINT_MAX, NULL, 0)) // The cursor is animated, because it can't draw the UINT_MAXth frame. (Non animated cursors would return true.)
    {
        while (DrawIconEx(dc, 0, 0, c, 0, 0, framecount, NULL, DI_DEFAULTSIZE | DI_NORMAL))
            framecount++;
    }
    else if (!dc || !DrawIconEx(dc, 0, 0, c, 0, 0, framecount++, NULL, DI_DEFAULTSIZE | DI_NORMAL))
        error = true;

    if (dc)
        bmp.ReturnDC();

    if (error || framecount == 0)
        return false;

    // Find the bits where the cursor was drawn on.
    cursorrect = Rect(128, 128, -1, -1);

    bits = bmp.LockBits(glmReadOnly);
    try
    {
        byte *line = (byte*)bits->Scan0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x)
            {
                if (int(*(line + x * 4)) != 0)
                {
                    if (cursorrect.top > cursorrect.bottom)
                        cursorrect.top = y;
                    
                    if (cursorrect.bottom <= y)
                        cursorrect.bottom = y + 1;

                    if (cursorrect.left > x)
                        cursorrect.left = x;

                    if (cursorrect.right <= x)
                        cursorrect.right = x + 1;
                }
            }
            line += bits->Stride;
        }
    }
    catch(...)
    {
        error = true;
    }
    bmp.UpdateBits();

    if (error == true)
        return false;

    return cursorrect.left < cursorrect.right;
}

Point ScreenCursor::Pos()
{
    CURSORINFO ci = { 0 };
    ci.cbSize = sizeof(CURSORINFO);

    GetCursorInfo(&ci);

    return ci.ptScreenPos;
}


// ----------------------------------------------------


DisplayMonitor::DisplayMonitor(HMONITOR handle) : handle(handle),primary(false)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(handle, &mi);

    fullarea = mi.rcMonitor;
    workarea = mi.rcWork;
    primary = (mi.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY;
}

DisplayMonitor::~DisplayMonitor()
{
}

/**
 * \return Boolean \a true or \a false, depending on whether this is the primary display monitor.
 * \sa Screen::PrimaryMonitor()
 */
bool DisplayMonitor::IsPrimary()
{
    return primary;
}

HMONITOR DisplayMonitor::Handle()
{
    return handle;
}

/**
 * The full area is the area of the display monitor including toolbars. The top-left
 * coordinates for the area can be negative if this is not the primary display monitor.
 * \sa WorkArea()
 */
Rect DisplayMonitor::FullArea()
{
    return fullarea;
}

/**
 * The work area is the area of the display monitor without the space taken up by
 * toolbars. The top-left coordinates for the area can be negative if this is not
 * the primary display monitor in the system.
 * \sa FullArea()
 */
Rect DisplayMonitor::WorkArea()
{
    return workarea;
}


// ----------------------------------------------------

    
Screen* Screen::instance = NULL;

/**
 * \return The same value which is in the global [screen](\ref ::screen) variable while the program is running. 
 */
Screen* Screen::GetInstance()
{
    if (!instance)
        instance = new Screen();
    screen = instance;
    return instance;
}

void Screen::FreeInstance()
{
    delete instance;
    instance = NULL;
    screen = NULL;
}

Screen::Screen() : primaryix(-1)
{
    HDC dc = GetDC(0);
    try
    {
        LogPixelsX = GetDeviceCaps(dc, LOGPIXELSX);
        LogPixelsY = GetDeviceCaps(dc, LOGPIXELSY);
        Scaling = float(float(LogPixelsY) / 96.0);
    }
    catch(...)
    {
        ;
    }
    ReleaseDC(0, dc);

    UpdateDisplays();
}

Screen::~Screen()
{
}

BOOL CALLBACK FillMonitorsProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    std::vector<DisplayMonitor*> *monitors = (std::vector<DisplayMonitor*>*)dwData;
    monitors->push_back(new DisplayMonitor(hMonitor));
    return true;
}

/**
 * When the [screen](\ref ::screen) object is first created it populates its monitor list with the available
 * display monitors. This function only needs to be called if the monitors might have changed,
 * and you need to access their properties.
 * \sa MonitorCount(), Monitors(), DisplayMonitor
 */
void Screen::UpdateDisplays()
{
    for (auto it = monitors.begin(); it != monitors.end(); it++)
        delete *it;
    monitors.clear();
    primaryix = -1;

    EnumDisplayMonitors(NULL, NULL, &FillMonitorsProc, (LPARAM)&monitors);

    for (unsigned int ix = 0; ix < monitors.size() && primaryix < 0; ix++)
        if (monitors[ix]->IsPrimary())
            primaryix = ix;
}

/**
 * \remark Call UpdateDisplays() if the number of displays or their properties might have changed.
 * \sa Monitors(), UpdateDisplays(), DisplayMonitor
 */
int Screen::MonitorCount()
{
    return monitors.size();
}

/**
 * \param ix The positive index of the display monitor less than the value of MonitorCount().
 * \return The monitor at index \a ix.
 * \remark Call UpdateDisplays() if the number of displays or their properties might have changed.
 * \sa MonitorCount(), UpdateDisplays()
 */
DisplayMonitor* Screen::Monitors(int ix)
{
    if (ix < 0 || ix >= (int)monitors.size())
        throw L"Monitor index out of range.";
    return monitors[ix];
}

/**
 * \remark Call UpdateDisplays() if the primary monitor might have changed.
 * \sa MonitorCount(), Monitors(), UpdateDisplays()
 */
DisplayMonitor* Screen::PrimaryMonitor()
{
    if (primaryix < 0)
        UpdateDisplays();
    if (primaryix < 0) // Check again.
        throw L"Monitors not initialized!";
    return monitors[primaryix];
}

/**
 * If the control is currently on multiple monitors, the one with the largest overlapping area is
 * returned. If the control is outside the area of all display monitors, the nearest one is returned
 * instead. If the control is a child control, its parent must have the handle created, otherwise
 * the monitor will be deduced from its parent form.
 * \param control The control, whose position determines the display monitor to return.
 * \return The display monitor overlapping with the largest area of the control, or the one nearest to it. NULL is returned on error.
 * \sa MonitorFromWindow(HWND), MonitorFromHandle()
 */
DisplayMonitor* Screen::MonitorFromWindow(Control *control)
{
    if (!control)
        return NULL;

    HMONITOR hm = NULL;

    if (control->Parent() == NULL && control->HandleCreated())
        hm = ::MonitorFromWindow(control->Handle(), MONITOR_DEFAULTTONEAREST);
    else
    {
        Rect r = control->WindowRect();
        if (control->Parent() != NULL)
        {
            if (control->Parent()->HandleCreated())
                r = control->Parent()->ClientToScreen(r);
            else
                r = control->ParentForm()->WindowRect();
        }
        return MonitorFromRect(r);
    }

    return MonitorFromHandle(hm);
}

/**
 * If the window is currently on multiple monitors, the one with the largest overlapping area is
 * returned. If the window is outside the area of all display monitors, the nearest one is returned
 * instead.
 * \param handle The handle of the control, whose position determines the display monitor to return.
 * \return The display monitor overlapping with the largest area of the control, or the one nearest to it. NULL is returned on error.
 * \sa MonitorFromWindow(Control*), MonitorFromHandle()
 */
DisplayMonitor* Screen::MonitorFromWindow(HWND handle)
{
    if (!handle)
        return NULL;
    Rect r;
    if (GetWindowRect(handle, &r) == FALSE)
        return NULL;

    return MonitorFromRect(r);
}

/**
 * If the rectangle overlaps with the area of multiple monitors, the one with the largest overlapping
 * area is returned. If the rectangle is outside the area of all display monitors, the nearest one is
 * returned.
 * \param r The rectangle which determines the display monitor to return.
 * \return The display monitor overlapping with the largest area of \a r, or the one nearest to it. NULL is returned on error.
 * \sa MonitorFromPoint(), MonitorFromWindow(), MonitorFromHandle()
 */
DisplayMonitor* Screen::MonitorFromRect(const Rect &r)
{
    HMONITOR hm = ::MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST);
    if (!hm)
        return NULL;
    return MonitorFromHandle(hm);
}

/**
 * If the point is outside the area of all display monitors, the nearest one is returned.
 * \param p The point which determines the display monitor to return.
 * \return The display monitor which holds the coordinates of \a p, or the one nearest to it. NULL is returned on error.
 * \sa MonitorFromRect(), MonitorFromWindow(), MonitorFromHandle()
 */
DisplayMonitor* Screen::MonitorFromPoint(const Point &p)
{
    HMONITOR hm = ::MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
    if (!hm)
        return NULL;
    return MonitorFromHandle(hm);
}

/**
 * The single Screen object caches the list of display monitors. If the passed handle doesn't
 * match any of the cached values, the cache is updated before the value is looked up again.
 * \param hmon The handle of the display monitor to return.
 * \return The display monitor belonging to \a hmon or NULL if no such monitor was found.
 * \sa UpdateDisplays(), MonitorFromWindow(), MonitorFromRect()
 */
DisplayMonitor* Screen::MonitorFromHandle(HMONITOR hmon)
{
    if (!hmon)
        return NULL;


    for (auto it = monitors.begin(); it != monitors.end(); it++)
        if ((*it)->Handle() == hmon)
            return *it;

    // It's possible that the monitors changed in the system. Let's try again.
    UpdateDisplays();

    for (auto it = monitors.begin(); it != monitors.end(); it++)
        if ((*it)->Handle() == hmon)
            return *it;

    return NULL;
}


// --------------------------------------------------------------------


}
/* End of NLIBNS */


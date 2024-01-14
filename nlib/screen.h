#pragma once


namespace NLIBNS
{


class Control;
class Application;

/**
 * Enumeration of the basic mouse cursors.
 * \sa ScreenCursor::Set(), Control::SetCursor()
 */
enum Cursors : int {
               cDefault = 0, cNormal = -32512, cIBeam = -32513, cWait = -32514, cCross = -32515,
               cUp = -32516, cSizeNWSE = -32642, cSizeNESW = -32643, cSizeWE = -32644, cSizeNS = -32645,
               cSizeAll = -32646, cNo = -32648, cHand = -32649, cAppStarting = -32650,
               cHelpArrow = -32651, cScrollNS = -32652, cScrollEW = -32653, cScrollAll = -32654,
               cScrollN = -32655, cScrollS = -32656, cScrollE = -32657, cScrollW = -32658,
               cScrollNE = -32659, cScrollNW = -32660, cScrollSE = -32661, cScrollSW = -32662,
               cSplitH = -1, cSplitV = -2, cSizeColumn = -3,
#ifdef DESIGNING
               cCount = 29,
#endif
            };

/// Singleton class of the global [screencursor](\ref ::screencursor) variable used to set the current cursor on screen.
/**
 * Applications usually don't need to use [screencursor](\ref ::screencursor) when the mouse cursor is
 * over a control, instead set the cursor for the control with the Control::SetCursor() method.
 * Apart from changing the cursor, the single instance of this class also keeps track which
 * control is currently under the mouse pointer. Use HoveredControl() or HoveredControlByArea() to get
 * the control under the mouse.
 * \sa Cursors
 */
struct ScreenCursor
{
private:
    static ScreenCursor *instance;

    static std::pair<const wchar_t*, int> modulecursors[];
    
    Control *control; // The control under the mouse. This can be NULL.
    bool nchover; // The mouse is over the non-client area of the control.

    ScreenCursor();
    ScreenCursor(const ScreenCursor &copy) { }
    ~ScreenCursor() { }

    bool MouseMovedOn(Control *mcontrol, bool nc);
    void MouseLeftFrom(Control *mcontrol);

    static void FreeInstance();
    friend class Application;
    friend class Control;
public:
    static ScreenCursor* GetInstance(); ///< Returns the single instance of ScreenCursor.

    Control* HoveredControl(); ///< Returns the control currently under the mouse.
    Control* HoveredControlByArea(bool nonclient); ///< Returns the control currently under the mouse, or NULL, depending on the nonclient parameter.
    bool HoveredOnBorder(); ///< Indicates whether the mouse is over the non-client area of a control.
    void Set(Cursors cursor); ///< Changes the currently displayed cursor by calling @::SetCursor().

    HCURSOR GetCursor(Cursors cursor); ///< Returns a cursor handle for the specified cursor.
    bool CursorSize(Cursors cursor, Rect &cursorrect, int &framecount); ///< Returns the smallest enclosing rectangle of the cursor image when it is drawn.

    Point Pos(); ///< Returns the current cursor position in screen coordinates.
};

/// %Object of the singleton ScreenCursor type. This value is initialized in Application::Run().
extern ScreenCursor *screencursor;

/// Stores a display monitor's dimensions and whether it is the primary monitor.
/**
 * Use the global [screen](\ref ::screen) object of type Screen, to access the DisplayMonitor objects
 * belonging to each display monitor in the system.
 * Call Screen::MonitorCount() for the number of display monitors available, and Screen::Monitors() to get the
 * DisplayMonitor objects for them.
 * \sa Screen, Screen::PrimaryMonitor(), Screen::MonitorFromWindow(), Screen::MonitorFromHandle()
 */
class DisplayMonitor
{
private:
    HMONITOR handle;

    Rect fullarea;
    Rect workarea;

    bool primary;

    DisplayMonitor(HMONITOR handle);
    ~DisplayMonitor();
    friend class Screen;
    friend BOOL CALLBACK FillMonitorsProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
public:
    HMONITOR Handle(); ///< Returns the handle for the display monitor.
    Rect FullArea(); ///< Full area for the display monitor in virtual-screen coordinates.
    Rect WorkArea(); ///< Work area for the display monitor in virtual-screen coordinates.
    bool IsPrimary(); ///< Indicates whether this is the primary display monitor.
};

/// Singleton class of the global [screen](\ref ::screen) variable for retrieving the properties of the screen.
/**
 * The primary monitor in the system can be retrieved with PrimaryMonitor(), while all display
 * monitors can be accessed separately by calling Monitors(). The display monitor overlapping a control or a
 * rectangular area can be looked up with MonitorFromWindow() or MonitorFromRect().
 *
 * The \a screen object caches the display monitors in the system. Call UpdateDisplays() if the cache
 * needs to be updated.
 */
class Screen
{
private:
    static Screen *instance;
    Screen();
    ~Screen();
    Screen(const Screen &copy) { }

    std::vector<DisplayMonitor*> monitors;

    int primaryix;

    static void FreeInstance();
    friend class Application;
public:
    static Screen* GetInstance(); ///< Returns the single instance of Screen.

    void UpdateDisplays(); ///< Updates the list of cached display monitors to reflect any changes since the creation of the [screen](\ref ::screen) object.

    int MonitorCount(); ///< The number of display monitors that can be retrieved with Monitors().
    DisplayMonitor* Monitors(int ix); ///< Retrieves objects holding the properties of display monitors.

    DisplayMonitor* PrimaryMonitor(); ///< The display monitor object for the primary monitor.
    DisplayMonitor* MonitorFromWindow(Control *control); ///< Returns the monitor displaying the control.
    DisplayMonitor* MonitorFromWindow(HWND handle); ///< Returns the monitor displaying the window.
    DisplayMonitor* MonitorFromRect(const Rect &r); ///< Returns the monitor with the largest overlapping area with a rectangle.
    DisplayMonitor* MonitorFromPoint(const Point &p); ///< Returns the monitor nearest to the specified point.
    DisplayMonitor* MonitorFromHandle(HMONITOR hmon); ///< Finds the display monitor object for the monitor handle.
};

/// %Object of the singleton Screen type. This value is initialized in Application::Run().
extern Screen *screen;


}
/* End of NLIBNS */


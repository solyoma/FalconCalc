#include "stdafx_zoli.h"
#include "themes.h"
#include "application.h"
#include "generalcontrol.h"
#include "dialog.h"
#include "canvas.h"
#include "screen.h"
#include "syscontrol.h"
#include "utility.h"
#include "menu.h"

#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

extern void Initialize();

namespace NLIBNS
{


    extern "C"
    {
        typedef HPAINTBUFFER (WINAPI *BeginBufferedPaint_T)(HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
        typedef HRESULT (WINAPI *EndBufferedPaint_T)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
        typedef HRESULT (WINAPI *BufferedPaintInit_T)();
        typedef HRESULT (WINAPI *BufferedPaintUnInit_T)();
        typedef HRESULT (WINAPI *DwmIsCompositionEnabled_T)(BOOL *pfEnabled);

        BeginBufferedPaint_T BeginBufferedPaint_REDEFINED = NULL;
        EndBufferedPaint_T EndBufferedPaint_REDEFINED = NULL;
        BufferedPaintInit_T BufferedPaintInit_REDEFINED = NULL;
        BufferedPaintUnInit_T BufferedPaintUnInit_REDEFINED = NULL;
        DwmIsCompositionEnabled_T DwmIsCompositionEnabled_REDEFINED = NULL;
    }

    //int CoInitModel = COINIT_MULTITHREADED;

    Application* Application::instance = NULL;
    int Application::initialshow = -1;
    ConstValue<Application, Application*> application;
    StockCanvas *StockCanvas::instance = NULL;
    ScreenCursor *screencursor = NULL;
    Screen *screen = NULL;
    Fonts *systemfonts = NULL;
    ConstValue<Application, DWORD> Win32MajorVersion;
    ConstValue<Application, DWORD> Win32MinorVersion;
    ConstValue<Application, HINSTANCE> hInstance;

    ConstValue<Application, DWORD> LOCALE_ENGLISH_DEFAULT;
    ConstValue<Application, DWORD> gen_lcid;

    ConstValue<Application, int> ArgsCnt;
    ConstVector<Application, std::wstring> ArgsVar;
    ConstWString<Application> ExecutablePath;
    ConstValue<Application, bool> WindowsIs64Bit;
    ConstValue<Application, DWORD> MainThreadId; // Id of the main thread.

    class AppMessageWindow : public MessageWindowBase
    {
    private:
        Application *owner;
    protected:
        virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            return owner->MessageProc(uMsg, wParam, lParam);
        }
    public:
        AppMessageWindow(Application *owner) : owner(owner) {}
    };

    void Application::SetInstance(HINSTANCE hinst)
    {
        hInstance = hinst;
    }

    void Application::SetCmdShow(int cmdshow)
    {
        initialshow = cmdshow;
    }

    void Application::SetArgsCnt(int argscnt)
    {
        ArgsCnt = argscnt;
    }

    void Application::AddArgsVar(wchar_t *argsvar)
    {
        ArgsVar.push_back(argsvar);
    }

    void Application::InitValues()
    {
        InitCommonControl(ICC_LISTVIEW_CLASSES);

        ExecutablePath.resize(MAX_PATH + 1);
        while(true)
        {
            unsigned int nlen = GetModuleFileName(NULL, const_cast<wchar_t*>(ExecutablePath.c_str()), ExecutablePath.length());
            if (nlen == 0) // Error
            {
                ExecutablePath.resize(0);
                break;
            }
            if (nlen == ExecutablePath.length() && (ExecutablePath[ExecutablePath.length() - 1] != 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER))
                ExecutablePath.resize(ExecutablePath.length() + 64);
            else
            {
                ExecutablePath = GetFilePath(ExecutablePath.substr(0, nlen));
                break;
            }
        }

        OSVERSIONINFO vi;
        vi.dwOSVersionInfoSize = sizeof(vi);
        GetVersionEx(&vi);
        Win32MajorVersion = vi.dwMajorVersion;
        Win32MinorVersion = vi.dwMinorVersion;

        MainThreadId = GetCurrentThreadId();

        if (WinVerSupported(6, 0))
        {
            HMODULE UxThemeHandle = LoadLibrary(L"UxTheme.dll");
            HMODULE DwmapiHandle = LoadLibrary(L"Dwmapi.dll");
            BeginBufferedPaint_REDEFINED = (BeginBufferedPaint_T)GetProcAddress(UxThemeHandle, "BeginBufferedPaint");
            EndBufferedPaint_REDEFINED = (EndBufferedPaint_T)GetProcAddress(UxThemeHandle, "EndBufferedPaint");
            BufferedPaintInit_REDEFINED = (BufferedPaintInit_T)GetProcAddress(UxThemeHandle, "BufferedPaintInit");
            BufferedPaintUnInit_REDEFINED = (BufferedPaintUnInit_T)GetProcAddress(UxThemeHandle, "BufferedPaintUnInit");
            DwmIsCompositionEnabled_REDEFINED = (DwmIsCompositionEnabled_T)GetProcAddress(DwmapiHandle, "DwmIsCompositionEnabled");
        }

    #if defined(_WIN64)
        WindowsIs64Bit = true;
    #else
        WindowsIs64Bit = false;
        BOOL proc64bit = FALSE;
        BOOL (WINAPI *IsWow64ProcessProc)(HANDLE, PBOOL);
        IsWow64ProcessProc = (BOOL (WINAPI*)(HANDLE, PBOOL))GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
        if (NULL != IsWow64ProcessProc)
        {
            IsWow64ProcessProc(GetCurrentProcess(), &proc64bit);
            WindowsIs64Bit = proc64bit != FALSE;
        }
    #endif

        LOCALE_ENGLISH_DEFAULT = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
        gen_lcid = Win32MajorVersion < 5 ? LOCALE_ENGLISH_DEFAULT : LOCALE_INVARIANT;

        srand(GetTickCount());
    }

    Application::Application() : mainform(NULL), activeform(NULL), active(false), terminated(false), dwmcomp(false),
                                 stockcanvas(NULL), dlgmode(dmDisableBranch), messageform(NULL), tooltip(NULL)
    {
        application = this;
    }

    Application::~Application()
    {
        //if (uifont)
        //    DeleteObject(uifont);
    }

    /**
     * When first called, constructs the Application object and fills the global [application](\ref ::application)
     * variable. Subsequent calls return the only instance of the already constructed application.
     * \return The single instance of the Application object.
     */
    Application* Application::GetInstance()
    {
        if (instance == NULL)
            instance = new Application();
        return instance;
    }

    /**
     * The first time the main form of the application is shown, it can be resized or minimized depending on this value
     * To avoid that, call the static OverrideShow() in main() before the application runs.
     * \return The value passed to WinMain via its nCmdShow argument. It is an SW_ constants accepted by ShowWindow.
     */
    int Application::InitialShow()
    {
        return initialshow;
    }

    /**
     * Call before the main form becomes visible, usually before Run() is called in main().
     */ 
    void Application::OverrideShow()
    {
        initialshow = -1;
    }

    /**
     * Controls are created with the default UI font.
     * \return A Font object with the properties of the default UI font.
     * \sa UILogFont(), MenuFont(), MenuLogFont(), NCMetrics()
     */
    Font Application::UIFont()
    {
        return uifont;
    }

    /**
     * The main menus and popup menus are drawn with the default font, unless an event handler is assigned to their item drawing event.
     * \return A Font object with the properties of the default menu font.
     * \sa MenuLogFont(), UIFont(), UILogFont(), NCMetrics()
     */
    Font Application::MenuFont()
    {
        return menufont;
    }

    /**
     * Controls are created with the default UI font.
     * \return The properties of the default UI font in a LOGFONT structure.
     * \sa UIFont(), MenuFont(), MenuLogFont(), NCMetrics()
     */
    const LOGFONT& Application::UILogFont()
    {
        return uilogfont;
    }

    /**
     * The main menus and popup menus are drawn with the default font, unless an event handler is assigned to their item drawing event.
     * \return The properties of the default menu font in a LOGFONT structure.
     * \sa MenuFont(), UIFont(), UILogFont(), NCMetrics()
     */
    const LOGFONT& Application::MenuLogFont()
    {
        return menulogfont;
    }

    /**
     * The non-client metrics includes the size of different parts of a window's non-client
     * area, as well as the properties of the default fonts used in the system.
     * \return The NONCLIENTMETRICS structure that was retrieved with a call to SystemParametersInfo passing SPI_GETNONCLIENTMETRICS as the uiAction parameter.
     * \sa UIFont(), UILogFont(), MenuFont(), MenuLogFont()
     */
    const NONCLIENTMETRICS& Application::NCMetrics()
    {
        return ncmetrics;
    }

    /**
     * An application can have many forms, but only one main form. The only difference between
     * it and normal forms is that closing the main form closes the program as well.
     * \return The main form.
     * \sa SetMainForm()
     */
    Form* Application::MainForm()
    {
        return mainform;
    }

    /**
     * The active form is the form which has the input focus in the program, or which last
     * had the input focus before another program became active. Once the program becomes
     * active again, this form gets the input focus, unless the program was activated by
     * clicking on another form.
     * \return The active form.
     * \sa Active()
     */
    Form* Application::ActiveForm()
    {
        return activeform;
    }

    /**
     * An application can have many forms, but only one main form. The only difference between
     * it and normal forms is that closing the main form closes the program as well.
     * \param newmainform The form to become the main form of the program. This value can be NULL.
     * \sa MainForm()
     */
    void Application::SetMainForm(Form *newmainform)
    {
        if (mainform == newmainform || (newmainform && newmainform->Parent()))
            return;
        if (mainform)
        {
            Form *tmp = mainform;
            mainform = NULL;
            tmp->SetToMainForm(false);
        }
        if (newmainform)
        {
            mainform = newmainform;
            newmainform->SetToMainForm(true);
        }
    }

    void Application::addform(Form *f)
    {
        auto it = std::find(forms.begin(), forms.end(), f);
        if (it != forms.end())
            return;
        forms.push_back(f);
    }

    void Application::formdestroyed(Form *f)
    {
        auto it = std::find(forms.begin(), forms.end(), f);
        if (it == forms.end())
            return;
        forms.erase(it);
    }

    /**
     * The application's window is message-only. It doesn't have a visual representation on screen,
     * and it only knows how to handle notification messages about system wide setting changes.
     * If the program's taskbar buttons need to be accessed, use the handle of the corresponding
     * windows.
     * \return A message-only window handle.
     */
    HWND Application::Handle()
    {
        return messageform->Handle();
    }

    //void Application::RegisterRawDevices()
    //{
    //    // Usage and usage pages taken from: http://www.usb.org/developers/devclass_docs/Hut1_12v2.pdf
    //    RAWINPUTDEVICE dev;
    //    dev.usUsagePage = 1; // Generic desktop controls.
    //    dev.usUsage = 2; // Mouse.
    //    dev.dwFlags = 0;
    //    //if (WinVerSupported(6, 0)) // Vista at least.
    //    //    dev.dwFlags |= RIDEV_DEVNOTIFY;
    //    dev.hwndTarget = 0;
    //    BOOL res = RegisterRawInputDevices(&dev, 1, sizeof(RAWINPUTDEVICE));
    //    if (!res)
    //        throw L"Unable to register raw input devices on program startup.";
    //}
    //
    //void Application::UnregisterRawDevices()
    //{
    //    RAWINPUTDEVICE dev;
    //    dev.usUsagePage = 1; // Generic desktop controls.
    //    dev.usUsage = 2; // Mouse.
    //    dev.dwFlags = RIDEV_REMOVE;
    //    //if (WinVerSupported(6, 0)) // Vista at least.
    //    //    dev.dwFlags |= RIDEV_DEVNOTIFY;
    //    dev.hwndTarget = 0;
    //    RegisterRawInputDevices(&dev, 1, sizeof(RAWINPUTDEVICE));
    //}

    /**
     * Run() must only be called once in the main function. The Application object is deleted
     * before execution quits the function. Do not access the global [application](\ref ::application)
     * variable after that.
     */
    int Application::Run()
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        UpdateColors();
        Screen::GetInstance();
        stockcanvas = StockCanvas::GetInstance();
        ScreenCursor::GetInstance();
        Themes::GetInstance();
        Fonts::GetInstance();
        UpdateSettings();

        messageform = new AppMessageWindow(this);

        //HRESULT r = CoInitializeEx(NULL, CoInitModel);
        /*HRESULT r =*/ OleInitialize(NULL);

        tooltip = new Tooltip();

        Initialize();

        MSG msg;
        //HACCEL hAccelTable = NULL;

        int rget = 0;


        bool buffinited = false;
        if (WinVerSupported(6, 0))
        {
            BOOL b;
            dwmcomp = DwmIsCompositionEnabled_REDEFINED(&b) == S_OK;
            if (dwmcomp)
                dwmcomp = b != FALSE;

            buffinited = BufferedPaintInit_REDEFINED() == S_OK;
        }

        while ((rget = GetMessage(&msg, NULL, 0, 0)) != FALSE)
        {
            if (rget < 0)
            {
                // error handling?
            }
            else 
            {
                MessageRound(&msg);
            }
        }

        if (buffinited)
            BufferedPaintUnInit_REDEFINED();

        // Clean up after terminate
        std::vector<Form*> destroylist;
        std::for_each(forms.begin(), forms.end(), [&destroylist](Form *f) {
            if (!f->Parent())
                destroylist.push_back(f);
        });
        std::for_each(destroylist.begin(), destroylist.end(), [this](Form *f) {
            if (std::find(forms.begin(), forms.end(), f) != forms.end())
                f->Destroy();
        });

        delete messageform;
        messageform = NULL;


        //CoUninitialize();
        OleUninitialize();

        Themes::FreeInstance();
        StockCanvas::FreeInstance();
        ScreenCursor::FreeInstance();
        Fonts::FreeInstance();

        Gdiplus::GdiplusShutdown(gdiplusToken);

        delete this;

        return (int) msg.wParam;
    }

    bool Application::AcceleratorKeyMessage(const MSG &msg)
    {
        if (msg.message < WM_KEYFIRST || msg.message > WM_KEYLAST || activeform == NULL)
            return false;

        VirtualKeyStateSet vkeys;
        WORD key;

        switch (msg.message)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            vkeys = PressedVirtualKeys();
            if (vkeys.contains(vksAlt) && KeyboardWithAltGr() && (GetKeyState(VK_RMENU) & (1 << 15)) != 0)
                break;

            key = 0;
            if (msg.wParam == VK_CONTROL || vkeys.contains(vksCtrl))
                key |= 1 << 8;
            if (msg.wParam == VK_SHIFT || vkeys.contains(vksShift))
                key |= 1 << 9;
            if (msg.wParam == VK_MENU || vkeys.contains(vksAlt))
                key |= 1 << 10;
            if (msg.wParam != VK_CONTROL && msg.wParam != VK_SHIFT && msg.wParam != VK_MENU && msg.wParam >= 8 && msg.wParam <= 255)
                key |= msg.wParam;

            return activeform->PassMessage(wmMenuAccelerator, key, 0) != 0;
        }
        return false;
    }

    bool Application::DialogKeyMessage(const MSG &msg)
    {
        if (msg.message < WM_KEYFIRST || msg.message > WM_KEYLAST || !activeform)
            return false;

        LPARAM r;
        Control *control;

        switch (msg.message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
    #ifdef DESIGNING
            if (!((Control*)activeform)->Designing())
    #endif
            //if (msg.wParam == VK_MENU && ((GetKeyState(VK_RMENU) & (1 << 15)) == 0 || (GetKeyState(VK_LCONTROL) & (1 << 15)) == 0))
            if (msg.wParam == VK_MENU && (!KeyboardWithAltGr() || (GetKeyState(VK_RMENU) & (1 << 15)) == 0))
                activeform->PassMessage(WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);

            if (msg.wParam == VK_TAB || msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT || msg.wParam == VK_UP || msg.wParam == VK_DOWN)
            {
                r = SendMessage(msg.hwnd, WM_GETDLGCODE, msg.wParam, (LPARAM)&msg);
                if ((r & DLGC_WANTALLKEYS) != DLGC_WANTALLKEYS && ((msg.wParam == VK_TAB && (r & DLGC_WANTTAB) != DLGC_WANTTAB) || (msg.wParam != VK_TAB && (r & DLGC_WANTARROWS) != DLGC_WANTARROWS)))
                {
                    activeform->PassMessage(WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);

                    control = ControlFromHandle(msg.hwnd);
                    if (!control) // Message for a window which wasn't derived from Control*. Probably a handle created by windows or by another control. Check for the latter and forward this request to any parent controls the window might have.
                    {
                        control = ParentControlFromHandle(msg.hwnd);
                        if (control)
                        {
                            r = SendMessage(control->Handle(), WM_GETDLGCODE, msg.wParam, (LPARAM)&msg);
                            if ((r & DLGC_WANTALLKEYS) == DLGC_WANTALLKEYS || (msg.wParam == VK_TAB && (r & DLGC_WANTTAB) == DLGC_WANTTAB) || (msg.wParam != VK_TAB && (r & DLGC_WANTARROWS) == DLGC_WANTARROWS))
                                return false;
                        }
                        else
                            return true;
                    }

                    if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP)
                        return true;

                    if (activeform != control->ParentForm())
                        return true;

                    if (activeform)
                    {
                        control = activeform->TabNext(NULL, msg.wParam == VK_TAB ? ((GetKeyState(VK_SHIFT) & (1 << 15)) == 0) : msg.wParam == VK_RIGHT || msg.wParam == VK_DOWN);
                        if (control)
                            control->TabFocus();
                    }

                    return true;
                }
            }
            else if (msg.wParam == VK_RETURN || msg.wParam == VK_ESCAPE)
            {
                r = SendMessage(msg.hwnd, WM_GETDLGCODE, msg.wParam, (LPARAM)&msg);
                if ((r & DLGC_WANTALLKEYS) != DLGC_WANTALLKEYS)
                {
                    control = ControlFromHandle(msg.hwnd);
                    if (!control)
                        return true;
                    //if (!control) // Message for a window which wasn't derived from Control*. Probably a handle created by windows or by another control. Check for the latter and forward this request to any parent controls the window might have.
                    //{
                    //    Control *tmp = ParentControlFromHandle(msg.hwnd);
                    //    if (tmp)
                    //    {
                    //        r = SendMessage(tmp->Handle(), WM_GETDLGCODE, msg.wParam, (LPARAM)&msg);
                    //        if ((r & DLGC_WANTALLKEYS) == DLGC_WANTALLKEYS)
                    //            return false;
                    //    }
                    //    return true;
                    //}

                    if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP)
                        return true;

                    if (activeform != control->ParentForm())
                        return true;

                    if (activeform)
                        activeform->PassMessage(wmDialogKey, msg.wParam,  (LPARAM)control);
                    else
                        control->PassMessage(wmDialogKey, msg.wParam, (LPARAM)control);

                    return true;
                }
            }
            break;
        //case WM_CHAR:
        //case WM_SYSCHAR:
        //case WM_DEADCHAR:
        //case WM_SYSDEADCHAR:
        //    if (msg.wParam == 9) // Tab key
        //    {
        //        r = SendMessage(msg.hwnd, WM_GETDLGCODE, 0, 0);
        //        if ((r & DLGC_WANTTAB) != DLGC_WANTTAB)
        //            return true;
        //    }
        }
        return false;
    }

    //bool UseDialogKey(UINT uMsg, WPARAM key, unsigned int vkeys)
    //{
    //    LRESULT r;
    //    Form *f;
    //    Control *c;
    //
    //    switch (uMsg)
    //    {
    //    case WM_KEYDOWN:
    //    case WM_KEYUP:
    //        if (key == VK_TAB || key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN)
    //        {
    //            r = PassMessage(WM_GETDLGCODE, 0, 0);
    //            if (key == VK_TAB && (r & DLGC_WANTTAB) != DLGC_WANTTAB || (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN) && (r & DLGC_WANTARROWS) != DLGC_WANTARROWS)
    //            {
    //                if (uMsg == WM_KEYUP)
    //                    return true;
    //
    //                f = ParentForm();
    //                if (!f)
    //                    f = dynamic_cast<Form*>(this);
    //
    //                if (f)
    //                {
    //                    c = f->TabNext(NULL, key == VK_TAB ? !(vkeys & vksShift) : key == VK_RIGHT || key == VK_DOWN);
    //                    if (c)
    //                        c->Focus();
    //                }
    //
    //                return true;
    //            }
    //        }
    //
    //
    //        break;
    //    case WM_CHAR:
    //    case WM_SYSCHAR:
    //        if (key == 9) // Tab character.
    //        {
    //            r = PassMessage(WM_GETDLGCODE, 0, 0);
    //            if ((r & DLGC_WANTTAB) != DLGC_WANTTAB)
    //                return true;
    //        }
    //        break;
    //    }
    //
    //    return false;
    //}

    void Application::MessageRound(MSG *msg)
    {
        try
        {
        //if ((msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP) && ((msg.lParam >> 29) & 1) || true /*!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)*/)
        //{
            // IsDialogMessage handles the TAB key, the ENTER in some circumstances and other system keys which help in navigation. Because for example ESC automatically closes windows when IsDialogMessage handles it, we have to do all the message handling by ourselves.
            if (/*msg->message == WM_SYSCHAR ||*/ !AcceleratorKeyMessage(*msg) && !DialogKeyMessage(*msg) /*&& !IsDialogMessage(activeform && activeform->HandleCreated() ? activeform->Handle() : NULL, msg)*/)
            {
                TranslateMessage(msg);
                DispatchMessage(msg);
            }
        //}
        }
        catch(const wchar_t *msg)
        {
            ShowMessageBox(msg, L"Exception caught!", mbOk);
        }
    }

    /**
     * If there is no message in the message queue, the function does nothing. The handled message
     * is removed from the queue. If WM_QUIT is encountered the application becomes terminated,
     * making further message processing not possible.
     * Calling HandleOneMessage() in an loop can cause the program to take up all processing time
     * of one processor core. To avoid that, call the winapi WaitMessage() function in case this
     * function returns false and the application is not terminated yet.
     * \return Whether a message was removed from the message queue. A value of \a false indicates that the message queue was empty or that no further message processing is possible.
     * \sa HandleMessages(), Terminated()
     */
    bool Application::HandleOneMessage()
    {
        MSG msg;

        bool handled = false;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message != WM_QUIT)
            {
                handled = true;
                MessageRound(&msg);
            }
            else
                terminated = true;
        }
        return handled;
    }

    /**
     * If there is no message in the message queue, the function does nothing. The handled messages
     * are removed from the queue. If WM_QUIT is encountered, no further message processing is possible.
     * Check the value of Terminated() to see if this happened.
     * \sa HandleOneMessage(), Terminated()
     */
    void Application::HandleMessages()
    {
        while (HandleOneMessage())
            ;
    }

    /**
     * Once PostQuitMessage() is called, WM_QUIT is inserted in the message queue and no more message
     * processing is possible.
     * \return Only \a true after the application received WM_QUIT and should stop.
     * \sa HandleOneMessage(), HandleMessages()
     */
    bool Application::Terminated()
    {
        return terminated;
    }

    /**
     * The function does not call DwmIsCompositionEnabled to generate a result, instead, the
     * application object stores the return value, and only updates it when a WM_DWMCOMPOSITIONCHANGED 
     * message was received.
     * \return Whether the drawing in the system is done with DWM enabled or not.
     */
    bool Application::DwmComposition()
    {
        return dwmcomp;
    }

    /**
     * If \a hwnd does not belong to an object of type derived from Control, the result is NULL. 
     * \param hwnd The handle of the control to retrieve.
     * \return A control owning \a hwnd or NULL.
     * \sa ParentControlFromHandle()
     */
    Control* Application::ControlFromHandle(HWND hwnd)
    {
        std::map<HWND,Control*>::iterator finder = controlhandles.find(hwnd);
        if (finder == controlhandles.end() || finder->first != hwnd)
            return NULL;
        return finder->second;
    }

    /**
     * The function fetches the handles of windows that are parents of the previous one, until one is
     * found which belongs to a Control object. For example a Combobox object is returned, if
     * the function was called with the handle of its inner editor.
     * \param hwnd The handle to a window which is the child of an object of type Control.
     * \return An object of type Control in the parent chain of \a hwnd, or NULL if no such object was found.
     * \sa ControlFromHandle()
     */
    Control* Application::ParentControlFromHandle(HWND hwnd)
    {
        while (hwnd)
        {
            hwnd = GetAncestor(hwnd, GA_PARENT);
            if (hwnd)
            {
                Control *c = ControlFromHandle(hwnd);
                if (c)
                    return c;
            }
        }
        return NULL;
    }

    void Application::addcontrolwithhandle(Control *control, HWND handle)
    {
        std::map<HWND,Control*>::const_iterator finder = controlhandles.find(handle);
        if (finder == controlhandles.end() || finder->first != handle)
            controlhandles[handle] = control;
        else if (control != finder->second)
            throw L"This handle is already set for a control.";
    }

    void Application::controlhandledestroyed(Control *control, HWND handle)
    {
        std::map<HWND,Control*>::iterator finder = controlhandles.find(handle);
        if (finder == controlhandles.end() || finder->first != handle)
            return;
        controlhandles.erase(finder);
    }


    void Application::getclass(const std::wstring &realclassname, std::wstring &classname, WNDPROC &classwndproc)
    {
        auto it = classes.find(realclassname);
        if (it == classes.end() || it->first != realclassname)
            return;
        classname = it->second.classname;
        classwndproc = it->second.wndproc;
    }

    void Application::registerclass(const std::wstring &classname, const std::wstring &regclassname, WNDPROC regclasswndproc)
    {
        auto it = classes.find(classname);
        if (it == classes.end() || it->first != classname)
            classes[classname] = RegClassData(regclassname, regclasswndproc);
        else if (regclassname != it->second.classname)
            throw L"This handle is already set for a control.";
    }

    LRESULT Application::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        MEASUREITEMSTRUCT *mi;
        DRAWITEMSTRUCT *ds;
        Form *f, *f2;

        switch (uMsg)
        {
        case WM_ACTIVATEAPP:
            active = (wParam == TRUE);
            break;
        case WM_CLOSE:
            /*
            if (wParam != 123)
            {
                std::vector<Form*> destroylist;
                std::for_each(forms.begin(), forms.end(), [&destroylist](Form *f) -> bool {
                    if (!f->Parent())
                        destroylist.push_back(f);
                    return true;
                });
                std::for_each(destroylist.begin(), destroylist.end(), [this](Form *f) -> bool {
                    if (std::find(forms.begin(), forms.end(), f) != forms.end())
                        f->Destroy();
                    return true;
                });
                PostMessage(Handle(), WM_CLOSE, 123, lParam);
                return true;
            }
            else
            */
            PostQuitMessage(lParam);
            return 0;
        case WM_SYSCOLORCHANGE:
            UpdateColors();
            SystemControl::DeleteBrushes();
            for (auto it = controlhandles.begin(); it != controlhandles.end(); ++it)
                PostMessage(it->first, wmSysColorChanged, 0, 0);
            break;
        case WM_SETTINGCHANGE:
            UpdateSettings();
            break;
        case WM_DISPLAYCHANGE:
            screen->UpdateDisplays();
            break;
        case WM_THEMECHANGED:
            themes->ThemesChanged();
            PostMessage(Handle(), wmThemeChanged, 0, 0);
            UpdateFontSizes();
            for (auto it = controlhandles.begin(); it != controlhandles.end(); ++it)
                PostMessage(it->first, wmThemeChanged, 0, 0);
            break;
        case wmControlDestroyed:
            controlhandledestroyed((Control*)wParam, (HWND)lParam);
            break;
        case wmFormActivated:
            // wParam is a pointer to the form being activated. lParam is the handle of the form being deactivated.
            f = NULL;
            if (active && wParam == 0 && lParam == 0 && activeform != NULL && activeform != (Form*)wParam) // wParam is 0 when a form sends this message before it is deactivated. lParam is only 0 if the window to be activated is unknown. It is set to 1 if the main form is being destroyed.
            {
                // Windows would select any random form that was active before the deactivated one got activated, even if that form belongs to a different thread or process. Avoid that by activating the second top window in our thread.
                f = GetNextForm(activeform);
                if (!f)
                {
                    f = GetTopForm();
                    if (f == activeform)
                        f = NULL;
                }

                if (f)
                {
                    f2 = f;
                    if (OnSelectActiveForm)
                    {
                        OnSelectActiveForm(this, AppSelectActiveFormParameters(activeform, f2));
                        if (f2 != activeform)
                            f = f2;
                    }
                }
            }

            if (f)
            {
                activeform = f;
                f->Focus();
            }
            else
                activeform = (Form*)wParam;
            break;
        case WM_MEASUREITEM:
            mi = (MEASUREITEMSTRUCT*)lParam;
            if (mi->CtlType != ODT_MENU)
                break;
            ((MenuItem*)mi->itemData)->Measure(mi->itemWidth, mi->itemHeight);
            break;
        case WM_DRAWITEM:
            if (wParam != 0)
                break;
            ds = (DRAWITEMSTRUCT*)lParam;
            ((MenuItem*)ds->itemData)->Draw(ds);
            break;
        case WM_DWMCOMPOSITIONCHANGED:
            BOOL b;
            if (DwmIsCompositionEnabled_REDEFINED)
            {
                dwmcomp = DwmIsCompositionEnabled_REDEFINED(&b) == S_OK;
                if (dwmcomp)
                    dwmcomp = b != FALSE;
            }
            break;
        }
        return DefWindowProc(messageform->Handle(), uMsg, wParam, lParam);
    }

    /**
     * The program is active if the window which has the input focus belongs to it.
     * \return \a true if the program is active and \a false if not.
     * \sa ActiveForm()
     */
    bool Application::Active()
    {
        return active;
    }

    void Application::UpdateColors()
    {
        Color::colors[clScrollbar] = GetSysColor(0);
        Color::colors[clBackground] = GetSysColor(1);
        Color::colors[clActiveCaption] = GetSysColor(2);
        Color::colors[clInactiveCaption] = GetSysColor(3);
        Color::colors[clMenu] = GetSysColor(4);
        Color::colors[clWindow] = GetSysColor(5);
        Color::colors[clWindowFrame] = GetSysColor(6);
        Color::colors[clMenuText] = GetSysColor(7);
        Color::colors[clWindowText] = GetSysColor(8);
        Color::colors[clCaptionText] = GetSysColor(9);
        Color::colors[clActiveBorder] = GetSysColor(10);
        Color::colors[clInactiveBorder] = GetSysColor(11);
        Color::colors[clAppWorkspace] = GetSysColor(12);
        Color::colors[clHighlight] = GetSysColor(13);
        Color::colors[clHighlightText] = GetSysColor(14);
        Color::colors[clBtnFace] = GetSysColor(15);
        Color::colors[cl3DShadow] = GetSysColor(16);
        Color::colors[clGrayText] = GetSysColor(17);
        Color::colors[clBtnText] = GetSysColor(18);
        if (Color::colors[clGrayText] == Color(0, 0, 0))
            Color::colors[clGrayText] = Color(clBtnFace).Mix(clBtnText);
        Color::colors[clInactiveCaptionText] = GetSysColor(19);
        Color::colors[cl3DHighlight] = GetSysColor(20);
        Color::colors[cl3DDkShadow] = GetSysColor(21);
        Color::colors[cl3DLight] = GetSysColor(22);
        Color::colors[clInfoText] = GetSysColor(23);
        Color::colors[clInfoBK] = GetSysColor(24);
        Color::colors[clHotlight] = GetSysColor(26);
        Color::colors[clGradientActiveCaption] = GetSysColor(27);
        Color::colors[clGradientInactiveCaption] = GetSysColor(28);
        Color::colors[clMenuHilight] = GetSysColor(29);
        Color::colors[clMenubar] = GetSysColor(30);

        Color::colors[clBlack] = Color(0, 0, 0);
        Color::colors[clWhite] = Color(255, 255, 255);
        Color::colors[clRed] = Color(255, 0, 0);
        Color::colors[clLime] = Color(0, 255, 0);
        Color::colors[clGreen] = Color(0, 128, 0);
        Color::colors[clBlue] = Color(0, 0, 255);
        Color::colors[clYellow] = Color(255, 255, 0);
        Color::colors[clFuchsia] = Color(255, 0, 255);
        Color::colors[clPurple] = Color(128, 0, 128);
        Color::colors[clCyan] = Color(0, 255, 255);
        Color::colors[clGray] = Color(128, 128, 128);
        Color::colors[clSilver] = Color(192, 192, 192);
        Color::colors[clMaroon] = Color(128, 0, 0);
        Color::colors[clNavy] = Color(0, 0, 128);
        Color::colors[clOlive] = Color(128, 128, 0);
        Color::colors[clTeal] = Color(0, 128, 128);

        if (stockcanvas)
            ((StockCanvas*)stockcanvas)->SettingsChanged();
    }

    void Application::UpdateSettings()
    {
        UpdateFontSizes();
        if (stockcanvas)
            ((StockCanvas*)stockcanvas)->SettingsChanged();
    }

    void Application::UpdateFontSizes()
    {
        Font tmp = std::move(uifont);
        Font tmp2 = std::move(menufont);
        memset(&ncmetrics, 0, sizeof(ncmetrics));
        ncmetrics.cbSize = sizeof(NONCLIENTMETRICS);

    #ifndef __MINGW32__
    // MinGW sources are so old, they don't have such things in the struct.
    #if (WINVER >= 0x0600)
        // The NONCLIENTMETRICS structure is 4 bytes smaller in pre Vista windows but only when the structure is accessed in Vista or later.
        if (Win32MajorVersion < 6)
            ncmetrics.cbSize -= sizeof(ncmetrics.iPaddedBorderWidth);
    #endif
    #else
        // In the MingW32 source, the NONCLIENTMETRICS structure doesn't include iPaddedBorderWidth, so its size must be decreased if they include it in the future.
        //if (Win32MajorVersion < 6)
        //    ncmetrics.cbSize -= sizeof(ncmetrics.iPaddedBorderWidth);
        static_assert(sizeof(NONCLIENTMETRICS) == 500, L"New mingw source updated. NONCLIENTMETRICS structure now includes extra value for iPaddedBorderWidth!");
    #endif
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncmetrics.cbSize, &ncmetrics, 0);
        uilogfont = ncmetrics.lfMessageFont;
        uifont = Font(uilogfont);
        menulogfont = ncmetrics.lfMenuFont;
        menufont = Font(menulogfont);
        if (tmp.HandleCreated())
        {
            for (auto it = controlhandles.begin(); it != controlhandles.end(); it++)
            {
                HWND h = it->first;
                if ((HFONT)SendMessage(h, WM_GETFONT, 0, 0) == tmp.Handle())
                    PostMessage(h, WM_SETFONT, (WPARAM)uifont.Handle(), (LPARAM)(it->second->IsVisible() ? TRUE : FALSE));
                if ((HFONT)SendMessage(h, WM_GETFONT, 0, 0) == tmp2.Handle())
                    PostMessage(h, WM_SETFONT, (WPARAM)menufont.Handle(), (LPARAM)(it->second->IsVisible() ? TRUE : FALSE));
            }
        }
    }

    /**
     * The dialog mode indicates which forms will be disabled if a form is shown with Form::ShowModal().
     * \return The default dialog mode.
     * \sa SetDialogMode();
     */
    DialogModes Application::DialogMode()
    {
        return dlgmode;
    }

    /**
     * The dialog mode indicates which forms will be disabled if a form is shown with Form::ShowModal().
     * \param newdialogmode The dialog mode to set as the new default.
     * \sa DialogMode();
     */
    void Application::SetDialogMode(DialogModes newdialogmode)
    {
        dlgmode = newdialogmode;
    }

    void Application::DeregisterControlTooltip(Control *control)
    {
        tooltip->Deregister(control);
    }

    void Application::RegisterControlTooltip(Control *control)
    {
        tooltip->Register(control);
    }

    LRESULT CALLBACK AppWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Control *control = application->ControlFromHandle(hwnd);
        if (!control)
        {
            if (uMsg == WM_CREATE || _creation_window)
            {
                if (uMsg == WM_CREATE)
                {
                    CREATESTRUCT cs = *(CREATESTRUCT*)lParam;
                    control = (Control*)cs.lpCreateParams;
                }
                else
                    control = _creation_window;
                _creation_window = NULL;
                control->handle = hwnd;
                application->addcontrolwithhandle(control,hwnd);
            }
            else
                throw L"Unknown handle.";
        }

        if (control)
            return control->PassMessage(uMsg, wParam, lParam);
        else 
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    /**
     * Use a message box to show important information to the user, or to present a question with choices that
     * need to be made before the program can continue its normal operation. The message box disables the
     * window which was active in the application before it is shown.
     * \param message Message text to show in the main part of the message box.
     * \param caption Title or caption of the message box.
     * \param buttons Buttons displayed in the message box.
     * \param icon An optional icon to show next to the message.
     * \param def The button selected by default.
     * \return The selection made by the user by pushing one of the buttons.
     */
    ModalResults ShowMessageBox(const std::wstring& message, const std::wstring& caption, MessageBoxButtons buttons, MessageBoxIcons icon, MessageBoxDefaultButtons def)
    {
        Dialog *dlg = new Dialog;

        HWND active = application->ActiveForm() ? application->ActiveForm()->Handle() : NULL; // Remember which form activated this one so it can be restored.

        ModalResults result = mrNone;
        dlg->DisableForms(application->ActiveForm());
        try
        {
            result = (ModalResults)MessageBox(GetForegroundWindow(), message.c_str(), caption.c_str(), (int)buttons | (int)icon | (int)def);
        }
        catch(...)
        {
            ;
        }
        dlg->Destroy();
        if (active && !IsWindow(active))
            active = NULL;
        if (active)
            SetActiveWindow(active);
        return result;
    }


}
/* End of NLIBNS */


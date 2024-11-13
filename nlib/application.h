#pragma once

#include "canvas.h"
#include "events.h"

#ifdef __MINGW32__
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#else
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
#endif

namespace NLIBNS
{

    class Application;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum DialogModes : int;
#else
    enum DialogModes;
#endif

    class Tooltip;

    class AppMessageWindow;
    /// Singleton class of the global [application](\ref ::application) variable running the main message loop, storing a list of controls, and managing some global values.
    /**
     * The global [application](\ref ::application) variable is valid till the program leaves the Run()
     * method.
     *
     * Use MainForm() to get the main form and ActiveForm() for the currently active form in the program.
     * Check the value of Active() to see if the program has the input focus in the system. Call
     * HandleOneMessage() and HandleMessages() to process system messages, or to create a local message loop
     * while Terminated() doesn't show that further message processing is not possible.
     *
     * The Application object has a message-only window, which can be accessed via Handle(). This handle
     * is only created to receive messages about global setting changes. If the program's taskbar buttons
     * need to be accessed, use the handle of the corresponding windows.
     */
    class Application
    {
    private:
        std::map<HWND,Control*> controlhandles;
        struct RegClassData
        {
            std::wstring classname;
            WNDPROC wndproc=nullptr;
            RegClassData() : wndproc(NULL) {}
            RegClassData(const std::wstring classname, WNDPROC wndproc) : classname(classname), wndproc(wndproc) {}
        };
        std::map<std::wstring, RegClassData> classes;
        std::list<Form*> forms;

        static void SetInstance(HINSTANCE hinst);
        static void SetCmdShow(int cmdshow);
        static void SetArgsCnt(int argscnt);
        static void AddArgsVar(wchar_t *argsvar);

        static void InitValues();

        Form *mainform = nullptr; // When this form closes, the application quits.
        Form *activeform = nullptr; // The form which is active when the application is.
        bool active = true; // The application is the active one.

        bool terminated = false; // PostQuitMessage has been sent.
    
        bool dwmcomp = false; // The current result of DwmIsCompositionEnabled.

        NONCLIENTMETRICS ncmetrics;
        Font uifont;
        Font menufont;
        LOGFONT uilogfont; // The logfont used to create uifont.
        LOGFONT menulogfont; // The logfont used to create menufont.

        Canvas *stockcanvas = nullptr;
    
        DialogModes dlgmode;

        static Application *instance;
        static int initialshow;
        Application();

        AppMessageWindow *messageform;

        LRESULT MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        Tooltip *tooltip = nullptr;

        inline void MessageRound(MSG *msg);
        bool AcceleratorKeyMessage(const MSG &msg); // Handles accelerator keys (Alt+Key). Returns true if the message was handled and false if it should go through DispatchMessage().
        bool DialogKeyMessage(const MSG &msg); // Handles keyboard input only. Returns true if the message was handled and false if it should go through DispatchMessage().

        void addcontrolwithhandle(Control *control, HWND handle);
        void controlhandledestroyed(Control *control, HWND handle);

        void addform(Form *f);
        void formdestroyed(Form *f);

        void getclass(const std::wstring &realclassname, std::wstring &classname, WNDPROC &classwndproc);
        void registerclass(const std::wstring &classname, const std::wstring &regclassname, WNDPROC regclasswndproc);

        Application(const Application *copy) { }
        ~Application();

        void DeregisterControlTooltip(Control *control);
        void RegisterControlTooltip(Control *control);

        void UpdateFontSizes();

        //void RegisterRawDevices();
        //void UnregisterRawDevices();

        void UpdateColors(); // Called at startup and when a WM_SYSCOLORCHANGE is received, update system-wide color settings.
        void UpdateSettings(); // Called at startup and when a WM_SETTINGCHANGE is received, to update system-wide settings, i.e. font changes.

        friend class Control;
        friend class Form;
        friend class AppMessageWindow;
        friend LRESULT CALLBACK AppWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef __MINGW32__
        friend int APIENTRY ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#else
        friend int APIENTRY ::_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
#endif

    public:
        static Application* GetInstance(); ///< Returns the single instance of Application.
        static int InitialShow(); ///< The value which determines how the application should be first shown.
        static void OverrideShow(); ///< Forces the main form of the application to be shown as designed, ignoring the value of InitialShow().

        Font UIFont(); ///< The default UI font of the system.
        Font MenuFont(); ///< The default menu font of the system.
        const LOGFONT& UILogFont(); ///< The default UI font of the system.
        const LOGFONT& MenuLogFont(); ///< The default menu font of the system.
        const NONCLIENTMETRICS& NCMetrics(); ///< The non-client metrics of the system.

        Form* MainForm(); ///< The main form of the program.
        Form* ActiveForm(); ///< The active form of the program. 
        void SetMainForm(Form *newmainform); ///< Sets the main form of the program.

        HWND Handle(); ///< The handle of the application's window.

        int Run(); ///< Initializes global objects and runs the main message loop.

        bool HandleOneMessage(); ///< Handles a single message in the message queue, if a message is available.
        void HandleMessages(); ///< Handles messages in the message queue, until the queue is emptied.

        Control* ControlFromHandle(HWND hwnd); ///< The Control object of the passed handle.
        Control* ParentControlFromHandle(HWND hwnd); ///< The first Control object above \a hwnd in the parent chain.

        bool Active(); ///< Indicates the active status of the program.

        DialogModes DialogMode(); ///< Global dialog mode used by dialog windows which have the dmDefault mode.
        void SetDialogMode(DialogModes newdialogmode); ///< Sets the global dialog mode used by dialog windows which have the dmDefault mode.

        bool Terminated(); ///< Indicates whether further message processing is unavailable.

        bool DwmComposition(); ///< Returns whether DwmIsCompositionEnabled would return TRUE.

        AppSelectActiveFormEvent OnSelectActiveForm; ///< Triggered when a form is being deactivated without activating another (i.e. when the active form is closed or becomes hidden). The event is not triggered if the program or the user selects a form manually. 
    };

    /// %Object of the singleton Application type.
    extern ConstValue<Application, Application*> application;

    /// The default window procedure for all controls, whose type is derived from Control.
    LRESULT CALLBACK AppWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


    //extern int CoInitModel; // Change this to one of the COINIT constants used by the CoInitializeEx function, before the application runs.


}
/* End of NLIBNS */


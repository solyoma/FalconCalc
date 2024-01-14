#pragma once

#include "controlbase.h"


namespace NLIBNS
{


    class Icon;

    enum PanelBorderStyles : int {
            pbsNone, pbsSunken, pbsRaised, pbsDoubleSunken, pbsDoubleRaised, pbsSunkenRaised, pbsRaisedSunken,
#ifdef DESIGNING
            pbsCount = 7
#endif
    };

    class Panel : public ScrollableControl
    {
    private:
        typedef ScrollableControl base;

        PanelBorderStyles borderstyle;
        bool showtext;

        TextAlignments halign;
        VerticalTextAlignments valign;
        int margin;
        int vmargin;

        int BorderWidth();
    protected:
#ifdef DESIGNING
        virtual Color DefaultColor(); // Update with clBtnFace.
#endif

        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual bool ExcludeOpaqueRegion(HRGN rgn, const Rect &rgnrect, const Point &origin);
        virtual void Paint(const Rect &updaterect);

        virtual void Resizing();
        virtual void Moving();

        virtual bool TextChanging(const std::wstring &newtext);

        Rect TextRect(const Rect& client, const std::wstring &str, TextAlignments h, VerticalTextAlignments v, int hmarg, int vmarg); // The rectangle of text inside the panel if it is drawn with the given alignments and margins.

        virtual ~Panel();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        Panel();

        PanelBorderStyles InnerBorderStyle();
        void SetInnerBorderStyle(PanelBorderStyles newborder);

        TextAlignments TextAlignment();
        void SetTextAlignment(TextAlignments newalignment);
        VerticalTextAlignments VerticalTextAlignment();
        void SetVerticalTextAlignment(VerticalTextAlignments newalignment);
        int TextMargin();
        void SetTextMargin(int newmargin);
        int VerticalTextMargin();
        void SetVerticalTextMargin(int newmargin);

        bool ShowText();
        void SetShowText(bool newshowtext);
    };

#ifdef DESIGNING
    struct IconData
    {
        std::wstring filename;
        char *filedata;
        unsigned int filesize;

        IconData();
        IconData(const IconData &orig);
        IconData(IconData &&orig) noexcept;
        ~IconData();

        void Clear(); // Frees the icon data.

        bool operator==(const IconData &other);
        IconData& operator=(const IconData &orig);
        IconData& operator=(IconData &&orig) noexcept;
    };
#endif

    /* values of FormShowPositions:
     * fspDefault: The window is positioned by the system, and its size and position members are ignored.
     * fspUnchanged: The window is shown at the exact coordinates the window was set. This means the correct monitor must be taken into account by the program when specifying its bounding rectangle.
     * fspActiveMonitor: The window is shown on the monitor of the currently active window, its left and top positions relative to the monitor.
     * fspActiveMonitorCenter: The window is shown centered on the monitor of the currently active window.
     * fspPrimaryMonitor: The window is shown on the primary monitor, its left and top positions relative to the monitor.
     * fspPrimaryMonitorCenter: The window is shown centered on the primary monitor.
     * fspMainFormMonitor: The window appears on the monitor where the main form currently is.
     * fspMainFormMonitorCenter: The window appears centered on the monitor where the main form currently is.
     * fspParentMonitor: The window is shown on the monitor where its parent form is.
     * fspParentMonitorCenter: Center window on the monitor its parent is displayed on.
     * fspParentWindowCenter: The window is shown centered on its parent window.
     */
    enum FormShowPositions : int {
            fspDefault, fspUnchanged, fspActiveMonitor, fspActiveMonitorCenter, fspPrimaryMonitor,
            fspPrimaryMonitorCenter, fspMainFormMonitor, fspMainFormMonitorCenter,
            fspParentMonitor, fspParentMonitorCenter, fspParentWindowCenter,
#ifdef DESIGNING
            fspCount = 11
#endif
    };

    enum FormStates { fsNormal, fsMaximized, fsMinimized };

    // Replaced with enum in application.h which is used for message box return values as well.
    //enum ModalResults : int {
    //        mrNone, mrOk, mrCancel, mrAbort, mrIgnore, mrRetry, mrYes, mrNo, mrDefault
    //};

    enum FormBorderStyles : int {
            fbsNormal, fbsNone, fbsSingle, fbsDialog, fbsToolWindow, fbsSizeableToolWindow,
#ifdef DESIGNING
            fbsCount = 6
#endif
    };

    enum FormBorderButtons : int {
            fbbSystemMenu = 1, fbbMinimizeBox = 2, fbbMaximizeBox = 4, fbbContextHelp = 8,
#ifdef DESIGNING
            fbbCount = 4
#endif
    };

    // Action to take when closing the window.
    enum CloseActions : int { caHide, caDestroyHandle, caDeleteForm, caPreventClose };

    typedef uintset<FormBorderButtons> FormBorderButtonSet;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum DialogModes : int;
#else
    enum DialogModes;
#endif
    class Form : public ScrollableControl
    {
#ifdef DESIGNING
    public:
        Size client; // Placed here so GCC doesn't complain for reordered initializer list in constructor.
#endif
    private:
        typedef ScrollableControl base;
#ifdef DESIGNING
        FormShowPositions designformpos;
        Control *designactivecontrol; // The designer form cannot have an active control. Set and show a fake control instead, while having a NULL activecontrol.
        bool designtopmost;
#endif

        std::vector<NonVisualControl*> nvs; // Non visual children that must be deleted when the form is destroyed.

        Dialog *dialog; // Class containing information about the modal state of this form.
    
        bool mainform;
    
        Menubar *menu;

        Icon *smallicon;
        Icon *largeicon;
        bool ownsmallicon;
        bool ownlargeicon;
    
        bool posinited; // Set the position on screen depending on formpos when the handle is created.
        bool initshow; // Specifies whether the form should be shown as soon as its handle is created.
        Rect initrect; // Initial rectangle of form when created independent from monitors. If the form is not shown on the center of the screen, this rectangle is used to find its place on the selected monitor. If any value of the rectangle is CW_USEDEFAULT, the form will be shown at the system defined default position.

        FormStates formstate;
        FormShowPositions formpos;
        FormBorderStyles formborder;
        FormBorderButtonSet borderbuttons;

        bool topmost;
        bool keypreview; // The form gets the keyboard messages before the controls handle it.

        bool modal; // The form is currently shown as a modal dialog box.
        DialogModes dlgmode;
        ModalResults modalresult;

        Control *activecontrol;
        bool activating; // SetActiveControl is handled. Don't allow changing the active control before it finishes.

        void MoveToShowPosition();
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual void SaveWindow();
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code);
        void DeleteNotify(Object *object);

        virtual void ActiveFormChanging(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow); // Called when the active window changes in the system, and the form gains or loses its active state. The base implementation calls the OnActiveFormChange event.
        virtual void Activating(Form *deactivated); // Called when the form becomes the active form in the application. The base implementation calls the OnActive event.
        virtual void Deactivating(Form *activated); // Called when the form loses its active state in the application. The base implementation calls the OnDeactive event.

        virtual ~Form();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual Size DesignSize();

        virtual std::wstring ClassName(bool namespacedname);

        FormShowPositions DesignShowPosition();
        void DesignSetShowPosition(FormShowPositions newformpos);
    
        bool DesignTopmost();
        void DesignSetTopmost(bool newtopmost);

        Control* DesignActiveControl();
        void DesignSetActiveControl(Control *newactivecontrol);
#endif
        Form();
        virtual void Destroy();

        void AddNVChild(NonVisualControl *nv); // Sets ownership of the passed non visual control to the form, which must delete it when it is destroyed.
        void RemoveNVChild(NonVisualControl *nv); // Sets ownership of the passed non visual control to the form, which must delete it when it is destroyed.

        bool IconFromRawData(const char *data, int datalen);
        bool IconFromData(HMODULE module, const char *data, int datalen);

        void SetToMainForm(bool setit); // Set this form to be the main form of the application.

        virtual void Show();
        ModalResults ShowModal(); // Shows the window like a modal dialog
        void Close(); // Hides the window by sending it a WM_CLOSE message.
        ModalResults ModalResult();
        void SetModalResult(ModalResults newmodalresult);

        DialogModes DialogMode(); // Returns a value indicating what windows will be disabled when this form is shown as a dialog.
        void SetDialogMode(DialogModes newdialogmode); // Changes what windows will be disabled when this form is shown as a dialog.

        Control* TabNext(Control *current, bool forward); // Returns the control next to the passed one in the tab order. If current is NULL, the active control is used.
        //void ToScreenCenter();

        bool Active(); // Returns true if the form is the active form in the application.
        virtual void Focus(); // Brings form to the foreground and sets focus to it.

        Control* ActiveControl();
        void SetActiveControl(Control *newactivecontrol);

        FormShowPositions ShowPosition();
        void SetShowPosition(FormShowPositions newformpos);

        FormStates FormState();
        void SetFormState(FormStates newstate);

        bool Topmost();
        void SetTopmost(bool newtopmost);
        void NoTopmostRecursive(); // Removes the topmost status from the form and all its top level children.

        Menubar* Menu();
        void SetMenu(Menubar *newmenu);

        bool IconFromRawResource(HMODULE module, const wchar_t *resname); // Returns false on failure.
        bool IconFromResource(HMODULE module, const wchar_t *resname); // Returns false on failure.
        Icon* SmallIcon();
        void SetSmallIcon(Icon *newicon, bool shared = false);
        Icon* LargeIcon();
        void SetLargeIcon(Icon *newicon, bool shared = false);

        virtual void SetClientRect(const Rect &newcrect); // Client rectangle override for setting the size when initializing the form.

        Rect ControlRect(Control *c); // Returns the position and size of the child control in form client coordinates.

        bool KeyPreview(); // If it returns true, the form gets the keyboard messages before the controls handle it.
        void SetKeyPreview(bool newkeypreview); // Set whether the form should pre-process keyboard messages before its controls do.

        FormBorderStyles BorderStyle();
        void SetBorderStyle(FormBorderStyles newborderstyle);
        FormBorderButtonSet BorderButtons();
        void SetBorderButtons(FormBorderButtonSet newborderbuttons);

        // events
        FormCloseEvent OnClose;
        ActiveFormChangeEvent OnActiveFormChange; // Called when the active window changes in the system, and the form gains or loses its active state.
        FormActivateEvent OnActivate; // Called when the form becomes the active form in the application.
        FormActivateEvent OnDeactivate; // Called when the form loses its active state in the application.
    };

    class Paintbox : public ScrollableControl
    {
    private:
        typedef ScrollableControl base;
        Rect clientrect;
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void EraseBackground();

        virtual ~Paintbox();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        Paintbox();

        //void SetAcceptInput(bool newacceptinput); // Set whether the paintbox can be focused for keyboard input.
    };


}
/* End of NLIBNS */


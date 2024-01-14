#pragma once

#include "objectbase.h"
#include "events.h"
#include "canvas.h"
#include "screen.h"
#include "menu.h"
#include "comdata.h"
#include "windowfunctions.h"


// wmUserbase is the base address of messages used in the library. If a new message type
// is needed in a program, it should start from WM_APP. I.e.:
// #define amDoStuff (WM_APP + 1)

#define wmUserbase          0x7000
#define wmCaptureChanged    (wmUserbase +  0)
#define wmStartSizeMove     (wmUserbase +  1) /* Sent when the user starts sizing or moving the control's window. */
#define wmSizeMoveEnded     (wmUserbase +  2) /* Sent when the user finished sizing or moving the control's window. */
#define wmSetBounds         (wmUserbase +  3) /* WPARAM lo-hi: x1, y1, LPARAM lo-hi: width, height (new bounding rectangles). Call the DelayedSetBounds function of controls to send the message with a rectangle. */
#define wmFocusKilled       (wmUserbase +  4) /* Sent after WM_KILLFOCUS so we can act more freely with events which would otherwise cause problems. */
#define wmFormActivated     (wmUserbase +  5) /* Sent to all child controls as well as to the form when the form has gained focus. Also sent to the application object when a form is deactivated. */
#define wmFormDeactivated   (wmUserbase +  6) /* Sent to all child controls as well as to the form when the form has lost focus. */
#define wmColorChanged      (wmUserbase +  7) /* Sent when a control's color was changed due to a call to SetColor or SetParentColor. lParam is 0 if the control's color changed, 1 if the parent's color changed. */
#define wmEnableChanged     (wmUserbase +  8) /* Sent when the enabled state changed by calling SetEnabled. The controlstate is updated but the window is not enabled or disabled yet */
#define wmRequestLayout     (wmUserbase +  9) /* Send a request for the container control to call its LayoutChildren function. WPARAM when set to TRUE specifies whether this should include anchored children as well. */
#define wmHandleCreated     (wmUserbase + 10) /* Sent as a signal that the handle is created and the derived classes finished initializing their windows.*/
#define wmControlDestroyed  (wmUserbase + 11) /* Sent to the application window when a control received the WM_DESTROY message to mark it for removal from the control/handle map. WPARAM is a pointer to control, LPARAM is its handle. */
#define wmDelayedDrag       (wmUserbase + 12) /* Sent to the control when a drag-drop operation is about to start. Sent by StartDelayedDragDrop, i.e. in case drag dropping right in a mouse message would cause problems. WPARAM is a BasicDataObject or derived, LPARAM is an integer representing the value of a DragDropEffectSet. */
#define wmDelete            (wmUserbase + 13) /* Sent when the control (mainly a form) is to be deleted. Calls the control's Destroy method. After receiving this message, no member of this control can be accessed. */
#define wmImagelistChanged  (wmUserbase + 14) /* Sent by some controls that need updating in a message when the imagelist changed. */
#define wmActiveChanged     (wmUserbase + 15) /* Sent by the form to the control that lost its active state and to the control that gained the active state. WPARAM is 0 for leaving and 1 for entering the control. LPARAM is the other control which either gained or lost the active state. */
#define wmDialogKey         (wmUserbase + 16) /* Sent to the control when the enter or escape keys were pressed. */
#define wmMenuAccelerator   (wmUserbase + 17) /* Handled by forms that have a main menu. WPARAM is the shortcut that must be activated if present in the menu. */
#define wmSysColorChanged   (wmUserbase + 18) /* Sent to all controls by the application once it handled the WM_SYSCOLORCHANGED message. */
#define wmThemeChanged      (wmUserbase + 19) /* The theme used in the system has changed or it was turned on or off. Broadcasted to all controls after a WM_THEMECHANGED message. */
#define wmChildMessage      (wmUserbase + 20) /* A child control sends this message to its parent if it is not a control object. For example the editor of a Combobox is created by the system. The wParam is unused, lParam is a pointer to a MessageStruct structure. The default handler in Control calls HandleChildMessage() which mainly handles keyboard messages. */
#define wmAutoResize        (wmUserbase + 21) /* Used by controls that must resize themselves when they change and it cannot be done without handling other messages first. */

#ifdef DESIGNING
#define wmTabOrderChanged   (wmUserbase + 118) /* Sent after the control's tab order has changed. WPARAM specifies the control, LPARAM is the change in the control's tab order value. Negative values mean the new tab order is lower. */
#endif


namespace NLIBNS
{


    struct ClassParams;
    struct WindowParams;
    class Dialog;


    // Values set temporarily that can often change during the control's lifetime.
    enum ControlStates {
            csDestroying       = 0x0001, /* The control is being destroyed and it will eventually be deleted as well. */
            csDestroyingHandle = 0x0002, /* Control handle is being destroyed, and won't be usable unless it is requested. */
            csCreatingHandle   = 0x0004, /* Registering the control's class and creating its handle. */
            csRecreating       = 0x0008, /* Control's handle is being destroyed but will be recreated shortly. Save all data to be able to restore them. */
            csVisible          = 0x0010, /* (default) The control is only visible on the screen, if all its parents are visible, but this state can be set even if we want the control to show when its parents will reappear. */
            csEnabled          = 0x0020, /* (default) The control can be manipulated by the user. */
            csFocused          = 0x0040, /* The control has the focus */
            csPainting         = 0x0080, /* The control is currently doing some painting operations. Any new painting started during this time is suppressed. */
            csChildPainting    = 0x0100, /* A child control is being painted, so any call to WM_ERASEBKGND and WM_PRINTCLIENT should be ignored which are sent by the system out of our control. Calling DrawParentBackground will turn this bit off temporarily so erasing will happen when that is our purpose. */
            csChildBackground  = 0x0200, /* A child control is painting its background with DrawParentBackground on the control with this state set. */
            csInvalidating     = 0x0400, /* The control's invalidate has been called. */
            csTabFocusing      = 0x0800, /* The control is gaining focus by the user pressing the tab key. */
            csWheelScrolling   = 0x1000, /* The control received a WM_MOUSE*WHEEL message and forwarded it to the appropriate window. When this state is set and the control receives the message again, it should either ignore or handle it, without forwarding again. */
#ifdef DESIGNING
            csDeserialize      = 0x8000
#endif
    };

    // Values that usually do not change during the control's lifetime or just in rare cases.
    enum ControlStyles {
            csChild                = 0x00000001, /* (default) WS_CHILD is specified in the window styles on creation. */
            csSelfDrawn            = 0x00000002, /* (default) Call the Paint function for drawing the control. Unless csNoDefaultPaint is specified this still calls the default painting for the control. */
            csNoDefaultPaint       = 0x00000004, /* (default) Don't allow the system's default painting in WM_PAINT when csSelfDrawn or the OnPaint event handler is specified. */
            csSystemErased         = 0x00000008, /* Let the system handle WM_ERASEBKGND messages. Only used for controls that do most of their drawing in the background, and which don't use their parent's erasing to do it for them. */
            csForceBgColor         = 0x00000010, /* Used by system controls that mimic DrawParentBackground in their default WM_PAINT handling by telling their parent control to erase their background, and not draw their backgrounds in WM_ERASEBKGND. When true for a control that has its ParentBackground set to false and the parent control receives a WM_ERASEBKGND from this control's WM_PAINT, it will erase with the color of that control. */
            csMouseCapture         = 0x00000020, /* (default) When the left mouse button is down on the control, it should capture all mouse input until the button is released. */
            csWantSysKey           = 0x00000040, /* Set when the control's text can contain a system key (& + char) which might be used for alt+key activation. */
            csTransparent          = 0x00000080, /* Forces the parent container control to erase its background even under this control. */
            csParentBackground     = 0x00000100, /* Draw the parent's background on the control when it is erased, instead of using the bgbrush and the current color. */
            csParentColor          = 0x00000200, /* (default) Use the parent's color when drawing the control and its background. */
            csInTabOrder           = 0x00000400, /* The control is in the tab order of its parent window. It can only be selected by pressing the tab key, if csAcceptInput is true as well. I.e. panels can be in the tab order but usually can't get mouse or keyboard input, only the controls they hold. */
            csUpdateOnTextChange   = 0x00000800, /* (default) The control should be redrawn when its text or font changes. */
            csEraseOnTextChange    = 0x00001000, /* (default) Combined with csUpdateOnTextChange, the control should be erased before update when its text or font changes. */
            csAcceptInput          = 0x00002000, /* Set for controls which can be focused and can accept keyboard input from the user. */
            //csWantTab              = 0x00004000, /* The control handles keyboard input and uses the tab character. */
            //csWantArrows           = 0x00008000, /* The control handles keyboard input and uses the arrow keys. */
            //csWantAllKeys          = 0x00010000, /* The control handles all keyboard input, including the ESC key. */
            csNoErase              = 0x00020000, /* The control doesn't have a background to be erased. This can prevent unnecessary drawing. */
            csShowTooltip          = 0x00040000, /* (default) A tooltip is shown when the mouse hovers over the control. */
            csParentTooltip        = 0x00080000, /* (default) Use the parent's csShowTooltip setting. */
            csEraseToColor         = 0x00100000, /* (default) The background is fully or partially erased to the control's current color. */
            csParentFont           = 0x00200000, /* (default) Use the parent's font when drawing the control or setting its font handle for controls that don't draw themselves. */
            csShowDontActivate     = 0x00400000, /* Prevents the activation of the control when its state changes to visible. */
    };

    typedef uintset<ControlStates> ControlStateSet;
    typedef uintset<ControlStyles> ControlStyleSet;

    enum ControlAnchors {
            caLeft = 0x00000001,
            caTop = 0x00000100,
            caRight = 0x00010000,
            caBottom = 0x01000000,
            caClient = caLeft | caTop | caRight | caBottom,
#ifdef DESIGNING
            casCount = 4
#endif
    };

    typedef uintset<ControlAnchors> ControlAnchorSet;

    enum ControlAlignments : int {
            alTop = 0,
            alBottom = 1,
            alLeft = 2, 
            alRight = 3, 
            alClient = 4, 
            alAnchor = 5, 
            alNone = 255 /* Must be highest value: */,
#ifdef DESIGNING
            caAlCount = 7
#endif
    };

    enum ControlAlignmentOrders : int {
            caoTopBeforeLeft = 1,
            caoTopBeforeRight = 2,
            caoBottomBeforeLeft = 4,
            caoBottomBeforeRight = 8,
#ifdef DESIGNING
            caoCount = 4
#endif
    };
    typedef uintset<ControlAlignmentOrders> ControlAlignmentOrderSet;

    enum BorderStyles : int {
            bsNone, 
            bsSingle, 
            bsNormal,
            bsModern,
#ifdef DESIGNING
            bsCount = 4
#endif
    };

    enum DialogCodes : int {
            dcButton = DLGC_BUTTON,
            dcDefaultButton = DLGC_DEFPUSHBUTTON,
            dcSetSelMessages = DLGC_HASSETSEL,
            dcRadioButton = DLGC_RADIOBUTTON,
            dcStatic = DLGC_STATIC,
            dcNormalButton = DLGC_UNDEFPUSHBUTTON,
            dcWantAllKeys = DLGC_WANTALLKEYS, /* Same as DLGC_WANTMESSAGE. */
            dcWantArrows = DLGC_WANTARROWS,
            dcChars = DLGC_WANTCHARS,
            dcWantTab = DLGC_WANTTAB,
    };
    typedef uintset<DialogCodes> DialogCodeSet;

    enum WantedKeys : int {
            wkArrows =  0x1,
            wkTab =     0x2,
            wkEnter =   0x4,
            wkEscape =  0x8,
            wkOthers =  0x10,
#ifdef DESIGNING
            wkCount = 5
#endif
    };
    typedef uintset<WantedKeys> WantedKeySet;

    enum TextAlignments : int {
            taLeft = 0, taRight = 1, taCenter = 2,
#ifdef DESIGNING
            taCount = 3
#endif
    };

    enum VerticalTextAlignments : int {
            vtaTop = 0, vtaMiddle = 1, vtaBottom = 2,
#ifdef DESIGNING
            vtaCount = 3
#endif
    };

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum PanelBorderStyles : int;
    enum FormShowPositions : int;
    enum FormBorderStyles : int;
    enum FormBorderButtons : int;
#else
    enum PanelBorderStyles;
    enum FormShowPositions;
    enum FormBorderStyles;
    enum FormBorderButtons;
#endif

#ifdef DESIGNING
    // Structure passed to controls when they have to fill up a menu with items without having access to the menu itself.
    struct menu_item_data
    {
        std::wstring text; // Text of the menu item.
        tagtype tag; // Optional data set as the Tag value of the menu item. Useful in events where the sender can be converted to menu item.
        NotifyEvent OnClick; // Event called when the menu item is clicked.

        menu_item_data(std::wstring text, tagtype tag, NotifyEvent OnClick) : text(text), tag(tag), OnClick(OnClick) {}
    };
#endif


    class Canvas;
    class ContainerControl;
    class ControlCanvas;
    class ControlImage;

#ifdef DESIGNING
    class DesignProperties;
    class DesignSerializer;
    class DesignProperty;
#endif


    class ControlList; // Class which keeps track of child controls that are placed on a parent control.
    class Control : public Object
    {
    private:
        typedef Object   base;

#ifdef DESIGNING
        static Font *defaultfont;

        PopupMenu *designpopupmenu; // Dummy popup menu set with and shown in the property editor, but otherwise it does nothing at all.
        bool designvisible;

        DesignProperty *parentproperty;

        DesignProperty* ParentProperty(); // The property the control belongs to, in case it is maintained by a property on a parent control. (i.e. TabPages on a TabControl)
        void SetParentProperty(DesignProperty *prop); // Set the property that is used to maintain the control in the designer.
#endif

        // Handles used during double buffered painting on the canvas.
        static union PaintBuffer
        {
            HPAINTBUFFER dblbuf;
            HBITMAP dblbmp;
        } paintbuffer; // Double buffer data used during WM_PAINT or when calling StartDoubleBuffering(). Only a single version exists, because double buffering won't work in multiple controls at the same time. 
        static Rect doublebufrect;
        static int bufcnt; // Counter for the number of requests for double buffering
        static bool vistabuf; 
        static HBITMAP olddblbmp;

        bool HandleOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &result); // Calls the message event handler if one is present. Returns whether the handler allowed further handling of the message.

        HWND handle;
        Control *parent; // Parent OR owner of this control. (In Windows only one of them makes sense at any time. Child controls are placed on their parent, top level controls are floating windows that always stay above their parent.)
        PopupMenu *popupmenu; // Menu to be shown when the user presses then releases the right mouse button over the control.
        ControlList *controls;

        int updatecnt; // Number of times EndUpdate() must be called to resume updating of the control.

        BasicDropTarget *droptarget; // Helper class handling drag and drop and calling our DragEnter, DragMove, DragLeave and DragDrop functions.

        int dlgid;

        Rect rect; // The current window rectangle including the control's frame.
        Rect clientrect; // Saved rectangle after last WM_WINDOWPOSCHANGED. Because derived classes can handle that message early, a call to OldClientRectangle() can get them the original rectangle, without having to save it themselves.

        HDC paintdc; // DC retrieved for BeginPaint or set when other functions are called. During handling of a paint request message, this dc is used in the canvas.
        ControlCanvas *canvas;
        Font *font;

        Color color;

        BorderStyles border;

        Cursors cursor;

        WantedKeySet wantedkeys;

        Brush *bgbrush; // Brush used when erasing the background. Only created when needed (i.e. when not using the parent's brush.)
        void CreateBGBrush(); // Called when bgbrush is NULL and parentbg is false before erasing the background of the control.
        Brush* GetBGBrush(); // Returns the brush used for erasing the background, creating it if necessary. It can be an owned brush or the brush of a parent container, depending on the ParentBackground() value.

        ControlAlignments align; // Alignment of a control. When it is not alNone, the anchors are ignored.
        ControlAlignmentOrderSet alignorder; // Order how child controls with alignment of top, left, right or bottom are aligned.
        ControlAnchorSet anchors; // Stores the sides of the control that should keep their original distance from the sides of the container.
        Rect anchorpos; // Initial position of control inside its container. It indicates the distance of the control's sides from the side of the container.
        void SaveAnchorPos(const Rect &bounds); // Save the distance of the control's sides from its container's walls. Pass the bounds of the control to save as the anchored / aligned bounding rectangle.
        void SaveAnchorPos(); // Save the distance of the control's sides from its container's walls, using current control dimensions.
        void AnchorSetBounds(const Rect &bounds); // Similar to SetBounds but only called when the control has no handle and its parent is arranging control.

        Rect alignmargin;

        Size minsize;
        Size maxsize;

        int sizewidthdiff;
        int sizemodx;
        int sizeheightdiff;
        int sizemody;

        std::wstring text; // Text displayed in the control.
        std::wstring tooltext; // Text to display in the tooltip when it appears for the control if csShowTooltip is in the controlstyle.

        // user is moving or sizing the window with the mouse
        bool usersizemove;
        Rect sizemoverect;

        bool doublebuffered;

        friend LRESULT CALLBACK AppWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


        class ControlFont : public OwnedFont
        {
        private:
            typedef OwnedFont base;

            Control *owner;
            ControlFont(const ControlFont &copy) : base(FontData()) { throw L"Cannot copy control fonts"; }
        protected:
            virtual void DoChanged(const FontData &saveddata);
        public:
            ControlFont(Control *owner, const LOGFONT &lf);
        };
        void UpdateFont(const FontData &saveddata); // Updates the control's font with a created handle and the canvas font.

        Rect NCRect(); // Returns the coordinates of the non-client area in client coordinates.
        HRGN NCRegion(); // The region of the non-client area in client coordinates. The region must be freed by the caller.

        int taborder; //  The position of a control in its parent's tab list.

        int childpaintcnt; // Number of times ChildPaintToggle has been called with true minus the number of times it has been called with false.
        void ChildPaintToggle(bool toggleon); // Called by a child control to prevent the parent to erase its background while the child paints.
        Control *erasechild;
        void ChildEraseToggle(Control *child); // Called by a child control to force this control to erase its background to the child's color.

        // Top level controls are windows or other non-child controls. Top children always float above the top parent and are not placed on it.
        void RemoveTopChildren(bool passtoparent);
        void AddTopChild(Control *control);
        void RemoveTopChild(Control *control);
        std::vector<Control*>::iterator TopChildPosition(Control *control);

        std::vector<Control*> topcontrols; // Top level controls that have this one as their parent.

        void RemovingChild(Control *parent, Control *child); // Called by a child control when it is about to be removed from another control. First the control's top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.
        void ChildRemoved(Control *parent, Control *child); // Called by a child control after it has been removed from another control. First the control's original top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.
        void AddingChild(Control *parent, Control *child); // Called by a child control when it is about to be added on another control. First the control's new top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.
        void ChildAdded(Control *parent, Control *child); // Called by a child control after it has been placed on another control. First the control's new top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.

        void ParentFormChanged(Form *oldform, Form *newform);

        void ProcGetDialogCode(WORD keycode, LRESULT &result); // Handler for WM_GETDLGCODE.
        void ProcKeyDown(WORD &keycode);
        void ProcKeyUp(WORD &keycode);
        void ProcChar(UINT uMsg, WCHAR &key);

        //void LayoutAnchoredChild(Control *child); // Tells the control to modify the bounds of the passed anchored child control. Has no effect for controls that are not children of this one or which are not anchor layouted, or which has the default (alLeft | alTop) anchor set.

        friend class ControlFont;
        friend class ControlList;
#ifdef DESIGNING
        friend class DesignForm;
#endif
    protected:
        std::wstring RegisteredClassName(); // Returns the window class name for this class as it will be registered if a window of it is created.
        WNDPROC GetBuiltinWndProc(); // Returns the window procedure used by the system for built in classes, if this control is a subclass of one. Otherwise returns 0. This then can be stored in derived classes that need to call the default window procedure. Only works if the derived class defines its version of CreateClassParams and sets the params structure's wndproc value to the window procedure of the system class. Be sure to also clear the classname value in params.

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        void CreateHandle();
        void DestroyHandle();
        virtual void SaveWindow(); // Called when the window's handle is being destroyed but the control itself is not, so the control can attempt to save anything to local members that were only stored by the system.
        virtual void InitHandle(); // Called in CreateHandle when creating the window handle. Call base first in derived classes if initialization of the control needs a working handle.

        virtual void DeleteNotify(Object *object);

        virtual void FontChanged(const FontData &data) {}

        virtual void RemovingChildNotify(Control *parent, Control *child) {} // Invoked for parent controls when a child is about to be removed at some level below. First the control's top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.
        virtual void ChildRemovedNotify(Control *parent, Control *child) {} // Invoked for a parent control after a child has been removed at some level below. First the control's original top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.
        virtual void AddingChildNotify(Control *parent, Control *child) {} // Invoked for parent controls when a child is about to be added at some level below. First the control's new top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.
        virtual void ChildAddedNotify(Control *parent, Control *child) {} // Invoked for a parent control after a child has been placed at some level below. First the control's new top level parent is called, than a control on the top level parent, etc. till the original control's parent is reached.

        // The controlstyle must contain csSelfDrawn for these to work!
        virtual void HandlePaint(const Rect &updaterect, HRGN &rgn); // Calls paint. Override this to change DC clipping region etc., but not for the painting itself. Update the rgn in the second argument to set the necessary clipping rectangle. Check to see if it is 0 initially and if needed create a new one.
        virtual void Paint(const Rect &updaterect); // Override for custom painting.
        virtual void HandleErase(const Rect &area); // Calls EraseBackground after setting the clipping region if needed.
        virtual void EraseBackground(); // Override for custom background painting.
        virtual bool NCPaint(); // Override for custom non-client painting. Return true to stop further processing.

        Rect InnerPadding() const;
        void SetInnerPadding(const Rect &newpadding);

        virtual void NeedsDialogCode(WORD key, DialogCodeSet &dialogcode) {} // Called in WM_GETDLGCODE to let the control decide which keys it needs to handle. The controlstyle can contain bits like csWantTab, so it is not always necessary to use this function. Override in special cases, i.e. when the keys to handle can change depending on the control's current state.

        bool HandleDialogKey(WORD vkey); // Sends the wmDialogKey message to all child controls until one handles it and returns true.

        virtual void CaptureChanged() { }

        // Client area mouse functions.
        virtual void MouseEnter() { }
        virtual void MouseLeave() { }

        // Non-client area mouse functions.
        virtual void NCMouseEnter() { }
        virtual void NCMouseLeave() { }

        // Called after both client and non-client.
        virtual void MouseEntered() { }
        virtual void MouseLeft() { }

        virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys); // Calls the OnMouseMove event.
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys); // Calls the OnMouseDown event.
        virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys); // Calls the OnMouseUp Event.
        virtual void MouseClick();
        virtual bool WantMouseWheel(bool vertical, short delta, VirtualKeyStateSet vkeys, short x, short y); // Called for the control which receives a WM_MOUSEWHEEL or WM_MOUSEHWHEEL message. It must return true if it handles the wheel input in place. Return false to let the control currently under the mouse cursor handle it in its MouseWheel function.
        virtual bool MouseWheel(bool &vertical, short &delta, VirtualKeyStateSet vkeys, short x, short y); // Lets the control handle the mouse wheel input. Return false if the parent of the control should handle the message instead. If true is returned, the default procedure handles the message, which might still send it to the parent.

        virtual void NCMouseMove(short x, short y, LRESULT hittest, VirtualKeyStateSet vkeys); // Calls the OnNCMouseMove Event.
        virtual void NCMouseDown(short x, short y, MouseButtons button, LRESULT hittest, VirtualKeyStateSet vkeys); // Calls the OnNCMouseDown Event.
        virtual void NCMouseUp(short x, short y, MouseButtons button, LRESULT hittest, VirtualKeyStateSet vkeys); // Calls the OnNCMouseUp Event.

        virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys); // Calls the OnKeyPush event. Either keycode or key is 0 when this function is called. When keycode is not NULL the function was called from the WM_KEYDOWN message handler and the OnKeyDown event is called. When key is not NULL the function was called from the WM_CHAR or WM_SYSCHAR message handler and the OnKeyPress event is called.
        virtual void KeyUp(WORD &keycode, VirtualKeyStateSet vkeys); // Calls the OnKeyUp event.

        virtual void GainFocus(HWND otherwindow); // Calls the OnGainFocus event.
        virtual void LoseFocus(HWND otherwindow); // Calls the OnLoseFocus event.

        virtual void ActiveEnter(Control *other); // Calls the OnEnter event.
        virtual void ActiveLeave(Control *other); // Calls the OnLeave event.

        // Position changes.
        virtual void WindowBoundsChanged(const Rect &oldrect, const Rect &newrect); // Called from WM_WINDOWPOSCHANGED, but only when the bounding rectangle changed. Calls resize and move, so always call the base implementation for the event handling.
        virtual void Resizing(); // Called when the window size has changed, but not when it is only moved. In the default implementation it calls OnResize.
        virtual void Moving(); // Called when the window's origin (top and left) has changed, but not when it is only resized. In the default implementation it calls OnMove.

        virtual void ComputeAlignBounds(Rect &bounds); // Called by the parent while it lays out its children by their selected alignment. The control can modify its bounding rectangle within the parent. The following controls in the alignment order will be aligned by this control's bounds.

        virtual void Showing(); // Called when the window is made visible.
        virtual void Hiding(); // Called when the window is made hidden.

        virtual bool TextChanging(const std::wstring &newtext); // The control received a WM_SETTEXT message. Handle this in a derived class if the control needs update. Return true if change was handled. (i.e. control is invalidated).
        virtual bool FontChanging(); // The font for the control was changed by the user. Return true if change was handled. (i.e. control is invalidated).

        void DeleteCanvas(); // Deletes the canvas object and sets it to NULL.
        void ReleaseCanvas(); // Causes the canvas to release all resources (delete its Gdiplus objects) and resets its dc value to NULL.
        void UpdateCanvas(HDC dc); // Updates the canvas' internal DC so it can create Gdiplus objects on a new one.
        void ResetCanvas(); // Sets the font, brush and pen to the default value.

        void InitControlList(); // Makes the control into a parent control for child controls, usually in the constructor. This cannot be undone.
        bool IsControlParent(); // Returns whether the given control is a control parent.

        Rect OldClientRectangle(); // The client rectangle before the window was resized. Only valid during handling WM_WINDOWPOSCHANGED in a derived control.
        virtual void MeasureControlArea(Rect &clientrect); // The control can specify the actual client area where child controls are layed out, given client coordinates. Inner padding should not be included.

        void ExcludeClipRegion(HRGN rgn, const Rect &rgnrect, const Point &origin); // Modifies the passed region to exclude any area of the control which is completely opaque considering child controls as well. The control is relative to the region and its origin is specified in the passed argument. If the control is not transparent and doesn't have ParentBackground set, it is completely excluded from the region. If it does have ParentBackground set, it calls ExcludeOpaqueRegion and then OpaqueRect if that was unsuccessful, and only excludes the returned area from the clipping region. Set origin to the top left of the control's client area relative to the (0, 0) coordinate of the region.
        virtual bool ExcludeOpaqueRegion(HRGN rgn, const Rect &rgnrect, const Point &origin) { return false; } // Exclude the opaque regions of this control from the passed region if the control's client area is at origin relative to the region. Make sure only the area in rgnrect is excluded. Return false if it is not possible to specify a region. If the opaque region of the control is a simple rectangle, override OpaqueRect instead.
        virtual Rect OpaqueRect(); // Returns the smallest rectangle within a control's client area which is always completely opaque. Returns an empty rectangle if it is not possible to get that rectangle because any pixel might be transparent. Only called if the control has the csTransparent style or if themeing is enabled and the control has the csParentBackground style set. Override ExcludeOpaqueRegion if a complex region can be specified that is always opaque, instead of this function.

        void PositionSizeStarted(); // Called after the user started sizing or moving the control.
        void PositionSizeChanged(const Rect &newrect); // Called after the user finished sizing or moving the control or when a call to some function changes the window position or size.

        ControlStateSet controlstate;
        ControlStyleSet controlstyle;

        // Called by the droptarget member during a drag and drop operation.
        virtual DragDropEffectSet DragEnter(int formatindex, VirtualKeyStateSet keys, Point p, DragDropEffectSet effects);
        virtual DragDropEffectSet DragMove(int formatindex, VirtualKeyStateSet keys, Point p, DragDropEffectSet effects);
        virtual void DragLeave();
        virtual DragDropEffectSet DragDrop(int formatindex, VirtualKeyStateSet keys, Point p, DragDropEffectSet effects, IDataObject *data);

        virtual bool HandleSysKey(WCHAR key); // Returns true if the given alt + key combination does something in the control, if it has an & character in its text. Use ContainsAccelerator() to check. The default implementation returns false.

        virtual void HandleChildMessage(HWND hwnd, UINT &uMsg, WPARAM &wParam, LPARAM &lParam, LRESULT &result); // Called on wmChildMessage. Handles keyboard messages to provide control events.
        virtual bool HandleChildCommand(HWND hwnd, WPARAM wParam); // Handles WM_COMMAND when it was sent to the parent by a system window without a control object. Return true if further handling is unnecessary. The default implementation returns false.
        virtual bool HandleChildNotify(HWND hwnd, LPARAM lParam, HRESULT &result); // Handles WM_NOTIFY when it was sent to the parent by a system window without a control object. Return true if further handling is unnecessary. Set result to the value that should be returned in WindowProc. The default implementation returns false and doesn't set result.

        int CurrentFontHeight(); // Returns the height of the string L"My" if written with the current font.

#ifdef DESIGNING
        Control(const Control &orig) : base(orig) { throw L"?"; }
#endif
        Control();

        virtual ~Control();

        friend BasicDropTarget;
    public:
        virtual void Destroy();

#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual Size DesignSize() { return Size(152, 80); } // The default size when newly adding a control to the designer.
        virtual void SetName(const std::wstring &newname);

        virtual void InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems) {} // Called before a popup menu is to be shown above the control in the designer. If the control needs to add menu items specific to the control, it should override this function, saving the mouse position to know where the popup menu was shown.

        virtual void StartDeserialize();
        virtual void FinishDeserialize();

        PopupMenu* DesignGetPopupMenu();
        void DesignSetPopupMenu(PopupMenu *newpopup);

        // Replaces Visible() when showing the value in the property editor, because designed controls shouldn't be hidden when the user changes the Visible property.
        bool DesignVisible(); 
        void DesignSetVisible(bool newvisible);

        bool DesignAcceptInput();
        void DesignSetAcceptInput(bool newacceptinput);

        virtual Color DefaultColor();
        virtual std::wstring DefaultText();
        virtual ControlAlignments DefaultAlignment();

        int DesignTop();
        void DesignSetTop(int newtop);
        Rect DesignWindowRect();
        void DesignSetBounds(const Rect& r);

        Font& DefaultFont();

        virtual std::wstring SerializeBounds();

        bool DesignDoubleBuffered() const;
#endif

        void RecreateHandle(); // Call when the handle of a control must be destroyed and created again to reflect changes to the control.
        void HandleUnneeded(); // Call when the handle for the control is unnecessary and should be freed up temporarily. A call to this function is only successful if the control is not visible or it doesn't have visible parents that would force the control to have a handle.

        void LayoutChildren(Rect newrect, bool excludeanchored); // Places anchored or aligned child controls on the control when its size changed or it needs updating its layout. The rectangle is the client rectangle of the control used to determine child control sizes. Set excludeanchored to true when anchored controls do not need realign. i.e. when an aligned control's size is modified.

        ControlCanvas* GetCanvas(); 

        LRESULT PassMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
        bool SysKeyPressed(WCHAR key); // Returns whether the control or a child control handles the ctrl+key combination. Used when the text of the control contains an & character. The csWantSysKey must be set in the controlstate for the control to handle system keys, otherwise only child controls can handle it.

        Control* Parent() const; // Returns NULL if not a child control even if it has a parent (i.e. it is a modeless form above another). Use TopLevelParent() to get parent of non-child windows.
        virtual Form* ParentForm() const; // Returns the form the control is placed on.
        void SetParent(Control *newparent); // Makes the given container the immediate parent of this control. If a non-child control receives a parent it will become a child control.
        bool ParentOf(Control *candidate) const; // This control can be reached starting from the candidate and getting its parents.

        void DrawParentBackground(const Rect &r); // Draws the parent's background on the control disabling csChildPainting so erasing the parent's background works. The rectangle is in child client coordinates. Only works for themed apps. See Themes::AppThemed().

        void SetTopLevelParent(Control *newparent); // Sets the given control as the parent of this one if both are non-child windows, and the change wouldn't create a loop. Raises an exception otherwise.
        Control* TopLevelParent() const; // Parent for a non-child window or NULL. As opposed to Parent(), which returns the immediate parent of a child window.
        bool IsTopLevelParent(Control *candidate) const; // The control is a top level control and has the candidate as a top level parent at some level.
        int TopChildCount() const; // Number of windows belonging under this one.
        Control* TopChild(int index) const; // Windows which have this control as their top parent.

        // Drag drop source functions
        DragDropEffects StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects); // Start a drag drop operation with the data in obj.
        DragDropEffects StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects, Bitmap *dragimage, Point dragpoint); // Start a drag drop operation with the data in obj, also providing a drag image.
        DragDropEffects StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects, Control *imageowner); // Start a drag drop operation with the data in obj, also providing a window which can receive drag image request messages. (I.e. a treeview or listview which automatically generates an image from selected items.)
        //DragDropEffects StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects, int dragimagewidth, int dragimageheight, HBITMAP dragimage, Point dragpoint); // Start a drag drop operation with the data in obj, also providing a drag image.
        void StartDelayedDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects); // Start a drag drop operation with the data in obj after receiving a message. A drag image must be provided if OnDelayedDrag is set.

        // Drag drop target functions
        bool DropTarget() const; // Can the control behave as the target in a drag drop operation?
        void SetDropTarget(bool newdroptarget); // Set or unset the control to be a target of a drag drop operation.

        bool ShowDropImage() const; // Returns whether the current control is a drop target and shows the drop image when the mouse is over it.
        void SetShowDropImage(bool newshowdropimage); // Set whether the current control as a drop target can show a drop image when the mouse is over it. The function fails if the control is not yet a drop target.

        void AddDropFormat(DragDropEffectSet dropeffects, CLIPFORMAT cf, StorageMediumTypes medtype = smtHGlobal, DataViewAspects aspect = dvaContent); // Adds a drag drop format this control can accept in a drag and drop operation. This control will only behave as a drop target if the data being dragged contains the specified format. Multiple formats can be specified, the first one found in the dragged data will be used.
        int DropFormatCount() const; // Returns the number of the formats this control can accept in a drag and drop operation.
        void RemoveDropFormat(int ix); // Removes one format from the acceptable formats during a drag and drop operation.
        void DropFormat(int ix, DragDropEffectSet &dropeffects, CLIPFORMAT &cf, StorageMediumTypes &medtype, DataViewAspects &aspect) const; // Retrieves a format that was added to this drop target.
        // END drag drop

        BorderStyles BorderStyle() const;
        void SetBorderStyle(BorderStyles newborderstyle);
        Size ControlBorderSize() const;

        Point ScreenToClient(int x, int y);
        Point ClientToScreen(int x, int y);
        Point ScreenToClient(Point screenpt);
        Point ClientToScreen(Point clientpt);
        Rect ClientToScreen(Rect orig);
        Rect ScreenToClient(Rect orig);
        Point ScreenToWindow(int x, int y);
        Point WindowToScreen(int x, int y);
        Point ScreenToWindow(const Point &screenpt);
        Point WindowToScreen(const Point &windowpt);
        Rect ScreenToWindow(Rect orig);
        Rect WindowToScreen(Rect orig);
        Point ClientToWindow(int x, int y);
        Point WindowToClient(int x, int y);
        Point ClientToWindow(const Point &clientpt);
        Point WindowToClient(const Point &windowpt);
        Rect ClientToWindow(const Rect &orig);
        Rect WindowToClient(const Rect &orig);
        Point ClientToClient(int x, int y, Control *dest); // Transforms a point in the client rectangle of this control to a point in the client rectangle of dest.
        Point ClientToClient(const Point &clientpt, Control *dest); // Transforms a point in the client rectangle of this control to a point in the client rectangle of dest.
        Rect ClientToClient(const Rect &clientrect, Control *dest); // Transforms a Rect in the client rectangle of this control to a Rect in the client rectangle of dest.

        Cursors Cursor();
        void SetCursor(Cursors newcursor);

        Font& GetFont();
        void SetFont(const Font &newfont);

        virtual std::wstring Text() const;
        virtual void SetText(const std::wstring &newtext);
        int TextLength() const;

        Color GetColor() const;
        void SetColor(Color newcolor);
        bool ParentBackground() const;
        void SetParentBackground(bool newparentbg);
        bool UsingParentBackground() const; // True if control has Parent(), ParentBackground() is true, the system has themes turned on and the application uses themes.
        bool ParentColor() const;
        bool UsingParentColor() const; // True if control has Parent(), ParentColor() is true, ParentBackground() is false or the system themes are not used for this application.
        void SetParentColor(bool newparentcolor);
        bool ParentFont() const;
        bool UsingParentFont() const; // True if control has Parent() and ParentFont() is true.
        void SetParentFont(bool newparentfont);

        bool ShowTooltip() const;
        void SetShowTooltip(bool newshowtt);
        bool ParentTooltip() const;
        void SetParentTooltip(bool newparenttt);
        const std::wstring& TooltipText() const;
        void SetTooltipText(const std::wstring& newtooltext);

        void Invalidate(bool erase = true);
        void InvalidateRect(const Rect &r, bool erase = true);
        void InvalidateRegion(HRGN rgn, bool erase = true);
        void InvalidateNC();
        void InvalidateNCRect(const Rect &r);
        void InvalidateNCRegion(HRGN rgn);

        virtual bool Focused(); // Returns true when the control has the input focus.
        virtual void Focus(); // Focuses the control in the system.
        virtual void TabFocus(); // Same as Focus() but the called events will contain a true value which specifies that the control was focused by pressing the tab key and not by some other method.

        int Top() const;
        void SetTop(int newtop);
        int Left() const;
        void SetLeft(int newleft);
        int Right() const;
        int Bottom() const;
        int Width() const;
        void SetWidth(int newwidth);
        int Height() const;
        void SetHeight(int newheight);

        // Changing these retrieves the ClientRect which creates the Handle().
        int ClientWidth();
        void SetClientWidth(int newcwidth);
        int ClientHeight();
        void SetClientHeight(int newcheight);
        virtual void SetClientRect(const Rect &newcrect); // Only the Width and Height of the rectangle is used.

        Size MinimumSize() const;
        void SetMinimumSize(Size newminsize);
        int MinimumWidth() const;
        void SetMinimumWidth(int newminw);
        int MinimumHeight() const;
        void SetMinimumHeight(int newminh);

        Size MaximumSize() const;
        void SetMaximumSize(Size newminsize);
        int MaximumWidth() const;
        void SetMaximumWidth(int newmaxw);
        int MaximumHeight() const;
        void SetMaximumHeight(int newmaxh);

        int SizeWidthDiff() const;
        void SetSizeWidthDiff(int newsizewidthdiff);
        int SizeHeightDiff() const;
        void SetSizeHeightDiff(int newsizeheightdiff);
        int SizeStepWidth() const;
        void SetSizeStepWidth(int newsizestepwidth);
        int SizeStepHeight() const;
        void SetSizeStepHeight(int newsizestepheight);

        bool Enabled();
        bool IsEnabled(); // True when the control has a created handle, it is enabled and all parents are enabled as well (until the first top-level parent which is not a child control).
        void SetEnabled(bool newenabled);
        bool Visible() const; // Value of visible.
        bool IsVisible() const; // True when the control has a created handle, it is visible and all parents are visible as well (until the first top-level parent which is not a child control).
        virtual void Show();
        virtual void Hide();
        void SetVisible(bool newvisible); // Does exactly the same as show / hide depending on the value.
        HWND Handle();
        bool HandleCreated() const;
        int DlgId() const;
        Rect WindowRect() const;
        Rect ClientRect(); // WARNING! creates handle. Only call for a child control once it has a parent.
        ControlAnchorSet Anchors() const;
        void SetAnchors(ControlAnchorSet newanchors);
        ControlAlignments Alignment() const;
        void SetAlignment(ControlAlignments newalign);
        ControlAlignmentOrderSet AlignmentOrder() const;
        void SetAlignmentOrder(ControlAlignmentOrderSet newalignorder);
        void SetBounds(const Rect &bounds); // Changes the control's position and size, which corresponds to the WindowRect() function.
        void DelayedSetBounds(const Rect &newbounds); // Posts the wmResize message to the control which in turn calls SetBounds if it has a working handle. Otherwise calls SetBounds directly. Only works with coordinates that fit into 0-65535.

        Rect Padding() const;
        void SetPadding(const Rect &newpadding); // Sets the padding border size of the control for child controls.

        Rect AlignMargin() const; // The space between the sides of the parent or another aligned control and the side of this control, when aligning to some side or client area.
        void SetAlignMargin(const Rect &newalignmargin);

        void RequestLayout(bool immediate, bool excludeanchored); // Tells the container to realign its children. When immediate is false and the control has a handle, a wmRequestLayout message is posted.
        void SetAllowLayout(bool newlayouting); // When set to false, the control won't layout its aligned children even if RequestLayout is called.
        bool AllowLayout() const; // If true, the control tries to layout its aligned children when their size, visibility etc. changes.

        Control* TabFirst(); // Returns the first control in the tab order, following the control hierarchy within this container. Only visible and enabled controls are returned.
        Control* TabLast(); // Returns the last control in the tab order within this control or NULL, if no available control is found. Only visible and enabled controls are returned.
        Control* TabNext(Control *current, bool forward, bool children); // Get the next or previous control from the tab order. Set children to true if the control's children can be retrieved too.

        bool AcceptInput() const; // The control can accept keyboard input from the user.
        void SetAcceptInput(bool newacceptinput); // Set whether the control can accept keyboard input from the user. When this is false, the control won't be focused on mouse clicks and by pressing the tab key.
        int TabOrder() const; // The order of a control in its parent's tab list. This determines the order of a control when changing focus with the tab key.
        void SetTabOrder(int newtaborder); // Change the taborder of the control to a new value. Changing this value automatically changes the order of all other controls on the same container.

        const WantedKeySet& WantedKeyTypes() const;
        void SetWantedKeyTypes(WantedKeySet newtypes);

        bool DoubleBuffered() const;
        void SetDoubleBuffered(bool newdblbuff);

        void BroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam); // Passes a message to all child controls ignoring the result.

        PopupMenu* GetPopupMenu();
        void SetPopupMenu(PopupMenu *newpopupmenu);

        int ControlCount() const;
        Control* Controls(int ix);
        Control* ControlFromHandle(HWND hwnd);
        Control* ControlFromId(int controlid);
        Control* ControlAt(int x, int y, bool recursive = true, bool containeronly = false, bool disabled = false, bool hidden = false); // When recursive is false, only direct child controls are returned, otherwise the control at the position tries to find a control in its client area.
        Control* ControlAt(const Point &pt, bool recursive = true, bool containeronly = false, bool disabled = false, bool hidden = false); // When recursive is false, only direct child controls are returned, otherwise the control at the position tries to find a control in its client area.

        // Updates the z-order of the passed control.
        void MoveToTop();
        void MoveToBottom();
        void MoveUp();
        void MoveDown();
        void MoveAbove(Control *other); // Places this control in the z-order above the one passed in *other.
        void MoveBelow(Control *other); // Places this control in the z-order below the one passed in *other.

        int BeginUpdate(); // Sends WM_SETREDRAW to the window to prevent computing updates to its items, so inserting can be faster. Returns the number of times EndUpdate() must be called before contents will be updated again.
        int EndUpdate(); // When the result is 0, it sends WM_SETREDRAW to allow the window to update its contents after several changes were done. Returns the number of times EndUpdate needs to be called to be paired with every BeginUpdate call.
        int UpdateCount(); // Returns the number of times EndUpdate() must be called to start updating the contents of the control.

        // Events:
        MessageEvent OnMessage; // Handle if we need special processing or even changing messages. Set allowprocessing to false to prevent default handling.

        PaintEvent OnPaint;
        NotifyEvent OnShow;
        NotifyEvent OnHide;

        NotifyEvent OnCaptureLost; // The mouse capture was changed to another window.
        NotifyEvent OnMouseEnter; // The mouse entered the client area of the control.
        NotifyEvent OnMouseLeave; // The mouse left the client area of the control. If you need to make sure that the mouse is not over the non-client area either, use OnMouseLeft instead.
        NotifyEvent OnNCMouseEnter; // The mouse entered the non-client area of the control.
        NotifyEvent OnNCMouseLeave; // The mouse left the non-client area of the control.
        NotifyEvent OnMouseEntered; // The mouse entered the area of the control. Called after an OnMouseEnter or OnNCMouseEnter.
        NotifyEvent OnMouseLeft; // The mouse left the area of the control, and its not over the client and the non-client area either. Called after OnMouseLeave or OnNCMouseLeave was handled.
        FocusChangedEvent OnGainFocus; // The event is called when the control gains the input focus, either because it was selected in the application, or because another form or application lost focus. See also OnEnter.
        FocusChangedEvent OnLoseFocus; // The event is called when the control loses the input focus, either because another control was selected on the same form, or because another form or application gained focus. See also OnLeave.
        ActiveChangedEvent OnEnter; // The control became the active control on its form. This event is not called when the focus is switched from another form or application. See also OnGainFocus.
        ActiveChangedEvent OnLeave; // The control lost the active control status on its form. This event is not called when the focus is switched to another form or application. See also OnLoseFocus.
        KeyEvent OnKeyDown;
        KeyEvent OnKeyUp;
        KeyPushEvent OnKeyPush;
        KeyPressEvent OnKeyPress;
        MouseMoveEvent OnMouseMove;
        MouseButtonEvent OnMouseDown;
        MouseButtonEvent OnMouseUp;
        MouseButtonEvent OnDblClick;
        NotifyEvent OnClick;
        MouseWheelEvent OnWantMouseWheel; // Sent to the focused control to determine whether it wants to handle mouse messages, which are intended to some other control under the cursor. Set 'handled' to true to prevent the event to be called for a different control.
        MouseWheelEvent OnMouseWheel; // Called when the user is rolling a mouse wheel. The control under the mouse receives the call, unless the currently focused control set 'handled' in OnWantMouseWheel to true. Change 'handled' to true if you don't want the same event to be called for the parent control. Descendents of ScrollableControl automatically set 'handled' to true if they have the appropriate scroll bar. In that case setting handled to false forwards the event to the parent control.

        NCMouseMoveEvent OnNCMouseMove;
        NCMouseButtonEvent OnNCMouseDown;
        NCMouseButtonEvent OnNCMouseUp;
        NCMouseButtonEvent OnNCDblClick;
        NotifyEvent OnStartSizeMove; 
        SizePositionChangedEvent OnSizeChanged; // Called after the window finished resizing, either because the user stopped resizing or because of some other operation that resulted in a size change.
        SizePositionChangedEvent OnPositionChanged; // Called after the window position changed, either because the user stopped moving it or because of some other operation that resulted in a position change.
        SizePositionChangedEvent OnEndSizeMove; // Called at the end of a resizing or moving operation, but only when either the size or position of the control has changed.
        NotifyEvent OnResize; // Called during window resize, not only when sizing finished.
        NotifyEvent OnMove; // Called when the window is being moved.
        ComputeAlignBoundsEvent OnComputeAlignBounds; // Called while the parent control lays out its children, to give the control a chance to change its bounding rectangle.
        DialogCodeEvent OnDialogCode; // Called when some keys (i.e. the tab or arrow keys) are pressed, to determine what dialog keys are used by the control and shouldn't be handled by the system.

        DragImageRequestEvent OnDragImageRequest; // Called to provide a drag image when a dragdrop operation is started with StartDelayedDragDrop. Each reference value must be set before the event returns.
        DragDropEndedEvent OnDragDropEnded; // Called when the operation was started with StartDragDrop or StartDelayedDragDrop and it finished with some result.
        NotifyEvent OnDragLeave; // Called when the mouse leaves the control during a drag drop operation or the user cancels.
        DragDropEvent OnDragEnter; // Called when the mouse enters the control during a drag drop operation. The event should set the effect of the drag drop operation if the mouse button is released in the current state. Setting the effect to ddeMove will result in the original data deleted from the source.
        DragDropEvent OnDragMove; // Called when the mouse moves in the control during a drag drop operation. The event should set the effect of the drag drop operation if the mouse button is released in the current state. Setting the effect to ddeMove will result in the original data deleted from the source.
        DragDropDropEvent OnDragDrop; // Called when the mouse button is released over the control during a drag drop operation. The event should get the data from the IDataObject value. Setting the effect to ddeMove will result in the original data deleted from the source.
    };

    extern Control* _creation_window; // window being created. Set before a CreateWindowEx and then set to NULL when it returns.


    class ScrollableControl;

    enum ScrollbarKind : int { skHorizontal = SB_HORZ, skVertical = SB_VERT};

    enum ScrollCode : int { scPosition = SB_THUMBPOSITION, scTrack = SB_THUMBTRACK, scLineUp = SB_LINEUP, scLineDown = SB_LINEDOWN, scTop = SB_TOP, scBottom = SB_BOTTOM, scPageUp = SB_PAGEUP, scPageDown = SB_PAGEDOWN, scEndScroll = SB_ENDSCROLL };

    class ControlScrollbar 
    {
    private:
        static const int WHEEL;

        ScrollableControl *owner;
        ScrollbarKind kind;
        int linestep;
        int pagestep;
        bool visible;
        int page;

        int wheeldelta;

        tagtype tag;

        int pos;
        int range;
        bool enabled;

        //bool autosize;

        void HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool invalidate);
        void HandleMouseWheel(short delta, VirtualKeyStateSet vkeys, short x, short y, bool invalidate);
        void innersetpos(int value, ScrollCode code = scPosition, bool invalidate = true);
        void innerset(int newmax, int newpage, bool newenabled);

        void setscrolldata();

        ControlScrollbar(ScrollableControl *aowner, ScrollbarKind akind);
        void Recreate();
        ScrollbarKind Kind();

        friend class ScrollableControl;
    public:
        int LineStep();
        void SetLineStep(int newlinestep);
        int PageStep();
        void SetPageStep(int newpagestep);
        int Range();
        void SetRange(int newrange);
        int Position();
        void SetPosition(int newpos);
        int ThumbSize();
        void SetThumbSize(int newsize);
        bool Visible();
        void SetVisible(bool newvisible);
        bool Enabled();
        void SetEnabled(bool newenabled);

        bool IsVisible(); // Returns true if the scroll bar is currently visible, taking up space in the non-client area of the control.
    };

    class ScrollableControl : public Control
    {
    private:
        typedef Control base;

        ControlScrollbar *vbar;
        ControlScrollbar *hbar;
        bool autosizescroll;
        bool scrollresizing;

        float hautopagestep;
        float vautopagestep;

        bool invalidateonscroll;

        friend class ControlScrollbar;
    protected:
        ScrollableControl(const ScrollableControl &copy) : base(copy) {};

        virtual void CreateClassParams(ClassParams &params);
        virtual void InitHandle();
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual bool MouseWheel(bool &vertical, short &delta, VirtualKeyStateSet vkeys, short x, short y);

        virtual void Showing();

        // Called by the scrollbar controls when the scrollbar position changes or any scroll message arrives.
        virtual void Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code);

        void NeedScrollbars(); // Call this to create scrollbars for controls which can display them but don't have it
        void MeasureScrollClient(int &uw, int &uh, int &hw, int &hh, bool &hvis, bool &vvis, bool &hnohide, bool &vnohide); // Calls GetOverflow to measure scroll overflow.

    
        // GetOverflow: called when auto scrollsize is true. override to be able to make an event for this in derived classes.
        // Initial parameters:
        //   uw - the future client width of the window.
        //   uh - the future client height of the window
        // Out parameters:
        //   uw - number of units that fit the future client width
        //   uh - number of units that fit the future client height (i.e. number of rows in a listview that fully fit the window's client height)
        //   hw - number of units horizontally only half visible or not visible outside the client size (usually [all_w - uw])
        //   hh - number of units vertically only half visible or not visible outside the client size (usually [all_h - uh])
        //   horzshowdisabled - In case hw is 0 (no hidden or partially hidden units), still show a horizontal scroll bar but make it disabled. Setting a scrollbar non visible still hides it. Default: false.
        //   vertshowdisabled - In case hh is 0 (no hidden or partially hidden units), still show a vertical scroll bar but make it disabled. Setting a scrollbar non visible still hides it. Default: false.
        // Default behavior:
        //   uw and uh is kept unchanged, hw and hh set to scrollbar range minus uw or uh
        virtual void GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide);

        virtual void WindowBoundsChanged(const Rect &oldrect, const Rect &newrect);

        ScrollableControl();
        virtual ~ScrollableControl();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        ControlScrollbar *VScroll();
        ControlScrollbar *HScroll();

        // When true and the client area changes or ScrollResize is called, calls GetOverflow and ScrollOverflowEvent events to determine whether the scrollbars should be visible, disabled or their sizes should be changed.
        bool AutoSizeScroll(); 
        void SetAutoSizeScroll(bool setauto);

        // Units to step when AutoSizeScroll is true, compared to the number of visible units when clicking on the page step area of the scrollbar with the mouse.
        // For example when this is 0.5 and 11 rows are visible in a grid, clicking in the scrollbar makes the control scroll int(11 * 0.5) rows.
        // Setting this to a negative number causes the bar to scroll visible units minus the absolute integer part of autopagestep.
        // If AutoSizeScroll is true and AutoPageStep is set to 0, the page step will be equivalent to either the number of visible units minus the current line step,
        // or a half line step if that's greater, but at least 1.
        float HAutoPageStep(); 
        void SetHAutoPageStep(float newhautopagestep);
        float VAutoPageStep();
        void SetVAutoPageStep(float newvautopagestep);

        void ScrollResize(); // Automatically called when the scrollable area changed (i.e. because of control resize), or can be called when the number of the items in the displayed area (i.e. the number of rows in a grid) changed. It can be used if ScrollOverflowEvent is set or AutoSizeScroll() is true.

        bool InvalidateOnScroll(); // True if the control invalidates itself during thumb tracking (when the mouse button is down and the scroll thumb bar is being moved).
        void SetInvalidateOnScroll(bool newinvalidateonscroll); // Set to true to make the control invalidate itself during thumb tracking (when the mouse button is down and the scroll thumb bar is being moved).

        // Short for setting the scrollbars' properties.
        int HPos();
        int VPos();
        int HRange();
        int VRange();
        int HLineStep();
        int VLineStep();
        int HPageStep();
        int VPageStep();

        void SetHPos(int val);
        void SetVPos(int val);
        void SetHRange(int val);
        void SetVRange(int val);
        void SetHLineStep(int val);
        void SetVLineStep(int val);
        void SetHPageStep(int val);
        void SetVPageStep(int val);

        // Triggered when one of the control's scrollbars position has changed.
        ScrollEvent OnScroll;
        // Triggered when computing the new size of the scrollbars if they are automatically updated. Set the number of visible
        // items (rows, pixels etc.) in the passed visible window size, and also those that are outside the visible window.
        ScrollOverflowEvent OnScrollOverflow;
        // Triggered after the scrollbars have been updated, if they update every time the control is resized.
        NotifyEvent OnScrollAutoSized;
    };

    class ControlList
    {
    private:
        typedef ScrollableControl base;

        Control *owner;

        // padding for aligned controls
        Rect padding;
        Rect innerpadding;
    
        bool allowlayout; // The control is allowed to layout its children when their position, visibility or the control's dimensions change.
        bool layoutrequested; // Non immediate RequestLayout was called which sends a message instead and sets this value to true. Once the message arrives this is set to false and all following wmRequestLayout messages are ignored.


        // Only used when aligning children, for saving their position after the alignment.
        struct DeferredPosition
        {
            Rect bounds;
            Control *control;

            bool operator< (const DeferredPosition &b) const;

            DeferredPosition() {};
            DeferredPosition(Control *control) : control(control) { }
        };
        std::vector<DeferredPosition> alignvector; // Only used when aligning children, for knowing the order how to align them.

        bool tabinited; // When set to true, the controls added to this form have a valid taborder value. Until that, the tab order is set arbitrarily. Once a tab order is requested from any child control, the tab order is initialized.
    
        std::vector<Control*> controls; // All controls on this container in their z-order. (Which is the order they were added.) Starting with the control on the bottom.
        std::vector<Control*> tablist; // List of all controls that can accept focus after the tab key is pressed in the order they should be activated. Before setting tabinited to true, this list contains controls in random order./
        void InitializeTabOrder(); // Orders the controls in the tablist by their taborder, then calls UpdateChildTabOrder().

#ifdef DESIGNING
        friend DesignForm;
#endif
    public:
        virtual ~ControlList();
        ControlList(Control *owner);
        void Destroy();

        void ChildrenRecreate();

        bool HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &result); // Called from the control owner's WindowProc. Returns true if it handled the message and further processing should be skipped. result reflects the return value, if the message was handled
        void Showing(); // Called when the window is made visible.
        //virtual void Hiding(); // Called when the window is made hidden.

        void CreateClassParams(ClassParams &params);
        void CreateWindowParams(WindowParams &params);

        void LayoutChildren(Rect newrect, bool excludeanchored); // Places anchored or aligned child controls on the control when its size changed or it needs updating its layout. The rectangle is the client rectangle of the control used to determine child control sizes. Set excludeanchored to true when anchored controls do not need realign. i.e. when an aligned control's size is modified.
        //void LayoutAnchoredChild(Control *child); // Tells the control to modify the bounds of the passed anchored child control. Has no effect for controls that have no handle, which are not children of this one or which are not anchor layouted, or which has the default (alLeft | alTop) anchor set.
        void UpdateChildTabOrder(); // Set the tab order number of child controls according to their place in the tablist. This is used when first loading the items or when designing.

        void UpdateChildFonts(); // Set the font for all children that have parent font set to true.

        // On input:
        // uw - usable client width. Use this value to decide whether a horizontal scroll bar should be shown.
        // uh - usable client height. Use this value to decide whether a vertical scroll bar should be shown.
        // On output:
        // uw - number of items that would be fully visible horizontally, if the client width is the input value of uw.
        // uh - number of items that would be fully visible vertically, if the client height is the input value of uh.
        // hw - number of partly or non visible items that would be outside the client area with the input value of uw.
        // hh - number of partly or non visible items that would be outside the client area with the input value of uh.
        // hnohide - set to true if the horizontal bar should be visible and disabled if there are not enough items to scroll.
        // vnohide - set to true if the vertical bar should be visible and disabled if there are not enough items to scroll.
        virtual void GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide); 

        // move to scrollablecontrol if called from anywhere!
        //void AutoScrollSize(int x, int y); // Sets innersize and turns on autoscroll which limits how much the area for child controls can shrink when the window size changes.
        //virtual void Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code);


        void ClipRegion(HRGN rgn, const Rect &cliprect, bool background = false /*, const Point &origin = Point()*/ ); // Excludes the area of non-transparent visible children intersected with cliprect from the passed region. Rgn must point to a valid region which is created in client coordinates before the function is called. Set background to true when clipping before erasing the background.
        void ExcludeChildRegion(HRGN rgn, const Rect &rgnrect, const Point &origin); // Excludes the area of non-transparent and non-parent background child controls from rgn. If a control uses its parent's background, tries to only extract the opaque parts.

        virtual void RemoveChildren(bool deletechildren = false);
        void CreateChildHandles();

        void InitHandle();

        void SaveChildrenAnchorPos(); // Call this when LayoutChildren was prevented by calling SetSetAllowLayout(false), and we finished correcting the window's size and SetSetAllowLayout(true) restores layouting. (Usually when showing the window for the first time.)
        //void UpdateClientAnchors(int widthdiff, int heightdiff); // Fix anchoring of children that are not on anchored controls themselves on window creation, when the client rectangle is finally available.
        void SaveChildren(); // Calls SaveWindow() for all child controls which have a handle when the parent control is being destroyed.

        std::vector<Control*>::iterator ChildPosition(Control *control);

        void InvalidateBelow(Control *child, Rect childclient); // Invalidates all controls below the given child window if they overlap with the rectangle in the child control's client area.
        void InvalidateAbove(Control *child, Rect childclient); // Invalidates all transparent controls and controls having parentbackground above the passed control if they overlap with the rectangle in the child control's client area.
        void InvalidateChildrenBackground(Rect invalidaterect); // Invalidates all transparent controls and controls having parentbackground which are placed on this control.
        void InvalidateBelow(Control *child, HRGN childclient); // Invalidates all controls below the given child window if they overlap with the rectangle in the child control's client area.
        void InvalidateAbove(Control *child, HRGN childclient); // Invalidates all transparent controls and controls having parentbackground above the passed control if they overlap with the rectangle in the child control's client area.
        void InvalidateChildrenBackground(HRGN invalidaterect); // Invalidates all transparent controls and controls having parentbackground which are placed on this control.
        void InvalidateChildren(bool parentcol = true); // Invalidates all controls on the control which uses ParentBackground and ParentColor depending on the passed argument.
        void InvalidateRectChildren(const Rect &r, bool parentcol = true); // Invalidates all controls on the control which uses ParentBackground and ParentColor depending on the passed argument.
        void InvalidateRegionChildren(HRGN rgn, bool parentcol = true); // Invalidates all controls on the control which uses ParentBackground and ParentColor depending on the passed argument.
        void PassMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool withhandle = true, bool recursive = true); // Passes the message to all child controls.
        void InvalidateTransparentAbove(Control *child, Rect childclient); // Invalidates all transparent controls placed above in the z-order of the passed child control that intersect with the child client area, recursively calling this function for all parent controls with adjusted rectangle.
        void InvalidateTransparentAbove(Control *child, HRGN childclient); // Invalidates all transparent controls placed above in the z-order of the passed child control that intersect with the child client area, recursively calling this function for all parent controls with adjusted rectangle.
        void InvalidateRecursively(Rect r); // Invalidates the control and all its child controls inside the given rectangle. The rectangle is in client coordinates, but the borders are also redrawn if it has parts outside the client area.
        void InvalidateRecursively(HRGN rgn); // Invalidates the control and all its child controls inside the given rectangle. The region is in client coordinates, but the borders are also redrawn if it has parts outside the client area.
        void InvalidateBelowTransparentAbove(Control *child, Rect childclient, const Point &diff); // Finds all transparent controls above in the z-order of the passed child control and calls InvalidateBelow with the rectangle that intersects the child client area offset by diff.

        void AddChild(Control *control);
        void RemoveChild(Control *control);

        void ChangeTabOrder(Control *control, int newtaborder); // Sets a new tab order for the control, which is not necesserily the same as newtaborder, in case it is outside the range of possible values. Negative newtaborder always puts the control to the front of the tablist while a too large value makes it appear at the back.
        int ChildTabOrder(const Control *control); // Returns the tab order of a child control.

        void MoveToTop(Control *control);
        void MoveToBottom(Control *control);
        void MoveUp(Control *control);
        void MoveDown(Control *control);
        void MoveAbove(Control *above, Control *below);
        void MoveBelow(Control *below, Control *above);
        void FixChildPosition(Control *control); // Moves the passed child control in the z-order to its normal place. Needed when a window has its handle recreated because such windows are usually placed to the bottom.

        void RequestLayout(bool immediate, bool excludeanchored); // Tells the container to realign its children. When immediate is false and the control has a handle, a wmRequestLayout message is posted.
        void SetAllowLayout(bool newlayouting); // When set to false, the control won't layout its aligned children even if RequestLayout is called.
        bool AllowLayout(); // If true, the control tries to layout its aligned children when their size, visibility etc. changes.

        Control* TabFirst(); // Returns the first control in the tab order, following the control hierarchy within this container. Only visible and enabled controls are returned.
        Control* TabLast(); // Returns the last control in the tab order within this container or NULL, if no available control is found. Only visible and enabled controls are returned.
        Control* TabNext(Control *current, bool forward, bool children);

        int ControlCount();
        Control* Controls(int ix);
        Control* ControlFromHandle(HWND hwnd);
        Control* ControlFromId(int controlid);
        Control* ControlAt(int x, int y, bool recursive = true, bool containeronly = false, bool disabled = false, bool hidden = false); // When recursive is false, only direct child controls are returned, otherwise the control at the position tries to find a control in its client area.
        Control* ControlAt(const Point &pt, bool recursive = true, bool containeronly = false, bool disabled = false, bool hidden = false); // When recursive is false, only direct child controls are returned, otherwise the control at the position tries to find a control in its client area.

        bool SysKeyPressed(WCHAR key); // Tries to pass each children control the given alt+key combination, until one handles it.
        bool HandleDialogKey(WORD vkey); // Asks the child controls to handle the VK_RETURN and VK_ESCAPE dialog keys and returns whether one handled it.

        void BroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam); // Passes a message to all child controls ignoring the result.

        Rect Padding() const;
        void SetPadding(const Rect &newpadding);
        Rect InnerPadding() const;
        void SetInnerPadding(const Rect &newpadding);
    };



}
/* End of NLIBNS */


#pragma once


namespace NLIBNS
{


    // Template class of event types, with a caller class that contains a function to be called when raising an event.
    template<typename EventType>
    class Event
    {
    private:
        typedef Event<EventType>  selftype;

        struct EventObjBase
        {
            virtual void Call(void *sender, EventType param) = 0;
        };

        template<typename T, class Caller>
        struct EventObj : public EventObjBase
        {
            typedef void (Caller::*EventProc)(void *sender, T type);

            Caller *caller;
            EventProc func;
            virtual void Call(void *sender, EventType param)
            {
                (caller->*func)(sender, param);
            }

            EventObj(Caller *c, EventProc f) : caller(c), func(f)
            {
            }
        };

        template<typename T>
        struct EventObj<T, void> : public EventObjBase
        {
            typedef void (*EventProc)(void *sender, T type);

            EventProc func;
            virtual void Call(void *sender, EventType param)
            {
                func(sender, param);
            }

            EventObj(EventProc f) : func(f)
            {
            }
        };

        std::shared_ptr<EventObjBase> eventobj;

    public:
        Event()
        {
        }

        ~Event()
        {
        }

        Event(const selftype &other)
        {
            eventobj = other.eventobj;
        }

        Event(selftype &&other) noexcept
        {
            eventobj.swap(other.eventobj);
        }

        template<class Caller>
        Event(Caller *c, void (Caller::*f)(void*, EventType) )
        {
            eventobj.reset(new EventObj<EventType, Caller>(c, f));
        }

        Event(void (*f)(void*, EventType) )
        {
            eventobj.reset(new EventObj<EventType, void>(f));
        }

        operator bool()
        {
            return (bool)eventobj;
        }

        void operator() (void *invoker, EventType param)
        {
            eventobj->Call(invoker, param);
        }

        selftype& operator=(const selftype &other)
        {
            eventobj = other.eventobj;
            return *this;
        }

        selftype& operator=(selftype &&other) noexcept
        {
            if (eventobj != other.eventobj)
            {
                //eventobj.reset();
                eventobj.swap(other.eventobj);
            }
            return *this;
        }

        selftype& operator=(selftype *other)
        {
            if (other)
                eventobj = other->eventobj;
            else
                eventobj.reset();
            return *this;
        }

    };

    template<typename Caller, typename EventType>
    Event<EventType> CreateEvent(Caller *c, void (Caller::*func)(void *sender, EventType t))
    {
        Event<EventType> e(c, func);
        return e;
    }

    template<typename EventType>
    Event<EventType> CreateEvent(void (*func)(void *sender, EventType t))
    {
        Event<EventType> e(func);
        return e;
    }

    struct EventParameters { };
    typedef Event<EventParameters> NotifyEvent;


#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum ScrollbarKind : int;
    enum ScrollCode : int;
    enum TextAlignments : int;
    enum VerticalTextAlignments : int;
#else
    enum ScrollbarKind;
    enum ScrollCode;
    enum TextAlignments;
    enum VerticalTextAlignments;
#endif
    struct ScrollParameters
    {
        ScrollbarKind kind;
        int oldpos;
        int pos;
        ScrollCode code;

        ScrollParameters(ScrollbarKind kind, int oldpos, int pos, ScrollCode code) : kind(kind),oldpos(oldpos),pos(pos),code(code) { }
    };
    typedef Event<ScrollParameters> ScrollEvent;

    struct ScrollOverflowParameters
    {
        int &visiblewidth;
        int &visibleheight;
        int &hiddenwidth;
        int &hiddenheight;
        bool &horzdisablenothide;
        bool &vertdisablenothide;

        ScrollOverflowParameters(int &visiblewidth, int &visibleheight, int &hiddenwidth, int &hiddenheight, bool &horzdisablenothide, bool &vertdisablenothide) : visiblewidth(visiblewidth), visibleheight(visibleheight), hiddenwidth(hiddenwidth), hiddenheight(hiddenheight), horzdisablenothide(horzdisablenothide), vertdisablenothide(vertdisablenothide) { }
    };
    typedef Event<ScrollOverflowParameters> ScrollOverflowEvent;

    class Canvas;
    struct PaintParameters
    {
        Canvas *canvas;
        Rect updaterect;
        PaintParameters(Canvas *canvas, Rect updaterect) : canvas(canvas), updaterect(updaterect) { }
    };
    typedef Event<PaintParameters> PaintEvent;

    struct SizePositionChangedParameters
    {
        Rect oldrect;
        Rect newrect;

        SizePositionChangedParameters(Rect oldrect, Rect newrect) : oldrect(oldrect), newrect(newrect) { }
    };
    typedef Event<SizePositionChangedParameters> SizePositionChangedEvent;


#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum VirtualKeyStates : int;
#else
    enum VirtualKeyStates;
#endif
    typedef uintset<VirtualKeyStates> VirtualKeyStateSet;
    struct MouseMoveParameters
    {
        short x;
        short y;
        const VirtualKeyStateSet &vkeys;

        MouseMoveParameters(short x, short y, const VirtualKeyStateSet &vkeys) : x(x), y(y), vkeys(vkeys) { }
    };
    typedef Event<MouseMoveParameters> MouseMoveEvent;

    struct NCMouseMoveParameters
    {
        short x;
        short y;
        LRESULT hittest;
        const VirtualKeyStateSet &vkeys;

        NCMouseMoveParameters(short x, short y, LRESULT hittest, const VirtualKeyStateSet &vkeys) : x(x), y(y), hittest(hittest), vkeys(vkeys) { }
    };
    typedef Event<NCMouseMoveParameters> NCMouseMoveEvent;

    struct MouseButtonParameters
    {
        short x;
        short y;
        MouseButtons button;
        const VirtualKeyStateSet &vkeys;

        MouseButtonParameters(short x, short y, MouseButtons button, const VirtualKeyStateSet &vkeys) : x(x), y(y), button(button), vkeys(vkeys) { }

    };
    typedef Event<MouseButtonParameters> MouseButtonEvent;

    struct NCMouseButtonParameters
    {
        short x;
        short y;
        MouseButtons button;
        LRESULT hittest;
        const VirtualKeyStateSet &vkeys;

        NCMouseButtonParameters(short x, short y, MouseButtons button, LRESULT hittest, const VirtualKeyStateSet &vkeys) : x(x), y(y), button(button), hittest(hittest), vkeys(vkeys) { }

    };
    typedef Event<NCMouseButtonParameters> NCMouseButtonEvent;

    struct WantMouseWheelParameters
    {
        bool vertical;
        short delta;
        VirtualKeyStateSet vkeys;
        short screen_x;
        short screen_y;

        bool &handled; // Whether the control handles the mouse wheel itself. If set to true, the event is not forwarded to the control under the mouse, nor to parent controls.

        WantMouseWheelParameters(bool vertical, short delta, VirtualKeyStateSet vkeys, short screen_x, short screen_y, bool &handled) : vertical(vertical), delta(delta), vkeys(vkeys), screen_x(screen_x), screen_y(screen_y), handled(handled) { ; }
    };
    typedef Event<WantMouseWheelParameters> WantMouseWheelEvent;

    struct MouseWheelParameters
    {
        bool &vertical;
        short &delta;
        VirtualKeyStateSet vkeys;
        short screen_x;
        short screen_y;

        bool &handled; // Whether the control handles the mouse wheel itself. If set to true, the event is not forwarded to the control under the mouse, nor to parent controls.

        MouseWheelParameters(bool &vertical, short &delta, VirtualKeyStateSet vkeys, short screen_x, short screen_y, bool &handled) : vertical(vertical), delta(delta), vkeys(vkeys), screen_x(screen_x), screen_y(screen_y), handled(handled) { ; }
    };
    typedef Event<MouseWheelParameters> MouseWheelEvent;

    struct KeyParameters
    {
        WORD &keycode;
        const VirtualKeyStateSet &vkeys;

        KeyParameters(WORD &keycode, const VirtualKeyStateSet &vkeys) : keycode(keycode), vkeys(vkeys) { }
    };
    typedef Event<KeyParameters> KeyEvent;

    struct KeyPushParameters
    {
        WORD &keycode;
        WCHAR &key;
        const VirtualKeyStateSet &vkeys;

        KeyPushParameters(WORD &keycode, WCHAR &key, const VirtualKeyStateSet &vkeys) : keycode(keycode), key(key), vkeys(vkeys) { }
    };
    typedef Event<KeyPushParameters> KeyPushEvent;

    struct KeyPressParameters
    {
        WCHAR &key;
        const VirtualKeyStateSet &vkeys;

        KeyPressParameters(WCHAR &key, const VirtualKeyStateSet &vkeys) : key(key), vkeys(vkeys) { }
    };
    typedef Event<KeyPressParameters> KeyPressEvent;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum CloseActions : int;
#else
    enum CloseActions;
#endif
    struct FormCloseParameters
    {
        CloseActions &action;
        FormCloseParameters(CloseActions &action) : action(action) { }
    };
    typedef Event<FormCloseParameters> FormCloseEvent;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum CheckboxStates : int;
#else
    enum CheckboxStates;
#endif
    struct CheckboxCheckParameters
    {
        CheckboxStates &newstate;
        CheckboxCheckParameters(CheckboxStates &newstate) : newstate(newstate) { }
    };
    typedef Event<CheckboxCheckParameters> CheckboxCheckEvent;

    struct RadioboxCheckParameters
    {
        bool &checked;
        RadioboxCheckParameters(bool &checked) : checked(checked) { }
    };
    typedef Event<RadioboxCheckParameters> RadioboxCheckEvent;

    // Parameter for event called when control gains or loses the input focus.
    struct FocusChangedParameters
    {
        HWND otherwindow; // Another window that gained/lost focus.
        FocusChangedParameters(HWND otherwindow) : otherwindow(otherwindow) { }
    };
    typedef Event<FocusChangedParameters> FocusChangedEvent;

    // Parameter for event called when control is activated or deactivated on its form.
    class Control;
    struct ActiveChangedParameters
    {
        Control *other; // The other control that gained/lost focus.
        bool tabactivate; // The control was activated or deactivated because of a tab or shift+tab key event.
        ActiveChangedParameters(Control *other, bool tabactivate) : other(other), tabactivate(tabactivate) { }
    };
    typedef Event<ActiveChangedParameters> ActiveChangedEvent;

    struct MessageParameters
    {
        LRESULT &result;
        UINT &uMsg;
        WPARAM &wParam;
        LPARAM &lParam;

        bool &allowprocessing; // Allow the control to process this message.

        MessageParameters(LRESULT &result, UINT &uMsg, WPARAM &wParam, LPARAM &lParam, bool &allowprocessing) : result(result), uMsg(uMsg), wParam(wParam), lParam(lParam), allowprocessing(allowprocessing) { }

    };
    typedef Event<MessageParameters> MessageEvent;

    struct MeasureItemParameters
    {
        Canvas *canvas;
        int index;
        UINT &width;
        UINT &height;
        void *data;

        MeasureItemParameters(Canvas *canvas, int index, UINT &width, UINT &height, ULONG_PTR data) : canvas(canvas), index(index), width(width), height(height), data((void*)data) { }
    };
    typedef Event<MeasureItemParameters> MeasureItemEvent;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum DrawItemStates : int;
#else
    enum DrawItemStates;
#endif
    typedef uintset<DrawItemStates> DrawItemStateSet;

    struct DrawItemParameters
    {
        Canvas *canvas;
        int index;
        Rect rect;
        const DrawItemStateSet &state;
        void *data;

        DrawItemParameters(Canvas *canvas, int index, Rect rect, const DrawItemStateSet &state, ULONG_PTR data) : canvas(canvas), index(index), rect(rect), state(state), data((void*)data) {}
    };
    typedef Event<DrawItemParameters> DrawItemEvent;

    struct PopupMenuParameters
    {
        Control *popupcontrol;
        Point &screenpos;
        bool &allow;

        PopupMenuParameters(Control *popupcontrol, Point &screenpos, bool &allow) : popupcontrol(popupcontrol), screenpos(screenpos), allow(allow) { }
    };
    typedef Event<PopupMenuParameters> PopupMenuEvent;

    class Bitmap;
    struct DragImageRequestParameters
    {
        Point &dragpoint;
        Bitmap* &dragimage;
        bool &freebitmap; // Set to true if the bitmap should be freed by the library and not the caller.

        DragImageRequestParameters(Point &dragpoint, Bitmap* &dragimage, bool &freebitmap) : dragpoint(dragpoint), dragimage(dragimage), freebitmap(freebitmap) {}
    };
    typedef Event<DragImageRequestParameters> DragImageRequestEvent;

#ifdef __MINGW32__
    enum DragDropEffects : unsigned int;
#else
    enum DragDropEffects;
#endif
    typedef uintset<DragDropEffects> DragDropEffectSet;
    struct DragDropEndedParameters
    {
        const DragDropEffects &result;

        DragDropEndedParameters(const DragDropEffects &result) : result(result) {}
    };
    typedef Event<DragDropEndedParameters> DragDropEndedEvent;

    struct DragDropParameters
    {
        int formatindex; // The index of the format which was added to the control's drop target object.
        const VirtualKeyStateSet &keys; // The keys currently pressed.
        Point p; // The mouse coordinates within the control's client area.
        const DragDropEffectSet &effects; // The allowed effects that were specified when the drag drop operation started.
        DragDropEffects &effect; //The effect of the drag drop operation if the mouse is released in the current state. If the effects value doesn't contain this effect, it will be set to ddeNone.

        DragDropParameters(int formatindex, const VirtualKeyStateSet &keys, Point p, const DragDropEffectSet &effects, DragDropEffects &effect) : formatindex(formatindex), keys(keys), p(p), effects(effects), effect(effect) {}
    };
    typedef Event<DragDropParameters> DragDropEvent;

    struct DragDropDropParameters
    {
        IDataObject *dataobject;
        int formatindex; // The index of the format which was added to the control's drop target object.
        const VirtualKeyStateSet &keys; // The keys currently pressed.
        Point p; // The mouse coordinates within the control's client area.
        const DragDropEffectSet &effects; // The allowed effects that were specified when the drag drop operation started.
        DragDropEffects &effect; //The effect of the drag drop operation if the mouse is released in the current state. If the effects value doesn't contain this effect, it will be set to ddeNone.

        DragDropDropParameters(IDataObject *dataobject, int formatindex, const VirtualKeyStateSet &keys, Point p, const DragDropEffectSet &effects, DragDropEffects &effect) : dataobject(dataobject), formatindex(formatindex), keys(keys), p(p), effects(effects), effect(effect) {}
    };
    typedef Event<DragDropDropParameters> DragDropDropEvent;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum DialogCodes : int;
#else
    enum DialogCodes;
#endif
    typedef uintset<DialogCodes> DialogCodeSet;
    struct DialogCodeParameters
    {
        WORD key;
        DialogCodeSet &result;
        DialogCodeParameters(WORD key, DialogCodeSet &result) : key(key), result(result) {}
    };
    typedef Event<DialogCodeParameters> DialogCodeEvent;

    class Form;
    // Called when the active window changes in the system.
    struct ActiveFormChangeParameters
    {
        bool activated; // True when the receiver is activated, false when deactivated.
        bool mouseactivate;
        Form *otherform; // If activated is true, the form being deactivated. If activated is false, either the form being activated, if it is a form in the same application, or NULL.
        HWND otherwindow; // The handle of the new window being activated or deactivated.
        ActiveFormChangeParameters(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow) : activated(activated), mouseactivate(mouseactivate), otherform(otherform), otherwindow(otherwindow) {}
    };
    typedef Event<ActiveFormChangeParameters> ActiveFormChangeEvent;

    // Called when the receiving form loses its active state or becomes active in the application.
    struct FormActivateParameters
    {
        Form *otherform; // The form being activated or deactivated in place of this one.
        FormActivateParameters(Form *otherform) : otherform(otherform) {}
    };
    typedef Event<FormActivateParameters> FormActivateEvent;

    // Called when the currently selected tab of a tab control is about to be changed. Set allowchange to false to prevent change of the currently selected tab.
    struct TabChangingParameters
    {
        int currenttab; // The tab currently selected, which is about to be deselected.
        bool &allowchange;

        TabChangingParameters(int currenttab, bool &allowchange) : currenttab(currenttab), allowchange(allowchange) {}
    };
    typedef Event<TabChangingParameters> TabChangingEvent;

    struct TabChangeParameters
    {
        int tabindex; // Index of the tab which is selected after the selection changed.

        TabChangeParameters(int tabindex) : tabindex(tabindex) {}
    };
    typedef Event<TabChangeParameters> TabChangeEvent;

    struct BeginListviewItemEditParameters
    {
        int itemindex;
        bool &allowedit;

        BeginListviewItemEditParameters(int itemindex, bool &allowedit) : itemindex(itemindex), allowedit(allowedit) {}
    };
    typedef Event<BeginListviewItemEditParameters> BeginListviewItemEditEvent;

    struct EndListviewItemEditParameters
    {
        int itemindex;
        std::wstring editortext;
        bool &canceled;

        EndListviewItemEditParameters(int itemindex, const std::wstring &editortext, bool &canceled) : itemindex(itemindex), editortext(editortext), canceled(canceled) {}
    };
    typedef Event<EndListviewItemEditParameters> EndListviewItemEditEvent;

    struct CancelListviewItemEditParameters
    {
        int itemindex;

        CancelListviewItemEditParameters(int itemindex) : itemindex(itemindex) {}
    };
    typedef Event<CancelListviewItemEditParameters> CancelListviewItemEditEvent;

    class ListviewGroup;
    struct ListviewGroupTaskParameters
    {
        ListviewGroup *group;
        ListviewGroupTaskParameters(ListviewGroup *group) : group(group) {}
    };
    typedef Event<ListviewGroupTaskParameters> ListviewGroupTaskEvent;

    struct BeginCellEditParameters
    {
        int col; // Column of the cell to be edited.
        int row; // Row of the cell to be edited.
        bool &allowedit; // Set to false to prevent the editor to be shown in the cell.
        BeginCellEditParameters(int col, int row, bool &allowedit) : col(col), row(row), allowedit(allowedit) {}
    };
    typedef Event<BeginCellEditParameters>  BeginCellEditEvent;

    struct CellEditedParameters
    {
        int col; // Column of the edited cell.
        int row; // Row of the edited cell.
        bool &allowupdate; // Set to false to prevent a cell's text to be updated. In this case changes to text are ignored.
        std::wstring &text; // The text currently in the editor. When 'allowupdate' is true, this can be updated to a different value to be used in the control instead of what was typed by the user.
        CellEditedParameters(int col, int row, bool &allowupdate, std::wstring &text) : col(col), row(row), allowupdate(allowupdate), text(text) {}
    };
    typedef Event<CellEditedParameters> CellEditedEvent;

    struct EndCellEditParameters
    {
        int col; // Column of the cell to be edited.
        int row; // Row of the cell to be edited.
        bool cancelled; // Editing was cancelled either by pressing the ESC key or by changing allowedit in CellEditedEvent to false.
        EndCellEditParameters(int col, int row, bool cancelled) : col(col), row(row), cancelled(cancelled) {}
    };
    typedef Event<EndCellEditParameters>  EndCellEditEvent;

    struct AllowColumnRowParameters
    {
        int index; // Index of the column or row to be sized. moved etc.
        bool &allow; // Can be set to false if the column or row shouldn't be modified.
        AllowColumnRowParameters(int index, bool &allow) : index(index), allow(allow) {}
    };
    typedef Event<AllowColumnRowParameters> AllowColumnRowEvent;

    struct ColumnRowSizeParameters
    {
        int index; // Index of the column or row being sized.
        int &size; // Can be set to a different value to change the new size of the column or row.
        ColumnRowSizeParameters(int index, int &size) : index(index), size(size) {}
    };
    typedef Event<ColumnRowSizeParameters> ColumnRowSizeEvent;

    struct GridCellAlignmentParameters
    {
        TextAlignments &horzalign;
        VerticalTextAlignments &vertalign;
        GridCellAlignmentParameters(TextAlignments &horzalign, VerticalTextAlignments &vertalign) : horzalign(horzalign), vertalign(vertalign) {}
    };
    typedef Event<GridCellAlignmentParameters> GridCellAlignmentEvent;

    struct AppSelectActiveFormParameters
    {
        Form *oldactive; // Form that was active before activating another. Only pass messages to the form's handle, do not call its functions directly, because it might be in the middle of destruction.
        Form* &newactive; // Form to be automatically activated. Change this value to a different form to activate it instead. Setting it to the value of oldactive equals to not changing it.
        AppSelectActiveFormParameters(Form *oldactive, Form* &newactive) : oldactive(oldactive), newactive(newactive) {}
    };
    typedef Event<AppSelectActiveFormParameters> AppSelectActiveFormEvent;

    struct ComputeAlignBoundsParameters
    {
        Rect &bounds;

        ComputeAlignBoundsParameters(Rect &bounds) : bounds(bounds) { }
    };
    typedef Event<ComputeAlignBoundsParameters> ComputeAlignBoundsEvent;

    struct ButtonMeasureSplitSizeParameters
    {
        int &splitwidth; // Width of the secondary button on the right side for split buttons.
        ButtonMeasureSplitSizeParameters(int &splitwidth) : splitwidth(splitwidth) {}
    };
    typedef Event<ButtonMeasureSplitSizeParameters> ButtonMeasureSplitSizeEvent;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum ButtonOwnerDrawOperations : int;
    enum ButtonOwnerDrawState : int;
#else
    enum ButtonOwnerDrawOperations;
    enum ButtonOwnerDrawState;
#endif
    struct ButtonOwnerDrawParameters
    {
        Canvas *canvas;
        ButtonOwnerDrawState state;
        ButtonOwnerDrawOperations operation;

        Rect area; // Area of the whole button. Use this if the content measurement made with the default contents are not useful for owner drawn contents.
        int splitwidth; // Width of the secondary button part for split buttons. This is 0 when the button is not a split button. The secondary button's area can be computed from the area and splitwidth: Rect(area.right - splitwidth, area.top, area.bottom, area.right).
        Point image; // Position of the image in the button's client area, if the button is drawn with the default contents and placement values. This is also the position of the image of the secondary button for split buttons when drawing the secondary button. Only set when the drawing operation is for contents drawing.
        Point text; // Position of the text in the button's client area, if the button is drawn with the default contents and placement values. Only set when the drawing operation is for contents drawing.
        bool &handled; // In and out parameter. Set this to true if the button should skip the current drawing operation.
        ButtonOwnerDrawParameters(Canvas *canvas, ButtonOwnerDrawState state, ButtonOwnerDrawOperations operation,
                                  Rect area, int splitwidth, Point image, Point text, bool &handled) : 
                canvas(canvas), state(state), operation(operation), area(area), splitwidth(splitwidth), image(image), text(text), handled(handled) {}
    };
    typedef Event<ButtonOwnerDrawParameters> ButtonOwnerDrawEvent;
}
/* End of NLIBNS */


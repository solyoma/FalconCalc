#pragma once

#include "generalcontrol.h"
#include "sparse_list.h"
#ifdef DESIGNING
#include "designerform.h"
#endif


namespace NLIBNS
{


    enum DrawItemStates : int {
            disChecked = ODS_CHECKED, disComboboxEdit = ODS_COMBOBOXEDIT, disDefault = ODS_DEFAULT, disDisabled = ODS_DISABLED, disFocus = ODS_FOCUS,
            disGrayed = ODS_GRAYED, disHotlight = ODS_HOTLIGHT, disInactive = ODS_INACTIVE, disNoAccel = ODS_NOACCEL, disNoFocusRect = ODS_NOFOCUSRECT,
            disSelected = ODS_SELECTED
    };


    class SystemControl : public Control // base class for controls that use the built-in windows system classes (buttons or edit boxes etc.)
    {
    private:
        typedef Control base;

        WNDPROC builtinwndproc; // The wndproc that was originally set for the class of this control.

        struct BrushData
        {
            HBRUSH brush;
            int usage;

            BrushData();
            BrushData(HBRUSH brush, int usage);
        };
        static std::map<COLORREF, BrushData> bgbrushes; // Map of brushes used by controls for erasing their background. Only one brush is allocated per color, to keep brush handle usage to minimum.
        static void UseBrush(COLORREF color);
        static void UnuseBrush(COLORREF color);
        static HBRUSH GetBrush(COLORREF color);
        static void DeleteBrushes();
        Color brushcolor; // Color used when registering the brush to be used for erasing the background.

        friend class Application;
    protected:
        virtual LRESULT CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();

        virtual bool HandleSysKey(WCHAR key); // Checks the text of the control for an & + char combination. Returns true if it does and sends a WM_COMMAND to its parent to activate the control.
#ifdef DESIGNING
        SystemControl(const SystemControl &orig) : base(orig) {}
#endif
        SystemControl();

        HBRUSH CtlBrush(); // Brush to return by HandleCtlColor if a brush is needed for the control

        virtual ~SystemControl();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        virtual bool HandleCommand(Control *parent, WPARAM wParam) { return false; }
        virtual bool HandleNotify(Control *parent, LPARAM lParam, HRESULT &result) { return false; }
        virtual bool HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush);
        virtual bool HandleMeasureItem(Control *parent, MEASUREITEMSTRUCT *measures) { return false; }
        virtual bool HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures) { return false; }
    };

    enum CheckboxStates : int { csUnchecked, csChecked, csIndeterminate,
#ifdef DESIGNING
                cbscCount = 3
#endif
    };
    class Checkbox : public SystemControl
    {
    private:
        typedef SystemControl base;

        bool textleft;
        CheckboxStates state;
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual void SaveWindow();

        virtual bool HandleCommand(Control *parent, WPARAM wParam);
        //virtual void EraseBackground();
        virtual bool HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush);

        virtual ~Checkbox();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif 
        Checkbox();


        void Restyle(); // Updates the button's styles.
        void UpdateState(); // Sets current checkbox state by sending BM_SETCHECK.

        bool TextLeft();
        void SetTextLeft(bool newtextleft);
        bool Checked();
        void SetChecked(bool newchecked);
        CheckboxStates State();
        void SetState(CheckboxStates newstate);

        void Click();

        NotifyEvent OnClick; // Called when the checked state changed.
        CheckboxCheckEvent OnCheck; // Caled before the checked state of the box changes, so it can be updated before the change occurs.
    };

    class Radiobox : public SystemControl
    {
    private:
        typedef SystemControl base;

        bool textleft;
        bool checked;
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual void SaveWindow();

        virtual bool HandleCommand(Control *parent, WPARAM wParam);
        //virtual void EraseBackground();
        virtual bool HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush);

        virtual ~Radiobox();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif 
        Radiobox();

        void Restyle(); // Updates the button's styles.
        void UpdateState(); // Sets current Radiobox state by sending BM_SETCHECK.

        bool TextLeft();
        void SetTextLeft(bool newtextleft);
        bool Checked();
        void SetChecked(bool newchecked);

        void Click();

        NotifyEvent OnClick;
        RadioboxCheckEvent OnCheck;
    };

    class Groupbox : public SystemControl
    {
    private:
        typedef SystemControl base;
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

    //    virtual void EraseBackground();

        virtual ~Groupbox();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        Groupbox();

        void Restyle(); // Updates the button's styles.
    };

    class Edit : public SystemControl
    {
    private:
        typedef SystemControl base;

        unsigned int maxlength;
        bool readonly;

        bool autoselect; // When the user clicks in the edit box without moving the mouse around, the full text will be selected.
        bool doautosel; // Set to true on mousedown and false if the mouse moved more than a given amount.
        Point mouseselpt; // The point in the client rectangle to measure the mouse movement from. If the distance is greater, autoselect won't activate.

        int leftmargin;
        int rightmargin;
        int selstart;
        int sellength;
        bool modified;
    
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        virtual void InitHandle();
        virtual void SaveWindow();

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual bool HandleCommand(Control *parent, WPARAM wParam);

        virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

        void _SelStartAndLength(); // Updates selstart and sellength with real values if handle is allocated.
        void _Margins(); // Updates left and right margin variables.

        virtual ~Edit();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif 
        Edit();

        unsigned int MaxLength() const;
        void SetMaxLength(unsigned int newmax);
        bool ReadOnly();
        void SetReadOnly(bool newreadonly);

        int SelStart();
        int SelLength();
        void SelStartAndLength(int &start, int &length);
        void SetSelStartAndLength(int newstart, int newlength);
        void SetSelStart(int newstart);
        void SetSelLength(int newlength);
        std::wstring SelText();
        void SetSelText(const std::wstring& newseltext);

        bool AutoSelect();
        void SetAutoSelect(bool newautoselect);

        int LeftMargin();
        void SetLeftMargin(int newleft);
        int RightMargin();
        void SetRightMargin(int newright);
        void GetMargins(int &left, int &right);
        void SetMargins(int newleft, int newright);

        void AddText(const std::wstring& str);

        bool Modified();
        void SetModified(bool newmodified);

        // Events
        NotifyEvent OnTextChanged;
    };

    /*
     * Edit control with the purpose to be placed in another control. The only extra functionality is the ability to
     * clip the editor by placing it on a panel like control, and to set a top margin, which is impossible with the
     * default control. As the top margin is solved by extracting from the y coordinate on mouse move and adding to
     * the top when painting, this control receives invalid coordinates, thus it is not appropriate for use anywhere
     * else.
     */ 
    class ControlEdit : public Control
    {
    private:
        typedef Control    base;
        int topmargin;
        DWORD lasttick;
        Point lastclickpt;

        Edit *edit;
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual ~ControlEdit();
    public:
        ControlEdit();

        HWND EditHandle(); // Handle of the real editor.

        int TopMargin();
        void SetTopMargin(int newtopmargin);
        int InnerLeftMargin();
        void SetInnerLeftMargin(int newleft); // Moves the editor within the ControlEdit's panel and changes the panel's width to fit the editor's current right side.
        Edit* Editor();

        void Focus();

        std::wstring Text() const;
        void SetText(const std::wstring &newtext);

        unsigned int MaxLength();
        void SetMaxLength(unsigned int newmax);
        bool ReadOnly();
        void SetReadOnly(bool newreadonly);

        int SelStart();
        int SelLength();
        void SelStartAndLength(int &start, int &length);
        void SetSelStartAndLength(int newstart, int newlength);
        void SetSelStart(int newstart);
        void SetSelLength(int newlength);
        std::wstring SelText();
        void SetSelText(const std::wstring& newseltext);

        bool AutoSelect();
        void SetAutoSelect(bool newautoselect);

        int LeftMargin();
        int RightMargin();
        void SetLeftMargin(int newleft);
        void SetRightMargin(int newright);
        void GetMargins(int &left, int &right);
        void SetMargins(int newleft, int newright);

        void AddText(const std::wstring& str);

        bool Modified();
        void SetModified(bool newmodified);

        NotifyEvent &OnTextChanged;
        NotifyEvent &OnCaptureLost;
        NotifyEvent &OnMouseEnter;
        NotifyEvent &OnMouseLeave;
        NotifyEvent &OnNCMouseEnter;
        NotifyEvent &OnNCMouseLeave;
        NotifyEvent &OnMouseEntered;
        NotifyEvent &OnMouseLeft;
        FocusChangedEvent &OnGainFocus;
        FocusChangedEvent &OnLoseFocus;
        ActiveChangedEvent &OnEnter;
        ActiveChangedEvent &OnLeave;
        KeyEvent &OnKeyDown;
        KeyEvent &OnKeyUp;
        KeyPressEvent &OnKeyPress;
        KeyPushEvent &OnKeyPush;
        MouseMoveEvent &OnMouseMove;
        MouseButtonEvent &OnMouseDown;
        MouseButtonEvent &OnMouseUp;
        MouseButtonEvent &OnDblClick;
        NCMouseMoveEvent &OnNCMouseMove;
        NCMouseButtonEvent &OnNCMouseDown;
        NCMouseButtonEvent &OnNCMouseUp;
        NCMouseButtonEvent &OnNCDblClick;
        NotifyEvent &OnStartSizeMove; 
        SizePositionChangedEvent &OnSizeChanged;
        SizePositionChangedEvent &OnPositionChanged;
        SizePositionChangedEvent &OnEndSizeMove;
        NotifyEvent &OnResize;
        NotifyEvent &OnMove;
        DialogCodeEvent &OnDialogCode;
    };

    class UpDown : public SystemControl
    {
    private:
        typedef SystemControl base;

        Edit *buddy;

        bool leftalign;
        bool horizontal;
        bool arrowkeys;
        bool thousands;
        bool wrap;
        bool hex;

        int minval;
        int maxval;
        int pos;

        int step;
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void DeleteNotify(Object *object); // Called by the buddy object when it is deleted.
        virtual void ChangeNotify(Object *object, int changetype); // Called by the buddy object when it is resized, so the up down must follow it.

        virtual bool HandleNotify(Control *parent, LPARAM lParam, HRESULT &result);

        virtual ~UpDown();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif 
        UpDown();

        bool AlignLeft();
        void SetAlignLeft(bool newalignleft);
        bool Horizontal();
        void SetHorizontal(bool newhorizontal);
        bool HandleArrowKeys();
        void SetHandleArrowKeys(bool newarrowkeys);
        bool ThousandSeparator();
        void SetThousandSeparator(bool newthousands);
        bool WrapValue();
        void SetWrapValue(bool newwrap);
        bool DisplayHexadecimal();
        void SetDisplayHexadecimal(bool newhex);
        Edit* AttachedEditor();
        void SetAttachedEditor(Edit *neweditor);

        void Range(int &minvalue, int &maxvalue);
        void SetRange(int minvalue, int maxvalue);
        int MinValue();
        void SetMinValue(int newmin);
        int MaxValue();
        void SetMaxValue(int newmax);
        int Position();
        void SetPosition(int newpos);
        int Step();
        void SetStep(int newstep);
    };

    class Memo : public Edit
    {
    private:
        typedef Edit base;

        bool wordwrap;
        bool vscroll;
        bool hscroll;

        using base::AutoSelect;
        using base::SetAutoSelect;

    protected:
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        void NeedsDialogCode(WORD key, DialogCodeSet &dialogcode);

        virtual ~Memo();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif 
        Memo();

        bool WordWrap() const;
        void SetWordWrap(bool wrap);
        bool VerticalScrollbar() const;
        //void SetVerticalScrollbar(bool newvscroll);
        bool HorizontalScrollbar() const;
        //void SetHorizontalScrollbar(bool newhscroll);

        //void AddText(std::wstring str); // Appends text to the end of the memo's buffer.
        void AddLine(const std::wstring& str, bool triggerevent = false); // Appends text and a newline at the end of the memo's buffer.

        void SetLines(const std::vector<std::wstring> &newlines, bool triggerevent = false);
        void GetLines(std::vector<std::wstring> &out);

        int LineCount(); // Returns the number of lines of text in the memo, but at least 1.
        int LineLength(int lineindex); // Length of a line not including the newline characters at the end.
        int LineLengthFromCharacter(int charindex); // Length of a line which contains the character at index, not including the newline characters at the end.
        int LineFromCharacter(int charindex); // Index of the line containing the character at the index.
        int LineStartIndex(int lineindex); // Index of the first character of the given line.
        std::wstring GetLine(int lineindex); // Returns the line without the newline characters with the given line index.
    };

    enum ProgressbarState { pbstNormal = 0x0001, pbstError = 0x0002, pbstPaused = 0x0003 };

    class Progressbar : public SystemControl
    {
    private:
        typedef SystemControl base;

        bool smooth; // only has effect when no themes used
        bool vertical;
        bool smoothreverse; // decrementing value gives smooth effect. only valid from Vista
        bool marqueemode;
        int marqueeupdatetime;

        int minrange;
        int maxrange;
        int pos;
        int step;

        Color bkcolor;
        Color barcolor;

        ProgressbarState state;

        void _MinMaxRangeMarqueePosStepStateColor(); // updates inner variables with real window values if handle is created

        void _MinMaxRange();
        void _MinMaxRangePos();
        void _Marquee();
        void _Pos();
        void _Step();
        void _State();
        void _Color();
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        virtual void InitHandle();
        virtual void SaveWindow();

        virtual ~Progressbar();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif 
        Progressbar();

        bool Marquee();
        void SetMarquee(bool turnon);
        int MarqueeUpdateTime();
        void SetMarqueeUpdateTime(int newupdatetime = 0);
        void SetMarqueeAndUpdateTime(bool turnon, int newupdatetime = 0);
        bool Vertical();
        void SetVertical(bool newvertical);
        bool Smooth();
        void SetSmooth(bool newsmooth);
        bool SmoothReverse();
        void SetSmoothReverse(bool newsmoothreverse);

        int MinRange();
        int MaxRange();
        void MinMaxRange(int &minr, int &maxr);
        void SetMinRange(int newmin);
        void SetMaxRange(int newmax);
        void SetMinMaxRange(int newmin, int newmax);
        int Position();
        void SetPosition(int newpos);
        int Step(); // unit to change value on StepIt calls
        void SetStep(int newstep);
        int StepIt(); // returns the previous position
        ProgressbarState State();
        void SetState(ProgressbarState newstate);
        Color BarColor();
        void SetBarColor(Color color);
        Color BKColor();
        void SetBKColor(Color color);
    };

    class ListControl;

    enum ControlElemMessages { cemInitStorage, cemGetCount, cemGetTextLen, cemGetText, cemAddString, cemInsertString,
                               cemDeleteString, cemGetItemData, cemSetItemData, cemResetContent, cemGetItemIndex, cemSetItemIndex,
                               // Error
                               cemError, cemNotEnoughSpace,
                               // No more messages after this
                               cemEnumEnd };

    class ControlElemList : public Object
    {
    private:
        typedef Object  base;

        ListControl *owner;

        int listmessages[cemEnumEnd];

        std::vector<std::pair<std::wstring, void*>> list;

        void SaveItems(); // Gets the items in the listbox and saves them in the strings vector. Usually called when destroying the listbox before recreating it.
        void InsertItems(); // Called when the listbox is recreated to restore the saved items to the listbox.
        bool nodata;
        int nodatacount;

        void SetNodata(bool newnodata);

        int ItemIndex() const;
        void SetItemIndex(int newindex);

        friend class ListControl;

#ifndef DESIGNING
        ControlElemList(const ControlElemList &copy) {}
#endif
    protected:
        ControlElemList(int messagelist[cemEnumEnd]);

        void SetOwner(ListControl *newowner);
        virtual void Destroy() { base::Destroy(); }
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        bool NoData();

        ControlElemList(const ControlElemList &copy) : base() { throw L"No default ControlElemList!"; }
        ControlElemList() : base() { /*throw L"No default ControlElemList!"*/ ; }
        bool operator==(const ControlElemList &other);

        void DesignAdd(const std::wstring& str) { Add(str); }
#endif
        virtual ~ControlElemList();

        void Add(const std::wstring& str);
        void Add(const std::wstring& str, void *data);
        void Insert(int position, const std::wstring& str);
        void Insert(int position, const std::wstring& str, void *data);
        void Swap(int a, int b);
        void Delete(int position);
        void Clear();
        int Count() const;
        void SetCount(int newcount);
        std::wstring Text(int position) const;
        void SetText(int position, const std::wstring& newtext);
        void* Data(int position) const;
        void SetData(int position, void *data);

        void AllocateCapacity(int numitems, int charcnt); // Call when adding a large number of items. Set 'numitems' to the number of items to add, and 'charcnt' to the sum of all characters, including the terminating null. Only works for controls with a created handle.

        void SetLines(const std::vector<std::wstring> &newlines);
        void GetLines(std::vector<std::wstring> &out) const;
    };

    enum ListControlNotifications { 
            lcnChanged,
            // No more notifications after this
            lcnEnumEnd
    };

    enum ListControlKinds {
            lckNormal, lckOwnerDraw, lckOwnerDrawVariable, lckVirtual, lckVirtualOwnerDraw,
#ifdef DESIGNING
            lckCount = 5
#endif
    };

    class ListControl : public SystemControl
    {
    private:
        typedef SystemControl base;

        ControlElemList items;
        int itemindex;

        int listnotify[lcnEnumEnd];
    protected:
        ListControl(int messagelist[cemEnumEnd], int notifylist[lcnEnumEnd]);
        ListControl(const ListControl &copy) : base(copy), items(copy.items) { }

        virtual bool HandleCommand(Control *parent, WPARAM wParam);

        virtual void SelChanged();

        virtual void InitHandle();
        virtual void SaveWindow();

        void SetNodata(bool newnodata);

        virtual ~ListControl();

        enum ListControlNotifyCodes { lcncClear, lcncDelete, lcncInsert, lcncSwap, lcncSetCount, lcncAllocate, lcncSelect };
        virtual void NotifyList(ListControlNotifyCodes code, int a, int b) { ; } // Called by the items ControlElemList to notify its owner of changes in the items.

        friend class ControlElemList;
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual void DesignSetItemIndex(int newindex); // Calls SetItemIndex with the default false for triggerevent.
#endif
        ControlElemList& Items();
        virtual int ItemIndex(); // Index of the currently selected item.
        virtual void SetItemIndex(int newindex, bool triggerevent = false); // Change the current selection.
        int Count();
        std::wstring ItemText(int ix);
        void* ItemData(int position);
        void SetItemText(int ix, const std::wstring &newtext);
        void SetItemData(int ix, void *data);
        void SwapItems(int a, int b);
        void DeleteItem(int ix);
        void AddItem(const std::wstring& str);
        void AddItem(const std::wstring& str, void *data);
        void InsertItem(int position, const std::wstring& str);
        void InsertItem(int position, const std::wstring& str, void *data);
        void Clear();

        void SetLines(const std::vector<std::wstring> &newlines);
        void GetLines(std::vector<std::wstring> &out) const;

        void AllocateCapacity(int numitems, int charcnt); // Call when adding a large number of items. Set 'numitems' to the number of items to add, and 'charcnt' to the sum of all characters, including the terminating null. Only works for controls with a created handle.

        NotifyEvent OnChanged; // The current selection has changed, for single selection list boxes.
    };

    enum ListboxSelectionTypes {
            lstSingleSelect, lstExtendedSelect, lstMultiSelect,
#ifdef DESIGNING
            lstCount = 3
#endif
    };

    class Listbox : public ListControl
    {
    private:
        typedef ListControl  base;

        ListControlKinds kind;
        ListboxSelectionTypes seltype;

        bool integralheight;

        //int sel;

        int itemheight;
        sparse_list<bool> sellist; // Used to save and retrieve selection when the control has no handle.

        static int lbmessages[cemEnumEnd];
        static int lbnotify[lcnEnumEnd];
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        virtual void InitHandle();
        virtual void SaveWindow();

        virtual bool HandleMeasureItem(Control *parent, MEASUREITEMSTRUCT *measures);
        virtual bool HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures);

        virtual void NotifyList(ListControlNotifyCodes code, int a, int b);

        virtual ~Listbox();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif
        Listbox();

        virtual int ItemIndex(); // Index of the currently selected item.
        virtual void SetItemIndex(int newindex, bool triggerevent = false); // Change the current selection.

        ListControlKinds Kind();
        void SetKind(ListControlKinds newkind);
        ListboxSelectionTypes SelectionType();
        void SetSelectionType(ListboxSelectionTypes newseltype);
        int ItemHeight();
        void SetItemHeight(int newheight);

        bool IntegralHeight();
        void SetIntegralHeight(bool newintegralheight);
        //int Selected();
        //void SetSelected(int index);
        int TopRow();
        int SelCount();
        bool Selected(int index);
        void SetSelected(int index, bool sel);
        void SelectAll();
        void DeselectAll();

        void ScrollIntoView(); // Scrolls the listbox until the selected item is fully visible.

        Rect RowRect(int index);
        int ItemAt(short x, short y);
        int ItemAt(Point pt);

        // Events
        MeasureItemEvent OnMeasureItem;
        DrawItemEvent OnDrawItem;
    };

    enum ComboboxTypes {
                ctDropdown, ctDropdownList,
#ifdef DESIGNING
                ctCount = 2
#endif
    };

    class Combobox : public ListControl
    {
    private:
        typedef ListControl base;

        ListControlKinds kind;
        ComboboxTypes type;
        int itemheight;

        bool autoselect; // When the user clicks in the edit box without moving the mouse around, the full text will be selected.
        bool doautosel; // Set to true on mousedown and false if the mouse moved more than a given amount.
        Point mouseselpt; // The point in the client rectangle to measure the mouse movement from. If the distance is greater, autoselect won't activate.

        int selstart;
        int sellength;

        HWND editwnd; // Handle to the editor window placed inside the combo box.
        PWndProc editproc; // Original window procedure for editwnd, to be called after messages were checked.
        HWND listwnd; // Handle to the dropdown list.

        void _SelStartAndLength(); // Updates selstart and sellength with real values if handle is allocated.

        static int cbmessages[cemEnumEnd];
        static int cbnotify[lcnEnumEnd];
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void HandleChildMessage(HWND hwnd, UINT &uMsg, WPARAM &wParam, LPARAM &lParam, LRESULT &result);
        virtual bool HandleChildCommand(HWND hwnd, WPARAM wParam);

        virtual void GainFocus(HWND otherwindow);
        virtual void LoseFocus(HWND otherwindow);

        virtual void InitHandle();
        virtual void SaveWindow();

        virtual ~Combobox();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif
        Combobox();

        virtual bool Focused(); // Returns true when the control has the input focus.
        virtual void Focus(); // Focuses the control in the system.
        virtual void TabFocus(); // Same as Focus() but the called events will contain a true value which specifies that the control was focused by pressing the tab key and not by some other method.

        ComboboxTypes Type();
        void SetType(ComboboxTypes newtype);
        ListControlKinds Kind();
        void SetKind(ListControlKinds newkind);
        int ItemHeight();
        void SetItemHeight(int newheight);

        // Selection functions only work when type is ctDropdown.
        int SelStart();
        int SelLength();
        void SelStartAndLength(int &start, int &length);
        void SetSelStartAndLength(int newstart, int newlength);
        void SetSelStart(int newstart);
        void SetSelLength(int newlength);
        std::wstring SelText();
        void SetSelText(const std::wstring& newseltext);

        bool AutoSelect();
        void SetAutoSelect(bool newautoselect);

        bool DroppedDown();
        void SetDroppedDown(bool newdropped);

        NotifyEvent OnTextChanged;
    };


    // TabControl helper classes

    class TabControl;
    class Tab : public Object
    {
    private:
        typedef Object base;

        TabControl *owner;

        std::wstring text;
        int imageindex;

        using base::ParentForm;
    protected:
        Tab(const Tab &other);
        virtual ~Tab();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        std::wstring DefaultText();
        virtual void SetName(const std::wstring& newname);
        virtual Object* MainControl();

        Imagelist* Images();

        virtual Object* SubOwner();
#endif
        Tab();

        virtual void Destroy();
        virtual Form* ParentForm() const;

        TabControl *OwnerControl();
        virtual void SetOwnerControl(TabControl *newowner);
        void SetOwnerAndIndex(TabControl *newowner, int index);
        int Index();
        void SetIndex(int newindex);

        const std::wstring& Text();
        void SetText(const std::wstring &newtext);
        int ImageIndex();
        void SetImageIndex(int newimageindex);
    };

    // Property for listing vector elements
    class TabControl;
    template<typename PropertyHolder> class TabDesignProperty;
    class TabPages : public Object
    {
    private:
        typedef Object  base;

        std::vector<Tab*> tabs;

        TabControl *owner;
        virtual ~TabPages();

        friend TabControl;
    public:
        TabPages(TabControl *owner);
        virtual void Destroy();

        typedef Tab  value_type; // To be checked from the vector.

        int Size();
        Tab* Items(int index);
        Tab* Add();

        int TabIndex(Tab *tab);
        void Insert(int index, Tab *tab);
        void Remove(int index);
    };

    enum ExtendedTabControlStyles { tcsExFlatSeparators = 0x00000001L, tcsExRegisterDrop = 0x00000002L }; // Manipulated with TCM_GETEXTENDEDSTYLE and TCM_SETEXTENDEDSTYLE messages.
    class TabControl : public SystemControl
    {
    private:
        typedef SystemControl  base;
        HWND scrollhwnd;
        TabPages *tabs;
        Imagelist *images;
        int tabindex;

        void UpdateTab(Tab *tab, UINT mask);
#ifdef DESIGNING
        void addtabclick(void *sender, EventParameters param);
        void deletetabclick(void *sender, EventParameters param);
        void nexttabclick(void *sender, EventParameters param);
        void prevtabclick(void *sender, EventParameters param);
        TabPages* GetTabPages();

        bool TabDesigned(Tab *tab); // The passed tab is the one currently selected on the designer form.
        void DesignSelectTab(DesignForm *form, int tabix);

        //void DoSelectTab(void *sender, DesignFormBase::SubObjectSelectParameters param);
#endif
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void InitHandle();

        virtual bool Paint();

        //virtual Rect OpaqueRect();
        virtual bool ExcludeOpaqueRegion(HRGN rgn, const Rect &rgnrect, const Point &origin);
        virtual void EraseBackground();

        virtual bool HandleNotify(Control *parent, LPARAM lParam, HRESULT &result); // Handles WM_NOTIFY messages sent to the parent control.

        virtual void DeleteNotify(Object *object); // Called by the imagelist when it is deleted.
        virtual void ChangeNotify(Object *object, int changetype); // Called by the imagelist or by the tabs that change.

        virtual void MeasureControlArea(Rect &clientrect);

        virtual void Resizing();

#ifdef DESIGNING
        virtual bool DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual bool DesignKeyPush(DesignForm *form, WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);
        virtual bool DesignTabNext(bool entering, bool backwards);

        virtual void DesignSubSelected(Object *subobj);
#endif

        virtual ~TabControl();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();

        virtual void InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems);
        virtual bool NeedDesignerHittest(int x, int y, LRESULT hittest);

        virtual void SetName(const std::wstring& newname);

        //virtual Object* NameOwner(const std::wstring &name);
        //virtual void Names(std::vector<std::wstring> &namelist);
        virtual void DesignAddTab();
#endif
        TabControl();
        virtual void Destroy();

        static TCITEM FillTabItemInfo(Tab *tab, UINT mask);

        virtual Tab* AddTab();
        Tab* AddTab(const std::wstring& tabtext);
        int AddTab(Tab *tab);
        int InsertTab(Tab *tab, int index);
        Tab* RemoveTab(int index);
        void RemoveTab(Tab *tab);
        void DeleteTab(int index);
        int TabCount() const;
        int TabIndex(Tab *tab);
        Tab* Tabs(int index);

        int TabAt(int x, int y);
        int TabAt(Point xy);
        Rect TabRect(int index);

        void InvalidateTabs();

        Imagelist* Images();
        void SetImages(Imagelist *newimages);

        int SelectedTab();
        void SetSelectedTab(int newseltab);

        TabChangingEvent OnTabChanging; // Called when the currently selected tab is about to be changed.
        TabChangeEvent OnTabChange; // Called after the currently selected tab has changed.
    };

    class TabPageOwner;
    class PageControl;
    // Panel like control in a PageControl which holds a single page under the tab.
    class TabPage : public Control
    {
    private:
        typedef Control   base;
        TabPageOwner *owner;
    protected:
#ifdef DESIGNING
        virtual void FinishDeserialize();
        virtual Object* MainControl();
        virtual Object* PropertyOwner();
#endif
        virtual void InitHandle();

        virtual void EraseBackground();

        TabPage(const TabPage &other) : base(other) {}
        virtual ~TabPage();

        friend class TabPageOwner;
    public:
        TabPage(TabPageOwner *owner);
        virtual void Destroy();

        TabPageOwner *Owner();
        virtual void Show();
        virtual void Hide();

        PageControl *OwnerControl();
        void SetOwnerControl(PageControl *newowner);
        void SetOwnerAndIndex(PageControl *newowner, int index);
        int Index();
        void SetIndex(int newindex);

        std::wstring Text() const;
        void SetText(const std::wstring &newtext);
        int ImageIndex() const;
        void SetImageIndex(int newimageindex);
    };

    // Derived Tab representing a tab with a page in the PageControl. It holds a TabPage control similar to a panel, to be able to contain controls on the page.
    class TabPageOwner : public Tab
    {
    private:
        typedef Tab   base;

        TabPage *panel;
    protected:
#ifdef DESIGNING
        virtual void InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems);

        virtual void SetName(const std::wstring& newname);
        friend Object* DesignCreateTabPage(Object*);

        int DesignIndex(); // Only for the property which calls the corresponding function in the TabPage.
        void DesignSetIndex(int newindex); // Only for the property which calls the corresponding function in the TabPage.

        virtual bool DesignSelectChanged(Object *control, bool selected);
#endif

        //virtual void DeleteNotify(Object *object); // Handle deletion of tab page panels.

        TabPageOwner(const TabPageOwner &other) { }
        virtual ~TabPageOwner();

        virtual void SetOwnerControl(TabControl *newowner);

        friend class PageControl;
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        int DesignLeft();
        int DesignTop();
        int DesignWidth();
        int DesignHeight();
        Rect DesignWindowRect();
#endif
        TabPageOwner();
        virtual void Destroy();

        PageControl *OwnerControl();
        void SetOwnerControl(PageControl *newowner);
        void SetOwnerAndIndex(PageControl *newowner, int index);
    };


    class PageControl : public TabControl
    {
    private:
        typedef TabControl  base;

        using base::AddTab;
        using base::InsertTab;
        using base::RemoveTab;
        using base::TabIndex;
        using base::Tabs;
        using base::DeleteTab;
        using base::TabCount;

        virtual Tab* AddTab();
    protected:
#ifdef DESIGNING
        virtual bool DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual bool DesignTabNext(bool entering, bool backwards) { return false; }
#endif

        virtual bool HandleNotify(Control *parent, LPARAM lParam, HRESULT &result); // Handles WM_NOTIFY messages sent to the parent control.
        virtual void ChildAddedNotify(Control *parent, Control *child);


        using base::SelectedTab;
        using base::SetSelectedTab;

        virtual ~PageControl();

        friend class TabPageOwner;
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        PageControl();

        TabPage* AddTabPage();
        TabPage* AddTabPage(const std::wstring& tabtext);
        int AddTabPage(TabPage *tab);
        int InsertTabPage(TabPage *tab, int index);
        TabPage* RemoveTabPage(int index);
        void RemoveTabPage(TabPage *tab);
        int TabPageIndex(TabPage *tab);
        TabPage* TabPages(int index);
        void DeleteTabPage(int index);
        int TabPageCount();

        TabPage* ActivePage();
        int ActivePageIndex();
        void SetActivePage(TabPage *newactivepage);
        void SetActivePageIndex(int pageindex);
    };


    // Listview

    enum ListviewDisplayStyles { ldsIcon, ldsSmallIcon, ldsList, ldsDetails, ldsTile,
#ifdef DESIGNING
                                 ldsCount = 5
#endif
    };

    enum HeaderColumnSortDirections { hcsdNone, hcsdUp, hcsdDown,
#ifdef DESIGNING
                                     hcsdCount = 3
#endif
    };

    enum ListviewSortDirections { lsdNone, lsdAscending, lsdDescending,
#ifdef DESIGNING
                                  lsdCount = 3
#endif
    };

    class Listview;

    class HeaderColumn : public Object
    {
    private:
        typedef Object  base;

        Listview *owner;

        /* Style and format */
        TextAlignments textalign; // Aligntment of text in columns. The column with an index of 0 cannot set anything byt taLeft.
        bool imageonright; // Items in this column show their images to the right of the text.

        int width; // Width of the column.
        std::wstring text; // Text shown in the header of the column.
        int subindex; // Which sub-items are shown in this column.
        int imageindex; // Index of image show in the header of the column from the listview's imagelist.
        int pos; // Position of this column among other columns.

        int defwidth; // Default width of a column. This is not used by the listview control, but can be set by the application.

        /* Undocumented style */
        HeaderColumnSortDirections sortdir;

        /* Style and format for Vista and above: */
        bool fixedwidth;
        bool nodpiscale;
        bool fixedratio;
        bool splitbutton;
    
        /* Vista and above: */
        int minwidth;

        HeaderColumn(Listview *owner);
#ifndef DESIGNING
        using base::Destroy;
#endif

        virtual ~HeaderColumn();
        void UpdateSubIndex(); // Sets the subindex after the column was created and added to a list view.

        friend class Listview;
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        //HeaderColumn(Listview *owner, const std::wstring &name);
        HeaderColumn(HeaderColumn *other);

        std::wstring DefaultText();
        int DefaultSubIndex();
        virtual void SetName(const std::wstring& newname);
        Imagelist* Images();

        virtual Object* SubOwner();
        virtual bool SubShown();
#endif
        virtual Form* ParentForm() const;
        Listview* OwnerControl();

        static LVCOLUMN FillData(HeaderColumn *column, UINT mask); // Fills the LVCOLUMN struct from the data in the column stored locally.
        int Index();

        const std::wstring& Text();
        void SetText(const std::wstring& newtext);
        TextAlignments TextAlignment();
        void SetTextAlignment(TextAlignments newalign);
        int Width();
        void SetWidth(int newwidth);
        int DefaultWidth();
        void SetDefaultWidth(int newdefwidth);
        int ImageIndex();
        void SetImageIndex(int newindex);
        bool ImagesOnRight();
        void SetImagesOnRight(bool newimageonright);

        HeaderColumnSortDirections SortDirection();
        void SetSortDirection(HeaderColumnSortDirections newsortdir);

        int Position();
        void SetPosition(int newposition);

        int SubIndex();
        void SetSubIndex(int newsubindex);

        /* Vista and above: */
        bool FixedWidth();
        void SetFixedWidth(bool newfixedwidth);
        bool NoDPIScale();
        void SetNoDPIScale(bool newnodpiscale);
        bool FixedRatio();
        void SetFixedRatio(bool newfixedratio);
        bool SplitButton();
        void SetSplitButton(bool newsplitbutton);

        int MinimumWidth();
        void SetMinimumWidth(int newminwidth);
    };

    enum ListviewGroupStates {
                //lgsNormal = LVGS_NORMAL, // The value of LVGS_NORMAL is 0, so if no other values are in the set, this is selected.
                lgsCollapsed = LVGS_COLLAPSED,
                lgsHidden = LVGS_HIDDEN,
                lgsNoHeader = LVGS_NOHEADER,
                lgsCollapsible = LVGS_COLLAPSIBLE,
                lgsFocused = LVGS_FOCUSED,
                lgsSelected = LVGS_SELECTED,
                lgsSubseted = LVGS_SUBSETED,
                lgsSubsetLinkFocused = LVGS_SUBSETLINKFOCUSED,
#ifdef DESIGNING
                lgsCount = 8
#endif
    };
    typedef uintset<ListviewGroupStates> ListviewGroupStateSet;

    class ListviewGroup : public Object
    {
    private:
        typedef Object  base;

        Listview *owner;

        unsigned int id;
        ListviewGroupStateSet state;
        std::wstring header;
        std::wstring footer;
        TextAlignments headeralign;
        TextAlignments footeralign;

        std::wstring subtitle;
        std::wstring task;

        int titleimageindex;
        int extimageindex;

        ListviewGroup(Listview *owner);
#ifndef DESIGNING
        using base::Destroy;
#endif
        virtual ~ListviewGroup();

        friend class Listview;
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        //ListviewGroup(Listview *owner);
        ListviewGroup(ListviewGroup *other);

        Imagelist* OwnerImagelist();

        virtual Object* SubOwner();
        virtual bool SubShown();
#endif
        virtual Form* ParentForm() const;
        Listview* OwnerControl();

        int Index();

        unsigned int Id();
        void SetId(unsigned int newid);
        ListviewGroupStateSet State();
        void SetState(ListviewGroupStateSet newstate);
        const std::wstring& Header();
        void SetHeader(const std::wstring& newheader);
        const std::wstring& Footer();
        void SetFooter(const std::wstring& newfooter);
        TextAlignments HeaderAlign();
        void SetHeaderAlign(TextAlignments newheaderalign);
        TextAlignments FooterAlign();
        void SetFooterAlign(TextAlignments newfooteralign);
        const std::wstring& Subtitle();
        void SetSubtitle(const std::wstring& newsubtitle);
        const std::wstring& Task();
        void SetTask(const std::wstring& newtask);

        int TitleImageIndex();
        void SetTitleImageIndex(int newtitleimageindex);
        int ExtendedImageIndex();
        void SetExtendedImageIndex(int newextendedimageindex);
    };

    enum ListviewOptions {
                    loShowHeader        = 0x01,
                    loButtonHeader      = 0x02,
                    loLabelWrap         = 0x04,
                    loAlwaysShowSelect  = 0x08,
                    loMultiselect       = 0x10,
                    loEditLabels        = 0x20,
#ifdef DESIGNING
                    loCount = 6
#endif
    };
    enum ListviewOptionsEx {
                    loxRowSelect        = 0x0001,
                    loxGridLines        = 0x0002,
                    loxHeaderDragDrop   = 0x0004,
                    loxHideIconLabels   = 0x0008,
                    loxOneClickActivate = 0x0010,
                    loxTwoClickActivate = 0x0020,
                    loxSimpleSelect     = 0x0040,
                    loxSnapToGrid       = 0x0080,
                    loxTrackSelect      = 0x0100,
                    loxUnderlineCold    = 0x0200,
                    loxUnderlineHot     = 0x0400,
#ifdef DESIGNING
                    loxCount = 11
#endif
    };

#ifdef DESIGNING
    class ListviewItem;
    // Sub item object for ListviewItems only used in the designer to provide properties for sub items.
    class ListviewSubitem : public Object 
    {
    private:
        typedef Object  base;

        ListviewItem *item;

        int Index() const;
    public:
        static void EnumerateProperties(DesignSerializer *serializer);
        ListviewSubitem(ListviewItem *item);
        Imagelist* OwnerImagelist();

        const ListviewItem* const Item() const;
        ListviewItem* Item();
        std::wstring Text() const;
        void SetText(const std::wstring &newtext);
        int ImageIndex() const;
        void SetImageIndex(int newindex);
    };
#endif

    enum ListviewItemParts { lipBounds = LVIR_BOUNDS, lipIcon = LVIR_ICON, lipLabel = LVIR_LABEL, lipIconAndLabel = LVIR_SELECTBOUNDS }; // Part of an item which can have a rectangle. lipIconAndLabel doesn't include the subitems, unlike lipBounds.
    enum ListviewSubitemParts { lspBounds = LVIR_BOUNDS, lspIcon = LVIR_ICON, lspLabel = LVIR_LABEL }; // Part of a sub item which can have a rectangle.
    class ListviewItem : public Object
    {
    private:
        typedef Object  base;
#ifdef DESIGNING
        std::vector<ListviewSubitem*> subobjects; // List of objects that represent sub items in the property editor which needs properties from real objects.
#endif
        Listview *owner;

        struct ListviewSubitemData
        {
            std::wstring text;
            int imageindex;

            ListviewSubitemData();
        };
        ListviewSubitemData **subitems;
        int subcount;

        byte state;
        byte overlaystateimage; // Low 4 bits are the overlay image index, high 4 bits are the state image index.
        std::wstring text;
        int imageindex;
        void *userdata;
        unsigned int indent; // Indentation.
        int group; // Index of the item's group or I_GROUPIDNONE when the item does not belong to any group.

        Point pos; // Position of the list view's item in views where the items can have positions.

        char tilecolcnt; // Number of subitems displayed for the item in tile view. 20 at most.
        UINT *tilecols; // tilecolcnt number of integers with the indexes of displayed subitems for the item in tile view.

        ListviewItem(Listview *owner);
        virtual ~ListviewItem();
        virtual void Destroy();

        friend class Listview;
        friend class ListviewGroup;
#ifdef DESIGNING
        friend class ListviewItemEditorList;
#endif
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        //ListviewItem(Listview *owner);
        ListviewItem(ListviewItem *other);

        void DesignAddSub();
        ListviewSubitem* Subitem(int subindex);
        int SubIndex(const ListviewSubitem *sub) const;
        Imagelist* OwnerImagelist();
        int OwnerGroupCount() const;
        std::wstring OwnerGroupStrings(int index);
        int OwnerGroupIds(int index);
#endif
        virtual Form* ParentForm() const;
        Listview* OwnerControl();

        int SubCount() const;
        int Index() const;
        int AddSub();
        void DeleteSubitem(int subindex);

        const std::wstring& Text() const;
        void SetText(const std::wstring &newtext);

        void* UserData() const; // Get a custom data set by SetUserData.
        void SetUserData(void *newuserdata); // Set a custom data used in sorting and looking up items.

        Point Position();
        void SetPosition(const Point& newpos);

        int ImageIndex() const;
        void SetImageIndex(int newindex);

        const std::wstring& SubitemText(int subindex) const;
        void SetSubitemText(int subindex, const std::wstring &newtext);
        int SubitemImageIndex(int subindex) const;
        void SetSubitemImageIndex(int subindex, int newindex);
        void SwapSubitems(int subindex1, int subindex2);
        unsigned int Indentation();
        void SetIndentation(unsigned int newindent);

        int GroupId();
        void SetGroupId(int newgroupid);
    };

    typedef uintset<ListviewOptions> ListviewOptionSet;
    typedef uintset<ListviewOptionsEx> ListviewOptionSetEx;
    class Listview : public SystemControl
    {
    private:
        typedef SystemControl   base;

        std::vector<HeaderColumn*> columns; // Columns of the listview visible in details view or if the header is always on.
        std::vector<ListviewGroup*> groups; // Groups for grouping items in the list view, when grouping is true.
        std::vector<ListviewItem*> items; // Items stored in the listview when the control is not virtual. Because only this class can be non-virtual and the real list view control is always virtual, this list holds all necessary data.
        std::vector<Rect> workareas; // Work areas within the list view, for grouping icons. These are simple rectangles, and when an icon is moved into them, the item of the icon becomes a member of the work area.

        Imagelist *largeimages;
        Imagelist *smallimages;
        Imagelist *stateimages;
        Imagelist *groupimages;

        ListviewDisplayStyles style;
        ListviewOptionSet options;
        ListviewOptionSetEx exoptions;
        ListviewSortDirections sortdir;

        int virtualcount; // Number of items listed in this list view control when the list view is virtual.
        bool virtualitems; // The list view control is virtual, and all items are provided by the user. Set virtualcount to the number of virtual items present in the listview, by calling SetVirtualCount.

        bool usegroups; // Only those items are visible in the list view that belong to a group, between group headers and footers.
        bool useworkareas; // Items are placed in work areas.

        Rect borders;

        void UpdateExOptions();
        void SaveColumns(); // Saves the current column widths and other data that can be changed by user interaction alone, so the columns will be recreated correctly.
        void SaveItems(); // Saves the item positions that can be changed by user interaction.
        void RestoreItemPositions(); // Sets the position of all items in the list view if it has a handle and the items are not virtual.

        int InsertColumn(HeaderColumn *column, int index = -1);
        int InsertGroup(ListviewGroup *group, int index = -1);
        int InsertItem(ListviewItem *item, int index = -1);
        int InsertPlaceholderItem(ListviewItem *item, int index = -1);

        LVGROUP FillGroupInfo(ListviewGroup *group);
        void UpdateGroupInfo(); // Specify a groupid if it is the id that must be updated.
        void UpdateGroupMetrics();
        void UpdateWorkAreas();
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual void SaveWindow();

        virtual bool HandleNotify(Control *parent, LPARAM lParam, HRESULT &result);
        virtual void DeleteNotify(Object *object);

        virtual ~Listview();
#ifdef DESIGNING
        friend class HeaderColumnEditorList;
        friend class ListviewItemEditorList;
#endif

        void UpdateColumnData(HeaderColumn *column, UINT mask); // Updates the columns in a list view from the information stored locally.
        void UpdateColumnOrder(const int *columnorder, int cnt);

        friend class HeaderColumn;
        friend class ListviewItem;
        friend class ListviewGroup;
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual void InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems);
        void EditColumns(void *sender, EventParameters param);
        void EditGroups(void *sender, EventParameters param);

        void SaveItems(std::vector<ListviewItem*> &datalist);
        void RestoreItems(std::vector<ListviewItem*> &datalist);

        void SaveGroups(std::vector<ListviewGroup*> &datalist, std::vector<int> &indexlist);
        void RestoreGroups(std::vector<ListviewGroup*> &datalist, std::vector<int> &indexlist);

        //virtual Object* NameOwner(const std::wstring &name);
        //virtual void Names(std::vector<std::wstring> &namelist);
        void DesignAddColumn();
        void DesignAddGroup();
        void DesignAddItem();
#endif
        Listview();
        virtual void Destroy();

        void InvalidateHeader(); // Invalidates the header area.

        bool Virtual();
        void SetVirtual(bool newvirtual);

        ListviewDisplayStyles Style();
        void SetStyle(ListviewDisplayStyles newstyle);

        ListviewOptionSet Options();
        void SetOptions(ListviewOptionSet newoptions);
        ListviewOptionSetEx OptionsEx();
        void SetOptionsEx(ListviewOptionSetEx newexoptions);

        ListviewSortDirections SortDirection();
        void SetSortDirection(ListviewSortDirections newsortdir);

        int ColumnCount() const;
        HeaderColumn* AddColumn();
        HeaderColumn* InsertColumn(int index = -1);
        HeaderColumn* Columns(int index);
        void DeleteColumn(int index);
        int ColumnIndex(HeaderColumn *column);

        Imagelist* LargeImages();
        void SetLargeImages(Imagelist *newlargeimages);
        Imagelist* SmallImages();
        void SetSmallImages(Imagelist *newsmallimages);
        Imagelist* StateImages();
        void SetStateImages(Imagelist *newstateimages);
        Imagelist* GroupImages();
        void SetGroupImages(Imagelist *newgroupimages);

        int ItemCount() const;
        void SetVirtualCount(int newcount);
        ListviewItem* AddItem();
        ListviewItem* InsertItem(int index);
        ListviewItem* Items(int index);
        int ItemIndex(const ListviewItem *item);
        void DeleteItem(int index);
        void SwapItems(int index1, int index2);

        Rect ItemRectangle(int index, ListviewItemParts part);
        void InvalidateItemPart(int index, ListviewItemParts part);
        Rect SubitemRectangle(int index, int subindex, ListviewSubitemParts part);
        void InvalidateSubitemPart(int index, int subindex, ListviewSubitemParts part);

        bool UseGroups();
        void SetUseGroups(bool newusegroups);

        int GroupCount() const;
        ListviewGroup* AddGroup();
        ListviewGroup* InsertGroup(int index);
        ListviewGroup* Groups(int index);
        int GroupIndex(ListviewGroup *group);
        void DeleteGroup(int index);
        void SwapGroups(int index1, int index2);

        void SetGroupBorders(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);
        unsigned int GroupBorderLeft();
        void SetGroupBorderLeft(unsigned int newborder);
        unsigned int GroupBorderTop();
        void SetGroupBorderTop(unsigned int newborder);
        unsigned int GroupBorderRight();
        void SetGroupBorderRight(unsigned int newborder);
        unsigned int GroupBorderBottom();
        void SetGroupBorderBottom(unsigned int newborder);

        bool UseWorkAreas();
        void SetUseWorkAreas(bool newuseworkareas);

        int WorkAreaCount();
        Rect WorkArea(int index);
        void AddWorkArea(const Rect &newarea);
        void InsertWorkArea(const Rect &newarea, int index);
        void SetWorkArea(const Rect &newarea, int index);
        void DeleteWorkArea(int index);
        void ClearWorkAreas();

        ListviewGroupTaskEvent OnGroupTaskClick;
        BeginListviewItemEditEvent OnBeginEdit;
        EndListviewItemEditEvent OnEndEdit;
        CancelListviewItemEditEvent OnCancelEdit;
    };


    // Status bar and helper types:

    enum StatusBarPartBevels : int { sbpbLowered, sbpbRaised, sbpbNone,
#ifdef DESIGNING
            sbpbCount = 3
#endif
    };
    class StatusBar;
#ifdef DESIGNING
    class StatusBarPart : public Object
    {
    private:
        typedef Object  base;

        StatusBar *owner;
        bool ownerdrawn;

        friend class StatusBar;
    protected:
        virtual ~StatusBarPart();
    public:
        static void EnumerateProperties(DesignSerializer *serializer);
        bool DesignOwnerDrawn() const;
        void DesignSetOwnerDrawn(bool newdrawn);

        StatusBarPart(StatusBar *owner);

        StatusBar* Owner() const;

        std::wstring Text() const;
        void SetText(const std::wstring &newtext);
        TextAlignments TextAlignment() const;
        void SetTextAlignment(TextAlignments newalign);
        StatusBarPartBevels Bevel() const;
        void SetBevel(StatusBarPartBevels newbevel);
        unsigned int Width() const;
        void SetWidth(int unsigned newwidth);
    };
#else
    class StatusBarPart
    {
    private:
        StatusBar *owner;
        int index;

        void CheckValidity() const;

        StatusBarPart(StatusBar *owner, int index);

        void MakeInvalid(); // Sets index to -1.
        void OwnerMakeInvalid(); // Sets index to -1 and owner to null.

        friend class StatusBar;
    public:
        StatusBar* Owner() const;
        StatusBarPart();
        ~StatusBarPart();

        StatusBarPart(const StatusBarPart &other);
        StatusBarPart(StatusBarPart &&other);
        StatusBarPart& operator=(const StatusBarPart &other);
        StatusBarPart& operator=(StatusBarPart &&other);

        bool Valid(); // The part is still usable for accessing its status bar owner.

        int Index() const;
        std::wstring Text();
        void SetText(const std::wstring &newtext);
        TextAlignments TextAlignment();
        void SetTextAlignment(TextAlignments newalign);
        StatusBarPartBevels Bevel();
        void SetBevel(StatusBarPartBevels newbevel);
        unsigned int Width();
        void SetWidth(int unsigned newwidth);
        Rect Area();
    };
#endif

    enum StatusBarSizeGrips : int { sbsgNone, sbsgSizeGrip, sbsgAuto,
#ifdef DESIGNING
            sbsgCount = 3
#endif
    };
    class StatusBar : public SystemControl
    {
    private:
        typedef SystemControl   base;

        StatusBarSizeGrips grip; // The sizing grip is visible or not. Set to sbgAuto (default) to show a grip if the parent form has one.
        bool simple; // The status bar has a single area for text.
        std::wstring simpletext; // Text shown in the single area.

#ifdef DESIGNING
        std::vector<StatusBarPart*> parts; // An object for all parts during designing. 
        bool designsimple; // Value of simple, as shown in the designer.
#else
        std::vector<StatusBarPart*> parts; // A pointer to the stack allocated objects that were handed out. Does not contain an object for all parts, but can contain multiple parts for a single index with different pointers.
#endif
        struct PartData
        {
            unsigned int width;
            std::wstring text;
            StatusBarPartBevels bevel;
            bool ownerdrawn;
            TextAlignments align;
        };
        std::vector<PartData> datavec;

        void UpdatePart(int ix);
        void UpdatePartText(int ix);
        void UpdateParts();
        void SetParts();

        friend class StatusBarPart;
#ifdef DESIGNING
        void SaveParts(std::vector<PartData> &datalist, bool &simpledata);
        void RestoreParts(std::vector<PartData> &datalist, bool simpledata);
        void RestoreSimple(bool simpledata);

        friend class StatusBarPartEditorList;
#else
        void PartAdded(StatusBarPart *part);
        void RemovePart(StatusBarPart *part);
#endif
    protected:
#ifdef DESIGNING
        virtual Size DesignSize();
#endif
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void SaveWindow();
        virtual void InitHandle();

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual bool HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures);

        virtual ~StatusBar();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        StatusBarPart* DesignParts(int ix);
        void DesignAddPart();

        bool DesignSimple();
        void DesignSetSimple(bool newsimple);

        StatusBarPart* Parts(int ix);
        int PartIndex(const StatusBarPart *part) const;
#endif
        StatusBar();
        virtual void Destroy();

        std::wstring Text() const; // Text shown in the status bar when it has a simple area.
        void SetText(const std::wstring &newtext); // Sets the text shown in the status bar when it has a simple area.

        StatusBarSizeGrips SizeGrip() const;
        void SetSizeGrip(StatusBarSizeGrips newgrip);

        bool SimpleArea() const;
        void SetSimpleArea(bool newsimple);

        int AddPart(const std::wstring &text, TextAlignments align, unsigned int width, StatusBarPartBevels bevel, bool ownerdrawn);
        void InsertPart(int index, const std::wstring &text, TextAlignments align, unsigned int width, StatusBarPartBevels bevel, bool ownerdrawn);
        std::wstring PartText(int index) const; // Returns the stored text value. If the status bar was modified via its handle from the outside, this can give different result to the non-const version.
        std::wstring PartText(int index);
        void SetPartText(int index, const std::wstring &newtext);
        TextAlignments PartTextAlignment(int index) const;
        TextAlignments PartTextAlignment(int index);
        void SetPartTextAlignment(int index, TextAlignments newalign);
        unsigned int PartWidth(int index) const; // Returns the stored width value. If the status bar was modified via its handle from the outside, this can give different result to the non-const version.
        unsigned int PartWidth(int index);
        void SetPartWidth(int index, unsigned int newwidth);
        bool PartOwnerDrawn(int index) const; // Returns the stored ownerdrawn value. If the status bar was modified via its handle from the outside, this can give different result to the non-const version.
        bool PartOwnerDrawn(int index);
        void SetPartOwnerDrawn(int index, bool newownerdrawn);
        StatusBarPartBevels PartBevel(int index) const; // Returns the stored bevel value. If the status bar was modified via its handle from the outside, this can give different result to the non-const version.
        StatusBarPartBevels PartBevel(int index);
        void SetPartBevel(int index, StatusBarPartBevels newbevel);
        Rect PartArea(int index);

        int PartCount() const; // Returns the stored number of parts. If the status bar was modified via its handle from the outside, this can give different result to the non-const version.
        //int PartCount();
        void DeletePart(int index);
        void SwapParts(int index1, int index2);

#ifndef DESIGNING
        StatusBarPart Parts(int index);
#endif

        DrawItemEvent OnDrawPart;
    };

    // Tooltip window

    class Tooltip : public SystemControl
    {
    private:
        typedef SystemControl   base;
        std::vector<Control *> controls; // List of controls registered to this tooltip.

        bool alwaysshow; // Shows the tooltip even for inactive controls
        bool balloon; // Make this tooltip into a balloon with rounded corners and a pointing stem.
        bool closebtn; // Show a close button on the tooltip. Only for balloon tips that have a title text.
        bool noanimate; // Disables sliding animation.
        bool nofade; // Don't fade in/out when shown.
        bool hideprefix; // Disable hiding the & prefixes.
        bool styledlinks; // Uses themed hyperlinks. The theme will define the styles for any links in the tooltip.
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();

        virtual ~Tooltip();
    public:
        Tooltip();

        bool AlwaysShow();
        void SetAlwaysShow(bool newalwaysshow);
        bool Balloon();
        void SetBalloon(bool newballoon);
        bool CloseButton();
        void SetCloseButton(bool newclosebtn);
        bool NoAnimate();
        void SetNoAnimate(bool newnoanimate);
        bool NoFade();
        void SetNoFade(bool newnofade);
        bool HidePrefix();
        void SetHidePrefix(bool newhideprefix);
        bool StyledHyperlinks();
        void SetStyledHyperlinks(bool newstyledlinks);

        void Register(Control *control);
        void Deregister(Control *control);
    };

}
/* End of NLIBNS */


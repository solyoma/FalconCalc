#include "stdafx_zoli.h"
#include "application.h"

#include "controlbase.h"
#include "syscontrol.h"
#include "canvas.h"
#include "screen.h"
#include "themes.h"
#ifdef DESIGNING
#include "property_controlbase.h"
#include "property_screen.h"
#include "property_canvas.h"
#include "property_menu.h"
#include "designproperties.h"
#include "designer.h"
//#include "designercontrols.h"
#include "serializer.h"
#include "designerform.h"

#define amHijackChild       (WM_APP + 5)
#endif

namespace NLIBNS
{


    Control* _creation_window = NULL;
    Rect Control::doublebufrect;
    Control::PaintBuffer Control::paintbuffer = { 0 };
    int Control::bufcnt = 0;
    bool Control::vistabuf = false;
    HBITMAP Control::olddblbmp = NULL;

    extern "C"
    {
        typedef HPAINTBUFFER (WINAPI *BeginBufferedPaint_T)(HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
        typedef HRESULT (WINAPI *EndBufferedPaint_T)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
        extern BeginBufferedPaint_T BeginBufferedPaint_REDEFINED;
        extern EndBufferedPaint_T EndBufferedPaint_REDEFINED;
    }

#ifdef DESIGNING

    ValuePair<ControlAlignments> ControlAlignmentStrings[] = {
        VALUEPAIR(alNone),
        VALUEPAIR(alClient),
        VALUEPAIR(alLeft),
        VALUEPAIR(alTop),
        VALUEPAIR(alRight),
        VALUEPAIR(alBottom),
        VALUEPAIR(alAnchor),
    };

    ValuePair<ControlAlignmentOrders> ControlAlignmentOrderStrings[] = {
        VALUEPAIR(caoTopBeforeLeft),
        VALUEPAIR(caoTopBeforeRight),
        VALUEPAIR(caoBottomBeforeLeft),
        VALUEPAIR(caoBottomBeforeRight),
    };

    ValuePair<BorderStyles> BorderStyleStrings[] = {
        VALUEPAIR(bsNormal),
        VALUEPAIR(bsNone),
        VALUEPAIR(bsModern),
        VALUEPAIR(bsSingle),
    };

    ValuePair<ControlAnchors> ControlAnchorStrings[] = {
        VALUEPAIR(caLeft),
        VALUEPAIR(caTop),
        VALUEPAIR(caRight),
        VALUEPAIR(caBottom),
    };

    ValuePair<TextAlignments> TextAlignmentStrings[] = {
        VALUEPAIR(taLeft),
        VALUEPAIR(taRight),
        VALUEPAIR(taCenter),
    };

    ValuePair<VerticalTextAlignments> VerticalTextAlignmentStrings[] = {
        VALUEPAIR(vtaTop),
        VALUEPAIR(vtaMiddle),
        VALUEPAIR(vtaBottom),
    };

    ValuePair<WantedKeys> WantedKeyStrings[] = {
        VALUEPAIR(wkArrows),
        VALUEPAIR(wkTab),
        VALUEPAIR(wkEnter),
        VALUEPAIR(wkEscape),
        VALUEPAIR(wkOthers),
    };

    bool MatchColorString4(const std::wstring& val, int &a, int &r, int &g, int &b)
    {
        int lpos = 0;
        int ppos = 0;

        int *c;
        int col = 0;
        int vlen = val.length();
        while (ppos <= vlen)
        {
            while (ppos < vlen && (val[ppos] == L' ' || val[ppos] == L'\t'))
                ++ppos;
            if (col == 4)
                return (ppos == vlen);

            lpos = ppos;
            while (ppos < vlen && (val[ppos] >= L'0' && val[ppos] <= L'9'))
                ppos++;
            if (lpos == ppos || ppos - lpos > 3 || (col != 3 && (ppos == vlen || (val[ppos] != L' ' && val[ppos] != L'\t' && val[ppos] != L','))))
                return false;

            c = (col == 0 ? &a : col == 1 ? &r : col == 2 ? &g : &b);
            if (!StrToInt(val, *c, lpos, ppos - lpos))
                return false;

            c++;
        }
        return false;
    }

    bool MatchColorString3(const std::wstring& val, int &r, int &g, int &b)
    {
        int lpos = 0;
        int ppos = 0;

        int *c;
        int col = 0;
        int vlen = val.length();
        while (ppos <= vlen)
        {
            while (ppos < vlen && (val[ppos] == L' ' || val[ppos] == L'\t'))
                ++ppos;
            if (col == 3)
                return (ppos == vlen);

            lpos = ppos;
            while (ppos < vlen && (val[ppos] >= L'0' && val[ppos] <= L'9'))
                ppos++;
            if (lpos == ppos || ppos - lpos > 3 || (col != 2 && (ppos == vlen || (val[ppos] != L' ' && val[ppos] != L'\t' && val[ppos] != L','))))
                return false;

            c = (col == 0 ? &r : col == 1 ? &g : &b);
            if (!StrToInt(val, *c, lpos, ppos - lpos))
                return false;

            c++;
        }
        return false;
    }


    //---------------------------------------------


#endif
    Control::ControlFont::ControlFont (Control *owner, const LOGFONT &lf) : base(FontData(lf)),
                //    lf.lfFaceName,lf.lfHeight < 0 ? FontSizeFromHeight(lf.lfHeight) : FontSizeFromLogfont(lf),
                //    clBtnText, lf.lfWeight >= FW_SEMIBOLD, lf.lfItalic == TRUE, lf.lfUnderline == TRUE,
                //    lf.lfStrikeOut == TRUE, (FontCharacterSets)lf.lfCharSet, (FontOutputQualities)lf.lfQuality
                //)),
                owner(owner)
    {
    }

    void Control::ControlFont::DoChanged(const FontData &saveddata)
    {
        owner->UpdateFont(saveddata);
    }


    //---------------------------------------------


#ifdef DESIGNING
    Font* Control::defaultfont = NULL;

    void Control::StartDeserialize()
    {
        if (controlstate.contains(csDeserialize))
            return;
        controlstate << csDeserialize;
    }

    void Control::FinishDeserialize()
    {
        if (!controlstate.contains(csDeserialize))
            return;
        controlstate -= csDeserialize;
        if (controls)
            LayoutChildren(HandleCreated() ? ClientRect() : WindowRect(), false);
        Show();
    }

    void Control::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->Add(L"SetLeft", new IntDesignProperty<Control>(L"Left", L"Positioning", &Control::Left, &Control::SetLeft))->DontExport();
        serializer->Add(L"SetTop", new IntDesignProperty<Control>(L"Top", L"Positioning", &Control::DesignTop, &Control::DesignSetTop))->DontExport();
        serializer->Add(L"SetWidth", new IntDesignProperty<Control>(L"Width", L"Dimensions", &Control::Width, &Control::SetWidth))->DontExport();
        serializer->Add(L"SetHeight", new IntDesignProperty<Control>(L"Height", L"Dimensions", &Control::Height, &Control::SetHeight))->DontExport();
        serializer->Add(L"SetBounds", new RectDesignProperty<Control>(L"Bounds", std::wstring(), true, false, &Control::DesignWindowRect, &Control::DesignSetBounds))->DontList()->DontSerialize();

        serializer->Add(L"SetText", new StringDesignProperty<Control>(L"Text", L"Contents", &Control::Text, &Control::SetText))->SetDefault(&Control::DefaultText)->SetDefaultWrite(std::wstring())->SetImmediateUpdate(true);
        serializer->Add(L"SetBorderStyle", new BorderStylesDesignProperty<Control>(L"BorderStyle", L"Appearance", &Control::BorderStyle, &Control::SetBorderStyle))->SetDefault(bsNone);
        serializer->Add(L"SetCursor", new CursorDesignProperty<Control>(L"Cursor", L"Behavior", &Control::Cursor, &Control::SetCursor))->SetDefault(cDefault);
        serializer->Add(L"SetEnabled", new BoolDesignProperty<Control>(L"Enabled", L"Behavior", &Control::Enabled, &Control::SetEnabled))->SetDefault(true);
        serializer->Add(L"SetVisible", new BoolDesignProperty<Control>(L"Visible", L"Behavior", &Control::DesignVisible, &Control::DesignSetVisible))->SetDefault(true);
        serializer->Add(L"SetAnchors", new ControlAnchorSetDesignProperty<Control>(L"Anchors", L"Positioning", &Control::Anchors, &Control::SetAnchors))->SetDefault(caLeft | caTop);
        serializer->Add(L"SetPadding", new RectDesignProperty<Control>(L"Padding", L"Positioning", false, true, &Control::Padding, &Control::SetPadding))->SetDefault(Rect());
        // Alignment must come after anchors, to be able to set it to something else than alAnchor.
        serializer->Add(L"SetAlignment", new ControlAlignmentsDesignProperty<Control>(L"Alignment", L"Positioning", &Control::Alignment, &Control::SetAlignment))->SetDefault(&Control::DefaultAlignment);
        serializer->Add(L"SetAlignmentOrder", new ControlAlignmentOrdersDesignProperty<Control>(L"AlignmentOrder", L"Positioning", &Control::AlignmentOrder, &Control::SetAlignmentOrder))->SetDefault(caoTopBeforeLeft | caoTopBeforeRight  | caoBottomBeforeLeft | caoBottomBeforeRight);
        serializer->Add(L"SetAlignMargin", new RectDesignProperty<Control>(L"AlignMargin", L"Positioning", false, true, &Control::AlignMargin, &Control::SetAlignMargin))->SetDefault(Rect());
        serializer->Add(L"GetFont", new FontDesignProperty<Control>(L"Font", L"Appearance", true, &Control::GetFont, &Control::SetFont))->SetDefault(&Control::DefaultFont)/*->SetAsPointerValue(false)*/;
        serializer->Add(L"SetParentFont", new BoolDesignProperty<Control>(L"ParentFont", L"Appearance", &Control::ParentFont, &Control::SetParentFont))->SetDefault(true);
        serializer->Add(L"SetColor", new ColorDesignProperty<Control>(L"Color", L"Appearance", true, false, false, &Control::GetColor, &Control::SetColor))->SetDefault(&Control::DefaultColor);
        serializer->Add(L"SetParentBackground", new BoolDesignProperty<Control>(L"ParentBackground", L"Appearance", &Control::ParentBackground, &Control::SetParentBackground))->SetDefault(true);
        serializer->Add(L"SetParentColor", new BoolDesignProperty<Control>(L"ParentColor", L"Appearance", &Control::ParentColor, &Control::SetParentColor))->SetDefault(true);
        serializer->Add(L"SetTabOrder", new IntDesignProperty<Control>(L"TabOrder", L"Behavior", &Control::TabOrder, &Control::SetTabOrder));
        serializer->Add(L"SetAcceptInput", new BoolDesignProperty<Control>(L"AcceptInput", L"Behavior", &Control::DesignAcceptInput, &Control::DesignSetAcceptInput))->SetDefault(true);
        serializer->Add(L"SetPopupMenu", new PopupMenuDesignProperty<Control>(L"PopupMenu", L"Behavior", &Control::DesignGetPopupMenu, &Control::DesignSetPopupMenu));

        serializer->Add(L"SetTooltipText", new StringDesignProperty<Control>(L"TooltipText", L"Behavior", &Control::TooltipText, &Control::SetTooltipText))->SetImmediateUpdate(false)->SetDefault(std::wstring());
        serializer->Add(L"SetShowTooltip", new BoolDesignProperty<Control>(L"ShowTooltip", L"Behavior", &Control::ShowTooltip, &Control::SetShowTooltip))->SetDefault(true);
        serializer->Add(L"SetParentTooltip", new BoolDesignProperty<Control>(L"ParentTooltip", L"Behavior", &Control::ParentTooltip, &Control::SetParentTooltip))->SetDefault(true);

        serializer->Add(L"SetWantedKeyTypes", new WantedKeySetDesignProperty<Control>(L"WantedKeyTypes", L"Behavior", &Control::WantedKeyTypes, &Control::SetWantedKeyTypes))->SetDefault(wkOthers);
        serializer->Add(L"SetDoubleBuffered", new BoolDesignProperty<Control>(L"DoubleBuffered", L"Appearance", &Control::DesignDoubleBuffered, &Control::SetDoubleBuffered))->SetDefault(false);

        /* Events */
        serializer->AddEvent<Control, MessageEvent>(L"OnMessage");
        serializer->AddEvent<Control, PaintEvent>(L"OnPaint", L"Drawing");
        serializer->AddEvent<Control, NotifyEvent>(L"OnShow", L"Visibility");
        serializer->AddEvent<Control, NotifyEvent>(L"OnHide", L"Visibility");

        serializer->AddEvent<Control, NotifyEvent>(L"OnCaptureLost", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnMouseEnter", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnMouseLeave", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnNCMouseEnter", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnNCMouseLeave", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnMouseEntered", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnMouseLeft", L"Mouse");
        serializer->AddEvent<Control, FocusChangedEvent>(L"OnGainFocus", L"Focus");
        serializer->AddEvent<Control, FocusChangedEvent>(L"OnLoseFocus", L"Focus");
        serializer->AddEvent<Control, ActiveChangedEvent>(L"OnEnter", L"Focus");
        serializer->AddEvent<Control, ActiveChangedEvent>(L"OnLeave", L"Focus");
        serializer->AddEvent<Control, KeyEvent>(L"OnKeyDown", L"Keys");
        serializer->AddEvent<Control, KeyEvent>(L"OnKeyUp", L"Keys");
        serializer->AddEvent<Control, KeyPushEvent>(L"OnKeyPush", L"Keys");
        serializer->AddEvent<Control, KeyPressEvent>(L"OnKeyPress", L"Keys");
        serializer->AddEvent<Control, MouseMoveEvent>(L"OnMouseMove", L"Mouse");
        serializer->AddEvent<Control, MouseButtonEvent>(L"OnMouseDown", L"Mouse");
        serializer->AddEvent<Control, MouseButtonEvent>(L"OnMouseUp", L"Mouse");
        serializer->AddEvent<Control, MouseButtonEvent>(L"OnDblClick", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnClick", L"Mouse");
        serializer->AddEvent<Control, MouseWheelEvent>(L"OnWantMouseWheel", L"Mouse");
        serializer->AddEvent<Control, MouseWheelEvent>(L"OnMouseWheel", L"Mouse");

        serializer->AddEvent<Control, NCMouseMoveEvent>(L"OnNCMouseMove", L"Mouse");
        serializer->AddEvent<Control, NCMouseButtonEvent>(L"OnNCMouseDown", L"Mouse");
        serializer->AddEvent<Control, NCMouseButtonEvent>(L"OnNCMouseUp", L"Mouse");
        serializer->AddEvent<Control, NCMouseButtonEvent>(L"OnNCDblClick", L"Mouse");
        serializer->AddEvent<Control, NotifyEvent>(L"OnStartSizeMove", L"Position");
        serializer->AddEvent<Control, SizePositionChangedEvent>(L"OnSizeChanged", L"Position");
        serializer->AddEvent<Control, SizePositionChangedEvent>(L"OnPositionChanged", L"Position");
        serializer->AddEvent<Control, SizePositionChangedEvent>(L"OnEndSizeMove", L"Position");
        serializer->AddEvent<Control, NotifyEvent>(L"OnResize", L"Position");
        serializer->AddEvent<Control, NotifyEvent>(L"OnMove", L"Position");

        serializer->AddEvent<Control, DialogCodeEvent>(L"OnDialogCode", L"Keys");

        serializer->AddEvent<Control, DragImageRequestEvent>(L"OnDragImageRequest", L"DragDrop");
        serializer->AddEvent<Control, DragDropEndedEvent>(L"OnDragDropEnded", L"DragDrop");
        serializer->AddEvent<Control, NotifyEvent>(L"OnDragLeave", L"DragDrop");
        serializer->AddEvent<Control, DragDropEvent>(L"OnDragEnter", L"DragDrop");
        serializer->AddEvent<Control, DragDropEvent>(L"OnDragMove", L"DragDrop");
        serializer->AddEvent<Control, DragDropDropEvent>(L"OnDragDrop", L"DragDrop");
    }

    PopupMenu* Control::DesignGetPopupMenu()
    {
        return designpopupmenu;
    }

    void Control::DesignSetPopupMenu(PopupMenu *newpopupmenu)
    {
        if (designpopupmenu == newpopupmenu)
            return;
        RemoveFromNotifyList(designpopupmenu, nrSubControl);
        designpopupmenu = newpopupmenu;
        AddToNotifyList(designpopupmenu, nrSubControl);
    }

    DesignProperty* Control::ParentProperty()
    {
        return parentproperty;
    }

    void Control::SetParentProperty(DesignProperty *prop)
    {
        parentproperty = prop;
    }

    void Control::SetName(const std::wstring& newname)
    {
        if (!Name().empty() && Text() == Name())
        {
            SetText(newname);
            if (Designing() && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"Text");
        }
        base::SetName(newname);
    }

    Color Control::DefaultColor()
    {
        if (ParentColor())
            return parent->GetColor();
        return clWindow;
    }

    std::wstring Control::DefaultText()
    {
        return Name(); /*std::wstring()*/;
    }

    Font& Control::DefaultFont()
    {
        if (controlstyle.contains(csChild) && controlstyle.contains(csParentFont))
            return GetFont();

        return *defaultfont;
    }

    ControlAlignments Control::DefaultAlignment()
    {
        bool defanc = (Anchors() == (caLeft | caTop));
        return (defanc ? alNone : alAnchor);
    }

    std::wstring Control::SerializeBounds()
    {
        std::wstringstream ws;
        Rect r = WindowRect();
        ws << r.left << L", " << r.top << L", " << r.Width() << L", " << r.Height();
        return ws.str();
    }

    bool Control::DesignVisible()
    {
        return designvisible;
    }

    void Control::DesignSetVisible(bool newvisible)
    {
        designvisible = newvisible;
    }

    bool Control::DesignAcceptInput()
    {
        return controlstyle.contains(csAcceptInput);
    }

    void Control::DesignSetAcceptInput(bool newacceptinput)
    {
        if (newacceptinput)
            controlstyle << csAcceptInput;
        else
            controlstyle -= csAcceptInput;
    }

    int Control::DesignTop()
    {
        if (!Designing() || !Parent() || !ParentForm() || Parent() != ParentForm() || !DesignParent() || dynamic_cast<DesignForm*>(DesignParent())->DesignGetMenu() == NULL)
            return Top();
        return Top() - GetSystemMetrics(SM_CYMENU);
    }

    void Control::DesignSetTop(int newtop)
    {
        if (!Designing() || !Parent() || !ParentForm() || Parent() != ParentForm() || !DesignParent() || dynamic_cast<DesignForm*>(DesignParent())->DesignGetMenu() == NULL)
            SetTop(newtop);
        else
            SetTop(newtop + GetSystemMetrics(SM_CYMENU));
    }

    Rect Control::DesignWindowRect()
    {
        Rect r = WindowRect();
        if (!Designing() || !Parent() || !ParentForm() || Parent() != ParentForm() || !DesignParent() || dynamic_cast<DesignForm*>(DesignParent())->DesignGetMenu() == NULL)
            return r;
        return r.Offset(0, -GetSystemMetrics(SM_CYMENU));
    }

    void Control::DesignSetBounds(const Rect& r)
    {
        if (!Designing() || !Parent() || !ParentForm() || Parent() != ParentForm() || !DesignParent() || dynamic_cast<DesignForm*>(DesignParent())->DesignGetMenu() == NULL)
            SetBounds(r);
        else
            SetBounds(r.Offset(0, GetSystemMetrics(SM_CYMENU)));
    }

    bool Control::DesignDoubleBuffered() const
    {
        return doublebuffered;
    }

    Control::Control() : base(), designpopupmenu(NULL), designvisible(true), parentproperty(NULL),
#else
    Control::Control() :
#endif
            handle(NULL), parent(NULL), popupmenu(NULL), controls(NULL), updatecnt(0), droptarget(NULL), paintdc(NULL),
            canvas(NULL), color(clWindow), border(bsNone), cursor(cDefault), wantedkeys(wkOthers), bgbrush(NULL),
            align(alNone), alignorder(caoTopBeforeLeft | caoTopBeforeRight  | caoBottomBeforeLeft | caoBottomBeforeRight),
            anchors(caLeft | caTop), sizewidthdiff(0), sizemodx(0), sizeheightdiff(0), sizemody(0), usersizemove(false),
            doublebuffered(false), taborder(-1), childpaintcnt(0), erasechild(NULL)
    {
        controlstyle << csSelfDrawn << csNoDefaultPaint << csMouseCapture << csChild << csParentColor << csParentBackground << csParentFont << csEraseOnTextChange << csUpdateOnTextChange << csParentTooltip << csShowTooltip << csEraseToColor;
        controlstate << csVisible << csEnabled;

#ifdef DESIGNING
        LOGFONT lf = application->UILogFont();
        lf.lfFaceName[0] = 0;
        if (defaultfont == 0)
            defaultfont = new Font(lf);

        font = new ControlFont(this, lf);
#else
        font = new ControlFont(this, application->UILogFont());
#endif
    }

    Control::~Control()
    {
        delete canvas;
        delete font;
        if (bgbrush)
            DeleteObject(bgbrush);
    }

    void Control::Destroy()
    {
        if (controlstate.contains(csDestroying))
            throw L"The control is already being destroyed!";

        controlstate << csDestroying;

        if (controls)
            controls->Destroy();

        if (parent != nullptr && !parent->controlstate.contains(csDestroying))
        {
            if (controlstyle.contains(csChild))
                SetParent(nullptr);
            else
                SetTopLevelParent(nullptr);
        }

        if (!controlstyle.contains(csChild))
            RemoveTopChildren(true);

        DestroyHandle();

        base::Destroy();
    }

    void Control::RemoveTopChildren(bool passtoparent)
    {
        std::vector<Control*> ccopy(topcontrols);
        topcontrols = std::vector<Control*>();
        for (auto it = ccopy.begin(); it != ccopy.end(); it++)
            (*it)->SetTopLevelParent(passtoparent ? TopLevelParent() : nullptr);
        ccopy.clear();
    }

    void Control::CreateClassParams(ClassParams &params)
    {
        params.brush = (HBRUSH)COLOR_WINDOW;
        params.classname = std::wstring();
        params.cursor = LoadCursor(NULL, IDC_ARROW);
        params.icon = NULL;
        params.iconsm = NULL;
        params.brush = 0;
        params.wndextra = 0;
        params.style << csDoubleClicks;
    }

    void Control::CreateWindowParams(WindowParams &params)
    {
        params.windowtext = text;
        params.parent = parent ? parent->Handle() : NULL;

        params.style << wsClipSiblings;

        if (controlstyle.contains(csInTabOrder) && controlstyle.contains(csChild) && AcceptInput())
            params.style << wsTabStop;

        if (controls)
        {
            params.style << wsClipChildren;
            params.extstyle << wsExControlParent;
        }

        //params.extstyle << wsExComposited;

        if (controlstate.contains(csVisible))
            params.style << wsVisible;
        if (!controlstate.contains(csEnabled))
            params.style << wsDisabled;
        if (controlstyle.contains(csChild))
            params.style << wsChild;
        if (border == bsSingle)
            params.style << wsBorder;
        else if (border == bsNormal || border == bsModern)
            params.extstyle << wsExClientEdge;
        if (controlstyle.contains(csTransparent))
            params.extstyle << wsExTransparent;

        params.x = rect.left;
        params.y = rect.top;
        params.width = (UINT)rect.right == CW_USEDEFAULT || (UINT)rect.left == CW_USEDEFAULT ? rect.right : rect.Width();
        params.height = (UINT)rect.bottom == CW_USEDEFAULT || (UINT)rect.top == CW_USEDEFAULT ? rect.bottom : rect.Height();
        //params.cursor = NULL;
        //params.icon = NULL;
        //params.iconsm = NULL;
        params.menu = NULL;
    }

    void Control::CreateHandle()
    {
        if (handle != NULL)
            throw L"Handle already created!";

        if (controlstate.contains(csDestroyingHandle))
            throw L"Cannot create handle when destroying Control.";

        if (controlstate.contains(csCreatingHandle))
            throw L"Handle is already being created.";

        if (!parent && controlstyle.contains(csChild))
            throw L"Cannot create handle for parentless child controls.";

        if (parent && !parent->HandleCreated())
        {
            std::vector<Control*> stack;
            stack.push_back(this);
            Control *c = parent;

            while (c && !c->HandleCreated())
            {
                stack.push_back(c);
                c = c->Parent();
            }

            for (auto it = stack.rbegin(); it != stack.rend(); ++it)
                if (!(*it)->HandleCreated())
                    (*it)->CreateHandle();
            return;
        }

        controlstate << csCreatingHandle;

        // Create the window handle and set its parameters by handle in derived classes.
        InitHandle();

        controlstate -= csCreatingHandle;
        controlstate -= csRecreating;

        // Move window to its rightful position in the z-order. (Recreated windows are usually placed at the bottom.)
        if (Parent())
        {
            Parent()->controls->FixChildPosition(this);

            if (controlstyle.contains(csChild) && align != alNone && align != alAnchor)
                Parent()->RequestLayout(false, true);
        }

        SendMessage(handle, wmHandleCreated, 0, 0);

#ifdef DESIGNING
        if (Designing())
        {
            DesignForm *df = dynamic_cast<DesignForm*>(ParentForm());
            if (df)
                PostMessage(df->Handle(), amHijackChild, (WPARAM)this, 0);
        }
#endif
    }

    std::wstring Control::RegisteredClassName()
    {
        return ANSIToWide(typeid(*this).name());
    }

    WNDPROC Control::GetBuiltinWndProc()
    {
        WNDPROC classwndproc = NULL;
        std::wstring classname = RegisteredClassName();
        application->getclass(classname, classname, classwndproc);
        return classwndproc;
    }

    void Control::InitHandle()
    {
#ifdef DESIGNING
        if (!controls && Serializer())
        {
            Serializer()->HideProperty(L"AlignmentOrder");
            Serializer()->HideProperty(L"Padding");
        }
#endif

        std::wstring realclassname = RegisteredClassName();

        WNDPROC classwndproc = NULL;
        std::wstring classname;
        application->getclass(realclassname, classname, classwndproc);

        if (!classname.size())
        {
            ClassParams params;
            CreateClassParams(params);

            if (params.classname.size())
                classname = params.classname;
            else
            {
                params.classname = realclassname;
                classname = realclassname;
            }

            RegisterWindowClass(params);
            application->registerclass(realclassname, params.classname, params.wndproc);
        }

        WindowParams params;
        CreateWindowParams(params);

        text = std::wstring();
        rect = RectS(params.x, params.y, params.width, params.height);
        if (params.style.contains(wsVisible))
        {
            if (!controlstate.contains(csVisible))
                controlstate << csVisible;
        }
        else
            controlstate -= csVisible;

        if (params.style.contains(wsChild))
            controlstyle << csChild;
        else
            controlstyle -= csChild;

        if (params.extstyle.contains(wsExTransparent))
            controlstyle << csTransparent;
        else
            controlstyle -= csTransparent;

        if (params.style.contains(wsChild) && params.style.contains(wsTabStop))
            controlstyle << csAcceptInput;
#ifndef DESIGNING
        else
            controlstyle -= csAcceptInput;
#endif

        //if (!params.style.contains(wsChild))
        //{
        //    controlstyle -= csParentBackground;
        //    controlstyle -= csParentColor;
        //    controlstyle -= csParentFont;
        //    controlstyle -= csParentTooltip;
        //}

        _creation_window = this;
        if ((handle = CreateWindowHandle(classname, params, this)) == NULL)
        {
            //LONG error = GetLastError();
            throw L"Error creating window!";
        }
        _creation_window = NULL;

        application->addcontrolwithhandle(this, handle);
        dlgid = GetWindowLongPtr(handle, GWLP_ID);
        if (dlgid == 0)
        {
            SetWindowLongPtr(handle, GWLP_ID, (LONG)handle);
            dlgid = (int)handle;
        }

        SendMessage(handle, WM_SETFONT, (WPARAM)font->Handle(), (LPARAM)(controlstyle.contains(csUpdateOnTextChange) && IsVisible() ? TRUE : FALSE));

        clientrect = ClientRect();
        if (controls)
            controls->InitHandle();

        if (ShowTooltip() && !tooltext.empty())
            application->RegisterControlTooltip(this);

#ifdef DESIGNING
        if (Designing())
            SendMessage(handle, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);
#endif
    }

    void Control::InitControlList()
    {
        if (controls)
            return;
        controls = new ControlList(this);
        if (HandleCreated())
            RecreateHandle();
    }

    bool Control::IsControlParent()
    {
        return controls != NULL;
    }

    LRESULT Control::CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProc(handle, uMsg, wParam, lParam);
    }

    bool Control::HandleOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &result)
    {
        result = 0;
        if (!OnMessage)
            return true;
        bool allow = true;
        OnMessage(this, MessageParameters(result, uMsg, wParam, lParam, allow));
        return allow;
    }

    LRESULT Control::PassMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result;

        // Preventing background erasing and painting at the root of the message handling 
        // because we don't want derived classes to put their hands on these messages while 
        // a child control is being painted.
        if ((uMsg == WM_ERASEBKGND || uMsg == WM_PRINTCLIENT) && controlstate.contains(csChildPainting))
        {
            if (erasechild)
            {
                ReleaseCanvas();
                int i = SaveDC((HDC)wParam);
                UpdateCanvas((HDC)wParam);
                Canvas *c = GetCanvas();
                c->SetBrush(erasechild->GetColor());
                c->FillRect(ClientRect());
                ReleaseCanvas();
                RestoreDC((HDC)wParam, i);
            }
            else if (uMsg == WM_ERASEBKGND)
            {
                if (!HandleOnMessage(uMsg, wParam, lParam, result))
                    return result;
                WindowProc(uMsg, wParam, lParam);
                if (!HandleOnMessage(WM_PRINTCLIENT, wParam, PRF_CLIENT, result))
                    return result;
                WindowProc(WM_PRINTCLIENT, wParam, PRF_CLIENT);
            }
            return 1;
        }

        if (!HandleOnMessage(uMsg, wParam, lParam, result))
            return result;

        return WindowProc(uMsg, wParam, lParam);
    }

    LRESULT Control::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Control *ccontrol;
        Form *form;
        WORD hittest;
        WORD keycode;
        WCHAR key;
        PAINTSTRUCT ps;
        WINDOWPOS *pos;
        MINMAXINFO *mm;
        Rect r;
        Rect r2;
        Rect r3;
        Point p;
        bool finished;
        LRESULT result;
        UINT vkeys;
        HDC dc;
        HRGN rgn;
        short sx, sy;
        bool b;
        bool vert;
        short delta;
        int i;
        ::Gdiplus::Color gcl;
        BasicDataObject *dragdataobj;
        DragDropEffectSet dragallowedeffects;
        DragDropEffects dragresult;
        Bitmap *bmp;
        Color col;
        DialogCodeSet dialogcodes;
        Canvas *c;
        Size s;
        LPNMTTDISPINFO toolinf;
        MEASUREITEMSTRUCT *mi;
        DRAWITEMSTRUCT *ds;
        //UINT ui;
        //void *vdata;
        NMHDR notify;
        HRESULT hr;
        HWND hwnd;

        if (controls && controls->HandleMessage(uMsg, wParam, lParam, result))
            return result;

        switch (uMsg)
        {
        case WM_DESTROY:
            PostMessage(application->Handle(), wmControlDestroyed, (WPARAM)this, (LPARAM)Handle());

            if (canvas)
                canvas->Release();

            if (controls && controlstate.contains(csRecreating))
                controls->ChildrenRecreate();
            if (!controlstate.contains(csDestroying) && (!controlstyle.contains(csChild) || (parent && parent->HandleCreated())))
            {
                SaveWindow();
                if (controls)
                    controls->SaveChildren();
            }

            if (controlstyle.contains(csChild) && parent && align != alNone && align != alAnchor)
                parent->RequestLayout(false, true);

            controlstate -= csFocused;

            if (ShowTooltip() && !tooltext.empty())
                application->DeregisterControlTooltip(this);

            handle = NULL;
            break;
        case WM_SETFONT:
            wParam = (WPARAM)font->Handle();
            if (!FontChanging() && controlstyle.contains(csUpdateOnTextChange))
                Invalidate(controlstyle.contains(csEraseOnTextChange));
            break;
        case WM_NOTIFY:
            if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
            {
                toolinf = (LPNMTTDISPINFO)lParam;
                if ((toolinf->uFlags & TTF_IDISHWND) != TTF_IDISHWND)
                    break;
                toolinf->lpszText = const_cast<wchar_t*>(tooltext.c_str());
            }
            else if (!controls)
            {
                notify = *(NMHDR*)lParam;
                hr = 0;
                if (HandleChildNotify(notify.hwndFrom, lParam, hr))
                    return hr;
            }
            break;
        case WM_COMMAND:
            if (controls)
                break;
            if (lParam && HandleChildCommand((HWND)lParam, wParam))
                return 0;
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
        case WM_SHOWWINDOW:
            if (lParam != 0)
                break;
            if (wParam != FALSE)
            {
                controlstate << csVisible;
                form = ParentForm();

                if (controls)
                    RequestLayout(false, false);

                if (IsVisible())
                    Showing();

                if (!form || form->ActiveControl() != NULL)
                    break;

                ccontrol = (!controlstyle.contains(csInTabOrder) || !AcceptInput()) ? TabFirst() : this;
                if (ccontrol)
                    form->SetActiveControl(ccontrol);
            }
            else
            {
                controlstate -= csFocused;
                if (!controlstate.contains(csDestroyingHandle) && !controlstate.contains(csRecreating))
                    controlstate -= csVisible;
                Hiding();

                form = ParentForm();
                if (!form || (ccontrol = form->ActiveControl()) == NULL)
                    break;

                if (this == ccontrol || ParentOf(ccontrol))
                {
                    ccontrol->controlstate -= csFocused;
                    form->SetActiveControl(NULL);
                }
            }
            break;
        case WM_WINDOWPOSCHANGING:
            if (!minsize.cx && !minsize.cy && !maxsize.cx && !maxsize.cy && !sizewidthdiff && !sizemodx && !sizeheightdiff && !sizemody)
                break;
            pos = (WINDOWPOS*)lParam;

            if ((pos->flags & SWP_NOSIZE) == SWP_NOSIZE)
                break;

            r = WindowRect();
            r2 = r;
            r3 = ClientRect();

            r2.right = r2.left + pos->cx;
            r2.bottom = r2.top + pos->cy;

            if ((pos->flags & SWP_NOMOVE) != SWP_NOMOVE)
                OffsetRect(&r2, -r.left + pos->x, -r.top + pos->y);
            ConstraintSizing( (r2.left != r.left ? WMSZ_LEFT : r2.right != r.right ? WMSZ_RIGHT : 0) + (r2.top != r.top ? WMSZ_TOP : r2.bottom != r.bottom ? WMSZ_BOTTOM : 0), r2,
                              minsize.cx, minsize.cy,
                              maxsize.cx == 0 ? 999999 : maxsize.cx, maxsize.cy == 0 ? 999999 : maxsize.cy,
                              sizewidthdiff + r.Width() - r3.Width(), sizemodx,
                              sizeheightdiff + r.Height() - r3.Height(), sizemody
                              );
            pos->x = r2.left;
            pos->y = r2.top;
            pos->cx = r2.Width();
            pos->cy = r2.Height();

            break;
        case WM_WINDOWPOSCHANGED:
            clientrect = ClientRect(); // Update rectangle returned by OldClientRectangle().

            pos = (WINDOWPOS*)lParam;

            r = rect;
            if ((pos->flags & SWP_NOSIZE) != SWP_NOSIZE)
                rect = RectS(rect.left, rect.top, pos->cx, pos->cy);
            if ((pos->flags & SWP_NOMOVE) != SWP_NOMOVE)
                rect = RectS(pos->x, pos->y, rect.Width(), rect.Height());

            if (r != rect)
            {
                if ((!(pos->flags & SWP_NOSIZE) || !(pos->flags & SWP_NOMOVE)) && align != alNone && align != alAnchor && Parent())
                    Parent()->RequestLayout(true, true);
                WindowBoundsChanged(r, rect);
                Changed(CHANGE_MOVED);
            }

            if (!controlstate.contains(csRecreating) && ((pos->flags & SWP_SHOWWINDOW) == SWP_SHOWWINDOW || (pos->flags & SWP_HIDEWINDOW) == SWP_HIDEWINDOW))
            {
                if ((pos->flags & SWP_SHOWWINDOW) == SWP_SHOWWINDOW)
                    controlstate << csVisible;
                else if (!controlstate.contains(csDestroyingHandle))
                    controlstate -= csVisible;

                if (Parent() && align != alNone && align != alAnchor)
                    Parent()->RequestLayout(true, true);

                //if ((pos->flags & SWP_SHOWWINDOW) == SWP_SHOWWINDOW)
                //    Showing();
                //else
                //    Hiding();
            }

#ifdef DESIGNING
            if (r != rect && Designing() && designer && designer->IsPropertyOwner(this))
            {
                designer->InvalidateRow(this, L"Top");
                designer->InvalidateRow(this, L"Left");
                designer->InvalidateRow(this, L"Width");
                designer->InvalidateRow(this, L"Height");
                designer->InvalidateRow(this, L"ClientWidth");
                designer->InvalidateRow(this, L"ClientHeight");
            }
#endif

            if (Parent())
            {
                Point dif = Point(rect.left - r.left, rect.top - r.top);

                r = WindowToClient(Rect(r.left - rect.left, r.top - rect.top, r.right - rect.left, r.bottom - rect.top));
                Parent()->controls->InvalidateTransparentAbove(this, r);
                r = WindowToClient(Rect(0, 0, rect.Width(), rect.Height()));
                Parent()->controls->InvalidateTransparentAbove(this, r);

                if ((pos->flags & SWP_NOMOVE) != SWP_NOMOVE)
                    Parent()->controls->InvalidateBelowTransparentAbove(this, r, dif);
            }
            break;
        case WM_GETMINMAXINFO:
            if (!minsize.cx && !minsize.cy && !maxsize.cx && !maxsize.cy && !sizewidthdiff && !sizemodx && !sizeheightdiff && !sizemody)
                break;
            result = CallDefaultProc(uMsg, wParam, lParam);
            mm = (MINMAXINFO*)lParam;

            if (maxsize.cx > 0) {
                mm->ptMaxSize.x = min(mm->ptMaxSize.x,maxsize.cx);
                mm->ptMaxTrackSize.x = min(mm->ptMaxTrackSize.x,maxsize.cx);
            }

            if (maxsize.cy > 0) {
                mm->ptMaxSize.y = min(mm->ptMaxSize.y,maxsize.cy);
                mm->ptMaxTrackSize.y = min(mm->ptMaxTrackSize.y,maxsize.cy);
            }

            if (minsize.cx > 0) {
                mm->ptMaxSize.x = max(mm->ptMaxSize.x,minsize.cx);
                mm->ptMaxTrackSize.x = max(mm->ptMaxTrackSize.x,minsize.cx);
                mm->ptMinTrackSize.x = minsize.cx;
            }

            if (minsize.cy > 0) {
                mm->ptMaxSize.y = max(mm->ptMaxSize.y,minsize.cy);
                mm->ptMaxTrackSize.y = max(mm->ptMaxTrackSize.y,minsize.cy);
                mm->ptMinTrackSize.y = minsize.cy;
            }

            return result;
        case WM_ENTERSIZEMOVE:
            if (usersizemove)
                break;
            usersizemove = true;
            PostMessage(handle, wmStartSizeMove, 0, 0);
            break;
        case wmStartSizeMove:
            PositionSizeStarted();
            break;
        case WM_EXITSIZEMOVE:
            if (!usersizemove)
                break;
            usersizemove = false;
            r = WindowRect();
            PostMessage(handle, wmSizeMoveEnded, 0, (LPARAM)&r);
            break;
        case wmSizeMoveEnded:
            PositionSizeChanged(*(Rect*)lParam);
            break;
        case WM_CAPTURECHANGED:
            if (HandleCreated())
            {
                // To make sure we get mouseup before capturechanged, post it again to ourselves.
                PostMessage(Handle(), wmCaptureChanged, wParam, lParam);
                break;
            }
        case wmCaptureChanged:
            if (GetCapture() == Handle())
                return 0;

            if (usersizemove)
            {
                usersizemove = false;
                PositionSizeChanged(WindowRect());
            }

            CaptureChanged();

            if (OnCaptureLost)
                OnCaptureLost(this,EventParameters());
            break;
        case WM_ERASEBKGND:
            if (controlstyle.contains(csNoErase))
                return 1;
            if (controlstyle.contains(csSystemErased) || controlstate.contains(csDestroying) || (!DoubleBuffered() && controlstate.contains(csPainting)))
                break;

            ReleaseCanvas();

            if (GetClipBox((HDC)wParam, &r) == NULLREGION)
                return 1;
            r = r.Intersect(ClientRect());

            i = SaveDC((HDC)wParam);

            UpdateCanvas((HDC)wParam);

            try
            {
                HandleErase(r);
            }
            catch(...)
            {
                ReleaseCanvas();
                RestoreDC((HDC)wParam, i);
                throw;
            }

            ReleaseCanvas();

            RestoreDC((HDC)wParam, i);

            return 1;
        case WM_NCPAINT:
            if (controlstate.contains(csDestroying) || controlstate.contains(csPainting) || (!controlstyle.contains(csSelfDrawn) && (border != bsModern || !themes->AppThemed())) )
                break;

            //controlstate << csPainting;

            ReleaseCanvas();

            dc = GetWindowDC(Handle());
            if (dc == NULL)
                break;
            try
            {
                UpdateCanvas(dc);
                c = GetCanvas();
                finished = false;

                controlstate << csPainting;

                if (controlstyle.contains(csSelfDrawn))
                {
                    auto cstate = c->SaveState();
                    r = ClientToWindow(ClientRect());
                    c->ExcludeClip(r);
                    finished = NCPaint();
                    c->RestoreState(cstate);
                }

                if (!finished && border == bsModern && themes->AppThemed())
                {
                    ReleaseCanvas();
                    ReleaseDC(Handle(), dc);

                    //s = themes->MeasureEditBorderWidth();
                    r = WindowRect();
                    r.Move(-r.left, -r.top);

                    //rgn = 0;
                    //if (wParam && wParam != 1)
                    //{
                    //    CreateCombinedRgn(rgn, r.Inflate(-s.cx, -s.cy), ClientToWindow(ClientRect()), rcmDiff);
                    //    rgn2 = (HRGN)wParam;
                    //    CombineRgn(rgn, rgn, rgn2, RGN_OR);
                    //}
                    //else
                    //    CreateCombinedRgn(rgn, r, r.Inflate(-s.cx, -s.cy), rcmDiff);
                    //if (rgn)
                    //    wParam = (WPARAM)rgn;
                    CallDefaultProc(uMsg, wParam, lParam);
                    //if (rgn)
                    //    DeleteObject(rgn);

                    dc = GetWindowDC(Handle());
                    UpdateCanvas(dc);
                    r = WindowRect();
                    r.Move(-r.left, -r.top);
                    themes->DrawControlBorder(c, r);
                    ReleaseCanvas();
                    finished = true;
                }
            }
            catch(...)
            {
                ReleaseCanvas();
                ReleaseDC(Handle(), dc);
                paintdc = NULL;
                controlstate -= csPainting;
                throw;
            }

            ReleaseCanvas();
            ReleaseDC(Handle(), dc);

            controlstate -= csPainting;

            if (finished)
                return 0;
            break;
        case WM_PRINTCLIENT:
        case WM_PAINT:
            if (controlstate.contains(csDestroying) || controlstate.contains(csPainting) || (!controlstyle.contains(csSelfDrawn) && !OnPaint) || (uMsg == WM_PRINTCLIENT && (lParam & PRF_CHECKVISIBLE) == PRF_CHECKVISIBLE && !IsVisible()))
            {
                if (controlstyle.contains(csNoDefaultPaint))
                    return 1;

                if (controlstate.contains(csDestroying) || controlstate.contains(csPainting) || (uMsg == WM_PRINTCLIENT && ((lParam & PRF_CHECKVISIBLE) == PRF_CHECKVISIBLE && !IsVisible())) || !Parent())
                    break;

                controlstate << csPainting;
                if (UsingParentBackground())
                    Parent()->ChildPaintToggle(true);
                else if (controlstyle.contains(csForceBgColor) && !UsingParentBackground())
                    Parent()->ChildEraseToggle(this);
                LRESULT lr = CallDefaultProc(WM_PAINT, wParam, lParam);
                if (UsingParentBackground())
                    Parent()->ChildPaintToggle(false);
                else if (controlstyle.contains(csForceBgColor) && !UsingParentBackground())
                    Parent()->ChildEraseToggle(NULL);
                controlstate -= csPainting;

                return lr;
            }

            if ((uMsg == WM_PAINT && GetUpdateRect(handle, &r, FALSE)) || (uMsg == WM_PRINTCLIENT && GetClipBox((HDC)wParam, &r) != ERROR))
            {
                ReleaseCanvas();

                r = r.Intersect(ClientRect());

                if (uMsg == WM_PRINTCLIENT)
                {
                    dc = (HDC)wParam;
                    if (!dc)
                        break;

                    if ((lParam & PRF_ERASEBKGND) == PRF_ERASEBKGND)
                    {
                        i = SaveDC(dc);
                        CallDefaultProc(WM_ERASEBKGND, wParam, 0);
                        RestoreDC(dc, i);
                    }
                }
                else
                {
                    dc = BeginPaint(handle, &ps);
                    if (!Rect(ps.rcPaint).Empty())
                    {
                        if (r.Empty())
                            r = ps.rcPaint;
                        else
                            r = r.Intersect(ps.rcPaint);
                    }
                }
                if (!dc)
                    return 0;

                if (r.Empty())
                {
                    if (uMsg == WM_PAINT)
                        EndPaint(handle, &ps);
                    break;
                }

                HDC origdc = dc;

                bool localbufcnt = false;

                if (uMsg == WM_PAINT && bufcnt == 0 && DoubleBuffered() && !r.Empty())
                {
                    if (application->DwmComposition())
                    {
                        if (paintbuffer.dblbuf == NULL)
                        {
                            paintbuffer.dblbuf = BeginBufferedPaint_REDEFINED(dc, &r, BPBF_TOPDOWNDIB, NULL, &dc);
                            if (paintbuffer.dblbuf == NULL || dc == NULL)
                                dc = origdc;
                            else
                            {
                                bufcnt = -1;
                                localbufcnt = true;
                                vistabuf = true;
                            }
                        }
                    }
                    else
                    {
                        if (paintbuffer.dblbmp == NULL || doublebufrect.Width() < r.Width() || doublebufrect.Height() < r.Height())
                        {
                            if (paintbuffer.dblbmp != NULL)
                                DeleteObject(paintbuffer.dblbmp);
                            else
                                doublebufrect = Rect();
                            paintbuffer.dblbmp = CreateCompatibleBitmap(dc, max(doublebufrect.Width(), r.Width()), max(doublebufrect.Height(), r.Height()));
                            if (paintbuffer.dblbmp == NULL)
                                doublebufrect = Rect();
                        }

                        if (paintbuffer.dblbmp != NULL)
                        {
                            doublebufrect = RectS(r.left, r.top, max(doublebufrect.Width(), r.Width()), max(doublebufrect.Height(), r.Height()));
                            dc = CreateCompatibleDC(dc);
                            if (!dc)
                                dc = origdc;
                            else
                            {
                                olddblbmp = (HBITMAP)SelectObject(dc, paintbuffer.dblbmp);
                                SetWindowOrgEx(dc, r.left, r.top, NULL);
                                bufcnt = -1;
                                localbufcnt = true;
                                vistabuf = false;
                            }
                        }

                    }
                }

                i = SaveDC(dc);

                try
                {
                    controlstate << csPainting;

                    if (bufcnt == -1)
                        PassMessage(WM_ERASEBKGND, (WPARAM)dc, (LPARAM)dc);

                    if (!controlstyle.contains(csNoDefaultPaint))
                    {
                        if (Parent())
                        {
                            if (UsingParentBackground())
                                Parent()->ChildPaintToggle(true);
                            else if (controlstyle.contains(csForceBgColor) && !UsingParentBackground())
                                Parent()->ChildEraseToggle(this);
                        }
                        result = CallDefaultProc(WM_PAINT, (WPARAM)dc, lParam);
                        if (Parent())
                        {
                            if (UsingParentBackground())
                                Parent()->ChildPaintToggle(false);
                            else if (controlstyle.contains(csForceBgColor) && !UsingParentBackground())
                                Parent()->ChildEraseToggle(NULL);
                        }
                        RestoreDC(dc, i);
                        i = SaveDC(dc);
                    }
                    else
                        result = 0;

                    UpdateCanvas(dc);

                    ControlCanvas *canvas = GetCanvas();
                    rgn = 0;
                    HandlePaint(r, rgn);
                    if (rgn)
                        canvas->SetClip(rgn);
                    else if (uMsg == WM_PAINT)
                        canvas->SetClip(ps.rcPaint);

                    if (!canvas->ClipEmpty() || r.Width() > 0 || r.Height() > 0)
                    {
                        CanvasGraphicsState *gs = NULL;
                        if (controlstyle.contains(csSelfDrawn) || OnPaint)
                            gs = new CanvasGraphicsState(canvas->SaveState());

                        if (controlstyle.contains(csSelfDrawn))
                        {
                            Paint(r);
                            if (OnPaint && gs)
                            {
                                canvas->RestoreState(*gs);
                                delete gs;
                                gs = new CanvasGraphicsState(canvas->SaveState());
                            }
                        }
                        if (OnPaint)
                            OnPaint(this, PaintParameters(canvas, r));

                        if (gs)
                        {
                            canvas->RestoreState(*gs);
                            delete gs;
                        }
                    }
                    if (rgn)
                    {
                        canvas->ResetClip();
                        DeleteObject(rgn);
                    }

                    ReleaseCanvas();
                    if (localbufcnt)
                    {
                        if (vistabuf && paintbuffer.dblbuf != NULL)
                        {
                            EndBufferedPaint_REDEFINED(paintbuffer.dblbuf, TRUE);
                            paintbuffer.dblbuf = 0;
                        }
                        else if (!vistabuf && paintbuffer.dblbmp != NULL)
                        {
                            SetWindowOrgEx(dc, 0, 0, NULL);
                            BitBlt(origdc, r.left, r.top, r.Width(), r.Height(), dc, 0, 0, SRCCOPY);
                            SelectObject(dc, olddblbmp);
                            DeleteDC(dc);
                            dc = origdc;
                        }
                        bufcnt = 0;
                        localbufcnt = false;
                    }
                    else
                        RestoreDC(dc, i);

                    if (uMsg == WM_PAINT)
                        EndPaint(handle, &ps);
                }
                catch(...)
                {
                    ReleaseCanvas();
                    if (localbufcnt)
                    {
                        if (vistabuf && paintbuffer.dblbuf != NULL)
                        {
                            EndBufferedPaint_REDEFINED(paintbuffer.dblbuf, FALSE);
                            paintbuffer.dblbuf = 0;
                        }
                        else if (!vistabuf && paintbuffer.dblbmp != NULL)
                        {
                            SetWindowOrgEx(dc, 0, 0, NULL);
                            SelectObject(dc, olddblbmp);
                            DeleteDC(dc);
                            dc = origdc;
                        }
                        bufcnt = 0;
                        localbufcnt = false;
                    }
                    else
                        RestoreDC(dc, i);

                    if (uMsg == WM_PAINT)
                        EndPaint(handle, &ps);
                    controlstate -= csPainting;
                    throw;
                }
                ReleaseCanvas();
                controlstate -= csPainting;
            }
            return uMsg == WM_PAINT ? 0 : 1;
        case WM_DWMCOMPOSITIONCHANGED:
            if (BeginBufferedPaint_REDEFINED)
            {
                if (!vistabuf && paintbuffer.dblbmp != NULL)
                    DeleteObject(paintbuffer.dblbmp);
                else if (vistabuf && paintbuffer.dblbuf != NULL)
                {
                    EndBufferedPaint_REDEFINED(paintbuffer.dblbuf, TRUE);
                    paintbuffer.dblbuf = 0;
                }

                paintbuffer.dblbmp = NULL;
                doublebufrect = Rect();
                bufcnt = 0;
            }
            break;
        case WM_MOUSEMOVE:
            b = screencursor->HoveredControlByArea(true) == this;
            if (screencursor->MouseMovedOn(this, false))
            {
                MouseEnter();
                if (!b)
                    MouseEntered();
                if (OnMouseEnter)
                    OnMouseEnter(this,EventParameters());
                if (!b && OnMouseEntered)
                    OnMouseEntered(this,EventParameters());
            }

            vkeys = VirtualKeysFromWParam(wParam);
            sx = GET_X_LPARAM(lParam);
            sy = GET_Y_LPARAM(lParam);

#ifdef DESIGNING
            if (Designing())
            {
                if (DesignMouseMove(dynamic_cast<DesignForm*>(ParentForm()), sx, sy, vkeys) == true)
                    return 0;
            }
#endif
            MouseMove(sx, sy, vkeys);

            break;
        case WM_NCMOUSEMOVE:
            b = screencursor->HoveredControlByArea(false) == this;
            if (screencursor->MouseMovedOn(this, true))
            {
                NCMouseEnter();
                if (OnNCMouseEnter)
                    OnNCMouseEnter(this,EventParameters());
                if (!b)
                    MouseEntered();
                if (!b && OnMouseEntered)
                    OnMouseEntered(this,EventParameters());
            }

            vkeys = PressedVirtualKeys();
            p = ScreenToWindow(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

#ifdef DESIGNING
            if (Designing())
            {
                if (DesignNCMouseMove(dynamic_cast<DesignForm*>(ParentForm()), p.x, p.y, wParam, vkeys) == true)
                    return 0;
            }
#endif
            NCMouseMove(p.x, p.y, wParam, vkeys);
            break;
        case WM_MOUSELEAVE:
            GetCursorPos(&p);
            b = WindowFromPoint(p) == Handle();
            if (!b)
                screencursor->MouseLeftFrom(this);

            MouseLeave();
            if (!b)
                MouseLeft();

            if (OnMouseLeave)
                OnMouseLeave(this, EventParameters());
            if (!b && OnMouseLeft)
                OnMouseLeft(this, EventParameters());
            break;
        case WM_NCMOUSELEAVE:
            GetCursorPos(&p);
            b = WindowFromPoint(p) == Handle();
            if (!b)
                screencursor->MouseLeftFrom(this);

            NCMouseLeave();
            if (!b)
                MouseLeft();

            if (OnNCMouseLeave)
                OnNCMouseLeave(this, EventParameters());
            if (!b && OnMouseLeft)
                OnMouseLeft(this, EventParameters());
            break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
            if (controlstyle.contains(csInTabOrder) && AcceptInput())
                Focus();
            if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) && controlstyle.contains(csMouseCapture))
                SetCapture(handle);
            vkeys = VirtualKeysFromWParam(wParam);
            if (uMsg == WM_LBUTTONDBLCLK || uMsg == WM_RBUTTONDBLCLK  || uMsg == WM_MBUTTONDBLCLK)
                vkeys |= vksDouble;
            sx = GET_X_LPARAM(lParam);
            sy = GET_Y_LPARAM(lParam);

#ifdef DESIGNING
            if (Designing())
            {
                if (DesignMouseDown(dynamic_cast<DesignForm*>(ParentForm()), sx, sy, ButtonFromMsg(uMsg), vkeys) == true)
                    return 0;
            }
#endif
            MouseDown(sx, sy, ButtonFromMsg(uMsg), vkeys);
            break;
        case WM_NCLBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDBLCLK:
            vkeys = PressedVirtualKeys();
            if (uMsg == WM_NCLBUTTONDBLCLK || uMsg == WM_NCRBUTTONDBLCLK  || uMsg == WM_NCMBUTTONDBLCLK)
                vkeys |= vksDouble;
            p = ScreenToWindow(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

#ifdef DESIGNING
            if (Designing())
            {
                if (DesignNCMouseDown(dynamic_cast<DesignForm*>(ParentForm()), p.x, p.y, NCButtonFromMsg(uMsg), wParam, vkeys) == true)
                    return 0;
            }
#endif
            NCMouseDown(p.x, p.y, NCButtonFromMsg(uMsg), wParam, vkeys);
            break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            if (uMsg == WM_LBUTTONUP && controlstyle.contains(csMouseCapture))
            {
                if (GetCapture() == handle)
                    ReleaseCapture();
            }

            vkeys = VirtualKeysFromWParam(wParam);
            sx = GET_X_LPARAM(lParam);
            sy = GET_Y_LPARAM(lParam);

#ifdef DESIGNING
            if (Designing())
            {
                if (DesignMouseUp(dynamic_cast<DesignForm*>(ParentForm()), sx, sy, ButtonFromMsg(uMsg), vkeys) == true)
                    return 0;
            }
#endif

            MouseUp(sx, sy, ButtonFromMsg(uMsg), vkeys);
            if (uMsg == WM_LBUTTONUP && ClientRect().Contains(sx, sy))
                MouseClick();
            break;
        case WM_CONTEXTMENU:

            if (/* !GetCapture() &&*/ popupmenu && popupmenu->Show(this, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
                return 0;
            break;
        case WM_NCLBUTTONUP:
        case WM_NCRBUTTONUP:
        case WM_NCMBUTTONUP:
            vkeys = PressedVirtualKeys();
            //sx = GET_X_LPARAM(lParam);
            //sy = GET_Y_LPARAM(lParam);
            p = ScreenToWindow(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
#ifdef DESIGNING
            if (Designing())
            {
                if (DesignNCMouseUp(dynamic_cast<DesignForm*>(ParentForm()), p.x, p.y, NCButtonFromMsg(uMsg), wParam, vkeys) == true)
                    return 0;
            }
#endif
            NCMouseUp(p.x, p.y, NCButtonFromMsg(uMsg), wParam, vkeys);
            break;
        case WM_SETCURSOR:
            if ((handle && (HWND)wParam != handle) || cursor == cDefault)
                break;

            hittest = lParam & 0xffff;
            if (lParam == hittest || (hittest != HTCLIENT && GetCapture() != handle))
                break;

            if (GetCapture() == handle && hittest != HTCLIENT)
                throw L"!";

#ifdef DESIGNING
            if (Designing())
                break;
#endif
            screencursor->Set(cursor);
            return true;
        case WM_GETDLGCODE:
            result = CallDefaultProc(uMsg, 0, 0); //wParam, lParam);

            ProcGetDialogCode(lParam != 0 ? ((MSG*)lParam)->wParam : wParam, result);
            return result;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            keycode = wParam;
            ProcKeyDown(keycode);
            wParam = keycode;
            if (wParam == 0)
                return 0;

            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            keycode = wParam;
            ProcKeyUp(keycode);
            wParam = keycode;
            if (wParam == 0)
                return 0;

            break;
        case WM_SYSCHAR:
        case WM_CHAR:
            key = (WCHAR)wParam;
            ProcChar(uMsg, key);
            wParam = (WPARAM)key;
            if (key == 0)
                return 0;
            break;
        case wmChildMessage:
            MessageStruct *msg;
            msg = (MessageStruct*)lParam;
            result = 0;

            HandleChildMessage(msg->hwnd, msg->uMsg, msg->wParam, msg->lParam, result);

            return result;
        case WM_SETFOCUS:
            controlstate << csFocused;

            if (controlstyle.contains(csChild) && ParentForm())
                ParentForm()->SetActiveControl(this);

            GainFocus((HWND)wParam);

            controlstate -= csTabFocusing;
            break;
        case WM_KILLFOCUS:
            controlstate -= csFocused;
            PassMessage(wmFocusKilled, wParam, 0);
            break;
        case wmFocusKilled:
            LoseFocus((HWND)wParam);
            return 0;
        case wmActiveChanged:
            if (wParam == 1)
                ActiveEnter((Control*)lParam);
            else
                ActiveLeave((Control*)lParam);
            return 0;
        case WM_SETTEXT:
            if (!TextChanging((wchar_t*)lParam) && controlstyle.contains(csUpdateOnTextChange))
                Invalidate(controlstyle.contains(csEraseOnTextChange));
            break;
        case wmColorChanged:
            if (bgbrush && bgbrush->GetColor() != GetColor().ToRGB())
            {
                delete bgbrush;
                bgbrush = NULL;
            }

            Invalidate(!controlstyle.contains(csNoErase) &&
                       ((lParam == 1 && (UsingParentColor() || UsingParentBackground() || controlstyle.contains(csTransparent))) ||
                       (lParam == 0 && controlstyle.contains(csEraseToColor) && !UsingParentBackground())));
            if (controls)
                controls->PassMessage(wmColorChanged, 0, 1);
            break;
        case wmEnableChanged:
            if (!HandleCreated())
                break;

            RedrawWindow(Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            EnableWindow(Handle(), controlstate.contains(csEnabled));
            InvalidateNC();
            break;
        case wmDelayedDrag:
            dragdataobj = (BasicDataObject*)wParam;
            dragallowedeffects = lParam;
            if (OnDragImageRequest)
            {
                bmp = NULL;
                b = false;
                OnDragImageRequest(this, DragImageRequestParameters(p, bmp, b));
                if (bmp)
                    dragresult = BeginDragDropOperation(dragdataobj, dragallowedeffects, bmp, p);
                else
                    dragresult = BeginDragDropOperation(dragdataobj, dragallowedeffects);
                if (b)
                    delete bmp;
            }
            else
                dragresult = BeginDragDropOperation(dragdataobj, dragallowedeffects);

            if (OnDragDropEnded)
                OnDragDropEnded(this, DragDropEndedParameters(dragresult));

            dragdataobj->Release();

            break;
        case wmDialogKey:
            if (lParam && (Control*)lParam == this)
                return false;
            else
                return HandleDialogKey(wParam);
        case wmSetBounds:
            SetBounds(RectS( (short)(WORD)(((int)wParam) & 0xffff), (short)(WORD)(((int)wParam >> 16) & 0xffff), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
            return 0;
        case wmDelete:
            Destroy();
            return 0;
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            vkeys = VirtualKeysFromWParam(wParam & 0xffff);
            b = controlstate.contains(csWheelScrolling);

            vert = uMsg == WM_MOUSEWHEEL;
            delta = HIWORD(wParam);

            // Controls usually forward the message to their parents if they don't handle it. Make sure we don't
            // forward the message to the control under the mouse in an infinite loop.
            if (!b)
            {
                controlstate << csWheelScrolling;
                if (WantMouseWheel(uMsg == WM_MOUSEWHEEL, HIWORD(wParam), vkeys, LOWORD(lParam), HIWORD(lParam)))
                    b = true;
                else
                {
                    hwnd = WindowFromPoint(Point(LOWORD(lParam), HIWORD(lParam)));
                    if (hwnd != 0 && hwnd == handle)
                        b = true;
                    else if (hwnd != 0 && GetWindowThreadProcessId(hwnd, nullptr) == MainThreadId)
                    {
                        result = SendMessage(hwnd, uMsg, wParam, lParam);
                        controlstate -= csWheelScrolling;
                        return result;
                    }
                }
                controlstate -= csWheelScrolling;
            }
            
            if (b)
            {
                b = MouseWheel(vert, delta, vkeys, LOWORD(lParam), HIWORD(lParam));
                if (!b)
                {
                    if (parent && controlstyle.contains(csChild))
                        PostMessage(parent->Handle(), uMsg, wParam, lParam);
                }
                if (!b || delta == 0)
                    return uMsg == WM_MOUSEHWHEEL ? TRUE : 0;
                
            }

            if (uMsg == WM_MOUSEHWHEEL) // Workaround for buggy mouse drivers that expect a TRUE value in return of WM_MOUSEHWHEEL. (But not for WM_MOUSEWHEEL.)
            {
                CallDefaultProc(uMsg, wParam, lParam);
                return TRUE;
            }
        }

        return CallDefaultProc(uMsg, wParam, lParam);
    }

    void Control::ProcGetDialogCode(WORD keycode, LRESULT &result)
    {
        bool found = false;
        if (/*keycode == VK_TAB &&*/ wantedkeys.contains(wkTab))
        {
            found = keycode == VK_TAB;
            result |= DLGC_WANTTAB;
        }
        else
            result &= ~DLGC_WANTTAB;
        if (/*(keycode == VK_UP || keycode == VK_DOWN || keycode == VK_LEFT || keycode == VK_RIGHT) &&*/ wantedkeys.contains(wkArrows))
        {
            found = found || (keycode == VK_UP || keycode == VK_DOWN || keycode == VK_LEFT || keycode == VK_RIGHT);
            result |= DLGC_WANTARROWS;
        }
        else
            result &= ~DLGC_WANTARROWS;

        if (found || (wantedkeys.contains(wkEnter) && keycode == VK_RETURN) || (wantedkeys.contains(wkEscape) && keycode == VK_ESCAPE) || (keycode != VK_RETURN && keycode != VK_ESCAPE && keycode != VK_TAB && keycode != VK_UP && keycode != VK_DOWN && keycode != VK_LEFT && keycode != VK_RIGHT && wantedkeys.contains(wkOthers)))
        {
            found = true;
            result |= DLGC_WANTALLKEYS;
        }

        if (!found && keycode != 0 && (!wantedkeys.contains(wkOthers) || keycode == VK_TAB || keycode == VK_RETURN || keycode == VK_ESCAPE || keycode == VK_UP || keycode == VK_DOWN || keycode == VK_LEFT || keycode == VK_RIGHT))
            result &= ~DLGC_WANTALLKEYS;

        DialogCodeSet dialogcodes = result;
        NeedsDialogCode(keycode, dialogcodes);

        if (OnDialogCode)
            OnDialogCode(this, DialogCodeParameters(keycode, dialogcodes));
        result = dialogcodes;
    }

    void Control::ProcKeyDown(WORD &keycode)
    {
        VirtualKeyStateSet vkeys = PressedVirtualKeys();

        WCHAR dummy = 0;
        if (ParentForm() && ParentForm() != this && ((Form*)ParentForm())->KeyPreview())
            ParentForm()->KeyPush(keycode, dummy, vkeys);

        if (keycode != 0)
        {
            dummy = 0;
            KeyPush(keycode, dummy, vkeys);
        }
    }

    void Control::ProcKeyUp(WORD &keycode)
    {
        VirtualKeyStateSet vkeys = PressedVirtualKeys();

        if (ParentForm() && ParentForm() != this && ((Form*)ParentForm())->KeyPreview())
            ParentForm()->KeyUp(keycode, vkeys);

        if (keycode != 0)
            KeyUp(keycode, vkeys);
    }

    void Control::ProcChar(UINT uMsg, WCHAR &key)
    {
        VirtualKeyStateSet vkeys = PressedVirtualKeys();
        if (uMsg == WM_SYSCHAR)
        {
            if (/*((lParam >> 29) & 1)*/ vkeys.contains(vksAlt) && key != VK_MENU)
            {
                Control *c = ParentForm();
                if (c && key != 0 && c->SysKeyPressed(key))
                {
                    key = 0;
                    return;
                }
            }
        }

        WORD dummy = 0;

        if (ParentForm() && ParentForm() != this && ((Form*)ParentForm())->KeyPreview())
            ParentForm()->KeyPush(dummy, key, vkeys);

        if (key != 0)
        {
            dummy = 0;
            KeyPush(dummy, key, vkeys);
        }
    }

    void Control::HandleChildMessage(HWND hwnd, UINT &uMsg, WPARAM &wParam, LPARAM &lParam, LRESULT &result)
    {
        WORD keycode;
        WCHAR key;

        switch (uMsg)
        {
        case WM_SETFOCUS:
            controlstate << csFocused;

            if (controlstyle.contains(csChild) && ParentForm())
                ParentForm()->SetActiveControl(this);

            GainFocus((HWND)wParam);

            controlstate -= csTabFocusing;
            break;
        case WM_KILLFOCUS:
            controlstate -= csFocused;
            PassMessage(wmFocusKilled, wParam, 0);
            break;
        case WM_GETDLGCODE:
            ProcGetDialogCode(lParam != 0 ? ((MSG*)lParam)->wParam : wParam, result);
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            result = 0;
            keycode = wParam;
            ProcKeyDown(keycode);
            wParam = keycode;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            result = 0;
            keycode = wParam;
            ProcKeyUp(keycode);
            wParam = keycode;
            break;
        case WM_SYSCHAR:
        case WM_CHAR:
            result = 0;
            key = (WCHAR)wParam;
            ProcChar(uMsg, key);
            wParam = (WPARAM)key;
            break;
        };
    }

    bool Control::HandleChildCommand(HWND hwnd, WPARAM wParam)
    {
        return false;
    }

    bool Control::HandleChildNotify(HWND hwnd, LPARAM lParam, HRESULT &result)
    {
        return false;
    }

    bool Control::HandleSysKey(WCHAR key)
    {
        return false;
    }

    bool Control::SysKeyPressed(WCHAR key)
    {
#ifdef DESIGNING
        if (Designing())
            return false;
#endif

        if (!controlstate.contains(csEnabled) || !IsVisible()) // Ctrl+key combinations can't activate disabled or invisible controls.
            return false;

        // Check whether we handle it.
        if (controlstyle.contains(csWantSysKey) && HandleSysKey(key))
            return true;

        // See if child controls handle the key combination.
        if (controls)
            return controls->SysKeyPressed(key);

        return false;
    }

    bool Control::HandleDialogKey(WORD vkey)
    {
        if (!controls)
            return false;
        return controls->HandleDialogKey(vkey);
    }

    void Control::GainFocus(HWND otherwindow)
    {
        if (OnGainFocus)
            OnGainFocus(this, FocusChangedParameters(otherwindow));
    }

    void Control::LoseFocus(HWND otherwindow)
    {
        if (OnLoseFocus)
            OnLoseFocus(this, FocusChangedParameters(otherwindow));
    }

    void Control::ActiveEnter(Control *other)
    {
        if (OnEnter)
            OnEnter(this, ActiveChangedParameters(other, controlstate.contains(csTabFocusing)));
    }

    void Control::ActiveLeave(Control *other)
    {
        if (OnLeave)
            OnLeave(this, ActiveChangedParameters(other, other && other->controlstate.contains(csTabFocusing)));
    }

    void Control::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
    {
        if (OnMouseMove)
            OnMouseMove(this, MouseMoveParameters(x, y, vkeys));
    }

    void Control::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (OnMouseDown)
            OnMouseDown(this, MouseButtonParameters(x, y, button, vkeys));

        if ((vkeys & vksDouble) == vksDouble && OnDblClick)
            OnDblClick(this, MouseButtonParameters(x, y, button, vkeys));
    }

    void Control::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (OnMouseUp)
            OnMouseUp(this, MouseButtonParameters(x, y, button, vkeys));
    }

    void Control::MouseClick()
    {
        if (OnClick)
            OnClick(this, EventParameters());
    }

    bool Control::WantMouseWheel(bool vertical, short delta, VirtualKeyStateSet vkeys, short x, short y)
    {
        bool handled = false;
        if (OnWantMouseWheel)
            OnWantMouseWheel(this, MouseWheelParameters(vertical, delta, vkeys, x, y, handled));
        return handled;
    }

    bool Control::MouseWheel(bool &vertical, short &delta, VirtualKeyStateSet vkeys, short x, short y)
    {
        bool handled = true;
        if (OnMouseWheel)
            OnMouseWheel(this, MouseWheelParameters(vertical, delta, vkeys, x, y, handled));
        return handled;
    }

    void Control::NCMouseMove(short x, short y, LRESULT hittest, VirtualKeyStateSet vkeys)
    {
        if (OnNCMouseMove)
            OnNCMouseMove(this, NCMouseMoveParameters(x, y, hittest, vkeys));
    }

    void Control::NCMouseDown(short x, short y, MouseButtons button, LRESULT hittest, VirtualKeyStateSet vkeys)
    {
        if (OnNCMouseDown)
            OnNCMouseDown(this, NCMouseButtonParameters(x, y, button, hittest, vkeys));
        if ((vkeys & vksDouble) == vksDouble && OnNCDblClick)
            OnNCDblClick(this, NCMouseButtonParameters(x, y, button, hittest, vkeys));
    }

    void Control::NCMouseUp(short x, short y, MouseButtons button, LRESULT hittest, VirtualKeyStateSet vkeys)
    {
        if (OnNCMouseUp)
            OnNCMouseUp(this, NCMouseButtonParameters(x, y, button, hittest, vkeys));
    }

    void Control::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        if (keycode == 0 && key == 0)
            return;

        if (OnKeyPush)
            OnKeyPush(this, KeyPushParameters(keycode, key, vkeys));

        if (keycode != 0 && OnKeyDown)
            OnKeyDown(this, KeyParameters(keycode, vkeys));
        if (key != 0 && OnKeyPress)
            OnKeyPress(this, KeyPressParameters(key, vkeys));
    }

    void Control::KeyUp(WORD &keycode, VirtualKeyStateSet vkeys)
    {
        if (keycode != 0 && OnKeyUp)
            OnKeyUp(this, KeyParameters(keycode, vkeys));
    }

    //void Control::KeyPress(WCHAR &key, VirtualKeyStateSet vkeys)
    //{
    //    if (key != 0 && OnKeyPress)
    //        OnKeyPress(this, KeyPressParameters(key,vkeys));
    //}

    void Control::PositionSizeStarted()
    {
        if (OnStartSizeMove)
            OnStartSizeMove(this, EventParameters());
    }

    void Control::PositionSizeChanged(const Rect &newrect)
    {
        if (sizemoverect == newrect)
            return;

        if (OnPositionChanged && (sizemoverect.left != newrect.left || sizemoverect.top != newrect.top))
            OnPositionChanged(this,SizePositionChangedParameters(sizemoverect, newrect));
        if (OnSizeChanged && (sizemoverect.Width() != newrect.Width() || sizemoverect.Height() != newrect.Height()))
            OnSizeChanged(this,SizePositionChangedParameters(sizemoverect, newrect));
        if (OnEndSizeMove /* */ )
            OnEndSizeMove(this, SizePositionChangedParameters(sizemoverect, newrect));
        sizemoverect = newrect;
    }

    void Control::Showing()
    {
        if (HandleCreated() && controls)
        {
            for (int ix = 0; ix < controls->ControlCount(); ++ix)
            {
                Control *c = controls->Controls(ix);
                if (!c->HandleCreated() || !c->Visible())
                    continue;
                c->Showing();
            }
        }

        if ((!Parent() || Parent()->IsVisible()) && !HandleCreated())
            CreateHandle();
        if (controls && !controlstate.contains(csCreatingHandle))
            controls->Showing();
        if (OnShow)
            OnShow(this, EventParameters());
    }

    void Control::Hiding()
    {
        if (OnHide)
            OnHide(this, EventParameters());

        if (HandleCreated() && !controlstate.contains(csDestroyingHandle) && controls)
        {
            for (int ix = 0; ix < controls->ControlCount(); ++ix)
            {
                Control *c = controls->Controls(ix);
                if (!c->HandleCreated() || !c->Visible())
                    continue;
                c->Hiding();
            }
        }
    }

    bool Control::TextChanging(const std::wstring &newtext)
    {
        return false;
    }

    bool Control::FontChanging()
    {
        return false;
    }

    void Control::HandlePaint(const Rect &updaterect, HRGN &rgn)
    {
        if (controls)
        {
            if (rgn == 0)
                rgn = CreateRectRgnIndirect(&updaterect);
            controls->ClipRegion(rgn, updaterect);
        }
    }

    void Control::Paint(const Rect &updaterect)
    {
    }

    //#include "syscontrol.cpp"
    void Control::HandleErase(const Rect &area)
    {
        if (controlstyle.contains(csTransparent) || area.Empty())
            return;// true;

        if (DoubleBuffered() && !controlstate.contains(csPainting) && !controlstate.contains(csChildBackground) && !controlstate.contains(csChildPainting) && erasechild == nullptr)
            return;

        HRGN rgn = 0;

        Gdiplus::Region gdirgn;

        Canvas *c = GetCanvas();
        if (controls)
        {
            Rect cr = area.Intersect(ClientRect());
            if (cr.Empty())
                return;// true;

            c->GetClip(gdirgn);
            rgn = CreateRectRgnIndirect(&cr);
            controls->ClipRegion(rgn, cr, controlstate.contains(csChildBackground) || controlstate.contains(csChildPainting));
            c->SetClip(rgn);
            c->IncludeClip(gdirgn);
        }
        else if (c->ClipEmpty())
            return;// true;

        /*result =*/ EraseBackground();

        if (rgn)
        {
            //if (gdirgn.IsInfinite())
            //    c->ResetClip();
            //else
            c->SetClip(gdirgn);
            DeleteObject(rgn);
        }

        //return result;
    }

    //#include <vsstyle.h>

    bool Control::NCPaint()
    {
        //if (border == bsModern)
        //{
        //    Rect r = WindowRect();
        //    OffsetRect(&r, -r.left, -r.top);
        //    Canvas *c = GetCanvas();
        //    themes->DrawEditBorder(Handle(), c, r, !controlstate.contains(csEnabled) ? tbdsDisabled : controlstate.contains(csFocused) ? tbdsFocused : tbdsNormal);
        //    int cx = GetSystemMetrics(SM_CXEDGE);
        //    int cy = GetSystemMetrics(SM_CYEDGE);
        //    Gdiplus::Region rgn(r);
        //    InflateRect(&r, -cx, -cy);
        //    rgn.Exclude(r);
        //    c->SetClip(rgn);
        //}
        return false;
    }

    void Control::EraseBackground()
    {
        Canvas *c = GetCanvas();

        if (!UsingParentBackground())
            c->FillRect(ClientRect());
        else
            DrawParentBackground(ClientRect());

        //return true;
    }

    void Control::ChildPaintToggle(bool toggleon)
    {
        if (toggleon)
            ++childpaintcnt;
        if (!toggleon)
        {
            if (!childpaintcnt)
                throw L"Calling ChildPaintToggle to remove the csChildPainting flag which is not set!";
            --childpaintcnt;
        }
        if (toggleon)
            controlstate << csChildPainting;
        else
            controlstate -= csChildPainting;
    }

    void Control::ChildEraseToggle(Control *child)
    {
        erasechild = child;
        if (child)
            controlstate << csChildPainting;
        else
            controlstate -= csChildPainting;
    }

    void Control::DrawParentBackground(const Rect &r)
    {
        if (!Parent())
            throw L"Only child controls are allowed to draw their parent's background!";

        Control *parent = Parent();

        parent->controlstate -= csChildPainting;
        parent->controlstate << csChildBackground;
        themes->DrawParentBackground(Handle(), GetCanvas(), r);
        parent->controlstate -= csChildBackground;
        if (parent->childpaintcnt || erasechild)
            parent->controlstate << csChildPainting;

    }

    void Control::ExcludeClipRegion(HRGN rgn, const Rect &rgnrect, const Point &origin)
    {
        if (!controlstyle.contains(csTransparent))
        {
            Rect r = ClientRect().Offset(origin).Intersect(rgnrect);
            if (!UsingParentBackground())
                CombineRgnWithRect(rgn, rgn, r, rcmDiff);
            else
            {
                if (!ExcludeOpaqueRegion(rgn, r, origin))
                {
                    r = OpaqueRect().Offset(origin).Intersect(r);
                    if (!r.Empty())
                        CombineRgnWithRect(rgn, rgn, r, rcmDiff);
                }
            }
        }
        if (controls)
            controls->ExcludeChildRegion(rgn, rgnrect, origin);
    }

    HWND Control::Handle()
    {
        if (handle == NULL)
            CreateHandle();
        return handle;
    }

    int Control::DlgId() const
    {
        return dlgid;
    }

    void Control::RecreateHandle()
    {
        if (HandleCreated())
        {
            controlstate << csRecreating;
            DestroyHandle();
        }
        CreateHandle();
    //#ifdef DESIGNING
    //    if (Designing())
    //    {
    //        DesignForm *df = dynamic_cast<DesignForm*>(ParentForm());
    //        if (df)
    //            SendMessage(df->Handle(), amHijackChild, (WPARAM)this, 0);
    //    }
    //#endif
    }

    void Control::HandleUnneeded()
    {
        if (!handle)
            return;
        if (!IsVisible() /*!controlstate.contains(csVisible) || (Parent() && !Parent()->IsVisible())*/)
            DestroyHandle();
    }

    bool Control::HandleCreated() const
    {
        return handle != NULL;
    }

    void MapWindowPoints(HWND h1, HWND h2, Rect &r)
    {
        Point pt[2] = { Point(r.left, r.top), Point(r.right, r.bottom) };
        MapWindowPoints(h1, h2, pt, 2);
        r = Rect(pt[0].x, pt[0].y, pt[1].x, pt[1].y);
    }

    Rect Control::WindowRect() const
    {
        if (handle == NULL)
            return rect;

        Rect r;

        GetWindowRect(handle, &r);
        if (Parent())
            MapWindowPoints(NULL, parent->handle, r);
        return r;
    }

    Rect Control::ClientRect()
    {
        Rect crect;

        if (!HandleCreated())
            CreateHandle();
        GetClientRect(Handle(), &crect);

        return crect;
    }

    Rect Control::OldClientRectangle()
    {
        return clientrect;
    }

    Rect Control::OpaqueRect()
    {
        return Rect();
    }

    Size Control::MinimumSize() const
    {
        return minsize;
    }

    void Control::SetMinimumSize(Size newminsize)
    {
        newminsize.cx = max(newminsize.cx, 0);
        newminsize.cy = max(newminsize.cy, 0);
        if (minsize == newminsize)
            return;
        minsize = newminsize;
    }

    int Control::MinimumWidth() const
    {
        return minsize.cx;
    }

    void Control::SetMinimumWidth(int newminw)
    {
        newminw = max(newminw, 0);

        if (minsize.cx == newminw)
            return;
        minsize.cx = newminw;
    }

    int Control::MinimumHeight() const
    {
        return minsize.cy;
    }

    void Control::SetMinimumHeight(int newminh)
    {
        newminh = max(newminh, 0);

        if (minsize.cy == newminh)
            return;
        minsize.cy = newminh;
    }

    Size Control::MaximumSize() const
    {
        return maxsize;
    }

    void Control::SetMaximumSize(Size newmaxsize)
    {
        newmaxsize.cx = max(newmaxsize.cx, 0);
        newmaxsize.cy = max(newmaxsize.cy, 0);
        if (maxsize == newmaxsize)
            return;
        maxsize = newmaxsize;
    }

    int Control::MaximumWidth() const
    {
        return maxsize.cx;
    }

    void Control::SetMaximumWidth(int newmaxw)
    {
        newmaxw = max(newmaxw, 0);

        if (maxsize.cx == newmaxw)
            return;
        maxsize.cx = newmaxw;
    }

    int Control::MaximumHeight() const
    {
        return maxsize.cy;
    }

    void Control::SetMaximumHeight(int newmaxh)
    {
        newmaxh = max(newmaxh, 0);

        if (maxsize.cy == newmaxh)
            return;
        maxsize.cy = newmaxh;
    }

    int Control::SizeWidthDiff() const
    {
        return sizewidthdiff;
    }

    void Control::SetSizeWidthDiff(int newsizewidthdiff)
    {
        newsizewidthdiff = max(newsizewidthdiff, 0);

        if (sizewidthdiff == newsizewidthdiff)
            return;
        sizewidthdiff = newsizewidthdiff;
    }

    int Control::SizeHeightDiff() const
    {
        return sizeheightdiff;
    }

    void Control::SetSizeHeightDiff(int newsizeheightdiff)
    {
        newsizeheightdiff = max(newsizeheightdiff, 0);

        if (sizeheightdiff == newsizeheightdiff)
            return;
        sizeheightdiff = newsizeheightdiff;
    }

    int Control::SizeStepWidth() const
    {
        return sizemodx;
    }

    void Control::SetSizeStepWidth(int newsizestepwidth)
    {
        newsizestepwidth = max(newsizestepwidth, 0);

        if (sizemodx == newsizestepwidth)
            return;
        sizemodx = newsizestepwidth;
    }

    int Control::SizeStepHeight() const
    {
        return sizemody;
    }

    void Control::SetSizeStepHeight(int newsizestepheight)
    {
        newsizestepheight = max(newsizestepheight, 0);

        if (sizemody == newsizestepheight)
            return;
        sizemody = newsizestepheight;
    }

    void Control::SetBounds(const Rect &bounds)
    {
        if (WindowRect() == bounds)
            return;

        SaveAnchorPos(bounds);

        if (handle == NULL)
        {
            AnchorSetBounds(bounds);
            return;
        }
        SetWindowPos(handle, NULL, bounds.left, bounds.top, bounds.Width(), bounds.Height(), SWP_BOUNDS | SWP_NOACTIVATE | (controlstyle.contains(csTransparent) ? SWP_NOCOPYBITS : 0));

        if (controls && (UsingParentBackground() || controlstyle.contains(csTransparent)))
            controls->InvalidateChildren(false);
    }

    void Control::DelayedSetBounds(const Rect &newbounds)
    {
        if (!HandleCreated())
        {
            SetBounds(newbounds);
            return;
        }
    
        PostMessage(Handle(), wmSetBounds, (((short)newbounds.left)) | (((short)newbounds.top) << 16), (((short)newbounds.Width())) | (((short)newbounds.Height()) << 16));
    }

    void Control::MeasureControlArea(Rect &clientrect)
    {
        ;
    }

    //void Control::LayoutAnchoredChild(Control *child)
    //{
    //    controls->LayoutAnchoredChild(child);
    //}

    void Control::LayoutChildren(Rect newrect, bool excludeanchored)
    {
        if (!controls)
            return;
        MeasureControlArea(newrect);
        controls->LayoutChildren(newrect, excludeanchored);
    }

    Rect Control::Padding() const
    {
        if (!controls)
            return Rect();
        return controls->Padding();
    }

    void Control::SetPadding(const Rect &newpadding)
    {
        //InitControlList();
        if (!controls)
            return;

        controls->SetPadding(newpadding);
    }

    Rect Control::InnerPadding() const
    {
        if (!controls)
            return Rect();
        return controls->InnerPadding();
    }

    void Control::SetInnerPadding(const Rect &newpadding)
    {
        //InitControlList();
        if (!controls)
            return;

        controls->SetInnerPadding(newpadding);
    }

    Rect Control::AlignMargin() const
    {
        return alignmargin;
    }

    void Control::SetAlignMargin(const Rect &newalignmargin)
    {
        if (alignmargin == newalignmargin)
            return;
        alignmargin = newalignmargin;

        if (align != alAnchor && align != alNone && Parent())
            Parent()->RequestLayout(false, true);
    }

    void Control::WindowBoundsChanged(const Rect &oldrect, const Rect &newrect)
    {
        if (oldrect.Width() != newrect.Width() || oldrect.Height() != newrect.Height())
            Resizing();
        if (oldrect.left != newrect.left || oldrect.top != newrect.top)
            Moving();

        if (!usersizemove)
            PositionSizeChanged(newrect);
    }

    void Control::Resizing()
    {
        if (ParentBackground())
            RedrawWindow(Handle(), NULL, 0, RDW_INVALIDATE | RDW_ERASE);
        if (OnResize)
            OnResize(this, EventParameters());
    }

    void Control::Moving()
    {
        if (ParentBackground())
            RedrawWindow(Handle(), NULL, 0, RDW_INVALIDATE | RDW_ERASE);
        if (OnMove)
            OnMove(this, EventParameters());
    }

    void Control::ComputeAlignBounds(Rect &bounds)
    {
        if (OnComputeAlignBounds)
            OnComputeAlignBounds(this, ComputeAlignBoundsParameters(bounds));
    }

    void Control::AnchorSetBounds(const Rect &bounds)
    {
        if (HandleCreated())
            throw L"Call SetAnchorPos instead of AnchorSetBounds for controls that have a handle!";
        rect = bounds;
        clientrect = WindowRect();
        OffsetRect(&clientrect, -clientrect.left, -clientrect.top);
        //if (controls && !controlstate.contains(csCreatingHandle))
        //    LayoutChildren(clientrect, false);
    }

    void Control::SaveAnchorPos()
    {
        SaveAnchorPos(rect);
    }

    void Control::SaveAnchorPos(const Rect &bounds)
    {
        if (!Parent())
            return;

        Rect wr = bounds;
        Rect pcr;
        if (parent->HandleCreated())
        {
            pcr = parent->ClientRect();
            parent->MeasureControlArea(pcr);
        }
        else
        {
            pcr = parent->WindowRect();
            pcr.Move(-pcr.left, -pcr.top);
        }
        anchorpos = Rect(wr.left - pcr.left, wr.top - pcr.top, pcr.right - wr.right, pcr.bottom - wr.bottom);
    }

    int Control::Top() const
    {
        return WindowRect().top;
    }

    void Control::SetTop(int value)
    {
        Rect r = WindowRect();
        r.bottom += value-r.top;
        r.top = value;
        SetBounds(r);
    }

    int Control::Left() const
    {
        return WindowRect().left;
    }

    void Control::SetLeft(int value)
    {
        Rect r = WindowRect();
        r.right += value-r.left;
        r.left = value;
        SetBounds(r);
    }

    int Control::Right() const
    {
        return WindowRect().right;
    }

    int Control::Bottom() const
    {
        return WindowRect().bottom;
    }

    int Control::Width() const
    {
        return WindowRect().Width();
    }

    void Control::SetWidth(int newwidth)
    {
        Rect r = WindowRect();
        r.right = r.left + newwidth;
        SetBounds(r);
    }

    int Control::Height() const
    {
        return WindowRect().Height();
    }

    void Control::SetHeight(int newheight)
    {
        Rect r = WindowRect();
        r.bottom = r.top + newheight;
        SetBounds(r);
    }

    int Control::ClientWidth()
    {
        return ClientRect().Width();
    }

    void Control::SetClientWidth(int newcwidth)
    {
        SetWidth(newcwidth - ClientRect().Width() + Width());
    }

    int Control::ClientHeight()
    {
        return ClientRect().Height();
    }

    void Control::SetClientHeight(int newcheight)
    {
        SetHeight(newcheight - ClientRect().Height() + Height());
    }

    void Control::SetClientRect(const Rect &newcrect)
    {
        Rect r = WindowRect();
        SetBounds(r.Inflate(newcrect.Width() - ClientRect().Width(), newcrect.Height() - ClientRect().Height()));
    }

    bool Control::IsEnabled()
    {
        bool en = Enabled() && HandleCreated();
        if (!en || !controlstyle.contains(csChild))
            return en;

        Control *p = Parent();
        if (p)
            return p->IsEnabled();

        return false;
    }

    bool Control::Visible() const
    {
        return controlstate.contains(csVisible);
    }

    void Control::SetVisible(bool newvisible)
    {
        if (newvisible)
            Show();
        else
            Hide();
    }

    bool Control::IsVisible() const
    {
        bool vis = Visible() && HandleCreated();
        if (!vis || !controlstyle.contains(csChild))
            return vis;

        Control *p = Parent();
        if (p)
            return p->IsVisible();

        return false;
    }

    void Control::Show()
    {
        if (controlstate.contains(csVisible))
        {
            if (!HandleCreated())
                CreateHandle();
            return;
        }

        if (!HandleCreated() && (!controlstyle.contains(csChild) || (parent && parent->IsVisible())))
            CreateHandle();
        if (HandleCreated())
            ShowWindow(handle, controlstyle.contains(csShowDontActivate) ? SW_SHOWNA : SW_SHOW);
        else
            controlstate << csVisible;
    }

    void Control::Hide()
    {
        if (!controlstate.contains(csVisible))
        {
            if (HandleCreated() && IsWindowVisible(handle))
                DestroyHandle();
            return;
        }

        if (!HandleCreated())
        {
            controlstate -= csVisible;
            return;
        }

        ShowWindow(Handle(), SW_HIDE);
    }

    void Control::Invalidate(bool erase)
    {
        if (!IsVisible() || controlstate.contains(csInvalidating))
            return;

        controlstate << csInvalidating;

        if (Parent())
        {
            Rect cr = ClientRect();
            if (controlstyle.contains(csTransparent))
                Parent()->controls->InvalidateBelow(this, cr);
            //Parent()->controls->InvalidateTransparentAbove(this, cr);
        }
        // Should this else come inside or outside the previous if?
        //else if (controls)
        //    controls->InvalidateChildrenBackground(ClientRect());

        if (controls)
            controls->InvalidateChildren(false);

        ::InvalidateRect(Handle(), NULL, erase);

        controlstate -= csInvalidating;
    }

    void Control::InvalidateRect(const Rect &r, bool erase)
    {
        if (!IsVisible() || controlstate.contains(csInvalidating))
            return;

        controlstate << csInvalidating;

        if (Parent())
        {
            if (controlstyle.contains(csTransparent))
                Parent()->controls->InvalidateBelow(this, r);
            //Parent()->controls->InvalidateTransparentAbove(this, r);
        }
        // Should this else come inside or outside the previous if?
        //else if (controls)
        //    controls->InvalidateChildrenBackground(r);

        if (controls)
            controls->InvalidateRectChildren(r, false);

        ::InvalidateRect(Handle(), &r, erase);

        controlstate -= csInvalidating;
    }

    void Control::InvalidateRegion(HRGN rgn, bool erase)
    {
        if (!IsVisible() || controlstate.contains(csInvalidating))
            return;

        controlstate << csInvalidating;

        if (Parent())
        {
            if (controlstyle.contains(csTransparent))
                Parent()->controls->InvalidateBelow(this, rgn);
            //Parent()->controls->InvalidateTransparentAbove(this, rgn);
        }
        // Should this else come inside or outside the previous if?
        //else if (controls)
        //    controls->InvalidateChildrenBackground(rgn);

        if (controls)
            controls->InvalidateRegionChildren(rgn, false);

        ::InvalidateRgn(Handle(), rgn, erase);

        controlstate -= csInvalidating;
    }

    Point Control::ScreenToClient(int x, int y)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Point screenpt(x, y);
        ::ScreenToClient(Handle(), &screenpt);
        return screenpt;
    }

    Point Control::ClientToScreen(int x, int y)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Point clientpt(x, y);
        ::ClientToScreen(Handle(), &clientpt);
        return clientpt;
    }

    Point Control::ScreenToClient(Point screenpt)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        ::ScreenToClient(Handle(), &screenpt);
        return screenpt;
    }

    Point Control::ClientToScreen(Point clientpt)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        ::ClientToScreen(Handle(), &clientpt);
        return clientpt;
    }

    Point Control::ScreenToWindow(int x, int y)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Rect r;
        GetWindowRect(Handle(), &r);
        return Point(x - r.left, y - r.top);
    }

    Point Control::WindowToScreen(int x, int y)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Rect r;
        GetWindowRect(Handle(), &r);
        return Point(r.left + x, r.top + y);
    }

    Point Control::ScreenToWindow(const Point &screenpt)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without a handle.";
        Rect r;
        GetWindowRect(Handle(), &r);
        return Point(screenpt.x - r.left, screenpt.y - r.top);
    }

    Point Control::WindowToScreen(const Point &windowpt)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Rect r;
        GetWindowRect(Handle(), &r);
        return Point(r.left + windowpt.x, r.top + windowpt.y);
    }

    Rect Control::ScreenToWindow(Rect orig)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Rect r;
        GetWindowRect(Handle(), &r);
        return orig.Move(-r.left, -r.top);
    }

    Rect Control::WindowToScreen(Rect orig)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";
        Rect r;
        GetWindowRect(Handle(), &r);
        return orig.Move(r.left, r.top);
    }

    Point Control::ClientToWindow(int x, int y)
    {
        return ScreenToWindow(ClientToScreen(x, y));
    }

    Point Control::WindowToClient(int x, int y)
    {
        return ScreenToClient(WindowToScreen(x, y));
    }

    Point Control::ClientToWindow(const Point &clientpt)
    {
        return ScreenToWindow(ClientToScreen(clientpt));
    }

    Point Control::WindowToClient(const Point &windowpt)
    {
        return ScreenToClient(WindowToScreen(windowpt));
    }

    Rect Control::ClientToWindow(const Rect &orig)
    {
        return ScreenToWindow(ClientToScreen(orig));
    }

    Rect Control::WindowToClient(const Rect &orig)
    {
        return ScreenToClient(WindowToScreen(orig));
    }

    Rect Control::ClientToScreen(Rect orig)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";

        MapWindowPoints(Handle(), 0, orig);
        return orig;
    }

    Rect Control::ScreenToClient(Rect orig)
    {
        if (!HandleCreated())
            throw L"Cannot map points for window without handle.";

        MapWindowPoints(0, Handle(), orig);
        return orig;
    }

    Point Control::ClientToClient(int x, int y, Control *dest)
    {
        return dest->ScreenToClient(ClientToScreen(x, y));
    }

    Point Control::ClientToClient(const Point &clientpt, Control *dest)
    {
        return dest->ScreenToClient(ClientToScreen(clientpt));
    }

    Rect Control::ClientToClient(const Rect &clientrect, Control *dest)
    {
        return dest->ScreenToClient(ClientToScreen(clientrect));
    }

    Rect Control::NCRect()
    {
        if (!HandleCreated())
            return ClientRect();
        Rect rc = ClientToScreen(ClientRect());
        Rect wc;
        GetWindowRect(Handle(), &wc);
        return Rect(wc.left - rc.left, wc.top - rc.top, wc.left - rc.left + wc.Width(), wc.top - rc.top + wc.Height());
    }

    HRGN Control::NCRegion()
    {
        if (!HandleCreated())
            throw L"Cannot create region for window without handle.";

        Rect rc = ClientToScreen(ClientRect());
        Rect wc;
        GetWindowRect(Handle(), &wc);

        //HRGN nc = CreateRectRgn(rc.left - wc.left, rc.top - wc.top, rc.left - wc.left + wc.right - wc.left, rc.top - wc.top + wc.bottom - wc.top);
        //HRGN nc = CreateRectRgn(wc.left - rc.left, wc.top - rc.top, wc.left - rc.left + wc.Width(), wc.top - rc.top + wc.Height());
        HRGN rgn = CreateRectRgn(0, 0, 0, 0);
        CreateCombinedRgn(rgn, Rect(wc.left - rc.left, wc.top - rc.top, wc.left - rc.left + wc.Width(), wc.top - rc.top + wc.Height()), Rect(0, 0, rc.Width(), rc.Height()), rcmDiff);

        return rgn;
    }

    void Control::InvalidateNC()
    {
        if (!HandleCreated())
            return;
        HRGN nc = NCRegion();
        RedrawWindow(Handle(), NULL, nc, RDW_FRAME | RDW_INVALIDATE);
        DeleteObject(nc);
    }

    void Control::InvalidateNCRect(const Rect &r)
    {
        if (!HandleCreated())
            return;
        HRGN nc = NCRegion();
        CombineRgnWithRect(nc, nc, r, rcmAnd);
        RedrawWindow(Handle(), NULL, nc, RDW_FRAME | RDW_INVALIDATE);
        DeleteObject(nc);
    }

    void Control::InvalidateNCRegion(HRGN rgn)
    {
        if (!HandleCreated())
            return;
        HRGN nc = NCRegion();
        CombineRgn(nc, nc, rgn, rcmAnd);
        RedrawWindow(Handle(), NULL, rgn, RDW_FRAME | RDW_INVALIDATE);
        DeleteObject(nc);
    }

    bool Control::Enabled()
    {
#ifdef DESIGNING
        if (!Designing())
#endif
        if (HandleCreated())
        {
            if ((GetWindowLongPtr(Handle(),GWL_STYLE) & wsDisabled) != wsDisabled)
                controlstate << csEnabled;
            else
                controlstate -= csEnabled;
        }
        return controlstate.contains(csEnabled);
    }

    void Control::SetEnabled(bool newenabled)
    {
        if (newenabled == controlstate.contains(csEnabled))
            return;

        if (newenabled)
            controlstate << csEnabled;
        else
            controlstate -= csEnabled;

        if (!HandleCreated())
            return;

        PassMessage(wmEnableChanged, 0, 0);
    }

    bool Control::Focused()
    {
        if (HandleCreated())
        {
            if (GetFocus() == handle)
                controlstate << csFocused;
            else
                controlstate -= csFocused;
            return controlstate.contains(csFocused);
        }
        return false;
    }

    void Control::Focus()
    {
        if (!HandleCreated() || Focused())
            return;
        SetFocus(handle);
    }

    void Control::TabFocus()
    {
        if (!HandleCreated() || Focused())
            return;
        controlstate << csTabFocusing;
        SetFocus(handle);
    }

    Control* Control::Parent() const
    {
        return controlstyle.contains(csChild) ? parent : NULL;
    }

    Control* Control::TopLevelParent() const
    {
        return !controlstyle.contains(csChild) ? parent : NULL;
    }

    void Control::SetTopLevelParent(Control *newparent)
    {
        if (newparent && newparent->controlstyle.contains(csChild))
            throw L"Can't set the given control as top level parent, because it has csChild in its styles.";
        if (newparent == this)
            throw L"A control can't be its own top level parent.";

        if (parent == newparent && !controlstyle.contains(csChild))
            return;

        if (newparent && newparent->IsTopLevelParent(this))
            throw L"Setting the given control to be the parent of this one would create a loop.";

        if (controlstyle.contains(csChild))
        {
            if (parent)
                ParentFormChanged(parent->Parent() ? parent->ParentForm() :  dynamic_cast<Form*>(parent), 0);
            if (HandleCreated())
                controlstate << csRecreating;
            SetParent(NULL);
            controlstyle -= csChild;
        }
        else if (parent != NULL)
        {
            Control *tmp = TopLevelParent();
            parent = NULL;
            tmp->RemoveTopChild(this);
        }

        parent = newparent;
        if (newparent != NULL)
            newparent->AddTopChild(this);

        if (HandleCreated())
        {
            SetWindowLongPtr(Handle(), GWLP_HWNDPARENT, newparent ? (LONG)newparent->Handle() : 0);
            if (newparent)
            {
                Form *f = dynamic_cast<Form*>(newparent);
                if (f && f->Topmost())
                {
                    if (dynamic_cast<Form*>(this))
                        ((Form*)this)->SetTopmost(true);
                    SetWindowPos(Handle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
            }
        }
    }

    bool Control::IsTopLevelParent(Control *candidate) const
    {
        if (controlstyle.contains(csChild) || candidate->controlstyle.contains(csChild))
            return false;
        Control *c = TopLevelParent();
        while (c)
        {
            if (c == candidate)
                return true;
            c = c->TopLevelParent();
        }
        return false;
    }

    void Control::AddTopChild(Control *control)
    {
        if (!control)
            return;

        if (TopChildPosition(control) == topcontrols.end())
        {
            topcontrols.push_back(control);
            try
            {
                control->SetTopLevelParent(this);
            }
            catch (...)
            {
                topcontrols.pop_back();
            }
        }
    }

    void Control::RemoveTopChild(Control *control)
    {
        std::vector<Control*>::iterator it = TopChildPosition(control);
        if (it != topcontrols.end())
        {
            topcontrols.erase(it);
            control->SetTopLevelParent(NULL);
        }
    }

    std::vector<Control*>::iterator Control::TopChildPosition(Control *control)
    {
        std::vector<Control*>::reverse_iterator rit = std::find(topcontrols.rbegin(), topcontrols.rend(), control);
        if (rit == topcontrols.rend())
            return topcontrols.end();

        return --rit.base();
    }

    int Control::TopChildCount() const
    {
        return topcontrols.size();
    }

    Control* Control::TopChild(int index) const
    {
        return topcontrols[index];
    }

    Form* Control::ParentForm() const
    {
        if (!controlstyle.contains(csChild))
            return nullptr;

        Control *c = Parent();
        while(c)
        {
            if (dynamic_cast<Form*>(c))
                return (Form*)c;
            c = c->Parent();
        }

        return nullptr;
    }

    void Control::RemovingChild(Control *parent, Control *child)
    {
        if (Parent())
            Parent()->RemovingChild(parent, child);
        RemovingChildNotify(parent, child);
    }

    void Control::ChildRemoved(Control *parent, Control *child)
    {
        if (Parent())
            Parent()->ChildRemoved(parent, child);
        ChildRemovedNotify(parent, child);
    }

    void Control::AddingChild(Control *parent, Control *child)
    {
        if (Parent())
            Parent()->AddingChild(parent, child);
        AddingChildNotify(parent, child);
    }

    void Control::ChildAdded(Control *parent, Control *child)
    {
        if (Parent())
            Parent()->ChildAdded(parent, child);
        ChildAddedNotify(parent, child);
    }

    void Control::SetParent(Control *newparent)
    {
        if (newparent == this)
            throw L"A control can't be the parent of itself.";

        if ((parent == newparent && controlstyle.contains(csChild)) || (!newparent && !controlstyle.contains(csChild)))
            return;

        Control *oldparent = Parent();
        if (newparent != NULL)
        {
            if (ParentOf(newparent))
                throw L"Setting this control as parent would create a loop.";
            if (!newparent->IsControlParent())
                newparent->InitControlList();
        }

        if (handle && (newparent == NULL || !newparent->IsVisible()))
            DestroyHandle();

        if (!controlstyle.contains(csChild))
        {
            RemoveTopChildren(true);
            if (HandleCreated())
            {
                controlstate << csRecreating;
                SetTopLevelParent(NULL);
            }
            controlstyle << csChild;
        }
        else if (oldparent != NULL)
        {
            oldparent->RemovingChild(oldparent, this);
            parent = NULL;
            oldparent->controls->RemoveChild(this);
            if (align != alNone && align != alAnchor && HandleCreated())
                oldparent->RequestLayout(false, true);
            oldparent->ChildRemoved(oldparent, this);
        }

        parent = newparent;
        if (newparent != NULL)
        {
            newparent->AddingChild(newparent, this);
            newparent->controls->AddChild(this);
            SaveAnchorPos();
            if (Visible() && align != alNone && align != alAnchor && HandleCreated())
                newparent->RequestLayout(true, true);
        }

        if (newparent != NULL && newparent->IsVisible())
            ::SetParent(Handle(), newparent->Handle());

        if (newparent != NULL)
        {
            newparent->ChildAdded(newparent, this);

            if (controlstyle.contains(csChild) && controlstyle.contains(csParentFont) && *font != parent->GetFont())
            {
                const FontData &saveddata = font->Data();
                *font = parent->GetFont();
                UpdateFont(saveddata);
            }
        }

        Form *oldf = oldparent && oldparent->controlstyle.contains(csChild) ? oldparent->ParentForm() : dynamic_cast<Form*>(oldparent);
        Form *f = newparent && newparent->controlstyle.contains(csChild) ? newparent->ParentForm() : dynamic_cast<Form*>(newparent);
        if (f != oldf)
            ParentFormChanged(oldf, f);
    }

    void Control::ParentFormChanged(Form *oldform, Form *newform)
    {
        if (oldform != NULL)
            RemoveFromNotifyList(oldform, nrOwnership);
        if (newform != NULL)
            AddToNotifyList(newform, nrOwnership);
        if (controls != NULL)
            for (int ix = controls->ControlCount() - 1; ix >= 0; --ix)
                controls->Controls(ix)->ParentFormChanged(oldform, newform);
    }

    bool Control::ParentOf(Control *othercontrol) const
    {
        if (controls == NULL || !othercontrol->controlstyle.contains(csChild))
            return false;

        Control *tmp = othercontrol->Parent();
        while (tmp)
        {
            if (tmp == this)
                return true;
            else
                tmp = tmp->Parent();
        }
        return false;
    }

    int Control::ControlCount() const
    {
        return controls != NULL ? controls->ControlCount() : 0;
    }

    Control* Control::Controls(int ix)
    {
        return controls != NULL ? controls->Controls(ix) : NULL;
    }

    Control* Control::ControlFromHandle(HWND hwnd)
    {
        return controls != NULL ? controls->ControlFromHandle(hwnd) : NULL;
    }

    Control* Control::ControlFromId(int controlid)
    {
        return controls != NULL ? controls->ControlFromId(controlid) : NULL;
    }

    Control* Control::ControlAt(int x, int y, bool recursive, bool containeronly, bool disabled, bool hidden)
    {
        return controls != NULL ? controls->ControlAt(x, y, recursive, containeronly, disabled, hidden) : NULL;
    }

    Control* Control::ControlAt(const Point &pt, bool recursive, bool containeronly, bool disabled, bool hidden)
    {
        return controls != NULL ? controls->ControlAt(pt, recursive, containeronly, disabled, hidden) : NULL;
    }

    void Control::MoveAbove(Control *other)
    {
        if (Parent() == NULL)
            return;
        Parent()->controls->MoveAbove(this, other);
    }

    void Control::MoveBelow(Control *other)
    {
        if (Parent() == NULL)
            return;
        Parent()->controls->MoveBelow(this, other);
    }

    void Control::MoveToTop()
    {
        if (Parent() == NULL)
            return;
        Parent()->controls->MoveToTop(this);
    }

    void Control::MoveToBottom()
    {
        if (Parent() == NULL)
            return;
        Parent()->controls->MoveToBottom(this);
    }

    void Control::MoveUp()
    {
        if (Parent() == NULL)
            return;
        Parent()->controls->MoveUp(this);
    }

    void Control::MoveDown()
    {
        if (Parent() == NULL)
            return;
        Parent()->controls->MoveDown(this);
    }

    Font& Control::GetFont()
    {
        return *font;
    }

    void Control::SetFont(const Font &newfont)
    {
        *font = newfont;
    }

    std::wstring Control::Text() const
    {
        if (HandleCreated())
        {
            int len = GetWindowTextLength(handle) + 1;
            wchar_t *str = NULL;
            std::wstring txt;
            try
            {
                str = new wchar_t[len];
                GetWindowText(handle, str, len);
                txt.assign(str, len - 1);
            }
            catch(...)
            {
                delete[] str;
                throw;
            }
            delete[] str;
            return txt;
        }
        return text;
    }

    void Control::SetText(const std::wstring &newtext)
    {
        if (!handle && text == newtext)  // SA was: &&
            return;

        if (handle != NULL)
            SetWindowText(handle, newtext.c_str());
        else
            text = newtext;
    }

    int Control::TextLength() const
    {
        if (HandleCreated())
            return GetWindowTextLength(handle);
        return text.size();
    }

    Color Control::GetColor() const
    {
        if (controlstyle.contains(csChild) && controlstyle.contains(csParentColor) && Parent() != NULL)
            return Parent()->GetColor();

        return color;
    }

    void Control::SetColor(Color newcolor)
    {
        newcolor = newcolor.Solid();
        if (color == newcolor && (!controlstyle.contains(csParentColor) || !Parent()))
        {
            controlstyle -= csParentColor;
            return;
        }

        color = newcolor;

#ifdef DESIGNING
        if (controlstyle.contains(csParentColor) && Designing() && designer && designer->IsPropertyOwner(this))
            designer->InvalidateRow(this, L"ParentColor");
#endif

        controlstyle -= csParentColor;

        PassMessage(wmColorChanged, 0, 0);
    }

    bool Control::ParentBackground() const
    {
        return controlstyle.contains(csChild) && controlstyle.contains(csParentBackground);
    }

    void Control::SetParentBackground(bool newparentbg)
    {
        if (controlstyle.contains(csParentBackground) == newparentbg)
            return;

        if (newparentbg)
            controlstyle << csParentBackground;
        else
            controlstyle -= csParentBackground;

        if (controlstyle.contains(csChild))
            Invalidate();
    }

    bool Control::UsingParentBackground() const
    {
        return themes->AppThemed() && controlstyle.contains(csChild) && controlstyle.contains(csParentBackground) && Parent();
    }

    bool Control::UsingParentColor() const
    {
        return (!themes->AppThemed() || !controlstyle.contains(csParentBackground)) && controlstyle.contains(csChild) && controlstyle.contains(csParentColor) && Parent();
    }

    bool Control::ParentColor() const
    {
        return controlstyle.contains(csChild) && controlstyle.contains(csParentColor);
    }

    void Control::SetParentColor(bool newparentcol)
    {
        if (controlstyle.contains(csParentColor) == newparentcol)
            return;

        if (newparentcol)
            controlstyle << csParentColor;
        else
            controlstyle -= csParentColor;

        if (controlstyle.contains(csChild))
            PassMessage(wmColorChanged, 0, 0);

#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Color");
#endif
    }

    bool Control::ParentFont() const
    {
        return controlstyle.contains(csParentFont);
    }

    bool Control::UsingParentFont() const
    {
        return controlstyle.contains(csChild) && controlstyle.contains(csParentFont) && Parent();
    }

    void Control::SetParentFont(bool newparentfont)
    {
        if (controlstyle.contains(csParentFont) == newparentfont)
            return;

        if (newparentfont)
        {
            const FontData &saveddata = font->Data();
            controlstyle << csParentFont;
            if (controlstyle.contains(csChild) && parent && *font != parent->GetFont())
                *font = parent->GetFont();
            UpdateFont(saveddata);
#ifdef DESIGNING
            if (Designing() && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"Font");
#endif
        }
        else
            controlstyle -= csParentFont;

    }

    bool Control::ShowTooltip() const
    {
        if (controlstyle.contains(csChild) && controlstyle.contains(csParentTooltip) && Parent() != NULL)
            return Parent()->ShowTooltip();
        return controlstyle.contains(csShowTooltip);
    }

    void Control::SetShowTooltip(bool newshowtt)
    {
        if (controlstyle.contains(csShowTooltip) == newshowtt && (!controlstyle.contains(csParentTooltip) || !Parent()))
            return;

        bool tipshown = HandleCreated() && ShowTooltip();

        if (newshowtt)
            controlstyle << csShowTooltip;
        else
            controlstyle -= csShowTooltip;

#ifdef DESIGNING
        if (controlstyle.contains(csParentTooltip) && Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"ParentTooltip");
#endif
        controlstyle -= csParentTooltip;
#ifdef DESIGNING
        if (Designing())
            return;
#endif

        if (HandleCreated() && ShowTooltip() != tipshown && !tooltext.empty())
        {
            if (tipshown)
                application->DeregisterControlTooltip(this);
            else
                application->RegisterControlTooltip(this);
        }
    }

    bool Control::ParentTooltip() const
    {
        return controlstyle.contains(csChild) && controlstyle.contains(csParentTooltip);
    }

    void Control::SetParentTooltip(bool newparenttt)
    {
        if (controlstyle.contains(csParentTooltip) == newparenttt)
            return;

        bool tipshown = HandleCreated() && ShowTooltip();

        if (newparenttt)
            controlstyle << csParentTooltip;
        else
            controlstyle -= csParentTooltip;

#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"ShowTooltip");

        if (Designing())
            return;
#endif

        if (HandleCreated() && ShowTooltip() != tipshown && !tooltext.empty())
        {
            if (tipshown)
                application->DeregisterControlTooltip(this);
            else
                application->RegisterControlTooltip(this);
        }
    }

    const std::wstring& Control::TooltipText() const
    {
        return tooltext;
    }

    void Control::SetTooltipText(const std::wstring& newtooltext)
    {
        if (tooltext == newtooltext)
            return;

        bool emptytext = tooltext.empty();

        tooltext = newtooltext;

#ifdef DESIGNING
        if (Designing())
            return;
#endif

        if (HandleCreated())
        {
            if (!emptytext)
                application->DeregisterControlTooltip(this);
            if (!tooltext.empty())
                application->RegisterControlTooltip(this);
        }
    }

    void Control::CreateBGBrush()
    {
        if (bgbrush)
            return;
        bgbrush = new Brush(color);
    }

    Brush* Control::GetBGBrush()
    {
        Control *c;
        if (controlstyle.contains(csChild) && controlstyle.contains(csParentColor) && (c = Parent()) != NULL)
            return c->GetBGBrush();

        if (!bgbrush)
            CreateBGBrush();
        else
            bgbrush->SetColor(color);
        return bgbrush;
    }

    void Control::DestroyHandle()
    {
        if (handle == NULL)
            return;

        controlstate << csDestroyingHandle;
        DestroyWindow(handle);
        handle = NULL;
        controlstate -= csDestroyingHandle;
    }

    void Control::SaveWindow()
    {
        text = Text(); // saves the text of the control
        rect = WindowRect(); // saves rectangle of control
        //SaveAnchorPos();
    }

    void Control::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

#ifdef DESIGNING
        if (designpopupmenu == object)
            designpopupmenu = NULL;
#endif

        if (popupmenu == object)
            popupmenu = NULL;
    }

    ControlAnchorSet Control::Anchors() const
    {
        return ControlAnchorSet(anchors);
    }

    void Control::SetAnchors(ControlAnchorSet newanchors)
    {
        if (memcmp(&anchors,&newanchors,sizeof(ControlAnchorSet)) == 0)
            return;

        anchors = newanchors;

        bool defalign = anchors == (caLeft | caTop);
#ifndef DESIGNING
        if (!defalign && align != alAnchor)
            align = alAnchor;
        else if (defalign && align != alNone)
            align = alNone;

        if (align == alAnchor)
            SaveAnchorPos();
#else
        bool alignchange = false;

        if (!defalign && align != alAnchor)
        {
            align = alAnchor;
            alignchange = true;
        }
        else if (defalign && align != alNone)
        {
            align = alNone;
            alignchange = true;
        }

        if (align == alAnchor)
            SaveAnchorPos();

        if (alignchange && Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Alignment");
#endif

    }

    ControlAlignments Control::Alignment() const
    {
        return align;
    }

    void Control::SetAlignment(ControlAlignments newalign)
    {
        if (align == newalign)
            return;

        if (newalign == alAnchor)
            SaveAnchorPos();

        ControlAlignments oldalign = align;
        align = newalign;
        Control *parent = Parent();
        if (parent && Visible() && HandleCreated() && ((align != alNone && align != alAnchor) || (oldalign != alNone && oldalign != alAnchor)))
            parent->RequestLayout(true, true);
    }

    ControlAlignmentOrderSet Control::AlignmentOrder() const
    {
        return alignorder;
    }

    void Control::SetAlignmentOrder(ControlAlignmentOrderSet newalignorder)
    {
        if (alignorder == newalignorder)
            return;

        alignorder = newalignorder;
        RequestLayout(true, true);
    }

    bool Control::AcceptInput() const
    {
#ifdef DESIGNING
        if (Designing() && Parent())
            return false;
#endif
        return controlstyle.contains(csAcceptInput);
    }

    void Control::SetAcceptInput(bool newacceptinput)
    {
#ifdef DESIGNING
        if (Designing() && Parent())
            return;
#endif

        if (newacceptinput == controlstyle.contains(csAcceptInput) || !controlstyle.contains(csInTabOrder))
            return;

        if (newacceptinput)
            controlstyle << csAcceptInput;
        else
            controlstyle -= csAcceptInput;

        if (HandleCreated() && controlstyle.contains(csChild))
        {
            LONG style = GetWindowLongPtr(Handle(), GWL_STYLE);
            if (((style & wsTabStop) == wsTabStop) == newacceptinput)
                return;
            if (newacceptinput)
                style |= wsTabStop;
            else
                style &= ~wsTabStop;

            SetWindowLongPtr(Handle(), GWL_STYLE, style);
        }
    }

    void Control::RequestLayout(bool immediate, bool excludeanchored)
    {
        InitControlList();
        controls->RequestLayout(immediate, excludeanchored);
    }

    void Control::SetAllowLayout(bool newlayouting)
    {
#ifdef DESIGNING
        if (controlstate.contains(csDeserialize))
            return;
#endif

        InitControlList();
        controls->SetAllowLayout(newlayouting);
        clientrect = HandleCreated() ? ClientRect() : WindowRect();
    }

    bool Control::AllowLayout() const
    {
        if (!controls)
            return false;
        return controls->AllowLayout();
    }

    Control* Control::TabFirst()
    {
        if (!controls)
            return NULL;
        return controls->TabFirst();
    }

    Control* Control::TabLast()
    {
        if (!controls)
            return NULL;
        return controls->TabLast();
    }

    Control* Control::TabNext(Control *current, bool forward, bool children)
    {
        if (!controls)
            return NULL;
        return controls->TabNext(current, forward, children);
    }

    int Control::TabOrder() const
    {
        if (!Parent())
            return taborder;
        return parent->controls->ChildTabOrder(this);
    }

    void Control::SetTabOrder(int newtaborder)
    {
        if (taborder == newtaborder || !controlstyle.contains(csChild) || !controlstyle.contains(csInTabOrder))
            return;

        if (!Parent())
        {
            taborder = newtaborder;
            return;
        }

        parent->controls->ChangeTabOrder(this, newtaborder);
    }

    void Control::BroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (!HandleCreated())
            return;
        PassMessage(uMsg, wParam, lParam);
        if (controls)
            controls->BroadcastMessage(uMsg, wParam, lParam);
    }

    ControlCanvas* Control::GetCanvas()
    {
        if (!HandleCreated())
            CreateHandle();

        if (!canvas)
        {
            canvas = new ControlCanvas(this);
            ResetCanvas();
        }
        if (paintdc)
            canvas->Update(paintdc, NULL);
        else
            canvas->Update(NULL, NULL);

        return canvas;
    }

    void Control::DeleteCanvas()
    {
        delete canvas;
        canvas = NULL;
    }

    void Control::ReleaseCanvas()
    {
        if (!canvas)
            return;
        canvas->Release();
        paintdc = NULL;
    }

    void Control::UpdateCanvas(HDC dc)
    {
        paintdc = dc;
        if (!canvas)
            return;
        ((ControlCanvas*)canvas)->Update(dc, NULL);
        ResetCanvas();
    }

    void Control::ResetCanvas()
    {
        if (!canvas)
            return;
        canvas->SetFont(*font);
        canvas->SetBrush(*GetBGBrush());
        canvas->SelectStockPen(spBtnFace);
    }

    void Control::UpdateFont(const FontData &saveddata)
    {
        if (controlstyle.contains(csParentFont) && (!Parent() || *font != Parent()->GetFont()))
        {
            controlstyle -= csParentFont;
#ifdef DESIGNING
            if (Designing() && designer && designer->IsPropertyOwner(this))
                designer->InvalidateRow(this, L"ParentFont");
#endif
        }

        if (controls)
            controls->UpdateChildFonts();

        if (canvas && font->Data() != saveddata)
            canvas->SetFont(*font);

        if (HandleCreated() && font->HandleCreated())
            SendMessage(handle, WM_SETFONT, (WPARAM)font->Handle(), MAKELPARAM(controlstyle.contains(csUpdateOnTextChange) && IsVisible() ? TRUE : FALSE, 0));

        if (font->Data() != saveddata)
            FontChanged(saveddata);
    }

    BorderStyles Control::BorderStyle() const
    {
        return border;
    }

    void Control::SetBorderStyle(BorderStyles newborder)
    {
        if (border == newborder)
            return;
        border = newborder;
        if (HandleCreated())
            RecreateHandle();
    }

    Size Control::ControlBorderSize() const
    {
        switch (BorderStyle())
        {
        case bsNormal:
            return Size(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE));
        case bsModern:
            return themes->MeasureEditBorderWidth();
        case bsSingle:
            return Size(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));
        case bsNone:
        default:
            return Size(0, 0);
        }
    }

    Cursors Control::Cursor()
    {
        return cursor;
    }

    void Control::SetCursor(Cursors newcursor)
    {
        if (cursor == newcursor)
            return;
        cursor = newcursor;

#ifdef DESIGNING
        if (Designing())
            return;
#endif

        if (HandleCreated() && screencursor->HoveredControlByArea(false) == this)
            screencursor->Set(cursor);
    }

    PopupMenu* Control::GetPopupMenu()
    {
        return popupmenu;
    }

    void Control::SetPopupMenu(PopupMenu *newpopupmenu)
    {
        if (popupmenu == newpopupmenu)
            return;
        RemoveFromNotifyList(popupmenu, nrSubControl);
        popupmenu = newpopupmenu;
        AddToNotifyList(popupmenu, nrSubControl);
    }

    DragDropEffects Control::StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects)
    {
        DragDropEffects r = BeginDragDropOperation(obj, allowedeffects);
        if (OnDragDropEnded)
            OnDragDropEnded(this, DragDropEndedParameters(r));
        return r;
    }

    DragDropEffects Control::StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects, Bitmap *dragimage, Point dragpoint)
    {
        DragDropEffects r = BeginDragDropOperation(obj, allowedeffects, dragimage, dragpoint);
        if (OnDragDropEnded)
            OnDragDropEnded(this, DragDropEndedParameters(r));
        return r;
    }

    DragDropEffects Control::StartDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects, Control *imageowner)
    {
        DragDropEffects r = BeginDragDropOperation(obj, allowedeffects, imageowner->Handle());
        if (OnDragDropEnded)
            OnDragDropEnded(this, DragDropEndedParameters(r));
        return r;
    }

    //DragDropEffects Control::StartDragDrop(BasicDataObject *obj, DragDropEffectSet allowedeffects, int dragimagewidth, int dragimageheight, HBITMAP dragimage, Point dragpoint)
    //{
    //    DragDropEffects r = BeginDragDropOperation(obj, allowedeffects, dragimagewidth, dragimageheight, dragimage, dragpoint);
    //    if (OnDragDropEnded)
    //        OnDragDropEnded(this, DragDropEndedParameters(r));
    //    return r;
    //}

    void Control::StartDelayedDrag(BasicDataObject *obj, DragDropEffectSet allowedeffects)
    {
        obj->AddRef();
        PostMessage(Handle(), wmDelayedDrag, (WPARAM)obj, (LPARAM)allowedeffects);
    }

    bool Control::DropTarget() const
    {
        return droptarget != NULL;
    }

    void Control::SetDropTarget(bool newdroptarget)
    {
        if (newdroptarget == (droptarget != NULL))
            return;

        if (!newdroptarget)
        {
            RevokeDragDrop(Handle());
            droptarget->Release();
            newdroptarget = NULL;
            return;
        }

        droptarget = new BasicDropTarget(this);
        RegisterDragDrop(Handle(), droptarget);
    }

    bool Control::ShowDropImage() const
    {
        return !droptarget || (droptarget->Helper());
    }

    void Control::SetShowDropImage(bool newshowdropimage)
    {
        if (!droptarget)
            return;
        droptarget->SetHelper(newshowdropimage);
    }

    void Control::AddDropFormat(DragDropEffectSet dropeffects, CLIPFORMAT cf, StorageMediumTypes medtype, DataViewAspects aspect)
    {
        if (!droptarget)
            return;
        droptarget->AddEnumFormat(dropeffects, cf, medtype, aspect);
    }

    int Control::DropFormatCount() const
    {
        if (!droptarget)
            return 0;
        return droptarget->EnumFormatCount();
    }

    void Control::RemoveDropFormat(int ix)
    {
        if (!droptarget)
            return;
        droptarget->DeleteEnumFormat(ix);
    }

    void Control::DropFormat(int ix, DragDropEffectSet &dropeffects, CLIPFORMAT &cf, StorageMediumTypes &medtype, DataViewAspects &aspect) const
    {
        if (!droptarget)
            return;
        droptarget->EnumFormat(ix, dropeffects, cf, medtype, aspect);
    }

    DragDropEffectSet Control::DragEnter(int formatindex, VirtualKeyStateSet keys, Point p, DragDropEffectSet effects)
    {
        DragDropEffects effect = effects.contains(ddeMove) ? ddeMove : effects.contains(ddeCopy) ? ddeCopy : effects.contains(ddeLink) ? ddeLink : ddeNone;
        if ((keys.contains(vksCtrl) || keys.contains(vksShift)) && effects.contains(ddeCopy))
            effect = ddeCopy;
        if (OnDragEnter)
        {
            OnDragEnter(this, DragDropParameters(formatindex, keys, ScreenToClient(p), effects, effect));
            if (!effects.contains(effect))
                effect = ddeNone;
        }

        return effect;
    }

    DragDropEffectSet Control::DragMove(int formatindex, VirtualKeyStateSet keys, Point p, DragDropEffectSet effects)
    {
        DragDropEffects effect = effects.contains(ddeMove) ? ddeMove : effects.contains(ddeCopy) ? ddeCopy : effects.contains(ddeLink) ? ddeLink : ddeNone;
        if ((keys.contains(vksCtrl) || keys.contains(vksShift)) && effects.contains(ddeCopy))
            effect = ddeCopy;
        if (OnDragMove)
        {
            OnDragMove(this, DragDropParameters(formatindex, keys, ScreenToClient(p), effects, effect));
            if (!effects.contains(effect))
                effect = ddeNone;
        }

        return effect;
    }

    void Control::DragLeave()
    {
        if (OnDragLeave)
            OnDragLeave(this, EventParameters());
    }

    DragDropEffectSet Control::DragDrop(int formatindex, VirtualKeyStateSet keys, Point p, DragDropEffectSet effects, IDataObject *data)
    {
        DragDropEffects effect = effects.contains(ddeMove) ? ddeMove : effects.contains(ddeCopy) ? ddeCopy : effects.contains(ddeLink) ? ddeLink : ddeNone;
        if ((keys.contains(vksCtrl) || keys.contains(vksShift)) && effects.contains(ddeCopy))
            effect = ddeCopy;
        if (OnDragDrop)
        {
            OnDragDrop(this, DragDropDropParameters(data, formatindex, keys, ScreenToClient(p), effects, effect));
            if (!effects.contains(effect))
                effect = ddeNone;
        }

        return effect;
    }

    int Control::CurrentFontHeight()
    {
        HDC dc = GetDC(0);
        if (!dc)
            return 0;
        HDC cdc = CreateCompatibleDC(dc);

        int h = 0;
        if (cdc)
        {
            HFONT hf = GetFont().Handle();
            if (hf)
            {
                HFONT tmp = (HFONT)SelectObject(cdc, hf);
                Size s;
                GetTextExtentPoint32(cdc, L"My", 2, &s);
                h = s.cy;
                SelectObject(cdc, tmp);
            }
            DeleteDC(cdc);
        }

        ReleaseDC(0, dc);
        return h;
    }


    //---------------------------------------------


    const int ControlScrollbar::WHEEL = 120;

    ControlScrollbar::ControlScrollbar(ScrollableControl *owner, ScrollbarKind kind) :
            owner(owner), kind(kind), linestep(1), pagestep(10), visible(true), page(1),
            wheeldelta(0), tag(-1), pos(0), range(0), enabled(true)
    {
        if (owner->HandleCreated()) {
            Recreate();
            SCROLLINFO inf;
            memset(&inf,0,sizeof(SCROLLINFO));
            inf.cbSize = sizeof(SCROLLINFO);
            inf.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
            GetScrollInfo(owner->Handle(), kind, &inf);
            if (inf.nPage != 0) {
                range = inf.nMax;
                pos = inf.nPos;
                page = inf.nPage;
            }
        }
    }

    void ControlScrollbar::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool invalidate)
    {
        if (!enabled || !owner->HandleCreated())
            return;

        int wp = (int)wParam;

        short cmd = (wp & 0xffff);

        //int tmp = wp >> 16;
        //short wtmp = *(short*)(&tmp); // Scroll position for thumb track.

        SCROLLINFO inf;
        inf.cbSize = sizeof(SCROLLINFO);

        int npos = 0;

        switch (cmd)
        {
        case SB_THUMBPOSITION:
            if (owner->HandleCreated())
                npos = GetScrollPos(owner->Handle(), kind);
            innersetpos(npos, scPosition, invalidate);
            break;
        case SB_THUMBTRACK:
            //if (range - (page - 1) <= 65535) // Only this handles scroll position for horizontal mouse wheel scrolling correctly, but the range is limited to 65535.
            //    innersetpos(wtmp, scTrack, invalidate);
            //else
            //{
                memset(&inf,0,sizeof(SCROLLINFO));
                inf.cbSize = sizeof(SCROLLINFO);
                inf.fMask = SIF_TRACKPOS;
                if (!GetScrollInfo(owner->Handle(), kind, &inf))
                {
                    // failed, revert to 16 bit
                    inf.nTrackPos = (wp & 0xffff0000) >> 16;
                }
                innersetpos(inf.nTrackPos, scTrack, invalidate);
            //}
            break;
        case SB_TOP:
            innersetpos(0, scTop, invalidate);
            break;
        case SB_BOTTOM:
            innersetpos(range - 1, scBottom, invalidate);
            break;
        case SB_LINEUP:
            innersetpos(max(pos - linestep, 0), scLineUp, invalidate);
            break;
        case SB_LINEDOWN:
            innersetpos(min(pos + linestep, range - (page - 1)), scLineDown, invalidate);
            break;
        case SB_PAGEUP:
            innersetpos(max(pos - pagestep, 0), scPageUp, invalidate);
            break;
        case SB_PAGEDOWN:
            innersetpos(min(pos + pagestep, range - (page - 1)), scPageDown, invalidate);
            break;
        case SB_ENDSCROLL:
            //innersetpos(GetScrollPos(owner->Handle(), kind),scEndScroll,invalidate);
            if (visible)
            {
                if (owner->InvalidateOnScroll())
                    owner->Invalidate();
                owner->Scrolled(kind, pos, pos, scEndScroll);
            }
            break;
        }
    }

    void ControlScrollbar::HandleMouseWheel(short delta, VirtualKeyStateSet vkeys, short x, short y, bool invalidate)
    {
        if (!visible || !enabled)
            return;

        int val;
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &val, 0);
        delta *= val;

        delta += wheeldelta;

        int step = delta / WHEEL;
        wheeldelta = delta - step * WHEEL;
        if (step)
        {
            if (kind == skHorizontal)
                step *= -1;
            step *= linestep;
            if (step > 0)
                innersetpos(max(pos - step, 0), scLineUp, invalidate);
            else
                innersetpos(min(pos - step, range - (page - 1)), scLineDown, invalidate);
        }
    }

    void ControlScrollbar::innersetpos(int value, ScrollCode code, bool invalidate)
    {
        if (value < 0)
            value = 0;
        if (value > range - (page - 1))
            value = max(0, range - (page - 1));
        if (value == pos)
            return;

        int oldpos = pos;
        pos = value;
        if (!visible || !enabled || !owner->HandleCreated())
            return;

        setscrolldata();
    /*    SCROLLINFO inf;
        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_POS;
        inf.nPos = value;
        SetScrollInfo(owner->Handle(), kind, &inf, true);
    */
        if (invalidate)
            owner->Invalidate();

        owner->Scrolled(kind, oldpos, pos, code);
    }

    void ControlScrollbar::Recreate()
    {
        if (!owner->HandleCreated())
            return;

#ifdef DESIGNING
        if (!owner->Designing())
#endif
        EnableScrollBar(owner->Handle(), kind, enabled && owner->Enabled() ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
#ifdef DESIGNING
        else
            EnableScrollBar(owner->Handle(),kind,enabled ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
#endif

        SCROLLINFO inf;
        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
        //inf.nMax = !visible ? 0 : range;
        //inf.nMin = 0;
        //inf.nPos = visible && enabled ? pos : 0;
        //inf.nPage = page;
        //SetScrollInfo(owner->Handle(), kind, &inf, false);

        // Fixing windows bugs by making sure the real values are set.
        if (!GetScrollInfo(owner->Handle(), kind, &inf) || (int)inf.nPage != page || inf.nMax != (!visible ? 0 : range) || inf.nPos != (visible && enabled ? pos : 0))
        {
            //inf.cbSize = sizeof(SCROLLINFO);
            //inf.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
            inf.nMax = 101;
            inf.nMin = 0;
            inf.nPos = visible && enabled ? pos : 0;
            inf.nPage = page;
            SetScrollInfo(owner->Handle(), kind, &inf, false);

            //inf.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
            inf.nMax = !visible ? 0 : range;
            inf.nMin = 0;
            inf.nPos = visible && enabled ? pos : 0;
            inf.nPage = page;
            SetScrollInfo(owner->Handle(), kind, &inf, true);
        }

        if (!enabled && visible)
            EnableScrollBar(owner->Handle(), kind, ESB_DISABLE_BOTH);
    }

    void ControlScrollbar::innerset(int newmax, int newpage, bool newenabled)
    {
        if (newpage < 1)
            newpage = 1;
        if (newmax < 0)
            newmax = 0;
        if (newmax == range && newpage == page)
            return;

        range = newmax;
        page = newpage;
        if (!visible || !owner->HandleCreated())
        {
            enabled = newenabled;
            return;
        }

        setscrolldata();
        if (enabled != newenabled)
            SetEnabled(newenabled);

        if (pos > range - (page - 1))
        {
            int oldpos = pos;
            pos = max(0, range - (page - 1));
            if (!visible || pos == oldpos)
                return;
            owner->Scrolled(kind, oldpos, pos, scPosition);
        }
    }

    int ControlScrollbar::LineStep()
    {
        return linestep;
    }

    void ControlScrollbar::SetLineStep(int newlinestep)
    {
        linestep = newlinestep;
    }

    int ControlScrollbar::PageStep()
    {
        return pagestep;
    }

    void ControlScrollbar::SetPageStep(int newpagestep)
    {
        pagestep = newpagestep;
    }

    int ControlScrollbar::Range()
    {
        return range;
    }

    void ControlScrollbar::SetRange(int newrange)
    {
        if (newrange < 0)
            newrange = 0;

        if (range == newrange)
            return;

        range = newrange;
        int oldpos = pos;
        if (pos > range - (page - 1))
            pos = max(0, range - (page - 1));
        if (!visible || !enabled || !owner->HandleCreated())
            return;

        setscrolldata();
    /*
        SCROLLINFO inf;
        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_RANGE;
        inf.nMax = newrange-1;
        inf.nMin = 0;
        SetScrollInfo(owner->Handle(), kind, &inf, true);
    */
        if (pos != oldpos)
            owner->Scrolled(kind, oldpos, pos, scPosition);
    }

    int ControlScrollbar::Position()
    {
        return pos;
    }

    void ControlScrollbar::SetPosition(int newpos)
    {
        innersetpos(newpos, scPosition, true);
    }

    int ControlScrollbar::ThumbSize()
    {
        return page;
    }

    void ControlScrollbar::SetThumbSize(int newsize)
    {
        if (newsize < 1)
            newsize = 1;
        if (newsize == page)
            return;

        page = newsize;
        if (!visible || !enabled || !owner->HandleCreated())
            return;

        setscrolldata();

        if (pos > range - (page - 1))
        {
            int oldpos = pos;
            pos = max(0, range - (page - 1));
            if (!visible || pos == oldpos)
                return;
            owner->Scrolled(kind, oldpos, pos, scPosition);
        }
        /*
        SCROLLINFO inf;
        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_PAGE;
        inf.nPage = page;
        SetScrollInfo(owner->Handle(), kind, &inf, false);
        */
    }
    bool ControlScrollbar::Visible()
    {
        return visible;
    }

    void ControlScrollbar::SetVisible(bool newvisible)
    {
        if (newvisible == visible)
            return;
        visible = newvisible;
        if (owner->AutoSizeScroll())
            owner->ScrollResize();

        setscrolldata();
    /*    if (!visible || !owner->HandleCreated())
        {
            SCROLLINFO inf;
            inf.cbSize = sizeof(SCROLLINFO);
            inf.fMask = SIF_RANGE;
            inf.nMax = !newvisible ? 0 : range;
            inf.nMin = 0;
            SetScrollInfo(owner->Handle(), kind, &inf, false);
        }
    */
    }

    bool ControlScrollbar::Enabled()
    {
        return enabled;
    }

    void ControlScrollbar::SetEnabled(bool newenabled)
    {
        if (enabled == newenabled)
            return;
        enabled = newenabled;
        if (!owner->HandleCreated())
            return;

        if (visible)
        {
#ifdef DESIGNING
            if (!owner->Designing())
                EnableScrollBar(owner->Handle(), kind, newenabled && owner->Enabled() ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
            else
                EnableScrollBar(owner->Handle(), kind, newenabled ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
#else
            EnableScrollBar(owner->Handle(), kind, newenabled && owner->Enabled() ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
#endif
            if (enabled)
                Recreate();
        }

    }

    void ControlScrollbar::setscrolldata()
    {
        SCROLLINFO inf;
        memset(&inf,0,sizeof(SCROLLINFO));
        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
        inf.nMax = !visible ? 0 : range;
        inf.nMin = 0;
        inf.nPos = visible && enabled ? pos : 0;
        inf.nPage = page;
        SetScrollInfo(owner->Handle(), kind, &inf, true);
    }

    /*
    bool ControlScrollbar::AutoSize()
    {
        return autosize;
    }

    void ControlScrollbar::SetAutoSize(bool newauto)
    {
        autosize = newauto;
    }
    */

    bool ControlScrollbar::IsVisible()
    {
        if (!owner->HandleCreated())
            return false;

        SCROLLINFO inf;
        memset(&inf,0,sizeof(SCROLLINFO));

        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;

        GetScrollInfo(owner->Handle(), kind, &inf); // Duplicate call to avoid bug in windows.
        return (GetScrollInfo(owner->Handle(), kind, &inf) && (inf.nMax - inf.nMin) + 1 > (int)inf.nPage && inf.nPage > 0);
    }


    //---------------------------------------------

#ifdef DESIGNING
    void ScrollableControl::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->AddEvent<ScrollableControl, ScrollEvent>(L"OnScroll", L"Scrollbars");
        serializer->AddEvent<ScrollableControl, ScrollOverflowEvent>(L"OnScrollOverflow", L"Scrollbars");
        serializer->AddEvent<ScrollableControl, NotifyEvent>(L"OnScrollAutoSized", L"Scrollbars");

        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);
    }
#endif

    ScrollableControl::ScrollableControl() : vbar(NULL), hbar(NULL), autosizescroll(false), scrollresizing(false), hautopagestep(0), vautopagestep(0), invalidateonscroll(false)
    {
        SetWantedKeyTypes(wkOthers | wkArrows);
    }

    ScrollableControl::~ScrollableControl()
    {
        if (vbar)
            delete vbar;
        if (hbar)
            delete hbar;
    }

    void ScrollableControl::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
        params.style -= csVRedraw;
        params.style -= csHRedraw;
    }

    void ScrollableControl::InitHandle()
    {
        base::InitHandle();

        if (OnScrollOverflow || OnScroll)
            NeedScrollbars();
        if (vbar)
        {
            vbar->Recreate();
            hbar->Recreate();
        }
        if (OnScrollOverflow && !AutoSizeScroll())
            SetAutoSizeScroll(true);
        else
            ScrollResize();
    }

    LRESULT ScrollableControl::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        ControlScrollbar *bar;

        if (vbar || hbar)
        {
            switch(uMsg)
            {
            //case WM_MOUSEWHEEL:
            //case WM_MOUSEHWHEEL:
            //    bar = uMsg == WM_MOUSEWHEEL && ((wParam & 0xffff) & MK_SHIFT) != MK_SHIFT ? vbar : hbar;
            //    if (bar->Visible() && bar->Enabled())
            //    {
            //        bar->HandleMessage(uMsg, wParam, lParam, false);
            //        if (invalidateonscroll && bar->tag != bar->Position())
            //            Invalidate();
            //        bar->tag = bar->Position();
            //    }
            //    return uMsg == WM_MOUSEHWHEEL ? TRUE : 0; // Returning TRUE for WM_MOUSEHWHEEL is a hack to handle some buggy mouse drivers. (i.e. Logitech)
            case WM_VSCROLL:
            case WM_HSCROLL:
                bar = uMsg == WM_VSCROLL ? vbar : hbar;
                if (lParam == 0 && bar->Visible() && bar->Enabled())
                {
                    if (IsVisible() && Enabled() && AcceptInput() && controlstyle.contains(csInTabOrder))
                        Focus();
                    bar->HandleMessage(uMsg, wParam, lParam, false);
                    if (invalidateonscroll && (bar->tag != bar->Position() || (wParam & SB_ENDSCROLL) == SB_ENDSCROLL))
                        Invalidate();
                    bar->tag = bar->Position();
                }
                return 0;
            }
        }

        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool ScrollableControl::MouseWheel(bool &vertical, short &delta, VirtualKeyStateSet vkeys, short x, short y)
    {
        if (!vbar && !hbar)
            return base::MouseWheel(vertical, delta, vkeys, x, y);

        bool handled = true;
        if (OnMouseWheel)
            OnMouseWheel(this, MouseWheelParameters(vertical, delta, vkeys, x, y, handled));
        if (!handled || delta == 0)
            return handled;

        ControlScrollbar *bar = !vertical /*|| vkeys.contains(vksShift)*/ ? hbar : vbar;
        if (!bar->Visible() || !bar->Enabled())
            return false;

        if (vertical && bar == hbar)
            delta *= -1;

        bar->HandleMouseWheel(delta, vkeys, x, y, false);
        if (invalidateonscroll && bar->tag != bar->Position())
            Invalidate();
        bar->tag = bar->Position();
        return true;
    }

    void ScrollableControl::Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code)
    {
        //if (pos != oldpos && AutoSizeScroll())
        //{
        //    if (HandleCreated())
        //        ScrollWindowEx(Handle(), kind == skHorizontal ? oldpos - pos : 0, kind == skVertical ? oldpos - pos : 0, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
        //}

        if (OnScroll)
            OnScroll(this, ScrollParameters(kind, oldpos, pos, code));
    }

    void ScrollableControl::WindowBoundsChanged(const Rect &oldrect, const Rect &newrect)
    {
        ScrollResize();
        base::WindowBoundsChanged(oldrect, newrect);
    }

    void ScrollableControl::Showing()
    {
        if (AutoSizeScroll())
            ScrollResize();
        base::Showing();
    }

    bool ScrollableControl::InvalidateOnScroll()
    {
        return invalidateonscroll;
    }

    void ScrollableControl::SetInvalidateOnScroll(bool newinvalidateonscroll)
    {
        invalidateonscroll = newinvalidateonscroll;
    }

    ControlScrollbar* ScrollableControl::HScroll()
    {
        if (!hbar)
            NeedScrollbars();
        return hbar;
    }

    ControlScrollbar* ScrollableControl::VScroll()
    {
        if (!vbar)
            NeedScrollbars();
        return vbar;
    }

    int ScrollableControl::HPos()
    {
        if (!hbar)
            return 0;
        return hbar->Position();
    }

    int ScrollableControl::VPos()
    {
        if (!vbar)
            return 0;
        return vbar->Position();
    }

    int ScrollableControl::HRange()
    {
        if (!hbar)
            return 0;
        return hbar->Range();
    }

    int ScrollableControl::VRange()
    {
        if (!vbar)
            return 0;
        return vbar->Range();
    }

    void ScrollableControl::SetHPos(int val)
    {
        if (!hbar)
            NeedScrollbars();
        hbar->SetPosition(val);
    }

    void ScrollableControl::SetVPos(int val)
    {
        if (!vbar)
            NeedScrollbars();
        vbar->SetPosition(val);
    }

    void ScrollableControl::SetHRange(int val)
    {
        if (!hbar)
            NeedScrollbars();
        hbar->SetRange(val);
    }

    void ScrollableControl::SetVRange(int val)
    {
        if (!vbar)
            NeedScrollbars();
        vbar->SetRange(val);
    }

    int ScrollableControl::HLineStep()
    {
        if (!hbar)
            return 0;
        return hbar->LineStep();
    }

    void ScrollableControl::SetHLineStep(int val)
    {
        if (!hbar)
            NeedScrollbars();
        hbar->SetLineStep(val);
    }

    int ScrollableControl::VLineStep()
    {
        if (!vbar)
            return 0;
        return vbar->LineStep();
    }

    void ScrollableControl::SetVLineStep(int val)
    {
        if (!vbar)
            NeedScrollbars();
        vbar->SetLineStep(val);
    }

    int ScrollableControl::HPageStep()
    {
        if (!hbar)
            return 0;
        return hbar->PageStep();
    }

    void ScrollableControl::SetHPageStep(int val)
    {
        if (!hbar)
            NeedScrollbars();
        hbar->SetPageStep(val);
    }

    int ScrollableControl::VPageStep()
    {
        if (!vbar)
            return 0;
        return vbar->PageStep();
    }

    void ScrollableControl::SetVPageStep(int val)
    {
        if (!vbar)
            NeedScrollbars();
        vbar->SetPageStep(val);
    }

    void ScrollableControl::NeedScrollbars()
    {
        if (vbar)
            return;
        vbar = new ControlScrollbar(this, skVertical);
        hbar = new ControlScrollbar(this, skHorizontal);

        vbar->Recreate();
        hbar->Recreate();
    }

    bool ScrollableControl::AutoSizeScroll()
    {
        if (!vbar || !hbar)
        {
            if (OnScrollOverflow)
                NeedScrollbars();
            else
                return false;
        }
        return autosizescroll || OnScrollOverflow;
    }

    void ScrollableControl::SetAutoSizeScroll(bool setauto)
    {
        if (autosizescroll == setauto)
            return;
        autosizescroll = setauto;
        NeedScrollbars();

        if (HandleCreated())
            ScrollResize();
    }

    float ScrollableControl::HAutoPageStep()
    {
        return hautopagestep;
    }

    void ScrollableControl::SetHAutoPageStep(float newhautopagestep)
    {
        if (newhautopagestep == hautopagestep)
            return;

        hautopagestep = newhautopagestep;

        if (!hbar || !HandleCreated() || !AutoSizeScroll())
            return;

        if (hautopagestep == 0)
            SetHPageStep(max(1, max(hbar->LineStep() / 2, hbar->ThumbSize() - hbar->LineStep() )));
        else if (hautopagestep > 0)
            SetHPageStep(max(1, (hbar->ThumbSize() * hautopagestep)));
        else
            SetHPageStep(max(1, hbar->ThumbSize() + int(hautopagestep)));
    }

    float ScrollableControl::VAutoPageStep()
    {
        return vautopagestep;
    }

    void ScrollableControl::SetVAutoPageStep(float newvautopagestep)
    {
        if (newvautopagestep == vautopagestep)
            return;

        vautopagestep = newvautopagestep;

        if (!vbar || !HandleCreated() || !AutoSizeScroll())
            return;

        if (vautopagestep == 0)
            SetVPageStep(max(1, max(vbar->LineStep() / 2, vbar->ThumbSize() - vbar->LineStep() )));
        else if (vautopagestep > 0)
            SetVPageStep(max(1, (vbar->ThumbSize() * vautopagestep)));
        else
            SetVPageStep(max(1, vbar->ThumbSize() + int(vautopagestep)));
    }

    void ScrollableControl::GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide)
    {
        if (!vbar || !hbar)
            throw L"No scrollbars created. Call NeedScrollbars() in constructor.";

        if (OnScrollOverflow)
        {
            OnScrollOverflow(this, ScrollOverflowParameters(uw, uh, hw, hh, hnohide, vnohide));
            return;
        }

        hw = 0;
        hh = 0;
    }

    void ScrollableControl::MeasureScrollClient(int &uw, int &uh, int &hw, int &hh, bool &hvis, bool &vvis, bool &hnohide, bool &vnohide)
    {
        int vwidth = GetSystemMetrics(SM_CXVSCROLL);
        int hheight = GetSystemMetrics(SM_CYHSCROLL);

        int w = uw;
        int h = uh;

        bool changed;
        do
        {
            changed = false;
            hw = 0;
            hh = 0;
            uw = w;
            uh = h;
            GetOverflow(uw, uh, hw, hh, hnohide, vnohide);
            if (hw < 0)
                hw = 0;
            if (hh < 0)
                hh = 0;

            if (hvis && !hw && !hnohide)
            {
                hvis = false;
                changed = true;
            }
            else if (!hvis && HScroll()->Visible() && (hw > 0 || hnohide))
            {
                hvis = true;
                changed = true;
                h -= hheight;
            }

            if (vvis && !hh && !vnohide)
            {
                vvis = false;
                changed = true;
            }
            else if (!vvis && VScroll()->Visible() && (hh > 0 || vnohide))
            {
                vvis = true;
                changed = true;
                w -= vwidth;
            }
        } while (changed);
    }

    void ScrollableControl::ScrollResize()
    {
        if (scrollresizing || !AutoSizeScroll() || !HandleCreated())
            return;

        scrollresizing = true;

        Rect cr = ClientRect();

        int vwidth = GetSystemMetrics(SM_CXVSCROLL);
        int hheight = GetSystemMetrics(SM_CYHSCROLL);

        int uw = cr.Width();
        int uh = cr.Height();
        int hw = 0;
        int hh = 0;
        bool hnohide = false;
        bool vnohide = false;

        bool vvis = false;
        bool hvis = false;

        SCROLLINFO inf;
        memset(&inf,0,sizeof(SCROLLINFO));

        inf.cbSize = sizeof(SCROLLINFO);
        inf.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;

        if (GetScrollInfo(Handle(), SB_VERT, &inf) && (inf.nMax - inf.nMin) + 1 > (int)inf.nPage && inf.nPage > 0)
            uw += vwidth;

        GetScrollInfo(Handle(), SB_HORZ, &inf);
        if (GetScrollInfo(Handle(), SB_HORZ, &inf) && (inf.nMax - inf.nMin) + 1 > (int)inf.nPage && inf.nPage > 0)
            uh += hheight;

        MeasureScrollClient(uw, uh, hw, hh, hvis, vvis, hnohide, vnohide);

        if (!hvis)
            hbar->innerset(0, 0, hbar->enabled);
        if (!vvis)
            vbar->innerset(0, 0, vbar->enabled);

        if (hvis)
        {
            if (!hnohide || (hw && (!hnohide || hw + uw - 1 > uw))) // There is either a hidden part (hw not null) or nohide is false so there shouldn't be a visible disabled scrollbar around.
                hbar->innerset(hw + uw - 1, uw, true);
            else
                hbar->innerset(max(1, uw + 1), max(0, uw), false);

            if (hautopagestep == 0)
                SetHPageStep(max(1, max(hbar->LineStep() / 2, hbar->ThumbSize() - hbar->LineStep() )));
            else if (hautopagestep > 0)
                SetHPageStep(max(1, (hbar->ThumbSize() * hautopagestep)));
            else
                SetHPageStep(max(1, hbar->ThumbSize() + int(hautopagestep)));
        }
        if (vvis)
        {
            if (!vnohide || (hh && (!vnohide || hh + uh - 1 > uh))) // There is either a hidden part (hh not null) or nohide is false so there shouldn't be a visible disabled scrollbar around.
                vbar->innerset(hh + uh - 1, uh, true);
            else
                vbar->innerset(max(1, uh + 1), max(0, uh), false);

            if (vautopagestep == 0)
                SetVPageStep(max(1, max(vbar->LineStep() / 2, vbar->ThumbSize() - vbar->LineStep() )));
            else if (vautopagestep > 0)
                SetVPageStep(max(1, (vbar->ThumbSize() * vautopagestep)));
            else
                SetVPageStep(max(1, vbar->ThumbSize() + int(vautopagestep)));
        }

        scrollresizing = false;

        if (OnScrollAutoSized)
            OnScrollAutoSized(this, EventParameters());
    }

    int Control::BeginUpdate()
    {
        if (!HandleCreated())
            return 0;
        if (!updatecnt)
            SendMessage(Handle(), WM_SETREDRAW, FALSE, 0);
        return ++updatecnt;
    }

    int Control::EndUpdate()
    {
        if (!HandleCreated() || updatecnt == 0)
            return 0;
        if (updatecnt == 1)
            SendMessage(Handle(), WM_SETREDRAW, TRUE, 0);
        return --updatecnt;
    }

    int Control::UpdateCount()
    {
        if (!HandleCreated())
            return 0;
        return updatecnt;
    }

    const WantedKeySet& Control::WantedKeyTypes() const
    {
        return wantedkeys;
    }

    void Control::SetWantedKeyTypes(WantedKeySet newtypes)
    {
        if (wantedkeys == newtypes)
            return;
        wantedkeys = newtypes;
    }

    bool Control::DoubleBuffered() const
    {
#ifdef DESIGNING
        if (Designing())
            return false;
#endif
        return doublebuffered;
    }

    void Control::SetDoubleBuffered(bool newdblbuff)
    {
        doublebuffered = newdblbuff;
    }


    //---------------------------------------------


    ControlList::ControlList(Control *owner) : owner(owner), allowlayout(true), tabinited(false)
    {
    }

    ControlList::~ControlList()
    {
    }

    void ControlList::Destroy()
    {
        RemoveChildren(true);
    }

    void ControlList::InitHandle()
    {
        //Rect r = owner->ClientRect();
        //Rect wr = owner->WindowRect();
        //if (r.Width() != wr.Width() || r.Height() != wr.Height())
        //{
        //    for (Control *c : controls)
        //    {
        //        if (c->align != alNone && c->align != alAnchor && c->IsControlParent())
        //            c->controls->UpdateClientAnchors(wr.Width() - r.Width(), wr.Height() - r.Height());
        //    }
        //}
    }

    void ControlList::ChildrenRecreate()
    {
        for (auto it = controls.begin(); it != controls.end(); it++)
        {
            Control *control = *it;
            if (!control->HandleCreated())
                continue;
            control->controlstate << csRecreating;
        }
    }

    void ControlList::SaveChildrenAnchorPos()
    {
        for (auto it = controls.begin(); it != controls.end(); it++)
            (*it)->SaveAnchorPos();
    }

    //void ControlList::UpdateClientAnchors(int widthdiff, int heightdiff)
    //{
    //    Rect r = owner->ClientRect();
    //    Rect wr = owner->WindowRect();
    //
    //    for (auto it = controls.begin(); it != controls.end(); it++)
    //    {
    //        Control *c = *it;
    //        if (c->align == alAnchor && c->anchors != (caLeft | caTop))
    //        {
    //            Rect &r = c->anchorpos;
    //
    //            if (owner->align == alClient || owner->align == alTop || owner->align == alBottom)
    //            {
    //                if (!c->anchors.contains(caLeft))
    //                {
    //                    if (c->anchors.contains(caRight))
    //                        r.left += widthdiff;
    //                    else
    //                    {
    //                        r.left += widthdiff / 2;
    //                        r.right -= widthdiff / 2;
    //                    }
    //                }
    //                if (c->anchors.contains(caRight))
    //                    r.right -= widthdiff;
    //            }
    //
    //            if (owner->align == alClient || owner->align == alLeft || owner->align == alRight)
    //            {
    //                if (!c->anchors.contains(caTop))
    //                {
    //                    if (c->anchors.contains(caBottom))
    //                        r.top += heightdiff;
    //                    else
    //                    {
    //                        r.top += heightdiff / 2;
    //                        r.bottom -= heightdiff / 2;
    //                    }
    //                }
    //                if (c->anchors.contains(caBottom))
    //                    r.bottom -= heightdiff;
    //            }
    //
    //            //Rect r = c->WindowRect();
    //
    //            //if (align == alClient || align == alTop || align == alBottom)
    //            //{
    //            //    if (!c->anchors.contains(caLeft))
    //            //    {
    //            //        if (c->anchors.contains(caRight))
    //            //            r.left += widthdiff;
    //            //        else
    //            //        {
    //            //            r.left += widthdiff / 2;
    //            //            r.right += widthdiff / 2;
    //            //        }
    //            //    }
    //            //    if (c->anchors.contains(caRight))
    //            //        r.right += widthdiff;
    //            //}
    //
    //            //if (align == alClient || align == alLeft || align == alRight)
    //            //{
    //            //    if (!c->anchors.contains(caTop))
    //            //    {
    //            //        if (c->anchors.contains(caBottom))
    //            //            r.top += heightdiff;
    //            //        else
    //            //        {
    //            //            r.top += heightdiff / 2;
    //            //            r.bottom += heightdiff / 2;
    //            //        }
    //            //    }
    //            //    if (c->anchors.contains(caBottom))
    //            //        r.bottom += heightdiff;
    //            //}
    //
    //            //c->SetBounds(r);
    //        }
    //
    //        if (c->align != alAnchor && c->align != alNone && (c->align == alClient || owner->align == alClient || ((owner->align == alLeft || owner->align == alRight) && (c->align == alLeft || c->align == alRight)) || ((owner->align == alTop || owner->align == alBottom) && (c->align == alTop || c->align == alBottom))) && c->IsControlParent())
    //            c->controls->UpdateClientAnchors(widthdiff, heightdiff);
    //    }
    //}

    void ControlList::SaveChildren()
    {
        for (auto control : controls)
        {
            if (!control->HandleCreated())
                continue;
            control->SaveWindow();
            if (control->controls)
                control->controls->SaveChildren();
        }
    }

    void ControlList::RemoveChildren(bool deletechildren)
    {
        std::vector<Control*> ccopy;
        ccopy.swap(controls);
        for (auto ctrl : ccopy)
        {
            if (deletechildren)
                ctrl->Destroy();
            else
                ctrl->SetParent(NULL);
        }
    }

    bool ControlList::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &result)
    {
        HRESULT r;
        HBRUSH brushresult;
        Control *control;
        SystemControl *syscontrol;
        Rect newclient;
        NMHDR notify;
        MEASUREITEMSTRUCT *mi;
        DRAWITEMSTRUCT *di;

        result = 0;

        switch (uMsg)
        {
        case wmHandleCreated:
            CreateChildHandles();
            break;
        case WM_COMMAND:
            // WParam: HI word notification code (0:Menu, 1: accelerator, 
            //          other), LOQ word : control ID
            // LParam: 0 for menus and accel, handle for control
            if (lParam) {
                control = ControlFromHandle((HWND)lParam);
                if (control)
                {
                    syscontrol = dynamic_cast<SystemControl*>(control);
                    if (syscontrol && syscontrol->HandleCommand(owner, wParam))
                        return true;
                }
                else if (owner->HandleChildCommand((HWND)lParam, wParam))
                    return true;
            }
            break;
        case WM_NOTIFY:
            notify = *(NMHDR*)lParam;
            control = ControlFromHandle(notify.hwndFrom);
            r = 0;
            if (control)
            {
                syscontrol = dynamic_cast<SystemControl*>(control);
                if (syscontrol && syscontrol->HandleNotify(owner, lParam, r))
                {
                    result = r;
                    return true;
                }
            }
            else if (owner->HandleChildNotify(notify.hwndFrom, lParam, r))
            {
                result = r;
                return true;
            }

            break;
        case WM_MEASUREITEM:
            mi = (MEASUREITEMSTRUCT*)lParam;
            if (mi->CtlType != ODT_MENU)
            {
                // This message is sent to controls during creation, but at that time we don't have an id
                // nor a handle to work with yet, so simply ignore the message.
                // The controls should call the measureitem event locally in the CreateHandle function,
                // and set the item height there.
                if (wParam == 0)
                    break;

                // Handle the message normally when we have an id.
                control = ControlFromId(wParam);
                syscontrol = dynamic_cast<SystemControl*>(control);
                if (syscontrol && syscontrol->HandleMeasureItem(owner, mi))
                {
                    result = TRUE;
                    return true;
                }
            }
            break;
        case WM_DRAWITEM:
            if (wParam != 0)
            {
                di = (DRAWITEMSTRUCT*)lParam;
                control = NULL;
                try {
                    control = ControlFromId(wParam);
                    if (control)
                        control->UpdateCanvas(di->hDC);
                    syscontrol = dynamic_cast<SystemControl*>(control);
                    if (syscontrol && syscontrol->HandleDrawItem(owner, di))
                    {
                        control->ReleaseCanvas();
                        result = TRUE;
                        return true;
                    }
                }
                catch(...)
                {
                    ;
                }
                if (control)
                    control->ReleaseCanvas();
            }
            break;
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
            syscontrol = dynamic_cast<SystemControl*>(ControlFromHandle((HWND)lParam));
            if (syscontrol)
            {
                brushresult = 0;
                if (syscontrol->HandleCtlColor(owner, (HDC)wParam, brushresult))
                {
                    result = (HRESULT)brushresult;
                    return true;
                }
            }
            break;
        case WM_WINDOWPOSCHANGED:
            newclient = owner->ClientRect();

            //newclient.bottom = max(newclient.bottom,innersize.cy);
            //newclient.right = max(newclient.right,innersize.cx);

            if (!owner->controlstate.contains(csCreatingHandle) && (((owner->OldClientRectangle().Width() != newclient.Width() || owner->OldClientRectangle().Height() != newclient.Height()) /*&& !owner->OldClientRectangle().Empty()*/) /*|| !newclient.Empty()*/))
                owner->LayoutChildren(newclient, false);
            break;
        case WM_ENTERSIZEMOVE:
            // notify children;
            std::for_each(controls.begin(), controls.end(), [](Control *c) {
                if (c->HandleCreated())
                    SendMessage(c->Handle(), WM_ENTERSIZEMOVE, 0, 0);
            });
            break;
        case WM_CAPTURECHANGED:
        case WM_EXITSIZEMOVE:
            std::for_each(controls.begin(), controls.end(), [](Control *c) {
                if (c->HandleCreated())
                    SendMessage(c->Handle(), WM_EXITSIZEMOVE, 0, 0);
            });
            break;
        case wmRequestLayout:
            if (!owner->controlstate.contains(csCreatingHandle) && layoutrequested)
                owner->LayoutChildren(owner->HandleCreated() ? owner->ClientRect() : owner->WindowRect(), wParam == TRUE);
            break;
        case WM_SETREDRAW:
            SetAllowLayout(wParam != FALSE);
            if (wParam != FALSE)
                owner->LayoutChildren(owner->ClientRect(), true);
            break;
        }

        return false;
    }

    void ControlList::Showing()
    {
        CreateChildHandles();
    }

    Control* ControlList::ControlFromHandle(HWND hwnd)
    {
        for (auto it = controls.begin(); it != controls.end(); it++)
            if ((*it)->HandleCreated() && (*it)->Handle() == hwnd)
                return *it;

        Control *r;
        for (auto it = controls.begin(); it != controls.end(); it++)
            if ((*it)->IsControlParent() && (r = (*it)->ControlFromHandle(hwnd)) != NULL)
                return r;
        return NULL;
    }

    Control* ControlList::ControlFromId(int controlid)
    {
        for (auto it = controls.begin(); it != controls.end(); it++)
            if ((*it)->HandleCreated() && (*it)->DlgId() == controlid)
                return *it;

        Control *r;
        for (auto it = controls.begin(); it != controls.end(); it++)
            if ((*it)->IsControlParent() && (r = (*it)->ControlFromId(controlid)) != NULL)
                return r;
        return NULL;
    }

    Control* ControlList::ControlAt(const Point &pt, bool recursive, bool containeronly, bool disabled, bool hidden)
    {
        return ControlAt(pt.x, pt.y, recursive, containeronly, disabled, hidden);
    }

    Control* ControlList::ControlAt(int x, int y, bool recursive, bool containeronly, bool disabled, bool hidden)
    {
        if (!controls.size())
            return NULL;

        for (auto it = controls.rbegin(); it != controls.rend(); ++it)
        {
            Control *c = *it;
            if ((!hidden && !c->Visible()) || (!disabled && !c->Enabled()))
                continue;

            Rect r = c->WindowRect();
            if (PtInRect(&r, Point(x, y)))
            {
                if (!containeronly && !recursive)
                    return c;

                if (c->IsControlParent())
                {
                    if (!recursive)
                        return c;

                    Point pt = c->HandleCreated() ? c->ScreenToClient(owner->ClientToScreen(Point(x, y))) : Point(x - r.left, y - r.top);
                    Rect r2 = c->HandleCreated() ? c->ClientRect() : c->WindowRect();
                    if (pt.x >= 0 && pt.y >= 0 && pt.x < r2.Width() && pt.y < r2.Height())
                    {
                        Control *c2 = c->ControlAt(pt.x, pt.y, recursive, containeronly, disabled, hidden);
                        if (c2)
                            return c2;
                    }
                    return c;
                }
                else if (!containeronly)
                    return c;
            }
        }
        return NULL;
    }

    void ControlList::GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide)
    {
        hw = 0;//max(0,innersize.cx - uw);
        hh = 0;//max(0,innersize.cy - uh);
        if (!hw)
            uw = 0;
        if (!hh)
            uh = 0;
    }


    void ControlList::ClipRegion(HRGN rgn, const Rect &cliprect, bool background /*, const Point &origin*/ )
    {
        if (cliprect.Empty())
            return;

        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            Control *c = (*it);

            if (!c->IsVisible())
                continue;

            Rect r = c->WindowRect()/*.Offset(origin)*/.Intersect(cliprect);
            if (r.Empty() || (c->controlstyle.contains(csTransparent) && !c->IsControlParent()))
                continue;

            if (background || c->controlstyle.contains(csTransparent))
            {
                Point pt = c->WindowToClient(r.TopLeft()); //owner->ScreenToClient(c->ClientToScreen(origin));

                if (c->UsingParentBackground() && !c->controlstyle.contains(csTransparent))
                    CombineRgnWithRect(rgn, rgn, r, rcmOr);
                c->ExcludeClipRegion(rgn, r, pt);
                continue;
            }
            CombineRgnWithRect(rgn, rgn, r, rcmDiff);
        }
    }

    void ControlList::ExcludeChildRegion(HRGN rgn, const Rect &rgnrect, const Point &origin)
    {
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            Control *c = (*it);

            if (!c->IsVisible() || c->controlstyle.contains(csTransparent))
                continue;

            Rect r = c->WindowRect().Offset(origin).Intersect(rgnrect);
            if (r.Empty())
                continue;

            Point pt = owner->ScreenToClient(c->ClientToScreen(0, 0)).Move(origin);
            c->ExcludeClipRegion(rgn, r, pt);
        }
    }

    void ControlList::CreateChildHandles()
    {
        for (auto it = controls.rbegin(); it != controls.rend(); ++it)
        {
            Control *c = *it;
            if (c->Visible())
            {
                if (c->HandleCreated() && GetParent(c->Handle()) != owner->Handle())
                    c->DestroyHandle();
                if (!c->HandleCreated())
                    c->CreateHandle();
                if (c->IsControlParent())
                    c->controls->CreateChildHandles();
            }
        }
    }

    void ControlList::AddChild(Control *control)
    {
        if (!control)
            return;

        if (ChildPosition(control) == controls.end())
        {
            controls.push_back(control);
            try
            {
                control->SetParent(owner);
                if (control->controlstyle.contains(csInTabOrder))
                {
                    tablist.push_back(control);
                    if (tabinited || control->taborder == -1)
                        control->taborder = tablist.size() - 1;
                }
            }
            catch (...)
            {
                controls.pop_back();
            }
        }
    }

    void ControlList::RemoveChild(Control *control)
    {
        std::vector<Control*>::iterator it = ChildPosition(control);
        if (it != controls.end())
        {
            if (control->controlstyle.contains(csInTabOrder))
            {
                auto tabit = std::find(tablist.begin(), tablist.end(), control);
                tabit = tablist.erase(tabit);
                if (tabinited)
                {
                    for ( ; tabit != tablist.end(); ++tabit)
                        (*tabit)->taborder = control->taborder++;
                }
                control->taborder = -1;
            }

            controls.erase(it);
            control->SetParent(NULL);
        }
    }

    void ControlList::ChangeTabOrder(Control *control, int newtaborder)
    {
        if (!tabinited)
        {
            control->taborder = newtaborder;
            return;
        }

        newtaborder = max(0, min(newtaborder, (int)tablist.size() - 1));
        if (newtaborder == control->taborder)
            return;

        int tabdelta = newtaborder - control->taborder;

        auto tabit = tablist.begin() + control->taborder;
        tabit = tablist.erase(tabit);
        if (newtaborder > control->taborder)
        {
            do
            {
                (*tabit)->taborder = control->taborder++;
                ++tabit;
            } while (control->taborder != newtaborder);
        }
        else
        {
            std::vector<Control*>::reverse_iterator rtabit(tabit);
            do
            {
                (*rtabit)->taborder = control->taborder--;
                ++rtabit;
            } while (control->taborder != newtaborder);
            tabit = rtabit.base();
        }
        tablist.insert(tabit, control);

#ifdef DESIGNING
        if (control->HandleCreated())
            control->PassMessage(wmTabOrderChanged, (WPARAM)control, tabdelta);
#endif
    }

    int ControlList::ChildTabOrder(const Control *control)
    {
        if (!control->controlstyle.contains(csChild) || control->parent != owner)
            throw L"The passed control is not on this container.";

        if (!tabinited)
            InitializeTabOrder();

        return control->taborder;
    }

    void ControlList::InitializeTabOrder()
    {
        if (tabinited)
            return;

        int maxtab = -1;
        std::multimap<int, Control*> tabs;
        for (auto it = tablist.begin(); it != tablist.end(); ++it)
        {
            if ((*it)->taborder >= 0)
            {
                maxtab = max((*it)->taborder, maxtab);
                tabs.insert( std::pair<int, Control*>((*it)->taborder, *it) );
            }
        }
        for (auto it = tablist.begin(); it != tablist.end(); ++it)
            if ((*it)->taborder < 0)
                tabs.insert( std::pair<int, Control*>(++maxtab, *it) );

        tablist.clear();
        for (auto it = tabs.begin(); it != tabs.end(); ++it)
            tablist.push_back(it->second);

        UpdateChildTabOrder();

        tabinited = true;
    }

    void ControlList::UpdateChildTabOrder()
    {
        int ix = 0;
        for (auto it = tablist.begin(); it != tablist.end(); ++it, ++ix)
            (*it)->taborder = ix;
    }

    void ControlList::UpdateChildFonts()
    {
        for (auto it = controls.begin(); it != controls.end(); ++it)
            if ((*it)->controlstyle.contains(csChild) && (*it)->controlstyle.contains(csParentFont))
                (*it)->GetFont() = owner->GetFont();
    }

    std::vector<Control*>::iterator ControlList::ChildPosition(Control *control)
    {
        std::vector<Control*>::reverse_iterator rit = std::find(controls.rbegin(), controls.rend(), control);
        if (rit == controls.rend())
            return controls.end();

        return --rit.base();
    }

    void ControlList::MoveAbove(Control *above, Control *below)
    {
        if (!below || !above || above == below)
            return;
        if (above->parent != below->parent)
            throw L"Cannot move control above another if they are not placed on the same control.";

        auto it = std::find(controls.begin(), controls.end(), above);

        if (it == controls.end() || (it != controls.begin() && *(it - 1) == below))
            return;

        controls.erase(it);
        it = controls.insert(std::find(controls.begin(), controls.end(), below) + 1, above) + 1;

        if (above->HandleCreated())
        {
            while (it != controls.end() && (!(*it)->HandleCreated() || !(*it)->Visible()))
                ++it;
            SetWindowPos(above->Handle(), it == controls.end() ? HWND_TOP : (*it)->Handle(), 0, 0, 0, 0, SWP_ZORDERONLY);
        }
    }

    void ControlList::MoveBelow(Control *below, Control *above)
    {
        if (!below || !above || above == below)
            return;
        if (above->parent != below->parent)
            throw L"Cannot move control above another if they are not placed on the same control.";
        auto it = std::find(controls.begin(), controls.end(), below);

        if (it == controls.end() || (it + 1 != controls.end() && *(it + 1) == above))
            return;

        controls.erase(it);
        it = controls.insert(std::find(controls.begin(), controls.end(), above), below) + 1;

        if (below->HandleCreated())
        {
            while (it != controls.end() && (!(*it)->HandleCreated() || !(*it)->Visible()))
                ++it;
            SetWindowPos(below->Handle(), it == controls.end() ? HWND_TOP : (*it)->Handle(), 0, 0, 0, 0, SWP_ZORDERONLY);
        }
    }

    void ControlList::MoveToTop(Control *control)
    {
        auto it = std::find(controls.begin(), controls.end(), control);
        if (it == controls.end() || control == controls.back())
            return;

        controls.erase(it);
        controls.push_back(control);

        if (control->HandleCreated())
            SetWindowPos(control->Handle(), HWND_TOP, 0, 0, 0, 0, SWP_ZORDERONLY);
    }

    void ControlList::MoveToBottom(Control *control)
    {
        auto it = std::find(controls.begin(), controls.end(), control);
        if (it == controls.end() || control == controls.front())
            return;

        controls.erase(it);
        controls.insert(controls.begin(), control);

        if (control->HandleCreated())
            SetWindowPos(control->Handle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_ZORDERONLY);
    }

    void ControlList::MoveUp(Control *control)
    {
        if (!controls.empty() && controls.back() == control)
            return;

        auto it = std::find(controls.begin(), controls.end(), control);
        if (it == controls.end())
            return;

        it = controls.erase(it);
        while (++it != controls.end() && !(*it)->Visible())
            ;
        it = controls.insert(it, control) - 1;

        if (control->HandleCreated())
            SetWindowPos(control->Handle(), control == controls.back() ? HWND_TOP : (*it)->Handle(), 0, 0, 0, 0, SWP_ZORDERONLY);
    }

    void ControlList::MoveDown(Control *control)
    {
        if (controls.front() == control)
            return;

        auto it = std::find(controls.begin(), controls.end(), control);
        if (it == controls.end())
            return;

        it = controls.erase(it);
        while (--it != controls.begin() && !(*it)->Visible())
            ;
        it = controls.insert(it, control) + 1;

        if (control->HandleCreated())
            SetWindowPos(control->Handle(), (*it)->Handle(), 0, 0, 0, 0, SWP_ZORDERONLY);
    }

    void ControlList::FixChildPosition(Control *control)
    {
        if (!control->HandleCreated())
            return;
        auto it = std::find(controls.begin(), controls.end(), control);
        if (it == controls.end())
            return;
        while (++it != controls.end() && !(*it)->Visible())
            ;

        SetWindowPos(control->Handle(), it == controls.end() ? HWND_TOP : (*it)->Handle(), 0, 0, 0, 0, SWP_ZORDERONLY);
    }

    int ControlList::ControlCount()
    {
        return controls.size();
    }

    Control* ControlList::Controls(int ix)
    {
        if (ix >= (int)controls.size())
            throw L"Index out of range in controls list.";

        return controls[ix];
    }

    void ControlList::RequestLayout(bool immediate, bool excludeanchored)
    {
        if (owner->HandleCreated() && (owner->controlstate.contains(csCreatingHandle) || !immediate))
        {
            layoutrequested = true;
            PostMessage(owner->Handle(), wmRequestLayout, excludeanchored ? TRUE : FALSE, 0);
        }
        else if (!owner->controlstate.contains(csCreatingHandle))
            owner->LayoutChildren(owner->HandleCreated() ? owner->ClientRect() : owner->WindowRect(), excludeanchored);
    }

    void ControlList::SetAllowLayout(bool newlayouting)
    {
        if (allowlayout == newlayouting)
            return;
        allowlayout = newlayouting;
        if (allowlayout)
            SaveChildrenAnchorPos();
    }

    bool ControlList::AllowLayout()
    {
        return allowlayout;
    }

    bool ControlList::DeferredPosition::operator< (const DeferredPosition &b) const
    {
        if (b.control->align != control->align || control->align == alClient || control->align == alAnchor)
            return (int)control->align < (int)b.control->align;

        switch (control->align)
        {
            case alTop:
                return control->Top() < b.control->Top();
            break;
            case alBottom:
                return control->Bottom() > b.control->Bottom();
            break;
            case alLeft:
                return control->Left() < b.control->Left();
            break;
            case alRight:
                return control->Right() > b.control->Right();
            break;
            default:
                return false;
        }
    }

    void ControlList::LayoutChildren(Rect newrect, bool excludeanchored)
    {
#ifdef DESIGNING
        if (owner->controlstate.contains(csDeserialize))
            return;
#endif

        layoutrequested = false;
        if (controls.empty() || !allowlayout)
            return;

        allowlayout = false;

        // Commented out when converting ContainerControl to ControlList. If something uses it, find a solution!
        //if (AutoSizeScroll())
        //    OffsetRect(&newrect, -HPos(), -VPos());

        // Space in the control for aligning children. After each children is processed, this rectangle is modified to hold the remaining space.
        Rect alignspace = Rect(newrect.left + padding.left + innerpadding.left, newrect.top + padding.top + innerpadding.top, newrect.right - padding.right - innerpadding.right, newrect.bottom - padding.bottom - innerpadding.bottom);
        Rect fullspace = alignspace;

        alignvector.clear();
        alignvector.reserve(controls.size());

        for (auto it = controls.begin(); it != controls.end(); it++)
        {
            Control *c  = *it;
            if (c->align != alNone && c->Visible() && (c->align != alAnchor || !excludeanchored))
                alignvector.push_back(DeferredPosition(c));
        }

        int defercnt = 0;
        std::sort(alignvector.begin(), alignvector.end());

        // Space in the control left, after all its children aligned to some side are placed.
        Rect minspace = alignspace;

        for (auto pos : alignvector)
        {
            Control *control = pos.control;

            //if ((control->align != alTop || !(owner->alignorder & ControlAlignmentOrderSet(caoTopBeforeLeft | caoTopBeforeRight))) &&
            //    (control->align != alBottom || !(owner->alignorder & ControlAlignmentOrderSet(caoBottomBeforeLeft | caoBottomBeforeRight))) &&
            //    (control->align != alLeft || owner->alignorder < ControlAlignmentOrderSet(caoTopBeforeLeft | caoBottomBeforeLeft)) &&
            //    (control->align != alRight || owner->alignorder < ControlAlignmentOrderSet(caoTopBeforeRight | caoBottomBeforeRight)))
            //    continue;
            if (control->align != alTop && control->align != alBottom && control->align != alLeft && control->align != alRight)
                continue;

            Rect wrect = control->WindowRect();
            switch (control->align)
            {
            case alTop:
                minspace.top += wrect.Height();
                break;
            case alBottom:
                minspace.bottom -= wrect.Height();
                break;
            case alLeft:
                minspace.left += wrect.Width();
                break;
            case alRight:
                minspace.right -= wrect.Width();
                break;
            }
        }

        for (auto &pos : alignvector)
        {
            Control *control = pos.control;
            Rect &bounds = pos.bounds;
            Rect wrect = control->WindowRect();
            int wl, wr, ht, hb;
            float f;

            if (control->HandleCreated())
                defercnt++;

            switch (control->align)
            {
            case alTop:
                bounds = Rect(0,
                              alignspace.top + control->alignmargin.top,
                              0,
                              0);
                if (owner->alignorder.contains(caoTopBeforeLeft))
                    bounds.left = fullspace.left + control->alignmargin.left;
                else
                    bounds.left = minspace.left + control->alignmargin.left;
                if (owner->alignorder.contains(caoTopBeforeRight))
                    bounds.right = max(bounds.left, fullspace.right - control->alignmargin.right);
                else
                    bounds.right = max(bounds.left, minspace.right);
                bounds.bottom = bounds.top + wrect.Height();
                control->ComputeAlignBounds(bounds);
                alignspace.top = bounds.bottom + control->alignmargin.bottom;
                break;
            case alBottom:
                bounds = Rect(0,
                              0,
                              0,
                              alignspace.bottom - control->alignmargin.bottom);
                if (owner->alignorder.contains(caoBottomBeforeLeft))
                    bounds.left = fullspace.left + control->alignmargin.left;
                else
                    bounds.left = minspace.left + control->alignmargin.left;
                if (owner->alignorder.contains(caoBottomBeforeRight))
                    bounds.right = max(bounds.left, fullspace.right - control->alignmargin.right);
                else
                    bounds.right = max(bounds.left, minspace.right);
                bounds.top = bounds.bottom - control->WindowRect().Height();
                control->ComputeAlignBounds(bounds);
                alignspace.bottom = bounds.top - control->alignmargin.top;
                break;
            case alLeft:
                bounds = Rect(alignspace.left + control->alignmargin.left,
                              0,
                              0,
                              0);
                if (!owner->alignorder.contains(caoTopBeforeLeft))
                    bounds.top = fullspace.top + control->alignmargin.top;
                else
                    bounds.top = minspace.top + control->alignmargin.top;
                if (!owner->alignorder.contains(caoBottomBeforeLeft))
                    bounds.bottom = max(bounds.top, fullspace.bottom - control->alignmargin.bottom);
                else
                    bounds.bottom = max(bounds.top, minspace.bottom);
                bounds.right = bounds.left + control->WindowRect().Width();
                control->ComputeAlignBounds(bounds);
                alignspace.left = bounds.right + control->alignmargin.right;
                break;
            case alRight:
                bounds = Rect(0,
                              0,
                              alignspace.right - control->alignmargin.right,
                              0);
                if (!owner->alignorder.contains(caoTopBeforeRight))
                    bounds.top = alignspace.top + control->alignmargin.top;
                else
                    bounds.top = minspace.top + control->alignmargin.top;
                if (!owner->alignorder.contains(caoBottomBeforeRight))
                    bounds.bottom = max(bounds.top, fullspace.bottom - control->alignmargin.bottom);
                else
                    bounds.bottom = max(bounds.top, minspace.bottom);
                bounds.left = bounds.right - control->WindowRect().Width();
                control->ComputeAlignBounds(bounds);
                alignspace.right = bounds.left - control->alignmargin.left;
                break;
            case alClient:
                bounds = Rect(minspace.left + control->alignmargin.left, minspace.top + control->alignmargin.top, minspace.right - control->alignmargin.right, minspace.bottom - control->alignmargin.bottom);
                bounds.right = max(bounds.left, bounds.right);
                bounds.bottom = max(bounds.top, bounds.bottom);
                control->ComputeAlignBounds(bounds);
                break;
            case alAnchor:
                if (control->anchors.contains(caLeft))
                    bounds.left = control->anchorpos.left + newrect.left;
                if (control->anchors.contains(caTop))
                    bounds.top = control->anchorpos.top + newrect.top;
                if (control->anchors.contains(caRight))
                    bounds.right = newrect.right - control->anchorpos.right;
                if (control->anchors.contains(caBottom))
                    bounds.bottom = newrect.bottom - control->anchorpos.bottom;

                if (!control->anchors.contains(caLeft) && control->anchors.contains(caRight))
                    bounds.left = bounds.right - wrect.Width();
                if (!control->anchors.contains(caTop) && control->anchors.contains(caBottom))
                    bounds.top = bounds.bottom - wrect.Height();
                if (!control->anchors.contains(caRight) && control->anchors.contains(caLeft))
                    bounds.right = bounds.left + wrect.Width();
                if (!control->anchors.contains(caBottom) && control->anchors.contains(caTop))
                    bounds.bottom = bounds.top + wrect.Height();

                if (!control->anchors.contains(caLeft) && !control->anchors.contains(caRight))
                {
                    wl = wrect.Width() / 2;
                    wr = wrect.Width() - wl;
                    f = float(control->anchorpos.left + wl) / (control->anchorpos.left + wl + control->anchorpos.right + wr);
                    bounds.left = int(newrect.Width() * f) - wl;
                    bounds.right = bounds.left + wrect.Width();
                }

                if (!control->anchors.contains(caTop) && !control->anchors.contains(caBottom))
                {
                    ht = wrect.Height() / 2;
                    hb = wrect.Height() - ht;
                    f = float(control->anchorpos.top + ht) / (control->anchorpos.top + ht + control->anchorpos.bottom + hb);
                    bounds.top = int(newrect.Height() * f) - ht;
                    bounds.bottom = bounds.top + wrect.Height();
                }

                if (control->anchors.contains(caLeft) && control->anchors.contains(caRight))
                    bounds.right = max(bounds.left, bounds.right);
                if (control->anchors.contains(caTop) && control->anchors.contains(caBottom))
                    bounds.bottom = max(bounds.top, bounds.bottom);

                control->ComputeAlignBounds(bounds);

                break;
            default:
                break;
            }
        }

        if (defercnt > 0)
        {
            HDWP def = NULL;
            try
            {
                def = BeginDeferWindowPos(defercnt);
                if (!def)
                    throw L"Cannot change window positions for an unknown reason.";
                for (auto pos : alignvector)
                {
                    Control *control = pos.control;
                    Rect &bounds = pos.bounds;
                    if (control->HandleCreated() && bounds != control->WindowRect())
                        def = DeferWindowPos(def, control->Handle(), NULL, bounds.left, bounds.top, bounds.Width(), bounds.Height(), SWP_BOUNDS);
                }
            }
            catch(...)
            {
                EndDeferWindowPos(def);
                allowlayout = true;
                throw;
            }
            EndDeferWindowPos(def);
        }
        for (auto pos : alignvector)
        {
            Control *control = pos.control;
            if (!control->HandleCreated())
            {
                //Rect &bounds = pos.bounds;
                control->AnchorSetBounds(pos.bounds);
            }
        }

        allowlayout = true;
    }

    void ControlList::PassMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool withhandle, bool recursive)
    {
        for (auto it = controls.begin(); it != controls.end(); it++)
        {
            Control *c = *it;
            if (withhandle && !c->HandleCreated())
                continue;
            c->PassMessage(uMsg, wParam, lParam);
            if (recursive && c->controls)
                c->controls->PassMessage(uMsg, wParam, lParam, true);
        }
    }

    bool ControlList::SysKeyPressed(WCHAR key)
    {
        for (auto it = controls.begin(); it != controls.end(); it++)
            if ((*it)->SysKeyPressed(key))
                return true;
        return false;
    }

    bool ControlList::HandleDialogKey(WORD vkey)
    {
        for (auto c : controls)
            if (c->PassMessage(wmDialogKey, vkey, 0))
                return true;
        return false;
    }

    void ControlList::BroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        for (auto c : controls)
        {
            if (!c->HandleCreated())
                continue;
            if (c->IsControlParent())
                c->BroadcastMessage(uMsg, wParam, lParam);
            else
                c->PassMessage(uMsg, wParam, lParam);
        }
    }

    Rect ControlList::Padding() const
    {
        return padding;
    }

    void ControlList::SetPadding(const Rect &newpadding)
    {
        if (padding == newpadding)
            return;
        padding = newpadding;

        //if (!owner->controlstate.contains(csCreatingHandle))
        //    owner->LayoutChildren(!owner->HandleCreated() ? owner->WindowRect() : owner->ClientRect(), true);
        RequestLayout(true, true); 
    }

    Rect ControlList::InnerPadding() const
    {
        return innerpadding;
    }

    void ControlList::SetInnerPadding(const Rect &newpadding)
    {
        if (innerpadding == newpadding)
            return;
        innerpadding = newpadding;

        RequestLayout(true, true);
    }

    Control* ControlList::TabFirst()
    {
#ifdef DESIGNING
        if (owner->Designing())
            return NULL;
#endif

        if (!tabinited)
            InitializeTabOrder();

        for (auto it = tablist.begin(); it != tablist.end(); ++it)
        {
            Control *c = *it;

            if (!c->IsVisible() || !c->IsEnabled())
                continue;

            if (c->controlstyle.contains(csInTabOrder) && c->controlstyle.contains(csAcceptInput))
                return c;

            if (c->IsControlParent())
            {
                Control *tf = c->controls->TabFirst();
                if (tf)
                    return tf;
            }
        }
        return NULL;
    }

    Control* ControlList::TabLast()
    {
#ifdef DESIGNING
        if (owner->Designing())
            return NULL;
#endif

        if (!tabinited)
            InitializeTabOrder();

        for (auto rit = tablist.rbegin(); rit != tablist.rend(); ++rit)
        {
            Control *c = *rit;
            if (!c->IsVisible() || !c->IsEnabled())
                continue;

            if (c->IsControlParent())
            {
                Control *tl = c->controls->TabLast();
                if (tl)
                    return tl;
            }

            if (c->controlstyle.contains(csInTabOrder) && c->controlstyle.contains(csAcceptInput))
                return c;
        }
        return NULL;
    }

    Control* ControlList::TabNext(Control *current, bool forward, bool children)
    {
#ifdef DESIGNING
        if (owner->Designing())
            return NULL;
#endif
        if (!tabinited)
            InitializeTabOrder();

        Control *c = NULL;

        bool is = owner->IsVisible() && owner->IsEnabled();

        if (forward && children && is)
        {
            if (current->IsControlParent() && current->Visible() && current->Enabled() && !current->controls->tablist.empty())
                if ((c = current->controls->TabFirst()) != NULL)
                    return c;
        }

        std::vector<Control*>::iterator it = !is ? tablist.end() : std::find(tablist.begin(), tablist.end(), current);
        if (is && it == tablist.end())
            return NULL;

        if (forward)
        {
            while (it != tablist.end())
            {
                while (is && ++it != tablist.end() && (!(*it)->Visible() || !(*it)->Enabled()))
                    ;

                if (it != tablist.end())
                {
                    c = *it;
                    if (c->controlstyle.contains(csAcceptInput) || (children && c->IsControlParent() && (c = c->controls->TabFirst()) != NULL))
                        return c;
                }
            }

            if (owner->Parent())
                return owner->Parent()->controls->TabNext(owner, true, false);

            return TabFirst();
        }
        else
        {
            std::vector<Control*>::reverse_iterator rit(it);
            if (!is)
                rit = tablist.rend();

            while (rit != tablist.rend())
            {
                while(rit != tablist.rend() && (!(*rit)->Visible() || !(*rit)->Enabled()))
                    ++rit;

                if (rit != tablist.rend())
                {
                    c = *rit;
                    if (c->controlstyle.contains(csInTabOrder) && c->IsControlParent() && (c = c->controls->TabLast()) != NULL)
                        return c;

                    c = *rit; // c might have been set to NULL in the previous if.

                    if (c->controlstyle.contains(csAcceptInput))
                        return c;

                    ++rit;
                }
            }

            if (is && owner->controlstyle.contains(csAcceptInput) && owner->controlstyle.contains(csInTabOrder))
                return owner;

            if (owner->Parent())
                return owner->Parent()->controls->TabNext(owner, false, false);

            return TabLast();
        }
    }

    void ControlList::InvalidateBelow(Control *child, Rect r)
    {
        if (!child->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end() || it == controls.begin())
            return;

        Rect cr = owner->ClientRect();
        r = child->ClientToClient(r, owner);
        if (r.right <= 0 || r.bottom <= 0 || r.left >= cr.right || r.top >= cr.bottom)
            return;

        std::vector<Control*>::reverse_iterator rit(it);

        for ( ; rit != controls.rend(); ++rit)
        {
            if (!(*rit)->Visible() || !(*rit)->HandleCreated())
                continue;
            Rect nrc = owner->ClientToClient((*rit)->WindowRect().Intersect(r), *rit);
            if (nrc.Empty())
                continue;
            if ((*rit)->controls != NULL)
                (*rit)->controls->InvalidateRecursively(nrc);
            else
            {
                Rect cr = (*rit)->ClientRect();
                if (nrc.left < 0 || nrc.top < 0 || nrc.right > cr.right || nrc.bottom > cr.bottom)
                    RedrawWindow((*rit)->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
                if (nrc.right > 0 && nrc.bottom > 0 && nrc.left < cr.right && nrc.top < cr.bottom)
                    (*rit)->InvalidateRect(nrc);
            }
        }

        owner->InvalidateRect(r);

        //if (owner->ParentForm())
        //{
        //    r.Move(child->ClientToClient(0, 0, owner->ParentForm()));
        //    RedrawWindow(owner->ParentForm()->Handle(), &r, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        //}
        //else
        //{
        //    r.Move(child->ClientToClient(0, 0, owner));
        //    RedrawWindow(owner->Handle(), &r, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        //}
    }

    void ControlList::InvalidateRecursively(Rect r)
    {
        if (!owner->IsVisible())
            return;

        Rect cr = owner->ClientRect();
        if (r.left < 0 || r.top < 0 || r.right > cr.right || r.bottom > cr.bottom)
            RedrawWindow(owner->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
        if (r.right <= 0 || r.bottom <= 0 || r.left >= cr.right || r.top >= cr.bottom)
            return;

        auto rit = controls.rbegin();

        for ( ; rit != controls.rend(); ++rit)
        {
            if (!(*rit)->Visible())
                continue;
            Rect nrc = owner->ClientToClient((*rit)->WindowRect().Intersect(r), *rit);
            if (nrc.Empty())
                continue;
            if ((*rit)->controls != NULL)
                (*rit)->controls->InvalidateRecursively(nrc);
            else
            {
                Rect cr = (*rit)->ClientRect();
                if (nrc.left < 0 || nrc.top < 0 || nrc.right > cr.right || nrc.bottom > cr.bottom)
                    RedrawWindow((*rit)->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
                if (nrc.right > 0 && nrc.bottom > 0 && nrc.left < cr.right && nrc.top < cr.bottom)
                    (*rit)->InvalidateRect(nrc);
            }
        }

        owner->InvalidateRect(r);
    }

    void ControlList::InvalidateAbove(Control *child, Rect r)
    {
        if (!child->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end())
            return;

        while(++it != controls.end())
        {
            Control *c = *it;
            if (!c->IsVisible() || (!c->controlstyle.contains(csTransparent) && !c->UsingParentBackground()))
                continue;
            Rect r2 = c->WindowRect().Intersect(r);
            if (r2.Empty())
                continue;
            r2.Move(c->ScreenToClient(owner->ClientToScreen(0, 0)));
            //c->InvalidateRect(r2);
            RedrawWindow(c->Handle(), &r2, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        }

        r.Move(owner->ScreenToClient(child->ClientToScreen(0, 0)));
        if (owner->Parent())
            owner->Parent()->controls->InvalidateAbove(owner, r);
    }

    void ControlList::InvalidateChildrenBackground(Rect r)
    {
        if (!owner->IsVisible() || controls.empty())
            return;
        auto it = controls.begin();
        while(it != controls.end())
        {
            Control *c = *it++;
            if (!c->IsVisible() || (!c->controlstyle.contains(csTransparent) && !c->UsingParentBackground()))
                continue;

            Rect r2 = c->WindowRect().Intersect(r);
            if (r2.Empty())
                continue;
            r2.Move(c->ScreenToClient(owner->ClientToScreen(0, 0)));
            Rect ror = c->OpaqueRect();
            if (ror.Empty())
            {
                if (c->controlstyle.contains(csTransparent))
                {
                    r2.Move(owner->ScreenToClient(c->ClientToScreen(0, 0)));
                    RedrawWindow(owner->Handle(), &r2, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                }
                else
                    //RedrawWindow(c->Handle(), &r2, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                    c->InvalidateRect(r2);
            }
            else
            {
                HRGN rgn;
                if (CreateCombinedRgn(rgn, r2, ror, rcmDiff) != NULLREGION)
                {
                    if (c->controlstyle.contains(csTransparent))
                    {
                        OffsetRgn(rgn, owner->ScreenToClient(c->ClientToScreen(0, 0)));
                        RedrawWindow(owner->Handle(), NULL, rgn, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                    }
                    else
                        //RedrawWindow(c->Handle(), NULL, rgn, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                        c->InvalidateRegion(rgn);
                }
                DeleteObject(rgn);
            }
        }
    }

    void ControlList::InvalidateBelow(Control *child, HRGN rgn)
    {
        if (!child->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end() || it == controls.begin())
            return;

        Rect cr = owner->ClientRect();
        Rect wr = child->WindowRect();
        OffsetRgn(rgn, child->ClientToWindow(wr.left, wr.top));
        if (!RgnIntersectsRect(rgn, cr))
            return;

        std::vector<Control*>::reverse_iterator rit(it);

        HRGN wrgn = CreateRectRgn(0, 0, 0, 0);

        for ( ; rit != controls.rend(); ++rit)
        {
            if (!(*rit)->Visible())
                continue;

            wr = (*rit)->WindowRect();
            HRGN nrgn = CreateRectRgnIndirect(wr);
            if (CombineRgnWithRgn(nrgn, nrgn, rgn, rcmAnd) == NULLREGION)
            {
                DeleteObject(nrgn);
                continue;
            }
            OffsetRgn(nrgn, (*rit)->WindowToClient(-wr.left, -wr.top));

            if ((*rit)->controls != NULL)
                (*rit)->controls->InvalidateRecursively(nrgn);
            else
            {
                cr = (*rit)->ClientRect();
                if (CombineRgnWithRect(wrgn, nrgn, cr, rcmDiff) != NULLREGION)
                    RedrawWindow((*rit)->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
                if (RgnIntersectsRect(nrgn, cr))
                    (*rit)->InvalidateRegion(nrgn);
            }

            DeleteObject(nrgn);
        }

        DeleteObject(wrgn);
        owner->InvalidateRegion(rgn);

        //auto rit = std::find(controls.rbegin(), controls.rend(), child);
        //if (rit == controls.rbegin() || rit == controls.rend())
        //    return;

        //HRGN orgn = CreateRgnCopy(rgn);
        //if (owner->ParentForm())
        //{
        //    OffsetRgn(orgn, owner->ParentForm()->ScreenToClient(child->ClientToScreen(0, 0)));
        //    RedrawWindow(owner->ParentForm()->Handle(), NULL, orgn, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        //}
        //else
        //{
        //    OffsetRgn(orgn, owner->ScreenToClient(child->ClientToScreen(0, 0)));
        //    RedrawWindow(owner->Handle(), NULL, orgn, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        //}

        //DeleteObject(orgn);
    }

    void ControlList::InvalidateRecursively(HRGN rgn)
    {
        if (!owner->IsVisible())
            return;

        Rect cr = owner->ClientRect();
        HRGN wrgn = CreateRectRgn(0, 0, 0, 0);
        if (CombineRgnWithRect(wrgn, rgn, cr, rcmDiff) != NULLREGION)
            RedrawWindow(owner->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
        if (RgnIntersectsRect(rgn, cr))
            owner->InvalidateRegion(rgn);

        Rect wr;

        auto rit = controls.rbegin();
        for ( ; rit != controls.rend(); ++rit)
        {
            if (!(*rit)->Visible())
                continue;

            wr = (*rit)->WindowRect();
            HRGN nrgn = CreateRectRgnIndirect(wr);
            if (CombineRgnWithRgn(nrgn, nrgn, rgn, rcmAnd) == NULLREGION)
            {
                DeleteObject(nrgn);
                continue;
            }
            OffsetRgn(nrgn, (*rit)->WindowToClient(-wr.left, -wr.top));

            if ((*rit)->controls != NULL)
                (*rit)->controls->InvalidateRecursively(nrgn);
            else
            {
                cr = (*rit)->ClientRect();
                if (CombineRgnWithRect(wrgn, nrgn, cr, rcmDiff) != NULLREGION)
                    RedrawWindow((*rit)->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
                if (RgnIntersectsRect(nrgn, cr))
                    (*rit)->InvalidateRegion(nrgn);
            }

            DeleteObject(nrgn);

        }

        DeleteObject(wrgn);
        owner->InvalidateRegion(rgn);
    }

    void ControlList::InvalidateAbove(Control *child, HRGN rgn)
    {
        if (!child->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end())
            return;

        HRGN orgn = CreateRgnCopy(rgn);
        OffsetRgn(orgn, owner->ScreenToClient(child->ClientToScreen(0, 0)));

        while(++it != controls.end())
        {
            Control *c = *it;
            if (!c->IsVisible() || (!c->controlstyle.contains(csTransparent) && !c->UsingParentBackground()))
                continue;

            HRGN rgn2 = CreateRectRgn(0, 0, 0, 0);
            if (CombineRgnWithRect(rgn2, orgn, c->WindowRect(), rcmAnd) == NULLREGION)
            {
                DeleteObject(rgn2);
                continue;
            }
            OffsetRgn(rgn2, c->ScreenToClient(owner->ClientToScreen(0, 0)));
            //c->InvalidateRegion(rgn2);
            RedrawWindow(c->Handle(), NULL, rgn2, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            DeleteObject(rgn2);
        }

        if (owner->Parent())
            owner->Parent()->controls->InvalidateAbove(owner, orgn);
        DeleteObject(orgn);
    }

    void ControlList::InvalidateChildrenBackground(HRGN rgn)
    {
        if (!owner->IsVisible() || controls.empty())
            return;
        auto it = controls.begin();
        while(it != controls.end())
        {
            Control *c = *it++;
            if (!c->IsVisible() || (!c->controlstyle.contains(csTransparent) && !c->UsingParentBackground()))
                continue;

            HRGN rgn2 = CreateRectRgn(0, 0, 0, 0);
            if (CombineRgnWithRect(rgn2, rgn, c->WindowRect(), rcmAnd) == NULLREGION)
            {
                DeleteObject(rgn2);
                continue;
            }
            OffsetRgn(rgn2, c->ScreenToClient(owner->ClientToScreen(0, 0)));
            Rect ror = c->OpaqueRect();
            if (ror.Empty())
            {
                if (c->controlstyle.contains(csTransparent))
                {
                    OffsetRgn(rgn2, owner->ScreenToClient(c->ClientToScreen(0, 0)));
                    RedrawWindow(owner->Handle(), NULL, rgn2, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                }
                else
                    //RedrawWindow(c->Handle(), NULL, rgn2, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                    c->InvalidateRegion(rgn2);

            }
            else
            {
                HRGN rgn3 = CreateRectRgn(0, 0, 0, 0);
                if (CombineRgnWithRect(rgn3, rgn2, ror, rcmDiff) != NULLREGION)
                {
                    if (c->controlstyle.contains(csTransparent))
                    {
                        OffsetRgn(rgn3, owner->ScreenToClient(c->ClientToScreen(0, 0)));
                        RedrawWindow(owner->Handle(), NULL, rgn3, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                    }
                    else
                        //RedrawWindow(c->Handle(), NULL, rgn3, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                        c->InvalidateRegion(rgn3);
                }
                DeleteObject(rgn3);
    /*
                HRGN rgn3 = CreateRectRgn(0, 0, 0, 0);
                if (CombineRgnWithRect(rgn3, rgn2, ror, RGN_DIFF) != NULLREGION)
                    RedrawWindow(c->Handle(), NULL, rgn3, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                    //c->InvalidateRegion(rgn3);
                DeleteObject(rgn3);
    */
            }
            //if (c->controls)
            //    c->controls->InvalidateChildrenBackground(rgn2);
            DeleteObject(rgn2);
        }
    }

    void ControlList::InvalidateChildren(bool parentcol)
    {
        if (!owner->IsVisible())
            return;
        Control *c;
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            c = *it;
            if (((parentcol && c->UsingParentColor()) || c->ParentBackground() || c->controlstyle.contains(csTransparent)) && c->Visible())
            {
                c->Invalidate();
                if (c->controls && parentcol)
                    c->controls->InvalidateChildren(parentcol);
            }
        }
    }

    void ControlList::InvalidateRectChildren(const Rect &r, bool parentcol)
    {
        if (!owner->IsVisible())
            return;
        Control *c;
        Rect r2;
        Point p;
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            c = *it;
            if (((parentcol && c->UsingParentColor()) || c->ParentBackground() || c->controlstyle.contains(csTransparent)) && c->Visible())
            {
                r2 = c->ClientToClient(c->ClientRect(), owner);
                p = r2.TopLeft();
                r2 = r.Intersect(r2).Offset(-p.x, -p.y);
                if (r2.Empty())
                    continue;
                RedrawWindow(c->Handle(), &r2, NULL, RDW_ERASE | RDW_INVALIDATE);
                if (c->controls && parentcol)
                    c->controls->InvalidateRectChildren(r2, parentcol);
            }
        }
    }

    void ControlList::InvalidateRegionChildren(HRGN rgn, bool parentcol)
    {
        if (!owner->IsVisible())
            return;
        Control *c;
        Rect r2;
        Point p;
        HRGN rgn2;
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            c = *it;
            if (((parentcol && c->UsingParentColor()) || c->ParentBackground() || c->controlstyle.contains(csTransparent)) && c->Visible())
            {
                r2 = c->ClientToClient(c->ClientRect(), owner);
                p = r2.TopLeft();
                rgn2 = CreateRectRgnIndirect(&r2);
                if (CombineRgn(rgn2, rgn2, rgn, RGN_AND) == NULLREGION)
                {
                    DeleteObject(rgn2);
                    continue;
                }
                OffsetRgn(rgn2, -p.x, -p.y);
                RedrawWindow(c->Handle(), NULL, rgn2, RDW_ERASE | RDW_INVALIDATE);
                if (c->controls && parentcol)
                    c->controls->InvalidateRegionChildren(rgn2, parentcol);
                DeleteObject(rgn2);
            }
        }
    }

    void ControlList::InvalidateTransparentAbove(Control *child, Rect childclient)
    {
        if (!owner->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end())
            return;

        childclient = child->ClientToClient(childclient, owner);

        if (it != controls.end() && ++it != controls.end())
        {
            for ( ; it != controls.end(); ++it)
            {
                Control *c = *it;
                Rect r;
                if (!c->controlstyle.contains(csTransparent) || !c->Visible())
                    continue;

                Rect cwr = c->WindowRect();
                r = cwr.Intersect(childclient);

                if (r.Empty())
                    continue;

                Point p = c->ClientToWindow(0, 0);
                r.Move(-cwr.left - p.x, -cwr.top - p.y);
                c->InvalidateRect(r);
            }
        }

        if (owner->Parent())
            owner->Parent()->controls->InvalidateTransparentAbove(owner, childclient);
    }

    void ControlList::InvalidateTransparentAbove(Control *child, HRGN childclient)
    {
        if (!owner->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end())
            return;

        Rect cwr = child->WindowRect();
        Point p = child->WindowToClient(0, 0);
        OffsetRgn(childclient, cwr.left + p.x, cwr.top + p.y);

        HRGN rgn = CreateRectRgn(0, 0, 0, 0);
        if (it != controls.end() && ++it != controls.end())
        {

            for ( ; it != controls.end(); ++it)
            {
                Control *c = *it;
                if (!c->controlstyle.contains(csTransparent) || !c->Visible())
                    continue;

                cwr = c->WindowRect();
                if (CombineRgnWithRect(rgn, childclient, cwr, rcmAnd) == NULLREGION)
                    continue;

                p = c->ClientToWindow(0, 0);
                OffsetRgn(rgn, -cwr.left - p.x, -cwr.top - p.y);
                c->InvalidateRegion(rgn);
            }

        }
        DeleteObject(rgn);

        if (owner->Parent())
            owner->Parent()->controls->InvalidateTransparentAbove(owner, childclient);
    }

    void ControlList::InvalidateBelowTransparentAbove(Control *child, Rect childclient, const Point &diff)
    {
        if (!owner->IsVisible())
            return;

        auto it = std::find(controls.begin(), controls.end(), child);
        if (it == controls.end())
            return;

        childclient = child->ClientToClient(childclient, owner);

        if (it != controls.end() && ++it != controls.end())
        {
            for ( ; it != controls.end(); ++it)
            {
                Control *c = *it;
                Rect r;
                if (!c->controlstyle.contains(csTransparent) || !c->Visible())
                    continue;

                Rect cwr = c->WindowRect();
                r = cwr.Offset(diff).Intersect(childclient);

                if (r.Empty())
                    continue;

                Point p = c->ClientToWindow(0, 0);
                r.Move(-cwr.left - p.x, -cwr.top - p.y);

                InvalidateBelow(c, r);
            }
        }

        if (owner->Parent())
            owner->Parent()->controls->InvalidateBelowTransparentAbove(owner, childclient, diff);
    }


    //---------------------------------------------


}
/* End of NLIBNS */


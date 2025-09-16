#include "stdafx_zoli.h"
#include "application.h"
#include "syscontrol.h"
#include "canvas.h"
#include "imagelist.h"
#include "themes.h"
#ifdef DESIGNING
#include "designproperties.h"
#include "serializer.h"
#include "designer.h"
//#include "designercontrols.h"
#include "property_controlbase.h"
#include "property_syscontrol.h"
#include "property_imagelist.h"
#include "property_objectbase.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING
    void SystemControl::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
    }
#endif

    std::map<COLORREF, SystemControl::BrushData> SystemControl::bgbrushes;
    SystemControl::BrushData::BrushData() : brush(NULL), usage(0)
    {
    }

    SystemControl::BrushData::BrushData(HBRUSH brush, int usage) : brush(brush), usage(usage)
    {
    }

    SystemControl::SystemControl() : builtinwndproc(NULL), brushcolor(clNone)
    {
        controlstyle -= csSelfDrawn;
        controlstyle -= csMouseCapture;
        controlstyle -= csNoDefaultPaint;
        controlstyle -= csEraseOnTextChange;
    }

    SystemControl::~SystemControl()
    {
        if (brushcolor != clNone)
            UnuseBrush(brushcolor);
        brushcolor = clNone;
    }


    void SystemControl::UseBrush(COLORREF color) /* static */
    {
        auto it = bgbrushes.find(color);
        if (it != bgbrushes.end() && it->first == color)
        {
            ++it->second.usage;
            return;
        }
        HBRUSH brush = CreateSolidBrush(color);
        bgbrushes[color] = BrushData(brush, 1);
    }

    void SystemControl::UnuseBrush(COLORREF color) /* static */
    {
        auto it = bgbrushes.find(color);
        if (it == bgbrushes.end() || it->first != color)
            return;
        if (--it->second.usage > 0)
            return;
        DeleteObject(it->second.brush);
        bgbrushes.erase(it);
    }

    HBRUSH SystemControl::GetBrush(COLORREF color) /* static */
    {
        auto it = bgbrushes.find(color);
        if (it == bgbrushes.end() || it->first != color)
            return NULL;
        return bgbrushes[color].brush;
    }

    void SystemControl::DeleteBrushes() /* static */
    {
        for (auto it = bgbrushes.begin(); it != bgbrushes.end(); ++it)
            DeleteObject(it->second.brush);
        bgbrushes.clear();
    }

    HBRUSH SystemControl::CtlBrush()
    {
        if (brushcolor == clNone)
            return NULL;
        return GetBrush(brushcolor);
    }

    LRESULT SystemControl::CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (HandleCreated())
            return builtinwndproc(Handle(), uMsg, wParam, lParam);
        return 0;
    }

    LRESULT SystemControl::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Color col;
        switch (uMsg)
        {
        case wmSysColorChanged:
            brushcolor = GetColor();
            UseBrush(brushcolor);
            break;
        case wmColorChanged:
            col = GetColor();
            if (brushcolor == col)
                break;
            if (brushcolor != clNone)
                UnuseBrush(brushcolor);
            brushcolor = col;
            UseBrush(brushcolor);
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void SystemControl::CreateClassParams(ClassParams &params)
    {
        if (params.classname.length())
        {
            WNDCLASSEX classex = {0};
            classex.cbSize = sizeof(WNDCLASSEXW);
            classex.hInstance = hInstance;
            if (GetClassInfoEx(hInstance, params.classname.c_str(), &classex))
            {
                params.classname = std::wstring();
                params.brush = classex.hbrBackground;
                params.cursor = classex.hCursor;
                params.icon = classex.hIcon;
                params.iconsm = classex.hIconSm;
                params.style = classex.style;
                params.wndextra = classex.cbWndExtra;
                params.wndproc = classex.lpfnWndProc;
                builtinwndproc = params.wndproc;
                return;
            }
        }
        base::CreateClassParams(params);
    }

    void SystemControl::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);

        if (WindowRect() == Rect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT))
        {
            params.x = 0;
            params.y = 0;
            params.width = 80;
            params.height = 25;
        }

    }

    void SystemControl::InitHandle()
    {
        builtinwndproc = GetBuiltinWndProc();
        base::InitHandle();
        if (!builtinwndproc)
            builtinwndproc = ReplaceWndProc(Handle(), &AppWndProc);
    }

    bool SystemControl::HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush)
    {
        if (brushcolor == clNone)
            return false;
        SetBkColor(dc, GetColor());
        SetTextColor(dc, GetFont().GetColor());
        bgbrush = GetBrush(brushcolor);
        return true;
    }

    bool SystemControl::HandleSysKey(WCHAR key)
    {
        if (Parent())
        {
            std::wstring t = Text();
            if (ContainsAccelerator(t, key))
            {
                Parent()->PassMessage(WM_COMMAND, 0, (LPARAM)Handle());
                return true;
            }
        }
        return base::HandleSysKey(key);
    }


    //---------------------------------------------


#ifdef DESIGNING
    ValuePair<CheckboxStates> CheckboxStateStrings[] = {
        VALUEPAIR(csUnchecked),
        VALUEPAIR(csChecked),
        VALUEPAIR(csIndeterminate),
    };

    void Checkbox::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"BorderStyle");
        serializer->Find<ColorDesignProperty<Control>>(L"Color")->SetDefault(clBtnFace);
        //serializer->Find<BoolDesignProperty<Control>>(L"ParentBackground")->SetDefault(true);
        serializer->Find<BoolDesignProperty<Control>>(L"ParentColor")->SetDefault(true);

        serializer->Add(L"SetChecked", new BoolDesignProperty<Checkbox>(L"Checked", L"Behavior", &Checkbox::Checked, &Checkbox::SetChecked))->SetDefault(false)->DontWrite();
        serializer->Add(L"SetTextLeft", new BoolDesignProperty<Checkbox>(L"TextLeft", L"Behavior", &Checkbox::TextLeft, &Checkbox::SetTextLeft))->SetDefault(false);
        serializer->Add(L"SetState", new CheckboxStatesDesignProperty<Checkbox>(L"State", L"Behavior", &Checkbox::State, &Checkbox::SetState))->SetDefault(csUnchecked);

        serializer->AddEvent<Checkbox, NotifyEvent>(L"OnClick", L"Control");
        serializer->AddEvent<Checkbox, CheckboxCheckEvent>(L"OnCheck", L"Control");
    }

    Size Checkbox::DesignSize()
    {
        return Size(80, max(13, BaseToPixel(0, 10).cy) );
    }

#endif
    Checkbox::Checkbox() : textleft(false), state(csUnchecked)
    {
        controlstyle << csWantSysKey << csInTabOrder << csAcceptInput << csForceBgColor;
        SetColor(clBtnFace);
        SetParentColor(true);
        SetParentBackground(true);
    }

    Checkbox::~Checkbox()
    {
    }

    void Checkbox::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Button";
    }

    void Checkbox::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << bs3State;
        if (textleft)
            params.style << bsLeftText << bsRight;
    }

    void Checkbox::InitHandle()
    {
        base::InitHandle();
        if (state == csUnchecked)
            return;
        UpdateState();
    }

    void Checkbox::SaveWindow()
    {
        LPARAM ckstate = SendMessage(Handle(), BM_GETSTATE, 0, 0);
        state = (ckstate & BST_INDETERMINATE) == BST_INDETERMINATE ? csIndeterminate : (ckstate & BST_CHECKED) == BST_CHECKED ? csChecked : csUnchecked;
        base::SaveWindow();
    }

    bool Checkbox::HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush)
    {
        if (!UsingParentBackground())
        {
            SetBkColor(dc, GetColor());
            bgbrush = CtlBrush();
            return true;
        }
        //else
        //{
        //    SetBkMode(dc, TRANSPARENT);
        //    SetBkColor(dc, RGB(0, 0, 0));
        //    SetTextColor(dc, RGB(255, 255, 255));
        //    bgbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        //    return true;
        //}
        return false;
    }

    void Checkbox::UpdateState()
    {
        if (!HandleCreated())
            return;

        SendMessage(Handle(), BM_SETCHECK, state == csUnchecked ? BST_UNCHECKED : state == csChecked ? BST_CHECKED : BST_INDETERMINATE, 0);
    }

    void Checkbox::Restyle()
    {
        if (!HandleCreated())
            return;

        WindowStyleSet style;
        style << bs3State;
        if (textleft)
            style << bsLeftText << bsRight;
        SendMessage(Handle(), BM_SETSTYLE, (WPARAM)style, TRUE);
    }

    bool Checkbox::HandleCommand(Control *parent, WPARAM wParam)
    {
        if (((wParam >> 16) & BN_CLICKED) == BN_CLICKED)
        {
            Click();
            return true;
        }
        return false;
    }

    void Checkbox::Click()
    {
        CheckboxStates newstate = state == csUnchecked || state == csIndeterminate ? csChecked : csUnchecked;
        if (OnCheck)
            OnCheck(this, CheckboxCheckParameters(newstate));

        state = newstate;
        UpdateState();
        if (OnClick)
            OnClick(this, EventParameters());
    }

    bool Checkbox::TextLeft()
    {
        return textleft;
    }

    void Checkbox::SetTextLeft(bool newtextleft)
    {
        if (textleft == newtextleft)
            return;
        textleft = newtextleft;
        Restyle();
    }

    bool Checkbox::Checked()
    {
        if (HandleCreated())
        {
            LPARAM ckstate = SendMessage(Handle(), BM_GETSTATE, 0, 0);
            state = (ckstate & BST_INDETERMINATE) == BST_INDETERMINATE ? csIndeterminate : (ckstate & BST_CHECKED) == BST_CHECKED ? csChecked : csUnchecked;
        }
        return state == csChecked;
    }

    void Checkbox::SetChecked(bool newchecked)
    {
        if ((newchecked && state == csChecked) || (!newchecked && state == csUnchecked))
            return;
        state = newchecked ? csChecked : csUnchecked;
        UpdateState();

#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"State");
#endif
    }

    CheckboxStates Checkbox::State()
    {
        return state;
    }

    void Checkbox::SetState(CheckboxStates newstate)
    {
        if (state == newstate)
            return;
        state = newstate;
        UpdateState();

#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Checked");
#endif
    }


    //---------------------------------------------

#ifdef DESIGNING
    void Radiobox::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"BorderStyle");
        serializer->Find<ColorDesignProperty<Control>>(L"Color")->SetDefault(clBtnFace);
        //serializer->Find<BoolDesignProperty<Control>>(L"ParentBackground")->SetDefault(true);
        serializer->Find<BoolDesignProperty<Control>>(L"ParentColor")->SetDefault(true);

        serializer->Add(L"SetChecked", new BoolDesignProperty<Radiobox>(L"Checked", L"Behavior", &Radiobox::Checked, &Radiobox::SetChecked))->SetDefault(false);
        serializer->Add(L"SetTextLeft", new BoolDesignProperty<Radiobox>(L"TextLeft", L"Behavior", &Radiobox::TextLeft, &Radiobox::SetTextLeft))->SetDefault(false);

        serializer->AddEvent<Radiobox, NotifyEvent>(L"OnClick", L"Control");
        serializer->AddEvent<Radiobox, RadioboxCheckEvent>(L"OnCheck", L"Control");
    }

    Size Radiobox::DesignSize()
    {
        return Size(80, max(13, BaseToPixel(0, 10).cy) );
    }

#endif
    Radiobox::Radiobox() : textleft(false), checked(false)
    {
        controlstyle << csWantSysKey << csInTabOrder << csAcceptInput << csForceBgColor;
        SetColor(clBtnFace);
        SetParentColor(true);
        SetParentBackground(true);
    }

    Radiobox::~Radiobox()
    {
    }

    void Radiobox::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Button";
    }

    void Radiobox::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << bsRadiobutton;
        if (textleft)
            params.style << bsLeftText << bsRight;
    }

    void Radiobox::InitHandle()
    {
        base::InitHandle();
        if (checked)
            UpdateState();
    }

    void Radiobox::SaveWindow()
    {
        checked = (SendMessage(Handle(), BM_GETCHECK, 0, 0) & BST_CHECKED) == BST_CHECKED;
        base::SaveWindow();
    }

    bool Radiobox::HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush)
    {
        if (!UsingParentBackground())
        {
            SetBkColor(dc, GetColor());
            //if (ctlbrush)
            //{
            //    LOGBRUSH lb = {0};
            //    GetObject(ctlbrush, sizeof(LOGBRUSH), &lb);
            //    if (lb.lbColor != GetColor())
            //    {
            //        DeleteObject(ctlbrush);
            //        ctlbrush = NULL;
            //    }
            //}

            //if (!ctlbrush)
            //    ctlbrush = CreateSolidBrush(GetColor());
            //bgbrush = ctlbrush;
            bgbrush = CtlBrush();
            return true;
        }
        return false;
    }

    void Radiobox::UpdateState()
    {
        if (!HandleCreated())
            return;

        SendMessage(Handle(), BM_SETCHECK, !checked ? BST_UNCHECKED : BST_CHECKED, 0);
    }

    void Radiobox::Restyle()
    {
        if (!HandleCreated())
            return;

        WindowStyleSet style;
        style << bsAutoRadiobutton;
        if (textleft)
            style << bsLeftText << bsRight;
        SendMessage(Handle(), BM_SETSTYLE, (WPARAM)style, TRUE);
    }

    bool Radiobox::HandleCommand(Control *parent, WPARAM wParam)
    {
        if (((wParam >> 16) & BN_CLICKED) == BN_CLICKED)
        {
            Click();
            return true;
        }
        return false;
    }

    void Radiobox::Click()
    {
        if (!checked)
        {
            bool docheck = true;
            if (OnCheck)
                OnCheck(this, RadioboxCheckParameters(docheck));
            if (docheck)
                SetChecked(true);
        }

        if (OnClick)
            OnClick(this, EventParameters());
    }

    bool Radiobox::TextLeft()
    {
        return textleft;
    }

    void Radiobox::SetTextLeft(bool newtextleft)
    {
        if (textleft == newtextleft)
            return;
        textleft = newtextleft;
        Restyle();
    }

    bool Radiobox::Checked()
    {
        if (HandleCreated())
            checked = (SendMessage(Handle(), BM_GETCHECK, 0, 0) & BST_CHECKED) == BST_CHECKED;
        return checked;
    }

    void Radiobox::SetChecked(bool newchecked)
    {
        if (newchecked == checked)
            return;
        checked = newchecked;

        UpdateState();
        if (checked && Parent())
        {
            int cnt = Parent()->ControlCount();
            for (int ix = 0; ix < cnt; ++ix)
            {
                Control *c = Parent()->Controls(ix);
                if (c == this || dynamic_cast<Radiobox*>(c) == NULL)
                    continue;
                ((Radiobox*)(c))->SetChecked(false);
            }
        }
    }


    //---------------------------------------------

#ifdef DESIGNING
    void Groupbox::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->SetContainerControl(true);
        serializer->HideProperty(L"AcceptInput");
        serializer->Find<RectDesignProperty<Control>>(L"Padding")->SetDefault(Rect(8 * Scaling, 16 * Scaling, 8 * Scaling, 8 * Scaling));
        //serializer->HideProperty(L"TabOrder");
    }
#endif

    Groupbox::Groupbox()
    {
        controlstyle << csWantSysKey << csInTabOrder << csForceBgColor;
        SetPadding(Rect(8 * Scaling, 16 * Scaling, 8 * Scaling, 8 * Scaling));
        SetColor(clBtnFace);
        SetParentColor(true);
        InitControlList();
    }

    Groupbox::~Groupbox()
    {
    }

    void Groupbox::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Button";
    }

    void Groupbox::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << bsGroupbox;
    }

    //bool Groupbox::EraseBackground()
    //{
    //
    //}

    void Groupbox::Restyle()
    {
        if (!HandleCreated())
            return;

        WindowStyleSet style;
        //style << bsAutoRadiobutton;
        //if (textleft)
        //    style << bsLeftText << bsRight;
        SendMessage(Handle(), BM_SETSTYLE, (WPARAM)style, TRUE);
    }


    //---------------------------------------------


#ifdef DESIGNING
    void Edit::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"ParentBackground");
        serializer->HideProperty(L"BorderStyle");
        serializer->Find<BoolDesignProperty<Control>>(L"ParentColor")->SetDefault(false);
        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);
        serializer->MakeDefault(L"Text");

        serializer->Add(L"SetMaxLength", new UnsignedIntDesignProperty<Edit>(L"MaxLength", L"Behavior", &Edit::MaxLength, &Edit::SetMaxLength))->SetDefault(0);
        serializer->Add(L"SetReadOnly", new BoolDesignProperty<Edit>(L"ReadOnly", L"Behavior", &Edit::ReadOnly, &Edit::SetReadOnly))->SetDefault(false);
        serializer->Add(L"SetAutoSelect", new BoolDesignProperty<Edit>(L"AutoSelect", L"Behavior", &Edit::AutoSelect, &Edit::SetAutoSelect))->SetDefault(true);
        serializer->Add(L"SetLeftMargin", new IntDesignProperty<Edit>(L"LeftMargin", L"Appearance", &Edit::LeftMargin, &Edit::SetLeftMargin))->SetDefault(3 * Scaling);
        serializer->Add(L"SetRightMargin", new IntDesignProperty<Edit>(L"RightMargin", L"Appearance", &Edit::RightMargin, &Edit::SetRightMargin))->SetDefault(3 * Scaling);
        serializer->AddEvent<Edit, NotifyEvent>(L"OnTextChanged", L"Control");
    }

    Size Edit::DesignSize()
    {
        return Size(80, CurrentFontHeight() + max(themes->MeasureEditBorderWidth().cy * 2, BaseToPixel(0, 5).cy) );
    }

#endif

    Edit::Edit() : 
            maxlength(0), readonly(false), autoselect(true), doautosel(false), mouseselpt(0,0),
            leftmargin(3 * Scaling), rightmargin(3 * Scaling), selstart(0), sellength(0), modified(false)
    {
        SetBorderStyle(bsNormal);
        controlstyle << csInTabOrder << csAcceptInput;
        SetParentBackground(false);
        SetParentColor(false);
        SetWantedKeyTypes(wkOthers | wkArrows);
    }

    Edit::~Edit()
    {
    }

    void Edit::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Edit";
    }

    void Edit::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << esLeft << esAutoHScroll;
        if (readonly)
            params.style << esReadonly;
    }

    LRESULT Edit::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
        case WM_SETFONT:
            if (leftmargin >= 0 || rightmargin >= 0)
                PostMessage(Handle(), EM_SETMARGINS, (leftmargin ? EC_LEFTMARGIN : 0) | (rightmargin >= 0 ? EC_RIGHTMARGIN : 0), (leftmargin >= 0 ? (leftmargin & 0xffff) : 0) | (rightmargin >= 0 ? ((rightmargin & 0xffff) << 16) : 0));
            break;
        case WM_MOUSEACTIVATE:
            if (autoselect && (lParam >> 16) == WM_LBUTTONDOWN && !controlstate.contains(csFocused))
                doautosel = true;
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool Edit::HandleCommand(Control *parent, WPARAM wParam)
    {
        if (HIWORD(wParam) == EN_UPDATE)
        {
            modified = true;
            if (OnTextChanged)
                OnTextChanged(this, EventParameters());
        }
        return true;
    }

    void Edit::InitHandle()
    {
        base::InitHandle();
        SendMessage(Handle(),EM_SETLIMITTEXT,maxlength,0);

        SetSelStartAndLength(selstart,sellength);
        if (leftmargin >= 0 || rightmargin >= 0)
            SendMessage(Handle(), EM_SETMARGINS, (leftmargin ? EC_LEFTMARGIN : 0) | (rightmargin >= 0 ? EC_RIGHTMARGIN : 0), (leftmargin >= 0 ? (leftmargin & 0xffff) : 0) | (rightmargin >= 0 ? ((rightmargin & 0xffff) << 16) : 0));
    }

    void Edit::SaveWindow()
    {
        _Margins();
        _SelStartAndLength();
        base::SaveWindow();
    }

    void Edit::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
    {
        if (vkeys.contains(vksLeft))
        {
            int xd = abs(GetSystemMetrics(SM_CXDRAG));
            int yd = abs(GetSystemMetrics(SM_CYDRAG));
            if (abs(mouseselpt.x - x) > xd || abs(mouseselpt.y - y) > yd)
                doautosel = false;
        }

        base::MouseMove(x, y, vkeys);
    }

    void Edit::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (button == mbLeft)
            mouseselpt = Point(x, y);

        base::MouseDown(x, y, button, vkeys);
    }

    void Edit::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (button == mbLeft && doautosel)
        {
            SetSelStartAndLength(0, Text().size());
            doautosel = false;
        }

        base::MouseUp(x, y, button, vkeys);
    }

    unsigned int Edit::MaxLength()  const
    {
        return maxlength;
    }

    void Edit::SetMaxLength(unsigned int newmax)
    {
        if (maxlength == newmax)
            return;
        maxlength = newmax;
        if (HandleCreated())
            SendMessage(Handle(), EM_SETLIMITTEXT, maxlength, 0);
    }

    bool Edit::ReadOnly()
    {
        if (HandleCreated())
            readonly = (GetWindowLongPtr(Handle(), GWL_STYLE) & esReadonly) == esReadonly;
        return readonly;
    }

    void Edit::SetReadOnly(bool newreadonly)
    {
        if (readonly == newreadonly)
            return;
        readonly = newreadonly;
        if (HandleCreated())
        {
            if (!SendMessage(Handle(), EM_SETREADONLY, readonly ? TRUE : FALSE,0))
            {
                readonly = !readonly;
                throw L"Couldn't set the edit box's readonly parameter";
            }
        }
    }

    int Edit::SelStart()
    {
        _SelStartAndLength();
        return selstart;
    }

    int Edit::SelLength()
    {
        _SelStartAndLength();
        return sellength;
    }

    void Edit::_SelStartAndLength()
    {
        if (HandleCreated())
        {
            SendMessage(Handle(), EM_GETSEL, (WPARAM)&selstart, (LPARAM)&sellength);
            sellength = sellength - selstart;
        }
    }

    void Edit::SelStartAndLength(int &start, int &length)
    {
        _SelStartAndLength();
        start = selstart;
        length = sellength;
    }

    void Edit::SetSelStartAndLength(int start, int length)
    {
        _SelStartAndLength();

        int len = TextLength();
        if (start < 0)
            start = 0;
        else if (start > len)
            start = len;
        length = min(start+length,len) - start;

        if (start == selstart && length == sellength)
            return;

        selstart = start;
        sellength = length;
        if (HandleCreated())
        {
            length += start;
            SendMessage(Handle(), EM_SETSEL, (WPARAM)selstart, (WPARAM)length);
    /*        sellength += length;
            SendMessage(Handle(), EM_SETSEL, (WPARAM)selstart, (WPARAM)sellength);
            sellength -= length;
    */
        }
    }

    void Edit::SetSelStart(int newstart)
    {
        _SelStartAndLength();
        int len = TextLength();
        if (newstart < 0)
            newstart = 0;
        else if (newstart > len)
            newstart = len;
        if (selstart == newstart)
            return;

        selstart = newstart;
        if (HandleCreated())
        {
            sellength = newstart;
            SendMessage(Handle(),EM_SETSEL,(WPARAM)selstart,(WPARAM)sellength);
        }
        sellength = 0;
    }

    void Edit::SetSelLength(int newlength)
    {
        _SelStartAndLength();
        int len = TextLength();
        if (newlength < 0)
            newlength = 0;
        if (selstart+newlength > len)
            newlength = len-selstart;
        if (newlength == sellength)
            return;
        if (HandleCreated())
        {
            sellength = selstart + newlength;
            SendMessage(Handle(), EM_SETSEL, (WPARAM)selstart, (WPARAM)sellength);
        }
        sellength = newlength;
    }

    std::wstring Edit::SelText()
    {
        _SelStartAndLength();
        if (sellength == 0)
            return L"";
        return Text().substr(selstart,sellength);
    }

    void Edit::SetSelText(const std::wstring& newseltext)
    {
        if (!HandleCreated())
        {
            _SelStartAndLength();
            std::wstring text = Text();
            text.replace(selstart, sellength, newseltext);
            selstart += newseltext.length();
            sellength = 0;
            SetText(text);
            return;
        }

        SendMessage(Handle(),EM_REPLACESEL,TRUE,(LPARAM)newseltext.c_str());
    }

    void Edit::AddText(const std::wstring& str)
    {
        SetSelStart(TextLength());
        SetSelText(str);
    }

    bool Edit::AutoSelect()
    {
        return autoselect;
    }

    void Edit::SetAutoSelect(bool newautoselect)
    {
        autoselect = newautoselect;
    }

    void Edit::_Margins()
    {
        if (HandleCreated())
        {
            LRESULT r = SendMessage(Handle(), EM_GETMARGINS, 0, 0);
            leftmargin = short(r & 0xffff);
            rightmargin = short((r >> 16) & 0xffff);
        }
    }

    int Edit::LeftMargin()
    {
        _Margins();
        return leftmargin;
    }

    int Edit::RightMargin()
    {
        _Margins();
        return rightmargin;
    }

    void Edit::SetLeftMargin(int newleft)
    {
        newleft = min(65535, max(0, newleft));
        if (leftmargin == newleft)
            return;
        leftmargin = newleft;
        if (HandleCreated())
            SendMessage(Handle(), EM_SETMARGINS, EC_LEFTMARGIN, newleft & 0xffff);
    }

    void Edit::SetRightMargin(int newright)
    {
        newright = min(65535, max(0, newright));
        if (rightmargin == newright)
            return;
        rightmargin = newright;
        if (HandleCreated())
            SendMessage(Handle(), EM_SETMARGINS, EC_RIGHTMARGIN, (newright & 0xffff) << 16);
    }

    void Edit::GetMargins(int &left, int &right)
    {
        _Margins();

        left = leftmargin;
        right = rightmargin;
    }

    void Edit::SetMargins(int newleft, int newright)
    {
        newright = min(65535, max(0, newright));
        newleft = min(65535, max(0, newleft));
        if (leftmargin == newleft && rightmargin == newright)
            return;
        rightmargin = newright;
        leftmargin = newleft;
        if (HandleCreated())
            SendMessage(Handle(), EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, (leftmargin & 0xffff) | ((rightmargin & 0xffff) << 16) );
    }

    bool Edit::Modified()
    {
        return modified;
    }

    void Edit::SetModified(bool newmodified)
    {
        modified = newmodified;
    }


    //---------------------------------------------

    ControlEdit::ControlEdit() :
            topmargin(0), lasttick(0), edit(new Edit()), OnTextChanged(edit->OnTextChanged), OnCaptureLost(edit->OnCaptureLost), OnMouseEnter(edit->OnMouseEnter), OnMouseLeave(edit->OnMouseLeave),
            OnNCMouseEnter(edit->OnNCMouseEnter), OnNCMouseLeave(edit->OnNCMouseLeave), OnMouseEntered(edit->OnMouseEntered), OnMouseLeft(edit->OnMouseLeft), OnGainFocus(edit->OnGainFocus),
            OnLoseFocus(edit->OnLoseFocus), OnEnter(edit->OnEnter), OnLeave(edit->OnLeave), OnKeyDown(edit->OnKeyDown), OnKeyUp(edit->OnKeyUp), OnKeyPress(edit->OnKeyPress), OnKeyPush(edit->OnKeyPush),
            OnMouseMove(edit->OnMouseMove), OnMouseDown(edit->OnMouseDown), OnMouseUp(edit->OnMouseUp), OnDblClick(edit->OnDblClick), OnNCMouseMove(edit->OnNCMouseMove),
            OnNCMouseDown(edit->OnNCMouseDown), OnNCMouseUp(edit->OnNCMouseUp), OnNCDblClick(edit->OnNCDblClick), OnStartSizeMove(edit->OnStartSizeMove), OnSizeChanged(edit->OnSizeChanged),
            OnPositionChanged(edit->OnPositionChanged), OnEndSizeMove(edit->OnEndSizeMove), OnResize(edit->OnResize), OnMove(edit->OnMove), OnDialogCode(edit->OnDialogCode)
    {
        InitControlList();
        SetBorderStyle(bsNone);
        SetCursor(cIBeam);
        SetWidth(100);
        edit->SetBorderStyle(bsNone);
        edit->SetMargins(2 * Scaling, 2 * Scaling);
        edit->SetBounds(Rect(0, 0, Width(), CurrentFontHeight()));
        edit->SetAnchors(caTop | caLeft | caRight);
        edit->SetBounds(Rect(0, 0, 100, edit->Height()));
        edit->SetParent(this);
        SetColor(edit->GetColor());
        SetParentBackground(false);
    }

    ControlEdit::~ControlEdit()
    {
    }

    LRESULT ControlEdit::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        short x;
        short y;
        DWORD tick;
        INPUT inp;

        switch (uMsg)
        {
        case WM_LBUTTONUP:
            tick = 0;
            break;
        case WM_LBUTTONDOWN:
            PostMessage(Handle(), WM_LBUTTONUP, wParam, lParam);
            x = (short)LOWORD(lParam);
            y = (short)HIWORD(lParam);

            // Simulate double clicks.
            tick = GetTickCount();
            if (lasttick > 0)
            {
                if ((tick < lasttick ? 4294967295 - lasttick + tick : tick - lasttick) <= GetDoubleClickTime() && GetSystemMetrics(SM_CXDOUBLECLK) / 2 > abs(lastclickpt.x - x) && GetSystemMetrics(SM_CYDOUBLECLK) / 2 > abs(lastclickpt.y - y) )
                    uMsg = WM_LBUTTONDBLCLK;
            }
            // lasttick is 0 when it is first initialized. When the time returned is exactly 0, it would be handled as if lasttick wasn't updated since its initialization.
            lasttick = uMsg == WM_LBUTTONDBLCLK ? 0 : tick == 0 ? 1 : tick;
            lastclickpt = Point(x, y);

            if (GetCapture() == Handle())
                ReleaseCapture();

            SendMessage(edit->Handle(), WM_MOUSEACTIVATE, (WPARAM)ParentForm()->Handle(), MAKELPARAM(SendMessage(edit->Handle(), WM_NCHITTEST, 0, MAKELPARAM(0, 0)) , WM_LBUTTONDOWN));
            edit->SetSelStartAndLength(0, 0);

            //edit->Focus();
            SetCapture(edit->Handle());

            lParam = MAKELPARAM(x, y - topmargin);
            PostMessage(edit->Handle(), uMsg, wParam, lParam);

            inp.type = INPUT_MOUSE;
            inp.mi.dx = 0;
            inp.mi.dy = 0;
            inp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            inp.mi.mouseData = 0;
            inp.mi.time = 0;
            inp.mi.dwExtraInfo = GetMessageExtraInfo();
            SendInput(1, &inp, sizeof(INPUT));

            //SetCapture(edit->Handle());

            return 1;
        case WM_SETFONT:
            edit->GetFont() = GetFont();
            edit->SetHeight(CurrentFontHeight());
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void ControlEdit::Focus()
    {
        edit->Focus();
    }

    int ControlEdit::TopMargin()
    {
        return topmargin;
    }

    void ControlEdit::SetTopMargin(int newtopmargin)
    {
        if (topmargin == newtopmargin)
            return;
        topmargin = newtopmargin;
        edit->SetTop(topmargin);
    }

    int ControlEdit::InnerLeftMargin()
    {
        return edit->Left();
    }

    void ControlEdit::SetInnerLeftMargin(int newleft)

    {
        edit->SetAnchors(caTop | caLeft);
        SetWidth(edit->Width() + newleft);
        edit->SetAnchors(caTop | caLeft | caRight);
        edit->SetLeft(newleft);
    }

    std::wstring ControlEdit::Text() const
    {
        return edit->Text();
    }

    void ControlEdit::SetText(const std::wstring &newtext)
    {
        edit->SetText(newtext);
    }

    unsigned int ControlEdit::MaxLength()
    {
        return edit->MaxLength();
    }

    void ControlEdit::SetMaxLength(unsigned int newmax)
    {
        edit->SetMaxLength(newmax);
    }

    bool ControlEdit::ReadOnly()
    {
        return edit->ReadOnly();
    }

    void ControlEdit::SetReadOnly(bool newreadonly)
    {
        edit->SetReadOnly(newreadonly);
    }

    int ControlEdit::SelStart()
    {
        return edit->SelStart();
    }

    int ControlEdit::SelLength()
    {
        return edit->SelLength();
    }

    void ControlEdit::SelStartAndLength(int &start, int &length)
    {
        edit->SelStartAndLength(start, length);
    }

    void ControlEdit::SetSelStartAndLength(int newstart, int newlength)
    {
        edit->SetSelStartAndLength(newstart, newlength);
    }

    void ControlEdit::SetSelStart(int newstart)
    {
        edit->SetSelStart(newstart);
    }

    void ControlEdit::SetSelLength(int newlength)
    {
        edit->SetSelLength(newlength);
    }

    std::wstring ControlEdit::SelText()
    {
        return edit->SelText();
    }

    void ControlEdit::SetSelText(const std::wstring& newseltext)
    {
        edit->SetSelText(newseltext);
    }

    bool ControlEdit::AutoSelect()
    {
        return edit->AutoSelect();
    }

    void ControlEdit::SetAutoSelect(bool newautoselect)
    {
        edit->SetAutoSelect(newautoselect);
    }

    int ControlEdit::LeftMargin()
    {
        return edit->LeftMargin();
    }

    int ControlEdit::RightMargin()
    {
        return edit->RightMargin();
    }

    void ControlEdit::SetLeftMargin(int newleft)
    {
        edit->SetLeftMargin(newleft);
    }

    void ControlEdit::SetRightMargin(int newright)
    {
        edit->SetRightMargin(newright);
    }

    void ControlEdit::GetMargins(int &left, int &right)
    {
        edit->GetMargins(left, right);
    }

    void ControlEdit::SetMargins(int newleft, int newright)
    {
        edit->SetMargins(newleft, newright);
    }

    void ControlEdit::AddText(const std::wstring& str)
    {
        edit->AddText(str);
    }

    bool ControlEdit::Modified()
    {
        return edit->Modified();
    }

    void ControlEdit::SetModified(bool newmodified)
    {
        edit->SetModified(newmodified);
    }

    HWND ControlEdit::EditHandle()
    {
        return edit->Handle();
    }

    Edit* ControlEdit::Editor()
    {
        return edit;
    }


    //---------------------------------------------

#ifdef DESIGNING
    Size UpDown::DesignSize()
    {
        return Size(16, 24);
    }

    void UpDown::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"Text");
        serializer->HideProperty(L"Font");
        serializer->HideProperty(L"BorderStyle");
        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"TabOrder");
        serializer->HideProperty(L"Color");
        serializer->HideProperty(L"ParentColor");
        serializer->HideProperty(L"ParentBackground");

        serializer->Add(L"SetAttachedEditor", new EditboxDesignProperty<UpDown>(L"AttachedEditor", L"Control", &UpDown::AttachedEditor, &UpDown::SetAttachedEditor));
        serializer->Add(L"SetAlignLeft", new BoolDesignProperty<UpDown>(L"AlignLeft", L"Behavior", &UpDown::AlignLeft, &UpDown::SetAlignLeft))->SetDefault(false);
        serializer->Add(L"SetHorizontal", new BoolDesignProperty<UpDown>(L"Horizontal", L"Behavior", &UpDown::Horizontal, &UpDown::SetHorizontal))->SetDefault(false);
        serializer->Add(L"SetHandleArrowKeys", new BoolDesignProperty<UpDown>(L"HandleArrowKeys", L"Behavior", &UpDown::HandleArrowKeys, &UpDown::SetHandleArrowKeys))->SetDefault(true);
        serializer->Add(L"SetThousandSeparator", new BoolDesignProperty<UpDown>(L"ThousandSeparator", L"Behavior", &UpDown::ThousandSeparator, &UpDown::SetThousandSeparator))->SetDefault(false);
        serializer->Add(L"SetWrapValue", new BoolDesignProperty<UpDown>(L"WrapValue", L"Behavior", &UpDown::WrapValue, &UpDown::SetWrapValue))->SetDefault(false);
        serializer->Add(L"SetDisplayHexadecimal", new BoolDesignProperty<UpDown>(L"DisplayHexadecimal", L"Behavior", &UpDown::DisplayHexadecimal, &UpDown::SetDisplayHexadecimal))->SetDefault(false);
        serializer->Add(L"SetMinValue", new IntDesignProperty<UpDown>(L"MinValue", L"Behavior", &UpDown::MinValue, &UpDown::SetMinValue))->SetDefault(0);
        serializer->Add(L"SetMaxValue", new IntDesignProperty<UpDown>(L"MaxValue", L"Behavior", &UpDown::MaxValue, &UpDown::SetMaxValue))->SetDefault(100);
        serializer->Add(L"SetPosition", new IntDesignProperty<UpDown>(L"Position", L"Behavior", &UpDown::Position, &UpDown::SetPosition))->SetDefault(0);
        serializer->Add(L"SetStep", new IntDesignProperty<UpDown>(L"Step", L"Behavior", &UpDown::Step, &UpDown::SetStep))->SetDefault(1);
    }
#endif

    UpDown::UpDown() : buddy(NULL), leftalign(false), horizontal(false), arrowkeys(true), thousands(false), wrap(false), hex(false), minval(0), maxval(100), pos(0), step(1)
    {
        controlstyle -= csEraseToColor;
        SetParentBackground(false);
    }

    UpDown::~UpDown()
    {
    }

    void UpDown::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_UPDOWN_CLASS);
        params.classname = UPDOWN_CLASS;
    }

    void UpDown::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style -= wsTabStop;
        params.style << udsSetBuddyint;
        if (leftalign)
            params.style << udsAlignLeft;
        else
            params.style << udsAlignRight;
        if (horizontal)
            params.style << udsHorizontal;
        if (arrowkeys)
            params.style << udsArrowKeys;
        if (!thousands)
            params.style << udsNoThousands;
        if (wrap)
            params.style << udsWrap;
    }

    void UpDown::InitHandle()
    {
        if (buddy)
            SetParent(buddy->Parent());
        base::InitHandle();
        SendMessage(Handle(), UDM_SETBASE, hex ? 16 : 10, 0);
#ifdef UNICODE
        SendMessage(Handle(), UDM_SETUNICODEFORMAT, TRUE, 0);
#endif

        SendMessage(Handle(), UDM_SETRANGE32, minval, maxval);
        SendMessage(Handle(), UDM_SETPOS32, 0, pos);
        //SetWindowPos(Handle(), HWND_TOP, 0, 0, 0, 0, SWP_ZORDERONLY);

        if (buddy)
        {
            Rect r = buddy->WindowRect();
            if (leftalign)
                buddy->SetBounds(Rect(r.left - Width() + (horizontal ? 2 : 1), r.top, r.right, r.bottom));
            else
                buddy->SetBounds(Rect(r.left, r.top, r.right + Width() - (horizontal ? 2 : 1), r.bottom));
            SendMessage(Handle(), UDM_SETBUDDY, (WPARAM)buddy->Handle(), 0);
            MoveAbove(buddy);
            SetWindowPos(Handle(), buddy->Handle(), 0, 0, 0, 0, SWP_ZORDERONLY);
        }
    }

    LRESULT UpDown::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WINDOWPOS *pos;
        if (buddy && buddy->HandleCreated() && uMsg == WM_WINDOWPOSCHANGING)
        {
            pos = (WINDOWPOS*)lParam;
            if ((pos->flags & SWP_NOMOVE) != SWP_NOMOVE)
            {
                Rect r = buddy->WindowRect();
#ifdef DESIGNING
                // Prevent resizing the buttons for "negative" widths when moving one edge by a sizer.
                if (Designing())
                {
                    if ((leftalign && pos->x + pos->cx > r.left + 1) || (!leftalign && pos->x < r.right - 1))
                        pos->cx = max(2, Width());
                }
#endif
                if (leftalign)
                {
                    pos->x = r.left - pos->cx + 1;
                    pos->y = r.top;
                    pos->cy = r.Height();
                }
                else
                {
                    pos->x = r.right - 1;
                    pos->y = r.top;
                    pos->cy = r.Height();
                }
            }
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool UpDown::HandleNotify(Control *parent, LPARAM lParam, HRESULT &result)
    {
        NMUPDOWN *ud = (NMUPDOWN*)lParam;
        ud->iDelta = step * (ud->iDelta < 0 ? -1 : 1);
        if (ud->iPos + ud->iDelta < minval)
            ud->iDelta = minval - ud->iPos;
        else if (ud->iPos + ud->iDelta > maxval)
            ud->iDelta = maxval - ud->iPos;
        return true;
    }

    bool UpDown::AlignLeft()
    {
        return leftalign;
    }

    bool UpDown::Horizontal()
    {
        return horizontal;
    }

    bool UpDown::HandleArrowKeys()
    {
        return arrowkeys;
    }

    bool UpDown::ThousandSeparator()
    {
        return thousands;
    }

    bool UpDown::WrapValue()
    {
        return wrap;
    }

    bool UpDown::DisplayHexadecimal()
    {
        if (HandleCreated())
            hex = SendMessage(Handle(),UDM_GETBASE,0,0) == 16;
        return hex;
    }

    Edit* UpDown::AttachedEditor()
    {
        return buddy;
    }

    void UpDown::SetAttachedEditor(Edit *neweditor)
    {
        if (buddy == neweditor || (neweditor && !neweditor->Parent()))
            return;

        bool recreate = false;

        SetAlignment(alNone);

        if (buddy)
            RemoveFromNotifyList(buddy, nrSubControl);
        buddy = neweditor;
        if (neweditor)
        {
            AddToNotifyList(neweditor, nrSubControl);
            buddy = neweditor;
            if (HandleCreated())
            {
                DestroyHandle();
                recreate = true;
            }
            //SetParent(buddy->Parent());
        }

        if (recreate || HandleCreated())
            RecreateHandle();
    }

    void UpDown::SetAlignLeft(bool newleftalign)
    {
        if (leftalign == newleftalign)
            return;
        leftalign = newleftalign;
        if (HandleCreated())
            RecreateHandle();
    }

    void UpDown::SetHorizontal(bool newhorizontal)
    {
        if (horizontal == newhorizontal)
            return;
        horizontal = newhorizontal;
        if (HandleCreated())
            RecreateHandle();
    }

    void UpDown::SetHandleArrowKeys(bool newarrowkeys)
    {
        if (arrowkeys == newarrowkeys)
            return;
        arrowkeys = newarrowkeys;
        if (HandleCreated())
            RecreateHandle();
    }

    void UpDown::SetThousandSeparator(bool newthousands)
    {
        if (thousands == newthousands)
            return;
        thousands = newthousands;
        if (HandleCreated())
            RecreateHandle();
    }

    void UpDown::SetWrapValue(bool newwrap)
    {
        if (wrap == newwrap)
            return;
        wrap = newwrap;
        if (HandleCreated())
            RecreateHandle();
    }

    void UpDown::SetDisplayHexadecimal(bool newhex)
    {
        if (hex == newhex)
            return;
        hex = newhex;
        if (HandleCreated())
            SendMessage(Handle(), UDM_SETBASE, hex ? 16 : 10,0);
    }

    void UpDown::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        if (object == buddy)
        {
            buddy = NULL;
            if (HandleCreated())
            {
                SendMessage(Handle(), UDM_SETBUDDY, (WPARAM)0, 0);
                Invalidate();
            }

#ifdef DESIGNING
            if (Designing() && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"AttachedEditor");
#endif
        }
    }

    void UpDown::ChangeNotify(Object *object, int changetype)
    {
        if (changetype != CHANGE_MOVED || object != buddy)
            return;

        Rect r = buddy->WindowRect();
        SetBounds(Rect((leftalign ? r.left - Width() + 1 : r.right - 1), r.top, (leftalign ? r.left + 1 : r.right + Width() - 1), r.bottom));
    }

    void UpDown::Range(int &minvalue, int &maxvalue)
    {
        if (HandleCreated())
            SendMessage(Handle(), UDM_GETRANGE32, (WPARAM)&minval, (LPARAM)&maxval);
        minvalue = minval;
        maxvalue = maxval;
    }

    int UpDown::MinValue()
    {
        if (HandleCreated())
            SendMessage(Handle(), UDM_GETRANGE32, (WPARAM)&minval, (LPARAM)&maxval);
        return minval;
    }

    int UpDown::MaxValue()
    {
        if (HandleCreated())
            SendMessage(Handle(), UDM_GETRANGE32, (WPARAM)&minval, (LPARAM)&maxval);
        return maxval;
    }

    int UpDown::Position()
    {
        if (HandleCreated())
        {
            BOOL err;
            int r = SendMessage(Handle(), UDM_GETPOS32, 0, (LPARAM)&err);
            if (!err)
                pos = r;
        }
        return pos;
    }

    void UpDown::SetRange(int minvalue, int maxvalue)
    {
        if (maxvalue < minvalue)
            maxvalue = minvalue;
        if (minval == minvalue && maxval == maxvalue)
            return;

        minval = minvalue;
        maxval = maxvalue;
        if (HandleCreated())
            SendMessage(Handle(), UDM_SETRANGE32, minval, maxval);
    }

    void UpDown::SetMinValue(int newmin)
    {
        if (newmin > maxval)
            newmin = maxval;
        if (newmin == minval)
            return;

        minval = newmin;
        if (HandleCreated())
            SendMessage(Handle(), UDM_SETRANGE32, minval, maxval);
    }

    void UpDown::SetMaxValue(int newmax)
    {
        if (newmax < minval)
            newmax = minval;
        if (newmax == maxval)
            return;

        maxval = newmax;
        if (HandleCreated())
            SendMessage(Handle(), UDM_SETRANGE32, minval, maxval);
    }

    void UpDown::SetPosition(int newpos)
    {
        if (newpos < minval)
            newpos = minval;
        if (newpos > maxval)
            newpos = maxval;
        pos = newpos;
        if (HandleCreated())
            SendMessage(Handle(), UDM_SETPOS32, 0, newpos);
    }

    int UpDown::Step()
    {
        return step;
    }

    void UpDown::SetStep(int newstep)
    {
        step = newstep;
    }


    //---------------------------------------------
#ifdef DESIGNING
    void Memo::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"AutoSelect");

        serializer->Add(L"SetWordWrap", new BoolDesignProperty<Memo>(L"WordWrap", L"Behavior", &Memo::WordWrap, &Memo::SetWordWrap))->SetDefault(false);
        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkArrows | wkOthers | wkEnter | wkTab);
    }

    Size Memo::DesignSize()
    {
        return BaseToPixel(100, 50);
    }
#endif

    Memo::Memo() : wordwrap(false), vscroll(true), hscroll(true)
    {
        //controlstyle << csForceBgColor;
        SetWantedKeyTypes(wkArrows | wkOthers | wkEnter | wkTab);
    }

    Memo::~Memo()
    {
    }

    void Memo::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << esMultiline << esAutoVScroll << wsVScroll;
        if (!ReadOnly())
            params.style << esWantReturn;
        if (wordwrap)
            params.style -= esAutoHScroll;
        else
            params.style << wsHScroll;
        hscroll = !wordwrap;
    }

    LRESULT Memo::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            case EM_SETREADONLY:

                if (wParam == TRUE)
                    SetWindowLongPtr(Handle(), GWL_STYLE, GetWindowLongPtr(Handle(), GWL_STYLE) & ~esWantReturn);
                else
                    SetWindowLongPtr(Handle(), GWL_STYLE, GetWindowLongPtr(Handle(), GWL_STYLE) | esWantReturn);
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void Memo::InitHandle()
    {
        base::InitHandle();
    }

    void Memo::NeedsDialogCode(WORD key, DialogCodeSet &dialogcode)
    {
        if (ReadOnly())
            dialogcode = DLGC_WANTARROWS;
    }

    bool Memo::WordWrap() const
    {
        return wordwrap;
    }

    void Memo::SetWordWrap(bool wrap)
    {
        if (wordwrap == wrap)
            return;
        wordwrap = wrap;
        hscroll = !wordwrap;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Memo::VerticalScrollbar() const
    {
        return vscroll;
    }

    bool Memo::HorizontalScrollbar() const
    {
        return hscroll;
    }

    void Memo::AddLine(const std::wstring& str, bool triggerevent)
    {
        SetSelStart(TextLength());
        SetSelText(str);
        SetSelText(L"\r\n");

        if (triggerevent && OnTextChanged)
            OnTextChanged(this, EventParameters());
    }

    void Memo::SetLines(const std::vector<std::wstring> &newlines, bool triggerevent)
    {
        for (auto it = newlines.begin(); it != newlines.end(); ++it)
            AddLine(*it, false);

        if (triggerevent && OnTextChanged)
            OnTextChanged(this, EventParameters());
    }

    void Memo::GetLines(std::vector<std::wstring> &out)
    {
        std::wstring text = Text();
        size_t pos = 0, prev;
        size_t cnt = text.length();
        int linecnt = 0;
        while (pos < cnt)
        {
            prev = pos;
            pos = text.find(L"\r\n", pos);
            if (pos == std::wstring::npos)
                pos = cnt;

            if (pos > prev || (pos == prev && pos != cnt))
                ++linecnt;

            pos += 2;
        }

        out.clear();
        out.resize(linecnt);
        pos = 0;
        linecnt = 0;
        while (pos < cnt)
        {
            prev = pos;
            pos = text.find(L"\r\n", pos);
            if (pos == std::wstring::npos)
                pos = cnt;

            if (pos > prev || (pos == prev && pos != cnt))
                out[linecnt++] = text.substr(prev, pos - prev);

            pos += 2;
        }
    }

    int Memo::LineCount()
    {
        if (HandleCreated())
        {
            return PassMessage(EM_GETLINECOUNT, 0, 0);
        }
        else
        {
            std::wstring text = Text();
            size_t pos = 0, prev;
            size_t cnt = text.length();
            if (cnt == 0)
                return 1;

            int linecnt = 0;
            while (pos < cnt)
            {
                prev = pos;
                pos = text.find(L"\r\n", pos);
                if (pos == std::wstring::npos)
                    pos = cnt;

                if (pos > prev || (pos == prev && pos != cnt))
                    ++linecnt;

                pos += 2;
            }

            return max(1, linecnt);
        }
    }

    int Memo::LineLength(int lineindex)
    {
        if (HandleCreated())
        {
            return LineLengthFromCharacter(LineStartIndex(lineindex));
        }
        else
        {
            std::wstring text = Text();
            size_t pos = 0, prev;
            size_t cnt = text.length();
            if (cnt == 0 || lineindex < 0)
                return 0;

            int linecnt = 0;
            while (pos < cnt && linecnt < lineindex)
            {
                prev = pos;
                pos = text.find(L"\r\n", pos);
                if (pos == std::wstring::npos)
                    pos = cnt;

                if (pos > prev || (pos == prev && pos != cnt))
                    ++linecnt;

                pos += 2;
            }

            prev = pos;
            pos = text.find(L"\r\n", pos);
            if (pos == std::wstring::npos)
                return cnt - prev;
            return pos - prev;
        }
    }

    int Memo::LineLengthFromCharacter(int charindex)
    {
        if (HandleCreated())
        {
            return PassMessage(EM_LINELENGTH, charindex, 0);
        }
        else
        {
            std::wstring text = Text();
            size_t pos = 0, prev;
            size_t cnt = text.length();
            if ((int)cnt <= charindex || charindex < 0)
                return 0;
            while (pos < cnt)
            {
                prev = pos;
                pos = text.find(L"\r\n", pos);
                if (pos == std::wstring::npos || (int)pos + 2 > charindex)
                {
                    pos = prev;
                    break;
                }

                pos += 2;
            }

            prev = pos;
            pos = text.find(L"\r\n", pos);
            if (pos == std::wstring::npos)
                return cnt - prev;
            return pos - prev;
        }
    }

    int Memo::LineFromCharacter(int charindex)
    {
        if (HandleCreated())
        {
            return PassMessage(EM_LINEFROMCHAR, charindex, 0);
        }
        else
        {
            std::wstring text = Text();
            size_t pos = 0, prev;
            size_t cnt = text.length();
            if ((int)cnt <= charindex || charindex < 0)
                return -1;

            int linecnt = 0;
            while (pos < cnt)
            {
                prev = pos;
                pos = text.find(L"\r\n", pos);
                if (pos == std::wstring::npos)
                    break;
                if ((int)pos + 2 > charindex)
                    break;

                if (pos > prev || (pos == prev && pos != cnt))
                    ++linecnt;

                pos += 2;
            }

            return linecnt;
        }
    }

    int Memo::LineStartIndex(int lineindex)
    {
        if (HandleCreated())
        {
            return PassMessage(EM_LINEINDEX, lineindex, 0);
        }
        else
        {
            std::wstring text = Text();
            size_t pos = 0, prev;
            size_t cnt = text.length();
            if (cnt == 0 || lineindex <= 0)
                return lineindex != 0 ? -1 : 0;

            int linecnt = 0;
            while (pos < cnt && linecnt < lineindex)
            {
                prev = pos;
                pos = text.find(L"\r\n", pos);
                if (pos == std::wstring::npos)
                {
                    if (lineindex != linecnt)
                        return -1;
                    return prev;
                }

                if (pos > prev || (pos == prev && pos != cnt))
                    ++linecnt;

                pos += 2;
            }

            return pos;
        }
    }

    std::wstring Memo::GetLine(int lineindex)
    {
        if (HandleCreated())
        {
            int len = LineLength(lineindex);
            std::wstring str(len + 3, 0);
            str.resize(PassMessage(EM_GETLINE, lineindex, (LPARAM)&str[0]));
            return str;
        }
        else
        {
            std::wstring text = Text();
            size_t pos = 0, prev;
            size_t cnt = text.length();
            if (cnt == 0)
                return std::wstring();

            int linecnt = 0;
            while (pos < cnt && linecnt < lineindex)
            {
                prev = pos;
                pos = text.find(L"\r\n", pos);
                if (pos == std::wstring::npos)
                {
                    if (lineindex != linecnt)
                        return std::wstring();
                    return text.substr(prev, pos - prev);
                }

                if (pos > prev || (pos == prev && pos != cnt))
                    ++linecnt;

                pos += 2;
            }

            prev = pos;
            pos = text.find(L"\r\n", pos);
            if (pos == std::wstring::npos)
                return text.substr(prev, cnt - prev);
            return text.substr(prev, pos - prev);
        }
    }


    //---------------------------------------------

#ifdef DESIGNING

    Size Progressbar::DesignSize()
    {
        return BaseToPixel(107, 8);
    }

    void Progressbar::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"Text");
        serializer->HideProperty(L"Font");
        serializer->HideProperty(L"BorderStyle");
        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"TabOrder");
        serializer->HideProperty(L"ParentBackground");
        serializer->HideProperty(L"ParentColor");

        serializer->Add( L"SetVertical", new BoolDesignProperty<Progressbar>(L"Vertical", L"Appearance", &Progressbar::Vertical, &Progressbar::SetVertical))->SetDefault(false);
        serializer->Add( L"SetSmooth", new BoolDesignProperty<Progressbar>(L"Smooth", L"Progress", &Progressbar::Smooth, &Progressbar::SetSmooth))->SetDefault(false);
        serializer->Add( L"SetSmoothReverse", new BoolDesignProperty<Progressbar>(L"SmoothReverse", L"Progress", &Progressbar::SmoothReverse, &Progressbar::SetSmoothReverse))->SetDefault(false);
        serializer->Add( L"SetMinRange", new IntDesignProperty<Progressbar>(L"MinRange", L"Progress", &Progressbar::MinRange, &Progressbar::SetMinRange))->SetDefault(0);
        serializer->Add( L"SetMaxRange", new IntDesignProperty<Progressbar>(L"MaxRange", L"Progress", &Progressbar::MaxRange, &Progressbar::SetMaxRange))->SetDefault(100);
        serializer->Add( L"SetPosition", new IntDesignProperty<Progressbar>(L"Position", L"Progress", &Progressbar::Position, &Progressbar::SetPosition))->SetDefault(0);
        serializer->Add( L"SetStep", new IntDesignProperty<Progressbar>(L"Step", L"Progress", &Progressbar::Step, &Progressbar::SetStep))->SetDefault(10);
        serializer->Add( L"SetMarquee", new BoolDesignProperty<Progressbar>(L"Marquee", L"Progress", &Progressbar::Marquee, &Progressbar::SetMarquee))->SetDefault(false);
        serializer->Add( L"SetMarqueeUpdateTime", new IntDesignProperty<Progressbar>(L"MarqueeUpdateTime", L"Progress", &Progressbar::MarqueeUpdateTime, &Progressbar::SetMarqueeUpdateTime))->SetDefault(0);

    }

#endif

    Progressbar::Progressbar() :
            smooth(false), vertical(false), smoothreverse(false), marqueemode(false), marqueeupdatetime(0),
            minrange(0), maxrange(100), pos(0), step(10), bkcolor((COLORREF)CLR_DEFAULT), barcolor((COLORREF)CLR_DEFAULT),
            state(pbstNormal)
    {
        controlstyle -= csEraseToColor;
        SetParentBackground(true);
    }

    Progressbar::~Progressbar()
    {
    }

    void Progressbar::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_PROGRESS_CLASS);
        params.classname = PROGRESS_CLASS;
    }


    void Progressbar::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        if (smooth)
            params.style << pbsSmooth;
        if (vertical)
            params.style << pbsVertical;
        if (smoothreverse)
            params.style << pbsSmoothReverse;
        if (marqueemode)
            params.style << pbsMarquee;
    }

    void Progressbar::InitHandle()
    {
        base::InitHandle();
        if (marqueemode)
            SendMessage(Handle(), PBM_SETMARQUEE, TRUE, marqueeupdatetime);
        else
        {
            if (minrange != 0 || maxrange != 100)
                SendMessage(Handle(), PBM_SETRANGE32, minrange, maxrange);
            SendMessage(Handle(), PBM_SETPOS, pos, 0);
            if (step != 10)
                SendMessage(Handle(), PBM_SETSTEP, step, 0);
            if (state != pbstNormal)
                SendMessage(Handle(), PBM_SETSTATE, (WPARAM)state, 0);
            if (barcolor != (COLORREF)CLR_DEFAULT)
                SendMessage(Handle(), PBM_SETBARCOLOR, 0, barcolor);
            if (bkcolor != (COLORREF)CLR_DEFAULT)
                SendMessage(Handle(), PBM_SETBKCOLOR, 0, bkcolor);
        }
    }

    void Progressbar::SaveWindow()
    {
        _MinMaxRangeMarqueePosStepStateColor();
        base::SaveWindow();
    }

    bool Progressbar::Marquee()
    {
        return marqueemode;
    }

    int Progressbar::MarqueeUpdateTime()
    {
        return marqueeupdatetime;
    }

    void Progressbar::SetMarqueeUpdateTime(int newupdatetime)
    {
        SetMarqueeAndUpdateTime(marqueemode, newupdatetime);
    }

    void Progressbar::SetMarquee(bool turnon)
    {
        SetMarqueeAndUpdateTime(turnon, marqueeupdatetime);
    }

    void Progressbar::SetMarqueeAndUpdateTime(bool turnon, int newupdatetime)
    {
        if ((!turnon && !marqueemode) || (turnon && marqueemode && marqueeupdatetime == newupdatetime))
            return;

        _Marquee();
        if (turnon && !marqueemode)
            _MinMaxRangeMarqueePosStepStateColor();

        marqueemode = turnon;
        marqueeupdatetime = max(0,newupdatetime);
        if (HandleCreated())
        {
            if (marqueemode)
            {
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) | pbsMarquee);
                SendMessage(Handle(),PBM_SETMARQUEE,TRUE, marqueeupdatetime);
            }
            else
            {
                SendMessage(Handle(),PBM_SETMARQUEE,FALSE, marqueeupdatetime);
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) & ~pbsMarquee);

                SendMessage(Handle(),PBM_SETRANGE32,minrange,maxrange);
            }
        }
    }

    bool Progressbar::Vertical()
    {
        return vertical;
    }

    void Progressbar::SetVertical(bool newvertical)
    {
        if (vertical == newvertical)
            return;
        vertical = newvertical;
        if (HandleCreated())
        {
            if (vertical)
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) | pbsVertical);
            else
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) & ~pbsVertical);
        }
    }

    bool Progressbar::Smooth()
    {
        return smooth;
    }

    void Progressbar::SetSmooth(bool newsmooth)
    {
        if (smooth == newsmooth)
            return;
        smooth = newsmooth;
        if (HandleCreated())
        {
            if (smooth)
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) | pbsSmooth);
            else
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) & ~pbsSmooth);
        }
    }

    bool Progressbar::SmoothReverse()
    {
        return smoothreverse;
    }

    void Progressbar::SetSmoothReverse(bool newsmoothreverse)
    {
        if (smoothreverse == newsmoothreverse)
            return;
        smoothreverse = newsmoothreverse;
        if (HandleCreated())
        {
            if (smoothreverse)
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) | pbsSmoothReverse);
            else
                SetWindowLong(Handle(),GWL_STYLE,GetWindowLong(Handle(),GWL_STYLE) & ~pbsSmoothReverse);
        }
    }

    void Progressbar::_MinMaxRangeMarqueePosStepStateColor()
    {
        if (!HandleCreated())
            return;
        barcolor = SendMessage(Handle(),PBM_GETBARCOLOR,0,0);
        bkcolor = SendMessage(Handle(),PBM_GETBKCOLOR,0,0);
        step = SendMessage(Handle(),PBM_GETSTEP,0,0);
        marqueemode = (GetWindowLong(Handle(),GWL_STYLE) & pbsMarquee) == pbsMarquee;

        if (marqueemode)
            return;

        PBRANGE range;
        memset(&range,0,sizeof(PBRANGE));
        SendMessage(Handle(),PBM_GETRANGE,TRUE,(LPARAM)&range);
        minrange = range.iLow;
        maxrange = range.iHigh;

        pos = SendMessage(Handle(),PBM_GETPOS,0,0);
        state = (ProgressbarState)SendMessage(Handle(),PBM_GETSTATE,0,0);
    }

    void Progressbar::_Marquee()
    {
        if (!HandleCreated())
            return;
        marqueemode = (GetWindowLong(Handle(),GWL_STYLE) & pbsMarquee) == pbsMarquee;
    }

    void Progressbar::_MinMaxRangePos()
    {
        if (!HandleCreated() || marqueemode)
            return;
        PBRANGE range;
        memset(&range,0,sizeof(PBRANGE));
        SendMessage(Handle(),PBM_GETRANGE,TRUE,(LPARAM)&range);
        minrange = range.iLow;
        maxrange = range.iHigh;
        pos = SendMessage(Handle(),PBM_GETPOS,0,0);
    }

    void Progressbar::_MinMaxRange()
    {
        if (!HandleCreated() || marqueemode)
            return;
        PBRANGE range;
        memset(&range,0,sizeof(PBRANGE));
        SendMessage(Handle(),PBM_GETRANGE,TRUE,(LPARAM)&range);
        minrange = range.iLow;
        maxrange = range.iHigh;
    }

    void Progressbar::_Pos()
    {
        if (!HandleCreated() || marqueemode)
            return;
        pos = SendMessage(Handle(),PBM_GETPOS,0,0);
    }

    void Progressbar::_Step()
    {
        if (!HandleCreated())
            return;
        step = SendMessage(Handle(),PBM_GETSTEP,0,0);
    }

    void Progressbar::_State()
    {
        if (!HandleCreated())
            return;
        state = (ProgressbarState)SendMessage(Handle(),PBM_GETSTATE,0,0);
    }

    void Progressbar::_Color()
    {
        if (!HandleCreated())
            return;
        barcolor = SendMessage(Handle(),PBM_GETBARCOLOR,0,0);
        bkcolor = SendMessage(Handle(),PBM_GETBKCOLOR,0,0);
    }

    int Progressbar::MinRange()
    {
        _MinMaxRange();
        return minrange;
    }

    int Progressbar::MaxRange()
    {
        _MinMaxRange();
        return maxrange;
    }

    void Progressbar::MinMaxRange(int &minr, int &maxr)
    {
        _MinMaxRange();
        minr = minrange;
        maxr = maxrange;
    }

    void Progressbar::SetMinRange(int newmin)
    {
        _MinMaxRangePos();
        if (maxrange <= newmin)
            maxrange = newmin+1;

        if (newmin == minrange)
            return;

        minrange = newmin;
        if (pos < minrange)
            pos = minrange;

        if (HandleCreated() && !marqueemode)
            SendMessage(Handle(),PBM_SETRANGE32,minrange,maxrange);
    }

    void Progressbar::SetMaxRange(int newmax)
    {
        _MinMaxRangePos();
        if (minrange >= newmax)
            minrange = newmax-1;

        if (newmax == maxrange)
            return;

        maxrange = newmax;
        if (pos > maxrange)
            pos = maxrange;

        if (HandleCreated() && !marqueemode)
            SendMessage(Handle(),PBM_SETRANGE32,minrange,maxrange);
    }

    void Progressbar::SetMinMaxRange(int newmin, int newmax)
    {
        _MinMaxRangePos();
        if (newmax <= newmin)
            newmax = newmin+1;

        if (newmin == minrange && newmax == maxrange)
            return;

        minrange = newmin;
        maxrange = newmax;
        if (pos < minrange)
            pos = minrange;
        else if (pos > maxrange)
            pos = maxrange;

        if (HandleCreated() && !marqueemode)
            SendMessage(Handle(),PBM_SETRANGE32,minrange,maxrange);
    }

    int Progressbar::Position()
    {
        _Pos();
        return pos;
    }

    void Progressbar::SetPosition(int newpos)
    {
        _MinMaxRangePos();
        if (newpos < minrange)
            newpos = minrange;
        if (newpos > maxrange)
            newpos = maxrange;
        if (pos == newpos)
            return;
        pos = newpos;
        if (HandleCreated() && !marqueemode)
            SendMessage(Handle(),PBM_SETPOS,pos,0);
    }

    int Progressbar::Step()
    {
        _Step();
        return step;
    }

    void Progressbar::SetStep(int newstep)
    {
        _Step();
        if (step == newstep)
            return;
        step = newstep;
        if (HandleCreated())
            SendMessage(Handle(),PBM_SETSTEP,newstep,0);
    }

    int Progressbar::StepIt()
    {
        int result;
        if (marqueemode)
            throw L"Cannot step in marquee mode. Use SetPosition instead.";
        else if (HandleCreated() && !marqueemode)
            result = SendMessage(Handle(),PBM_STEPIT,0,0);
        else
        {
            result = pos;
            pos = pos+step;
            while (pos > maxrange)
                pos -= maxrange;
        }
        _Pos();
        return result;
    }

    ProgressbarState Progressbar::State()
    {
        _State();
        return state;
    }

    void Progressbar::SetState(ProgressbarState newstate)
    {
        _State();
        if (HandleCreated() && state != newstate)
            SendMessage(Handle(),PBM_SETSTATE,(WPARAM)newstate,0);
        state = newstate;
    }

    Color Progressbar::BarColor()
    {
        _Color();
        return barcolor;
    }

    void Progressbar::SetBarColor(Color color)
    {
        _Color();
        if (barcolor == color)
            return;
        barcolor = color;
        if (HandleCreated())
            SendMessage(Handle(),PBM_SETBARCOLOR,0,barcolor);
    }

    Color Progressbar::BKColor()
    {
        _Color();
        return bkcolor;
    }

    void Progressbar::SetBKColor(Color color)
    {
        _Color();
        if (bkcolor == color)
            return;
        bkcolor = color;
        if (HandleCreated())
            SendMessage(Handle(),PBM_SETBARCOLOR,0,bkcolor);
    }


    //---------------------------------------------


#ifdef DESIGNING
    void ControlElemList::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideAll();
        serializer->Add(L"Add", new ControlElemListItemsDesignProperty(L"Text"));
    }

    bool ControlElemList::operator==(const ControlElemList &other)
    {
        return false;
    }

    bool ControlElemList::NoData()
    {
        return nodata;
    }

#endif

    ControlElemList::ControlElemList(int messagelist[cemEnumEnd]) : owner(NULL), nodata(false)
    {
        memcpy(listmessages, messagelist, sizeof(int) * cemEnumEnd);
    }

    ControlElemList::~ControlElemList()
    {
    }

    void ControlElemList::SetOwner(ListControl *newowner)
    {
        owner = newowner;
    }

    void ControlElemList::InsertItems()
    {
        if (!owner->HandleCreated())
            throw L"The owner list control must have a handle when inserting items from the strings list.";

        if (nodata)
        {
            SetCount(nodatacount);
            return;
        }

        // Count size needed for all items.
        int s = 0;
        for (auto it = list.begin(); it != list.end(); ++it)
            s += it->first.size() + 1;
        SendMessage(owner->Handle(), listmessages[cemInitStorage], list.size(), s * 2);

        owner->BeginUpdate();
        for (auto it = list.begin(); it != list.end(); ++it)
            Add(it->first, it->second);
        owner->EndUpdate();
        owner->Invalidate();

        list.clear();
    }

    void ControlElemList::SaveItems()
    {
        if (!owner->HandleCreated())
            throw L"The owner box must have a handle when saving items from the strings list.";

        if (nodata)
        {
            nodatacount = Count();
            return;
        }

        list.clear();

        int l = 0;
        int alloced = 0;
        wchar_t *buf = NULL;
        int count = SendMessage(owner->Handle(), listmessages[cemGetCount], 0, 0);
        if (count == listmessages[cemError])
            throw L"Error while getting combobox size.";

        for (int ix = 0; ix < count; ++ix)
        {
            l = SendMessage(owner->Handle(), listmessages[cemGetTextLen], ix, 0);
            if (l == listmessages[cemError])
                throw L"Error. Listbox changed?";

            if (alloced < l+1)
            {
                delete[] buf;
                alloced = l+1;
                buf = new wchar_t[alloced];
            }
            SendMessage(owner->Handle(), listmessages[cemGetText], ix, (LPARAM)buf);
            void *data = (void*)SendMessage(owner->Handle(), listmessages[cemGetItemData], ix, 0);
            if ((int)data == listmessages[cemError])
                data = NULL;
            list.push_back(std::pair<std::wstring, void*>(std::wstring(buf, l), data));
        }
        delete[] buf;
    }

    void ControlElemList::Add(const std::wstring& str)
    {
        Add(str, NULL);
    }

    void ControlElemList::Add(const std::wstring& str, void *data)
    {
        Insert(-1, str, data);
    }

    void ControlElemList::Insert(int position, const std::wstring& str)
    {
        Insert(position, str, NULL);
    }

    void ControlElemList::Insert(int position, const std::wstring& str, void *data)
    {
        if (nodata)
            throw L"Cannot change the items of a virtual list.";

        if (owner->HandleCreated())
        {
            LRESULT error = SendMessage(owner->Handle(), listmessages[cemInsertString], position, (LPARAM)str.c_str());
            if (error == listmessages[cemError])
                throw L"Error adding item to combobox.";
            else if (error == listmessages[cemNotEnoughSpace])
                throw L"Not enough memory to add item to combobox.";

            // Error holds the insert position if no error occured.
            if (data)
            {
                error = SendMessage(owner->Handle(), listmessages[cemSetItemData], error, (LPARAM)data);
                if (error == listmessages[cemError])
                    throw L"Couldn't add data to listbox.";
            }
            return;
        }

        if (position == -1)
            position = list.size();
        else if (position < -1 || position > (int)list.size())
            throw L"Index of item is out of range.";
        list.insert(list.begin() + position, std::pair<std::wstring, void*>(str, data));
        owner->NotifyList(ListControl::lcncInsert, position, 1);
    }

    void ControlElemList::Swap(int a, int b)
    {
        if (nodata)
            throw L"Cannot change the items of a virtual list.";
        int cnt = Count();
        if (a < 0 || a >= cnt || b < 0 || b >= cnt)
            throw L"Index of item is out of range.";
        if (a == b)
            return;

        if (a > b)
        {
            int tmp = a;
            a = b;
            b = tmp;
        }

        if (owner->HandleCreated())
        {
            owner->BeginUpdate();
            std::wstring stra = Text(a);
            std::wstring strb = Text(b);
            void *dataa = Data(a);
            void *datab = Data(b);
            Delete(b);
            Delete(a);
            Insert(a, strb, datab);
            Insert(b, stra, dataa);
            owner->EndUpdate();
            owner->Invalidate();

            return;
        }

        std::swap(list[a], list[b]);
        //std::pair<std::wstring, void*> tmp = list[a];
        //list[a] = list[b];
        //list[b] = tmp;

        owner->NotifyList(ListControl::lcncSwap, a, b);
    }

    void ControlElemList::Delete(int position)
    {
        if (nodata)
            throw L"Cannot change the items of a virtual list.";

        if (owner->HandleCreated())
        {
            LRESULT error = SendMessage(owner->Handle(), listmessages[cemDeleteString], position, 0);
            if (error == listmessages[cemError])
                throw L"Bad index when deleting box item.";
            return;
        }

        if (position < 0 || position >= (int)list.size())
            throw L"Index of item is out of range.";

        list.erase(list.begin() + position);
        owner->NotifyList(ListControl::lcncDelete, position, 1);
    }

    void ControlElemList::Clear()
    {
        if (nodata)
            SetCount(0);

        if (owner->HandleCreated())
            SendMessage(owner->Handle(), listmessages[cemResetContent], 0, 0);
        else
        {
            list.clear();
            owner->NotifyList(ListControl::lcncClear, 0, 0);
        }
    }

    int ControlElemList::Count() const
    {
        if (owner->HandleCreated())
            return SendMessage(owner->Handle(), listmessages[cemGetCount], 0, 0);
        else if (!nodata)
            return list.size();
        else
            return nodatacount;
    }

    void ControlElemList::SetCount(int newcount)
    {
        if (!nodata)
            throw L"Cannot change the count in a non-virtual list.";

        // Only list boxes can be virtual. We can use LB_SETCOUNT here.
        if (owner->HandleCreated())
        {
            LRESULT error = SendMessage(owner->Handle(), LB_SETCOUNT, newcount, 0);
            if (error == listmessages[cemError] || error == listmessages[cemNotEnoughSpace])
                throw L"Couldn't change count in list.";
        }
        else
        {
            int oldcount = nodatacount;
            nodatacount = newcount;
            owner->NotifyList(ListControl::lcncSetCount, newcount, oldcount);
        }
    }

    std::wstring ControlElemList::Text(int position) const
    {
        if (nodata)
            throw L"Cannot retrieve the items of a virtual list.";

        if (owner->HandleCreated())
        {
            int l = SendMessage(owner->Handle(), listmessages[cemGetTextLen], position, 0);
            if (l == listmessages[cemError])
                throw L"Index of item is out of range.";

            if (l)
            {
                wchar_t *buf = new wchar_t[l+1];
                SendMessage(owner->Handle(), listmessages[cemGetText], position, (LPARAM)buf);
                std::wstring str(buf, l);
                delete[] buf;
                return str;
            }
            return std::wstring();
        }
        else if (position < 0 || position >= (int)list.size())
            throw L"Index of item is out of range.";

        return list[position].first;
    }

    void ControlElemList::SetText(int position, const std::wstring& newtext)
    {
        if (nodata)
            throw L"Cannot set the items of a virtual list.";

        if (position < 0 || position >= (int)Count())
            throw L"Index of item is out of range.";

        if (Text(position) == newtext)
            return;

        if (owner->HandleCreated())
        {
            void *data = (void*)SendMessage(owner->Handle(), listmessages[cemGetItemData], position, 0);
            owner->BeginUpdate();
            Delete(position);
            Insert(position, newtext, data);
            owner->EndUpdate();
            owner->Invalidate();

#ifdef DESIGNING
            if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
                designer->InvalidateRow(owner, L"ItemIndex");
#endif
        }
        else
            list[position].first = newtext;
    }

    void* ControlElemList::Data(int position) const
    {
        if (nodata)
            throw L"Cannot retrieve the items of a virtual list.";

        if (position < 0 || position >= (int)Count())
            throw L"Index of item is out of range.";

        if (owner->HandleCreated())
            return (void*)SendMessage(owner->Handle(), listmessages[cemGetItemData], position, 0);

        return list[position].second;
    }

    void ControlElemList::SetData(int position, void *data)
    {
        if (nodata)
            throw L"Cannot set the items of a virtual list.";

        if (position < 0 || position >= (int)Count())
            throw L"Index of item is out of range.";

        if (owner->HandleCreated())
            SendMessage(owner->Handle(), listmessages[cemSetItemData], position, (LRESULT)data);
        else
            list[position].second = data;
    }

    void ControlElemList::SetNodata(bool newnodata)
    {
        if (nodata == newnodata)
            return;
        nodata = newnodata;
        if (nodata)
            Clear();
    }

    void ControlElemList::GetLines(std::vector<std::wstring> &out) const
    {
        out.clear();
        out.resize(Count());
        for (int ix = 0; ix < Count(); ++ix)
            out[ix] = Text(ix);
    }

    // Function added by SA
    int ControlElemList::IndexOf(std::wstring& str)
    {
        using MyPair = std::pair<std::wstring, void*>;
        using MyList = std::vector<MyPair>;
        using MyIter = MyList::iterator;

        MyPair key(str, nullptr);

        MyIter iter = std::find_if(list.begin(), list.end(), [&](const MyPair arg) {return arg.first == str; });
        return iter == list.end() ? -1: (iter -list.begin());
    }

    void ControlElemList::SetLines(const std::vector<std::wstring> &newlines)
    {
        Clear();

        if (owner->HandleCreated())
        {
            int s = 0;
            for (auto it = newlines.begin(); it != newlines.end(); ++it)
                s += it->length() + 1;
            AllocateCapacity(newlines.size(), s);
        }

        for(auto it = newlines.begin(); it != newlines.end(); ++it)
            //if (!it->empty())
            Add(*it);
    }

    void ControlElemList::AllocateCapacity(int numitems, int charcnt)
    {
        if (!owner->HandleCreated())
        {
            owner->NotifyList(ListControl::lcncAllocate, numitems, charcnt);
            return;
        }

        SendMessage(owner->Handle(), listmessages[cemInitStorage], numitems, charcnt * sizeof(wchar_t));
    }

    int ControlElemList::ItemIndex() const
    {
        if (owner->HandleCreated())
        {
            LRESULT r = SendMessage(owner->Handle(), listmessages[cemGetItemIndex], 0, 0);
            if (r == LB_ERR)
                return -1;
            return r;
        }
        return -1;
    }

    void ControlElemList::SetItemIndex(int newindex)
    {
        if (owner->HandleCreated())
        {
            LRESULT error = SendMessage(owner->Handle(), listmessages[cemSetItemIndex], newindex, 0);
            if (newindex >= 0 && error == LB_ERR)
            {
                if (newindex < -1 || newindex >= Count())
                    throw L"Couldn't select the item with the given index.";
            }
#ifdef DESIGNING
            if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
                designer->InvalidateRow(owner, L"ItemIndex");
#endif
            return;
        }
    }


    //---------------------------------------------


#ifdef DESIGNING
    ValuePair<ListControlKinds> ListControlKindStrings[] = {
        VALUEPAIR(lckNormal),
        VALUEPAIR(lckOwnerDraw),
        VALUEPAIR(lckOwnerDrawVariable),
        VALUEPAIR(lckVirtual),
        VALUEPAIR(lckVirtualOwnerDraw),
    };

    void ListControl::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"ParentBackground");
        serializer->Find<ColorDesignProperty<Control>>(L"Color")->SetDefault(clWindow);
        serializer->Find<BoolDesignProperty<Control>>(L"ParentColor")->SetDefault(false);
        serializer->Add(L"Items", new ControlElemListDesignProperty<ListControl>(L"Items", L"Control", &ListControl::Items))->MakeDefault();
        serializer->Add(L"SetItemIndex", new IntDesignProperty<ListControl>(L"ItemIndex", L"Control", &ListControl::ItemIndex, &ListControl::DesignSetItemIndex))->SetDefault(-1);

        /* Events */
        serializer->AddEvent<ListControl, NotifyEvent>(L"OnChanged", L"Control");
    }

    void ListControl::DesignSetItemIndex(int newindex)
    {
        SetItemIndex(newindex);
    }

#endif

    ListControl::ListControl(int messagelist[cemEnumEnd], int notifylist[lcnEnumEnd]) : items(messagelist), itemindex(-1)
    {

        memcpy(listnotify, notifylist, sizeof(int) * lcnEnumEnd);

        items.SetOwner(this);
        SetParentBackground(false);
        SetColor(clWindow);
    }

    ListControl::~ListControl()
    {
    }

    bool ListControl::HandleCommand(Control *parent, WPARAM wParam)
    {
        if (base::HandleCommand(parent, wParam))
            return true;

        if (HIWORD(wParam) == listnotify[lcnChanged])
        {
            SelChanged();
            return true;
        }
        return false;
    }

    void ListControl::SelChanged()
    {
        if (OnChanged)
            OnChanged(this, EventParameters());
    }

    ControlElemList& ListControl::Items()
    {
        return items;
    }

    void ListControl::InitHandle()
    {
        base::InitHandle();
        items.InsertItems();

        int oldindex = itemindex;
        itemindex = -1;
        SetItemIndex(oldindex);
    }

    void ListControl::SaveWindow()
    {
        itemindex = ItemIndex();
        items.SaveItems();

        base::SaveWindow();
    }

    void ListControl::SetNodata(bool newnodata)
    {
        if (items.nodata == newnodata)
            return;
        items.SetNodata(newnodata);
#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
        {
            designer->InvalidateRow(this, L"Items");
            designer->InvalidateRow(this, L"Text");
        }
#endif
    }

    int ListControl::ItemIndex()
    {
        if (HandleCreated())
            return items.ItemIndex();
        return itemindex;
    }

    void ListControl::SetItemIndex(int newindex, bool triggerevent)
    {
        newindex = min(max(newindex, -1), Count() - 1);
        if (HandleCreated())
        {
            int oldix = 0;
            if (triggerevent)
                oldix = items.ItemIndex();
            items.SetItemIndex(newindex);
            if (triggerevent && oldix != newindex)
                SelChanged();
        }
        else
        {
            int oldindex = itemindex;
            itemindex = newindex;
            NotifyList(ListControl::lcncSelect, itemindex, oldindex);
        }
#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Text");
#endif
    }

    int ListControl::Count()
    {
        return items.Count();
    }

    std::wstring ListControl::ItemText(int ix)
    {
        return items.Text(ix);
    }

    void* ListControl::ItemData(int ix)
    {
        return items.Data(ix);
    }

    void ListControl::SetItemText(int ix, const std::wstring &newtext)
    {
        items.SetText(ix, newtext);
    }

    void ListControl::SetItemData(int ix, void *data)
    {
        items.SetData(ix, data);
    }

    void ListControl::SwapItems(int a, int b)
    {
        items.Swap(a, b);
    }

    void ListControl::DeleteItem(int ix)
    {
        items.Delete(ix);
    }

    void ListControl::Clear()
    {
        items.Clear();
    }

    void ListControl::AddItem(const std::wstring& str)
    {
        items.Add(str);
    }

    void ListControl::AddItem(const std::wstring& str, void *data)
    {
        items.Add(str, data);
    }

    void ListControl::InsertItem(int position, const std::wstring& str)
    {
        items.Insert(position, str);
    }

    void ListControl::InsertItem(int position, const std::wstring& str, void *data)
    {
        items.Insert(position, str, data);
    }

    void ListControl::SetLines(const std::vector<std::wstring> &newlines)
    {
        items.SetLines(newlines);
    }

    void ListControl::GetLines(std::vector<std::wstring> &out) const
    {
        items.GetLines(out);
    }

    void ListControl::AllocateCapacity(int numitems, int charcnt)
    {
        items.AllocateCapacity(numitems, charcnt);
    }

    //---------------------------------------------


    int Listbox::lbmessages[cemEnumEnd] = {
            LB_INITSTORAGE, LB_GETCOUNT, LB_GETTEXTLEN, LB_GETTEXT, LB_ADDSTRING, LB_INSERTSTRING,
            LB_DELETESTRING, LB_GETITEMDATA, LB_SETITEMDATA, LB_RESETCONTENT, LB_GETCURSEL, LB_SETCURSEL,
            LB_ERR, LB_ERRSPACE,
    };

    int Listbox::lbnotify[lcnEnumEnd] = {
            LBN_SELCHANGE,
    };

#ifdef DESIGNING
    ValuePair<ListboxSelectionTypes> ListboxSelectionTypeStrings[] = {
            VALUEPAIR(lstSingleSelect),
            VALUEPAIR(lstExtendedSelect),
            VALUEPAIR(lstMultiSelect),
    };

    void Listbox::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"BorderStyle");
        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);

        serializer->Add(L"SetIntegralHeight", new BoolDesignProperty<Listbox>(L"IntegralHeight", L"Appearance", &Listbox::IntegralHeight, &Listbox::SetIntegralHeight))->SetDefault(true);
        serializer->Add(L"SetKind", new ListControlKindsDesignProperty<Listbox>(L"Kind", L"Control", &Listbox::Kind, &Listbox::SetKind))->SetDefault(lckNormal);
        serializer->Add(L"SetSelectionType", new ListboxSelectionTypesDesignProperty<Listbox>(L"SelectionType", L"Control", &Listbox::SelectionType, &Listbox::SetSelectionType))->SetDefault(lstSingleSelect);
        serializer->Add(L"SetItemHeight", new IntDesignProperty<Listbox>(L"ItemHeight", L"Appearance", &Listbox::ItemHeight, &Listbox::SetItemHeight))->SetDefault(-1);

        serializer->AddEvent<Listbox, MeasureItemEvent>(L"OnMeasureItem", L"Control");
        serializer->AddEvent<Listbox, DrawItemEvent>(L"OnDrawItem", L"Drawing");
    }

    Size Listbox::DesignSize()
    {
        return Size(80, CurrentFontHeight() * 3 + themes->MeasureEditBorderWidth().cy * 2);
    }

#endif

    Listbox::Listbox() : base(lbmessages, lbnotify), kind(lckNormal), seltype(lstSingleSelect), integralheight(true), itemheight(-1)
    {
        SetBorderStyle(bsNormal);
        controlstyle << csInTabOrder << csAcceptInput;
        SetWantedKeyTypes(wkOthers | wkArrows);
    }

    Listbox::~Listbox()
    {
    }

    void Listbox::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Listbox";
    }

    void Listbox::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << lbsNotify << wsVScroll;
        if (kind != lckVirtual && kind != lckVirtualOwnerDraw)
        {
            params.style << lbsHasStrings;
            if (kind == lckOwnerDraw)
                params.style << lbsOwnerdrawFixed;
            else if (kind == lckOwnerDrawVariable)
                params.style << lbsOwnerdrawVariable;
        }
        else
            params.style << lbsNodata << lbsOwnerdrawFixed;

        if (!integralheight && kind != lckOwnerDrawVariable)
            params.style << lbsNoIntegralHeight;

        if (seltype == lstSingleSelect)
            params.style -= lbsExtendedSel | lbsMultipleSel;
        if (seltype == lstExtendedSelect)
            params.style << lbsExtendedSel;
        if (seltype == lstMultiSelect)
            params.style << lbsMultipleSel;
    }

    void Listbox::SaveWindow()
    {
        int selix = ItemIndex();
        itemheight = ItemHeight();

        int *sels = NULL; // Array of selected items when appropriate.

        if (seltype != lstSingleSelect)
        {
            int cnt = Count();
            sellist.resize(cnt);
            int scnt = SelCount(); // Returns 0 when the original seltype was lstSingleSelect and RecreateHandle() was called for changing the selection type.

            if (scnt > 0)
            {
                sels = new int[scnt];
                LRESULT res = SendMessage(Handle(), LB_GETSELITEMS, scnt, (LPARAM)sels);
                if (res == LB_ERR) // For single selection boxes the seltype can still be something else when it was changed and RecreateHandle() was called. In those cases LB_ERR is returned. The variable sel will contain the correct selection.
                {
                    delete[] sels;
                    sels = NULL;
                }
                else
                {
                    for (int ix = 0; ix < scnt; ++ix)
                        sellist[sels[ix]] = true;
                }
            }
            if (sels == NULL && selix >= 0 && selix < cnt)
                sellist[selix] = true;
            delete[] sels;
            sels = NULL;
        }

        base::SaveWindow();
    }

    void Listbox::InitHandle()
    {
        Rect r = WindowRect();

        base::InitHandle();

        SetWindowPos(Handle(), NULL, r.left, r.top, r.Width(), r.Height(), SWP_BOUNDS);

        //if (kind != lkOwnerDrawVariable)
        //{
        //    MEASUREITEMSTRUCT mi;
        //    memset(&mi, 0, sizeof(mi));
        //    mi.CtlType = ODT_LISTBOX;
        //    if (HandleMeasureItem(Parent(), &mi))
        //        itemheight = mi.itemHeight;
        //}

        if (seltype != lstSingleSelect)
        //{
        //    if (sel >= 0 && sel < Items().Count())
        //        SetSelected(sel);
        //}
        //else
        {
            BeginUpdate();
            for (auto it = sellist.sbegin(); it != sellist.send(); ++it)
                SetSelected(it.base() - sellist.begin(), true);
            EndUpdate();
            Invalidate();
        }
        sellist.clear();

        if (kind != lckOwnerDrawVariable && itemheight >= 0)
            SetItemHeight(itemheight);
    }

    bool Listbox::HandleMeasureItem(Control *parent, MEASUREITEMSTRUCT *measures)
    {
        if (!OnMeasureItem || kind != lckOwnerDrawVariable)
            return false;

        OnMeasureItem(this, MeasureItemParameters(GetCanvas(), measures->itemID, measures->itemWidth, measures->itemHeight, measures->itemData));
        return true;
    }

    bool Listbox::HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures)
    {
        if (!OnDrawItem)
            return false;

        Canvas *c = GetCanvas();
        if ((int)measures->itemID >= 0)
        {
            if ((measures->itemState & ODS_SELECTED) == ODS_SELECTED)
            {
                c->SelectStockBrush(sbHighlight);
                c->GetFont().SetColor(clHighlightText);
            }
            OnDrawItem(this, DrawItemParameters(c, measures->itemID, measures->rcItem, measures->itemState, measures->itemData));
        }
        else
        {
            c->FillRect(measures->rcItem);
            if ((measures->itemState & ODS_FOCUS) == ODS_FOCUS)
            {
                HDC dc = c->GetDC();
                DrawFocusRect(dc, &measures->rcItem);
                c->ReturnDC();
            }
        }
        return true;
    }

    ListControlKinds Listbox::Kind()
    {
        return kind;
    }

    void Listbox::SetKind(ListControlKinds newkind)
    {
        if (kind == newkind)
            return;

        int itemh = 0;

        if (kind == lckOwnerDrawVariable)
            itemh = ItemHeight();

        ListControlKinds oldkind = kind;
        kind = newkind;
        SetNodata(kind == lckVirtual || kind == lckVirtualOwnerDraw);

        if (HandleCreated())
            RecreateHandle();

        if (oldkind == lckOwnerDrawVariable)
            SetItemHeight(itemh);
    }

    ListboxSelectionTypes Listbox::SelectionType()
    {
        return seltype;
    }

    void Listbox::SetSelectionType(ListboxSelectionTypes newseltype)
    {
        if (seltype == newseltype)
            return;

        bool updatesel = !HandleCreated() && (newseltype == lstSingleSelect) != (seltype == lstSingleSelect);
        int sel = updatesel || newseltype == lstSingleSelect ? ItemIndex() : -1;

        if (!HandleCreated() && newseltype == lstSingleSelect)
            base::SetItemIndex(ItemIndex());

        seltype = newseltype;

        if (HandleCreated())
        {
            RecreateHandle();
            if (newseltype == lstSingleSelect)
                SetItemIndex(sel);
        }
        else if (updatesel)
        {
            sellist.clear();

            if (seltype != lstSingleSelect)
            {
                sellist.resize(Count());
                if (sel >= 0 && sel < Count())
                    SetItemIndex(sel);
            }
        }
    }

    void Listbox::NotifyList(ListControlNotifyCodes code, int a, int b)
    {
        switch (code)
        {
        case lcncClear:
            if (seltype != lstSingleSelect)
                break;
            sellist.resize(0);
            break;
        case lcncDelete:
            if (seltype != lstSingleSelect)
                break;
            sellist.erase(sellist.begin() + a, sellist.begin() + a + b);
            break;
        case lcncInsert:
            if (seltype != lstSingleSelect)
                break;
            sellist.insert_space(sellist.begin() + a, b);
            break;
        case lcncSwap:
            if (seltype != lstSingleSelect)
                break;
            if (sellist.is_set(a) == sellist.is_set(b))
                break;
            if (sellist.is_set(a))
            {
                sellist[b] = true;
                sellist.unset(a);
            }
            else
            {
                sellist[a] = true;
                sellist.unset(b);
            }
            break;
        case lcncSetCount:
        case lcncAllocate:
            if (seltype != lstSingleSelect)
                break;
            sellist.resize(a);
            break;
        case lcncSelect:
            if (seltype != lstSingleSelect)
                break;
            sellist.clear();
            sellist.resize(Count());
            if (a >= 0)
                sellist[a] = true;
            break;
        }

        base::NotifyList(code, a, b);
    }

    int Listbox::SelCount()
    {
        if (HandleCreated())
        {
            LRESULT r = SendMessage(Handle(), LB_GETSELCOUNT, 0, 0);
            if (r != LB_ERR) // Single selection boxes return LB_ERR.
                return r;
        }

        if (seltype == lstSingleSelect)
        {
            int s = ItemIndex();
            return s < 0 || s >= Count() ? 0 : 1;
        }

        return sellist.data_size();
    }

    bool Listbox::IntegralHeight()
    {
        return integralheight;
    }

    void Listbox::SetIntegralHeight(bool newintegralheight)
    {
        if (newintegralheight == integralheight)
            return;

        integralheight = newintegralheight;
        if (kind != lckOwnerDrawVariable && HandleCreated())
            RecreateHandle();
    }

    /*
    int Listbox::Selected()
    {
        // Only works correctly for single selection listboxes. Use the Selected(int) function for multiple selection boxes instead.
        if (HandleCreated())
        {
            LRESULT r = SendMessage(Handle(), LB_GETCURSEL, 0, 0);
            if (r == LB_ERR)
                return -1;
            return r;
        }
        else
            return sel;
    }

    void Listbox::SetSelected(int index)
    {
        if (Selected() == index)
            return;

        index = min(max(index, -1), Items().Count() - 1);
        if (HandleCreated())
        {
            LRESULT error = SendMessage(Handle(), LB_SETCURSEL, index, 0);
            if (index >= 0 && error == LB_ERR)
                throw L"Couldn't select the item with the given index.";

            SelChanged();
            if (OnChanged)
                OnChanged(this, EventParameters());
        }

        sel = index;
    }
    */

    int Listbox::TopRow()
    {
        if (!HandleCreated())
            return 0;

        return SendMessage(Handle(), LB_GETTOPINDEX, 0, 0);
    }

    bool Listbox::Selected(int index)
    {
        if (seltype == lstSingleSelect)
            return index == ItemIndex();

        if (HandleCreated())
        {
            LRESULT r = SendMessage(Handle(), LB_GETSEL, index, 0);
            if (r == LB_ERR)
                throw L"Error getting item selection.";

            return r > 0;
        }
        if (index < 0 || index >= Count())
            throw L"Item index out of bounds.";

        return sellist.is_set(index);
    }

    void Listbox::SetSelected(int index, bool sel)
    {
        if (seltype == lstSingleSelect)
        {
            if (sel)
                SetItemIndex(index, true);
            else if (ItemIndex() == index)
                SetItemIndex(-1, true);
            return;
        }

        if (HandleCreated())
        {
            LRESULT error = SendMessage(Handle(), LB_SETSEL, sel ? TRUE : FALSE, index);
            if (error == LB_ERR)
                throw L"Error selecting items.";
            SelChanged();
            return;
        }

        if (index < -1 || index >= Count())
            throw L"Item index out of bounds.";
        if (index == -1)
        {
            sellist.clear();
            if (!sel)
                sellist.resize(Count());
            else
                sellist.insert(sellist.begin(), Count(), true);
            return;
        }

        if (sel)
            sellist[index] = true;
        else
            sellist.unset(index);
    }

    int Listbox::ItemIndex()
    {
        if (seltype == lstSingleSelect)
            return base::ItemIndex();

        if (!HandleCreated())
            return -1;
        LRESULT res = SendMessage(Handle(), LB_GETCARETINDEX, 0, 0);
        if (res == LB_ERR)
            return -1;
        return Selected(res) ? res : -1;
    }

    void Listbox::SetItemIndex(int newindex, bool triggerevent)
    {
        if (seltype == lstSingleSelect)
        {
            base::SetItemIndex(newindex, triggerevent);
            return;
        }

        newindex = min(max(newindex, -1), Count() - 1);
        if (!HandleCreated())
        {
            sellist.clear();
            sellist.resize(Count());
            if (newindex >= 0)
                sellist[newindex] = true;
        }
        else
        {
            BeginUpdate();
            LRESULT error = SendMessage(Handle(), LB_SETSEL, FALSE, -1);
            if (newindex >= 0 && error != LB_ERR)
                error = SendMessage(Handle(), LB_SETSEL, TRUE, newindex);
            EndUpdate();
            if (error == LB_ERR)
                throw L"Couldn't change selection in list box.";
            Invalidate();
        }

#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Text");
#endif
    }


    void Listbox::SelectAll()
    {
        SetSelected(-1, true);
    }

    void Listbox::DeselectAll()
    {
        SetSelected(-1, false);
    }

    void Listbox::ScrollIntoView()
    {
        if (!HandleCreated())
            return;

        int ix = ItemIndex();
        if (ix < 0)
            return;

        Rect r;
        SendMessage(Handle(), LB_GETITEMRECT, ix, (LPARAM)&r);
        if (!r.Empty())
        {
            Rect cr = ClientRect();
            if (r.top >= cr.top && r.bottom <= cr.bottom)
                return;
        }

        SendMessage(Handle(), LB_SETCURSEL, ix, 0);
    }

    int Listbox::ItemHeight()
    {
        if (!HandleCreated())
            return itemheight;

        if (kind != lckOwnerDrawVariable && HandleCreated())
        {
            LRESULT r = SendMessage(Handle(), LB_GETITEMHEIGHT, 0, 0);
            if (r == LB_ERR)
                throw L"Error reading list box item height.";
            itemheight = r;
        }

        return itemheight;
    }

    void Listbox::SetItemHeight(int newheight)
    {
        newheight = max(0, min(newheight, 255));
        if (kind != lckOwnerDrawVariable && HandleCreated())
        {
            LRESULT error = SendMessage(Handle(), LB_SETITEMHEIGHT, 0, newheight);
            if (error == LB_ERR)
                throw L"Error setting item height.";
        }
        else
            itemheight = newheight;
    }

    Rect Listbox::RowRect(int index)
    {
        if (!HandleCreated())
            return Rect();

        Rect r;
        LRESULT error = SendMessage(Handle(), LB_GETITEMRECT, index, (LPARAM)&r);
        if (error == LB_ERR)
            throw L"Error getting item rectangle.";
        return r;

    /*    int tr = TopRow();
        if (index < tr || index >= Items()->Count())
            return Rect(0,0,0,0);

        index -= tr;

        Rect r;
        r.left = 0;
        r.right = ClientRect().Width();
        int h = ItemHeight();
        r.top = index * h;
        r.bottom = r.top + h;

        return r;
    */

    }

    int Listbox::ItemAt(Point pt)
    {
        return ItemAt(pt.x, pt.y);
    }

    int Listbox::ItemAt(short x, short y)
    {
        if (!HandleCreated() || Count() == 0)
            return -1;

        return LOWORD(SendMessage(Handle(), LB_ITEMFROMPOINT, 0, MAKELPARAM(x, y)));
    }


    //---------------------------------------------


    LRESULT CALLBACK ComboboxEditorProc(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        HWND parent = GetAncestor(wnd, GA_PARENT);
        if (parent)
        {
            MessageStruct msg;
            msg.hwnd = wnd;
            msg.uMsg = uMsg;
            msg.lParam = lParam;
            msg.wParam = wParam;
            msg.result = 0;
            return SendMessage(parent, wmChildMessage, 0, (LPARAM)&msg);
        }
        throw L"Combobox editor outside a combobox?";
    }


    int Combobox::cbmessages[cemEnumEnd] = { CB_INITSTORAGE, CB_GETCOUNT, CB_GETLBTEXTLEN, CB_GETLBTEXT, CB_ADDSTRING, 
											 CB_INSERTSTRING, CB_DELETESTRING, CB_GETITEMDATA, CB_SETITEMDATA, CB_RESETCONTENT, 
											 CB_GETCURSEL, CB_SETCURSEL, CB_ERR, CB_ERRSPACE };

    int Combobox::cbnotify[lcnEnumEnd] = {
            CBN_SELCHANGE,
    };


#ifdef DESIGNING


    ValuePair<ComboboxTypes> ComboboxTypeStrings[] = {
        VALUEPAIR(ctDropdown),
        VALUEPAIR(ctDropdownList),
    };

    void Combobox::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);

        serializer->HideProperty(L"BorderStyle");
        serializer->Add(L"SetKind", new ListControlKindsDesignProperty<Combobox>(L"Kind", L"Control", &Combobox::Kind, &Combobox::SetKind))->SetDefault(lckNormal);
        serializer->Add(L"SetType", new ComboboxTypesDesignProperty<Combobox>(L"Type", L"Control", &Combobox::Type, &Combobox::SetType))->SetDefault(ctDropdown);
        serializer->MoveHere(L"Items");
        serializer->MoveHere(L"ItemIndex");
    }

    Size Combobox::DesignSize()
    {
        return Size(80, CurrentFontHeight() + max(themes->MeasureEditBorderWidth().cy * 2, BaseToPixel(0, 5).cy) );
    }
#endif

    Combobox::Combobox() : base(cbmessages, cbnotify), kind(lckNormal), type(ctDropdown), itemheight(-1), autoselect(true),
            doautosel(false), mouseselpt(0,0), selstart(0), sellength(0), editwnd(NULL)
    {
        SetBorderStyle(bsNormal);
        SetWantedKeyTypes(wkOthers | wkArrows);
        controlstyle << csInTabOrder << csAcceptInput;
    }

    Combobox::~Combobox()
    {
    }

    void Combobox::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Combobox";
    }

    void Combobox::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        if (type == ctDropdown)
            params.style << cbsDropdown << cbsAutoHScroll << wsVScroll;
        else if (type == ctDropdownList)
            params.style << cbsDropdownList;
        else
            throw L"Not implemented";

        if (kind != lckVirtual && kind != lckVirtualOwnerDraw)
        {
            params.style << cbsHasStrings;
            if (kind == lckOwnerDraw)
                params.style << cbsOwnerdrawFixed;
            else if (kind == lckOwnerDrawVariable)
                params.style << cbsOwnerdrawVariable;
        }
        else
            params.style << cbsOwnerdrawFixed;
    }

    LRESULT Combobox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //switch (uMsg)
        //{
        //case WM_DESTROY:
        //    if (editwnd)
        //        SetWindowLongPtr(editwnd, GWLP_WNDPROC, (LONG)editproc);
        //    editwnd = NULL;
        //    break;
        //}

        return base::WindowProc(uMsg, wParam, lParam);
    }

    void Combobox::HandleChildMessage(HWND hwnd, UINT &uMsg, WPARAM &wParam, LPARAM &lParam, LRESULT &result)
    {
        if (hwnd != editwnd)
            return;

        switch (uMsg)
        {
        case WM_DESTROY:
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);

            SetWindowLongPtr(editwnd, GWLP_WNDPROC, (LONG)editproc);
            HWND origwnd;
            origwnd = editwnd;
            editwnd = NULL;
            listwnd = NULL;
            result = CallWindowProc(editproc, origwnd, uMsg, wParam, lParam);
            editproc = NULL;
            break;
        case WM_GETDLGCODE:
            result = CallWindowProc(editproc, editwnd, uMsg, wParam, lParam);
            if (!autoselect)
                result &= ~DLGC_HASSETSEL; // DLGC_HASSETSEL is used to notify windows that it can send the control an EM_SETSEL message to select the whole text.
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
            return;
        case WM_MOUSEACTIVATE:
            if (autoselect && (lParam >> 16) == WM_LBUTTONDOWN && !Focused())
                doautosel = true;
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
            break;
        case WM_MOUSEMOVE:
            if ((wParam & MK_LBUTTON) == MK_LBUTTON)
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                int xd = abs(GetSystemMetrics(SM_CXDRAG));
                int yd = abs(GetSystemMetrics(SM_CYDRAG));
                if (abs(mouseselpt.x - x) > xd || abs(mouseselpt.y - y) > yd)
                    doautosel = false;
            }
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            int x;
            x = GET_X_LPARAM(lParam);
            int y;
            y = GET_Y_LPARAM(lParam);

            mouseselpt = Point(x, y);
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
            break;
        case WM_LBUTTONUP:
            if (doautosel)
            {
                SetSelStartAndLength(0, Text().size());
                doautosel = false;
            }
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
        case WM_CHAR:
            result = 0;
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
            if (wParam == 0)
                return;
            break;
        default:
            result = 0;
            base::HandleChildMessage(hwnd, uMsg, wParam, lParam, result);
        }
        result = CallWindowProc(editproc, editwnd, uMsg, wParam, lParam);
    }

    static bool isfocused = false;
    void Combobox::GainFocus(HWND otherwindow)
    {
        if ((editwnd && editwnd == otherwindow) || DroppedDown())
            return;
        isfocused = true;
        base::GainFocus(otherwindow);
    }

    void Combobox::LoseFocus(HWND otherwindow)
    {
        if ((editwnd && editwnd == otherwindow) || DroppedDown())
            return;
        isfocused = false;
        base::LoseFocus(otherwindow);
    }

    bool Combobox::DroppedDown()
    {
        if (!HandleCreated())
            return false;
        return SendMessage(Handle(), CB_GETDROPPEDSTATE, 0, 0) != FALSE;
    }

    void Combobox::SetDroppedDown(bool newdropped)
    {
        if (!HandleCreated())
            return;
        SendMessage(Handle(), CB_SHOWDROPDOWN, newdropped ? TRUE : FALSE, 0);
    }

    bool Combobox::HandleChildCommand(HWND hwnd, WPARAM wParam)
    {
        if (hwnd != editwnd)
            return false;

        if (HIWORD(wParam) == EN_UPDATE)
        {
            if (OnTextChanged)
                OnTextChanged(this, EventParameters());
            return true;
        }
        return false;
    }

    void Combobox::InitHandle()
    {
        // The text is not set for the editor in the combo box at window creation. We have to save it before InitHandle() which clears it.
        std::wstring txt = Text();

        base::InitHandle();

        //if (kind != ckOwnerDrawVariable)
        //{
        //    MEASUREITEMSTRUCT mi;
        //    memset(&mi, 0, sizeof(mi));
        //    mi.CtlType = ODT_COMBOBOX;
        //    if (HandleMeasureItem(Parent(), &mi))
        //        itemheight = mi.itemHeight;
        //}

        //if (sel >= 0 && sel < Items()->Count())
        //    SetSelected(sel);

        SetText(txt);

        if (itemheight >= 0 && kind != lckOwnerDrawVariable)
            SetItemHeight(itemheight);

        COMBOBOXINFO inf;
        inf.cbSize = sizeof(COMBOBOXINFO);
        SendMessage(Handle(), CB_GETCOMBOBOXINFO, 0, (LPARAM)&inf);
        listwnd = inf.hwndList;
        if (type == ctDropdown)
        {
            editwnd = inf.hwndItem; //GetWindow(Handle(), GW_CHILD);
            if (editwnd)
            {
                editproc = (PWndProc)SetWindowLongPtr(editwnd, GWLP_WNDPROC, (LONG)&ComboboxEditorProc);
                if (!editproc)
                    editwnd = NULL;
                else
                    SetSelStartAndLength(selstart, sellength);
            }
        }
    }

    void Combobox::SaveWindow()
    {
        itemheight = ItemHeight();
        _SelStartAndLength();
        base::SaveWindow();
    }

    bool Combobox::Focused()
    {
        if (!HandleCreated())
            return false;

        HWND h = GetFocus();
        return (editwnd && h == editwnd) || (Handle() == h);
    }

    void Combobox::Focus()
    {
        if (Focused() || !HandleCreated())
            return;

        if (editwnd)
            SetFocus(editwnd);
        else
            base::Focus();
    }

    void Combobox::TabFocus()
    {
        if (!HandleCreated() || Focused())
            return;
        controlstate << csTabFocusing;
        if (editwnd)
            SetFocus(editwnd);
        else
            SetFocus(Handle());
    }

    ComboboxTypes Combobox::Type()
    {
        return type;
    }

    void Combobox::SetType(ComboboxTypes newtype)
    {
        if (type == newtype)
            return;

        type = newtype;
        if (HandleCreated())
            RecreateHandle();
    }

    ListControlKinds Combobox::Kind()
    {
        return kind;
    }

    void Combobox::SetKind(ListControlKinds newkind)
    {
        if (kind == newkind)
            return;

        kind = newkind;
        SetNodata(newkind == lckVirtual || newkind == lckVirtualOwnerDraw);
        if (HandleCreated())
            RecreateHandle();
    }

    int Combobox::ItemHeight()
    {
        if (HandleCreated() && kind != lckOwnerDrawVariable)
        {
            LRESULT r = SendMessage(Handle(), CB_GETITEMHEIGHT, 0, 0);
            if (r == LB_ERR)
                throw L"Error reading list box item height.";
            itemheight = r;
        }

        return itemheight;
    }

    void Combobox::SetItemHeight(int newheight)
    {
        newheight = max(0,min(newheight, 255));
        if (HandleCreated() && kind != lckOwnerDrawVariable)
        {
            LRESULT error = SendMessage(Handle(), CB_SETITEMHEIGHT, 0, newheight);
            if (error == LB_ERR)
                throw L"Error setting item height.";
        }
        else
            itemheight = newheight;
    }

    int Combobox::SelStart()
    {
        if (type != ctDropdown)
            return 0;

        _SelStartAndLength();
        return selstart;
    }

    int Combobox::SelLength()
    {
        if (type != ctDropdown)
            return 0;

        _SelStartAndLength();
        return sellength;
    }

    void Combobox::_SelStartAndLength()
    {
        if (type != ctDropdown)
            return;

        if (editwnd)
        {
            SendMessage(editwnd, EM_GETSEL, (WPARAM)&selstart, (LPARAM)&sellength);
            sellength = sellength - selstart;
        }
    }

    void Combobox::SelStartAndLength(int &start, int &length)
    {
        if (type != ctDropdown)
            return;

        _SelStartAndLength();
        start = selstart;
        length = sellength;
    }

    void Combobox::SetSelStartAndLength(int start, int length)
    {
        if (type != ctDropdown)
            return;

        _SelStartAndLength();

        int len = TextLength();
        if (start < 0)
            start = 0;
        else if (start > len)
            start = len;
        length = min(start + length, len) - start;

        if (start == selstart && length == sellength)
            return;

        selstart = start;
        sellength = length;
        if (editwnd)
        {
            length += start;
            SendMessage(editwnd, EM_SETSEL, (WPARAM)selstart, (WPARAM)length);
        }
    }

    void Combobox::SetSelStart(int newstart)
    {
        if (type != ctDropdown)
            return;

        _SelStartAndLength();
        int len = TextLength();
        if (newstart < 0)
            newstart = 0;
        else if (newstart > len)
            newstart = len;
        if (selstart == newstart)
            return;

        selstart = newstart;
        if (editwnd)
        {
            sellength = newstart;
            SendMessage(editwnd, EM_SETSEL, (WPARAM)selstart, (WPARAM)sellength);
        }
        sellength = 0;
    }

    void Combobox::SetSelLength(int newlength)
    {
        if (type != ctDropdown)
            return;

        _SelStartAndLength();
        int len = TextLength();
        if (newlength < 0)
            newlength = 0;
        if (selstart + newlength > len)
            newlength = len - selstart;
        if (newlength == sellength)
            return;
        if (editwnd)
        {
            sellength = selstart + newlength;
            SendMessage(editwnd, EM_SETSEL, (WPARAM)selstart, (WPARAM)sellength);
        }
        sellength = newlength;
    }

    std::wstring Combobox::SelText()
    {
        if (type != ctDropdown)
            return L"";

        _SelStartAndLength();
        if (sellength == 0)
            return L"";
        return Text().substr(selstart, sellength);
    }

    void Combobox::SetSelText(const std::wstring& newseltext)
    {
        if (type != ctDropdown)
            return;

        if (!editwnd)
        {
            _SelStartAndLength();
            std::wstring text = Text();
            text.replace(selstart, sellength, newseltext);
            selstart += newseltext.length();
            sellength = 0;
            SetText(text);
            return;
        }

        SendMessage(editwnd, EM_REPLACESEL, TRUE, (LPARAM)newseltext.c_str());
    }

    bool Combobox::AutoSelect()
    {
        return autoselect;
    }

    void Combobox::SetAutoSelect(bool newautoselect)
    {
        autoselect = newautoselect;
    }


    //---------------------------------------------


#ifdef DESIGNING
    std::wstring Tab::DefaultText()
    {
        return std::wstring();
    }

    Imagelist* Tab::Images()
    {
        if (!owner)
            return NULL;
        return owner->Images();
    }

    Object* Tab::SubOwner()
    {
        return owner;
    }

    Object* Tab::MainControl()
    {
        return owner;
    }

    void Tab::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"Name");
        serializer->HideProperty(L"AccessLevel");
        serializer->Add(L"SetText", new StringDesignProperty<Tab>(L"Text", L"Appearance", &Tab::Text, &Tab::SetText))->SetDefault(&Tab::DefaultText)->SetDefaultWrite(std::wstring())->SetImmediateUpdate(true)->DontExport()->MakeDefault();
        serializer->Add(L"SetIndex", new IntDesignProperty<Tab>(L"Index", L"Position", &Tab::Index, &Tab::SetIndex))->DontWrite();
        serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<Tab>(L"ImageIndex", L"Appearance", true, &Tab::Images, &Tab::ImageIndex, &Tab::SetImageIndex))->SetDefault(-1);
    }
#endif

    Tab::Tab() : owner(NULL), imageindex(-1)
    {}

    Tab::Tab(const Tab &other)
    {}

    Tab::~Tab()
    {
    }

    void Tab::Destroy()
    {
        if (owner && owner->HandleCreated())
            owner->RemoveTab(this);
        owner = NULL;

        base::Destroy(); // TODO: Wasn't added. Check if this was a bug.
        //delete this;
    }

    TabControl* Tab::OwnerControl()
    {
        return owner;
    }

    void Tab::SetOwnerAndIndex(TabControl *newowner, int index)
    {
        if (newowner)
            newowner->InsertTab(this, index);
        else if (owner)
            SetOwnerControl(NULL);
    }

    void Tab::SetOwnerControl(TabControl *newowner)
    {
        if (owner == newowner)
            return;

        TabControl *tmp = owner;
        owner = newowner;
        if (owner)
            owner->AddTab(this);
        else
            tmp->RemoveTab(this);
    }

    int Tab::Index()
    {
        return owner ? owner->TabIndex(this) : -1;
    }

    void Tab::SetIndex(int newindex)
    {
        if (!owner || newindex < 0)
            return;
        SetOwnerAndIndex(owner, newindex);
    }

    const std::wstring& Tab::Text()
    {
        return text;
    }

    void Tab::SetText(const std::wstring &newtext)
    {
        if (text == newtext)
            return;
        text = newtext;
        if (!owner || !owner->HandleCreated())
            return;

        TCITEM item = TabControl::FillTabItemInfo(this, TCIF_TEXT);
        SendMessage(owner->Handle(), TCM_SETITEM, Index(), (LPARAM)&item);

#ifdef DESIGNING
        Changed(CHANGE_TAB);
#endif
    }

#ifdef DESIGNING
    void Tab::SetName(const std::wstring& newname)
    {
        if (Text() == Name())
        {
            SetText(newname);
            if (owner && owner->Designing() && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"Text");
        }
        base::SetName(newname);
    }
#endif

    Form* Tab::ParentForm() const
    {
        if (owner)
            return owner->ParentForm();
        return NULL;
    }

    int Tab::ImageIndex()
    {
        return imageindex;
    }

    void Tab::SetImageIndex(int newimageindex)
    {
        if (imageindex == newimageindex)
            return;
        imageindex = newimageindex;
        if (!owner || !owner->HandleCreated())
            return;

        TCITEM item = TabControl::FillTabItemInfo(this, TCIF_IMAGE);
        SendMessage(owner->Handle(), TCM_SETITEM, Index(), (LPARAM)&item);

#ifdef DESIGNING
        if (owner->Images())
            Changed(CHANGE_TAB);
#endif
    }


    //---------------------------------------------


    TabPages::TabPages(TabControl *owner) : owner(owner)
    {
    }

    TabPages::~TabPages()
    {
    }

    void TabPages::Destroy()
    {
        ;
    }

    Tab* TabPages::Items(int index)
    {
        if (index < 0 || index >= (int)tabs.size())
            throw L"Index of tab out of range.";

        return tabs[index];
    }

    int TabPages::Size()
    {
        if (tabs.empty())
            return 0;

        return tabs.size();
    }

    Tab* TabPages::Add()
    {
        return owner->AddTab();
    }

    int TabPages::TabIndex(Tab *tab)
    {
        auto it = std::find(tabs.begin(), tabs.end(), tab);
        if (it == tabs.end())
            return -1;
        return it - tabs.begin();
    }

    void TabPages::Insert(int index, Tab *tab)
    {
        if (index < 0 || index >= (int)tabs.size())
            index = tabs.size();
        tabs.insert(tabs.begin() + index, tab);
    }

    void TabPages::Remove(int index)
    {
        if (index < 0 || index >= (int)tabs.size())
            throw L"Index of tab out of range.";

        tabs.erase(tabs.begin() + index);
    }


    //---------------------------------------------


#ifdef DESIGNING
    Size TabControl::DesignSize()
    {
        return Size(180, 80);
    }

    std::wstring TabsSerializeArgs(Object *control)
    {
        Tab *tpo = dynamic_cast<Tab*>(control);
        if (!tpo)
            return std::wstring();
        return L"L\"" + EscapeCString(tpo->Text()) + L"\"";
    }

    void TabControl::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->SetContainerControl(true);
        serializer->AllowSubType(typeid(Tab));
        //serializer->HideProperty(L"AcceptInput");
        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);

        serializer->Add(L"SetImages", new ImagelistDesignProperty<TabControl>(L"Images", L"Appearance", &TabControl::Images, &TabControl::SetImages));
        serializer->Add(L"AddTab", new TabVectorDesignProperty<TabControl>(L"Tabs", &TabControl::TabCount, &TabControl::Tabs, &TabControl::DesignAddTab));
        serializer->Add(L"SetSelectedTab", new IntDesignProperty<TabControl>(L"SelectedTab", L"Behavior", &TabControl::SelectedTab, &TabControl::SetSelectedTab));

        serializer->AddEvent<TabControl, TabChangingEvent>(L"OnTabChanging", L"Control");
        serializer->AddEvent<TabControl, TabChangeEvent>(L"OnTabChange", L"Control");
    }

    TabPages* TabControl::GetTabPages()
    {
        return tabs;
    }

    void TabControl::DesignAddTab()
    {
        AddTab();
    }

    void TabControl::InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems)
    {
        inserteditems.push_back( menu_item_data(std::wstring(L"Add tab"), 0, CreateEvent(this, &TabControl::addtabclick)) );

        if (tabs->Size())
        {
            int tabclickpos = TabAt(clientpos);
            if (tabclickpos < 0)
                tabclickpos = tabindex;

            inserteditems.push_back( menu_item_data(L"Delete tab", tabclickpos, CreateEvent(this, &TabControl::deletetabclick)) );

            if (tabindex < tabs->Size() - 1)
                inserteditems.push_back( menu_item_data(L"Next tab", tabindex, CreateEvent(this, &TabControl::nexttabclick)) );
            if (tabindex > 0)
                inserteditems.push_back( menu_item_data(L"Previous tab", tabindex, CreateEvent(this, &TabControl::prevtabclick)) );
        }
    }

    bool TabControl::NeedDesignerHittest(int x, int y, LRESULT hittest)
    {
        Rect r;
        if (scrollhwnd && IsWindowVisible(scrollhwnd) && GetWindowRect(scrollhwnd, &r) != FALSE && PtInRect(&r, ClientToScreen(x, y)))
            return true;

        return TabAt(x, y) >= 0;
    }

    void TabControl::addtabclick(void *sender, EventParameters param)
    {
        AddTab();
        SetSelectedTab(tabs->Size() - 1);
    }

    void TabControl::deletetabclick(void *sender, EventParameters param)
    {
        DeleteTab(((MenuItem*)sender)->Tag());
    }

    void TabControl::nexttabclick(void *sender, EventParameters param)
    {
        SetSelectedTab(((MenuItem*)sender)->Tag() + 1);
    }

    void TabControl::prevtabclick(void *sender, EventParameters param)
    {
        SetSelectedTab(((MenuItem*)sender)->Tag() - 1);
    }

    bool TabControl::DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (button != mbLeft)
            return false;

        int tab = TabAt(x, y);
        if (tab < 0)
            return false;

        DesignSelectTab(form, tab);

        if (vkeys.contains(vksDouble))
        {
            DesignProperty *prop = SerializerByTypeInfo(typeid(Tab))->DefaultProperty();
            if (prop != nullptr)
                designer->EditProperty(prop->Name(), true);
        }

        return false;
    }

    bool TabControl::DesignKeyPush(DesignForm *form, WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        if (tabs->Size() == 0)
            return false;

        bool update = false;

        if (keycode == VK_DELETE || key == VK_BACK)
        {
            DeleteTab(tabindex);
            update = true;
        }
        else if (keycode == VK_LEFT)
        {
            if (tabindex > 0)
            {
                SetSelectedTab(tabindex - 1);
                update = true;
            }
        }
        else if (keycode == VK_RIGHT)
        {
            if (tabindex < tabs->Size() - 1)
            {
                SetSelectedTab(tabindex + 1);
                update = true;
            }
        }
        //else if (keycode == VK_TAB && tabs->Size() > 1)
        //{
        //    SetSelectedTab(0);
        //    update = true;
        //}
        else
            return false;

        if (update)
            DesignSelectTab(dynamic_cast<DesignForm*>(DesignParent()), SelectedTab());

        return true;
    }

    bool TabControl::DesignTabNext(bool entering, bool backwards)
    {
        if (tabs->Size() == 0 || (entering && !backwards))
            return false;

        DesignForm *form = dynamic_cast<DesignForm*>(DesignParent());
        Control *control;
        Object *propowner;
        tagtype tag;
        form->GetSelectData(this, control, propowner, tag);

        if (dynamic_cast<Tab*>(propowner) != nullptr && (backwards || !entering))
        {
            if (!backwards)
                return false;
            form->SelectDesignControl(this);
            return true;
        }
        else if (dynamic_cast<Tab*>(propowner) == nullptr && backwards && !entering)
            return false;

        DesignSelectTab(form, SelectedTab());

        return true;
    }

    void TabControl::DesignSelectTab(DesignForm *form, int tabix)
    {
        form->SelectDesignControl(this, TabRect(tabix), Tabs(tabix), tabix);
    }

    //void TabControl::DoSelectTab(void *sender, DesignFormBase::SubObjectSelectParameters param)
    //{
    //    int ix = tabs->TabIndex((Tab*)param.subobj);
    //    if (ix < 0)
    //        return;
    //    SetSelectedTab(ix);
    //    DesignSelectTab((DesignForm*)sender, ix);
    //}

    bool TabControl::TabDesigned(Tab *tab)
    {
        DesignForm *form = dynamic_cast<DesignForm*>(DesignParent());
        if (!form)
            return false;
        Control *c;
        Object *p;
        int t;
        return form->GetSelectData(this, c, p, t) && p == tab;
    }

    void TabControl::SetName(const std::wstring& newname)
    {
        std::wstring oldname = Name();

        base::SetName(newname);

        if (Designing() && DesignParent())
        {
            for (int ix = 0; ix < tabs->Size(); ++ix)
            {
                Tab *t = tabs->Items(ix);
                DesignParent()->SubObjectRenamed(this, t, oldname + L"->Tabs(" + IntToStr(ix) + L")", Name() + L"->Tabs(" + IntToStr(ix) + L")");
            }
        }
    }


    //Object* TabControl::NameOwner(const std::wstring &name)
    //{
    //    Object *obj = base::NameOwner(name);
    //    if (!obj)
    //    {
    //        for (int ix = 0; ix < tabs->Size(); ++ix)
    //            if (tabs->Items(ix)->Name() == name)
    //                return tabs->Items(ix);
    //    }
    //    return obj;
    //}

    //void TabControl::Names(std::vector<std::wstring> &namelist)
    //{
    //    base::Names(namelist);
    //    for (int ix = 0; ix < tabs->Size(); ++ix)
    //        namelist.push_back(tabs->Items(ix)->Name());
    //}

    void TabControl::DesignSubSelected(Object *subobj)
    {
        if (dynamic_cast<Tab*>(subobj) != nullptr)
        {
            int ix = tabs->TabIndex((Tab*)subobj /*param.subobj*/);
            if (ix < 0)
                return;
            SetSelectedTab(ix);
            DesignSelectTab(/*(DesignForm*)sender*/ dynamic_cast<DesignForm*>(DesignParent()), ix);
        }
    }
#endif

    TabControl::TabControl() : scrollhwnd(NULL), tabs(NULL), images(NULL), tabindex(-1)
    {
        controlstyle /* << csSelfErased */ << csInTabOrder << csAcceptInput;
        tabs = new TabPages(this);
        SetWantedKeyTypes(wkOthers | wkArrows);
        InitControlList();
    }

    TabControl::~TabControl()
    {
    }

    void TabControl::Destroy()
    {
        for (int i = tabs->Size() - 1; i >= 0; --i)
            DeleteTab(i);

        delete tabs;
        tabs = nullptr;

        base::Destroy();
    }

    void TabControl::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_TAB_CLASSES);
        params.classname = L"SysTabControl32";
        base::CreateClassParams(params);
        params.style -= csVRedraw;
        params.style -= csHRedraw;
    }

    void TabControl::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << tcsFocusNever;
    }

    LRESULT TabControl::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
#ifdef DESIGNING
        DesignForm *form;
        Object *o;
        Control *c;
        Tab *tp;
        int i;
#endif
        Imagelist *tmp;

        switch (uMsg)
        {
        //case WM_DESTROY:
        //    if (!controlstate.contains(csDestroying))
        //        break;
        //    i = tabs->Size();
        //    for (; i >= 0; --i)
        //        DeleteTab(i);
        //    break;
#ifdef DESIGNING
        case WM_HSCROLL:
            form = dynamic_cast<DesignForm*>(DesignParent());
            if (!form || !form->GetSelectData(this, c, o, i))
                break;
            tp = dynamic_cast<Tab*>(o);
            if (!tp)
                break;
            ChangeNotify(tp, CHANGE_TAB);
            break;
#endif
        case WM_PARENTNOTIFY:
            if (LOWORD(wParam) != WM_CREATE && (LOWORD(wParam) != WM_DESTROY || scrollhwnd != (HWND)lParam))
                break;
            if (LOWORD(wParam) == WM_CREATE)
            {
                if (application->ControlFromHandle((HWND)lParam) == NULL && HIWORD(wParam) == 1/*GetWindowLongPtr((HWND)lParam, GWLP_ID) == 1*/)
                    scrollhwnd = (HWND)lParam;
            }
            else
                scrollhwnd = NULL;
            break;
        case wmImagelistChanged:
            tmp = images;
            SetImages(NULL);
            SetImages(tmp);
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void TabControl::InitHandle()
    {
        base::InitHandle();
        if (images)
            SendMessage(Handle(), TCM_SETIMAGELIST, 0, (LPARAM)images->Handle());
        for (int ix = 0; ix < tabs->Size(); ++ix)
        {
            TCITEM item = FillTabItemInfo(tabs->Items(ix), TCIF_IMAGE | TCIF_TEXT);
            SendMessage(Handle(), TCM_INSERTITEM, ix, (LPARAM)&item);
        }
        tabindex = min(max(0, tabindex), tabs->Size() - 1);
        if (tabindex >= 0)
        {
            SendMessage(Handle(), TCM_SETCURSEL, tabindex, 0);
            NMHDR nmh;
            nmh.hwndFrom = Handle();
            nmh.idFrom = GetWindowLongPtr(Handle(), GWLP_ID);
            nmh.code = TCN_SELCHANGE;
            HRESULT tmp = 0;
            HandleNotify(Parent(), (LPARAM)&nmh, tmp);
        }
    }

    /*Rect TabControl::OpaqueRect()
    {
        if (!tabs->Size() || !UsingParentBackground())
            return base::OpaqueRect();
        Rect cr = ClientRect();
        return Rect(0, TabRect(0).bottom, cr.right, cr.bottom);
    }*/

    bool TabControl::ExcludeOpaqueRegion(HRGN rgn, const Rect &rgnrect, const Point &origin)
    {
        return false;
        Rect cr = ClientRect().Offset(origin).Intersect(rgnrect);
        Rect r = TabRect(0);
        cr.top += min(cr.Height(), tabs->Size() ? r.bottom : 2);
        CombineRgnWithRect(rgn, rgn, cr, rcmDiff);

        for (int ix = 0; ix < tabs->Size(); ++ix)
            CombineRgnWithRect(rgn, rgn, TabRect(ix).Offset(origin).Intersect(rgnrect), rcmDiff);

        return true;
    }

    void TabControl::EraseBackground()
    {
        Rect r = ClientRect();
        if (!tabs->Size())
            r.bottom = 2;
        else
            r.bottom = TabRect(0).bottom;
        Canvas *c = GetCanvas();

        HRGN rgn = 0;
        if (tabs->Size())
        {
            rgn = CreateRectRgn(0, 0, 0, 0);
            for (int ix = 0; ix < tabs->Size(); ++ix)
                CombineRgnWithRect(rgn, rgn, TabRect(ix), rcmOr);
            if (scrollhwnd && IsWindowVisible(scrollhwnd))
            {
                Rect r;
                GetWindowRect(scrollhwnd, &r);
                CombineRgnWithRect(rgn, rgn, r, rcmOr);
            }
            c->ExcludeClip(rgn);
        }
        if (UsingParentBackground())
            DrawParentBackground(r);
        else
            c->FillRect(r);

        if (rgn)
        {
            c->ResetClip();
            DeleteObject(rgn);
        }

        //return true;
    }

    bool TabControl::Paint()
    {
        return false;
    }

    TCITEM TabControl::FillTabItemInfo(Tab *tab, UINT mask)
    {
        TCITEM item;
        memset(&item, 0, sizeof(TCITEM));
        item.mask = mask;
        if ((mask & TCIF_TEXT) == TCIF_TEXT)
        {
            item.pszText = const_cast<wchar_t*>(tab->Text().c_str());
            item.cchTextMax = tab->Text().length() + 1;
        }
        if ((mask & TCIF_IMAGE) == TCIF_IMAGE)
            item.iImage = tab->ImageIndex();
        return item;
    }

    bool TabControl::HandleNotify(Control *parent, LPARAM lParam, HRESULT &result)
    {
        NMHDR *nmh = (NMHDR*)lParam;
        bool b;

        switch (nmh->code)
        {
        case TCN_SELCHANGING:
            b = true;
            if (OnTabChanging)
                OnTabChanging(this, TabChangingParameters(tabindex, b));
            result = !b;
            return true;
        case TCN_SELCHANGE:
            if (HandleCreated())
                tabindex = PassMessage(TCM_GETCURSEL, 0, 0);

            if (OnTabChange)
                OnTabChange(this, TabChangeParameters(tabindex));
            return true;
        }
        return false;
    }

    void TabControl::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        if (object == images)
        {
            images = nullptr;
            if (HandleCreated())
                SendMessage(Handle(), TCM_SETIMAGELIST, 0, 0);
            return;
        }
    }

    void TabControl::ChangeNotify(Object *object, int changetype)
    {
        base::ChangeNotify(object, changetype);

        if (object == images && HandleCreated())
        {
            PostMessage(Handle(), wmImagelistChanged, 0, 0);
            //Imagelist *tmp = images;
            //SetImages(NULL);
            //SetImages(tmp);

            //InvalidateTabs();
        }
#ifdef DESIGNING
        else if (Designing() && changetype == CHANGE_TAB) // Update the size indication of a tab page. This notification is not useful for changing tabs in the designer.
        {
            DesignForm *form = dynamic_cast<DesignForm*>(DesignParent());
            if (!form)
                return;

            Tab *tab = dynamic_cast<Tab*>(object);
            int tabix;
            if (!tab || (tabix = tabs->TabIndex(tab)) < 0 || !TabDesigned(tab))
                return;

            DesignSelectTab(form, tabix);
        }
#endif
    }

    void TabControl::InvalidateTabs()
    {
        for (int ix = 0; ix < tabs->Size(); ++ix)
            InvalidateRect(TabRect(ix));
    }

    void TabControl::UpdateTab(Tab *tab, UINT mask)
    {
        if (tab->OwnerControl() != this)
            return;
    }

    Tab* TabControl::AddTab()
    {
        Tab *tab = new Tab();

#ifdef DESIGNING
        int num = 1;
        if (Designing())
        {
            for (int i = 0; i < tabs->Size(); ++i)
            {
                Tab *t = tabs->Items(i);
                int l = t->Text().length();
                while (l > 3 && t->Text()[l - 1] >= L'0' || t->Text()[l - 1] <= L'9')
                    --l;
                if (l != 3 || t->Text().substr(0, 3) != L"Tab")
                    continue;

                int tmp;
                if (StrToInt(t->Text(), tmp, 3))
                    num = max(num, tmp + 1);
            }
        }
        tab->SetText(L"Tab" + IntToStr(num));
#endif

        InsertTab(tab, -1);
#ifdef DESIGNING
        //tab->SetName(name);
        if (Designing() && DesignParent())
            DesignParent()->RegisterSubObject(this, tab, Name() + L"->Tabs(" + IntToStr(tabs->Size() - 1) + L")");
#endif

        return tab;
    }

    int TabControl::AddTab(Tab *tab)
    {
        if (!tab || tab->OwnerControl() == this)
            return -1;
        return InsertTab(tab, -1);
    }

    Tab* TabControl::AddTab(const std::wstring& tabtext)
    {
        Tab *tab = AddTab();
        tab->SetText(tabtext);
        return tab;
    }

    int TabControl::InsertTab(Tab *tab, int index)
    {
        if (!tab)
            return -1;

        int ix = TabIndex(tab);
        index = min(tabs->Size() - (ix >= 0 ? 1 : 0), index < 0 ? tabs->Size() : index);
        if (tab->OwnerControl() == this && ix == index)
            return index;
        if (tab->OwnerControl())
            tab->OwnerControl()->RemoveTab(tab);

        tabs->Insert(index, tab);
        if (HandleCreated())
        {
            TCITEM item = FillTabItemInfo(tab, TCIF_IMAGE | TCIF_TEXT);
            index = SendMessage(Handle(), TCM_INSERTITEM, index, (LPARAM)&item);
        }
        tab->SetOwnerControl(this);
        AddToNotifyList(tab, nrSubControl);

#ifdef DESIGNING
        if (TabDesigned(tab))
            ChangeNotify(tab, CHANGE_TAB);
#endif

        if (tabs->Size() == 1)
            Invalidate(true);

        if (tabindex < 0)
            SetSelectedTab(index);
        return index;
    }

    int TabControl::TabCount() const
    {
        return tabs->Size();
    }

    int TabControl::TabIndex(Tab *tab)
    {
        if (tab->OwnerControl() != this)
            return -1;
        int tabix = tabs->TabIndex(tab);
        if (tabix < 0)
        {
            RemoveTab(tab);
            return -1;
        }
        else
            return tabix;
    }

    Tab* TabControl::Tabs(int index)
    {
        if (index < 0 || index >= tabs->Size())
            return nullptr;
        return tabs->Items(index);
    }

    Tab* TabControl::RemoveTab(int index)
    {
        if (index < 0 || index >= tabs->Size())
            return nullptr;

        Tab *tab = tabs->Items(index);

        bool selected = SelectedTab() == index;
#ifdef DESIGNING
        bool designselected = !controlstate.contains(csDestroying) && Designing() && dynamic_cast<DesignForm*>(DesignParent())->IsPropertyOwnerSelected(this, tab, false);
#endif
        if (!controlstate.contains(csDestroying) && HandleCreated())
        {
            if (SendMessage(Handle(), TCM_DELETEITEM, index, 0) == FALSE)
                return nullptr;
        }

        RemoveFromNotifyList(tab, nrSubControl);
        tabs->Remove(index);

#ifdef DESIGNING
        if (Designing() && DesignParent())
        {
            DesignParent()->UnregisterSubObject(this, tab);

            for (int ix = index; ix < tabs->Size(); ++ix)
            {
                Tab *t = tabs->Items(ix);
                DesignParent()->SubObjectRenamed(this, t, Name() + L"->Tabs(" + IntToStr(ix + 1) + L")", Name() + L"->Tabs(" + IntToStr(ix) + L")");
            }
        }
#endif

        tab->SetOwnerControl(nullptr);

        if (controlstate.contains(csDestroying))
            return tab;

        index = min(tabs->Size() - 1, index);

        if (selected)
        {
            if (index >= 0)
            {
                int newtabindex = max(0, tabindex - 1);
                tabindex = -1;
                SetSelectedTab(newtabindex);
#ifdef DESIGNING
                if (HandleCreated() && designselected)
                    DesignSelectTab(dynamic_cast<DesignForm*>(DesignParent()), tabindex);
#endif
            }
#ifdef DESIGNING
            else
            {
                tabindex = -1;
                if (Designing() && designselected)
                    dynamic_cast<DesignForm*>(DesignParent())->SelectDesignControl(this);
            }
#endif
        }

        return tab;
    }

    void TabControl::RemoveTab(Tab *tab)
    {
        int ix = TabIndex(tab);
        if (ix < 0)
            return;
        RemoveTab(ix);
    }

    void TabControl::DeleteTab(int index)
    {
        tabs->Items(index)->Destroy();
    }

    int TabControl::TabAt(int x, int y)
    {
        TCHITTESTINFO inf;
        inf.pt = Point(x, y);
        return SendMessage(Handle(), TCM_HITTEST, 0, (LPARAM)&inf);
    }

    int TabControl::TabAt(Point xy)
    {
        return TabAt(xy.x, xy.y);
    }

    Rect TabControl::TabRect(int index)
    {
        if (index < 0 || index >= tabs->Size())
            return Rect();

        Rect r;
        if (SendMessage(Handle(), TCM_GETITEMRECT, index, (LPARAM)&r) != FALSE)
            return r;
        return Rect();
    }

    Imagelist* TabControl::Images()
    {
        return images;
    }

    void TabControl::SetImages(Imagelist *newimages)
    {
        if (images == newimages)
            return;
        if (images)
            RemoveFromNotifyList(images, nrSubControl);
        images = newimages;
        if (images)
            AddToNotifyList(images, nrSubControl);
        if (!HandleCreated())
            return;

        SendMessage(Handle(), TCM_SETIMAGELIST, 0, !images ? 0 : (LPARAM)images->Handle());
    }

    void TabControl::MeasureControlArea(Rect &clientrect)
    {
        if (!HandleCreated())
            return;
        SendMessage(Handle(), TCM_ADJUSTRECT, FALSE, (LPARAM)&clientrect);
    }

    void TabControl::Resizing()
    {
        //Invalidate(false);
        RedrawWindow(Handle(), NULL, 0, RDW_INVALIDATE | RDW_ERASE);
        base::Resizing();
    }

    int TabControl::SelectedTab()
    {
        return tabindex;
    }

    void TabControl::SetSelectedTab(int newseltab)
    {
        newseltab = min(max(0, newseltab), tabs->Size() - 1);
        if (tabindex == newseltab)
            return;
        if (HandleCreated())
            SendMessage(Handle(), TCM_SETCURSEL, newseltab, 0);
        else
            tabindex = newseltab;

        if (Parent())
        {
            NMHDR nmh;
            nmh.hwndFrom = HandleCreated() ? Handle() : NULL;
            nmh.idFrom = HandleCreated() ? GetWindowLongPtr(Handle(), GWLP_ID) : 0;
            nmh.code = TCN_SELCHANGE;
            HRESULT tmp = 0;
            HandleNotify(Parent(), (LPARAM)&nmh, tmp);
        }
    }


    //---------------------------------------------


#ifdef DESIGNING

    void TabPage::FinishDeserialize()
    {
        base::FinishDeserialize();
        Show();
    }

    Object* TabPage::PropertyOwner()
    {
        return owner;
    }

    Object* TabPage::MainControl()
    {
        return owner->OwnerControl();
    }
#endif

    TabPage::TabPage(TabPageOwner *owner) : owner(owner)
    {
        SetParentBackground(false);

        controlstyle << csInTabOrder;

        SetAlignment(alClient);
        SetText(std::wstring());
        InitControlList();
    }

    TabPage::~TabPage()
    {
    }

    void TabPage::Destroy()
    {
        if (owner)
        {
            owner->Destroy();
            return;
        }
        base::Destroy();
    }

    void TabPage::InitHandle()
    {
        base::InitHandle();
        Show();
    }

    TabPageOwner* TabPage::Owner()
    {
        return owner;
    }

    void TabPage::EraseBackground()
    {
        //themes->TestMeasure();
        int b = Parent()->ScreenToClient(ClientToScreen(0, 0)).x;
        Rect r = ClientRect().Offset(-b, -b, b, b);
        themes->DrawTabPane(GetCanvas(), r);
        //return true;
    }

    void TabPage::Show()
    {
        if (!owner->OwnerControl() || !owner->OwnerControl()->HandleCreated())
        {
            Hide();
            return;
        }

        if (owner->Index() == owner->OwnerControl()->PassMessage(TCM_GETCURSEL, 0, 0))
            base::Show();
        else
            base::Hide();
    }

    void TabPage::Hide()
    {
        if (owner->OwnerControl() && HandleCreated())
            if (owner->Index() == owner->OwnerControl()->PassMessage(TCM_GETCURSEL, 0, 0))
                return;
        base::Hide();
    }

    PageControl* TabPage::OwnerControl()
    {
        return owner->OwnerControl();
    }

    void TabPage::SetOwnerControl(PageControl *newowner)
    {
        owner->SetOwnerControl(newowner);
    }

    void TabPage::SetOwnerAndIndex(PageControl *newowner, int index)
    {
        owner->SetOwnerAndIndex(newowner, index);
    }

    int TabPage::Index()
    {
        return owner->Index();
    }

    void TabPage::SetIndex(int newindex)
    {
        owner->SetIndex(newindex);
    }

    std::wstring TabPage::Text() const
    {
        return owner->Text();
    }

    void TabPage::SetText(const std::wstring &newtext)
    {
        owner->SetText(newtext);
    }

    int TabPage::ImageIndex() const
    {
        return owner->ImageIndex();
    }

    void TabPage::SetImageIndex(int newimageindex)
    {
        owner->SetImageIndex(newimageindex);
    }


    //---------------------------------------------


#ifdef DESIGNING
    Object* DesignCreateTabPage(Object *owner)
    {
        return (new TabPageOwner())->panel;
    }

    void TabPageOwner::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->SetCreateFunction(&DesignCreateTabPage);

        serializer->Find<NameDesignProperty<Object>>(L"Name", dpuHidden)->DoList()->DoSerialize();
        serializer->Find<AccessLevelDesignProperty<Object>>(L"AccessLevel", dpuHidden)->DoList()->DoSerialize();
        serializer->Find<StringDesignProperty<Tab>>(L"Text")->DontExport();
        serializer->SetContainerControl(true);

        serializer->Add(L"SetIndex", new IntDesignProperty<TabPageOwner>(L"Index", L"Position", &TabPageOwner::DesignIndex, &TabPageOwner::DesignSetIndex))->DontWrite();
        serializer->Add(std::wstring(), new IntDesignProperty<TabPageOwner>(L"Left", std::wstring(), &TabPageOwner::DesignLeft, NULL))->DontList()->DontExport();
        serializer->Add(std::wstring(), new IntDesignProperty<TabPageOwner>(L"Top", std::wstring(), &TabPageOwner::DesignTop, NULL))->DontList()->DontExport();
        serializer->Add(std::wstring(), new IntDesignProperty<TabPageOwner>(L"Width", std::wstring(), &TabPageOwner::DesignWidth, NULL))->DontList()->DontExport();
        serializer->Add(std::wstring(), new IntDesignProperty<TabPageOwner>(L"Height", std::wstring(), &TabPageOwner::DesignHeight, NULL))->DontList()->DontExport();
        serializer->Add(L"SetBounds", new RectDesignProperty<TabPageOwner>(std::wstring(), std::wstring(), true, false, &TabPageOwner::DesignWindowRect, NULL))->DontList()->DontSerialize();
    }

    void TabPageOwner::InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems)
    {
        PageControl *owner = OwnerControl();
        if (owner)
            owner->InitDesignerMenu(owner->ScreenToClient(panel->ClientToScreen(clientpos)), inserteditems);
    }

    void TabPageOwner::SetName(const std::wstring& newname)
    {
        base::SetName(newname);
        panel->SetName(newname);
    }

    int TabPageOwner::DesignIndex()
    {
        return Index();
    }

    void TabPageOwner::DesignSetIndex(int newindex)
    {
        SetIndex(newindex);

        // Tell the designer form about the index change and move this page in the z-order. (Controls are written in their z-order to the project file.)
        if (panel->Designing() && OwnerControl() && dynamic_cast<DesignForm*>(OwnerControl()->ParentForm()))
        {
            PageControl *pg = OwnerControl();
            DesignForm *f = dynamic_cast<DesignForm*>(pg->ParentForm());

            int ix = Index();
            if (pg->TabCount() > 1)
            {
                if (ix > 0)
                    f->MoveControlAbove(panel, pg->TabPages(ix - 1));
                else
                    f->MoveControlBelow(panel, pg->TabPages(ix + 1));
            }
        }
    }

    int TabPageOwner::DesignLeft()
    {
        if (OwnerControl() && OwnerControl()->HandleCreated())
        {
            Rect r = OwnerControl()->ClientRect();
            OwnerControl()->MeasureControlArea(r);
            return r.left;
        }
        return panel->Left();
    }

    int TabPageOwner::DesignTop()
    {
        if (OwnerControl() && OwnerControl()->HandleCreated())
        {
            Rect r = OwnerControl()->ClientRect();
            OwnerControl()->MeasureControlArea(r);
            return r.top;
        }
        return panel->Top();
    }

    int TabPageOwner::DesignWidth()
    {
        if (OwnerControl() && OwnerControl()->HandleCreated())
        {
            Rect r = OwnerControl()->ClientRect();
            OwnerControl()->MeasureControlArea(r);
            return r.Width();
        }
        return panel->Width();
    }

    int TabPageOwner::DesignHeight()
    {
        if (OwnerControl() && OwnerControl()->HandleCreated())
        {
            Rect r = OwnerControl()->ClientRect();
            OwnerControl()->MeasureControlArea(r);
            return r.Height();
        }
        return panel->Height();
    }

    Rect TabPageOwner::DesignWindowRect()
    {
        if (OwnerControl() && OwnerControl()->HandleCreated())
        {
            Rect r = OwnerControl()->ClientRect();
            OwnerControl()->MeasureControlArea(r);
            return r;
        }
        return panel->WindowRect();
    }

    bool TabPageOwner::DesignSelectChanged(Object *control, bool selected)
    {
        if (selected && OwnerControl())
            OwnerControl()->SetActivePageIndex(Index());
        return false;
    }
#endif

    TabPageOwner::TabPageOwner() : base(), panel(NULL)
    {
        panel = new TabPage(this);
        panel->Hide();
    }

    TabPageOwner::~TabPageOwner()
    {
    }

    void TabPageOwner::Destroy()
    {
        TabPage *page = panel;
        base::Destroy();
        page->owner = nullptr;
        page->Destroy();
    }

    void TabPageOwner::SetOwnerControl(TabControl *newowner)
    {
        if (OwnerControl() == newowner)
            return;
        base::SetOwnerControl(newowner);

        if (newowner)
        {
#ifdef DESIGNING
            if (newowner->Designing())
                ((DesignForm*)newowner->ParentForm())->PlaceControl(panel, newowner, OwnerControl()->Serializer()->Find(L"Tabs"));
            else
                panel->SetParent(newowner);
#else
            panel->SetParent(newowner);
#endif
            AddToNotifyList(panel, nrSubControl);
        }
        else if (panel)
        {
            panel->Hide();
            panel->SetParent(NULL);
        }
    }

    PageControl* TabPageOwner::OwnerControl()
    {
        return (PageControl*)base::OwnerControl();
    }

    void TabPageOwner::SetOwnerControl(PageControl *newowner)
    {
        SetOwnerControl((TabControl*)newowner);
    }

    void TabPageOwner::SetOwnerAndIndex(PageControl *newowner, int index)
    {
        base::SetOwnerAndIndex((TabControl*)newowner, index);
    }

    //void TabPageOwner::DeleteNotify(Object *object)
    //{
    //    base::DeleteNotify(object);
    //    if (object == panel && OwnerControl())
    //        OwnerControl()->DeleteTabPage(Index());
    //}


    //---------------------------------------------


#ifdef DESIGNING
    void PageControl::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        int ix = serializer->PropertyIndex(L"Tabs");
        serializer->RenameSerializer(ix, L"AddTabPage");
        serializer->Find<TabVectorDesignProperty<TabControl>>(L"Tabs")->Derive()->SetParentCreationReferenceType(pcrtFormDeclare);

        auto tabs = serializer->Find<TabVectorDesignProperty<TabControl>>(L"Tabs");

        //serializer->HideProperty(L"AcceptInput");

        serializer->HideProperty(L"SelectedTab");
        serializer->SetContainerControl(false);
        serializer->DisallowSubType(typeid(Tab));
        serializer->AllowSubType(typeid(TabPageOwner));
        serializer->Add(L"SetActivePage", new TabPageDesignProperty<PageControl>(L"ActivePage", L"Behavior", &PageControl::ActivePage, &PageControl::SetActivePage, &PageControl::TabCount, &PageControl::TabPages));
    }

    bool PageControl::DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        form->SelectDesignControl(this);
        return false;
    }
#endif

    PageControl::PageControl()
    {
    }

    PageControl::~PageControl()
    {
    }

    bool PageControl::HandleNotify(Control *parent, LPARAM lParam, HRESULT &result)
    {
        NMHDR *nmh = (NMHDR*)lParam;
        int i;
        int sel;
        Rect r;
    //#ifdef DESIGNING
    //    bool selchange = false;
    //#endif

        switch (nmh->code)
        {
        case TCN_SELCHANGE:
            i = HandleCreated() ? PassMessage(TCM_GETCURSEL, 0, 0) : -1; // New selected tab index.
            sel = SelectedTab(); // Old value not updated yet.
            if (i >= 0)
            {
#ifdef DESIGNING
                if (HandleCreated() && Designing() && /*DesignParent()->IsControlSelected(this, true) && !*/ dynamic_cast<DesignForm*>(DesignParent())->IsControlSelected(this, false))
                {
                    //selchange = true;
                    dynamic_cast<DesignForm*>(DesignParent())->SelectDesignControl(TabPages(i));
                }
#endif
                TabPages(i)->Show();
            }
            if (sel != i && sel >= 0 && TabCount() > sel) // The old value is the same as the new when a new tab is added.
            {
    //#ifdef DESIGNING
    //            if (!selchange && Designing() && DesignParent()->IsControlSelected(this, true) && !DesignParent()->IsControlSelected(this, false))
    //                DesignParent()->SelectDesignControl(this);
    //#endif
                TabPages(sel)->Hide();
            }
            break;
        }

        return base::HandleNotify(parent, lParam, result);
    }

    //Control* PageControl::CreateSubControl(ControlTypes type)
    //{
    //    if (type != ctTabPage)
    //        return NULL;
    //    return (new TabPageOwner())->panel;
    //}

    void PageControl::ChildAddedNotify(Control *parent, Control *child)
    {
        base::ChildAddedNotify(parent, child);
        if (parent != this)
            return;
        TabPage *panel = dynamic_cast<TabPage*>(child);
        if (!panel || TabIndex(panel->Owner()) >= 0)
            return;

        AddTabPage(panel);
    }

    Tab* PageControl::AddTab()
    {
        std::wstringstream name;
        TabPageOwner *tab = new TabPageOwner();

#ifdef DESIGNING
        int num = 1;
        if (Designing() && ParentForm())
            num = max(num, ((DesignForm*)ParentForm())->NameNext(L"TabPage"));
        name << L"TabPage" << num;
        tab->SetText(name.str());
#endif

        InsertTabPage(tab->panel, -1);
#ifdef DESIGNING
        tab->SetName(name.str());
#endif

        return tab;
    }

    TabPage* PageControl::AddTabPage()
    {
        return ((TabPageOwner*)AddTab())->panel;
    }

    TabPage* PageControl::AddTabPage(const std::wstring& tabtext)
    {
        TabPageOwner *owner = ((TabPageOwner*)base::AddTab(tabtext));
        return owner ? owner->panel : NULL;
    }

    int PageControl::AddTabPage(TabPage *tab)
    {
        return base::AddTab(tab->Owner());
    }

    int PageControl::InsertTabPage(TabPage *tab, int index)
    {
        return base::InsertTab(tab->Owner(), index);
    }

    TabPage* PageControl::RemoveTabPage(int index)
    {
        TabPageOwner *owner = ((TabPageOwner*)base::RemoveTab(index));
        return owner ? owner->panel : NULL;
    }

    void PageControl::RemoveTabPage(TabPage *tab)
    {
        base::RemoveTab(tab->Owner());
    }

    int PageControl::TabPageIndex(TabPage *tab)
    {
        return base::TabIndex(tab->Owner());
    }

    TabPage* PageControl::TabPages(int index)
    {
        TabPageOwner *owner = (TabPageOwner*)base::Tabs(index);
        if (!owner)
            throw L"Tab page index out of range.";
        return owner ? owner->panel : NULL;
    }

    void PageControl::DeleteTabPage(int index)
    {
        TabPages(index)->Destroy();
    }

    int PageControl::TabPageCount()
    {
        return base::TabCount();
    }

    TabPage* PageControl::ActivePage()
    {
        if (SelectedTab() < 0)
            return NULL;
        return TabPages(SelectedTab());
    }

    int PageControl::ActivePageIndex()
    {
        return SelectedTab();
    }

    void PageControl::SetActivePage(TabPage *newactivepage)
    {
        if (!newactivepage || newactivepage->OwnerControl() != this)
            return;

        SetActivePageIndex(TabPageIndex(newactivepage));
    }

    void PageControl::SetActivePageIndex(int index)
    {
        int sel = SelectedTab();
        if (index < 0 || index >= base::TabCount() || sel == index)
            return;

        SetSelectedTab(index);
    }


    //---------------------------------------------


#ifdef DESIGNING
    ValuePair<HeaderColumnSortDirections> HeaderColumnSortDirectionStrings[] = {
            VALUEPAIR(hcsdNone),
            VALUEPAIR(hcsdUp),
            VALUEPAIR(hcsdDown),
};

    Imagelist* HeaderColumn::Images()
    {
        if (!owner)
            return nullptr;
        return owner->SmallImages();
    }

    Object* HeaderColumn::SubOwner()
    {
        return owner;
    }

    bool HeaderColumn::SubShown()
    {
        return false;
    }

    std::wstring HeaderColumn::DefaultText()
    {
        return Name();
    }

    int HeaderColumn::DefaultSubIndex()
    {
        if (!owner)
            return false;
        int index = Index();
        int minval = -1;
        for (int ix = 0; ix < index; ++ix)
            minval = max(minval, owner->Columns(ix)->SubIndex());

        return minval + 1;
    }

    void HeaderColumn::SetName(const std::wstring& newname)
    {
        if (Text() == Name())
        {
            SetText(newname);
            if ((!owner || owner->Designing()) && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"Text");
        }
        base::SetName(newname);
    }

    void HeaderColumn::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->Add(L"SetText", new StringDesignProperty<HeaderColumn>(L"Text", L"Appearance", &HeaderColumn::Text, &HeaderColumn::SetText))->SetDefault(&HeaderColumn::DefaultText)->SetDefaultWrite(std::wstring())->SetImmediateUpdate(true);
        serializer->Add(L"SetTextAlignment", new TextAlignmentsDesignProperty<HeaderColumn>(L"TextAlignment", L"Layout", &HeaderColumn::TextAlignment, &HeaderColumn::SetTextAlignment))->SetDefault(taLeft);
        serializer->Add(L"SetWidth", new IntDesignProperty<HeaderColumn>(L"Width", L"Dimensions", &HeaderColumn::Width, &HeaderColumn::SetWidth))->SetDefault(64);
        serializer->Add(L"SetDefaultWidth", new IntDesignProperty<HeaderColumn>(L"DefaultWidth", L"Dimensions", &HeaderColumn::DefaultWidth, &HeaderColumn::SetDefaultWidth))->SetDefault(0);
        serializer->Add(L"SetImagesOnRight", new BoolDesignProperty<HeaderColumn>(L"ImagesOnRight", L"Items", &HeaderColumn::ImagesOnRight, &HeaderColumn::SetImagesOnRight))->SetDefault(false);
        serializer->Add(L"SetSortDirection", new HeaderColumnSortDirectionsDesignProperty<HeaderColumn>(L"SortDirection", L"Appearance", &HeaderColumn::SortDirection, &HeaderColumn::SetSortDirection))->SetDefault(hcsdNone);
        serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<HeaderColumn>(L"ImageIndex", L"Appearance", true, &HeaderColumn::Images, &HeaderColumn::ImageIndex, &HeaderColumn::SetImageIndex))->SetDefault(-1);
        serializer->Add(L"SetSubIndex", new IntDesignProperty<HeaderColumn>(L"SubIndex", L"Behavior", &HeaderColumn::SubIndex, &HeaderColumn::SetSubIndex))->SetDefault(&HeaderColumn::DefaultSubIndex);
        /* Properties only useful after Vista: */
        serializer->Add(L"SetFixedWidth", new BoolDesignProperty<HeaderColumn>(L"FixedWidth", L"Behavior", &HeaderColumn::FixedWidth, &HeaderColumn::SetFixedWidth))->SetDefault(false);
        serializer->Add(L"SetNoDPIScale", new BoolDesignProperty<HeaderColumn>(L"NoDPIScale", L"Dimensions", &HeaderColumn::NoDPIScale, &HeaderColumn::SetNoDPIScale))->SetDefault(false);
        serializer->Add(L"SetFixedRatio", new BoolDesignProperty<HeaderColumn>(L"FixedRatio", L"Dimensions", &HeaderColumn::FixedRatio, &HeaderColumn::SetFixedRatio))->SetDefault(false);
        serializer->Add(L"SetSplitButton", new BoolDesignProperty<HeaderColumn>(L"SplitButton", L"Behavior", &HeaderColumn::SplitButton, &HeaderColumn::SetSplitButton))->SetDefault(false);
    }
#endif

    HeaderColumn::HeaderColumn(Listview *owner) :
            owner(owner), textalign(taLeft), imageonright(false),  width(64), subindex(0), imageindex(-1), pos(0), defwidth(0), sortdir(hcsdNone),
            /* Vista and above: */
            fixedwidth(false), nodpiscale(false), fixedratio(false), splitbutton(false), minwidth(0)
    {
    }

    void HeaderColumn::UpdateSubIndex()
    {
        if (!owner)
            return;

        int index = Index();
        int minval = -1;
        for (int ix = 0; ix < index; ++ix)
            minval = max(minval, owner->Columns(ix)->SubIndex());
        subindex = minval + 1;
        if (owner->HandleCreated())
            owner->UpdateColumnData(this, LVCF_FMT | LVCF_SUBITEM);
    }


#ifdef DESIGNING
    HeaderColumn::HeaderColumn(HeaderColumn *other) : base(*other), owner(NULL),
        textalign(other->textalign), imageonright(other->imageonright), width(other->Width()), subindex(other->subindex),
        imageindex(other->imageindex), pos(other->Position()), defwidth(other->defwidth), sortdir(other->sortdir),
        /* Vista and above: */
        fixedwidth(other->fixedwidth), nodpiscale(other->nodpiscale), fixedratio(other->fixedratio), splitbutton(other->splitbutton), minwidth(other->minwidth)
    {
        SetName(other->Name());
        text = other->text;
    }
#endif

    HeaderColumn::~HeaderColumn()
    {
    }

    LVCOLUMN HeaderColumn::FillData(HeaderColumn *column, UINT mask)
    {
        LVCOLUMN lvc = {0};

        lvc.mask = mask;

        if ((mask & LVCF_FMT) == LVCF_FMT)
        {
            switch (column->textalign)
            {
            case taLeft:
                lvc.fmt = LVCFMT_LEFT;
                break;
            case taRight:
                lvc.fmt = LVCFMT_RIGHT;
                break;
            case taCenter:
                lvc.fmt = LVCFMT_CENTER;
                break;
            default:
                break;
            }

            if (column->imageindex >= 0 && column->owner && column->owner->SmallImages())
                lvc.fmt |= LVCFMT_IMAGE | LVCFMT_COL_HAS_IMAGES;

            if (column->imageonright)
                lvc.fmt |= LVCFMT_BITMAP_ON_RIGHT;

            if (column->fixedwidth)
                lvc.fmt |= LVCFMT_FIXED_WIDTH;
            if (column->nodpiscale)
                lvc.fmt |= LVCFMT_NO_DPI_SCALE;
            if (column->fixedratio)
                lvc.fmt |= LVCFMT_FIXED_RATIO;
            if (column->splitbutton)
                lvc.fmt |= LVCFMT_SPLITBUTTON;

            if (column->sortdir == hcsdUp)
                lvc.fmt |= HDF_SORTUP;
            else if (column->sortdir == hcsdDown)
                lvc.fmt |= HDF_SORTDOWN;
        }
        if ((mask & LVCF_WIDTH) == LVCF_WIDTH)
            lvc.cx = column->width;
        if ((mask & LVCF_TEXT) == LVCF_TEXT)
            lvc.pszText = const_cast<wchar_t*>(column->text.c_str());

        if ((mask & LVCF_SUBITEM) == LVCF_SUBITEM)
            lvc.iSubItem = column->subindex;

        lvc.iImage = column->imageindex;
        if (column->imageindex < 0)
            mask &= ~LVCF_IMAGE;

        if ((mask & LVCF_ORDER) == LVCF_ORDER)
            lvc.iOrder = column->pos;
        if ((mask & LVCF_MINWIDTH) == LVCF_MINWIDTH)
            lvc.cxMin = column->minwidth;

        if ((mask & LVCF_DEFAULTWIDTH) == LVCF_DEFAULTWIDTH)
            lvc.cxDefault = column->defwidth;

        return lvc;
    }

    Form* HeaderColumn::ParentForm() const
    {
        return owner->ParentForm();
    }

    Listview* HeaderColumn::OwnerControl()
    {
        return owner;
    }

    int HeaderColumn::Index()
    {
        return owner ? owner->ColumnIndex(this) : -1;
    }

    const std::wstring& HeaderColumn::Text()
    {
        return text;
    }

    void HeaderColumn::SetText(const std::wstring& newtext)
    {
        if (text == newtext)
            return;
        text = newtext;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_TEXT);
    }

    TextAlignments HeaderColumn::TextAlignment()
    {
        return textalign;
    }

    void HeaderColumn::SetTextAlignment(TextAlignments newalign)
    {
        if (textalign == newalign)
            return;
        textalign = newalign;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_FMT);
    }

    int HeaderColumn::Width()
    {
        if (owner && owner->HandleCreated() && owner->Style() == ldsDetails)
            width = SendMessage(owner->Handle(), LVM_GETCOLUMNWIDTH, Index(), 0);
        return width;
    }

    void HeaderColumn::SetWidth(int newwidth)
    {
        newwidth = max(minwidth, newwidth);
        if (width == newwidth)
            return;

        width = newwidth;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_WIDTH);
    }

    int HeaderColumn::DefaultWidth()
    {
        return defwidth;
    }

    void HeaderColumn::SetDefaultWidth(int newdefwidth)
    {
        if (defwidth == newdefwidth)
            return;
        defwidth = newdefwidth;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_DEFAULTWIDTH);
    }

    int HeaderColumn::ImageIndex()
    {
        return imageindex;
    }

    void HeaderColumn::SetImageIndex(int newindex)
    {
        if (imageindex == newindex)
            return;
        imageindex = newindex;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_IMAGE | LVCF_FMT);
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    bool HeaderColumn::ImagesOnRight()
    {
        return imageonright;
    }

    void HeaderColumn::SetImagesOnRight(bool newimageonright)
    {
        if (imageonright == newimageonright)
            return;
        imageonright = newimageonright;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    int HeaderColumn::Position()
    {
        if (owner && owner->HandleCreated() && owner->Style() == ldsDetails)
        {
            LVCOLUMN lvc = {0};
            lvc.mask = LVCF_ORDER;
            if (SendMessage(owner->Handle(), LVM_GETCOLUMN, Index(), (LPARAM)&lvc) != FALSE)
                pos = lvc.iOrder;
        }
        return pos;
    }

    void HeaderColumn::SetPosition(int newposition)
    {
        newposition = max(0, newposition);
        if (owner)
            newposition = min (newposition, owner->ColumnCount() - 1);

        if (pos == newposition)
            return;

        pos = newposition;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_ORDER);
    }

    HeaderColumnSortDirections HeaderColumn::SortDirection()
    {
        return sortdir;
    }

    void HeaderColumn::SetSortDirection(HeaderColumnSortDirections newsortdir)
    {
        if (sortdir == newsortdir)
            return;
        sortdir = newsortdir;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    /* Vista and above: */
    bool HeaderColumn::FixedWidth()
    {
        return fixedwidth;
    }

    void HeaderColumn::SetFixedWidth(bool newfixedwidth)
    {
        if (fixedwidth == newfixedwidth)
            return;
        fixedwidth = newfixedwidth;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    bool HeaderColumn::NoDPIScale()
    {
        return nodpiscale;
    }

    void HeaderColumn::SetNoDPIScale(bool newnodpiscale)
    {
        if (nodpiscale == newnodpiscale)
            return;
        nodpiscale = newnodpiscale;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    bool HeaderColumn::FixedRatio()
    {
        return fixedratio;
    }

    void HeaderColumn::SetFixedRatio(bool newfixedratio)
    {
        if (fixedratio == newfixedratio)
            return;
        fixedratio = newfixedratio;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    bool HeaderColumn::SplitButton()
    {
        return splitbutton;
    }

    void HeaderColumn::SetSplitButton(bool newsplitbutton)
    {
        if (splitbutton == newsplitbutton)
            return;
        splitbutton = newsplitbutton;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT);
    }

    int HeaderColumn::MinimumWidth()
    {
        return minwidth;
    }

    void HeaderColumn::SetMinimumWidth(int newminwidth)
    {
        if (minwidth == newminwidth)
            return;
        minwidth = newminwidth;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_MINWIDTH);
    }

    int HeaderColumn::SubIndex()
    {
        return subindex;
    }

    void HeaderColumn::SetSubIndex(int newsubindex)
    {
        if (subindex == newsubindex)
            return;
        subindex = newsubindex;
        if (!owner || !owner->HandleCreated())
            return;
        owner->UpdateColumnData(this, LVCF_FMT | LVCF_SUBITEM);
    }


    //---------------------------------------------


#ifdef DESIGNING
    ValuePair<ListviewGroupStates> ListviewGroupStateStrings[] = {
        VALUEPAIR(lgsCollapsed),
        VALUEPAIR(lgsCollapsible),
        VALUEPAIR(lgsFocused),
        VALUEPAIR(lgsHidden),
        VALUEPAIR(lgsNoHeader),
        //VALUEPAIR(lgsNormal),
        VALUEPAIR(lgsSelected),
        VALUEPAIR(lgsSubseted),
        VALUEPAIR(lgsSubsetLinkFocused),
    };

    Imagelist* ListviewGroup::OwnerImagelist()
    {
        return owner->GroupImages();
    }

    Object* ListviewGroup::SubOwner()
    {
        return owner;
    }

    bool ListviewGroup::SubShown()
    {
        return false;
    }

    void ListviewGroup::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->Add(L"SetId", new UnsignedIntDesignProperty<ListviewGroup>(L"Id", L"Properties", &ListviewGroup::Id, &ListviewGroup::SetId));
        serializer->Add(L"SetState", new ListviewGroupStateSetDesignProperty<ListviewGroup>(L"State", L"Properties", &ListviewGroup::State, &ListviewGroup::SetState))->SetDefault(0);
        serializer->Add(L"SetHeader", new StringDesignProperty<ListviewGroup>(L"Header", L"Appearance", &ListviewGroup::Header, &ListviewGroup::SetHeader))->SetDefault(std::wstring());
        serializer->Add(L"SetFooter", new StringDesignProperty<ListviewGroup>(L"Footer", L"Appearance", &ListviewGroup::Footer, &ListviewGroup::SetFooter))->SetDefault(std::wstring());
        serializer->Add(L"SetHeaderAlign", new TextAlignmentsDesignProperty<ListviewGroup>(L"HeaderAlign", L"Appearance", &ListviewGroup::HeaderAlign, &ListviewGroup::SetHeaderAlign))->SetDefault(taLeft);
        serializer->Add(L"SetFooterAlign", new TextAlignmentsDesignProperty<ListviewGroup>(L"FooterAlign", L"Appearance", &ListviewGroup::FooterAlign, &ListviewGroup::SetFooterAlign))->SetDefault(taLeft);
        serializer->Add(L"SetSubtitle", new StringDesignProperty<ListviewGroup>(L"Subtitle", L"Appearance", &ListviewGroup::Subtitle, &ListviewGroup::SetSubtitle))->SetDefault(std::wstring());
        serializer->Add(L"SetTask", new StringDesignProperty<ListviewGroup>(L"Task", L"Appearance", &ListviewGroup::Task, &ListviewGroup::SetTask))->SetDefault(std::wstring());
        serializer->Add(L"SetTitleImageIndex", new ImagelistIndexDesignProperty<ListviewGroup>(L"TitleImageIndex", L"Images", true, &ListviewGroup::OwnerImagelist, &ListviewGroup::TitleImageIndex, &ListviewGroup::SetTitleImageIndex))->SetDefault(-1);
        serializer->Add(L"SetExtendedImageIndex", new ImagelistIndexDesignProperty<ListviewGroup>(L"ExtendedImageIndex", L"Images", true, &ListviewGroup::OwnerImagelist, &ListviewGroup::ExtendedImageIndex, &ListviewGroup::SetExtendedImageIndex))->SetDefault(-1);
    }

    ListviewGroup::ListviewGroup(ListviewGroup *other) : base(*other), owner(NULL), id(other->id), state(other->state),
            header(other->header), footer(other->footer), headeralign(other->headeralign), footeralign(other->footeralign),
            subtitle(other->subtitle), task(other->task), titleimageindex(other->titleimageindex), extimageindex(other->extimageindex)
    {
    }
#endif

    ListviewGroup::ListviewGroup(Listview *owner) : owner(owner), id(0), state(0), headeralign(taLeft), footeralign(taLeft), titleimageindex(-1), extimageindex(-1)
    {
    }

    ListviewGroup::~ListviewGroup()
    {
    }

    Form* ListviewGroup::ParentForm() const
    {
        return owner->ParentForm();
    }

    Listview* ListviewGroup::OwnerControl()
    {
        return owner;
    }

    int ListviewGroup::Index()
    {
        return owner->GroupIndex(this);
    }

    unsigned int ListviewGroup::Id()
    {
        return id;
    }

    void ListviewGroup::SetId(unsigned int newid)
    {
        if (id == newid)
            return;
        int cnt = owner->GroupCount();
        for (int ix = 0; ix < cnt; ++ix)
            if (owner->Groups(ix)->Id() == newid)
                return;
        int oldid = id;
        id = newid;
        for (auto item : owner->items)
            if (item->group == oldid)
                item->group = newid;

        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    ListviewGroupStateSet ListviewGroup::State()
    {
        return state;
    }

    void ListviewGroup::SetState(ListviewGroupStateSet newstate)
    {
        if (state == newstate)
            return;
        state = newstate;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    const std::wstring& ListviewGroup::Header()
    {
        return header;
    }

    void ListviewGroup::SetHeader(const std::wstring& newheader)
    {
        if (header == newheader)
            return;
        header = newheader;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    const std::wstring& ListviewGroup::Footer()
    {
        return footer;
    }

    void ListviewGroup::SetFooter(const std::wstring& newfooter)
    {
        if (footer == newfooter)
            return;
        footer = newfooter;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    TextAlignments ListviewGroup::HeaderAlign()
    {
        return headeralign;
    }

    void ListviewGroup::SetHeaderAlign(TextAlignments newheaderalign)
    {
        if (headeralign == newheaderalign)
            return;
        headeralign = newheaderalign;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    TextAlignments ListviewGroup::FooterAlign()
    {
        return footeralign;
    }

    void ListviewGroup::SetFooterAlign(TextAlignments newfooteralign)
    {
        if (footeralign == newfooteralign)
            return;
        footeralign = newfooteralign;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    const std::wstring& ListviewGroup::Subtitle()
    {
        return subtitle;
    }

    void ListviewGroup::SetSubtitle(const std::wstring& newsubtitle)
    {
        if (subtitle == newsubtitle)
            return;
        subtitle = newsubtitle;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    const std::wstring& ListviewGroup::Task()
    {
        return task;
    }

    void ListviewGroup::SetTask(const std::wstring& newtask)
    {
        if (task == newtask)
            return;
        task = newtask;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    int ListviewGroup::TitleImageIndex()
    {
        return titleimageindex;
    }

    void ListviewGroup::SetTitleImageIndex(int newtitleimageindex)
    {
        if (titleimageindex == newtitleimageindex)
            return;
        titleimageindex = newtitleimageindex;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }

    int ListviewGroup::ExtendedImageIndex()
    {
        return extimageindex;
    }

    void ListviewGroup::SetExtendedImageIndex(int newextendedimageindex)
    {
        if (extimageindex == newextendedimageindex)
            return;
        extimageindex = newextendedimageindex;
        if (!owner->HandleCreated())
            return;
        owner->UpdateGroupInfo();
    }


    //---------------------------------------------


#ifdef DESIGNING
    void ListviewSubitem::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"Name");
        serializer->HideProperty(L"AccessLevel");

        serializer->Add(L"SetText", new StringDesignProperty<ListviewSubitem>(L"Text", L"Contents", &ListviewSubitem::Text, &ListviewSubitem::SetText))->SetDefault(std::wstring());
        serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<ListviewSubitem>(L"ImageIndex", L"Appearance", true, &ListviewSubitem::OwnerImagelist, &ListviewSubitem::ImageIndex, &ListviewSubitem::SetImageIndex))->SetDefault(-1);
    }

    ListviewSubitem::ListviewSubitem(ListviewItem *item) : base(), item(item)
    {
    }

    Imagelist* ListviewSubitem::OwnerImagelist()
    {
        return item->OwnerImagelist();
    }

    const ListviewItem* const ListviewSubitem::Item() const
    {
        return item;
    }

    ListviewItem* ListviewSubitem::Item()
    {
        return item;
    }

    int ListviewSubitem::Index() const
    {
        return item->SubIndex(this);
    }

    std::wstring ListviewSubitem::Text() const
    {
        return item->SubitemText(Index());
    }

    void ListviewSubitem::SetText(const std::wstring &newtext)
    {
        item->SetSubitemText(Index(), newtext);
    }

    int ListviewSubitem::ImageIndex() const
    {
        return item->SubitemImageIndex(Index());
    }

    void ListviewSubitem::SetImageIndex(int newindex)
    {
        item->SetSubitemImageIndex(Index(), newindex);
    }

#endif


    //---------------------------------------------


    ListviewItem::ListviewSubitemData::ListviewSubitemData() : imageindex(-1)
    {
    }


    //---------------------------------------------


#ifdef DESIGNING
    void ListviewItem::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"Name");
        serializer->HideProperty(L"AccessLevel");
        serializer->Add(L"SetText", new StringDesignProperty<ListviewItem>(L"Text", L"Contents", &ListviewItem::Text, &ListviewItem::SetText))->SetDefault(std::wstring());
        serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<ListviewItem>(L"ImageIndex", L"Appearance", true, &ListviewItem::OwnerImagelist, &ListviewItem::ImageIndex, &ListviewItem::SetImageIndex))->SetDefault(0);
        serializer->Add(std::wstring(), new ListviewSubitemVectorDesignProperty<ListviewItem>(L"Subitems", &ListviewItem::SubCount, &ListviewItem::Subitem, &ListviewItem::DesignAddSub));
        serializer->Add(L"SetIndentation", new UnsignedIntDesignProperty<ListviewItem>(L"Indentation", L"Appearance", &ListviewItem::Indentation, &ListviewItem::SetIndentation))->SetDefault(0);
        serializer->Add(L"SetGroupId", new ListviewGroupIdDesignProperty<ListviewItem>(L"GroupId", L"Classification", &ListviewItem::OwnerGroupCount, &ListviewItem::OwnerGroupStrings, &ListviewItem::OwnerGroupIds, &ListviewItem::GroupId, &ListviewItem::SetGroupId))->SetDefault(-1);
    }

    ListviewSubitem* ListviewItem::Subitem(int subindex)
    {
        if (subindex < 0 || subindex >= (int)subobjects.size())
            throw L"Subindex out of range.";
        return subobjects[subindex];
    }

    int ListviewItem::SubIndex(const ListviewSubitem *sub) const
    {
        if (sub->Item() != this)
            throw L"Passed subitem not owned by this ListviewItem.";
        auto it = std::find(subobjects.begin(), subobjects.end(), sub);
        if (it == subobjects.end())
            throw L"?";
        return it - subobjects.begin();
    }

    Imagelist* ListviewItem::OwnerImagelist()
    {
        if (!owner)
            return NULL;
        if (!owner->SmallImages())
            return owner->LargeImages();
        return owner->SmallImages();
    }

    int ListviewItem::OwnerGroupCount() const
    {
        if (!owner)
            return 0;
        return owner->GroupCount();
    }

    std::wstring ListviewItem::OwnerGroupStrings(int index)
    {
        if (!owner)
            return NULL;
        ListviewGroup *g = owner->Groups(index);
        return IntToStr(g->Id()) + L" [" + g->Header() + L"]";
    }

    int ListviewItem::OwnerGroupIds(int index)
    {
        if (!owner)
            return 0;
        ListviewGroup *g = owner->Groups(index);
        return g->Id();
    }

    ListviewItem::ListviewItem(ListviewItem *other) : base(*other), owner(NULL), subitems(NULL), subcount(other->subcount),
            state(other->state), overlaystateimage(other->overlaystateimage), text(other->text), imageindex(other->imageindex),
            userdata(other->userdata), indent(other->indent), group(other->group), pos(other->pos),
            tilecolcnt(other->tilecolcnt), tilecols(NULL)
    {
        if (subcount)
            subitems = new ListviewSubitemData*[subcount];
        for (int ix = 0; ix < subcount; ++ix)
        {
            subitems[ix] = new ListviewSubitemData;
            subitems[ix]->text = other->subitems[ix]->text;
            subitems[ix]->imageindex = other->subitems[ix]->imageindex;
            subobjects.push_back(new ListviewSubitem(this));
        }

        if (tilecolcnt)
        {
            tilecols = new UINT[tilecolcnt];
            memcpy(tilecols, other->tilecols, sizeof(UINT) * tilecolcnt);
        }
    }

    void ListviewItem::DesignAddSub()
    {
        AddSub();
    }
#endif

    ListviewItem::ListviewItem(Listview *owner) :
            owner(owner), subitems(NULL), subcount(0), state(0), overlaystateimage(0), imageindex(0), userdata(NULL),
            indent(0), group(-1), pos(INT_MAX, INT_MAX), tilecolcnt(0), tilecols(NULL)
    {
    }

    ListviewItem::~ListviewItem()
    {
    }

    void ListviewItem::Destroy()
    {
#ifdef DESIGNING
        for (auto sub : subobjects)
            sub->Destroy();
        subobjects.clear();
#endif

        for (int ix = 0; ix < subcount; ++ix)
            delete subitems[ix];
        delete[] subitems;

        base::Destroy();
    }

    Form* ListviewItem::ParentForm() const
    {
        return owner->ParentForm();
    }

    Listview* ListviewItem::OwnerControl()
    {
        return owner;
    }

    int ListviewItem::SubCount() const
    {
        return subcount;
    }

    int ListviewItem::Index() const
    {
        return owner->ItemIndex(this);
    }

    int ListviewItem::AddSub()
    {
        //int index = Index();
        ++subcount;

        ListviewSubitemData **tmp = subitems;
        subitems = new ListviewSubitemData*[subcount];
        if (tmp)
            memcpy(subitems, tmp, sizeof(ListviewSubitemData*) * (subcount - 1));
        subitems[subcount - 1] = new ListviewSubitemData;

        delete[] tmp;

#ifdef DESIGNING
        subobjects.push_back(new ListviewSubitem(this));
#endif

        return subcount - 1;
    }

    void ListviewItem::DeleteSubitem(int subindex)
    {
        if (subindex < 0 || subindex >= subcount)
            throw L"Subitem index out of range.";

        ListviewSubitemData **tmp = subitems;
        if (subcount > 1)
        {
            subitems = new ListviewSubitemData*[subcount - 1];
            if (subindex)
                memcpy(subitems, tmp, sizeof(ListviewSubitemData*) * subindex);
            if (subindex < subcount - 1)
                memcpy(subitems + subindex, tmp + subindex + 1, sizeof(ListviewSubitemData*) * (subcount - subindex - 1));
        }
        else
            subitems = NULL;
        delete tmp[subindex];
        delete[] tmp;
        subcount--;

#ifdef DESIGNING
        subobjects[subindex]->Destroy();
        subobjects.erase(subobjects.begin() + subindex);
#endif
    }

    const std::wstring& ListviewItem::Text() const
    {
        return text;
    }

    void ListviewItem::SetText(const std::wstring &newtext)
    {
        if (text == newtext)
            return;
        text = newtext;

        if (owner->HandleCreated())
            ListView_SetItemText(owner->Handle(), Index(), 0, LPSTR_TEXTCALLBACK);
    }

    int ListviewItem::ImageIndex() const
    {
        return imageindex;
    }

    void ListviewItem::SetImageIndex(int newindex)
    {
        if (imageindex == newindex)
            return;

        imageindex = newindex;

        owner->InvalidateItemPart(Index(), lipIcon);
    }

    const std::wstring& ListviewItem::SubitemText(int subindex) const
    {
        if (subindex < 0 || subindex >= subcount)
            throw L"Subitem index out of range.";

        return subitems[subindex]->text;
    }

    void ListviewItem::SetSubitemText(int subindex, const std::wstring &newtext)
    {
        if (subindex < 0 || subindex >= subcount)
            throw L"Subitem index out of range.";

        subitems[subindex]->text = newtext;
        if (owner->HandleCreated())
            ListView_SetItemText(owner->Handle(), Index(), subindex + 1, LPSTR_TEXTCALLBACK);
    }

    int ListviewItem::SubitemImageIndex(int subindex) const
    {
        if (subindex < 0 || subindex >= subcount)
            throw L"Subitem index out of range.";

        return subitems[subindex]->imageindex;
    }

    void ListviewItem::SetSubitemImageIndex(int subindex, int newindex)
    {
        if (subindex < 0 || subindex >= subcount)
            throw L"Subitem index out of range.";

        subitems[subindex]->imageindex = newindex;
        owner->InvalidateSubitemPart(Index(), subindex, lspBounds);
    }

    void ListviewItem::SwapSubitems(int index1, int index2)
    {
        if (index1 < 0 || index2 < 0 || index1 >= subcount || index2 >= subcount)
            throw L"Item index out of range";
        if (index1 == index2)
            return;

        ListviewSubitemData *i = subitems[index1];
        subitems[index1] = subitems[index2];
        subitems[index2] = i;

#ifdef DESIGNING
        ListviewSubitem *d = subobjects[index1];
        subobjects[index1] = subobjects[index2];
        subobjects[index2] = d;
#endif

        owner->InvalidateItemPart(Index(), lipBounds);
    }

    unsigned int ListviewItem::Indentation()
    {
        return indent;
    }

    void ListviewItem::SetIndentation(unsigned int newindent)
    {
        if (indent == newindent)
            return;
        indent = newindent;
        if (!owner->HandleCreated())
            return;
        LVITEM lvi = {0};
        lvi.mask = LVIF_INDENT;
        lvi.iItem = Index();
        lvi.iIndent = indent;
        SendMessage(owner->Handle(), LVM_SETITEM, 0, (LPARAM)&lvi);

        owner->InvalidateItemPart(lvi.iItem, lipBounds);
    }

    int ListviewItem::GroupId()
    {
        return group;
    }

    void ListviewItem::SetGroupId(int newgroupid)
    {
        if (group == newgroupid)
            return;
        group = newgroupid;
        if (!owner->HandleCreated())
            return;
        LVITEM lvi = {0};
        lvi.mask = LVIF_GROUPID;
        lvi.iItem = Index();
        lvi.iGroupId = group; //I_GROUPIDCALLBACK;
        SendMessage(owner->Handle(), LVM_SETITEM, 0, (LPARAM)&lvi);
    }

    void* ListviewItem::UserData() const
    {
        return userdata;
    }

    void ListviewItem::SetUserData(void *newuserdata)
    {
        userdata = newuserdata;
    }

    Point ListviewItem::Position()
    {
        if (!owner->HandleCreated())
            return pos;

        SendMessage(owner->Handle(), LVM_GETITEMPOSITION, Index(), (LPARAM)&pos);
        return pos;
    }

    void ListviewItem::SetPosition(const Point& newpos)
    {
        if (pos == newpos)
            return;
        pos = newpos;
        if (!owner->HandleCreated())
            return;
        SendMessage(owner->Handle(), LVM_SETITEMPOSITION32, Index(), (LPARAM)&pos);
    }


    //---------------------------------------------


#ifdef DESIGNING
    ValuePair<ListviewDisplayStyles> ListviewDisplayStyleStrings[] = {
            VALUEPAIR(ldsDetails),
            VALUEPAIR(ldsIcon),
            VALUEPAIR(ldsList),
            VALUEPAIR(ldsSmallIcon),
            VALUEPAIR(ldsTile),
    };

    ValuePair<ListviewSortDirections> ListviewSortDirectionStrings[] = {
            VALUEPAIR(lsdNone),
            VALUEPAIR(lsdAscending),
            VALUEPAIR(lsdDescending),
    };

    ValuePair<ListviewOptions> ListviewOptionStrings[] = {
            VALUEPAIR(loAlwaysShowSelect),
            VALUEPAIR(loButtonHeader),
            VALUEPAIR(loEditLabels),
            VALUEPAIR(loLabelWrap),
            VALUEPAIR(loMultiselect),
            VALUEPAIR(loShowHeader),
    };

    ValuePair<ListviewOptionsEx> ListviewOptionStringsEx[] = {
            VALUEPAIR(loxGridLines),
            VALUEPAIR(loxHeaderDragDrop),
            VALUEPAIR(loxHideIconLabels),
            VALUEPAIR(loxOneClickActivate),
            VALUEPAIR(loxRowSelect),
            VALUEPAIR(loxSimpleSelect),
            VALUEPAIR(loxSnapToGrid),
            VALUEPAIR(loxTrackSelect),
            VALUEPAIR(loxTwoClickActivate),
            VALUEPAIR(loxUnderlineCold),
            VALUEPAIR(loxUnderlineHot),
    };

    void Listview::InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems)
    {
        inserteditems.push_back(menu_item_data(L"Edit Columns...", 0, CreateEvent(this, &Listview::EditColumns)));
        inserteditems.push_back(menu_item_data(L"Edit Groups...", 0, CreateEvent(this, &Listview::EditGroups)));
    }

    void Listview::EditColumns(void *sender, EventParameters param)
    {
        designer->EditProperty(L"Columns", true);
    }

    void Listview::EditGroups(void *sender, EventParameters param)
    {
        designer->EditProperty(L"Groups", true);
    }

    void Listview::SaveItems(std::vector<ListviewItem*> &datalist)
    {
        SaveItems();
        for (auto it = items.begin(); it != items.end(); ++it)
            datalist.push_back(new ListviewItem(*it));
    }

    void Listview::RestoreItems(std::vector<ListviewItem*> &datalist)
    {
        BeginUpdate();

        SendMessage(Handle(), LVM_DELETEALLITEMS, 0, 0);
        int cnt = datalist.size();
        for (auto it = items.begin(); it != items.end(); ++it)
            (*it)->Destroy();
        items.clear();
        items.reserve(cnt);
        for (auto it = datalist.begin(); it != datalist.end(); ++it)
        {
            (*items.insert(items.end(), *it))->owner = this;
            InsertPlaceholderItem(*it);
        }
        datalist.clear();

        RestoreItemPositions();

        EndUpdate();
        Invalidate();
    }

    void Listview::SaveGroups(std::vector<ListviewGroup*> &datalist, std::vector<int> &indexlist)
    {
        for (auto group : groups)
            datalist.push_back(new ListviewGroup(group));
        for (auto it : items)
            indexlist.push_back(it->group);
    }

    void Listview::RestoreGroups(std::vector<ListviewGroup*> &datalist, std::vector<int> &indexlist)
    {
        BeginUpdate();

        int cnt = datalist.size();
        for (auto it = groups.begin(); it != groups.end(); ++it)
            (*it)->Destroy();
        groups.clear();
        groups.reserve(cnt);
        for (auto group : datalist)
        {
            (*groups.insert(groups.end(), group))->owner = this;
            DesignParent()->RegisterSubObject(this, group, group->Name());
        }
        datalist.clear();

        if (indexlist.size() == items.size())
        {
            auto iit = indexlist.begin();
            for (auto it = items.begin(); it != items.end(); ++it, ++iit)
                (*it)->group = *iit;
        }
        else
            for (auto it = items.begin(); it != items.end(); ++it)
                (*it)->group = -1;
        indexlist.clear();

        UpdateGroupInfo();

        EndUpdate();
        Invalidate();
    }

    void Listview::DesignAddColumn()
    {
        AddColumn();
    }

    void Listview::DesignAddGroup()
    {
        AddGroup();
    }

    void Listview::DesignAddItem()
    {
        AddItem();
    }

    //Object* Listview::NameOwner(const std::wstring &name)
    //{
    //    Object *obj = base::NameOwner(name);
    //    if (!obj)
    //    {
    //        for (auto it = columns.begin(); it != columns.end(); ++it)
    //            if ((*it)->Name() == name)
    //                return *it;
    //        for (auto it = groups.begin(); it != groups.end(); ++it)
    //            if ((*it)->Name() == name)
    //                return *it;
    //    }
    //    return obj;
    //}

    //void Listview::Names(std::vector<std::wstring> &namelist)
    //{
    //    for (auto it = columns.begin(); it != columns.end(); ++it)
    //        namelist.push_back((*it)->Name());
    //    for (auto it = groups.begin(); it != groups.end(); ++it)
    //        namelist.push_back((*it)->Name());
    //    base::Names(namelist);
    //}

    void Listview::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"ParentBackground");
        serializer->Find<BoolDesignProperty<Control>>(L"ParentColor")->SetDefault(false);
        serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);

        serializer->Add(L"SetStyle", new ListviewDisplayStylesDesignProperty<Listview>(L"Style", L"Behavior", &Listview::Style, &Listview::SetStyle))->SetDefault(ldsIcon);
        serializer->Add(L"SetOptions", new ListviewOptionSetDesignProperty<Listview>(L"Options", L"Behavior", &Listview::Options, &Listview::SetOptions))->SetDefault(loShowHeader | loLabelWrap | loMultiselect);
        serializer->Add(L"SetOptionsEx", new ListviewOptionSetExDesignProperty<Listview>(L"OptionsEx", L"Behavior", &Listview::OptionsEx, &Listview::SetOptionsEx))->SetDefault(0);
        serializer->Add(L"SetSortDirection", new ListviewSortDirectionsDesignProperty<Listview>(L"SortDirection", L"Behavior", &Listview::SortDirection, &Listview::SetSortDirection))->SetDefault(lsdNone);
        serializer->Add(L"AddColumn", new HeaderColumnVectorDesignProperty<Listview>(L"Columns", &Listview::ColumnCount, &Listview::Columns, &Listview::DesignAddColumn));
        serializer->Add(L"AddGroup", new ListviewGroupVectorDesignProperty<Listview>(L"Groups", &Listview::GroupCount, &Listview::Groups, &Listview::DesignAddGroup));
        serializer->Add(L"AddItem", new ListviewItemVectorDesignProperty<Listview>(L"Items", &Listview::ItemCount, &Listview::Items, &Listview::DesignAddItem));

        serializer->Add(L"SetLargeImages", new ImagelistDesignProperty<Listview>(L"LargeImages", L"Appearance", &Listview::LargeImages, &Listview::SetLargeImages));
        serializer->Add(L"SetSmallImages", new ImagelistDesignProperty<Listview>(L"SmallImages", L"Appearance", &Listview::SmallImages, &Listview::SetSmallImages));
        serializer->Add(L"SetStateImages", new ImagelistDesignProperty<Listview>(L"StateImages", L"Appearance", &Listview::StateImages, &Listview::SetStateImages));
        serializer->Add(L"SetGroupImages", new ImagelistDesignProperty<Listview>(L"GroupImages", L"Appearance", &Listview::GroupImages, &Listview::SetGroupImages));

        serializer->Add(L"SetUseGroups", new BoolDesignProperty<Listview>(L"UseGroups", L"Behavior", &Listview::UseGroups, &Listview::SetUseGroups))->SetDefault(false);
        serializer->Add(L"SetUseWorkAreas", new BoolDesignProperty<Listview>(L"UseWorkAreas", L"Behavior", &Listview::UseWorkAreas, &Listview::SetUseWorkAreas))->SetDefault(false);

        serializer->Add(L"SetGroupBorderLeft", new UnsignedIntDesignProperty<Listview>(L"GroupBorderLeft", L"Group metrics", &Listview::GroupBorderLeft, &Listview::SetGroupBorderLeft))->SetDefault(0);
        serializer->Add(L"SetGroupBorderTop", new UnsignedIntDesignProperty<Listview>(L"GroupBorderTop", L"Group metrics", &Listview::GroupBorderTop, &Listview::SetGroupBorderTop))->SetDefault(0);
        serializer->Add(L"SetGroupBorderRight", new UnsignedIntDesignProperty<Listview>(L"GroupBorderRight", L"Group metrics", &Listview::GroupBorderRight, &Listview::SetGroupBorderRight))->SetDefault(0);
        serializer->Add(L"SetGroupBorderBottom", new UnsignedIntDesignProperty<Listview>(L"GroupBorderBottom", L"Group metrics", &Listview::GroupBorderBottom, &Listview::SetGroupBorderBottom))->SetDefault(0);

        serializer->AddEvent<Listview, ListviewGroupTaskEvent>(L"OnGroupTaskClick", L"Control");
        serializer->AddEvent<Listview, BeginListviewItemEditEvent>(L"OnBeginEdit", L"Control");
        serializer->AddEvent<Listview, EndListviewItemEditEvent>(L"OnEndEdit", L"Control");
        serializer->AddEvent<Listview, CancelListviewItemEditEvent>(L"OnCancelEdit", L"Control");

    }

#endif

    Listview::Listview() :
            largeimages(NULL), smallimages(NULL), stateimages(NULL), groupimages(NULL), style(ldsIcon),
            options(loShowHeader | loLabelWrap | loMultiselect), exoptions(), sortdir(lsdNone),
            virtualcount(0), virtualitems(false), usegroups(false), useworkareas(false)
    {
        SetBorderStyle(bsNormal);
        SetParentBackground(false);
        SetParentColor(false);
        SetWantedKeyTypes(wkOthers | wkArrows);
        controlstyle << csInTabOrder << csAcceptInput;
    }

    Listview::~Listview()
    {
    }

    void Listview::Destroy()
    {
        BeginUpdate();

        int cnt = items.size();
        for (int ix = 0; ix < cnt; ++ix)
            items[ix]->Destroy();
        items.clear();

        // The listview control tries to read items when deleting columns. We have to make sure the control has no items before deleting the columns or it will try to read the now empty items list.
        if (HandleCreated())
            SendMessage(Handle(), LVM_DELETEALLITEMS, 0, 0);

        while (ColumnCount())
            DeleteColumn(0);
        while (GroupCount())
            DeleteGroup(0);

        EndUpdate();

        base::Destroy();
    }

    void Listview::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_LISTVIEW_CLASSES);
        params.classname = L"SysListView32";
    }

    void Listview::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);

        if (virtualitems)
            params.style << lvsOwnerData;

        if (style == ldsIcon)
            params.style << lvsIcon;
        if (style == ldsSmallIcon)
            params.style << lvsSmallIcon;
        if (style == ldsList)
            params.style << lvsList;
        if (style == ldsDetails)
        {
            params.style << lvsDetails;
            params.extstyle << lvsExSubitemImages;
        }
        // ldsTile doesn't have a style and can only be set with a message in InitHandle().
        // if (style == ldsTile)
        //    ;

        params.style << lvsShareImagelists;

        if (!options.contains(loShowHeader))
            params.style << lvsNoColumnHeader;
        if (!options.contains(loButtonHeader))
            params.style << lvsNoSortHeader;
        if (!options.contains(loLabelWrap))
            params.style << lvsNoLabelWrap;
        if (options.contains(loAlwaysShowSelect))
            params.style << lvsAlwaysShowSelect;
        if (!options.contains(loMultiselect))
            params.style << lvsSingleSelect;

        //if (sortdir == lsdAscending)
        //    params.style << lvsSortAscending;
        //if (sortdir == lsdDescending)
        //    params.style << lvsSortDescending;
    }

    bool Listview::HandleNotify(Control *parent, LPARAM lParam, HRESULT &result)
    {
        NMHDR &hdr = *(NMHDR*)lParam;
        if (hdr.code == LVN_BEGINLABELEDIT)
        {
            if (!OnBeginEdit)
            {
                result = TRUE;
                return true;
            }

            NMLVDISPINFO &inf = *(NMLVDISPINFO*)lParam;

            bool allowedit = true;
            OnBeginEdit(this, BeginListviewItemEditParameters(inf.item.iItem, allowedit));
            if (!allowedit)
                result = TRUE;
            else
                result = FALSE;
            return true;
        }
        else if (hdr.code == LVN_ENDLABELEDIT)
        {
            if (!OnEndEdit)
                return false;

            NMLVDISPINFO &inf = *(NMLVDISPINFO*)lParam;

            bool cancel;
            std::wstring text;
            if (inf.item.pszText == NULL)
            {
                OnCancelEdit(this, CancelListviewItemEditParameters(inf.item.iItem));
            }
            else
            {
                cancel = false;
                text = inf.item.pszText;
                OnEndEdit(this, EndListviewItemEditParameters(inf.item.iItem, text, cancel));
                if (cancel)
                    result = FALSE;
                else
                    result = TRUE;
            }
            return true;
        }
        else if (hdr.code == LVN_GETDISPINFO)
        {
            NMLVDISPINFO &inf = *(NMLVDISPINFO*)lParam;

            LVITEM &lvi = inf.item;
            ListviewItem *item = items[lvi.iItem];
            if (lvi.iSubItem == 0)
            {
                if ((lvi.mask & LVIF_TEXT) == LVIF_TEXT)
                    lvi.pszText = const_cast<wchar_t*>(item->text.c_str());
                if ((lvi.mask & LVIF_IMAGE) == LVIF_IMAGE)
                    lvi.iImage = item->imageindex;
                if ((lvi.mask & LVIF_STATE) == LVIF_STATE)
                    lvi.state = (item->state | (item->overlaystateimage << 8)) & (lvi.stateMask == (UINT)-1 ? 0xffff : lvi.stateMask);
                if ((lvi.mask & LVIF_INDENT) == LVIF_INDENT)
                    lvi.iIndent = item->indent;
                if ((lvi.mask & LVIF_GROUPID) == LVIF_GROUPID)
                    lvi.iGroupId = item->group;
                if ((lvi.mask & LVIF_COLFMT) == LVIF_COLFMT || (lvi.mask & LVIF_COLUMNS) == LVIF_COLUMNS)
                {
                    lvi.cColumns = item->tilecolcnt;
                    lvi.puColumns = item->tilecols;
                }
                if ((lvi.mask & LVIF_PARAM) == LVIF_PARAM)
                    lvi.lParam = (LPARAM)item->userdata;
            }
            else
            {
                --lvi.iSubItem;
                if (lvi.iSubItem < 0 || lvi.iSubItem >= item->subcount)
                    return false;

                ListviewItem::ListviewSubitemData *sub = item->subitems[lvi.iSubItem];

                if ((lvi.mask & LVIF_TEXT) == LVIF_TEXT)
                    lvi.pszText = const_cast<wchar_t*>(sub->text.c_str());
                if ((lvi.mask & LVIF_IMAGE) == LVIF_IMAGE)
                    lvi.iImage = sub->imageindex;
            }
        }
        else if (hdr.code == LVN_LINKCLICK)
        {
            NMLVLINK &lnk = *(NMLVLINK*)lParam;
            if (lnk.iItem < 0)
            {
                if (OnGroupTaskClick)
                {
                    for (auto it = groups.begin(); it != groups.end(); ++it)
                        if ((int)(*it)->id == lnk.iSubItem)
                        {
                            OnGroupTaskClick(this, ListviewGroupTaskParameters(*it));
                            break;
                        }
                }
            }
            else
            {
            }
        }

        return false;
    }

    void Listview::InitHandle()
    {
        base::InitHandle();
        if (style == ldsTile)
            SendMessage(Handle(), LVM_SETVIEW, LV_VIEW_TILE, 0);
        SendMessage(Handle(), LVM_SETCALLBACKMASK, LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK, 0);

        UpdateExOptions();

        BeginUpdate();

        UpdateWorkAreas();

        if (largeimages)
            SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM)largeimages->Handle());
        if (smallimages)
            SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)smallimages->Handle());
        if (stateimages)
            SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_STATE, (LPARAM)stateimages->Handle());
        if (!virtualitems && groupimages)
            SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_GROUPHEADER, (LPARAM)groupimages->Handle());

        int ccnt = columns.size();
        if (ccnt)
        {
            int *orders = new int[ccnt];
            // Add columns to the list view.
            for (int ix = 0; ix < ccnt; ++ix)
            {
                LVCOLUMN lvc = HeaderColumn::FillData(columns[ix], LVCF_FMT | LVCF_WIDTH | LVCF_IMAGE | LVCF_MINWIDTH | LVCF_DEFAULTWIDTH | LVCF_SUBITEM | LVCF_IDEALWIDTH | LVCF_TEXT);
                SendMessage(Handle(), LVM_INSERTCOLUMN, ix, (LPARAM)&lvc);
                orders[ix] = columns[ix]->pos;
            }

            UpdateColumnOrder(orders, ccnt);
            delete[] orders;
        }

        if (!virtualitems)
        {
            for (auto it = items.begin(); it != items.end(); ++it)
                InsertPlaceholderItem(*it);
        }
        else
            SendMessage(Handle(), LVM_SETITEMCOUNT, items.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

        if (usegroups)
            SendMessage(Handle(), LVM_ENABLEGROUPVIEW, (WPARAM)TRUE, 0);
        UpdateGroupMetrics();
        for (auto it = groups.begin(); it != groups.end(); ++it)
        {
            LVGROUP lvg = FillGroupInfo(*it);
            SendMessage(Handle(), LVM_INSERTGROUP, -1, (LPARAM)&lvg);
        }

        RestoreItemPositions();

        EndUpdate();

        //themes->SetWindowTheme(Handle(), L"Explorer", NULL);
    }

    void Listview::SaveWindow()
    {
        SaveColumns();
        SaveItems();
        base::SaveWindow();
    }

    void Listview::SaveColumns()
    {
        int ccnt = columns.size();
        if (!ccnt || !HandleCreated())
            return;

        int *orders = new int[ccnt];
        SendMessage(Handle(), LVM_GETCOLUMNORDERARRAY, ccnt, (LPARAM)orders);
        for (int ix = 0; ix < ccnt; ++ix)
        {
            columns[ix]->Width();
            columns[ix]->pos = orders[ix];
        }

        delete[] orders;
    }

    void Listview::SaveItems()
    {
        if (virtualitems || !HandleCreated())
            return;

        int icnt = items.size();
        for (int ix = 0; ix < icnt; ++ix)
            SendMessage(Handle(), LVM_GETITEMPOSITION, ix, (LPARAM)&items[ix]->pos);
    }

    void Listview::RestoreItemPositions()
    {
        if (virtualitems || !HandleCreated())
            return;

        int icnt = items.size();
        for (int ix = 0; ix < icnt; ++ix)
        {
            ListviewItem *it = items[ix];
            if (it->pos != Point(INT_MAX, INT_MAX))
                SendMessage(Handle(), LVM_SETITEMPOSITION32, ix, (LPARAM)&it->pos);
        }
    }

    void Listview::UpdateColumnOrder(const int *columnorder, int cnt)
    {
        SendMessage(Handle(), LVM_SETCOLUMNORDERARRAY, cnt, (LPARAM)columnorder);
    }

    void Listview::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);
        if (object == largeimages)
            SetLargeImages(NULL);
        if (object == smallimages)
            SetSmallImages(NULL);
        if (object == stateimages)
            SetStateImages(NULL);
        if (object == groupimages)
            SetGroupImages(NULL);
    }

    ListviewDisplayStyles Listview::Style()
    {
        return style;
    }

    void Listview::SetStyle(ListviewDisplayStyles newstyle)
    {
        if (newstyle == style)
            return;
        if (style == ldsDetails && HandleCreated())
            SaveColumns();
        style = newstyle;
        if (!HandleCreated())
            return;
        switch(style)
        {
        case ldsIcon:
            SendMessage(Handle(), LVM_SETVIEW, LV_VIEW_ICON, 0);
            break;
        case ldsSmallIcon:
            SendMessage(Handle(), LVM_SETVIEW, LV_VIEW_SMALLICON, 0);
            break;
        case ldsList:
            SendMessage(Handle(), LVM_SETVIEW, LV_VIEW_LIST, 0);
            break;
        case ldsDetails:
            SendMessage(Handle(), LVM_SETVIEW, LV_VIEW_DETAILS, 0);
            SendMessage(Handle(), LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_SUBITEMIMAGES, LVS_EX_SUBITEMIMAGES);
            break;
        case ldsTile:
            SendMessage(Handle(), LVM_SETVIEW, LV_VIEW_TILE, 0);
            break;
        default:
            break;
        }
    }

    bool Listview::Virtual()
    {
        return virtualitems;
    }

    void Listview::SetVirtual(bool newvirtual)
    {
        if (virtualitems == newvirtual)
            return;
        virtualitems = newvirtual;

        int cnt = items.size();
        for (int ix = 0; ix < cnt; ++ix)
            items[ix]->Destroy();
        items.clear();
        cnt = groups.size();
        for (int ix = 0; ix < cnt; ++ix)
            groups[ix]->Destroy();
        groups.clear();

        if (!HandleCreated())
            return;

        RecreateHandle();
    }


    int Listview::ColumnCount() const
    {
        return columns.size();
    }

    HeaderColumn* Listview::AddColumn()
    {
        HeaderColumn *col = new HeaderColumn(this);
#ifdef DESIGNING
        std::wstring colname;
        if (Designing())
        {
            int nxt = dynamic_cast<DesignForm*>(ParentForm())->NameNext(L"HeaderColumn");
            colname = L"HeaderColumn" + IntToStr(nxt);
        }
        col->SetName(colname);
#endif
        col->pos = columns.size();
        if (InsertColumn(col, -1) < 0)
            col->Destroy();
        else
            col->UpdateSubIndex();
        return col;
    }

    HeaderColumn* Listview::InsertColumn(int index)
    {
        if (index < 0 || index >= (int)columns.size())
            return AddColumn();

        int colpos = columns[index]->Position();

        HeaderColumn *col = new HeaderColumn(this);
#ifdef DESIGNING
        std::wstring colname;
        if (Designing())
        {
            int nxt = dynamic_cast<DesignForm*>(ParentForm())->NameNext(L"HeaderColumn");
            colname = L"HeaderColumn" + IntToStr(nxt);
        }
        col->SetName(colname);
#endif

        col->pos = colpos;
        if ((index = InsertColumn(col, index)) >= 0)
        {
            columns[index]->UpdateSubIndex();
            return columns[index];
        }
        col->Destroy();
        return NULL;
    }

    int Listview::InsertColumn(HeaderColumn *column, int index)
    {
        if (column->owner && column->owner != this)
            throw L"Trying to add column belonging to some other list view!";

        column->owner = this;

        if (index < 0 || index > (int)columns.size())
            index = columns.size();

        LVCOLUMN lvc = HeaderColumn::FillData(column, LVCF_FMT | LVCF_WIDTH | LVCF_IMAGE | LVCF_MINWIDTH | LVCF_DEFAULTWIDTH | LVCF_SUBITEM | LVCF_IDEALWIDTH | LVCF_ORDER | LVCF_TEXT);
        if (HandleCreated() && (index = SendMessage(Handle(), LVM_INSERTCOLUMN, index, (LPARAM)&lvc)) < 0)
            return -1;
        columns.insert(columns.begin() + index, column);

#ifdef DESIGNING
        if (Designing() && DesignParent())
            DesignParent()->RegisterSubObject(this, column, column->Name());
#endif

        return index;
    }

    HeaderColumn* Listview::Columns(int index)
    {
        if (index < 0 || index >= (int)columns.size())
            throw L"Column index out of range.";

        return columns[index];
    }

    void Listview::DeleteColumn(int index)
    {
        if (index < 0 || index >= (int)columns.size())
            throw L"Column index out of range.";
        if (HandleCreated() && SendMessage(Handle(), LVM_DELETECOLUMN, index, 0) == FALSE)
            return;

        columns[index]->Destroy();
        columns.erase(columns.begin() + index);
    }

    void Listview::UpdateColumnData(HeaderColumn *column, UINT mask)
    {
        if (column->OwnerControl() != this || !HandleCreated())
            return;
        LVCOLUMN lvc = HeaderColumn::FillData(column, mask);
        SendMessage(Handle(), LVM_SETCOLUMN, column->Index(), (LPARAM)&lvc);
    }

    int Listview::ColumnIndex(HeaderColumn *column)
    {
        if (column->OwnerControl() != this)
            return -1;

        auto it = std::find(columns.begin(), columns.end(), column);
        if (it == columns.end())
            return -1;
        return it - columns.begin();
    }

    Imagelist* Listview::LargeImages()
    {
        return largeimages;
    }

    void Listview::SetLargeImages(Imagelist *newlargeimages)
    {
        if (largeimages == newlargeimages)
            return;
        if (largeimages && (largeimages != smallimages && largeimages != stateimages && largeimages != groupimages))
            RemoveFromNotifyList(largeimages, nrSubControl);
        largeimages = newlargeimages;
        if (largeimages)
            AddToNotifyList(largeimages, nrSubControl);
        if (!HandleCreated())
            return;
        SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_NORMAL, largeimages ? (LPARAM)largeimages->Handle() : 0);
    }

    Imagelist* Listview::SmallImages()
    {
        return smallimages;
    }

    void Listview::SetSmallImages(Imagelist *newsmallimages)
    {
        if (smallimages == newsmallimages)
            return;
        if (smallimages && (smallimages != largeimages && smallimages != stateimages && smallimages != groupimages))
            RemoveFromNotifyList(smallimages, nrSubControl);
        smallimages = newsmallimages;
        if (smallimages)
            AddToNotifyList(smallimages, nrSubControl);
        if (!HandleCreated())
            return;
        SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_SMALL, smallimages ? (LPARAM)smallimages->Handle() : 0);

        // Fix error in Windows that prevents column text to go back to the front of the line when the image index or imagelist changes. Update the columns having an imageindex specified.
        for (unsigned int ix = 0; ix < columns.size(); ++ix)
        {
            if (columns[ix]->ImageIndex() >= 0)
            {
                if (smallimages)
                    UpdateColumnData(columns[ix], LVCF_FMT | LVCF_IMAGE);
                else
                    UpdateColumnData(columns[ix], LVCF_FMT);
            }
        }
    }

    Imagelist* Listview::StateImages()
    {
        return stateimages;
    }

    void Listview::SetStateImages(Imagelist *newstateimages)
    {
        if (stateimages == newstateimages)
            return;
        if (stateimages && (stateimages != smallimages && stateimages != largeimages && stateimages != groupimages))
            RemoveFromNotifyList(stateimages, nrSubControl);
        stateimages = newstateimages;
        if (stateimages)
            AddToNotifyList(stateimages, nrSubControl);
        if (!HandleCreated())
            return;
        SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_STATE, stateimages ? (LPARAM)stateimages->Handle() : 0);
    }

    Imagelist* Listview::GroupImages()
    {
        return groupimages;
    }

    void Listview::SetGroupImages(Imagelist *newgroupimages)
    {
        if (groupimages == newgroupimages)
            return;
        if (groupimages && (groupimages != smallimages && groupimages != stateimages && groupimages != largeimages))
            RemoveFromNotifyList(groupimages, nrSubControl);
        groupimages = newgroupimages;
        if (groupimages)
            AddToNotifyList(groupimages, nrSubControl);
        if (virtualitems || !HandleCreated())
            return;
        SendMessage(Handle(), LVM_SETIMAGELIST, LVSIL_GROUPHEADER, groupimages ? (LPARAM)groupimages->Handle() : 0);
    }

    void Listview::InvalidateHeader()
    {
        if (style != ldsDetails || columns.empty())
            return;
        HWND h = (HWND)SendMessage(Handle(), LVM_GETHEADER, 0, 0);
        if (!h)
            return;
        ::InvalidateRect(h, NULL, false);
    }

    ListviewOptionSet Listview::Options()
    {
        return options;
    }

    void Listview::SetOptions(ListviewOptionSet newoptions)
    {
        if (options == newoptions)
            return;
        options = newoptions;
        if (HandleCreated())
            RecreateHandle();
    }

    ListviewOptionSetEx Listview::OptionsEx()
    {
        return exoptions;
    }

    void Listview::SetOptionsEx(ListviewOptionSetEx newexoptions)
    {
        if (exoptions == newexoptions)
            return;
        exoptions = newexoptions;
        UpdateExOptions();
    }

    void Listview::UpdateExOptions()
    {
        if (!HandleCreated())
            return;
        int val = 0;
        if (exoptions.contains(loxRowSelect))
            val |= lvsExFullRowSelect;
        if (exoptions.contains(loxGridLines))
            val |= lvsExGridLines;
        if (exoptions.contains(loxHeaderDragDrop))
            val |= lvsExHeaderDragDrop;
        if (exoptions.contains(loxHideIconLabels))
            val |= lvsExHideLabels;
        if (exoptions.contains(loxOneClickActivate))
            val |= lvsExOneClickActivate;
        if (exoptions.contains(loxTwoClickActivate))
            val |= lvsExTwoClickActivate;
        if (exoptions.contains(loxSimpleSelect))
            val |= lvsExSimpleSelect;
        if (exoptions.contains(loxSnapToGrid))
            val |= lvsExSnapToGrid;
        if (exoptions.contains(loxTrackSelect))
            val |= lvsExTrackSelect;
        if (exoptions.contains(loxUnderlineCold))
            val |= lvsExUnderlineCold;
        if (exoptions.contains(loxUnderlineHot))
            val |= lvsExUnderlineHot;
        if (useworkareas)
            val |= lvsExMultiWorkareas;
        SendMessage(Handle(), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)val);
    }

    ListviewSortDirections Listview::SortDirection()
    {
        return sortdir;
    }

    void Listview::SetSortDirection(ListviewSortDirections newsortdir)
    {
        if (sortdir == newsortdir)
            return;
        sortdir = newsortdir;
        //if (!HandleCreated())
        //    return;
        //RecreateHandle();
    }

    int Listview::ItemCount() const
    {
        return !virtualitems ? items.size() : virtualcount;
    }

    void Listview::SetVirtualCount(int newcount)
    {
        if (!virtualitems || virtualcount == newcount)
            return;
        virtualcount = newcount;
        if (!HandleCreated())
            return;

        SendMessage(Handle(), LVM_SETITEMCOUNT, virtualcount, virtualcount ? LVSICF_NOINVALIDATEALL : 0);
    }

    ListviewItem* Listview::AddItem()
    {
        ListviewItem *item = !virtualitems ? new ListviewItem(this) : NULL;
        if (InsertItem(item, -1) < 0)
        {
            if (!virtualitems)
                item->Destroy();
            return NULL;
        }
        return item;
    }

    ListviewItem* Listview::InsertItem(int index)
    {
        if (index < 0)
            return AddItem();

        ListviewItem *item = !virtualitems ? new ListviewItem(this) : NULL;
        if (InsertItem(item, index) < 0)
        {
            if (!virtualitems)
                item->Destroy();
            return NULL;
        }
        return item;
    }

    int Listview::InsertItem(ListviewItem *item, int index)
    {
        if (!virtualitems && item == NULL)
            throw L"Trying to add NULL item.";
        if (item && item->owner && item->owner != this)
            throw L"Trying to add an item belonging to some other list view.";

        if (!virtualitems)
        {
            if (index < 0 || index > (int)items.size())
                index = items.size();
            item->owner = this;
            items.insert(items.begin() + index, item);

            if (HandleCreated() && (index = InsertPlaceholderItem(item)) < 0)
                return -1;
        }
        else
        {
            if (index < 0 || virtualcount)
                index = virtualcount;
            ++virtualcount;

            if (HandleCreated())
                SendMessage(Handle(), LVM_SETITEMCOUNT, virtualcount, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
        }

        return index;
    }

    int Listview::InsertPlaceholderItem(ListviewItem *item, int index)
    {
        if (virtualitems)
            throw L"?";
        if (!HandleCreated())
            return -1;

        LVITEM lvi = {0};
        lvi.iItem = index < 0 ? INT_MAX : index;
        lvi.mask = LVIF_COLUMNS | LVIF_IMAGE | LVIF_TEXT | LVIF_INDENT | LVIF_PARAM | LVIF_GROUPID;
        lvi.pszText = LPSTR_TEXTCALLBACK;
        lvi.iImage = I_IMAGECALLBACK;
        lvi.lParam = (LPARAM)item;
        lvi.iIndent = item->indent;
        lvi.cColumns = 0; //I_COLUMNSCALLBACK;
        lvi.puColumns = NULL;
        lvi.iGroupId = -1;
        return SendMessage(Handle(), LVM_INSERTITEM, 0, (LPARAM)&lvi);
    }

    ListviewItem* Listview::Items(int index)
    {
        if (!virtualitems)
        {
            if (index < 0 || index > (int)items.size())
                throw L"Item index out of range.";

            return items[index];
        }
        return NULL;
    }

    void Listview::DeleteItem(int index)
    {
        if (!virtualitems)
        {
            if (index < 0 || index >= (int)items.size())
                throw L"Item index out of range.";
            items[index]->Destroy();
            items.erase(items.begin() + index);
        }
        else
        {
            if (index < 0 || index >= virtualcount)
                throw L"Item index out of range.";
            --virtualcount;
        }

        if (HandleCreated())
        {
            if (SendMessage(Handle(), LVM_DELETEITEM, index, 0) == FALSE && !virtualitems)
                throw L"Couldn't delete item from the list view.";
            if (virtualitems)
                SendMessage(Handle(), LVM_SETITEMCOUNT, virtualcount, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
        }
    }

    void Listview::SwapItems(int index1, int index2)
    {
        if (virtualitems)
            return;

        int siz = items.size();
        if (index1 < 0 || index2 < 0 || index1 >= siz || index2 >= siz)
            throw L"Item index out of range";
        if (index1 == index2)
            return;

        ListviewItem *i = items[index1];
        items[index1] = items[index2];
        items[index2] = i;

        InvalidateItemPart(index1, lipBounds);
        InvalidateItemPart(index2, lipBounds);
    }

    int Listview::ItemIndex(const ListviewItem *item)
    {
        if (item->owner != this)
            throw L"Item is not added to this list view!";

        auto it = std::find(items.begin(), items.end(), item);
        if (it == items.end())
            throw L"?";
        return it - items.begin();
    }

    Rect Listview::ItemRectangle(int index, ListviewItemParts part)
    {
        if (index < 0 || index >= (int)items.size())
            throw L"Item index out of range.";

        if (!HandleCreated())
            return Rect();

        Rect r;
        r.left = part;
        SendMessage(Handle(), LVM_GETITEMRECT, index, (LPARAM)&r);
        return r;
    }

    void Listview::InvalidateItemPart(int index, ListviewItemParts part)
    {
        if (index < 0 || index >= (int)items.size())
            throw L"Item index out of range.";

        if (!HandleCreated())
            return;

        Rect r = ItemRectangle(index, part);
        InvalidateRect(r, false);
    }

    Rect Listview::SubitemRectangle(int index, int subindex, ListviewSubitemParts part)
    {
        if (index < 0 || index >= (int)items.size())
            throw L"Item index out of range.";
        if (subindex < 0 || subindex >= items[index]->subcount)
            throw L"Subitem index out of range.";

        if (!HandleCreated())
            return Rect();

        LVITEMINDEX lii;
        lii.iItem = index;
        lii.iGroup = -1;
        ListviewItem *litem = items[index];
        if (usegroups && litem->group >= 0)
        {
            int ix = 0;
            for (auto it = groups.begin(); it != groups.end(); ++it, ++ix)
            {
                if ((int)(*it)->id == litem->group)
                {
                    lii.iGroup = ix;
                    break;
                }
            }
        }

        Rect r;
        r.top = subindex + 1;
        r.left = part;
        SendMessage(Handle(), LVM_GETITEMINDEXRECT, (WPARAM)&lii, (LPARAM)&r);
        return r;
    }

    void Listview::InvalidateSubitemPart(int index, int subindex, ListviewSubitemParts part)
    {
        if (index < 0 || index >= (int)items.size())
            throw L"Item index out of range.";
        if (subindex < 0 || subindex >= items[index]->subcount)
            throw L"Subitem index out of range.";

        if (!HandleCreated())
            return;

        Rect r = SubitemRectangle(index, subindex, part);
        InvalidateRect(r, false);
    }

    bool Listview::UseGroups()
    {
        return usegroups;
    }

    void Listview::SetUseGroups(bool newusegroups)
    {
        if (usegroups == newusegroups)
            return;
        usegroups = newusegroups;
        if (!HandleCreated())
            return;
        ListView_EnableGroupView(Handle(), usegroups);
    }

    int Listview::GroupCount() const
    {
        return groups.size();
    }

    ListviewGroup* Listview::AddGroup()
    {
        if (virtualitems)
            return NULL;

        ListviewGroup *group = new ListviewGroup(this);
#ifdef DESIGNING
        std::wstring gname;
        if (Designing())
        {
            int nxt = dynamic_cast<DesignForm*>(ParentForm())->NameNext(L"ListviewGroup");
            gname = L"ListviewGroup" + IntToStr(nxt);
        }
        group->SetName(gname);
#endif

        int maxid = -1;
        for (auto it = groups.begin(); it != groups.end(); ++it)
            maxid = max(maxid, (int)(*it)->id);
        group->id = maxid + 1;

        if (InsertGroup(group, -1) < 0)
        {
            group->Destroy();
            return NULL;
        }
        return group;
    }

    ListviewGroup* Listview::InsertGroup(int index)
    {
        if (virtualitems)
            return NULL;

        if (index < 0)
            return AddGroup();

        ListviewGroup *group = new ListviewGroup(this);
#ifdef DESIGNING
        std::wstring gname;
        if (Designing())
        {
            int nxt = dynamic_cast<DesignForm*>(ParentForm())->NameNext(L"ListviewGroup");
            gname = L"ListviewGroup" + IntToStr(nxt);
        }
        group->SetName(gname);
#endif

        int maxid = -1;
        for (auto it = groups.begin(); it != groups.end(); ++it)
            maxid = max(maxid, (int)(*it)->id);
        group->id = maxid + 1;

        if (InsertGroup(group, index) < 0)
        {
            group->Destroy();
            return NULL;
        }
        return group;
    }

    int Listview::InsertGroup(ListviewGroup *group, int index)
    {
        if (virtualitems)
            throw L"Can't add group to virtual listview.";
        if (group == NULL)
            throw L"Group to insert not specified.";
        if (group != NULL && group->owner && group->owner != this)
            throw L"Trying to add an item belonging to some other list view.";

        if (index < 0 || index > (int)groups.size())
            index = groups.size();
        group->owner = this;
        groups.insert(groups.begin() + index, group);

#ifdef DESIGNING
        if (Designing() && DesignParent())
            DesignParent()->RegisterSubObject(this, group, group->Name());
#endif

        if (!HandleCreated())
            return index;

        LVGROUP lvg = {0};
        lvg.cbSize = sizeof(LVGROUP);
        lvg.mask = LVGF_HEADER | LVGF_STATE | LVGF_ALIGN | LVGF_GROUPID;
        lvg.stateMask = group->state;
        lvg.state = group->state;
        lvg.uAlign = LVGA_HEADER_LEFT | (Win32MajorVersion < 6 ? 0 : LVGA_FOOTER_LEFT);
        lvg.iGroupId = group->id;
#ifdef DESIGNING
        group->header = group->Name();
#endif
        lvg.pszHeader = const_cast<wchar_t*>(group->header.c_str());

        if (SendMessage(Handle(), LVM_INSERTGROUP, index, (LPARAM)&lvg) < 0)
            return -1;
        memset(&lvg, 0, sizeof(LVGROUP));
        lvg.cbSize = sizeof(LVGROUP);
        lvg.mask = LVGF_GROUPID;
        if (SendMessage(Handle(), LVM_GETGROUPINFOBYINDEX, groups.size() - 1, (LPARAM)&lvg) == TRUE)
            group->id = lvg.iGroupId;
        return index;
    }

    ListviewGroup* Listview::Groups(int index)
    {
        if (!virtualitems)
        {
            if (index < 0 || index > (int)groups.size())
                throw L"Item index out of range.";

            return groups[index];
        }
        return NULL;
    }

    int Listview::GroupIndex(ListviewGroup *group)
    {
        if (group->owner != this)
            throw L"Group is not added to this list view!";

        auto it = std::find(groups.begin(), groups.end(), group);
        if (it == groups.end())
            throw L"?";
        return it - groups.begin();
    }

    void Listview::DeleteGroup(int index)
    {
        if (index < 0 || index >= (int)groups.size())
            throw L"Group index out of range.";

        int id = groups[index]->id;
        groups[index]->Destroy();
        groups.erase(groups.begin() + index);
        for (auto item : items)
            if (item->group == id)
                item->group = -1;
        if (HandleCreated())
            SendMessage(Handle(), LVM_REMOVEGROUP, id, 0);

    }

    void Listview::SwapGroups(int index1, int index2)
    {
        if (index1 < 0 || index2 < 0 || index1 >= (int)groups.size() || index2 >= (int)groups.size())
            throw L"Group index out of range.";
        if (index1 == index2)
            return;

        std::swap(groups[index1], groups[index2]);
        if (!HandleCreated())
            return;
        if (index1 > index2)
            std::swap(index1, index2);

        SendMessage(Handle(), LVM_REMOVEGROUP, groups[index2]->id, 0);
        SendMessage(Handle(), LVM_REMOVEGROUP, groups[index1]->id, 0);

        LVGROUP lvg = FillGroupInfo(groups[index2]);
        SendMessage(Handle(), LVM_INSERTGROUP, index1, (LPARAM)&lvg);
        lvg = FillGroupInfo(groups[index1]);
        SendMessage(Handle(), LVM_INSERTGROUP, index2, (LPARAM)&lvg);
    }

    LVGROUP Listview::FillGroupInfo(ListviewGroup *group)
    {
        LVGROUP lvg = {0};
        lvg.cbSize = sizeof(LVGROUP);

        lvg.mask = LVGF_HEADER | LVGF_ALIGN | LVGF_STATE | LVGF_GROUPID;
        lvg.iGroupId = group->id;

        if (group->headeralign != taCenter || group->titleimageindex < 0)
        {
            lvg.pszHeader = const_cast<wchar_t*>(group->header.c_str());
            if (!group->subtitle.empty())
            {
                lvg.pszSubtitle = const_cast<wchar_t*>(group->subtitle.c_str());
                lvg.mask |= LVGF_SUBTITLE;
            }
        }
        else
        {
            lvg.pszDescriptionTop = const_cast<wchar_t*>(group->header.c_str());
            if (Win32MajorVersion >= 6)
                lvg.mask |= LVGF_DESCRIPTIONTOP;
            if (!group->subtitle.empty())
            {
                lvg.mask |= LVGF_SUBTITLE;
                if (Win32MajorVersion >= 6)
                    lvg.mask |= LVGF_DESCRIPTIONBOTTOM;
                lvg.pszDescriptionBottom = const_cast<wchar_t*>(group->subtitle.c_str());
            }
        }

        if (!group->footer.empty())
        {
            lvg.mask |= LVGF_FOOTER;
            lvg.pszFooter = const_cast<wchar_t*>(group->footer.c_str());
        }

        lvg.stateMask = 0;
        lvg.state = group->state;
        if (Win32MajorVersion < 6)
            lvg.state &= LVGS_COLLAPSED | LVGS_HIDDEN;

        lvg.uAlign = (group->headeralign == taLeft ? LVGA_HEADER_LEFT : group->headeralign == taRight ? LVGA_HEADER_RIGHT : LVGA_HEADER_CENTER) | (Win32MajorVersion < 6 ? 0 : group->footeralign == taLeft ? LVGA_FOOTER_LEFT : group->footeralign == taRight ? LVGA_FOOTER_RIGHT : LVGA_FOOTER_CENTER);

        if (!group->task.empty())
        {
            lvg.mask |= LVGF_TASK;
            lvg.pszTask = const_cast<wchar_t*>(group->task.c_str());
        }

        if (Win32MajorVersion >= 6 && group->titleimageindex >= 0)
        {
            lvg.mask |= LVGF_TITLEIMAGE;
            lvg.iTitleImage = group->titleimageindex;
        }
        if (Win32MajorVersion >= 6 && group->extimageindex >= 0)
        {
            lvg.mask |= LVGF_EXTENDEDIMAGE;
            lvg.iExtendedImage = group->extimageindex;
        }
        return lvg;
    }

    void Listview::UpdateGroupInfo()
    {
        if (!HandleCreated())
            return;

        BeginUpdate();

        int ix = 0;
        for (auto it = items.begin(); it != items.end(); ++it, ++ix)
        {
            LVITEM lvi = {0};
            lvi.mask = LVIF_GROUPID;
            lvi.iItem = ix;
            lvi.iGroupId = -1;
            SendMessage(Handle(), LVM_SETITEM, 0, (LPARAM)&lvi);
        }

        SendMessage(Handle(), LVM_REMOVEALLGROUPS, 0, 0);
        ix = 0;
        for (auto it = groups.begin(); it != groups.end(); ++it, ++ix)
        {
            LVGROUP lvg = FillGroupInfo(*it);
            SendMessage(Handle(), LVM_INSERTGROUP, ix, (LPARAM)&lvg);
        }


        ix = 0;
        for (auto it = items.begin(); it != items.end(); ++it, ++ix)
        {
            LVITEM lvi = {0};
            lvi.mask = LVIF_GROUPID;
            lvi.iItem = ix;
            lvi.iGroupId = (*it)->group;
            SendMessage(Handle(), LVM_SETITEM, 0, (LPARAM)&lvi);
        }

        EndUpdate();
        Invalidate();
    }

    void Listview::UpdateGroupMetrics()
    {
        if (!HandleCreated())
            return;

        LVGROUPMETRICS lvgm = {0};
        lvgm.cbSize = sizeof(LVGROUPMETRICS);
        lvgm.mask = LVGMF_BORDERSIZE;
        lvgm.Left = borders.left;
        lvgm.Top = borders.top;
        lvgm.Right = borders.right;
        lvgm.Bottom = borders.bottom;
        SendMessage(Handle(), LVM_SETGROUPMETRICS, 0, (LPARAM)&lvgm);
    }

    void Listview::SetGroupBorders(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom)
    {
        Rect newborders = Rect(max(0, left), max(0, top), max(0, right), max(0, bottom));
        if (borders == newborders)
            return;
        borders = newborders;
        if(!HandleCreated())
            return;
        UpdateGroupMetrics();
    }

    unsigned int Listview::GroupBorderLeft()
    {
        return borders.left;
    }

    void Listview::SetGroupBorderLeft(unsigned int newborder)
    {
        newborder = max((int)newborder, 0);
        if ((int)newborder == borders.left)
            return;
        SetGroupBorders(newborder, borders.top, borders.right, borders.bottom);
    }

    unsigned int Listview::GroupBorderTop()
    {
        return borders.top;
    }

    void Listview::SetGroupBorderTop(unsigned int newborder)
    {
        newborder = max((int)newborder, 0);
        if ((int)newborder == borders.top)
            return;
        SetGroupBorders(borders.left, newborder, borders.right, borders.bottom);
    }

    unsigned int Listview::GroupBorderRight()
    {
        return borders.right;
    }

    void Listview::SetGroupBorderRight(unsigned int newborder)
    {
        newborder = max((int)newborder, 0);
        if ((int)newborder == borders.right)
            return;
        SetGroupBorders(borders.left, borders.top, newborder, borders.bottom);
    }

    unsigned int Listview::GroupBorderBottom()
    {
        return borders.bottom;
    }

    void Listview::SetGroupBorderBottom(unsigned int newborder)
    {
        newborder = max((int)newborder, 0);
        if ((int)newborder == borders.bottom)
            return;
        SetGroupBorders(borders.left, borders.top, borders.right, newborder);
    }

    bool Listview::UseWorkAreas()
    {
        return useworkareas;
    }

    void Listview::SetUseWorkAreas(bool newuseworkareas)
    {
        if (useworkareas == newuseworkareas)
            return;
        useworkareas = newuseworkareas;
        if (!HandleCreated())
            return;
        RecreateHandle();
    }

    void Listview::UpdateWorkAreas()
    {
        if (!HandleCreated())
            return;

        if (!workareas.empty())
            SendMessage(Handle(), LVM_SETWORKAREAS, workareas.size(), (LPARAM)&workareas.front());
        else
            SendMessage(Handle(), LVM_SETWORKAREAS, 0, 0);
    }

    int Listview::WorkAreaCount()
    {
        return workareas.size();
    }

    Rect Listview::WorkArea(int index)
    {
        if (index < 0 || index >= (int)workareas.size())
            throw L"Work area index out of range.";

        return workareas[index];
    }

    void Listview::AddWorkArea(const Rect &newarea)
    {
        if (workareas.size() >= LV_MAX_WORKAREAS)
            return;

        workareas.push_back(newarea);
        if (!HandleCreated())
            return;
        UpdateWorkAreas();
    }

    void Listview::InsertWorkArea(const Rect &newarea, int index)
    {
        if (index < 0 || index >= (int)workareas.size())
        {
            AddWorkArea(newarea);
            return;
        }

        if (workareas.size() >= LV_MAX_WORKAREAS)
            return;

        workareas.insert(workareas.begin() + index, newarea);
        if (!HandleCreated())
            return;
        UpdateWorkAreas();
    }

    void Listview::SetWorkArea(const Rect &newarea, int index)
    {
        if (index < 0 || index >= (int)workareas.size())
            throw L"Work area index out of range.";

        Rect &r = workareas[index];
        if (r == newarea)
            return;
        r = newarea;
        if (!HandleCreated())
            return;
        UpdateWorkAreas();
    }

    void Listview::DeleteWorkArea(int index)
    {
        if (index < 0 || index >= (int)workareas.size())
            throw L"Work area index out of range.";

        workareas.erase(workareas.begin() + index);
        if (!HandleCreated())
            return;
        UpdateWorkAreas();
    }

    void Listview::ClearWorkAreas()
    {
        if (workareas.empty())
            return;

        workareas.clear();
        if (!HandleCreated())
            return;
        UpdateWorkAreas();
    }


    //---------------------------------------------

#ifdef DESIGNING
    ValuePair<StatusBarPartBevels> StatusBarPartBevelStrings[] = {
            VALUEPAIR(sbpbLowered),
            VALUEPAIR(sbpbRaised),
            VALUEPAIR(sbpbNone),
    };

    std::wstring StatusBarPartArgs(Object *obj)
    {
        StatusBarPart *p = ((StatusBarPart*)obj);

        return L"L\"" + EscapeCString(p->Text()) + L"\", " NLIBNS_STRING + TextAlignmentStrings[(int)p->TextAlignment()].second + L", " + IntToStr(p->Width()) + L", " NLIBNS_STRING + StatusBarPartBevelStrings[(int)p->Bevel()].second + L", " + (p->DesignOwnerDrawn() ? L"true" : L"false");
    }

    void StatusBarPart::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"Name");
        serializer->HideProperty(L"Tag");
        serializer->HideProperty(L"AccessLevel");

        serializer->Add(std::wstring(), new StatusBarPartBevelsDesignProperty<StatusBarPart>(L"Bevel", L"Appearance", &StatusBarPart::Bevel, &StatusBarPart::SetBevel))->SetDefault(sbpbLowered);
        serializer->Add(std::wstring(), new UnsignedIntDesignProperty<StatusBarPart>(L"Width", L"Appearance", &StatusBarPart::Width, &StatusBarPart::SetWidth))->SetDefault(60);
        serializer->Add(std::wstring(), new StringDesignProperty<StatusBarPart>(L"Text", L"Appearance", &StatusBarPart::Text, &StatusBarPart::SetText))->SetDefault(std::wstring())->SetImmediateUpdate(true)->MakeDefault();
        serializer->Add(std::wstring(), new TextAlignmentsDesignProperty<StatusBarPart>(L"TextAlignment", L"Appearance", &StatusBarPart::TextAlignment, &StatusBarPart::SetTextAlignment))->SetDefault(taLeft);
        serializer->Add(std::wstring(), new BoolDesignProperty<StatusBarPart>(L"OwnerDrawn", L"Appearance", &StatusBarPart::DesignOwnerDrawn, &StatusBarPart::DesignSetOwnerDrawn))->SetDefault(false);
    }

    StatusBarPart::StatusBarPart(StatusBar *owner) : owner(owner), ownerdrawn(false)
    {
    }

    StatusBarPart::~StatusBarPart()
    {
    }

    StatusBar* StatusBarPart::Owner() const
    {
        return owner;
    }

    unsigned int StatusBarPart::Width() const
    {
        return owner->PartWidth(owner->PartIndex(this));
    }

    void StatusBarPart::SetWidth(unsigned int newwidth)
    {
        owner->SetPartWidth(owner->PartIndex(this), newwidth);
    }

    std::wstring StatusBarPart::Text() const
    {
        return owner->PartText(owner->PartIndex(this));
    }

    void StatusBarPart::SetText(const std::wstring &newtext)
    {
        owner->SetPartText(owner->PartIndex(this), newtext);
    }

    StatusBarPartBevels StatusBarPart::Bevel() const
    {
        return owner->PartBevel(owner->PartIndex(this));
    }

    void StatusBarPart::SetBevel(StatusBarPartBevels newbevel)
    {
        owner->SetPartBevel(owner->PartIndex(this), newbevel);
    }

    TextAlignments StatusBarPart::TextAlignment() const
    {
        return owner->PartTextAlignment(owner->PartIndex(this));
    }

    void StatusBarPart::SetTextAlignment(TextAlignments newalign)
    {
        owner->SetPartTextAlignment(owner->PartIndex(this), newalign);
    }

    bool StatusBarPart::DesignOwnerDrawn() const
    {
        return ownerdrawn;
    }

    void StatusBarPart::DesignSetOwnerDrawn(bool newownerdrawn)
    {
        ownerdrawn = newownerdrawn;
    }

    ValuePair<StatusBarSizeGrips> StatusBarSizeGripStrings[] = {
            VALUEPAIR(sbsgNone),
            VALUEPAIR(sbsgSizeGrip),
            VALUEPAIR(sbsgAuto),
    };

    Size StatusBar::DesignSize()
    {
        return Size(240, 23);
    }

    int StatusBar::PartIndex(const StatusBarPart *part) const
    {
        if (!part || part->Owner() != this)
            return -1;

        auto it = std::find(parts.begin(), parts.end(), part);
        if (it == parts.end())
            return -1;

        return it - parts.begin();
    }

    StatusBarPart* StatusBar::Parts(int ix)
    {
        if (ix < 0 || ix >= (int)parts.size())
            throw L"Part index out of range.";

        return parts[ix];
    }

    void StatusBar::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"ParentBackground");
        serializer->HideProperty(L"ParentColor");
        serializer->HideProperty(L"Color");
        serializer->HideProperty(L"BorderStyle");
        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"TabOrder");
        serializer->HideProperty(L"WantedKeyTypes");
        serializer->Find<ControlAlignmentsDesignProperty<Control>>(L"Alignment")->SetDefault(alBottom);
        serializer->Find<ColorDesignProperty<Control>>(L"Color", dpuHidden)->SetDefault(CLR_DEFAULT);

        serializer->Add(L"SetSizeGrip", new StatusBarSizeGripsDesignProperty<StatusBar>(L"SizeGrip", L"Behavior", &StatusBar::SizeGrip, &StatusBar::SetSizeGrip))->SetDefault(sbsgAuto);
        serializer->Add(L"SetSimpleArea", new BoolDesignProperty<StatusBar>(L"SimpleArea", L"Appearance", &StatusBar::DesignSimple, &StatusBar::DesignSetSimple))->SetDefault(false);
        serializer->Add(L"AddPart", new StatusBarPartDesignProperty<StatusBar>(L"Parts", L"Appearance", &StatusBar::PartCount, &StatusBar::DesignParts, &StatusBar::DesignAddPart))->MakeDefault();
        serializer->Add(L"SetText", new StringDesignProperty<StatusBar>(L"Text", L"Appearance", &StatusBar::Text, &StatusBar::SetText))->SetDefault(std::wstring())->SetImmediateUpdate(true);

        serializer->AddEvent<StatusBar, DrawItemEvent>(L"OnDrawPart", L"Drawing");
    }

    void StatusBar::SaveParts(std::vector<PartData> &datalist, bool &simpledata)
    {
        simpledata = simple;
        datalist = datavec;
        if (simple)
            SetSimpleArea(false);
    }

    void StatusBar::RestoreParts(std::vector<PartData> &datalist, bool simpledata)
    {
        datavec.clear();
        std::swap(datavec, datalist);
        SetParts();
    }

    void StatusBar::RestoreSimple(bool simpledata)
    {
        if (!simple && simpledata)
            SetSimpleArea(true);
    }

    StatusBarPart* StatusBar::DesignParts(int ix)
    {
        return parts[ix];
    }

    void StatusBar::DesignAddPart()
    {
        parts.push_back(new StatusBarPart(this));
        datavec.push_back(PartData());
        datavec.back().ownerdrawn = false;
        datavec.back().width = 60;
        datavec.back().align = taLeft;
        UpdateParts();
    }

    bool StatusBar::DesignSimple()
    {
        return designsimple;
    }

    void StatusBar::DesignSetSimple(bool newsimple)
    {
        designsimple = newsimple;
        SetSimpleArea(newsimple);
    }
#else
    StatusBarPart::StatusBarPart() : owner(nullptr), index(-1) // Constructs an invalid StatusBarPart which can't be used.
    {
    }

    StatusBarPart::StatusBarPart(StatusBar *owner, int index) : owner(owner), index(index)
    {
        if (owner)
            owner->PartAdded(this);
    }

    StatusBarPart::~StatusBarPart()
    {
        if (owner)
            owner->RemovePart(this);
    }

    void StatusBarPart::MakeInvalid()
    {
        index = -1;
    }

    void StatusBarPart::OwnerMakeInvalid()
    {
        index = -1;
        owner = nullptr;
    }

    StatusBar* StatusBarPart::Owner() const
    {
        CheckValidity();
        return owner;
    }

    void StatusBarPart::CheckValidity() const
    {
        if (index == -1 || owner == nullptr)
            throw L"This status bar part is invalid.";
    }

    StatusBarPart::StatusBarPart(const StatusBarPart &other)
    {
        *this = other;
    }

    StatusBarPart::StatusBarPart(StatusBarPart &&other)
    {
        *this = std::move(other);
    }

    StatusBarPart& StatusBarPart::operator=(const StatusBarPart &other)
    {
        if (owner != other.owner)
        {
            if (owner)
                owner->RemovePart(this);
            if (other.owner)
                other.owner->PartAdded(this);
        }

        owner = other.owner;
        index = other.index;
        return *this;
    }

    StatusBarPart& StatusBarPart::operator=(StatusBarPart &&other)
    {
        if (owner != other.owner)
        {
            if (owner)
                owner->RemovePart(this);
            if (other.owner)
                other.owner->RemovePart(&other);
        }

        std::swap(owner, other.owner);
        std::swap(index, other.index);

        if (owner != other.owner)
        {
            if (owner)
                owner->PartAdded(this);
            if (other.owner)
                other.owner->PartAdded(&other);
        }

        return *this;
    }

    bool StatusBarPart::Valid()
    {
        return index != -1 && owner != nullptr;
    }

    int StatusBarPart::Index() const
    {
        CheckValidity();
        return index;
    }

    std::wstring StatusBarPart::Text()
    {
        CheckValidity();
        return owner->PartText(index);
    }

    void StatusBarPart::SetText(const std::wstring &newtext)
    {
        CheckValidity();
        owner->SetPartText(index, newtext);
    }

    TextAlignments StatusBarPart::TextAlignment()
    {
        CheckValidity();
        return owner->PartTextAlignment(index);
    }

    void StatusBarPart::SetTextAlignment(TextAlignments newalign)
    {
        CheckValidity();
        owner->SetPartTextAlignment(index, newalign);
    }

    StatusBarPartBevels StatusBarPart::Bevel()
    {
        CheckValidity();
        return owner->PartBevel(index);
    }

    void StatusBarPart::SetBevel(StatusBarPartBevels newbevel)
    {
        CheckValidity();
        owner->SetPartBevel(index, newbevel);
    }

    unsigned int StatusBarPart::Width()
    {
        CheckValidity();
        return owner->PartWidth(index);
    }

    void StatusBarPart::SetWidth(int unsigned newwidth)
    {
        CheckValidity();
        owner->SetPartWidth(index, newwidth);
    }

    Rect StatusBarPart::Area()
    {
        CheckValidity();
        return owner->PartArea(index);
    }

#endif

    StatusBar::StatusBar() : base(), grip(sbsgAuto), simple(false)
#ifdef DESIGNING
        , designsimple(false)
#endif
    {
        //controlstyle << csChild << csParentColor << csParentBackground << csParentFont << csUpdateOnTextChange << csParentTooltip << csShowTooltip << csEraseToColor;
        controlstyle << csSystemErased << csEraseOnTextChange;

        SetBorderStyle(bsNone);
        SetParentBackground(false);
        SetParentColor(false);
        SetAlignment(alBottom);
    }

    StatusBar::~StatusBar()
    {
    }

    void StatusBar::Destroy()
    {
#ifdef DESIGNING
        if (Designing())
            for(auto *part : parts)
                part->Destroy();
#else
        for (auto *part : parts)
            part->OwnerMakeInvalid();
#endif

        base::Destroy();
    }

    void StatusBar::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_BAR_CLASSES);
        params.classname = STATUSCLASSNAME;
    }

    void StatusBar::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);

        if (grip == sbsgSizeGrip || (grip == sbsgAuto && (ParentForm()->BorderStyle() == fbsNormal || ParentForm()->BorderStyle() == fbsSizeableToolWindow)))
        {
            params.style -= ccsTop;
            params.style << stbsSizeGrip;
        }
        else
        {
            params.style -= stbsSizeGrip;
            params.style << ccsTop;
        }
    }

    void StatusBar::SaveWindow()
    {
        size_t partcnt = PassMessage(SB_GETPARTS, 256, 0);
        if (!partcnt)
            return;

        partcnt = min(partcnt, datavec.size());
        std::vector<int> w(partcnt);
        PassMessage(SB_GETPARTS, partcnt, (LPARAM)&w[0]);

        INT sizes[3];
        PassMessage(SB_GETBORDERS, 0, (LPARAM)sizes);

        for (size_t ix = 0; ix < partcnt; ++ix)
        {
            int p = PassMessage(SB_GETTEXTLENGTH, ix, 0);
            WORD len = LOWORD(p);
            WORD data = HIWORD(p);
            WORD data2 = data & ~(WORD)(SBT_OWNERDRAW | SBT_RTLREADING);

            auto &part = datavec[ix];

            part.text.resize(len + 1, 0);
            PassMessage(SB_GETTEXT, ix, (LPARAM)&part.text[0]);
            part.text.resize(len);
            part.bevel = (data & SBT_POPOUT) != 0 ? sbpbRaised : (data & SBT_NOBORDERS) != 0 ? sbpbNone : sbpbLowered;
            part.ownerdrawn = (data & SBT_OWNERDRAW) != 0;
            part.width = w[ix] - (ix == 0 ? sizes[1] : w[ix - 1] + sizes[2]);
            part.align = (len == 0 || part.text[0] != L'\t' ? taLeft : len == 1 || part.text[1] != L'\t' ? taCenter : taRight);
            if (part.align != taLeft)
                part.text.erase(0, part.align == taCenter ? 1 : 2);
        }

        base::SaveWindow();
    }

    void StatusBar::InitHandle()
    {
        base::InitHandle();
        PassMessage(SB_SIMPLE, (WPARAM)(simple ? TRUE : FALSE), 0);
        //PassMessage(SB_SETBKCOLOR, 0, CLR_DEFAULT);
        SetParts();
    }

    LRESULT StatusBar::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_SIZE:
            Invalidate(true);
            return 0;
        case wmColorChanged:
            return 0;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool StatusBar::HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures)
    {
        if (!OnDrawPart)
            return false;

        Canvas *c = GetCanvas();
        if ((int)measures->itemID >= 0)
        {
            if ((measures->itemState & ODS_SELECTED) == ODS_SELECTED)
            {
                c->SelectStockBrush(sbHighlight);
                c->GetFont().SetColor(clHighlightText);
            }
            OnDrawPart(this, DrawItemParameters(c, measures->itemID, measures->rcItem, measures->itemState, measures->itemData));
        }
        else
        {
            c->FillRect(measures->rcItem);
            if ((measures->itemState & ODS_FOCUS) == ODS_FOCUS)
            {
                HDC dc = c->GetDC();
                DrawFocusRect(dc, &measures->rcItem);
                c->ReturnDC();
            }
        }
        return true;
    }

    std::wstring StatusBar::Text() const
    {
        return simpletext;
    }

    void StatusBar::SetText(const std::wstring &newtext)
    {
        if (newtext == simpletext)
            return;
        simpletext = newtext;
        if (HandleCreated() && simple)
            PassMessage(SB_SETTEXT, SB_SIMPLEID | SBT_NOBORDERS, (LPARAM)simpletext.c_str());
    }

    void StatusBar::UpdatePart(int ix)
    {
        if (!HandleCreated() || ix < 0 || ix >= (int)datavec.size())
            return;
        UpdateParts();
        UpdatePartText(ix);
    }

    void StatusBar::UpdatePartText(int ix)
    {
        if (!HandleCreated() || ix < 0 || ix >= (int)datavec.size())
            return;

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        int dcnt = datavec.size();
        if (cnt < dcnt)
        {
            UpdateParts();
            for (int iy = cnt; iy < dcnt; ++iy)
                if (iy != ix)
                    UpdatePartText(iy);
        }

        std::wstring settext;
        if (datavec[ix].align != taLeft)
            settext = (datavec[ix].align == taCenter ? L"\t" : L"\t\t") + datavec[ix].text;
        else
            settext = datavec[ix].text;

        PassMessage(SB_SETTEXT, MAKEWPARAM(ix | ((datavec[ix].ownerdrawn ? SBT_OWNERDRAW : 0) | (datavec[ix].bevel == sbpbNone ? SBT_NOBORDERS : datavec[ix].bevel == sbpbRaised ? SBT_POPOUT : 0)), 0), (LPARAM)settext.c_str());
    }

    void StatusBar::UpdateParts()
    {
        if (!HandleCreated())
            return;

        if (datavec.empty() || simple)
        {
            PassMessage(SB_SIMPLE, (WPARAM)TRUE, 0);
            wchar_t ch[] = L"";
            PassMessage(SB_SETTEXT, SB_SIMPLEID | SBT_NOBORDERS, (LPARAM)(!simple ? ch : &simpletext[0]));
            return;
        }

        PassMessage(SB_SIMPLE, (WPARAM)FALSE, 0);

        INT sizes[3];
        PassMessage(SB_GETBORDERS, 0, (LPARAM)sizes);
        if (datavec.empty())
        {
            PassMessage(SB_SETPARTS, 0, 0);
            return;
        }

        size_t partcnt = datavec.size();

        std::vector<int> partx(partcnt);
        for (size_t ix = 0; ix < partcnt; ++ix)
            partx[ix] = ix == 0 ? sizes[1] + datavec[ix].width : (ix == partcnt - 1) ? -1 : sizes[2] + partx[ix - 1] + datavec[ix].width;
        PassMessage(SB_SETPARTS, partcnt, (LPARAM)&partx[0]);
    }

    void StatusBar::SetParts()
    {
        if (!HandleCreated())
            return;

        UpdateParts();

        if (simple)
            return;

        std::wstring settext;

        int cnt = datavec.size();
        for (int ix = 0; ix < cnt; ++ix)
        {
            if (datavec[ix].align != taLeft)
                settext = (datavec[ix].align == taCenter ? L"\t" : L"\t\t") + datavec[ix].text;
            else
                settext = datavec[ix].text;

            PassMessage(SB_SETTEXT, MAKEWPARAM(ix | ((datavec[ix].ownerdrawn ? SBT_OWNERDRAW : 0) | (datavec[ix].bevel == sbpbNone ? SBT_NOBORDERS : datavec[ix].bevel == sbpbRaised ? SBT_POPOUT : 0)), 0), (LPARAM)settext.c_str());
        }
    }

#ifndef DESIGNING
    void StatusBar::PartAdded(StatusBarPart *part)
    {
        auto it = std::find(parts.begin(), parts.end(), part);
        if (it != parts.end())
            return;
        parts.push_back(part);
    }

    void StatusBar::RemovePart(StatusBarPart *part)
    {
        auto it = std::find(parts.begin(), parts.end(), part);
        if (it == parts.end())
            return;
        parts.erase(it);
    }
#endif

    StatusBarSizeGrips StatusBar::SizeGrip() const
    {
        return grip;
    }

    void StatusBar::SetSizeGrip(StatusBarSizeGrips newgrip)
    {
        if (grip == newgrip)
            return;
        grip = newgrip;

        if (HandleCreated())
            RecreateHandle();
    }

    bool StatusBar::SimpleArea() const
    {
        return simple;
    }

    void StatusBar::SetSimpleArea(bool newsimple)
    {
        if (simple == newsimple)
            return;
        simple = newsimple;

        if (HandleCreated())
        {
            bool smp = simple || datavec.empty();
            PassMessage(SB_SIMPLE, (WPARAM)(smp ? TRUE : FALSE), 0);
            if (smp)
            {
                wchar_t ch[] = L"";
                PassMessage(SB_SETTEXT, SB_SIMPLEID | SBT_NOBORDERS, (LPARAM)(!simple ? ch : &simpletext[0]));
            }
        }
    }

    int StatusBar::AddPart(const std::wstring &text, TextAlignments align, unsigned int width, StatusBarPartBevels bevel, bool ownerdrawn)
    {
        if (datavec.size() == 256)
            return -1; // Cannot add more than 256 parts.

        datavec.push_back(PartData());
        datavec.back().text = text;
        datavec.back().align = align;
        datavec.back().width = width;
        datavec.back().bevel = bevel;
        datavec.back().ownerdrawn = ownerdrawn;
#ifdef DESIGNING
        if (Designing())
        {
            parts.push_back(new StatusBarPart(this));
            parts.back()->ownerdrawn = ownerdrawn;
            datavec.back().ownerdrawn = false;
        }
#endif
        if (!simple)
            SetParts();
        return datavec.size() - 1;
    }

    void StatusBar::InsertPart(int index, const std::wstring &text, TextAlignments align, unsigned int width, StatusBarPartBevels bevel, bool ownerdrawn)
    {
        if (datavec.size() == 256)
            throw L"Cannot add more than 256 status bar parts.";

        if (index < 0)
            index = 0;
        else if (index > (int)datavec.size())
            index = datavec.size();

        PartData data;
        data.text = text;
        data.align = align;
        data.width = width;
        data.bevel = bevel;
        data.ownerdrawn = ownerdrawn;
        datavec.insert(datavec.begin() + index, data);

#ifndef DESIGNING
        for (auto part : parts)
            if (part->index >= index)
                ++part->index;
#endif
    }

    std::wstring StatusBar::PartText(int ix) const
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        return datavec[ix].text;
    }

    std::wstring StatusBar::PartText(int ix)
    {
        if (!HandleCreated() || simple)
        {
            if (ix < 0 || ix >= (int)datavec.size())
                throw L"Part index out of range.";
            return datavec[ix].text;
        }

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        cnt = min(cnt, (int)datavec.size());
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        if (ix >= cnt)
            return datavec[ix].text;

        WORD tlen = LOWORD(PassMessage(SB_GETTEXTLENGTH, ix, 0));
        if (tlen == 0)
            return std::wstring();

        std::wstring str(tlen + 1, 0);
        PassMessage(SB_GETTEXT, ix, (LPARAM)&str[0]);
        str.resize(tlen);
        if (str[0] == L'\t')
            str.erase(0, (tlen > 1 && str[1] == L'\t' ? 2 : 1));
        return str;
    }

    void StatusBar::SetPartText(int ix, const std::wstring &newtext)
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        datavec[ix].text = newtext;
        if (!simple)
            UpdatePartText(ix);
    }

    TextAlignments StatusBar::PartTextAlignment(int ix) const
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        return datavec[ix].align;
    }

    TextAlignments StatusBar::PartTextAlignment(int ix)
    {
        if (!HandleCreated() || simple)
        {
            if (ix < 0 || ix >= (int)datavec.size())
                throw L"Part index out of range.";
            return datavec[ix].align;
        }

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        cnt = min(cnt, (int)datavec.size());
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        if (ix >= cnt)
            return datavec[ix].align;

        WORD tlen = LOWORD(PassMessage(SB_GETTEXTLENGTH, ix, 0));
        if (tlen == 0)
            return taLeft;

        std::wstring str(tlen + 1, 0);
        PassMessage(SB_GETTEXT, ix, (LPARAM)&str[0]);
        return str[0] != L'\t' || tlen <= 1 ? taLeft : str[1] != L'\t' ? taCenter : taRight;
    }

    void StatusBar::SetPartTextAlignment(int ix, TextAlignments newalign)
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        datavec[ix].align = newalign;
        if (!simple)
            UpdatePartText(ix);
    }

    void StatusBar::SwapParts(int ix1, int ix2)
    {
        if (ix1 < 0 || ix1 >= (int)datavec.size() || ix2 < 0 || ix2 >= (int)datavec.size())
            throw L"Part index out of range.";

        std::swap(datavec[ix1], datavec[ix2]);
#ifdef DESIGNING
        std::swap(parts[ix1], parts[ix2]);
#else
        for (auto part : parts)
        {
            if (part->index == ix1)
                part->index = ix2;
            else if (part->index == ix2)
                part->index = ix1;
        }
#endif

        if (!simple)
            SetParts();
    }

#ifndef DESIGNING
    StatusBarPart StatusBar::Parts(int index)
    {
        return StatusBarPart(this, index);;
    }
#endif

    unsigned int StatusBar::PartWidth(int ix) const
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        return datavec[ix].width;
    }

    unsigned int StatusBar::PartWidth(int ix)
    {
        if (!HandleCreated() || simple)
        {
            if (ix < 0 || ix >= (int)datavec.size())
                throw L"Part index out of range.";
            return datavec[ix].width;
        }

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        cnt = min(cnt, (int)datavec.size());
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        if (ix >= cnt)
            return datavec[ix].width;

        std::vector<INT> w(cnt);
        PassMessage(SB_GETPARTS, cnt, (LPARAM)&w[0]);

        INT sizes[3];
        PassMessage(SB_GETBORDERS, 0, (LPARAM)sizes);

        int ww = w[ix];
        if (ww != -1)
            return (unsigned int)(ix == 0 ? ww - sizes[1] : ww  - w[ix - 1] - sizes[2]);
        return datavec[ix].width;
    }

    void StatusBar::SetPartWidth(int ix, unsigned int newwidth)
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        datavec[ix].width = newwidth;

        if (!simple)
            UpdatePart(ix);
    }

    bool StatusBar::PartOwnerDrawn(int ix) const
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        return datavec[ix].ownerdrawn;
    }

    bool StatusBar::PartOwnerDrawn(int ix)
    {
        if (!HandleCreated() || simple)
        {
            if (ix < 0 || ix >= (int)datavec.size())
                throw L"Part index out of range.";
            return datavec[ix].ownerdrawn;
        }

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        cnt = min(cnt, (int)datavec.size());
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        if (ix >= cnt)
            return datavec[ix].ownerdrawn;

        return (HIWORD(PassMessage(SB_GETTEXTLENGTH, ix, 0)) & SBT_OWNERDRAW) != 0;

    }

    void StatusBar::SetPartOwnerDrawn(int ix, bool newownerdrawn)
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        datavec[ix].ownerdrawn = newownerdrawn;

        if (!simple)
            UpdatePartText(ix);
    }

    StatusBarPartBevels StatusBar::PartBevel(int ix) const
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        return datavec[ix].bevel;
    }

    StatusBarPartBevels StatusBar::PartBevel(int ix)
    {
        if (!HandleCreated() || simple)
        {
            if (ix < 0 || ix >= (int)datavec.size())
                throw L"Part index out of range.";
            return datavec[ix].bevel;
        }

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        cnt = min(cnt, (int)datavec.size());
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        if (ix >= cnt)
            return datavec[ix].bevel;

        WORD data = HIWORD(PassMessage(SB_GETTEXTLENGTH, ix, 0)) & ~(WORD)(SBT_RTLREADING | SBT_OWNERDRAW);
        if (data == 0)
            return sbpbLowered;
        if (data == SBT_NOBORDERS)
            return sbpbNone;
        return sbpbRaised;
    }

    void StatusBar::SetPartBevel(int ix, StatusBarPartBevels newbevel)
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        datavec[ix].bevel = newbevel;
        if (!simple)
            UpdatePartText(ix);
    }

    Rect StatusBar::PartArea(int ix)
    {
        if (!HandleCreated() || simple)
            return Rect();

        int cnt = PassMessage(SB_GETPARTS, 256, 0);
        cnt = min(cnt, (int)datavec.size());
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";
        if (cnt <= ix)
            return Rect();

        Rect r;
        if (PassMessage(SB_GETRECT, ix, (LPARAM)&r) == FALSE)
            return Rect();

        return r;
    }

    int StatusBar::PartCount() const
    {
        return datavec.size();
    }

    void StatusBar::DeletePart(int ix)
    {
        if (ix < 0 || ix >= (int)datavec.size())
            throw L"Part index out of range.";

        datavec.erase(datavec.begin() + ix);
#ifdef DESIGNING
        parts[ix]->Destroy();
        parts.erase(parts.begin() + ix);
#else
        for (auto part : parts)
        {
            if (part->index == ix)
                part->MakeInvalid();
            else if (part->index > ix)
                --part->index;
        }
#endif
        if (!simple)
            SetParts();
    }


    //---------------------------------------------


    Tooltip::Tooltip() : base(), alwaysshow(true), balloon(false), closebtn(false), noanimate(false), nofade(false), hideprefix(false), styledlinks(true)
    {
        controlstyle << csTransparent;
        controlstyle -= csChild;
        controlstyle -= csEraseToColor;

        controlstate -= csVisible;
        CreateHandle();
        SetWindowPos(Handle(), HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    Tooltip::~Tooltip()
    {
    }

    void Tooltip::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_BAR_CLASSES);
        params.classname = TOOLTIPS_CLASS;
    }

    void Tooltip::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.x = CW_USEDEFAULT;
        params.y = CW_USEDEFAULT;
        params.width = CW_USEDEFAULT;
        params.height = CW_USEDEFAULT;

        params.style << wsPopup;
        params.style -= wsClipSiblings;
        params.extstyle << wsExNoActivate;

        if (alwaysshow)
            params.style << ttsAlwaysTip;
        if (balloon)
            params.style << ttsBalloon;
        if (closebtn)
            params.style << ttsCloseButton;
        if (noanimate)
            params.style << ttsNoAnimate;
        if (nofade)
            params.style << ttsNoFade;
        if (!hideprefix)
            params.style << ttsNoPrefix;
        if (styledlinks)
            params.style << ttsUseVisualStyle;
    }

    void Tooltip::InitHandle()
    {
        base::InitHandle();
        for (auto it = controls.begin(); it != controls.end(); ++it)
            Register(*it);
        PassMessage(TTM_SETMAXTIPWIDTH, 0, -1);
    }

    bool Tooltip::AlwaysShow()
    {
        return alwaysshow;
    }

    void Tooltip::SetAlwaysShow(bool newalwaysshow)
    {
        if (alwaysshow == newalwaysshow)
            return;
        alwaysshow = newalwaysshow;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Tooltip::Balloon()
    {
        return balloon;
    }

    void Tooltip::SetBalloon(bool newballoon)
    {
        if (balloon == newballoon)
            return;
        balloon = newballoon;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Tooltip::CloseButton()
    {
        return closebtn;
    }

    void Tooltip::SetCloseButton(bool newclosebtn)
    {
        if (closebtn == newclosebtn)
            return;
        closebtn = newclosebtn;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Tooltip::NoAnimate()
    {
        return noanimate;
    }

    void Tooltip::SetNoAnimate(bool newnoanimate)
    {
        if (noanimate == newnoanimate)
            return;
        noanimate = newnoanimate;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Tooltip::NoFade()
    {
        return nofade;
    }

    void Tooltip::SetNoFade(bool newnofade)
    {
        if (nofade == newnofade)
            return;
        nofade = newnofade;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Tooltip::HidePrefix()
    {
        return hideprefix;
    }

    void Tooltip::SetHidePrefix(bool newhideprefix)
    {
        if (hideprefix == newhideprefix)
            return;
        hideprefix = newhideprefix;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Tooltip::StyledHyperlinks()
    {
        return styledlinks;
    }

    void Tooltip::SetStyledHyperlinks(bool newstyledlinks)
    {
        if (styledlinks == newstyledlinks)
            return;
        styledlinks = newstyledlinks;
        if (HandleCreated())
            RecreateHandle();
    }

    void Tooltip::Register(Control *control)
    {
        if (HandleCreated())
        {
            TOOLINFO inf = {0};
            inf.cbSize = sizeof(TOOLINFO);
            inf.uFlags = TTF_IDISHWND | TTF_TRANSPARENT | TTF_SUBCLASS;
            inf.hwnd = control->Handle();
            inf.uId = (UINT_PTR)control->Handle();
            inf.lpszText = LPSTR_TEXTCALLBACK;

            PassMessage(TTM_ADDTOOL, 0, (LPARAM)&inf);
        }

        auto it = std::find(controls.begin(), controls.end(), control);
        if (it == controls.end())
            controls.push_back(control);
    }

    void Tooltip::Deregister(Control *control)
    {
        if (HandleCreated())
        {
            TOOLINFO inf = {0};
            inf.cbSize = sizeof(TOOLINFO);
            inf.uFlags = TTF_IDISHWND;
            inf.hwnd = control->Handle();
            inf.uId = (UINT)control->Handle();

            PassMessage(TTM_DELTOOL, 0, (LPARAM)&inf);
        }

        auto it = std::find(controls.begin(), controls.end(), control);
        if (it != controls.end())
            controls.erase(it);
    }


    //---------------------------------------------


}
/* End of NLIBNS */


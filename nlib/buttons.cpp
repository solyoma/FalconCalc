#include "stdafx_zoli.h"
#include "buttons.h"
#include "imagelist.h"
#include "themes.h"

#ifdef DESIGNING
#include "property_images.h"
#include "property_buttons.h"
#include "property_controlbase.h"
#include "designerform.h"
#include "designer.h"
//#include "designercontrols.h"
#include "designproperties.h"
#include "serializer.h"
#endif

//---------------------------------------------

namespace NLIBNS
{


#ifdef DESIGNING
    void Button::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"ParentBackground");
        serializer->HideProperty(L"ParentColor");
        serializer->HideProperty(L"Color");
        serializer->HideProperty(L"BorderStyle");

        serializer->AddEvent<Button, NotifyEvent>(L"OnClick", L"Control");
        serializer->Add(L"SetCancel", new BoolDesignProperty<Button>(L"Cancel", L"Behavior", &Button::Cancel, &Button::SetCancel))->SetDefault(false);
    }

    Size Button::DesignSize()
    {
        return BaseToPixel(50, 14);
    }
#endif

    Button::Button() : cancel(false)
    {
        controlstyle << csWantSysKey << csInTabOrder << csAcceptInput /* << csSelfErased */;
        controlstyle -= csEraseToColor;
        SetParentBackground(true);
    }

    Button::~Button()
    {
    }

    void Button::CreateClassParams(ClassParams &params)
    {
        params.classname = L"Button";
    }

    void Button::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        //params.style << bsOwnerdraw;
    }

    LRESULT Button::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT r, r2;
    
        switch (uMsg)
        {
        case WM_SETFOCUS:
            r2 = base::WindowProc(uMsg, wParam, lParam);
            r = PassMessage(WM_GETDLGCODE, 0, 0);
            if ((r & DLGC_BUTTON) == DLGC_BUTTON && (r & DLGC_UNDEFPUSHBUTTON) == DLGC_UNDEFPUSHBUTTON)
                PassMessage(BM_SETSTYLE, BS_DEFPUSHBUTTON | BS_TEXT, TRUE);
            return r2;
        case WM_KILLFOCUS:
            r2 = base::WindowProc(uMsg, wParam, lParam);
            r = PassMessage(WM_GETDLGCODE, 0, 0);
            if ((r & DLGC_BUTTON) == DLGC_BUTTON && (r & DLGC_DEFPUSHBUTTON) == DLGC_DEFPUSHBUTTON)
                PassMessage(BM_SETSTYLE, BS_PUSHBUTTON | BS_TEXT, TRUE);
            return r2;
        case wmDialogKey:
            if ((wParam == VK_RETURN && Focused()) || (wParam == VK_ESCAPE && cancel))
            {
                Click();
                return true;
            }
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool Button::HandleCommand(Control *parent, WPARAM wParam)
    {
        if (((wParam >> 16) & BN_CLICKED) == BN_CLICKED)
        {
            Click();
            return true;
        }
        return false;
    }

    //bool Button::HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush)
    //{
    //    if (!themes->AppThemed())
    //    {
    //        Canvas *c = GetCanvas();
    //        DrawParentBackground(ClientRect());
    //
    //        bgbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    //        SetTextColor(dc, GetFont()->Color());
    //        SetBkColor(dc, CLR_NONE);
    //        return true;
    //    }
    //
    //    return false;
    //}

    void Button::Paint(const Rect &updaterect)
    {
        base::Paint(updaterect);
    }

    void Button::EraseBackground()
    {
        if (themes->AppThemed())
        {
            DrawParentBackground(ClientRect());
            //return true;
        }
        //return false;
    }

    //#include <vsstyle.h>

    Rect Button::OpaqueRect()
    {
        //themes->TestMeasure(tcButton, BP_PUSHBUTTON, PBS_NORMAL, ClientRect());
        return themes->MeasureButtonBackgroundArea(ClientRect());
    }

    void Button::Click()
    {
        if (OnClick)
            OnClick(this, EventParameters());
    }

    bool Button::Cancel()
    {
        return cancel;
    }

    void Button::SetCancel(bool newcancel)
    {
        cancel = newcancel;
    }


    //---------------------------------------------


#ifdef DESIGNING
    ValuePair<FlatButtonTypes> FlatButtonTypeStrings[] = { 
        VALUEPAIR(fbtCheckbutton),
        VALUEPAIR(fbtCheckRadiobutton),
        VALUEPAIR(fbtPushbutton),
        VALUEPAIR(fbtRadiobutton),
        VALUEPAIR(fbtDropdownButton),
        VALUEPAIR(fbtSplitbutton),
    };

    ValuePair<ButtonImagePositions> ButtonImagePositionStrings[] = { 
        VALUEPAIR(bipAbove),
        VALUEPAIR(bipBelow),
        VALUEPAIR(bipCenter),
        VALUEPAIR(bipLeft),
        VALUEPAIR(bipRight),
    };

    ValuePair<ButtonContentPositions> ButtonContentPositionStrings[] = { 
        VALUEPAIR(bcpBottom),
        VALUEPAIR(bcpCenter),
        VALUEPAIR(bcpLeft),
        VALUEPAIR(bcpRight),
        VALUEPAIR(bcpTop),
    };

    Size FlatButton::DesignSize()
    {
        return BaseToPixel(50, 14);
    }

    void FlatButton::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->Find<BoolDesignProperty<Control>>(L"AcceptInput")->SetDefault(false);
        //serializer->Find<BoolDesignProperty<Control>>(L"ParentBackground")->SetDefault(true);
        serializer->Find<ColorDesignProperty<Control>>(L"Color")->SetDefault(clBtnFace);
        serializer->Add(L"Image", new ControlImageDesignProperty<FlatButton>(L"Image", L"Appearance", &FlatButton::Image));
        serializer->Add(L"SetCancel", new BoolDesignProperty<FlatButton>(L"Cancel", L"Behavior", &FlatButton::Cancel, &FlatButton::SetCancel))->SetDefault(false);
        serializer->Add(L"SetType", new FlatButtonTypesDesignProperty<FlatButton>(L"Type", L"Behavior", &FlatButton::Type, &FlatButton::SetType))->SetDefault(fbtPushbutton);
        serializer->Add(L"SetImagePosition", new ButtonImagePositionsDesignProperty<FlatButton>(L"ImagePosition", L"Appearance", &FlatButton::ImagePosition, &FlatButton::SetImagePosition))->SetDefault(bipLeft);
        serializer->Add(L"SetContentPosition", new ButtonContentPositionsDesignProperty<FlatButton>(L"ContentPosition", L"Appearance", &FlatButton::ContentPosition, &FlatButton::SetContentPosition))->SetDefault(bcpCenter);
        serializer->Add(L"SetDown", new BoolDesignProperty<FlatButton>(L"Down", L"Behavior", &FlatButton::Down, &FlatButton::SetDown))->SetDefault(false);
        serializer->Add(L"SetGroupIndex", new IntDesignProperty<FlatButton>(L"GroupIndex", L"Behavior", &FlatButton::GroupIndex, &FlatButton::SetGroupIndex))->SetDefault(0);
        serializer->Add(L"SetFlat", new BoolDesignProperty<FlatButton>(L"Flat", L"Appearance", &FlatButton::Flat, &FlatButton::SetFlat))->SetDefault(true);
        serializer->Add(L"SetMargin", new IntDesignProperty<FlatButton>(L"Margin", L"Appearance", &FlatButton::Margin, &FlatButton::SetMargin))->SetDefault(-1);
        serializer->Add(L"SetSpacing", new IntDesignProperty<FlatButton>(L"Spacing", L"Appearance", &FlatButton::Spacing, &FlatButton::SetSpacing))->SetDefault(-1);
        serializer->Add(L"SetShowText", new BoolDesignProperty<FlatButton>(L"ShowText", L"Appearance", &FlatButton::ShowText, &FlatButton::SetShowText))->SetDefault(true);

        serializer->AddEvent<FlatButton, NotifyEvent>(L"OnClick", L"Control");
        serializer->AddEvent<FlatButton, NotifyEvent>(L"OnSplitClick", L"Control");
        serializer->AddEvent<FlatButton, NotifyEvent>(L"OnDownChanged", L"Control");
        serializer->AddEvent<FlatButton, ButtonMeasureSplitSizeEvent>(L"OnMeasureSplitSize", L"Drawing");
        serializer->AddEvent<FlatButton, ButtonOwnerDrawEvent>(L"OnPaint", L"Drawing");
    }
#endif

    FlatButton::FlatButton() :
            type(fbtPushbutton), flat(true), down(false), groupindex(0), hovered(false), splithovered(false), pressed(false),
            showtext(true), imagepos(bipLeft), contentpos(bcpCenter), margin(-1), spacing(-1), showfocus(false), accelvisible(false),
            cancel(false), image(NULL)
    {
        bool oldpbg = ParentColor();
        SetColor(clBtnFace);
        SetParentColor(oldpbg);
        SetParentBackground(true);

        image = new ControlImage(this);
        controlstyle << csInTabOrder;
    }

    FlatButton::~FlatButton()
    {
        image->Destroy();
    }

    void FlatButton::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
        params.style << csVRedraw << csHRedraw;
    }

    LRESULT FlatButton::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_UPDATEUISTATE:
            if (HIWORD(wParam) == UISF_HIDEFOCUS && ((LOWORD(wParam) == UIS_CLEAR && !showfocus) || (LOWORD(wParam) == UIS_SET && showfocus)))
            {
                showfocus = LOWORD(wParam) == UIS_CLEAR;
                Invalidate();
            }
            else if (HIWORD(wParam) == UISF_HIDEACCEL && (LOWORD(wParam) == UIS_CLEAR || LOWORD(wParam) == UIS_SET))
            {
                if (accelvisible != (LOWORD(wParam) == UIS_CLEAR))
                {
                    accelvisible = LOWORD(wParam) == UIS_CLEAR;
                    Invalidate();
                }
            }
            break;
        case wmDialogKey:
            if ((wParam == VK_RETURN && Focused()) || (wParam == VK_ESCAPE && cancel))
            {
                Click();
                return true;
            }
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void FlatButton::EraseBackground()
    {
        if (!flat && !themes->AppThemed())
            return;// true;

        Canvas *c = GetCanvas();
        if (down && !themes->AppThemed())
        {
            if (type == fbtSplitbutton)
                c->FillRect(SplitRect());
            c->SetBrush(cl3DHighlight);
            c->FillRect(ButtonRect());
        }
        else if (UsingParentBackground())
            DrawParentBackground(ClientRect());
        else
            c->FillRect(ClientRect());

        //return true;
    }

    void FlatButton::Resizing()
    {
        base::Resizing();

        //if (!flat && UsingParentBackground())
        //    Invalidate();
    }

    void FlatButton::Paint(const Rect &updaterect)
    {
        Rect r = ClientRect();

#ifdef DESIGNING
        bool hovered = true;
        bool splithovered = true;
        if (!flat || !Designing())
        {
            hovered = this->hovered;
            splithovered = this->splithovered;
        }
#endif

        Canvas *c = GetCanvas();

        Rect rb = ButtonRect();
        Rect rs = SplitRect();

        bool skip = false;

        ButtonOwnerDrawState drawstate = hovered ? (down ? bodsDown : pressed ? bodsPressed : bodsHovered) : splithovered ? (pressed ? bodsSecondaryPressed : bodsSecondaryHovered) : bodsNormal;

        if (OnPaint)
            OnPaint(this, ButtonOwnerDrawParameters(c, drawstate, bodoBackground, r, rs.Width(), Point(), Point(), skip));
        if (!flat)
        {
            if (!skip)
            {
                themes->DrawButton(c, r, !Enabled() ? tbsDisabled : hovered || splithovered || down ? (down || pressed ? tbsPressed : tbsHot) : Focused() ? tbsDefaulted : tbsNormal);
                if (type == fbtSplitbutton)
                {
                    c->SelectStockPen(sp3DShadow);
                    c->Line(rs.left - 1, rs.top + 4 * Scaling, rs.left - 1, rs.bottom - 4 * Scaling);
                    c->SelectStockPen(sp3DHighlight);
                    c->Line(rs.left, rs.top + 4 * Scaling, rs.left, rs.bottom - 4 * Scaling);
                }
            }
        }
        else
        {
            if (type == fbtDropdownButton)
            {
                if (!skip)
                    themes->DrawToolbarDropdownButton(c, r, !Enabled() ? ttsDisabled : hovered || down ? (down || pressed ? ttsPressed : ttsHot) : Focused() ? ttsDefaulted : ttsNormal);
            }
            else if (type == fbtSplitbutton)
            {
                if (!skip)
                    themes->DrawToolbarSplitbutton(c, rb, !Enabled() ? ttsDisabled : hovered || splithovered || down ? (down || pressed ? ttsPressed : ttsHot) : Focused() ? ttsDefaulted : ttsNormal);
                skip = false;
                if (OnPaint)
                    OnPaint(this, ButtonOwnerDrawParameters(c, drawstate, bodoSecondaryBackground, r, rs.Width(), Point(), Point(), skip));
                if (!skip)
                    themes->DrawToolbarSplitbuttonDropdown(c, rs, !Enabled() ? ttsDisabled : splithovered ? (pressed ? ttsPressed : ttsHot) : Focused() ? ttsDefaulted : ttsNormal);
            }
            else
            {
                if (!skip)
                    themes->DrawToolbarButton(c, r, !Enabled() ? ttsDisabled : hovered || down ? (down || pressed ? ttsPressed : ttsHot) : Focused() ? ttsDefaulted : ttsNormal);
            }
        }
        skip = false;

        if (AcceptInput() && Focused() && showfocus)
        {
            Rect ir = themes->MeasureButtonFocusRectangle(r.Inflate(0, 0, -SplitWidth(), 0));
            HDC dc = c->GetDC();
            DrawFocusRect(dc, &ir);
            c->ReturnDC();
        }

        ButtonMeasurements m;
        std::wstring t = showtext ? Text() : std::wstring();
        MeasureButtonContents(c, r.Inflate(0, 0, -SplitWidth(), 0), showtext ? Text() : std::wstring(),
                              image->HasImage() ? Size(image->Width(), image->Height()) : Size(0, 0),
                              imagepos, contentpos, margin, spacing, m);

        if (OnPaint)
            OnPaint(this, ButtonOwnerDrawParameters(c, drawstate, bodoContents, r, rs.Width(), Point(m.content.left + m.imgpos.x, m.content.top + m.imgpos.y), Point(r.left + m.content.left + m.textpos.x, r.top + m.content.top + m.textpos.y), skip));
        if (!skip)
        {
            if (image->HasImage())
                image->Draw(c, m.content.left + m.imgpos.x, m.content.top + m.imgpos.y, !Enabled() ? cisDisabled : pressed && hovered ? cisPressed : hovered ? cisHovered : down ? cisDown : cisNormal);
            if (showtext && !t.empty())
            {
                if (!Enabled())
                    c->FormatGrayText(Rect(r.left + m.content.left + m.textpos.x, r.top  + m.content.top + m.textpos.y, r.right, r.bottom), t, tdoSingleLine | (accelvisible ? 0 : tdoHidePrefix));
                else
                    c->FormatText(Rect(r.left + m.content.left + m.textpos.x, r.top + m.content.top + m.textpos.y, r.right, r.bottom), t, tdoSingleLine | (accelvisible ? 0 : tdoHidePrefix));
            }
        }
        skip = false;

        if (type == fbtDropdownButton || (!flat && type == fbtSplitbutton))
        {
            Size gs = themes->MeasureToolbarDropButtonGlyph();
            r.top = (r.Height() - gs.cy) / 2;
            r.bottom = r.top + gs.cy;
            r.right -= 4 * Scaling + 1;
            r.left = r.right - gs.cx;
            if (OnPaint)
                OnPaint(this, ButtonOwnerDrawParameters(c, drawstate, bodoSecondaryContents, r, rs.Width(), Point(r.left, r.top), Point(), skip));
            if (!skip)
                themes->DrawToolbarDropdownButtonGlyph(c, r, !Enabled() ? ttsDisabled : splithovered ? (pressed ? ttsPressed : ttsHot) : Focused() ? ttsDefaulted : ttsNormal);
        }
        //skip = false;
    }

    void FlatButton::GainFocus(HWND otherwindow)
    {
        Invalidate();
        base::GainFocus(otherwindow);
    }

    void FlatButton::LoseFocus(HWND otherwindow)
    {
        Invalidate();
        base::LoseFocus(otherwindow);
    }

    void FlatButton::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        if (key == L' ' && Focused())
            Click();

        base::KeyPush(keycode, key, vkeys);
    }

    void FlatButton::CaptureChanged()
    {
        if (pressed)
        {
            pressed = false;
            Invalidate();
        }
    }

    void FlatButton::MouseEnter()
    {
        base::MouseEnter();
        //hovered = true;
        //Invalidate();
    }

    void FlatButton::MouseLeave()
    {
        base::MouseLeave();

        hovered = false;
        splithovered = false;
        Invalidate();
    }

    void FlatButton::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
    {
        Rect r = ClientRect();
        int s = type == fbtSplitbutton ? SplitWidth() : 0;
        bool newhovered = x >= 0 && y >= 0 && x < r.Width() - s && y < r.Height();
        bool newsplithovered = type == fbtSplitbutton && x >= r.Width() - s && y >= 0 && x < r.Width() && y < r.Height();
        if (newhovered != hovered)
        {
            hovered = newhovered;
            InvalidateRect(r.Inflate(0, 0, -s, 0));
        }
        if (newsplithovered != splithovered)
        {
            splithovered = newsplithovered;
            InvalidateRect(SplitRect());
        }

        base::MouseMove(x, y, vkeys);
    }

    void FlatButton::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (button == mbLeft && (!vkeys.contains(vksDouble) || (type != fbtRadiobutton && type != fbtCheckRadiobutton) || !down))
        {
            if (down && type == fbtRadiobutton)
            {
                base::MouseDown(x, y, button, vkeys);
                return;
            }

            pressed = true;
            if (type == fbtSplitbutton)
            {
                if (splithovered)
                    InvalidateRect(SplitRect());
                else
                    InvalidateRect(ButtonRect());
            }
            else
                Invalidate();
        }

        base::MouseDown(x, y, button, vkeys);
    }

    void FlatButton::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if (button == mbLeft && pressed)
        {
            bool olddown = down;
            if (hovered)
            {
                if ((type == fbtCheckbutton || (type == fbtCheckRadiobutton && down)))
                    down = !down;
                else if (!down && (type == fbtCheckRadiobutton || type == fbtRadiobutton))
                {
                    PopButtons(false);
                    down = !down;
                }
            }
            pressed = false;
            Invalidate();

            if (hovered && OnClick)
                OnClick(this, EventParameters());
            if (down != olddown && OnDownChanged)
                OnDownChanged(this, EventParameters());
            if (splithovered && OnSplitClick)
                OnSplitClick(this, EventParameters());
        }

        base::MouseUp(x, y, button, vkeys);
    }

    Rect FlatButton::ButtonRect()
    {
        Rect r = ClientRect();
        if (type != fbtSplitbutton)
            return r;
        r.right -= SplitWidth(); 
        return r;
    }

    Rect FlatButton::SplitRect()
    {
        if (type != fbtSplitbutton)
            return Rect();
        Rect r = ClientRect();
        r.left = r.right - SplitWidth();
        return r;
    }

    int FlatButton::SplitWidth()
    {
        if (type != fbtSplitbutton && type != fbtDropdownButton)
            return 0;

        int result = (type == fbtSplitbutton ? (4 * Scaling) * 2 : 4 * Scaling + Scaling) + themes->MeasureToolbarDropButtonGlyph().cx;
        if (OnMeasureSplitSize)
            OnMeasureSplitSize(this, ButtonMeasureSplitSizeParameters(result));
        return result;
    }

    void FlatButton::PopButtons(bool thistoo)
    {
        Control *cc = Parent();
        if (!cc)
            return;

        for (int ix = 0; ix < cc->ControlCount(); ++ix)
        {
            FlatButton *btn = dynamic_cast<FlatButton*>(cc->Controls(ix));
            if (!btn || (btn == this && !thistoo) || !btn->Enabled() || btn->groupindex != groupindex || btn->type != type)
                continue;
            if (btn->down)
            {
                btn->down = false;
                btn->Invalidate();
                //if (btn->OnDownChanged)
                //    btn->OnDownChanged(btn, EventParameters());
#ifdef DESIGNING
                if (btn->Designing() && designer && designer->MainPropertyOwner(btn))
                    designer->InvalidateRow(btn, L"Down");
#endif
            }
        }
    }

    void FlatButton::UpdateButtonTypes(FlatButtonTypes newtype)
    {
        Control *cc = Parent();
        if (!cc)
            return;

        for (int ix = 0; ix < cc->ControlCount(); ++ix)
        {
            FlatButton *btn = dynamic_cast<FlatButton*>(cc->Controls(ix));
            if (!btn || btn == this || btn->groupindex != groupindex || (btn->type != fbtCheckRadiobutton && btn->type != fbtRadiobutton))
                continue;
            btn->type = newtype;
#ifdef DESIGNING
            if (btn->Designing() && designer && designer->MainPropertyOwner(btn))
                designer->InvalidateRow(btn, L"Type");
#endif
        }
    }

    bool FlatButton::DownInGroup()
    {
        Control *cc = Parent();
        if (!cc)
            return false;

        for (int ix = 0; ix < cc->ControlCount(); ++ix)
        {
            FlatButton *btn = dynamic_cast<FlatButton*>(cc->Controls(ix));
            if (!btn || btn == this || btn->groupindex != groupindex || (btn->type != fbtCheckRadiobutton && btn->type != fbtRadiobutton))
                continue;
            if (btn->down)
                return true;
        }
        return false;
    }

    int FlatButton::UnusedIndex()
    {
        int highest = -1;
        Control *cc = Parent();
        if (!cc)
            return -1;

        for (int ix = 0; ix < cc->ControlCount(); ++ix)
        {
            FlatButton *btn = dynamic_cast<FlatButton*>(cc->Controls(ix));
            if (!btn || btn == this || (btn->type != fbtCheckRadiobutton && btn->type != fbtRadiobutton))
                continue;
            highest = max(highest, btn->groupindex);
        }
        return highest + 1;
    }

    FlatButtonTypes FlatButton::GroupType()
    {
        Control *cc = Parent();
        if (!cc)
            return type;

        for (int ix = 0; ix < cc->ControlCount(); ++ix)
        {
            FlatButton *btn = dynamic_cast<FlatButton*>(cc->Controls(ix));
            if (!btn || btn == this || btn->groupindex != groupindex || (btn->type != fbtCheckRadiobutton && btn->type != fbtRadiobutton))
                continue;
            return btn->type;
        }
        return type;
    }

    bool FlatButton::Down()
    {
        return down;
    }

    void FlatButton::SetDown(bool newdown)
    {
        if (down == newdown || type == fbtPushbutton || type == fbtDropdownButton || type == fbtSplitbutton || (type == fbtRadiobutton && !newdown))
            return;
        if ((type == fbtCheckRadiobutton && newdown) || type == fbtRadiobutton)
            PopButtons(false);
        down = newdown;
        Invalidate();
        if (OnDownChanged)
            OnDownChanged(this, EventParameters());
    }

    FlatButtonTypes FlatButton::Type()
    {
        return type;
    }

    void FlatButton::SetType(FlatButtonTypes newtype)
    {
        if (type == newtype)
            return;

        switch (newtype)
        {
        case fbtPushbutton:
        case fbtDropdownButton:
        case fbtSplitbutton:
            if (down)
            {
                down = false;
                if (OnDownChanged)
                    OnDownChanged(this, EventParameters());
#ifdef DESIGNING
                if (Designing() && designer && designer->MainPropertyOwner(this))
                    designer->InvalidateRow(this, L"Down");
#endif
            }
            break;
        case fbtCheckbutton:
            // no change
            break;
        case fbtCheckRadiobutton:
        case fbtRadiobutton:
            if (type != fbtCheckRadiobutton && type != fbtRadiobutton)
            {
                UpdateButtonTypes(newtype);

                if (type == fbtCheckbutton && down && DownInGroup())
                {
                    down = false;
                    Invalidate();
                    if (OnDownChanged)
                        OnDownChanged(this, EventParameters());
#ifdef DESIGNING
                    if (Designing() && designer && designer->MainPropertyOwner(this))
                        designer->InvalidateRow(this, L"Down");
#endif
                }
            }
            else
                groupindex = UnusedIndex();
            break;
        default:
            break;
        }

        type = newtype;
        Invalidate();
    }

    int FlatButton::GroupIndex()
    {
        return groupindex;
    }

    void FlatButton::SetGroupIndex(int newindex)
    {
        if (groupindex == newindex)
            return;

        groupindex = newindex;
        if (type == fbtCheckRadiobutton || type == fbtRadiobutton)
        {
            type = GroupType();
            if (down && DownInGroup())
            {
                down = false;
                Invalidate();
                if (OnDownChanged)
                    OnDownChanged(this, EventParameters());
#ifdef DESIGNING
                if (Designing() && designer && designer->MainPropertyOwner(this))
                    designer->InvalidateRow(this, L"Down");
#endif
            }
        }
    }

    bool FlatButton::Flat()
    {
        return flat;
    }

    void FlatButton::SetFlat(bool newflat)
    {
        if (flat == newflat)
            return;
        flat = newflat;
        if (IsVisible())
            Invalidate();
    }

    ControlImage* FlatButton::Image()
    {
        return image;
    }

    bool FlatButton::ShowText()
    {
        return showtext;
    }

    void FlatButton::SetShowText(bool newshowtext)
    {
        if (showtext == newshowtext)
            return;
        showtext = newshowtext;

        Invalidate();
    }

    ButtonImagePositions FlatButton::ImagePosition()
    {
        return imagepos;
    }

    void FlatButton::SetImagePosition(ButtonImagePositions newpos)
    {
        if (imagepos == newpos)
            return;
        imagepos = newpos;

        Invalidate();
    }

    ButtonContentPositions FlatButton::ContentPosition()
    {
        return contentpos;
    }

    void FlatButton::SetContentPosition(ButtonContentPositions newpos)
    {
        if (contentpos == newpos)
            return;
        contentpos = newpos;

        Invalidate();
    }

    int FlatButton::Margin()
    {
        return margin;
    }

    void FlatButton::SetMargin(int newmargin)
    {
        if (margin == newmargin)
            return;
        margin = newmargin;

        Invalidate();
    }

    int FlatButton::Spacing()
    {
        return spacing;
    }

    void FlatButton::SetSpacing(int newspacing)
    {
        if (spacing == newspacing)
            return;
        spacing = newspacing;

        Invalidate();
    }

    void FlatButton::SetImageLayout(ButtonImagePositions newpos, int newmargin, int newspacing)
    {
        if (imagepos == newpos && margin == newmargin && spacing == newspacing)
            return;

        imagepos = newpos;
        margin = newmargin;
        spacing = newspacing;

        Invalidate();
    }

    bool FlatButton::Cancel()
    {
        return cancel;
    }

    void FlatButton::SetCancel(bool newcancel)
    {
        cancel = newcancel;
    }

    void FlatButton::Click()
    {
        if (OnClick)
            OnClick(this, EventParameters());
    }

    void FlatButton::SplitClick()
    {
        if (OnSplitClick)
            OnSplitClick(this, EventParameters());
    }

    void FlatButton::ChangeNotify(Object *object, int changetype)
    {
        base::ChangeNotify(object, changetype);
        if (object == image)
            Invalidate();
    }


    void MeasureButtonContents(Canvas *c, const Rect &r, const std::wstring &text, const Size &isiz, ButtonImagePositions imagepos, ButtonContentPositions contentpos, int margin, int spacing, ButtonMeasurements &m)
    {
        bool hastext = !text.empty();
        bool hasimg = isiz.cx != 0 && isiz.cy != 0;

        if (!hastext && !hasimg)
        {
            memset(&m, 0, sizeof(ButtonMeasurements));
            return;
        }

        m.btnarea = r;
        m.imgsize = isiz;
        m.ts = !hastext ? Size(0, 0) : c->MeasureFormattedText(text, tdoSingleLine);
        m.spacing = !hastext || m.ts == Size(0, 0) ? 0 : spacing;
        m.margin = margin;
        Size siz;

        if (!hastext || !hasimg)
        {
            if (m.spacing < 0)
                m.spacing = 0;

            m.textpos = Point(0, 0);
            m.imgpos = Point(0, 0);

            siz = !hastext ? isiz : m.ts;
        }
        else
        {
            switch (imagepos)
            {
            case bipLeft:
                if (m.spacing < 0)
                    m.spacing = max(3 * Scaling, (r.Width() - 8 - m.ts.cx - isiz.cx) * 0.2);
                else
                    m.spacing *= Scaling;

                siz = Size(m.spacing + m.ts.cx + isiz.cx, max(m.ts.cy, isiz.cy));
                m.imgpos = Point(0, (siz.cy - isiz.cy) / 2);
                m.textpos = Point(isiz.cx + m.spacing, (siz.cy - m.ts.cy) / 2);
                break;
            case bipAbove:
                if (m.spacing < 0)
                    m.spacing = max(3 * Scaling, (r.Height() - 8 - m.ts.cy - isiz.cy) * 0.2);
                else
                    m.spacing *= Scaling;

                siz = Size(max(m.ts.cx, isiz.cx), m.spacing + m.ts.cy + isiz.cy);
                m.imgpos = Point((siz.cx - isiz.cx) / 2, 0);
                m.textpos = Point((siz.cx - m.ts.cx) / 2, isiz.cy + m.spacing);
                break;
            case bipRight:
                if (m.spacing < 0)
                    m.spacing = max(3 * Scaling, (r.Width() - 8 - m.ts.cx - isiz.cx) * 0.2);
                else
                    m.spacing *= Scaling;

                siz = Size(m.spacing + m.ts.cx + isiz.cx, max(m.ts.cy, isiz.cy));
                m.imgpos = Point(siz.cx - isiz.cx, (siz.cy - isiz.cy) / 2);
                m.textpos = Point(0, (siz.cy - m.ts.cy) / 2);
                break;
            case bipBelow:
                if (m.spacing < 0)
                    m.spacing = max(3 * Scaling, (r.Height() - 8 - m.ts.cy - isiz.cy) * 0.2);
                else
                    m.spacing *= Scaling;

                siz = Size(max(m.ts.cx, isiz.cx), m.spacing + m.ts.cy + isiz.cy);
                m.imgpos = Point((siz.cx - isiz.cx) / 2, siz.cy - isiz.cy);
                m.textpos = Point((siz.cx - m.ts.cx) / 2, 0);
                break;
            case bipCenter:
                m.spacing = 0;
                siz = Size(max(isiz.cx, m.ts.cx), max(isiz.cy, m.ts.cy));
                m.textpos = Point((siz.cx - m.ts.cx) / 2, (siz.cy - m.ts.cy) / 2);
                m.imgpos = Point((siz.cx - isiz.cx) / 2, (siz.cy - isiz.cy) / 2);
                break;
            }
        }

        switch (contentpos)
        {
        case bcpLeft:
            if (m.margin == -1)
                m.margin = 6 * Scaling;
            else
                m.margin *= Scaling;

            m.content = RectS(m.margin, (r.Height() - siz.cy) / 2, siz.cx, siz.cy);
            break;
        case bcpTop:
            if (m.margin == -1)
                m.margin = 6 * Scaling;
            else
                m.margin *= Scaling;

            m.content = RectS((r.Width() - siz.cx) / 2, m.margin, siz.cx, siz.cy);
            break;
        case bcpRight:
            if (m.margin == -1)
                m.margin = 6 * Scaling;
            else
                m.margin *= Scaling;

            m.content = RectS(r.Width() - m.margin - siz.cx, (r.Height() - siz.cy) / 2, siz.cx, siz.cy);
            break;
        case bcpBottom:
            if (m.margin == -1)
                m.margin = 6 * Scaling;
            else
                m.margin *= Scaling;

            m.content = RectS((r.Width() - siz.cx) / 2, r.Height() - m.margin - siz.cy, siz.cx, siz.cy);
            break;
        case bcpCenter:
            m.margin = -1;
            m.content = RectS((r.Width() - siz.cx) / 2, (r.Height() - siz.cy) / 2, siz.cx, siz.cy);
            break;
        }
    }


}
/* End of NLIBNS */



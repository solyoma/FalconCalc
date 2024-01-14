#include "stdafx_zoli.h"

#include "graphiccontrol.h"
#include "themes.h"
#ifdef DESIGNING
#include "property_controlbase.h"
#include "property_graphiccontrol.h"
#include "serializer.h"
#include "designproperties.h"
#include "designer.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING
    // Bevel enum strings
    ValuePair<BevelLineTypes> BevelLineTypeStrings[] = {
        VALUEPAIR(bltSunken),
        VALUEPAIR(bltRaised),
        VALUEPAIR(bltDoubleSunken),
        VALUEPAIR(bltDoubleRaised),
        VALUEPAIR(bltSunkenRaised),
        VALUEPAIR(bltRaisedSunken),
    };

        ValuePair<BevelShapeTypes> BevelShapeTypeStrings[] = {
        VALUEPAIR(bstLeftLine),
        VALUEPAIR(bstTopLine),
        VALUEPAIR(bstRightLine),
        VALUEPAIR(bstBottomLine),
        VALUEPAIR(bstMiddleHorzLine),
        VALUEPAIR(bstMiddleVertLine),
        VALUEPAIR(bstBox),
        VALUEPAIR(bstSpacer),
    };

        ValuePair<LabelShowAccelerators> LabelShowAcceleratorStrings[] = {
        VALUEPAIR(lsaShow),
        VALUEPAIR(lsaHide),
        VALUEPAIR(lsaAuto),
    };

    Size Label::DesignSize()
    {
        return Size(80, 21);
    }

    void Label::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"TabOrder");
        serializer->HideProperty(L"ParentBackground");
        serializer->Add(L"SetTransparent", new BoolDesignProperty<Label>(L"Transparent", L"Appearance", &Label::Transparent, &Label::SetTransparent))->SetDefault(false);
        serializer->Add(L"SetAutoSize", new BoolDesignProperty<Label>(L"AutoSize", L"Positioning", &Label::AutoSize, &Label::SetAutoSize))->SetDefault(true);
        serializer->Add(L"SetWordWrap", new BoolDesignProperty<Label>(L"WordWrap", L"Layout", &Label::WordWrap, &Label::SetWordWrap))->SetDefault(false);
        serializer->Add(L"SetShowAccelerator", new LabelShowAcceleratorDesignProperty<Label>(L"ShowAccelerator", L"Appearance", &Label::ShowAccelerator, &Label::SetShowAccelerator))->SetDefault(lsaAuto);
        serializer->Add(L"SetTextAlignment", new TextAlignmentsDesignProperty<Label>(L"TextAlignment", L"Layout", &Label::TextAlignment, &Label::SetTextAlignment))->SetDefault(taLeft);
        serializer->Add(L"SetVerticalTextAlignment", new VerticalTextAlignmentsDesignProperty<Label>(L"VerticalTextAlignment", L"Layout", &Label::VerticalTextAlignment, &Label::SetVerticalTextAlignment))->SetDefault(vtaTop);
        serializer->Add(L"SetAccessControl", new AccessControlDesignProperty<Label>(L"AccessControl", L"Control", &Label::AccessControl, &Label::SetAccessControl));

        serializer->Find(L"Text")->MakeDefault();
    }

#endif


    Label::Label() : accesscontrol(NULL), autosize(true), wordwrap(false), accel(lsaAuto), accelvisible(false), textalign(taLeft), valign(vtaTop)
    {
        controlstyle -= csParentBackground;
        controlstyle -= csMouseCapture;
        controlstyle << csWantSysKey << csUpdateOnTextChange;
        SetParentBackground(false);
    }

    Label::~Label()
    {
    }

    void Label::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
    }

    void Label::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
    }

    void Label::InitHandle()
    {
        base::InitHandle();

        if (autosize)
            Resize();
    }

    LRESULT Label::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_SETTEXT:
        case WM_SETFONT:
            if (!autosize || controlstate.contains(csRecreating))
                break;
            if (HandleCreated())
                PostMessage(Handle(), wmAutoResize, 0, 0);
            else
                Resize();
            break;
        case WM_UPDATEUISTATE:
            if (accel != lsaAuto || HIWORD(wParam) != UISF_HIDEACCEL)
                break;
            if (LOWORD(wParam) == UIS_CLEAR && !accelvisible)
            {
                accelvisible = true;
                Invalidate();
            }
            else if (LOWORD(wParam) == UIS_SET && accelvisible)
            {
                accelvisible = false;
                Invalidate();
            }
            break;
        case wmAutoResize:
            Resize();
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool Label::HandleSysKey(WCHAR key)
    {
        if (accesscontrol && accesscontrol->AcceptInput() && ContainsAccelerator(Text(), key))
        {
            accesscontrol->Focus();
            return true;
        }
        return base::HandleSysKey(key);
    }

    void Label::Showing()
    {
        if (autosize)
            Resize();
    }

    bool Label::Transparent()
    {
        return controlstyle.contains(csTransparent);
    }

    void Label::SetTransparent(bool newtrans)
    {
        if (controlstyle.contains(csTransparent) == newtrans)
            return;
        if (!newtrans)
            controlstyle -= csTransparent;
        else
            controlstyle << csTransparent;
        if (HandleCreated())
            RecreateHandle();
    }

    bool Label::AutoSize()
    {
        return autosize;
    }

    void Label::SetAutoSize(bool newauto)
    {
        if (newauto == autosize)
            return;
        autosize = newauto;
        //if (HandleCreated())
        if (autosize)
            Resize();
    }

    bool Label::WordWrap()
    {
        return wordwrap;
    }

    void Label::SetWordWrap(bool newwordwrap)
    {
        if (wordwrap == newwordwrap)
            return;
        wordwrap = newwordwrap;
        if (autosize)
            Resize();
        if (HandleCreated())
        {
            if (IsVisible())
                Invalidate();
        }
    }

    LabelShowAccelerators Label::ShowAccelerator()
    {
        return accel;
    }

    void Label::SetShowAccelerator(LabelShowAccelerators newshowaccelerator)
    {
        if (accel == newshowaccelerator)
            return;
        accel = newshowaccelerator;
        accelvisible = accel == lsaShow;
#ifdef DESIGNING
        if (Designing() && accel == lsaAuto)
            accelvisible = true;
#endif
        if (HandleCreated() && IsVisible())
            Invalidate();
    }

    bool Label::AcceleratorVisible()
    {
        return accelvisible;
    }

    Control *Label::AccessControl()
    {
        return accesscontrol;
    }

    void Label::SetAccessControl(Control *newaccesscontrol)
    {
        if (accesscontrol == newaccesscontrol)
            return;
        if (accesscontrol)
            RemoveFromNotifyList(accesscontrol, nrActivation);
        accesscontrol = newaccesscontrol;
        if (accesscontrol)
            AddToNotifyList(accesscontrol, nrActivation);
    }

    void Label::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        if (accesscontrol == object)
        {
            accesscontrol = NULL;
#ifdef DESIGNING
            if (Designing() && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"AccessControl");
#endif
        }
    }

    TextAlignments Label::TextAlignment()
    {
        return textalign;
    }

    void Label::SetTextAlignment(TextAlignments newtextalign)
    {
        if (textalign == newtextalign)
            return;
        textalign = newtextalign;
        if (HandleCreated() && IsVisible())
            Invalidate();
    }

    VerticalTextAlignments Label::VerticalTextAlignment()
    {
        return valign;
    }

    void Label::SetVerticalTextAlignment(VerticalTextAlignments newvalign)
    {
        if (valign == newvalign)
            return;
        valign = newvalign;
        if (HandleCreated() && IsVisible())
            Invalidate();
    }

    void Label::Resize()
    {
        if (!autosize || controlstate.contains(csRecreating))
            return;
        Rect cr = HandleCreated() ? ClientRect() : WindowRect();
        Size s = Measure(cr);
        Rect r = WindowRect();
        SetBounds(RectS(textalign == taLeft ? r.left : textalign == taRight ? r.right - s.cx : r.left - (s.cx - r.Width()) / 2, 
                        valign == vtaTop ? r.top : valign == vtaBottom ? r.bottom - s.cx : r.top - (s.cx - r.Height()) / 2,
                        s.cx + r.Width() - cr.Width(), s.cy + r.Height() - cr.Height()));
    }

    void Label::WindowBoundsChanged(const Rect &oldrect, const Rect &newrect)
    {
        base::WindowBoundsChanged(oldrect, newrect);

        if ((oldrect.Width() == newrect.Width() || textalign == taLeft) && (oldrect.Height() == newrect.Height() || valign == vtaTop))
            return;
        Invalidate();
    }

    void Label::Paint(const Rect &updaterect)
    {
        Rect cr = ClientRect();
        if (valign != vtaTop)
        {
            Size s = Measure(cr);
            if (valign == vtaBottom)
                cr.top = cr.bottom - s.cy;
            else if (valign == vtaMiddle)
                cr.top = cr.top + (cr.Height() - s.cy) / 2;
        }

        Canvas *c = GetCanvas();
        Gdiplus::Region old;
        c->GetClip(old);

        CanvasTextAlignmentSet cs = c->TextAlignment();
        c->SetTextAlignment(ctaLeft | ctaTop);

        c->SetClip(updaterect);

        TextDrawOptionSet opt(tdoExpandTabs);
        if (textalign == taRight)
            opt << tdoRight;
        else if (textalign == taCenter)
            opt << tdoCenter;
        if (wordwrap)
            opt << tdoWordBreak;
        if (!accelvisible)
            opt << tdoHidePrefix;
        c->FormatText(cr, Text(), opt);

        c->SetClip(old);

        c->SetTextAlignment(cs);
    }

    void Label::EraseBackground()
    {
        Canvas *c = GetCanvas();
        c->SetBrush(GetColor());
        c->FillRect(ClientRect());
    }

    Size Label::Measure(const Rect &rect)
    {
        Size s;
    
        Canvas *c = HandleCreated() ? GetCanvas() : NULL;
        CanvasTextAlignmentSet cs;
        HFONT oldfont = NULL;
        if (HandleCreated())
        {
            cs = c->TextAlignment();
            c->SetTextAlignment(ctaLeft | ctaTop);
        }
        HDC dc = HandleCreated() ? c->GetDC() : CreateCompatibleDC(0);
        if (!dc)
        {
            if (HandleCreated())
                c->SetTextAlignment(cs);
            return Size(rect.Width(), rect.Height());
        }
        if (!HandleCreated())
        {
            oldfont = (HFONT)SelectObject(dc, GetFont().Handle());
        }

        const std::wstring &t = Text();

        if (!t.empty())
        {
            UINT format = DT_CALCRECT | (textalign == taLeft ? DT_LEFT : textalign == taRight ? DT_RIGHT : DT_CENTER) | (!wordwrap ? 0 : DT_WORDBREAK) | (accelvisible ? DT_HIDEPREFIX : 0) | DT_EXPANDTABS;
            Rect calc = rect;
            if (calc.left == calc.right)
            {
                if (textalign != taLeft)
                    --calc.left;
                else
                    ++calc.right;
            }
            DrawTextW(dc, t.c_str(), -1, &calc, format);
            s.cx = calc.Width();
            s.cy = calc.Height();
        }
        else
        {
            GetTextExtentPoint32(dc, L"WyLtl", 5, &s); 
            s.cx = 0;
        }

        if (HandleCreated())
        {
            c->ReturnDC();
            c->SetTextAlignment(cs);
        }
        else
        {
            SelectObject(dc, oldfont);
            DeleteDC(dc);
        }

        return s;
    }

    void Label::SetText(const std::wstring &newtext)
    {
        base::SetText(newtext);
        if (!HandleCreated())
            Resize();
    }


    //---------------------------------------------


#ifdef DESIGNING
    void Bevel::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"TabOrder");
        serializer->HideProperty(L"BorderStyle");
        serializer->HideProperty(L"ParentFont");
        serializer->HideProperty(L"Font");
        serializer->HideProperty(L"Text");
        serializer->HideProperty(L"WantedKeyTypes");
        serializer->HideProperty(L"Enabled");

        serializer->Add(L"SetLineType", new BevelLineTypesDesignProperty<Bevel>(L"LineType", L"Appearance", &Bevel::LineType, &Bevel::SetLineType))->SetDefault(bltSunken);
        serializer->Add(L"SetShape", new BevelShapeTypesDesignProperty<Bevel>(L"ShapeType", L"Appearance", &Bevel::Shape, &Bevel::SetShape))->SetDefault(bstBox);
        serializer->Find<ColorDesignProperty<Control>>(L"Color")->SetDefault(clBtnFace);
        serializer->Find<BoolDesignProperty<Control>>(L"ParentBackground")->SetDefault(false);
    }

    Size Bevel::DesignSize()
    {
        return Size(80, 35);
    }
#endif

    Bevel::Bevel() : linetype(bltSunken), shape(bstBox), client(Rect(0, 0, 0, 0))
    {
        controlstyle -= csParentBackground;
        SetColor(clBtnFace);
        SetParentColor(true);
    }

    Bevel::~Bevel()
    {
    }

    LRESULT Bevel::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Rect crect;
        WINDOWPOS pos;
        int linew;
        switch (uMsg)
        {
        case WM_WINDOWPOSCHANGED:
            if (shape == bstSpacer || shape == bstTopLine || shape == bstLeftLine)
                break;

            pos = *(WINDOWPOS*)lParam;
            if (
                ((pos.flags & SWP_NOSIZE) == SWP_NOSIZE) || 
                (client.right == pos.cx && client.bottom == pos.cy) || 
                (client.right == pos.cx && 
                    (shape == bstRightLine || shape == bstMiddleVertLine)
                )
                ||
                (client.bottom == pos.cy &&
                    (shape == bstBottomLine || shape == bstMiddleHorzLine)
                )
            
               )
                break;
            linew = linetype == bltRaised || linetype == bltSunken ? 2 : 4;
            if (shape == bstBox)
            {
                InvalidateRect(Rect(client.right - linew, 0, client.right, client.bottom));
                InvalidateRect(Rect(0, client.bottom - linew, client.right, client.bottom));
                InvalidateRect(Rect(pos.cx - linew, 0, pos.cx, pos.cy));
                InvalidateRect(Rect(0, pos.cy - linew, pos.cx, pos.cy));
            }
            else if (shape == bstBottomLine)
            {
                InvalidateRect(Rect(0, client.bottom - linew, client.right, client.bottom));
                InvalidateRect(Rect(0, pos.cy - linew, pos.cx, pos.cy));
            }
            else if (shape == bstRightLine)
            {
                InvalidateRect(Rect(client.right - linew, 0, client.right, client.bottom));
                InvalidateRect(Rect(pos.cx - linew, 0, pos.cx, pos.cy));
            }
            else if (shape == bstMiddleHorzLine)
            {
                InvalidateRect(Rect(0, (client.Height() - linew) / 2, client.right, (client.Height() + linew) / 2 + linew));
                InvalidateRect(Rect(0, (pos.cy - linew) / 2, pos.cx, (pos.cy + linew) / 2 + linew));
            }
            else if (shape == bstMiddleVertLine)
            {
                InvalidateRect(Rect((client.Width() - linew) / 2, 0, (client.Width() + linew) / 2, client.bottom));
                InvalidateRect(Rect((pos.cx - linew) / 2, 0, (pos.cx + linew) / 2, pos.cy));
            }
       

            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    BevelLineTypes Bevel::LineType()
    {
        return linetype;
    }

    void Bevel::SetLineType(BevelLineTypes newlinetype)
    {
        if (linetype == newlinetype)
            return;
        linetype = newlinetype;
        if (IsVisible())
            Invalidate();
    }

    BevelShapeTypes Bevel::Shape()
    {
        return shape;
    }

    void Bevel::SetShape(BevelShapeTypes newshape)
    {
        if (shape == newshape)
            return;
        shape = newshape;
        if (IsVisible())
            Invalidate();
    }

    Rect Bevel::LineRect(Rect r)
    {
        if (shape == bstSpacer)
            return Rect(0, 0, 0, 0);
        int linew = linetype == bltRaised || linetype == bltSunken ? 2 : 4;

        switch (shape)
        {
        case bstLeftLine:
        case bstRightLine:
        case bstMiddleVertLine:
            r.left = shape == bstMiddleVertLine ? (r.Width() - linew) / 2 : shape == bstRightLine ? r.Width() - linew : 0;
            r.right = r.left + linew;
            break;
        case bstTopLine:
        case bstBottomLine:
        case bstMiddleHorzLine:
            r.top = shape == bstMiddleHorzLine ? (r.Height() - linew) / 2 : shape == bstBottomLine ? r.Height() - linew : 0;
            r.bottom  = r.top + linew;
            break;
        case bstBox:
            break;
        default:
            break;
        }

        return r;
    }

    void Bevel::Paint(const Rect &updaterect)
    {
        if (shape == bstSpacer)
            return;

        Canvas *c = GetCanvas();
        client = ClientRect();
        Rect r = LineRect(client);

        //int linew = linetype == bltRaised || linetype == bltSunken ? 2 : 4;

        switch (shape)
        {
        case bstLeftLine:
        case bstRightLine:
        case bstMiddleVertLine:
            if (linetype == bltRaised || linetype == bltSunken)
            {
                c->SelectStockPen(linetype == bltRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top, r.left, r.bottom);
                c->SelectStockPen(linetype == bltSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left + 1, r.top, r.left + 1, r.bottom);
            }
            else if (linetype == bltDoubleSunken || linetype == bltDoubleRaised)
            {
                c->SelectStockPen(linetype == bltDoubleRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top, r.left, r.bottom);
                c->Line(r.left + 1, r.top, r.left + 1, r.bottom);
                c->SelectStockPen(linetype == bltDoubleSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left + 2, r.top, r.left + 2, r.bottom);
                c->Line(r.left + 3, r.top, r.left + 3, r.bottom);
            }
            else
            {
                c->SelectStockPen(linetype == bltRaisedSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top, r.left, r.bottom);
                c->SelectStockPen(linetype == bltSunkenRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left + 1, r.top, r.left + 1, r.bottom);
                c->SelectStockPen(linetype == bltRaisedSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left + 2, r.top, r.left + 2, r.bottom);
                c->SelectStockPen(linetype == bltSunkenRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left + 3, r.top, r.left + 3, r.bottom);
            }
            break;
        case bstTopLine:
        case bstBottomLine:
        case bstMiddleHorzLine:
            if (linetype == bltRaised || linetype == bltSunken)
            {
                c->SelectStockPen(linetype == bltRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top, r.right, r.top);
                c->SelectStockPen(linetype == bltSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top + 1, r.right, r.top + 1);
            }
            else if (linetype == bltDoubleSunken || linetype == bltDoubleRaised)
            {
                c->SelectStockPen(linetype == bltDoubleRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top, r.right, r.top);
                c->Line(r.left, r.top + 1, r.right, r.top + 1);
                c->SelectStockPen(linetype == bltDoubleSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top + 2, r.right, r.top + 2);
                c->Line(r.left, r.top + 3, r.right, r.top + 3);
            }
            else
            {
                c->SelectStockPen(linetype == bltRaisedSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top, r.right, r.top);
                c->SelectStockPen(linetype == bltSunkenRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top + 1, r.right, r.top + 1);
                c->SelectStockPen(linetype == bltRaisedSunken ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top + 2, r.right, r.top + 2);
                c->SelectStockPen(linetype == bltSunkenRaised ? sp3DHighlight : sp3DShadow);
                c->Line(r.left, r.top + 3, r.right, r.top + 3);
            }
            break;
        case bstBox:
            do
            {
                c->DrawFrame(r, (linetype == bltRaised || linetype == bltDoubleRaised || (linetype == (r.left == 0 ? bltRaisedSunken : bltSunkenRaised))));

                InflateRect(&r,-1,-1);
                if (r.left > 1 || linetype == bltSunken || linetype == bltRaised)
                    break;
            } while(true);
            break;
        default:
            break;
        }
    }


}
/* End of NLIBNS */


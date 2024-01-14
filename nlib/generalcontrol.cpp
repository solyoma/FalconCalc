#include "stdafx_zoli.h"
#include "application.h"
#include "generalcontrol.h"
#include "dialog.h"
#ifdef DESIGNING
#include "serializer.h"
#include "designproperties.h"
#include "property_generalcontrol.h"
#include "property_menu.h"
#include "property_controlbase.h"
#include "designer.h"

//#include <array>
#endif


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING

    ValuePair<FormBorderStyles> FormBorderStyleStrings[] = {
        VALUEPAIR(fbsNormal),
        VALUEPAIR(fbsNone),
        VALUEPAIR(fbsSingle),
        VALUEPAIR(fbsDialog),
        VALUEPAIR(fbsToolWindow),
        VALUEPAIR(fbsSizeableToolWindow),
    };

    ValuePair<FormBorderButtons> FormBorderButtonStrings[] = {
        VALUEPAIR(fbbSystemMenu),
        VALUEPAIR(fbbMinimizeBox),
        VALUEPAIR(fbbMaximizeBox),
        VALUEPAIR(fbbContextHelp),
    };

    ValuePair<PanelBorderStyles> PanelBorderStyleStrings[] = {
        VALUEPAIR(pbsNone),
        VALUEPAIR(pbsDoubleRaised),
        VALUEPAIR(pbsDoubleSunken),
        VALUEPAIR(pbsRaised),
        VALUEPAIR(pbsRaisedSunken),
        VALUEPAIR(pbsSunken),
        VALUEPAIR(pbsSunkenRaised),
    };

    ValuePair<FormShowPositions> FormShowPositionStrings[] = {
        VALUEPAIR(fspActiveMonitor),
        VALUEPAIR(fspActiveMonitorCenter),
        VALUEPAIR(fspDefault),
        VALUEPAIR(fspMainFormMonitor),
        VALUEPAIR(fspMainFormMonitorCenter),
        VALUEPAIR(fspPrimaryMonitor),
        VALUEPAIR(fspPrimaryMonitorCenter),
        VALUEPAIR(fspParentMonitor),
        VALUEPAIR(fspParentMonitorCenter),
        VALUEPAIR(fspParentWindowCenter),
        VALUEPAIR(fspUnchanged),
    };


    //---------------------------------------------


    void Panel::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->SetContainerControl(true);
        serializer->Find<BoolDesignProperty<Control>>(L"AcceptInput")->SetDefault(false);
        serializer->Find<BoolDesignProperty<Control>>(L"ParentBackground")->SetDefault(false);
        serializer->Add(L"SetShowText", new BoolDesignProperty<Panel>(L"ShowText", L"Appearance", &Panel::ShowText, &Panel::SetShowText))->SetDefault(true);
        serializer->Add(L"SetInnerBorderStyle", new PanelBorderStylesDesignProperty<Panel>(L"InnerBorderStyle", L"Appearance", &Panel::InnerBorderStyle, &Panel::SetInnerBorderStyle))->SetDefault(pbsRaised);
        serializer->Add(L"SetTextAlignment", new TextAlignmentsDesignProperty<Panel>(L"TextAlignment", L"Appearance", &Panel::TextAlignment, &Panel::SetTextAlignment))->SetDefault(taCenter);
        serializer->Add(L"SetVerticalTextAlignment", new VerticalTextAlignmentsDesignProperty<Panel>(L"VerticalTextAlignment", L"Appearance", &Panel::VerticalTextAlignment, &Panel::SetVerticalTextAlignment))->SetDefault(vtaMiddle);
        serializer->Add(L"SetTextMargin", new IntDesignProperty<Panel>(L"TextMargin", L"Appearance", &Panel::TextMargin, &Panel::SetTextMargin))->SetDefault(4);
        serializer->Add(L"SetVerticalTextMargin", new IntDesignProperty<Panel>(L"VerticalTextMargin", L"Appearance", &Panel::VerticalTextMargin, &Panel::SetVerticalTextMargin))->SetDefault(4);
    }

    Color Panel::DefaultColor()
    {
        if (ParentBackground())
            return Parent()->GetColor();
        return clBtnFace;
    }
#endif

    Panel::Panel() : borderstyle(pbsRaised), showtext(true), halign(taCenter), valign(vtaMiddle), margin(4), vmargin(4)
    {
        controlstyle << csInTabOrder;
        //controlstyle -= csUpdateOnTextChange;
        controlstyle -= csAcceptInput;
        controlstyle -= csParentBackground;
        SetColor(clBtnFace);
        SetParentColor(true);

        int bw = BorderWidth();

        InitControlList();
        SetInnerPadding(Rect(bw, bw, bw, bw));
    }

    Panel::~Panel()
    {
    }

    void Panel::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
    }

    void Panel::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
    }

    LRESULT Panel::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Rect oldrect,newrect;
        int bw;

        switch (uMsg)
        {
        case WM_WINDOWPOSCHANGED:
            if (ParentBackground())
                break;

            oldrect = OldClientRectangle();
            newrect = ClientRect();
            bw = BorderWidth();

            if (newrect.right < oldrect.right)
                InvalidateRect(Rect(newrect.right - bw, 0, newrect.right, newrect.bottom), false);
            else if (newrect.right > oldrect.right)
                InvalidateRect(Rect(oldrect.right - bw, 0, newrect.right, newrect.bottom), true);
            if (newrect.bottom < oldrect.bottom)
                InvalidateRect(Rect(0, newrect.bottom - bw, newrect.right, newrect.bottom), false);
            else if (newrect.bottom > oldrect.bottom)
                InvalidateRect(Rect(0, oldrect.bottom - bw, newrect.right, newrect.bottom), true);

            if (showtext && ((halign != taLeft && oldrect.Width() != newrect.Width()) || (valign != vtaTop && oldrect.Height() != newrect.Height())))
            {
                std::wstring t = Text();
                if (!t.empty())
                {
                    //Canvas *c = GetCanvas();
                    //Size s = c->MeasureText(t);

                    //oldrect.Expand(-bw);
                    //newrect.Expand(-bw);

                    //InvalidateRect(RectS(oldrect.left + (oldrect.Width() - s.cx) / 2, oldrect.top + (oldrect.Height() - s.cy) / 2, s.cx, s.cy), true);
                    //InvalidateRect(RectS(newrect.left + (newrect.Width() - s.cx) / 2, newrect.top + (newrect.Height() - s.cy) / 2, s.cx, s.cy), true);
                    InvalidateRect(TextRect(oldrect, t, halign, valign, margin, vmargin));
                    InvalidateRect(TextRect(newrect, t, halign, valign, margin, vmargin));
                }
            }
            break;
        }

        return base::WindowProc(uMsg, wParam, lParam);
    }

    void Panel::Resizing()
    {
        //if (ParentBackground())
        //    RedrawWindow(Handle(), NULL, 0, RDW_INVALIDATE | RDW_ERASE);
        base::Resizing();
    }

    void Panel::Moving()
    {
        //if (ParentBackground())
        //    RedrawWindow(Handle(), NULL, 0, RDW_INVALIDATE | RDW_ERASE);
        base::Moving();
    }

    TextAlignments Panel::TextAlignment()
    {
        return halign;
    }

    void Panel::SetTextAlignment(TextAlignments newalignment)
    {
        if (halign == newalignment)
            return;
        if (showtext)
        {
            std::wstring t = Text();
            if (HandleCreated() && !t.empty())
            {
                Rect cr = ClientRect();
                InvalidateRect(TextRect(cr, t, halign, valign, margin, vmargin));
                InvalidateRect(TextRect(cr, t, newalignment, valign, margin, vmargin));
            }
        }
        halign = newalignment;
    }

    VerticalTextAlignments Panel::VerticalTextAlignment()
    {
        return valign;
    }

    void Panel::SetVerticalTextAlignment(VerticalTextAlignments newalignment)
    {
        if (valign == newalignment)
            return;
        if (showtext)
        {
            std::wstring t = Text();
            if (!t.empty())
            {
                Rect cr = ClientRect();
                InvalidateRect(TextRect(cr, t, halign, valign, margin, vmargin));
                InvalidateRect(TextRect(cr, t, halign, newalignment, margin, vmargin));
            }
        }
        valign = newalignment;
    }

    int Panel::TextMargin()
    {
        return margin;
    }

    void Panel::SetTextMargin(int newmargin)
    {
        if (margin == newmargin)
            return;
        if (showtext && (halign == taLeft || halign == taRight))
        {
            std::wstring t = Text();
            if (!t.empty())
            {
                Rect cr = ClientRect();
                InvalidateRect(TextRect(cr, t, halign, valign, margin, vmargin));
                InvalidateRect(TextRect(cr, t, halign, valign, newmargin, vmargin));
            }
        }
        margin = newmargin;
    }

    int Panel::VerticalTextMargin()
    {
        return vmargin;
    }

    void Panel::SetVerticalTextMargin(int newmargin)
    {
        if (vmargin == newmargin)
            return;
        if (showtext && (valign == vtaTop || valign == vtaBottom))
        {
            std::wstring t = Text();
            if (!t.empty())
            {
                Rect cr = ClientRect();
                InvalidateRect(TextRect(cr, t, halign, valign, margin, vmargin));
                InvalidateRect(TextRect(cr, t, halign, valign, margin, newmargin));
            }
        }
        vmargin = newmargin;
    }

    Rect Panel::TextRect(const Rect& cr, const std::wstring &str, TextAlignments h, VerticalTextAlignments v, int hmarg, int vmarg)
    {
        Canvas *c = GetCanvas();
        Size s = c->MeasureText(str);
        int bw = BorderWidth();
        Rect r = cr.Inflate(-bw);

        int x, y;
        switch(h)
        {
        case taLeft:
            x = r.left + hmarg;
            break;
        case taRight:
            x = r.right - s.cx - hmarg;
            break;
        case taCenter:
            x = r.left + (r.Width() - s.cx) / 2;
            break;
        default:
            x = 0;
            break;
        }
        switch(v)
        {
        case vtaTop:
            y = r.top + vmarg;
            break;
        case vtaBottom:
            y = r.bottom - s.cy - vmarg;
            break;
        case vtaMiddle:
            y = r.top + (r.Height() - s.cy) / 2;
            break;
        default:
            y = 0;
            break;
        }

        return RectS(x, y, s.cx, s.cy);
    }

    bool Panel::TextChanging(const std::wstring &newtext)
    {
        if (!showtext)
            return true;

        std::wstring oldtext = Text();
        if (oldtext != newtext)
        {
            Rect r = ClientRect();
            //int bw = BorderWidth();

            //Canvas *c = GetCanvas();

            //Size s1 = c->MeasureText(oldtext);
            //Size s2 = c->MeasureText(newtext);

            //Size s(max(s1.cx, s2.cx), max(s1.cy, s2.cy));

            //InflateRect(&r,-bw,-bw);

            //InvalidateRect(RectS(r.left+(r.Width()-s.cx) / 2, r.top+(r.Height()-s.cy) / 2,s.cx,s.cy));

            Rect r1 = TextRect(r, oldtext, halign, valign, margin, vmargin);
            Rect r2 = TextRect(r, newtext, halign, valign, margin, vmargin);
            InvalidateRect(Rect(min(r1.left, r2.left), min(r1.top, r2.top), max(r1.right, r2.right), max(r1.bottom, r2.bottom)));

        }

        return true;
    }

    bool Panel::ExcludeOpaqueRegion(HRGN rgn, const Rect &rgnrect, const Point &origin)
    {
        int bw = BorderWidth();
        if (!bw)
            return true;

        Rect cr = ClientRect().Offset(origin);
        Rect r;
        r = Rect(cr.left, cr.top, cr.left + cr.Width(), cr.top + bw).Intersect(rgnrect);
        if (!r.Empty())
            CombineRgnWithRect(rgn, rgn, r, rcmDiff);
        r = Rect(cr.left, cr.top + cr.Height() - bw, cr.left + cr.Width(), cr.top + cr.Height()).Intersect(rgnrect);
        if (!r.Empty())
            CombineRgnWithRect(rgn, rgn, r, rcmDiff);
        r = Rect(cr.left, cr.top + bw, cr.left + bw, cr.top + cr.Height() - bw).Intersect(rgnrect);
        if (!r.Empty())
            CombineRgnWithRect(rgn, rgn, r, rcmDiff);
        r = Rect(cr.left + cr.Width() - bw, cr.top + bw, cr.left + cr.Width(), cr.top + cr.Height() - bw).Intersect(rgnrect);
        if (!r.Empty())
            CombineRgnWithRect(rgn, rgn, r, rcmDiff);

        return true;
    }

    void Panel::Paint(const Rect &updaterect)
    {
        Canvas *c = GetCanvas();
        int bw = BorderWidth();
        Rect r = ClientRect();//.Inflate(-bw);

        if (r.Width() > 0 && r.Height() > 0)
        {
            if (showtext)
            {
                std::wstring t = Text();
                Rect tr = TextRect(r, t, halign, valign, margin, vmargin);
                c->TextDraw(r.Inflate(-bw), tr.left, tr.top, t);
                //Size s = c->MeasureText(t);
                //c->TextDraw(r, r.left + (r.Width() - s.cx) / 2, r.top + (r.Height() - s.cy) / 2, t);
            }
        }

        if (borderstyle == pbsNone)
            return;

        //r.Expand(bw);

        do
        {
            c->DrawFrame(r, (borderstyle == pbsRaised || borderstyle == pbsDoubleRaised || (borderstyle == (r.left == 0 ? pbsRaisedSunken : pbsSunkenRaised))));

            r.Expand(-1);
            if (r.left > 1 || borderstyle == pbsSunken || borderstyle == pbsRaised)
                break;
        } while(true);

    }


    int Panel::BorderWidth()
    {
        switch(borderstyle) //pbsNone, pbsSunken, pbsRaised, pbsDoubleSunken, pbsDoubleRaised, pbsSunkenRaised, pbsRaisedSunken
        {
            case pbsNone:
                return 0;
            case pbsSunken:
            case pbsRaised:
                return 1;
            default:
                return 2;
        }
    }

    PanelBorderStyles Panel::InnerBorderStyle()
    {
        return borderstyle;
    }

    void Panel::SetInnerBorderStyle(PanelBorderStyles newborder)
    {
        if (newborder == borderstyle)
            return;
        borderstyle = newborder;

        int bw = BorderWidth();
        SetInnerPadding(Rect(bw, bw, bw, bw));

        if (IsVisible())
            Invalidate();
    }

    bool Panel::ShowText()
    {
        return showtext;
    }

    void Panel::SetShowText(bool newshowtext)
    {
        if (showtext == newshowtext)
            return;

        showtext = newshowtext;

        if (showtext)
            controlstyle << csUpdateOnTextChange;
        else
            controlstyle -= csUpdateOnTextChange;

        if (!IsVisible())
            return;

        std::wstring t = Text();
        if (!t.empty())
            InvalidateRect(TextRect(ClientRect(), t, halign, valign, margin, vmargin));

    }


    //---------------------------------------------


#ifdef DESIGNING
    IconData::IconData() : filedata(NULL), filesize(0)
    {
    }

    IconData::IconData(const IconData &orig) : filedata(NULL), filesize(0)
    {
        *this = orig;
    }

    IconData::IconData(IconData &&orig) noexcept : filedata(NULL), filesize(0)
    {
        *this = std::move(orig);
    }

    IconData::~IconData()
    {
        Clear();
    }

    void IconData::Clear()
    {
        delete[] filedata;
        filedata = 0;
        filesize = 0;
        filename.clear();
    }

    bool IconData::operator==(const IconData &other)
    {
        return filename.empty() && other.filename.empty() && filedata == NULL && other.filedata == NULL && filesize == 0 && other.filesize == 0;
    }

    IconData& IconData::operator=(const IconData &orig)
    {
        delete[] filedata;
        filesize = orig.filesize;
        filename = orig.filename;
        if (filesize)
        {
            filedata = new char[filesize];
            memcpy(filedata, orig.filedata, filesize);
        }
        else
            filedata = 0;

        return *this;
    }

    IconData& IconData::operator=(IconData &&orig) noexcept
    {
        std::swap(filedata, orig.filedata);
        std::swap(filesize, orig.filesize);
        std::swap(filename, orig.filename);

        return *this;
    }


    //---------------------------------------------


#endif
#ifdef DESIGNING
    Size Form::DesignSize()
    {
        return Size(480, 320);
    }

    void Form::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->SetContainerControl(true);

        serializer->HideProperty(L"ParentBackground");
        serializer->HideProperty(L"ParentColor");
        serializer->HideProperty(L"Alignment");
        serializer->HideProperty(L"AlignMargin");
        serializer->HideProperty(L"Anchors");
        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"TabOrder");
        serializer->HideProperty(L"AccessLevel");
        serializer->HideProperty(L"ParentFont");
        serializer->HideProperty(L"ParentTooltip");

        serializer->Find<IntDesignProperty<Control>>(L"Left")->DoExport();
        serializer->Find<IntDesignProperty<Control>>(L"Top")->DoExport();
        serializer->Find<IntDesignProperty<Control>>(L"Width")->DontSerialize();
        serializer->Find<IntDesignProperty<Control>>(L"Height")->DontSerialize();
        serializer->Find<RectDesignProperty<Control>>(L"Bounds")->DontExport();
        serializer->Find<BoolDesignProperty<Control>>(L"Visible")->SetDefault(false);


        serializer->Add(L"SetActiveControl", new ActiveControlDesignProperty<Form>(L"ActiveControl", L"Controls", &Form::DesignActiveControl, &Form::DesignSetActiveControl))->SetDefault(nullptr);
        serializer->Add(L"SetMinimumWidth", new IntDesignProperty<Form>(L"MinimumWidth", L"Dimensions", &Form::MinimumWidth, &Form::SetMinimumWidth))->SetDefault(0);
        serializer->Add(L"SetMinimumHeight", new IntDesignProperty<Form>(L"MinimumHeight", L"Dimensions", &Form::MinimumHeight, &Form::SetMinimumHeight))->SetDefault(0);
        serializer->Add(L"SetMaximumWidth", new IntDesignProperty<Form>(L"MaximumWidth", L"Dimensions", &Form::MaximumWidth, &Form::SetMaximumWidth))->SetDefault(0);
        serializer->Add(L"SetMaximumHeight", new IntDesignProperty<Form>(L"MaximumHeight", L"Dimensions", &Form::MaximumHeight, &Form::SetMaximumHeight))->SetDefault(0);
        serializer->Add(L"SetShowPosition", new FormShowPositionsDesignProperty<Form>(L"ShowPosition", L"Behavior", &Form::DesignShowPosition, &Form::DesignSetShowPosition))->SetDefault(fspActiveMonitor);
        serializer->Add(L"SetTopmost", new BoolDesignProperty<Form>(L"Topmost", L"Behavior", &Form::DesignTopmost, &Form::DesignSetTopmost))->SetDefault(false);
        serializer->Add(L"SetBorderStyle", new FormBorderStylesDesignProperty<Form>(L"BorderStyle", L"Border", &Form::BorderStyle, &Form::SetBorderStyle))->SetDefault(fbsNormal);
        serializer->Add(L"SetBorderButtons", new FormBorderButtonSetDesignProperty<Form>(L"BorderButtons", L"Border", &Form::BorderButtons, &Form::SetBorderButtons))->SetDefault(fbbSystemMenu | fbbMinimizeBox | fbbMaximizeBox);
        serializer->Add(L"SetKeyPreview", new BoolDesignProperty<Form>(L"KeyPreview", L"Behavior", &Form::KeyPreview, &Form::SetKeyPreview))->SetDefault(false);

        serializer->Find<ColorDesignProperty<Control>>(L"Color")->SetDefault(clBtnFace);

        serializer->AddEvent<Form, FormCloseEvent>(L"OnClose");
        serializer->AddEvent<Form, ActiveFormChangeEvent>(L"OnActiveFormChange");
        serializer->AddEvent<Form, FormActivateEvent>(L"OnActivate");
        serializer->AddEvent<Form, FormActivateEvent>(L"OnDeactivate");
    }

    std::wstring Form::ClassName(bool namespacedname)
    {
        return designer->FormClassName(Name());
    }

    FormShowPositions Form::DesignShowPosition()
    {
        return designformpos;
    }

    void Form::DesignSetShowPosition(FormShowPositions newformpos)
    {
        designformpos = newformpos;
    }

    bool Form::DesignTopmost()
    {
        return designtopmost;
    }

    void Form::DesignSetTopmost(bool newtopmost)
    {
        designtopmost = newtopmost;
    }

    Form::Form() : base(), client(-1, -1), designformpos(fspActiveMonitor), designactivecontrol(NULL), designtopmost(false),
#else
    Form::Form() :
#endif
            mainform(false), menu(NULL), smallicon(NULL), largeicon(NULL), ownsmallicon(false), ownlargeicon(false), posinited(false),
            formstate(fsNormal), formpos(fspActiveMonitor), formborder(fbsNormal), borderbuttons(fbbSystemMenu | fbbMinimizeBox | fbbMaximizeBox),
            topmost(false), keypreview(false), modal(false), dlgmode(dmDefault), modalresult(mrNone), activecontrol(NULL), activating(false)
    {
        if (application->MainForm() == NULL)
            application->SetMainForm(this);
        Hide();

#ifdef DESIGNING
        DesignSetVisible(false);
#endif

        SetBounds(Rect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT));
        controlstyle << csInTabOrder;
        controlstyle -= csChild;
        controlstyle -= csParentBackground;
        controlstyle -= csUpdateOnTextChange;
        controlstyle -= csAcceptInput;
        SetColor(clBtnFace);
        InitControlList();

        application->addform(this);
    }

    Form::~Form()
    {
    }

    void Form::Destroy()
    {
        if (Visible())
            Hide();
        while (nvs.size())
        {
            auto c = nvs.front();
            c->Destroy();
            if (!nvs.empty() && c == nvs.front())
                nvs.erase(nvs.begin());
        }
        application->formdestroyed(this);
        base::Destroy();
    }

    void Form::AddNVChild(NonVisualControl *nv)
    {
        if (AddToNotifyList(nv, nrOwnership))
            nvs.push_back(nv);
    }

    void Form::RemoveNVChild(NonVisualControl *nv)
    {
        RemoveFromNotifyList(nv, nrOwnership);
        for (auto it = nvs.begin(); it != nvs.end(); ++it)
        {
            if (*it == nv)
            {
                nvs.erase(it);
                break;
            }
        }
    }

    FormShowPositions Form::ShowPosition()
    {
        return formpos;
    }

    void Form::SetShowPosition(FormShowPositions newformpos)
    {
        formpos = newformpos;
    }

    void Form::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
    }

    void Form::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        params.style << wsOverlappedWindow;

        params.style -= wsBorder;
        params.style -= wsThickFrame;
        params.style -= wsDlgFrame;
        params.style -= wsCaption;
        params.style -= wsSysMenu;
        params.style -= wsMinimizeBox;
        params.style -= wsMaximizeBox;


        params.extstyle -= wsExClientEdge;
        params.extstyle -= wsExDlgModalFrame;
        params.extstyle -= wsExStaticEdge;
        params.extstyle -= wsExWindowEdge;
        params.extstyle -= wsExToolWindow;
        params.extstyle -= wsExContextHelp;

#ifdef DESIGNING
        if (Designing())
            params.style << wsCaption << wsThickFrame << wsSysMenu << wsMinimizeBox << wsMaximizeBox;
        else
        {
#endif
            switch(formborder)
            {
            case fbsNone:
                params.style << wsPopup;
                break;
            case fbsNormal:
                params.style << wsCaption << wsThickFrame;
                break;
            case fbsSingle:
                params.style << wsCaption << wsBorder;
                break;
            case fbsDialog:
                params.style << wsCaption;
                params.extstyle << wsExDlgModalFrame << wsExWindowEdge;
                break;
            case fbsToolWindow:
                params.style << wsCaption << wsBorder;
                params.extstyle << wsExToolWindow;
                break;
            case fbsSizeableToolWindow:
                params.style << wsCaption << wsThickFrame;
                params.extstyle << wsExToolWindow;
                break;
            default:
                break;
            }
#ifdef DESIGNING
        }

        if (!Designing())
        {
#endif

            if (formborder != fbsNone)
            {
                if (borderbuttons.contains(fbbSystemMenu))
                    params.style << wsSysMenu;
                if (borderbuttons.contains(fbbMinimizeBox))
                    params.style << wsMinimizeBox;
                if (borderbuttons.contains(fbbMaximizeBox))
                    params.style << wsMaximizeBox;
                if (borderbuttons.contains(fbbContextHelp))
                    params.extstyle << wsExContextHelp;
            }

#ifdef DESIGNING
        }
#endif
        if (params.style.contains(wsChild))
            posinited = true;

        if (!topmost && TopLevelParent())
        {
            Form *f = dynamic_cast<Form*>(TopLevelParent());

#ifdef DESIGNING
          if (!Designing())
#endif
            if (f && f->Topmost())
                topmost = true;
        }

        if (topmost)
            params.extstyle << wsExTopmost;

        if (!posinited)
        {
            initrect = WindowRect();
            DisplayMonitor *m = screen->MonitorFromWindow(this);
            OffsetRect(&initrect, (UINT)initrect.left == CW_USEDEFAULT ? 0 : -m->WorkArea().left, (UINT)initrect.top == CW_USEDEFAULT ? 0 : -m->WorkArea().top);

            initshow = Visible();
            params.style -= wsVisible;

            params.x = CW_USEDEFAULT;
            params.y = CW_USEDEFAULT;
            params.width = CW_USEDEFAULT;
            params.height = CW_USEDEFAULT;

            params.style -= wsMinimize;
            params.style -= wsMaximize;
        }

        if (menu)
            params.menu = menu->Handle();
    }

    void Form::InitHandle()
    {
        base::InitHandle();

        if (largeicon != NULL)
            PassMessage(WM_SETICON, ICON_BIG, (LPARAM)largeicon->Handle());
        if (smallicon != NULL)
            PassMessage(WM_SETICON, ICON_SMALL, (LPARAM)smallicon->Handle());

#ifdef DESIGNING
        client = Size(-1, -1);
#endif

        if (topmost)
            SetWindowPos(Handle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        else
            SetWindowPos(Handle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        if (!posinited)
        {
            Rect r;

            //HWND fwnd = GetForegroundWindow();
            DisplayMonitor *m = screen->MonitorFromWindow(this);
            if (!m)
            {
                screen->UpdateDisplays();
                m = screen->MonitorFromWindow(this);
            }

            if (formpos == fspDefault)
                initrect = WindowRect();
            else if ((UINT)initrect.left == CW_USEDEFAULT || (UINT)initrect.top == CW_USEDEFAULT || (UINT)initrect.right == CW_USEDEFAULT || (UINT)initrect.bottom == CW_USEDEFAULT)
            {
                r = WindowRect();
                initrect = RectS((UINT)initrect.left == CW_USEDEFAULT ? r.left : m->WorkArea().left + initrect.left, (UINT)initrect.top == CW_USEDEFAULT ? r.top : m->WorkArea().top + initrect.top,
                                 (UINT)initrect.right == CW_USEDEFAULT ? r.Width() : (UINT)initrect.left == CW_USEDEFAULT ? initrect.right : initrect.Width(),
                                 (UINT)initrect.bottom == CW_USEDEFAULT ? r.Height() : (UINT)initrect.top == CW_USEDEFAULT ? initrect.bottom : initrect.Height());
            }
            else
                initrect = RectS(m->WorkArea().left + initrect.left, m->WorkArea().top + initrect.top, initrect.Width(), initrect.Height());

            // Because the window's size is at the system default values and not as designed, the alignments and anchors are all useless.
            // We first have to restore the window's original size before allowing LayoutChildren to be called.
            SetAllowLayout(false);
            SetWindowPos(Handle(), NULL, initrect.left, initrect.top, initrect.Width(), initrect.Height(), SWP_BOUNDS);
            SetAllowLayout(true);

            posinited = true;

            if (initshow)
                Show();
        }
    }

    void Form::SaveWindow()
    {
        SetMenu(NULL);
        Topmost();
        base::SaveWindow();
    }

    void Form::Show()
    {
        if (controlstate.contains(csVisible) && HandleCreated())
            return;

        if (controlstyle.contains(csChild))
        {
            base::Show();
            return;
        }

        if (!HandleCreated())
            CreateHandle();
        MoveToShowPosition();

        controlstate << csVisible;

        if (formstate == fsMinimized)
            ShowWindow(Handle(), SW_SHOWMINIMIZED);
        else if (formstate == fsMaximized)
            ShowWindow(Handle(), SW_SHOWMAXIMIZED);
        else
            ShowWindow(Handle(), controlstyle.contains(csShowDontActivate) ? SW_SHOWNOACTIVATE : SW_SHOWNORMAL);
    }

    ModalResults Form::ShowModal()
    {
        CloseActions closeaction = caPreventClose;

        modal = true;

        HWND active = application->ActiveForm() ? application->ActiveForm()->Handle() : TopLevelParent() ? TopLevelParent()->Handle() : NULL; // Remember which form activated this one so it can be restored.

        dialog = new Dialog();

        try
        {
            dialog->SetDialogMode(dlgmode);
            dialog->DisableForms(TopLevelParent());
            Show();
            modalresult = mrNone;
            while (closeaction == caPreventClose && Visible())
            {
                if (!application->HandleOneMessage())
                {
                    if (application->Terminated())
                    {
                        modalresult = mrCancel;
                        break;
                    }
                    else
                        WaitMessage();
                }
                else if (modalresult != mrNone)
                {
                    closeaction = application->MainForm() == this ? caDestroyHandle : caHide;
                    if (OnClose)
                        OnClose(this, FormCloseParameters(closeaction));
                    if (closeaction == caPreventClose)
                        modalresult = mrNone;
                }
            }

        }
        catch(...)
        {
            dialog->EnableForms();
            modal = false;
            dialog->Destroy();
            throw;
        }
        dialog->EnableForms();
        modal = false;
        if (!active || !IsWindow(active))
        {
            if (TopLevelParent())
                active = TopLevelParent()->Handle();
            else
                active = NULL;
        }

        if (active)
            SetActiveWindow(active);

        dialog->Destroy();

        switch (closeaction)
        {
        case caHide:
            Hide();
#ifdef DESIGNING
            // Form hiding is prevented when in designing mode, so let's hide our window here directly.
            if (closeaction == caHide && Designing())
                ShowWindow(Handle(), SW_HIDE);
#endif
            break;
        case caDeleteForm:
            ShowWindow(Handle(), SW_HIDE);
            PostMessage(Handle(), wmDelete, 0, 0);
            break;
        case caDestroyHandle:
            DestroyWindow(Handle());
            break;
        default:
            break;
        }

        return modalresult;
    }

    ModalResults Form::ModalResult()
    {
        return modalresult;
    }

    void Form::SetModalResult(ModalResults newmodalresult)
    {
        modalresult = newmodalresult;
    }

    DialogModes Form::DialogMode()
    {
        return dlgmode;
    }

    void Form::SetDialogMode(DialogModes newdialogmode)
    {
        dlgmode = newdialogmode;
    }

    bool Form::Topmost()
    {
        if (HandleCreated())
            topmost = (GetWindowLongPtr(Handle(), GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST;
        return topmost;
    }

    void Form::SetTopmost(bool newtopmost)
    {
        if (newtopmost == topmost)
            return;

        if (!newtopmost)
        {
            if (dynamic_cast<Form*>(TopLevelParent()))
                ((Form*)TopLevelParent())->SetTopmost(false);
            else if (TopLevelParent() && TopLevelParent()->HandleCreated())
                SetWindowPos(TopLevelParent()->Handle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }

        topmost = newtopmost;

        if (HandleCreated())
        {
            if (topmost)
                SetWindowPos(Handle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            else
                SetWindowPos(Handle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }

        if (newtopmost)
        {
            for (int ix = TopChildCount() - 1; ix >= 0; --ix)
            {
                Control *c = TopChild(ix);
                if (dynamic_cast<Form*>(c) != NULL)
                    ((Form*)c)->SetTopmost(true);
                if (c->HandleCreated())
                    SetWindowPos(Handle(), c->Handle(), 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
        }
    }

    void Form::NoTopmostRecursive()
    {
        if (topmost)
            SetTopmost(false);
        for (int ix = TopChildCount() - 1; ix >= 0; --ix)
        {
            Control *c = TopChild(ix);
            if (dynamic_cast<Form*>(c) != NULL)
                ((Form*)c)->NoTopmostRecursive();
            if (c->HandleCreated())
                SetWindowPos(c->Handle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }

    void Form::MoveToShowPosition()
    {
        if (formpos == fspUnchanged || formpos == fspDefault)
            return;

        if (((formpos == fspMainFormMonitor || formpos == fspMainFormMonitorCenter) && (!application->MainForm() || application->MainForm() == this)) ||
            ((formpos == fspParentMonitor || formpos == fspParentWindowCenter || formpos == fspParentMonitorCenter) && (TopLevelParent() == NULL || !TopLevelParent()->Visible())))
            formpos = (formpos == fspMainFormMonitor || formpos == fspParentMonitor || formpos == fspParentWindowCenter ? fspActiveMonitor : fspActiveMonitorCenter);

        Rect r = WindowRect();
        if (formpos == fspParentWindowCenter)
        {
            Rect pr = TopLevelParent()->WindowRect();
            SetAllowLayout(false);
            SetWindowPos(Handle(),NULL,pr.left - (r.Width() - pr.Width()) / 2,pr.top - (r.Height() - pr.Height()) / 2,0,0,SWP_POSITIONONLY);
            SetAllowLayout(true);
            return;
        }


        DisplayMonitor *m = NULL;
        Form *f;
        switch (formpos)
        {
        case fspActiveMonitor:
        case fspActiveMonitorCenter:
            m = screen->MonitorFromWindow(GetForegroundWindow());
            if (!m)
                m = screen->MonitorFromPoint(screencursor->Pos());
            if (m)
                break;
        case fspPrimaryMonitor:
        case fspPrimaryMonitorCenter:
            m = screen->PrimaryMonitor();
            break;
        case fspMainFormMonitor:
        case fspMainFormMonitorCenter:
            f = application->MainForm();
            if (f != nullptr && f->IsVisible())
                m = screen->MonitorFromWindow(f);
            else
                m = screen->MonitorFromRect(f->WindowRect());
            break;
        case fspParentMonitor:
        case fspParentMonitorCenter:
            m = screen->MonitorFromWindow(application->MainForm());
            break;
        default:
            break;
        }

        DisplayMonitor *oldm = screen->MonitorFromHandle(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST));
        if (!oldm)
        {
            screen->UpdateDisplays();
            oldm = screen->MonitorFromHandle(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST));
        }

        if (!m || !oldm)
            return; // For some reason we couldn't get the monitor to reposition the form.

        OffsetRect(&r, -oldm->WorkArea().left, -m->WorkArea().top);
        if (formpos == fspActiveMonitor || formpos == fspPrimaryMonitor || formpos == fspMainFormMonitor || formpos == fspParentMonitor)
            OffsetRect(&r, m->WorkArea().left, m->WorkArea().top);
        else // Center
            r = RectS(m->WorkArea().left + (m->WorkArea().Width() - r.Width()) / 2, m->WorkArea().top + (m->WorkArea().Height() - r.Height()) / 2, r.Width(), r.Height());

        SetAllowLayout(false);
        if (HandleCreated())
            SetWindowPos(Handle(), NULL, r.left, r.top, 0, 0, SWP_POSITIONONLY);
        else
            SetBounds(r);
        SetAllowLayout(true);

        return;
    }

    LRESULT Form::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CloseActions closeaction;
        Control *c;
        Form *f;
        bool b;

        switch (uMsg)
        {
        case WM_CLOSE:
            if (modal)
            {
                if (modalresult == mrNone)
                    modalresult = mrCancel;
                return 0;
            }

            closeaction = application->MainForm() == this ? caDestroyHandle : caHide;

            if (OnClose)
                OnClose(this, FormCloseParameters(closeaction));

            if (closeaction == caPreventClose)
                return 0;

            if (closeaction == caHide)
            {
                Hide();
#ifdef DESIGNING
                // Form hiding is prevented when in designing mode, so let's hide our window here directly.
                if (closeaction == caHide && Designing())
                    ShowWindow(Handle(), SW_HIDE);
#endif
            }

            if (closeaction == caDeleteForm || (closeaction == caDestroyHandle && application->MainForm() == this))
            {
                ShowWindow(Handle(), SW_HIDE);
                PostMessage(Handle(), wmDelete, 0, 0);
                return 0;
            }

            if (closeaction != caDestroyHandle)
                return 0;

            break;
        case WM_DESTROY:
            b = mainform;
            if (mainform)
            {
                application->SetMainForm(NULL);
                PostMessage(application->Handle(), WM_CLOSE, 0, 0);
            }
            if (application->ActiveForm() == this)
                SendMessage(application->Handle(), wmFormActivated, 0, b ? 1 : 0);
            break;
        case WM_SHOWWINDOW:
            if (lParam != 0)
                break;
            if (wParam == FALSE && (application->ActiveForm() == this) && application->Active())
                SendMessage(application->Handle(), wmFormActivated, 0, 0);
            break;
        case WM_MOUSEACTIVATE:
            if (application->ActiveForm() != this && !Parent())
                SendMessage(application->Handle(), wmFormActivated, (WPARAM)this, NULL);
            break;
        case WM_ACTIVATE:
            f = application->ActiveForm();
            if (lParam == 0 && f && f != this)
            {
                if (((wParam & WA_ACTIVE) != 0 /*|| (wParam & WA_CLICKACTIVE) != 0*/))
                    f->Focus();
                return 0;
            }

            if ((wParam & WA_ACTIVE) != 0 || (wParam & WA_CLICKACTIVE) != 0)
            {
                if (!IsVisible())
                    return 0;

                if (f != this)
                {
                    SendMessage(application->Handle(), wmFormActivated, (WPARAM)this, lParam);
                    if (f)
                        f->Deactivating(this);
                    Activating(f);
                }

                if (!activecontrol)
                    activecontrol = TabFirst();

                if (activecontrol)
                    activecontrol->Focus();

                BroadcastMessage(wmFormActivated, wParam >> 16, lParam);
                ActiveFormChanging(true, (wParam & WA_CLICKACTIVE) != 0, f == this ? NULL : f, (HWND)lParam);

                return 0;
            }

            BroadcastMessage(wmFormDeactivated, wParam >> 16, lParam);
            f = dynamic_cast<Form*>(application->ControlFromHandle((HWND)lParam));
            ActiveFormChanging(false, false, f == this ? NULL : f, (HWND)lParam);

            return 0;
        case WM_SETFOCUS:
            if (activecontrol)
            {
                activecontrol->Focus();
                return 0;
            }
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == 0 && menu) // menu item;
            {
                if (menu->HandleCommand(LOWORD(wParam)))
                    return 0;
            }
            break;
        case WM_NEXTDLGCTL:
            if (LOWORD(lParam) == TRUE)
            {
                SetFocus((HWND)wParam);
                break;
            }

            if (!activecontrol)
            {
                activecontrol = TabFirst();
                if (activecontrol)
                    wParam = (WPARAM)activecontrol->Handle();
            }
            else
            {
                lParam = TRUE;
                wParam = (WPARAM)TabNext(activecontrol, wParam == 0);
            }

            break;
        case WM_ENTERMENULOOP:
            if (activecontrol != nullptr)
                activecontrol->PassMessage(WM_CANCELMODE, 0, 0);
            //else - causes the menu to be hidden immediately
            //    PassMessage(WM_CANCELMODE, 0, 0);
            break;
        case wmDialogKey:
            c = (Control*)lParam;
            if (!c)
                break;
            if (c->PassMessage(wmDialogKey, wParam, (lParam)))
                break;
            else
                HandleDialogKey(wParam);
            break;
        case wmMenuAccelerator:
#ifdef DESIGNING
            if (Designing())
                return 0;
#endif
            if (!menu)
                return 0;
            return menu->HandleShortcut(wParam);
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void Form::Activating(Form *deactivated)
    {
        if (OnActivate)
            OnActivate(this, FormActivateParameters(deactivated));
    }

    void Form::Deactivating(Form *activated)
    {
        if (OnDeactivate)
            OnDeactivate(this, FormActivateParameters(activated));
    }

    void Form::ActiveFormChanging(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow)
    {
        if (OnActiveFormChange)
            OnActiveFormChange(this, ActiveFormChangeParameters(activated, mouseactivate, otherform, otherwindow));
    }

    Control* Form::TabNext(Control *current, bool forward)
    {
        if (!current)
        {
            if (!activecontrol)
                return TabFirst();
            current = activecontrol;
        }

        if (!current || current->ParentForm() != this)
            return NULL;

        return current->Parent()->TabNext(current, forward, true);

    }

    void Form::SetToMainForm(bool setit)
    {
        if (setit == mainform || (setit && Parent())) /* TODO: Remove owners too! */
            return;
        if (!setit)
        {
            mainform = false;
            application->SetMainForm(NULL);
        }
        else
        {
            mainform = true;
            application->SetMainForm(this);
        }
    }

    bool Form::Active()
    {
        return application->ActiveForm() == this;
    }

    void Form::Focus()
    {
        if (!HandleCreated())
            return;

        base::Focus();
        SetForegroundWindow(Handle());
    }

    void Form::Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code)
    {
        if (oldpos == pos)
            return;
        ScrollWindowEx(Handle(), kind == skHorizontal ? oldpos - pos : 0, kind == skVertical ? oldpos - pos : 0, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    }

    void Form::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        if (dynamic_cast<NonVisualControl*>(object) != NULL)
        {
            for (auto it = nvs.begin(); it != nvs.end(); ++it)
            {
                if (*it == object)
                {
                    nvs.erase(it);
                    break;
                }
            }
        }

#ifdef DESIGNING
        if (object == designactivecontrol)
            designactivecontrol = NULL;
#endif
        if (object == activecontrol)
            activecontrol = NULL;
        if (object == menu)
            SetMenu(NULL);
    }

    void Form::Close()
    {
        if (HandleCreated())
            PostMessage(Handle(), WM_CLOSE, 0, 0);
    }

    /*
    void Form::ToScreenCenter()
    {
        Rect r = WindowRect();
        DisplayMonitor *m = screen->MonitorFromWindow(this);
        Rect wa = m->WorkArea();

        SetBounds(RectS(wa.left + (RectWidth(wa) - r.Width()) / 2, wa.top + (RectHeight(wa) - r.Height()) / 2, r.Width(), r.Height()));
    }
    */

    Menubar* Form::Menu()
    {
        return menu;
    }

    void Form::SetMenu(Menubar *newmenu)
    {
        if (menu == newmenu)
            return;
        RemoveFromNotifyList(menu, nrSubControl);
        menu = newmenu;
        if (menu && menu->ParentForm() && menu->ParentForm() != this)
            menu->ParentForm()->SetMenu(NULL);
        AddToNotifyList(menu, nrSubControl);
        if (menu)
            menu->SetParent(this);
        if (HandleCreated())
            ::SetMenu(Handle(), menu ? menu->Handle() : NULL);
    }

    bool Form::IconFromResource(HMODULE module, const wchar_t *resname)
    {
        HRSRC res = FindResource(module, resname, RT_GROUP_ICON);
        if (res == NULL)
            return false;
        HGLOBAL lres = LoadResource(module, res);
        if (lres == NULL)
            return false;
        return IconFromData(module, (char*)LockResource(lres), SizeofResource(module, res));
    }

    bool Form::IconFromData(HMODULE module, const char *data, int datalen)
    {
        Icon *oldlarge = ownlargeicon ? largeicon : NULL;
        Icon *oldsmall = ownsmallicon ? smallicon : NULL;
        largeicon = NULL;
        smallicon = NULL;
        ownlargeicon = false;
        ownsmallicon = false;

        if (data == NULL || !datalen)
        {
            PassMessage(WM_SETICON, ICON_BIG, 0);
            PassMessage(WM_SETICON, ICON_SMALL, 0);
            delete oldlarge;
            delete oldsmall;
            return true;
        }

        HDC dc = GetDC(0);
        if (!dc)
        {
            PassMessage(WM_SETICON, ICON_BIG, 0);
            PassMessage(WM_SETICON, ICON_SMALL, 0);
            delete oldlarge;
            delete oldsmall;
            return false;
        }

        std::vector<IconEntry> entries;
        FillIconEntryVector(data, datalen, entries, true);
        if (entries.empty())
        {
            ReleaseDC(0, dc);

            PassMessage(WM_SETICON, ICON_BIG, 0);
            PassMessage(WM_SETICON, ICON_SMALL, 0);
            delete oldlarge;
            delete oldsmall;
            return false;
        }

        int largeid = FindNearestIconEntry(entries, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), GetDeviceCaps(dc, BITSPIXEL), true);
        int smallid = FindNearestIconEntry(entries, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), GetDeviceCaps(dc, BITSPIXEL), true);
        largeid = entries[largeid].groupicondirentry.resid;
        smallid = entries[smallid].groupicondirentry.resid;

        ReleaseDC(0, dc);

        HRSRC res = FindResource(module, MAKEINTRESOURCE(largeid), RT_ICON);
        HGLOBAL lres = 0;
        if (res)
            lres = LoadResource(module, res);
        if (lres)
        {
            HICON large = CreateIconFromResourceEx((byte*)LockResource(lres), SizeofResource(module, res), TRUE,  0x00030000, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
            if (large)
            {
                largeicon = new Icon(large);
                ownlargeicon = true;
                PassMessage(WM_SETICON, ICON_BIG, (LPARAM)large);
            }
        }

        lres = 0;
        res = FindResource(module, MAKEINTRESOURCE(smallid), RT_ICON);
        if (res)
            lres = LoadResource(module, res);
        if (lres)
        {
            HICON sm = CreateIconFromResourceEx((byte*)LockResource(lres), SizeofResource(module, res), TRUE,  0x00030000, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
            if (sm)
            {
                smallicon = new Icon(sm);
                ownsmallicon = true;
                PassMessage(WM_SETICON, ICON_SMALL, (LPARAM)sm);
            }
        }

        if (!largeicon)
            PassMessage(WM_SETICON, ICON_BIG, 0);
        if (!smallicon)
            PassMessage(WM_SETICON, ICON_SMALL, 0);
        delete oldlarge;
        delete oldsmall;

        return largeicon != NULL || smallicon != NULL;
    }

    bool Form::IconFromRawResource(HMODULE module, const wchar_t *resname)
    {
        HRSRC res = FindResource(module, resname, RT_GROUP_ICON);
        if (!res)
            return false;
        HGLOBAL lres = LoadResource(module, res);
        if (!lres)
            return false;
        return IconFromRawData((char*)LockResource(lres), SizeofResource(module, res));
    }

    bool Form::IconFromRawData(const char *data, int datalen)
    {
        Icon *oldlarge = ownlargeicon ? largeicon : NULL;
        Icon *oldsmall = ownsmallicon ? smallicon : NULL;
        largeicon = NULL;
        smallicon = NULL;
        ownlargeicon = false;
        ownsmallicon = false;

        if (data == NULL || !datalen)
        {
            PassMessage(WM_SETICON, ICON_BIG, 0);
            PassMessage(WM_SETICON, ICON_SMALL, 0);
            delete oldlarge;
            delete oldsmall;
            return true;
        }

        HDC dc = GetDC(0);
        if (!dc)
        {
            PassMessage(WM_SETICON, ICON_BIG, 0);
            PassMessage(WM_SETICON, ICON_SMALL, 0);
            delete oldlarge;
            delete oldsmall;
            return false;
        }

        std::vector<IconEntry> entries;
        FillIconEntryVector(data, datalen, entries, false);
        if (entries.empty())
        {
            ReleaseDC(0, dc);
            PassMessage(WM_SETICON, ICON_BIG, 0);
            PassMessage(WM_SETICON, ICON_SMALL, 0);
            delete oldlarge;
            delete oldsmall;
            return false;
        }

        int largeid = FindNearestIconEntry(entries, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), GetDeviceCaps(dc, BITSPIXEL), false);
        int smallid = FindNearestIconEntry(entries, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), GetDeviceCaps(dc, BITSPIXEL), false);
        IconDirEntry &largeentry = entries[largeid].icondirentry;
        IconDirEntry &smallentry = entries[smallid].icondirentry;

        ReleaseDC(0, dc);

        HICON large = CreateIconFromResourceEx((byte*)data + largeentry.dataoffset, largeentry.datasize, TRUE,  0x00030000, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
        HICON sm = CreateIconFromResourceEx((byte*)data + smallentry.dataoffset, smallentry.datasize, TRUE,  0x00030000, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

        if (large)
        {
            largeicon = new Icon(large);
            ownlargeicon = true;
        }
        if (sm)
        {
            smallicon = new Icon(sm);
            ownsmallicon = true;
        }

        if (!largeicon)
            PassMessage(WM_SETICON, ICON_BIG, 0);
        else
            PassMessage(WM_SETICON, ICON_BIG, (LPARAM)large);
        if (!smallicon)
            PassMessage(WM_SETICON, ICON_SMALL, 0);
        else
            PassMessage(WM_SETICON, ICON_SMALL, (LPARAM)sm);
        delete oldlarge;
        delete oldsmall;

        return largeicon != NULL || smallicon != NULL;
    }

    Icon* Form::SmallIcon()
    {
        return smallicon;
    }

    void Form::SetSmallIcon(Icon *newicon, bool shared)
    {
        if (newicon == smallicon)
            return;
        if (ownsmallicon)
            delete smallicon;
        smallicon = newicon;
        ownsmallicon = !shared;

        if (HandleCreated())
            PassMessage(WM_SETICON, ICON_SMALL, smallicon == NULL ? 0 : (LPARAM)smallicon->Handle());
    }

    Icon* Form::LargeIcon()
    {
        return largeicon;
    }

    void Form::SetLargeIcon(Icon *newicon, bool shared)
    {
        if (newicon == largeicon)
            return;
        if (ownlargeicon)
            delete largeicon;
        largeicon = newicon;
        ownlargeicon = !shared;

        if (HandleCreated())
            PassMessage(WM_SETICON, ICON_BIG, largeicon == NULL ? 0 : (LPARAM)largeicon->Handle());
    }

    void Form::SetClientRect(const Rect &newcrect)
    {
        if (!HandleCreated())
        {
            WindowParams params;
            CreateWindowParams(params);
#ifndef DESIGNING
            Size client;
#endif
            client = Size(max(0, newcrect.Width()), max(0, newcrect.Height()));
            Rect r = Rect(0, 0, client.cx, client.cy);
            if (AdjustWindowRectEx(&r, params.style, menu != NULL, params.extstyle) != FALSE)
            {
                Rect wr = WindowRect();
                r = Rect(wr.left, wr.top, ((UINT)wr.left == (UINT)CW_USEDEFAULT ? 0 : wr.left) + r.Width(), ((UINT)wr.top == (UINT)CW_USEDEFAULT ? 0 : wr.top) + r.Height());
                SetBounds(r);
            }
            return;
        }
        base::SetClientRect(newcrect);
    }

    Rect Form::ControlRect(Control *c)
    {
        if (!c)
            throw L"Control is not specified.";

        if (c->ParentForm() != this || c == this)
            throw L"Control is not placed on this form.";

        if (c->HandleCreated())
        {
            Rect r;
            if (!::GetWindowRect(c->Handle(), &r))
                throw L"Couldn't determine window rectangle";

            Point topleft = ScreenToClient(Point(r.left, r.top));
            Point bottomright = ScreenToClient(Point(r.right, r.bottom));
            return Rect(topleft.x, topleft.y, bottomright.x, bottomright.y);
        }

        Rect r = c->WindowRect();
        Control *p = c->Parent();
        do
        {
            if (p->HandleCreated())
            {
                Point topleft = ScreenToClient(p->ClientToScreen(Point(r.left, r.top)));
                Point bottomright = ScreenToClient(p->ClientToScreen(Point(r.right, r.bottom)));
                return Rect(topleft.x, topleft.y, bottomright.x, bottomright.y);
            }
            else
            {
                if (p == this)
                    return r;

                Rect r2 = p->WindowRect();
                OffsetRect(&r, r2.left, r2.top);
                p = p->Parent();
            }
        } while (p);

        throw L"Error";
    }

    bool Form::KeyPreview()
    {
        return keypreview;
    }

    void Form::SetKeyPreview(bool newkeypreview)
    {
        keypreview = newkeypreview;
    }

    FormBorderStyles Form::BorderStyle()
    {
        return formborder;
    }

    void Form::SetBorderStyle(FormBorderStyles newborderstyle)
    {
        if (formborder == newborderstyle)
            return;
        formborder = newborderstyle;

#ifdef DESIGNING
        if (Designing())
            return;
#endif

        if (HandleCreated())
            RecreateHandle();
    }

    FormBorderButtonSet Form::BorderButtons()
    {
        return borderbuttons;
    }

    void Form::SetBorderButtons(FormBorderButtonSet newborderbuttons)
    {
        if (borderbuttons == newborderbuttons)
            return;

        borderbuttons = newborderbuttons;
#ifdef DESIGNING
        if (Designing())
            return;
#endif

        if (HandleCreated() && formborder != fbsNone)
            RecreateHandle();
    }

#ifdef DESIGNING
    Control *Form::DesignActiveControl()
    {
        return designactivecontrol;
    }

    void Form::DesignSetActiveControl(Control *newactivecontrol)
    {
        if (designactivecontrol == newactivecontrol || (newactivecontrol && (newactivecontrol->ParentForm() != this)))
            return;
        designactivecontrol = newactivecontrol;
    }
#endif

    Control* Form::ActiveControl()
    {
        return activecontrol;
    }

    void Form::SetActiveControl(Control *newactivecontrol)
    {
        if (HandleCreated() && newactivecontrol == NULL && activecontrol->Parent() != this /*&& GetForegroundWindow() != Handle()*/)
        {
            newactivecontrol = activecontrol;
            do
            {
                newactivecontrol = newactivecontrol->Parent();
            } while (newactivecontrol && !newactivecontrol->IsVisible());
        }

        if (activating || activecontrol == newactivecontrol || (newactivecontrol && (newactivecontrol->ParentForm() != this || !newactivecontrol->Visible() || (HandleCreated() && !newactivecontrol->HandleCreated()))))
            return;

        activating = true;

        RemoveFromNotifyList(activecontrol, nrActivation);
        if (activecontrol)
            activecontrol->PassMessage(wmActiveChanged, 0, (LPARAM)newactivecontrol);
        Control *old = activecontrol;
        activecontrol = newactivecontrol;
        AddToNotifyList(activecontrol, nrActivation);

        if (activecontrol != NULL && HandleCreated() && GetForegroundWindow() == Handle() && !activecontrol->Focused())
            activecontrol->Focus();

        if (activecontrol)
            activecontrol->PassMessage(wmActiveChanged, 1, (LPARAM)old);

        activating = false;
    }


    //---------------------------------------------


    Paintbox::Paintbox()
    {
        SetBorderStyle(bsModern);
        SetParentBackground(false);
        SetParentColor(false);
        SetColor(clWindow);

        controlstyle << csInTabOrder;
        controlstyle -= csUpdateOnTextChange;
        controlstyle -= csAcceptInput;
        controlstyle -= csEraseToColor;
        controlstyle -= csParentBackground;
    }

    Paintbox::~Paintbox()
    {
    }

#ifdef DESIGNING
    void Paintbox::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->HideProperty(L"ParentBackground");
        serializer->HideProperty(L"ParentColor");
        serializer->HideProperty(L"Color");
        serializer->HideProperty(L"Text");

        serializer->Find<BorderStylesDesignProperty<Control>>(L"BorderStyle")->SetDefault(bsModern);
        serializer->Find<BoolDesignProperty<Control>>(L"AcceptInput")->SetDefault(false);
    }
#endif

    void Paintbox::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
    }

    void Paintbox::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
    }

    LRESULT Paintbox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Rect oldrect, newrect;
        //int bw;
        switch (uMsg)
        {
            case WM_WINDOWPOSCHANGED:
#ifdef DESIGNING
                if (!Designing())
                    break;
                oldrect = clientrect;
                newrect = ClientRect();

                if (newrect.right < oldrect.right)
                    InvalidateRect(Rect(newrect.right - 1, 0, 1, newrect.bottom));
                else if (newrect.right > oldrect.right)
                    InvalidateRect(Rect(oldrect.right - 1, 0, newrect.right, newrect.bottom));
                if (newrect.bottom < oldrect.bottom)
                    InvalidateRect(Rect(0, newrect.right - 1, newrect.right, 1));
                else if (newrect.bottom > oldrect.bottom)
                    InvalidateRect(Rect(0, oldrect.bottom - 1, newrect.right, newrect.bottom));
#endif
            break;
        }

        return base::WindowProc(uMsg, wParam, lParam);
    }


    void Paintbox::EraseBackground()
    {

#ifdef DESIGNING
        if (!Designing() && OnPaint)
            return;
#else
        if (OnPaint)
            return;
#endif

        Rect r = ClientRect();
        Canvas *canvas = GetCanvas();
        canvas->SelectStockBrush(sbWhite);
        canvas->FillRect(r);
        
#ifdef DESIGNING
        if (Designing())
        {
            canvas->SetPen(clWindowText);
            canvas->GetPen().SetDashStyle(pdsDot);
            canvas->FrameRect(r);
        }
#endif

    }

    /*
    void Paintbox::SetAcceptInput(bool newacceptinput)
    {
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
    */


    //---------------------------------------------


}
/* End of NLIBNS */


#pragma once

#include "designproperties.h"
#include "designerdialogs.h"

//---------------------------------------------


namespace NLIBNS
{


extern ColorDialog *colordialog;

bool MatchColorString4(const std::wstring& val, int &a, int &r, int &g, int &b);
bool MatchColorString3(const std::wstring& val, int &r, int &g, int &b);

void GetTabActivatedControls(DesignForm *form, std::vector<std::pair<std::wstring, Control*> > &controlstrings, const std::wstring &controlname);

extern ValuePair<ControlAlignments> ControlAlignmentStrings[];
template<typename PropertyHolder>
class ControlAlignmentsDesignProperty : public EnumDesignProperty<PropertyHolder, ControlAlignments>
{
private:
    typedef EnumDesignProperty<PropertyHolder, ControlAlignments>    base;
public:
    template<typename GetterProc, typename SetterProc>
    ControlAlignmentsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ControlAlignmentStrings)
    {}
};

extern ValuePair<ControlAlignmentOrders> ControlAlignmentOrderStrings[];
template<typename PropertyHolder>
class ControlAlignmentOrdersDesignProperty : public SetDesignProperty<PropertyHolder, ControlAlignmentOrders>
{
private:
    typedef SetDesignProperty<PropertyHolder, ControlAlignmentOrders>    base;
public:
    template<typename GetterProc, typename SetterProc>
    ControlAlignmentOrdersDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ControlAlignmentOrderStrings)
    {}
};

extern ValuePair<BorderStyles> BorderStyleStrings[];
template<typename PropertyHolder>
class BorderStylesDesignProperty : public EnumDesignProperty<PropertyHolder, BorderStyles>
{
private:
    typedef EnumDesignProperty<PropertyHolder, BorderStyles>    base;
public:
    template<typename GetterProc, typename SetterProc>
    BorderStylesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, BorderStyleStrings)
    {}
};

// Color property based on a list
extern ValuePair<Colors> ColorStrings[];
template<typename PropertyHolder>
class GeneralColorDesignProperty : public GeneralListDesignProperty<PropertyHolder, Color, Colors>
{
private:
    typedef GeneralListDesignProperty<PropertyHolder, Color, Colors> base;
    
    bool allowother;
    bool allownone;
    bool usealpha;
public:
    GeneralColorDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, bool allowother, bool allownone, bool usealpha) : base(name, category, reader, ColorStrings, EnumStringsCount(ColorStrings)),
            allowother(allowother), allownone(allownone), usealpha(usealpha)
    {
        this->propertystyle << psThumbImage;
    }

    virtual std::wstring Value(Object *propholder)
    {
        Color val = this->CallGetter(propholder);
        if (val.EnumIndexed())
            return base::Value(propholder);

        std::wstringstream ss;
        if (usealpha)
            ss << val.A() << L", ";
        ss << val.R() << L", " << val.G() << L", " << val.B();
        return ss.str();
    }

    virtual bool SelectItemValue(Form *parentform, Object *propholder, Colors val)
    {
        if (int(val) == INT_MAX - 1) // Select color
        {
            return ClickEdit(parentform, propholder);

        }
        else
        {
            if (val == clNone && !allownone)
                return false;
            return base::SelectItemValue(parentform, propholder, val); // Base calls SetValue with the selected string.
        }
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        try
        {
            colordialog->SetColor(this->CallGetter(propholder));
            colordialog->SetExpanded(true);
            if (colordialog->Show(parentform))
            {
                this->CallSetter(propholder, colordialog->GetColor());
                return true;
            }
        }
        catch(...)
        {
            ;
        }
        return false;
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& aval)
    {
        std::wstring val = GenToLower(aval);
        if (val.substr(0, 2) == L"cl")
        {
            if ((val == L"clother" && !allowother) || (val == L"clnone" && !allownone))
                return false;
            return base::SetValue(parentform, propholder, val);
        }

        Color c;
        int vallen = val.length();
        if (val == GenToLower(ColorStrings[0].second))
            return false;
        else if ((vallen == 8 && val.substr(0, 2) == L"0x") || (vallen == 7 && val[0] == L'#'))
        {
            int r, g, b;
            int base = val[0] == L'#' ? 1 : 2;
            if (!StrHexToInt(/*L"0x" +*/ val.substr(base, 2), r) || !StrHexToInt(/*L"0x" +*/ val.substr(base + 2, 2), g) || !StrHexToInt(/*L"0x" +*/ val.substr(base + 4, 2), b))
                return false;
            this->CallSetter(propholder, Color(r, g, b));
        }
        else if (usealpha && ((vallen == 10 && val.substr(0, 2) == L"0x") || (vallen == 9 && val[0] == L'#')))
        {
            int a, r, g, b;
            int base = val[0] == L'#' ? 1 : 2;
            if (!StrHexToInt(/*L"0x" +*/ val.substr(base, 2), a) || !StrHexToInt(/*L"0x" +*/ val.substr(base + 2, 2), r) || !StrHexToInt(/*L"0x" +*/ val.substr(base + 4, 2), g) || !StrHexToInt(/*L"0x" +*/ val.substr(base + 6, 2), b))
                return false;
            this->CallSetter(propholder, Color(a, r, g, b));
        }
        else
        {
            int a, r, g, b;
            if (usealpha && MatchColorString4(val, a, r, g, b))
            {
                if (a > 255 || r > 255 || g > 255 || b > 255 || a < 0 || r < 0 || g < 0 || b < 0)
                    return false;
                this->CallSetter(propholder, Color(a, r, g, b));
            }
            else if (MatchColorString3(val, r, g, b))
            {
                if (r > 255 || g > 255 || b > 255 || r < 0 || g < 0 || b < 0)
                    return false;
                this->CallSetter(propholder, Color(r, g, b));
            }
        }

        return false;
    }

    virtual int Selected(Object *propholder)
    {
        Color cl = this->CallGetter(propholder);
        if (!cl.EnumIndexed())
            return 0;
        int ix = base::Selected(propholder);
        if (ix > 1 && !allownone)
            ix--;
        if (ix > 0 && !allowother)
            ix--;
        return ix;
    }

    virtual int ListCount(Object *propholder)
    {
        return base::ListCount(propholder) - (allowother ? 0 : 1) - (allownone ? 0 : 1);
    }

    virtual std::wstring ListItem(Object *propholder, int index)
    {
        if (!allowother)
            ++index;
        if (index >= 1 && !allownone)
            ++index;
        return base::ListItem(propholder, index);
    }

    virtual const Colors& ListItemValue(Object *propholder, int index)
    {
        if (!allowother)
            ++index;
        if (index >= 1 && !allownone)
            ++index;
        return base::ListItemValue(propholder, index);
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        Color cc = index == INT_MAX ? this->CallGetter(propholder) : ListItemValue(propholder, index);
        if (!allowother)
            ++index;
        if (index >= 1 && !allownone)
            ++index;
        bool fill = true;
        if (index == 0)
        {
            cc = this->CallGetter(propholder);
            if (cc.EnumIndexed())
                fill = false;
        }
        else if (index == 1 || cc == clNone)
            fill = false;

        c->SetPen(clBtnText);
        c->FrameRect(r);
        if (fill)
        {
            c->SetBrush(cc);
            c->FillRect(Rect( r.left + 1, r.top + 1, r.right - 1, r.bottom - 1));
        }
    }

    virtual std::wstring ExportValue(Object *propholder)
    {
        Color val = this->CallGetter(propholder);
        if (val.EnumIndexed())
            return base::ExportValue(propholder);

        std::wstringstream ss;
        ss << NLIBNS_STRING L"Color(";
        if (usealpha)
            ss << val.A() << L", ";
        ss << val.R() << L", " << val.G() << L", " << val.B() << L")";
        return ss.str();
    }
};

template<typename PropertyHolder>
class ColorDesignProperty : public GeneralColorDesignProperty<PropertyHolder>
{
private:
    typedef GeneralColorDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    ColorDesignProperty(const std::wstring &name, const std::wstring &category, bool allowother, bool allownone, bool usealpha, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Color>(getter, setter), allowother, allownone, usealpha)
    {
        this->propertystyle << psThumbImage;
    }
};


extern ValuePair<ControlAnchors> ControlAnchorStrings[];
template<typename PropertyHolder>
class ControlAnchorSetDesignProperty : public SetDesignProperty<PropertyHolder, ControlAnchors>
{
private:
    typedef SetDesignProperty<PropertyHolder, ControlAnchors>    base;
public:
    template<typename GetterProc, typename SetterProc>
    ControlAnchorSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ControlAnchorStrings)
    {}
};

extern ValuePair<TextAlignments> TextAlignmentStrings[];
template<typename PropertyHolder>
class TextAlignmentsDesignProperty : public EnumDesignProperty<PropertyHolder, TextAlignments>
{
private:
    typedef EnumDesignProperty<PropertyHolder, TextAlignments>    base;
public:
    template<typename GetterProc, typename SetterProc>
    TextAlignmentsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, TextAlignmentStrings)
    {}
};

extern ValuePair<VerticalTextAlignments> VerticalTextAlignmentStrings[];
template<typename PropertyHolder>
class VerticalTextAlignmentsDesignProperty : public EnumDesignProperty<PropertyHolder, VerticalTextAlignments>
{
private:
    typedef EnumDesignProperty<PropertyHolder, VerticalTextAlignments>    base;
public:
    template<typename GetterProc, typename SetterProc>
    VerticalTextAlignmentsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, VerticalTextAlignmentStrings)
    {}
};

extern ValuePair<WantedKeys> WantedKeyStrings[];
template<typename PropertyHolder>
class WantedKeySetDesignProperty : public SetDesignProperty<PropertyHolder, WantedKeys>
{
private:
    typedef SetDesignProperty<PropertyHolder, WantedKeys>    base;
public:
    template<typename GetterProc, typename SetterProc>
    WantedKeySetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, WantedKeyStrings)
    {}
};


}
/* End of NLIBNS */


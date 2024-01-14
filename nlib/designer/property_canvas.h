#pragma once

#include "designproperties.h"

//---------------------------------------------


namespace NLIBNS
{


const std::wstring PROPDEFAULTFONTNAME = L"<System UI Default>";
void FontFamilyAllowUIDefault(DesignSerializer* serializer);

// Properties
// Font property based on the class property.
template<typename PropertyHolder>
class GeneralFontDesignProperty : public GeneralClassDesignProperty<PropertyHolder, Font&>
{
private:
    typedef GeneralClassDesignProperty<PropertyHolder, Font&>  base;
    typedef GeneralFontDesignProperty<PropertyHolder>   selftype;

    bool usedefault;
public:
    GeneralFontDesignProperty(const std::wstring &name, const std::wstring &category, bool defaultfamily, typename base::readertype *reader) : base(name, category, reader), usedefault(defaultfamily)
    {
        this->propertystyle << psThumbImage << psEditButton;
        if (defaultfamily)
        {
            DesignSerializer* serializer = SerializerByTypeInfo(typeid(Font));
            FontFamilyAllowUIDefault(serializer);
        }
    }

    virtual std::wstring Value(Object *propholder)
    {
        const Font &f = this->CallGetter(propholder);

        std::wstring str;
        str = (f.Family().empty() && usedefault ? PROPDEFAULTFONTNAME : f.Family().empty() ? L"[Not set]" : f.Family().c_str()) + L", " + IntToStr(round(f.Size()));
        if (f.Bold())
            str += L", Bold";
        if (f.Italic())
            str += L", Italic";
        if (f.Strikeout())
            str += L", Strikeout";
        if (f.Underline())
            str += L", Underline";
        return str;
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        Font f = this->CallGetter(propholder);
        f.SetHeight(-r.Height());
        f.SetAngle(0);
        c->SetTextAlignment(ctaBaseline | ctaLeft);
        c->SetFont(f);
        TEXTMETRIC m = c->TextMetrics();
        int y = r.bottom - 2;
        Size s = c->MeasureText(L"A");
        c->TextDraw(r, r.left + (r.Width() - s.cx) / 2, y, L"A");
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        const Font &f = this->CallGetter(propholder);
        std::unique_ptr<FontDialog, object_deleter> dialog(new FontDialog());
        dialog->SetFont(f);
        if (dialog->Show(parentform))
        {
            this->CallSetter(propholder, dialog->GetFont());
            return true;
        }
        return false;
    }
};

template<typename PropertyHolder>
class FontDesignProperty : public GeneralFontDesignProperty<PropertyHolder>
{
private:
    typedef GeneralFontDesignProperty<PropertyHolder>  base;
public:
    template<typename GetterProc, typename SetterProc>
    FontDesignProperty(const std::wstring &name, const std::wstring &category, bool defaultfamily, GetterProc getter, SetterProc setter) : base(name, category, defaultfamily, CreatePropertyReader<PropertyHolder, Font&>(getter, setter))
    {
    }
};

// Font family property.
template<typename PropertyHolder>
class GeneralFontFamilyDesignProperty : public GeneralListDesignProperty<PropertyHolder, std::wstring>
{
private:
    typedef GeneralListDesignProperty<PropertyHolder, std::wstring> base;

    bool sysdef; // True if the listing of font family names contain the system default font, and is set to be the default.
    std::wstring dummyemptystring;
public:
    GeneralFontFamilyDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader, (const std::vector<std::wstring>)*Fonts::GetInstance()), sysdef(false)
    {
        this->propertystyle << psDrawItem;
    }

    void AllowUIDefault()
    {
        sysdef = true;
        this->SetDefault(std::wstring());
    }

    virtual void MeasureListItem(Object *propholder, MeasureItemParameters param)
    {
        param.height = 26 * Scaling;
    }

    virtual void DrawListItem(Object *propholder, DrawItemParameters param)
    {
        Canvas *c = param.canvas;

        std::wstring str = this->ListItem(propholder, param.index);
        std::wstring fontname = param.index == 0 ? application->UILogFont().lfFaceName : str;
        if (fontname[0] == L'@')
            fontname = fontname.substr(1);
        c->SetFont(fontname, 16, 0, 0, c->GetFont().GetColor(), false, false, false, false, fcsDefault, foqDefault);

        Size s = c->MeasureText(str);
        int x = param.rect.left + param.rect.left + (2 * Scaling),
            y = param.rect.top + ((param.rect.Height() - s.cy) / 2);
        c->TextDraw(param.rect, x, y, str);
    }

    virtual std::wstring ExportValue(Object *propholder)
    {
        return L"L\"" + EscapeCString(this->CallGetter(propholder)) + L"\"";
    }

    // Updates from base class
    virtual std::wstring Value(Object *propholder)
    {
        if (sysdef)
        {
            std::wstring val = this->CallGetter(propholder);
            if (val.empty())
                return PROPDEFAULTFONTNAME;
        }

        return base::Value(propholder);
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
    {
        if (sysdef && val == PROPDEFAULTFONTNAME)
        {
            this->CallSetter(propholder, std::wstring());
            return true;
        }
        return base::SetValue(parentform, propholder, val);
    }

    virtual int ListCount(Object *propholder)
    {
        return base::ListCount(propholder) + (sysdef ? 1 : 0);
    }

    virtual std::wstring ListItem(Object *propholder, int index)
    {
        if (sysdef)
        {
            if (index == 0)
                return PROPDEFAULTFONTNAME;
            --index;
        }
        
        return base::ListItem(propholder, index);
    }

    virtual const std::wstring& ListItemValue(Object *propholder, int index)
    {
        if (sysdef)
        {
            if (index == 0)
                return dummyemptystring;
            --index;
        }

        return base::ListItemValue(propholder, index);
    }

    virtual int Selected(Object *propholder)
    {
        std::wstring val = this->CallGetter(propholder);
        if (sysdef)
        {
            if (val.empty())
                return 0;
            return base::Selected(propholder) + 1;
        }

        return base::Selected(propholder);
    }

    virtual bool SelectItemValue(Form *parentform, Object *propholder, std::wstring val)
    {
        if (sysdef && val.empty())
            return SetValue(parentform, propholder, PROPDEFAULTFONTNAME);

        return base::SelectItemValue(parentform, propholder, val);
    }
};

template<typename PropertyHolder>
class FontFamilyDesignProperty : public GeneralFontFamilyDesignProperty<PropertyHolder>
{
private:
    typedef GeneralFontFamilyDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    FontFamilyDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
    {
    }
};

extern const ValuePair<float> propfontsizes[];
extern const int propfontsizes_cnt;
template<typename PropertyHolder>
class GeneralFontSizeDesignProperty : public GeneralListDesignProperty<PropertyHolder, float>
{
private:
    typedef GeneralListDesignProperty<PropertyHolder, float> base;
public:
    GeneralFontSizeDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader, propfontsizes, propfontsizes_cnt)
    {
    }

    virtual float CallGetter(Object *propholder)
    {
        return round(base::CallGetter(propholder));
    }
    
    virtual std::wstring Value(Object *propholder)
    {
        return IntToStr(round(this->CallGetter(propholder)));
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
    {
        try
        {
            double f;
            if (!StrToFloat(val, f))
                return false;
            this->CallSetter(propholder, f);
        }
        catch(...)
        {
            return false;
        }
        return true;
    }
};

template<typename PropertyHolder>
class FontSizeDesignProperty : public GeneralFontSizeDesignProperty<PropertyHolder>
{
private:
    typedef GeneralFontSizeDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    FontSizeDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, float>(getter, setter))
    {
    }
};

extern ValuePair<FontCharacterSets> FontCharacterSetStrings[];
template<typename PropertyHolder>
class FontCharacterSetsDesignProperty : public EnumDesignProperty<PropertyHolder, FontCharacterSets>
{
private:
    typedef EnumDesignProperty<PropertyHolder, FontCharacterSets>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FontCharacterSetsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FontCharacterSetStrings)
    {}
};

extern ValuePair<FontOutputQualities> FontOutputQualityStrings[];
template<typename PropertyHolder>
class FontOutputQualitiesDesignProperty : public EnumDesignProperty<PropertyHolder, FontOutputQualities>
{
private:
    typedef EnumDesignProperty<PropertyHolder, FontOutputQualities>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FontOutputQualitiesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FontOutputQualityStrings)
    {}
};


}
/* End of NLIBNS */


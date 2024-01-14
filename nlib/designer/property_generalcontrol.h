#pragma once

#include "designproperties.h"
#include "designerdialogs.h"

//---------------------------------------------


namespace NLIBNS
{


extern ValuePair<PanelBorderStyles> PanelBorderStyleStrings[];
template<typename PropertyHolder>
class PanelBorderStylesDesignProperty : public EnumDesignProperty<PropertyHolder, PanelBorderStyles>
{
private:
    typedef EnumDesignProperty<PropertyHolder, PanelBorderStyles>    base;
public:
    template<typename GetterProc, typename SetterProc>
    PanelBorderStylesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, PanelBorderStyleStrings)
    {}
};

extern ValuePair<FormShowPositions> FormShowPositionStrings[];
template<typename PropertyHolder>
class FormShowPositionsDesignProperty : public EnumDesignProperty<PropertyHolder, FormShowPositions>
{
private:
    typedef EnumDesignProperty<PropertyHolder, FormShowPositions>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FormShowPositionsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FormShowPositionStrings)
    {}
};

extern ValuePair<FormBorderStyles> FormBorderStyleStrings[];
template<typename PropertyHolder>
class FormBorderStylesDesignProperty : public EnumDesignProperty<PropertyHolder, FormBorderStyles>
{
private:
    typedef EnumDesignProperty<PropertyHolder, FormBorderStyles>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FormBorderStylesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FormBorderStyleStrings)
    {}
};

extern ValuePair<FormBorderButtons> FormBorderButtonStrings[];
template<typename PropertyHolder>
class FormBorderButtonSetDesignProperty : public SetDesignProperty<PropertyHolder, FormBorderButtons>
{
private:
    typedef SetDesignProperty<PropertyHolder, FormBorderButtons>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FormBorderButtonSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FormBorderButtonStrings)
    {}
};


// Property for listing all controls on a form that can be activated.
template<typename PropertyHolder>
class GeneralActiveControlDesignProperty : public GeneralDesignProperty<PropertyHolder, Control*>
{
private:
    typedef GeneralDesignProperty<PropertyHolder, Control*>   base;
    
    std::vector<std::pair<std::wstring, Control*> > controlstrings;
public:
    // Constructors using a ListType string array.
    GeneralActiveControlDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) :    base(name, category, reader)
    {
        this->propertystyle << psDelayedRestore;
    }
    virtual ~GeneralActiveControlDesignProperty() {}

    virtual std::wstring Value(Object *propholder)
    {
        Control *val = this->CallGetter(propholder);
        if (!val)
            return std::wstring();

        std::wstringstream str;

        if (!DesignFormIsForm(this->GetDesignForm(propholder), val->ParentForm()))
            str << val->ParentForm()->Name() << L"->";
        str << val->Name();

        return str.str();
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
    {
        if (val.length() == 0)
        {
            this->CallSetter(propholder, NULL);
            return true;
        }

        DesignForm *f = this->GetDesignForm(propholder);

        std::vector<std::pair<std::wstring, Control*> > tmpcontrolstrings;
        GetTabActivatedControls(f, tmpcontrolstrings, val);

        if (tmpcontrolstrings.empty())
            return false;

        this->CallSetter(propholder, dynamic_cast<Control*>(tmpcontrolstrings.front().second));
        return true;
    }

    virtual int ListCount(Object *propholder)
    {
        DesignForm *f = this->GetDesignForm(propholder);
        controlstrings.clear();
        GetTabActivatedControls(f, controlstrings, std::wstring());
        return controlstrings.size();
    }

    virtual std::wstring ListItem(Object *propholder, int index)
    {
        return controlstrings[index].first;
    }

    virtual int Selected(Object *propholder)
    {
        Control *val = this->CallGetter(propholder);
        if (!val)
            return -1;

        int ix = 0;
        for (auto it = controlstrings.begin(); it != controlstrings.end(); ++it, ++ix)
            if ((*it).second == val)
                return ix;
        return -1;
    }

    virtual void* ListValue(Object *propholder, int index)
    {
        return controlstrings[index].second;
    }

    virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
    {
        for (auto it = controlstrings.begin(); it != controlstrings.end(); ++it)
            if ((*it).second == val)
            {
                this->CallSetter(propholder, (Control*)val);
                return true;
            }
        return false;
    }
};

template<typename PropertyHolder>
class ActiveControlDesignProperty : public GeneralActiveControlDesignProperty<PropertyHolder>
{
private:
    typedef GeneralActiveControlDesignProperty<PropertyHolder>   base;
    
    std::vector<std::pair<std::wstring, Control*> > controlstrings;
public:
    // Constructors using a ListType string array.
    template<typename GetterProc, typename SetterProc>
    ActiveControlDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Control*>(getter, setter))
    {}
};

class Icon;
template<typename PropertyHolder>
class GeneralIconDesignProperty : public GeneralDesignProperty<PropertyHolder, IconData*>
{
protected:
    typedef Icon* (PropertyHolder::*ThumbGetter)();
private:
    typedef GeneralDesignProperty<PropertyHolder, IconData*>    base;
    ThumbGetter thumbgetter;
public:
    GeneralIconDesignProperty(const std::wstring &name, const std::wstring &category, ThumbGetter thumbgetter, typename base::readertype *reader) : base(name, category, reader), thumbgetter(thumbgetter)
    {
        this->propertystyle << psReadonly << psThumbImage << psEditButton << psInnerBinary << psBinaryResource;
        this->propertystyle -= psEditShared;
    }

    virtual bool IsDefaultWrite(Object *propholder)
    {
        return this->CallGetter(propholder)->filedata == NULL;
    }

    virtual std::wstring Value(Object *propholder)
    {
        const IconData &icon = *this->CallGetter(propholder);
        if (!icon.filedata)
            return L"Empty";
        if (*(WORD*)icon.filedata != 0 || *(WORD*)(icon.filedata + 2) != 1)
        {
            IconData icon;
            this->CallSetter(propholder, &icon);
            return L"Empty";
        }
        return L"[" + IntToStr(*(WORD*)(icon.filedata + 4)) + L" icons in group]";
    }

    virtual void BinaryValue(Object *propholder, std::vector<byte> &result)
    {
        const IconData &icon = *this->CallGetter(propholder);
        int headersize = sizeof(WORD) + icon.filename.length() * sizeof(wchar_t) + sizeof(unsigned int);
        uLongf sizemax = compressBound(icon.filesize);
        result.resize(headersize + sizemax);
        byte *arr = &result[0];
        *(WORD*)arr = (WORD)icon.filename.length();
        arr += sizeof(WORD);
        memcpy(arr, icon.filename.c_str(), icon.filename.length() * sizeof(wchar_t));
        arr += icon.filename.length() * sizeof(wchar_t);
        *(unsigned int*)arr = icon.filesize;
        arr += sizeof(unsigned int); // unsigned int

        if (compress(arr, &sizemax, (byte*)icon.filedata, icon.filesize) != Z_OK)
            throw L"Couldn't compress icon data!";
        result.resize(headersize + sizemax);
    }

    virtual void SetBinaryValue(Form *parentform, Object *propholder, std::vector<byte> &val)
    {
        byte *arr = &val[0];
        IconData icon;
        icon.filename.resize(*(WORD*)arr);
        arr += sizeof(WORD);
        memcpy(const_cast<wchar_t*>(icon.filename.c_str()), arr, icon.filename.length() * sizeof(wchar_t));
        arr += icon.filename.length() * sizeof(wchar_t);
        uLongf fsiz = icon.filesize = *(unsigned int*)arr;
        arr += sizeof(unsigned int);
        icon.filedata = new char[icon.filesize];
        if (uncompress((Bytef*)icon.filedata, &fsiz, arr, val.size() - (arr - &val[0])) != Z_OK)
            return;
        this->CallSetter(propholder, &icon);
    }

    virtual void ResourceValue(Object *propholder, std::vector<byte> &result)
    {
        const IconData &icon = *this->CallGetter(propholder);
        result.resize(icon.filesize);
        memcpy(&result[0], icon.filedata, icon.filesize);
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        IconSelectorDialog dialog;
        try
        {
            IconData *data = this->CallGetter(propholder);
            bool result;
            if (data)
                result = dialog.Show(parentform, *data);
            else
                result = dialog.Show(parentform, IconData());
            if (result)
            {
                this->CallSetter(propholder, &dialog.GetIcon());
                return true;
            }
        }
        catch(...)
        {
            ;
        }
        return false;
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        Icon *ico = (dynamic_cast<PropertyHolder*>(propholder)->*thumbgetter)();
        
        if (!ico)
        {
            c->SelectStockPen(spBtnText);
            c->FrameRect(r);
            return;
        }

        Bitmap bmp;
        ico->ToBitmap(bmp);
        double div = min((double)r.Width() / bmp.Width(), (double)r.Height() / bmp.Height());
        int draww = bmp.Width() * min(1, div);
        int drawh = bmp.Height() * min(1, div);
        c->Draw(&bmp, r.left + (r.Width() - draww) / 2, r.top + (r.Height() - drawh) / 2, draww, drawh, 0, 0, bmp.Width(), bmp.Height());
    }

    virtual std::wstring ResourceExportValue(Object *propholder, int resourceid)
    {
        return L"NULL, " + base::ResourceExportValue(propholder, resourceid);
    }

};

template<typename PropertyHolder>
class IconDesignProperty : public GeneralIconDesignProperty<PropertyHolder>
{
private:
    typedef GeneralIconDesignProperty<PropertyHolder>   base;
public:
    template<typename GetterProc, typename SetterProc>
    IconDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter, typename base::ThumbGetter thumbgetter) : base(name, category, thumbgetter, CreatePropertyReader<PropertyHolder, IconData*>(getter, setter))
    {}
};

template<typename PropertyHolder>
class FormIconDesignProperty : public IconDesignProperty<PropertyHolder>
{
private:
    typedef IconDesignProperty<PropertyHolder>   base;
public:
    template<typename GetterProc, typename SetterProc>
    FormIconDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter, typename base::ThumbGetter thumbgetter) : base(name, category, getter, setter, thumbgetter)
    {}

    virtual bool IsDefaultExport(Object *propholder)
    {
        return true;
    }

    virtual bool StoresBinaryResource(Object *propholder)
    {
        return base::StoresBinaryResource(propholder) && this->CallGetter(propholder)->filedata != NULL;
    }

    virtual std::wstring ExportValue(Object *propholder)
    {
        return L"NULL, MAKEINTRESOURCE(IDI_MAINICON)";
    }
};

bool FormHasMenu(Object *propholder);
template<typename PropertyHolder>
class ClientRectDesignProperty : public RectDesignProperty<PropertyHolder>
{
private:
    typedef RectDesignProperty<PropertyHolder>  base;

    typedef DesignForm*(DesignForm::*OwnerGetter)();
    OwnerGetter ownergetter;
protected:
    virtual std::wstring ExportValue(Object *propholder)
    {
        const Rect &r = this->CallGetter(propholder);

        std::wstringstream ws;
        ws << NLIBNS_STRING L"Rect(" << r.left << L", " << r.top << L", " << r.right << L", " << r.bottom << (FormHasMenu(propholder) ? L" + GetSystemMetrics(SM_CYMENU)" : L"") << L")";

        return ws.str();
    }
public:
    template<typename GetterProc>
    ClientRectDesignProperty(const std::wstring &name, const std::wstring &category, OwnerGetter ownergetter, GetterProc getter/*, typename base::ConstSetter setter*/) : base(name, category, true, false, getter, NULL), ownergetter(ownergetter)
    {}
};


}
/* End of NLIBNS */


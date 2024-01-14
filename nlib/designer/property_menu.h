#pragma once

#include "designproperties.h"
#include "menu.h"


//---------------------------------------------


namespace NLIBNS
{


template<typename PropertyHolder>
class GeneralMenubarDesignProperty : public GeneralControlByTypeDesignProperty<PropertyHolder, Menubar>
{
private:
    typedef GeneralControlByTypeDesignProperty<PropertyHolder, Menubar>    base;
public:
    GeneralMenubarDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
    {
    }
};

template<typename PropertyHolder>
class MenubarDesignProperty : public GeneralMenubarDesignProperty<PropertyHolder>
{
private:
    typedef GeneralMenubarDesignProperty<PropertyHolder>    base;
public:
    template<typename GetterProc, typename SetterProc>
    MenubarDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Menubar*>(getter, setter))
    { }
};

template<typename PropertyHolder>
class GeneralPopupMenuDesignProperty : public GeneralControlByTypeDesignProperty<PropertyHolder, PopupMenu>
{
private:
    typedef GeneralControlByTypeDesignProperty<PropertyHolder, PopupMenu>    base;
public:
    GeneralPopupMenuDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
    {
    }
};

template<typename PropertyHolder>
class PopupMenuDesignProperty : public GeneralPopupMenuDesignProperty<PropertyHolder>
{
private:
    typedef GeneralPopupMenuDesignProperty<PropertyHolder>    base;
public:
    template<typename GetterProc, typename SetterProc>
    PopupMenuDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, PopupMenu*>(getter, setter))
    { }
};


template<typename PropertyHolder>
class ShortcutDesignProperty : public GeneralStringDesignProperty<PropertyHolder>
{
private:
    typedef GeneralStringDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    ShortcutDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
    {
        this->SetDefault(std::wstring());
    }

    virtual std::wstring Value(Object *propholder)
    {
        std::wstring result = this->CallGetter(propholder);
        if (result.empty())
            return L"Empty";
        return result;
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
    {
        bool strempty = val == L"Empty";
        WORD tmp;
        if (!strempty && !StrToShortcut(val, tmp))
            return false;
        return base::SetValue(parentform, propholder, strempty ? std::wstring() : val);
    }

    virtual std::wstring InnerValue(Object *propholder)
    {
        if (this->CallGetter(propholder).empty())
            return std::wstring();
        return base::InnerValue(propholder);
    }

    virtual std::wstring ExportValue(Object *propholder)
    {
        if (this->CallGetter(propholder).empty())
            return std::wstring();
        return base::ExportValue(propholder);
    }
};

extern ValuePair<MenuBreakTypes> MenuBreakTypeStrings[];
template<typename PropertyHolder>
class MenuBreakTypesDesignProperty : public EnumDesignProperty<PropertyHolder, MenuBreakTypes>
{
private:
    typedef EnumDesignProperty<PropertyHolder, MenuBreakTypes>  base;
public:
    template<typename GetterProc, typename SetterProc>
    MenuBreakTypesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, MenuBreakTypeStrings)
    {
    }
};


std::wstring MenuItemSerializeArgs(Object *control);

template<typename PropertyHolder>
class MenuItemVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, MenuItem*>
{
private:
    typedef GeneralClassVectorDesignProperty<PropertyHolder, MenuItem*>  base;
public:
    MenuItemVectorDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, category, countgetter, elemgetter, elemadder)
    {
        this->DontSerialize();
        this->SetParentCreation(pcrtFormDeclare, &MenuItemSerializeArgs);
        this->propertystyle << psReadonly << psEditButton;
        this->propertystyle -= psEditShared;
        this->propertystyle -= psGuestEditable;
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        DesignFormBase *form = propholder->DesignParent();
        if (!form || dynamic_cast<PopupMenu*>(propholder) == NULL)
            return false;
        DesignFormEditPopupMenu(form, dynamic_cast<PopupMenu*>(propholder));
        return false;
    }
};


}
/* End of NLIBNS */


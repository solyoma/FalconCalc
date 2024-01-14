#pragma once

#include "designproperties.h"
#include "dialog.h"

//---------------------------------------------


namespace NLIBNS
{


extern ValuePair<DialogModes> DialogModeStrings[];
template<typename PropertyHolder>
class DialogModesDesignProperty : public EnumDesignProperty<PropertyHolder, DialogModes>
{
private:
    typedef EnumDesignProperty<PropertyHolder, DialogModes>    base;
public:
    template<typename GetterProc, typename SetterProc>
    DialogModesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, DialogModeStrings)
    {}
};

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum FontDialogOptions : int;
#else
enum FontDialogOptions;
#endif
extern ValuePair<FontDialogOptions> FontDialogOptionStrings[];
template<typename PropertyHolder>
class FontDialogOptionSetDesignProperty : public SetDesignProperty<PropertyHolder, FontDialogOptions>
{
private:
    typedef SetDesignProperty<PropertyHolder, FontDialogOptions>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FontDialogOptionSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FontDialogOptionStrings)
    {}
};



#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum FolderDialogOptions : int;
#else
//enum FolderDialogOptions;
#endif
extern ValuePair<FolderDialogOptions> FolderDialogOptionStrings[];
template<typename PropertyHolder>
class FolderDialogOptionSetDesignProperty : public SetDesignProperty<PropertyHolder, FolderDialogOptions>
{
private:
    typedef SetDesignProperty<PropertyHolder, FolderDialogOptions>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FolderDialogOptionSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FolderDialogOptionStrings)
    {}
};

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum FileDialogOptions : int;
#else
//enum FileDialogOptions;
#endif
extern ValuePair<FileDialogOptions> FileDialogOptionStrings[];
template<typename PropertyHolder>
class FileDialogOptionSetDesignProperty : public SetDesignProperty<PropertyHolder, FileDialogOptions>
{
private:
    typedef SetDesignProperty<PropertyHolder, FileDialogOptions>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FileDialogOptionSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FileDialogOptionStrings)
    {}
};

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum FileDialogOptionsEx : int;
#else
//enum FileDialogOptionsEx;
#endif
extern ValuePair<FileDialogOptionsEx> FileDialogOptionStringsEx[];
template<typename PropertyHolder>
class FileDialogOptionSetExDesignProperty : public SetDesignProperty<PropertyHolder, FileDialogOptionsEx>
{
private:
    typedef SetDesignProperty<PropertyHolder, FileDialogOptionsEx>    base;
public:
    template<typename GetterProc, typename SetterProc>
    FileDialogOptionSetExDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FileDialogOptionStringsEx)
    {}
};

bool ShowFileExtensionDialog(Form *topparent, std::vector<std::pair<std::wstring, std::wstring>> &filters);
template<typename PropertyHolder>
class FileFilterDesignProperty : public GeneralStringDesignProperty<PropertyHolder>
{
private:
    typedef GeneralStringDesignProperty<PropertyHolder>    base;

    typedef std::vector<std::pair<std::wstring, std::wstring>>& (PropertyHolder::*FilterGetter)();

    FilterGetter filtergetter;
public:
    template<typename GetterProc, typename SetterProc>
    FileFilterDesignProperty(const std::wstring &name, const std::wstring &category, FilterGetter filtergetter, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter)), filtergetter(filtergetter)
    {
        this->propertystyle << psEditButton;
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        return ShowFileExtensionDialog(parentform, (((PropertyHolder*)propholder)->*filtergetter)());
    }
};


}
/* End of NLIBNS */


#pragma once

#include "designproperties.h"
#include "objectbase.h"

//---------------------------------------------


namespace NLIBNS
{


    // Property for the name of the control variables used in the designer.
    template<typename PropertyHolder>
    class TagDesignProperty : public IntDesignProperty<PropertyHolder>
    {
    private:
        typedef IntDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        TagDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter)
        {}
    };

    template<typename PropertyHolder>
    class GeneralNameDesignProperty : public GeneralVariableStringDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralVariableStringDesignProperty<PropertyHolder>    base;
    protected:
        virtual bool InvalidValue(Object *propholder, const std::wstring &val)
        {
            DesignFormBase *f = this->GetDesignFormBase(propholder);
            return base::InvalidValue(propholder, val) || (f && NameTaken(f, val)) || (DesignFormIsObjectBase(f, propholder) && FormNameTaken(f, val));
        }
    public:
        GeneralNameDesignProperty(typename base::readertype *reader) : base(L"Name", std::wstring(), reader)
        {
        }
    };

    template<typename PropertyHolder>
    class NameDesignProperty : public GeneralNameDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralNameDesignProperty<PropertyHolder>   base;
    public:
        NameDesignProperty() : base(CreatePropertyReader<PropertyHolder, std::wstring>(&PropertyHolder::Name, &PropertyHolder::SetName))
        {}
    };

    extern ValuePair<AccessLevels> AccessLevelStrings[];
    template<typename PropertyHolder>
    class AccessLevelDesignProperty : public EnumDesignProperty<PropertyHolder, AccessLevels>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, AccessLevels>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        AccessLevelDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, AccessLevelStrings) {}
    };


    //void ParentSetterDesignPropertySerializerFunc(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum);
    //template<typename PropertyHolder>
    //class ParentFormSetterDesignProperty : public GeneralDesignProperty<PropertyHolder, std::nullptr_t>
    //{
    //private:
    //    typedef GeneralDesignProperty<PropertyHolder, std::nullptr_t>   base;
    //public:
    //    ParentFormSetterDesignProperty() : base(std::wstring(), std::wstring(), NULL)
    //    {
    //        this->DontList();
    //        this->DontSerialize();
    //        this->SetSerializerFunc(&ParentSetterDesignPropertySerializerFunc);
    //    }
    //};


}
/* End of NLIBNS */


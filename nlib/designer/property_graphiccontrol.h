#pragma once

#include "designproperties.h"

//---------------------------------------------


namespace NLIBNS
{


// Bevel properties
extern ValuePair<BevelLineTypes> BevelLineTypeStrings[];
template<typename PropertyHolder>
class BevelLineTypesDesignProperty : public EnumDesignProperty<PropertyHolder, BevelLineTypes>
{
private:
    typedef EnumDesignProperty<PropertyHolder, BevelLineTypes>    base;
public:
    template<typename GetterProc, typename SetterProc>
    BevelLineTypesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, BevelLineTypeStrings)
    {}
};

extern ValuePair<BevelShapeTypes> BevelShapeTypeStrings[];
template<typename PropertyHolder>
class BevelShapeTypesDesignProperty : public EnumDesignProperty<PropertyHolder, BevelShapeTypes>
{
private:
    typedef EnumDesignProperty<PropertyHolder, BevelShapeTypes>    base;
public:
    template<typename GetterProc, typename SetterProc>
    BevelShapeTypesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, BevelShapeTypeStrings)
    {}
};

extern ValuePair<LabelShowAccelerators> LabelShowAcceleratorStrings[];
template<typename PropertyHolder>
class LabelShowAcceleratorDesignProperty : public EnumDesignProperty<PropertyHolder, LabelShowAccelerators>
{
private:
    typedef EnumDesignProperty<PropertyHolder, LabelShowAccelerators>    base;
public:
    template<typename GetterProc, typename SetterProc>
    LabelShowAcceleratorDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, LabelShowAcceleratorStrings)
    {}
};

bool ValidAccessControlProc(Object *obj)
{
    Control *c;
    if ((c = dynamic_cast<Control*>(obj)) == NULL)
        return false;
    if (c->AcceptInput())
        return true;
    DesignSerializer *serializer = c->Serializer();
    return SerializerFind(serializer, L"AcceptInput") != NULL;
}

template<typename PropertyHolder>
class GeneralAccessControlDesignProperty : public GeneralControlDesignProperty<PropertyHolder, Control>
{
private:
    typedef GeneralControlDesignProperty<PropertyHolder, Control> base;
public:
    // Constructors using a ListType string array.
    GeneralAccessControlDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, ValidAccessControlProc, reader)
    {
    }
    virtual ~GeneralAccessControlDesignProperty() {}
};

template<typename PropertyHolder>
class AccessControlDesignProperty : public GeneralAccessControlDesignProperty<PropertyHolder>
{
private:
    typedef GeneralAccessControlDesignProperty<PropertyHolder> base;
public:
    // Constructors using a ListType string array.
    template<typename GetterProc, typename SetterProc>
    AccessControlDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Control*>(getter, setter))
    {
    }
    virtual ~AccessControlDesignProperty() {}
};


}
/* End of NLIBNS */



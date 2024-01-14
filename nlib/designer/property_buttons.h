#pragma once

#include "designproperties.h"
#include "buttons.h"

//---------------------------------------------


namespace NLIBNS
{


    extern ValuePair<FlatButtonTypes> FlatButtonTypeStrings[];
    template<typename PropertyHolder>
    class FlatButtonTypesDesignProperty : public EnumDesignProperty<PropertyHolder, FlatButtonTypes>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, FlatButtonTypes>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        FlatButtonTypesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, FlatButtonTypeStrings)
        {}
    };

    extern ValuePair<ButtonImagePositions> ButtonImagePositionStrings[];
    template<typename PropertyHolder>
    class ButtonImagePositionsDesignProperty : public EnumDesignProperty<PropertyHolder, ButtonImagePositions>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ButtonImagePositions>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ButtonImagePositionsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ButtonImagePositionStrings)
        {}
    };

    extern ValuePair<ButtonContentPositions> ButtonContentPositionStrings[];
    template<typename PropertyHolder>
    class ButtonContentPositionsDesignProperty : public EnumDesignProperty<PropertyHolder, ButtonContentPositions>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ButtonContentPositions>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ButtonContentPositionsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ButtonContentPositionStrings)
        {}
    };


}
/* End of NLIBNS */


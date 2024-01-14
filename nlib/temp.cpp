#include "stdafx.h"
#include "controlbase.h"

#ifdef DESIGNING
#include "designproperties.h"
#include "serializer.h"
#endif

#include "temp.h"

namespace temporary
{
    void InitTypes();
    void InitControls();
}

//NLIBNS::RegisterInitializerFunctions RegisterTemporary(L"temporary", &temporary::InitTypes, &temporary::InitControls);


namespace temporary
{

#ifdef DESIGNING
    const std::pair<TempEnum, std::wstring> TempStrings[] = {
            std::make_pair(tempFirst, L"tempFirst"),
            std::make_pair(tempSecond, L"tempSecond"),
    };

    void InitTypes()
    {
        using NLIBNS::RegisterControlEvent;
        using NLIBNS::RegisterEnumStrings;

        RegisterControlEvent<TempEvent>(L"TempParameters");

        RegisterEnumStrings(TempStrings, tempCount);
    }

    void InitControls()
    {
        using NLIBNS::CreateDesignProperties;
        using NLIBNS::otVisual;
        using NLIBNS::otNonVisual;

        CreateDesignProperties<TempObj, otVisual> (L"TempObj", L"Temp Category");
    }


    void TempObj::EnumerateProperties(nlib::DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->Add(L"SetTemp", new TempDesignProperty<TempObj>(L"Temp", L"Temp category", &TempObj::GetTemp, &TempObj::SetTemp))->SetDefault(tempFirst);
        serializer->AddEvent<TempObj, TempEvent>(L"OnTemp", L"Temp category");
    }
#endif

    TempObj::TempObj() : tmp(tempFirst)
    {
    }

    TempObj::~TempObj()
    {
    }

    TempEnum TempObj::GetTemp() const
    {
        return tmp;
    }

    void TempObj::SetTemp(TempEnum &newtmp)
    {
        if (tmp == newtmp)
            return;
        tmp = newtmp;
        if (OnTemp)
            OnTemp(this, TempParameters(newtmp));
    }

}


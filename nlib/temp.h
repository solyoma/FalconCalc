#pragma once

namespace temporary
{
    enum TempEnum {
        tempFirst, tempSecond,
#ifdef DESIGNING
        tempCount = 2
#endif
    };

#ifdef DESIGNING
    extern const std::pair<TempEnum, std::wstring> TempStrings[];

    template<typename PropertyHolder>
    class TempDesignProperty : public nlib::EnumDesignProperty<PropertyHolder, TempEnum>
    {
    private:
        typedef nlib::EnumDesignProperty<PropertyHolder, TempEnum>  base;
    public:
        template<typename GetterProc, typename SetterProc>
        TempDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, TempStrings) {}
    };
#endif

    struct TempParameters
    {
        TempEnum val;
        TempParameters(TempEnum val) : val(val) {}
    };
    typedef nlib::Event<TempParameters> TempEvent;


    class TempObj : public nlib::Control
    {
    private:
        typedef nlib::Control   base;

        TempEnum tmp;
    protected:
        ~TempObj();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(nlib::DesignSerializer *serializer);
#endif
        TempObj();

        TempEnum GetTemp() const;
        void SetTemp(TempEnum &newtmp);

        TempEvent OnTemp;
    };
}

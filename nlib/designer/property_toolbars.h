#pragma once

#ifdef TOOLBARS_ENABLED

#include "designproperties.h"
#include "toolbars.h"

namespace NLIBNS
{

    std::wstring ToolbarButtonSerializeArgs(Object *control);

    template<typename PropertyHolder>
    class ToolbarButtonVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ToolbarButton*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, ToolbarButton*>  base;
    public:
        ToolbarButtonVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->DontList();
            this->SetParentCreation(pcrtFormDeclare, &ToolbarButtonSerializeArgs);
        }
    };

    extern ValuePair<ToolbarButtonCheckStates> ToolbarButtonCheckStateStrings[];
    template<typename PropertyHolder>
    class ToolbarButtonCheckStatesDesignProperty : public EnumDesignProperty<PropertyHolder, ToolbarButtonCheckStates>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ToolbarButtonCheckStates> base;
    public:
        template<typename Getter, typename Setter>
        ToolbarButtonCheckStatesDesignProperty(const std::wstring &name, const std::wstring &category, Getter getter, Setter setter) : base(name, category, getter, setter, ToolbarButtonCheckStateStrings) {}
    };

    extern ValuePair<ToolbarButtonTypes> ToolbarButtonTypeStrings[];
    template<typename PropertyHolder>
    class ToolbarButtonTypesDesignProperty : public EnumDesignProperty<PropertyHolder, ToolbarButtonTypes>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ToolbarButtonTypes> base;
    public:
        template<typename Getter, typename Setter>
        ToolbarButtonTypesDesignProperty(const std::wstring &name, const std::wstring &category, Getter getter, Setter setter) : base(name, category, getter, setter, ToolbarButtonTypeStrings) {}
    };

    extern ValuePair<ToolbarButtonStyles> ToolbarButtonStyleStrings[];
    template<typename PropertyHolder>
    class ToolbarButtonStylesDesignProperty : public EnumDesignProperty<PropertyHolder, ToolbarButtonStyles>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ToolbarButtonStyles> base;
    public:
        template<typename Getter, typename Setter>
        ToolbarButtonStylesDesignProperty(const std::wstring &name, const std::wstring &category, Getter getter, Setter setter) : base(name, category, getter, setter, ToolbarButtonStyleStrings) {}
    };

    extern ValuePair<ToolbarKinds> ToolbarKindStrings[];
    template<typename PropertyHolder>
    class ToolbarKindsDesignProperty : public EnumDesignProperty<PropertyHolder, ToolbarKinds>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ToolbarKinds> base;
    public:
        template<typename Getter, typename Setter>
        ToolbarKindsDesignProperty(const std::wstring &name, const std::wstring &category, Getter getter, Setter setter) : base(name, category, getter, setter, ToolbarKindStrings) {}
    };

    //template<typename PropertyHolder>
    //class ToolbarButtonDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ToolbarButton*>
    //{
    //private:
    //    typedef GeneralClassVectorDesignProperty<PropertyHolder, ToolbarButton*>   base;
    //public:
    //    ToolbarButtonDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, category, countgetter, elemgetter, elemadder)
    //    {
    //        this->propertystyle << psEditButton;
    //        this->SetParentCreation(pcrtFormDeclare);
    //    }

    //    virtual bool ClickEdit(Form *parentform, Object *propholder)
    //    {
    //        ToolbarButtonEditorList editlist(dynamic_cast<Toolbar*>(propholder));
    //        PropertyEditorDialog *diag = new PropertyEditorDialog();
    //        return diag->Show(parentform, &editlist);
    //    }
    //};
}

#endif
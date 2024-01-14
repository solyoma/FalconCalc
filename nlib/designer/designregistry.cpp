#include "stdafx_zoli.h"
#include "designregistry.h"
#include "serializer.h"
#include "syscontrol.h"
#include "generalcontrol.h"
#include "graphiccontrol.h"
#include "buttons.h"
#include "utility.h"
#include "gridbase.h"
#include "dialog.h"
#include "designer.h"
#include "imagelist.h"
#ifdef TOOLBARS_ENABLED
#include "toolbars.h"
#endif
#include "resource.h"

//---------------------------------------------


namespace NLIBNS
{


    // Expand each of these when a new designable control is added.
    //const wchar_t* ControlTypeNames[] = {
    //        // Non controls
    //        L"Form", L"Container",
    //
    //        // Visual controls
    //        L"Panel", L"Paintbox", L"FlatButton", L"Label", 
    //        L"Bevel", L"Button", L"Checkbox", L"Radiobox",
    //        L"Groupbox", L"Edit", L"Memo", L"UpDown",
    //        L"Progressbar", L"Listbox", L"Combobox", L"TabControl",
    //        L"PageControl", L"Listview", L"StringGrid",
    //        
    //        // Non-visual controls
    //        L"Imagelist", L"ColorDialog", L"FontDialog", L"OpenDialog",
    //        L"SaveDialog", L"FolderDialog", L"Menubar", L"PopupMenu", 
    //        L"Timer",
    //
    //        // Non-controls that must be serialized
    //        L"Project", L"Font", L"ControlImage", L"Tab",
    //        L"TabPage", L"HeaderColumn", L"ListviewGroup", L"ListviewItem",
    //        L"ListviewSubitem", L"ControlElemList", L"MenuItem", L"GridColumn",
    //        L"GridRow",
    //
    //        // Built-in types
    //        L"int", L"bool", L"string", L"wstring"
    //                                    };

    //const char* ControlTypeInfoNames[] = {
    //        // Non controls
    //        typeid(DesignForm).name(), typeid(Container).name(),
    //
    //        // Visual controls
    //        typeid(Panel).name(), typeid(Paintbox).name(), typeid(FlatButton).name(), typeid(Label).name(), 
    //        typeid(Bevel).name(), typeid(Button).name(), typeid(Checkbox).name(), typeid(Radiobox).name(),
    //        typeid(Groupbox).name(), typeid(Edit).name(), typeid(Memo).name(), typeid(UpDown).name(),
    //        typeid(Progressbar).name(), typeid(Listbox).name(), typeid(Combobox).name(), typeid(TabControl).name(),
    //        typeid(PageControl).name(), typeid(Listview).name(), /*typeid(ListGrid).name(),*/ typeid(StringGrid).name(),
    //        
    //        // Non-visual controls
    //        typeid(Imagelist).name(), typeid(ColorDialog).name(), typeid(FontDialog).name(), typeid(OpenDialog).name(),
    //        typeid(SaveDialog).name(), typeid(FolderDialog).name(), typeid(Menubar).name(), typeid(PopupMenu).name(),
    //        typeid(Timer).name(),
    //
    //        // Non-controls that must be serialized
    //        typeid(Project).name(), typeid(Font).name(), typeid(ControlImage).name(), typeid(Tab).name(), typeid(TabPageOwner).name(),
    //        typeid(HeaderColumn).name(), typeid(ListviewGroup).name(), typeid(ListviewItem).name(), typeid(ListviewSubitem).name(),
    //        /*typeid(ListGridColumn).name(), typeid(ListGridRow).name(), typeid(ListGridRowSub).name(),*/ typeid(ControlElemList).name(),
    //        typeid(MenuItem).name(), typeid(GridColumn).name(), typeid(GridRow).name(),
    //
    //        // Built-in types
    //        typeid(int).name(), typeid(bool).name(), typeid(std::string).name(), typeid(std::wstring).name(),
    //                                    };
    //
    //const ObjectTypes ObjectTypeValues[] = {
    //        // Non controls
    //        otVisual, otNonVisual,
    //
    //        // Visual controls
    //        otVisual, otVisual, otVisual, otVisual,
    //        otVisual, otVisual, otVisual, otVisual,
    //        otVisual, otVisual, otVisual, otVisual,
    //        otVisual, otVisual, otVisual, otVisual,
    //        otVisual, otVisual, otVisual,
    //
    //        // Non-visual controls
    //        otNonVisual, otNonVisual, otNonVisual, otNonVisual,
    //        otNonVisual, otNonVisual, otNonVisual, otNonVisual,
    //        otNonVisual,
    //
    //        // Non-controls that must be serialized.
    //        otTopLevel, otClass, otClass, otSubControl, otSubControl,
    //        otSubControl, otSubControl, otClass, otClass,
    //        /*otSubControl, otClass, otClass,*/ otClass,
    //        otSubControl, otClass, otClass,
    //
    //        // Built-in types
    //        otNative, otNative, otNative, otNative
    //};
    //
    //const int ControlImageIndexes[] = {
    //        // Non controls
    //        -1, -1,
    //
    //        // Visual controls
    //        0, 1, 2, 3,
    //        22, 4, 5, 6,
    //        7, 8, 9, 10,
    //        11, 12, 13, 23,
    //        24, 25, /*26,*/ 27,
    //
    //        // Non-visual controls
    //        14, 15, 16, 17,
    //        18, 19, 20, 21,
    //        29,
    //
    //        // Non-controls and built-in types have no image index.
    //};
    //
    //const bool ContainerAcceptedControls[] = {
    //        // Non controls
    //        false, false,
    //
    //        // Visual controls
    //        false, false, false, false,
    //        false, false, false, false,
    //        false, false, false, false,
    //        false, false, false, false,
    //        false, false, /*false,*/ false,
    //
    //        // Non-visual controls
    //        true, true, true, true,
    //        true, true, false, true,
    //        true,
    //
    //        // Non-controls and built-in types do not apply.
    //};


    //---------------------------------------------


    extern Imagelist *ilControls;


    //---------------------------------------------


    static std::wstring *regnamespace;
    static const std::wstring& SetNamespace(const std::wstring &namespc)
    {
        static std::wstring rn;
        regnamespace = &rn;
        rn = namespc;
        return rn;
    }

    static const std::wstring& GetNamespace()
    {
        return *regnamespace;
    }

    static std::vector<std::wstring> *controlcategories;
    static std::vector<std::wstring>& ControlCategories()
    {
        static std::vector<std::wstring> cc;
        controlcategories = &cc;
        return *controlcategories;
    }

    static int AddControlCategory(const std::wstring &category)
    {
        auto &cc = ControlCategories();
        auto it = std::find(cc.begin(), cc.end(), category);
        if (it != cc.end())
            return it - cc.begin();

        cc.push_back(category);
        return cc.size() - 1;
    }

    int ControlCategoryCount()
    {
        return ControlCategories().size();
    }

    const std::wstring& GetControlCategory(int index)
    {
        return ControlCategories()[index];
    }


    //---------------------------------------------


    struct ObjectProperty
    {
        std::function<Object*()> constructor;

        type_info const *type;
        DesignSerializer *serializer;
        std::wstring namespc;
        std::wstring displayname;
        ObjectTypes objtype;
        int imgindex;
        int categindex;

        ObjectProperty(std::function<Object*()> constructor, const type_info &type, ObjectTypes objtype, const std::wstring &namespc, const std::wstring &displayname, const std::wstring &category = std::wstring(), Bitmap *controlimage = nullptr, Rect imagerect = Rect()) :
                    constructor(constructor), type(&type), serializer(NULL), namespc(namespc), displayname(displayname), objtype(objtype), imgindex( -1), categindex( -1)
        {
            if (objtype == otNative)
                return;

            serializer = new DesignSerializer();

            if (objtype == otVisual || objtype == otNonVisual || objtype == otSingleNonVisual)
            {
                if (!category.empty())
                    categindex = AddControlCategory(category);
                else
                    categindex = AddControlCategory(L"Unknown");
            }

            if (controlimage != nullptr)
            {
                Bitmap controlbmp(ilControls->Width(), ilControls->Height());
                if (imagerect.Width() != ilControls->Width() || imagerect.Height() != ilControls->Height())
                {
                    controlbmp.SetInterpolationMode(imHQBicubic);
                    controlbmp.SetPixelOffsetMode(pomHalf);
                }
                controlbmp.Draw(controlimage, 0, 0, ilControls->Width(), ilControls->Height(), imagerect.left, imagerect.top, imagerect.Width(), imagerect.Height());
                imgindex = ilControls->Count();
                ilControls->Add(&controlbmp);
            }
        }

        ObjectProperty() : type(NULL), serializer(NULL), objtype(otUnknown), imgindex(-1), categindex(-1)
        {
        
        }

        ObjectProperty(ObjectProperty &&other) noexcept
        {
            std::swap(type, other.type);
            std::swap(serializer, other.serializer);
            std::swap(namespc, other.namespc);
            std::swap(displayname, other.displayname);
            std::swap(objtype, other.objtype);
            std::swap(imgindex, other.imgindex);
            std::swap(categindex, other.categindex);
        }

    };


    //---------------------------------------------


    static std::map<void*, std::pair<std::wstring, int>> *stringsmap;
    static std::map<void*, std::pair<std::wstring, int>>& StringsMap()
    {
        static std::map<void*, std::pair<std::wstring, int>> sm;
        stringsmap = &sm;
        return *stringsmap;
    }


    //---------------------------------------------


    static std::map<std::string, std::wstring> *eventmap;
    static std::map<std::string, std::wstring>& EventMap()
    {
        static std::map<std::string, std::wstring> em;
        eventmap = &em;
        return *eventmap;
    }


    //---------------------------------------------


    static std::map<std::string, ObjectProperty> *propertymap;
    static std::map<std::string, ObjectProperty>& PropertyMap()
    {
        static std::map<std::string, ObjectProperty> pm;
        propertymap = &pm;
        return *propertymap;
    }

    int RegisteredControlCount()
    {
        int cnt = 0;

        auto &pm = PropertyMap();
        for (auto p : pm)
        {
            if (p.second.objtype == otVisual || p.second.objtype == otNonVisual || p.second.objtype == otSingleNonVisual)
                ++cnt;
        }

        return cnt;
    }

    const type_info& GetRegisteredControlType(int index)
    {
        auto &pm = PropertyMap();
        for (auto p : pm)
        {
            if (p.second.objtype == otVisual || p.second.objtype == otNonVisual || p.second.objtype == otSingleNonVisual)
                --index;
            if (index == -1)
                return *p.second.type;
        }

        throw L"Registered control index outside range.";
    }

    int GetRegisteredControlCategoryIndex(const type_info &type)
    {
        auto &pm = PropertyMap();
        return pm[type.name()].categindex;
    }


    //---------------------------------------------


    void DesignPropertiesHelper::AddEnumStrings(void *strings, int size)
    {
        if (StringsMap().find(strings) != StringsMap().end())
            return;

        StringsMap()[strings] = std::make_pair(GetNamespace(), size);
    }

    int DesignPropertiesHelper::EnumStringsCount(void *strings)
    {
        auto it = StringsMap().find(strings);
        if (it == StringsMap().end())
            throw L"The passed enum strings array has not been registered.";
        return it->second.second;
    }

    bool DesignPropertiesHelper::EnumStringsRegistered(void *strings)
    {
        return StringsMap().find(strings) != StringsMap().end();
    }

    const std::wstring& DesignPropertiesHelper::EnumStringsNamespace(void *strings)
    {
        auto it = StringsMap().find(strings);
        if (it == StringsMap().end())
            throw L"The passed enum strings array has not been registered.";
        return it->second.first;
    }

    void DesignPropertiesHelper::AddControlEvent(const type_info &eventtype, const std::wstring &paramstring)
    {
        const char *tname = eventtype.name();
        std::map<std::string, std::wstring>::iterator it;
        if ((it = EventMap().find(tname)) != EventMap().end())
            return;

        EventMap()[tname] = GetNamespace() + L"::" + paramstring;
    }

    void DesignPropertiesHelper::AddObjectProperty(std::function<Object*()> func, const type_info &type, ObjectTypes objtype, const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect)
    {
        const char *tname = type.name();
        std::map<std::string, ObjectProperty>::iterator it;
        if ((it = PropertyMap().find(tname)) != PropertyMap().end())
            return;

        PropertyMap()[tname] = ObjectProperty(func, type, objtype, GetNamespace(), displayname, category, controlimage, imagerect);
    }


    //---------------------------------------------

    // Creates a properties list and serializer object by a control's class name, creating them if necessary.
    template<typename PropertyHolder>
    struct _CreateDesignProperties<PropertyHolder, otNative>
    {
        _CreateDesignProperties(const std::wstring &displayname)
        {
            const type_info &type = typeid(PropertyHolder);
            const char *tname = type.name();
            std::map<std::string, ObjectProperty>::iterator it;
            if ((it = PropertyMap().find(tname)) != PropertyMap().end())
                return;

            PropertyMap()[tname] = ObjectProperty(nullptr, type, otNative, GetNamespace(), displayname);
        }
    };

    template<>
    void CreateDesignProperties<int, otNative>(const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect)
    {
        _CreateDesignProperties<int, otNative> dummy(displayname);
    }

    template<>
    void CreateDesignProperties<bool, otNative>(const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect)
    {
        _CreateDesignProperties<bool, otNative> dummy(displayname);
    }

    template<>
    void CreateDesignProperties<std::string, otNative>(const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect)
    {
        _CreateDesignProperties<std::string, otNative> dummy(displayname);
    }

    template<>
    void CreateDesignProperties<std::wstring, otNative>(const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect)
    {
        _CreateDesignProperties<std::wstring, otNative> dummy(displayname);
    }


    //---------------------------------------------



    static std::vector<std::pair<std::wstring, void(*)()>> *typinitializers;
    static std::vector<std::pair<std::wstring, void(*)()>> *ctrlinitializers;

    RegisterInitializerFunctions::RegisterInitializerFunctions(const std::wstring &namespacename, void(*typeinitializer)(), void(*controlinitializer)())
    {
        static std::vector<std::pair<std::wstring, void(*)()>> ti;
        static std::vector<std::pair<std::wstring, void(*)()>> ci;
        typinitializers = &ti;
        ctrlinitializers = &ci;

        ti.push_back(std::make_pair(namespacename, typeinitializer));
        ci.push_back(std::make_pair(namespacename, controlinitializer));
    }

    void RunTypeInitializers()
    {
        for (auto func : *typinitializers)
        {
            SetNamespace(func.first);
            func.second();
        }
    }

    void RunControlInitializers()
    {
        for (auto func : *ctrlinitializers)
        {
            SetNamespace(func.first);
            func.second();
        }
    }


    //---------------------------------------------


    void InitializeDesignTypes();
    void InitializeDesignProperties();
    RegisterInitializerFunctions MyControlRegister(NLIB_TOSTRING(NLIBNS), &InitializeDesignTypes, &InitializeDesignProperties);

    extern ValuePair<Colors> ColorStrings[];

    extern ValuePair<FlatButtonTypes> FlatButtonTypeStrings[];
    extern ValuePair<ButtonImagePositions> ButtonImagePositionStrings[];
    extern ValuePair<ButtonContentPositions> ButtonContentPositionStrings[];
    extern ValuePair<FontCharacterSets> FontCharacterSetStrings[];
    extern ValuePair<FontOutputQualities> FontOutputQualityStrings[];
    extern ValuePair<ControlAlignments> ControlAlignmentStrings[];
    extern ValuePair<ControlAlignmentOrders> ControlAlignmentOrderStrings[];
    extern ValuePair<BorderStyles> BorderStyleStrings[];
    extern ValuePair<TextAlignments> TextAlignmentStrings[];
    extern ValuePair<VerticalTextAlignments> VerticalTextAlignmentStrings[];
    extern ValuePair<DialogModes> DialogModeStrings[];
    extern ValuePair<PanelBorderStyles> PanelBorderStyleStrings[];
    extern ValuePair<FormShowPositions> FormShowPositionStrings[];
    extern ValuePair<FormBorderStyles> FormBorderStyleStrings[];
    extern ValuePair<BevelLineTypes> BevelLineTypeStrings[];
    extern ValuePair<BevelShapeTypes> BevelShapeTypeStrings[];
    extern ValuePair<LabelShowAccelerators> LabelShowAcceleratorStrings[];
    extern ValuePair<GridSelectionKinds> GridSelectionKindStrings[];
    extern ValuePair<ImagelistColorDepths> ImagelistColorDepthStrings[];
    extern ValuePair<MenuBreakTypes> MenuBreakTypeStrings[];
    extern ValuePair<AccessLevels> AccessLevelStrings[];
    extern ValuePair<ListviewDisplayStyles> ListviewDisplayStyleStrings[];
    extern ValuePair<ListviewSortDirections> ListviewSortDirectionStrings[];
    extern ValuePair<HeaderColumnSortDirections> HeaderColumnSortDirectionStrings[];
    extern ValuePair<CheckboxStates> CheckboxStateStrings[];
    extern ValuePair<ComboboxTypes> ComboboxTypeStrings[];
    extern ValuePair<ListControlKinds> ListControlKindStrings[];
    extern ValuePair<ListboxSelectionTypes> ListboxSelectionTypeStrings[];

    extern ValuePair<ControlAnchors> ControlAnchorStrings[];
    extern ValuePair<WantedKeys> WantedKeyStrings[];
    extern ValuePair<FontDialogOptions> FontDialogOptionStrings[];
    extern ValuePair<FolderDialogOptions> FolderDialogOptionStrings[];
    extern ValuePair<FileDialogOptions> FileDialogOptionStrings[];
    extern ValuePair<FileDialogOptionsEx> FileDialogOptionStringsEx[];
    extern ValuePair<FormBorderButtons> FormBorderButtonStrings[];
    extern ValuePair<GridOptions> GridDrawOptionStrings[];
    extern ValuePair<ListviewOptions> ListviewOptionStrings[];
    extern ValuePair<ListviewOptionsEx> ListviewOptionStringsEx[];
    extern ValuePair<ListviewGroupStates> ListviewGroupStateStrings[];

    extern ValuePair<StatusBarSizeGrips> StatusBarSizeGripStrings[];
    extern ValuePair<StatusBarPartBevels> StatusBarPartBevelStrings[];
#ifdef TOOLBARS_ENABLED
    extern ValuePair<ToolbarKinds> ToolbarKindStrings[];
    extern ValuePair<ToolbarButtonCheckStates> ToolbarButtonCheckStateStrings[];
    extern ValuePair<ToolbarButtonTypes> ToolbarButtonTypeStrings[];
    extern ValuePair<ToolbarButtonStyles> ToolbarButtonStyleStrings[];
#endif
    void InitializeDesignTypes()
    {
        RegisterControlEvent<NotifyEvent>(L"EventParameters");
        RegisterControlEvent<ScrollEvent>(L"ScrollParameters");
        RegisterControlEvent<MessageEvent>(L"MessageParameters");
        RegisterControlEvent<FocusChangedEvent>(L"FocusChangedParameters");
        RegisterControlEvent<KeyEvent>(L"KeyParameters");
        RegisterControlEvent<KeyPressEvent>(L"KeyPressParameters");
        RegisterControlEvent<MouseMoveEvent>(L"MouseMoveParameters");
        RegisterControlEvent<MouseButtonEvent>(L"MouseButtonParameters");
        RegisterControlEvent<NCMouseMoveEvent>(L"NCMouseMoveParameters");
        RegisterControlEvent<NCMouseButtonEvent>(L"NCMouseButtonParameters");
        RegisterControlEvent<SizePositionChangedEvent>(L"SizePositionChangedParameters");
        RegisterControlEvent<DialogCodeEvent>(L"DialogCodeParameters");
        RegisterControlEvent<DragImageRequestEvent>(L"DragImageRequestParameters");
        RegisterControlEvent<DragDropEndedEvent>(L"DragDropEndedParameters");
        RegisterControlEvent<DragDropEvent>(L"DragDropParameters");
        RegisterControlEvent<DragDropDropEvent>(L"DragDropDropParameters");
        RegisterControlEvent<CheckboxCheckEvent>(L"CheckboxCheckParameters");
        RegisterControlEvent<MeasureItemEvent>(L"MeasureItemParameters");
        RegisterControlEvent<DrawItemEvent>(L"DrawItemParameters");
        RegisterControlEvent<TabChangingEvent>(L"TabChangingParameters");
        RegisterControlEvent<TabChangeEvent>(L"TabChangeParameters");
        RegisterControlEvent<ListviewGroupTaskEvent>(L"ListviewGroupTaskParameters");
        RegisterControlEvent<BeginListviewItemEditEvent>(L"BeginListviewItemEditParameters");
        RegisterControlEvent<EndListviewItemEditEvent>(L"EndListviewItemEditParameters");
        RegisterControlEvent<CancelListviewItemEditEvent>(L"CancelListviewItemEditParameters");
        RegisterControlEvent<FormCloseEvent>(L"FormCloseParameters");
        RegisterControlEvent<ActiveFormChangeEvent>(L"ActiveFormChangeParameters");
        RegisterControlEvent<FormActivateEvent>(L"FormActivateParameters");
        RegisterControlEvent<PaintEvent>(L"PaintParameters");
        RegisterControlEvent<ScrollOverflowEvent>(L"ScrollOverflowParameters");
        RegisterControlEvent<AllowColumnRowEvent>(L"AllowColumnRowParameters");
        RegisterControlEvent<ColumnRowSizeEvent>(L"ColumnRowSizeParameters");
        RegisterControlEvent<RadioboxCheckEvent>(L"RadioboxCheckParameters");
        RegisterControlEvent<BeginCellEditEvent>(L"BeginCellEditParameters");
        RegisterControlEvent<CellEditedEvent>(L"CellEditedParameters");
        RegisterControlEvent<EndCellEditEvent>(L"EndCellEditParameters");
        RegisterControlEvent<GridCellAlignmentEvent>(L"GridCellAlignmentParameters");
        RegisterControlEvent<ActiveChangedEvent>(L"ActiveChangedParameters");
        RegisterControlEvent<KeyPushEvent>(L"KeyPushParameters");
        RegisterControlEvent<AppSelectActiveFormEvent>(L"AppSelectActiveFormParameters");
        RegisterControlEvent<WantMouseWheelEvent>(L"WantMouseWheelParameters");
        RegisterControlEvent<MouseWheelEvent>(L"MouseWheelParameters");
        RegisterControlEvent<ButtonMeasureSplitSizeEvent>(L"ButtonMeasureSplitSizeParameters");
        RegisterControlEvent<ButtonOwnerDrawEvent>(L"ButtonOwnerDrawParameters");

        RegisterEnumStrings(ColorStrings, clCount);
        RegisterEnumStrings(FlatButtonTypeStrings, fbtCount);
        RegisterEnumStrings(ButtonImagePositionStrings, bipCount);
        RegisterEnumStrings(ButtonContentPositionStrings, bcpCount);
        RegisterEnumStrings(FontCharacterSetStrings, fcsCount);
        RegisterEnumStrings(FontOutputQualityStrings, foqCount);
        RegisterEnumStrings(ControlAlignmentStrings, caAlCount);
        RegisterEnumStrings(ControlAlignmentOrderStrings, caoCount);
        RegisterEnumStrings(BorderStyleStrings, bsCount);
        RegisterEnumStrings(TextAlignmentStrings, taCount);
        RegisterEnumStrings(VerticalTextAlignmentStrings, vtaCount);
        RegisterEnumStrings(DialogModeStrings, dmCount);
        RegisterEnumStrings(PanelBorderStyleStrings, pbsCount);
        RegisterEnumStrings(FormShowPositionStrings, fspCount);
        RegisterEnumStrings(FormBorderStyleStrings, fbsCount);
        RegisterEnumStrings(BevelLineTypeStrings, bltCount);
        RegisterEnumStrings(BevelShapeTypeStrings, bstCount);
        RegisterEnumStrings(LabelShowAcceleratorStrings, lsaCount);
        RegisterEnumStrings(GridSelectionKindStrings, gskCount);
        RegisterEnumStrings(ImagelistColorDepthStrings, icdCount);
        RegisterEnumStrings(MenuBreakTypeStrings, mbtCount);
        RegisterEnumStrings(AccessLevelStrings, alevCount);
        RegisterEnumStrings(ListviewDisplayStyleStrings, ldsCount);
        RegisterEnumStrings(ListviewSortDirectionStrings, lsdCount);
        RegisterEnumStrings(HeaderColumnSortDirectionStrings, hcsdCount);
        RegisterEnumStrings(CheckboxStateStrings, cbscCount);
        RegisterEnumStrings(ComboboxTypeStrings, ctCount);
        RegisterEnumStrings(ListControlKindStrings, lckCount);
        RegisterEnumStrings(ListboxSelectionTypeStrings, lstCount);

        RegisterEnumStrings(ControlAnchorStrings, casCount);
        RegisterEnumStrings(WantedKeyStrings, wkCount);
        RegisterEnumStrings(FontDialogOptionStrings, fntoCount);
        RegisterEnumStrings(FolderDialogOptionStrings, fodoCount);
        RegisterEnumStrings(FileDialogOptionStrings, fdoCount);
        RegisterEnumStrings(FileDialogOptionStringsEx, fdoxCount);
        RegisterEnumStrings(FormBorderButtonStrings, fbbCount);
        RegisterEnumStrings(GridDrawOptionStrings, gskCount);
        RegisterEnumStrings(ListviewOptionStrings, loCount);
        RegisterEnumStrings(ListviewOptionStringsEx, loxCount);
        RegisterEnumStrings(ListviewGroupStateStrings, lgsCount);

        RegisterEnumStrings(StatusBarSizeGripStrings, sbsgCount);
        RegisterEnumStrings(StatusBarPartBevelStrings, sbpbCount);

#ifdef TOOLBARS_ENABLED
        RegisterEnumStrings(ToolbarKindStrings, tbkCount);
        RegisterEnumStrings(ToolbarButtonCheckStateStrings, tbbcsCount);
        RegisterEnumStrings(ToolbarButtonTypeStrings, tbctCount);
        RegisterEnumStrings(ToolbarButtonStyleStrings, tbtsCount);
#endif
    }

    void InitializeDesignProperties()
    {

        Bitmap bmp((HMODULE)NULL, MAKEINTRESOURCE(IDI_CONTROLIMAGES));
        Bitmap bmp2((HMODULE)NULL, MAKEINTRESOURCE(IDI_CONTROLIMAGES2));

        // Built-in types
        CreateDesignProperties<int, otNative>           (L"int");
        CreateDesignProperties<bool, otNative>          (L"bool");
        CreateDesignProperties<std::string, otNative>   (L"string");
        CreateDesignProperties<std::wstring, otNative>  (L"wstring");

        // Non-control types which need properties
        CreateDesignProperties<Project, otTopLevel>     (L"Project");
        CreateDesignProperties<Font, otClass>           (L"Font");
        CreateDesignProperties<ControlImage, otClass>   (L"ControlImage");
        CreateDesignProperties<Tab, otSubControl>       (L"Tab");

        CreateDesignProperties<TabPageOwner, otSubControl>  (L"TabPage");
        CreateDesignProperties<HeaderColumn, otSubControl>  (L"HeaderColumn");
        CreateDesignProperties<ListviewGroup, otSubControl> (L"ListviewGroup");
        CreateDesignProperties<ListviewItem, otClass>       (L"ListviewItem");

        CreateDesignProperties<ListviewSubitem, otClass>    (L"ListviewSubitem");
        CreateDesignProperties<ControlElemList, otClass>    (L"ControlElemList");
        CreateDesignProperties<MenuItem, otSubControl>      (L"MenuItem");

        CreateDesignProperties<GridColumn, otClass>         (L"GridColumn");
        CreateDesignProperties<GridRow, otClass>            (L"GridRow");
        CreateDesignProperties<StatusBarPart, otClass>      (L"StatusBarPart");

        CreateDesignProperties<DesignForm, otForm>          (L"Form");
        CreateDesignProperties<DesignContainerForm, otForm> (L"Container");

#ifdef TOOLBARS_ENABLED
        CreateDesignProperties<ToolbarButton, otSubControl> (L"ToolbarButton");
#endif

        #define BR(n)   &bmp, RectS(n * 19, 0, 19, 19)
        #define BR2(n)   &bmp2, RectS(n * 19, 0, 19, 19)

        // Visual controls
        CreateDesignProperties<Panel, otVisual>         (L"Panel", L"Containers", BR(0));
        CreateDesignProperties<Paintbox, otVisual>      (L"Paintbox", L"General Controls", BR(1));
        CreateDesignProperties<FlatButton, otVisual>    (L"FlatButton", L"Buttons", BR(2));
        CreateDesignProperties<Label, otVisual>         (L"Label", L"General Controls", BR(3));

        CreateDesignProperties<Bevel, otVisual>         (L"Bevel", L"General Controls", BR(22));
        CreateDesignProperties<Button, otVisual>        (L"Button", L"Buttons", BR(4));
        CreateDesignProperties<Checkbox, otVisual>      (L"Checkbox", L"Buttons", BR(5));
        CreateDesignProperties<Radiobox, otVisual>      (L"Radiobox", L"Buttons", BR(6));

        CreateDesignProperties<Groupbox, otVisual>      (L"Groupbox", L"Containers", BR(7));
        CreateDesignProperties<Edit, otVisual>          (L"Edit", L"Edit Controls", BR(8));
        CreateDesignProperties<Memo, otVisual>          (L"Memo", L"Edit Controls", BR(9));
        CreateDesignProperties<UpDown, otVisual>        (L"UpDown", L"Buttons", BR(10));

        CreateDesignProperties<Progressbar, otVisual>   (L"Progressbar", L"General Controls", BR(11));
        CreateDesignProperties<Listbox, otVisual>       (L"Listbox", L"List Controls", BR(12));
        CreateDesignProperties<Combobox, otVisual>      (L"Combobox", L"List Controls", BR(13));
        CreateDesignProperties<TabControl, otVisual>    (L"TabControl", L"Containers", BR(23));

        CreateDesignProperties<PageControl, otVisual>   (L"PageControl", L"Containers", BR(24));
        CreateDesignProperties<Listview, otVisual>      (L"Listview", L"List Controls", BR(25));
        CreateDesignProperties<StringGrid, otVisual>    (L"StringGrid", L"List Controls", BR(27));

        CreateDesignProperties<StatusBar, otVisual>     (L"StatusBar", L"Bar Controls", BR(30));

#ifdef TOOLBARS_ENABLED
        CreateDesignProperties<Toolbar, otVisual>       (L"Toolbar", L"Bar Controls", BR2(0));
#endif
        //CreateDesignProperties<Rebar, otVisual>     (L"Rebar", L"System Controls", BR2(1));

        // Non visual controls
        CreateDesignProperties<Imagelist, otNonVisual>      (L"Imagelist", L"Non-Visual Controls", BR(14));
        CreateDesignProperties<ColorDialog, otNonVisual>    (L"ColorDialog", L"Dialogs", BR(15));
        CreateDesignProperties<FontDialog, otNonVisual>     (L"FontDialog", L"Dialogs", BR(16));
        CreateDesignProperties<OpenDialog, otNonVisual>     (L"OpenDialog", L"Dialogs", BR(17));

        CreateDesignProperties<SaveDialog, otNonVisual>     (L"SaveDialog", L"Dialogs", BR(18));
        CreateDesignProperties<FolderDialog, otNonVisual>   (L"FolderDialog", L"Dialogs", BR(19));
        CreateDesignProperties<Menubar, otSingleNonVisual>  (L"Menubar", L"Non-Visual Controls", BR(20));
        CreateDesignProperties<PopupMenu, otNonVisual>      (L"PopupMenu", L"Non-Visual Controls", BR(21));

        CreateDesignProperties<Timer, otNonVisual>      (L"Timer", L"Non-Visual Controls", BR(29));

        #undef BR
        #undef BR2

    }


    //---------------------------------------------


    std::wstring NamespaceByTypeInfo(const type_info &type)
    {
        auto it = PropertyMap().find(type.name());
        if (it == PropertyMap().end())
            return NULL;
        return it->second.namespc;
    }


    //---------------------------------------------


    const type_info& TypeInfoByDisplayName(const std::wstring &aname, bool namespacedname)
    {
        std::wstring nmspc;
        std::wstring name;
        if (namespacedname)
        {
            name = FullNamespaceName(aname);
            size_t pos = name.rfind(L"::");
            if (pos != std::wstring::npos)
            {
                nmspc = name.substr(0, pos);
                name.erase(0, pos + 2);
            }
        }
        else
            name = aname;

        for (auto &item : PropertyMap())
            if ((!namespacedname || item.second.namespc == nmspc) && item.second.displayname == name)
                return *item.second.type;
        throw L"Design name not found";
    }

    ObjectTypes ObjectTypeByTypeDisplayName(const std::wstring &aname, bool namespacedname)
    {
        std::wstring nmspc;
        std::wstring name;
        if (namespacedname)
        {
            name = FullNamespaceName(aname);
            size_t pos = name.rfind(L"::");
            if (pos != std::wstring::npos)
            {
                nmspc = name.substr(0, pos);
                name.erase(0, pos + 2);
            }
        }
        else
            name = aname;

        for (auto &item : PropertyMap())
            if ((!namespacedname || item.second.namespc == nmspc) && item.second.displayname == name)
            {
                if (item.second.objtype == otSingleNonVisual)
                    return otNonVisual;
                return item.second.objtype;
            }
        throw L"Design name not found";
    }

    ObjectTypes ObjectTypeByTypeInfo(const type_info &info)
    {
        auto ot = PropertyMap()[info.name()].objtype;
        if (ot == otSingleNonVisual)
            return otNonVisual;
        return ot;
    }

    std::wstring DisplayNameByTypeInfo(const type_info &info, bool namespacedname)
    {
        if (!namespacedname)
            return PropertyMap()[info.name()].displayname;
        return PropertyMap()[info.name()].namespc + L"::" + PropertyMap()[info.name()].displayname;
    }

    bool ShareableByTypeInfo(const type_info &info)
    {
        return PropertyMap()[info.name()].objtype == otNonVisual;
    }

    DesignSerializer* SerializerByTypeInfo(const type_info &info)
    {
        auto it = PropertyMap().find(info.name());
        if (it == PropertyMap().end())
            return NULL;
        return it->second.serializer;
    }

    DesignSerializer* SerializerByDisplayName(const std::wstring &name, bool namespacedname)
    {
        return SerializerByTypeInfo(TypeInfoByDisplayName(name, namespacedname));
    }

    int ImageIndexByTypeInfo(const type_info &info)
    {
        return PropertyMap()[info.name()].imgindex;
    }

    Control* CreateControlOfType(const type_info &info)
    {
        return dynamic_cast<Control*>(PropertyMap()[info.name()].constructor());
    }

    NonVisualControl* CreateNVControlOfType(const type_info &info)
    {
        return dynamic_cast<NonVisualControl*>(PropertyMap()[info.name()].constructor());
    }


    //---------------------------------------------


    std::wstring FullNamespaceName(const std::wstring &name)
    {
        if (name.find(L"::") == std::wstring::npos)
            return NLIB_TOSTRING(NLIBNS) L"::" + name;
        return name;
    }

        std::wstring ShortNamespaceName(const std::wstring &name)
    {
        const int nliblen = wcslen(NLIB_TOSTRING(NLIBNS) L"::");
        if (name.compare(0, nliblen, NLIB_TOSTRING(NLIBNS) L"::") == 0 && name.find(L"::", nliblen) == std::wstring::npos)
            return name.substr(nliblen);
        return name;
    }

    bool EventTypesMatch(const std::wstring &type1, const std::wstring &type2)
    {
        auto pos1 = type1.find(L"::");
        auto pos2 = type2.find(L"::");
        if (pos1 == std::wstring::npos)
        {
            if (pos2 == std::wstring::npos)
                return type1 == type2;
            return NLIB_TOSTRING(NLIBNS) L"::" + type1 == type2;
        }
        if (pos2 == std::wstring::npos)
            return NLIB_TOSTRING(NLIBNS) L"::" + type2 == type1;

        return type1 == type2;
    }

    std::wstring RegisteredEventType(const type_info &type)
    {
        auto it = EventMap().find(type.name());
        if (it == EventMap().end())
            throw L"This event is not present.";
        return it->second;
    }

    bool EventTypeRegistered(const std::wstring &eventtype)
    {
        std::wstring t = FullNamespaceName(eventtype);
        for (auto m : EventMap())
            if (m.second == eventtype)
                return true;
        return false;
    }


    //---------------------------------------------


}
/* End of NLIBNS */

#pragma once

#include "designproperties.h"
#include "designerdialogs.h"
#include "syscontrol.h"

//---------------------------------------------


namespace NLIBNS
{


    std::wstring TabsSerializeArgs(Object *control);

    template<typename PropertyHolder>
    class TabVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, Tab*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, Tab*>  base;
    public:
        TabVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->DontList();
            this->SetParentCreation(pcrtLocalDeclare, &TabsSerializeArgs);
        }
    };

    template<typename PropertyHolder>
    class TabPageDesignProperty : public GeneralDesignProperty<PropertyHolder, TabPage*>
    {
    public:
        typedef int(PropertyHolder::*CountGetter)() const;
        typedef TabPage*(PropertyHolder::*PageGetter)(int index);
    private:
        typedef GeneralDesignProperty<PropertyHolder, TabPage*> base;

        CountGetter countgetter;
        PageGetter pagegetter;
    protected:
        virtual std::wstring Value(Object *propholder)
        {
            TabPage *tab = this->CallGetter(propholder);
            if (!tab)
                return std::wstring();

            return tab->Name();
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            if (!val.length())
                return false;

            if (val.length() && val[0] >= L'0' && val[0] <= L'9')
            {
                int j;
                if (!StrToInt(val, j))
                    return false;
                //if (countgetter)
                    j = min(max(0, j),(((PropertyHolder*)propholder)->*countgetter)());
                //else
                //    j = min(max(0, j),(((PropertyHolder*)propholder)->*constcountgetter)());
                if (j < 0)
                    return false;
                this->CallSetter(propholder, (((PropertyHolder*)propholder)->*pagegetter)(j));
                return true;
            }

            int cnt;
            //if (countgetter)
                cnt = (((PropertyHolder*)propholder)->*countgetter)();
            //else
            //    cnt = (((PropertyHolder*)propholder)->*constcountgetter)();
            for (int ix = 0; ix < cnt; ++ix)
            {
                if ((((PropertyHolder*)propholder)->*pagegetter)(ix)->Name() == val)
                {
                    this->CallSetter(propholder, (((PropertyHolder*)propholder)->*pagegetter)(ix));
                    return true;
                }
            }
            return false;
        }

        virtual int ListCount(Object *propholder)
        {
            //if (countgetter)
                return (((PropertyHolder*)propholder)->*countgetter)();
            //else
            //    return (((PropertyHolder*)propholder)->*constcountgetter)();
        }

        virtual std::wstring ListItem(Object *propholder, int index)
        {
            return (((PropertyHolder*)propholder)->*pagegetter)(index)->Name();
        }

        virtual void* ListValue(Object *propholder, int index)
        {
            return (((PropertyHolder*)propholder)->*pagegetter)(index);
        }

        virtual int Selected(Object *propholder)
        {
            TabPage *tab = this->CallGetter(propholder);
            if (!tab)
                return -1;

            return tab->Index();
        }

        virtual std::wstring InnerValue(Object *propholder)
        {
            TabPage *tab = this->CallGetter(propholder);
            if (!tab)
                return std::wstring();

            return IntToStr(tab->Index());
        }

    public:
        template<typename GetterProc, typename SetterProc>
        TabPageDesignProperty(const std::wstring &name, const std::wstring & category, GetterProc getter, SetterProc setter, CountGetter countgetter, PageGetter pagegetter) : base(name, category, CreatePropertyReader<PropertyHolder, TabPage*>(getter, setter)), countgetter(countgetter), pagegetter(pagegetter)
        {
            this->propertystyle -= psEditShared;
            this->propertystyle << psDelayedRestore;
            this->SetDefault(nullptr);
        }
    };

    extern ValuePair<ListviewDisplayStyles> ListviewDisplayStyleStrings[];
    template<typename PropertyHolder>
    class ListviewDisplayStylesDesignProperty : public EnumDesignProperty<PropertyHolder, ListviewDisplayStyles>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ListviewDisplayStyles>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListviewDisplayStylesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListviewDisplayStyleStrings)
        {}
    };

    extern ValuePair<ListviewSortDirections> ListviewSortDirectionStrings[];
    template<typename PropertyHolder>
    class ListviewSortDirectionsDesignProperty : public EnumDesignProperty<PropertyHolder, ListviewSortDirections>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ListviewSortDirections>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListviewSortDirectionsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListviewSortDirectionStrings)
        {}
    };

    extern ValuePair<ListviewOptions> ListviewOptionStrings[];
    template<typename PropertyHolder>
    class ListviewOptionSetDesignProperty : public SetDesignProperty<PropertyHolder, ListviewOptions>
    {
    private:
        typedef SetDesignProperty<PropertyHolder, ListviewOptions>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListviewOptionSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListviewOptionStrings)
        {}
    };

    extern ValuePair<ListviewOptionsEx> ListviewOptionStringsEx[];
    template<typename PropertyHolder>
    class ListviewOptionSetExDesignProperty : public SetDesignProperty<PropertyHolder, ListviewOptionsEx>
    {
    private:
        typedef SetDesignProperty<PropertyHolder, ListviewOptionsEx>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListviewOptionSetExDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListviewOptionStringsEx)
        {}
    };

    class HeaderColumnEditorList : public PropertyEditorList
    {
    private:
        Listview *owner;

        std::vector<HeaderColumn*> cols;
    public:
        HeaderColumnEditorList(Listview *owner) : owner(owner)
        {
            int cnt = owner->ColumnCount();
            cols.reserve(cnt);
            for (int ix = 0; ix < cnt; ++ix)
                cols.push_back(new HeaderColumn(owner->Columns(ix)));
        }

        virtual ~HeaderColumnEditorList()
        {
            int cnt = cols.size();
            for (int ix = 0; ix < cnt; ++ix)
                cols[ix]->Destroy();
            cols.clear();
        }

        virtual PropertyEditorDialogButtonSet Buttons()
        {
            return pedbAdd | pedbDelete | pedbMove;
        }

        virtual void Finished(bool canceled)
        {
            int cnt;
            if (canceled)
            {
                cnt = owner->ColumnCount();
                while (cnt--)
                    owner->DeleteColumn(0);
                std::vector<int> columnorder;
                cnt = cols.size();
                columnorder.reserve(cnt);
                for (int ix = 0; ix < cnt; ++ix)
                {
                    columnorder.push_back(cols[ix]->Position());
                    owner->InsertColumn(cols[ix]);
                }
                if (cnt)
                    owner->UpdateColumnOrder(&columnorder.front(), cnt);

                cols.clear();
            }
        }

        virtual int Count()
        {
            return owner->ColumnCount();
        }

        virtual void Move(int pos, int subpos, int diff)
        {
            int a = pos + (diff > 0 ? 0 : -1);
            int b = pos + (diff > 0 ? 1 : 0);
            HeaderColumn *olda = new HeaderColumn(owner->Columns(a));
            HeaderColumn *oldb = new HeaderColumn(owner->Columns(b));
            owner->DeleteColumn(b);
            owner->DeleteColumn(a);
            int bpos = oldb->Position();
            oldb->SetPosition(olda->Position());
            olda->SetPosition(bpos);
            owner->InsertColumn(oldb, a);
            owner->InsertColumn(olda, b);
        }

        virtual int SubCount(int pos)
        {
            return 0;
        }

        virtual int Add()
        {
            return owner->AddColumn()->Index();
        }

        virtual int AddSub(int pos)
        {
            return 0;
        }

        virtual std::wstring Texts(int pos, int subpos)
        {
            return owner->Columns(pos)->Text();
        }

        virtual Object* Objects(int pos, int subpos)
        {
            return owner->Columns(pos);
        }

        virtual void Delete(int pos, int subpos)
        {
            owner->DeleteColumn(pos);
        }

        virtual bool CanDelete(int pos, int subpos)
        {
            return true;
        }
    };

    template<typename PropertyHolder>
    class HeaderColumnVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, HeaderColumn*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, HeaderColumn*>  base;
    public:
        HeaderColumnVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->propertystyle << psEditButton;
            this->SetParentCreation(pcrtFormDeclare);
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            HeaderColumnEditorList editlist(dynamic_cast<Listview*>(propholder));
            PropertyEditorDialog *diag = new PropertyEditorDialog();
            return diag->Show(parentform, &editlist);
        }
    };

    extern ValuePair<HeaderColumnSortDirections> HeaderColumnSortDirectionStrings[];
    template<typename PropertyHolder>
    class HeaderColumnSortDirectionsDesignProperty : public EnumDesignProperty<PropertyHolder, HeaderColumnSortDirections>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, HeaderColumnSortDirections>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        HeaderColumnSortDirectionsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, HeaderColumnSortDirectionStrings)
        {}
    };


    class ListviewItemEditorList : public PropertyEditorList
    {
    private:
        Listview *owner;

        std::vector<ListviewItem*> datalist;
    public:
        ListviewItemEditorList(Listview *owner) : owner(owner)
        {
            owner->SaveItems(datalist);
        }

        virtual ~ListviewItemEditorList()
        {
            int cnt = datalist.size();
            for (int ix = 0; ix < cnt; ++ix)
                datalist[ix]->Destroy();
        }

        virtual PropertyEditorDialogButtonSet Buttons()
        {
            return pedbAdd | pedbAddSub | pedbDelete | pedbMove;
        }

        virtual void Finished(bool canceled)
        {
            if (canceled)
                owner->RestoreItems(datalist);
        }

        virtual int Count()
        {
            return owner->ItemCount();
        }

        virtual void Move(int pos, int subpos, int diff)
        {
            if (subpos < 0)
                owner->SwapItems(pos, pos + diff);
            else
                owner->Items(pos)->SwapSubitems(subpos, subpos + diff);
        }

        virtual int SubCount(int pos)
        {
            return owner->Items(pos)->SubCount();
        }

        virtual int Add()
        {
            return owner->AddItem()->Index();
        }

        virtual int AddSub(int pos)
        {
            owner->Items(pos)->AddSub();
            return owner->Items(pos)->SubCount() - 1;
        }

        virtual std::wstring Texts(int pos, int subpos)
        {
            return subpos < 0 ? owner->Items(pos)->Text() : owner->Items(pos)->SubitemText(subpos);
        }

        virtual Object* Objects(int pos, int subpos)
        {
            ListviewItem *i = owner->Items(pos);
            return subpos < 0 ? (Object*)i : (Object*)i->Subitem(subpos);
        }

        virtual void Delete(int pos, int subpos)
        {
            if (subpos < 0)
                owner->DeleteItem(pos);
            else
                owner->Items(pos)->DeleteSubitem(subpos);
        }

        virtual bool CanDelete(int pos, int subpos)
        {
            return true;
        }
    };

    template<typename PropertyHolder>
    class ListviewItemVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ListviewItem*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, ListviewItem*>  base;
    public:
        ListviewItemVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->propertystyle << psEditButton;
            this->SetParentCreation(pcrtFormDeclare);
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            Listview *listview = dynamic_cast<Listview*>(propholder);
            if (listview->Virtual())
                return false;
            ListviewItemEditorList editlist(listview);
            PropertyEditorDialog *diag = new PropertyEditorDialog();
            return diag->Show(parentform, &editlist);
        }
    };

    class ListviewSubitem;
    template<typename PropertyHolder>
    class ListviewSubitemVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ListviewSubitem*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, ListviewSubitem*>  base;
    public:
        ListviewSubitemVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->DontList()->DontExport();
        }
    };

    class ListviewGroupEditorList : public PropertyEditorList
    {
    private:
        Listview *owner;

        std::vector<ListviewGroup*> datalist;
        std::vector<int> indexlist;
    public:
        ListviewGroupEditorList(Listview *owner) : owner(owner)
        {
            owner->SaveGroups(datalist, indexlist);
        }

        virtual ~ListviewGroupEditorList()
        {
            int cnt = datalist.size();
            for (int ix = 0; ix < cnt; ++ix)
                datalist[ix]->Destroy();
        }

        virtual PropertyEditorDialogButtonSet Buttons()
        {
            return pedbAdd | pedbDelete | pedbMove;
        }

        virtual void Finished(bool canceled)
        {
            if (canceled)
                owner->RestoreGroups(datalist, indexlist);
        }

        virtual int Count()
        {
            return owner->GroupCount();
        }

        virtual void Move(int pos, int subpos, int diff)
        {
            owner->SwapGroups(pos, pos + diff);
        }

        virtual int SubCount(int pos)
        {
            return 0;
        }

        virtual int Add()
        {
            return owner->AddGroup()->Index();
        }

        virtual int AddSub(int pos)
        {
            return 0;
        }

        virtual std::wstring Texts(int pos, int subpos)
        {
            return owner->Groups(pos)->Header();
        }

        virtual Object* Objects(int pos, int subpos)
        {
            ListviewGroup *i = owner->Groups(pos);
            return i;
        }

        virtual void Delete(int pos, int subpos)
        {
            owner->DeleteGroup(pos);
        }

        virtual bool CanDelete(int pos, int subpos)
        {
            return true;
        }
    };

    template<typename PropertyHolder>
    class ListviewGroupVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ListviewGroup*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, ListviewGroup*>  base;
    public:
        ListviewGroupVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->propertystyle << psEditButton;
            this->SetParentCreation(pcrtFormDeclare);
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            Listview *listview = dynamic_cast<Listview*>(propholder);
            if (listview->Virtual())
                return false;
            ListviewGroupEditorList editlist(listview);
            PropertyEditorDialog *diag = new PropertyEditorDialog();
            return diag->Show(parentform, &editlist);
        }
    };

    extern ValuePair<ListviewGroupStates> ListviewGroupStateStrings[];
    template<typename PropertyHolder>
    class ListviewGroupStateSetDesignProperty : public SetDesignProperty<PropertyHolder, ListviewGroupStates>
    {
    private:
        typedef SetDesignProperty<PropertyHolder, ListviewGroupStates>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListviewGroupStateSetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListviewGroupStateStrings)
        {}
    };

    template<typename PropertyHolder>
    class ListviewGroupIdDesignProperty : public GeneralValueListDesignProperty<PropertyHolder, int>
    {
    private:
        typedef GeneralValueListDesignProperty<PropertyHolder, int> base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListviewGroupIdDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::StringGetter stringgetter, typename base::ValueGetter valuegetter, GetterProc getter, SetterProc setter) : base(name, category, countgetter, stringgetter, valuegetter, new IntDesignProperty<PropertyHolder>(std::wstring(), std::wstring(), getter, setter))
        {}
    };

    extern ValuePair<CheckboxStates> CheckboxStateStrings[];
    template<typename PropertyHolder>
    class CheckboxStatesDesignProperty : public EnumDesignProperty<PropertyHolder, CheckboxStates>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, CheckboxStates>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        CheckboxStatesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, CheckboxStateStrings)
        {}
    };



    template<typename PropertyHolder>
    class GeneralEditboxDesignProperty : public GeneralControlByTypeDesignProperty<PropertyHolder, Edit>
    {
    private:
        typedef GeneralControlByTypeDesignProperty<PropertyHolder, Edit>    base;
    public:
        GeneralEditboxDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {
        }
    };

    template<typename PropertyHolder>
    class EditboxDesignProperty : public GeneralEditboxDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralEditboxDesignProperty<PropertyHolder> base;
    public:
        template<typename GetterProc, typename SetterProc>
        EditboxDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Edit*>(getter, setter))
        {}
    };


    extern ValuePair<ComboboxTypes> ComboboxTypeStrings[];
    template<typename PropertyHolder>
    class ComboboxTypesDesignProperty : public EnumDesignProperty<PropertyHolder, ComboboxTypes>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ComboboxTypes>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ComboboxTypesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ComboboxTypeStrings)
        {}
    };

    extern ValuePair<ListControlKinds> ListControlKindStrings[];
    template<typename PropertyHolder>
    class ListControlKindsDesignProperty : public EnumDesignProperty<PropertyHolder, ListControlKinds>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ListControlKinds>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListControlKindsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListControlKindStrings)
        {}
    };

    extern ValuePair<ListboxSelectionTypes> ListboxSelectionTypeStrings[];
    template<typename PropertyHolder>
    class ListboxSelectionTypesDesignProperty : public EnumDesignProperty<PropertyHolder, ListboxSelectionTypes>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, ListboxSelectionTypes>    base;
    public:
        template<typename GetterProc, typename SetterProc>
        ListboxSelectionTypesDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ListboxSelectionTypeStrings)
        {}
    };

    template<typename PropertyHolder>
    class GeneralControlElemListDesignProperty : public GeneralClassDesignProperty<PropertyHolder, ControlElemList&>
    {
    private:
        typedef GeneralClassDesignProperty<PropertyHolder, ControlElemList&>    base;
    protected:
        virtual std::wstring Value(Object *propholder)
        {
            const ControlElemList &list = this->CallGetter(propholder);
            int cnt = list.Count();
            if (!cnt)
                return L"Empty";
            else
                return IntToStr(cnt) + L" items";
        }
    public:
        GeneralControlElemListDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {
            this->propertystyle << psEditButton;
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            ControlElemList &list = this->CallGetter(propholder);
            if (list.NoData())
            {
                ShowMessageBox(L"Cannot edit the items of virtual controls.", L"Error", mbOk);
                return false;
            }

            std::vector<std::wstring> lines;
            list.GetLines(lines);
            LineEditorDialog dialog;
            if (dialog.Show(parentform, lines))
            {
                dialog.GetLines(lines);
                list.SetLines(lines);
                return true;
            }
            return false;
        }
    };

    template<typename PropertyHolder>
    class ControlElemListDesignProperty : public GeneralControlElemListDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralControlElemListDesignProperty<PropertyHolder>    base;
    public:
        template<typename GetterProc>
        ControlElemListDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter) : base(name, category, CreatePropertyReader<PropertyHolder, ControlElemList&>(getter, nullptr))
        {
        }
    };

    class ControlElemListItemsDesignProperty : public GeneralStringVectorDesignProperty<ControlElemList>
    {
    private:
        typedef GeneralStringVectorDesignProperty<ControlElemList>  base;
    public:
        ControlElemListItemsDesignProperty(const std::wstring &name) : base(name, &ControlElemList::Count, &ControlElemList::Text, &ControlElemList::DesignAdd)
        {
            this->DontList();
        }
    };

    extern ValuePair<StatusBarSizeGrips> StatusBarSizeGripStrings[];
    enum StatusBarSizeGrips : int;
    template<typename PropertyHolder>
    class StatusBarSizeGripsDesignProperty : public EnumDesignProperty<PropertyHolder, StatusBarSizeGrips>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, StatusBarSizeGrips>  base;
    public:
        template<typename GetterProc, typename SetterProc>
        StatusBarSizeGripsDesignProperty(const std::wstring &name, const std::wstring category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, StatusBarSizeGripStrings) 
        {
        }
    };

    extern ValuePair<StatusBarPartBevels> StatusBarPartBevelStrings[];
    enum StatusBarPartBevels : int;
    template<typename PropertyHolder>
    class StatusBarPartBevelsDesignProperty : public EnumDesignProperty<PropertyHolder, StatusBarPartBevels>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, StatusBarPartBevels>  base;
    public:
        template<typename GetterProc, typename SetterProc>
        StatusBarPartBevelsDesignProperty(const std::wstring &name, const std::wstring category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, StatusBarPartBevelStrings) 
        {
        }
    };

    class StatusBarPartEditorList : public PropertyEditorList
    {
    private:
        StatusBar *owner;

        std::vector<StatusBar::PartData> datalist;
        bool simpledata;
    public:
        StatusBarPartEditorList(StatusBar *owner) : owner(owner)
        {
            owner->SaveParts(datalist, simpledata);
        }

        virtual ~StatusBarPartEditorList()
        {
        }

        virtual PropertyEditorDialogButtonSet Buttons()
        {
            return (owner->PartCount() < 256 ? pedbAdd : 0) | pedbDelete | pedbMove;
        }

        virtual void Finished(bool canceled)
        {
            owner->RestoreSimple(simpledata);
            if (canceled)
                owner->RestoreParts(datalist, simpledata);
        }

        virtual int Count()
        {
            return owner->PartCount();
        }

        virtual void Move(int pos, int subpos, int diff)
        {
            owner->SwapParts(pos, pos + diff);
        }

        virtual int SubCount(int pos)
        {
            return 0;
        }

        virtual int Add()
        {
            return owner->AddPart(std::wstring(), taLeft, 60, sbpbLowered, false);
        }

        virtual int AddSub(int pos)
        {
            return -1;
        }

        virtual std::wstring Texts(int pos, int subpos)
        {
            return owner->PartText(pos);
        }

        virtual Object* Objects(int pos, int subpos)
        {
            StatusBarPart *i = owner->Parts(pos);
            return (Object*)i;
        }

        virtual void Delete(int pos, int subpos)
        {
            owner->DeletePart(pos);
        }

        virtual bool CanDelete(int pos, int subpos)
        {
            return true;
        }
    };

    std::wstring StatusBarPartArgs(Object *obj);
    template<typename PropertyHolder>
    class StatusBarPartDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, StatusBarPart*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, StatusBarPart*> base;
    public:
        StatusBarPartDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, category, countgetter, elemgetter, elemadder)
        {
            this->propertystyle << psEditButton;
            this->SetParentCreation(pcrtNoDeclare, &StatusBarPartArgs);
            //this->DontWrite();
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            StatusBarPartEditorList editlist(dynamic_cast<StatusBar*>(propholder));
            PropertyEditorDialog *diag = new PropertyEditorDialog();
            return diag->Show(parentform, &editlist);
        }
    };


}
/* End of NLIBNS */


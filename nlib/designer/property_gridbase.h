#pragma once

#include "designproperties.h"
#include "designerdialogs.h"
#include "gridbase.h"

//---------------------------------------------


namespace NLIBNS
{


    extern ValuePair<GridSelectionKinds> GridSelectionKindStrings[];
    template<typename PropertyHolder>
    class GridSelectionKindsDesignProperty: public EnumDesignProperty<PropertyHolder, GridSelectionKinds>
    {
    private:
        typedef EnumDesignProperty<PropertyHolder, GridSelectionKinds>  base;
    public:
        template<typename GetterProc, typename SetterProc>
        GridSelectionKindsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, GridSelectionKindStrings)
        {}
    };

    extern ValuePair<GridOptions> GridDrawOptionStrings[];
    template<typename PropertyHolder>
    class GridOptionsDesignProperty: public SetDesignProperty<PropertyHolder, GridOptions>
    {
    private:
        typedef SetDesignProperty<PropertyHolder, GridOptions>  base;
    public:
        template<typename GetterProc, typename SetterProc>
        GridOptionsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, GridDrawOptionStrings)
        {}
    };

    template<typename GridElemType>
    class GridElemEditorList : public PropertyEditorList
    {
    private:
        CustomGrid *owner;
        GridElemType *current;

        int fixcnt;
        int cnt;
        sparse_list<int> whlist;
        sparse_list<int> defwhlist;
        sparse_list<bool> sizlist;
        sparse_list<bool> vislist;
        sparse_list<std::pair<int, int>> order;
    public:
        GridElemEditorList(CustomGrid *owner) : owner(owner), current(NULL)
        {
            Save(current);
        }

        void Save(GridColumn*)
        {
            owner->SaveColumns();
        }

        void Save(GridRow*)
        {
            owner->SaveRows();
        }

        virtual ~GridElemEditorList()
        {
            if (current)
                current->Destroy();
            owner->FreeSaved();
        }

        virtual PropertyEditorDialogButtonSet Buttons()
        {
            return pedbAdd | pedbDelete;
        }

        virtual void Finished(bool canceled)
        {
            if (canceled)
                Restore(current);
        }

        void Restore(GridColumn*)
        {
            owner->RestoreColumns();
        }

        void Restore(GridRow*)
        {
            owner->RestoreRows();
        }

        virtual int Count()
        {
            return Count(current);
        }

        int Count(GridColumn*)
        {
            return owner->ColCount();
        }

        int Count(GridRow*)
        {
            return owner->RowCount();
        }

        virtual void Move(int pos, int subpos, int diff)
        {
            //owner->DesignMoveColumn(pos, pos + diff);
        }

        virtual int SubCount(int pos)
        {
            return 0;
        }

        virtual int Add()
        {
            return Add(current);
        }
    
        int Add(GridColumn*)
        {
            owner->SetColCount(owner->ColCount() + 1);
            return owner->ColCount() - 1;
        }

        int Add(GridRow*)
        {
            owner->SetRowCount(owner->RowCount() + 1);
            return owner->RowCount() - 1;
        }

        virtual int AddSub(int pos)
        {
            return 0;
        }

        virtual std::wstring Texts(int pos, int subpos)
        {
            return Texts(pos, current);
        }

        std::wstring Texts(int pos, GridColumn*)
        {
            return L"Column[" + IntToStr(pos) + L"]";
        }

        std::wstring Texts(int pos, GridRow*)
        {
            return L"Row[" + IntToStr(pos) + L"]";
        }

        virtual Object* Objects(int pos, int subpos)
        {
            if (current)
                current->Destroy();
            CreateCurrent(pos, current);
            return current;
        }

        void CreateCurrent(int pos, GridColumn* &c)
        {
            c = new GridColumn(owner, pos);
        }

        void CreateCurrent(int pos, GridRow* &c)
        {
            c = new GridRow(owner, pos);
        }

        virtual void Delete(int pos, int subpos)
        {
            if (current && current->Index() == pos)
            {
                current->Destroy();
                current = NULL;
            }
            Delete(pos, current);
        }

        void Delete(int pos, GridColumn*)
        {
            owner->DeleteColumn(pos);
        }

        void Delete(int pos, GridRow*)
        {
            owner->DeleteRow(pos);
        }

        virtual bool CanDelete(int pos, int subpos)
        {
            return CanDelete(pos, current);
        }

        bool CanDelete(int pos, GridColumn*)
        {
            return owner->ColCount() > 1;
        }

        bool CanDelete(int pos, GridRow*)
        {
            return owner->RowCount() > 1;
        }

    };

    template<typename PropertyHolder, typename GridElemType>
    class GridElemDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, GridElemType*>
    {
    private:
        typedef GeneralClassVectorDesignProperty<PropertyHolder, GridElemType*> base;
    public:
        GridElemDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, category, countgetter, elemgetter, elemadder)
        {
            this->propertystyle << psEditButton;
            this->DontWrite();
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            GridElemEditorList<GridElemType> editlist(dynamic_cast<CustomGrid*>(propholder));
            PropertyEditorDialog *diag = new PropertyEditorDialog();
            return diag->Show(parentform, &editlist);
        }
    };

    //class ListGridColumnEditorList : public PropertyEditorList
    //{
    //private:
    //    ListGrid *owner;
    //
    //    std::vector<ListGridColumn*> cols;
    //public:
    //    ListGridColumnEditorList(ListGrid *owner) : owner(owner)
    //    {
    //        owner->SaveColumns(cols);
    //    }
    //
    //    virtual ~ListGridColumnEditorList()
    //    {
    //        int cnt = cols.size();
    //        for (int ix = 0; ix < cnt; ++ix)
    //            cols[ix]->Destroy();
    //    }
    //
    //    virtual PropertyEditorDialogButtonSet Buttons()
    //    {
    //        return pedbAdd | pedbDelete | pedbMove;
    //    }
    //
    //    virtual void Finished(bool canceled)
    //    {
    //        if (canceled)
    //            owner->RestoreColumns(cols);
    //    }
    //
    //    virtual int Count()
    //    {
    //        return owner->ColumnCount();
    //    }
    //
    //    virtual void Move(int pos, int subpos, int diff)
    //    {
    //        owner->DesignMoveColumn(pos, pos + diff);
    //    }
    //
    //    virtual int SubCount(int pos)
    //    {
    //        return 0;
    //    }
    //
    //    virtual int Add()
    //    {
    //        return owner->AddColumn()->Index();
    //    }
    //
    //    virtual int AddSub(int pos)
    //    {
    //        return 0;
    //    }
    //
    //    virtual std::wstring Texts(int pos, int subpos)
    //    {
    //        return owner->Columns(pos)->Text();
    //    }
    //
    //    virtual Object* Objects(int pos, int subpos)
    //    {
    //        return owner->Columns(pos);
    //    }
    //
    //    virtual void Delete(int pos, int subpos)
    //    {
    //        owner->DeleteColumn(pos);
    //    }
    //
    //    virtual bool CanDelete(int pos, int subpos)
    //    {
    //        return true;
    //    }
    //};
    //
    //template<typename PropertyHolder>
    //class ListGridColumnVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ListGridColumn*>
    //{
    //private:
    //    typedef GeneralClassVectorDesignProperty<PropertyHolder, ListGridColumn*>  base;
    //public:
    //    ListGridColumnVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
    //    {
    //        this->propertystyle << psEditButton;
    //        this->SetParentCreation(true);
    //    }
    //
    //    virtual bool ClickEdit(Form *parentform, Object *propholder)
    //    {
    //        ListGridColumnEditorList editlist(dynamic_cast<ListGrid*>(propholder));
    //        PropertyEditorDialog *diag = new PropertyEditorDialog();
    //        return diag->Show(parentform, &editlist);
    //    }
    //
    //    //virtual void DeclaredNames(Object *propholder, std::vector<std::pair<std::wstring, std::wstring>> &names, AccessLevels access)
    //    //{
    //    //    int cnt = this->Count(propholder);
    //    //    for (int ix = 0; ix < cnt; ++ix)
    //    //        names.push_back(std::make_pair(L"ListGridColumn", this->Items(propholder, ix)->Name()));
    //    //}
    //};
    //
    //class ListGridRowEditorList : public PropertyEditorList
    //{
    //private:
    //    ListGrid *owner;
    //
    //    std::vector<ListGridRow*> rows;
    //public:
    //    ListGridRowEditorList(ListGrid *owner) : owner(owner)
    //    {
    //        owner->SaveRows(rows);
    //    }
    //
    //    virtual ~ListGridRowEditorList()
    //    {
    //        int cnt = rows.size();
    //        for (int ix = 0; ix < cnt; ++ix)
    //            rows[ix]->Destroy();
    //    }
    //
    //    virtual PropertyEditorDialogButtonSet Buttons()
    //    {
    //        return pedbAdd | pedbDelete | pedbMove | pedbAddSub | pedbMoveSub;
    //    }
    //
    //    virtual void Finished(bool canceled)
    //    {
    //        if (canceled)
    //            owner->RestoreRows(rows);
    //    }
    //
    //    virtual int Count()
    //    {
    //        return owner->RowCount();
    //    }
    //
    //    virtual void Move(int pos, int subpos, int diff)
    //    {
    //        if (subpos < 0)
    //            owner->MoveRow(pos, pos + diff);
    //        else
    //            owner->Rows(pos)->MoveSub(subpos, subpos + diff);
    //    }
    //
    //    virtual int SubCount(int pos)
    //    {
    //        return owner->Rows(pos)->SubCount();
    //    }
    //
    //    virtual int Add()
    //    {
    //        int r = owner->AddRow()->Index();
    //        return r;
    //    }
    //
    //    virtual int AddSub(int pos)
    //    {
    //        return owner->Rows(pos)->InsertText(-1, std::wstring());
    //    }
    //
    //    virtual std::wstring Texts(int pos, int subpos)
    //    {
    //        if (subpos >= 0)
    //            return owner->Rows(pos)->Text(subpos);
    //        return std::wstring(L"Row ") + IntToStr(pos);
    //    }
    //
    //    virtual Object* Objects(int pos, int subpos)
    //    {
    //        if (subpos < 0)
    //            return owner->Rows(pos);
    //        return owner->Rows(pos)->SubObjects(subpos);
    //    }
    //
    //    virtual void Delete(int pos, int subpos)
    //    {
    //        if (subpos < 0)
    //            owner->DeleteRow(pos);
    //        else
    //            owner->Rows(pos)->DeleteSub(subpos);
    //    }
    //
    //    virtual bool CanDelete(int pos, int subpos)
    //    {
    //        return true;
    //    }
    //};
    //
    //template<typename PropertyHolder>
    //void ListGridRowItemsSerializer(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum);
    //
    //template<typename PropertyHolder>
    //class ListGridRowVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ListGridRow*>
    //{
    //private:
    //    typedef GeneralClassVectorDesignProperty<PropertyHolder, ListGridRow*>  base;
    //    //typedef ListGridRowItemsSerializer<PropertyHolder> ItemSerializer;
    //public:
    //    ListGridRowVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
    //    {
    //        this->propertystyle << psEditButton;
    //        this->SetSerializerFunc(&ListGridRowItemsSerializer<PropertyHolder>);
    //        //this->ForceSerialize();
    //    }
    //
    //    virtual bool ClickEdit(Form *parentform, Object *propholder)
    //    {
    //        ListGridRowEditorList editlist(dynamic_cast<ListGrid*>(propholder));
    //        PropertyEditorDialog *diag = new PropertyEditorDialog();
    //        return diag->Show(parentform, &editlist);
    //    }
    //};
    //
    //template<typename PropertyHolder>
    //void ListGridRowItemsSerializer(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum)
    //{
    //    ListGridRowVectorDesignProperty<PropertyHolder> *p = dynamic_cast<ListGridRowVectorDesignProperty<PropertyHolder>*>(prop);
    //    if (!p)
    //        return;
    //
    //    int cnt = p->Count(propholder);
    //
    //    DesignSerializer *serializer = propholder->Serializer();
    //    int pix = SerializerPropertyIndex(serializer, prop);
    //
    //    if (!cnt)
    //        return;
    //
    //    for (int ix = 0; ix < cnt; ++ix)
    //    {
    //        std::wstring rowname = std::wstring(L"listgridrow") + IntToStr(DesignerNextMemberIndex(L"listgridrow"));
    //        ws << indent << L"ListGridRow *" << rowname << L";" << std::endl;
    //
    //        ws << indent << rowname << L" = " << prefix << (pointerprefix ? L"->" : L".") << SerializerNames(serializer, pix) << L"();" << std::endl;
    //        ListGridRow *row = p->Items(propholder, ix);
    //        int cnt2 = row->SubCount();
    //        ws << indent << rowname << L"->SubResize(" << cnt2 << L");" << std::endl;
    //        for (int iy = 0; iy < cnt2; ++iy)
    //        {
    //            ws << indent << rowname << L"->SetText(" << iy << L", L\"" << EscapeCString(row->Text(iy)) << L"\");" << std::endl;
    //        }
    //    }
    //}
    //
    //template<typename PropertyHolder>
    //class ListGridRowSubVectorDesignProperty : public GeneralClassVectorDesignProperty<PropertyHolder, ListGridRowSub*>
    //{
    //private:
    //    typedef GeneralClassVectorDesignProperty<PropertyHolder, ListGridRowSub*>  base;
    //public:
    //    ListGridRowSubVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
    //    {
    //        this->DontList()->DontExport();
    //    }
    //};


}
/* End of NLIBNS */


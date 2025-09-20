#include "stdafx_zoli.h"
#include "application.h"
#include "utility.h"
//#include <regex>

#include "designer.h"
#include "designerform.h"
#include "designercontrols.h"
#include "canvas.h"
#include "imagelist.h"
#include "buttons.h"
#include "graphiccontrol.h"
#include "gridbase.h"

#include "themes.h"

#include "designer.h"
#include "designproperties.h"
#include "property_generalcontrol.h"
#include "property_menu.h"
#include "serializer.h"
#include "designermenu.h"

#include "dialog.h"
#include "objectbase.h"
#include "designregistry.h"

#include "comdata.h"


#define amPlaceSizers       (WM_APP + 3)
#define amChildCreated      (WM_APP + 4)
#define amHijackChild       (WM_APP + 5)


//---------------------------------------------


namespace NLIBNS
{
    extern Imagelist *ilAlign;

    ColorDialog *colordialog = NULL;

    const int gridsize = 8;
    const bool snaptogrid = true;

    //---------------------------------------------

    typedef std::list<HWND> HWNDList;

    LRESULT CALLBACK SubcontrolReplacementProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DesignForm::Subcontrol *sub = (DesignForm::Subcontrol*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!sub)
            throw L"?";

        if (uMsg == WM_NCHITTEST)
        {
            bool nohittest = true;
            if ((GetKeyState(VK_RBUTTON) & (1 << 15)) == 0 && !sub->parentform->sizing && sub->parentform->seltype == DesignForm::stNone && !sub->parentform->dragging && !sub->parentform->placing)
            {
                Control *c = sub->owner->control;
                Point pt = c->ScreenToWindow(Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                if (c->NeedDesignerHittest(pt.x, pt.y, HTNOWHERE))
                    nohittest = false;
            }
            if (nohittest)
                return HTTRANSPARENT;
        }

        LRESULT result = 0;

        if (uMsg == WM_PAINT && sub->owner->selected)
        {
            Rect r;
            if (GetUpdateRect(hwnd,&r,FALSE))
            {
                PAINTSTRUCT ps = {0};
                HDC paintdc = BeginPaint(hwnd,&ps);
                ControlCanvas *canvas = NULL;
                if (!paintdc)
                    return 1;
                try
                {
                    result = CallWindowProc(sub->proc, hwnd, uMsg, (WPARAM)paintdc, lParam);

                    Rect r2 = ps.rcPaint;
                    canvas = new ControlCanvas(paintdc);
                    if (r2.Empty())
                        ps.rcPaint = r;

                    canvas->SetClip(ps.rcPaint);

                    if (!canvas->ClipEmpty())
                    {
                        Rect cr;
                        GetClientRect(hwnd, &cr);
                        cr = cr.Intersect(ps.rcPaint);
                        bool mainsel = sub->parentform->LastSelected() == sub->owner;
                        if (sub->parentform->ActiveDesigner() && application->Active())
                            canvas->SetBrush(Color(mainsel ? clHotlight : clHighlight).SetA(mainsel ? 70 : 50));
                        else
                            canvas->SetBrush(Color(mainsel ? clBlack : clGray).SetA(60));
                        canvas->FillRect(cr);
                    }
                    EndPaint(hwnd,&ps);
                }
                catch(...)
                {
                    paintdc = NULL;
                    EndPaint(hwnd,&ps);
                    delete canvas;
                    throw;
                }
                paintdc = NULL;
                delete canvas;
            }
        }

        if (uMsg == WM_DESTROY)
        {
            // The window shouldn't be destroyed while its proc is replaced.
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)&sub->proc);
            sub->owner->subcontrols.erase(std::find(sub->owner->subcontrols.begin(), sub->owner->subcontrols.end(), sub));
            auto proc = sub->proc;
            delete sub;
            return CallWindowProc(proc, hwnd, uMsg, wParam, lParam);
        }

        result = CallWindowProc(sub->proc, hwnd, uMsg, wParam, lParam);
        return result;
    }


    //---------------------------------------------


    DesignSizer::DesignSizer(DesignForm *form, DesignSizerSides side) : base(), form(form), side(side)
    {
        controlstyle -= csMouseCapture;

        Hide();

        switch (side)
        {
            case dssLeft:
            case dssRight:
                SetCursor(cSizeWE);
            break;
            case dssTop:
            case dssBottom:
                SetCursor(cSizeNS);
            break;
            case dssTopLeft:
            case dssBottomRight:
                SetCursor(cSizeNWSE);
            break;
            case dssTopRight:
            case dssBottomLeft:
                SetCursor(cSizeNESW);
            break;
        }

        SetParent(form);

        if (form->DesignMain())
            form->DesignMain()->MoveToTop();
    }

    void DesignSizer::PlaceOnRect(Rect r)
    {
        int w, h;
        switch (side)
        {
            case dssLeft:
            case dssRight:
                h = max(3, min(r.Height() / 4, r.Height() - 14 * Scaling));

                SetBounds(RectS(side == dssLeft ? r.left - 2 : r.right - 2, r.top + (r.Height() - h) / 2, 4, h));
            break;
            case dssTop:
            case dssBottom:
                w = max(3, min(r.Width() / 4, r.Width() - 14 * Scaling));

                SetBounds(RectS(r.left + (r.Width() - w) / 2, side == dssTop ? r.top - 2 : r.bottom - 2, w, 4));
            break;
            default:
                SetBounds(RectS(side == dssTopLeft || side == dssBottomLeft ? r.left - 3 : r.right - 4, side == dssTopLeft || side == dssTopRight ? r.top - 3 : r.bottom - 4, 7, 7));
            break;
        }
    }

    void DesignSizer::Paint(const Rect &updaterect)
    {
        Canvas *c = GetCanvas();

        Rect r = ClientRect();

        c->SetAntialias(true);

        Color c1 = form->ActiveDesigner() ? Color(/*170,*/ 80, 120, 170) : Color(/*170,*/ 210, 210, 210);
        Color c2 = form->ActiveDesigner() ? Color(/*170,*/ 110, 170, 210) : Color(/*170,*/ 200, 200, 200);
        Color c3 = form->ActiveDesigner() ? clWhite : Color(/*170,*/ 200, 200, 200);
        Color c4 = form->ActiveDesigner() ? Color(/*170,*/ 15, 50, 70) : Color(/*170,*/ 160, 160, 160);

        c->SetBrush(r, c1, c2, side == dssTop || side == dssBottom ? 90 : 0);

        if (side == dssLeft || side == dssRight || side == dssTop || side == dssBottom)
            c->FillRect(r);
        else
            c->FillRoundRect(r, 3, 3);

        c->SetAntialias(false);
        if (side == dssLeft || side == dssRight || side == dssTop || side == dssBottom)
        {
            c->SetPen(c3);
            if (side == dssLeft)
                c->Line(r.right - 2, r.top, r.right - 2, r.bottom);
            if (side == dssRight)
                c->Line(r.left + 1, r.top, r.left + 1, r.bottom);
            if (side == dssTop)
                c->Line(r.left, r.bottom - 2, r.right, r.bottom - 2);
            if (side == dssBottom)
                c->Line(r.left, r.top + 1, r.right, r.top + 1);
        }
        else
        {
            c->SetPen(c3);
            if (side == dssTopLeft || side == dssBottomLeft)
                c->Line(3, 3, 5, 3);
            else
                c->Line(1, 3, 3, 3);
            if (side == dssTopLeft || side == dssTopRight)
                c->Line(3, 3, 3, 5);
            else
                c->Line(3, 1, 3, 3);

        }
        c->SetAntialias(true);

        c->SetPen(c4);
        if (side == dssLeft || side == dssRight || side == dssTop || side == dssBottom)
            c->FrameRect(r);
        else
            c->RoundFrameRect(r, 3, 3);
    }

    void DesignSizer::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        base::MouseDown(x, y, button, vkeys);
        if (button != mbLeft)
            return;

        SendMessage(Handle(), WM_LBUTTONUP, 0, MAKELPARAM(x,y));
        form->StartSizing(side, form->ScreenToClient(ClientToScreen(Point(x, y))) );
    }


    //---------------------------------------------


    // Properties
    template<typename PropertyHolder, typename GetterPropertyType = const std::wstring&, typename SetterPropertyType = const std::wstring&>
    class UnitNameDesignProperty : public GeneralFileNameDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralFileNameDesignProperty<PropertyHolder> base;
    public:
        template<typename GetterProc, typename SetterProc>
        UnitNameDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) :
                base(name, category, true, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
        {
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            std::wstring str = trim(val);
            if (((dynamic_cast<PropertyHolder*>(propholder))->Name() != val && designer->FormNameTaken(val)) ||
                ((dynamic_cast<PropertyHolder*>(propholder))->UnitName() != val && designer->UnitNameTaken(val)))
                return false;
            return base::SetValue(parentform, propholder, val);
        }
    };


    //---------------------------------------------


    DesignFormBase::EventListItem::EventListItem(const std::wstring &eventtype, const std::wstring &eventname, Object *obj, const std::wstring &funcname) : type(eventtype), func(funcname)
    {
        //const int nliblen = wcslen(NLIB_TOSTRING(NLIBNS) L"::");
        //if (type.compare(0, nliblen, NLIB_TOSTRING(NLIBNS) L"::") == 0)
        //    type.erase(0, nliblen);
        events.push_back(EventNameObjPair(eventname, obj));
    }

    DesignFormBase::EventListItem::EventListItem(const std::wstring &eventtype, const std::wstring &funcname, const std::wstring exportedfunc) : type(eventtype), func(funcname), exportedfunc(exportedfunc)
    {
        //const int nliblen = wcslen(NLIB_TOSTRING(NLIBNS) L"::");
        //if (type.compare(0, nliblen, NLIB_TOSTRING(NLIBNS) L"::") == 0)
        //    type.erase(0, nliblen);
    }


    //---------------------------------------------


    void DesignFormBase::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->SetNonVisualContainerControl(true);

        serializer->Add(std::wstring(), new UnitNameDesignProperty<DesignFormBase>(L"UnitName", std::wstring(), &DesignFormBase::UnitName, &DesignFormBase::SetUnitName))->SetDefault(std::wstring())->SetPriority(999)->DontExport();
        serializer->Add(std::wstring(), new BoolDesignProperty<DesignFormBase>(L"DesignVisible", std::wstring(), &DesignFormBase::Visible, &DesignFormBase::SetDesignVisible))->DontList()->DontExport();
        serializer->Add(std::wstring(), new StringDesignProperty<DesignFormBase>(L"LastExportedName", std::wstring(), &DesignFormBase::LastExportedName, &DesignFormBase::SetLastExportedName))->DontList()->DontExport()->SetDefault(std::wstring());
        serializer->Add(std::wstring(), new StringDesignProperty<DesignFormBase>(L"LastExportedEvents", std::wstring(), &DesignFormBase::LastExportedEvents, &DesignFormBase::SetLastExportedEvents))->DontList()->DontExport()->SetDefault(std::wstring());
        serializer->Add(std::wstring(), new BoolDesignProperty<DesignFormBase>(L"AutoCreate", std::wstring(), &DesignFormBase::AutoCreate, &DesignFormBase::SetAutoCreate))->SetDefault(false)->DontExport();
        serializer->Add(std::wstring(), new UnsignedIntDesignProperty<DesignFormBase>(L"CreationOrder", std::wstring(), &DesignFormBase::CreationOrder, &DesignFormBase::SetCreationOrder))->DontExport();
        serializer->Add(std::wstring(), new StringDesignProperty<DesignFormBase>(L"GuestControls", std::wstring(), &DesignFormBase::GetGuestControls, &DesignFormBase::AddGuestControls))->DontList()->DontExport()->SetDefault(std::wstring());
    }

    void DesignFormBase::Destroy()
    {
        CloseEditedMenu();

        //for (DesignMenu *mnu : designmenus)
        //    mnu->Destroy();

        //for(DesignPopupMenu *mnu : designpopups)
        //    mnu->Destroy();

        if (designmain != nullptr)
            designmain->Destroy();
        designmain = nullptr;

        base::Destroy();
    }

    bool DesignFormBase::IsDesignVisible()
    {
        return designvis;
    }

    void DesignFormBase::SetDesignVisible(bool newvis)
    {
        designvis = newvis;
    }

    DesignFormBase::DesignFormBase() : base(), designmain(nullptr), editedmenu(nullptr), designvis(true), autocreate(false)
    {
        SetWantedKeyTypes(wkArrows | wkTab | wkEnter | wkEscape | wkOthers);
    }

    LRESULT DesignFormBase::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
        case wmMenuAccelerator:
            if (designer)
                return designer->PassMessage(uMsg, wParam, lParam);
            break;
        case wmThemeChanged:
            if (designmain)
                designmain->PassMessage(wmThemeChanged, 0, 0);
            if (editedmenu && editedmenu != designmain)
                editedmenu->PassMessage(wmThemeChanged, 0, 0);
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void DesignFormBase::DeleteNotify(Object *object)
    {
        if (object == editedmenu)
        {
            editedmenu = nullptr;
            UpdateSelection();
        }
        base::DeleteNotify(object);
    }

    void DesignFormBase::ChangeNotify(Object *object, int changetype)
    {
        if (changetype == CHANGE_PARENT)
        {
            //if (dynamic_cast<Menubar*>(object) != nullptr)
            //{
            //    if (((Menubar*)object)->/*IsParent(this) */ParentForm() == this)
            //    {
            //        if (std::find_if(designmenus.begin(), designmenus.end(), [object](DesignMenu *menu) { return menu->Menu() == object; }) == designmenus.end())
            //            designmenus.push_back(new DesignMenu(this, (Menubar*)object));
            //    }
            //    else
            //    {
            //        auto it = std::find_if(designmenus.begin(), designmenus.end(), [object](DesignMenu *menu) { return menu->Menu() == object; });
            //        if (it != designmenus.end())
            //            designmenus.erase(it);
            //    }
            //}
            //else if (dynamic_cast<PopupMenu*>(object) != nullptr)
            //{
            //    if (((PopupMenu*)object)->/*IsParent(this)*/ParentForm() == this)
            //    {
            //        if (std::find_if(designpopups.begin(), designpopups.end(), [object](DesignPopupMenu *menu) { return menu->Menu() == object; }) == designpopups.end())
            //            designpopups.push_back(new DesignPopupMenu(this, (PopupMenu*)object));
            //    }
            //    else
            //    {
            //        auto it = std::find_if(designpopups.begin(), designpopups.end(), [object](DesignPopupMenu *menu) { return menu->Menu() == object; });
            //        if (it != designpopups.end())
            //            designpopups.erase(it);
            //    }
            //}
        }

        base::ChangeNotify(object, changetype);
    }

    void DesignFormBase::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        base::KeyPush(keycode, key, vkeys);

        if (editedmenu)
            editedmenu->KeyPush(keycode, key, vkeys);
        if (key >= 0x20)
            designer->DesignKeyPress(key);
    }

    void DesignFormBase::KeyUp(WORD &keycode, VirtualKeyStateSet vkeys)
    {
        base::KeyUp(keycode, vkeys);

        if (editedmenu)
            editedmenu->KeyUp(keycode, vkeys);
    }

    void DesignFormBase::CloseEditedMenu()
    {
        if (!editedmenu)
            return;
        editedmenu->Close();
        if (editedmenu != designmain)
            editedmenu->Destroy();
        editedmenu = nullptr;
    }

    void DesignFormBase::EditingMenu(TopDesignMenuItems *menu)
    {
        if (editedmenu != menu && editedmenu != nullptr && editedmenu != designmain)
            editedmenu->Destroy();
        editedmenu = menu;
    }

    void DesignFormBase::EditPopupMenu(PopupMenu *menu, MenuItem *item)
    {
        if (editedmenu && editedmenu->Menu() == menu)
        {
            if (item != nullptr)
                editedmenu->SelectSubMenu(editedmenu->GetSubItem(item));
            return;
        }

        CloseEditedMenu();

        if (!menu)
            return;

        editedmenu = new DesignPopupMenu(this, menu);

        if (editedmenu)
        {
            if (!editedmenu->HasAddButton())
                editedmenu->CreateSubmenu(false);
            editedmenu->Show();
            AddToNotifyList(editedmenu, nrRelation3);

            if (item)
                editedmenu->SelectSubMenu(editedmenu->GetSubItem(item));
        }
    }

    void DesignFormBase::EditDesignMenu(Menubar *menubar, MenuItem *item)
    {
        if (designmain && designmain->Menu() == menubar)
        {
            if (item != nullptr)
            {
                if (editedmenu && editedmenu != designmain)
                    editedmenu->Destroy();
                editedmenu = designmain;
                editedmenu->SelectSubMenu(editedmenu->GetSubItem(item));
            }
            return;
        }

        int menuheight = GetSystemMetrics(SM_CYMENU);

        if (designmain)
            designmain->Destroy();

        if (menubar)
        {
            if (Visible())
                BeginUpdate();

            SetInnerPadding(Rect(0, menuheight, 0, 0));
            if (designmain == nullptr) // Move everything down by menu height pixels, so they act like the menu is part of the border not the client area.
            {
                for (int ix = 0; ix < Form::ControlCount(); ++ix)
                {
                    Control *c = Form::Controls(ix);
                    if (dynamic_cast<DesignMenu*>(c) != nullptr)
                        continue;

                    if (c->Alignment() != alNone && (c->Alignment() != alAnchor || !c->Anchors().contains(caTop)))
                        continue;
                    Rect r = c->WindowRect();
                    if (c->Alignment() != alAnchor || !c->Anchors().contains(caBottom))
                        r = r.Offset(0, menuheight);
                    else
                    {
                        r = r.Inflate(0, -menuheight, 0, 0);
                        if (r.bottom < r.top)
                            r.bottom = r.top;
                    }
                    c->SetBounds(r);
                }
                LayoutChildren(ClientRect(), true);
            }

            designmain = new DesignMenu(this, menubar);
            if (!designmain->HasAddButton())
                designmain->CreateSubmenu(false);

            designmain->Show();
            if (Visible())
            {
                EndUpdate();
                RedrawWindow(Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            }

            if (item)
            {
                if (editedmenu && editedmenu != designmain)
                    editedmenu->Destroy();
                editedmenu = designmain;
                editedmenu->SelectSubMenu(editedmenu->GetSubItem(item));
            }
        }
        else
        {
            if (!designmain)
                return;

            designmain = nullptr;

            if (Visible())
                BeginUpdate();
            SetInnerPadding(Rect());

            // Move controls up menu height pixels so they take up their normal position on the form.
            for (int ix = 0; ix < Form::ControlCount(); ++ix)
            {
                Control *c = Form::Controls(ix);
                if (dynamic_cast<DesignMenu*>(c) != nullptr)
                    continue;

                if (c->Alignment() != alNone && (c->Alignment() != alAnchor || !c->Anchors().contains(caTop)))
                    continue;
                Rect r = c->WindowRect();
                if (c->Alignment() != alAnchor || !c->Anchors().contains(caBottom))
                    r = r.Offset(0, -menuheight);
                else
                {
                    r = r.Inflate(0, menuheight, 0, 0);
                    if (r.bottom < r.top)
                        r.bottom = r.top;
                }

                c->SetBounds(r);
            }
            LayoutChildren(ClientRect(), true);
            if (Visible())
            {
                EndUpdate();
                RedrawWindow(Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            }
        }

        if (designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"ClientHeight");
    }

    DesignMenu* DesignFormBase::DesignMain()
    {
        return designmain;
    }

    //bool DesignFormBase::EditingPopupMenu()
    //{
    //    return designpopup != nullptr;
    //}

    //void DesignFormBase::AddSelectedPopupItems(std::list<Object*> &sellist)
    //{
    //    if (!designpopup)
    //        return;
    //
    //    int cnt = designpopup->SelCount();
    //    for (int ix = 0; ix < cnt; ++ix)
    //        sellist.push_back(designpopup->SelItems(ix));
    //}

    TopDesignMenuItems* DesignFormBase::EditedMenu()
    {
        return editedmenu;
    }

    //DesignMenu* DesignFormBase::GetDesignMenu(Menubar *menu)
    //{
    //    auto it = std::find_if(designmenus.begin(), designmenus.end(), [menu](DesignMenu *m) { return m->Menu() == menu; });
    //    if (it == designmenus.end())
    //        return NULL;
    //    return *it;
    //}

    bool DesignFormBase::GuestReferenceOn(DesignFormBase *guestparent)
    {
        std::set<DesignFormBase*> checked;
        return GuestReferenceOn(guestparent, checked);
    }

    void DesignFormBase::WindowBoundsChanged(const Rect &oldrect, const Rect &newrect)
    {
        base::WindowBoundsChanged(oldrect, newrect);

        if (editedmenu)
            editedmenu->UpdateBounds();
    }

    bool DesignFormBase::AutoCreate()
    {
        return autocreate;
    }

    void DesignFormBase::SetAutoCreate(bool newauto)
    {
        autocreate = newauto;
    }

    unsigned int DesignFormBase::CreationOrder()
    {
        return project->FormCreationOrder(this);
    }

    void DesignFormBase::SetCreationOrder(unsigned int neworder)
    {
        project->SetFormCreationOrder(this, neworder);
    }

    void DesignFormBase::Modify(bool tomodified)
    {
        if (modified == tomodified)
            return;
        modified = tomodified;
        if (modified)
            designer->Modify();
    }

    bool DesignFormBase::Modified()
    {
        return modified;
    }

    void DesignFormBase::SetName(const std::wstring& newname)
    {
        std::wstring oldname = Name();
        base::SetName(newname);
        if (designer->ActiveForm() == this)
            designer->FormNameChanged(newname);
        RenameEvents(this, oldname, newname);
    }

    void DesignFormBase::RenameEvents(Object *obj, std::wstring oldname, std::wstring newname)
    {
        if (oldname.empty() || newname.empty())
            return;

        // Collect all events whose name starts with the object's new name to avoid collision.
        std::vector<std::wstring> taken;
        for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
            if (it->func.find(newname) == 0)
                taken.insert(taken.end(), it->func)->replace(0, newname.length(), oldname); // Inserting name as it would still be the old function name, for easier equality check in the next round.

        // Rename events if their names don't collide.
        for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
        {
            if (it->func.find(oldname) != 0 || it->func.length() == oldname.length()) // Name doesn't start with object's old name.
                continue;
            std::wstring eventstr = it->func.substr(oldname.length()); // Name of the event from OnEventstr.

            bool found = false;
            for (auto it2 = it->events.begin(); it2 != it->events.end(); ++it2) // See whether the event is assigned to this object and it has the correct name.
            {
                if (it2->second != obj || it2->first.find(L"On") != 0 || eventstr.length() != it2->first.length() - 2 || it2->first.find(eventstr) != 2) // Either the object or the event name doesn't match.
                    continue;
                found = true;
            }
            if (!found)
                continue;

            if (std::find(taken.begin(), taken.end(), it->func) != taken.end()) // Old and new names would collide.
                continue;

            it->func.replace(0, oldname.length(), newname); // Replace the old name with the new.

            if (designer)
            {
                for (auto it2 = it->events.begin(); it2 != it->events.end(); ++it2)
                    if (designer->IsPropertyOwner(it2->second))
                        designer->InvalidateEventRows(it2->first);
            }
        }
    }

    void DesignFormBase::NameChangeNotify(Object *obj, const std::wstring &oldname)
    {
        base::NameChangeNotify(obj, oldname);

        if (!InNotifyList(obj, nrOwnership) || !obj->Designing() || designstate.contains(dsDeserializing))
            return;

        // Change event names based on the name of the object. If an event's name starts with the object's old name,
        // that part of the name will be replaced with the new name, unless there is already an event with the same name.

        RenameEvents(obj, oldname, obj->Name());
    }

    void DesignFormBase::NamesList(std::vector<std::wstring> &namelist)
    {
        for (SubObject &sub : subobjects)
            namelist.push_back(sub.name);
    }

    bool DesignFormBase::NameTaken(const std::wstring &name)
    {
        return NameOwner(name) != NULL;
    }

    bool DesignFormBase::FormNameTaken(const std::wstring &name)
    {
        return designer->FormNameTaken(name);
    }

    void DesignFormBase::RegisterSubObject(Object *owner, Object *subobj, const std::wstring &subobjname)
    {
        if (std::find_if(subobjects.begin(), subobjects.end(), [owner, subobj](SubObject &sub) { return sub.owner == owner && sub.obj == subobj; }) != subobjects.end())
            return;

        subobjects.push_back(SubObject(owner, subobj, subobjname));
        if (subobj->SubShown() && designer && designer->ActiveForm() == this)
            designer->ControlAdded(subobjects.back().obj, ControlName(subobjects.back().owner, subobjname));
    }

    void DesignFormBase::UnregisterSubObject(Object *owner, Object *subobj)
    {
        auto it = std::find_if(subobjects.begin(), subobjects.end(), [owner, subobj](const SubObject &sub) { return sub.owner == owner && sub.obj == subobj; });
        if (it == subobjects.end())
            return;
        if (it->obj->SubShown() && designer && designer->ActiveForm() == this)
            designer->ControlDeleted(it->obj, ControlName(it->owner, it->name));
        subobjects.erase(it);
    }

    void DesignFormBase::SubObjectRenamed(Object *owner, Object *subobj, const std::wstring &oldname, const std::wstring &newname)
    {
        if (oldname == newname)
            return;

        auto it = std::find_if(subobjects.begin(), subobjects.end(), [owner, subobj](const SubObject &sub) { return sub.owner == owner && sub.obj == subobj; });

        if (it == subobjects.end())
            return;

        if (it->obj->SubShown() && designer && designer->ActiveForm() == this)
            designer->ControlNameChanged(it->obj, ControlName(it->owner, oldname), ControlName(it->owner, newname));

        it->name = newname;

        RenameEvents(subobj, oldname, newname);
    }

    std::wstring DesignFormBase::SubObjectName(Object *owner, Object *subobj)
    {
        auto it = std::find_if(subobjects.begin(), subobjects.end(), [owner, subobj](const SubObject &sub) { return sub.owner == owner && sub.obj == subobj; });
        if (it == subobjects.end())
            return std::wstring();
        return it->name;
    }

    bool DesignFormBase::SelectSubObject(const std::wstring& name)
    {
        for (SubObject &sub : subobjects)
        {
            if (!sub.obj->SubShown() || ControlName(sub.owner, sub.name) != name)
                continue;

            sub.owner->DesignSubSelected(sub.obj);
            return true;
        }
        return false;
    }

    Object* DesignFormBase::NameOwner(const std::wstring &name)
    {
        for (SubObject &sub : subobjects)
        {
            if (sub.name == name)
                return sub.obj;
        }

        return nullptr;
    }

    int DesignFormBase::NameNext(const std::wstring &name)
    {
        std::vector<std::wstring> namelist;
        NamesList(namelist);

        int num = 1;
        unsigned int namelen = name.length();

        for (std::wstring n : namelist)
        {
            int cnum;
            unsigned int p = n.length();
            unsigned int p2 = p;
            while (p != 0 && n[p - 1] >= L'0' && n[p - 1] <= L'9')
                p--;

            if (p == p2 || namelen != p || n.compare(0, p, name) != 0)
                continue;

            if (StrToInt(n, cnum, p, -1))
                num = max(num, cnum + 1);
        }

        return num;

    }

    void DesignFormBase::CollectObjects(std::vector<std::pair<std::wstring, Object*> > &objectstrings, bool (*collector)(Object*), const std::wstring &objectname)
    {
        for (SubObject &sub : subobjects)
        {
            if (collector == NULL || collector(sub.obj))
            {
                std::wstring str = ControlName(sub.owner, sub.name);

                if (!objectname.empty())
                {
                    if (str == objectname)
                    {
                        objectstrings.push_back(std::pair<std::wstring, Object*>(str, sub.obj));
                        return;
                    }
                }
                else
                    objectstrings.push_back(std::pair<std::wstring, Object*>(str, sub.obj));
            }
        }
    }

    void DesignFormBase::Showing()
    {
        base::Showing();
        Changed(CHANGE_SHOW);
    }

    void DesignFormBase::Hiding()
    {
        base::Hiding();
        if (controlstate.contains(csDestroyingHandle))
            return;
        Changed(CHANGE_SHOW);
    }

    std::wstring DesignFormBase::LastExportedName()
    {
        if (exportedname.empty())
            return Name();
        return exportedname;
    }

    void DesignFormBase::SetLastExportedName(const std::wstring &newexportedname)
    {
        exportedname = newexportedname;
    }

    std::wstring DesignFormBase::LastExportedEvents()
    {
        std::wstring str;
        for (auto e : eventlist)
        {
            if (e.exportedfunc.empty())
                continue;
            str += (str.empty() ? L"" : L"|") + e.func + L"|" + e.exportedfunc + L"|" + e.type;
        }

        if (str.empty())
            return str;

        std::wstringstream stream;

        uLongf sizemax = compressBound(str.length() * 2);
        Bytef *dest = new Bytef[sizemax];
        if (compress(dest, &sizemax, (byte*)str.c_str(), str.length() * 2) != Z_OK)
        {
            delete[] dest;
            throw L"Couldn't compress exported events data!";
        }

        HexEncodeW(IntToStr(str.length()) + L"|", stream);
        HexEncodeW(dest, sizemax, stream);
        delete[] dest;

        return stream.str();
    }

    void DesignFormBase::SetLastExportedEvents(const std::wstring &str)
    {
        if (!eventlist.empty())
            throw L"Cannot set exported event names after the form has already loaded.";

        if (str.empty())
            return;

        std::wstringstream stream;

        unsigned int lensiz = str.length();
        unsigned int siz = lensiz;
        int wlen = 0; // Receives the uncompressed length of the string in wide characters.
        if (str.empty() || !HexDecodeStringToken(str.c_str(), lensiz, L'|', stream) || !StrToInt(stream.str(), wlen))
            return;
        stream.clear();
        stream.str(L"");
        siz -= lensiz;

        unsigned int srclen = siz / 2;
        std::string src(srclen, 0);
        if (!HexDecodeBytes(str.c_str() + lensiz, siz, (byte*)const_cast<char*>(src.c_str()), srclen) || siz != srclen)
            return;

        uLongf destlen = wlen * 2;
        std::wstring dest;
        dest.resize(wlen);
        if (uncompress((Bytef*)const_cast<wchar_t*>(dest.c_str()), &destlen, (byte*)const_cast<char*>(src.c_str()), srclen) != Z_OK)
            return;

        std::wstring func;
        std::wstring exportedfunc;
        std::wstring type;
        int p = -1, prep;
        do
        {
            prep = ++p;
            p = dest.find(L"|", p);
            func = dest.substr(prep, p != (int)std::wstring::npos ? p - prep : p);
            prep = ++p;
            p = dest.find(L"|", p);
            exportedfunc = dest.substr(prep, p != (int)std::wstring::npos ? p - prep : p);
            prep = ++p;
            p = dest.find(L"|", p);
            type = dest.substr(prep, p != (int)std::wstring::npos ? p - prep : p);
            if (exportedfunc.empty() || !EventTypeRegistered(type)) // One of the required fields is missing.
                break;

            eventlist.push_back(EventListItem(type, func, exportedfunc));

            func.clear();
            exportedfunc.clear();
            type.clear();
        } while (p != (int)std::wstring::npos);
    }

    void DesignFormBase::FinalizeExport()
    {
        if (Name() != exportedname)
        {
            exportedname = Name(); // Set the form's exported name.
            Modify();
        }

        // Update the names of events on the form.
        auto it = eventlist.begin();
        while (it != eventlist.end())
        {
            if (it->func.empty()) // The event was deleted if its function name was an empty string.
            {
                it = eventlist.erase(it);
                continue;
            }
            if (it->exportedfunc != it->func)
            {
                it->exportedfunc = it->func;
                Modify();
            }
            ++it;
        }
    }

    std::wstring DesignFormBase::LastExportedClassName()
    {
        if (exportedname.empty())
            return ClassName(false);
        return designer->FormClassName(exportedname);
    }

    std::wstring DesignFormBase::DefaultUnitName()
    {
        return Name();
    }

    const std::wstring& DesignFormBase::UnitName()
    {
        return unit;
    }

    void DesignFormBase::SetUnitName(const std::wstring &newunit)
    {
        std::wstring str = trim(newunit);
        if ((Name() != newunit && designer->FormNameTaken(newunit)) || (unit != newunit && designer->UnitNameTaken(newunit)))
            return;
        if (!str.empty() && !ValidFileName(str, true))
            return;

        if (!unit.empty())
        {
            std::wstring root = project->ResolvedSourcePath();
            std::wstring fname = AppendToPath(root, unit);
            std::wstring fhname = fname + L"." + project->HeaderExt();
            std::wstring fsname = fname + L"." + project->CppExt();
            bool fh = false, fs = false;
            if (((ValidFileName(fhname, true) && (fh = FileExists(fhname))) || (ValidFileName(fsname, true) && (fs = FileExists(fsname)))) &&
                ShowMessageBox(L"Rename the source and header files with the same name on the disk?", L"Query", mbYesNo) == mrYes)
            {
                std::wstring fname2 = AppendToPath(root, str);
                std::wstring fhname2 = fname2 + L"." + project->HeaderExt();
                std::wstring fsname2 = fname2 + L"." + project->CppExt();
                if (FileExists(fhname2) && ShowMessageBox(L"The file '" + GetFileName(fhname2) + L"' already exists. Would you like to rename it?", L"Query", mbYesNo) == mrYes)
                    DeleteFile(fhname2.c_str());
                if (FileExists(fsname2) && ShowMessageBox(L"The file '" + GetFileName(fsname2) + L"' already exists. Would you like to rename it?", L"Query", mbYesNo) == mrYes)
                    DeleteFile(fsname2.c_str());

                MoveFile(fhname.c_str(), fhname2.c_str());
                MoveFile(fsname.c_str(), fsname2.c_str());
            }
        }

        unit = str;
    }

    int DesignFormBase::GetEventCountById(const std::wstring &eventtype)
    {
        return std::count_if(eventlist.begin(), eventlist.end(), [&eventtype](const EventListItem &item) { return EventTypesMatch(item.type, eventtype) && !item.func.empty(); });
    }

    auto DesignFormBase::GetEventByFuncname(const std::wstring &funcname) -> EventListIterator
    {
        if (funcname.empty())
            return eventlist.end();
        for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
            if ((*it).func == funcname)
                return it;
        return eventlist.end();
    }

    auto DesignFormBase::GetEventForObject(const std::wstring &eventtype, Object *obj, const std::wstring &eventname) -> std::pair<EventListIterator, EventNameObjVectorIterator>
    {
        for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
        {
            EventListItem &e = *it;
            if (!EventTypesMatch(e.type, eventtype))
                continue;
            auto it2 = std::find_if(e.events.begin(), e.events.end(), [&](std::pair<std::wstring, Object*> &p) { return p.first == eventname && p.second == obj; });
            if (it2 != e.events.end())
                return std::make_pair(it, it2);
        }
        return std::make_pair(eventlist.end(), EventNameObjVectorIterator());
    }

    DesignFormBase::EventListItem* DesignFormBase::GetEventFunction(const std::wstring &eventtype, Object *obj, const std::wstring &eventname)
    {
        auto it = GetEventForObject(eventtype, obj, eventname);
        if (it.first == eventlist.end())
            return NULL;
        return &(*it.first);
    }

    bool DesignFormBase::SetEventFunction(const std::wstring &eventtype, Object *obj, const std::wstring &eventname, const std::wstring &value)
    {
        if (!value.empty() && !ValidVarName(value))
            return false;

        auto ito = GetEventForObject(eventtype, obj, eventname);
        std::wstring oldval = ito.first == eventlist.end() ? std::wstring() : (*ito.first).func;
        if (oldval == value)
            return true;

        auto itf = GetEventByFuncname(value);
        if (itf != eventlist.end() && !EventTypesMatch(itf->type, eventtype))
            return false;

        if (!oldval.empty()) // A function for the same event was found, rename or remove it.
        {
            EventListItem &e = *ito.first;

            if (!value.empty() && itf == eventlist.end())
            {
                ModalResults mr = e.exportedfunc.empty() && e.events.size() == 1 ? mrYes : ShowMessageBox(L"Would you like to rename the event instead of replacing it?", L"Event query", mbYesNoCancel);
                switch (mr)
                {
                case mrCancel:
                    return false;
                case mrYes:
                    ito.first->func = value;
                    designer->InvalidateEventRows(value);
                    return true;
                default:
                    break;
                }
            }

            e.events.erase(ito.second);

            if (e.events.empty() && (e.exportedfunc.empty() || ShowMessageBox(L"The function " + ito.first->func  + L" is not associated with any event. Would you like to delete it?", L"Event query", mbYesNo) == mrYes))
            {
                if (ito.first->exportedfunc.empty())
                    eventlist.erase(ito.first);
                else
                    ito.first->func.clear();
            }
            if (value.empty()) // An empty value specifies that we only wanted to remove the old event. Return without adding an event.
                return true;
        }

        if (itf == eventlist.end())
        {
            eventlist.push_back(EventListItem(eventtype, eventname, obj, value));
            return true;
        }

        EventListItem &e = *itf;
        e.events.push_back(EventNameObjPair(eventname, obj));
        return true;
    }

    std::wstring DesignFormBase::GetEventFunctionByIndex(const std::wstring &eventtype, int index)
    {
        std::vector<std::wstring> events;
        std::for_each(eventlist.begin(), eventlist.end(), [&eventtype, &events](EventListItem &item) {
            if (EventTypesMatch(item.type, eventtype) && !item.func.empty())
                events.push_back(item.func);
        });

        std::sort(events.begin(), events.end(), [](const std::wstring &first, const std::wstring &second) -> bool { return GenToLower(first) < GenToLower(second); });
        return events[index];
    }

    int DesignFormBase::GetEventFunctionIndex(const std::wstring &eventtype, Object *obj, const std::wstring &funcname)
    {
        if (funcname.empty())
            return -1;

        std::vector<std::wstring> events;
        std::for_each(eventlist.begin(), eventlist.end(), [&eventtype, &events](EventListItem &item) {
            if (EventTypesMatch(item.type, eventtype) && !item.func.empty())
                events.push_back(item.func);
        });

        std::sort(events.begin(), events.end(), [](const std::wstring &first, const std::wstring &second) -> bool { return GenToLower(first) < GenToLower(second); });

        int ix = 0;
        for (auto it = events.begin(); it != events.end(); ++it, ++ix)
            if (funcname == *it)
                return ix;
        return -1;
    }

    void DesignFormBase::print_class_line(Indentation &indent, std::wiostream &stream)
    {
        stream << indent << L"class " << ClassName(false) << L" : public " NLIBNS_STRING << BaseClass() << std::endl;
    }

    void DesignFormBase::print_public_header_base(Indentation &indent, std::wiostream &stream)
    {
        stream << indent << L"public:" << std::endl;
        ++indent;
        stream << indent << ClassName(false) << L"();" << std::endl;
        stream << indent << "virtual void Destroy();" << std::endl;
        --indent;
    }

    void DesignFormBase::print_public_header(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events)
    {
        print_public_header_base(indent, stream);
        print_members(indent, stream, events, alPublic, true);
    }

    void DesignFormBase::print_protected_header_base(Indentation &indent, std::wiostream &stream)
    {
        stream << indent << L"protected:" << std::endl;
        ++indent;
        stream << indent << L"virtual ~" << ClassName(false) << L"(); /* Don't make public. Call Destroy() to delete the object. */" << std::endl;
        --indent;
    }

    void DesignFormBase::print_protected_header(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events)
    {
        print_protected_header_base(indent, stream);
        print_members(indent, stream, events, alProtected, true);
    }

    void DesignFormBase::print_private_header_base(Indentation &indent, std::wiostream &stream)
    {
        stream << indent << L"private:" << std::endl;
    }

    void DesignFormBase::print_private_header(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events)
    {
        print_private_header_base(indent, stream);
        print_members(indent, stream, events, alPrivate, true);
    }

    void DesignFormBase::print_members(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, AccessLevels access, bool printheader)
    {
        const wchar_t *levels[] = { L"public", L"protected", L"private" };
        const wchar_t *nlevels[] = { PUBLIC_DECLARATIONS, PROTECTED_DECLARATIONS, PRIVATE_DECLARATIONS };
        if (printheader)
            stream << nlevels[access] << L": /* Designer generated list of " << levels[access] << L" members. Do not edit by hand. */" << std::endl;

        auto spos = stream.tellp();

        ++indent;

        if (access == alPrivate)
        {
            stream << indent << L"void " << INITIALIZATION_FUNCTION << "(); /* Control initializations. Do not remove. */" << std::endl;
        }

        HeaderMemberList(indent, stream, access);

        if (access == alPublic)
        {
            bool first = spos < stream.tellp(); // Put a newline between the last control and the first event. If no controls were listed, the position in the stream didn't change.
            for (auto e : events)
            {
                if (e->func.empty())
                    continue;
                if (first)
                    stream << std::endl;
                first = false;
                stream << indent << L"void " << e->func << L"(void *sender, " << e->type << L" param);" << std::endl;
            }
        }
        --indent;
    }

    void DesignFormBase::print_member_initialization(Indentation &indent, std::wiostream &stream)
    {
        std::wstringstream sdelayed; // String stream for properties that must be serialized at the end of the constructor.
        std::wstringstream sevents; // String stream for events that must be printed last.

        // Print the control initialization function.
        stream << indent << L"void " << ClassName(false) << L"::" << INITIALIZATION_FUNCTION << L"() /* Control initialization function generated by the designer. Modifications will be lost. */" << std::endl;
        stream << indent << L"{" << std::endl;
        ++indent;
        stream << indent << L"/* Generated member initialization. Do not modify. */" << std::endl;

        CppMemberList(indent, stream, sdelayed, sevents);

        stream << sdelayed.str();
        if ((int)sevents.tellp() != 0)
            stream << std::endl;
        stream << sevents.str();

        if (PrintShowAfterMemberList())
            stream << std::endl << indent << L"Show();" << std::endl;

        --indent;
        stream << indent << L"}" << std::endl << std::endl;
    }

    void DesignFormBase::print_object_declaration(Indentation &indent, std::wiostream &stream, bool printextern)
    {
        stream << indent;
        if (printextern)
            stream << L"extern ";
        stream << ClassName(false) << L" *" << Name() << L";" << std::endl << std::endl;
    }

    void DesignFormBase::print_c_and_d_structor_definition(Indentation &indent, std::wiostream &stream, WhichDestructorDefs which)
    {
        if (which.contains(wddConstructor))
        {
            stream << indent << ClassName(false) << L"::" << ClassName(false) << L"()" << std::endl;
            stream << indent << L"{" << std::endl;
            ++indent;
            stream << indent << INITIALIZATION_FUNCTION << L"(); /* Control initializations. Do not remove. */" << std::endl;
            --indent;
            stream << indent << L"}" << std::endl << std::endl;
        }
        if (which.contains(wddDestructor))
        {
            stream << indent << ClassName(false) << L"::~" << ClassName(false) << L"()" << std::endl;
            stream << indent << L"{" << std::endl;
            ++indent;
            stream << indent << L"/* Don't 'delete' the form. Call Destroy() instead which has access to the protected destructor. */" << std::endl;
            --indent;
            stream << indent << L"}" << std::endl << std::endl;
        }

        if (which.contains(wddDestroyer))
        {
            stream << indent << L"void " << ClassName(false) << L"::Destroy()" << std::endl;
            stream << indent << L"{" << std::endl;
            ++indent;
            stream << indent << L"/* Controls are valid here, but not in the destructor. */" << std::endl;
            stream << indent << L"/* ... Your code here ... */" << std::endl << std::endl;
            stream << indent << BaseClass() << L"::Destroy();" << std::endl;
            --indent;
            stream << indent << L"}" << std::endl << std::endl;
        }
    }

    void DesignFormBase::print_event_handler_lines(Indentation &indent, std::wiostream &stream, const std::wstring& handlername, const std::wstring& paramtype)
    {
        stream << indent << L"void " << ClassName(false) << L"::" << handlername << L"(void *sender, " << paramtype << L" param)" << std::endl;
        stream << indent << L"{" << std::endl;
        ++indent;
        stream << indent << L";" << std::endl;
        --indent;
        stream << indent << L"}" << std::endl << std::endl;
    }

    bool __definematch(const std::wstring &str, const wchar_t *cmp)
    {
        unsigned int cmplen = wcslen(cmp);
        return str.compare(0, cmplen, cmp) == 0 && (cmplen == str.length() || str[cmplen] == L' ' || str[cmplen] == L'\t');
    }

    void DesignFormBase::HeaderExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, bool printpragma)
    {
        if (printpragma)
            stream << L"#pragma once" << std::endl << std::endl;
        print_class_line(indent, stream);
        stream << indent << L"{" << std::endl;

        bool publictop = GetSettings().code_publictop;

        if (!publictop)
            print_private_header(indent, stream, events);
        else
            print_public_header(indent, stream, events);
        print_protected_header(indent, stream, events);
        if (!publictop)
            print_public_header(indent, stream, events);
        else
            print_private_header(indent, stream, events);

        stream << indent << L"};" << std::endl << std::endl;

        print_object_declaration(indent, stream, true);
    }

    void DesignFormBase::HeaderUpdate(Indentation &indent, std::wstringstream &stream, const std::vector<EventListItem*> &events)
    {
        Settings settings = GetSettings();

        SourceStream src(stream);

        bool classfound = false;

        // Look until we enter the class declaration.
        while (true)
        {
            if (!src.skip_to(L"class", 0))
            {
                src.fforward();
                if (src.level() != 0)
                    throw L"Incomplete header file. Not all curly brackets are closed.";
                break;
            }

            while (src.get() && src.last() != LastExportedClassName() && src.last() != ClassName(false) && src.last() != L":" && src.last() != L"::" && src.last() != L";" && src.level() == 0) // Look for the "class [Form Name] : public nlib::Form" line.
                ;
            if (src.eof())
            {
                if (src.level() == 0)
                    break;
                else
                    throw L"Incomplete class definition.";
            }
            if (src.level() > 0 || src.last() == L":" || src.last() == L"::" || src.last() == L";") // Different class definition found. Go to the next one.
                continue;

            // Form name found, mark the name in the stream for update if necessary.
            if (LastExportedClassName() != ClassName(false))
                src.record_update(ClassName(false));

            // See if this is a forward declaration and if not look for the opening bracket of the class definition.
            // Replace the part that comes after the colon with "public nlib::Form" if there is a colon.
            int classword = 0;
            bool forward = false; // Forward declaration?
            SourceStream::Position classpos = src.position_after();
            while (src.get() && src.level() == 0 && !forward)
            {
                if (src.last() == L";" || (classword == 0 && src.last() == L"::"))
                    forward = true;

                ++classword;
            }

            if (forward)
                continue;

            if (src.eof())
                throw L"Incomplete class definition.";

            src.record_insert(classpos, (L" : public " NLIB_TOSTRING(NLIBNS) L"::") + BaseClass() + L"\n");
            src.record_remove(classpos);
            classfound = true;

            break; // We are inside the class.
        }

        bool privatefound = false;
        bool protectedfound = false;
        bool publicfound = false;

        // Inject members to header while removing old lines.
        bool removing = false;
        SourceStream::Position pos;
        bool lastnewblock;
        bool newblock = true;

        bool destr = false, constr = false, destroyer = false, // Constructor and destructors found.
             prot = false, publ = false; // Protected and public found.
        SourceStream::Position protpos, publpos;
        src.skip_after(false);
        SourceStream::Position toppos = src.position_after(); // Position right after the class' opening curly braces.

        while (src.level() >= 1)
        {
            // Look for the access level string on level 1.
            while (src.get() && (src.level() != 0 && (!newblock || src.level() != 1 || (src.last() != PRIVATE_DECLARATIONS && src.last() != PROTECTED_DECLARATIONS && src.last() != PUBLIC_DECLARATIONS))))
            {
                lastnewblock = newblock;
                newblock = src.last() == L"}" || src.last() == L"{" || src.last() == L";" || src.last() == L":" || src.last().empty();
                if (!removing)
                {
                    if (!lastnewblock || newblock)
                        continue;

                    if ((!publ && src.last() == L"public") || (!prot && src.last() == L"protected"))
                    {
                        bool pub = src.last() == L"public";

                        if (!src.get())
                            throw L"Unexpected end of file after access specifier.";
                        if (src.last() == L":")
                        {
                            src.skip_after(false);
                            if (pub)
                            {
                                publpos = src.position_after();
                                publ = true;
                            }
                            else
                            {
                                protpos = src.position_after();
                                prot = true;
                            }
                            src.unget();
                        }
                        continue;
                    }
                    else if (src.last() == L"virtual" /*|| src.last() == L"~"*/) // Possible destructor.
                    {
                        enum HeaderFuncReadPos { hfrpBegin, hfrpEnd, hfrpDestroy, hfrpOpen, hfrpClose, hfrpTilde };
                        HeaderFuncReadPos fp = hfrpBegin;

                        bool dfunc = false;
                        bool trued = false;
                        while (fp != hfrpEnd && src.get())
                        {
                            switch (fp)
                            {
                            case hfrpBegin:
                                if (src.last() == L"void") // might be destructor.
                                    fp = hfrpDestroy;
                                else if (src.last() == L"~")
                                    fp = hfrpTilde;
                                else
                                    fp = hfrpEnd;
                                break;
                            case hfrpTilde:
                                if (src.last() == LastExportedClassName() || src.last() == ClassName(false))
                                {
                                    src.record_update(ClassName(false));
                                    fp = hfrpOpen;
                                    trued = true;
                                }
                                break;
                            case hfrpDestroy:
                                if (src.last() != L"Destroy")
                                    fp = hfrpEnd;
                                else
                                {
                                    dfunc = true;
                                    fp = hfrpOpen;
                                }
                                break;
                            case hfrpOpen:
                                if (src.last() != L"(")
                                    fp = hfrpEnd;
                                else
                                    fp = hfrpClose;
                                break;
                            case hfrpClose:
                                if (src.last() != L")")
                                {
                                    fp = hfrpEnd;
                                    break;
                                }
                                if (dfunc)
                                    destroyer = true;
                                else
                                    destr = true;
                                break;    
                            }
                        }
                        if (fp != hfrpEnd)
                            throw L"Unexpected end of file in the middle of the form class.";

                        if (src.last() == L"}" || src.last() == L"{" || src.last() == L";" || src.last() == L":" || src.last().empty())
                            src.unget();
                        continue;
                    }
                    else if (src.last() == LastExportedClassName() || src.last() == ClassName(false)) // Possible constructor.
                    {
                        src.record_update(ClassName(false));

                        enum HeaderFuncReadPos { hfrpBegin, hfrpEnd, hfrpClose };
                        HeaderFuncReadPos fp = hfrpBegin;
                        while (fp != hfrpEnd && src.get())
                        {
                            switch (fp)
                            {
                            case hfrpBegin:
                                if (src.last() != L"(")
                                    fp = hfrpEnd;
                                else
                                    fp = hfrpClose;
                                break;
                            case hfrpClose:
                                if (src.last() == L")")
                                    constr = true;
                                fp = hfrpEnd;
                                break;
                            }
                        }

                        if (fp != hfrpEnd)
                            throw L"Unexpected end of file after form type name.";

                        if (src.last() == L"}" || src.last() == L"{" || src.last() == L";" || src.last() == L":" || src.last().empty())
                            src.unget();
                    }
                }
                else if (src.level() == 0 || (src.level() == 1 && lastnewblock && (src.last() == L"private" || src.last() == L"protected" || src.last() == L"public" || src.last() == PRIVATE_DECLARATIONS || src.last() == PROTECTED_DECLARATIONS || src.last() == PUBLIC_DECLARATIONS)))
                {
                    src.record_remove(pos);
                    src.unget();
                    newblock = true;
                    removing = false;
                }
            }
            if (src.eof())
                throw L"Incomplete class definition.";

            if (src.level() == 0)
            {
                if (removing)
                    src.record_remove(pos, src.position());
                break;
            }

            if (removing)
            {
                src.record_remove(pos);
                removing = false;
            }

            pos = src.position();

            AccessLevels access;
            access = (src.last() == PRIVATE_DECLARATIONS ? alPrivate : src.last() == PROTECTED_DECLARATIONS ? alProtected : alPublic);

            if (!src.get() || src.last() != L":")
                throw L"Incomplete class definition.";

            if ((access == alPrivate && privatefound) || (access == alProtected && protectedfound) || (access == alPublic && publicfound)) // Duplicate designer private declarations. Remove whole line and don't print twice.
                removing = true;
            else
            {
                // Inject member list code.
                std::wstringstream memberstream;
                print_members(indent, memberstream, events, access, true);

                src.record_insert_lines(memberstream, pos);

                if (access == alPrivate)
                    privatefound = true;
                else if (access == alProtected)
                    protectedfound = true;
                else
                    publicfound = true;

                removing = true;
            }
        }

        if (classfound)
        {
            if (!publ || !constr || !destroyer) // Insert the public code at the top of the class if missing. Even if public should go below private by default.
            {
                std::wstringstream str;
                if (!publ)
                {
                    publpos = toppos;
                    str << indent << L"public:" << std::endl;
                }
                ++indent;
                if (!constr)
                    str << indent << ClassName(false) << L"();" << std::endl;
                if (!destroyer)
                    str << indent << L"virtual void Destroy();" << std::endl;
                --indent;
                src.record_insert_lines(str, publpos, 0);
            }

            if (!prot || !destr)
            {
                std::wstringstream str;
                if (!prot)
                {
                    protpos = toppos;
                    str << indent << L"protected:" << std::endl;
                }
                if (!destr)
                {
                    ++indent;
                    str << indent << L"virtual ~" << ClassName(false) << L"(); /* Don't make public. Call Destroy() to delete the object. */" << std::endl;
                    --indent;
                }
                src.record_insert_lines(str, protpos, 25);
            }

            while (!privatefound || !protectedfound || !publicfound) // Inject our code before the closing curly bracket of the class.
            {
                AccessLevels access;
                if (!privatefound && !settings.code_publictop)
                    access = alPrivate;
                else if (!publicfound && settings.code_publictop)
                    access = alPublic;
                else if (!protectedfound)
                    access = alProtected;
                else if (!privatefound && settings.code_publictop)
                    access = alPrivate;
                else
                    access = alPublic;

                std::wstringstream memberstream;
                print_members(indent, memberstream, events, access, true);
                src.record_insert_lines(memberstream, false);

                if (access == alPrivate)
                    privatefound = true;
                else if (access == alProtected)
                    protectedfound = true;
                else if (access == alPublic)
                    publicfound = true;
            }

            src.rewind();
            while (true)
            {
                if (!src.skip_to(L"extern", 0))
                    break;

                pos = src.position();
                if (!src.get())
                    throw L"Incomplete extern.";
                if (src.last() != LastExportedClassName())
                    continue;
                if (!src.get())
                    throw L"Incomplete extern.";
                if (src.last() != L"*")
                    continue;
                if (!src.get())
                    throw L"Incomplete extern.";
                if (src.last() != LastExportedName())
                    continue;
                if (!src.get())
                    throw L"Incomplete extern.";
                if (src.last() != L";")
                    continue;

                std::wstringstream str;
                str << L"extern " << ClassName(false) << L" *" << Name() << L";";
                src.record_insert(pos, str.str());
                src.record_remove(pos, src.position_after());
                break;
            }
        }
        else
        {
            src.rewind();
            src.get();
            std::wstringstream str;
            HeaderExport(indent, str, events, src.position() <= SourceStream::Position(0, 0));
            src.record_insert(src.position(), str.str());
        }

        std::wstringstream output;
        src.update(output);
#ifdef __MINGW32__
        stream.clear();
        stream.str(L"");
        stream << output.rdbuf();
#else
        stream.swap(output);
#endif
    }

    namespace
    {
        // Position data for functions belonging to the form that return nothing. Other type of functions are never exported.
        struct CppFunctionPositionData
        {
            SourceStream::Size pos; // Starting and ending position of the whole function to the last curly braces: [void formclassname::funcname(void *sender, paramtype param) { ... }]
            std::pair<std::wstring, SourceStream::Size> classname; // Position and text of formclassname: void [formclassname]::funcname(..)
            std::pair<std::wstring, SourceStream::Size> funcname; // Position and text of the function name: void formclassname::[funcname](..)
            int eventix; // Index of the function name in the event functions. -1 when not found.
            SourceStream::Size brackets; // Position to the argument list: void formclassname::funcname[(..)]
            std::vector<std::pair<std::wstring, SourceStream::Size>> args; // Position and text to the function arguments and their names: void formclassname::funcname([void *sender], [paramtype param], ...). WARNING: if there are 2 arguments, and the second does not fit the matched event, it is removed from this list, but shouldn't be chnaged in the file.
            bool forward; // True if this is just a forward declaration.

            CppFunctionPositionData() {}
            CppFunctionPositionData(CppFunctionPositionData &&other) noexcept
            {
                std::swap(pos, other.pos);
                std::swap(classname, other.classname);
                std::swap(funcname, other.funcname);
                std::swap(eventix, other.eventix);
                std::swap(brackets, other.brackets);
                std::swap(args, other.args);
                std::swap(forward, other.forward);
            }
            // Operator used in sorting found possible handler functions. The order is important because only the first version should be used.
            // Group functions that correspond to the same event, and group forward declarations, because only the very first one should be kept.
            // Make the function with the exact arguments come before others, so those are not removed or modified.
            bool operator<(CppFunctionPositionData &b)
            {
                int c;
                if (eventix != b.eventix)
                {
                    c = funcname.first.compare(b.funcname.first);
                    if (c != 0)
                        return c < 0;
                }

                if (args.size() != b.args.size())
                {
                    if ((args.size() == 2) != (b.args.size() == 2))
                        return args.size() == 2;
                    return args.size() < b.args.size();
                }

                bool argmatch = true;
                for (int ix = args.size() - 1; ix >= 0; --ix)
                {
                    c = args[ix].first.compare(b.args[ix].first);
                    if (c != 0)
                    {
                        argmatch = false;
                        if (args.size() != 2 && b.args.size() != 2)
                            return c < 0;
                    }
                }
                if (!argmatch)
                {
                    if (args[0].first != L"void * sender")
                    {
                        if (b.args[0].first == L"void * sender")
                            return false;
                        c = args[0].first.compare(b.args[0].first);
                        return c < 0;
                    }
                    else if (b.args[0].first != L"void * sender")
                        return true;
                    // The second argument of non matching functions were thrown out when adding to the list. It should match here, so the two declarations are equal.
                }

                if (forward != b.forward)
                    return forward;

                return pos.first < b.pos.first;
            }
        };

        std::wstringstream cppreadresult;
        // Reads a single string of valid variable names connected with the namespace's double colon, or a single token and position if no variable name and namespace found.
        std::wstring CppReadNamespacedValue(SourceStream &src, SourceStream::Position &startpos)
        {
            if (!src.get())
                return std::wstring();
            startpos = src.position();
            if (!ValidVarName(src.last()))
                return src.last();

            cppreadresult.clear();
            cppreadresult.str(src.last());
            //cppreadresult.seekg(0);
            cppreadresult.seekp(0, std::ios_base::end);

            bool prevdbl = false; // Double colon was read.
            while (src.get())
            {
                if (src.last() == L"::")
                {
                    if (prevdbl)
                        throw L"Double namespace specifier.";
                    prevdbl = true;
                }
                else
                {
                    if (!ValidVarName(src.last()) || !prevdbl)
                    {
                        if (prevdbl)
                            throw L"A variable name was expected after the namespace specifier.";
                        src.unget();
                        break;
                    }
                    prevdbl = false;
                }
                cppreadresult << src.last();
            }

            if (prevdbl)
                throw L"End of stream after namespace specifier. Variable name expected.";

            return cppreadresult.str();
        }


        std::wstringstream cppfuncresult;
        // Returns whether the starting position marked a function in the stream. Fills the out parameter with data about the void function.
        bool CppReadVoidFunction(DesignFormBase *form, SourceStream &src, CppFunctionPositionData &out) 
        {
            out.pos.first = src.position();
            enum FuncPos { fpClass, fpFormNameSpace /* :: */, fpName, fpOpening /* ( */, fpHead /* between () */, fpBody /* between {} */, fpDone };

            bool skip = false;

            src.get();
            if (src.level() != 0 && src.last() != form->LastExportedClassName() && src.last() != form->ClassName(false))
                skip = true;
            else
                src.unget();

            FuncPos fp = fpClass;
            std::wstring valstr; // Last fetched string with CppReadNamespacedValue.
            SourceStream::Position valpos; // Position of last fetched string .
            std::wstring argstr; // Argument type text
            SourceStream::Position argpos; // Position of type text.

            while (!skip && fp != fpDone)
            {
                src.get();
                if (src.eof())
                    throw L"End of file in the middle of a function.";
                switch (fp)
                {
                case fpClass: // void [formclassname]
                    out.classname = std::make_pair(src.last(), src.wordposition());
                    fp = fpFormNameSpace;
                    break;
                case fpFormNameSpace: // void formclassname[::]
                    if (src.last() != L"::") // Whatever it is, not a function for the form.
                        skip = true;
                    else
                        fp = fpName;
                    break;
                case fpName: // void formclassname::[funcname]
                    if (!ValidVarName(src.last())) // Double colon for namespaces should be followed by some member name. Whatever this is, not what we were looking for.
                        skip = true;
                    else
                    {
                        out.funcname = std::make_pair(src.last(), src.wordposition());
                        fp = fpOpening;
                    }
                    break;
                case fpOpening:// void formclassname::funcname[(]
                    if (src.last() != L"(") // This is definitely not a function in a form...
                    {
                        skip = true;
                        break;
                    }
                    else
                    {
                        out.brackets.first = src.position();
                        fp = fpHead;
                    }
                    break;
                case fpHead: // void formclassname::funcname([typename const * etc.] OR void formclassname::funcname(typename [argname], ..
                    if (src.last() == L";")
                        throw L"Open function head. Semicolon after opening bracket.";

                    src.unget();

                    cppfuncresult.clear();
                    cppfuncresult.str(std::wstring());
                    //cppfuncresult.seekg(0);
                    //cppfuncresult.seekp(0);
                    while (true)
                    {
                        valstr = CppReadNamespacedValue(src, (int)cppfuncresult.tellp() == 0 ? argpos : valpos);
                        if (src.eof() || valstr == L";")
                            throw L"Invalid end of function argument list.";

                        if (valstr != L"," && valstr != L")")
                        {
                            if ((int)cppfuncresult.tellp() != 0)
                                cppfuncresult << L" ";
                            cppfuncresult << valstr;
                        }
                        else
                            break;
                    }

                    if ((int)cppfuncresult.tellp() == 0)
                    {
                        if (valstr == L")")
                        {
                            out.brackets.second = src.position_after();
                            fp = fpBody;
                            break;
                        }
                        throw L"Invalid empty argument in function argument list.";
                    }
                    
                    src.unget();
                    out.args.push_back(std::make_pair(cppfuncresult.str(), SourceStream::Size(argpos, src.position_after())));
                    src.get();

                    if (valstr == L")")
                    {
                        out.brackets.second = src.position_after();
                        fp = fpBody;
                    }
                    break;
                case fpBody:
                    if (src.last() != L";" && src.last() != L"{") // After a seemingly correct function head, we don't get the expected function body or semicolon for forward declarations.
                        throw L"Function body expected.";
                    if (src.last() == L";")
                    {
                        out.forward = true;
                        src.skip_after(true);
                        out.pos.second = src.position_after();
                    }
                    else
                    {
                        while (!src.eof() && src.level() > 0)
                            src.get();
                        if (src.eof())
                            throw L"Unexpected end of file in the middle of a function.";
                        out.forward = false;
                        src.skip_after(true);
                        out.pos.second = src.position_after();
                    }
                    fp = fpDone;
                    break;
                default:
                    throw L"There is no default.";
                }

            }

            if (skip)
            {
                // Not what we were looking for. Skip till the ;
                while (!src.eof() && (src.level() != 0 || (src.last() != L";" && src.last() != L"}")))
                    src.get();
                if (src.eof())
                    throw L"Unexpected end of file after void.";

                // Replace the class name just in case.
                if (out.classname.first == form->LastExportedClassName() && out.classname.first != form->ClassName(false))
                    src.record_update(out.classname.second, form->ClassName(false));

                return false;
            }
            return true;
        }
    }

    void DesignFormBase::CppUpdate(Indentation &indent, std::wstringstream &stream, const std::vector<EventListItem*> &events)
    {
        SourceStream src(stream);

        int funcword = 0; // Index of last read word in a function being processed.

        bool initmatch = false; // We found the initialization function.
        SourceStream::Position initpos; // Position of the initialization function. This is undefined if initmatch is false, and in that case the function is injected at the top before the first word encountered but after all leading include and define lines.

        std::vector<CppFunctionPositionData> eventmatches;

        SourceStream::Position funcpos; // Position before the "void" keyword.
        SourceStream::Position funcpos2; // Position after the closing ) for the function head.

        bool newblock = true; // Always true after }, { and ;. Used for detecting the form's members (functions, constants etc.).
        bool namespc = false; // True after : characters. Namespace is marked with :: but because src can only return a single : at once, and this works anywhere which can be followed by our class name this is enough.

        bool aftername = false; // After our class name in a new block. This could be the constructor or the form object declaration.
        bool formobj = false; // Processing the form object declaration.

        bool formnamespace = false; // True if the form's class name was found and the : character came after it.

        EventListItem *e = NULL;
        EventListItem *e2 = NULL;
        SourceStream::Position namepos;
        SourceStream::Position nameposafter;
        
        bool destr = false, constr = false, destroyer = false; // Whether the code contained a destructor, constructor and destroy functions.

        while (true)
        {
            newblock = src.last() == L"}" || src.last() == L"{" || src.last() == L";" || src.last().empty();
            namespc = src.last() == L"::";

            if (!src.get())
                break;

            while (src.level() > 0)
            {
                newblock = src.last() == L"}" || src.last() == L"{" || src.last() == L";" || src.last().empty();
                namespc = src.last() == L"::";
                aftername = false;
                formobj = false;
                formnamespace = false;

                if (!src.get())
                    break;
                ;
            }

            if (newblock && src.last() == L"void")  // Potential event function definition.
            {
                funcpos = src.position();
                funcword = 0;
                aftername = false;
                formobj = false;
                formnamespace = false;

                CppFunctionPositionData funcdata;
                if (CppReadVoidFunction(this, src, funcdata)) 
                {
                    // Check if we really found an event or an initializer function. Delete the latter, save the event data.
                    if (funcdata.funcname.first == INITIALIZATION_FUNCTION && funcdata.args.empty())
                    {
                        if (!funcdata.forward)
                        {
                            initmatch = true;
                            initpos = funcdata.pos.first;
                            src.record_remove(funcdata.pos);
                        }
                        else
                            src.record_update(funcdata.classname.second, ClassName(false));
                        continue;
                    }

                    if (funcdata.funcname.first == L"Destroy" && funcdata.args.empty() && !funcdata.forward)
                    {
                        src.record_update(funcdata.classname.second, ClassName(false));
                        destroyer = true;
                        continue;
                    }

                    funcdata.eventix = -1;
                    for (int ix = events.size() - 1; ix >= 0; --ix)
                    {
                        if ((events[ix]->func.empty() && events[ix]->exportedfunc == funcdata.funcname.first) || 
                            (!events[ix]->func.empty() && events[ix]->func == funcdata.funcname.first) ||
                            (!events[ix]->exportedfunc.empty() && events[ix]->exportedfunc == funcdata.funcname.first))
                        {
                            if (funcdata.args.size() == 2 && funcdata.args[1].first != events[ix]->type + L" param")
                                funcdata.args.pop_back(); // Vandalize the function argument list if it does not fit our expectations by removing the second argument. Any function that has 2 arguments will be taken as a valid event handler.
                            funcdata.eventix = ix;
                            break;
                        }
                    }
                    eventmatches.push_back(std::move(funcdata));
                }

                continue;
            }

            if (aftername)
            {
                if (src.last() == L"*")
                    formobj = true;
                else if (src.last() == L"::")
                    formnamespace = true;
                else
                    src.unget();
                aftername = false;
                continue;
            }

            if (formnamespace) // Look for constructors and destructors.
            {
                bool tilde = false;

                while (formnamespace && !src.eof())
                {
                    switch (funcword)
                    {
                    case 0:
                        if (src.last() == L"~") // Destructor found. Otherwise this can be a constructor.
                            tilde = true;
                        else
                            src.unget();
                        break;
                    case 1:
                        if (src.last() == LastExportedClassName())
                            src.record_update(ClassName(false));
                        else if (src.last() != ClassName(false))
                            formnamespace = false;
                        break;
                    case 2:
                        if (src.last() != L"(") // Not even a function.
                            formnamespace = false;
                        break;
                    case 3:
                        if (src.last() != L")") // Probably copy constructor or similar. Error if tilde was present.
                        {
                            if (tilde)
                                throw L"Destructor shouldn't have arguments.";
                            formnamespace = false;
                        }
                        break;
                    case 4:
                        if (src.last() != L";") // Constructor or destructor with body found. If the next character is not { this still could be a valid function. (i.e. noexcept follows before function body)
                        {
                            if (tilde)
                                destr = true;
                            else
                                constr = true;
                        }
                        formnamespace = false;
                        break;
                    }

                    if (formnamespace)
                    {
                        src.get();
                        if (src.eof())
                            throw L"Unexpected end of file in unfinished destructor.";
                    }

                    ++funcword;
                    continue;
                }
            }

            if (formobj)
            {
                switch (funcword)
                {
                case 0:
                    if (src.last() != LastExportedName() && src.last() != Name())
                        formobj = false;
                    else if (LastExportedName() != Name())
                        src.record_update(Name());
                    break;
                default:
                    formobj = false;
                }

                ++funcword;
                continue;
            }

            if (src.level() > 0)
                continue;

            if (!namespc && (src.last() == LastExportedClassName() || src.last() == ClassName(false)))
            {
                src.record_update(ClassName(false));
                if (!newblock) // We only have to update the constructor and the ClassName *Form = NULL lines which all come after a new block.
                    continue;
                aftername = true;
                funcword = 0;
            }

        }

        // Multiple functions might have the same name as the event handlers. Sort the functions we found earlier and only pick one
        // version of each handler. If multiple forward declarations were found with exact same arguments, only leave the first one.
        std::sort(eventmatches.begin(), eventmatches.end());
        for (int ix = eventmatches.size() - 1; ix >= 0; --ix)
        {
            auto &b = eventmatches[ix];
            auto &a = ix > 0 ? eventmatches[ix - 1] : b;

            if (b.eventix == -1 || (ix > 0 && b.args.size() != 2 && a.eventix == b.eventix))
            {
                src.record_update(b.classname.second, ClassName(false));
                eventmatches.erase(eventmatches.begin() + ix);
                continue;
            }

            if (a.eventix != b.eventix || ix == 0)
                continue;

            bool match = a.args.size() == b.args.size();

            // If the events match but the arguments do not, the first one will be used only.
            if (match)
            {
                for (int ix = a.args.size() - 1; match && ix >= 0; --ix)
                    if (a.args[ix].first != b.args[ix].first)
                        match = false;
            }
            if (!match) // The two functions do not match. Replace the old class names of all functions with this event index.
            {
                int cnt = eventmatches.size();
                int iy = ix + 1;
                for ( ; iy < cnt; ++iy)
                {
                    if (eventmatches[iy].eventix != b.eventix)
                        break;
                    src.record_update(b.classname.second, ClassName(false));
                }
                eventmatches.erase(eventmatches.begin() + ix, eventmatches.begin() + iy);
                continue;
            }
        }

        std::set<int> inserted;
        // Only events with matching arguments are still in eventmatches. Delete all but the first forward declarations and function bodies, and dropped handlers.
        for (int ix = eventmatches.size() - 1; ix >= 0; --ix)
        {
            auto &b = eventmatches[ix];
            auto &a = ix > 0 ? eventmatches[ix - 1] : b;

            if ((ix != 0 && a.eventix == b.eventix && b.forward == a.forward) || events[b.eventix]->func.empty()) // Duplicate or dropped.
            {
                src.record_remove(b.pos);
                continue;
            }

            // This is the event we were looking for.
            src.record_update(b.classname.second, ClassName(false));
            src.record_update(b.funcname.second, events[b.eventix]->func);

            src.record_update(b.brackets, L"(void *sender, " + events[b.eventix]->type + L" param)");
            inserted.insert(b.eventix);
        }

        if (formobj || formnamespace)
            throw L"Line not closed in source file.";
        if (src.level() != 0)
            throw L"All curly braces must be closed in the source file.";

        // Insert new events that were not found in the source file.
        for (int ix = 0; ix < (int)events.size(); ++ix)
        {
            if (inserted.count(ix) || events[ix]->func.empty())
                continue;

            std::wstring paramstr = events[ix]->type;
            EventListItem *e = events[ix];
            std::wstringstream s;
            print_event_handler_lines(indent, s, e->func, paramstr);
            src.record_insert_lines(s, true);
        }

        // Find a position at the top of the source file for the missing initialization function.
        if (!initmatch)
        {
            src.rewind();
            src.get();

            // The get call skips all lines starting with #, so the initialization function is inserted right after all defines and includes at the top.
            initpos = src.position();
        }

        // Print the initialization function at the position specified.
        std::wstringstream s;
        print_member_initialization(indent, s);
        if (!constr || !destr || !destroyer)
            print_c_and_d_structor_definition(indent, s, (!constr ? wddConstructor : 0) | (!destr ? wddDestructor : 0) | (!destroyer ? wddDestroyer : 0));

        src.record_insert_lines(s, initpos, 0);

        std::wstringstream output;
        src.update(output);
#ifdef __MINGW32__
        stream.clear();
        stream.str(L"");
        stream << output.rdbuf();
#else
        stream.swap(output);
#endif
    }

    void DesignFormBase::CollectEvents(std::vector<EventListItem*> &events)
    {
        for (auto it = eventlist.begin(); it != eventlist.end(); ++it)
        {
            if (!it->func.empty() || !it->exportedfunc.empty())
                events.push_back(&*it);
        }
    }

    void DesignFormBase::SerializeNonVisualSubItems(Indentation &indent, std::wiostream &stream, std::list<NonVisualSubItem> &items)
    {
        std::list<std::pair<std::list<NonVisualSubItem>*, std::list<NonVisualSubItem>::iterator>> parents;

        std::list<NonVisualSubItem> *list = &items;
        std::list<NonVisualSubItem>::iterator it = list->begin();
        while (it != list->end())
        {
            Object *subitem = it->item;
            DesignSerializer *serializer = subitem->Serializer();
            if (!serializer)
                throw L"Trying to serialize a sub type which has no serializer.";

            stream << std::endl << indent << L"subtype " << it->propname << L" : " << ShortNamespaceName(subitem->ClassName(true)) << std::endl;
            stream << indent << L"{" << std::endl;
            ++indent;
            serializer->Serialize(indent, stream, subitem);

            if (!it->subitems.empty())
            {
                parents.push_back(std::make_pair(list, it));
                list = &it->subitems;
                it = list->begin();
                continue;
            }
            --indent;
            stream << indent << L"}" << std::endl;

            ++it;

            while (it == list->end() && !parents.empty())
            {
                --indent;
                stream << indent << L"}" << std::endl;

                list = parents.back().first;
                it = ++parents.back().second;
                parents.pop_back();
            }
        }
    }

    void DesignFormBase::FinishProcessing(const std::vector<Object*> objects)
    {
        for(Object *obj : objects)
        {
            bool done = false;
            if (dynamic_cast<Menubar*>(obj) != nullptr /*|| dynamic_cast<PopupMenu*>(obj) != nullptr*/)
            {
                ;
            //    for (DesignMenu *menu : designmenus)
            //    {
            //        if (menu->Menu() == obj)
            //        {
            //            menu->Synchronize();
            //            done = true;
            //            break;
            //        }
            //    }
            //    if (done)
            //        continue;
            //    for (DesignPopupMenu *menu : designpopups)
            //    {
            //        if (menu->Menu() == obj)
            //        {
            //            menu->Synchronize();
            //            done = true;
            //            break;
            //        }
            //    }
            //    if (done)
            //        continue;
            }
        }
    }


    //---------------------------------------------

    bool FormHasMenu(Object *propholder)
    {
        return dynamic_cast<DesignForm*>(propholder) != NULL && dynamic_cast<DesignForm*>(propholder)->DesignGetMenu() != NULL;
    }


    void DesignForm::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->Find(L"Visible")->DontExport();

        serializer->SetContainerControl(true);
        serializer->Add(L"IconFromResource", new FormIconDesignProperty<DesignForm>(L"Icon", L"Appearance", &DesignForm::DesignFormIcon, &DesignForm::DesignSetFormIcon, &DesignForm::SmallIcon));
        serializer->Add(L"SetMenu", new MenubarDesignProperty<DesignForm>(L"Menu", L"User commands", &DesignForm::DesignGetMenu, &DesignForm::DesignSetMenu));

        serializer->Add(L"SetClientWidth", new IntDesignProperty<DesignForm>(L"ClientWidth", L"Dimensions", &DesignForm::ClientWidth, &DesignForm::DesignSetClientWidth))->DontExport();
        serializer->Add(L"SetClientHeight", new IntDesignProperty<DesignForm>(L"ClientHeight", L"Dimensions", &DesignForm::DesignClientHeight, &DesignForm::DesignSetClientHeight))->DontExport();
        serializer->Add(L"SetClientRect", new ClientRectDesignProperty<DesignForm>(L"ClientRect", std::wstring(), &DesignForm::DesignReturnSelf, &DesignForm::DesignClientRect/*, &DesignForm::DesignSetClientRect*/))->DontList()->DontSerialize();
    }

    DesignForm* DesignForm::DesignReturnSelf()
    {
        return this;
    }

    IconData* DesignForm::DesignFormIcon()
    {
        return &formicon;
    }

    void DesignForm::DesignSetFormIcon(IconData* newicon)
    {
        if (newicon)
            formicon = std::move(*newicon);
        else
            formicon.Clear();

        IconFromRawData(formicon.filedata, formicon.filesize);
    }

    Menubar* DesignForm::DesignGetMenu()
    {
        return menubar;
    }

    void DesignForm::DesignSetMenu(Menubar *newmenu)
    {
        if (menubar == newmenu && (newmenu || !DesignMain()))
            return;

        if (designstate.contains(dsDeserializing))
        {
            if (menubar)
                RemoveFromNotifyList(menubar, nrRelation2);
            menubar = newmenu;
            if (menubar)
                AddToNotifyList(menubar, nrRelation2);
            return;
        }

        HideSizers();

        if (menubar)
            RemoveFromNotifyList(menubar, nrRelation2);

        menubar = newmenu;
        if (menubar)
            AddToNotifyList(menubar, nrRelation2);

        EditDesignMenu(menubar);

        if (current)
            PlaceSizers();
    }

    Size DesignForm::WindowSizeFromClient(const Size &s)
    {
        WindowParams params;
        CreateWindowParams(params);
        Rect r = Rect(0, 0, s.cx, s.cy);
        if (AdjustWindowRectEx(&r, params.style, DesignMain() != nullptr, params.extstyle) != FALSE)
            return Size(max(0, r.Width()), max(0, r.Height()));
        return Size(0, 0);
    }

    int DesignForm::DesignClientHeight()
    {
        if (DesignMain() == nullptr)
            return ClientHeight();
        return ClientHeight() - DesignMain()->Height();
    }

    Rect DesignForm::DesignClientRect()
    {
        if (DesignMain() == nullptr)
            return ClientRect();
        Rect r = ClientRect();
        r.bottom -= DesignMain()->Height();
        return r;
    }

    void DesignForm::DesignSetClientWidth(int newcwidth)
    {
        if (!HandleCreated())
        {
            Rect r = WindowRect();
            r.right = r.left + WindowSizeFromClient(Size(newcwidth, client.cy)).cx;
            client.cx = max(0, newcwidth);
            SetBounds(r);
            return;
        }
        base::SetClientWidth(newcwidth);
    }

    void DesignForm::DesignSetClientHeight(int newcheight)
    {
        if (DesignMain())
            newcheight += DesignMain()->Height();

        if (!HandleCreated())
        {
            Rect r = WindowRect();
            r.bottom = r.top + WindowSizeFromClient(Size(client.cx, newcheight)).cy;
            client.cy = max(0, newcheight);
            SetBounds(r);
            return;
        }
        base::SetClientHeight(newcheight);
    }

    DesignForm::DesignForm(const std::wstring &name, Form *owner, ButtonPanel *controlbuttons) :
                    controlbuttons(controlbuttons), containerform(nullptr), menubar(nullptr), menualt(false), current(nullptr),
                    selpropowner(nullptr), selmaincontrol(nullptr), seltag(0),
                    bgfill(nullptr), placing(false), sizing(false), dragging(false), seltype(stNone), cancelalt(false)
    {
        controlstyle << csSelfDrawn;

        pmenu = new PopupMenu();
        pmenu->SetImages(ilAlign);

        pmenu->OnShow = CreateEvent(this, &DesignForm::pmenushow);
        pmenu->OnHide = CreateEvent(this, &DesignForm::pmenuhide);

        pmiPos = pmenu->Add(L"Z Order");
        pmiPosFront = pmiPos->Add(L"Bring to Front");
        pmiPosFront->OnClick = CreateEvent(this, &DesignForm::posfrontclick);
        pmiPosUp = pmiPos->Add(L"Move Up");
        pmiPosUp->OnClick = CreateEvent(this, &DesignForm::posupclick);
        pmiPosDown = pmiPos->Add(L"Move Down");
        pmiPosDown->OnClick = CreateEvent(this, &DesignForm::posdownclick);
        pmiPosBottom = pmiPos->Add(L"Send to Bottom");
        pmiPosBottom->OnClick = CreateEvent(this, &DesignForm::posbottomclick);

        pmiAlign = pmenu->Add(L"Align");
        pmiLefts = pmiAlign->Add(L"Left");
        pmiLefts->SetImageIndex(0);
        pmiLefts->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiRights = pmiAlign->Add(L"Right");
        pmiRights->SetImageIndex(1);
        pmiRights->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiCenters = pmiAlign->Add(L"Center");
        pmiCenters->SetImageIndex(2);
        pmiCenters->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiHSpace = pmiAlign->Add(L"Distribute");
        pmiHSpace->SetImageIndex(3);
        pmiHSpace->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiAlign->AddSeparator();
        pmiTops = pmiAlign->Add(L"Top");
        pmiTops->SetImageIndex(4);
        pmiTops->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiBottoms = pmiAlign->Add(L"Bottom");
        pmiBottoms->SetImageIndex(5);
        pmiBottoms->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiVCenters = pmiAlign->Add(L"Center");
        pmiVCenters->SetImageIndex(6);
        pmiVCenters->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiVSpace = pmiAlign->Add(L"Distribute");
        pmiVSpace->SetImageIndex(7);
        pmiVSpace->OnClick = CreateEvent(this, &DesignForm::pmalignclick);

        pmiSize = pmenu->Add(L"Size");
        pmiWidths = pmiSize->Add(L"Equal Width");
        pmiWidths->SetImageIndex(8);
        pmiWidths->OnClick = CreateEvent(this, &DesignForm::pmalignclick);
        pmiHeights = pmiSize->Add(L"Equal Height");
        pmiHeights->SetImageIndex(9);
        pmiHeights->OnClick = CreateEvent(this, &DesignForm::pmalignclick);


        SetPopupMenu(pmenu);

        SetText(name);
        SetName(name);

        SetDesigning();

        SetBounds(RectS(0,0,450,350));
        SetShowPosition(fspUnchanged);
        SetTopLevelParent(owner);

        RecreateBGFill();

        sizers[dssLeft] = new DesignSizer(this, dssLeft);
        sizers[dssTop] = new DesignSizer(this, dssTop);
        sizers[dssRight] = new DesignSizer(this, dssRight);
        sizers[dssBottom] = new DesignSizer(this, dssBottom);
        sizers[dssTopLeft] = new DesignSizer(this, dssTopLeft);
        sizers[dssTopRight] = new DesignSizer(this, dssTopRight);
        sizers[dssBottomLeft] = new DesignSizer(this, dssBottomLeft);
        sizers[dssBottomRight] = new DesignSizer(this, dssBottomRight);

        OnStartSizeMove = CreateEvent(this, &DesignForm::formstartsizemove);
        OnEndSizeMove = CreateEvent(this, &DesignForm::formendsizemove);

        // TMP drag drop test of a string
        //SetDropTarget(true);
        //SetShowDropImage(true);
        //AddDropFormat(ddeCopy | ddeMove, CF_UNICODETEXT);
        //AddDropFormat(ddeCopy | ddeMove, CF_TEXT);
        //OnDragDrop = CreateEvent(this, &DesignForm::dodrop);
        // TMP end
    }

    void DesignForm::Show()
    {
        bool newcreate = false;
        if (!HandleCreated())
            newcreate = true;
        base::Show();

        //for (auto *c : controls)
        //    HijackChildrenProc(c);

        SetShowPosition(fspUnchanged);
        SetProperties();
    }

    DesignForm::~DesignForm()
    {
        delete bgfill;
    }

    void DesignForm::Destroy()
    {
        if (containerform)
        {
            containerform->Destroy();
            containerform = nullptr;
        }

        base::Destroy();
    }

    LRESULT DesignForm::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        HWND hwnd;
        PlacementControl *pc;

        switch (uMsg)
        {
        case WM_NCACTIVATE:
            if (!containerform)
                break;

            if ((containerform && application->ActiveForm() == containerform && (Form*)lParam != containerform) || wParam)
                wParam = TRUE;
            else
                wParam = FALSE;

            if (containerform && (Form*)lParam != containerform)
                containerform->PassMessage(WM_NCACTIVATE, wParam, (LPARAM)this);
            if ((Form*)lParam == containerform)
                lParam = 0;
            break;
        case wmSysColorChanged:
        case wmColorChanged:
            RecreateBGFill();
            Invalidate();
            break;
        case wmEnableChanged:
            return 0;
        case amPlaceSizers:
            PlaceSizers();
            return 0;
        case amChildCreated:
            hwnd = (HWND)lParam;
            pc = (PlacementControl*)wParam;
            HijackChildProc(pc, hwnd);
            return 0;
        case amHijackChild:
            pc = FindPlacementControl((Control*)wParam);
            if (pc)
                HijackChildrenProc(pc);
            return 0;
        }

        return base::WindowProc(uMsg, wParam, lParam);
    }

    void DesignForm::EraseBackground()
    {
        Canvas *c = GetCanvas();
        Rect r = ClientRect();

        c->SetBrush(*bgfill);
        c->FillRect(r);

        //c->SetBrush(Color(160, 100, 225, 205));
        //c->SetPen(Color(200, 100, 225, 205), .0f);

        //int w = 24;
        //int h = 16;

        //int x1 = 40;
        //c->SetAntialias(true);
        //c->SetPixelOffsetMode(pomDefault);
        //c->GradientRect(x1, 24, x1 + w, 24 + h, clWhite, clBlack, lgmVertical);
        //////c->LineF(x1, 24.f, x1 + w, 24.f + h);

        //c->SetPixelOffsetMode(pomHighQuality);
        //c->GradientRect(x1, 88, x1 + w, 88 + h, clWhite, clBlack, lgmVertical);
        //////c->LineF(x1, 88.f, x1 + w, 88.f + h);

        //int x2 = 104;
        //c->SetAntialias(false);
        //c->SetPixelOffsetMode(pomDefault);
        //c->GradientRect(x2, 24, x2 + w, 24 + h, clWhite, clBlack, lgmVertical);
        //////c->LineF(x2, 24.f, x2 + w, 24.f + h);

        //c->SetPixelOffsetMode(pomHighQuality);
        //c->GradientRect(x2, 88, x2 + w, 88 + h, clWhite, clBlack, lgmVertical);
        //////c->LineF(x2, 88.f, x2 + w, 88.f + h);

        //c->SetPixelOffsetMode(pomDefault);
    }

    void DesignForm::Paint(const Rect &updaterect)
    {
    }

    void DesignForm::RecreateBGFill()
    {
        delete bgfill;
        bgfill = 0;

        Bitmap *bmp = new Bitmap(gridsize, gridsize);
        Color c = GetColor().ToRGB();
        bmp->SetBrush(c);
        bmp->FillRect(0, 0, gridsize, gridsize);
        bool toodark = (c.R() * 0.5 + c.G() + c.B() * 0.2) < 192;
        bmp->SelectStockBrush(!toodark ? sbBlack : sbWhite);
        bmp->FillRect(0,0,1,1);

        bgfill = new Brush(bmp);

        delete bmp;
    }

    void DesignForm::CaptureChanged()
    {
        CancelAction();
    }

    bool DesignForm::CancelAction()
    {
        if (EditedMenu() == nullptr && (placing || sizing || dragging || seltype != stNone || (DesignMain() && !DesignMain()->Active() && DesignMain()->IsOpen())))
        {
            if (placing)
                controlbuttons->UnpressButtons();
            HideSelRect();
            placing = false;
            sizing = false;
            dragging = false;
            seltype = stNone;
            if (DesignMain())
                DesignMain()->Close();

            //HWND h = GetCapture();
            //Control *c = application->ControlFromHandle(h);

            if (current)
                PlaceSizers();

            return true;
        }
        return false;
    }

    void DesignForm::CloseMenus()
    {
        if (DesignMain())
            DesignMain()->Close();
        CloseEditedMenu();
    }

    void DesignForm::MouseEnter()
    {
        base::MouseEnter();
    }

    void DesignForm::MouseLeave()
    {
        base::MouseLeave();
    }

    void DesignForm::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
    {
        base::MouseMove(x, y, vkeys);

        if ((vkeys.contains(vksLeft) || vkeys.contains(vksRight)) && vkeys.contains(vksAlt))
            cancelalt = true;

        if (seltype == stNone && snaptogrid && !vkeys.contains(vksCtrl))
        {
            x -= x % gridsize;
            y -= y % gridsize;
        }

        static Point oldxy = Point(-1, -1);
        if (oldxy.x == x && oldxy.y == y)
            return;

        oldxy = Point(x, y);

        if (!sizing && placing && (abs(mousepos.x - x) > abs(GetSystemMetrics(SM_CXDRAG)) * 2 || abs(mousepos.y - y) > abs(GetSystemMetrics(SM_CYDRAG)) * 2))
        {
            sizing = true;
            PlaceControl(*(const type_info*)controlbuttons->PressedId(), mousepos.x, mousepos.y, x, y);
            return;
        }

        if (sizing && placing)
        {
            SizeCurrentControl(mousepos.x, mousepos.y, x, y, false);
            return;
        }

        if (sizing && !placing)
        {
            Control *c = current->control;
            Rect r = ControlRect(c);
            int x2 = x - sizepos.x, y2 = y - sizepos.y;
            switch(sizeside)
            {
            case dssRight:
            case dssLeft:
                y2 = r.bottom;
                break;
            case dssTop:
            case dssBottom:
                x2 = r.right;
                break;
            default:
                break;
            }
            SizeCurrentControl(mousepos.x, mousepos.y, x2, y2, false);
        }

        if (dragging)
        {
            if (!current)
                FilterSelection();

            if (current)
            {
                HideSizers();
                Control *c = current->control;
                Point p = c->ScreenToClient(ClientToScreen(Point(x, y)));
                Rect r = ControlRect(c);
                SizeCurrentControl(r.left - mousepos.x + p.x, r.top - mousepos.y + p.y, r.left + r.Width() - mousepos.x + p.x, r.top + r.Height() - mousepos.y + p.y, false);
            }
            else
            {
                Point dif = LastSelected()->control->ScreenToClient(ClientToScreen(Point(x, y)));
                dif = Point(dif.x - mousepos.x, dif.y - mousepos.y);

                std::list<ControlAlignments> alist;
                for (auto pc : selected)
                {
                    alist.push_back(pc->control->Alignment());
                    pc->control->SetAlignment(alNone);
                }

                HDWP hdwp = BeginDeferWindowPos(selected.size());
                try
                {
                    for (auto it = selected.begin(); it != selected.end(); ++it)
                    {
                        Rect r = ControlRect((*it)->control);
                        hdwp = DeferSizeControl(hdwp, *it, r.left + dif.x, r.top + dif.y, r.left + r.Width() + dif.x, r.top + r.Height() + dif.y);
                    }
                }
                catch(...)
                {
                    ;
                }
                if (hdwp)
                    EndDeferWindowPos(hdwp);

                for (auto it = selected.rbegin(); it != selected.rend(); ++it)
                {
                    PlacementControl *c = *it;
                    c->control->SetAlignment(alist.back());
                    alist.pop_back();
                    RedrawWindow(c->control->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                }

            }
            return;
        }

        if (seltype != stNone)
        {
            selrect = Rect(min(mousepos.x, x), min(mousepos.y, y), max(mousepos.x, x), max(mousepos.y, y));
            DrawSelRect(selrect);
        }
    }

    // TMP drag drop test of a string
    //std::wstring dragtext = L"This is a test of drag and drop text!";
    //void DesignForm::dodrop(void *sender, DragDropDropParameters param)
    //{
    //    HGLOBAL glob = NULL;
    //    FORMATETC format;
    //    STGMEDIUM medium;
    //    if (DataObjectContainsFormat(param.dataobject, CF_UNICODETEXT, smtHGlobal, dvaContent))
    //    {
    //        FillFormatEtc(format, CF_UNICODETEXT, smtHGlobal, dvaContent);
    //        if (param.dataobject->GetData(&format, &medium) == S_OK)
    //        {
    //            wchar_t *s = (wchar_t*)GlobalLock(medium.hGlobal);
    //            dragtext = s;
    //            GlobalUnlock(medium.hGlobal);
    //            ReleaseStgMedium(&medium);
    //        }
    //    }
    //    else if (DataObjectContainsFormat(param.dataobject, CF_TEXT, smtHGlobal, dvaContent))
    //    {
    //        FillFormatEtc(format, CF_TEXT, smtHGlobal, dvaContent);
    //        if (param.dataobject->GetData(&format, &medium) == S_OK)
    //        {
    //            char *s = (char*)GlobalLock(medium.hGlobal);
    //            std::wstringstream str;
    //            str << s;
    //            GlobalUnlock(medium.hGlobal);
    //            ReleaseStgMedium(&medium);
    //            dragtext = str.str();
    //        }
    //    }
    //}
    // TMP end

    void DesignForm::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        if ((DesignMain() && DesignMain()->Active()) || EditedMenu() != nullptr)
            SetCurrent(nullptr);

        menualt = false;

        if (vkeys.contains(vksAlt))
            cancelalt = true;

        if (button == mbLeft)
        {
            if (controlbuttons->ButtonPressed())
            {
                const type_info &ctype = *(const type_info *)controlbuttons->PressedId();

                // Non-visual controls can only be placed, but not sized.
                if (ObjectTypeByTypeInfo(ctype) == otNonVisual)
                {
                    CreateNonVisualControl(ctype);
                    return;
                }

                if (snaptogrid)
                {
                    x -= x % gridsize;
                    y -= y % gridsize;
                }

                placing = true;
                ClearSelection(this);
                mousepos = Point(x, y);
                return;
            }
            else if (controlbuttons->NVButtonPressed()) // The button of a non-visual control is down on another form or container.
            {
                ClearSelection(this);

                if (GuestReferenceOn(controlbuttons->PressedNVControl()->DesignParent()))
                {
                    if (controlbuttons->PressedNVControl()->DesignParent() != this)
                        ShowMessageBox(L"Placing the non-visual control on this form would create a circular reference with the parent of the control, which is not allowed.", L"Message", mbOk);
                    controlbuttons->UnpressButtons();
                    return;
                }

                AddNonVisualControl(controlbuttons->PressedNVControl());
                project->FormReferenced(controlbuttons->PressedNVControl()->DesignParent(), this);
                controlbuttons->UnpressButtons();

                return;
            }
        }

        PlacementControl *c = FindPlacementControl(ControlAt(x, y, true, false, true, false));
        if (!c || vkeys.contains(vksShift) || vkeys.contains(vksCtrl))
        {
            selparent = c && HasPlacementChildren(c) ? c : NULL;
            while (selparent && !selparent->control->IsControlParent())
                selparent = c->parent;

            seltype = vkeys.contains(vksAlt) ? stDeselect : vkeys.contains(vksCtrl) ? stToggle : vkeys.contains(vksShift) ? stAdd : stSelect;
            mousepos = Point(x, y);
            selrect = Rect(x, y, x, y);

            if (!c && vkeys.contains(vksDouble))
            {
                DesignProperty *prop = Serializer()->DefaultProperty();
                if (prop != nullptr)
                    designer->EditProperty(prop->Name(), true);
            }

            // TMP drag drop test of a string
            //if (!deselecting && !vkeys.contains(vksShift) && !vkeys.contains(vksCtrl))
            //{
            //    TextDataObject *dobj = new TextDataObject(dragtext);
            //    const int bmpw = (int)max(1, ((float)rand() / RAND_MAX) * 300);
            //    const int bmph = (int)max(1, ((float)rand() / RAND_MAX) * 300);
            //    Bitmap bmp(bmpw, bmph, PixelFormat32bppARGB);
            //
            //    for (int ix = 0; ix < bmpw; ++ix)
            //    {
            //        bmp.SetBrush(Color(255 - (ix % 256), ((float)rand() / RAND_MAX) * 255, ((float)rand() / RAND_MAX) * 255, ((float)rand() / RAND_MAX) * 255));
            //        bmp.FillRect(ix, 0, ix+1, bmph);
            //    }
            //    bmp.SetPen(clBlack);
            //    bmp.FrameRect(Rect(0, 0, bmpw, bmph));

            //    StartDrag(dobj, ddeCopy, &bmp, Point(bmpw / 2, bmph / 2));
            //    dobj->Release();
            //}
            // TMP end


            return;
        }

        if (c->selected && !vkeys.contains(vksDouble))
        {
            if (LastSelected() != c && button == mbLeft)
            {
                RedrawWindow(LastSelected()->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
                selected.remove(c);
                selected.push_back(c);
                RedrawWindow(LastSelected()->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
            }
        }
        else
        {
            SetCurrent(c);
            if (vkeys.contains(vksDouble) && button == mbLeft)
            {
                DesignProperty *prop = c->control->Serializer()->DefaultProperty();
                if (prop != nullptr)
                    designer->EditProperty(prop->Name(), true);
            }
        }

        dragging = button == mbLeft;
        if (snaptogrid && !vkeys.contains(vksCtrl))
        {
            x -= x % gridsize;
            y -= y % gridsize;
        }
        mousepos = c->control->ScreenToClient(ClientToScreen(Point(x, y)));
    }

    void DesignForm::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        HideSelRect();

        if (seltype == stNone && snaptogrid && !vkeys.contains(vksCtrl))
        {
            x -= x % gridsize;
            y -= y % gridsize;
        }

        if (placing)
        {
            if (button != mbLeft)
                return;

            placing = false;
            if (!sizing)
                PlaceControl(*(const type_info*)controlbuttons->PressedId(), x, y);
            else
            {
                sizing = false;
                PlaceSizers();
                SetProperties();
            }
            return;
        }

        if (sizing)
        {
            if (button != mbLeft)
                return;

            sizing = false;
            Control *c = current->control;
            Rect r = ControlRect(c);
            int x2 = x - sizepos.x, y2 = y - sizepos.y;
            switch(sizeside)
            {
            case dssRight:
            case dssLeft:
                y2 = r.bottom;
                break;
            case dssTop:
            case dssBottom:
                x2 = r.right;
                break;
            default:
                break;
            }
            SizeCurrentControl(mousepos.x, mousepos.y, x2, y2, true);
        }

        if (dragging)
        {
            dragging = false;

            if (!current)
                FilterSelection();

            if (current)
            {
                Control *c = current->control;
                Point p = c->ScreenToClient(ClientToScreen(Point(x, y)));
                Rect r = ControlRect(c);
                SizeCurrentControl(r.left - mousepos.x + p.x, r.top - mousepos.y + p.y, r.left + r.Width() - mousepos.x + p.x, r.top + r.Height() - mousepos.y + p.y, true);
            }
            else
            {
                Point dif = LastSelected()->control->ScreenToClient(ClientToScreen(Point(x, y)));
                dif = Point(dif.x - mousepos.x, dif.y - mousepos.y);

                if (dif != Point(0, 0))
                {
                    for (auto pc : selected)
                    {
                        Rect r = ControlRect(pc->control);
                        SizeControl(pc, r.left + dif.x, r.top + dif.y, r.left + r.Width() + dif.x, r.top + r.Height() + dif.y);
                    }
                }
            }

            return;
        }

        if (seltype != stNone)
        {
            if (selrect.Width() == 0 && selrect.Height() == 0) // The mouse was released without moving much. Only select a single element.
            {
                PlacementControl *pc = FindPlacementControl(ControlAt(x,y, true, false, true, false));
                if (seltype != stAdd && seltype != stToggle)
                    SetCurrent(pc);
                else if (pc)
                {
                    if (pc != current)
                    {
                        if (current)
                        {
                            selected.push_back(current);
                            current->selected = true;
                            current->control->Invalidate();

                            if (current)
                                NotifySelectionChange(current, false);

                            current = NULL;
                            HideSizers();

                            selected.push_back(pc);
                            pc->selected = true;
                        }
                        else if (pc->selected)
                        {
                            auto it = std::find(selected.begin(), selected.end(), pc);
                            auto sit = it;
                            bool backerase = LastSelected() == pc;
                            selected.erase(it);
                            if (backerase && !selected.empty() && (selected.front() != selected.back() || !containerform || containerform->SelCount()))
                                RedrawWindow(LastSelected()->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
                            pc->selected = false;
                        }
                        else
                        {
                            if (LastSelected())
                                RedrawWindow(LastSelected()->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
                            selected.push_back(pc);
                            pc->selected = true;
                        }

                        // When there is only one element left in the selected list, it is cleared and current is set to that one element.
                        if (!selected.empty() && selected.front() == selected.back() && (!containerform || !containerform->SelCount()))
                            SetCurrent(selected.front());

                        pc->control->Invalidate();
                        if (!current)
                            SetProperties();
                    }
                    else
                        SetCurrent(NULL);
                }
            }
            else // The selection is done by a selection rectangle, more controls might be selected at once.
            {
                PlacementControl *oldsellast = LastSelected();

                if (seltype != stAdd && seltype != stDeselect && seltype != stToggle)
                    ClearSelection(this);

                Rect dummy;

                if (seltype == stAdd && current)
                {
                    selected.push_back(current);
                    current->selected = true;
                }

                if (current)
                    NotifySelectionChange(current, false);

                current = NULL;

                for (auto pc : controllist)
                {
                    if (pc->parent != selparent)
                        continue;

                    if (!ControlRect(pc->control).DoesIntersect(selrect))
                        continue;

                    if (seltype == stDeselect || seltype == stToggle)
                    {
                        if (!pc->selected)
                            continue;

                        auto it2 = std::find(selected.begin(), selected.end(), pc);
                        pc->selected = false;
                        selected.erase(it2);

                        pc->control->Invalidate();
                    }
                    else if (!pc->selected)
                    {
                        selected.push_back(pc);
                        pc->selected = true;
                    }
                }

                if (!selected.empty() && selected.front() == selected.back() && (!containerform || !containerform->SelCount())) // When there is only one element in the selected list, it is cleared and current is set to that one element.
                    SetCurrent(selected.front());
                else
                {
                    HideSizers();

                    if (!selected.empty())
                    {
                        if (std::find(selected.begin(), selected.end(), oldsellast) != selected.end())
                        {
                            selected.remove(oldsellast);
                            selected.push_back(oldsellast);
                        }
                    }

                    for (auto pc : selected)
                        pc->control->Invalidate();

                    SetProperties();
                }
            }
            seltype = stNone;
        }
    }

    DesignForm::PlacementControl* DesignForm::GetPlacementParent(Control *control)
    {
        if (!control || control == this || control->Parent() == this)
            return NULL;

        control = control->Parent();
        return FindPlacementControl(control);
    }

    bool DesignForm::HasAsChild(PlacementControl *placement, PlacementControl *otherplacement)
    {
        if (placement == otherplacement)
            return false;
        while (otherplacement && otherplacement != placement)
            otherplacement = otherplacement->parent;
        return otherplacement == placement;
    }

    bool DesignForm::HasAsParent(PlacementControl *placement, PlacementControl *otherplacement)
    {
        return HasAsChild(otherplacement, placement);
    }

    bool DesignForm::HasPlacementChildren(PlacementControl *placement)
    {
        return !placement->controls.empty();
    }

    DesignForm::PlacementControl* DesignForm::NextPlacementControl(PlacementControl *placement, bool allowoverflow)
    {
        if (tablist.empty())
            return NULL;

        if (placement == NULL)
            return tablist.front();

        if (!placement->tablist.empty())
            return placement->tablist.front();

        do
        {
            PlacementList &tabs = placement->parent ? placement->parent->tablist : tablist;
            PlacementIterator it = placement->tabpos;
            ++it;

            if (it != tabs.end())
                return *it;
            placement = placement->parent;
        } while (placement);

        if (!allowoverflow)
            return NULL;

        return tablist.front();
    }

    DesignForm::PlacementControl* DesignForm::PrevPlacementControl(PlacementControl *placement, bool allowoverflow)
    {
        if (tablist.empty())
            return NULL;

        if (placement == NULL)
            placement = tablist.front();

        PlacementList &tabs = placement->parent ? placement->parent->tablist : tablist;
        PlacementRIterator it(placement->tabpos);

        if (it != tabs.rend())
        {
            placement = *it;
            while (!placement->tablist.empty())
                placement = placement->tablist.back();

            return placement;
        }

        if (placement->parent)
            return placement->parent;

        if (!allowoverflow)
            return NULL;

        placement = tablist.back();
        while (!placement->tablist.empty())
            placement = placement->tablist.back();

        return placement;
    }

    void DesignForm::DeletePlacementControl(PlacementControl *placement)
    {
        if (placement == current || HasAsChild(placement, current))
        {
            PlacementControl *nextsel = PrevPlacementControl(placement, true);
            if (nextsel != placement && !HasAsChild(placement, nextsel))
                SetCurrent(nextsel);
            else
                SetCurrent(NULL);
        }

        designstate << dsDeleting;
        RemovePlacementControl(placement, true);
        designstate -= dsDeleting;
    }

    void DesignForm::RemovePlacementControl(PlacementControl *placement, bool deleteit)
    {
        Modify();

        if (designer && designer->ActiveForm() == this)
            designer->BeginControlChange();

        while (!placement->controls.empty())
            RemovePlacementControl(placement->controls.back(), deleteit);

        if (!placement->control->HandleCreated())
        {
            if (designer && designer->ActiveForm() == this)
                designer->ControlDeleted(placement->control, ControlName(placement->control, placement->control->Name()));

            placement->control->OnMessage = NULL;
            placement->control->OnPaint = NULL;
            placement->control->OnEndSizeMove = NULL;
            RemoveFromNotifyList(placement->control, nrNoReason);
            if (deleteit)
                placement->control->Destroy();
            if (placement->parent)
            {
                placement->parent->controls.erase(placement->controlpos);
                placement->parent->tablist.erase(placement->tabpos);
            }
            else
            {
                controls.erase(placement->controlpos);
                tablist.erase(placement->tabpos);
            }
            controllist.erase(find(controllist.begin(), controllist.end(), placement));
            controlmap.erase(placement->control);

            if (designer && designer->ActiveForm() == this)
                designer->EndControlChange();
            return;
        }

        HWNDList list;
        list.push_front(placement->control->Handle());
        EnumChildWindowsInZOrder(placement->control->Handle(), list);

        for (auto it = list.rbegin(); it != list.rend(); it++)
        {
            Control *ctrl = application->ControlFromHandle(*it);
            if (ctrl != NULL)
            {
                PlacementControl *pc = FindPlacementControl(ctrl);
                ctrl->OnMessage = NULL;
                ctrl->OnPaint = NULL;
                ctrl->OnEndSizeMove = NULL;
                if (pc)
                {
                    if (designer && designer->ActiveForm() == this)
                        designer->ControlDeleted(pc->control, ControlName(pc->control, pc->control->Name()));

                    for (auto sub : pc->subcontrols)
                    {
                        SetWindowLongPtr(sub->handle, GWLP_WNDPROC, (LONG)sub->proc);
                        delete sub;
                    }
                    pc->subcontrols.clear();
                    RemoveFromNotifyList(pc->control, nrNoReason);
                    Control *c = pc->control;
                    if (pc->parent)
                    {
                        pc->parent->controls.erase(pc->controlpos);
                        pc->parent->tablist.erase(pc->tabpos);
                    }
                    else
                    {
                        controls.erase(pc->controlpos);
                        tablist.erase(pc->tabpos);
                    }
                    controllist.erase(find(controllist.begin(), controllist.end(), pc));
                    controlmap.erase(pc->control);
                    DeleteNotify(pc->control);
                    if (deleteit)
                        c->Destroy();
                }
            }
        }

        if (designer && designer->ActiveForm() == this)
            designer->EndControlChange();
    }

    void DesignForm::RemovingChildNotify(Control *parent, Control *child)
    {
        base::RemovingChildNotify(parent, child);
        if (!InNotifyList(child, nrOwnership))
            return;

        if (designer && designer->ActiveForm() == this)
            designer->BeginControlChange();

        // The child being removed stays on the notify list, if the deletion was invoked from outside the form.
        // If the control was deleted by the designer form's DeletePlacementControl function, the child is first
        // removed from the notify list before it is deleted, so this function never reaches this point.
        // Do the clean up so we don't crash because of invalid controls in the controls list.
        for (auto it = controllist.begin(); it != controllist.end(); ++it)
        {
            PlacementControl *pc = *it;
            if (pc->control == child)
            {
                if (current == pc || HasAsChild(pc, current))
                {
                    PlacementControl *nextsel = PrevPlacementControl(pc, true);
                    if (nextsel != pc && !HasAsChild(pc, nextsel))
                        SetCurrent(nextsel);
                    else
                        SetCurrent(NULL);
                }

                bool deleting = designstate.contains(dsDeleting);
                if (!deleting)
                    designstate << dsDeleting;
                RemovePlacementControl(*it, false);
                if (!deleting)
                    designstate -= dsDeleting;

                break;
            }
        }

        if (designer && designer->ActiveForm() == this)
            designer->EndControlChange();

    }

    void DesignForm::NameChangeNotify(Object *obj, const std::wstring &oldname)
    {
        base::NameChangeNotify(obj, oldname);

        if (designstate.contains(dsDeserializing))
            return;

        if (containerform && dynamic_cast<NonVisualControl*>(obj) != NULL)
            containerform->NameChangeNotify(obj, oldname);
        else if (designer && designer->ActiveForm() == this)
            designer->ControlNameChanged(obj, ControlName(obj, oldname), ControlName(obj, obj->Name()));

    }

    void DesignForm::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        if (keycode == VK_ESCAPE && CancelAction())
            return;

        if (keycode == VK_MENU && DesignMain())
        {
            keycode = 0;
            //if (designmenu->Active())
            //{
            //    menualt = false;
            //    designmenu->Close();
            //    ClearSelection(this);
            //    return;
            //}
            //ClearSelection(this);
            //menualt = true;
            //designmenu->Activate();
            //if (designmenu->Active())
            //    menualt = false;
            //UpdateSelection();
            return;
        }

        if (DesignMain() && selected.empty() && !current && (DesignMain()->Active() || menualt))
        {
            DesignMain()->KeyPush(keycode, key, vkeys);
            menualt = false;
            return;
        }

        menualt = false;

        if (EditedMenu() != nullptr)
        {
            base::KeyPush(keycode, key, vkeys);
            return;
        }

        if (keycode != VK_ESCAPE && key != VK_ESCAPE && !sizing && seltype == stNone && !dragging && !placing && current && !current->selected && !selarea.Empty())
        {
            Object *pobj = current->control->PropertyOwner();
            if (pobj->DesignKeyPush(this, keycode, key, vkeys))
                return;
        }

        if (!selected.empty() && (keycode == VK_LEFT || keycode == VK_UP || keycode == VK_DOWN || keycode == VK_RIGHT) && vkeys.contains(vksCtrl))
            FilterSelection();

        if ((current || !selected.empty() || (containerform && containerform->SelCount())) && vkeys.contains(vksCtrl) && (key == ckC || key == ckX ||  keycode == VK_INSERT || keycode == VK_DELETE))
        {
            CopySerialized();
            if (key == ckX || keycode == VK_DELETE)
            {
                if (current)
                    DeletePlacementControl(current);
                else
                    DeleteSelected();
            }
            return;
        }

        if ((vkeys.contains(vksCtrl) && key == ckV) || (vkeys.contains(vksShift) && keycode == VK_INSERT))
        {
            PlacementControl *parent = current;
            if (!selected.empty())
            {
                int level = -1;
                for (auto it = selected.begin(); it != selected.end(); ++it)
                {
                    PlacementControl *c = *it;
                    int clevel = 0;
                    while (c)
                    {
                        ++clevel;
                        c = c->parent;
                    }
                    if (level < 0 || clevel < level)
                    {
                        level = clevel;
                        parent = (*it)->parent;
                    }
                }
            }
            else if (containerform && containerform->SelCount())
                containerform->ClearSelection(this);

            PasteSerialized(parent);
            return;
        }

        if (keycode == VK_TAB && ((!current && selected.empty() && (!containerform || containerform->SelCount() == 0)) || current || (!current && selected.empty() && containerform && containerform->SelCount() == 1) || LastSelected() != nullptr || (containerform && containerform->LastSelected() != nullptr) ))
        {
            SelectNext(vkeys.contains(vksShift));
            return;
        }

        if (current)
        {
            if (keycode == VK_ESCAPE)
                SetCurrent(selarea.Empty() ? current->parent : current);

            if (keycode == VK_DELETE)
                DeletePlacementControl(current);

            if (keycode == VK_LEFT || keycode == VK_UP || keycode == VK_DOWN || keycode == VK_RIGHT)
            {
                if (vkeys.contains(vksCtrl))
                {
                    Rect r = ControlRect(current->control);
                    int dif = 1;
                    if (vkeys.contains(vksShift))
                        dif = gridsize;
                    OffsetRect(&r, keycode == VK_LEFT ? -dif : keycode == VK_RIGHT ? dif : 0, keycode == VK_UP ? -dif : keycode == VK_DOWN ? dif : 0);
                    SizeCurrentControl(r.left, r.top, r.right, r.bottom, true);
                }
                else if (vkeys.contains(vksShift))
                {
                    Rect r = ControlRect(current->control);
                    int dif = 1;
                    r.right = max(r.left, r.right + (keycode == VK_LEFT ? -dif : keycode == VK_RIGHT ? dif : 0));
                    r.bottom = max(r.top, r.bottom + (keycode == VK_UP ? -dif : keycode == VK_DOWN ? dif : 0));
                    SizeCurrentControl(r.left, r.top, r.right, r.bottom, true);
                }
                else
                {
                    PlacementControl *next = FindNextControlOnSide(current, keycode == VK_LEFT ? dirLeft : keycode == VK_UP ? dirUp : keycode == VK_DOWN ? dirDown : dirRight);
                    if (next)
                        SetCurrent(next);
                }
            }
        }
        else if (!selected.empty() || (containerform && containerform->SelCount()))
        {
            if (keycode == VK_ESCAPE)
                SetCurrent(LastSelected());

            if (keycode == VK_DELETE)
                DeleteSelected();

            if ( (keycode == VK_LEFT || keycode == VK_UP || keycode == VK_DOWN || keycode == VK_RIGHT) && (vkeys.contains(vksCtrl) || vkeys.contains(vksShift)) && !selected.empty())
            {
                for (auto it = selected.begin(); it != selected.end(); it++)
                {
                    if (vkeys.contains(vksCtrl))
                    {
                        Rect r = ControlRect((*it)->control);
                        int dif = 1;
                        if (vkeys.contains(vksShift))
                            dif = gridsize;
                        OffsetRect(&r, keycode == VK_LEFT ? -dif : keycode == VK_RIGHT ? dif : 0, keycode == VK_UP ? -dif : keycode == VK_DOWN ? dif : 0);
                        SizeControl(*it, r.left, r.top, r.right, r.bottom);
                    }
                    else if (vkeys.contains(vksShift))
                    {
                        Rect r = ControlRect((*it)->control);
                        int dif = 1;
                        r.right = max(r.left, r.right + (keycode == VK_LEFT ? -dif : keycode == VK_RIGHT ? dif : 0));
                        r.bottom = max(r.top, r.bottom + (keycode == VK_UP ? -dif : keycode == VK_DOWN ? dif : 0));
                        SizeControl(*it, r.left, r.top, r.right, r.bottom);
                    }
                }
            }
        }

        base::KeyPush(keycode, key, vkeys);
    }

    void DesignForm::KeyUp(WORD &keycode, VirtualKeyStateSet vkeys)
    {
        if (cancelalt && keycode == VK_MENU)
        {
            cancelalt = false;
            return;
        }

        if (sizing || seltype != stNone || dragging || placing)
            return;

        if (keycode == VK_MENU && DesignMain() && vkeys == 0)
        {
            keycode = 0;
            if (DesignMain()->Active())
            {
                menualt = false;
                DesignMain()->Close();
                ClearSelection(this);
                return;
            }
            ClearSelection(this);
            menualt = true;
            DesignMain()->Activate();
            if (DesignMain()->Active())
                menualt = false;
            UpdateSelection();
            return;
        }

        if (DesignMain() && selected.empty() && !current && DesignMain()->Active())
        {
            DesignMain()->KeyUp(keycode, vkeys);
            return;
        }

        if (EditedMenu() != nullptr)
        {
            base::KeyUp(keycode, vkeys);
            return;
        }

        if (current && keycode != VK_ESCAPE && !current->selected && !selarea.Empty())
        {
            Object *pobj = current->control->PropertyOwner();
            pobj->DesignKeyUp(this, keycode, vkeys);
            return;
        }
    }

    void DesignForm::GetPlacePosition(PlacementControl *p, PlacementControl *placed, Point pt, int &x, int &y)
    {
        x = pt.x;
        y = pt.y;

        std::vector<Point> pos; // List of positions of all controls placed on p.
        PlacementList &ctrl = (p ? p->controls : controls);
        for (auto it = ctrl.begin(); it != ctrl.end(); ++it)
        {
            if (*it != placed)
                pos.push_back((*it)->control->WindowRect().TopLeft());
        }

        std::sort(pos.begin(), pos.end(), [](const Point &p1, const Point &p2) -> bool {
            if (p1.y == p2.y)
                return p1.x < p2.x;
            return p1.y < p2.y;
        });

        auto it = pos.begin();
        do
        {
            for ( ; it != pos.end(); ++it)
                if (y <= it->y && x <= it->x)
                    break;
            if (it == pos.end() || (y != it->y || x != it->x))
                return;
            y += gridsize;
            x += gridsize;
            if (y >= (p ? p->control->ClientHeight() : ClientHeight()))
            {
                x += gridsize;
                while (y - gridsize >= 0)
                {
                    y -= gridsize;
                    x -= gridsize;
                    if (x < 0)
                    {
                        while (x + gridsize < (p ? p->control->ClientWidth() : ClientWidth()))
                            x += gridsize;
                    }
                }
                it = pos.begin();
            }
            if (x >= (p ? p->control->ClientWidth() : ClientWidth()))
            {
                y += gridsize;
                while (x - gridsize >= 0)
                {
                    x -= gridsize;
                    y -= gridsize;
                    if (y < 0)
                    {
                        while (y + gridsize < (p ? p->control->ClientHeight() : ClientHeight()))
                            y += gridsize;
                    }
                }
                it = pos.begin();
            }
        } while(x != pt.x || y != pt.y);
    }

    void DesignForm::PlaceControl(const type_info &ctype)
    {
        if (ObjectTypeByTypeInfo(ctype) == otNonVisual)
            CreateNonVisualControl(ctype);
        else
            PlaceControl(ctype, INT_MIN, INT_MIN);
    }

    bool DesignForm::GuestReferenceOn(DesignFormBase *guestparent, std::set<DesignFormBase*> &checkfinished)
    {
        if (!containerform)
            return false;

        return containerform->GuestReferenceOn(guestparent, checkfinished);
    }

    void DesignForm::PlaceControl(const type_info &ctype, int x, int y)
    {
        bool nopos = (x == INT_MIN && y == INT_MIN); // Place the control somewhere randomly, either on the currently selected placement control or on the form.

        PlacementControl *p = !nopos ? FindPlacementControl(ControlAt(x, y, true, true, true, false)) : (current && current->control->IsControlParent() ? current : NULL);
        CreateControl(ctype, p);

        Point dif;
        if (p && !nopos)
            dif = p->control->ScreenToClient(ClientToScreen(Point(0, 0)));

        controlbuttons->UnpressButtons();

        Size s = current->control->DesignSize();
        if (nopos)
        {
            Rect cr = p ? p->control->ClientRect() : ClientRect();
            Size wh(cr.Width(), cr.Height());
            GetPlacePosition(p, current, Point((wh.cx - s.cx) / 2, (wh.cy - s.cy) / 2), x, y);
        }
        current->control->SetBounds(RectS(x + dif.x, y + dif.y, s.cx, s.cy));
        current->control->Show();

        PostMessage(Handle(), amPlaceSizers, 0, 0);

        if (designer && designer->ActiveForm() == this)
            designer->ControlAdded(current->control, ControlName(current->control, current->control->Name()));

        //PlaceSizers();
        SetProperties();
    }

    void DesignForm::PlaceControl(const type_info &ctype, int x1, int y1, int x2, int y2)
    {
        PlacementControl *p = FindPlacementControl(ControlAt(x1, y1, true, true, true, false));
        CreateControl(ctype, p);

        Point dif(0, 0);
        if (p)
            dif = p->control->ScreenToClient(ClientToScreen(Point(0, 0)));

        controlbuttons->UnpressButtons();
        current->control->SetBounds(Rect(min(x1, x2) + dif.x, min(y1, y2) + dif.y, max(x1, x2) + dif.x, max(y1, y2) + dif.y));
        current->control->Show();

        if (designer && designer->ActiveForm() == this)
            designer->ControlAdded(current->control, ControlName(current->control, current->control->Name()));
    }

    void DesignForm::PlaceControl(Control *control, Control *parent, DesignProperty *prop)
    {
        static int createcnt = 0;

        PlacementControl *cpc = NULL;
        if (designstate.contains(dsDeserializing))
        {
            auto it = controlmap.find(control);
            if (it != controlmap.end())
            {
                cpc = (*it).second;

                control->SetDesigning();
                control->Hide();

                control->OnMessage = CreateEvent(this, &DesignForm::controlmessage);
                control->OnPaint = CreateEvent(this, &DesignForm::controlpaint);
                control->OnEndSizeMove = CreateEvent(this, &DesignForm::controlendsizemove);

                control->SetParent(parent);
                if (DesignMain())
                    DesignMain()->MoveToTop();
            }
            return;
        }

        // Ignore the call if the caller (objectparent) has no access to the current selection.
        //if (!current || current->selected || (current->control->MainControl() != control->MainControl() && current->control != control->MainControl()))
        //    return;

        PlacementControl *ppc;
        auto it = controlmap.find(parent);
        if (it == controlmap.end())
            throw L"The parent control is not a valid designed control on the form.";
        ppc = (*it).second;

        if (!createcnt)
            designstate << dsCreating;
        ++createcnt;

        cpc = new PlacementControl;
        control->SetParentProperty(prop);
        cpc->control = control;
        cpc->selected = false;
        control->SetDesigning();

        control->OnMessage = CreateEvent(this, &DesignForm::controlmessage);
        control->OnPaint = CreateEvent(this, &DesignForm::controlpaint);
        control->OnEndSizeMove = CreateEvent(this, &DesignForm::controlendsizemove);

        controllist.push_back(cpc);
        controlmap[cpc->control] = cpc;
        cpc->control->SetParent(parent);
        cpc->parent = ppc;

        // Check for any child controls that might be designable and add them too.
        for (int ix = 0; ix < control->ControlCount(); ++ix)
        {
            if (control->Controls(ix)->Designing())
                PlaceControl(control->Controls(ix), control, NULL);
        }

        //// Replace the window proc of all non-designable child controls on the newly created control.
        //HijackChildrenProc(cpc);

        PlacementList &tabs = ppc->tablist;
        PlacementList &ctrls = ppc->controls;
        cpc->controlpos = ctrls.insert(ctrls.end(), cpc);
        if (!cpc->control->controlstyle.contains(csInTabOrder) || tabs.empty())
            cpc->tabpos = tabs.insert(tabs.end(), cpc);
        else
        {
            PlacementRIterator rit(tabs.end());
            while (rit != tabs.rend() && !(*rit)->control->controlstyle.contains(csInTabOrder))
                ++rit;
            cpc->tabpos = tabs.insert(rit.base(), cpc);
        }

        --createcnt;
        if (!createcnt)
            designstate -= dsCreating;

        if (DesignMain())
            DesignMain()->MoveToTop();
    }

    void DesignForm::SizeCurrentControl(int x1, int y1, int x2, int y2, bool showsizers)
    {
        if (current->control == NULL)
            return;

        SizeControl(current, x1, y1, x2, y2);

        if (showsizers)
            PlaceSizers();
    }

    void DesignForm::SizeControl(PlacementControl *placement, int x1, int y1, int x2, int y2)
    {
        Control *cparent = placement->control->Parent();

        if (x1 > x2)
            std::swap(x1, x2);
        if (y1 > y2)
            std::swap(y1, y2);

        Point dif(0, 0);
        if (cparent != this)
            dif = cparent->ScreenToClient(ClientToScreen(0, 0));

        placement->control->SetBounds(Rect(x1 + dif.x, y1 + dif.y, x2 + dif.x, y2 + dif.y));
        //RedrawWindow(placement->control->Parent()->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }

    HDWP DesignForm::DeferSizeControl(HDWP hdefer, PlacementControl *placement, int x1, int y1, int x2, int y2)
    {
        Control *cparent = placement->control->Parent();

        if (x1 > x2)
            std::swap(x1, x2);
        if (y1 > y2)
            std::swap(y1, y2);

        Point dif(0, 0);
        if (cparent != this)
            dif = cparent->ScreenToClient(ClientToScreen(Point(0, 0)));

        return DeferWindowPos(hdefer, placement->control->Handle(), NULL, x1 + dif.x, y1 + dif.y, x2 - x1, y2 - y1, SWP_BOUNDS);
    }

    DesignForm::PlacementControl* DesignForm::CreateControl(const type_info &ctype)
    {
        Control *c;

        if (ObjectTypeByTypeInfo(ctype) != otVisual)
            throw L"Invalid control type";

        c = CreateControlOfType(ctype);

        //if (ctype == typeid(Panel))
        //    c = new Panel();
        //else if (ctype == typeid(Paintbox))
        //    c = new Paintbox();
        //else if (ctype == typeid(ToolButton))
        //    c = new ToolButton();
        //else if (ctype == typeid(Label))
        //    c = new Label();
        //else if (ctype == typeid(Bevel))
        //    c = new Bevel();
        //else if (ctype == typeid(Button))
        //    c = new Button();
        //else if (ctype == typeid(Checkbox))
        //    c = new Checkbox();
        //else if (ctype == typeid(Radiobox))
        //    c = new Radiobox();
        //else if (ctype == typeid(Groupbox))
        //    c = new Groupbox();
        //else if (ctype == typeid(Edit))
        //    c = new Edit();
        //else if (ctype == typeid(Memo))
        //    c = new Memo();
        //else if (ctype == typeid(UpDown))
        //    c = new UpDown();
        //else if (ctype == typeid(Progressbar))
        //    c = new Progressbar();
        //else if (ctype == typeid(Listbox))
        //    c = new Listbox();
        //else if (ctype == typeid(Combobox))
        //    c = new Combobox();
        //else if (ctype == typeid(TabControl))
        //    c = new TabControl();
        //else if (ctype == typeid(PageControl))
        //    c = new PageControl();
        //else if (ctype == typeid(Listview))
        //    c = new Listview();
        //else if (ctype == typeid(StringGrid))
        //    c = new StringGrid();

        c->SetDesigning();
        c->Hide();
        std::wstring name = DisplayNameByTypeInfo(ctype, false);
        int num = 1;
        num = max(num, NameNext(name));
        std::wstringstream ws;
        ws << name << num;
        c->SetName(ws.str());
        if (!designstate.contains(dsDeserializing))
            c->SetText(ws.str());

        PlacementControl *pc;
        pc = new PlacementControl;
        pc->control = c;
        pc->selected = false;

        c->OnMessage = CreateEvent(this, &DesignForm::controlmessage);
        c->OnPaint = CreateEvent(this, &DesignForm::controlpaint);
        c->OnEndSizeMove = CreateEvent(this, &DesignForm::controlendsizemove);

        Modify();

        return pc;
    }

    void DesignForm::CreateControl(const type_info &ctype, PlacementControl* &parent)
    {
        designstate << dsCreating;

        current = CreateControl(ctype);

        while (parent && !parent->control->PropertyOwner()->Serializer()->ParentFor(ctype))
            parent = parent->parent;

        controllist.push_back(current);
        controlmap[current->control] = current;
        current->control->SetParent(parent ? parent->control : this);
        current->parent = parent;

        //// Replace the window proc of all non-designeable child controls on the newly created control.
        //HijackChildrenProc(current);

        PlacementList &tabs = parent ? parent->tablist : tablist;
        PlacementList &ctrls = parent ? parent->controls : controls;
        current->controlpos = ctrls.insert(ctrls.end(), current);

        if (!current->control->controlstyle.contains(csInTabOrder) || tabs.empty())
            current->tabpos = tabs.insert(tabs.end(), current);
        else
        {
            PlacementRIterator rit(tabs.end());
            while (rit != tabs.rend() && !(*rit)->control->controlstyle.contains(csInTabOrder))
                ++rit;

            current->tabpos = tabs.insert(rit.base(), current);
        }

        designstate -= dsCreating;

        NotifySelectionChange(current, true);

        if (DesignMain())
            DesignMain()->MoveToTop();
    }

    void DesignForm::HijackChildrenProc(PlacementControl *parent)
    {
        if (!parent->control->HandleCreated())
            return;
        PlacementList ctr;

        ctr.push_back(parent);
        for (auto *c : ctr)
            ctr.insert(ctr.end(), c->controls.begin(), c->controls.end());

        ctr.reverse();
        for (auto *c : ctr)
        {
            HWNDList list;
            EnumChildWindowsInZOrder(c->control->Handle(), list);
            for (HWND hwnd : list)
                HijackChildProc(c, hwnd);
        }

    }

    void DesignForm::HijackChildProc(PlacementControl *parent, HWND handle)
    {
        Control *ctrl = application->ControlFromHandle(handle);
        if (ctrl != NULL)
        {
            if (ctrl->OnMessage)
                return;
            ctrl->OnMessage = CreateEvent(this, &DesignForm::controlmessage);
            ctrl->OnPaint = CreateEvent(this, &DesignForm::controlpaint);
        }
        else
        {
            if (GetWindowLongPtr(handle, GWLP_WNDPROC) == (LONG)&SubcontrolReplacementProc)
                return;

            Subcontrol *subcurrent = new Subcontrol;
            subcurrent->handle = handle;
            subcurrent->owner = parent;
            subcurrent->parentform = this;
            SetWindowLongPtr(handle, GWLP_USERDATA, (LONG)subcurrent);
            subcurrent->proc = (PWndProc)GetWindowLongPtr(handle, GWLP_WNDPROC);
            SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG)&SubcontrolReplacementProc);
            parent->subcontrols.push_back(subcurrent);
        }
    }

    void DesignForm::controlmessage(void *sender, MessageParameters param)
    {
        if (param.uMsg == WM_NCHITTEST && param.wParam != 999 && GetCapture() != sender)
        {
            if ((GetKeyState(VK_RBUTTON) & (1 << 15)) == 0 && !sizing && seltype == stNone && !dragging && !placing)
            {
                Control *c = FindPlacementControl((Control*)sender)->control;
                Point pt = c->ScreenToWindow(GET_X_LPARAM(param.lParam), GET_Y_LPARAM(param.lParam));
                if (c->NeedDesignerHittest(pt.x, pt.y, c->PassMessage(WM_NCHITTEST, 999, param.lParam)))
                    return;
            }
            param.result = HTTRANSPARENT;
            param.allowprocessing = false;
        }
        else if (param.uMsg == wmTabOrderChanged)
        {
            PlacementControl *pc = FindPlacementControl((Control*)param.wParam);
            if (!pc)
                return;
            int step = param.lParam;
            PlacementControl *pcparent = pc->parent;
            PlacementList &tabs = pcparent ? pcparent->tablist : tablist;
            if (step > 0)
            {
                PlacementIterator it = pc->tabpos;
                it = tabs.erase(it);
                while (step--)
                    it++;
                pc->tabpos = tabs.insert(it, pc);
            }
            else if (step < 0)
            {
                PlacementIterator it = pc->tabpos;
                it = tabs.erase(it);
                PlacementRIterator rit(it);
                while (step++)
                    rit++;
                pc->tabpos = tabs.insert(rit.base(), pc);
            }
        }
        else if (param.uMsg == WM_PARENTNOTIFY && LOWORD(param.wParam) == WM_CREATE && !designstate.contains(dsCreating) && !designstate.contains(dsDeserializing) && !designstate.contains(dsDeleting))
            PostMessage(Handle(), amChildCreated, (WPARAM)FindPlacementControl((Control*)sender), param.lParam);
    }

    void DesignForm::controlpaint(void *sender, PaintParameters param)
    {
        Control *control = (Control*)sender;
        PlacementControl *pc = FindPlacementControl(control);
        if (pc->selected)
        {
            Rect r = control->ClientRect();
            if (!param.updaterect.Empty())
                r = r.Intersect(param.updaterect);

            bool mainsel = LastSelected() == pc;
            if (ActiveDesigner() && application->Active())
                param.canvas->SetBrush(Color(mainsel ? clHotlight : clHighlight).SetA(mainsel ? 70 : 50));
            else
                param.canvas->SetBrush(Color(mainsel ? clBlack : clGray).SetA(60));
            param.canvas->FillRect(r);
        }
    }

    void DesignForm::controlendsizemove(void *sender, SizePositionChangedParameters param)
    {
        Modify();

        if (!current || sender != current->control || sizing || dragging || placing)
            return;
        PlaceSizers();
    }

    void DesignForm::PlaceSizers()
    {
        if (designstate.contains(dsCreating) || designstate.contains(dsDeserializing) || !current)
            return;

        Rect r;
        if (selarea.Empty())
            r = ControlRect(current->control);
        else
        {
            r = selarea;
            Point p = ScreenToClient(current->control->ClientToScreen(0, 0));
            OffsetRect(&r, p.x, p.y);
        }

        for (auto it = sizers.begin(); it != sizers.end(); it++)
        {
            DesignSizer *sizer = it->second;

            sizer->PlaceOnRect(r);
            if (DesignMain())
                sizer->MoveBelow(DesignMain());
            else
                sizer->MoveToTop();
            sizer->Show();
            RedrawWindow(sizer->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
    }

    void DesignForm::HideSizers()
    {
        for (auto it = sizers.begin(); it != sizers.end(); it++)
            it->second->Hide();
    }

    void DesignForm::InvalidateSizers()
    {
        for (auto it = sizers.begin(); it != sizers.end(); it++)
            it->second->Invalidate();
    }

    void DesignForm::SelectNext(bool backwards)
    {
        bool overflowed = false;
        if (containerform && containerform->Open() && containerform->LastSelected() != nullptr)
        {
            if (containerform->SelectNext(backwards, false))
                return;
            else
                overflowed = true;
        }

        if (current != nullptr && current->control->DesignTabNext(false, backwards))
            return;

        PlacementControl *p = backwards ? PrevPlacementControl(current, overflowed || containerform == NULL || !containerform->Open()) : NextPlacementControl(current, overflowed || containerform == NULL || !containerform->Open());
        if (containerform && !p && containerform->Open() && containerform->Count())
            containerform->SetSelected(backwards ? containerform->Count() - 1 : 0);

        if (p)
        {
            if (p->control->DesignTabNext(true, backwards))
                return;
            if (p != current)
                SetCurrent(p);
        }
    }

    void DesignForm::SetCurrent(PlacementControl *placement)
    {
        if (selected.empty() && current == placement && !selpropowner && selarea.Empty() && (!containerform || !containerform->SelCount()) && EditedMenu() == nullptr)
            return;

        ClearSelection(this);
        if (placement == NULL)
        {
            if (current)
                NotifySelectionChange(current, false);

            current = NULL;
            HideSizers();
            SetProperties();
            return;
        }

        if (current && current != placement)
            NotifySelectionChange(current, false);

        current = placement;
        PlaceSizers();
        SetProperties();

        if (current)
            NotifySelectionChange(current, true);
    }

    void DesignForm::SetCurrent(PlacementControl *placement, Rect area, Object *propowner, tagtype tag)
    {
        if (selected.empty() && placement->control == selmaincontrol && current == placement && selarea == area && selpropowner == propowner && tag == seltag)
            return;

        if (!selected.empty() || current != placement)
            ClearSelection(this);

        if (current && current != placement)
            NotifySelectionChange(current, false);

        current = placement;
        selarea = area;
        selmaincontrol = placement->control;

        if (selpropowner != propowner || !propowner || seltag != tag)
        {
            seltag = tag;
            selpropowner = propowner;
            SetProperties();
        }

        if (current)
            NotifySelectionChange(current, true);

        if (!selarea.Empty())
            DisableSizing();
        else
            EnableSizing();
        PlaceSizers();
    }

    void DesignForm::NotifySelectionChange(PlacementControl *pc, bool selected)
    {
        if (designstate.contains(dsDeserializing))
            return;

        Control *c = pc->control;
        while (pc && pc->control->PropertyOwner()->DesignSelectChanged(c->PropertyOwner(), selected))
            pc = pc->parent;
    }

    void DesignForm::SelectDesignControl(Control *control, Rect area, Object *propowner, tagtype tag)
    {
        PlacementControl *pc = FindPlacementControl(control);
        if (!pc)
            return;

        SetCurrent(pc, area, propowner, tag);
    }

    bool DesignForm::GetSelectData(Object *maincontrol, Control* &control, Object* &propowner, tagtype &tag)
    {
        if (!current || current->selected || (current->control != maincontrol && (current->control->MainControl() == NULL || current->control->MainControl() != maincontrol)))
        {
            control = NULL;
            propowner = NULL;
            tag = 0;
            return false;
        }

        control = current->control;
        propowner = selpropowner;
        tag = seltag;
        return true;
    }

    bool DesignForm::IsControlSelected(Object *maincontrol, bool checksubcontrol)
    {
        return (current && !current->selected) && (current->control == maincontrol || (checksubcontrol && current->control->MainControl() == maincontrol));
    }

    bool DesignForm::IsPropertyOwnerSelected(Object *maincontrol, Object *propowner, bool truecontrolpart)
    {
        return (current && !current->selected) && ((truecontrolpart && current->control->PropertyOwner() == propowner && (current->control == maincontrol || current->control->MainControl() == maincontrol)) || (!truecontrolpart && selpropowner == propowner && selmaincontrol == maincontrol));
    }

    void DesignForm::DisableSizing()
    {
        for (auto it = sizers.begin(); it != sizers.end(); ++it)
            (*it).second->SetEnabled(false);
    }

    void DesignForm::EnableSizing()
    {
        for (auto it = sizers.begin(); it != sizers.end(); ++it)
            (*it).second->SetEnabled(true);
    }

    void DesignForm::SetProperties()
    {
        if (designstate.contains(dsCreating) || designstate.contains(dsDeserializing) || !ActiveDesigner() || !designer)
            return;

        if (current)
            designer->SetProperties(selpropowner ? selpropowner : current->control->PropertyOwner());
        else if (selected.empty() && (!containerform || containerform->SelCount() == 0) && (EditedMenu() == nullptr || EditedMenu()->SelCount() == 0))
            designer->SetProperties(this);
        else
        {
            std::list<Object*> sellist;
            for (auto it = selected.begin(); it != selected.end(); ++it)
                sellist.push_back((*it)->control->PropertyOwner());

            for (int ix = 0; containerform && ix < containerform->SelCount(); ++ix)
                sellist.push_back(containerform->SelControls(ix));

            //for (int ix = 0; designmain && ix < designmain->SelCount(); ++ix)
            //    sellist.push_back(designmain->SelItems(ix)->Menu());

            TopDesignMenuItems *menu = EditedMenu();
            for (int ix = 0; menu && ix < menu->SelCount(); ++ix)
                sellist.push_back(menu->SelItems(ix)->Menu());

            if (!sellist.empty())
                designer->SetProperties(sellist);
            else
                designer->SetProperties(this);
        }
    }

    DesignForm::PlacementControl* DesignForm::FindPlacementControl(Control *c)
    {
        if (c == NULL)
            return NULL;

        PlacementMapIterator it;
        if ((it = controlmap.find(c)) != controlmap.end())
            return (*it).second;

        return GetPlacementParent(c);
    }

    void DesignForm::StartSizing(DesignSizerSides side, Point clientpos)
    {
        SetCapture(Handle());
        HideSizers();
        sizing = true;
        sizeside = side;

        Rect r = ControlRect(current->control);

        if (snaptogrid)
        {
            clientpos.x -= clientpos.x % gridsize;
            clientpos.y -= clientpos.y % gridsize;
        }


        switch(sizeside)
        {
            case dssLeft:
            case dssRight:
                mousepos.y = r.top;
                mousepos.x = sizeside == dssLeft ? r.right : r.left;
            break;
            case dssTop:
            case dssBottom:
                mousepos.x = r.left;
                mousepos.y = sizeside == dssTop ? r.bottom : r.top;
            break;
            case dssTopLeft:
            case dssTopRight:
                mousepos.x = sizeside == dssTopLeft ? r.right : r.left;
                mousepos.y = r.bottom;
            break;
            case dssBottomLeft:
            case dssBottomRight:
                mousepos.x = sizeside == dssBottomLeft ? r.right : r.left;
                mousepos.y = r.top;
            break;
        }

        switch(side)
        {
            case dssBottomLeft:
            case dssLeft:
                sizepos = Point(clientpos.x - r.left, clientpos.y - r.bottom);
            break;
            case dssTopRight:
            case dssRight:
                sizepos = Point(clientpos.x - r.right, clientpos.y - r.top);
            break;
            case dssBottomRight:
            case dssBottom:
                sizepos = Point(clientpos.x - r.right, clientpos.y - r.bottom);
            break;
            case dssTopLeft:
            case dssTop:
                sizepos = Point(clientpos.x - r.left, clientpos.y - r.top);
            break;
        }
    }

    DesignForm::PlacementControl* DesignForm::FindNextControlOnSide(PlacementControl *placement, DesignDirections dir)
    {
        Rect r = ControlRect(placement->control);

        float midpt = dir == dirLeft || dir == dirRight ? r.left + float(r.Width()) / 2.0f : r.top + float(r.Height()) / 2.0f;
        float dist;

        Control *onside = NULL;
        PlacementControl *pc = FindPlacementControl(placement->control->Parent());
        Control *cc = pc != NULL ? pc->control : this;
        for (int ix = cc->ControlCount() - 1; ix >= 0; --ix)
        {
            Control *c = cc->Controls(ix);

            if (!FindPlacementControl(c) || c == placement->control)
                continue;

            Rect r2 = ControlRect(c);
            if ((RelativeSide(r, r2) & dir) != dir)
                continue;

            float midpt2 = dir == dirLeft || dir == dirRight ? r2.left + float(r2.Width()) / 2.0f : r2.top + float(r2.Height()) / 2.0f;
            float dist2 = abs(midpt - midpt2);

            if (!onside || dist > dist2)
            {
                onside = c;
                dist = dist2;
            }
        }
        if (!onside)
            return NULL;
        return FindPlacementControl(onside);
    }

    DesignDirections DesignForm::RelativeSide(Rect r, Rect r2)
    {
        float x = r.left + float(r.Width()) / 2.0f;
        float x2 = r2.left + float(r2.Width()) / 2.0f;

        float y = r.top + float(r.Height()) / 2.0f;
        float y2 = r2.top + float(r2.Height()) / 2.0f;

        float dx = abs(x2 - x);
        float dy = abs(y2 - y);
        if (dx < 0.05 && dy < 0.05)
            return dirNone;

        DesignDirections result = dirNone;
        if (abs(x2 - x) > abs(y2 - y))
            result = DesignDirections(int(result) | int(x < x2 ? dirRight : dirLeft));
        else if (abs(x2 - x) < abs(y2 - y))
            result = DesignDirections(int(result) | int(y < y2 ? dirDown : dirUp));

        float ymin = r.top - dx;
        float ymax = r.bottom + dx;

        float xmin = r.left - dy;
        float xmax = r.right + dy;

        float xmatch = max(min(ymax, r2.bottom) - max(ymin, r2.top) , 0);
        float ymatch = max(min(xmax, r2.right) - max(xmin, r2.left), 0);

        bool same = abs(xmatch - ymatch) < 0.05;

        if ((xmatch > ymatch && dx > 0.05) || same)
            result = DesignDirections(int(result) | int(x < x2 ? dirRight : dirLeft));

        if ((xmatch < ymatch && dy > 0.05) || same)
            result = DesignDirections(int(result) | int(y < y2 ? dirDown : dirUp));

        return result;
    }

    void DesignForm::DrawSelRect(const Rect &r)
    {
        if (lastselrect == r)
            return;

        HideSelRect();

        lastselrect = r;
        if (r.Width() == 0 && r.Height() == 0)
            return;

        HDC dc = GetDCEx(Handle(), NULL, DCX_PARENTCLIP);
        DrawFocusRect(dc, &r);
        ReleaseDC(Handle(), dc);
    }

    void DesignForm::HideSelRect()
    {
        if (lastselrect.Width() == 0 && lastselrect.Height() == 0)
            return;

        HDC dc = GetDCEx(Handle(), NULL, DCX_PARENTCLIP);
        DrawFocusRect(dc, &lastselrect);
        ReleaseDC(Handle(), dc);

        lastselrect = Rect();
    }

    void DesignForm::ClearSelection(Object *sender)
    {
        if (current)
            current->selected = false;
        for (auto it = selected.begin(); it != selected.end(); it++)
        {
            (*it)->selected = false;
            (*it)->control->Invalidate();
        }
        selected.clear();

        selarea = Rect();
        selpropowner = NULL;
        seltag = 0;

        EnableSizing();

        if (containerform)
            containerform->ClearSelection(sender);
        //if (designmenu && sender != designmenu)
        //    designmenu->Close();
        if (EditedMenu() && dynamic_cast<TopDesignMenuItems*>(sender) != EditedMenu())
            CloseEditedMenu();

        if (current)
            NotifySelectionChange(current, false);

        current = NULL;

        HideSizers();
    }

    bool DesignForm::ActiveDesigner()
    {
        return designer && designer->ActiveForm() == this; //(designer && (designer->ActiveForm() == this /*|| (containerform && designer->ActiveForm() == containerform)*/)) || Active();// || (containerform && containerform->Active());
    }

    std::wstring DesignForm::BaseClass()
    {
        return L"Form";
    }

    void DesignForm::ActiveFormChanging(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow)
    {
        base::ActiveFormChanging(activated, mouseactivate, otherform, otherwindow);

        if (EditedMenu())
            EditedMenu()->Invalidate();

        if (!designer)
            return;

        if (activated || (containerform && otherform == containerform))
        {
            designer->SetActiveForm(this);
            SetProperties();
        }
        if (!containerform || otherform != containerform)
        {
            for (auto pc : selected)
                RedrawWindow(pc->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN); 
        }
    }

    void DesignForm::DeactivateSelection()
    {
        for (auto pc : selected)
            RedrawWindow(pc->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN); 
        InvalidateSizers();
        if (containerform)
            containerform->DeactivateSelection();
        CancelAction();
        CloseMenus();
    }

    void DesignForm::ActivateSelection()
    {
        for (auto pc : selected)
            RedrawWindow(pc->control->Handle(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN); 
        if (current)
        {
            PlaceSizers();
            InvalidateSizers();
        }
        if (containerform)
            containerform->ActivateSelection();
    }

    void DesignForm::UpdateSelection()
    {
        if (!containerform && EditedMenu() == nullptr)
            return;

        if (containerform && !containerform->Count())
        {
            containerform->Destroy();
            containerform = nullptr;
            if (EditedMenu() == nullptr)
                return;
        }

        if ((current != NULL) == ((!containerform || containerform->SelCount() == 0) && (EditedMenu() == nullptr || EditedMenu()->SelCount() == 0)) || (current == NULL && selected.empty()) || (!selected.empty() && selected.front() != selected.back()))
        {
            SetProperties();
            return;
        }

        if (current) // Change from current to selected, as there is something selected on the container or in the menu.
        {
            current->selected = true;
            selected.push_back(current);
            current->control->Invalidate();

            NotifySelectionChange(current, false);

            current = NULL;
            HideSizers();
        }
        else // Change from selected to current as the last item was deselected on the container or in the menu.
        {
            current = selected.front();
            selected.clear();
            current->selected = false;
            current->control->Invalidate();
            PlaceSizers();
        }

        SetProperties();

        if (current)
            NotifySelectionChange(current, true);
    }

    void DesignForm::DeleteSelected()
    {
        if (designer && designer->ActiveForm() == this)
            designer->BeginControlChange();

        if (containerform)
            containerform->DeleteSelected();

        // The containerform can be destroyed in the previous line after a non-visual control notified the main form of its destruction. Check for containerform again.
        if (containerform && !containerform->Count())
        {
            containerform->Destroy();
            containerform = nullptr;
        }

        std::list<PlacementControl*> delsel;
        for (auto it = selected.begin(); it != selected.end(); it++)
            if (!HasSelectedParent(*it))
                delsel.push_back(*it);

        PlacementControl *nextcurrent = !delsel.empty() ? PrevPlacementControl(LastSelected(), containerform == NULL || !containerform->Open()) : controls.empty() ? NULL : controls.front();
        PlacementControl *firstcurrent = nextcurrent;
        while (nextcurrent && (nextcurrent->selected || HasSelectedParent(nextcurrent)) && nextcurrent != LastSelected())
        {
            nextcurrent = PrevPlacementControl(nextcurrent, containerform == NULL || !containerform->Open());
            if (firstcurrent == nextcurrent)
                nextcurrent = NULL;
        }
        if (nextcurrent == LastSelected())
            nextcurrent = NULL;

        selected.clear();

        for (auto it = delsel.begin(); it != delsel.end(); it++)
            DeletePlacementControl(*it);

        if (designer && designer->ActiveForm() == this)
            designer->EndControlChange();

        if (!nextcurrent && containerform && containerform->Open() && containerform->Count())
        {
            containerform->SetSelected(containerform->Count() - 1);
            return;
        }

        SetCurrent(nextcurrent);
    }

    void DesignForm::DeleteNotify(Object *object)
    {
        if (object == menubar)
        {

            DesignSetMenu(nullptr);
        }

        base::DeleteNotify(object);

        if (dynamic_cast<NonVisualControl*>(object) != NULL && containerform)
            containerform->DeleteNotify(object);
    }

    void DesignForm::ChangeNotify(Object *object, int changetype)
    {
        base::ChangeNotify(object, changetype);

        if (dynamic_cast<NonVisualControl*>(object) != NULL && containerform)
            containerform->ChangeNotify(object, changetype);
    }

    void DesignForm::RemoveNotify(Object *object)
    {
        if (object == menubar)
            DesignSetMenu(NULL);

        for (PlacementControl *pc : controls)
            pc->control->DeleteNotify(object);

        if (containerform)
            containerform->RemoveNotify(object);
    }

    bool DesignForm::HasSelectedChild(PlacementControl *placement)
    {
        for (PlacementControl *pc : selected)
            if (HasAsChild(placement, pc))
                return true;
        return false;
    }

    bool DesignForm::HasSelectedParent(PlacementControl *placement)
    {
        for (auto it = selected.begin(); it != selected.end(); it++)
            if (HasAsChild(*it, placement))
                return true;
        return false;
    }

    void DesignForm::FilterSelection()
    {
        if (containerform)
            containerform->ClearSelection(this);

        PlacementControl *p = LastSelected()->parent;
        bool sameparent = true;
        for (auto it = selected.begin(); it != selected.end() && sameparent; it++)
            if (*it != LastSelected() && (*it)->parent != p)
                sameparent = false;
        if (!sameparent)
        {
            std::list<PlacementControl*> keepsel;

            for (auto pc : selected)
            {
                if (pc == LastSelected() || pc->parent == p)
                    keepsel.push_back(pc);
                else
                {
                    pc->control->Invalidate();
                    pc->selected = false;
                }
            }

            selected = keepsel;
        }

        if (!selected.empty() && selected.front() == selected.back()) // Only one item left in selection list. Set it as current instead.
            SetCurrent(selected.front());
    }

    DesignForm::PlacementControl* DesignForm::LastSelected()
    {
        return selected.empty() ? nullptr : selected.back();
    }

    void DesignForm::formstartsizemove(void *sender, EventParameters param)
    {
        HideSizers();
    }

    void DesignForm::formendsizemove(void *sender, SizePositionChangedParameters param)
    {
        if (current)
            PlaceSizers();
    }

    void DesignForm::pmalignclick(void *sender, EventParameters param)
    {
        MenuItem *mi = (MenuItem*)sender;

        PlacementControl *last = LastSelected();

        if (mi != pmiHSpace && mi != pmiVSpace)
        {
            for (auto pc : selected)
            {
                if (mi != pmiWidths && mi != pmiHeights && pc->parent != last->parent)
                    continue;
                if (mi == pmiLefts)
                    pc->control->SetLeft(last->control->Left());
                else if (mi == pmiRights)
                    pc->control->SetLeft(last->control->Left() + last->control->Width() - pc->control->Width());
                else if (mi == pmiCenters)
                    pc->control->SetLeft(last->control->Left() + last->control->Width() / 2 - pc->control->Width() / 2);
                else if (mi == pmiTops)
                    pc->control->SetTop(last->control->Top());
                else if (mi == pmiBottoms)
                    pc->control->SetTop(last->control->Top() + last->control->Height() - pc->control->Height());
                else if (mi == pmiVCenters)
                    pc->control->SetTop(last->control->Top() + last->control->Height() / 2 - pc->control->Height() / 2);
                else if (mi == pmiWidths)
                    pc->control->SetWidth(last->control->Width());
                else if (mi == pmiHeights)
                    pc->control->SetHeight(last->control->Height());
            }
        }
        else if (std::next(selected.front()) != selected.back())
        {
            std::vector<PlacementControl*> list;
            for (auto pc : selected)
                if (pc->parent == last->parent)
                    list.push_back(pc);

            if (mi == pmiHSpace)
                std::sort(list.begin(), list.end(), [](PlacementControl *a, PlacementControl *b) { return a->control->Left() < b->control->Left(); } );
            else
                std::sort(list.begin(), list.end(), [](PlacementControl *a, PlacementControl *b) { return a->control->Top() < b->control->Top(); } );
            float pos = 0;
            float size;
            if (mi == pmiHSpace)
                size = list.back()->control->Left() - (list.front()->control->Left() + list.front()->control->Width());
            else
                size = list.back()->control->Top() - (list.front()->control->Top() + list.front()->control->Height());
            for (auto it = std::next(list.begin()); it != std::prev(list.end()); ++it)
                if (mi == pmiHSpace)
                    size -= (*it)->control->Width();
                else
                    size -= (*it)->control->Height();
            size /= list.size() - 1;
            for (auto it = list.begin(), it2 = std::next(list.begin()); it2 != std::prev(list.end()); it = it2++)
            {
                Control *pc1 = (*it)->control;
                Control *pc2 = (*it2)->control;

                if (mi == pmiHSpace)
                    pc2->SetLeft(pc1->Left() + pc1->Width() + int((pos + size) - int(pos)));
                else
                    pc2->SetTop(pc1->Top() + pc1->Height() + int((pos + size) - int(pos)));
                pos += size;
            }
        }
    }

    void DesignForm::CollectEvents(std::vector<EventListItem*> &events)
    {
        base::CollectEvents(events);
        if (containerform)
            containerform->CollectEvents(events);

        //DesignSerializer *serializer = Serializer();
        //serializer->CollectEvents(events, this);
        //if (containerform)
        //    containerform->CollectEvents(events);

        //std::vector<PlacementControl*> list;
        //PlacementControl *pc = controls.empty() ? NULL : controls.front();

        //Object *control;
        //while (pc)
        //{
        //    control = pc->control->PropertyOwner();
        //    control->Serializer()->CollectEvents(events, control);

        //    if (!pc->controls.empty())
        //    {
        //        pc = pc->controls.front();
        //        control = pc->control->PropertyOwner();
        //    }
        //    else
        //    {
        //        do {
        //            PlacementList &ctrls = pc->parent ? pc->parent->controls : controls;
        //            PlacementIterator it = pc->controlpos;
        //            ++it;

        //            if (it != ctrls.end())
        //            {
        //                pc = *it;
        //                break;
        //            }
        //            else
        //                pc = pc->parent;
        //        } while (pc);
        //    }
        //}
    }

    void DesignForm::HeaderMemberList(Indentation &indent, std::wiostream &stream, AccessLevels access)
    {
        if (containerform)
            containerform->HeaderMemberList(indent, stream, access);

        std::vector<PlacementControl*> list;
        PlacementControl *pc = controls.empty() ? NULL : controls.front();

        Object *control;
        while (pc)
        {
            control = pc->control->PropertyOwner();
            control->Serializer()->DeclareSerialize(indent, stream, control, access);
            if (!pc->controls.empty())
            {
                pc = pc->controls.front();
                control = pc->control->PropertyOwner();
                continue;
            }

            do
            {
                PlacementList &ctrls = pc->parent ? pc->parent->controls : controls;
                PlacementIterator it = pc->controlpos;
                ++it;

                if (it != ctrls.end())
                {
                    pc = *it;
                    break;
                }
                else
                    pc = pc->parent;
            } while (pc);
        }
    }

    void DesignForm::HeaderExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, bool printpragma)
    {
        base::HeaderExport(indent, stream, events, printpragma);
    }

    bool DesignForm::PrintShowAfterMemberList()
    {
        return DesignVisible();
    }

    void DesignForm::CppMemberList(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents)
    {
        DesignSerializer *serializer = Serializer();

        serializer->CppExport(indent, L"", true, stream, sdelayed, sevents, this);
        stream << std::endl;

        if (containerform)
            containerform->CppMemberList(indent, stream, sdelayed, sevents);

        std::vector<PlacementControl*> list;
        PlacementControl *pc = controls.empty() ? NULL : controls.front();

        Object *control = pc ? pc->control->PropertyOwner() : this;
        while (pc)
        {
            control->Serializer()->ConstructExport(indent, stream, sdelayed, sevents, pc->parent == NULL ? NULL : pc->parent->control, control, pc->control->ParentProperty());

            if (!pc->controls.empty())
            {
                pc = pc->controls.front();
                control = pc->control->PropertyOwner();
                continue;
            }

            do
            {
                PlacementList &ctrls = pc->parent ? pc->parent->controls : controls;
                PlacementIterator it = pc->controlpos;
                ++it;

                if (it != ctrls.end())
                {
                    pc = *it;
                    control = pc->control->PropertyOwner();
                    break;
                }
                else
                    pc = pc->parent;
            } while (pc);
        }
    }

    void DesignForm::CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events)
    {
        print_object_declaration(indent, stream, false);

        print_member_initialization(indent, stream);

        // Print the constructor calling the initialization function.
        print_c_and_d_structor_definition(indent, stream, wddConstructor | wddDestructor | wddDestroyer);

        for (auto e : events)
        {
            if (e->func.empty())
                continue;

            print_event_handler_lines(indent, stream, e->func, e->type);
        }
    }

    void DesignForm::Serialize(Indentation &indent, std::wiostream &stream, const std::vector<PlacementControl*> &list, std::vector<NonVisualControl*> &nvlist)
    {
        std::vector<PlacementControl*> controlstack;

        if (containerform)
            containerform->Serialize(indent, stream, nvlist);

        for (auto *pc : list)
        {
            Object *control = pc->control->PropertyOwner();

            while (controlstack.size() && pc->parent != controlstack.back())
            {
                stream << --indent << L"}" << std::endl;

                controlstack.pop_back();
            }

            controlstack.push_back(pc);

            DesignSerializer *serializer = control->Serializer();
            if (!serializer)
                throw L"No serializer for control during export.";

            if (!pc->control->ParentProperty())
                stream << std::endl << indent << L"type " << ShortNamespaceName(control->ClassName(true)) << std::endl;
            else
                stream << std::endl << indent << L"subtype " << pc->control->ParentProperty()->Name() << L" : " << ShortNamespaceName(control->ClassName(true)) << std::endl;
            stream << indent << L"{" << std::endl;
            indent++;
            serializer->Serialize(indent, stream, control);
        }

        while (controlstack.size())
        {
            stream << --indent << L"}" << std::endl;

            controlstack.pop_back();
        }
    }

    void DesignForm::CollectControls(std::vector<PlacementControl*> &list, std::vector<NonVisualControl*> &nvlist, bool noguest, bool selected)
    {
        PlacementControl *pc = controls.empty() ? NULL : controls.front();
        int addinglevel = 0;
        while (pc)
        {
            if (addinglevel || !selected || pc->selected || pc == current)
                list.push_back(pc);

            if (!pc->controls.empty())
            {
                if (!selected || pc->selected || pc == current || addinglevel)
                    ++addinglevel;
                pc = pc->controls.front();
            }
            else
            {
                do
                {
                    PlacementList &ctrls = pc->parent ? pc->parent->controls : controls;
                    PlacementIterator it = pc->controlpos;
                    ++it;

                    if (it != ctrls.end())
                    {
                        pc = *it;
                        break;
                    }
                    else
                    {
                        if (addinglevel)
                            --addinglevel;
                        pc = pc->parent;
                    }
                } while (pc);
            }
        }

        if (!containerform)
            return;

        containerform->CollectControls(nvlist, noguest, selected);
    }

    void DesignForm::WriteToStream(Indentation &indent, std::wiostream &stream)
    {
        std::vector<PlacementControl*> vec;
        std::vector<NonVisualControl*> nvec;
        CollectControls(vec, nvec, true, false);

        stream << indent << L"type Form" << std::endl;
        stream << indent << L"{" << std::endl;

        DesignSerializer *serializer = Serializer();
        ++indent;
        serializer->Serialize(indent, stream, this);

        Serialize(indent, stream, vec, nvec);

        --indent;
        stream << indent << L"}" << std::endl;
    }

    void DesignForm::ReadFromStream(TokenStream &token)
    {
        if (token.peek() != L"{")
            throw TokenE(L"Excpected \"{\"", token);

        std::vector<ControlDeserializerItem*> items;

        DeserializeFromStream(NULL, token, items, BaseClass());

        if (!ProcessDeserializedItems(NULL, items))
            throw Exception(L"Fix this");
    }

    void DesignForm::CopySerialized()
    {
        std::wstringstream stream;
        std::vector<PlacementControl*> vec;
        std::vector<NonVisualControl*> nvec;
        CollectControls(vec, nvec, false, true);
        Indentation indent(false, 4);
        Serialize(indent, stream, vec, nvec);

        CopyToClipboard(stream.str());
    }

    void DesignForm::PasteSerialized(PlacementControl *parent)
    {
        std::wstring pasted;
        if (!PasteFromClipboard(pasted))
            return;

        std::wstringstream stream(pasted);
        std::vector<ControlDeserializerItem*> items;
        TokenStream token(stream);

        try
        {
            while (parent && !parent->control->IsControlParent())
                parent = parent->parent;
            DeserializeFromStream(parent ? parent->control : this, token, items);
            if (designer && designer->ActiveForm() == this)
                designer->BeginControlChange();
            ProcessDeserializedItems(parent, items);
            if (designer && designer->ActiveForm() == this)
                designer->EndControlChange();

            //if (HandleCreated())
            //{
            //    for (auto *c : controls)
            //        HijackChildrenProc(c);
            //}
        }
        catch(TokenE &e)
        {
            FreeDeserializerList(items);
            ShowMessageBox(e.what(), L"Paste error", mbOk);
            return;
        }
    }

    bool DesignForm::ProcessDeserializedItems(PlacementControl *parent, std::vector<ControlDeserializerItem*> &items)
    {
        bool formparent = items.size() && items[0]->type == typeid(DesignForm); // The first type is a form. This can only happen if a form is being initialized, i.e. when loading a project.
        if (formparent && parent != NULL)
        {
            FreeDeserializerList(items);
            return false;
        }

        for (auto it = items.begin() + 1; it != items.end(); ++it)
        {
            if ((*it)->type == typeid(DesignForm) || (formparent && !(*it)->parent))
            {
                FreeDeserializerList(items);
                return false;
            }
            else if (formparent && (*it)->parent->type == typeid(DesignForm))
                (*it)->parent = NULL;
        }

        PlacementList &tlist = parent ? parent->tablist : tablist;
        PlacementList &clist = parent ? parent->controls : controls;

        std::vector<std::pair<Object*, ControlDeserializerItem*>> subparents; // List of parents for sub controls.

        std::vector<PlacementControl*> pitems; // Items that are directly placed on the given parent.
        std::vector<Rect> itembounds; // New bounding rectangle of items, which is set only at the end. The right and bottom coordinates are used to set the width and height instead.
        PlacementIterator tinsert = tlist.end(); // Position to insert new controls in the lowest parent of insertion.
        PlacementIterator cinsert = clist.end();
        // Find the first item in the parent's tab list which doesn't use the tab character, so we can place everything before it.
        PlacementRIterator rit = tlist.rbegin();

        while (rit != tlist.rend() && !(*rit)->control->controlstyle.contains(csInTabOrder))
            rit++;
        tinsert = rit.base();

        std::vector<int> taborderlist;

        //SerializerPropertyIterator tabpos;

        designstate << dsDeserializing;
        // Disable layouting while pasting controls.
        if (parent && parent->control->HandleCreated())
            parent->control->BeginUpdate();
        else if (HandleCreated())
            BeginUpdate();

        int lastnegativetaborder = -1;
        std::vector<ControlDeserializerItem*> nvitems; // List for non-visual controls passed to the containerform later in this function.
        std::vector<std::pair<Object*, ControlDeserializerProperty*>> delayedprop; // Properties that are restored at the end of deserialization.

        std::wstring menuname; // Saved name for the menu property to check whether the form has a real menu or not. If there is a menu, every control must be placed a few pixels lower.

#ifdef MEASURETIME
        LARGE_INTEGER time_freq;
        if (QueryPerformanceFrequency(&time_freq) != FALSE)
            ShowMessageBox(L"We have a timer!", L"", mbOk);

        LARGE_INTEGER time_create;
        time_create.QuadPart = 0;
        LARGE_INTEGER time_findparent;
        time_findparent.QuadPart = 0;
        LARGE_INTEGER time_setparent;
        time_setparent.QuadPart = 0;
        LARGE_INTEGER time_hijack;
        time_hijack.QuadPart = 0;
        LARGE_INTEGER time_proploop;
        time_proploop.QuadPart = 0;
        LARGE_INTEGER time_propfill;
        time_propfill.QuadPart = 0;

        LARGE_INTEGER time1, time2, time3, time4;
#endif

        for (unsigned int ix = 0; ix < items.size(); ++ix)
        {
            ControlDeserializerItem *cd = items[ix];

            bool nvsub = false;
            if (ObjectTypeByTypeInfo(cd->type) == otSubControl) // Check whether this sub control is on a non visual control.
            {
                ControlDeserializerItem *item = cd;
                while (!nvsub && item && ObjectTypeByTypeInfo(item->type) == otSubControl)
                {
                    item = item->parent;
                    if (item && ObjectTypeByTypeInfo(item->type) == otNonVisual)
                        nvsub = true;

                }
            }

            if (nvsub || ObjectTypeByTypeInfo(cd->type) == otNonVisual)
            {
                nvitems.push_back(cd);
                items.erase(items.begin() + ix);
                --ix;
                continue;
            }

            PlacementControl *pc = nullptr; // Placeholder which manages the control placed on the form.
            if (ObjectTypeByTypeInfo(cd->type) == otSubControl)
            {
                if (!cd->parent && !parent)
                    throw L"Sub controls must have a parent!";

                while(!subparents.empty() && subparents.back().second != cd->parent)
                    subparents.pop_back();

                //// Find the parent of this sub control
                //for (unsigned int ixp = formparent ? 1 : 0; ixp < ix; ++ixp)
                //{
                //    ControlDeserializerItem *cdp = items[ixp];
                //    if (cd->parent == cdp)
                //    {
                //        PlacementControl *pcp = pitems[ixp - (formparent ? 1 : 0)];
                //        Control *cc = pcp->control->IsControlParent() ? pcp->control : NULL;
                //        if (!cc)
                //            throw L"The passed control is not a control parent!?";

                        pc = new PlacementControl;
                        Object *tmp = cd->serializer->CreateObject(subparents.empty() ? parent->control : subparents.back().first);
                        pc->control = dynamic_cast<Control*>(tmp);
                        if (!pc->control)
                        {
                            if (tmp)
                                tmp->Destroy();
                            delete pc;
                            throw L"Creation of sub control failed.";
                        }
                        pc->selected = false;
                        pc->control->SetParentProperty(cd->prop);

                        int num = 1;
                        num = max(num, NameNext(DisplayNameByTypeInfo(cd->type, false)));
                        pc->control->PropertyOwner()->SetName(DisplayNameByTypeInfo(cd->type, false) + IntToStr(num));
                //        break;
                //    }
                //}
            }
            else
            {
                //subparents.clear();
#ifdef MEASURETIME
                QueryPerformanceCounter(&time1);
                pc = cd->type != ctForm ? CreateControl(cd->type) : NULL;
                QueryPerformanceCounter(&time2);
                time_create.QuadPart += time2.QuadPart - time1.QuadPart;
#else
                pc = cd->type != typeid(DesignForm) ? CreateControl(cd->type) : NULL;
#endif
            }

            subparents.push_back(std::make_pair(pc ? pc->control : this, cd));

            if (pc)
            {
                controllist.push_back(pc);
                controlmap[pc->control] = pc;
                pitems.push_back(pc);
                pc->control->StartDeserialize();
            }

            if (cd->parent)
            {
                bool found = false;
#ifdef MEASURETIME
                QueryPerformanceCounter(&time1);
#endif
                for (unsigned int ixp = formparent ? 1 : 0; ixp < ix; ++ixp)
                {
                    ControlDeserializerItem *cdp = items[ixp];
                    if (cd->parent == cdp)
                    {
                        PlacementControl *pcp = pitems[ixp - (formparent ? 1 : 0)];
                        pc->parent = pcp;
                        Control *cc = pcp->control->IsControlParent() ? pcp->control : NULL;
                        if (cc)
                        {
                            found = true;
#ifdef MEASURETIME
                            QueryPerformanceCounter(&time3);
#endif
                            pc->controlpos = pcp->controls.insert(pcp->controls.end(), pc);
                            pc->tabpos = pcp->tablist.insert(pcp->tablist.end(), pc);
                            pc->control->SetParent(cc);
#ifdef MEASURETIME
                            QueryPerformanceCounter(&time4);
                            time_setparent.QuadPart += time4.QuadPart - time3.QuadPart;

                            QueryPerformanceCounter(&time3);
#endif
                            //-HijackChildrenProc(pc);
#ifdef MEASURETIME
                            QueryPerformanceCounter(&time4);
                            time_hijack.QuadPart += time4.QuadPart - time3.QuadPart;
#endif
                            pc->parent = pcp;
                        }
                        break;
                    }
                }
                if (!found)
                {
                    cd->parent = nullptr;
                }
#ifdef MEASURETIME
                QueryPerformanceCounter(&time2);
                time_findparent.QuadPart += time2.QuadPart - time1.QuadPart;
#endif
            }
            else if (ObjectTypeByTypeInfo(cd->type) == otSubControl)
            {
                pc->parent = parent;
                Control *cc = parent->control->IsControlParent() ? parent->control : NULL;
                if (cc)
                {
#ifdef MEASURETIME
                    QueryPerformanceCounter(&time3);
#endif
                    pc->controlpos = parent->controls.insert(parent->controls.end(), pc);
                    pc->tabpos = parent->tablist.insert(parent->tablist.end(), pc);
                    pc->control->SetParent(cc);
#ifdef MEASURETIME
                    QueryPerformanceCounter(&time4);
                    time_setparent.QuadPart += time4.QuadPart - time3.QuadPart;

                    QueryPerformanceCounter(&time3);
#endif
                    //-HijackChildrenProc(pc);
#ifdef MEASURETIME
                    QueryPerformanceCounter(&time4);
                    time_hijack.QuadPart += time4.QuadPart - time3.QuadPart;
#endif
                    pc->parent = parent;
                }
            }

            if (!cd->parent && pc && ObjectTypeByTypeInfo(cd->type) != otSubControl)
            {
#ifdef MEASURETIME
                QueryPerformanceCounter(&time3);
#endif
                pc->tabpos = tlist.insert(tinsert, pc);
                pc->controlpos = clist.insert(cinsert, pc);
                pc->control->SetParent(parent ? parent->control : this);
#ifdef MEASURETIME
                QueryPerformanceCounter(&time4);
                time_setparent.QuadPart += time4.QuadPart - time3.QuadPart;
                QueryPerformanceCounter(&time3);
#endif
                //-HijackChildrenProc(pc);
#ifdef MEASURETIME
                QueryPerformanceCounter(&time4);
                time_hijack.QuadPart += time4.QuadPart - time3.QuadPart;
#endif
                pc->parent = parent;
            }

            int newtaborder;
            Rect bounds;
            bool leftfound = false;
            bool topfound = false;
            bool widthfound = false;
            bool heightfound = false;
            bool orderfound = false;
#ifdef MEASURETIME
            QueryPerformanceCounter(&time1);
#endif
            for (auto pit = cd->properties.begin(); pit != cd->properties.end(); ++pit)
            {
                ControlDeserializerPropertyNameValue *val = dynamic_cast<ControlDeserializerPropertyNameValue*>(*pit);
                if (!leftfound && val && val->Name() == L"Left" && StrToInt(val->Value(), bounds.left))
                {
                    leftfound = true;
                    continue;
                }

                if (!topfound && val && val->Name() == L"Top" && StrToInt(val->Value(), bounds.top))
                {
                    topfound = true;
                    continue;
                }

                if (!widthfound && val && val->Name() == L"Width" && StrToInt(val->Value(), bounds.right))
                {
                    widthfound = true;
                    continue;
                }

                if (!heightfound && val && val->Name() == L"Height" && StrToInt(val->Value(), bounds.bottom))
                {
                    heightfound = true;
                    continue;
                }

                if (!orderfound && pc && pc->control->controlstyle.contains(csInTabOrder) && val && val->Name() == L"TabOrder" && StrToInt(val->Value(), newtaborder))
                {
                    taborderlist.push_back(newtaborder);
                    orderfound = true;
                    continue;
                }

                if (!pc && val && val->Name() == L"Menu")
                {
                    menuname = val->Value();
                    continue;
                }

                if ((*pit)->prop->Delayed())
                {
                    delayedprop.push_back( std::pair<Object*, ControlDeserializerProperty*>(!pc ? this : pc->control->PropertyOwner(), *pit));
                    continue;
                }

#ifdef MEASURETIME
                QueryPerformanceCounter(&time3);
#endif
                if (pc)
                    (*pit)->SetValue(pc->control->PropertyOwner());
                else
                    (*pit)->SetValue(this);
#ifdef MEASURETIME
                QueryPerformanceCounter(&time4);
                time_propfill.QuadPart += time4.QuadPart - time3.QuadPart;
#endif
            }
#ifdef MEASURETIME
            QueryPerformanceCounter(&time2);
            time_proploop.QuadPart += time2.QuadPart - time1.QuadPart;
#endif

            if (!orderfound && pc)
                taborderlist.push_back(lastnegativetaborder--);

            bounds = RectS(bounds.left, bounds.top, bounds.right, bounds.bottom);
            if (!pc)
                SetBounds(RectS(bounds.left, bounds.top, Width(), Height()));
            else
            {
                if (!cd->parent && !formparent && ObjectTypeByTypeInfo(cd->type) != otSubControl)
                {
                    int x = bounds.left;
                    int y = bounds.top;
                    GetPlacePosition(pc->parent, pc, bounds.TopLeft(), x, y);

                    OffsetRect(&bounds, x - bounds.left, y - bounds.top);
                }
                itembounds.push_back(bounds);
            }

            if (pc && pc->control->HandleCreated() && pc->control->Parent())
                HijackChildrenProc(pc);
        }

#ifdef MEASURETIME
        time_findparent.QuadPart -= time_setparent.QuadPart - time_hijack.QuadPart;
        time_proploop.QuadPart -= time_propfill.QuadPart;

        std::wstringstream msg;
        msg << L"Frequency: " << time_freq.HighPart << L"/" << time_freq.LowPart << std::endl;
        msg << L"create: " << time_create.HighPart << L"/" << time_create.LowPart << L": " << std::setprecision(3) << ((double)time_create.LowPart / time_freq.LowPart) << L"sec" << std::endl;
        msg << L"findparent: " << time_findparent.HighPart << L"/" << time_findparent.LowPart << L": " << std::setprecision(3) << ((double)time_findparent.LowPart / time_freq.LowPart) << L"sec" << std::endl;
        msg << L"setparent: " << time_setparent.HighPart << L"/" << time_setparent.LowPart << L": " << std::setprecision(3) << ((double)time_setparent.LowPart / time_freq.LowPart) << L"sec" << std::endl;
        msg << L"hijack: " << time_hijack.HighPart << L"/" << time_hijack.LowPart << L": " << std::setprecision(3) << ((double)time_hijack.LowPart / time_freq.LowPart) << L"sec" << std::endl;
        msg << L"proploop: " << time_proploop.HighPart << L"/" << time_proploop.LowPart << L": " << std::setprecision(3) << ((double)time_proploop.LowPart / time_freq.LowPart) << L"sec" << std::endl;
        msg << L"propfill: " << time_propfill.HighPart << L"/" << time_propfill.LowPart << L": " << std::setprecision(3) << ((double)time_propfill.LowPart / time_freq.LowPart) << L"sec" << std::endl;
        ShowMessageBox(msg.str(), L"", mbOk);
#endif

        if (!nvitems.empty())
        {
            if (!containerform)
                containerform = new DesignContainerForm(std::wstring(), this, controlbuttons);
            containerform->ProcessDeserializedItems(nvitems);
        }

        for (auto it = delayedprop.rbegin(); it != delayedprop.rend(); ++it)
            (*it).second->SetValue((*it).first);

        FreeDeserializerList(items);

        // Sort the tab order for every new control.
        for (int ix = -1; ix < (int)pitems.size(); ++ix)
        {
            PlacementControl *tabparent = ix < 0 ? parent : pitems[ix];
            Control *cc = tabparent && tabparent->control->IsControlParent() ? tabparent->control : !tabparent ? this : NULL;
            if (!cc)
                continue;

            PlacementList &ptlist = tabparent ? tabparent->tablist : tablist;
            PlacementIterator ptbegin = ptlist.end(), ptend;
            std::vector<int> newtaborder;
            for (unsigned int iy = (ix + 1); iy < pitems.size(); ++iy)
            {
                if (pitems[iy]->parent == tabparent)
                {
                    if (ptbegin == ptlist.end())
                        ptbegin = pitems[iy]->tabpos;
                    (ptend = pitems[iy]->tabpos)++;
                    newtaborder.push_back(taborderlist[iy]);
                }
            }
            if (ptbegin != ptlist.end())
                TabOrderSort(ptlist, ptbegin, ptend, newtaborder);

            cc->controls->tablist.clear();
            for (auto it = ptlist.begin(); it != ptlist.end() && (*it)->control->controlstyle.contains(csInTabOrder); ++it)
                cc->controls->tablist.push_back((*it)->control);
            cc->controls->UpdateChildTabOrder();
        }

        std::vector<Object*> objects;

        for (unsigned int ix = 0; ix < pitems.size(); ++ix)
        {
            objects.push_back(pitems[ix]->control);
            pitems[ix]->control->SetBounds(itembounds[ix]);
        }

        FinishProcessing(objects);


        if (!formparent)
        {
            if (pitems.size() == 1)
                SetCurrent(pitems.front());
            else
            {
                SetCurrent(NULL);

                for (auto pc : pitems)
                {
                    if (pc->parent != parent)
                        continue;
                    pc->selected = true;
                    pc->control->Invalidate();
                    selected.push_back(pc);
                }
                if (!selected.empty() && selected.front() == selected.back())
                    SetCurrent(selected.front());
            }
        }

        // Re-enable layouting of children controls on parent.
        if (parent && parent->control->HandleCreated())
        {
            parent->control->EndUpdate();
            RedrawWindow(parent->control->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
        else if (HandleCreated())
        {
            EndUpdate();
            RedrawWindow(Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }

        for (unsigned int ix = 0; ix < pitems.size(); ++ix)
        {
            Control *c = pitems[ix]->control;
            c->FinishDeserialize();
            if (designer && designer->ControlsChanging() && designer->ActiveForm() == this)
                designer->ControlAdded(c, ControlName(c, c->Name()));
        }

        designstate -= dsDeserializing;

        if (!menuname.empty())
        {
            DesignSerializer *serializer = Serializer();
            serializer->Find<MenubarDesignProperty<DesignForm>>(L"Menu")->SetValue(designer, this, menuname);
            if (DesignMain())
                SetHeight(Height() + DesignMain()->Height());
        }

        if (current)
            PlaceSizers();
        SetProperties();

        return true;
    }

    // Helper for PasteSerialized. Sets the tab order of pasted controls on the parent form.
    void DesignForm::TabOrderSort(PlacementList &tlist, PlacementIterator tbegin, PlacementIterator tend, std::vector<int> &newtaborder)
    {
        int mintab = 0;
        int minpos = -1;
        unsigned int mincursor = 0;
        PlacementIterator tpos = tbegin;
        PlacementIterator tpos2;
        PlacementIterator tpostmp;

        bool haspos; // A positive tab order was found. If not, start ordering negative tab positions.
        bool negorder = false; // Ordering of negative tab pos which should come at the end has started.
        PlacementIterator nbegin;
        PlacementIterator nend = tlist.end();
        while (mincursor < newtaborder.size())
        {
            tpostmp = tpos2 = tpos;
            haspos = false;
            for (unsigned int ix = mincursor; ix < newtaborder.size(); ++ix)
            {

                if ((!negorder && newtaborder[ix] >= 0 && (minpos < 0 || newtaborder[ix] < mintab)) || (negorder && (minpos < 0 || newtaborder[ix] > mintab || (*tpos2)->control->controlstyle.contains(csInTabOrder) )))
                {
                    tpos2 = tpostmp;
                    minpos = ix;
                    mintab = newtaborder[ix];
                    haspos = true;
                    if (negorder && (*tpos2)->control->controlstyle.contains(csInTabOrder))
                    {
                        (nbegin = tpos2)++;
                        break;
                    }
                }
                ++tpostmp;
            }
            if (!negorder && !haspos)
            {
                negorder = true;
                mintab = 0;
                nbegin = tpos;
                continue;
            }

            if (tpos2 != tpos)
            {
                PlacementControl *tmp = *tpos;
                *tpos = *tpos2;
                *tpos2 = tmp;
                (*tpos)->tabpos = tpos;
                (*tpos2)->tabpos = tpos2;
                int otmp = newtaborder[mincursor];
                newtaborder[mincursor] = newtaborder[minpos];
                newtaborder[minpos] = otmp;
            }
            ++tpos;
            ++mincursor;
            minpos = -1;

            if (negorder)
                nend = tpos;
        }

        if (tend == tlist.end() || nend == tlist.end()) // If tlist would be empty of negative numbers or we have reached the end of tlist after ordering, there is nothing left to do.
            return;

        // Put the negative tab items to the end of the tablist, unregarding the tend position.
        PlacementList neglist(nbegin, nend);
        tlist.erase(nbegin, nend);
        // tlist is still not empty, so no need to check;
        tpos = --tlist.end();
        tlist.insert(tlist.end(), neglist.begin(), neglist.end());
        while (++tpos != tlist.end())
            (*tpos)->tabpos = tpos;
    }

    Object* DesignForm::NameOwner(const std::wstring &name)
    {
        Object *obj = base::NameOwner(name);
        if (obj)
            return obj;

        for (PlacementControl *pc : controllist)
        {
            //obj = pc->control->NameOwner(name);
            //if (obj)
            //    return obj;
            if (pc->control->Name() == name)
                return pc->control;
        }

        if (containerform)
            return containerform->NameOwner(name);

        return nullptr;
    }

    void DesignForm::NamesList(std::vector<std::wstring> &namelist)
    {
        base::NamesList(namelist);

        for (auto it = controllist.begin(); it != controllist.end(); it++)
        {
            namelist.push_back((*it)->control->Name());
            //(*it)->control->Names(namelist);
        }

        if (containerform)
            containerform->NamesList(namelist);
    }

    std::wstring DesignForm::ControlName(Object *c, const std::wstring &name)
    {
        std::wstring str;
        if (c->DesignParent() != this)
            str = c->DesignParent()->Name() + L"->";
        return str + name;
    }

    void DesignForm::CollectObjects(std::vector<std::pair<std::wstring, Object*> > &objectstrings, bool (*collector)(Object*), const std::wstring &objectname)
    {
        base::CollectObjects(objectstrings, collector, objectname);

        for (PlacementControl *pc : controllist)
        {
            if (collector == NULL || collector(pc->control))
            {
                std::wstring str = ControlName(pc->control, pc->control->Name());

                if (!objectname.empty())
                {
                    if (str == objectname)
                    {
                        objectstrings.push_back(std::pair<std::wstring, Object*>(str, pc->control));
                        return;
                    }
                }
                else
                    objectstrings.push_back(std::pair<std::wstring, Object*>(str, pc->control));
            }
        }

        if (containerform)
            containerform->CollectObjects(objectstrings, collector, objectname);
    }

    std::wstring DesignForm::GetGuestControls()
    {
        if (!containerform)
            return std::wstring();

        return containerform->GetGuestControls();
    }

    void DesignForm::AddGuestControls(const std::wstring &guests)
    {
        if (guests.empty())
            return;

        if (containerform == nullptr)
            containerform = new DesignContainerForm(std::wstring(), this, controlbuttons);

        containerform->AddGuestControls(guests);

        if (containerform->Count() == 0)
        {
            containerform->Destroy();
            containerform = nullptr;
        }
    }

    void DesignForm::GetTabActivatedControls(std::vector<std::pair<std::wstring, Control*> > &controlstrings, const std::wstring &controlname)
    {
        for (auto it = controllist.begin(); it != controllist.end(); ++it)
        {
            if ((*it)->control->controlstyle.contains(csInTabOrder))
            {
                std::wstring str = ControlName((*it)->control, (*it)->control->Name());

                if (controlname.length())
                {
                    if (str == controlname)
                    {
                        controlstrings.push_back(std::pair<std::wstring, Control*>(str, (*it)->control));
                        return;
                    }
                }
                else
                    controlstrings.push_back(std::pair<std::wstring, Control*>(str, (*it)->control));
            }
        }

        if (containerform)
            containerform->GetTabActivatedControls(controlstrings, controlname);
    }

    std::pair<std::wstring, Object*> DesignForm::SelectedObject()
    {
        int cnt = (current ? 1 : selected.size()) + (containerform ? containerform->SelCount() : 0);

        if (cnt == 1)
        {
            if (current)
            {
                if (selpropowner != nullptr)
                    return std::make_pair(ControlName(selmaincontrol, SubObjectName(selmaincontrol, selpropowner)), selpropowner);
                else
                    return std::make_pair(ControlName(current->control, current->control->Name()), current->control);
            }
            else
                return containerform->SelectedObject();
        }

        //if (designmain && designmain->SelCount())
        //{
        //    if (designmain->SelCount() > 1)
        //        return std::make_pair(IntToStr(designmain->SelCount()), (Object*)nullptr);

        //    DesignMenuItem *selitem = designmain->SelItems(0);
        //    return std::make_pair(ControlName(designmain, SubObjectName(selitem->Owner(), selitem->Menu())), selitem->Menu());
        //}

        if (EditedMenu() && EditedMenu()->SelCount() != 0)
        {
            TopDesignMenuItems *menu = EditedMenu();
            if (menu->SelCount() > 1)
                return std::make_pair(IntToStr(menu->SelCount()), (Object*)nullptr);

            DesignMenuItem *selitem = menu->SelItems(0);
            return std::make_pair(ControlName(menu, SubObjectName(selitem->Menu()->TopMenu(), selitem->Menu())), selitem->Menu());
        }

        return std::make_pair(IntToStr(cnt), (Object*)nullptr);
    }

    void DesignForm::SelectObject(std::wstring name)
    {
        if (name.empty())
        {
            SetCurrent(nullptr);
            return;
        }

        if (base::SelectSubObject(name))
            return;

        for (auto c : controllist)
        {
            if (name == ControlName(c->control, c->control->Name()))
            {
                SetCurrent(c);
                return;
            }
        }

        if (containerform)
            containerform->SelectObject(name);
    }

    void DesignForm::SelectObject(Object *obj)
    {
        if (!obj)
        {
            SetCurrent(nullptr);
            return;
        }

        if (dynamic_cast<Control*>(obj) != nullptr)
        {
            auto it = controlmap.find((Control*)obj);
            if (it != controlmap.end())
            {
                SetCurrent(controlmap[(Control*)obj]);
                return;
            }
        }

        if (containerform)
            containerform->SelectObject(obj);
    }

    void DesignForm::pmenushow(void *sender, PopupMenuParameters param)
    {
        menuextra = 0;
        Point pt = ScreenToClient(param.screenpos);
        PlacementControl *pc = FindPlacementControl(ControlAt(pt));

        if (pc)
        {
            std::vector< menu_item_data > extraitems;
            Object *prowner = pc->control->PropertyOwner();
            prowner->InitDesignerMenu(pc->control->ScreenToClient(param.screenpos), extraitems);
            menuextra += extraitems.size();
            if (menuextra)
            {
                ++menuextra;
                pmenu->InsertItem(pmenu->Items(0), L"-");
            }

            std::for_each(extraitems.rbegin(), extraitems.rend(), [this](menu_item_data &item)
            {
                MenuItem *mi = pmenu->InsertItem(pmenu->Items(0), item.text);
                mi->SetTag(item.tag);
                mi->OnClick = item.OnClick;
            });
        }

        pmiPos->SetEnabled(current || !selected.empty());
        pmiAlign->SetEnabled(!selected.empty() && selected.size() >= 2);
        pmiSize->SetEnabled(!selected.empty() && selected.size() >= 2);
    }

    void DesignForm::pmenuhide(void *sender, EventParameters param)
    {
        while (menuextra-- > 0)
            pmenu->Delete(0);
    }

    void DesignForm::SendControlToBottom(PlacementControl *pc)
    {
        if (!pc)
            return;

        PlacementList &list = pc->parent ? pc->parent->controls : controls;
        if (pc == list.front() || !pc->control->Parent())
            return;

        HideSizers();
        pc->control->MoveToBottom();
        PlaceSizers();

        list.erase(pc->controlpos);
        pc->controlpos = list.insert(list.begin(), pc);

        if (pc->control->controlstyle.contains(csInTabOrder))
            return;

        // Find position of pc in the tablist among other controls not in the tab order, so tabbing through the controls give it in the correct order.
        PlacementList &list2 = pc->parent ? pc->parent->tablist : tablist;

        // Find first control's iterator which is not in the tab order of the parent.
        auto rit = std::find_if(list2.rbegin(), list2.rend(), [](PlacementControl *p) { return p->control->controlstyle.contains(csInTabOrder); });
        if (rit.base() == pc->tabpos)
            return;
        list2.erase(pc->tabpos);
        pc->tabpos = list2.insert(rit.base(), pc);
    }

    void DesignForm::MoveControlDown(PlacementControl *pc)
    {
        if (!pc)
            return;

        PlacementList &list = pc->parent ? pc->parent->controls : controls;
        if (pc == list.front() || !pc->control->Parent())
            return;

        HideSizers();
        pc->control->MoveDown();
        PlaceSizers();

        pc->controlpos = list.insert(std::prev(list.erase(pc->controlpos)), pc);

        if (pc->control->controlstyle.contains(csInTabOrder))
            return;

        // Find the first control in the control list which is not in the tab order and is above pc.
        auto above = std::find_if(std::next(pc->controlpos), list.end(), [](PlacementControl *p) { return !p->control->controlstyle.contains(csInTabOrder); });

        PlacementList &list2 = pc->parent ? pc->parent->tablist : tablist;

        // Insert the control in the tablist below the control found before.
        list2.erase(pc->tabpos);
        pc->tabpos = list2.insert(above != list.end() ? (*above)->tabpos : list2.end(), pc);
    }

    void DesignForm::MoveControlUp(PlacementControl *pc)
    {
        if (!pc)
            return;

        PlacementList &list = pc->parent ? pc->parent->controls : controls;
        if (pc == list.back() || !pc->control->Parent())
            return;

        HideSizers();
        pc->control->MoveUp();
        PlaceSizers();

        pc->controlpos = list.insert(std::next(list.erase(pc->controlpos)), pc);

        if (pc->control->controlstyle.contains(csInTabOrder))
            return;

        // Find the first control in the control list which is not in the tab order and is above pc.
        auto above = std::find_if(std::next(pc->controlpos), list.end(), [](PlacementControl *p) { return !p->control->controlstyle.contains(csInTabOrder); });

        PlacementList &list2 = pc->parent ? pc->parent->tablist : tablist;

        // Insert the control in the tablist below the control found before.
        list2.erase(pc->tabpos);
        pc->tabpos = list2.insert(above != list.end() ? (*above)->tabpos : list2.end(), pc);
    }

    void DesignForm::BringControlToFront(PlacementControl *pc)
    {
        if (!pc)
            return;

        PlacementList &list = pc->parent ? pc->parent->controls : controls;
        if (pc == list.back() || !pc->control->Parent())
            return;

        HideSizers();
        if (DesignMain())
            pc->control->MoveBelow(DesignMain());
        else
            pc->control->MoveToTop();
        PlaceSizers();

        list.erase(pc->controlpos);
        pc->controlpos = list.insert(list.end(), pc);

        if (pc->control->controlstyle.contains(csInTabOrder))
            return;

        // Controls not in the tab order come after all others, so it is safe to insert the control at the end of the tablist without checking.
        PlacementList &list2 = pc->parent ? pc->parent->tablist : tablist;
        list2.erase(pc->tabpos);
        pc->tabpos = list2.insert(list2.end(), pc);
    }

    void DesignForm::MoveControlAbove(PlacementControl *what, PlacementControl *where)
    {
        if (!what || !where || what == where || what->parent != where->parent)
            return;

        HideSizers();
        what->control->MoveAbove(where->control);
        PlaceSizers();

        PlacementList &list = what->parent ? what->parent->controls : controls;
        list.erase(what->controlpos);

        what->controlpos = list.insert(std::next(where->controlpos), what);

        if (what->control->controlstyle.contains(csInTabOrder))
            return;

        // Find the first control in the control list which is not in the tab order and is above pc.
        auto above = std::find_if(std::next(what->controlpos), list.end(), [](PlacementControl *p) { return !p->control->controlstyle.contains(csInTabOrder); });

        PlacementList &list2 = what->parent ? what->parent->tablist : tablist;

        // Insert the control in the tablist below the control found before.
        list2.erase(what->tabpos);
        what->tabpos = list2.insert(above != list.end() ? (*above)->tabpos : list2.end(), what);
    }

    void DesignForm::MoveControlBelow(PlacementControl *what, PlacementControl *where)
    {
        if (!what || !where || what == where || what->parent != where->parent)
            return;

        HideSizers();
        what->control->MoveBelow(where->control);
        PlaceSizers();

        PlacementList &list = what->parent ? what->parent->controls : controls;
        list.erase(what->controlpos);
        what->controlpos = list.insert(where->controlpos, what);

        if (what->control->controlstyle.contains(csInTabOrder))
            return;
        // Find position of what in the tablist among other controls not in the tab order, so tabbing through the controls give it in the correct order.

        // Find the first control in the control list which is not in the tab order and is above pc.
        auto above = std::find_if(std::next(what->controlpos), list.end(), [](PlacementControl *p) { return !p->control->controlstyle.contains(csInTabOrder); });

        PlacementList &list2 = what->parent ? what->parent->tablist : tablist;

        // Insert the control in the tablist below the control found before.
        list2.erase(what->tabpos);
        what->tabpos = list2.insert(above != list.end() ? (*above)->tabpos : list2.end(), what);
    }

    void DesignForm::SendControlToBottom(Control *control)
    {
        SendControlToBottom(FindPlacementControl(control));
    }

    void DesignForm::MoveControlDown(Control *control)
    {
        MoveControlDown(FindPlacementControl(control));
    }

    void DesignForm::MoveControlUp(Control *control)
    {
        MoveControlUp(FindPlacementControl(control));
    }

    void DesignForm::BringControlToFront(Control *control)
    {
        BringControlToFront(FindPlacementControl(control));
    }

    void DesignForm::MoveControlAbove(Control *what, Control *where)
    {
        MoveControlAbove(FindPlacementControl(what), FindPlacementControl(where));
    }

    void DesignForm::MoveControlBelow(Control *what, Control *where)
    {
        MoveControlBelow(FindPlacementControl(what), FindPlacementControl(where));
    }

    void DesignForm::posbottomclick(void *sender, EventParameters param)
    {
        if (current)
            SendControlToBottom(current);
        else
        {
            for (auto it = selected.begin(); it != selected.end(); ++it)
                SendControlToBottom(*it);
        }
    }

    void DesignForm::posdownclick(void *sender, EventParameters param)
    {
        if (current)
            MoveControlDown(current);
        else
        {
            for (auto it = selected.begin(); it != selected.end(); ++it)
                MoveControlDown(*it);
        }
    }

    void DesignForm::posupclick(void *sender, EventParameters param)
    {
        if (current)
            MoveControlUp(current);
        else
        {
            for (auto it = selected.begin(); it != selected.end(); ++it)
                MoveControlUp(*it);
        }
    }

    void DesignForm::posfrontclick(void *sender, EventParameters param)
    {
        if (current)
            BringControlToFront(current);
        else
        {
            for (auto it = selected.begin(); it != selected.end(); ++it)
                BringControlToFront(*it);
        }
    }

    void DesignForm::CreateNonVisualControl(const type_info &ctype)
    {
        ClearSelection(this);
        if (!containerform)
            containerform = new DesignContainerForm(std::wstring(), this, controlbuttons);
        containerform->PlaceControl(ctype);
    }

    void DesignForm::AddNonVisualControl(NonVisualControl *control)
    {
        if (control->ParentForm() == this)
            return;

        ClearSelection(this);
        if (!containerform)
            containerform = new DesignContainerForm(std::wstring(), this, controlbuttons);
        containerform->AddControl(control);
    }

    void DesignForm::WindowBoundsChanged(const Rect &oldrect, const Rect &newrect)
    {
        if (oldrect != newrect)
            Modify();

        base::WindowBoundsChanged(oldrect, newrect);
        if (containerform)
        {
            Rect r = containerform->WindowRect();
            OffsetRect(&r, newrect.right - oldrect.right, newrect.top - oldrect.top);
            containerform->SetBounds(r);
        }
        if (DesignMain())
            DesignMain()->UpdateBounds();
    }

    void DesignForm::Showing()
    {
        base::Showing();

        if (containerform)
            containerform->Show();
    }

    void DesignForm::Hiding()
    {
        base::Hiding();

        if (containerform)
            containerform->Hide();
        if (DesignMain())
            DesignMain()->Close();
    }


    //---------------------------------------------


    void DesignContainerForm::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideAll();
        //serializer->Find<UnitNameDesignProperty<DesignFormBase>>(L"UnitName", true)->ShowProperty();
        //serializer->Find(L"Name", true)->Show();
    }

    const int NVControlWidth = 125;
    const int NVControlHeight = 30;

    /* constructor */ DesignContainerForm::DesignContainerForm(const std::wstring &name, DesignForm *owner, ButtonPanel *controlbuttons) :
            controlbuttons(controlbuttons), pressed(-1), open(false), hovered(false), rows(0)
    {
        controlstyle << csShowDontActivate;
        SetColor(clAppWorkspace);

        NeedScrollbars();
        VScroll()->SetEnabled(false);
        SetVRange(999999);
        VScroll()->SetLineStep(NVControlHeight * Scaling / 1.75);
        SetVAutoPageStep(-NVControlHeight * Scaling);
        SetAutoSizeScroll(true);

        container = new Container();

        SetText(L"Non-visual controls");
        SetName(name);

        designform = owner;

        //Rect r = designform->WindowRect();

        UpdateSize(true);

        SetShowPosition(fspUnchanged);
        SetTopLevelParent(owner);
    }

    /* constructor */ DesignContainerForm::DesignContainerForm(const std::wstring &name, Form *owner, ButtonPanel *controlbuttons) :
            controlbuttons(controlbuttons), pressed(-1), open(true)
    {
        SetColor(clAppWorkspace);

        NeedScrollbars();
        VScroll()->SetEnabled(false);
        SetVRange(999999);
        VScroll()->SetLineStep(NVControlHeight * Scaling / 1.75);
        SetVAutoPageStep(-NVControlHeight * Scaling);
        SetAutoSizeScroll(true);

        //SetSizeStepWidth(NVControlWidth * Scaling);
        //SetSizeStepHeight(NVControlHeight * Scaling);

        container = new Container();

        SetText(name);
        SetName(name);

        designform = NULL;

        SetBounds(RectS(0,0,350,250));

        SetTopLevelParent(owner);
    }

    /* destructor */ DesignContainerForm::~DesignContainerForm()
    {
    }

    void DesignContainerForm::Destroy()
    {
        container->Destroy();
        base::Destroy();
    }

    void DesignContainerForm::Modify(bool tomodified)
    {
        if (!designform)
            base::Modify(tomodified);
        else
            designform->Modify(tomodified);
    }

    void DesignContainerForm::Show()
    {
        base::Show();
        if (designform)
            return;
        SetShowPosition(fspUnchanged);
        SetProperties();
    }

    void DesignContainerForm::SetProperties()
    {
        if (designstate.contains(dsCreating) || designstate.contains(dsDeserializing) || !ActiveDesigner() || !designer)
            return;

        if (selected.empty())
            designer->SetProperties(FormObj());
        else
        {
            std::list<Object*> sellist;
            for (auto it = selected.begin(); it != selected.end(); ++it)
                sellist.push_back((*it)->GetControl());

            TopDesignMenuItems *menu = EditedMenu();
            for (int ix = 0; menu && ix < menu->SelCount(); ++ix)
                sellist.push_back(menu->SelItems(ix)->Menu());

            if (!sellist.empty())
                designer->SetProperties(sellist);
            else
                designer->SetProperties(FormObj());
        }
    }

    LRESULT DesignContainerForm::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT r;
        switch(uMsg)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
            if (!designform)
                break;

            designform->PassMessage(uMsg, wParam, lParam);
            return 0;
        case WM_NCHITTEST:
            if (!designform)
                break;

            r = base::WindowProc(uMsg, wParam, lParam);
            if (open && r != HTBOTTOM && r != HTCLOSE && r != HTCLIENT && r != HTHSCROLL && r != HTVSCROLL)
                r = HTBORDER;
            return r;
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONDBLCLK:
            if (!designform)
                break;

            if (open && wParam == HTCAPTION)
                wParam = HTCLIENT;
            FormObj()->Focus();
            break;
        case WM_CLOSE:
            if (!designform)
                break;

            SetOpen(false);
            designform->Focus();
            designform->UpdateSelection();
            return 0;
        case WM_NCACTIVATE:
            if (!designform)
                break;

            if ((application->ActiveForm() == designform && (Form*)lParam != designform) || wParam != FALSE)
                wParam = TRUE;
            else
                wParam = FALSE;

            if ((Form*)lParam != designform)
                designform->PassMessage(WM_NCACTIVATE, wParam, (LPARAM)this);
            if ((Form*)lParam == designform)
                lParam = 0;
            break;
        }

        return base::WindowProc(uMsg, wParam, lParam);
    }

    DesignFormBase* DesignContainerForm::FormObj()
    {
        if (designform)
            return designform;
        return this;
    }

    void DesignContainerForm::UpdateSize(bool shrink)
    {
        if (!designform)
            return;

        Rect r = FormObj()->WindowRect();

        if (!open)
        {
            for (auto it = controls.begin(); it != controls.end(); ++it)
                (*it)->Hide();
            SetBorderStyle(fbsNone);
            SetBounds(RectS(r.right + 2 * Scaling, r.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME), 12 * Scaling, 26 * Scaling));
        }
        else
        {
            PositionControls();
            for (auto it = controls.begin(); it != controls.end(); ++it)
                (*it)->Show();

            SetBorderStyle(fbsSizeableToolWindow);
            if (!IsVisible())
                SetBounds(RectS(0, 0, 1, 1));

            Rect wr = WindowRect();
            Rect cr = ClientRect();

            SetBounds(RectS(r.right + 2 * Scaling, r.top, wr.Width() - cr.Width() + NVControlWidth * Scaling, max((shrink ? 1 : wr.Height()), min(r.Height(), wr.Height() - cr.Height() + (NVControlHeight * Scaling) * (int)controls.size() )) ));
        }
    }

    void DesignContainerForm::GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide)
    {
        hw = 0;
        vnohide = true;
        if (designform)
        {
            int cnt = controls.size();
            hh = cnt <= 1 ? 0 : NVControlHeight * Scaling * cnt - uh;
        }
        else
        {
            hh = rows <= 1 ? 0 : NVControlHeight * Scaling * rows - uh;
        }
    }

    void DesignContainerForm::Paint(const Rect &updaterect)
    {
        Canvas *c = GetCanvas();
        Rect r = ClientRect();
        if (!open)
        {
            Color c1 = hovered ? Color(186, 198, 210) : Color(176, 176, 176);
            Color c2 = hovered ? Color(216, 228, 236) : Color(208, 208, 208);
            c->GradientRect(Rect(0, 0, r.Width() * 0.6, r.Height()), c1, clWhite, lgmHorizontal);
            c->GradientRect(Rect(r.Width() * 0.6, 0, r.Width(), r.Height()), c1, c2, lgmHorizontal);
            c->SetPen(Color(128, 0, 0, 0));
            c->FrameRect(r);
            c->SelectStockBrush(sbBlack);
            c->SetAntialias(true);
            Point pts[3] = { Point(r.Width() * 0.2, r.Height() * 0.33), Point(r.Width() * 0.8, r.Height() * 0.495), Point(r.Width() * 0.2, r.Height() *0.66)  };
            c->FillPolygon(pts, 3);
            c->SetAntialias(false);
        }
        else
        {
            //int top = -VPos();
            //Rect nvr = RectS(0, top, NVControlWidth, NVControlHeight);
            //c->SetPen(MixColors(clBtnFace, clBlack, 0.7));
            //c->SetBrush(MixColors(clBtnFace, clBlack, 0.9));
            //while(nvr.top < r.Height())
            //{
            //    if (nvr.bottom > 0)
            //    {
            //        c->GradientRect(Rect(nvr.left + 2, nvr.top + 2, nvr.right - 2, nvr.top + nvr.Height() * 0.2 - 2), MixColors(clBtnFace, clBlack, 0.7), MixColors(clBtnFace, clBlack, 0.9), lgmVertical);
            //        c->FillRect(Rect(nvr.left + 2, nvr.top + nvr.Height() * 0.2 - 2, nvr.right - 2, nvr.bottom - 2));
            //        c->FrameRect(Rect(nvr.left + 1, nvr.top + 1, nvr.right - 1, nvr.bottom - 1));
            //    }

            //    nvr.Move(0, NVControlHeight + 1);
            //}
        }
    }

    void DesignContainerForm::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        base::MouseDown(x, y, button, vkeys);

        if (button != mbLeft)
            return;

        if (!open)
            SetOpen(true);
        else
        {
            if (!controlbuttons->ButtonPressed() || !PlaceControl(*(const type_info*)controlbuttons->PressedId()) )
            {
                if (controlbuttons->NVButtonPressed())
                {
                    if (!FormObj()->GuestReferenceOn(controlbuttons->PressedNVControl()->DesignParent()))
                    {
                        AddControl(controlbuttons->PressedNVControl());
                        project->FormReferenced(controlbuttons->PressedNVControl()->DesignParent(), FormObj());
                        controlbuttons->UnpressButtons();
                    }
                    else if (controlbuttons->PressedNVControl()->DesignParent() != FormObj())
                        ShowMessageBox(L"Placing the non-visual control on this form would create a circular reference with the parent of the control, which is not allowed.", L"Message", mbOk);
                }
                else
                    FormObj()->ClearSelection(this);
            }
            controlbuttons->UnpressButtons();
        }
    }

    void DesignContainerForm::SetOpen(bool opening)
    {
        if (!designform)
            return;

        if (opening)
        {
            if (!open)
                Hide();
            hovered = false;
            open = true;
            VScroll()->SetVisible(true);
            UpdateSize(true);
            if (designform->Visible())
                Show();
        }
        else
        {
            if (open)
                Hide();
            VScroll()->SetVisible(false);
            open = false;
            ClearSelection(this);
            UpdateSize(true);
            if (designform->Visible())
                Show();
        }
    }

    bool DesignContainerForm::PlaceControl(const type_info &ctype)
    {
        // Visual controls can only be placed on the main form if there is one.
        if (ObjectTypeByTypeInfo(ctype) == otVisual)
        {
            if (designform)
                designform->PlaceControl(ctype);
            return true;
        }

        if (!ShareableByTypeInfo(ctype) && designform == nullptr)
            return false;

        PlaceholderControl *c = CreateControl(ctype);
        NonVisualControl *nvc = c->GetControl();
        controlbuttons->UnpressButtons();
        if (ctype == typeid(Menubar) && c != NULL && designform && designform->Menu() == NULL)
            designform->DesignSetMenu((Menubar*)nvc);

        UpdateSize(false);
        ScrollResize();

        if (designer && designer->ActiveForm() == FormObj())
            designer->ControlAdded(nvc, ControlName(nvc, nvc->Name()));

        return c != NULL;
    }

    DesignContainerForm::PlaceholderControl* DesignContainerForm::CreateControl(const type_info &ctype)
    {
        if ((!ShareableByTypeInfo(ctype) && designform == nullptr) || ObjectTypeByTypeInfo(ctype) != otNonVisual)
            return nullptr;

        PlaceholderControl *pc = NULL;
        NonVisualControl *c = NULL;
        std::wstring name = DisplayNameByTypeInfo(ctype, false);

        c = CreateNVControlOfType(ctype);

        //if (typeid(Imagelist) == ctype)
        //    c = new Imagelist();
        //else if (typeid(ColorDialog) == ctype)
        //    c = new ColorDialog;
        //else if (typeid(FontDialog) == ctype)
        //    c = new FontDialog;
        //else if (typeid(OpenDialog) == ctype)
        //    c = new OpenDialog;
        //else if (typeid(SaveDialog) == ctype)
        //    c = new SaveDialog;
        //else if (typeid(FolderDialog) == ctype)
        //    c = new FolderDialog;
        //else if (typeid(Menubar) == ctype)
        //    c = new Menubar;
        //else if (typeid(PopupMenu) == ctype)
        //    c = new PopupMenu;
        //else if (typeid(Timer) == ctype)
        //    c = new Timer;
        //else
        //    return NULL;

        c->SetDesigning();
        c->AddParent(FormObj());

        int num = 1;

        num = FormObj()->NameNext(name);
        c->SetName(name + IntToStr(num));

        pc = new PlaceholderControl(this, c);

        PlaceholderIterator it = ControlPosition(c);
        controls.insert(it, pc);
        PositionControls();

        pc->Show();
        SetSelected(pc);

        Modify();

        return pc;
    }

    bool DesignContainerForm::GuestReferenceOn(DesignFormBase *guestparent, std::set<DesignFormBase*> &checkfinished)
    {
        std::set<DesignFormBase*> parents;
        checkfinished.insert(FormObj());

        for (PlaceholderControl *ctrl : controls)
        {
            NonVisualControl *c = ctrl->GetControl();
            if (c->DesignParent() != FormObj())
                continue;

            for (int ix = 0; ix < c->ParentCount(); ++ix)
            {
                if (c->Parents(ix) == guestparent)
                    return true;
                if (checkfinished.count(c->Parents(ix)) == 1)
                    continue;
                parents.insert(c->Parents(ix));
            }
        }

        for (DesignFormBase *form : parents)
            if (form->GuestReferenceOn(guestparent, checkfinished))
                return true;

        return false;
    }

    void DesignContainerForm::AddControl(NonVisualControl *control)
    {
        if (!ShareableByTypeInfo(typeid(*control)) && designform == nullptr)
            return;

        AddToNotifyList(control, nrRelation1);
        AddToNotifyList(control->ParentForm(), nrRelation1);

        PlaceholderControl *pc = NULL;

        PlaceholderIterator it = ControlPosition(control);
        if (it != controls.end() && (*it)->GetControl() == control)
        {
            SetSelected(*it);
            return;
        }

        Modify();

        pc = new PlaceholderControl(this, control);

        controls.insert(it, pc);
        PositionControls();

        control->AddParent(FormObj());

        pc->Show();

        if (designer && designer->ActiveForm() == FormObj())
            designer->ControlAdded(control, ControlName(control, control->Name()));

        SetSelected(pc);
        UpdateSize(false);
        ScrollResize();
    }

    Object* DesignContainerForm::NameOwner(const std::wstring &name)
    {
        Object *obj = base::NameOwner(name);
        if (obj)
            return obj;

        for (PlaceholderControl *pc : controls)
        {
            if (pc->GetControl()->ParentForm() != FormObj())
                continue;

            //obj = pc->GetControl()->NameOwner(name);
            //if (obj)
            //    return obj;
            if (pc->GetControl()->Name() == name)
                return pc->GetControl();
        }

        return NULL;
    }

    void DesignContainerForm::NamesList(std::vector<std::wstring> &namelist)
    {
        base::NamesList(namelist);

        for (auto it = controls.begin(); it != controls.end(); it++)
        {
            namelist.push_back((*it)->GetControl()->Name());
            //(*it)->GetControl()->Names(namelist);
        }
    }

    std::wstring DesignContainerForm::ControlName(Object *c, const std::wstring &name)
    {
        std::wstring str;
        if (c->DesignParent() != FormObj())
            str = c->DesignParent()->Name() + L"->";
        return str + name;
    }

    void DesignContainerForm::CollectObjects(std::vector<std::pair<std::wstring, Object*>> &objectstrings, bool (*collector)(Object*), const std::wstring &objectname)
    {
        base::CollectObjects(objectstrings, collector, objectname);

        for (PlaceholderControl *phc : controls)
        {
            NonVisualControl *nvc = phc->GetControl();
            if (collector == NULL || collector(nvc))
            {
                std::wstring str = ControlName(nvc, nvc->Name());

                if (objectname.length())
                {
                    if (str == objectname)
                    {
                        objectstrings.push_back(std::pair<std::wstring, Object*>(str, nvc));
                        return;
                    }
                }
                else
                    objectstrings.push_back(std::pair<std::wstring, Object*>(str, nvc));

            }
        }
    }

    std::wstring DesignContainerForm::GetGuestControls()
    {
        std::wstringstream gstr;
        bool first = true;

        for (auto pc : controls)
        {
            NonVisualControl *nvc = pc->GetControl();
            if (nvc->ParentForm() == FormObj())
                continue;

            if (!first)
                gstr << L", ";
            gstr << nvc->ClassName(true) + L"* " + ControlName(nvc,  nvc->Name());
            first = false;
        }

        return gstr.str();
    }

    void DesignContainerForm::AddGuestControls(const std::wstring &guests)
    {
        std::vector<std::wstring> names;
        splitstring(guests, L',', names);
        if (names.empty())
            return;

        for (std::wstring name : names)
        {
            name = trim(name);

            // Find the control type.
            auto pos = name.find(L'*');
            if (pos == std::wstring::npos || pos == 0)
                continue;

            std::wstring ctype = trim(name.substr(0, pos));

            name = trim(name.erase(0, pos + 1));

            // Find the parent form type.
            pos = name.find(L"->");
            if (pos == std::wstring::npos || pos == 0)
                continue;

            std::wstring ptype = trim(name.substr(0, pos));

            name = trim(name.erase(0, pos + 2)); // The rest is the name of the control.

            DesignFormBase *cparent = project->FindForm(ptype); // Forms are written to the file by their type which has an extra T character in front of the name.
            if (cparent == nullptr)
                continue;

            NonVisualControl *nvc = dynamic_cast<NonVisualControl*>(cparent->NameOwner(name));
            if (nvc == nullptr || nvc->ClassName(true) != ctype)
                continue;

            AddControl(nvc);
        }
    }

    void DesignContainerForm::GetTabActivatedControls(std::vector<std::pair<std::wstring, Control*> > &controlstrings, const std::wstring &controlname)
    {
        return;
    }


    std::pair<std::wstring, Object*> DesignContainerForm::SelectedObject()
    {
        if (selected.size() == 1)
        {
            NonVisualControl *nvc = selected.front()->GetControl();
            return std::make_pair(ControlName(nvc, nvc->Name()), nvc);
        }

        if (EditedMenu() && EditedMenu()->SelCount() != 0)
        {
            TopDesignMenuItems *menu = EditedMenu();
            if (menu->SelCount() > 1)
                return std::make_pair(IntToStr(menu->SelCount()), (Object*)nullptr);

            return std::make_pair(ControlName(menu, SubObjectName(menu, menu->SelItems(0))), menu->SelItems(0)->Menu());
        }

        return std::make_pair(IntToStr(selected.size()), (Object*)NULL);
    }

    void DesignContainerForm::SelectObject(std::wstring name)
    {
        if (name.empty())
        {
            SetSelected(nullptr);
            return;
        }

        if (base::SelectSubObject(name))
            return;

        for (auto c : controls)
            if (name == ControlName(c->GetControl(), c->GetControl()->Name()))
            {
                SetSelected(c);
                break;
            }
    }

    void DesignContainerForm::SelectObject(Object *obj)
    {
        auto it = std::find_if(controls.begin(), controls.end(), [obj](PlaceholderControl *pc) { return pc->GetControl() == obj; });
        if (it == controls.end())
            return;

        SetSelected(*it);
    }

    DesignContainerForm::PlaceholderIterator DesignContainerForm::ControlPosition(NonVisualControl *control)
    {
        for (PlaceholderIterator it = controls.begin(); it != controls.end(); ++it)
        {
            int i = (*it)->GetControl()->Name().compare(control->Name());
            if (i >= 0)
                return it;
        }

        return controls.end();
    }

    int DesignContainerForm::ControlIndex(NonVisualControl *control)
    {
        int ix = 0;
        for (PlaceholderIterator it = controls.begin(); it != controls.end(); ++it, ++ix)
        {
            int i = (*it)->GetControl() == control;
            if (i >= 0)
                return ix;
        }

        return -1;
    }

    int DesignContainerForm::ControlIndex(PlaceholderControl *pc)
    {
        int ix = 0;
        for (PlaceholderIterator it = controls.begin(); it != controls.end(); ++it, ++ix)
        {
            int i = (*it) == pc;
            if (i >= 0)
                return ix;
        }

        return -1;
    }

    void DesignContainerForm::PositionControls()
    {
        int top = -VPos() - NVControlHeight * Scaling;
        int left = 0;
        Rect rc = ClientRect();
        rows = 0;
        bool newline = true;
        std::for_each(controls.begin(), controls.end(), [&](PlaceholderControl *pc)
        {
            if (designform || newline)
            {
                top += NVControlHeight * Scaling;
                left = 0;
                newline = false;
                rows++;
            }

            pc->SetBounds(RectS(left, top, NVControlWidth * Scaling, NVControlHeight * Scaling));

            if (!designform)
            {
                left += NVControlWidth * Scaling;
                if (left && left + NVControlWidth * Scaling > rc.Width())
                    newline = true;
            }
        });
    }

    void DesignContainerForm::Resizing()
    {
        base::Resizing();
        if (designform)
            return;
        PositionControls();
    }

    void DesignContainerForm::MouseEnter()
    {
        base::MouseEnter();

        hovered = true;
        Invalidate();
    }

    void DesignContainerForm::MouseLeave()
    {
        base::MouseLeave();

        hovered = false;
        Invalidate();
    }

    void DesignContainerForm::PressButton(PlaceholderControl *control)
    {
        int ix = 0;
        for (PlaceholderIterator it = controls.begin(); it != controls.end(); ++it, ++ix)
            if (*it == control)
            {
                PressButton(ix);
                break;
            }
    }

    void DesignContainerForm::UnpressButton(PlaceholderControl *control)
    {
        if (pressed < 0 || controls[pressed] != control)
            return;
        UnpressButton();
    }

    void DesignContainerForm::UnpressButton()
    {
        if (pressed < 0)
            return;

        PlaceholderControl *pc = controls[pressed];
        controlbuttons->UnregisterButtonPress(pc->GetControl());
        pc->button->SetDown(false);
        pressed = -1;
    }

    void DesignContainerForm::PressButton(int index)
    {
        PlaceholderControl *pc;
        if (index != pressed && pressed >= 0)
            UnpressButton();

        if (index >= 0 && index < (int)controls.size())
        {
            pressed = index;
            pc = controls[pressed];
            controlbuttons->RegisterButtonPress(pc->GetControl(), CreateEvent(this, &DesignContainerForm::notifyunpress));
        }
        else
            pressed = -1;
    }

    void DesignContainerForm::notifyunpress(void *sender, EventParameters param)
    {
        UnpressButton();
    }

    int DesignContainerForm::Count()
    {
        return controls.size();
    }

    NonVisualControl* DesignContainerForm::Controls(int ix)
    {
        return controls[ix]->GetControl();
    }

    int DesignContainerForm::SelCount()
    {
        return selected.size();
    }

    NonVisualControl* DesignContainerForm::SelControls(int ix)
    {
        return selected[ix]->GetControl();
    }

    void DesignContainerForm::ClearSelection(Object *sender)
    {
        for (PlaceholderIterator it = selected.begin(); it != selected.end(); it++)
        {
            (*it)->Deselect();
            InvalidateControl(*it);
        }
        selected.clear();

        if (designform)
            designform->UpdateSelection();
    }

    void DesignContainerForm::DeleteSelected()
    {
        if (designer && designer->ActiveForm() == FormObj())
            designer->BeginControlChange();

        for (int ix = controls.size() - 1; ix >= 0; --ix)
        {
            if (controls[ix]->Selected())
            {
                Modify();

                if (controls[ix]->button->Down())
                {
                    pressed = -1;
                    controlbuttons->UnregisterButtonPress(controls[ix]->GetControl());
                }

                NonVisualControl *nvc = controls[ix]->GetControl();
                if (designer && designer->ActiveForm() == FormObj())
                    designer->ControlDeleted(nvc, ControlName(nvc, nvc->Name()));

                bool lastcontrol = controls.size() == 1 && designform;
                if (lastcontrol && designer && designer->ActiveForm() == FormObj())
                    designer->EndControlChange();


                if (nvc->ParentForm() == FormObj())
                {
                    nvc->Destroy();
                    if (lastcontrol)
                        return; // At this point this container is already destroyed by its form.
                }
                else
                {
                    nvc->RemoveParent(FormObj());
                    FormObj()->RemoveNotify(nvc);
                    DeletePlaceholder(nvc);
                    if (lastcontrol)
                        return; // At this point this container is already destroyed by its form.
                }
            }
        }

        selected.clear();

        if (designer && designer->ActiveForm() == FormObj())
            designer->EndControlChange();
    }

    bool DesignContainerForm::ActiveDesigner()
    {
    //    return (designer && (designer->ActiveForm() == this || (containerform && designer->ActiveForm() == containerform))) || Active() || (containerform && containerform->Active());
        return designer && designer->ActiveForm() == FormObj(); //(!designform && (Active() || designer && designer->ActiveForm() == this)) || (designform && designform->ActiveDesigner());
    }

    std::wstring DesignContainerForm::BaseClass()
    {
        return L"Container";
    }

    void DesignContainerForm::ActiveFormChanging(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow)
    {
        base::ActiveFormChanging(activated, mouseactivate, otherform, otherwindow);

        if (!designer)
            return;

        if (activated)
        {
            if (designform && otherform != designform)
                PostMessage(designform->Handle(), WM_ACTIVATE, (WPARAM)Handle(), WA_ACTIVE);//designform->Focus();
            else if (FormObj() == this)
                SetProperties();
        }
        else if (otherform != designform)
            designform->ActiveFormChanging(activated, mouseactivate, otherform, otherwindow);
    }

    void DesignContainerForm::DeactivateSelection()
    {
        std::for_each(selected.begin(), selected.end(), [this](PlaceholderControl* pc)
        {
            pc->SetColor(MixColors(clBtnFace, clGray, 0.7));
            InvalidateControl(pc);
        });
    }

    void DesignContainerForm::ActivateSelection()
    {
        std::for_each(selected.begin(), selected.end(), [this](PlaceholderControl* pc)
        {
            pc->SetColor(MixColors(clHighlight, clBtnFace, 0.7));
            InvalidateControl(pc);
        });
    }

    void DesignContainerForm::RemoveNotify(Object *object)
    {
        bool sameparent = false;
        for (int ix = controls.size() - 1; ix >= 0; --ix)
        {
            NonVisualControl *obj = controls[ix]->GetControl();
            if (obj == object)
                continue;

            if (!sameparent)
                for (auto it = controls.begin(); !sameparent && it != controls.end(); ++it)
                    sameparent = (*it)->GetControl() != object && (*it)->GetControl()->ParentForm() == object->ParentForm();
            obj->DeleteNotify(object);
        }
        if (!sameparent)
            RemoveFromNotifyList(object->ParentForm(), nrNoReason);
    }

    void DesignContainerForm::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        NonVisualControl *nvc = dynamic_cast<NonVisualControl*>(object);
        if (!nvc)
            return;

        DeletePlaceholder(nvc);
    }

    void DesignContainerForm::DeletePlaceholder(NonVisualControl *nvc)
    {
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            PlaceholderControl *pc = *it;
            if (pc->GetControl() != nvc)
                continue;

            auto sit = std::find(selected.begin(), selected.end(), pc);
            if (sit != selected.end())
                selected.erase(sit);

            controls.erase(it);
            pc->Destroy();

            Modify();
            break;
        }

        if (designform)
            designform->UpdateSelection(); // This call will delete the container itself if it has no more controls.
    }

    void DesignContainerForm::NameChangeNotify(Object *object, const std::wstring &oldname)
    {
        base::NameChangeNotify(object, oldname);

        if (designstate.contains(dsDeserializing))
            return;

        if (dynamic_cast<Form*>(object))
        {
            std::for_each(controls.begin(), controls.end(), [object](PlaceholderControl *pc)
            {
                if (pc->GetControl()->ParentForm() == object)
                    pc->Invalidate();
            });
            return;
        }

        NonVisualControl *nvc = dynamic_cast<NonVisualControl*>(object);
        if (!nvc)
            return;

        PlaceholderControl *pc = NULL;
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            if ((*it)->GetControl() == object)
            {
                pc = *it;
                controls.erase(it);
                break;
            }
        }
        if (!pc)
            return;

        PlaceholderIterator it = ControlPosition(nvc);
        controls.insert(it, pc);
        PositionControls();
        pc->Invalidate();

        if (designer && designer->ActiveForm() == FormObj())
            designer->ControlNameChanged(object, ControlName(object, oldname), ControlName(object, object->Name()));

        Modify();
    }

    void DesignContainerForm::SetSelected(PlaceholderControl *pc)
    {
        FormObj()->ClearSelection(this);

        if (pc)
        {
            selected.push_back(pc);
            pc->Select();

            if (ActiveDesigner() && application->Active())
                pc->SetColor(MixColors(clHighlight, clBtnFace, 0.7));
            else
                pc->SetColor(MixColors(clBtnFace, clGray, 0.7));
            InvalidateControl(pc);
        }

        SetOpen(true);

        if (designform)
            designform->UpdateSelection();
    }

    void DesignContainerForm::SetSelected(int index)
    {
        SetSelected(controls[index]);
    }

    void DesignContainerForm::Select(PlaceholderControl *pc)
    {
        PlaceholderIterator it = std::find(selected.begin(), selected.end(), pc);
        if (it != selected.end())
            return;

        selected.push_back(pc);

        pc->Select();
        if (ActiveDesigner() && application->Active())
        {
            pc->SetColor(MixColors(clHighlight, clBtnFace, 0.7));
            InvalidateControl(pc);
        }

        if (designform)
            designform->UpdateSelection();
    }

    void DesignContainerForm::Deselect(PlaceholderControl *pc)
    {
        PlaceholderIterator it = std::find(selected.begin(), selected.end(), pc);
        if (it == selected.end())
            return;

        selected.erase(it);
        pc->SetColor(clBtnFace);
        InvalidateControl(pc);
        pc->Deselect();

        if (designform)
            designform->UpdateSelection();
    }

    void DesignContainerForm::InvalidateControl(PlaceholderControl *pc)
    {
        Rect r = pc->WindowRect();
        r = Rect(0, r.top - 2 * Scaling, r.right + 2 * Scaling, r.bottom + 2 * Scaling);
        if (r.bottom > 0 && r.top < ClientRect().Height())
            InvalidateRect(r);
    }

    DesignContainerForm::PlaceholderControl* DesignContainerForm::LastSelected()
    {
        return selected.empty() ? nullptr : selected.back();
    }

    void DesignContainerForm::SelectLastSelected()
    {
        SetSelected(selected.back());
    }

    bool DesignContainerForm::Open()
    {
        return open;
    }

    bool DesignContainerForm::SelectNext(bool backwards, bool allowoverflow)
    {
        if (controls.empty())
            return false;

        if (selected.empty())
        {
            SetSelected(nullptr);
            return true;
        }

        PlaceholderIterator it = std::find(controls.begin(), controls.end(), LastSelected());
        if (!allowoverflow && ((!backwards && LastSelected() == controls.back()) || (backwards && LastSelected() == controls.front())))
            return false;

        if (!backwards && LastSelected() == controls.back())
        {
            SetSelected(controls.front());
            return true;
        }
        else if (backwards && LastSelected() == controls.front())
        {
            SetSelected(controls.back());
            return true;
        }

        SetSelected(backwards ? *(it - 1) : *(it + 1));
        return true;
    }

    void DesignContainerForm::CollectControls(std::vector<NonVisualControl*> &nvlist, bool noguest, bool onlyselected)
    {
        for (unsigned int ix = 0; ix < (onlyselected ? selected.size() : controls.size()); ++ix)
        {
            NonVisualControl *nvc = onlyselected ? selected[ix]->GetControl() : controls[ix]->GetControl();
            if (noguest && nvc->ParentForm() != FormObj())
                continue;
            nvlist.push_back(nvc);
        }
    }

    //void DesignContainerForm::CollectEvents(std::vector<std::pair<void*, std::wstring>> &events)
    //{
    //    DesignSerializer *serializer = Serializer();
    //    serializer->CollectEvents(events, this);
    //
    //    for (auto it = controls.begin(); it != controls.end(); ++it)
    //    {
    //        PlaceholderControl *pc = *it;
    //        NonVisualControl *nvc = pc->GetControl();
    //        if (nvc->ParentForm() != FormObj())
    //            continue;
    //        nvc->Serializer()->CollectEvents(events, nvc);
    //    }
    //}

    void DesignContainerForm::HeaderMemberList(Indentation &indent, std::wiostream &stream, AccessLevels access)
    {
        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            PlaceholderControl *pc = *it;
            NonVisualControl *nvc = pc->GetControl();
            if (nvc->ParentForm() != FormObj()/* || nvc->AccessLevel() != access*/)
                continue;
            //stream << indent << nvc->ClassName() << L" *" << nvc->Name() << L";" << std::endl;
            nvc->Serializer()->DeclareSerialize(indent, stream, nvc, access);
        }
    }

    void DesignContainerForm::HeaderExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, bool printpragma)
    {
        if (designform)
            return;
        container->SetName(Name());

        base::HeaderExport(indent, stream, events, printpragma);
    }

    void DesignContainerForm::CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events)
    {
        std::wstringstream sdelayed;
        CppExport(indent, stream, events, sdelayed);
    }

    bool DesignContainerForm::PrintShowAfterMemberList()
    {
        return false;
    }

    void DesignContainerForm::CppMemberList(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents)
    {
        DesignSerializer *serializer = container->Serializer();

        for (auto it = controls.begin(); it != controls.end(); ++it)
        {
            PlaceholderControl *pc = *it;
            NonVisualControl *nvc = pc->GetControl();
            if (nvc->ParentForm() != FormObj())
                continue;

            serializer = nvc->Serializer();

            serializer->ConstructExport(indent, stream, sdelayed, sevents, NULL, nvc, /*std::wstring(),*/ NULL);
        }
    }

    void DesignContainerForm::CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, std::wstringstream &sdelayed)
    {
        container->SetName(Name());
        print_object_declaration(indent, stream, false);

        print_member_initialization(indent, stream);

        print_c_and_d_structor_definition(indent, stream, wddConstructor | wddDestructor | wddDestroyer);

        for (auto e : events)
        {
            if (e->func.empty())
                continue;

            print_event_handler_lines(indent, stream, e->func, e->type);
        }

        //DesignSerializer *serializer;
        //if (!designform)
        //{
        //    serializer = container->Serializer();
        //    container->SetName(Name());

        //    print_object_declaration(indent, stream, false);

        //    stream << indent << L"void " << ClassName() << L"::" << INITIALIZATION_FUNCTION << L"()" << std::endl;
        //    stream << indent << L"{" << std::endl;
        //    ++indent;
        //    stream << indent << L"/* Generated member initialization. Do not modify. */" << std::endl;
        //    stream << MEMBER_INITIALIZATIONS_B << std::endl;

        //    serializer->Serialize(indent, L"", stream, sdelayed, container);
        //    stream << std::endl;
        //}

        //for (auto it = controls.begin(); it != controls.end(); ++it)
        //{
        //    PlaceholderControl *pc = *it;
        //    NonVisualControl *nvc = pc->GetControl();
        //    if (nvc->ParentForm() != FormObj())
        //        continue;

        //    serializer = nvc->Serializer();

        //    serializer->ConstructExport(indent, stream, sdelayed, NULL, nvc, /*std::wstring(),*/ NULL);

        //}

        //if (!designform)
        //{
        //    stream << sdelayed.str();
        //    stream << MEMBER_INITIALIZATIONS_E << std::endl << std::endl;
        //    --indent;
        //    stream << indent << L"}" << std::endl;
        //    stream << std::endl;

        //    // Print the constructor calling the initialization function.
        //    stream << indent << ClassName() << L"::" << ClassName() << L"()" << std::endl;
        //    stream << indent << L"{" << std::endl;
        //    ++indent;
        //    stream << indent << INITIALIZATION_FUNCTION << L"();" << std::endl;
        //    --indent;
        //    stream << indent << L"}" << std::endl << std::endl;

        //    print_destructor_definition(indent, stream, L"Container");

        //    for (auto it = events.begin(); it != events.end(); ++it)
        //    {
        //        if (((EventListItem*)it->first)->func.empty())
        //            continue;

        //        print_event_handler_line(indent, stream, ((EventListItem*)it->first)->func, it->second);
        //        stream << indent << L"{" << std::endl;
        //        ++indent;
        //        stream << indent << L";" << std::endl;
        //        --indent;
        //        stream << indent << L"}" << std::endl << std::endl;
        //    }
        //}
    }

    void DesignContainerForm::WriteToStream(Indentation &indent, std::wiostream &stream)
    {
        if (designform)
            throw L"Do not call WriteToStream for containers that belong to a form.";

        std::vector<NonVisualControl*> nvec;
        CollectControls(nvec, true, false);

        stream << indent << L"type Container" << std::endl;
        stream << indent << L"{" << std::endl;
        ++indent;

        container->SetName(Name());
        DesignSerializer *serializer = container->Serializer();
        serializer->Serialize(indent, stream, container);

        Serialize(indent, stream, nvec);

        --indent;
        stream << indent << L"}" << std::endl;
    }

    void DesignContainerForm::ReadFromStream(TokenStream &token)
    {
        if (designform)
            throw L"Do not call ReadFromStream for containers that belong to a form.";

        if (token.peek() != L"{")
            throw TokenE(L"Excpected \"{\"", token);

        std::vector<ControlDeserializerItem*> items;
        DeserializeFromStream(NULL, token, items, BaseClass());
        ProcessDeserializedItems(items);
    }

    void DesignContainerForm::Serialize(Indentation &indent, std::wiostream &stream, std::vector<NonVisualControl*> &nvlist)
    {
        for (auto it = nvlist.begin(); it != nvlist.end(); ++it)
        {
            NonVisualControl *nvc = *it;
            DesignSerializer *serializer = nvc->Serializer();
            stream << std::endl << indent << L"type " << ShortNamespaceName(nvc->ClassName(true)) << std::endl;
            stream << indent << L"{" << std::endl;
            ++indent;
            serializer->Serialize(indent, stream, nvc);

            std::list<NonVisualSubItem> subitems;
            nvc->CollectSubItems(subitems);
            SerializeNonVisualSubItems(indent, stream, subitems);

            --indent;
            stream << indent << L"}" << std::endl;
        }
    }

    void DesignContainerForm::CopySerialized()
    {
        if (designform)
        {
            designform->CopySerialized();
            return;
        }

        std::wstringstream stream;
        std::vector<NonVisualControl*> nvec;
        CollectControls(nvec, false, true);
        Indentation indent(false, 4);
        Serialize(indent, stream, nvec);

        CopyToClipboard(stream.str());
    }

    void DesignContainerForm::PasteSerialized()
    {
        if (designform)
        {
            WORD keycode = 0;
            WCHAR key = ckV;
            designform->KeyPush(keycode, key, vksCtrl);
            return;
        }

        std::wstring pasted;
        if (!PasteFromClipboard(pasted))
            return;

        std::wstringstream stream(pasted);
        std::vector<ControlDeserializerItem*> items;
        TokenStream token(stream);

        DeserializeFromStream(NULL, token, items);

        if (designer && designer->ActiveForm() == FormObj())
            designer->BeginControlChange();
        ProcessDeserializedItems(items);
        if (designer && designer->ActiveForm() == FormObj())
            designer->EndControlChange();
    }

    void DesignContainerForm::ProcessDeserializedItems(std::vector<ControlDeserializerItem*> &items)
    {
        std::vector<std::pair<Object*, ControlDeserializerProperty*> > delayedprop; // Properties that are restored at the end of deserialization.
        std::vector<std::pair<Object*, ControlDeserializerItem*>> subparents; // List of parents for sub controls.

        std::vector<Object*> nitems;

        for (unsigned int ix = 0; ix < items.size(); ++ix)
        {
            ControlDeserializerItem *cd = items[ix];
            if (ObjectTypeByTypeInfo(cd->type) == otVisual)
                continue;

            Object *object = NULL;
            if (ObjectTypeByTypeInfo(cd->type) == otNonVisual)
            {
                object = cd->type != typeid(DesignContainerForm) ? CreateControl(cd->type)->GetControl() : (Object*)container;
                if (cd->type != typeid(DesignContainerForm))
                    nitems.push_back(object);
                subparents.clear();
            }
            else if (ObjectTypeByTypeInfo(cd->type) == otSubControl)
            {
                while (!subparents.empty() && cd->parent != subparents.back().second)
                    subparents.pop_back();
                object = cd->serializer->CreateObject(subparents.empty() ? NULL : subparents.back().first);
                if (!object)
                    throw L"Creation of sub control failed.";
            }
            else
                throw L"Invalid control type. Don't know how to create";
            subparents.push_back(std::make_pair(object, cd));

            for (auto pit = cd->properties.begin(); pit != cd->properties.end(); ++pit)
            {
                ControlDeserializerPropertyNameValue *val = dynamic_cast<ControlDeserializerPropertyNameValue*>(*pit);
                if (ObjectTypeByTypeInfo(cd->type) == otSubControl && val && val->Name() == L"Name" && NameTaken(val->Value()))
                {
                    object->SetName(DisplayNameByTypeInfo(cd->type, false) + IntToStr(NameNext(DisplayNameByTypeInfo(cd->type, false))));
                    continue;
                }

                if ((*pit)->prop->Delayed())
                {
                    delayedprop.push_back(std::pair<Object*, ControlDeserializerProperty*>(object, *pit));
                    continue;
                }
                (*pit)->SetValue(object);
            }
        }

        for (auto it = delayedprop.begin(); it != delayedprop.end(); ++it)
            (*it).second->SetValue((*it).first);
        for (Object *nitem : nitems)
        {
            if (designer && designer->ControlsChanging() && designer->ActiveForm() == FormObj())
                designer->ControlAdded(nitem, ControlName(nitem, nitem->Name()));
        }
        FreeDeserializerList(items);

        FormObj()->FinishProcessing(nitems);
    }


    //---------------------------------------------


    DesignContainerForm::PlaceholderControl::PlaceholderControl(DesignContainerForm *owner, NonVisualControl *control) :
            owner(owner), control(control), selected(false)
    {
        GetFont().SetSize(7);
        GetFont().SetBold(true);
        SetParentBackground(false);
        SetColor(clBtnFace);

        SetInnerBorderStyle(pbsSunkenRaised);

        SetBounds(RectS(0, 0, NVControlWidth * Scaling, NVControlHeight * Scaling));
        SetShowText(false);
        Hide();
        SetParent(owner);
        button = new ToolButton();
        button->SetBounds(RectS(2 * Scaling, 2 * Scaling, (NVControlHeight - 4) * Scaling, (NVControlHeight - 4) * Scaling));
        button->Image()->SetImages(owner->controlbuttons->Images());
        button->Image()->SetImageIndex(ImageIndexByTypeInfo(typeid(*control->PropertyOwner())));
        button->SetImagePosition(bipCenter);
        button->SetType(fbtCheckbutton);
        button->OnClick = CreateEvent(this, &DesignContainerForm::PlaceholderControl::buttonclick);
        if (!ShareableByTypeInfo(typeid(*control->PropertyOwner())))
            button->SetEnabled(false);

        button->SetParent(this);
    }

    DesignContainerForm::PlaceholderControl::~PlaceholderControl()
    {
    }

    NonVisualControl* DesignContainerForm::PlaceholderControl::GetControl()
    {
        return control;
    }

    void DesignContainerForm::PlaceholderControl::Paint(const Rect &updaterect)
    {
        Canvas *c = GetCanvas();
        Size s = c->MeasureText(L"qMWljp");

        Rect cr = ClientRect();
        Rect imgr = RectS(3 * Scaling, 3 * Scaling, (NVControlHeight - 6) * Scaling, (NVControlHeight - 6) * Scaling);
        Rect r = Rect(imgr.right + 4 * Scaling, (cr.Height() - (s.cy * 2 + 1)) / 2, cr.right - 3 * Scaling, (cr.Height() - (s.cy * 2 + 1)) / 2 + s.cy * 2 + 1);

        if (control->ParentForm() != owner->FormObj())
        {
            c->TextDraw(r, r.left, r.top + Scaling, control->ParentForm()->Name() + L"->");
            r.top += s.cy;
        }
        c->TextDraw(r, r.left, r.top + Scaling, control->Name());
    }

    void DesignContainerForm::PlaceholderControl::buttonclick(void *sender, EventParameters param)
    {
        if (((ToolButton*)sender)->Down())
            owner->PressButton(this);
        else
            owner->UnpressButton(this);
    }

    void DesignContainerForm::PlaceholderControl::Select()
    {
        selected = true;
    }

    void DesignContainerForm::PlaceholderControl::Deselect()
    {
        SetColor(clBtnFace);
        selected = false;
    }

    bool DesignContainerForm::PlaceholderControl::Selected()
    {
        return selected;
    }

    void DesignContainerForm::PlaceholderControl::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        base::MouseDown(x, y, button, vkeys);

        if (button == mbLeft)
        {
            if (owner->controlbuttons->ButtonPressed() || owner->controlbuttons->NVButtonPressed())
            {
                owner->MouseDown(x, y, button, vkeys);
                return;
            }
            if (vkeys.contains(vksShift) || vkeys.contains(vksCtrl))
            {
                if (selected)
                    owner->Deselect(this);
                else
                    owner->Select(this);
            }
            else 
            {
                owner->SetSelected(this);
                if (vkeys.contains(vksDouble))
                {
                    DesignProperty *prop = control->Serializer()->DefaultProperty();
                    if (prop != nullptr)
                        designer->EditProperty(prop->Name(), true);
                }
            }
        }
    }


}
/* End of NLIBNS */


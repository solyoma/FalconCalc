#include "stdafx_zoli.h"
#include "menu.h"
#include "generalcontrol.h"
#include "application.h"
#include "themes.h"
#include "imagelist.h"

#ifdef DESIGNING
#include "designproperties.h"
#include "serializer.h"
#include "property_menu.h"
#include "designer.h"
#include "designerform.h"
#include "property_images.h"
#include "property_imagelist.h"
#include "designermenu.h"
#endif


#define N_OWNERDRAWN_MENUS


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING
    std::wstring MenuItemSerializeArgs(Object *control)
    {
        MenuItem *item = dynamic_cast<MenuItem*>(control);
        if (!item)
            return std::wstring();
        if (item->Separator())
            return L"L\"-\"";
        else
            return L"L\"" + EscapeCString(item->Text()) + L"\"";
    }

    void MenuBase::CollectSubItems(std::list<NonVisualSubItem> &subitems)
    {
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            subitems.push_back(NonVisualSubItem(L"Items", *it));
            (*it)->CollectSubItems(subitems.back().subitems);
        }
    }


    void MenuBase::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->Add(L"Add", new MenuItemVectorDesignProperty<MenuBase>(L"Items", L"Contents", &MenuBase::Count, &MenuBase::Items, &MenuBase::DesignAdd))->DontList();
        serializer->AllowSubType(typeid(MenuItem));
    }

    void MenuBase::DesignAdd()
    {
        Add(L"");
    }

    //void MenuBase::SetDesignOwner(Object *obj)
    //{
    //    if (designowner == nullptr)
    //        designowner = obj;
    //}

    Object* MenuBase::SubOwner()
    {
        //return designowner;
        return topmenu == nullptr || topmenu == this ? nullptr : topmenu;
    }

#endif

    MenuBase::MenuBase() : handle(NULL), topmenu(NULL), autosep(true)
    {
//#ifdef DESIGNING
//        designowner = nullptr;
//#endif
        topmenu = this;
    }

    MenuBase::MenuBase(MenuBase *atopmenu) : handle(NULL), topmenu(atopmenu), autosep(true)
    {
//#ifdef DESIGNING
//        designowner = nullptr;
//#endif

        if (topmenu == NULL)
            topmenu = this;
    }

    void MenuBase::Destroy()
    {
        for (auto it : items)
        {
            it->parent = NULL;
            it->Destroy();
        }
        if (handle)
            DestroyMenu(handle);

        base::Destroy();
    }

    void MenuBase::CheckIndexInRange(int index) const
    {
        if (index < 0 || (int)items.size() <= index)
            throw L"Menu item index out of range.";
    }

    void MenuBase::Delete(int ix)
    {
        CheckIndexInRange(ix);
        DeleteItem(Items(ix));
    }

    void MenuBase::RemoveItem(MenuItem *item)
    {
        auto it = std::find(items.begin(), items.end(), item);
        if (it == items.end())
            return;

        bool vis = item->options.contains(mioVisible);
        MenuBreakTypes btype = item->breaktype;
        UpdateItem(IndexOf(item), false, mbtNoBreak);
        item->options -= mioAutoHidden;
        if (vis)
            item->options << mioVisible;
        item->breaktype = btype;

        if (it != items.begin())
        {
            MenuItem *sub = *(it - 1);
            sub->next = item->next;
        }
        items.erase(it);

        item->next = NULL;
        item->parent = NULL;
        item->RecursiveTopmenu(item);
        item->UpdateItems();

    }

    void MenuBase::UpdateItems()
    {
        std::vector<MenuItem*> subcolumn; // Vector for collecting single columns in the menu.
        auto sit = items.begin();
        MenuItem *sub;

        while (sit != items.end())
        {
            subcolumn.resize(0);

            // Collect items that are visible and are in a single column of the menu.
            while (sit != items.end())
            {
                sub = *sit;
                if (sub->breaktype != mbtNoBreak && !subcolumn.empty())
                    break;

                if (sub->options.contains(mioVisible))
                    subcolumn.push_back(sub);
                ++sit;
            }

            auto rit = subcolumn.rbegin();
            while (rit != subcolumn.rend() && (*rit)->Separator()) // Find the first non separator item at the end of the menu column.
                ++rit;

            bool prevsep = true;
            for (auto it = subcolumn.begin(); it != subcolumn.end(); ++it)
            {
                sub = *it;

                if (rit.base() == it)
                    prevsep = true;

                if (!sub->Separator() || !sub->options.contains(mioVisible))
                {
                    if (sub->options.contains(mioVisible))
                    {
                        prevsep = false;
                        if (sub->options.contains(mioAutoHidden))
                        {
                            sub->options -= mioAutoHidden;
                            //if (sub->parent->HandleCreated())
                            sub->parent->CreateAppend(sub, sub->next);
                        }
                        else
                            sub->Update(MIIM_STATE | MIIM_FTYPE);

                        sub->UpdateItems();
                    }
                    continue;
                }

                if (!topmenu->autosep)
                {
                    if (sub->options.contains(mioAutoHidden))
                    {
                        sub->options -= mioAutoHidden;
                        //if (sub->parent->HandleCreated())
                        sub->parent->CreateAppend(sub, sub->next);
                    }
                    else
                        sub->Update(MIIM_STATE | MIIM_FTYPE);
                    continue;
                }

                if (!prevsep && sub->options.contains(mioAutoHidden) && it != subcolumn.begin() && sub != subcolumn.back())
                {
                    sub->options -= mioAutoHidden;
                    //if (sub->parent->HandleCreated())
                    sub->parent->CreateAppend(sub, sub->next);
                    prevsep = true;
                    continue;
                }

                if (!sub->options.contains(mioAutoHidden) && topmenu->autosep && prevsep)
                {
                    sub->options << mioAutoHidden;
                    sub->Hide();
                }
                else
                    sub->Update(MIIM_STATE | MIIM_FTYPE);

                prevsep = true;
            }
        }
    }

    void MenuBase::NullifyHandles()
    {
        handle = NULL;
        for (auto it = items.begin(); it != items.end(); ++it)
            (*it)->NullifyHandles();
    }

    void MenuBase::DeleteItem(MenuItem *item)
    {
        UpdateItem(IndexOf(item), false, mbtNoBreak);
        item->Destroy();
    }

    HMENU MenuBase::CreateHandle()
    {
        HMENU hmnu = CreatePopupMenu();
        MENUINFO info = {0};
        info.cbSize = sizeof(MENUINFO);
        info.fMask = MIM_STYLE;
        GetMenuInfo(hmnu, &info);
        info.fMask &= ~MNS_MODELESS;
        SetMenuInfo(hmnu, &info);
        return hmnu;
    }

    bool MenuBase::HandleCreated()
    {
        return handle != NULL;
    }

    int MenuBase::Count() const
    {
        return items.size();
    }

    MenuItem* MenuBase::Items(int ix)
    {
        CheckIndexInRange(ix);
        return items[ix];
    }

    int MenuBase::IndexOf(MenuItem *subitem)
    {
        auto it = std::find(items.begin(), items.end(), subitem);
        if (it == items.end())
            return -1;
        return it - items.begin();
    }

    bool MenuBase::AutoHideSeparators()
    {
        return autosep;
    }

    void MenuBase::SetAutoHideSeparators(bool newautosep)
    {
        if (autosep == newautosep)
            return;
        autosep = newautosep;
        if (this == topmenu)
            UpdateItems();
    }

    bool MenuBase::SubmenuVisible()
    {
        for (auto mi : items)
            if (mi->Visible())
                return true;
        return false;
    }

    void MenuBase::CheckSubmenu()
    {
        if (!handle || SubmenuVisible())
            return;
        DestroyMenu(handle);
        handle = NULL;
        UpdateMenuItemInfo(dynamic_cast<MenuItem*>(this), MIIM_SUBMENU);
    }

    void MenuBase::RecursiveTopmenu(MenuBase *newtopmenu)
    {
        topmenu = newtopmenu;
        for (auto it = items.begin(); it != items.end(); ++it)
            (*it)->RecursiveTopmenu(newtopmenu);
    }

    void MenuBase::CreateHandles()
    {
        if (handle != NULL)
            return;

        if (topmenu != this)
        {
            topmenu->CreateHandles();
            return;
        }

        handle = CreateHandle();
        for (auto it = items.begin(); it != items.end(); ++it)
            CreateAppend(*it, NULL);
    }

    void MenuBase::CreateAppend(MenuItem *item, MenuItem *nextitem)
    {
        if (!item->Visible() || !topmenu->HandleCreated())
            return;

        MenuItem *self = dynamic_cast<MenuItem*>(this);
        if (!handle && self)
        {
            self->parent->CreateAppend(self, self->next);
            return;
        }

        while (nextitem && !nextitem->Visible())
            nextitem = nextitem->next;

        MENUITEMINFO mi;
        FillMenuItemInfo(item, mi, MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE);
#ifdef N_OWNERDRAWN_MENUS 
        mi.fMask |= MIIM_DATA;
        mi.dwItemData = (ULONG_PTR)item;
#endif
        if (item->SubmenuVisible())
        {
            mi.fMask |= MIIM_SUBMENU;
            item->handle = mi.hSubMenu = CreatePopupMenu();
        }

        if (!InsertMenuItem(handle, nextitem ? nextitem->id : items.size() - 1, nextitem ? FALSE : TRUE, &mi))
        {
            //int err = GetLastError();
            throw L"Couldn't insert menu item!";
        }

        for (auto it = item->items.begin(); it != item->items.end(); ++it)
            item->CreateAppend(*it, NULL);
    }

    void MenuBase::FillMenuItemInfo(MenuItem *item, MENUITEMINFO &mi, UINT mask)
    {
        bool separator = item->text == L"-";
        if (separator)
        {
            mask &= ~MIIM_STRING;
            mask |= MIIM_FTYPE;
        }

        memset(&mi,0,sizeof(MENUITEMINFO));
        mi.cbSize = sizeof(MENUITEMINFO);
        mi.fMask = mask; //MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE;
        mi.fType = (separator ? MFT_SEPARATOR : MFT_STRING) | (item->Grouped() ? MFT_RADIOCHECK : 0) | item->ComputedBreakType();
#ifdef N_OWNERDRAWN_MENUS
        if (item->parent && (dynamic_cast<Menubar*>(item->parent) == NULL || (item->TopMenuImagelist() != NULL && ((item->imgix >= 0 && item->imgix < item->TopMenuImagelist()->Count()) || (item->chkix >= 0 && item->chkix < item->TopMenuImagelist()->Count())))))
            mi.fType |= MFT_OWNERDRAW;
#endif
        mi.fState = separator ? 0 : (item->Checked() ? MFS_CHECKED : MFS_UNCHECKED) | (item->Enabled() ? MFS_ENABLED : MFS_DISABLED);
        mi.dwTypeData = item->InnerText();
        mi.wID = item->id;

        if ((mask & MIIM_SUBMENU) == MIIM_SUBMENU /*&& item->items.size()*/)
        {
            mi.fMask |= MIIM_SUBMENU;
            mi.hSubMenu = item->handle;
        }
    }

    void MenuBase::UpdateMenuItemInfo(MenuItem *item, UINT mask)
    {
        if (!item || !item->parent || !item->parent->HandleCreated() || !item->Visible())
            return;

        MENUITEMINFO mi;
        FillMenuItemInfo(item, mi, mask);
        SetMenuItemInfo(item->Parent()->Handle(), item->Id(), FALSE, &mi);

        if (item->TopMenu()->ParentForm())
            DrawMenuBar(item->TopMenu()->ParentForm()->Handle());
    }

    MenuBase* MenuBase::TopMenu()
    {
        return topmenu;
    }

    MenuItem* MenuBase::Insert(int ix, const std::wstring &text)
    {
        ix = max(0, min(ix, (int)items.size()));
        return InsertItem(ix < (int)items.size() ? items[ix] : NULL, text);
    }

    void MenuBase::Insert(int ix, MenuItem *what)
    {
        ix = max(0, min(ix, (int)items.size()));
        InsertItem(ix < (int)items.size() ? items[ix] : NULL, what);
    }

    void MenuBase::InsertItem(MenuItem *nextitem, MenuItem *what)
    {
        if (!what)
            throw L"Invalid argument, NULL for menu item.";
        if (nextitem && nextitem->Parent() != this)
            throw L"The specified item is not part of this menu.";
        if (nextitem && what->next == nextitem)
            return;

        MenuItem *checker = nextitem; // Check whether there would be a loop if the item were inserted here.
        while (checker)
        {
            if (checker == what)
                throw L"Inserting item before nextitem would create a loop in the menu.";
            checker = checker->ParentItem();
        }

        if (what->Parent())
            what->Parent()->RemoveItem(what);

        //MenuItem *item = new MenuItem(topmenu, this, nextitem);
        what->RecursiveTopmenu(topmenu);
        what->next = nextitem;
        what->parent = this;

        if (nextitem)
        {
            auto pos = std::find(items.begin(), items.end(), nextitem);
            if (pos != items.begin())
                (*(pos - 1))->next = what;
            items.insert(pos, what);
        }
        else
        {
            if (!items.empty())
                items.back()->next = what;
            items.push_back(what);
        }

        bool vis = what->options.contains(mioVisible);
        what->options -= mioVisible;
        UpdateItem(IndexOf(what), vis, what->breaktype);
    }


    MenuItem* MenuBase::InsertItem(MenuItem *nextitem, const std::wstring &text)
    {
        if (nextitem && nextitem->Parent() != this)
            throw L"The specified item is not part of this menu.";

        MenuItem *item = new MenuItem();
#ifdef DESIGNING
        if (!text.empty())
            item->SetName(text);
#endif

        int scpos = text.find(L'\t');
        if (scpos != (int)std::wstring::npos) // The tab character in the menu text tells the system to put a text there which will show up as the shortcut of the menu item. Check whether it would work as a shortcut and if yes, set it as the shortcut, otherwise as the shortcut text.
        {
            WORD sc;
            if (StrToShortcut(text.substr(scpos + 1), sc))
                item->shortcut = sc;
            else
                item->shortcuttext = text.substr(scpos + 1);
            item->text = text.substr(0, scpos);
        }
        else
            item->text = text;

        InsertItem(nextitem, item);

    //#ifdef DESIGNING
    //    if (DesignOwner() != nullptr)
    //        DesignOwner()->AddItem(nextitem ? dynamic_cast<DesignMenuItem*>(nextitem->DesignOwner()) : nullptr, item);
    //#endif

        return item;
    }

    //#ifdef DESIGNING
    //TopDesignMenuItems* MenuBase::TopDesignOwner()
    //{
    //    if (!Designing())
    //        return nullptr;
    //
    //    return dynamic_cast<TopDesignMenuItems*>(dynamic_cast<TopMenuBase*>(TopMenu())->DesignOwner());
    //}
    //
    //DesignFormBase* MenuBase::DesignForm()
    //{
    //    TopDesignMenuItems *desowner = TopDesignOwner();
    //
    //    return desowner ? desowner->Form() : nullptr;
    //}
    //#endif

    void MenuBase::CreateSubHandle()
    {
        if (HandleCreated())
            throw L"Cannot create handle when it is already created.";
        handle = CreatePopupMenu();
    }
 
    MenuItem* MenuBase::Add(const std::wstring &text)
    {
        return InsertItem(NULL, text);
    }

    MenuItem* MenuBase::AddSeparator()
    {
        return InsertItem(NULL, L"-");
    }

    HMENU MenuBase::Handle()
    {
        if (!topmenu->HandleCreated())
            topmenu->CreateHandles();
        return handle;
    }

    bool MenuBase::HandleCommand(UINT itemid)
    {
        for (auto mi : items)
            if (mi->HandleCommand(itemid))
                return true;
        return false;
    }

    bool MenuBase::LastInColumn(int index) const
    {
        CheckIndexInRange(index);
        MenuItem *mi = NULL;
        int cnt = items.size();
        while (++index < cnt)
        {
            mi = items[index];
            if (mi->breaktype != mbtNoBreak)
                break;
            if (mi->options.contains(mioVisible) && !mi->options.contains(mioAutoHidden))
                return false;
        }
        return true;
    }

    bool MenuBase::FirstInColumn(int index) const
    {
        CheckIndexInRange(index);
        MenuItem *mi = NULL;
        do
        {
            if (mi && mi->options.contains(mioVisible) && !mi->options.contains(mioAutoHidden))
                return false;
            mi = items[index];
            if (mi->breaktype != mbtNoBreak)
                break;
        } while (--index >= 0);
        return true;
    }

    void MenuBase::UpdateItem(int index, bool visible, MenuBreakTypes abreaktype)
    {
        CheckIndexInRange(index);
        MenuItem *item = items[index];

        if (item->breaktype == abreaktype && item->options.contains(mioVisible) == visible)
            return;
        bool sep = item->Separator();
        if (sep && item->breaktype == abreaktype && (item->options.contains(mioAutoHidden) || (!item->options.contains(mioVisible) && !visible)))
        {
            if (visible)
                item->options << mioVisible;
            return;
        }
    
        int cnt = item->parent->Count();
        int ix = index;
        bool hideifsep = false; // Will be set to true if this item is a separator and either at the top of its column or there are any visible (or auto hidden) separators in the same column directly above.
        bool septop = false; // Set to true if any item below would be at the top of a column or right below a separator.
        bool mustupdate = false; // Should be true if any item was hidden above us which had a break type other than mbtNoBreak.

        // Let's see if there is a separator above this item which might be removed.
        if (TopMenu()->AutoHideSeparators())
        {
            hideifsep = sep && ((abreaktype != mbtNoBreak) || ix == 0 || ix == cnt - 1);
            septop = ((ix == 0 || abreaktype != mbtNoBreak) && !visible) || (sep && visible); 
            MenuItem *mi = NULL;

            // Find the top separator above this item in the same column in consecutive separators. If the first visible item above is not a separator, or the separator found is at the top of the column the value of sepix is set to -1.
            int sepix = ix;
            while (--sepix >= 0)
            {
                MenuItem *curr = mi;
                mi = item->parent->Items(sepix);

                bool misep = mi->Separator();
                if ((misep && mi->options.contains(mioVisible)) || (!mi->options.contains(mioVisible) && (sepix == 0 || mi->breaktype != mbtNoBreak))) // Make sure the first separator below will know that it is at the top of its column or below a separator.
                    septop = true;
                if (visible && !hideifsep && sep && ((misep && mi->options.contains(mioVisible)) || ((sepix == 0 || mi->breaktype != mbtNoBreak) && !mi->options.contains(mioVisible)))) // Hide our item if another separator is found above it in the same column, or it is the item at the top of the column.
                    hideifsep = true;

                if (!misep && curr && curr->Separator() && curr->options.contains(mioVisible)) // Found the first separator in the same column above our item.
                {
                    ++sepix;
                    break;
                }

                if ((mi->Visible() && !misep) || mi->breaktype != mbtNoBreak)
                    sepix = 0;
            }
            if (sepix >= 0) // Found a separator in the same column and it is in the middle of the menu. Check whether it is the last visible separator in the column to determine whether it must be hidden or shown.
            {
                bool dohide = (abreaktype != mbtNoBreak);
                int mix = ix;
                while (mix < cnt && !dohide) // Go down in the column until a break is found or a visible item which is not a separator (including this one in the search). If no such item is found, the top separator must be hidden, otherwise shown.
                {
                    mi = item->parent->Items(mix);
                    if ((mix == ix && abreaktype != mbtNoBreak) || (mix != ix && mi->breaktype != mbtNoBreak)) // New column means the separator above would be at the end of the column and it must be hidden.
                    {
                        if (visible && !hideifsep && sep)
                            hideifsep = true;
                        dohide = true;
                    }
                    else if (!mi->Separator() && ((mix != ix && mi->options.contains(mioVisible)) || (mix == ix && visible))) // Found a visible non separator item so the separator above can be shown.
                        break;
                
                    ++mix;
                }
                if (visible && !hideifsep && sep && mix == cnt)
                    hideifsep = true;

                // Show or hide the separator above depending on the results.
                mi = item->parent->Items(sepix);
                if (mi->options.contains(mioAutoHidden) != dohide)
                {
                    if (dohide)
                    {
                        mi->options << mioAutoHidden;
                        mi->Hide();
                        if (mi->breaktype != mbtNoBreak)
                            mustupdate = true;
                    }
                    else
                    {
                        mi->options -= mioAutoHidden;
                        //if (mi->parent->HandleCreated())
                        mi->parent->CreateAppend(mi, mi->next);
                    }
                }
            }
        }

        //bool breakchanged = item->breaktype != abreaktype;
        bool nobreakchanged = (item->breaktype == mbtNoBreak) != (abreaktype == mbtNoBreak);
        item->breaktype = abreaktype;

        if (!visible && item->options.contains(mioVisible))
        {
            item->options -= mioVisible;
            if (!item->options.contains(mioAutoHidden))
                item->Hide();
            item->options -= mioAutoHidden;
        }
        else if (visible && sep) // Check whether auto hidden should be applied to this item.
        {
            if (hideifsep && !item->options.contains(mioAutoHidden))
            {
                item->options << mioAutoHidden;
                if (item->options.contains(mioVisible))
                    item->Hide();
            }
            else if (!hideifsep && (item->options.contains(mioAutoHidden) || !item->options.contains(mioVisible)))
            {
                item->options -= mioAutoHidden;
                //if (item->parent->HandleCreated())
                item->parent->CreateAppend(item, item->next);
            }
            item->options << mioVisible;
        }
        else if (visible != item->options.contains(mioVisible))
        {
            if (visible)
            {
                item->options << mioVisible;
                //if (item->parent->HandleCreated())
                item->parent->CreateAppend(item, item->next);
            }
            else
            {
                item->options -= mioVisible;
                item->Hide();
            }
        }
        if (item->options.contains(mioVisible) && !item->options.contains(mioAutoHidden) && mustupdate)
            item->Update(MIIM_STATE | MIIM_FTYPE);
        if (nobreakchanged)
        {
            if (item->options.contains(mioVisible) && !item->options.contains(mioAutoHidden) && item->options.contains(mioGrouped))
                item->GroupChanged();
        }

        // Look for the first visible item below to show/hide if it is a separator and its visible position in columns changed.
        if (TopMenu()->AutoHideSeparators())
        {
            int sepix = ix;
            MenuItem *mi = NULL;
            while (++sepix < cnt)
            {
                mi = item->parent->Items(sepix);
                if (mi->breaktype != mbtNoBreak)
                {
                    sepix = cnt;
                    break;
                }
                else if (mi->options.contains(mioVisible))
                    break;
            }
            if (sepix < cnt && mi->Separator()) // A visible separator is found right below. Hide or show it depending on its new visible position.
            {
                if ((item->options.contains(mioVisible) && sep) || (!item->options.contains(mioVisible) && (item->breaktype != mbtNoBreak || septop))) // The found item is a top separator in the column, hide if visible.
                {
                    if (!mi->options.contains(mioAutoHidden))
                    {
                        mi->options << mioAutoHidden;
                        mi->Hide();
                    }
                }
                else // There is a visible and not separator item above, show the found separator below if it is auto hidden.
                {
                    if (mi->options.contains(mioAutoHidden))
                    {
                        mi->options -= mioAutoHidden;
                        //if (mi->parent->HandleCreated())
                        mi->parent->CreateAppend(mi, mi->next);
                    }
                }
            }
        }

        int mix = ix;
        MenuItem *mi = NULL;
        while (++mix != cnt)
        {
            mi = Items(mix);
            if (mi->Separator())
                continue;
            if (mi->breaktype != mbtNoBreak)
                mix = cnt;
            if (mi->Visible())
                break;
        }
        if (mix < cnt)
            mi->Update(MIIM_STATE | MIIM_FTYPE);
    }

    bool MenuBase::CheckShortcut(WORD shortcut)
    {
        return false;
    }

    bool MenuBase::HandleShortcut(WORD shortcut)
    {
        if (CheckShortcut(shortcut))
            return true;

        if (!SubmenuVisible())
            return false;

        for (auto mi : items)
            if (mi->HandleShortcut(shortcut))
                return true;

        return false;
    }

    Imagelist* MenuBase::TopMenuImagelist()
    {
        TopMenuBase *top = dynamic_cast<TopMenuBase*>(topmenu);
        return top ? top->Images() : NULL;
    }


    //---------------------------------------------

#ifdef DESIGNING
    Object* DesignCreateMenuItem(Object *owner)
    {
        MenuBase *menu = dynamic_cast<MenuBase*>(owner);
        if (menu)
            return menu->Add(L"");
        else
            return new MenuItem();
    }

    //DesignMenuItems* MenuItem::DesignOwner()
    //{
    //    return designowner;
    //}
    //
    //void MenuItem::SetDesignOwner(DesignMenuItem *newdesignowner)
    //{
    //    designowner = newdesignowner;
    //}

    ValuePair<MenuBreakTypes> MenuBreakTypeStrings[] = {
        VALUEPAIR(mbtNoBreak),
        VALUEPAIR(mbtBarBreak),
        VALUEPAIR(mbtBreak),
    };

    void MenuItem::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->SetCreateFunction(&DesignCreateMenuItem);
        serializer->Add(L"SetText", new StringDesignProperty<MenuItem>(L"Text", L"Appearance", &MenuItem::Text, &MenuItem::SetText))->DontExport()->SetDefault(std::wstring());
        serializer->Add(L"SetChecked", new BoolDesignProperty<MenuItem>(L"Checked", L"Behavior", &MenuItem::Checked, &MenuItem::SetChecked))->SetDefault(false);
        serializer->Add(L"SetAutoCheck", new BoolDesignProperty<MenuItem>(L"AutoCheck", L"Behavior", &MenuItem::AutoCheck, &MenuItem::SetAutoCheck))->SetDefault(false);
        serializer->Add(L"SetEnabled", new BoolDesignProperty<MenuItem>(L"Enabled", L"Behavior", &MenuItem::Enabled, &MenuItem::SetEnabled))->SetDefault(true);
        serializer->Add(L"SetVisible", new BoolDesignProperty<MenuItem>(L"Visible", L"Behavior", &MenuItem::Visible, &MenuItem::SetVisible))->SetDefault(true);
        serializer->Add(L"SetGrouped", new BoolDesignProperty<MenuItem>(L"Grouped", L"Behavior", &MenuItem::Grouped, &MenuItem::SetGrouped))->SetDefault(false);
        serializer->Add(L"SetBreakType", new MenuBreakTypesDesignProperty<MenuItem>(L"BreakType", L"Appearance", &MenuItem::BreakType, &MenuItem::SetBreakType))->SetDefault(mbtNoBreak);
        serializer->Add(L"SetShortcut", new ShortcutDesignProperty<MenuItem>(L"Shortcut", L"Behavior", &MenuItem::Shortcut, &MenuItem::SetShortcut))->SetDefault(std::wstring())->SetDefault(std::wstring());
        serializer->Add(L"SetShortcutText", new StringDesignProperty<MenuItem>(L"ShortcutText", L"Appearance", &MenuItem::ShortcutText, &MenuItem::SetShortcutText))->SetDefault(std::wstring());

        serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<MenuItem>(L"ImageIndex", L"Images", true, &MenuItem::TopMenuImagelist, &MenuItem::ImageIndex, &MenuItem::SetImageIndex))->SetDefault(-1);
        serializer->Add(L"SetDisabledIndex", new ImagelistIndexDesignProperty<MenuItem>(L"DisabledIndex", L"Images", true, &MenuItem::TopMenuImagelist, &MenuItem::DisabledIndex, &MenuItem::SetDisabledIndex))->SetDefault(-1);
        serializer->Add(L"SetCheckedIndex", new ImagelistIndexDesignProperty<MenuItem>(L"CheckedIndex", L"Images", true, &MenuItem::TopMenuImagelist, &MenuItem::CheckedIndex, &MenuItem::SetCheckedIndex))->SetDefault(-1);

        serializer->AddEvent<MenuItem, NotifyEvent>(L"OnClick", L"Control");
    }

    void MenuItem::SetName(const std::wstring& newname)
    {
        std::wstring oldname = Name();

        if ((Name().empty() && !newname.empty() && text.empty()) || Name() == text)
        {
            SetText(newname);
            if (Designing() && designer && designer->MainPropertyOwner(this))
                designer->InvalidateRow(this, L"Text");
        }

        base::SetName(newname);

        //if (Designing() && !oldname.empty() && dynamic_cast<TopMenuBase*>(TopMenu()))
        //    DesignForm()->SubObjectRenamed(TopDesignOwner(), this, oldname, Name());
    }

    void MenuItem::SetDesignerItem(DesignMenuItem *item)
    { 
        designeritem = item;
    }
#endif

    MenuItem::MenuItem(MenuBase *topmenu, MenuBase *parent, MenuItem *next) : base(topmenu),
#ifdef DESIGNING
            designeritem(nullptr),
#endif
            id(GenerateCommandId()), parent(parent), next(next), innertext(NULL), shortcut(0), options(mioEnabled | mioVisible), breaktype(mbtNoBreak),
            imgix(-1), disix(-1), chkix(-1)
    {
    }

#ifdef DESIGNING
    MenuItem::MenuItem() : base(), designeritem(nullptr),
#else
    MenuItem::MenuItem() : base(NULL),
#endif
            id(GenerateCommandId()), parent(NULL), next(NULL), innertext(NULL), shortcut(0), options(mioEnabled | mioVisible), breaktype(mbtNoBreak),
            imgix(-1), disix(-1), chkix(-1)
    {
    }

    MenuItem::~MenuItem()
    {
        delete[] innertext;
    }

    void MenuItem::Destroy()
    {
        if (parent)
            parent->RemoveItem(this);
        base::Destroy();
    }

    Form* MenuItem::ParentForm() const
    {
        Form *f = base::ParentForm();
        if (!f && parent)
            return parent->ParentForm();
        return NULL;
    }

    UINT MenuItem::Id()
    {
        return id;
    }

    MenuBase* MenuItem::Parent()
    {
        return parent;
    }

    MenuItem* MenuItem::ParentItem()
    {
        return dynamic_cast<MenuItem*>(parent);
    }

    MenuItem* MenuItem::NextItem()
    {
        return next;
    }

    void MenuItem::Measure(UINT &width, UINT &height)
    {
        Imagelist *list = TopMenuImagelist();

        HDC dc = GetDC(0);
        if (parent == TopMenu() && dynamic_cast<Menubar*>(parent) != NULL)
        {
            ControlCanvas c(dc);
            Size s = themes->MeasureMenubarItem(list && imgix >= 0 && imgix < list->Count() ? list->Width() : 0);
            height = s.cy;
            width = 0;//s.cx;
            width += themes->MeasureMenubarTextExtent(&c, Text(), tdoSingleLine).cx;
        }
        else if (Separator())
        {
            Size s = themes->MeasurePopupMenuSeparator(list ? list->Width() : 0);
            height = s.cy;
            width = s.cx;
        }
        else 
        {
            TopMenuBase *top = dynamic_cast<TopMenuBase*>(TopMenu());
            if (!top)
                return;

            if (breaktype != mbtNoBreak || parent->Items(0) == this)
                top->textw = 0, top->shortcutw = 0;

            ControlCanvas c(dc);
            Size s = themes->MeasurePopupMenuItem(list ? list->Width() : 0, list ? list->Height() : 0);
            height = s.cy;
            width = s.cx;
            top->textw = max(top->textw, themes->MeasurePopupMenuItemTextExtent(&c, Text(), tdoSingleLine).cx);
            shortcutw = top->shortcutw = max(top->shortcutw, themes->MeasurePopupMenuItemTextExtent(&c, DisplayedShortcut(), tdoSingleLine | tdoNoPrefix).cx);
            width += top->textw + top->shortcutw;
        }
        if (dc)
            ReleaseDC(0, dc);
    }

    int MenuItem::ShortcutWidth()
    {
        int r = shortcutw;
        MenuItem *n = next;
        while (n)
        {
            if (n->Visible())
            {
                if (n->breaktype != mbtNoBreak)
                    break;
                r = n->shortcutw;
            }
            n = n->next;
        }
        return r;
    }

    void MenuItem::Draw(DRAWITEMSTRUCT *ds)
    {
        SelectClipRgn(ds->hDC, NULL);

        Imagelist *images = TopMenuImagelist();

        ControlCanvas c(ds->hDC);
        UINT s = ds->itemState;
        if (parent == TopMenu() && dynamic_cast<Menubar*>(parent) != NULL)
        {
            //if ((s & ODA_DRAWENTIRE) == ODA_DRAWENTIRE)
                //themes->DrawMenubarBackground(&c, ds->rcItem, true);
            themes->DrawMenubarItem(&c, ds->rcItem, Text(), (s & ODS_DISABLED) ? ((s & ODS_HOTLIGHT) ? tmbisDisabledHot : (s & ODS_SELECTED) ? tmbisDisabledPushed : tmbisDisabled) : ((s & ODS_HOTLIGHT) ? tmbisHot : (s & ODS_SELECTED) ? tmbisPushed : tmbisNormal), images && imgix >= 0 && imgix <= images->Count() ? images->Width() : 0);
            if (images != NULL && imgix >= 0 && imgix <= images->Count())
            {
                Point pt = themes->MenubarImagePosition(ds->rcItem, images->Height());
                if (s & ODS_DISABLED)
                {
                    if (disix >= 0 && disix < images->Count())
                        images->Draw(&c, disix, pt.x, pt.y);
                    else
                    {
                        Bitmap bmp(images->Width(), images->Height());
                        images->Draw(&bmp, imgix, 0, 0);
                        ColorMatrix cm;
                        cm.Grayscale().TransformAlpha(0.5F);
                        c.SetColorMatrix(cm);
                        c.Draw(&bmp, pt.x, pt.y, 0, 0, bmp.Width(), bmp.Height());
                        c.ResetColorMatrix();
                    }
                }
                else
                    images->Draw(&c, imgix, pt.x, pt.y);
            }
        }
        else if (Separator())
            themes->DrawPopupMenuSeparatorItem(&c, ds->rcItem, tmisNormal, images ? images->Width() : 0);
        else
        {
            themes->DrawPopupMenuItem(&c, ds->rcItem, Text(), DisplayedShortcut(), (s & ODS_DISABLED) ? ((s & ODS_HOTLIGHT) || (s & ODS_SELECTED) ? tmisDisabledHot : tmisDisabled) : ((s & ODS_HOTLIGHT) || (s & ODS_SELECTED) ? tmisHot : tmisNormal), ((s & ODS_CHECKED) == ODS_CHECKED) && (!images || chkix < 0 || chkix >= images->Count()), Grouped(), Count() != 0, images ? images->Width() : 0, images ? images->Height() : 0, ShortcutWidth()); 

            if (images != NULL)
            {
                if (s & ODS_CHECKED)
                {
                    if (chkix >= 0 && chkix < images->Count())
                    {
                        Point pt = themes->PopupMenuItemImagePosition(ds->rcItem, images->Width(), images->Height());
                        if (s & ODS_DISABLED)
                        {
                            Bitmap bmp(images->Width(), images->Height());
                            images->Draw(&bmp, chkix, 0, 0);
                            ColorMatrix cm;
                            cm.Grayscale().TransformAlpha(0.5F);
                            c.SetColorMatrix(cm);
                            c.Draw(&bmp, pt.x, pt.y, 0, 0, bmp.Width(), bmp.Height());
                            c.ResetColorMatrix();
                        }
                        else
                            images->Draw(&c, chkix, pt.x, pt.y);
                    }
                }
                else if (imgix != -1 && imgix < images->Count())
                {
                    Point pt = themes->PopupMenuItemImagePosition(ds->rcItem, images->Width(), images->Height());
                    if (s & ODS_DISABLED)
                    {
                        if (disix >= 0 && disix < images->Count())
                            images->Draw(&c, disix, pt.x, pt.y);
                        else
                        {
                            Bitmap bmp(images->Width(), images->Height());
                            images->Draw(&bmp, imgix, 0, 0);
                            ColorMatrix cm;
                            cm.Grayscale().TransformAlpha(0.5F);
                            c.SetColorMatrix(cm);
                            c.Draw(&bmp, pt.x, pt.y, 0, 0, bmp.Width(), bmp.Height());
                            c.ResetColorMatrix();
                        }
                    }
                    else
                        images->Draw(&c, imgix, pt.x, pt.y);
                }
            }
        }
        HRGN rgn = CreateRectRgn(0, 0, 0, 0);
        SelectClipRgn(ds->hDC, rgn);
        DeleteObject(rgn);
    }

    std::wstring MenuItem::Text()
    {
        if (parent && parent->HandleCreated() && Visible())
        {
            MENUITEMINFO mi;
            memset(&mi, 0, sizeof(MENUITEMINFO));
            mi.cbSize = sizeof(MENUITEMINFO);
            mi.fMask = MIIM_STRING;
            GetMenuItemInfo(parent->Handle(), id, FALSE, &mi);

            if (mi.cch == 0 /*&& mi.dwTypeData != 0*/)
            {
                memset(&mi, 0, sizeof(MENUITEMINFO));
                mi.cbSize = sizeof(MENUITEMINFO);
                mi.fMask = MIIM_TYPE;
                GetMenuItemInfo(parent->Handle(), id, FALSE, &mi);
                if (((int)mi.fType & MFT_SEPARATOR) == MFT_SEPARATOR)
                    text = L"-";
                int scpos = text.find(L'\t');
                if (scpos != (int)std::wstring::npos)
                    return text.substr(0, scpos);
                return text;
            }

            wchar_t *t = new wchar_t[mi.cch+1];
            t[mi.cch] = 0;
            mi.dwTypeData = t;
            ++mi.cch;
            GetMenuItemInfo(parent->Handle(), id, FALSE, &mi);

            text = t;
            delete[] t;

            int scpos = text.find(L'\t');
            if (scpos != (int)std::wstring::npos)
                return text.substr(0, scpos);
            return text;
        }

        return text;
    }

    wchar_t* MenuItem::InnerText()
    {
        delete[] innertext;
        innertext = 0;

        if (shortcut == 0 && shortcuttext.empty())
            return const_cast<wchar_t*>(text.c_str());

        std::wstring sctext = shortcut == 0 ? shortcuttext : ShortcutToStr(shortcut);
        int innerlen = text.length() + 1 + sctext.length() + 1;
        innertext = new wchar_t[innerlen];
        text.copy(innertext, text.length());
        innertext[text.length()] = L'\t';
        sctext.copy(innertext + text.length() + 1, sctext.length());
        innertext[innerlen - 1] = 0;
        return innertext;
    }

    void MenuItem::SetText(const std::wstring &newtext)
    {
        if (text == newtext)
            return;

        text = newtext;

        if (parent && options.contains(mioVisible))
        {
            if (text == L"-")
            {
                options -= mioVisible;
                if (!options.contains(mioAutoHidden))
                    Hide();
                options -= mioAutoHidden;
                parent->UpdateItem(parent->IndexOf(this), true, breaktype);
            }
            else
            {
                if (options.contains(mioAutoHidden))
                {
                    options -= mioAutoHidden;
                    options -= mioVisible;
                    parent->UpdateItem(parent->IndexOf(this), true, breaktype);
                }
                else
                {
                    Update(MIIM_STRING);
                }
            }
        }

#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    std::wstring MenuItem::Shortcut()
    {
        return ShortcutToStr(shortcut);
    }

    void MenuItem::SetShortcut(const std::wstring& newshortcut)
    {
        WORD newval;
        if (!StrToShortcut(newshortcut, newval))
            return;
        if (newval == shortcut)
            return;
        shortcut = newval;
        if (shortcuttext.empty())
            Update(MIIM_STRING);

#ifdef DESIGNING
        if (shortcuttext.empty() && Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    const std::wstring& MenuItem::ShortcutText()
    {
        return shortcuttext;
    }

    void MenuItem::SetShortcutText(const std::wstring& newshortcuttext)
    {
        if (shortcuttext == newshortcuttext)
            return;
        shortcuttext = newshortcuttext;
        Update(MIIM_STRING);

#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    std::wstring MenuItem::DisplayedShortcut()
    {
        if (shortcuttext.empty())
            return Shortcut();
        return ShortcutText();
    }

    bool MenuItem::HandleShortcut(WORD shortcut)
    {
        if (!Visible())
            return false;
        return base::HandleShortcut(shortcut);
    }

    bool MenuItem::HandleCommand(UINT itemid)
    {
        if (id == itemid)
            return Click();

        return base::HandleCommand(itemid);
    }

    bool MenuItem::Click()
    {
        if (options.contains(mioAutoCheck))
            SetChecked(!options.contains(mioChecked));

        if (OnClick)
        {
            OnClick(this, EventParameters());
            return true;
        }
        return false;
    }

    bool MenuItem::Checked()
    {
        return options.contains(mioChecked);
    }

    void MenuItem::SetChecked(bool newchecked)
    {
        if (options.contains(mioChecked) == newchecked)
            return;
        if (newchecked)
        {
            options << mioChecked;
            if (options.contains(mioGrouped))
            {
                int index, first, last;
                GroupRange(index, first, last);
                for (int ix = first; ix <= last; ++ix)
                {
                    if (ix == index)
                        continue;
                    else if (parent->Items(ix)->Checked())
                        parent->Items(ix)->SetChecked(false);
                }
            }
        }
        else
            options -= mioChecked;
        Update(MIIM_STATE);

#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    bool MenuItem::AutoCheck()
    {
        return options.contains(mioAutoCheck);
    }

    void MenuItem::SetAutoCheck(bool newautocheck)
    {
        if (newautocheck == options.contains(mioAutoCheck))
            return;

        if (newautocheck)
            options << mioAutoCheck;
        else
            options -= mioAutoCheck;

        if (!options.contains(mioGrouped))
            return;

        int index, first, last;
        GroupRange(index, first, last);
        for (int ix = first; ix <= last; ++ix)
            if (ix != index)
                parent->Items(ix)->SetAutoCheck(newautocheck);
    }

    bool MenuItem::Enabled()
    {
        return options.contains(mioEnabled);
    }

    void MenuItem::SetEnabled(bool newenabled)
    {
        if (options.contains(mioEnabled) == newenabled)
            return;
        if (newenabled)
            options << mioEnabled;
        else
            options -= mioEnabled;
        Update(MIIM_STATE);

#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    bool MenuItem::Grouped()
    {
        return options.contains(mioGrouped);
    }

    void MenuItem::SetGrouped(bool newgrouped)
    {
        if (options.contains(mioGrouped) == newgrouped)
            return;
        if (newgrouped)
        {
            options << mioGrouped;
            GroupChanged();
        }
        else
            options -= mioGrouped;

        Update(MIIM_STATE | MIIM_FTYPE);

#ifdef DESIGNING
        if (Checked() && Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    void MenuItem::GroupRange(int &index, int &first, int &last)
    {
        index = parent->IndexOf(this);
        if (index < 0)
            throw L"Item not in its parent's items list?";

        int cnt = parent->Count();
        if (!parent->Items(index)->Grouped())
            throw L"Item to specify range not in group.";

        first = index;
        last = index;
        while (--first >= 0 && parent->Items(first)->Grouped())
            if (parent->Items(first + 1)->BreakType() != mbtNoBreak)
                break;

        ++first;
        while (++last < cnt && parent->Items(last)->Grouped() && parent->Items(last)->BreakType() == mbtNoBreak)
            ;
        --last;
    }

    void MenuItem::GroupChanged()
    {
        if (!parent)
            return;

        int index, first, last;
        GroupRange(index, first, last);

        if (index == first && index == last)
            return;

        bool checkfound = false;
        bool autocheck = parent->Items(index == first ? index + 1 : first)->AutoCheck();
        SetAutoCheck(autocheck);
        
        for (int ix = first; ix < index; ++ix)
        {
            MenuItem *mi = parent->Items(ix);
            if (mi->Visible() && mi->Checked())
                checkfound = true;
        }
        if (checkfound)
            options -= mioChecked;
        for (int ix = index + 1; ix <= last; ++ix)
        {
            bool changed = false;
            MenuItem *mi = parent->Items(ix);

            if (mi->Checked() && checkfound)
            {
                mi->options -= mioChecked;
                changed = true;
            }
            if (autocheck != mi->AutoCheck())
            {
                if (autocheck)
                    mi->options << mioAutoCheck;
                else
                    mi->options -= mioAutoCheck;
            }
            if (changed)
                mi->Update(MIIM_STATE);
        }
    }

    void MenuItem::Update(UINT mask)
    {
        if (!parent || !parent->HandleCreated())
            return;

        UpdateMenuItemInfo(this, mask);
    }

    MenuItem* MenuItem::InsertItem(MenuItem *nextitem, const std::wstring &text)
    {
        if (!HandleCreated() && Visible() && parent && parent->HandleCreated())
        {
            CreateSubHandle();

            MENUITEMINFO info = {0};
            info.cbSize = sizeof(MENUITEMINFO);
            info.fMask = MIIM_SUBMENU;
            info.hSubMenu = Handle();

            SetMenuItemInfo(parent->Handle(), id, FALSE, &info);
        }

        return base::InsertItem(nextitem, text);
    }

    void MenuItem::InsertItem(MenuItem *nextitem, MenuItem *what)
    {
        if (!HandleCreated() && Visible() && parent && parent->HandleCreated())
        {
            CreateSubHandle();

            MENUITEMINFO info = {0};
            info.cbSize = sizeof(MENUITEMINFO);
            info.fMask = MIIM_SUBMENU;
            info.hSubMenu = Handle();

            SetMenuItemInfo(parent->Handle(), id, FALSE, &info);
        }

        base::InsertItem(nextitem, what);
    }

    bool MenuItem::Visible()
    {
        return options.contains(mioVisible) && !options.contains(mioAutoHidden);
    }

    void MenuItem::SetVisible(bool newvisible)
    {
        if (options.contains(mioVisible) == newvisible)
            return;

        parent->UpdateItem(parent->IndexOf(this), newvisible, breaktype);
    }

    MenuBreakTypes MenuItem::BreakType()
    {
        return breaktype;
    }

    MenuBreakTypes MenuItem::ComputedBreakType()
    {
        if (breaktype != mbtNoBreak)
            return breaktype;

        int ix = parent->IndexOf(this);
        while (ix > 0)
        {
            MenuItem *mi = parent->Items(--ix);
            if (mi->Visible())
                break;
            if (mi->BreakType() != mbtNoBreak)
                return mi->BreakType();
        }
        return breaktype;
    }

    void MenuItem::SetBreakType(MenuBreakTypes newbreaktype)
    {
        if (breaktype == newbreaktype)
            return;

        parent->UpdateItem(parent->IndexOf(this), options.contains(mioVisible), newbreaktype);

#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    bool MenuItem::Separator()
    {
        return Text() == L"-";
    }

    void MenuItem::Hide()
    {
        if (parent && parent->HandleCreated())
        {
            DeleteMenu(parent->Handle(), id, MF_BYCOMMAND);
            if (parent != TopMenu())
            {
                MenuItem *parentitem = dynamic_cast<MenuItem*>(parent);
                parentitem->CheckSubmenu();
            }
        }
        NullifyHandles();
    }

    bool MenuItem::CheckShortcut(WORD ashortcut)
    {
        if (Visible() && shortcut == ashortcut)
        {
            Click();
            return true;
        }
        return false;
    }

    int MenuItem::ImageIndex()
    {
        return imgix;
    }

    void MenuItem::SetImageIndex(int newimageindex)
    {
        if (imgix == newimageindex)
            return;
        imgix = newimageindex;
#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    int MenuItem::DisabledIndex()
    {
        return disix;
    }

    void MenuItem::SetDisabledIndex(int newdisabledindex)
    {
        if (disix == newdisabledindex)
            return;
        disix = newdisabledindex;
#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }

    int MenuItem::CheckedIndex()
    {
        return chkix;
    }

    void MenuItem::SetCheckedIndex(int newcheckedindex)
    {
        if (chkix == newcheckedindex)
            return;
        chkix = newcheckedindex;
#ifdef DESIGNING
        if (Designing())
        {
            Changed(CHANGE_MENU);
            if (designeritem)
                designeritem->Update(true);
        }
#endif
    }


    //---------------------------------------------


#ifdef DESIGNING
    //TopDesignMenuItems* TopMenuBase::TopDesignOwner()
    //{
    //    return topdesignowner;
    //}
    //
    //void TopMenuBase::SetTopDesignOwner(TopDesignMenuItems *newtopdesignowner)
    //{
    //    topdesignowner = newtopdesignowner;
    //}
    //
    //DesignMenuItems* TopMenuBase::DesignOwner()
    //{
    //    return topdesignowner;
    //}

    Object* TopMenuBase::SubOwner()
    {
        if (TopMenu() == this)
            return base::SubOwner();
        return TopMenu()->SubOwner();
    }

    void TopMenuBase::DesignSubSelected(Object *subobj)
    {
        DesignFormBase *form = dynamic_cast<DesignFormBase*>(ParentForm());

        MenuItem *item = dynamic_cast<MenuItem*>(subobj);
        if (item == nullptr || item->TopMenu() != this)
            throw L"Non-menu item passed as selected sub-object.";

        if (dynamic_cast<PopupMenu*>(this) != nullptr)
            form->EditPopupMenu((PopupMenu*)this, item);
        else
            form->EditDesignMenu((Menubar*)this, item);
    }

    void TopMenuBase::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->Add(L"SetImages", new ImagelistDesignProperty<TopMenuBase>(L"Images", std::wstring(), &TopMenuBase::Images, &TopMenuBase::SetImages))->Delay();
    }
#endif

    TopMenuBase::TopMenuBase() : images(NULL)
    {
    }

    Imagelist* TopMenuBase::Images()
    {
        return images;
    }

    void TopMenuBase::SetImages(Imagelist *newimages)
    {
        if (images == newimages)
            return;

        if (images)
            RemoveFromNotifyList(images, nrSubControl);
        images = newimages;
        if (images)
            AddToNotifyList(images, nrSubControl);

#ifdef DESIGNING
        if (Designing())
            Changed(CHANGE_TOPMENU);
#endif
    }

    void TopMenuBase::DeleteNotify(Object *obj)
    {
        if (obj == images)
        {
            images = NULL;
#ifdef DESIGNING
            if (Designing())
                Changed(CHANGE_TOPMENU);
#endif

        }
        base::DeleteNotify(obj);
    }

    void TopMenuBase::ChangeNotify(Object *obj, int changetype)
    {
#ifdef DESIGNING
        if (Designing() && obj == images)
            Changed(CHANGE_TOPMENU);
#endif

        base::ChangeNotify(obj, changetype);
    }


    //---------------------------------------------


#ifdef DESIGNING
    void Menubar::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
    }
#endif

    Menubar::Menubar() : base()
    {
    }

    Menubar::~Menubar()
    {
    }

    HMENU Menubar::CreateHandle()
    {
        return CreateMenu();
    }


    //---------------------------------------------


#ifdef DESIGNING
    void PopupMenu::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->Find<MenuItemVectorDesignProperty<MenuBase>>(L"Items")->DoList()->MakeDefault();
    
    }
#endif

    PopupMenu::PopupMenu() : base(), selbutton(pmbLeft)
    {
    }

    PopupMenu::~PopupMenu()
    {
    }

    bool PopupMenu::Show(Control *popupcontrol, Point pos)
    {
        bool autopt = (pos.x == -1 && pos.y == -1);
        if (OnShow)
        {
            bool allow = true;
            OnShow(this, PopupMenuParameters(popupcontrol, pos, allow));
            if (!allow)
                return false;
        }
        if (popupcontrol && autopt && (pos.x == -1 && pos.y == -1))
            pos = popupcontrol->ClientToScreen(0, 0);

        UINT flags = (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0 ? (TPM_RIGHTALIGN | TPM_HORPOSANIMATION) : (TPM_LEFTALIGN | TPM_HORNEGANIMATION)) |
                            TPM_VERPOSANIMATION | TPM_RETURNCMD | TPM_NONOTIFY | (selbutton == pmbLeft ? TPM_LEFTBUTTON : TPM_RIGHTBUTTON) ;
        UINT itemid = (UINT)TrackPopupMenuEx(Handle(), flags, pos.x, pos.y, /*popupcontrol->Handle()*/ application->Handle(), NULL);

        if (!itemid)
        {
            if (OnHide)
                OnHide(this, EventParameters());
            return true;
        }

        HandleCommand(itemid);

        if (OnHide)
            OnHide(this, EventParameters());

        return true;
    }

    bool PopupMenu::Show(Control *owner, int x, int y)
    {
        return Show(owner, Point(x, y));
    }

    PopupMenuButtons PopupMenu::AcceptedButton()
    {
        return selbutton;
    }

    void PopupMenu::SetAcceptedButton(PopupMenuButtons newbutton)
    {
        selbutton = newbutton;
    }


    //---------------------------------------------


}
/* End of NLIBNS */


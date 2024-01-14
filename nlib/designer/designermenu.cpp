#include "stdafx_zoli.h"
#include "designermenu.h"
#include "designerform.h"
#include "menu.h"
#include "themes.h"
#include "application.h"
#include "designio.h"
#include "serializer.h"
#include "imagelist.h"

//---------------------------------------------


namespace NLIBNS
{


    DesignMenuItems::DesignMenuManager::DesignMenuManager(DesignMenuItems *owner) : owner(owner), hovered(NULL), hoverpos(-1, -1), addbuttonhovered(false)
    {
    }

    void DesignMenuItems::DesignMenuManager::SetHovered(DesignMenuItem *ahovered, const Point &pt)
    {
        DesignMenuItem *old = hovered;
        Point oldpt = hoverpos;
        hovered = ahovered;
        hoverpos = pt;
        addbuttonhovered = ahovered == NULL && pt.x >= 0;
        HoverChanged(old, oldpt, hovered, hoverpos);
    }

    void DesignMenuItems::DesignMenuManager::SetHovered(DesignMenuItem *ahovered, int x, int y)
    {
        SetHovered(ahovered, Point(x, y));
    }

    DesignMenuItem* DesignMenuItems::DesignMenuManager::Hovered()
    {
        return hovered;
    }

    void DesignMenuItems::DesignMenuManager::HoverChanged(DesignMenuItem *oldhovered, Point oldpt, DesignMenuItem *newhovered, Point newpt)
    {
        if (oldhovered == newhovered && (oldhovered != NULL || ((oldpt != Point(-1, -1)) == (newpt != Point(-1, -1)))))
            return;

        if (oldpt != Point(-1, -1))
        {
            Rect r = oldhovered ? ItemArea(oldhovered) : AreaForAddButton();
            owner->InvalidateRect(r);
        }

        if (newpt != Point(-1, -1))
        {
            Rect r = newhovered ? ItemArea(newhovered) : AreaForAddButton();
            owner->InvalidateRect(r);
        }
    }

    int DesignMenuItems::DesignMenuManager::ItemIndex(DesignMenuItem *item)
    {
        if (item && item->Owner() != owner)
            throw L"Item not in owner menu.";

        int ix = 0;
        for (auto it = owner->items.begin(); it != owner->items.end() && *it != item; ++it, ++ix)
            ;
        return ix;
    }

    bool DesignMenuItems::DesignMenuManager::AddButtonHovered()
    {
        return addbuttonhovered;
    }


    //---------------------------------------------


    DesignMenuItems::DesignMenubarManager::DesignMenubarManager(DesignMenuItems *owner) : base(owner)
    {
    }

    void DesignMenuItems::DesignMenubarManager::EraseBackground()
    {
        Canvas *c = owner->GetCanvas();
        Rect cr = owner->ClientRect();
        --cr.bottom;
        themes->DrawMenubarBackground(c, cr, owner->ParentForm()->Active());
    
        c->SetPen(clMenu);
        c->Line(cr.left, cr.bottom, cr.right, cr.bottom);
    }

    void DesignMenuItems::DesignMenubarManager::HoverChanged(DesignMenuItem *oldhovered, Point oldpt, DesignMenuItem *newhovered, Point newpt)
    {
        base::HoverChanged(oldhovered, oldpt, newhovered, newpt);
    }

    void DesignMenuItems::DesignMenubarManager::UpdateSize()
    {
        items.resize(owner->items.size() - (!owner->items.empty() && owner->items.back() == NULL ? 1 : 0), Rect());
    }

    void DesignMenuItems::DesignMenubarManager::ItemChanged(DesignMenuItem *item)
    {
        int ix = ItemIndex(item);
        while (ix < (int)items.size())
            items[ix++] = Rect();
        owner->Invalidate();
    }

    void DesignMenuItems::DesignMenubarManager::ItemAdded(DesignMenuItem *newitem, DesignMenuItem *next)
    {
        newitem->CreateSubmenu(false);

        int ix = ItemIndex(next);
        items.insert(items.begin() + (ix - 1), Rect());
        while (ix < (int)items.size())
            items[ix++] = Rect();
        owner->Invalidate();
    }

    void DesignMenuItems::DesignMenubarManager::ItemDeleted(DesignMenuItem *next)
    {
        int ix = ItemIndex(next);
        items.erase(items.begin() + ix);
        while (ix < (int)items.size())
            items[ix++] = Rect();
        owner->Invalidate();
    }

    Rect DesignMenuItems::DesignMenubarManager::ItemArea(DesignMenuItem *item)
    {
        if (item == NULL)
            throw L"Trying to get NULL item area.";

        if (items.empty() != owner->items.empty())
            UpdateSize();

        Imagelist *images = owner->BaseMenu()->TopMenuImagelist();

        int ix = 0;
        Rect r;
        Canvas *c = owner->GetCanvas();
        for (auto it = owner->items.begin(); it != owner->items.end(); ++it, ++ix)
        {
            if (*it == nullptr)
                continue;

            if (items[ix].Empty())
            {
                //if (r.Empty())
                //    r.bottom = owner->Height() - 1;
                r.left = r.right;

                Size s = themes->MeasureMenubarItem(images && (*it)->Menu()->ImageIndex() >= 0 && (*it)->Menu()->ImageIndex() < images->Count() ? images->Width() : 0);

                r = RectS(r.left, r.top, s.cx + themes->MeasureMenubarTextExtent(c, (*it)->Menu()->Text(), tdoSingleLine).cx, s.cy);
                items[ix] = r;
            }
            else
                r = items[ix];
            if (*it == item)
                break;
        }

        return r;
    }

    Rect DesignMenuItems::DesignMenubarManager::AreaForAddButton()
    {
        Imagelist *images = owner->BaseMenu()->TopMenuImagelist();

        DesignMenuItem *item = owner->LastItem();
        Rect r = item != NULL ? ItemArea(item) : Rect(0, 0, 0, owner->Height() - 1);
        Canvas *c = owner->GetCanvas();
        r.left = r.right;
        r.right = r.left + themes->MeasureMenubarItem(images ? images->Width() : 0).cx + themes->MeasureMenubarTextExtent(c, L". . . . . .", tdoSingleLine).cx;
        return r;
    }

    void DesignMenuItems::DesignMenubarManager::DrawItem(DesignMenuItem *item)
    {
        ThemeMenubarItemStates state;
        Canvas *c = owner->GetCanvas();
        Imagelist *images = owner->BaseMenu()->TopMenuImagelist();

        if (item)
        {
            Rect r = ItemArea(item);
            state = hovered == item || owner->IsSelected(item) ? tmbisHot : item->IsVisible() ? (item->Enabled() ? tmbisPushed : tmbisDisabledPushed) : !item->Enabled() ? tmbisDisabled : tmbisNormal;
            themes->DrawMenubarItem(c, r, item->Menu()->Text(), state, images && item->Menu()->ImageIndex() >= 0 && item->Menu()->ImageIndex() <= images->Count() ? images->Width() : 0);

            if (images != NULL && item->Menu()->ImageIndex() >= 0 && item->Menu()->ImageIndex() <= images->Count())
            {
                Point pt = themes->MenubarImagePosition(r, images->Height());
                if (!item->Menu()->Enabled())
                {
                    if (item->Menu()->DisabledIndex() >= 0 && item->Menu()->DisabledIndex() < images->Count())
                        images->Draw(c, item->Menu()->DisabledIndex(), pt.x, pt.y);
                    else
                    {
                        Bitmap bmp(images->Width(), images->Height());
                        images->Draw(&bmp, item->Menu()->ImageIndex(), 0, 0);
                        ColorMatrix cm;
                        cm.Grayscale().TransformAlpha(0.5F);
                        c->SetColorMatrix(cm);
                        c->Draw(&bmp, pt.x, pt.y, 0, 0, bmp.Width(), bmp.Height());
                        c->ResetColorMatrix();
                    }
                }
                else
                    images->Draw(c, item->Menu()->ImageIndex(), pt.x, pt.y);
            }

        }
        else
        {
            Rect r = AreaForAddButton();
            themes->DrawMenubarItemHighlight(c, r, addbuttonhovered ? tmbisHot : tmbisNormal);
            c->SetPen(Color(clMenuText).SetA(100));
            c->FrameRect(r.Inflate(-4));
        }
    }


    //---------------------------------------------


    DesignMenuItems::DesignPopupMenuManager::DesignPopupMenuManager(DesignMenuItems *owner) : base(owner)
    {
    }

    void DesignMenuItems::DesignPopupMenuManager::EraseBackground()
    {
        Canvas *c = owner->GetCanvas();
        Rect cr = owner->ClientRect();
        themes->DrawPopupMenuBackground(c, cr);
    }

    void DesignMenuItems::DesignPopupMenuManager::HoverChanged(DesignMenuItem *oldhovered, Point oldpt, DesignMenuItem *newhovered, Point newpt)
    {
        base::HoverChanged(oldhovered, oldpt, newhovered, newpt);
    }

    void DesignMenuItems::DesignPopupMenuManager::Resize()
    {
        Rect wr = owner->WindowRect();
        auto sizes = themes->MenuSizes();
        int w = 0;
        int h = 0;
        for (auto it = owner->items.begin(); it != owner->items.end(); ++it)
        {
            if (!*it)
            {
                Rect r = AreaForAddButton();
                h = max(h, r.bottom);
                w = max(w, r.right);
                break;
            }
            Rect r = ItemArea(*it);
            h = max(h, r.bottom);
            w = max(w, r.right);
        }

        owner->SetBounds(RectS(wr.left, wr.top, w + sizes.popupbordersiz.cx, h + sizes.popupbordersiz.cy));
    }

    void DesignMenuItems::DesignPopupMenuManager::UpdateSize()
    {
        if (items.size() == owner->items.size() - (!owner->items.empty() && owner->items.back() == NULL ? 1 : 0))
        {
            Resize();
            return;
        }

        // Count columns and reset items.
        items.clear();
        columns.clear();

        items.resize(owner->items.size() - (!owner->items.empty() && owner->items.back() == NULL ? 1 : 0), Rect());
        int prev = 0;
        int ix = 0;
        for (auto it = owner->items.begin(); it != owner->items.end(); ++it, ++ix)
        {
            if (!*it)
                break;
            if (it == owner->items.begin() || (*it)->Menu()->BreakType() != mbtNoBreak)
            {
                if (!columns.empty())
                {
                    columns.back().itemcnt = ix - prev;
                    prev = ix;
                }
                columns.push_back(MenuColumn());
            }
        }
        if (!columns.empty())
            columns.back().itemcnt = ix - prev;

        Resize();
    }

    int DesignMenuItems::DesignPopupMenuManager::ItemColumn(int ix, int &pos)
    {
        int column = 0;
        int cnt = columns.size();
        for ( ; column < cnt && (ix > columns[column].itemcnt || (ix == columns[column].itemcnt && column < cnt - 1)) ; ix -= columns[column++].itemcnt)
            ;
        pos = ix;
        return column;
    }

    void DesignMenuItems::DesignPopupMenuManager::ItemChanged(DesignMenuItem *item)
    {
        if (items.size() != owner->items.size() - (owner->items.empty() || owner->items.back() != NULL ? 0 : 1))
            return;

        int ix = ItemIndex(item);
        int pos;
        int column = ItemColumn(ix, pos);
        if (pos == 0 && ix > 0 && item->Menu()->BreakType() == mbtNoBreak)
        {
            pos = columns[column - 1].itemcnt;
            columns[column - 1].itemcnt += columns[column].itemcnt;
            columns.erase(columns.begin() + column);
        }
        else if (pos > 0 && item->Menu()->BreakType() != mbtNoBreak)
        {
            columns.insert(columns.begin() + (column + 1), MenuColumn());
            columns[column + 1].itemcnt = columns[column].itemcnt - pos;
            columns[column].itemcnt = pos;
            pos = 0;
        }
    
        ix -= pos;
        while(ix < (int)items.size())
            items[ix++] = Rect();

        owner->InvalidateRect(ItemArea(item));

        Resize();
    }

    void DesignMenuItems::DesignPopupMenuManager::ItemAdded(DesignMenuItem *newitem, DesignMenuItem *next)
    {
        int ix = ItemIndex(next);
        int pos;
        int column = ItemColumn(ix - 1, pos);

        items.insert(items.begin() + (ix - 1), Rect());

        if (items.size() == 1 || (newitem->Menu()->BreakType() != mbtNoBreak && (ix > 0 || !next || next->Menu()->BreakType() != mbtNoBreak)) || (ix == 0 && (!next || next->Menu()->BreakType() != mbtNoBreak)))
        {
            columns.insert(columns.begin() + column, MenuColumn());
            if (newitem->Menu()->BreakType() == mbtNoBreak || column == 0)
            {
                columns[column].itemcnt = 1;
                pos = 0;
                //--column;
            }
            else
            {
                columns[column].itemcnt = 1 + (columns[column - 1].itemcnt - pos);
                columns[column - 1].itemcnt = pos;
            }
        }
        else
            ++columns[column].itemcnt;
    
        ix -= pos;
        while(ix < (int)items.size())
            items[ix++] = Rect();

        if (owner->TopOwner() != nullptr)
            Resize();
    }

    void DesignMenuItems::DesignPopupMenuManager::ItemDeleted(DesignMenuItem *next)
    {
        int ix = ItemIndex(next);
        int pos;
        int column = ItemColumn(ix + 1, pos);

        items.erase(items.begin() + ix);

        if (pos == 0)
        {
            --column;
            --columns[column].itemcnt;
            if (!columns[column].itemcnt)
                columns.erase(columns.begin() + column);
            else
                pos = columns[column].itemcnt;
        }
        else
        { 
            --columns[column].itemcnt;
            --pos;
        }

        ix -= pos;
        while(ix < (int)items.size())
            items[ix++] = Rect();

        Resize();
    }

    int DesignMenuItems::DesignPopupMenuManager::ItemColumn(DesignMenuItem *item)
    {
        int column = -1;

        for (auto it = owner->items.begin(); it != owner->items.end(); ++it)
        {
            if (!*it)
                break;
            if (it == owner->items.begin() || (*it)->Menu()->BreakType() != mbtNoBreak)
                ++column;

            if (*it == item)
                break;
        }

        return column;
    }

    Rect DesignMenuItems::DesignPopupMenuManager::ItemArea(DesignMenuItem *item)
    {
        if (item == NULL)
            throw L"Trying to get NULL item area.";

        auto sizes = themes->MenuSizes();

        int ix = 0;
        int pos = 0;
        int column = -1;
        Canvas *c = owner->GetCanvas();
        Rect r = RectS(sizes.popupbordersiz.cx, sizes.popupbordersiz.cy, 0, 0);

        Rect found;
        Imagelist *images = owner->BaseMenu()->TopMenuImagelist();

        for (auto it = owner->items.begin(); it != owner->items.end(); ++it, ++ix, ++pos)
        {
            if (!*it)
                break;
            if (!ix || (*it)->Menu()->BreakType() != mbtNoBreak)
            {
                if (!found.Empty())
                    break;

                ++column;
                pos = 0;

                if (!items[ix].Empty())
                    r = items[ix];
                else
                {
                    columns[column].textw = 0;
                    columns[column].shortcutw = 0;

                    if (column > 0)
                        r.left = r.right + 4;
                    r.right = r.left;
                    r.top = r.bottom = sizes.popupbordersiz.cy;
                }
            }
            else
            {
                if (!items[ix].Empty())
                    r = items[ix];
                else
                    r.top = r.bottom;
            }

            if (r.Empty())
            {
                columns[column].textw = max(columns[column].textw, themes->MeasurePopupMenuItemTextExtent(c, (*it)->Menu()->Text(), tdoSingleLine).cx);
                columns[column].shortcutw = max(columns[column].shortcutw, themes->MeasurePopupMenuItemTextExtent(c, (*it)->Menu()->DisplayedShortcut(), tdoSingleLine | tdoNoPrefix).cx);
                Size s = (*it)->Menu()->Separator() ? themes->MeasurePopupMenuSeparator(images ? images->Width() : 0) : themes->MeasurePopupMenuItem(images ? images->Width() : 0, images ? images->Height() : 0);
                r.bottom = r.top + s.cy;
                r.right = r.left + s.cx + columns[column].textw + columns[column].shortcutw + 14;
                items[ix] = r;
            }

            if (*it == item)
                found = r;
        }

        return RectS(found.left, found.top, r.Width(), found.Height());
    }

    Rect DesignMenuItems::DesignPopupMenuManager::AreaForAddButton()
    {
        //if (IsEmpty() && !HasAddButton())
        //    return Rect();

        //auto sizes = themes->MenuSizes();
        //int h;
        //if (themes->MenuThemed())
        //{
        //    h = sizes.popcheck.cy + sizes.popcheckmargin.cyTopHeight + sizes.popcheckmargin.cyBottomHeight + sizes.popcheckbgmargin.cyTopHeight + sizes.popcheckbgmargin.cyBottomHeight;
        //    return RectS(ColumnLeft(columns.size() - 1), columns[columns.size() - 1].height + sizes.popupbordersiz.cy - h, columns[columns.size() - 1].width, h);
        //}
        //else
        //{
        //    h = sizes.rowheight;
        //    return RectS(ColumnLeft(columns.size() - 1), columns[columns.size() - 1].height + sizes.popupbordersiz.cy - h, columns[columns.size() - 1].width, h);
        //}

        Imagelist *images = owner->BaseMenu()->TopMenuImagelist();

        auto sizes = themes->MenuSizes();
        DesignMenuItem *item = owner->LastItem();
        Canvas *c = owner->GetCanvas();
        Rect r = item != NULL ? ItemArea(item) : RectS(sizes.popupbordersiz.cx, sizes.popupbordersiz.cy, themes->MeasurePopupMenuItemTextExtent(c, L". . . . . .", tdoSingleLine).cx, 0);
        r.top = r.bottom;
        Size s = themes->MeasurePopupMenuItem(images ? images->Width() : 0, images ? images->Height() : 0);
        r.bottom = r.top + s.cy;
        //r.right = r.left + themes->MeasureMenubarTextExtent(c, L". . . . . .", tdoSingleLine).cx + sizes.baritemmargin.cxLeftWidth + sizes.baritemmargin.cxRightWidth;
        return r;

    }

    void DesignMenuItems::DesignPopupMenuManager::DrawItem(DesignMenuItem *item)
    {
        Canvas *c = owner->GetCanvas();
        ThemeMenuItemStates state;
        //bool addhovered = false;
        Imagelist *images = owner->BaseMenu()->TopMenuImagelist();
        if (item == NULL)
            state = addbuttonhovered ? tmisHot : tmisNormal;
        else
        //{
        //    int hoverwidth = themes->MenuThemed() ? sizes.popitemmargin.cxRightWidth + sizes.popupbordersiz.cx + sizes.popupsubmnusiz.cx + sizes.popupsubmnucontent.cxLeftWidth + sizes.popupsubmnucontent.cxRightWidth : sizes.itemmargin;
        //    addhovered = item->IsEmpty() && !item->HasAddButton() && hoveritem && hovered && hoveritem == item && !item->Menu()->Separator() && hoverpos.x >= 0 && r.right - hoverwidth <= hoverpos.x;
            state = hovered == item || owner->IsSelected(item) ? (item->Enabled() ? tmisHot : tmisDisabledHot) : !item->Enabled() ? tmisDisabled : tmisNormal;
        //}

        if (item)
        {
            Rect r = ItemArea(item);
            if (!item->Menu()->Separator())
            {
                themes->DrawPopupMenuItem(c, r, item->Menu()->Text(), item->Menu()->DisplayedShortcut(), state, item->Menu()->Checked() && (!images || item->Menu()->CheckedIndex() < 0 || item->Menu()->CheckedIndex() >= images->Count()), item->Menu()->Grouped(), !item->IsEmpty() || item->HasAddButton(), images ? images->Width() : 0, images ? images->Height() : 0, columns[ItemColumn(item)].shortcutw);

                if (images != NULL)
                {
                    if (item->Menu()->Checked())
                    {
                        if (item->Menu()->CheckedIndex() >= 0 && item->Menu()->CheckedIndex() < images->Count())
                        {
                            Point pt = themes->PopupMenuItemImagePosition(r, images->Width(), images->Height());
                            if (!item->Menu()->Enabled())
                            {
                                Bitmap bmp(images->Width(), images->Height());
                                images->Draw(&bmp, item->Menu()->CheckedIndex(), 0, 0);
                                ColorMatrix cm;
                                cm.Grayscale().TransformAlpha(0.5F);
                                c->SetColorMatrix(cm);
                                c->Draw(&bmp, pt.x, pt.y, 0, 0, bmp.Width(), bmp.Height());
                                c->ResetColorMatrix();
                            }
                            else
                                images->Draw(c, item->Menu()->CheckedIndex(), pt.x, pt.y);
                        }
                    }
                    else if (item->Menu()->ImageIndex() >= 0 && item->Menu()->ImageIndex() < images->Count())
                    {
                        Point pt = themes->PopupMenuItemImagePosition(r, images->Width(), images->Height());
                        if (!item->Menu()->Enabled())
                        {
                            if (item->Menu()->DisabledIndex() >= 0 && item->Menu()->DisabledIndex() < images->Count())
                                images->Draw(c, item->Menu()->DisabledIndex(), pt.x, pt.y);
                            else
                            {
                                Bitmap bmp(images->Width(), images->Height());
                                images->Draw(&bmp, item->Menu()->ImageIndex(), 0, 0);
                                ColorMatrix cm;
                                cm.Grayscale().TransformAlpha(0.5F);
                                c->SetColorMatrix(cm);
                                c->Draw(&bmp, pt.x, pt.y, 0, 0, bmp.Width(), bmp.Height());
                                c->ResetColorMatrix();
                            }
                        }
                        else
                            images->Draw(c, item->Menu()->ImageIndex(), pt.x, pt.y);
                    }
                }
            }
            else
                themes->DrawPopupMenuSeparatorItem(c, r, state, images ? images->Width() : 0);
        }
        else
        {
            Rect r = AreaForAddButton();
            themes->DrawPopupMenuItem(c, r, std::wstring(), std::wstring(), state, false, false, false, images ? images->Width() : 0, images ? images->Height() : 0, 0);
            c->SetPen(Color(clMenuText).SetA(100));
            c->FrameRect(r.Inflate(-4));
        }
    }


    //---------------------------------------------


    DesignMenuItems::DesignMenuItems(DesignMenuItems *owner, MenuBase *menu, ItemIterator pos, DesignMenuItemsType type) : base(), owner(owner), pos(pos), type(type), shadow(NULL)
    {
        if (Type() != dmitMenubar)
        {
            controlstyle -= csChild;
            controlstate -= csVisible;
            controlstyle -= csUpdateOnTextChange;
            controlstyle -= csAcceptInput;

            controlstyle << csShowDontActivate;

            manager = new DesignPopupMenuManager(this);
        }
        else
        {
            controlstate -= csVisible;
            manager = new DesignMenubarManager(this);
        }

        //menu->SetDesignOwner(this);

        if (menu)
        {
            int cnt = menu->Count();
            for (int ix = 0; ix != cnt; ++ix)
            {
                MenuItem *mitem = menu->Items(ix);
                mitem->SetDesigning();
                AddMenuItem(nullptr, mitem);

                //auto it = items.insert(items.end(), NULL);
                //*it = new DesignMenuItem(this, mitem, it);
                //mitem->SetDesigning();
                //AddToNotifyList(mitem, nrOwnership);
            }
        }
    

        if (!items.empty() || (owner && items.empty() && owner->Type() == dmitMenubar))
            items.push_back(nullptr);
    }

    void DesignMenuItems::Destroy()
    {
        RestoreShadow();
        std::for_each(items.begin(), items.end(), [](DesignMenuItem* item) {
            if (item)
                item->Destroy();
        });
        delete manager;

        base::Destroy();
    }

    void DesignMenuItems::InitHandle()
    {
        if (Type() != dmitMenubar && TopLevelParent() == NULL)
            SetTopLevelParent(dynamic_cast<DesignMenuItem*>(Owner()) != NULL ? (Control*)Owner() : (Control*)TopOwner()->GetForm());
        base::InitHandle();

        ::SetWindowPos(Handle(), NULL, 0, 0, 0, 0, SWP_FRAMEONLY);

        //if (Type() != dmitMenubar)
        //    Resize();
    }

    void DesignMenuItems::SaveWindow()
    {
        RestoreShadow();
    }

    void DesignMenuItems::RestoreShadow()
    {
        if (shadow && IsWindow(shadow))
        {
            LONG_PTR *udata = (LONG_PTR*)GetWindowLongPtr(shadow, GWLP_USERDATA);
            SetWindowLongPtr(shadow, GWLP_WNDPROC, udata[1]);
            SetWindowLongPtr(shadow, GWLP_USERDATA, 0);
            SetWindowLongPtr(shadow, GWLP_HWNDPARENT, (long)0);
            if (HandleCreated())
                SetWindowLongPtr(Handle(), GWLP_HWNDPARENT, (long)TopLevelParent()->Handle());
            shadow = 0;
        }
    }

    LRESULT CALLBACK ShadowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LONG_PTR *udata = (LONG_PTR*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (udata != NULL)
        {
            DesignMenuItems *item = (DesignMenuItem*)udata[0];
            if (item->shadow != hwnd)
                item->shadow = hwnd; // In case the handle changes. Though unlikely.
            if (uMsg == WM_DESTROY)
                item->RestoreShadow();
        }
        return CallWindowProc((WNDPROC)udata[1], hwnd, uMsg, wParam, lParam);
    }

    void DesignMenuItems::CreateClassParams(ClassParams &params)
    {
        base::CreateClassParams(params);
        if (Parent())
            return;

        params.style << csDropShadow << csHRedraw << csVRedraw;
    }

    void DesignMenuItems::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);
        if (Parent())
            return;

        params.style = wsPopup;
        params.extstyle = wsExNoActivate;
    }

    LRESULT DesignMenuItems::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_MOUSEACTIVATE:
            if (Parent()) // Menu placed on a form.
                break;

            if (!TopOwner()->GetForm()->Focused())
                TopOwner()->GetForm()->Focus();
            return MA_NOACTIVATE;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void DesignMenuItems::Showing()
    {
        base::Showing();
        manager->UpdateSize();
        UpdateBounds();
    }

    void DesignMenuItems::Show()
    {
        if (IsVisible() || (IsEmpty() && !HasAddButton()))
            return;

        base::Show();

        if (Parent() == nullptr)
            if (dynamic_cast<DesignMenu*>(owner) != nullptr)
                owner->InvalidateRect(owner->ItemArea(dynamic_cast<DesignMenuItem*>(this)));


        BOOL spi = FALSE;
        if (Parent() || (shadow && IsWindow(shadow)) || SystemParametersInfo(SPI_GETDROPSHADOW, 0, &spi, 0) == 0 || spi == FALSE)
            return;

        HWND next = Handle();
        while ((next = GetNextWindow(next, GW_HWNDNEXT)) != NULL)
        {
            if (GetClassName(next) == L"SysShadow")
            {
                if (shadow == next)
                    break;
                if (GetWindowLongPtr(next, GWLP_USERDATA) != 0) // Wrong shadow used by someone else already.
                    continue;


                shadow = next;
                LONG_PTR oldproc = GetWindowLongPtr(next, GWLP_WNDPROC);
                shadowdata[0] = (LONG_PTR)this;
                shadowdata[1] = oldproc;
                SetWindowLongPtr(next, GWLP_USERDATA, (long)shadowdata);
                SetWindowLongPtr(next, GWLP_WNDPROC, (long)&ShadowProc);
                SetWindowLongPtr(shadow, GWLP_HWNDPARENT, (long)TopLevelParent()->Handle());
                SetWindowLongPtr(Handle(), GWLP_HWNDPARENT, (long)shadow);
                break;
            }
        }
    }

    void DesignMenuItems::ItemChanged(DesignMenuItem *item)
    {
        manager->ItemChanged(item);
    }

    void DesignMenuItems::ChangeNotify(Object *object, int changetype)
    {
        base::ChangeNotify(object, changetype);
        //if (changetype ==  CHANGE_MENU || dynamic_cast<MenuItem*>(object) != nullptr)
        //{
        //    auto it = std::find_if(Begin(), End(), [object](DesignMenuItem *item) { return item && item->Menu() == object; });
        //    if (it == End())
        //        return;

        //    manager->ItemChanged(*it);

        //    //if (Type() == dmitMenubar)
        //    //{
        //    //    for ( ; it != End(); ++it)
        //    //        if (*it)
        //    //            (*it)->AreaChanged();
        //    //    Invalidate();
        //    //}
        //    //else
        //    //{
        //    //    for (it = Begin() ; it != End(); ++it)
        //    //    {
        //    //        if (*it)
        //    //        {
        //    //            if ((*it)->Menu() == object && (*it)->Menu()->Separator() && (!(*it)->IsEmpty() || (*it)->HasAddButton()))
        //    //                (*it)->DeleteSubmenu();
        //    //            (*it)->AreaChanged();
        //    //        }
        //    //    }
        //    //    Resize();
        //    //    Invalidate();
        //    //}
        //}
    }

    void DesignMenuItems::EraseBackground()
    {
        manager->EraseBackground();
    }

    DesignMenuItems* DesignMenuItems::Owner()
    {
        return owner;
    }

    TopDesignMenuItems* DesignMenuItems::TopOwner()
    {
        return Owner() ? Owner()->TopOwner() : dynamic_cast<TopDesignMenuItems*>(this);
    }

    DesignFormBase* DesignMenuItems::DesignParent()
    {
        return TopOwner() ? TopOwner()->GetForm() : dynamic_cast<DesignFormBase*>(ParentForm());
    }

    DesignMenuItemsType DesignMenuItems::Type()
    {
        return type;
    }

    void DesignMenuItems::CreateSubmenu(bool newitem)
    {
        if (items.empty())
        {
            items.push_back(NULL);
            if (newitem)
            {
                DesignMenuItem *item = AddItem();
                DesignParent()->RegisterSubObject(item->Menu()->TopMenu(), item->Menu(), item->Menu()->Name());

            }
        }
    }

    DesignMenuItem* DesignMenuItems::OpenItem()
    {
        auto it = std::find_if(items.begin(), items.end(), [](DesignMenuItem *item){ return item && item->IsVisible(); });
        if (it != items.end())
            return *it;
        return NULL;
    }

    //DesignMenuItem* DesignMenuItems::Hovered()
    //{
    //    return hovered;
    //}

    DesignMenuItem* DesignMenuItems::AddItem()
    {
        return Insert(NULL);
    }

    DesignMenuItem* DesignMenuItems::InsertItem(DesignMenuItem *next, MenuItem *what)
    {
        //if ((!next && this != TopOwner()) || (next && next->Owner() != this))
        //{
        //    if (next)
        //        return next->Owner()->InsertItem(next, what);
        //    else
        //        return ((DesignMenuItems*)TopOwner())->InsertItem(next, what);
        //}

        if (next && next->Owner() != this)
            throw L"The item where the new item is inserted is not part of this menu";
        if (items.empty())
            items.push_back(NULL);

        auto last = next ? next->pos : --items.end();
        auto insertpos = items.insert(last, 0);
        BaseMenu()->InsertItem(next == NULL ? NULL : next->Menu(), what);
        DesignMenuItem *result = *insertpos = new DesignMenuItem(this, what, insertpos);
        what->SetDesigning();
        what->SetDesignerItem(result);

        //AddToNotifyList(what, nrOwnership);
        //bb what->SetDesignOwner(*insertpos);

        //InvalidateRect(result->Area());
        //for( ; insertpos != items.end(); ++insertpos)
        //    if (*insertpos)
        //        (*insertpos)->AreaChanged();

        manager->ItemAdded(result, next);

        DesignFormBase *form = TopOwner()->GetForm();
        //if (!what->Name().empty())
        //    form->RegisterSubObject(this /*TopOwner()*/, what, what->Name(), CreateEvent(this, &DesignMenuItems::DoSelectItem));
        form->Modify();

        return result;
    }

    DesignMenuItem* DesignMenuItems::AddMenuItem(DesignMenuItem *next, MenuItem *what)
    {
        if (next && next->Owner() != this)
            throw L"The item where the new item is inserted is not part of this menu";

        if (what == nullptr)
            throw L"Calling AddItem with null pointer for what.";

        ItemIterator insertpos = items.empty() ? items.end() : next != nullptr ? std::find(items.begin(), items.end(), next) : items.back() == nullptr ? --items.end() : items.end();
        insertpos = items.insert(insertpos, nullptr);
        *insertpos = new DesignMenuItem(this, what, insertpos);
        what->SetDesigning();
        what->SetDesignerItem(*insertpos);
        //AddToNotifyList(what, nrOwnership);
        //bb what->SetDesignOwner(*insertpos);

        manager->ItemAdded(*insertpos, next);

        if (TopOwner())
        {
            DesignFormBase *form = TopOwner()->GetForm();
            //if (!what->Name().empty())
            //    form->RegisterSubObject(this, what, what->Name());
            form->Modify();
        }

        return *insertpos;
    }

    DesignMenuItem* DesignMenuItems::Insert(DesignMenuItem *next)
    {
        if (next && next->Owner() != this)
            throw L"The item where the new item is inserted is not part of this menu";

        if (items.empty())
            throw L"Calling Insert instead of CreateSubmenu for menu with no items.";

        MenuItem *mitem;
        if (next == nullptr)
            mitem = BaseMenu()->Add(L"MenuItem" + IntToStr(TopOwner()->GetForm()->NameNext(L"MenuItem")));
        else
            mitem = BaseMenu()->InsertItem(next->Menu(), L"MenuItem" + IntToStr(TopOwner()->GetForm()->NameNext(L"MenuItem")));

        DesignMenuItem *added = AddMenuItem(next, mitem); //bb

        //ItemIterator insertpos;
        //insertpos = next != NULL ? std::find(items.begin(), items.end(), next) : --items.end();
        //insertpos = items.insert(insertpos, NULL);
        //*insertpos = new DesignMenuItem(this, mitem, insertpos);
        //mitem->SetDesigning();
        //AddToNotifyList(mitem, nrOwnership);

        //manager->ItemAdded(*insertpos, next);

        auto insertpos = std::find(items.begin(), items.end(), added);

        Select(*insertpos, true);

        //InvalidateRect((*insertpos)->Area());
        //while (next)
        //{
        //    next->AreaChanged();
        //    InvalidateRect(next->Area());
        //    next = next->NextItem();
        //}

        //DesignFormBase *form = TopOwner()->Form();
        //if (!mitem->Name().empty())
        //    form->RegisterSubObject(TopOwner(), mitem, mitem->Name(), CreateEvent(this, &DesignMenuItems::DoSelectItem));
        //form->Modify();

        return *insertpos;
    }


    bool DesignMenuItems::IsHovered(DesignMenuItem *item)
    {
        return (item == NULL && manager->AddButtonHovered()) || (item != NULL && manager->Hovered() == item);
    }

    bool DesignMenuItems::IsSelected(DesignMenuItem *item)
    {
        return item != NULL && std::find(selected.begin(), selected.end(), item) != selected.end();
    }

    int DesignMenuItems::SelCount()
    {
        int r = selected.size();
        for (auto it = items.begin(); it != items.end(); ++it)
            if (*it)
                r += (*it)->SelCount();
        return r;
    }

    DesignMenuItem* DesignMenuItems::SelItems(int ix)
    {
        if (!selected.empty() && ix < (int)selected.size())
        {
            auto it = selected.begin();
            while(ix--)
                ++it;
            return *it;
        }

        ix -= selected.size();

        for (auto it = items.begin(); it != items.end() && ix >= 0; ++it)
        {
            if (!*it)
                break;
            int sc = (*it)->SelCount();
            if (ix < sc)
                return (*it)->SelItems(ix);
            ix -= sc;
        }
        return NULL;
    }

    void DesignMenuItems::MouseLeave()
    {
        base::MouseLeave();

        //if (addbtnhovered)
        //{
        //    InvalidateRect(AreaForAddButton());
        //    addbtnhovered = false;
        //}
        manager->SetHovered(NULL, -1, -1);
    }

    void DesignMenuItems::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
    {
        base::MouseMove(x, y, vkeys);

        if ((manager->Hovered() != NULL && ItemArea(manager->Hovered()).Contains(x, y)) || (manager->AddButtonHovered() && manager->AreaForAddButton().Contains(x, y)))
        {
            manager->SetHovered(manager->Hovered(), Point(x, y) - (!manager->AddButtonHovered() ? ItemArea(manager->Hovered()) : manager->AreaForAddButton()).TopLeft());
            return;
        }

        for (auto it = items.begin(); it != items.end(); ++it)
        {
            if (*it == NULL) // Only the last item can be null.
            {
                Rect r = AreaForAddButton();
                if (manager->AddButtonHovered() != r.Contains(x, y))
                {
                    //InvalidateRect(r);
                    //addbtnhovered = !addbtnhovered;
                    manager->SetHovered(NULL, r.Contains(x, y) ? Point(x, y) - r.TopLeft() : Point(-1, -1));
                    return;
                }
                else if (r.Contains(x, y))
                    return;
                break;
            }

            if (ItemArea(*it).Contains(x, y))
            {
                //if (hovered != NULL)
                //    InvalidateRect(hovered->Area());
                //else if (addbtnhovered)
                //{
                //    addbtnhovered = false;
                //    InvalidateRect(AreaForAddButton());
                //}
                //addbtnhovered = false;
                //hovered = *it;
                Rect r = ItemArea(*it);
                manager->SetHovered(*it, Point(x, y) - r.TopLeft());
                //InvalidateRect(hovered->Area());
                return;
            }
        }
        //if (hovered != NULL)
        //    InvalidateRect(hovered->Area());
        //hovered = NULL;
        //hoverpos = Point(-1, -1);
        manager->SetHovered(NULL, -1, -1);
    }

    //void DesignMenuItemBase::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
    //{
    //    base::MouseMove(x, y, vkeys);
    //    if (Type() == dmitMenubar)
    //        return;
    //
    //    if (hoveritem && !hoveritem->Menu()->Separator() && hoveritem == Hovered())
    //    {
    //        auto sizes = themes->MenuSizes();
    //        int hoverwidth = themes->MenuThemed() ? sizes.popitemmargin.cxRightWidth + sizes.popupbordersiz.cx + sizes.popupsubmnusiz.cx + sizes.popupsubmnucontent.cxLeftWidth + sizes.popupsubmnucontent.cxRightWidth : sizes.itemmargin;
    //
    //        bool addhovered = hoverpos.x >= 0 && Width() - hoverwidth <= hoverpos.x;
    //        hoverpos = Point(x, y - Hovered()->Area().top);
    //        bool newaddhovered = hoverpos.x >= 0 && Width() - hoverwidth <= hoverpos.x;
    //        if (addhovered != newaddhovered)
    //        {
    //            if (!newaddhovered)
    //                hoverpos = Point(-1, -1);
    //            InvalidateRect(hoveritem->Area());
    //        }
    //    }
    //    else
    //    {
    //        hoveritem = Hovered();
    //        hoverpos = Point(-1, -1);
    //    }
    //}


    void DesignMenuItems::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        base::MouseDown(x, y, button, vkeys);
        if (button != mbLeft)
            return;

        if (!manager->AddButtonHovered() && manager->Hovered() == NULL) // The cursor is not over any item.
        {
            if (Type() == dmitMenubar)
            {
                Close();
                TopOwner()->GetForm()->UpdateSelection();
            }
            return;
        }
        Rect addrect = AreaForAddButton();
        bool multi = vkeys.contains(vksShift) || vkeys.contains(vksCtrl); // Multiselect?
        if ((manager->AddButtonHovered() && !addrect.Contains(x, y)) || (manager->Hovered() && !ItemArea(manager->Hovered()).Contains(x, y))) // Cursor outside previously hovered item.
        {
            //InvalidateRect(addbtnhovered ? addrect : hovered->Area());
            //addbtnhovered = false;
            //hovered = NULL;
            //hoverpos = Point(-1, -1);
            manager->SetHovered(NULL, -1, -1);
            return;
        }

        if ((!multi && IsSelected(manager->Hovered()) && selected.size() == 1) || (multi && manager->AddButtonHovered())) // Either the same item would be selected or multi selecting the + menu item which makes no sense.
        {
            if (!multi)
                manager->Hovered()->Show();
            return;
        }

        TopOwner()->GetForm()->ClearSelection(TopOwner()); // Deselect everything on the main form.

        if (manager->AddButtonHovered())
        {
            CreateItem();
            return;
        }
        else if (IsSelected(manager->Hovered()) && multi) // Deselect a hovered item.
        {
            auto it = std::find(selected.begin(), selected.end(), manager->Hovered());
            TopOwner()->RemoveCurrent(manager->Hovered());
            selected.erase(it);
            InvalidateRect(ItemArea(manager->Hovered()));
            if (!TopOwner()->Current())
            {
                Close();
                if (Owner())
                    Owner()->Select(dynamic_cast<DesignMenuItem*>(this), true);
            }
        }
        else if (multi) // Select a hovered item.
        {
            TopOwner()->AddCurrent(manager->Hovered());
            selected.push_back(manager->Hovered());
            InvalidateRect(ItemArea(manager->Hovered()));
        }
        else // Select item under cursor, deselect all other items and close all open sub menus.
        {
            if (!manager->Hovered()->Visible())
            {
                CloseSubmenus();
                if (manager->Hovered()->HasAddButton())
                    manager->Hovered()->Show();
            }
            else
                manager->Hovered()->CloseSubmenus();
            TopOwner()->DeselectAll(); 
            TopOwner()->SetCurrent(manager->Hovered());
            selected.push_back(manager->Hovered());
            InvalidateRect(ItemArea(manager->Hovered()));
        }

        TopOwner()->GetForm()->UpdateSelection();
    }

    //void DesignMenuItemBase::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    //{
    //    base::MouseDown(x, y, button, vkeys);
    //    if (Type() == dmitMenubar)
    //        return;
    //
    //    if (button != mbLeft)
    //        return;
    //
    //    if (hoveritem && !hoveritem->Menu()->Separator() && hoveritem == Hovered())
    //    {
    //        auto sizes = themes->MenuSizes();
    //        int hoverwidth = themes->MenuThemed() ? sizes.popitemmargin.cxRightWidth + sizes.popupbordersiz.cx + sizes.popupsubmnusiz.cx + sizes.popupsubmnucontent.cxLeftWidth + sizes.popupsubmnucontent.cxRightWidth : sizes.itemmargin;
    //        bool addhovered = hoverpos.x >= 0 && Width() - hoverwidth <= hoverpos.x;
    //
    //        if (addhovered)
    //        {
    //            hoveritem->CreateSubmenu(true);
    //            hoveritem->Show();
    //            InvalidateRect(hoveritem->Area());
    //        }
    //    }
    //}
    //
    DesignMenuItem* DesignMenuItems::CreateItem()
    {
        DesignMenuItem *item = AddItem(); // Adds a new menu item.
        DesignParent()->RegisterSubObject(item->Menu()->TopMenu(), item->Menu(), item->Menu()->Name());

        Select(item, true);


        //InvalidateRect(AreaForAddButton());
        return item;
    }

    void DesignMenuItems::CloseSubmenus()
    {
        for (DesignMenuItem *item : items)
            if (item && item->IsOpen())
            {
                item->Close();
                InvalidateRect(ItemArea(item));
            }
    }

    bool DesignMenuItems::SubmenuOpen()
    {
        for (DesignMenuItem *item : items)
            if (item && item->IsOpen())
                return true;
        return false;
    }

    void DesignMenuItems::Close()
    {
        std::for_each(selected.begin(), selected.end(), [this](DesignMenuItem *item) { TopOwner()->RemoveCurrent(item); });
        selected.clear();

        if (OpenItem() != nullptr)
            OpenItem()->Close();
        CloseSubmenus();

        if (Parent() == nullptr)
        {
            if (dynamic_cast<DesignMenu*>(owner) != nullptr)
                owner->InvalidateRect(owner->ItemArea(dynamic_cast<DesignMenuItem*>(this)));
            Hide();
        }
    }

    bool DesignMenuItems::IsOpen()
    {
        return !selected.empty() || OpenItem() != nullptr || SubmenuOpen() || (Parent() == nullptr && Visible());
    }

    void DesignMenuItems::DeselectItems()
    {
        std::for_each(selected.begin(), selected.end(), [this](DesignMenuItem *item) { this->InvalidateRect(ItemArea(item)); TopOwner()->RemoveCurrent(item); });
        selected.clear();

        if (OpenItem())
            OpenItem()->DeselectItems();
    }

    void DesignMenuItems::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        base::MouseUp(x, y, button, vkeys);
    }

    void DesignMenuItems::Paint(const Rect &updaterect)
    {
        Canvas *c = GetCanvas();
        Rect r;

        for (auto it = items.begin(); it != items.end(); ++it)
        {
            r = *it != NULL ? ItemArea(*it) : AreaForAddButton();
            if (!updaterect.DoesIntersect(r))
                continue;

            auto oldstate = c->SaveState();
            DrawItem(*it);
            c->RestoreState(oldstate);
        }
    }

    bool DesignMenuItems::IsLast(DesignMenuItem *item)
    {
        return item->pos == --items.end();
    }

    int DesignMenuItems::Count()
    {
        return items.size();
    }

    DesignMenuItems::ItemIterator DesignMenuItems::Begin()
    {
        return items.begin();
    }

    DesignMenuItems::ItemIterator DesignMenuItems::End()
    {
        return items.end();
    }

    bool DesignMenuItems::IsEmpty()
    {
        return items.empty() || (items.size() == 1 && items.back() == NULL);
    }

    DesignMenuItem* DesignMenuItems::LastItem()
    {
        return items.empty() || (items.size() == 1 && items.back() == NULL) ? NULL : items.back() != NULL ? items.back() : *--(--items.end());
    }

    DesignMenuItem* DesignMenuItems::FirstItem()
    {
        return items.empty() || (items.size() == 1 && items.back() == NULL) ? NULL : items.front();
    }

    bool DesignMenuItems::HasAddButton()
    {
        return !items.empty() && items.back() == NULL;
    }

    DesignMenuItem* DesignMenuItems::PrevItem()
    {
        if (!owner || pos == owner->Begin())
            return NULL;
        auto it = pos;
        return *--it;
    }

    DesignMenuItem* DesignMenuItems::NextItem()
    {
        if (!owner || owner->LastItem() == this)
            return NULL;
        auto it = pos;
        return *++it;
    }

    void DesignMenuItems::Select(DesignMenuItem *item, bool closeothers)
    {
        if (this != item->Owner())
            throw L"Item to select is not part of this menu.";

        TopOwner()->DeselectAll();
        if (!item)
            return;

        if (closeothers && OpenItem() && item != OpenItem())
            OpenItem()->Close();

        TopOwner()->SetCurrent(item);
        selected.push_back(item);
        InvalidateRect(ItemArea(item));
        if (type == dmitMenubar)
            item->Show();
        TopOwner()->GetForm()->UpdateSelection();
    }

    void DesignMenuItems::SelectSubMenu(DesignMenuItem *item)
    {
        std::list<DesignMenuItem*> sub;
        sub.push_front(item);
        while (sub.front()->Owner() != this && sub.front()->Owner())
            sub.push_front((DesignMenuItem*)sub.front()->Owner());

        if (!sub.front()->Owner() || sub.front()->Owner() != this)
            throw L"Item is not part of the submenu under this menu.";

        TopOwner()->GetForm()->ClearSelection(TopOwner());

        TopOwner()->DeselectAll();
        if (OpenItem() && item != OpenItem())
            OpenItem()->Close();

        for (DesignMenuItem *menu : sub)
        {
            DesignMenuItems *owner = menu->Owner();
            if (menu == item)
            {
                owner->selected.push_back(item);
                if (owner->type == dmitMenubar)
                    menu->Show();
            }
            else
                menu->Show();
            owner->InvalidateRect(owner->ItemArea(menu));
        }
        TopOwner()->GetForm()->UpdateSelection();
    }

    DesignMenuItem* DesignMenuItems::GetSubItem(MenuItem *item)
    {
        if (item->TopMenu() != BaseMenu())
            throw "The passed menu item is not part of the menu of this designer.";

        std::list<MenuItem*> sub;
        sub.push_front(item);
        while(dynamic_cast<MenuItem*>(sub.front()->Parent()) != nullptr && sub.front()->Parent() != BaseMenu())
            sub.push_front(dynamic_cast<MenuItem*>(sub.front()->Parent()));

        auto it = items.begin();

        while(true)
        {
            while ((*it)->Menu() != sub.front())
                ++it;
            if (sub.begin() == --sub.end())
                return *it;
            sub.pop_front();
            it = (*it)->items.begin();
        }
    }

    void DesignMenuItems::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        base::KeyPush(keycode, key, vkeys);

        DesignMenuItem *item = TopOwner()->Current();
        if (item && item->Owner() != this)
        {
            item->Owner()->KeyPush(keycode, key, vkeys);
            return;
        }

        if (keycode == VK_DELETE || key == VK_BACK)
        {
            DeleteSelected();
            return;
        }

        if (SelCount() && vkeys.contains(vksCtrl) && ((keycode == VK_INSERT || keycode == VK_DELETE) || (key == ckC || key == ckX)))
        {
            TopOwner()->CopySerialized();
            if (keycode == VK_DELETE || key == ckX)
                DeleteSelected();
            return;
        }

        if ((vkeys.contains(vksShift) && keycode == VK_INSERT) || (vkeys.contains(vksCtrl) && key == ckV))
        {
            TopOwner()->PasteSerialized();
            return;
        }

        if (keycode == VK_INSERT)
        {
            DesignMenuItems *parent = TopOwner()->Current() ? TopOwner()->Current()->Owner() : this;
            DesignMenuItem *where = TopOwner()->Current() ? TopOwner()->Current() : FirstItem();
            parent->Insert(where);
            return;
        }

        if (Type() == dmitList || Type() == dmitPopupMenu)
        {
            if (item && (keycode == VK_ESCAPE || keycode == VK_LEFT || (keycode == VK_RIGHT && (!manager->Hovered() || !manager->Hovered()->IsVisible()))))
            {
                if (keycode == VK_RIGHT)
                {
                    // Open new sub menu.
                    if (vkeys.contains(vksCtrl) && item->IsEmpty() && !item->HasAddButton() && !item->Menu()->Separator())
                    {
                        item->CreateSubmenu(true);
                        item->Show();
                        item->Select(item->FirstItem(), true);
                        return;
                    }
                    else if (!item->IsEmpty() || item->HasAddButton())
                    {
                        item->Show();
                        if (!item->IsEmpty())
                            item->Select(item->FirstItem(), true);
                        return;
                    }
                }

                if (Owner() && Owner()->Type() == dmitMenubar)
                {
                    if (keycode == VK_LEFT && this != Owner()->FirstItem())
                    {
                        Close();
                        //InvalidateArea();
                        if (!PrevItem()->IsEmpty())
                        {
                            PrevItem()->Show();
                            PrevItem()->Select(PrevItem()->FirstItem(), true);
                        }
                        else
                            Owner()->Select(PrevItem(), true);
                        //Owner()->InvalidateRect(PrevItem()->Area());
                    }
                    else if (keycode == VK_RIGHT && this != Owner()->LastItem())
                    {
                        Close();
                        //InvalidateArea();
                        if (!NextItem()->IsEmpty())
                        {
                            NextItem()->Show();
                            NextItem()->Select(NextItem()->FirstItem(), true);
                        }
                        else
                            Owner()->Select(NextItem(), true);
                        //Owner()->InvalidateRect(NextItem()->Area());
                    }
                    else if (keycode == VK_ESCAPE)
                    {
                        if (OpenItem())
                            OpenItem()->Close();
                        else
                            Owner()->Select(dynamic_cast<DesignMenuItem*>(this), true);
                    }
                    return;
                }

                if (keycode != VK_RIGHT)
                {
                    if (Owner())
                        Owner()->Select(dynamic_cast<DesignMenuItem*>(this), true);
                    Close();
                }
                return;
            }

            if (item && keycode == VK_UP)
            {
                item = item->PrevItem();
                Select(item ? item : LastItem(), true);
            }
            else if (keycode == VK_DOWN)
            {
                if (vkeys.contains(vksCtrl))
                    CreateItem();
                else if (item)
                {
                    item = item->NextItem();
                    Select(item ? item : FirstItem(), true);
                }
            }
        }
        else if (Type() == dmitMenubar)
        {
            if (keycode == VK_LEFT && item && item != FirstItem())
            {
                //InvalidateRect(item->Area());
                Select(item->PrevItem(), true);
                //InvalidateRect(item->PrevItem()->Area());
            }
            else if (keycode == VK_RIGHT)
            {
                if (vkeys.contains(vksCtrl))
                    CreateItem();
                else if (item && item != LastItem())
                {
                    //InvalidateRect(item->Area());
                    Select(item->NextItem(), true);
                    //InvalidateRect(item->NextItem()->Area());
                }
            }
            else if (item && keycode == VK_DOWN)
            {
                //InvalidateRect(item->Area());
                item->Show();
                if (vkeys.contains(vksCtrl))
                    item->CreateItem();
                else if (!item->IsEmpty())
                    item->Select(item->FirstItem(), true);
            }
            else if (item && keycode == VK_ESCAPE)
            {
                TopOwner()->DeselectAll();
                CloseSubmenus();
                TopOwner()->GetForm()->UpdateSelection();
            }
        }
    }

    void DesignMenuItems::KeyUp(WORD &keycode, VirtualKeyStateSet vkeys)
    {
        base::KeyUp(keycode, vkeys);
    }

    void DesignMenuItems::DeleteSubmenu()
    {
        Close();
        DesignFormBase *parent = DesignParent();
        std::for_each(items.begin(), items.end(), [parent](DesignMenuItem *item) {
            if (item)
            {
                parent->UnregisterSubObject(item->Menu()->TopMenu(), item->Menu());
                item->DeleteSubmenu();
                item->Destroy();
            }
        });
        items.clear();
    }

    void DesignMenuItems::DeleteItem(DesignMenuItem *item)
    {
        if (item->Owner() != this)
            throw L"Cannot delete item which does not belong to this menu.";

        DesignParent()->UnregisterSubObject(item->Menu()->TopMenu(), item->Menu());


        DesignMenuItem *next = item->NextItem();

        if (manager->Hovered() == item)
            manager->SetHovered(NULL, -1, -1);
        bool sel = IsSelected(item);
        if (sel)
            selected.erase(std::find(selected.begin(), selected.end(), item));
        item->DeleteSubmenu();
        TopOwner()->RemoveCurrent(item);

        DesignFormBase *form = TopOwner()->GetForm();
        //MenuItem *what = item->Menu();
        //if (what && !what->Name().empty())
        //    form->UnregisterSubObject(this, what);
        if (item->Menu())
            item->Menu()->Destroy();

        items.erase(item->pos);
        item->Destroy();
        if (Owner() && Owner()->Type() != dmitMenubar && items.size() == 1 && items.back() == NULL)
            DeleteSubmenu();

        manager->ItemDeleted(next);

        if (sel)
            TopOwner()->GetForm()->UpdateSelection();

        TopOwner()->GetForm()->Modify();
    }

    void DesignMenuItems::DeleteSelected()
    {
        if (Owner())
        {
            Owner()->DeleteSelected();
            return;
        }

        DesignMenuItems* item = this;
        ItemList dellist;
        while (item)
        {
            DesignMenuItem *open = item->OpenItem();
            bool opensel = open == NULL;
            for (auto it : item->selected)
            {
                if (it == open)
                    opensel = true;
                if (item->manager->Hovered() == it)
                    item->manager->SetHovered(NULL, -1, -1);
                dellist.push_back(it);
            }
            if (!opensel)
                item = open;
            else
                item = NULL;
        }
        DesignMenuItem* current = TopOwner()->Current();
        while (!dellist.empty())
        {
            DesignMenuItem* prevcurr = TopOwner()->Current();
            if (current && current->Owner() && current->Owner()->IsSelected(current))
            {
                bool down = current != current->Owner()->LastItem();
                while(current && current->Owner() && current->Owner()->IsSelected(current))
                {
                    if (down)
                    {
                        if (current == current->Owner()->LastItem())
                            down = false;
                        else
                            current = current->NextItem();
                    }
                    if (!down)
                    {
                        if (current == current->Owner()->FirstItem())
                        {
                            current = dynamic_cast<DesignMenuItem*>(current->Owner());
                            down = true;
                        }
                        else
                            current = current->PrevItem();
                    }
                }
            }
            dellist.back()->Owner()->DeleteItem(dellist.back());
            dellist.pop_back();
            if (TopOwner()->Current() && TopOwner()->Current() != prevcurr)
                current = TopOwner()->Current();
        }

        if (current)
            current->Owner()->Select(current, false);
        else
            TopOwner()->GetForm()->UpdateSelection();
    }

    void DesignMenuItems::UpdateBounds()
    {
        if (OpenItem())
            OpenItem()->UpdateBounds();
    }

    void DesignMenuItems::CollectItems(std::list<NonVisualSubItem> &list, bool onlyselected)
    {
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            if (!(*it))
                break;
            if (!onlyselected || IsSelected(*it))
            {
                list.push_back(NonVisualSubItem(L"Items", (*it)->Menu()));
                (*it)->CollectItems(list.back().subitems, false);
            }
            else if ((*it)->IsVisible())
                (*it)->CollectItems(list, true);
        }
    }

    void DesignMenuItems::CopySerialized()
    {
        std::list<NonVisualSubItem> list;
        TopOwner()->CollectItems(list, true);
        Indentation indent(false, 4);
        std::wstringstream str;
        TopOwner()->GetForm()->SerializeNonVisualSubItems(indent, str, list);
        CopyToClipboard(str.str());
    }

    void DesignMenuItems::DrawItem(DesignMenuItem *item/*Canvas *c, DesignMenuItem *item, const Rect &r, bool hovered*/)
    {
        manager->DrawItem(item);
    }

    Rect DesignMenuItems::AreaForAddButton()
    {
        return manager->AreaForAddButton();
    }

    Rect DesignMenuItems::ItemArea(DesignMenuItem *item)
    {
        return manager->ItemArea(item);
    }

    //void DesignMenuItems::RecursiveRegister(ItemIterator begin, ItemIterator end)
    //{
    //    std::for_each(begin, end, [this](DesignMenuItem *item) {
    //        if (!item)
    //            return;
    //        TopOwner()->Form()->RegisterSubObject(TopOwner(), item->Menu(), item->Menu()->Name(), CreateEvent(this, &DesignMenuItems::DoSelectItem));
    //        if (item->Count() != 0)
    //            RecursiveRegister(item->Begin(), item->End());
    //    });
    //}

    void DesignMenuItems::NameChangeNotify(Object *obj, const std::wstring &oldname)
    {
        base::NameChangeNotify(obj, oldname);
        if (dynamic_cast<TopMenuBase*>(obj) != nullptr)
            return;

        //DesignFormBase *form = TopOwner()->GetForm();
        //if (oldname.empty())
        //    form->RegisterSubObject(this, obj, obj->Name());
        //else
        //    form->SubObjectRenamed(this, obj, oldname, obj->Name());
    }

    void DesignMenuItems::DeleteNotify(Object *object)
    {
        //if (dynamic_cast<TopMenuBase*>(obj) != nullptr)
        //    return;

        for (DesignMenuItem *item : items)
        {
            if (item && item->Menu() == object)
            {
                TopOwner()->GetForm()->UnregisterSubObject(this, object);
                break;
            }
        }
    
        base::DeleteNotify(object);
    }


    //---------------------------------------------


    DesignMenuItem::DesignMenuItem(DesignMenuItems *owner, MenuItem *menu, ItemIterator pos) : base(owner, menu, pos, dmitList), menu(menu)
    {
        SetBounds(Rect(0, 0, 100, 100)); // Setting random initial bounds to the window because the csDropShadow style has no effect when the window is shown with no size.
    }

    void DesignMenuItem::Destroy()
    {
        //if (menu != nullptr)
        //    menu->Destroy();

        base::Destroy();
    }

    MenuBase* DesignMenuItem::BaseMenu()
    {
        return menu;
    }

    MenuItem* DesignMenuItem::Menu()
    {
        return menu;
    }

    void DesignMenuItem::UpdateBounds()
    {
        if ((IsEmpty() && !HasAddButton()) || Parent() != NULL)
            return;
        auto sizes = themes->MenuSizes();
        Rect r = Owner()->ClientToScreen(Owner()->ItemArea(this));
        DisplayMonitor *mon = screen->MonitorFromWindow(TopOwner()->GetForm());
        Rect srect = mon->FullArea();
        bool right = GetSystemMetrics(SM_MENUDROPALIGNMENT) == 0;

        int top = Owner()->Type() == dmitMenubar ? r.bottom : r.top - sizes.popupbordersiz.cy;
        if (top + Height() > srect.bottom)
            top = max((Owner()->Type() == dmitMenubar ? r.top : r.bottom + sizes.popupbordersiz.cy) - Height(), srect.top);

        int left = Owner()->Type() == dmitMenubar ? (right ? r.left : r.right - Width()) : r.right - sizes.popupbordersiz.cx;
        if (left + Width() > srect.right)
            left = Owner()->Type() == dmitMenubar ?  max(srect.right - Width(), srect.left) : max(Owner()->Left() + sizes.popupbordersiz.cx - Width(), srect.left);

        SetBounds(RectS(left, top, Width(), Height()));

        base::UpdateBounds();
    }

    void DesignMenuItem::Update(bool contentschanged)
    {
        if (contentschanged)
            UpdateBounds();
        Owner()->ItemChanged(this);
    }

    //const Rect& DesignMenuItem::Area()
    //{
    //    auto sizes = themes->MenuSizes();
    //
    //    if (!validarea)
    //    {
    //        Canvas *c = Owner()->GetCanvas();
    //        c->SetFont(application->MenuFont());
    //
    //        if (Owner()->Type() == dmitMenubar)
    //        {
    //            Rect r = Owner()->FirstItem() == this ? Rect(0, 0, 0, Owner()->Height() - 1) : PrevItem()->Area();
    //            area = Rect(r.right, r.top, r.right + themes->MeasureMenubarTextExtent(c, Menu()->Text(), tdoSingleLine).cx + sizes.baritemmargin.cxLeftWidth + sizes.baritemmargin.cxRightWidth, r.Height());
    //        }
    //        else
    //        {
    //            area = RectS(Owner()->ItemTopLeft(this), Owner()->ColumnWidth(Owner()->ItemColumn(this)), Owner()->ItemHeight(this));
    //        }
    //        validarea = true;
    //    }
    //
    //    return area;
    //}
    //
    //void DesignMenuItem::AreaChanged(bool recursive)
    //{
    //    validarea = false;
    //    if (recursive)
    //    {
    //        Resize();
    //        std::for_each(Begin(), End(), [](DesignMenuItem *item) { if (item) item->AreaChanged(true); });
    //    }
    //}
    //
    //void DesignMenuItem::InvalidateArea()
    //{
    //    Owner()->InvalidateRect(Area());
    //}

    std::wstring DesignMenuItem::Text()
    {
        return menu->Text();
    }

    bool DesignMenuItem::Enabled()
    {
        return menu->Enabled();
    }


    //---------------------------------------------


    TopDesignMenuItems::TopDesignMenuItems(DesignFormBase *form, TopMenuBase *menu, DesignMenuItemsType type) : base(NULL, menu, ItemIterator(), type), form(form), menu(menu)
    {
    }

    void TopDesignMenuItems::Destroy()
    {
        base::Destroy();
    }

    //void TopDesignMenuItems::Synchronize()
    //{
    //    if (Count())
    //        return;

    //    CreateSubmenu(false);
    //    std::list<std::pair<MenuBase*, DesignMenuItem*>> items;
    //    items.push_back(std::make_pair<MenuBase*, DesignMenuItem*>((MenuBase*)menu, nullptr));

    //    while (!items.empty())
    //    {
    //        MenuBase *item = items.front().first;
    //        DesignMenuItem *owner = items.front().second;
    //        items.pop_front();
    //        for (int ix = 0; ix < item->Count(); ++ix)
    //        {
    //            DesignMenuItem *added;
    //            if (owner == nullptr)
    //                added = AddMenuItem(nullptr, item->Items(ix));
    //            else
    //                added = owner->AddMenuItem(nullptr, item->Items(ix));
    //            items.push_back(std::make_pair((MenuBase*)item->Items(ix), added));
    //        }
    //    }
    //}

    LRESULT TopDesignMenuItems::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case wmThemeChanged:
            Close();
            //for (auto it = Begin(); it != End(); ++it)
            //    if (*it)
            //        (*it)->AreaChanged(true);
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    void TopDesignMenuItems::DeselectAll()
    {
        DeselectItems();
    }

    DesignFormBase* TopDesignMenuItems::GetForm()
    {
        return form;
    }

    void TopDesignMenuItems::PasteSerialized()
    {
        std::wstring pasted;
        if (!PasteFromClipboard(pasted))
            return;

        std::wstringstream stream(pasted);
        std::vector<ControlDeserializerItem*> subitems;
        TokenStream token(stream);

        try
        {
            MenuBase *parentmenu = Current() ? Current()->Owner()->BaseMenu() : BaseMenu();
            DesignMenuItem *insertpos = Current() ? Current() : NULL;

            DesignMenuItem *newcurrent = NULL;

            DeserializeFromStream(parentmenu, token, subitems);
            DeselectAll();
            if (subitems.empty())
                return;

            // Processing the items here:
            std::vector<std::pair<Object*, ControlDeserializerProperty*> > delayedprop; // Properties that are restored at the end of deserialization.
            std::vector<std::pair<Object*, ControlDeserializerItem*>> subparents; // List of parents for sub controls.

            for (unsigned int ix = 0; ix < subitems.size(); ++ix)
            {
                ControlDeserializerItem *cd = subitems[ix];
                if (cd->type != typeid(MenuItem))
                    throw TokenE(L"Only menu items are valid in paste", token);

                Object *object = NULL;
                while (!subparents.empty() && cd->parent != subparents.back().second)
                    subparents.pop_back();

                object = cd->serializer->CreateObject(subparents.empty() ? NULL : subparents.back().first);

                if (!object)
                    throw L"Creation of sub control failed.";
                subparents.push_back(std::make_pair(object, cd));

                for (auto pit = cd->properties.begin(); pit != cd->properties.end(); ++pit)
                {
                    ControlDeserializerPropertyNameValue *val = dynamic_cast<ControlDeserializerPropertyNameValue*>(*pit);
                    if (val && val->Name() == L"Name" && GetForm()->NameTaken(val->Value()))
                        continue;

                    if ((*pit)->prop->Delayed())
                    {
                        delayedprop.push_back(std::pair<Object*, ControlDeserializerProperty*>(object, *pit));
                        continue;
                    }
                    (*pit)->SetValue(object);
                }

                if (ix == subitems.size() - 1 || subitems[ix + 1]->parent == NULL) // Insert deserialized menu item with all sub items at the current position.
                {
                    MenuItem *item = dynamic_cast<MenuItem*>(object);
                    while (item && item->Parent())
                        item = dynamic_cast<MenuItem*>(item->Parent());
                    if (!item)
                        throw TokenE(L"Couldn't create menu item for insertion", token);

                    DesignMenuItem *mitem = insertpos->Owner()->InsertItem(insertpos, item);
                    if (newcurrent == NULL)
                        newcurrent = mitem;

                    // Add a unique name to all items after the insert when the form knows about added indexes
                    while(mitem)
                    {
                        if (mitem->Menu()->Name().empty() || GetForm()->NameOwner(mitem->Menu()->Name()) != mitem->Menu())
                        {
                            const std::wstring &menucontrolname = DisplayNameByTypeInfo(typeid(MenuItem), false);
                            mitem->Menu()->SetName(menucontrolname + IntToStr(GetForm()->NameNext(menucontrolname)));
                        }

                        if (!mitem->IsEmpty())
                            mitem = mitem->FirstItem();
                        else
                        {
                            while (mitem->Owner()->LastItem() == mitem && mitem->Menu() != item)
                                mitem = dynamic_cast<DesignMenuItem*>(mitem->Owner());
                            if (mitem->Menu() == item)
                                break;
                            mitem = mitem->NextItem();
                        }
                    } 

                }
            }

            for (auto it = delayedprop.begin(); it != delayedprop.end(); ++it)
                (*it).second->SetValue((*it).first);

            if (newcurrent)
                newcurrent->Owner()->Select(newcurrent, true);

            FreeDeserializerList(subitems);
        }
        catch(TokenE &e)
        {
            FreeDeserializerList(subitems);
            ShowMessageBox(e.what(), L"Paste error", mbOk);
            return;
        }
    }

    void TopDesignMenuItems::SetCurrent(DesignMenuItem* newcurrent)
    {
        if ((currents.empty() && !newcurrent) || (currents.size() == 1 && currents.back() == newcurrent))
            return;
        currents.clear();
        if (newcurrent)
            currents.push_back(newcurrent);

        if (!currents.empty())
            GetForm()->EditingMenu(this);
    }

    void TopDesignMenuItems::AddCurrent(DesignMenuItem* newcurrent)
    {
        if (!newcurrent || (currents.size() > 0 && currents.back() == newcurrent))
            return;
        auto it = std::find(currents.begin(), currents.end(), newcurrent);
        if (it != currents.end())
            currents.erase(it);
        currents.push_back(newcurrent);

        if (!currents.empty())
            GetForm()->EditingMenu(this);
    }

    void TopDesignMenuItems::RemoveCurrent(DesignMenuItem* newcurrent)
    {
        if (!newcurrent)
            return;
        auto it = std::find(currents.begin(), currents.end(), newcurrent);
        if (it != currents.end())
            currents.erase(it);
    }

    DesignMenuItem* TopDesignMenuItems::Current()
    {
        return currents.empty() ? NULL : currents.back();
    }

    Form* TopDesignMenuItems::ParentForm() const
    {
        return form;
    }

    TopMenuBase* TopDesignMenuItems::Menu()
    {
        return menu;
    }


    //---------------------------------------------


    DesignMenu::DesignMenu(DesignFormBase *form, Menubar *menu) : base(form, menu, dmitMenubar), menu(menu)
    {
        SetParentFont(false);
        SetAnchors(caLeft | caTop | caRight);
        SetBounds(RectS(0, 0, form->ClientWidth(), GetSystemMetrics(SM_CYMENU)));
        SetParent(form);
        //if (menu)
        //    AddToNotifyList(menu, nrOwnership);
    }

    void DesignMenu::InitHandle()
    {
        base::InitHandle();
        //MoveToTop(); // Don't call this because it can cause problems when the controllist of the form creates all controls the first time...
    }

    LRESULT DesignMenu::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case wmThemeChanged:
            SetHeight(GetSystemMetrics(SM_CYMENU));
            Invalidate();
            break;
        }
        return base::WindowProc(uMsg, wParam, lParam);
    }

    //void DesignMenu::EraseBackground()
    //{
    //    Canvas *c = GetCanvas();
    //    Rect cr = ClientRect();
    //    --cr.bottom;
    //    themes->DrawMenubarBackground(c, cr, ParentForm()->Active());
    //    
    //    c->SetPen(clMenu);
    //    c->Line(cr.left, cr.bottom, cr.right, cr.bottom);
    //}
    //
    //void DesignMenu::ChangeNotify(Object *object, int changetype)
    //{
    //    base::ChangeNotify(object, changetype);
    //    if (changetype ==  CHANGE_MENU || dynamic_cast<MenuItem*>(object) != NULL)
    //    {
    //        auto it = std::find_if(Begin(), End(), [object](DesignMenuItem *item) { return item && item->Menu() == object; });
    //        if (it == End())
    //            return;
    //        for ( ; it != End(); ++it)
    //            if (*it)
    //                (*it)->AreaChanged();
    //        Invalidate();
    //    }
    //}

    void DesignMenu::ChangeNotify(Object *object, int changetype)
    {
        if (object == menu && changetype == CHANGE_TOPMENU)
            Close();
        base::ChangeNotify(object, changetype);
    }

    MenuBase* DesignMenu::BaseMenu()
    {
        return menu;
    }

    //void DesignMenu::DrawItem(Canvas *c, DesignMenuItem *item, const Rect &r, bool hovered)
    //{
    //    ThemeMenubarItemStates state;
    //    if (item)
    //    {
    //        hovered = hovered || IsSelected(item);
    //        state = hovered ? tmbisHot : item->IsVisible() ? (item->Enabled() ? tmbisPushed : tmbisDisabledPushed) : !item->Enabled() ? tmbisDisabled : tmbisNormal;
    //        themes->DrawMenubarItem(c, r, state);
    //    }
    //    else
    //        themes->DrawMenubarItem(c, r, hovered ? tmbisHot : tmbisNormal);
    //    if (!item) // Item used for adding new items.
    //    {
    //        c->SetPen(Color(clMenuText).SetA(100));
    //        c->FrameRect(r.Inflate(-4));
    //    }
    //    else // Normal item
    //    {
    //        auto sizes = themes->MenuSizes();
    //        Rect r2 = r;
    //        r2.left += sizes.baritemmargin.cxLeftWidth;
    //        r2.top += (Height() - sizes.bartextheight) / 2;
    //
    //        themes->DrawMenubarText(c, r2, item->Text(), tdoSingleLine, state);
    //    }
    //}
    //
    //DesignMenuItem* DesignMenu::InsertItem(DesignMenuItem *next, MenuItem *what)
    //{
    //    auto result = base::InsertItem(next, what);
    //    Invalidate();
    //    return result;
    //}
    //
    //DesignMenuItem* DesignMenu::Insert(DesignMenuItem *next)
    //{
    //    DesignMenuItem *added = base::Insert(next);
    //    added->CreateSubmenu(false);
    //    added->Show();
    //    return added;
    //}

    void DesignMenu::Resizing()
    {
        base::Resizing();
        Invalidate();
    }

    //Rect DesignMenu::AreaForAddButton()
    //{
    //    auto sizes = themes->MenuSizes();
    //    DesignMenuItem *last = LastItem();
    //
    //    Rect r = last == NULL ? Rect(0, 0, 0, Height() - 1) : last->Area();
    //    Canvas *c = GetCanvas();
    //    c->SetFont(application->MenuFont());
    //    return RectS(r.right, r.top, c->MeasureText(L". . . . . .").cx + sizes.baritemmargin.cxLeftWidth + sizes.baritemmargin.cxRightWidth, r.Height());
    //}

    //void DesignMenu::DeleteItem(DesignMenuItem *item)
    //{
    //    base::DeleteItem(item);
    //}

    bool DesignMenu::Active()
    {
        return Current() != NULL;
    }

    void DesignMenu::Activate()
    {
        if (Active())
            return;
        if (IsEmpty())
            CreateItem();
        Select(*Begin(), true);
    }

    void DesignMenu::Close()
    {
        for (int ix = 0; ix < SelCount(); ++ix)
            InvalidateRect(ItemArea(SelItems(ix)));
        base::Close();
    }



    //---------------------------------------------


    DesignPopupMenu::DesignPopupMenu(DesignFormBase *form, PopupMenu *menu) : base(form, menu, dmitPopupMenu), menu(menu)
    {
        SetTopLevelParent(form);
        //if (menu)
        //    AddToNotifyList(menu, nrOwnership);
        SetBounds(Rect(0, 0, 100, 100)); // Setting random initial bounds to the window because the csDropShadow style has no effect when the window is shown with no size.
    }

    MenuBase* DesignPopupMenu::BaseMenu()
    {
        return menu;
    }

    //PopupMenu* DesignPopupMenu::Menu()
    //{
    //    return menu;
    //}

    void DesignPopupMenu::ChangeNotify(Object *object, int changetype)
    {
        if (object == menu && changetype == CHANGE_TOPMENU)
            Close();
        base::ChangeNotify(object, changetype);
    }

    void DesignPopupMenu::UpdateBounds()
    {
        Point lefttop = GetForm()->ClientToScreen(8 * Scaling, 8 * Scaling);
        DisplayMonitor *mon = screen->MonitorFromWindow(GetForm());
        Rect srect = mon->FullArea();

        if (lefttop.x + Width() > srect.right)
            lefttop.x = max(srect.left, srect.right - Width());
        if (lefttop.y + Height() > srect.bottom)
            lefttop.y = max(srect.top, srect.bottom - Height());

        SetBounds(RectS(lefttop.x, lefttop.y, Width(), Height()));

        base::UpdateBounds();
    }

    //---------------------------------------------


}
/* End of NLIBNS */


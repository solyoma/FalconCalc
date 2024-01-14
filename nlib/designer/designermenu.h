#pragma once

#include "controlbase.h"
#include "designerform.h"

//---------------------------------------------


namespace NLIBNS
{


    class Menubar;
    class PopupMenu;
    class DesignFormBase;
    class DesignMenu;
    class DesignMenuItems;
    class DesignMenuItem;
    struct ThemePopupMenuSizes;

    class TopDesignMenuItems;

    enum DesignMenuItemsType { dmitMenubar, dmitPopupMenu, dmitList };

    class DesignMenuItems : public Control
    {
    protected:
        typedef std::list<DesignMenuItem*>              ItemList;
        typedef std::list<DesignMenuItem*>::iterator    ItemIterator;
    private:
        typedef Control  base;

        class DesignMenuManager
        {
        protected:
            DesignMenuItems *owner;
            DesignMenuItem *hovered;
            Point hoverpos;
            bool addbuttonhovered;
            virtual void HoverChanged(DesignMenuItem *oldhovered, Point oldpt, DesignMenuItem *newhovered, Point newpt);
            int ItemIndex(DesignMenuItem *item); // Returns the index of a given item in the owner object.

        public:
            DesignMenuManager(DesignMenuItems *owner);
            virtual void EraseBackground() = 0; // Erases the background of the owner menu.
            virtual void UpdateSize() = 0; // Check whether the menu should be resized and its items measured and sets the size of the menu window.
            virtual void ItemChanged(DesignMenuItem *item) = 0; // Makes the necessary changes to the sizes of menu items that are affected when an item changed.
            virtual void ItemAdded(DesignMenuItem *newitem, DesignMenuItem *next) = 0; // Changes the stored item data if a new item is inserted in front of next.
            virtual void ItemDeleted(DesignMenuItem *next) = 0;
            virtual Rect ItemArea(DesignMenuItem *item) = 0;
            virtual Rect AreaForAddButton() = 0;
            virtual void DrawItem(DesignMenuItem *item) = 0;

            void SetHovered(DesignMenuItem *ahovered, const Point &pt);  // Called when the mouse moves within an item.
            void SetHovered(DesignMenuItem *ahovered, int x, int y);
            DesignMenuItem *Hovered();
            bool AddButtonHovered();

            virtual ~DesignMenuManager() {}
        };

        class DesignMenubarManager : public DesignMenuManager
        {
        private:
            typedef DesignMenuManager   base;
            std::vector<Rect> items;
        protected:
            virtual void HoverChanged(DesignMenuItem *oldhovered, Point oldpt, DesignMenuItem *newhovered, Point newpt);
        public:
            DesignMenubarManager(DesignMenuItems *owner);
            virtual void EraseBackground();
            virtual void UpdateSize();
            virtual void ItemChanged(DesignMenuItem *item);
            virtual void ItemAdded(DesignMenuItem *newitem, DesignMenuItem *next);
            virtual void ItemDeleted(DesignMenuItem *next);
            virtual Rect ItemArea(DesignMenuItem *item);
            virtual Rect AreaForAddButton();
            virtual void DrawItem(DesignMenuItem *item);
        };

        class DesignPopupMenuManager : public DesignMenuManager
        {
        private:
            typedef DesignMenuManager   base;
            struct MenuColumn
            {
                int itemcnt;

                int textw;
                int shortcutw;
            };
            std::vector<MenuColumn> columns;
            std::vector<Rect> items; // Top left corner of an item and its height. The width is meaningless without the text width of all items in the same column.
            int ItemColumn(int ix, int &pos); // Returns the column of the item at index and sets its position within the column in pos.
            int ItemColumn(DesignMenuItem *item);

            void Resize(); // Changes the size of the owner control to fit all menu items and the + button.
        protected:
            virtual void HoverChanged(DesignMenuItem *oldhovered, Point oldpt, DesignMenuItem *newhovered, Point newpt);
        public:
            DesignPopupMenuManager(DesignMenuItems *owner);
            virtual void EraseBackground();
            virtual void UpdateSize();
            virtual void ItemChanged(DesignMenuItem *item);
            virtual void ItemAdded(DesignMenuItem *newitem, DesignMenuItem *next);
            virtual void ItemDeleted(DesignMenuItem *next);
            virtual Rect ItemArea(DesignMenuItem *item);
            virtual Rect AreaForAddButton();
            virtual void DrawItem(DesignMenuItem *item);
        };

        DesignMenuManager *manager;

        DesignMenuItems *owner;
        ItemIterator pos; // Position of the menu item in its owner's items list.

        ItemList items; // Items belonging to this menu item. If this list has a tailing NULL, that is the place for a button to add more menu items.
        ItemList selected; // List of selected items.
        DesignMenuItemsType type;

        //DesignMenuItem *hovered; // Position of the item under the mouse.
        //bool addbtnhovered; // The add button is hovered.

        HWND shadow; // Pointer to the drop shadow under the menu.
        LONG_PTR shadowdata[2];


        void CollectItems(std::list<NonVisualSubItem> &list, bool onlyselected);
        void CopySerialized(); // Copies the selected menu items to the clipboard.

        void RestoreShadow(); // Returns the window procedure of the shadow window and deletes its data.

        friend LRESULT CALLBACK ShadowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        friend class DesignMenuManager;
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual void SaveWindow();
        virtual void Showing();

        virtual void MouseLeave();
        virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

        virtual void ChangeNotify(Object *object, int changetype);
        virtual void NameChangeNotify(Object *obj, const std::wstring &oldname);
        virtual void DeleteNotify(Object *object);

        bool IsHovered(DesignMenuItem *item); // Pass null to check whether the add button is hovered.
        bool IsSelected(DesignMenuItem *item);
        bool IsLast(DesignMenuItem *item);

        virtual void EraseBackground();
        virtual void Paint(const Rect &updaterect);
        virtual void DrawItem(DesignMenuItem *item/*Canvas *c, DesignMenuItem *item, const Rect &r, bool hovered*/);

        TopDesignMenuItems* TopOwner();

        int Count(); // Number of sub items.
        ItemIterator Begin(); // Iterator to the first item.
        ItemIterator End(); // Iterator pointing after the last item.

        //void RecursiveRegister(ItemIterator begin, ItemIterator end); // Register added menu items with the designer form, so the designer lists them.

        //virtual void InvalidateArea() {}
        //virtual void AreaChanged(bool recursive = false) {};

        /* constructor */ DesignMenuItems(DesignMenuItems *owner, MenuBase *menu, ItemIterator pos, DesignMenuItemsType type);
        virtual ~DesignMenuItems() {}
    public:
        virtual void Destroy();
        virtual void Show();

        DesignMenuItemsType Type();
        DesignMenuItems* Owner();
        virtual DesignFormBase* DesignParent();

        DesignMenuItem* PrevItem(); // Returns the item before this one in the same sub menu or null if this is the first item.
        DesignMenuItem* NextItem(); // Returns the next item after this one in the same sub menu or null if this was the last item.
        DesignMenuItem* FirstItem(); // Returns the first sub menu item in the list of this menu item or NULL if no items are in the list.
        DesignMenuItem* LastItem(); // Returns the last sub menu item in the list of this menu item or NULL if no items are in the list.

        DesignMenuItem* AddItem(); // Adds new item after the last one.
        virtual DesignMenuItem* InsertItem(DesignMenuItem *next, MenuItem *what); // Insert a menu item in front of next, creating the corresponding design menu item.
        virtual DesignMenuItem* Insert(DesignMenuItem *next); // Inserts new item before the position of next.

        DesignMenuItem* AddMenuItem(DesignMenuItem *next, MenuItem *what); // Adds item before "next" for an existing menu item.

        void Select(DesignMenuItem *item, bool closeothers); // Used for navigating in the menu with the keyboard or selecting the next item after a delete.
        void SelectSubMenu(DesignMenuItem *item); // Opens the menu item found somewhere under this item, deselecting and closing all others.
        DesignMenuItem* GetSubItem(MenuItem *item); // Returns the designer for a menu sub-item, if it is part of this menu.

        virtual void CreateSubmenu(bool newitem); // Creates a submenu for the menu item. Set newitem to true if the new submenu should hold a new item as well not just the + button.
        void DeleteSubmenu(); // Delete all sub menu items of the given item, including the + button.
        virtual void DeleteItem(DesignMenuItem *item); // Deletes the item and all its sub items.
        void DeleteSelected(); // Delete all items from the whole menu, not just from the submenu of this one.
        DesignMenuItem* OpenItem();
        DesignMenuItem* Hovered();
        DesignMenuItem* CreateItem(); // Behaves the same way as clicking on the + button.
        virtual void DeselectItems(); // Deselects everything in this menu and all sub menus.
        virtual void CloseSubmenus();
        virtual bool SubmenuOpen(); // Returns true if calling Close() on at least one submenu has any effect.

        Rect ItemArea(DesignMenuItem *item);
        Rect AreaForAddButton(); // Area of the + button in the list if this item has a submenu.

        bool IsEmpty(); // No items, including the add button.
        bool HasAddButton(); // Returns true if there is a NULL item at the end of the items list which is a placeholder for the add button.

        virtual void Close(); // Deselects all menu items under this menu, closes all open submenus and hides itself, unless this is the main menu.
        virtual bool IsOpen(); //Returns true if calling Close() has any effect.

        virtual MenuBase* BaseMenu() = 0; // Returns the menu or menu item associated with the DesignMenuItems. Call Menu() of the derived class to get the final class of the menu item or menu.

        int SelCount();
        DesignMenuItem* SelItems(int ix);

        virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);
        virtual void KeyUp(WORD &keycode, VirtualKeyStateSet vkeys);

        virtual void UpdateBounds();
        void ItemChanged(DesignMenuItem *item);
    };

    class DesignMenuItem : public DesignMenuItems
    {
    private:
        typedef DesignMenuItems  base;

        MenuItem *menu; // Menu item or main menu or popup menu directly owning this item.
    protected:
        virtual MenuBase* BaseMenu();

        virtual ~DesignMenuItem() {}
    public:
        /* constructor */ DesignMenuItem(DesignMenuItems *owner, MenuItem *menu, ItemIterator pos);

        virtual void Destroy() override;

        MenuItem* Menu();

        std::wstring Text();
        bool Enabled();
        virtual void UpdateBounds();
        void Update(bool contentschanged); // Called by a menu item when its text or other property changed which affects its appearance. Set contentschanged to true if the bounds of the menu item should be updated as well.
    };

    class Form;
    class TopDesignMenuItems : public DesignMenuItems
    {
    private:
        typedef DesignMenuItems  base;

        DesignFormBase *form;
        TopMenuBase *menu;

        ItemList currents; // A list of menu items in the order they were clicked or selected by the keyboard. The one at the end of the list is the item selected last.
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual ~TopDesignMenuItems() {}
        virtual MenuBase* BaseMenu() = 0;
    public:
        TopDesignMenuItems(DesignFormBase *form, TopMenuBase *menu, DesignMenuItemsType type);
        virtual void Destroy();

        virtual Form* ParentForm() const;

        void DeselectAll(); // Starting from the main menu, recursively deselects items in all sub menus.

        DesignFormBase* GetForm();
        void SetCurrent(DesignMenuItem* newcurrent);
        void AddCurrent(DesignMenuItem* newcurrent);
        void RemoveCurrent(DesignMenuItem* newcurrent);
        DesignMenuItem* Current();

        void PasteSerialized(); // Pastes menu items from the clipboard.
        //void Synchronize(); // Creates sub items to match the items of the real menu. Does nothing if it already has sub items.

        TopMenuBase* Menu();
    };

    class DesignMenu : public TopDesignMenuItems
    {
    private:
        typedef TopDesignMenuItems  base;

        Menubar *menu; // Menu item or main menu or popup menu directly owning this item.
    protected:
        virtual void InitHandle();
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void ChangeNotify(Object *object, int changetype);

        //virtual void EraseBackground();
        virtual void Resizing();

        virtual MenuBase* BaseMenu();

        virtual ~DesignMenu() {}
    public:
        DesignMenu(DesignFormBase *form, Menubar *menu);
        virtual bool Active(); // The design menu has open sub menus or the menu bar has something selected.
        virtual void Activate(); // Act like a normal menu when the user presses the alt button.
        virtual void Close();
        //Menubar* Menu();
    };

    class DesignPopupMenu : public TopDesignMenuItems
    {
    private:
        typedef TopDesignMenuItems  base;
        PopupMenu *menu; // Menu item or main menu or popup menu directly owning this item.
    protected:
        virtual MenuBase* BaseMenu();
        virtual void ChangeNotify(Object *object, int changetype);

        virtual ~DesignPopupMenu() {}
    public:
        DesignPopupMenu(DesignFormBase *form, PopupMenu *menu);

        //PopupMenu *Menu();
        virtual void UpdateBounds();
    };


}
/* End of NLIBNS */


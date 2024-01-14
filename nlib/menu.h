#pragma once

#include "events.h"
#include "objectbase.h"


namespace NLIBNS
{

    class MenuItem;
#ifdef DESIGNING
    class DesignMenuItem;
#endif

    enum MenuBreakTypes {
            mbtNoBreak = 0, mbtBarBreak = MFT_MENUBARBREAK, mbtBreak = MFT_MENUBREAK,
#ifdef DESIGNING
            mbtCount = 3
#endif
    };

//#ifdef DESIGNING
    //class TopDesignMenuItems;
    //class DesignMenuItems;
    //class DesignMenuItem;
//#endif

    class Imagelist;

    class MenuBase : public NonVisualControl
    {
    private:
        typedef NonVisualControl    base;
//#ifdef DESIGNING
//        Object *designowner; // Object representing the menu item in the designer (type is DesignMenuItems)
//#endif

        HMENU handle; // Handle to the menu or submenu this item contains. This is not created unless the menu is on a form which has a created handle.
        std::vector<MenuItem*> items;
        bool SubmenuVisible(); // Returns whether the subitems array contains any visible items.

        MenuBase *topmenu; // The mainmenu or popupmenu containing this item or sub item.

        bool autosep; // Don't show duplicate separators or separators at the end of a menu or menu column.

        void CreateHandles(); // Creates the handle of the top level menu and all all sub items.
        void RecursiveTopmenu(MenuBase *newtopmenu);
    protected:
        MenuBase(); // Creates a top level menu.
        MenuBase(MenuBase *topmenu); // Creates a sub level menu with a specified topmenu.

        void CheckIndexInRange(int index) const; // Throws an exception if the passed index is out of range of submenu items.

        virtual HMENU CreateHandle(); // Returns a window menu or popup menu handle.
        void NullifyHandles(); // Sets the handle variable in this item and all its sub items to NULL recursively, after deleting a menu item on some level above which invalidates all handles.
        void CreateSubHandle(); // Creates a submenu handle for the given item.
        void CreateAppend(MenuItem *item, MenuItem *nextitem); // Helper to CreateHandles and Insert, to tell the system about the menu items.

        void UpdateItems(); // Hide or show separators depending on the autosep value.

        static void UpdateMenuItemInfo(MenuItem *item, UINT mask); // Updates the menuitem's information if it is visible and is in a menu with a valid handle.
        static void FillMenuItemInfo(MenuItem *item, MENUITEMINFO &info, UINT mask); // Fills the passed MENUITEMINFO with information matching the menuitem's members.

        void CheckSubmenu(); // Checks if the submenu still contains items and if not, removes it from the system, but not the objects.

        virtual bool CheckShortcut(WORD shortcut);

        virtual ~MenuBase() {}
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        //virtual Object* NameOwner(const std::wstring &name);
        //virtual void Names(std::vector<std::wstring> &namelist); // Fills the namelist with the names of all subitems in the menu.

        void DesignAdd();

        //void SetDesignOwner(Object *obj); // Set the designmenuitem owner of this control. Once it is changed it can't be updated.
        virtual Object* SubOwner(); // Returns the object set with SetDesignOwner as the owner of this sub item.

        virtual void CollectSubItems(std::list<NonVisualSubItem> &subitems);
#endif
        virtual void Destroy();

        virtual bool HandleCommand(UINT itemid); // Calls the click event of the menu item with the given id. The menu looks for subitems recursively until on with this id is found.

        HMENU Handle(); // Retrieves the handle of the main menu, popup menu or sub menu, creating it if necessary.
        bool HandleCreated();

        MenuBase* TopMenu(); // The top menu (i.e. a main menu or popup menu) which holds all menu items, including this one.
        Imagelist* TopMenuImagelist();

        bool AutoHideSeparators();
        void SetAutoHideSeparators(bool newautosep);

        int Count() const; // Number of sub items in this menu item.
        MenuItem* Items(int index); // Access to each of the sub items.
        int IndexOf(MenuItem *subitem);

        virtual bool HandleShortcut(WORD shortcut);

        MenuItem* Insert(int index, const std::wstring &text); // Inserts new text menu item at the given position.
        virtual MenuItem* InsertItem(MenuItem *nextitem, const std::wstring &text); // Inserts new text menu item before nextitem.
        void Insert(int index, MenuItem *what); // Inserts menu item at the given position.
        virtual void InsertItem(MenuItem *nextitem, MenuItem *what); // Inserts existing menu item before nextitem, removing it from its previous position.
        MenuItem* Add(const std::wstring &text); // Adds new text menu item at the end of the menu or submenu.
        MenuItem* AddSeparator(); // Adds a new separator at the end of the menu or submenu. Behaves the same way as Add(L"-").
        void Delete(int index); // Removes the item from the menu and deletes its memory.
        void DeleteItem(MenuItem *item); // Removes the item from the menu and deletes its memory.
        void RemoveItem(MenuItem *item); // Removes the item from the menu without deleting its memory.

        void UpdateItem(int index, bool visible, MenuBreakTypes breaktype); // Shows/hides the item while updating its properties and other items in the same menu around it. Calling this function has the same effect as changing the visibility or break type of a menu item.

        bool LastInColumn(int index) const; // Returns true if there is no visible item below the index or it belongs to the next visible column. Even if the item at index is invisible itself.
        bool FirstInColumn(int index) const; // Returns true if there is no visible item above the index or it belongs to the previous visible column. Even if the item at index is invisible itself.
    };


    enum MenuItemOptions { mioAutoCheck = 0x0001, mioAutoHidden = 0x0002, mioGrouped = 0x0004, mioChecked = 0x0008, mioEnabled = 0x0010, mioVisible = 0x0020 };
    typedef uintset<MenuItemOptions> MenuItemOptionSet;

    // MenuItem is the only class which can directly modify the menu. The MenuBase, Menubar and PopupMenu indirectly modify menu items, calling their MenuItem object's functions. 
    class MenuItem : public MenuBase
    {
    private:
        typedef MenuBase    base;
    
#ifdef DESIGNING
        DesignMenuItem *designeritem;
#endif

        UINT id;
        MenuBase *parent; // The menu which contains this item as a sub item.
        MenuItem *next; // The next menu item in the menu this item belongs to.

        std::wstring text;
        wchar_t *innertext; // Generated from text and shortcut or shortcuttext to pass to the system when the text must be updated.

        WORD shortcut; // Virtual key code in the low order byte and a combination of Ctrl (bit 1), Shift (bit 2) or Alt (bit 3) in the high order byte.
        std::wstring shortcuttext; // Text shown as the shortcut for the menu item. This is usually the same that we could get from the shortcut's value, but can be a combination such as "Ctrl+K, L" when the shortcut is Ctrl+K where some code must wait for another key press.

        MenuItemOptionSet options;
        void Update(UINT mask); // Updates the enabled or checked etc. states of the menu item when its parent has a working handle. 

        void GroupRange(int &index, int &first, int &last);
        void GroupChanged(); // Updates the options for this menu item when it is added into a group.

        void Hide(); // Removes the menu item from the system, but doesn't delete the object itself. Used mainly when hiding an item because its visible state has changed or it must be autohidden.

        MenuBreakTypes breaktype;
        MenuBreakTypes ComputedBreakType(); // Returns the break type that is used when the menu is shown. This can be different from breaktype if a column breaking item is hidden just before this one, so this item must be put to the next column.

        wchar_t* InnerText(); // Generates text in the innertext member that will be set as the text for the menu item.

        // Indexes in the imagelist for the different states of the menu item.
        int imgix;
        int disix;
        int chkix;

        int shortcutw; // Width of the longest shortcut text up to this menu item in its own menu column.

        int ShortcutWidth(); // Returns the highest shortcutw value in the menu column of this menu item.

        void Measure(UINT &width, UINT &height);
        void Draw(DRAWITEMSTRUCT *ds);

        friend class MenuBase;
        friend class Control;
        friend class Application;
        virtual ~MenuItem();
    protected:
        MenuItem(MenuBase *topmenu, MenuBase *parent, MenuItem *next);

        virtual bool CheckShortcut(WORD shortcut);
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual void SetName(const std::wstring& newname);

        void SetDesignerItem(DesignMenuItem *item);
#endif
        MenuItem();

        virtual void Destroy();

        virtual Form* ParentForm() const;

        virtual bool HandleCommand(UINT itemid); // Calls the click event of the menu item with the given id. The menu looks for subitems recursively until one with this id is found. Returns true when a menu item had an OnClick event and it was called.
        bool Click(); // Performs the action of the menu item, changing the checked state if it is automatic and calling its OnClick event. Returns true if the event was called.


        virtual MenuItem* InsertItem(MenuItem *nextitem, const std::wstring &text); // Inserts new text menu item before nextitem.
        virtual void InsertItem(MenuItem *nextitem, MenuItem *what); // Inserts existing menu item before nextitem, removing it from its previous position.

        MenuBase* Parent(); // The item or menu holding this one.
        MenuItem* ParentItem(); // The item which contains this one as a sub item. It is the same as Parent() if the parent is a MenuItem, otherwise NULL.
        UINT Id(); // The id used in identifying this menu item.
        MenuItem* NextItem();
        std::wstring Text(); // Displayed text for this item.
        void SetText(const std::wstring &newtext); // Change the displayed text for this item.

        std::wstring Shortcut(); // Text representation of the shortcut of this menu item. This shows up in the menu unless ShortcutText is set as well.
        void SetShortcut(const std::wstring& newshortcut); // Set the text representation of the shortcut of this menu item. 
        const std::wstring& ShortcutText(); // The text shown as the shortcut for this menu item. If not set the value will not affect how the shortcut keys are shown in the menu.
        void SetShortcutText(const std::wstring& newshortcuttext); // Set the text shown as the shortcut for this menu item.
        std::wstring DisplayedShortcut(); // Returns the ShortcutText() text if set, otherwise the value of Shortcut().

        virtual bool HandleShortcut(WORD shortcut);

        bool Separator(); // Returns whether the given menu item is a separator line. That is, its text is L"-".

        bool Checked();
        void SetChecked(bool newchecked);

        bool AutoCheck();
        void SetAutoCheck(bool newautocheck);

        bool Enabled();
        void SetEnabled(bool newenabled);

        bool Visible();
        void SetVisible(bool newvisible);

        bool Grouped();
        void SetGrouped(bool newgrouped);

        MenuBreakTypes BreakType();
        void SetBreakType(MenuBreakTypes newbreaktype);

        int ImageIndex();
        void SetImageIndex(int newimageindex);
        int DisabledIndex();
        void SetDisabledIndex(int newdisabledindex);
        int CheckedIndex();
        void SetCheckedIndex(int newcheckedindex);

        NotifyEvent OnClick;
    };

    class TopMenuBase : public MenuBase
    {
    private:
        typedef MenuBase base;
        int textw, shortcutw; // Values used when measuring menu items. The longest text and shortcut text in the currently measured row.

        Imagelist *images; // List of images shown in menu items under this menu.

        friend class MenuItem;
    protected:
        virtual ~TopMenuBase() {}

        virtual void DeleteNotify(Object *obj);
        virtual void ChangeNotify(Object *obj, int changetype);

#ifdef DESIGNING
        virtual void DesignSubSelected(Object *subobj);
#endif

        TopMenuBase();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Object* SubOwner();
#endif

        Imagelist* Images();
        void SetImages(Imagelist *newimages);
    };

    class Menubar : public TopMenuBase
    {
    private:
        typedef TopMenuBase base;

    protected:
        virtual HMENU CreateHandle();

        virtual ~Menubar();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        Menubar();
    };

    enum PopupMenuButtons { pmbLeft, pmbBoth };
    class PopupMenu : public TopMenuBase
    {
    private:
        typedef TopMenuBase base;

        PopupMenuButtons selbutton;
    protected:
        virtual ~PopupMenu();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        PopupMenu();

        // Pops up the menu at the specified position. Returns false if showing the menu was prevented in the OnShow event.
        bool Show(Control *owner, int x, int y);
        bool Show(Control *owner, Point pos);

        PopupMenuButtons AcceptedButton();
        void SetAcceptedButton(PopupMenuButtons newbutton);

        PopupMenuEvent OnShow;
        NotifyEvent OnHide;
    };


}
/* End of NLIBNS */


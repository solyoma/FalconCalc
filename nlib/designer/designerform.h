#pragma once

#include <set>
#include "generalcontrol.h"

//---------------------------------------------


namespace NLIBNS
{

    class TokenStream;
    class Indentation;

    class TransparentColorPickerDialog;
    class BitmapSelectorDialog;

    class DesignForm;
    class ButtonPanel;
    class DesignProperty;
    struct ControlDeserializerItem;

    enum DesignSizerSides { dssLeft = 0x01, dssTop = 0x02, dssRight = 0x04, dssBottom = 0x08, dssTopLeft = 0x03, dssTopRight = 0x06, dssBottomLeft = 0x09, dssBottomRight = 0x0C };
    class DesignSizer : public Control
    {
    private:
        typedef Control base;
        DesignForm *form;
    
        DesignSizerSides side;
    protected:
        virtual void Paint(const Rect &updaterect);
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

        void PlaceOnRect(Rect r);

        DesignSizer(DesignForm *form, DesignSizerSides side);
        friend class DesignForm;
    public:
    };

    class Designer;
    class DesignContainerForm;
    class PropertyEditorForm;
    class NonVisualControl;
    class Menubar;
    class DesignMenu;
    class TopDesignMenuItems;
    //class DesignPopupMenu;

    enum DesignDirections { dirNone = 0, dirLeft = 1, dirRight = 2, dirUp = 4, dirDown = 8  };

    // Base class of DesignForm and DesignContainerForm that can hold visual or non visual controls, defining many virtual abstract functions, to give a general interface to designer windows.
    class DesignFormBase : public Form
    {
    public:
        typedef std::pair<std::wstring, Object*> EventNameObjPair;
        typedef std::vector<EventNameObjPair> EventNameObjVector;
        typedef std::vector<EventNameObjPair>::iterator EventNameObjVectorIterator;
        struct EventListItem {
            std::wstring type; // Unique type of the event in the format "eventnamespace::eventparam". The type cannot contain the pipe | character. When the type is the same for two functions, they are compatible and both can be assigned for the event.
            std::wstring func; // Name of the function specified for this event. If func is an empty string, this event was deleted since it was last exported.
            std::wstring exportedfunc; // Name that was last exported to source files, so renaming can be handled.
            EventNameObjVector events; // A vector containing a pair of event name (i.e. OnKeyDown) and object responding to that event.

            EventListItem(const std::wstring &eventtype, const std::wstring &funcname, const std::wstring exportedfunc);
            EventListItem(const std::wstring &eventtype, const std::wstring &eventname, Object *obj, const std::wstring &funcname);
        };

    private:
        typedef Form base;

        DesignMenu *designmain; // Main menu selected for the form.
        TopDesignMenuItems *editedmenu; // Menu currently being edited.

        bool designvis;
        bool autocreate; // Create the form automatically in the generated project initializer function.

        std::wstring namespc; // Namespace where the form will be placed. This can be empty.
        std::wstring unit; // Name of cpp file which will be output when exporting the forms.
        std::wstring exportedname; // Name used by the form when it was last exported to the cpp files, or an empty string. In the latter case, OriginalName() returns the same as Name().

        bool modified; // The form was modified in the editor and its cpp files must be re-exported.

        typedef std::list<EventListItem> EventList;
        typedef std::list<EventListItem>::iterator EventListIterator;
        EventList eventlist; // List of event function types and names and objects using those events. Each item holds an event and event name, a value which is the name of the function and owner objects responding to that event.
        EventListIterator GetEventByFuncname(const std::wstring &funcname); // Returns an iterator to the event with the given function name.
        std::pair<EventListIterator, EventNameObjVectorIterator> GetEventForObject(const std::wstring &eventtype, Object *obj, const std::wstring &eventname); // Returns an iterator to the event in eventlist of the given object with the given event type and name.

        // Data of a sub-object registered by a control placed on the form.
        struct SubObject
        {
            Object *owner; // Owner control containing the sub-object. The control must be a visual or non visual control placed on this form.
            Object *obj; // The sub-object.
            std::wstring name; // Name of the object which must be unique. The form makes sure that when renaming other controls they don't get a conflicting name.

            SubObject(Object *owner, Object *obj, const std::wstring &name) : owner(owner), obj(obj), name(name) {}
        };

        std::list<SubObject> subobjects; // List of sub-objects registered with RegisterSubObject by objects placed on design forms. The list is used for showing these sub objects in the property editor's control list, and for selecting them by name.

        void RenameEvents(Object *obj, std::wstring oldname, std::wstring newname);
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void WindowBoundsChanged(const Rect &oldrect, const Rect &newrect);

        virtual void Showing();
        virtual void Hiding();

        virtual void NameChangeNotify(Object *obj, const std::wstring &oldname);

        virtual void NamesList(std::vector<std::wstring> &namelist); // Fills a vector with the names of the form's controls. The base implementation adds all registered sub control names.

        virtual bool ActiveDesigner() = 0;
        virtual std::wstring BaseClass() = 0;

        virtual void DeleteNotify(Object *object);
        virtual void ChangeNotify(Object *object, int changetype);

        virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);
        virtual void KeyUp(WORD &keycode, VirtualKeyStateSet vkeys);

        enum DesignStates { dsCreating = 0x01, dsDeserializing = 0x02, dsDeleting = 0x04 };
        typedef uintset<DesignStates> DesignStateSet;
        DesignStateSet designstate;

        // Functions to print simple lines in the header and cpp sources.
        void print_class_line(Indentation &indent, std::wiostream &stream);

        void print_private_header_base(Indentation &indent, std::wiostream &stream);
        void print_protected_header_base(Indentation &indent, std::wiostream &stream);
        void print_public_header_base(Indentation &indent, std::wiostream &stream);

        void print_private_header(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events);
        void print_protected_header(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events);
        void print_public_header(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events);
        void print_members(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, AccessLevels access, bool printheader); // Prints the form member controls and events to the header.

        void print_object_declaration(Indentation &indent, std::wiostream &stream, bool printextern);
        enum WhichDestructorDef { wddConstructor = 1, wddDestructor = 2, wddDestroyer = 4};
        typedef uintset<WhichDestructorDef> WhichDestructorDefs;
        void print_c_and_d_structor_definition(Indentation &indent, std::wiostream &stream, WhichDestructorDefs which);
        void print_event_handler_lines(Indentation &indent, std::wiostream &stream, const std::wstring& handlername, const std::wstring& paramtype);

        void print_member_initialization(Indentation &indent, std::wiostream &stream);

        /* destructor */ virtual ~DesignFormBase() {}
    public:
        static void EnumerateProperties(DesignSerializer *serializer);
        /* constructor */ DesignFormBase();
        virtual void Destroy();

        virtual void Modify(bool tomodified = true);
        bool Modified();

        virtual void SetName(const std::wstring& newname);
   
        bool IsDesignVisible(); // Returns whether the form is to be made visible in the editor when it is loaded in the project.
        void SetDesignVisible(bool newvis); // Makes the design form visible or hidden when it is loaded in the project. Don't mix with DesignSetVisible which is the setter for the Visible property.

        virtual void WriteToStream(Indentation &indent, std::wiostream &stream) = 0;
        virtual void ReadFromStream(TokenStream &token) = 0;

        virtual void HeaderExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, bool printpragma);

        virtual void HeaderMemberList(Indentation &indent, std::wiostream &stream, AccessLevels access) = 0;
        virtual void CppMemberList(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents) = 0;
        virtual bool PrintShowAfterMemberList() = 0;
        virtual void CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events) = 0;

        virtual void HeaderUpdate(Indentation &indent, std::wstringstream &stream, const std::vector<EventListItem*> &events);
        virtual void CppUpdate(Indentation &indent, std::wstringstream &stream, const std::vector<EventListItem*> &events);
        virtual void CollectEvents(std::vector<EventListItem*> &events);

        virtual void ClearSelection(Object *sender) = 0;

        virtual void UpdateSelection() { ; }

        virtual Object* NameOwner(const std::wstring &name);
        int NameNext(const std::wstring &name);

        virtual void RemoveNotify(Object *object) = 0;

        virtual void CollectObjects(std::vector<std::pair<std::wstring, Object*>> &objectstrings, bool (*collector)(Object*) = NULL, const std::wstring &objectname = std::wstring()); // Returns a vector of <object name, object> pairs placed on the designer form. The string for the object name is same as object->Name() for objects created by the form, and owner_form->Name()::name->Name() when not. When collector function is passed, call that function for every object and only include them in the list when it returns true. When objectname is set, only the object with that name should be added to the vector.

        virtual std::wstring GetGuestControls() = 0; // Returns a comma separated list with the name of non-visual guest controls, with their type, owner form and control name.
        virtual void AddGuestControls(const std::wstring &guests) = 0; // Adds non-visual guest controls to the form, by their type, owner form and control name.

        virtual std::pair<std::wstring, Object*> SelectedObject() = 0; // Returns the currently selected object and its name, if only a single item is selected. The string for the object name is same as object->Name() for objects created by the form, and owner_form->Name()::name->Name() when not. If more than one items are selected, returns the number of selected items as a string and NULL for the object.
        virtual void SelectObject(std::wstring name) = 0; // Selects an object by name. The name follows the format returned by SelectedObject and CollectObjects. If name is empty, select the form.
        virtual void SelectObject(Object *obj) = 0; // Selects an object. The object must be placed on the form, and it cannot be a sub-object. If the passed object is null, the form is selected.
        virtual void ActivateSelection() = 0; // Updates the state of the form to reflect that it gained the active designer form status.
        virtual void DeactivateSelection() = 0; // Updates the state of the form to reflect that it lost the active designer form status.
        virtual std::wstring ControlName(Object *c, const std::wstring &name) = 0; // Returns the display name of a control. If the control is owned by a different form, it is included in the name.

        bool GuestReferenceOn(DesignFormBase *guestparent); // Checks whether guestparent contains guest controls from this form or other forms, that would create a circular reference of guests between the forms.
        virtual bool GuestReferenceOn(DesignFormBase *guestparent, std::set<DesignFormBase*> &checkfinished) = 0; // Checks whether guestparent contains guest controls from this form or other forms, that would create a circular reference of guests between the forms. Set checkfinished to a set which contains all forms that have already been checked for circular references to exclude in further checks.

        bool NameTaken(const std::wstring &name); // Returns true if NameOwner would return an object holding the passed name.
        virtual bool FormNameTaken(const std::wstring &name);

        void RegisterSubObject(Object *owner, Object *subobj, const std::wstring &subobjname); // Register a sub object which is not a real control, but has editable properties and can be accessed on the form. I.e. tabs of a tab page or menu items in a menu. An event must be provided that when called, selects the sub object.
        void UnregisterSubObject(Object *owner, Object *subobj); // Unregister a registered sub object.
        void SubObjectRenamed(Object *owner, Object *subobj, const std::wstring &oldname, const std::wstring &newname); // Call when a registered sub object has been renamed.
        std::wstring SubObjectName(Object *owner, Object *subobj); // Returns the current saved name of a registered sub object.
        bool SelectSubObject(const std::wstring& name); // Iterates through the registered sub object owners, asking them to select their sub object with the specified name, until one is found which contains the sub object. Returns whether there was a control which reported a matching name. Controls that are not shown cannot be selected this way.

        const std::wstring& UnitName();
        void SetUnitName(const std::wstring &newunit);
        std::wstring DefaultUnitName();

        std::wstring LastExportedName(); // Name of the form before it was renamed. If this is the same as Name(), the form hasn't been renamed since last export.
        std::wstring LastExportedClassName(); // Name of the form's class before it was renamed. Same as ClassName() if Name() hasn't changed.
        void SetLastExportedName(const std::wstring &neworigname);
        void FinalizeExport(); // Sets all strings in the form that hold the names of previously exported members to source files. This function is called after each export.

        std::wstring LastExportedEvents();
        void SetLastExportedEvents(const std::wstring &str);

        int GetEventCountById(const std::wstring &eventtype); // Return the number of events on this form that have the specified type.
        std::wstring GetEventFunctionByIndex(const std::wstring &eventtype, int index); // Return the indexth function name of the event with the specified type, when listing events. The events are ordered alphabetically.
        int GetEventFunctionIndex(const std::wstring &eventtype, Object *obj, const std::wstring &funcname); // Returns the index of the passed function name from a sorted list of functions with the same event type.
        EventListItem* GetEventFunction(const std::wstring &eventtype, Object *obj, const std::wstring &eventname);
        bool SetEventFunction(const std::wstring &eventtype, Object *obj, const std::wstring &eventname, const std::wstring &value);

        bool AutoCreate();
        void SetAutoCreate(bool newauto);

        unsigned int CreationOrder(); // The construction line of forms and containers is generated in the destination project source in the order of the value of the creation order. Forms not auto created should have a valid order to be shown in the project settings and the designer's menu.
        void SetCreationOrder(unsigned int neworder);

        void SerializeNonVisualSubItems(Indentation &indent, std::wiostream &stream, std::list<NonVisualSubItem> &items);
        virtual void SetProperties() = 0;

        void FinishProcessing(const std::vector<Object*> objects); // Called after ProcessDeserializedItems() has done deserializing, to do anything that might have to be done. 

        virtual void CloseEditedMenu();
        void EditingMenu(TopDesignMenuItems *menu); // Called by a menu to notify the form that it received user interaction and is being edited.
        void EditPopupMenu(PopupMenu *menu, MenuItem *item = nullptr); // Shows the popup menu designer, selecting an item if passed.
        void EditDesignMenu(Menubar *menubar, MenuItem *item = nullptr); // Shows the main menu designer, selecting an item if passed.

        DesignMenu* DesignMain(); // Designer of a main menu.

        //bool EditingPopupMenu();
        //void AddSelectedPopupItems(std::list<Object*> &sellist);
        //std::pair<std::wstring, Object*> SelectedPopupObject();
        //int PopupMenuSelCount();

        TopDesignMenuItems* EditedMenu();
        //DesignMenu* GetDesignMenu(Menubar *menu);
    };

    // Form designer which is used to place, move, edit and delete controls.
    class DesignForm : public DesignFormBase
    {
    private:
        typedef DesignFormBase  base;

        IconData formicon;

        ButtonPanel *controlbuttons; // Panel containing buttons for control placement. The form must be able to tell it to unpress a button once the control is placed.
        DesignContainerForm *containerform; // Form holding non visual controls belonging to this window.

        struct PlacementControl;
        typedef std::list<PlacementControl*> PlacementList;
        typedef std::map<Control*, PlacementControl*> PlacementMap;
        typedef PlacementList::iterator PlacementIterator;
        typedef PlacementList::reverse_iterator PlacementRIterator;
        typedef PlacementMap::iterator PlacementMapIterator;

        Menubar *menubar; // Design menu currently shown as the menu bar on the form.
        bool menualt; // The alt key was pressed so send every menu keys to the menu. Invalidated on focus change or when the alt is pressed again.

        PopupMenu *pmenu;

        MenuItem *pmiPos;
        MenuItem *pmiPosBottom;
        MenuItem *pmiPosDown;
        MenuItem *pmiPosUp;
        MenuItem *pmiPosFront;

        MenuItem *pmiAlign;
        MenuItem *pmiLefts;
        MenuItem *pmiRights;
        MenuItem *pmiCenters;
        MenuItem *pmiHSpace;
        MenuItem *pmiTops;
        MenuItem *pmiBottoms;
        MenuItem *pmiVCenters;
        MenuItem *pmiVSpace;

        MenuItem *pmiSize;
        MenuItem *pmiWidths;
        MenuItem *pmiHeights;

        int menuextra; // Number of extra rows added at the top of the popup menu when it is sent to a specific control which uses design time popup menu events.

        struct Subcontrol
        {
            HWND handle;
            PWndProc proc;
            PlacementControl *owner;
            DesignForm *parentform;
        };
    
        struct PlacementControl
        {
            Control *control; // The underlying control.

            PlacementControl *parent;

            PlacementIterator tabpos; // The position of this control in its parent's tab list.
            PlacementIterator controlpos; // The position of this control in its parent's controls list.
            bool selected; // The control is selected in the designer. This is false if the designer only has one control selected which is in the current variable.

            std::vector<Subcontrol*> subcontrols; // A list of windows controls, which belong to the placed control and cannot be designed separately.

            PlacementList tablist; // A list of direct placement child controls in their tab order 
            PlacementList controls; // A list of direct placement child controls in their z-order;
        };

        PlacementControl *current; // The current control when only one is selected.

        // These are only set when the current control was selected with SelectDesignControl within a parent object. In that case the current placement control's propowner and objparent are ignored, and selpropowner and selobjparent are used instead.
        Rect selarea; // The area which has to be shown as selected, in the current selected control's client coordinates.
        Object *selpropowner; // When an area within a control is selected, selpropowner is the object providing the properties for that area.
        Object *selmaincontrol; // The real selected control when an area is selected that provides properties for a part of this control.
        int seltag; // User defined data.
    
        PlacementList tablist; // A list of direct child controls in their tab order on the form 
        PlacementList controls; // A list of direct child controls in their z-order on the form;

        PlacementList controllist; // A list of all controls placed on the form or on other controls, in random order.
        PlacementMap controlmap; // Similar to controllist, this contains all controls placed on the form, but by mapping the control's pointer to the placementcontrol, making placement lookup from control faster.

        PlacementList selected; // A list of all selected controls when more are selected.

        std::map<DesignSizerSides, DesignSizer*> sizers;

        Brush *bgfill; // Brush for filling the background with the grid points.
        void RecreateBGFill(); // Deletes the bgfill brush if exists and creates it again.

        PlacementControl* FindPlacementControl(Control *control);
        PlacementControl* NextPlacementControl(PlacementControl *placement, bool allowoverflow); // Finds the placement control which comes right after the passed one when selecting with the tab key.
        PlacementControl* PrevPlacementControl(PlacementControl *placement, bool allowoverflow); // Finds the placement control which comes right before the passed one when selecting with the shift+tab key.
        PlacementControl* FindNextControlOnSide(PlacementControl *placement, DesignDirections dir);
        DesignDirections RelativeSide(Rect r, Rect r2);

        bool placing; // Control is being placed with the mouse.
        Point mousepos; // Position of the mouse in form client coordinates when the button is pressed.
        bool sizing; // Control is being sized. This happens when the placepos and the current position differs by some pixels before the mouse button is unpressed.
        Point sizepos; // Position of the mouse relative to the sized control when sizing starts.
    
        bool dragging; // Dragging in progress.
    
        enum SelectType { stNone, stSelect, stAdd, stDeselect, stToggle };
        SelectType seltype;
        Rect selrect;
        Rect lastselrect;

        bool cancelalt;

        DesignSizerSides sizeside; // Side of control being resized. 

        void DrawSelRect(const Rect &r);
        void HideSelRect();

        void DeleteSelected();
        void FilterSelection(); // Only keeps those controls selected which have selparent as their parent.
        PlacementControl* selparent; // The control whose children will be selected after the mouse button is released. Set to the control under the mouse when the selection operation started, unless it has no PlacementControl children.
        PlacementControl* LastSelected(); // The selected control that will be used as a base when we have to deselect elements that are not on the same parent as this one.
    
        void CreateControl(const type_info &ctype, PlacementControl* &parent); // Creates a control by type and places it on the parent. If the parent doesn't accept the control of the specified type, it is changed to the first placement control which accepts it.
        PlacementControl* CreateControl(const type_info &ctype/*, const std::wstring &preferredname*/); // Creates a control and sets it a preferred name or a default one if the preferred name is taken. It doesn't place the control anywyere. This function is called when pasting.
        void HijackChildrenProc(PlacementControl *parent); // Replaces the message handling procedure of the child controls of the passed parent to behave as intended in the designer.
        void HijackChildProc(PlacementControl *parent, HWND handle); // Replaces the message handling procedure of a child control with the given handle to force it to behave as intended in the designer.

        void CreateNonVisualControl(const type_info &ctype);
        void AddNonVisualControl(NonVisualControl *control/*, ControlTypes ctype*/);

        void SetCurrent(PlacementControl *placement); // Selects a control as the currently edited one.
        void SetCurrent(PlacementControl *placement, /*Object *maincontrol,*/ Rect area, Object *propowner, tagtype tag); // Selects a control as the currently edited one, also setting the area which should be shown as selected in control client coordinates, the propowner which has its properties edited and tag is user defined data.
        void SelectNext(bool backwards); // Selects the next control after pressing the tab key. Backwards is true if shift was pressed too and the previous item should be selected.

        void NotifySelectionChange(PlacementControl *pc, bool selected); // Calls DesignSelectChanged function for the control and all its container controls with the same arguments until one returns false.

        virtual void SetProperties(); // Fills the property editor with the properties of the currently selected control or controls.

        void ReplaceSubcontrolWndProc(HWND hwnd); // Subclass the given control by replacing its wndproc and 
        friend BOOL CALLBACK ChildEnumProc(HWND hwnd, LPARAM data);
        friend LRESULT CALLBACK SubcontrolReplacementProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        void PlaceSizers();
        void HideSizers();
        void InvalidateSizers();

        //PlacementControl* GetPlacementParent(PlacementControl *placement); // Selects the direct parent placement control of the passed one.
        PlacementControl* GetPlacementParent(Control *control); // Selects the direct parent placement control of the passed control, which is not necessarily stored in a PlacementControl.
        void DeletePlacementControl(PlacementControl *placement); // Removes the control and all its children from the form, deleting the controls.
        void RemovePlacementControl(PlacementControl *placement, bool deleteit); // Removes a child placement control from the form, deleting it if deleteit is set to true. Called from DeletePlacementControl to do the actual deleting.
        bool HasAsChild(PlacementControl *placement, PlacementControl *childcandidate); // Returns true if the placement control contains a given other control as a child.
        bool HasAsParent(PlacementControl *placement, PlacementControl *parentcandidate); // Returns true if the other placement control is the direct or indirect parent of the placement control.
        bool HasPlacementChildren(PlacementControl *placement); // Returns true if the placement control contains another placement control as children.
        bool HasSelectedChild(PlacementControl *placement); // Determines whether the given placement control has a selected control as a child. It doesn't work for the current control. Use HasPlacementChild for that.
        bool HasSelectedParent(PlacementControl *placement); // Determines whether the given placement control has a selected control as a parent. It doesn't work for the current control. Use HasPlacementChild for that.

        void StartSizing(DesignSizerSides side, Point clientpos);
        void SizeCurrentControl(int x1, int y1, int x2, int y2, bool showsizers = true);
        void SizeControl(PlacementControl *placement, int x1, int y1, int x2, int y2); // Resize or move passed control.
        HDWP DeferSizeControl(HDWP hdefer, PlacementControl *placement, int x1, int y1, int x2, int y2); // Deferred resize or move of the passed control. The control's alignment should be set to clNone and then reset after the size or move, or the control might be restored to its original position immediately.

        bool CancelAction(); // Cancel whatever we were doing, either when the form loses focus or when the user presses esc.
        void CloseMenus(); // Closes open menus, hiding the edited popup menu if needed.

        // Event handlers:
        void controlmessage(void *sender, MessageParameters param);
        void controlpaint(void *sender, PaintParameters param);
        void controlendsizemove(void *sender, SizePositionChangedParameters param);
        void formstartsizemove(void *sender, EventParameters param);
        void formendsizemove(void *sender, SizePositionChangedParameters param);

        void pmalignclick(void *sender, EventParameters param);

        /* Creates serialized string of the passed controls to be recreated in a copy/paste operation later.
         * The controls must be in their z-order, those that are placed on other controls must be included
         * and they must come right after their parent or previous sibling, or after the controls placed on
         * their previous sibling.
         * i.e. ctrl1[ ctrl2[ctrl3, ctrl4], ctrl5, ctrl6[ ctrl7 ] ], ctrl8[]
         */
        void Serialize(Indentation &indent, std::wiostream &stream, const std::vector<PlacementControl*> &list, std::vector<NonVisualControl*> &nvlist);
        void CollectControls(std::vector<PlacementControl*> &list, std::vector<NonVisualControl*> &nvlist, bool noguest, bool onlyselected); /* Creates a list of controls on the form in their logical order. Controls placed on other controls come immediately after their parent in their z-order. */

        // Creates the controls listed in 'items' and places on the parent control. The main item can be a 'form' type and the form will update its properties as well. In that case 'parent' must be NULL. No child 'form' is valid.
        bool ProcessDeserializedItems(PlacementControl *parent, std::vector<ControlDeserializerItem*> &items); // Returns false on unrecoverable error.

        /* Sorts the items in tablist in [tabbegin, tabend), taking the correct taborder from newtaborder which should have the same number of elements.
         * When the element in newtaborder is negative, it must come at the end of the tablist. The smaller negative numbers will have to come later in the list.
         * After TabOrderSort finishes we must make sure that:
         *  - all positive tabordered items come in ascending order starting with the tbegin position, and
         *  - all negative tabordered items come at the end of the tablist in descending order.
         */
        void TabOrderSort(PlacementList &tlist, PlacementIterator tbegin, PlacementIterator tend, std::vector<int> &newtaborder); 

        void pmenushow(void *sender, PopupMenuParameters param);
        void pmenuhide(void *sender, EventParameters param);

        void posbottomclick(void *sender, EventParameters param);
        void posdownclick(void *sender, EventParameters param);
        void posupclick(void *sender, EventParameters param);
        void posfrontclick(void *sender, EventParameters param);

        Size WindowSizeFromClient(const Size &s);

        void GetPlacePosition(PlacementControl *parent, PlacementControl *placed, Point pt, int &x, int &y); // Finds a position for the automatically placed control if it were to be placed at the given point on parent.

        friend class DesignContainerForm;
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void RemovingChildNotify(Control *parent, Control *child);
        virtual void NameChangeNotify(Object *obj, const std::wstring &oldname);

        virtual void NamesList(std::vector<std::wstring> &namelist); // Fills a vector with the names of the form's controls.

        virtual void EraseBackground();
        virtual void Paint(const Rect &updaterect);

        virtual void CaptureChanged();
        virtual void MouseEnter();
        virtual void MouseLeave();
        virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

        virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);
        virtual void KeyUp(WORD &keycode, VirtualKeyStateSet vkeys);

        virtual void WindowBoundsChanged(const Rect &oldrect, const Rect &newrect);

        virtual void Showing();
        virtual void Hiding();

        virtual bool ActiveDesigner();
        std::wstring BaseClass();

        virtual void ActiveFormChanging(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow);

        virtual ~DesignForm();

        void PlaceControl(const type_info &ctype, int x, int y);
        void PlaceControl(const type_info &ctype, int x1, int y1, int x2, int y2);

        // Changes the position of the given control in the z-order.
        void SendControlToBottom(PlacementControl *pc);
        void MoveControlDown(PlacementControl *pc);
        void MoveControlUp(PlacementControl *pc);
        void BringControlToFront(PlacementControl *pc);
        void MoveControlAbove(PlacementControl *what, PlacementControl *next);
        void MoveControlBelow(PlacementControl *what, PlacementControl *next);

        friend class DesignSizer;
    public:
        static void EnumerateProperties(DesignSerializer *serializer);
        DesignForm(const std::wstring &name, Form *owner, ButtonPanel *controlbuttons);
        virtual void Destroy();

        IconData* DesignFormIcon(); // Getter of the form icon structure for the form's icon property.
        void DesignSetFormIcon(IconData* newicon); // Updates the small and large icons of the form based on the form icon structure.
        Menubar* DesignGetMenu();
        void DesignSetMenu(Menubar *newmenu); // Use as a design equivalent for SetMenu(). It calls EditDesignMenu to start showing the menu in the editor.

        void DesignSetClientWidth(int newcwidth);
        int DesignClientHeight();
        void DesignSetClientHeight(int newcheight);
        Rect DesignClientRect();
        //void DesignSetClientRect(const Rect &newrect);

        DesignForm* DesignReturnSelf();

        virtual void Show();
        virtual void DeactivateSelection();
        virtual void ActivateSelection();

        virtual void CollectEvents(std::vector<EventListItem*> &events);
        virtual void HeaderExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, bool printpragma); // Serialize the window with all its controls into c++ header code. stream receives the class declarations.
        virtual void HeaderMemberList(Indentation &indent, std::wiostream &stream, AccessLevels access); // Print the declaration for the members on the form to the header file.
        virtual void CppMemberList(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents); // Print the member initialization list in the source file.
        virtual bool PrintShowAfterMemberList(); // Returns the value of DesignVisible(). 
        virtual void CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events); // Serialize the window with all its controls into c++ source code. stream receives the form constructor and destructor definitions.

        virtual void WriteToStream(Indentation &indent, std::wiostream &stream);
        virtual void ReadFromStream(TokenStream &token); // Initializes the form from a stream. Throws TokenE exception if the stream format was invalid.

        virtual Object* NameOwner(const std::wstring &name); // Returns the object that has the given name on the form.

        virtual void CollectObjects(std::vector<std::pair<std::wstring, Object*>> &objectstrings, bool (*collector)(Object*) = NULL, const std::wstring &objectname = std::wstring()); // Returns a vector of <object name, object> pairs placed on the designer form. The string for the object name is same as object->Name() for objects created by the form, and owner_form->Name()::name->Name() when not. When collector function is passed, call that function for every object and only include them in the list when it returns true. When objectname is set, only the object with that name should be added to the vector.

        virtual std::wstring GetGuestControls(); // Returns a comma separated list with the name of non-visual guest controls, with their type, owner form and control name.
        virtual void AddGuestControls(const std::wstring &guests); // Adds non-visual guest controls to the form, by their type, owner form and control name.

        virtual void GetTabActivatedControls(std::vector<std::pair<std::wstring, Control*>> &controlstrings, const std::wstring &controlname); // Returns a list of strings and controls which can be activated by the user by pressing the tab key or clicking on it. When controlname is set, only the control with that name is returned in the list.
        virtual std::pair<std::wstring, Object*> SelectedObject(); // Returns the currently selected object and its name, if only a single item is selected. The string for the object name is same as object->Name() for objects created by the form, and owner_form->Name()::name->Name() when not. If more than one items are selected, returns the number of selected items as a string and NULL for the object.
        virtual void SelectObject(std::wstring name); // Selects an object by name. The name follows the format returned by SelectedObject and CollectObjects. If name is empty, selects the form.
        virtual void SelectObject(Object *obj); // Selects an object. The object must be placed on the form. If the passed object is null, the form is selected.

        virtual void ClearSelection(Object *sender); // Deselects every control on the form.
        virtual void UpdateSelection(); // Checks whether the controls on the form should be shaded as more items are selected or just a single current should be used. Because the non visual control container form and the menus can also select items, it must update the state of the selection on the form.
        virtual void DeleteNotify(Object *object);
        virtual void ChangeNotify(Object *object, int changetype);
        virtual void RemoveNotify(Object *object); // Tell each control on the form, which might have the passed object as one of their member values, that the given object no longer belongs to the same container.

        // Copy selected or current placement control to the clipboard as serialized text.
        void CopySerialized();
        // Paste clipboard text as serialized controls onto the form or passed placement control parent.
        void PasteSerialized(PlacementControl *parent);

        // Called by controls at design time if they have some non-control area which can be edited in the property editor.
        // *control is the calling control itself, which must be an individual item placed on the form.
        // *area is the rectangle within the control's client area which will be shown as selected. It must be given in the control's client coordinates.
        // *propowner marks the object, which doesn't have to be a real control, which will have its properties shown in the property editor.
        // *tag is user defined data.
        void SelectDesignControl(Control *control, Rect area = Rect(), Object *propowner = NULL, tagtype tag = 0);
        // Returns true if the passed control is the only selected control on the form. The function fills the other arguments with values
        // passed in a previous call to SelectDesignControl in case the selection hasn't changed, otherwise those values are zeroed.
        bool GetSelectData(Object *maincontrol, Control* &control, Object* &propowner, tagtype &tag);
        bool IsControlSelected(Object *maincontrol, bool checksubcontrol); // Returns true if the control or its sub control is selected.
        // Returns true if one of the specified property owners (parts of a control having their own properties) are selected on a
        // main control. Set truecontrolpart to true if the function should only check real controls that have a separate main control.
        // Otherwise the function only checks non control parts selected with SelectDesignControl.
        bool IsPropertyOwnerSelected(Object *maincontrol, Object *propowner, bool truecontrolpart); 

        // Called by controls when they must create a control and place it on themselves so it can be selected for designing. i.e. pages of a PageControl. Set prop to the property managing the control when the control must be constructed by the parent.
        void PlaceControl(Control *control, Control *parent, DesignProperty *prop);

        void DisableSizing(); // Disables the sizing edges, redirecting mouse messages to the control below them.
        void EnableSizing(); // Enables the sizing edges, making them receive mouse messages again.

        // Changes the position of the given control in the z-order.
        void SendControlToBottom(Control *control);
        void MoveControlDown(Control *control);
        void MoveControlUp(Control *control);
        void BringControlToFront(Control *control);
        void MoveControlAbove(Control *what, Control *where); // Moves control in the z-order to be above next.
        void MoveControlBelow(Control *what, Control *where); // Moves control in the z-order to be above next.

        virtual std::wstring ControlName(Object *c, const std::wstring &name);

        using base::GuestReferenceOn;
        virtual bool GuestReferenceOn(DesignFormBase *guestparent, std::set<DesignFormBase*> &checkfinished); // Checks whether guestparent contains guest controls from this form or other forms, that would create a circular reference of guests between the forms. Set checkfinished to a set which contains all forms that have already been checked for circular references to exclude in further checks.

        void PlaceControl(const type_info &ctype);

        // TMP drag drop test of a string
        //void dodrop(void *sender, DragDropDropParameters param);
        // TMP end
    };

    class NonVisualControl;
    class FlatButton;
    // Form holding non-visual control placeholder panels, i.e. dialogs or image lists.
    class DesignContainerForm : public DesignFormBase
    {
    private:
        typedef DesignFormBase    base;

        class PlaceholderControl;
        typedef std::vector<PlaceholderControl*>            PlaceholderList;
        typedef std::vector<PlaceholderControl*>::iterator  PlaceholderIterator;

        class PlaceholderControl : public Panel
        {
        private:
            typedef Panel   base;
            DesignContainerForm *owner;
        
            NonVisualControl *control;
            bool selected;


            void buttonclick(void *sender, EventParameters param);
        protected:
            virtual void Paint(const Rect &updaterect);
            virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

            virtual ~PlaceholderControl();
        public:
            PlaceholderControl(DesignContainerForm *owner, NonVisualControl *control);

            FlatButton *button;

            NonVisualControl* GetControl();

            void Select();
            void Deselect();
            bool Selected();
        };

        DesignForm *designform; // A form when the container is not standalone.
        Container *container; // The true object that will be used when the edited program runs.
        ButtonPanel *controlbuttons; // Panel containing buttons for control placement. The form must be able to tell it to unpress a button once the control is placed.

        int pressed; // Index of the button currently pressed.
        bool open; // The whole form is visible with the controls contained on it.
        bool hovered; // The mouse is over the form while it is not open.

        PlaceholderList controls;
        PlaceholderList selected;
        PlaceholderControl* LastSelected();    

        int rows; // Number of rows where the non visual controls fit when the container is stand alone.
    
        PlaceholderIterator ControlPosition(NonVisualControl *control); // Returns the position where the control should be inserted taken its name.
        void PositionControls();

        int ControlIndex(NonVisualControl *control); // Returns the index of a given control in the controls list.
        int ControlIndex(PlaceholderControl *pc);

        void notifyunpress(void *sender, EventParameters param);

        void UpdateSize(bool shrink); // Sets the form size, border style etc. depending on its open state.
        void SetOpen(bool opening);

        void Serialize(Indentation &indent, std::wiostream &stream, std::vector<NonVisualControl*> &nvlist);
        void CollectControls(std::vector<NonVisualControl*> &nvlist, bool noguest, bool onlyselected); /* Adds controls to the nvlist which correspond to the arguments.  */
        // Copy selected or current placement control to the clipboard as serialized text.
        void CopySerialized();
        // Paste clipboard text as serialized controls onto the form or passed placement control parent.
        void PasteSerialized();
        // Creates the controls listed in 'items' and places on the parent control. The main item can be a 'form' type and the form will update its properties as well. In that case 'parent' must be NULL. No child 'form' is valid.
        void ProcessDeserializedItems(std::vector<ControlDeserializerItem*> &items); // Returns false on unrecoverable error.

        void InvalidateControl(PlaceholderControl *pc);

        void DeletePlaceholder(NonVisualControl *nvc); // Called when removing a non-visual control from this container.

        friend class DesignForm;
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void Paint(const Rect &updaterect);

        void GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide);

        //virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        //virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

        virtual void MouseEnter();
        virtual void MouseLeave();

        virtual void DeleteNotify(Object *object);
        virtual void NameChangeNotify(Object *object, const std::wstring &oldname);
        virtual void RemoveNotify(Object *object); // Tell each control on the form which might have the passed object as one of their member values, that the given object no longer belongs to the same container.

        virtual void NamesList(std::vector<std::wstring> &namelist); // Fills a vector with the names of the form's controls.

        virtual bool ActiveDesigner();
        std::wstring BaseClass();

        virtual void ActiveFormChanging(bool activated, bool mouseactivate, Form *otherform, HWND otherwindow);

        virtual void Resizing();

        virtual ~DesignContainerForm();
    public:
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual void Destroy();

        virtual void Modify(bool tomodified = true);

        DesignContainerForm(const std::wstring &name, DesignForm *owner, ButtonPanel *controlbuttons); // Constructor creating a designform's container, which holds the non visual controls of a form.
        DesignContainerForm(const std::wstring &name, Form *owner, ButtonPanel *controlbuttons); // Constructor creating a normal container.
        virtual void Show();

        virtual void DeactivateSelection();
        virtual void ActivateSelection();

        virtual void SetProperties(); // Fills the property editor with the properties of the currently selected control or controls, when the container is standalone without a designform.

        bool PlaceControl(const type_info &ctype); // Places a new non visual control on the container.
        PlaceholderControl* CreateControl(const type_info &ctype/*, const std::wstring &preferredname*/); // Creates a non visual control with this as its container. Set preferredname when restoring previous state of control which had a name.
        void AddControl(NonVisualControl *control); // Adds an existing non visual control to the container.

        virtual Object* NameOwner(const std::wstring &name); // Returns the object that has the given name on the form or the parent designer if specified.

        virtual void CollectObjects(std::vector<std::pair<std::wstring, Object*>> &objectstrings, bool (*collector)(Object*) = NULL, const std::wstring &objectname = std::wstring()); // Returns a vector of <object name, object> pairs placed on the designer form. The string for the object name is same as object->Name() for objects created by the form, and owner_form->Name()::name->Name() when not. When collector function is passed, call that function for every object and only include them in the list when it returns true. When objectname is set, only the object with that name should be added to the vector.

        virtual std::wstring GetGuestControls(); // Returns a comma separated list with the name of non-visual guest controls, with their type, owner form and control name.
        virtual void AddGuestControls(const std::wstring &guests); // Adds non-visual guest controls to the form, by their type, owner form and control name.

        virtual void GetTabActivatedControls(std::vector<std::pair<std::wstring, Control*> > &controlstrings, const std::wstring &controlname); // Returns a list of strings and controls which can be activated by the user by pressing the tab key or clicking on it. When controlname is set, only the control with that name is returned in the list. Does nothing in this class.
        virtual std::pair<std::wstring, Object*> SelectedObject(); // Returns the currently selected object and its name, if only a single item is selected. The string for the object name is same as object->Name() for objects created by the form, and owner_form->Name()::name->Name() when not. If more than one items are selected, returns the number of selected items as a string and NULL for the object.
        virtual void SelectObject(std::wstring name); // Selects an object by name. The name follows the format returned by SelectedObject and CollectObjects. If name is empty, select the form.
        virtual void SelectObject(Object *obj); // Selects an object. The object must be placed on the form, and it cannot be a sub-object. If the passed object is null, the form is selected.

        DesignFormBase* FormObj(); // Returns either a DesignForm if the object belongs to one, or itself otherwise.

        //virtual void CollectEvents(std::vector<std::pair<void*, std::wstring>> &events);
        virtual void HeaderExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, bool printpragma); // Serialize the window with all its controls into c++ header code. stream receives the class declarations.
        virtual void HeaderMemberList(Indentation &indent, std::wiostream &stream, AccessLevels access); // Print the declaration for the members on the form to the header file.
        virtual void CppMemberList(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents); // Print the member initialization list in the source file.
        virtual bool PrintShowAfterMemberList(); // Always returns false.
        virtual void CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events); // Serialize the window with all its controls into c++ source code. The stream receives the form constructor and destructor definitions.
        void CppExport(Indentation &indent, std::wiostream &stream, const std::vector<EventListItem*> &events, std::wstringstream &sdelayed); // Serialize the window with all its controls into c++ source code. The streams receive the form constructor and destructor definitions, stream is for normal constructor lines, sdelayed is for those values that must be restored at the end of the constructor.

        virtual void WriteToStream(Indentation &indent, std::wiostream &stream);
        virtual void ReadFromStream(TokenStream &token); // Initializes the container from a stream. Throws TokenE exception if the stream format was invalid.

        void PressButton(PlaceholderControl *control);
        void PressButton(int index);
        void UnpressButton(PlaceholderControl *control);
        void UnpressButton();

        int Count();
        NonVisualControl* Controls(int ix);
        int SelCount();
        NonVisualControl *SelControls(int ix);
        virtual void ClearSelection(Object *sender);
        void DeleteSelected();
        void SetSelected(int index); // Selects an object deselecting everything else.
        void SetSelected(PlaceholderControl *pc); // Selects an object deselecting everything else.
        void Select(PlaceholderControl *pc); // Selects an object if it is not already selected, not changing the selection otherwise.
        void Deselect(PlaceholderControl *pc); // Selects an object if it is not already selected, not changing the selection otherwise.
        void SelectLastSelected();
        bool Open();

        virtual std::wstring ControlName(Object *c, const std::wstring &name);

        virtual bool GuestReferenceOn(DesignFormBase *guestparent, std::set<DesignFormBase*> &checkfinished); // Checks whether guestparent contains guest controls from this form or other forms, that would create a circular reference of guests between the forms. Set checkfinished to a set which contains all forms that have already been checked for circular references to exclude in further checks.

        bool SelectNext(bool backwards, bool allowoverflow); // Selects the control which comes after sellast, or before, if backwards is true. Set allowoverflow to true if after the last item, the first control should be selected, or after the first one the last, when backwards is true.
    };


}
/* End of NLIBNS */


#pragma once

#ifdef DESIGNING
#include "designregistry.h"

// Predefined change types
#define CHANGE_SHOW         0xA000
#define CHANGE_TAB          0xA001
#define CHANGE_MENU         0xA002 // Passed to ChangeNotify when a property which changes the appearance of a menu item has changed.
#define CHANGE_TOPMENU      0xA003
#define CHANGE_PARENT       0xA004 // Passed to ChangeNotify after a parent is added to or removed from a non-visual control's parent list.

namespace NLIBNS
{
    class DesignSerializer;
    class DesignFormBase;
    class DesignForm;
    struct menu_item_data;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum MouseButtons : int;
    enum VirtualKeyStates : int;
#else
    enum MouseButtons;
    enum VirtualKeyStates;
#endif /* __MINGW32__ */

    typedef uintset<VirtualKeyStates> VirtualKeyStateSet;
}
/* End of NLIBNS */
#endif /* DESIGNING */


namespace NLIBNS
{

    typedef int tagtype;
    class Form;

#define CHANGE_MOVED        0x0001 ///< Passed to ChangeNotify() when a control is moved.

    /**
     * \defgroup enumsets Enumeration Sets
     *
     * \brief Enumerations with values of distinct bits and their sets.
     * 
     * Enumeration sets are used in classes to save space and group the boolean values of the same kind of behavior or state in a single variable.
     * \sa uintset
     * @{
     */

    /// \defgroup objectstates Object States
    ///
    /// \brief The current state of an object.
    ///
    /// Shows whether the object is in a notification loop when notifying other objects of a
    /// change, or it is about to be deleted after a call to Object::Destroy(). Don't call
    /// Object::Changed() on objects that have a state set.
    /// \sa Object, Object::ObjectState()
    /// @{
    /**
     * Possible states of an Object.
     */
    enum ObjectStates {
        osDestroying = 0x0001, ///< The object is being destroyed. Calling Object::Destroy() when this state is set does nothing. Objects can react to others being destroyed by overriding Object::DeleteNotify().
        osNotifying = 0x0002, ///< The object is in a notification loop to notify other objects of a change, when a member function called Object::Changed(). Objects can react to others being changed by overriding Object::ChangeNotify().
    };
    /**
     * Set of ObjectStates.
     */
    typedef uintset<ObjectStates> ObjectStateSet;
    /// @}

    /** @} */

#ifdef DESIGNING
    enum AccessLevels { alPublic = 0, alProtected = 1, alPrivate = 2, 

            alevCount = 3
    };
#endif

    /** \addtogroup enumsets
     * @{
     */

    /// \defgroup notifyrelations Notify Relations
    ///
    /// \brief Indicates the relation between two objects that are in each other's notify list.
    ///
    /// Objects are put into each other's notify list if some change in one of them can cause
    /// the other object to change as well. In most cases the relation between the two objects must
    /// be clear. The most common relations are between a parent and its child, a form and its
    /// active control, a control and its sub control. 
    /// \sa Object, ObjectStates, Object::AddToNotifyList(), Object::RemoveFromNotifyList(), Object::InNotifyList(), Object::DeleteNotify(), Object::ChangeNotify()
    /// @{
    /**
     * The relation between two objects which are in each other's notify list.
     */
    enum NotifyRelations : unsigned int {
        nrNoReason     = 0, ///< Only used to remove or find an object in the notify list of another, when the result doesn't depend on the relation between them.

        nrOwnership    = 0x00000001, ///< Relation between a Form or Container and an Object placed on it.
        nrActivation   = 0x00000002, ///< Relation between the active Control and its parent Form, or between a Label and a Control it activates.
        nrSubControl   = 0x00000004, ///< Relation between the a main and its sub control. (I.e. Tab on a TabControl, Menubar of a Form, PopupMenu and a Control.)
        // Custom reasons that can be used in derived classes.
        nrRelation1      = 0x00001000, ///< Custom relation that can be freely used by projects.
        nrRelation2      = 0x00002000, ///< Custom relation that can be freely used by projects.
        nrRelation3      = 0x00004000, ///< Custom relation that can be freely used by projects.
        nrRelation4      = 0x00008000, ///< Custom relation that can be freely used by projects.
        nrRelation5      = 0x00010000, ///< Custom relation that can be freely used by projects.
        nrRelation6      = 0x00020000, ///< Custom relation that can be freely used by projects.
        nrRelation7      = 0x00040000, ///< Custom relation that can be freely used by projects.
        nrRelation8      = 0x00080000, ///< Custom relation that can be freely used by projects.
        nrRelation9      = 0x00100000, ///< Custom relation that can be freely used by projects.
        nrRelation10     = 0x00200000, ///< Custom relation that can be freely used by projects.
        nrRelation11     = 0x00400000, ///< Custom relation that can be freely used by projects.
        nrRelation12     = 0x00800000, ///< Custom relation that can be freely used by projects.
        nrRelation13     = 0x01000000, ///< Custom relation that can be freely used by projects.
        nrRelation14     = 0x02000000, ///< Custom relation that can be freely used by projects.
        nrRelation15     = 0x04000000, ///< Custom relation that can be freely used by projects.
        nrRelation16     = 0x08000000, ///< Custom relation that can be freely used by projects.
        nrRelation17     = 0x10000000, ///< Custom relation that can be freely used by projects.
        nrRelation18     = 0x20000000, ///< Custom relation that can be freely used by projects.
        nrRelation19     = 0x40000000, ///< Custom relation that can be freely used by projects.
        nrRelation20     = 0x80000000, ///< Custom relation that can be freely used by projects.
    };
    /**
     * Set of NotifyRelations.
     */
    typedef uintset<NotifyRelations> NotifyRelationSet;
    /// @}

    /** @} */

    /// Base class for most types in the library.
    /**
     * Objects can notify each other of changes and are automatically destroyed by their top-level parent,
     * when the parent is being destroyed. 
     * Call Destroy() to delete the object, because the destructor is protected in derived classes just as well.
     * \sa NonVisualControl, Control, object_deleter
     */
    class Object
    {
    private:
#ifdef DESIGNING
        std::wstring name; // Name of object as given during designing.

        AccessLevels access;
    
        bool designing; // The object is placed on a form in the designer.
#endif
        ObjectStateSet objectstate;

        std::list<std::pair<Object*, NotifyRelationSet>> notifylist; // List of objects that must be notified when this one is changed or freed.
        std::list<std::pair<Object*, NotifyRelationSet>>::iterator deleteiterator;
        std::list<std::pair<Object*, NotifyRelationSet>>::iterator changeiterator;
        tagtype tag; // custom user data

        void EraseFromNotify(std::list<std::pair<Object*, NotifyRelationSet>>::iterator it, NotifyRelations relation); // Removing object from the notify list without invalidating the delete and change iterators if a delete or change notification is in progress.
        void AddToNotify(Object *object, NotifyRelations relation); // Adding object to the notify list without invalidating the delete and change iterators if a delete or change notification is in progress.

    protected:
#ifdef DESIGNING
        //Object(ControlTypes type);

        void NameChanged(const std::wstring &oldname);
        virtual void NameChangeNotify(Object *object, const std::wstring& oldname) {}
#endif
        Object(); ///< Constructor for objects.
        Object(const Object &orig); ///< Copy constructor that does nothing.

        void Changed(int changetype = 0); ///< Calls ChangeNotify() for all objects added to the notify list.
        virtual void DeleteNotify(Object *object); ///< Called by another object in the notify list when it is being deleted with Destroy().
        virtual void ChangeNotify(Object *object, int changetype); ///< Called when another object in the notify list calls its Changed() method after being changed.
        bool AddToNotifyList(Object *object, NotifyRelations relation); ///< The passed and the called Object mutually add each other to their notify list with the passed relation.
        void RemoveFromNotifyList(Object *object, NotifyRelations relation); ///< Removes the passed object and this one from each other's notify list with the given relation.
        bool InNotifyList(Object *object, NotifyRelations relation); ///< Returns whether the passed object is in the notify list with the specified relation.

        virtual void TagChanged(tagtype oldtag); ///< Called when the user changes the tag with the SetTag() method.

        const ObjectStateSet& ObjectState(); ///< \copydoc objectstates
        virtual ~Object(); ///< The destructor.
    public:
        virtual void Destroy(); ///< Deletes the object and notifies all objects wich are in the notify list of its destruction.

#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual DesignFormBase* DesignParent(); // Returns the form used in the designer which holds this object, the object itself if it is the designer form or NULL if the object is not being designed.
        virtual std::wstring ClassName(bool namespacedname);
        const type_info& TypeInfo();

        const std::wstring& Name() const;
        //virtual Object* NameOwner(const std::wstring &name); // Returns the object or an object held by this object if it is named or holds a subobject with the passed name. Override in derived classes that override the Names(...) function as well.
        //virtual void Names(std::vector<std::wstring> &namelist) {} // Fills the list with names of the object's child objects. Override this if the object contains other objects that must be named, when creating a listing in a cpp header file. (Don't include child controls, just extra non-control object names, i.e. tabs in a tab control.)
        virtual void SetName(const std::wstring& newname);
        virtual Object* SubOwner(); // Sub-objects should implement this function by returning an object which is responsible for registering them on the designer form. Returns null by default.
        virtual bool SubShown(); // Returns true for objects that are directly selectable on the designer form. Objects that have a name and are editable only via another object's property editor should return false.
   
        virtual Object* PropertyOwner(); // Either the control itself, or an object which provides properties in the property editor for this control.
        virtual Object* MainControl(); // Usually NULL, but when the control is just part of another and it cannot exist alone, this will return that control. (i.e. the PageControl of TabPages.)

        DesignSerializer* Serializer();

        AccessLevels AccessLevel();
        void SetAccessLevel(AccessLevels newacc);

        virtual void InitDesignerMenu(Point clientpos, std::vector< menu_item_data > &inserteditems) {} // Called before a popup menu is to be shown above the control in the designer. If the control needs to add menu items specific to the control, it should override this function, saving the mouse position to know where the popup menu was shown.
        virtual bool NeedDesignerHittest(int x, int y, LRESULT hittest) { return false; } // Called when the mouse is moved over a control in the designer, and it wants to process mouse messages at the given point. Be aware, that processing mouse messages mean, that the control cannot be dragged and moved when the mouse is pressed at the specified location.

        virtual bool DesignSelectChanged(Object *control, bool selected) { return true; } // Called by the designer form when a control (or rather its property holder) is deselected/selected and the sizers are moved. The function is called for both the control that lost the selection and the one being selected, and then their container controls, until one returns false. Show/hide or move parts to reflect the selection, but don't manipulate the designer in any way to avoid infinite loops.

        // Mouse and keyboard input handlers for design time. These are called before the normal handlers if the object
        // has a control placed on a DesignForm, to separate designer and normal input handling. The functions return
        // whether they handled all input. If the return value is true, the normal mouse functions won't be called
        // and the mouse message won't be forwarded to the default handler.
        virtual bool DesignMouseMove(DesignForm *form, short x, short y, VirtualKeyStateSet vkeys) { return false; }
        virtual bool DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys) { return false; }
        virtual bool DesignMouseUp(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys) { return false; }

        virtual bool DesignNCMouseMove(DesignForm *form, short x, short y, LRESULT hittest, VirtualKeyStateSet vkeys) { return false; }
        virtual bool DesignNCMouseDown(DesignForm *form, short x, short y, MouseButtons button, LRESULT hittest, VirtualKeyStateSet vkeys) { return false; }
        virtual bool DesignNCMouseUp(DesignForm *form, short x, short y, MouseButtons button, LRESULT hittest, VirtualKeyStateSet vkeys) { return false; }

        virtual bool DesignKeyPush(DesignForm *form, WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys) { return false; } // Returns whether the designer form should stop processing the key message.
        virtual void DesignKeyUp(DesignForm *form, WORD &keycode, VirtualKeyStateSet vkeys) {}

        virtual bool DesignTabNext(bool entering, bool backwards) { return false; } // Called when switching between designed controls with the tab character. Entering is true if the control is to become active. Backwards specifies whether the control should be selected with Shift-Tab. The function should return true, if it selected a sub element and doesn't want the form to process tabbing further.

        bool Designing() const; // The control is placed on a designer form and can be modified in the designer directly.
        void SetDesigning(); // Initialize control for designing on a designer form.

        virtual void DesignSubSelected(Object *sub) {} // Called by the designer form if this is an owner control of a subcontrol which has been selected in the designer.
#endif
        virtual Form* ParentForm() const; ///< Returns the Form which is responsible for the object's deletion.

        tagtype Tag() const; ///< A value set by SetTag() which can be used for anything.
        void SetTag(tagtype newtag); ///< Sets a value which can be retrieved with Tag().
    };

    /// A simple type with the only purpose to call Object::Destroy() for Object values. Pass to std::unique_ptr<> or other STL classes that need a way to destroy objects.
    struct object_deleter
    {
        /// Constructor that does nothing.
        object_deleter() {}
        /// Copy constructor that does nothing.
        object_deleter(const object_deleter&) {}
        /// Called by STL classes when an Object must be deleted.
        void operator()(Object *obj)
        {
            obj->Destroy();
        }
    };

#ifdef DESIGNING
    struct NonVisualSubItem
    {
        std::wstring propname;
        std::list<NonVisualSubItem> subitems;
        Object *item;

        NonVisualSubItem(const std::wstring& propname, Object *item);
        NonVisualSubItem(NonVisualSubItem &&orig);
    };
#endif

    class Container;

    /// Class of objects that are not controls, but can have a parent form to destroy them.
    /**
     * Types from NonVisualControl are usually created for a single purpose. Despite its name, a NonVisualControl can
     * have a visual representation in a Control, if it stores some data that can be displayed by it.
     */
    class NonVisualControl : public Object
    {
    private:
        typedef Object base;

        Object *parent; // The window which holds this control as a member and which also has to delete it.
#ifdef DESIGNING
        std::vector<DesignFormBase*> parents; // A list of forms which list this non-visual control as a member.
#endif

    protected:
        virtual ~NonVisualControl(); ///< \copydoc Object::~Object()

#ifdef DESIGNING
        friend class DesignContainerForm; // For calling removal notification.
#else
#endif
        NonVisualControl(); ///< Constructor.
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual void CollectSubItems(std::list<NonVisualSubItem> &subitems) {}

        virtual void AddParent(DesignFormBase *newparent); // Add a new parent to the list of parents. The same non-visual control can be placed on several forms.
        virtual void RemoveParent(DesignFormBase *oldparent); // Remove a parent from the list of parents. The same non-visual control can be placed on several forms.
        int ParentCount(); // Number of parents this non-visual control is placed on. The same non-visual control can be placed on several forms.
        DesignFormBase* Parents(int ix); // Returns a parent by index that this non-visual control is placed on. The same non-visual control can be placed on several forms.
        bool IsParent(DesignFormBase *form); // Returns whether the passed form was added with AddParent.
#endif

        virtual void Destroy();
        virtual Form* ParentForm() const; ///< \copybrief Object::ParentForm()
        virtual Container* ParentContainer(); ///< Returns the Container which is responsible for the object's deletion.
        void SetParent(Object *newparent); ///< Sets a parent which will be responsible for destroying this object.
    };

    /// Class of objects to be the parent of NonVisualControl objects.
    /**
     * The only purpose of Container is to destroy the NonVisualControl objects which are its children,
     * when the Container itself is destroyed.
     */
    class Container : public Object
    {
    private:
        typedef Object base;

        std::vector<NonVisualControl*> nvs; // Non visual children that must be deleted when the container is destroyed.
    protected:
        virtual ~Container(); ///< \copydoc Object::~Object()

        virtual void DeleteNotify(Object *object);
    public:
        Container(); ///< Constructor.

        virtual void Destroy();

        void AddNVChild(NonVisualControl *nv); ///< Gives ownership of the passed NonVisualControl to the container.
        void RemoveNVChild(NonVisualControl *nv); ///< Removes ownership of the passed NonVisualControl from the container.
    };


}
/* End of NLIBNS */


#pragma once

#include "events.h"
#include "zlib/zlib.h"
#include "objectbase.h"


namespace NLIBNS
{

    class Indentation;

    class DesignProperties;
    class DesignSerializer;

    class Form;
    class DesignForm;
    class DesignFormBase;
    class Object;
    class Control;
    class DesignProperty;
    class PopupMenu;

    template<typename T>
    T* ReturnAsPointer(T* val)
    {
        return val;
    }

    template<typename T>
    T* ReturnAsPointer(T& val)
    {
        return &val;
    }

    template<typename T>
    T ReturnAsValue(T* val)
    {
        return *val;
    }

    template<typename T>
    T ReturnAsValue(T& val)
    {
        return val;
    }

    template<typename T>
    struct ValuePair : public std::pair<T, std::wstring>
    {
    private:
        typedef std::pair<T, std::wstring> base;
    public:
        ValuePair(T val, const std::wstring &name) : base(val, name) {}
        ValuePair() : base() {}
        ValuePair(ValuePair &&other)
        {
            *this = std::move(other);
        }
        ValuePair& operator=(ValuePair &&other)
        {
            first = std::move(other.first);
            second = std::move(other.second);
            return *this;
        }
    };

    template<typename T>
    ValuePair<T> make_ValuePair(T val, const std::wstring &name) { return ValuePair<T>(val, name); }

#define VALUEPAIR(val)   make_ValuePair(val, L ## #val)

    // Casting to incomplete type DesignForm is only possible from outside if we can't include their source files here.
    DesignForm* CastToDesignForm(Object *obj);
    DesignForm* CastToDesignForm(Form *form);
    DesignFormBase* CastToDesignFormBase(Object *obj);
    DesignFormBase* CastToDesignFormBase(Form *form);
    const std::wstring& SerializerFormName(Form *form); // Returns the name of a form.
    // These don't do anything but call the passed form's function of the same name.
    void CollectObjects(DesignFormBase *form, std::vector<std::pair<std::wstring, Object*>> &objectstrings, bool (*collector)(Object*) = NULL, const std::wstring &objectname = std::wstring());
    bool NameTaken(DesignFormBase *form, const std::wstring &val);
    bool FormNameTaken(DesignFormBase *form, const std::wstring &val);
    void GetTabActivatedControls(DesignForm *form, std::vector<std::pair<std::wstring, Control*>> &controlstrings, const std::wstring &controlname);
    bool DesignFormIsObjectBase(DesignFormBase *form, Object *obj);
    bool DesignFormIsForm(DesignFormBase *dform, Form *form);
    bool DesignFormIsForm(DesignForm *dform, Form *form);
    std::wstring DesignFormEventFunction(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &name);
    void* DesignFormEvent(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &name);
    void DesignFormSetEventFunction(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &name, const std::wstring &newvalue);
    int DesignFormEventCountById(DesignFormBase *form, const std::wstring &eventtype);
    std::wstring DesignFormEventFunctionByIndex(DesignFormBase *form, const std::wstring &eventtype, int index);
    int DesignFormEventFunctionIndex(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &eventfunction);
    void DesignFormEditPopupMenu(DesignFormBase *form, PopupMenu *menu);

    DesignProperty* SerializerFind(DesignSerializer *serializer, const std::wstring& pname);
    DesignProperty* SerializerProperties(DesignSerializer *serializer, int index);

    enum DesignPropertyUsages { dpuListed = 0x01, dpuSerialized = 0x02, dpuExported = 0x04, dpuDerived = 0x08, dpuHidden = 0x10 };
    typedef uintset<DesignPropertyUsages> DesignPropertyUsageSet;
    /* 
     * dpuListed: The property is listed in the property list.
     * dpuSerialized: The property is serialized to the project file, unless it has the default write value.
     * dpuExported: The property is exported to the cpp file, unless it has the default write value.
     * dpuDerived: The property is not listed, serialized or exported, but some other property or control references it and must be found. This value can't be mixed with the others.
     * dpuHidden: The property is not listed, nor serialized nor exported. This property shouldn't be found with a Find or PropertyIndex call. This value can't be mixed with the others.
     */    

    int SerializerPropertiesIndex(DesignSerializer *serializer, int index, DesignPropertyUsageSet condition);
    int SerializerPropertyCount(DesignSerializer *serializer, DesignPropertyUsageSet condition);
    int SerializerPropertyIndex(DesignSerializer *serializer, DesignProperty *prop);
    const std::wstring& SerializerNames(DesignSerializer *serializer, int index);

    int DesignerNextMemberIndex(std::wstring typestr);
    int DesignerExportToResource(std::vector<byte> &res);

    enum PropertyStyles : int {
                psEditShared        = 0x0001, /* Show the property when multiple controls are selected which all have it. This is the default. */
                psImmediateUpdate   = 0x0002, /* Calls SetValue every time the edit box contents change. */
                psReadonly          = 0x0004, /* The property is shown on the property editor but it can't be edited from the edit box. (There might be other means.) */ 
                psThumbImage        = 0x0008, /* The property provides a small thumb image to show in the property editor. */
                psDrawItem          = 0x0010, /* The property measures and draws its own list items when showing the list in the property editor. */
                psCheckbox          = 0x0020, /* A checkbox is used to switch between two states of the property. Usually True / False.*/
                psEditButton        = 0x0040, /* A button is shown for editing the property and ClickEdit is called when that happens. */
                psDelayedRestore    = 0x0080, /* The property is restored at the end of the de-serialization and constructors. */
                psInnerBinary       = 0x0100, /* The BinaryValue() and SetBinaryValue() functions are used to retrieve and set the property's data in a char array. InnerValue() must encoded that to a hexa string when serializing. */
                psBinaryResource    = 0x0200, /* The property stores data in a resource file via the ResourceValue() function, to be able to initialize in the constructor of the exported cpp program. See comment for ResourceValue(). */
                psGuestEditable     = 0x0400, /* Allows editing the property when the non-visual control it belongs to is placed on a second form as guest. Set as default. */
    };

    // ParentCreationRefType enum: used with DesignProperty::SetParentCreation() to determine how a sub-item or a child control will be referred to after creation in the form controls initialization function.
    enum ParentCreationReferenceType { pcrtFormDeclare, pcrtLocalDeclare, pcrtNoDeclare };
    // * pcrtFormDeclare:  the form holds a pointer to the item and it is declared in the form's header file. There is no need for explicit
    //                     declaration in the form initializer function. I.e.:
    //                     page1 = PageControl1->AddTabPage(L"Page title text");
    // * pcrtLocalDeclare: there is no declaration for the item in the form, so it must be declared in the control initialization function. It must exist
    //                     to be able to set its properties via its object. I.e.:
    //                     Tab *tab1 = TabControl1->AddTab(L"Tab title text");
    //                     tab1->SetText(L"Tab Text");
    //                     WARNING: The item MUST have a name, or the output will be incorrect. If the name cannot be edited by the user,
    //                              it must be constructed in a way that it does not conflict with existing names. For example the item
    //                              can override the Name() function and generate an index for itself checking existing names, and getting
    //                              a new one with NameNext(). The item can be in a vector, in which case that must be taken into account.
    // * pcrtNoDeclare:    There is no need to declare the item, because its initialization is done directly when it is constructed. I.e.:
    //                     StatusBar1->AddPart(L"part name", 60, nlib::sbpbLowered, false);

    typedef uintset<PropertyStyles> PropertyStyleSet;
    typedef std::wstring (*SerializeArgsFunc)(Object *obj);

    // Abstract base class for all properties the designer can show in its property list.
    class DesignProperty
    {
    private:
        std::wstring name;
        std::wstring category;
        int priority;

        //bool pointervalue;

        DesignPropertyUsageSet usage;

        //bool hidden; // If a property is hidden, it is not listed in the property list of the form editor.

        // Only used when the property refers to a real control on the form or a vector containing such controls.
        bool parentcreate; // When set to true, the control is constructed by its parent (i.e. [ControlType *]control = parent->createfuncname()) and the SetParent() line is not added during serialization. False by default.
        // Values used when parentcreate is true:

        ParentCreationReferenceType reftype; // When set to true, the construction of the control will have to contain the "ControlType *" part, as it is not declared in the header separately.

        SerializeArgsFunc argsfunc;
        std::wstring ConstructSerializeArgs(Object *obj);

        DesignProperties *owner;
        void SetOwner(DesignProperties *newowner);

        friend class DesignProperties;
    protected:
        PropertyStyleSet propertystyle;

        typedef void (*SerializerFunc)(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum);
        SerializerFunc sfunc;

        virtual bool Indexed(); // Returns true if the property has several values that can be reached through indexes.
        virtual int Index();
        virtual void ChangeIndex(int newindex);
    public:
        DesignProperty(const std::wstring &name, const std::wstring &category);
        DesignProperty();
        virtual ~DesignProperty();

        const std::wstring& Category();
        const std::wstring& Name();

        void SetParentCreation(ParentCreationReferenceType areftype, SerializeArgsFunc argsfunc = NULL); // Only valid for properties of controls with sub-items or child controls that must be created by the owner of the property. Set areftype to the method the sub-item or control will be referred as in the form control initializer function.
        void SetParentCreationReferenceType(ParentCreationReferenceType areftype); // Changes how to refer to a sub-item or child in the form's control initializer function. Similar to calling SetParentCreation(), but does not change the argument listing function.
        virtual bool ParentCreated(); // Returns true when SetParentCreation was called.
        ParentCreationReferenceType ReferenceType(); // Should the control represented by the property be declared in the header file.
        void ConstructExport(Indentation &indent, std::wiostream &stream, Object *parent, Object *control, std::wstring &printedname); // Called by the serializer when ParentCreated(), to print a line that creates an object or adds an element to a vector in the cpp constructor. If a control or object must be declared locally in the initialization function, printedname will receive the name used for the declaration.

        void SetSerializerFunc(SerializerFunc sfunc); // Sets a function that prints custom creation lines to the cpp constructor when the property represents a member of some parent.
        bool HasSerializerFunc();
        void CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum); // Calls the set serializer function to serialize the property to the passed stream.

        virtual void DeclaredNames(std::vector<std::pair<std::wstring, std::wstring>> &names, Object *propholder, AccessLevels access); // Lists members and their types represented by the property for the generated header.
        //virtual void DeclaredName(std::vector<std::pair<std::wstring, std::wstring>> &names, Object *propholder, Object *control, AccessLevels access); // Lists members and their types represented by the property for the generated header in vectors.

        virtual bool HasDefault() = 0; // Should return in derived classes whether the property has a default value or default checker function set, which can be used to restore the value of the property to the default for a control.
        virtual bool DefaultsEqual(Object *propholder, Object *otherpropholder, DesignProperty *otherprop) = 0; // Checks whether the default value of this and another property is set and are the same.
        virtual void Reset(Object *propholder) = 0; // Resets the value of the property in the control to its default.

        void MakeDefault(); // Sets this property as the default for a control in its serializer.

        void DontList();
        void DontWrite();
        void DontSerialize();
        void DontExport();
        void DontDerive();
        void Hide();

        void DoList();
        void DoWrite();
        void DoSerialize();
        void DoExport();
        void Derive();

        //bool Hidden(); // The property is hidden. That is, it is either replaced by another one with the same name in a derived class, or some derived class itself manages that value.
        //virtual void Show(); // Restores a property's visible state.

        virtual bool IsListed();
        virtual bool IsSerialized();
        virtual bool IsExported();
        virtual bool IsDerived();
        virtual bool IsWritten();

        //virtual bool SerializeHidden() = 0; // Whether the property must be serialized even when it is hidden. (GeneralDesignProperty gives the full functionality.)
        //virtual bool InnerSerializeHidden() = 0; // Whether the property must be serialized for inner use even when it is hidden. (GeneralDesignProperty gives the full functionality.)

        virtual bool Delayed(); // Specifies if the property is restored at the end of the de-serialization and constructors.
        void Delay(); // Forces the property being restored at the end of de-serialization among the other delayed properties.

        int Priority(); // A number that represents the position of the property in the list of properties. If it is the same as others', the order is decided by the name of the properties.
        void SetPriority(int newpriority); // The property will appear above properties with lower priority and below those with higher priority. Default is 0.
        void SetImmediateUpdate(bool immediate); // Sets whether psImmediateUpdate is included in propertystyle.
        void DisableSharedEdit(); // Disallow the property to show up when multiple controls are selected.

        virtual const type_info& CreatorType() = 0; // Type information about the class providing the property. Because a class can only provide a property with a given name once, this info must be checked when adding properties.
        virtual const type_info& ValueType() = 0; // Type information of the property.

        virtual bool IsPointerValue(); // True if the value of this class or vector elem property is serialized in the cpp constructor with a function returning a pointer type. If for example an object is returned by reference, its members must be serialized with the . operator and not ->.
        //void SetAsPointerValue(bool newpvalue); // Sets whether the value of this class or vector elem property is serialized in the cpp constructor with a function returning a pointer type. If for example an object is returned by reference, its members must be serialized with the . operator and not ->.

        bool HasPropertyStyle(PropertyStyles style); // Called by the designer to check the property's capabilities. Derived properties should change the propertystyle set in their constructor.

        virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index); // Draw thumbnail image for property. When index is INT_MAX the property's current value should be drawn, otherwise the list item at the given index. Only called, when psThumbImage is in the property styles. Doesn't do anything by default.
        virtual void MeasureListItem(Object *propholder, MeasureItemParameters param); // Called when measuring items in the property editor's list. Only used when psDrawItem is in the styles and ListCount() returns a non-null positive value.
        virtual void DrawListItem(Object *propholder, DrawItemParameters param); // Called when drawing items in the property editor's list. Only used when psDrawItem is in the styles and ListCount() returns a non-null positive value.

        virtual std::wstring Value(Object *propholder); // Return the string representation of the current property value.
        virtual std::wstring InnerValue(Object *propholder); // Returns the string representation of a property used when saving to the project file or in a copy/paste operation. Calls Value() by default unless the property has the psInnerBinary specified, in which case returns a hex encoded string of the result of BinaryValue().
        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val); // Called when the user tries to change the value to a new string representation, when psReadonly is not in the styles. Not called when the user clicked an item in the drop-down list for properties with a list of items, instead SelectValue is called (SelectListValue in derived classes with list type). The function must return true when the string is successfully converted and false when not. parentform must be set to the form to be the parent for dialogs that might come up. parentform can be used as a parent for dialog windows.
        virtual void SetInnerValue(Form *parentform, Object *propholder, const std::wstring& val); // Called when deserializing a property. It calls SetValue by default, but overrides should be provided in all sub properties which provide their own InnerValue function. If psInnerBinary is specified in the property style, tries to uncode the string from hex format and calls SetBinaryValue().
        virtual void SetInnerValue(Form *parentform, Object *propholder, int index, const std::wstring& val); // Called when deserializing an array property. Does nothing in any other case.
        virtual bool IsDefault(Object *propholder) = 0; // Indicates whether the current value is the default.
        virtual void BinaryValue(Object *propholder, std::vector<byte> &result); // Used when psInnerBinary is set in the property style. Returns the value of the property in a byte array in the passed vector. This version does nothing.
        virtual void SetBinaryValue(Form *parentform, Object *propholder, std::vector<byte> &val); // Sets the inner value of the property when psInnerBinary is specified in the property style. This version does nothing.

        virtual void ResourceValue(Object *propholder, std::vector<byte> &result); // Sets data in the result vector that will be exported to a resource file as RCDATA for properties having the psBinaryResource property. If psInnerBinary is also set, the default behavior calls BinaryValue(). Overwrite if psInnerBinary is not specified or a different behavior is needed.
        virtual bool StoresBinaryResource(Object *propholder); // Returns true if psBinaryResource is set in the propertystyle.

        // These must be specified when a property shows a drop down list for possible values.
        virtual int ListCount(Object *propholder); // Number of items in the property's listbox for predefined valid values. Returns 0 when no listbox should be shown.
        virtual std::wstring ListItem(Object *propholder, int index); // The indexth value in the list when a number of predefined values are valid.
        virtual void* ListValue(Object *propholder, int index); // The value at the indexth position when there are items in the list of this property.
        virtual int Selected(Object *propholder); // Currently selected value in the list of valid values.
        virtual bool SelectValue(Form *parentform, Object *propholder, void *val); // Called when the user clicks an item in the drop-down list, for properties where the ListCount is not 0. SetValue is not called in those cases. The default implementation calls ListCount and ListValue as many times as needed till the given value is found. Then it calls ListItem and SetValue. Overload this function for more efficient implementation.

        virtual bool ClickEdit(Form *parentform, Object *propholder); // Called when the user either double-clicks the property's line or clicks on the thumb image or clicks the editor button if present for the property. parentform can be used as a parent for dialog windows.

        virtual int SubCount(Object *propholder); // Number of subitems in the property editor. 0 by default.
        virtual DesignProperty* SubProperty(Object *propholder, int index); // Properties belonging to some class or vector when SubCount is not 0.
        //virtual Object* SubItem(Object *propholder, int index); // Returns the item of a vector or other list that has items which cannot be shown in a list as they might have properties themselves.
        virtual void AddItem(Object *propholder); // Adds a new item in the list of subproperties when the item is a vector whose count can change. Does nothing by default.
        virtual void AddItem(Object *propholder, const std::wstring &str); // Adds a new item in the list of subproperties, when the item is a vector whose count can change. This version is for vectors holding native data types, that can set their data from the given string right away. Does nothing by default.

        virtual Object* SubHolder(Object *propholder); // Returns the object which manages the property, when a property is a separate class. (i.e. Font) Otherwise returns the main object that owns the property.

        // Functions used in serialization.
        virtual std::wstring ExportValue(Object *propholder); // Value of property in generated cpp code. Returns same as Value in the default implementation.
        virtual std::wstring ResourceExportValue(Object *propholder, int resourceid); // Same as ExportValue, but called when psBinaryResource is in the property's style. Returns "MAKEINTRESOURCE([resourceid value])" by default.
    };

    // Interface class as a base for classes held by properties to get and set the attributes of objects through their getter and setter functions.
    // Passing the object which holds the property as propholder to CallGetter and CallSetter will update the object's attribute (a variable or some more complex state).
    template<class PropertyType>
    class IPropertyReader
    {
    public:
        virtual PropertyType CallGetter(Object *propholder) = 0;
        virtual void CallSetter(Object *propholder, PropertyType newvalue) = 0;
        virtual IPropertyReader<PropertyType>* Clone() const = 0;
        virtual ~IPropertyReader() {}
    };

    // Used as a direct getter and setter class of object properties. It calls a given function to get and another to set a value. The property can be read only if only a getter function is specified in its constructor.
    template<class PropertyHolder, typename PropertyType,
            typename GetterPropertyType = PropertyType, class GetterPropertyHolder = PropertyHolder, typename GetterProc = GetterPropertyType (GetterPropertyHolder::*)(), 
            typename SetterPropertyType = GetterPropertyType, class SetterPropertyHolder = PropertyHolder, typename SetterProc = void (SetterPropertyHolder::*)(SetterPropertyType)>
    class PropertyReader : public IPropertyReader<PropertyType>
    {
    private:
        typedef IPropertyReader<PropertyType> base;
        typedef PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterProc, SetterPropertyType, SetterPropertyHolder, SetterProc>    selftype;
        typedef PropertyHolder holdertype;
    
        GetterProc getter;
        SetterProc setter;
    public:
        PropertyReader(GetterProc getter, SetterProc setter) : getter(getter), setter(setter) {}
        PropertyReader(GetterProc getter) : getter(getter), setter(NULL) { }

        virtual PropertyType CallGetter(Object *propholder)
        {
            return ((dynamic_cast<GetterPropertyHolder*>(propholder))->*getter)();
        }

        virtual void CallSetter(Object *propholder, PropertyType newvalue)
        {
            if (!setter)
                throw L"This is a read only property!";
            ((dynamic_cast<SetterPropertyHolder*>(propholder))->*setter)(newvalue);
        }

        virtual IPropertyReader<PropertyType>* Clone() const
        {
            return new selftype(getter, setter);
        }
    };

    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder>* CreatePropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(), int nullp)
    {
        return new PropertyReader<PropertyHolder, PropertyType, GetterPropertyType>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)() const>* CreatePropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)() const, int nullp)
    {
        return new PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)() const>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder>* CreatePropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(), std::nullptr_t null)
    {
        return new PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)() const>* CreatePropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)() const, std::nullptr_t null)
    {
        return new PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)() const>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder, typename SetterPropertyType, typename SetterPropertyHolder>
    PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(), SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(SetterPropertyType)>* CreatePropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(), void (SetterPropertyHolder::*setter)(SetterPropertyType))
    {
        return new PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(), SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(SetterPropertyType)>(getter, setter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder, typename SetterPropertyType, typename SetterPropertyHolder>
    PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)() const, SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(SetterPropertyType)>* CreatePropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)() const, void (SetterPropertyHolder::*setter)(SetterPropertyType))
    {
        return new PropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)() const, SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(SetterPropertyType)>(getter, setter);
    }


    // Used as a getter and setter class for object properties that must be accessed with an index.
    template<typename OwnerProperty, typename PropertyType, typename GetterPropertyType = PropertyType, typename SetterPropertyType = GetterPropertyType>
    class SubPropertyReader : public IPropertyReader<PropertyType>
    {
    protected:
        typedef GetterPropertyType(OwnerProperty::*Getter)(Object*, int);
        typedef void (OwnerProperty::*Setter)(Object*, int, SetterPropertyType);
    private:
        typedef IPropertyReader<PropertyType> base;
        typedef SubPropertyReader<OwnerProperty, PropertyType, GetterPropertyType, SetterPropertyType>  selftype;
    
        OwnerProperty* parent;
        int index;
        Getter getter;
        Setter setter;

    public:
        SubPropertyReader(OwnerProperty *parent, int index, Getter getter, Setter setter) : parent(parent), index(index), getter(getter), setter(setter) { }
        SubPropertyReader(OwnerProperty *parent, int index, Getter getter) : parent(parent), index(index), getter(getter), setter(NULL) { }

        virtual PropertyType CallGetter(Object *propholder)
        {
            return ((dynamic_cast<OwnerProperty*>(parent))->*getter)(propholder, index);
        }

        virtual void CallSetter(Object *propholder, PropertyType newproperty)
        {
            if (!setter)
                throw L"This is a read only property!";
            ((dynamic_cast<OwnerProperty*>(parent))->*setter)(propholder, index, newproperty);
        }

        virtual IPropertyReader<PropertyType>* Clone() const
        {
            return new selftype(parent, index, getter, setter);
        }
    };


    // First template base class for all properties. It defines some functions that were abstract in DesignProperty but not all. In most cases the properties
    // can be accessed via a pointer of this type, as the derived types are usually not returned.
    template<typename PropertyHolder, typename PropertyType>
    class GeneralDesignProperty : public DesignProperty
    {
    public:
        typedef IPropertyReader<PropertyType>    readertype;
    protected:
        typedef typename std::remove_pointer<typename std::remove_reference<PropertyType>::type>::type* PropertyTypePtr;
        typedef typename std::remove_const<typename std::remove_reference<PropertyType>::type>::type PropertyTypeNoConstRef;
        typedef const PropertyTypeNoConstRef& PropertyTypeConstRef;
        typedef PropertyTypePtr const PropertyTypePtrConst;
        typedef PropertyType (PropertyHolder::*Getter)();
        typedef void (PropertyHolder::*Setter)(PropertyType);
        typedef PropertyTypeConstRef (PropertyHolder::*ConstGetter)();
        typedef void (PropertyHolder::*ConstSetter)(PropertyTypeConstRef);
        typedef PropertyTypePtrConst (PropertyHolder::*PtrConstGetter)();
    private:
        readertype *reader;
    
        typedef DesignProperty base;
        typedef GeneralDesignProperty<PropertyHolder, PropertyType>    selftype;

        /* Values indicating how to show and serialize a property.
         * defaultshow: Determines whether the property has the default value or shown in bold instead in the property list. If a property has the default value, it is not serialized or written to the cpp file, unless set otherwise.
         *      pdsNoDefault - The value has no default set, and it is never shown in bold.
         *      pdsValue - When the property has the same value as the saved defaultshowvalue variable, the property is shown as default.
         *      pdsFunction - The defaultshowgetter function pointer is set and the function returns the same as the current value.
         * defaultwrite: Determines if a value is serialized or written to the cpp files.
         *      pdwAsShown - Serialize the value if it is not hidden or its show value is not shown as the default.
         *      pdwNoDefault - The value is serialized even if it is shown as the default. 
         *      pdwValue - If the value in defaultwritevalue is the same as the property's value, it is not serialized nor written to the cpp file.
         *      pdwFunction - The defaultwritechecker function returns true if the property should be serialized or written to the cpp file.
         */ 
        typedef PropertyType (PropertyHolder::*DefaultGetter)();
        typedef bool (PropertyHolder::*DefaultWriteChecker)() const;
        enum PropertyDefaultShow { pdsNoDefault, pdsValue, pdsFunction };
        PropertyDefaultShow defaultshow;
        PropertyTypeNoConstRef defaultshowvalue;
        DefaultGetter defaultshowgetter;

        enum PropertyDefaultWrite { pdwAsShown, pdwNoDefault, pdwValue, pdwFunction };
        PropertyDefaultWrite defaultwrite;
        PropertyTypeNoConstRef defaultwritevalue;
        DefaultWriteChecker defaultwritechecker;

        using base::SetPriority;
        using base::SetSerializerFunc;
    protected:
        GeneralDesignProperty(const std::wstring &name, const std::wstring &category, readertype *reader) : base(name, category), reader(reader),
                defaultshow(pdsNoDefault), defaultwrite(pdwAsShown) { }

        DesignForm* GetDesignForm(Object *propholder)
        {
            DesignForm *f = CastToDesignForm(propholder);
            if (!f)
                f = CastToDesignForm(propholder->SubOwner() && !propholder->ParentForm() ? propholder->SubOwner()->ParentForm() : propholder->ParentForm());
            return f;
        }

        DesignFormBase* GetDesignFormBase(Object *propholder)
        {
            DesignFormBase *f = CastToDesignFormBase(propholder);
            if (!f)
                f = CastToDesignFormBase(propholder->SubOwner() && !propholder->ParentForm() ? propholder->SubOwner()->ParentForm() : propholder->ParentForm());
            return f;
        }

        void SetReader(readertype *areader)
        {
            reader = areader;
        }

        virtual void _SetDefault(const PropertyType &newdefault)
        {
            defaultshow = pdsValue;
            defaultshowvalue = newdefault;
        }

        virtual void _SetDefault(DefaultGetter getterproc)
        {
            defaultshow = pdsFunction;
            defaultshowgetter = getterproc;
        }


        virtual void _SetDefaultWrite(const PropertyType &newdefault)
        {
            defaultwrite = pdwValue;
            defaultwritevalue = newdefault;
        }

        virtual void _SetDefaultWrite(DefaultWriteChecker checkerproc)
        {
            defaultwrite = pdwFunction;
            defaultwritechecker = checkerproc;
        }

        PropertyDefaultShow DefaultShow()
        {
            return defaultshow;
        }

        PropertyDefaultWrite DefaultWrite()
        {
            return defaultwrite;
        }

    public:
        virtual ~GeneralDesignProperty()
        {
            delete reader;
        }

        virtual PropertyType CallGetter(Object *propholder)
        {
            if (!reader)
                throw L"No reader in class!";
            return reader->CallGetter(propholder);
        }

        PropertyTypePtr CallGetterAsPointer(Object *propholder)
        {
            if (!reader)
                throw L"No reader in class!";
            return ReturnAsPointer(reader->CallGetter(propholder));
        }

        virtual void CallSetter(Object *propholder, PropertyType newvalue)
        {
            if (!reader)
                throw L"No reader in class!";
            reader->CallSetter(propholder, newvalue);
        }

        const readertype& Reader()
        {
            return *reader;
        }

        virtual const type_info& CreatorType()
        {
            return typeid(PropertyHolder);
        }

        virtual const type_info& ValueType()
        {
            return typeid(PropertyType);
        }

        //selftype* SetAsPointerValue(bool newpvalue)
        //{
        //    base::SetAsPointerValue(newpvalue);
        //    return this;
        //}

        // The real value of the property, not its string representation (as opposed to Value()).
        PropertyType GetPropertyValue(Object *propholder)
        {
            return CallGetter(propholder);
        }

        PropertyTypePtr GetPropertyValueAsPointer(Object *propholder)
        {
            return CallGetterAsPointer(propholder);
        }

        template<typename T>
        selftype* SetDefault(T val)
        {
            _SetDefault(val);
            return this;
        }

        //template<>
        selftype* SetDefault/*<std::nullptr_t>*/(std::nullptr_t val)
        {
            _SetDefault((PropertyType)NULL);
            return this;
        }

        PropertyTypeNoConstRef GetDefault(Object *propholder)
        {
            if (defaultshow == pdsValue)
                return defaultshowvalue;
            if (defaultshow == pdsFunction)
                return ((dynamic_cast<PropertyHolder*>(propholder))->*defaultshowgetter)();

            return PropertyTypeNoConstRef();
        }

        virtual bool HasDefault()
        {
            if (defaultshow == pdsValue || defaultshow == pdsFunction)
                return true; 
            else
                return false;
        }

        virtual bool DefaultsEqual(Object *propholder, Object *otherpropholder, DesignProperty *otherprop)
        {
            selftype *prop = dynamic_cast<selftype*>(otherprop);
            if (!prop || !HasDefault() || !prop->HasDefault())
                return false;
            return GetDefault(propholder) == prop->GetDefault(otherpropholder);
        }

        virtual void Reset(Object *propholder)
        {
            PropertyTypeNoConstRef def = GetDefault(propholder);
            CallSetter(propholder, def);
        }

        template<typename T>
        selftype* SetDefaultWrite(T val)
        {
            _SetDefaultWrite(val);
            return this;
        }

        selftype* SetDefaultWrite(std::nullptr_t val)
        {
            _SetDefaultWrite((PropertyType)nullptr);
            return this;
        }


        selftype* SetPriority(int newpriority)
        {
            base::SetPriority(newpriority);
            return this;
        }

        selftype* SetSerializerFunc(typename base::SerializerFunc sfunc)
        {
            base::SetSerializerFunc(sfunc);
            return this;
        }

        selftype* SetImmediateUpdate(bool immediate)
        {
            base::SetImmediateUpdate(immediate);
            return this;
        }
    
        selftype* DisableSharedEdit()
        {
            base::DisableSharedEdit();
            return this;
        }

        virtual bool IsDefault(Object *propholder)
        {
            switch(defaultshow)
            {
            case pdsNoDefault:
                return true;
            case pdsValue:
                return CallGetter(propholder) == defaultshowvalue;
            case pdsFunction:
                return CallGetter(propholder) == ((dynamic_cast<PropertyHolder*>(propholder))->*defaultshowgetter)();
            default:
                return false;
            }
        }

        virtual bool IsDefaultWrite(Object *propholder)
        { 
            switch(defaultwrite)
            {
            case pdwAsShown:
                return defaultshow != pdsNoDefault && IsDefault(propholder);
            case pdwValue:
                return CallGetter(propholder) == defaultwritevalue;
            case pdwFunction:
                return ((dynamic_cast<PropertyHolder*>(propholder))->*defaultwritechecker)();
            default: // pdwNoDefault
                return false;
            }
        }

        virtual bool IsDefaultExport(Object *propholder)
        {
            return IsDefaultWrite(propholder);
        }

        // Determines whether the property should be written to the cpp file.
        virtual bool MustExport(Object *propholder)
        {
            return IsExported() && !IsDefaultExport(propholder);
        }

        // Determines whether the property is used when writing to the project file or copy/pasting.
        virtual bool MustSerialize(Object *propholder)
        {
            return IsSerialized() && !IsDefaultWrite(propholder);
        }

        selftype* DontList()
        {
            base::DontList();
            return this;
        }

        selftype* DontWrite()
        {
            base::DontWrite();
            return this;
        }

        selftype* DontSerialize()
        {
            base::DontSerialize();
            return this;
        }

        selftype* DontExport()
        {
            base::DontExport();
            return this;
        }

        selftype* DontDerive()
        {
            base::DontDerive();
            return this;
        }

        selftype* Hide()
        {
            base::Hide();
            return this;
        }

        selftype* DoList()
        {
            base::DoList();
            return this;
        }

        selftype* MakeDefault()
        {
            base::MakeDefault();
            return this;
        }

        selftype* DoWrite()
        {
            base::DoWrite();
            return this;
        }

        selftype* DoSerialize()
        {
            base::DoSerialize();
            return this;
        }

        selftype* DoExport()
        {
            base::DoExport();
            return this;
        }

        selftype* Derive()
        {
            base::Derive();
            return this;
        }

    };

    template<typename PropertyHolder>
    class GeneralStringDesignProperty : public GeneralDesignProperty<PropertyHolder, std::wstring>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, std::wstring> base;
    public:
        GeneralStringDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {
        }

        virtual std::wstring Value(Object *propholder)
        {
            return this->CallGetter(propholder);
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                this->CallSetter(propholder, val);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }

        virtual std::wstring InnerValue(Object *propholder)
        {
            return EscapeCString(Value(propholder));
        }

        virtual std::wstring ExportValue(Object *propholder)
        {
            return L"L\"" + EscapeCString(Value(propholder)) + L"\"";
        }
    };

    template<typename PropertyHolder>
    class StringDesignProperty : public GeneralStringDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralStringDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        StringDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
        {}
    };

    template<typename PropertyHolder>
    class GeneralVariableStringDesignProperty : public GeneralStringDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralStringDesignProperty<PropertyHolder>    base;
    protected:
        virtual bool InvalidValue(Object *propholder, const std::wstring &val)
        {
            return !ValidVarName(val);
        }
    public:
        GeneralVariableStringDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {
            this->propertystyle -= psImmediateUpdate;
        }

        virtual std::wstring Value(Object *propholder)
        {
            return this->CallGetter(propholder);
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            DesignFormBase *f = this->GetDesignFormBase(propholder);
            if (InvalidValue(propholder, val))
                return false;
            base::SetValue(parentform, propholder, val);
            return true;
        }
    };

    template<typename PropertyHolder>
    class VariableStringDesignProperty : public GeneralVariableStringDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralVariableStringDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        VariableStringDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
        {}
    };

    template<typename PropertyHolder>
    class GeneralFileNameDesignProperty : public GeneralStringDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralStringDesignProperty<PropertyHolder> base;
        bool allowpath;
    public:
        GeneralFileNameDesignProperty(const std::wstring &name, const std::wstring &category, bool allowpath, typename base::readertype *reader) : base(name, category, reader), allowpath(allowpath)
        {}
    
        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            if (val.length() && !ValidFileName(val, allowpath))
                return false;
            return base::SetValue(parentform, propholder, allowpath ? ShortenRelativePath(val) : val);
        }
    };

    template<typename PropertyHolder>
    class FileNameDesignProperty : public GeneralFileNameDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralFileNameDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        FileNameDesignProperty(const std::wstring &name, const std::wstring &category, bool allowpath, GetterProc getter, SetterProc setter) : base(name, category, allowpath, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
        {}
    };

    template<typename PropertyHolder>
    class GeneralFilePathDesignProperty : public GeneralStringDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralStringDesignProperty<PropertyHolder> base;
    public:
        GeneralFilePathDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {}
    
        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            if (val.length() && !ValidFilePath(val))
                return false;
            return base::SetValue(parentform, propholder, ShortenRelativePath(val));
        }
    };

    template<typename PropertyHolder>
    class FilePathDesignProperty : public GeneralFilePathDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralFilePathDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        FilePathDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, std::wstring>(getter, setter))
        {}
    };

    template<typename PropertyHolder, typename inttype>
    class GeneralIntDesignProperty : public GeneralDesignProperty<PropertyHolder, inttype>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, inttype> base;
    public:
        GeneralIntDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {}

        virtual std::wstring Value(Object *propholder)
        {
            //std::wstringstream str;
            //str << this->CallGetter(propholder);
            //return str.str();

            return IntToStr(this->CallGetter(propholder));
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                inttype i;
                if (!StrToInt(val, i))
                    return false;
                this->CallSetter(propholder, i);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }
    };

    template<typename PropertyHolder>
    class IntDesignProperty : public GeneralIntDesignProperty<PropertyHolder, int>
    {
    private:
        typedef GeneralIntDesignProperty<PropertyHolder, int>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        IntDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter = nullptr) : base(name, category, CreatePropertyReader<PropertyHolder, int>(getter, setter))
        {}
    };

    template<typename PropertyHolder>
    class UnsignedIntDesignProperty : public GeneralIntDesignProperty<PropertyHolder, unsigned int>
    {
    private:
        typedef GeneralIntDesignProperty<PropertyHolder, unsigned int>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        UnsignedIntDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, unsigned int>(getter, setter))
        {}
    };

    template<typename PropertyHolder>
    class WordDesignProperty : public GeneralIntDesignProperty<PropertyHolder, WORD>
    {
    private:
        typedef GeneralIntDesignProperty<PropertyHolder, WORD>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        WordDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, WORD>(getter, setter))
        {}
    };

    template<typename PropertyHolder>
    class GeneralFloatDesignProperty : public GeneralDesignProperty<PropertyHolder, float>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, float>  base;
    public:
        GeneralFloatDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {
        
        }

        virtual std::wstring Value(Object *propholder)
        {
            //std::wstringstream str;
            //str << this->CallGetter(propholder);
            //return str.str();

            return FloatToStr(this->CallGetter(propholder));
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                double f;
                if (!StrToFloat(val, f))
                    return false;
                this->CallSetter(propholder, f);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }
    };

    template<typename PropertyHolder>
    class FloatDesignProperty : public GeneralFloatDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralFloatDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        FloatDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, float>(getter, setter))
        {}
    };


    template<typename PropertyHolder>
    class GeneralBoolDesignProperty : public GeneralDesignProperty<PropertyHolder, bool>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, bool> base;
    protected:
        const bool trueval;
        const bool falseval;
    public:
        GeneralBoolDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader),
                trueval(true), falseval(false)
        {
            this->propertystyle << psCheckbox;
        }

        virtual std::wstring Value(Object *propholder)
        {
            bool val = this->CallGetter(propholder);
            return val ? L"True" : L"False";
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                bool bval = (val.size() == 1 && val[0] == L'1') || GenToLower(val) == L"true";
                this->CallSetter(propholder, bval);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }

        virtual void* ListValue(Object *propholder, int index)
        {
            return (void*)&(index == 0 ? trueval : falseval);
        }

        virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
        {
            return SetValue(parentform, propholder, (*(bool*)val) ? L"True" : L"False");
        }

        virtual int ListCount(Object *propholder)
        {
            return 2;
        }

        virtual std::wstring ListItem(Object *propholder, int index)
        {
            if (index < 0 || index >= 2)
                throw L"Index out of bounds";
            return index == 0 ? L"True" : L"False";
        }

        virtual int Selected(Object *propholder)
        {
            bool val = this->CallGetter(propholder);
            if (val)
                return 0;
            return 1;
        }

        virtual std::wstring ExportValue(Object *propholder)
        {
            return this->CallGetter(propholder) ? L"true" : L"false";
        }
    };

    template<typename PropertyHolder>
    class BoolDesignProperty : public GeneralBoolDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralBoolDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        BoolDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, bool>(getter, setter))
        {
            if (!setter)
                this->propertystyle << psReadonly;
        }
    };

    // Base property for listing values, i.e. strings of an enum, font families.
    template<typename PropertyHolder, typename PropertyType, typename ListType = PropertyType >
    class GeneralListDesignProperty : public GeneralDesignProperty<PropertyHolder, PropertyType>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, PropertyType> base;

        const ValuePair<ListType> *strings; // Array of strings corresponding to each list value.
        int stringcount;
        std::wstring namespc;
        bool ownstrings;

        virtual void* ListValue(Object *propholder, int index)
        {
            return (void*)&ListItemValue(propholder, index);
        }

        virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
        {
            return SelectItemValue(parentform, propholder, *(ListType*)val);
        }
    public:
        // Constructors using a ListType string array.
        GeneralListDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, const ValuePair<ListType> strings[], int stringcount) : base(name, category, reader),
                strings(strings), stringcount(stringcount), ownstrings(false)
        {
            if (EnumStringsRegistered(strings))
                namespc = EnumStringsNamespace(strings) + L"::";
        }

        // Constructors using a string vector where the value is the same string.
        GeneralListDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, const std::vector<std::wstring> &stringvec) : base(name, category, reader),
                stringcount(stringvec.size()), ownstrings(true)
        {
            ValuePair<ListType> *ownstrings = new ValuePair<ListType>[stringcount];
            for (auto it = stringvec.begin(); it != stringvec.end(); ++it)
                ownstrings[it - stringvec.begin()] = ValuePair<ListType>(*it, *it);
            strings = ownstrings;
        }

        virtual ~GeneralListDesignProperty()
        {
            if (ownstrings)
                delete[] strings;
        }

        virtual std::wstring Value(Object *propholder)
        {
            PropertyType val = this->CallGetter(propholder);
            for (int ix = 0; ix < stringcount; ix++)
                if (val == strings[ix].first)
                    return strings[ix].second;

            return L"";
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                std::wstring str = GenToLower(val);
                for (int ix = 0; ix < stringcount; ix++)
                {
                    if (GenToLower(strings[ix].second) == str)
                    {
                        this->CallSetter(propholder, strings[ix].first);
                        break;
                    }
                }
            }
            catch(...)
            {
                return false;
            }
            return true;
        }

        virtual int ListCount(Object *propholder)
        {
            return stringcount;
        }

        virtual std::wstring ListItem(Object *propholder, int index)
        {
            return strings[index].second;
        }

        virtual const ListType& ListItemValue(Object *propholder, int index)
        {
            return strings[index].first;
        }

        virtual int Selected(Object *propholder)
        {
            PropertyType val = this->CallGetter(propholder);
            for (int ix = 0; ix < stringcount; ix++)
                if (strings[ix].first == val)
                    return ix;
            return -1;
        }

        virtual bool SelectItemValue(Form *parentform, Object *propholder, ListType val)
        {
            int ix = 0;
            for ( ; ix < stringcount; ++ix)
                if (strings[ix].first == val)
                    return SetValue(parentform, propholder, strings[ix].second);
        
            return false;
        }

        virtual std::wstring ExportValue(Object *propholder)
        {
            PropertyType val = this->CallGetter(propholder);

            for (int ix = 0; ix < stringcount; ix++)
            {
                if (val == strings[ix].first)
                    return namespc + strings[ix].second;
            }

            return L""; 
        }

    };

    template<typename PropertyHolder, typename PropertyType>
    class GeneralEnumDesignProperty : public GeneralListDesignProperty<PropertyHolder, PropertyType>
    {
    private:
        typedef GeneralListDesignProperty<PropertyHolder, PropertyType> base;
    public:
        GeneralEnumDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, const ValuePair<PropertyType> strings[]) : base(name, category, reader, strings, EnumStringsCount(strings))
        {}
    };

    template<typename PropertyHolder, typename PropertyType>
    class EnumDesignProperty : public GeneralEnumDesignProperty<PropertyHolder, PropertyType>
    {
    private:
        typedef GeneralEnumDesignProperty<PropertyHolder, PropertyType> base;
    public:
        template<typename GetterProc, typename SetterProc>
        EnumDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter, const ValuePair<PropertyType> strings[]) : base(name, category, CreatePropertyReader<PropertyHolder, PropertyType>(getter, setter), strings)
        {}
    };

    template<typename PropertyHolder, typename PropertyType>
    class GeneralValueListDesignProperty : public GeneralDesignProperty<PropertyHolder, PropertyType>
    {
    public:
        typedef int (PropertyHolder::*CountGetter)() const;
        typedef std::wstring (PropertyHolder::*StringGetter)(int index);
        typedef PropertyType (PropertyHolder::*ValueGetter)(int index);

        virtual void* ListValue(Object *propholder, int index)
        {
            return (void*)index; //(void*)&ListItemValue(propholder, index);
        }

        virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
        {
            return SelectItemValue(parentform, propholder, (int)val);
        }
    private:
        typedef GeneralDesignProperty<PropertyHolder, PropertyType> base;
        typedef GeneralValueListDesignProperty<PropertyHolder, PropertyType>    selftype;

        typedef GeneralDesignProperty<PropertyHolder, PropertyType> ValueProperty;

        ValueProperty *valueproperty;

        CountGetter countgetter;
        StringGetter stringgetter;
        ValueGetter valuegetter;

        PropertyType returnval; // Dummy value for returning an address when requested.
    public:
        GeneralValueListDesignProperty(const std::wstring &name, const std::wstring &category, CountGetter countgetter, StringGetter stringgetter, ValueGetter valuegetter, ValueProperty *valueproperty) : base(name, category, valueproperty->Reader().Clone() ),
                valueproperty(valueproperty), countgetter(countgetter), stringgetter(stringgetter), valuegetter(valuegetter)
        {
        }

        virtual ~GeneralValueListDesignProperty()
        {
            delete valueproperty;
        }

        virtual std::wstring Value(Object *propholder)
        {
            return valueproperty->Value(propholder);
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            if (!valueproperty->SetValue(parentform, propholder, val))
            {
                int cnt = ListCount(propholder);
                for (int ix = 0; ix < cnt; ++ix)
                    if (ListItem(propholder, ix) == val)
                        return SelectItemValue(parentform, propholder, ix);
            }
            return false;
        }

        virtual int ListCount(Object *propholder)
        {
            return ((dynamic_cast<PropertyHolder*>(propholder))->*countgetter)();
        }

        virtual std::wstring ListItem(Object *propholder, int index)
        {
            return ((dynamic_cast<PropertyHolder*>(propholder))->*stringgetter)(index);
        }

        virtual const PropertyType& ListItemValue(Object *propholder, int index)
        {
            returnval = ((dynamic_cast<PropertyHolder*>(propholder))->*valuegetter)(index);
            return returnval;
        }

        virtual int Selected(Object *propholder)
        {
            int cnt = ListCount(propholder);
            PropertyType val = this->CallGetter(propholder);
            for (int ix = 0; ix < cnt; ix++)
            {
                if (ListItemValue(propholder, ix) == val)
                    return ix;
            }
            return -1;
        }

        virtual bool SelectItemValue(Form *parentform, Object *propholder, int index)
        {
            //int cnt = ListCount(propholder);
            //for (int ix = 0 ; ix < cnt; ++ix)
            //    if (ListItemValue(propholder, ix) == val)
            //        return SetValue(parentform, propholder, strings[ix].second);
            this->CallSetter(propholder, ListItemValue(propholder, index) );
        
            return this->CallGetter(propholder) == ListItemValue(propholder, index);
        }
    };

    // Property lister for rectangles.
    class CollectionDesignPropertyElem
    {
    private:
        DesignProperty *prop;
    public:
        CollectionDesignPropertyElem(DesignProperty *prop) : prop(prop) { }
        virtual ~CollectionDesignPropertyElem()
        {
            delete prop;
        }

        virtual DesignProperty* Property()
        {
            return prop;
        }
    };

    template<typename PropertyHolder, typename PropertyType>
    class GeneralCollectionDesignProperty : public GeneralDesignProperty<PropertyHolder, PropertyType>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, PropertyType > base;

        CollectionDesignPropertyElem **values; 
        int valuecount;
    protected:
        void SetCollectionValues(CollectionDesignPropertyElem **vals, int valcnt)
        {
            values = vals;
            valuecount = valcnt;
        }
    public:
        GeneralCollectionDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader),
                values(NULL), valuecount(0)  { }

        virtual ~GeneralCollectionDesignProperty() { }

        virtual int SubCount(Object *propholder) 
        {
            return valuecount;
        }

        virtual DesignProperty* SubProperty(Object *propholder, int index)
        {
            return values[index]->Property();
        }
    };

    template<typename PropertyHolder>
    class GeneralRectDesignProperty : public GeneralCollectionDesignProperty<PropertyHolder, Rect>
    {
    private:
        typedef GeneralCollectionDesignProperty<PropertyHolder, Rect>   base;
    
        typedef GeneralRectDesignProperty<PropertyHolder>  self;
        CollectionDesignPropertyElem *values[4];

        bool sizebox;
        bool neg;

        long getsub(Object *propholder, int index)
        {
            const Rect &r = this->CallGetter(propholder);
            switch (index)
            {
            case 0:
                return r.left;
            case 1:
                return r.top;
            case 2:
                if (sizebox)
                    return r.right - r.left;
                return r.right;
            case 3:
                if (sizebox)
                    return r.bottom - r.top;
                return r.bottom;
            default:
                throw L"A rectangle only has 4 sides!";
            }
        }

        void setsub(Object *propholder, int index, long value)
        {
            Rect r = this->CallGetter(propholder);
            switch(index)
            {
            case 0:
                if (!neg)
                    r.right = max(value, r.right - r.left + value);
                r.left = value;
                break;
            case 1:
                if (!neg)
                    r.bottom = max(value, r.bottom - r.top + value);
                r.top = value;
                break;
            case 2:
                if (sizebox)
                {
                    if (!neg)
                        r.right = r.left + max(0, value);
                    else
                        r.right = r.left + value;
                }
                else
                {
                    if (!neg)
                        r.right = max(r.left, value);
                    else
                        r.right = value;
                }
                break;
            case 3:
                if (sizebox)
                {
                    if (!neg)
                        r.bottom = r.top + max(0, value);
                    else
                        r.bottom = r.top + value;
                }
                else
                {
                    if (!neg)
                        r.bottom = max(r.top, value);
                    else
                        r.bottom = value;
                }
                break;
            default:
                throw L"A rectangle only has 4 sides!";
            }

            this->CallSetter(propholder, r);
        }

    public:
        GeneralRectDesignProperty(const std::wstring &name, const std::wstring &category, bool sizebox, bool allownegative, typename base::readertype *reader) : base(name, category, reader), sizebox(sizebox), neg(allownegative)
        {
            const wchar_t *names1[] = { L"Left", L"Top", L"Width", L"Height" };
            const wchar_t *names2[] = { L"Left", L"Top", L"Right", L"Bottom" };

            this->SetCollectionValues(values, 4);
            for (int ix = 0; ix < 4; ix++)
                values[ix] = new CollectionDesignPropertyElem(new GeneralIntDesignProperty<self, long>(sizebox ? names1[ix] : names2[ix], std::wstring(), new SubPropertyReader<self, long>(this, ix, &self::getsub, &self::setsub) ) );
        }

        virtual ~GeneralRectDesignProperty()
        {
            for (int ix = 0; ix < 4; ix++)
                delete values[ix];
        }

        virtual std::wstring Value(Object *propholder)
        {
            const Rect &r = this->CallGetter(propholder);

            std::wstringstream ws;
            if (sizebox)
                ws << r.left << L", " << r.top << L", " << r.right - r.left << L", " << r.bottom - r.top;
            else
                ws << r.left << L", " << r.top << L", " << r.right << L", " << r.bottom;

            return ws.str();
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                Rect r;

                std::vector<std::wstring> split;
                splitstring(GenToLower(val), L" ,", split);
                if (split.size() != 4)
                    return false;

                int i;
                if (!StrToInt(split[0], i))
                    return false;
                r.left = i;
                if (!StrToInt(split[1], i))
                    return false;
                r.top = i;
                if (!StrToInt(split[2], i))
                    return false;
                if (sizebox)
                    r.right = r.left + i;
                else
                    r.right = i;
                if (!StrToInt(split[3], i))
                    return false;
                if (sizebox)
                    r.bottom = r.top + i;
                else
                    r.bottom = i;

                if (!neg)
                {
                    r.right = max(r.left, r.right);
                    r.bottom = max(r.top, r.bottom);
                }

                this->CallSetter(propholder, r);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }

        virtual std::wstring ExportValue(Object *propholder)
        {
            const Rect &r = this->CallGetter(propholder);

            std::wstringstream ws;
            ws << NLIBNS_STRING L"Rect(" << r.left << L", " << r.top << L", " << r.right << L", " << r.bottom << L")";

            return ws.str();
        }
    };

    template<typename PropertyHolder>
    class RectDesignProperty : public GeneralRectDesignProperty<PropertyHolder>
    {
    private:
        typedef GeneralRectDesignProperty<PropertyHolder>   base;
    public:
        template<typename GetterProc, typename SetterProc>
        RectDesignProperty(const std::wstring &name, const std::wstring &category, bool sizebox, bool allownegative, GetterProc getter, SetterProc setter) : base(name, category, sizebox, allownegative, CreatePropertyReader<PropertyHolder, Rect>(getter, setter)) // Set sizebox to false if the displayed sub properties should be right and bottom instead of the usual width and height. The allownegative value determines whether negative sizes are possible or the right and bottom edge can come before the left and top edge.
        {}
    };


    //-----------------------------
    // Interface class as a base for classes held by properties to get and set the attributes of objects through their getter and setter functions.
    // Passing the object which holds the property as propholder to CallGetter and CallSetter will update the object's attribute (a variable or some more complex state).
    template<typename PropertyType>
    class IArrayPropertyReader
    {
    public:
        virtual PropertyType CallGetter(Object *propholder, int index) = 0;
        virtual void CallSetter(Object *propholder, int index, PropertyType newvalue) = 0;
        virtual ~IArrayPropertyReader() {}
    };

    // Used as a direct getter and setter class of object properties. It calls a given function to get and another to set a value. The property can be read only if only a getter function is specified in its constructor.
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType = PropertyType,
            typename GetterPropertyHolder = PropertyHolder, typename GetterProc = GetterPropertyType (GetterPropertyHolder::*)(int), 
            typename SetterPropertyType = GetterPropertyType, typename SetterPropertyHolder = PropertyHolder, typename SetterProc = void (SetterPropertyHolder::*)(int, SetterPropertyType)>
    class ArrayPropertyReader : public IArrayPropertyReader<PropertyType>
    {
    private:
        typedef IArrayPropertyReader<PropertyType> base;
        typedef ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterProc, SetterPropertyType, SetterPropertyHolder, SetterProc>    selftype;
        typedef PropertyHolder holdertype;
    
        GetterProc getter;
        SetterProc setter;
    public:
        ArrayPropertyReader(GetterProc getter, SetterProc setter) : getter(getter), setter(setter) {}
        ArrayPropertyReader(GetterProc getter) : getter(getter), setter(NULL) { }

        virtual PropertyType CallGetter(Object *propholder, int index)
        {
            return ((dynamic_cast<GetterPropertyHolder*>(propholder))->*getter)(index);
        }

        virtual void CallSetter(Object *propholder, int index, PropertyType newvalue)
        {
            if (!setter)
                throw L"This is a read only property!";
            ((dynamic_cast<SetterPropertyHolder*>(propholder))->*setter)(index, newvalue);
        }

        //virtual IArrayPropertyReader<PropertyType>* Clone() const
        //{
        //    return new selftype(getter, setter);
        //}
    };

    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder>* CreateArrayPropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(int), int nullp)
    {
        return new ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int) const>* CreateArrayPropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(int) const, int nullp)
    {
        return new ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int) const>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder>* CreateArrayPropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(int), std::nullptr_t null)
    {
        return new ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder>
    ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int) const>* CreateArrayPropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(int) const, std::nullptr_t null)
    {
        return new ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int) const>(getter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder, typename SetterPropertyType, typename SetterPropertyHolder>
    ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int), SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(int, SetterPropertyType)>* CreateArrayPropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(int), void (SetterPropertyHolder::*setter)(int, SetterPropertyType))
    {
        return new ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int), SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(int, SetterPropertyType)>(getter, setter);
    }
    template<typename PropertyHolder, typename PropertyType, typename GetterPropertyType, typename GetterPropertyHolder, typename SetterPropertyType, typename SetterPropertyHolder>
    ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int) const, SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(int, SetterPropertyType)>* CreateArrayPropertyReader(GetterPropertyType (GetterPropertyHolder::*getter)(int) const, void (SetterPropertyHolder::*setter)(int, SetterPropertyType))
    {
        return new ArrayPropertyReader<PropertyHolder, PropertyType, GetterPropertyType, GetterPropertyHolder, GetterPropertyType (GetterPropertyHolder::*)(int) const, SetterPropertyType, SetterPropertyHolder, void (SetterPropertyHolder::*)(int, SetterPropertyType)>(getter, setter);
    }

    /* Base design property handling values with indexes. */
    template<typename PropertyHolder, typename ElemType>
    class GeneralArrayDesignProperty : public DesignProperty
    {
    private:
        typedef DesignProperty  base;
        typedef GeneralArrayDesignProperty<PropertyHolder, ElemType>    selftype;
    protected:
        typedef void (PropertyHolder::*IndexGetter)(std::vector<int> &);
        typedef IArrayPropertyReader<ElemType>    Reader;
    private:
        IndexGetter indexgetter;
        Reader *reader;
    protected:
        virtual bool IsListed()
        {
            return false;
        }

        virtual bool HasDefault()
        {
            return false;
        }

        virtual bool DefaultsEqual(Object *propholder, Object *otherpropholder, DesignProperty *otherprop)
        {
            return true;
        }

        virtual void Reset(Object *propholder)
        {
            ;
        }

        virtual const type_info& CreatorType()
        {
            return typeid(PropertyHolder);
        }

        virtual const type_info& ValueType()
        {
            return typeid(ElemType);
        }

        virtual bool IsDefault(Object *propholder)
        {
            return false;
        }

    public:
        GeneralArrayDesignProperty(const std::wstring &name, IndexGetter indexgetter, Reader *reader) : base(name, std::wstring()), indexgetter(indexgetter), reader(reader)
        {
        }

        void GetIndexes(Object *propholder, std::vector<int> &indexes)
        {
            (dynamic_cast<PropertyHolder*>(propholder)->*indexgetter)(indexes);
        }

        virtual ElemType CallGetter(Object *propholder, int index)
        {
            if (!reader)
                throw L"No reader in class!";
            return reader->CallGetter(propholder, index);
        }

        virtual void CallSetter(Object *propholder, int index, ElemType newvalue)
        {
            if (!reader)
                throw L"No reader in class!";
            reader->CallSetter(propholder, index, newvalue);
        }

        virtual std::wstring Value(Object *propholder, int index) = 0;

        virtual bool MustSerialize(Object *propholder)
        {
            return true;
        }

        virtual bool MustExport(Object *propholder)
        {
            return true;
        }

        virtual std::wstring ExportValue(Object *propholder)
        {
            throw L"Calling the wrong ExportValue";
        }

        virtual std::wstring ExportValue(Object *propholder, int index)
        {
            return Value(propholder, index);
        }
    };

    template<typename PropertyHolder>
    class IntArrayDesignProperty : public GeneralArrayDesignProperty<PropertyHolder, int>
    {
    private:
        typedef GeneralArrayDesignProperty<PropertyHolder, int> base;
    public:
        template<typename ElemGetter, typename ElemSetter>
        IntArrayDesignProperty(const std::wstring &name, typename base::IndexGetter indexgetter, ElemGetter elemgetter, ElemSetter elemsetter) : base(name, indexgetter, CreateArrayPropertyReader<PropertyHolder, int>(elemgetter, elemsetter))
        {
        }

        virtual std::wstring Value(Object *propholder, int index)
        {
            return IntToStr(this->CallGetter(propholder, index));
        }

        virtual void SetInnerValue(Form *parentform, Object *propholder, int index, const std::wstring& val) override
        {
            int i;
            if (!StrToInt(val, i))
                return;
            this->CallSetter(propholder, index, i);
        }
    };

    template<typename PropertyHolder>
    class BoolArrayDesignProperty : public GeneralArrayDesignProperty<PropertyHolder, bool>
    {
    private:
        typedef GeneralArrayDesignProperty<PropertyHolder, bool> base;
    public:
        template<typename ElemGetter, typename ElemSetter>
        BoolArrayDesignProperty(const std::wstring &name, typename base::IndexGetter indexgetter, ElemGetter elemgetter, ElemSetter elemsetter) : base(name, indexgetter, CreateArrayPropertyReader<PropertyHolder, bool>(elemgetter, elemsetter))
        {
        }

        virtual std::wstring Value(Object *propholder, int index)
        {
            bool val = this->CallGetter(propholder, index);
            return val ? L"True" : L"False";
        }

        virtual void SetInnerValue(Form *parentform, Object *propholder, int index, const std::wstring& val) override
        {
            bool bval = (val.size() == 1 && val[0] == L'1') || GenToLower(val) == L"true";
            this->CallSetter(propholder, index, bval);
        }
    };


    //-----------------------------


    // Base property for sets, like the anchor of controls which has bits holding separate values.
    template<typename PropertyHolder, typename SetType>
    class GeneralBoolListDesignProperty : public GeneralDesignProperty<PropertyHolder, uintset<SetType> >
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, uintset<SetType> > base;

        typedef GeneralBoolListDesignProperty<PropertyHolder, SetType>  selftype;

        std::pair<SetType, GeneralBoolDesignProperty<selftype>* > *subs; // Array of strings corresponding to each set value.
        int stringcount;
        std::wstring namespc;

        bool getsub(Object *propholder, int index)
        {
            const uintset<SetType>& set = this->CallGetter(propholder);
            return set.contains(subs[index].first);
        }

        void setsub(Object *propholder, int index, bool value)
        {
            uintset<SetType> aset = this->CallGetter(propholder);
            if (value)
                aset << subs[index].first;
            else
                aset -= subs[index].first;
            this->CallSetter(propholder, aset);
        }
    protected:
        virtual void _SetDefault(const uintset<SetType> &newdefault)
        {
            base::_SetDefault(newdefault);
            for (int ix = 0; ix < stringcount; ++ix)
                subs[ix].second->SetDefault(newdefault.contains(subs[ix].first) ? true : false);
        }
    public:
        GeneralBoolListDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, const ValuePair<SetType> strings[], int stringcount) : base(name, category, reader),
                stringcount(stringcount)
        {
            if (EnumStringsRegistered(strings))
                namespc = EnumStringsNamespace(strings) + L"::";

            subs = new std::pair<SetType, GeneralBoolDesignProperty<selftype>* > [stringcount];

            for (int ix = 0; ix < stringcount; ix++)
                subs[ix] = std::pair<SetType, GeneralBoolDesignProperty<selftype>* >(strings[ix].first, new GeneralBoolDesignProperty<selftype>(strings[ix].second, std::wstring(), new SubPropertyReader<selftype, bool>(this, ix, &selftype::getsub, &selftype::setsub)));
        }

        virtual ~GeneralBoolListDesignProperty()
        {
            for (int ix = 0; ix < stringcount; ix++)
                delete subs[ix].second;
            delete[] subs;
        }

        selftype* IncludeInDefault(const uintset<SetType> &added)
        {
            auto type = this->GetDefault(NULL);
            type << added;
            this->SetDefault(type);

            return this;
        }
    
        selftype* ExcludeFromDefault(const uintset<SetType> &removed)
        {
            auto type = this->GetDefault(NULL);
            type -= removed;
            this->SetDefault(type);

            return this;
        }

        virtual std::wstring Value(Object *propholder)
        {
            const uintset<SetType> &aset = this->CallGetter(propholder);

            std::wstring res;
            for (int ix = 0; ix < stringcount; ix++)
                if (aset.contains(subs[ix].first))
                {
                    if (res.length() > 0)
                        res += L", ";
                    res += subs[ix].second->Name();
                }

            return res;
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                uintset<SetType> aset;

                std::vector<std::wstring> split;
                splitstring(GenToLower(val), L" ,|+", split);
                for (auto it = split.begin(); it != split.end(); it++)
                {
                    for (int ix = 0; ix < stringcount; ix++)
                        if (*it == GenToLower(subs[ix].second->Name()))
                        {
                            aset << subs[ix].first;
                            break;
                        }
                }
                this->CallSetter(propholder, aset);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }

        virtual int SubCount(Object *propholder) 
        {
            return stringcount;
        }

        virtual DesignProperty* SubProperty(Object *propholder, int index)
        {
            return subs[index].second;
        }

        virtual std::wstring ExportValue(Object *propholder)
        {
            const uintset<SetType> &aset = this->CallGetter(propholder);

            std::wstring res;
            if (aset.empty())
                return L"0";
            for (int ix = 0; ix < stringcount; ix++)
                if (aset.contains(subs[ix].first))
                {
                    if (res.length() > 0)
                        res += L" | ";
                    res += namespc + subs[ix].second->Name();
                }

            return res;
        }

    };

    template<typename PropertyHolder, typename SetType>
    class GeneralSetDesignProperty : public GeneralBoolListDesignProperty<PropertyHolder, SetType>
    {
    private:
        typedef GeneralBoolListDesignProperty<PropertyHolder, SetType> base;
    public:
        GeneralSetDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, const ValuePair<SetType> strings[]) : base(name, category, reader, strings, EnumStringsCount(strings))
        {}
    };

    template<typename PropertyHolder, typename ClassType>
    class GeneralClassDesignProperty : public GeneralDesignProperty<PropertyHolder, ClassType>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, ClassType> base;
        typedef typename std::add_pointer<typename std::remove_pointer<ClassType>::type>::type   ClassTypePointer;

        DesignSerializer *serializer; // Serializer of the class held in the class property.
    public:
        GeneralClassDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
        {
            this->propertystyle << psReadonly;

            serializer = SerializerByTypeInfo(typeid(typename std::remove_pointer<ClassType>::type));
        }

        virtual ~GeneralClassDesignProperty()
        {
        }

        virtual const type_info& ValueType()
        {
            return typeid(typename std::remove_pointer<ClassType>::type);
        }

        virtual bool HasDefault()
        {
            if (base::HasDefault())
                return true;
            int cnt = SerializerPropertyCount(serializer, dpuListed);
            for (int ix = 0, pix = 0; pix != cnt; ++ix)
            {
                if (!SerializerProperties(serializer, ix)->IsListed())
                    continue;
                ++pix;
                if (!SerializerProperties(serializer, ix)->HasDefault())
                    return false;
            }
            return true;
        }

        virtual bool IsDefault(Object *propholder)
        {
            if (base::HasDefault())
                return base::IsDefault(propholder);
            int cnt = SerializerPropertyCount(serializer, 0);
            ClassTypePointer holderclass = this->CallGetterAsPointer(propholder);
            for (int ix = 0; ix != cnt; ++ix)
            {
                if (!SerializerProperties(serializer, ix)->IsDefault(holderclass))
                    return false;
            }
            return true;
        }

        virtual bool IsPointerValue()
        {
            return std::is_pointer<ClassType>::value;
        }

        virtual bool DefaultsEqual(Object *propholder1, Object *propholder2, DesignProperty *other)
        {
            //ClassTypePointer obj1 = dynamic_cast<ClassTypePointer>(propholder1);
            //ClassTypePointer obj2 = dynamic_cast<ClassTypePointer>(propholder2);
            if (base::HasDefault() /*|| !obj1 || !obj2*/)
                return base::DefaultsEqual(propholder1, propholder2, other);

            int cnt = SerializerPropertyCount(serializer, dpuListed);
            ClassTypePointer holderclass1 = this->CallGetterAsPointer(propholder1);
            ClassTypePointer holderclass2 = this->CallGetterAsPointer(propholder2);
            for (int ix = 0, pix = 0; pix != cnt; ++ix)
            {
                DesignProperty *prop = SerializerProperties(serializer, ix);
                if (!prop->IsListed())
                    continue;
                ++pix;
                if (!prop->DefaultsEqual(holderclass1, holderclass2, prop))
                    return false;
            }
            return true;
        }

        virtual void Reset(Object *propholder)
        {
            if (base::HasDefault())
            {
                base::Reset(propholder);
                return;
            }
            int cnt = SerializerPropertyCount(serializer, dpuListed);
            ClassTypePointer holderclass = this->CallGetterAsPointer(propholder);
            for (int ix = 0, pix = 0; pix != cnt; ++ix)
            {
                DesignProperty *prop = SerializerProperties(serializer, ix);
                if (!prop->IsListed())
                    continue;
                ++pix;
                prop->Reset(holderclass);
            }
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            return false;
        }

        virtual bool Delayed()
        {
            if (base::Delayed())
                return true;

            for (int ix = 0; ix < SubCount(NULL); ++ix)
                if (SubProperty(NULL, ix)->Delayed())
                    return true;
            return false;
        }

        virtual int SubCount(Object *propholder)
        {
            return SerializerPropertyCount(serializer, dpuListed);
        }

        virtual DesignProperty* SubProperty(Object *propholder, int index)
        {
            return SerializerProperties(serializer, SerializerPropertiesIndex(serializer, index, dpuListed));
        }

        virtual Object* SubHolder(Object *propholder)
        {
            return dynamic_cast<Object*>(this->CallGetterAsPointer(propholder));
        }
    };

    template<typename PropertyHolder, typename ElemType, typename ElemGetterProc, typename ElemAdderProc>
    class GeneralVectorDesignProperty : public GeneralDesignProperty<PropertyHolder, ElemType>
    {
    public:
        typedef int (PropertyHolder::*CountGetter)() const;
        typedef ElemGetterProc  ElemGetter;
        typedef ElemAdderProc   ElemAdder;

        CountGetter countgetter;
        ElemGetter elemgetter;
        ElemAdder elemadder;
    private:
        typedef GeneralDesignProperty<PropertyHolder, ElemType> base;
    public:
        GeneralVectorDesignProperty(const std::wstring &name, CountGetter countgetter, ElemGetter elemgetter, ElemAdder elemadder) : base(name, std::wstring(), NULL), countgetter(countgetter), elemgetter(elemgetter), elemadder(elemadder)
        {
            this->propertystyle -= psEditShared;
        }

        GeneralVectorDesignProperty(const std::wstring &name, const std::wstring &category, CountGetter countgetter, ElemGetter elemgetter, ElemAdder elemadder) : base(name, category, NULL), countgetter(countgetter), elemgetter(elemgetter), elemadder(elemadder)
        {
            this->propertystyle -= psEditShared;
        }

        virtual ~GeneralVectorDesignProperty()
        {
        }

        virtual const type_info& ValueType()
        {
            //if (std::is_pointer<ElemType>::value)
            return typeid(typename std::remove_pointer<ElemType>::type);
            //return typeid(ElemType);
        }

        virtual std::wstring Value(Object *propholder)
        {
            int cnt = Count(propholder);
            if (!cnt)
                return L"Empty";
            else
                return std::wstring(L"Count: ") + IntToStr(cnt);
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            return false;
        }

        virtual int Count(Object *propholder) 
        {
            return (((PropertyHolder*)propholder)->*countgetter)();
        }

        virtual ElemType Items(Object *propholder, int index)
        {
            return (((PropertyHolder*)propholder)->*elemgetter)(index);
        }
    };

    template<typename PropertyHolder, typename ElemType> class GeneralClassVectorDesignProperty;
    template<typename PropertyHolder, typename ElemType>
    class GeneralVectorClassElemDesignProperty : public GeneralClassDesignProperty<GeneralVectorClassElemDesignProperty<PropertyHolder, ElemType>, ElemType>
    {
    public:
        typedef ElemType (PropertyHolder::*ItemGetter)(int index);
    private:
        typedef GeneralClassDesignProperty<GeneralVectorClassElemDesignProperty<PropertyHolder, ElemType>, ElemType>    base;
        typedef GeneralVectorClassElemDesignProperty<PropertyHolder, ElemType>    selftype;

        typedef GeneralClassVectorDesignProperty<PropertyHolder, ElemType>  ownertype;
        ownertype *owner;

        ItemGetter itemgetter;
        ElemType getsub(Object *subholder, int index)
        {
            return (((PropertyHolder*)subholder)->*itemgetter)(index);
        }
    protected:
        virtual bool ParentCreated()
        {
            return owner->ParentCreated();
        }
    public:
        GeneralVectorClassElemDesignProperty(ownertype *owner, int index, ItemGetter itemgetter) : base(std::wstring(), std::wstring(), NULL), owner(owner), itemgetter(itemgetter)
        {
            this->SetReader(new SubPropertyReader<selftype, ElemType>(this, index, &selftype::getsub));
        }

        virtual std::wstring Value(Object *propholder)
        {
            return std::wstring();
        }
    };

    template<typename PropertyHolder, typename ElemType>
    class GeneralClassVectorDesignProperty : public GeneralVectorDesignProperty<PropertyHolder, ElemType, ElemType (PropertyHolder::*)(int), void (PropertyHolder::*)()>
    {
    public:
        typedef GeneralVectorClassElemDesignProperty<PropertyHolder, ElemType> ElemPropertyType;
    private:
        typedef GeneralVectorDesignProperty<PropertyHolder, ElemType, ElemType (PropertyHolder::*)(int), void (PropertyHolder::*)()>    base;
        DesignSerializer *serializer; // Serializer of the vector class held in the property.
    public:
        GeneralClassVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->propertystyle -= psEditShared;
        }

        GeneralClassVectorDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, category, countgetter, elemgetter, elemadder)
        {
            this->propertystyle -= psEditShared;
        }

        virtual ~GeneralClassVectorDesignProperty()
        {
        }

        virtual ElemPropertyType* ItemProperty(int index)
        {
            return new ElemPropertyType(this, index, this->elemgetter);
        }

        virtual DesignProperty* SubProperty(Object *propholder, int index)
        {
            return ItemProperty(index);
        }

        virtual void AddItem(Object *propholder)
        {
            if (!this->elemadder)
                throw L"Trying to add vector elem with property that has no elem adder.";
            (dynamic_cast<PropertyHolder*>(propholder)->*this->elemadder)();
        }
    };

    template<typename PropertyHolder, typename ElemType, typename GetType = ElemType, typename SetType = ElemType>
    class GeneralNativeVectorDesignProperty : public GeneralVectorDesignProperty<PropertyHolder, ElemType, GetType (PropertyHolder::*)(int) const, void (PropertyHolder::*)(SetType)>
    {
    private:
        typedef GeneralVectorDesignProperty<PropertyHolder, ElemType, GetType (PropertyHolder::*)(int) const, void (PropertyHolder::*)(SetType)>    base;
    public:
        GeneralNativeVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
            this->propertystyle -= psEditShared;
        }

        GeneralNativeVectorDesignProperty(const std::wstring &name, const std::wstring &category, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, category, countgetter, elemgetter, elemadder)
        {
            this->propertystyle -= psEditShared;
        }

        virtual ElemType ValueFromString(const std::wstring &str) = 0;
        virtual std::wstring ValueToString(ElemType elem) = 0;

        virtual ~GeneralNativeVectorDesignProperty()
        {
        }

        virtual void AddItem(Object *propholder, const std::wstring &val)
        {
            if (!this->elemadder)
                throw L"Trying to add vector elem value with property that has no elem value adder.";
            (dynamic_cast<PropertyHolder*>(propholder)->*this->elemadder)(ValueFromString(val));
        }
    };

    template<typename PropertyHolder, typename GetType = std::wstring, typename SetType = const std::wstring&>
    class GeneralStringVectorDesignProperty : public GeneralNativeVectorDesignProperty<PropertyHolder, std::wstring, GetType, SetType>
    {
    private:
        typedef GeneralNativeVectorDesignProperty<PropertyHolder, std::wstring, GetType, SetType> base;
    public:
        GeneralStringVectorDesignProperty(const std::wstring &name, typename base::CountGetter countgetter, typename base::ElemGetter elemgetter, typename base::ElemAdder elemadder) : base(name, countgetter, elemgetter, elemadder)
        {
        }

        virtual std::wstring ValueFromString(const std::wstring &str)
        {
            return str;
        }
        virtual std::wstring ValueToString(std::wstring elem)
        {
            return elem;
        }
    };

    // Base property for listing visual and non-visual controls. Inclusion is decided by a bool(*collector)(Object*) function.
    template<typename PropertyHolder, typename PropertyType>
    class GeneralControlDesignProperty : public GeneralDesignProperty<PropertyHolder, PropertyType*>
    {
    private:
        typedef GeneralDesignProperty<PropertyHolder, PropertyType*> base;

        //std::vector<std::pair<std::wstring, Object*>> objectstrings;

        std::vector<std::pair<std::wstring, Object*>> ObjectStrings(Object *propholder, std::wstring value = std::wstring())
        {
            DesignFormBase *f = this->GetDesignFormBase(propholder);
            std::vector<std::pair<std::wstring, Object*>> objectstrings;
            CollectObjects(f, objectstrings, collector, value);
            return objectstrings;
        }

    protected:
        typedef bool(*Collector)(Object*);
        Collector collector;
    public:
        // Constructors using a ListType string array.
        GeneralControlDesignProperty(const std::wstring &name, const std::wstring &category, Collector collector, typename base::readertype *reader) : base(name, category, reader), collector(collector)
        {
            this->propertystyle -= psGuestEditable;
            this->propertystyle << psDelayedRestore;
            this->SetDefault(nullptr);
        }
        virtual ~GeneralControlDesignProperty() {}

        virtual std::wstring Value(Object *propholder)
        {
            PropertyType *val = this->CallGetter(propholder);
            if (!val)
                return std::wstring();

            std::wstringstream str;

            if (!DesignFormIsForm(this->GetDesignFormBase(propholder), val->SubOwner() && !val->ParentForm() ? val->SubOwner()->ParentForm() : val->ParentForm()))
                str << val->ParentForm()->Name() << L"->";
            str << val->Name();

            return str.str();
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            if (val.length() == 0)
            {
                this->CallSetter(propholder, NULL);
                return true;
            }

            auto objectstrings = ObjectStrings(propholder, val);
            if (objectstrings.empty())
                return false;
            this->CallSetter(propholder, dynamic_cast<PropertyType*>(objectstrings.front().second));
            return true;
        }

        virtual int ListCount(Object *propholder)
        {
            return ObjectStrings(propholder).size();
        }

        virtual std::wstring ListItem(Object *propholder, int index)
        {
            return ObjectStrings(propholder)[index].first;
        }

        virtual int Selected(Object *propholder)
        {
            PropertyType *val = this->CallGetter(propholder);
            if (!val)
                return -1;

            int ix = 0;
            auto objectstrings = ObjectStrings(propholder);
            for (auto it = objectstrings.begin(); it != objectstrings.end(); ++it, ++ix)
                if ((*it).second == val)
                    return ix;
            return -1;
        }

        virtual void* ListValue(Object *propholder, int index)
        {
            return ObjectStrings(propholder)[index].second;
        }

        virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
        {
            auto objectstrings = ObjectStrings(propholder);
            for (auto it = objectstrings.begin(); it != objectstrings.end(); ++it)
                if ((*it).second == val)
                {
                    this->CallSetter(propholder, (PropertyType*)val);
                    return true;
                }
            return false;
        }
    };

    template<const type_info &info>
    bool GeneralControlByTypeDesignPropertyCollectorFunc(Object *obj)
    {
        return obj->TypeInfo() == info;
    }

    template<typename PropertyHolder, typename PropertyType>
    class GeneralControlByTypeDesignProperty : public GeneralControlDesignProperty<PropertyHolder, PropertyType>
    {
    private:
        typedef GeneralControlDesignProperty<PropertyHolder, PropertyType> base;
    public:
        // Constructors using a ListType string array.
        GeneralControlByTypeDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, GeneralControlByTypeDesignPropertyCollectorFunc<typeid(PropertyType)>, reader)
        {
        }
        virtual ~GeneralControlByTypeDesignProperty() {}
    };

    template<typename PropertyHolder, typename SetType>
    class SetDesignProperty : public GeneralSetDesignProperty<PropertyHolder, SetType>
    {
    private:
        typedef GeneralSetDesignProperty<PropertyHolder, SetType> base;
    public:
        template<typename GetterProc, typename SetterProc>
        SetDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter, ValuePair<SetType> strings[]) : base(name, category, CreatePropertyReader<PropertyHolder, uintset<SetType> >(getter, setter), strings)
        {}
    };


    class IEventProperty
    {
    public:
        virtual void* Event(Object *propholder) = 0;
    };

    //template<typename EventType> int EventDesignPropSpecializer();
    //extern const wchar_t* EventParameterStrings[];
    // Events
    template<typename EventHolder, typename EventType>
    class GeneralEventDesignProperty : public GeneralDesignProperty<EventHolder, std::wstring>, public IEventProperty
    {
    private:
        typedef GeneralDesignProperty<EventHolder, std::wstring>   base;
        typedef GeneralEventDesignProperty<EventHolder, EventType> selftype;

        //int id; // Index of the event in the EventParameterStrings array.
        std::wstring type; // Type of event which is the same as its exported parameter type with namespace included.
    protected:
    public:
        GeneralEventDesignProperty(const std::wstring &name, const std::wstring &category) : base(name, category, NULL)
        {
            this->propertystyle << psDelayedRestore;

            this->SetDefault(std::wstring());
            //id = EventDesignPropSpecializer<EventType>();
            type = RegisteredEventType(typeid(EventType));
        }

        //virtual const wchar_t* ParamString()
        //{
        //    return EventParameterStrings[id];
        //}

        //virtual int Id()
        //{
        //    return id;
        //}

        virtual const std::wstring& Type()
        {
            return type;
        }

        virtual std::wstring Value(Object *propholder)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return DesignFormEventFunction(form, type, propholder, this->Name());
        }

        virtual void* Event(Object *propholder)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return DesignFormEvent(form, type, propholder, this->Name());
        }

        virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
        {
            try
            {
                if (val.length() && !ValidVarName(val))
                    return false;
                DesignFormBase *form = this->GetDesignFormBase(propholder);
                DesignFormSetEventFunction(form, type, propholder, this->Name(), val);
            }
            catch(...)
            {
                return false;
            }
            return true;
        }

        virtual bool IsDefault(Object *propholder)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return !DesignFormEventFunction(form, type, propholder, this->Name()).length();
        }

        virtual bool IsDefaultWrite(Object *propholder)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return !DesignFormEventFunction(form, type, propholder, this->Name()).length();
        }

        virtual int ListCount(Object *propholder)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return DesignFormEventCountById(form, type );
        }

        virtual std::wstring ListItem(Object *propholder, int index)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return DesignFormEventFunctionByIndex(form, type, index);
        }

        virtual void* ListValue(Object *propholder, int index)
        {
            return (void*)(index + 1);
        }

        virtual int Selected(Object *propholder)
        {
            DesignFormBase *form = this->GetDesignFormBase(propholder);
            return DesignFormEventFunctionIndex(form, type, propholder, DesignFormEventFunction(form, type, propholder, this->Name()));
        }

        virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
        {
            if ((int)val < 1)
                return false;
            return SetValue(parentform, propholder, ListItem(propholder, (int)val - 1));
        }

        virtual bool ClickEdit(Form *parentform, Object *propholder)
        {
            return false;
        }

    };

    template<typename EventHolder, typename EventType>
    class EventDesignProperty : public GeneralEventDesignProperty<EventHolder, EventType>
    {
    private:
        typedef GeneralEventDesignProperty<EventHolder, EventType> base;
    public:
        EventDesignProperty(const std::wstring &name, const std::wstring &category) : base(name, category)
        {
        }
    };


    // Collection of properties, which doesn't allow duplicates for the same class.
    class DesignProperties : protected std::vector<DesignProperty*>
    {
    private:
        typedef std::vector<DesignProperty*>  base;

        std::vector<DesignProperty*> overriden;
        DesignSerializer *serializer;
    public:
        DesignProperties();
        virtual ~DesignProperties();

        void SetSerializer(DesignSerializer *aserializer);
        DesignSerializer* Serializer();

        // Adds a property to the list of properties. Returns a property if it was overriden with the newly added one.
        template<typename PropertyHolder, typename PropertyType>
        DesignProperty* AddProperty(GeneralDesignProperty<PropertyHolder, PropertyType> *prop) 
        {
            DesignProperty *deleted = NULL;

            for (auto it = begin(); it != end(); it++)
            {
                if ((*it)->Name() == prop->Name() && (*it)->CreatorType() == typeid(PropertyHolder))
                    throw L"The class already provided a property with the same name.";
            }

            for (auto it = overriden.begin(); it != overriden.end(); it++)
            {
                if ((*it)->Name() == prop->Name() && (*it)->CreatorType() == typeid(PropertyHolder))
                    throw L"The property of the same class with the same name was overriden.";
            }

            for (auto it = rbegin(); it != rend(); ++it)
            {
                if ((*it)->Name() == prop->Name())
                {
                    deleted = *it;
                    deleted->Hide();
                    overriden.push_back(deleted);
                    erase(it.base() - 1);
                    break;
                }
            }

            push_back(prop);
            prop->SetOwner(this);

            return deleted;
        }
        // Adds a property to the list of properties. Returns a property if it was overriden with the newly added one.
        template<typename PropertyHolder, typename ElemType>
        DesignProperty* AddProperty(GeneralArrayDesignProperty<PropertyHolder, ElemType> *prop) 
        {
            DesignProperty *deleted = NULL;

            for (auto it = begin(); it != end(); it++)
            {
                if ((*it)->Name() == prop->Name() && (*it)->CreatorType() == typeid(PropertyHolder))
                    throw L"The class already provided a property with the same name.";
            }

            for (auto it = overriden.begin(); it != overriden.end(); it++)
            {
                if ((*it)->Name() == prop->Name() && (*it)->CreatorType() == typeid(PropertyHolder))
                    throw L"The property of the same class with the same name was overriden.";
            }

            for (auto it = rbegin(); it != rend(); ++it)
            {
                if ((*it)->Name() == prop->Name())
                {
                    deleted = *it;
                    deleted->Hide();
                    overriden.push_back(deleted);
                    erase(it.base() - 1);
                    break;
                }
            }

            push_back(prop);
            prop->SetOwner(this);

            return deleted;
        }

        DesignProperty* MoveToEnd(int index)
        {
            DesignProperty *p = operator[](index);
            erase(begin() + index);
            push_back(p);
            return p;
        }

        DesignProperty* operator[](int index)
        {
            return base::operator[](index);
        }

        using base::begin;
        using base::end;
        using base::rbegin;
        using base::rend;
        using base::size;
    };


}
/* End of NLIBNS */


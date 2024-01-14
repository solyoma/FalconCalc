#pragma once
#include <functional>


// How to notify the designer about a new control and designer types:
// Create a global RegisterInitializerFunctions object. The parameters are:
//      namespacename = a name specifying the namespace which contains the control and the types
//      typeinitializer = a void function that takes no arguments. The function should call RegisterControlEvent and RegisterEnumStrings for event and enum type registrations.
//      controlinitializer = a void function that takes no arguments. The function should call CreateDesignProperties to register new control(s).
//
// Example:
//
//  void MyTypeInitializer();
//  void MyControlInitializer();
//
//  RegisterInitializerFunctions MyRegisterObject(L"mynamespacename", &MyTypeInitializer, &MyControlInitializer);
//
//  // Forward declare enum string arrays, if the type initializer is in a different file:
//  extern const ValuePair<MyEnumTypes1> MyEnumStrings1[];
//  extern const ValuePair<MyEnumTypes2> MyEnumStrings1[];
//
//  void MyTypeInitializer()
//  {
//      using nlib::RegisterControlEvent;
//      using nlib::RegisterEnumStrings;
//
//      RegisterControlEvent<MyHandler1Event>(L"MyHandler1Parameters"); // Template param is the type of the event, the string is for printing event handler function arguments to an exported cpp file.
//      RegisterControlEvent<MyHandler2Event>(L"MyHandler2Parameters");
//      // ...
//
//      RegisterEnumStrings(MyEnumStrings1, mes1Count); // mes1Count is an element of MyEnumTypes1, the exact number of strings in MyEnumStrings1. It is only required when DESIGNING is defined.
//      RegisterEnumStrings(MyEnumStrings2, mes2Count);
//      // ...
//
//  }
//
//  void MyControlInitializer()
//  {
//      using nlib::CreateDesignProperties;
//      using nlib::otVisual;
//      using nlib::otNonVisual;
//
//      // This will call the static EnumerateProperties for the registered control class and make sure that a button for it is included in the designer:
//      CreateDesignProperties<MyVisControl1, otVisual> (L"MyVisControl1", L"My Visual Controls", Bitmap((HMODULE)modulehandle, MAKEINTRESOURCE(IDI_MYCONTROLIMAGE1)));
//      CreateDesignProperties<MyNVisControl2, otNonVisual> (L"MyNVisControl2", L"My NonVisual Controls", Bitmap((HMODULE)modulehandle, MAKEINTRESOURCE(IDI_MYCONTROLIMAGE2)));
//      // ...
//  }


namespace NLIBNS
{


    // otForm : the form and container form
    // otVisual : any control placed on a form which can be seen and the user can interact with it (mostly)
    // otNonVisual : non visual controls that give some functionality or data to controls. i.e. an imagelist or dialog. They can be placed on containers and shared between forms.
    // otSingleNonVisual : non visual controls that can only be placed on a single form. i.e. a main menu.
    // otClass : properties of visual and non visual controls that are more complex than a simple value, i.e. the font property or items in a list view.
    // otSubControl : visual or non-visual named sub part of another control. i.e. pages on a tabcontrol.
    // otVector : a list of classes, subproperties, vectors and native types. i.e. the list for pages on a tabcontrol or column headers of a listview.
    // otNative : string, int etc.
    // otTopLevel : the project or any other object that cannot be inside controls or classes.
    enum ObjectTypes : int {
            otForm, otVisual, otNonVisual, otSingleNonVisual, otClass,
            otSubControl, otVector, otNative, otTopLevel, otUnknown
    };

    class DesignSerializer;
    class Object;
    class Bitmap;

    class Control;
    class NonVisualControl;
    class DesignForm;
    class DesignContainerForm;

    template<typename T> struct ValuePair;

    class DesignPropertiesHelper // Helper struct, do not use.
    {
        template<typename EnumType>
        friend class _RegisterEnumStrings;
        template<typename EnumType>
        friend int EnumStringsCount(const ValuePair<EnumType> *strings);
        template<typename EventType>
        friend class _RegisterControlEvent;
        template<typename PropertyHolder, ObjectTypes objtype>
        friend class _CreateDesignProperties;
        template<typename EnumType>
        friend bool EnumStringsRegistered(const ValuePair<EnumType> *strings);
        template<typename EnumType>
        friend const std::wstring& EnumStringsNamespace(const ValuePair<EnumType> *strings);
        template<typename T>
        friend std::wstring EnumToEnumString(ValuePair<T> strings[], T value);

        static void AddEnumStrings(void *strings, int size);
        static int EnumStringsCount(void *strings);

        static bool EnumStringsRegistered(void *strings);
        static const std::wstring& EnumStringsNamespace(void *strings);

        static void AddControlEvent(const type_info &eventtype, const std::wstring &paramstring);
        static void AddObjectProperty(std::function<Object*()> func, const type_info &type, ObjectTypes objtype, const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect);
    };

    template<typename EnumType>
    class _RegisterEnumStrings
    {
        template<typename EnumType>
        friend void RegisterEnumStrings(const ValuePair<EnumType> strings[], int size);

        template<typename EnumType>
        friend void RegisterEnumStrings(const ValuePair<EnumType> strings[], EnumType size);

        _RegisterEnumStrings(const ValuePair<EnumType> strings[], int size)
        {
            DesignPropertiesHelper::AddEnumStrings((void*)strings, size);
        }
    };


    template<typename EnumType>
    void RegisterEnumStrings(const ValuePair<EnumType> strings[], int size)
    {
        _RegisterEnumStrings<EnumType> dummy(strings, size);
    }

    template<typename EnumType>
    void RegisterEnumStrings(const ValuePair<EnumType> strings[], EnumType size)
    {
        _RegisterEnumStrings<EnumType> dummy(strings, (int)size);
    }

    template<typename EnumType>
    int EnumStringsCount(const ValuePair<EnumType> *strings)
    {
        return DesignPropertiesHelper::EnumStringsCount((void*)strings);
    }

    template<typename EnumType>
    bool EnumStringsRegistered(const ValuePair<EnumType> *strings)
    {
        return DesignPropertiesHelper::EnumStringsRegistered((void*)strings);
    }

    template<typename EnumType>
    const std::wstring& EnumStringsNamespace(const ValuePair<EnumType> *strings)
    {
        return DesignPropertiesHelper::EnumStringsNamespace((void*)strings);
    }

    template<typename EventType>
    class _RegisterControlEvent
    {
        template<typename EventType>
        friend void RegisterControlEvent(const std::wstring &paramstring);

        _RegisterControlEvent(const std::wstring &paramstring)
        {
            DesignPropertiesHelper::AddControlEvent(typeid(EventType), paramstring);
        }
    };

    template<typename EventType>
    void RegisterControlEvent(const std::wstring &paramstring)
    {
        _RegisterControlEvent<EventType> dummy(paramstring);
    }

    template<typename PropertyHolder, ObjectTypes objtype>
    class _CreateDesignProperties
    {
        typedef PropertyHolder  propholder;

        template<typename PropertyHolder, ObjectTypes ot>
        struct IsValid : public std::false_type
        {
        };

        template<typename PropertyHolder>
        struct IsValid<PropertyHolder, otVisual> : public std::true_type
        {
        };
        template<typename PropertyHolder>
        struct IsValid<PropertyHolder, otNonVisual> : public std::true_type
        {
        };

        template<typename PropertyHolder>
        struct IsValid<PropertyHolder, otSingleNonVisual> : public std::true_type
        {
        };

        template<typename PropertyHolder, ObjectTypes ot>
        typename std::enable_if<IsValid<PropertyHolder, ot>::value, std::function<Object*()>>::type GetConstructor()
        {
            return []() { return new PropertyHolder; };
        }

        template<typename PropertyHolder, ObjectTypes ot>
        typename std::enable_if<!IsValid<PropertyHolder, ot>::value, std::function<Object*()>>::type GetConstructor()
        {
            return nullptr;
        }
    public:
        _CreateDesignProperties(const std::wstring &displayname, const std::wstring &category, Bitmap *controlimage, Rect imagerect)
        {
            DesignPropertiesHelper::AddObjectProperty(GetConstructor<PropertyHolder, objtype>(), typeid(PropertyHolder), objtype, displayname, category, controlimage, imagerect);
            PropertyHolder::EnumerateProperties(SerializerByTypeInfo(typeid(PropertyHolder)));
        }
    };

    template<typename PropertyHolder, ObjectTypes objtype>
    void CreateDesignProperties(const std::wstring &displayname, const std::wstring &category = std::wstring(), Bitmap *controlimage = nullptr, Rect imagerect = Rect())
    {
        _CreateDesignProperties<PropertyHolder, objtype> dummy(displayname, category, controlimage, imagerect);
    }

    //std::wstring FixNamespacedName(const std::wstring &name); // Returns a name string with correct namespacing. If the name only contains the nlib namespace, the namespace is removed. Otherwise the string is returned as is.
    std::wstring NamespaceByTypeInfo(const type_info &type); // Returns the registered namespace for a type.

    const type_info& TypeInfoByDisplayName(const std::wstring &name, bool namespacedname);
    ObjectTypes ObjectTypeByTypeDisplayName(const std::wstring &name, bool namespacedname);
    ObjectTypes ObjectTypeByTypeInfo(const type_info &info);
    std::wstring DisplayNameByTypeInfo(const type_info &info, bool namespacedname);
    bool ShareableByTypeInfo(const type_info &info);
    int ImageIndexByTypeInfo(const type_info &info);
    DesignSerializer* SerializerByTypeInfo(const type_info &info);
    DesignSerializer* SerializerByDisplayName(const std::wstring &name, bool namespacedname);


    class Control;
    class NonVisualControl;
    Control* CreateControlOfType(const type_info &info);
    NonVisualControl* CreateNVControlOfType(const type_info &info);

    std::wstring FullNamespaceName(const std::wstring &name); // Adds the nlib namespace to names with no namespace specifier.
    std::wstring ShortNamespaceName(const std::wstring &name); // Removes the nlib namespace string from names with only the nlib namespace.
    bool EventTypesMatch(const std::wstring &type1, const std::wstring &type2); // Returns true if two event type names (namespace included) match. If no namespace is present, the string is checked with the nlib namespace.
    bool EventTypeRegistered(const std::wstring &eventtype);
    std::wstring RegisteredEventType(const type_info &type);

    template<typename T>
    std::wstring EnumToEnumString(ValuePair<T> strings[], T value)
    {
        if (!EnumStringsRegistered(strings))
            throw L"Enum strings not registered.";

        int cnt = DesignPropertiesHelper::EnumStringsCount((void*)strings);
        std::wstring nmspace = DesignPropertiesHelper::EnumStringsNamespace((void*)strings);
        for (int ix = 0; ix < cnt; ++ix)
            if (strings[ix].first == value)
                return nmspace + L"::" + strings[ix].second;

        throw "Enum value not found in enum strings.";
    }

    class RegisterInitializerFunctions
    {
    public:
        RegisterInitializerFunctions(const std::wstring &namespacename, void(*typeinitializer)(), void(*controlinitializer)());
    };


}
/* End of NLIBNS */


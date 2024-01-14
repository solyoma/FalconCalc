#pragma once

#include <set>
#include "designproperties.h"
#include "sparse_list.h"
#include "designio.h"

// Definitions to be output to files to be later recognized by the designer when it has to update them.
// General define used to mark anything designer related.
#define PUBLIC_DECLARATIONS L"N_PUBLIC"
#define PROTECTED_DECLARATIONS L"N_PROTECTED"
#define PRIVATE_DECLARATIONS L"N_PRIVATE"
// Control initialization defines.
#define INITIALIZATION_FUNCTION L"InitializeFormAndControls"


//---------------------------------------------


namespace NLIBNS
{


    class DesignSerializer;
    class DesignProperty;

    DesignSerializer* SerializerByTypeInfo(const type_info &info);

    /* Deserialization:
     * In the first round these property structures get the name and value strings of the properties. The class properties get their sub property names and values.
     * In the second round those property deserialization structures which cannot find their given property in the serializer's list get deleted. Classes are
     * deleted together with all their sub properties if even only one sub property is missing. In this round the DesignProperty variables are filled with the
     * corresponding property.
     * In the third round the properties are deserialized via the property variables, and the passed controls. Classes automatically go through all their sub properties.
     */

    // Classes for serializer values which will later be translated into properties when loading or pasting.

    // Data structures for de-serialization of controls. They hold every property with name and value, classes etc. used in restoring the original object.
    enum ControlDeserializerPropertyType { cdptSimple, cdptList, cdptClassVector, cdptNativeVector, cdptArray };
    struct ControlDeserializerProperty 
    {
        DesignProperty *prop; // The property responsible for setting the value in derived classes. The true properties are looked up in the serializers and passed to these variables.
        bool freeprop; // The property must be freed by this struct on destruction.
        ControlDeserializerPropertyType type;

        ControlDeserializerProperty(DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type) : prop(prop), freeprop(freeprop), type(type)  {}
        virtual ~ControlDeserializerProperty()
        {
            if (freeprop)
                delete prop;
        };

        virtual const std::wstring& Name() = 0;

        virtual void SetValue(Object *propholder) = 0; // Use the properties to set the real values in the property holders, which are controls or other objects.
    };
    typedef std::list<ControlDeserializerProperty*> SerializerPropertyList;
    typedef std::list<ControlDeserializerProperty*>::iterator SerializerPropertyIterator;
    typedef std::list<ControlDeserializerProperty*>::reverse_iterator SerializerPropertyRIterator;

    // Property name/value pair for properties that can be set by simply passing a string to SetValue.
    struct ControlDeserializerPropertyNameValue : public ControlDeserializerProperty
    {
    private:
        typedef ControlDeserializerProperty base;
    public:
        ControlDeserializerPropertyNameValue(const std::wstring &name, const std::wstring &value, DesignProperty *prop);
        virtual ~ControlDeserializerPropertyNameValue();

        std::pair<std::wstring, std::wstring> data;

        virtual const std::wstring& Name();
        const std::wstring& Value();
        virtual void SetValue(Object *propholder);
    };

    // Base property deserializer for properties that have separate items. Like classes, vectors and arrays.
    struct ControlDeserializerPropertyCollection : public ControlDeserializerProperty
    {
    private:
        typedef ControlDeserializerProperty base;
    public:
        ControlDeserializerPropertyCollection(const std::wstring &name, DesignSerializer *serializer, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type);
        virtual ~ControlDeserializerPropertyCollection();

        std::wstring name; // Name of the list property.
        DesignSerializer *serializer; // Serializer for the property. i.e. if this is a font class, the value will be the serializer for fonts. In vector types this is a serializer for vectors.
        SerializerPropertyIterator pos; // Position in parent item or list's properties list.

        virtual const std::wstring& Name();
    };

    // Array property deserializer.
    struct ControlDeserializerPropertyArray : public ControlDeserializerPropertyCollection
    {
    private:
        typedef ControlDeserializerPropertyCollection base;
    public:
        ControlDeserializerPropertyArray(const std::wstring &name, DesignSerializer *serializer, DesignProperty *prop, bool freeprop = false);
        virtual ~ControlDeserializerPropertyArray();

        sparse_list<std::wstring> values;   

        void AddValue(unsigned int index, const std::wstring &value);
        virtual void SetValue(Object *propholder);
    };

    // Class or vector property list for properties which have properties themselves and those have to be set separately.
    struct ControlDeserializerPropertyList : public ControlDeserializerPropertyCollection
    {
    private:
        typedef ControlDeserializerPropertyCollection base;
    public:
        ControlDeserializerPropertyList(const std::wstring &name, DesignSerializer *serializer, DesignProperty *prop, bool freeprop = false, ControlDeserializerPropertyType type = cdptList);
        virtual ~ControlDeserializerPropertyList();

        SerializerPropertyList properties; // List of properties.

        virtual void SetValue(Object *propholder);
    };

    struct GeneralControlDeserializerPropertyVector : public ControlDeserializerPropertyList
    {
    private:
        typedef ControlDeserializerPropertyList base;
    public:
        GeneralControlDeserializerPropertyVector(const std::wstring &name, const type_info &elemtype, DesignSerializer *serializer, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type);
        virtual ~GeneralControlDeserializerPropertyVector();

        const type_info &elemtype; // Type of the elements in the vector
    };

    struct ControlDeserializerPropertyClassVector : public GeneralControlDeserializerPropertyVector
    {
    private:
        typedef GeneralControlDeserializerPropertyVector base;
    public:
        ControlDeserializerPropertyClassVector(const std::wstring &name, const type_info &elemtype, DesignSerializer *serializer, DesignProperty *prop, bool freeprop = false, ControlDeserializerPropertyType type = cdptClassVector);
        virtual ~ControlDeserializerPropertyClassVector();

        virtual void SetValue(Object *propholder);
    };

    struct ControlDeserializerPropertyNativeVector : public GeneralControlDeserializerPropertyVector
    {
    private:
        typedef GeneralControlDeserializerPropertyVector base;
    public:
        ControlDeserializerPropertyNativeVector(const std::wstring &name, const type_info &elemtype, DesignProperty *prop, bool freeprop = false, ControlDeserializerPropertyType type = cdptNativeVector);
        virtual ~ControlDeserializerPropertyNativeVector();

        virtual void SetValue(Object *propholder);
    };

    // Holds all properties and classes of a single control being de-serialized, and the control itself during de-serialization.
    struct ControlDeserializerItem
    {
        const type_info &type; // Control type to create.
        //std::wstring name;

        DesignProperty *prop; // Property for subtypes.

        SerializerPropertyList properties; // List of properties
        ControlDeserializerItem *parent; // Container control that will be the parent of the recreated control.
        DesignSerializer *serializer;

        ControlDeserializerItem(const type_info &type, ControlDeserializerItem *parent, DesignProperty *prop = NULL) : type(type), prop(prop), parent(parent), serializer(SerializerByTypeInfo(type)) {}
        ~ControlDeserializerItem();
    };

    // Vector holding properties and their serializer objects, instantiated for every control and class that needs to be edited with the designer's properties list.
    class DesignSerializer
    {
    private:
        // The control of the serializer can accept children. Only used when the serializer is for visual control or subcontrol.
        bool container;
        // The serialized object is a form or container for non-visual controls. This is usually not set for any other control.
        bool nvcontainer;
        // A list of otSubControl object names that can be placed on the control of this serializer. If container is false but subchildtypes contains otSubControls, the control only acts as a parent for the otSubControls types. Only used when the serializer is for a visual control or subcontrol.
        std::set<std::string> subchildtypes;

        DesignProperty *defaultproperty; // See MakeDefault().
        DesignProperty *userdefaultproperty;

        // Base class for property serializers.
        class ISerializeItem
        {
        public:
            virtual bool Holds(DesignProperty *prop) = 0; // Returns true if the property the serializer item refers to equals to prop.

            // Serialize is for writing a cpp code, Serialize is for copy/pasting or saving in the form format.
            virtual void CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder) = 0;
            virtual void DeclareSerialize(Indentation &indent, std::wiostream &ws, Object *propholder, AccessLevels access) = 0;
            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder) = 0;
            virtual bool MustExport(Object *propholder) = 0;
            virtual bool MustSerialize(Object *propholder) = 0;
            virtual bool Delayed() = 0;
            virtual const std::wstring& Name() = 0;
            virtual void SetName(const std::wstring& newname) = 0;
    
            virtual ~ISerializeItem() {}
        };
        template<typename DesignPropertyType> class SerializeItem;
        template<typename DesignPropertyType> class SerializeClassItem;
        template<typename DesignPropertyType> class SerializeClassVectorItem;
        template<typename DesignPropertyType> class SerializeNativeVectorItem;
        template<typename DesignPropertyType> class SerializeArray;
        std::vector<ISerializeItem*> serializers;
        DesignProperties properties;

        class ISerializeEvent
        {
        public:
            virtual bool Holds(DesignProperty *prop) = 0;

            virtual const std::wstring& Type() = 0;
            virtual bool MustSerialize(Object *propholder) = 0;
            virtual bool MustExport(Object *propholder) = 0;
            virtual void CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder) = 0;
            //virtual void DeclareSerialize(Indentation &indent, std::wiostream &ws, Object *propholder, AccessLevels access) = 0;
            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder) = 0;

            virtual ~ISerializeEvent() {}
        };
        template<typename DesignPropertyType> class SerializeEvent;
        std::vector<ISerializeEvent*> eventserializers;
        DesignProperties events;

        template<typename PropertyHolder, typename PropertyType>
        static SerializeItem<GeneralDesignProperty<PropertyHolder, PropertyType>>* CreateSerializeItem(const std::wstring& funcname, GeneralDesignProperty<PropertyHolder, PropertyType> *propitem)
        {
            return new SerializeItem<GeneralDesignProperty<PropertyHolder, PropertyType>>(funcname, propitem);
        }
        // Registers a property representing a class which itself has properties.
        template<typename PropertyHolder, typename ClassType>
        static SerializeClassItem<GeneralClassDesignProperty<PropertyHolder, ClassType>>* CreateSerializeItem(const std::wstring& funcname, GeneralClassDesignProperty<PropertyHolder, ClassType> *propitem)
        {
            return new SerializeClassItem<GeneralClassDesignProperty<PropertyHolder, ClassType>>(funcname, propitem);
        }
        // Registers a property representing elements of a vector to be serialized.
        template<typename PropertyHolder, typename ElemType>
        static SerializeClassVectorItem<GeneralClassVectorDesignProperty<PropertyHolder, ElemType>>* CreateSerializeItem(const std::wstring& funcname, GeneralClassVectorDesignProperty<PropertyHolder, ElemType> *propitem)
        {
            return new SerializeClassVectorItem<GeneralClassVectorDesignProperty<PropertyHolder, ElemType>>(funcname, propitem);
        }
        // Registers a property representing elements of a vector to be serialized.
        template<typename PropertyHolder, typename ElemType, typename GetType, typename SetType>
        static SerializeNativeVectorItem<GeneralNativeVectorDesignProperty<PropertyHolder, ElemType, GetType, SetType>>* CreateSerializeItem(const std::wstring& funcname, GeneralNativeVectorDesignProperty<PropertyHolder, ElemType, GetType, SetType> *propitem)
        {
            return new SerializeNativeVectorItem<GeneralNativeVectorDesignProperty<PropertyHolder, ElemType, GetType, SetType>>(funcname, propitem);
        }
        // Registers a property representing elements in an array to be serialized.
        template<typename PropertyHolder, typename ElemType>
        static SerializeArray<GeneralArrayDesignProperty<PropertyHolder, ElemType>>* CreateSerializeItem(const std::wstring& funcname, GeneralArrayDesignProperty<PropertyHolder, ElemType> *propitem)
        {
            return new SerializeArray<GeneralArrayDesignProperty<PropertyHolder, ElemType>>(funcname, propitem);
        }
        // Registers an event.
        template<typename EventHolder, typename EventType>
        static SerializeEvent<GeneralEventDesignProperty<EventHolder, EventType>>* CreateSerializeEvent(const std::wstring& eventname, GeneralEventDesignProperty<EventHolder, EventType> *eventitem)
        {
            return new SerializeEvent<GeneralEventDesignProperty<EventHolder, EventType>>(eventname, eventitem);
        }

        template<typename DesignPropertyType>
        class SerializeItem : public ISerializeItem
        {
        private:
            typedef ISerializeItem   base;

        protected:
            std::wstring funcname;
            DesignPropertyType* prop;
        public:
            SerializeItem(const std::wstring& funcname, DesignPropertyType* prop) : funcname(funcname), prop(prop) { }

            virtual bool Holds(DesignProperty *prop)
            {
                return this->prop == prop;
            }

            virtual bool MustSerialize(Object *propholder)
            {
                return prop->MustSerialize(propholder);
            }

            virtual bool MustExport(Object *propholder)
            {
                return prop->MustExport(propholder);
            }

            virtual const std::wstring& Name()
            {
                return funcname;
            }

            void SetName(const std::wstring &newname)
            {
                funcname = newname;
            }

            virtual void CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder)
            {
                int resnum = -1;
                if (prop->StoresBinaryResource(propholder))
                {
                    std::vector<byte> res;
                    prop->ResourceValue(propholder, res);
                    resnum = DesignerExportToResource(res);
                }

                if (prop->HasSerializerFunc())
                {
                    prop->CppExport(indent, prefix, pointerprefix, ws, propholder, prop, resnum);
                    return;
                }

                DoExport(indent, prefix, pointerprefix, ws, sevents, propholder, resnum);
            }

            virtual void DoExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder, int resnum)
            {
                if (funcname.length() == 0)
                    return;

                ws << indent << prefix;
                if (prefix.length())
                    ws << (pointerprefix ? L"->" : L".");

                ws << funcname << L"(" << (prop->StoresBinaryResource(propholder) ? prop->ResourceExportValue(propholder, resnum) : prop->ExportValue(propholder)) << L")";

                ws << L";" << std::endl;
            }

            void ExportNames(Indentation &indent, std::wiostream &ws, std::vector<std::pair<std::wstring, std::wstring>> names)
            {
                for (auto it = names.begin(); it != names.end(); ++it)
                    ws << indent << it->first << L" *" << it->second << L";" << std::endl;
            }

            virtual void DeclareSerialize(Indentation &indent, std::wiostream &ws, Object *propholder, AccessLevels access)
            {
                if (prop->ReferenceType() != pcrtFormDeclare)
                    return;
                std::vector<std::pair<std::wstring, std::wstring>> names;
                prop->DeclaredNames(names, propholder, access);
                ExportNames(indent, ws, names);
            }

            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
            {
                ws << indent << prop->Name() << L" = \"" << prop->InnerValue(propholder) << L"\"" << std::endl;
            }

            virtual bool Delayed()
            {
                return prop->Delayed();
            }
        };

        template<typename DesignPropertyType>
        class SerializeClassItem : public SerializeItem<DesignPropertyType>
        {
        private:
            typedef SerializeItem<DesignPropertyType> base;
        public:
            SerializeClassItem(const std::wstring& funcname, DesignPropertyType* prop) : base(funcname, prop) { }

            virtual bool MustSerialize(Object *propholder)
            {
                if (!base::MustSerialize(propholder))
                    return false;

                DesignSerializer *serializer = SerializerByTypeInfo(this->prop->ValueType());
                if (serializer == nullptr)
                    throw L"Cannot serialize an object with no serializable properties.";
                return serializer->MustSerialize(this->prop->GetPropertyValueAsPointer(propholder));
            }

            virtual void DoExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder, int resnum)
            {
                //if (this->funcname.length() == 0)
                //    return;

                DesignSerializer *serializer = SerializerByTypeInfo(this->prop->ValueType());
                if (serializer == nullptr)
                    throw L"Cannot serialize an object with no serializable properties.";

                std::wstringstream prefstr;
                prefstr << prefix;
                if (this->funcname.length())
                {
                    if (prefix.length())
                        prefstr << (pointerprefix ? L"->" : L".");
                    prefstr << this->funcname;
                    if (!this->prop->ParentCreated())
                        prefstr << L"()";
                }

                serializer->CppExport(indent, prefstr.str(), this->prop->IsPointerValue(), ws, ws, sevents, this->prop->GetPropertyValueAsPointer(propholder));
            }

            virtual void DeclareSerialize(Indentation &indent, std::wiostream &ws, Object *propholder, AccessLevels access)
            {
                DesignSerializer *serializer = SerializerByTypeInfo(this->prop->ValueType());
                if (serializer == nullptr)
                    throw L"Cannot serialize an object with no serializable properties.";

                serializer->DeclareSerialize(indent, ws, this->prop->GetPropertyValueAsPointer(propholder), access);
            }

            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
            {
                std::wstring name = this->prop->Name();
                if (name.length())
                    ws << indent << L"class " << name << std::endl;
                else
                    ws << indent << std::endl;
                ws << indent << L"{" << std::endl;

                indent++;

                DesignSerializer *serializer = SerializerByTypeInfo(this->prop->ValueType());
                if (serializer == nullptr)
                    throw L"Cannot serialize an object with no serializable properties.";

                serializer->Serialize(indent, ws, this->prop->GetPropertyValueAsPointer(propholder));

                indent--;

                ws << indent << L"}" << std::endl;
            }

            virtual bool Delayed()
            {
                return this->prop->Delayed();
            }
        };

        template<typename DesignPropertyType>
        class SerializeClassVectorItem : public SerializeItem<DesignPropertyType>
        {
        private:
            typedef SerializeItem<DesignPropertyType> base;
        public:
            SerializeClassVectorItem(const std::wstring& funcname, DesignPropertyType* prop) : base(funcname, prop) { }

            virtual bool MustSerialize(Object *propholder)
            {

                if (!base::MustSerialize(propholder) || this->prop->Count(propholder) == 0)
                    return false;

                return true;
            }

            virtual void DoExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder, int resnum)
            {
                //std::wstringstream prefstr;
                //prefstr << prefix;
                //if (prefix.length())
                //    prefstr << (pointerprefix ? L"->" : L".");

                int size = this->prop->Count(propholder);
                for (int ix = 0; ix < size; ++ix)
                {
                    typename DesignPropertyType::ElemPropertyType* itemprop = this->prop->ItemProperty(ix);
                    Object *elemobj = itemprop->GetPropertyValue(propholder);
                    std::wstring nameprefix;
                    this->prop->ConstructExport(indent, ws, propholder, elemobj, nameprefix);
                    ISerializeItem *sitem = CreateSerializeItem(elemobj->Name(), itemprop);
                    sitem->CppExport(indent, nameprefix, this->prop->IsPointerValue(), ws, sevents, propholder);
                    delete sitem;
                    delete itemprop;
                }
            }

            virtual void DeclareSerialize(Indentation &indent, std::wiostream &ws, Object *propholder, AccessLevels access)
            {
                if (this->prop->ReferenceType() != pcrtFormDeclare)
                    return;

                int size = this->prop->Count(propholder);
                for (int ix = 0; ix < size; ++ix)
                {
                    typename DesignPropertyType::ElemPropertyType* itemprop = this->prop->ItemProperty(ix);
                    Object *elemobj = itemprop->GetPropertyValue(propholder);

                    ISerializeItem *sitem = CreateSerializeItem(elemobj->Name(), itemprop);
                    sitem->DeclareSerialize(indent, ws, propholder, access);
                    delete sitem;
                    delete itemprop;
                }
            }

            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
            {
                ws << indent << L"vector" << L"<" << DisplayNameByTypeInfo(this->prop->ValueType(), true) << L"> " << this->prop->Name() << L" = { ";

                ++indent;

                int size = this->prop->Count(propholder);
                for (int ix = 0; ix < size; ++ix)
                {
                    typename DesignPropertyType::ElemPropertyType* itemprop = this->prop->ItemProperty(ix);
                    ISerializeItem *sitem = CreateSerializeItem(std::wstring(), itemprop);
                    sitem->Serialize(indent, ws, propholder);
                    delete sitem;
                    delete itemprop;

                    if (ix < size - 1)
                        ws << indent << L", ";
                    else
                        break;
                }

                --indent;

                ws << indent << L"}" << std::endl;
            }

            virtual bool Delayed()
            {
                return this->prop->Delayed();
            }
        };

        template<typename DesignPropertyType>
        class SerializeNativeVectorItem : public SerializeItem<DesignPropertyType>
        {
        private:
            typedef SerializeItem<DesignPropertyType> base;
        public:
            SerializeNativeVectorItem(const std::wstring& funcname, DesignPropertyType* prop) : base(funcname, prop) { }

            virtual bool MustSerialize(Object *propholder)
            {

                if (!base::MustSerialize(propholder) || this->prop->Count(propholder) == 0)
                    return false;

                return true;
            }

            virtual void DoExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder, int resnum)
            {
                if (this->funcname.length() == 0)
                    return;
                int size = this->prop->Count(propholder);
                for (int ix = 0; ix < size; ++ix)
                {
                    auto value = this->prop->Items(propholder, ix);

                    ws << indent << prefix;
                    if (prefix.length())
                        ws << (pointerprefix ? L"->" : L".");

                    ws << this->funcname << L"(L\"" << EscapeCString(value) << L"\");" << std::endl;
                }
            }

            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
            {
            
                ws << indent << L"vector" << L"<" << DisplayNameByTypeInfo(this->prop->ValueType(), true) << L"> " << this->prop->Name() << L" = { ";

                ++indent;

                int size = this->prop->Count(propholder);
                for (int ix = 0; ix < size; ++ix)
                {
                    auto value = this->prop->Items(propholder, ix);

                    ws << std::endl << indent << L"\"" << EscapeCString(this->prop->ValueToString(value)) << L"\"";
                    if (ix < size - 1)
                        ws << indent << L", ";
                    else
                        ws << std::endl;
                }

                --indent;

                ws << indent << L"}" << std::endl;
            }

            virtual bool Delayed()
            {
                return this->prop->Delayed();
            }
        };

        template<typename DesignPropertyType>
        class SerializeArray: public SerializeItem<DesignPropertyType>
        {
        private:
            typedef SerializeItem<DesignPropertyType> base;
        public:
            SerializeArray(const std::wstring& funcname, DesignPropertyType* prop) : base(funcname, prop) { }

            virtual bool MustSerialize(Object *propholder)
            {

                if (!base::MustSerialize(propholder))
                    return false;

                return true;
            }

            virtual void DoExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &sevents, Object *propholder, int resnum) override
            {
                if (this->funcname.length() == 0)
                    return;

                std::vector<int> indexes;
                this->prop->GetIndexes(propholder, indexes);

                for (auto it = indexes.begin(); it != indexes.end(); ++it)
                {
                    int index = *it;
                    ws << indent << prefix;
                    if (prefix.length())
                        ws << (pointerprefix ? L"->" : L".");

                    ws << this->funcname << L"(" << IntToStr(index) << L", " <<  this->prop->ExportValue(propholder, index) << L")";

                    ws << L";" << std::endl;
                }
            }

            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
            {
                std::vector<int> indexes;
                this->prop->GetIndexes(propholder, indexes);
                if (indexes.empty())
                    return;
                ws << indent << L"array " << this->prop->Name() << L" = {" << std::endl;
                ++indent;
                for (auto it = indexes.begin(); it != indexes.end(); ++it)
                    ws << indent << L"[" << IntToStr(*it) << L"] = \"" << this->prop->Value(propholder, *it) << L"\"" << std::endl;
                --indent;
                ws << indent << L"}" << std::endl;
            }

            virtual bool Delayed()
            {
                return this->prop->Delayed();
            }
        };

        template<typename DesignPropertyType>
        class SerializeEvent : public ISerializeEvent
        {
        private:
            typedef ISerializeEvent   base;

        protected:
            std::wstring eventname;
            DesignPropertyType* prop;
        public:
            SerializeEvent(const std::wstring& eventname, DesignPropertyType* prop) : eventname(eventname), prop(prop) { }

            virtual bool Holds(DesignProperty *prop)
            {
                return this->prop == prop;
            }

            virtual const std::wstring& Type()
            {
                return prop->Type();
            }

            virtual bool MustSerialize(Object *propholder)
            {
                return prop->MustSerialize(propholder);
            }

            virtual bool MustExport(Object *propholder)
            {
                return prop->MustExport(propholder);
            }

            virtual void CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder)
            {
                if (eventname.length() == 0)
                    return;

                ws << indent << prefix;
                if (prefix.length())
                    ws << (pointerprefix ? L"->" : L".");

                ws << eventname << L" = CreateEvent(this, &T" << (propholder->ParentForm() ? SerializerFormName(propholder->ParentForm()) : propholder->Name()) << L"::" << prop->ExportValue(propholder) << L")";
                ws << L";" << std::endl;
            }

            virtual void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
            {
                ws << indent << L"event " << prop->Name() << L" = \"" << prop->InnerValue(propholder) << L"\"" << std::endl;
            }

            virtual bool Delayed()
            {
                return prop->Delayed();
            }
        };

        typedef Object* (*CreateFunction)(Object*);
        CreateFunction createfunc;

    public:
        DesignSerializer(); // Displayable name of the owner of the properties serialized. This is used when exporting to cpp files and when creating the project file.
        ~DesignSerializer();

        Object* CreateObject(Object *owner);
        void SetCreateFunction(CreateFunction newcreatefunc);

        int PropertiesIndex(int index, DesignPropertyUsageSet condition); // Returns the real index of a property, given an index among those which satisfy the condition.
        int EventsIndex(int index, DesignPropertyUsageSet condition); // Returns the real index of a property, given an index among those which satisfy the condition.

        DesignProperty* Properties(int index);
        int PropertyCount(DesignPropertyUsageSet condition);
        DesignProperty* Events(int index);
        int EventCount(DesignPropertyUsageSet condition);
        const std::wstring& Names(int index); // The name of the function passed when adding new properties. i.e. SetName for property Name.
        const std::wstring& EventParam(int index);
        void RenameSerializer(int index, const std::wstring &newname);

        // Return whether the there are any properties within this property collection which must be serialized.
        bool MustSerialize(Object *propholder);

        int PropertyIndex(DesignProperty *prop);
        // Return the index of a property with the passed name. Only non hidden properties are considered if the specified condition is zero, otherwise one of the conditions must be met. Returns -1 if the property is not found.
        int PropertyIndex(const std::wstring& name, DesignPropertyUsageSet condition = 0);
        int EventIndex(const std::wstring& name, DesignPropertyUsageSet condition = 0);
    
        // Returns the true property index of a property by name, but only if it is the property type of the template parameter.
        template<typename PropertyType>
        int PropertyIndex(const std::wstring& name, DesignPropertyUsageSet condition = 0)
        {
            for (int ix = properties.size() - 1; ix >= 0; --ix)
            {
                DesignProperty *prop = properties[ix];
                if (prop->Name() == name)
                {
                    if (!dynamic_cast<PropertyType*>(prop) || (!condition.empty() && !condition.contains(dpuHidden) && (!condition.contains(dpuListed) || !prop->IsListed()) && (!condition.contains(dpuSerialized) || !prop->IsSerialized()) && (!condition.contains(dpuExported) || !prop->IsExported()) && (!condition.contains(dpuDerived) || !prop->IsDerived())) || 
                        (condition.empty() && !prop->IsListed() && !prop->IsSerialized() && !prop->IsExported() && !prop->IsDerived()) || 
                        (condition.contains(dpuHidden) && (prop->IsListed() || prop->IsSerialized() || prop->IsExported() || prop->IsDerived())))
                        break;
                    return ix;
                }
            }
            return -1;
        }

        template<typename EventType>
        int EventIndex(const std::wstring& name, DesignPropertyUsageSet condition = 0)
        {
            for (int ix = events.size() - 1; ix >= 0; --ix)
            {
                DesignProperty *prop = events[ix];
                if (prop->Name() == name)
                {
                    if (!dynamic_cast<EventType*>(prop) || (!condition.empty() && !condition.contains(dpuHidden) && (!condition.contains(dpuListed) || !prop->IsListed()) && (!condition.contains(dpuSerialized) || !prop->IsSerialized()) && (!condition.contains(dpuExported) || !prop->IsExported())) ||
                        (condition.empty() && !prop->IsListed() && !prop->IsSerialized() && !prop->IsExported() && !prop->IsDerived()) || 
                        (condition.contains(dpuHidden) && (prop->IsListed() || prop->IsSerialized() || prop->IsExported() || prop->IsDerived())))
                        break;
                    return ix;
                }
            }
            return -1;
        }

        // Hides the last property with the given name or index, so it won't show up in the property editor unless a new property with the same name is added. There is no need to hide a property if it a derived class adds a new one with the same name.
        void HideProperty(const std::wstring& name);
        void HideEvent(const std::wstring& name);

        // Forces a property with the given name to be serialized after all other items that were added before the call.
        void MoveHere(const std::wstring& name);


        // Hides all properties added till now to the serializer.
        void HideAll();
        // Get the type_info of the property creator and the property value.
        const type_info& PropertyCreatorType(int index);
        const type_info& PropertyValueType(int index);
        const type_info& EventCreatorType(int index);
        const type_info& EventValueType(int index);

        bool ContainerControl(); // This is the serializer for a control which can act as a container for other controls.
        bool NonVisualContainerControl(); // This is the serializer for a form or container which holds non-visual controls.
        bool ParentFor(const type_info &type); // Called when a control is being placed (pasted etc.) on the control of the serializer. Returns whether this is allowed.

        void SetContainerControl(bool iscontainer);
        void SetNonVisualContainerControl(bool isnvcontainer);
        void AllowSubType(const type_info &type);
        void DisallowSubType(const type_info &type);
        void ClearAllowSubTypes();

        // Write the string representation of all properties of this serializer to the stream of the project file or the cpp/h files.
        void ConstructExport(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents, Object *parent, Object *control, DesignProperty *prop); // Adds the constructor line of the control depending on the passed property, then calls CppExport() to serialize the control's properties. If prop is null, the default behavior is to create the control with controlname = new ControlType(), and write SetParent() after serialization.
        void CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &wsdelayed, std::wiostream &wsevents, Object *propholder); // As C++ code.
        void Serialize(Indentation &indent, std::wiostream &ws, Object *propholder); // As code to copy and paste.
        void DeclareSerialize(Indentation &indent, std::wiostream &stream, Object *control, AccessLevels access);
        //void CollectEvents(std::vector<std::pair<void*, std::wstring>> &events, Object *control);

        // The default property for a control is the property that activates in the editor, if the control is double-clicked.
        // Setting the default before the designer is shown updates the defaultproperty value. After it the userdefaultproperty is updated, to differentiate the user choice.
        void MakeDefault(int index); // Sets the default property by index.
        void MakeDefault(const std::wstring &propname); // Sets the default property by name.
        void MakeDefault(DesignProperty *prop); // Sets the default property by its pointer.
        void ClearDefault(); // Clears the default property.
        DesignProperty* DefaultProperty(); // Returns the default property which was set with MakeDefault. If the property is not listed in the editor, this returns null.
        std::wstring UserDefaultProperty(); // Returns the name of the property, which was selected in the editor by the user. If there is no property set or this matches the original default, an empty string is returned.

        DesignProperty* Find(const std::wstring &name, DesignPropertyUsageSet condition = 0);
    
        template<typename PropertyType>
        PropertyType* Find(const std::wstring &name, DesignPropertyUsageSet condition = 0)
        {
            int ix = PropertyIndex<PropertyType>(name, condition);
            if (ix < 0)
                return NULL;

            return dynamic_cast<PropertyType*>(properties[ix]);
        }

        void DeletePropertySerializer(DesignProperty *prop)
        {
            if (defaultproperty == prop)
                defaultproperty = NULL;

            for (auto it = serializers.begin(); it != serializers.end(); ++it)
            {
                if ((*it)->Holds(prop))
                {
                    delete *it;
                    serializers.erase(it);
                    break;
                }
            }
        }

        void DeleteEventSerializer(DesignProperty *prop)
        {
            for (auto it = eventserializers.begin(); it != eventserializers.end(); ++it)
            {
                if ((*it)->Holds(prop))
                {
                    delete *it;
                    eventserializers.erase(it);
                    break;
                }
            }
        }

        // Registers a property to be serialized, by using the passed setter function.
        template<typename PropertyHolder, typename PropertyType>
        GeneralDesignProperty<PropertyHolder, PropertyType>* Add(const std::wstring& funcname, GeneralDesignProperty<PropertyHolder, PropertyType> *propitem)
        {
            DesignProperty *deleted = properties.AddProperty(propitem);
            ISerializeItem *item = CreateSerializeItem(funcname, propitem);
            serializers.push_back(item);
            if (deleted)
                DeletePropertySerializer(deleted);

            return propitem;
        }

        // Registers a property representing an instance of some class to be serialized by using the passed getter function to get the instance, and its serializer to serialize the object.
        template<typename PropertyType, typename ClassType>
        GeneralClassDesignProperty<PropertyType, ClassType>* Add(const std::wstring& funcname, GeneralClassDesignProperty<PropertyType, ClassType> *propitem)
        {
            DesignProperty *deleted = properties.AddProperty(propitem);
            ISerializeItem *item = CreateSerializeItem(funcname, propitem);
            serializers.push_back(item);
            if (deleted)
                DeletePropertySerializer(deleted);

            return propitem;
        }

        // Registers a property representing elements of a vector to be serialized.
        template<typename PropertyType, typename ElemType>
        GeneralClassVectorDesignProperty<PropertyType, ElemType>* Add(const std::wstring& funcname, GeneralClassVectorDesignProperty<PropertyType, ElemType> *propitem)
        {
            DesignProperty *deleted = properties.AddProperty(propitem);
            ISerializeItem *item = CreateSerializeItem(funcname, propitem);
            serializers.push_back(item);
            if (deleted)
                DeletePropertySerializer(deleted);

            return propitem;
        }

        // Registers a property representing elements of a vector to be serialized.
        template<typename PropertyType, typename ElemType, typename GetType, typename SetType>
        GeneralNativeVectorDesignProperty<PropertyType, ElemType, GetType, SetType>* Add(const std::wstring& funcname, GeneralNativeVectorDesignProperty<PropertyType, ElemType, GetType, SetType> *propitem)
        {
            DesignProperty *deleted = properties.AddProperty(propitem);
            ISerializeItem *item = CreateSerializeItem(funcname, propitem);
            serializers.push_back(item);
            if (deleted)
                DeletePropertySerializer(deleted);

            return propitem;
        }

        template<typename PropertyType, typename ElemType>
        GeneralArrayDesignProperty<PropertyType, ElemType>* Add(const std::wstring& funcname, GeneralArrayDesignProperty<PropertyType, ElemType> *propitem)
        {
            DesignProperty *deleted = properties.AddProperty(propitem);
            ISerializeItem *item = CreateSerializeItem(funcname, propitem);
            serializers.push_back(item);
            if (deleted)
                DeletePropertySerializer(deleted);

            return propitem;
        }

        // Registers an event property used for object events.
        template<typename HolderType, typename EventType>
        void AddEvent(const std::wstring& eventname, const std::wstring& category = std::wstring())
        {
            auto eventitem = new EventDesignProperty<HolderType, EventType>(eventname, category);
            DesignProperty *deleted = events.AddProperty(eventitem);
            ISerializeEvent *item = CreateSerializeEvent(eventname, eventitem);
            eventserializers.push_back(item);
            if (deleted)
                DeleteEventSerializer(deleted);
        }
    };



    // Deserialize passed stream into a list of strings which can be evaluated as properties, types and classes. Returns true if successful. Call FreeDeserializerList when returned items are processed. Set containertype to "Form" or "Container" if the stream starts with a form or container member listing with appropriate curly brackets. Set containertype to "Project" for reading project properties and stopping at the first type.
    void DeserializeFromStream(Object *parent, TokenStream &token, std::vector<ControlDeserializerItem*> &items, std::wstring containertype = std::wstring());
    // Call to free the items returned by DeserializeFromStream after they are processed.
    void FreeDeserializerList(std::vector<ControlDeserializerItem*> &items);


}
/* End of NLIBNS */


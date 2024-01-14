#include "stdafx_zoli.h"

//#include <regex>
#include "designproperties.h"
#include "designerform.h"
#include "serializer.h"
#include "application.h"
#include "property_objectbase.h"

//---------------------------------------------


namespace NLIBNS
{


ControlDeserializerItem::~ControlDeserializerItem()
{
     for (auto it = properties.begin(); it != properties.end(); ++it)
         delete *it;
}


//---------------------------------------------


ControlDeserializerPropertyNameValue::ControlDeserializerPropertyNameValue(const std::wstring &name, const std::wstring &value, DesignProperty *prop) : base(prop, false, cdptSimple), 
        data(std::pair<std::wstring, std::wstring>(name, value))
{
}

ControlDeserializerPropertyNameValue::~ControlDeserializerPropertyNameValue()
{
}

const std::wstring& ControlDeserializerPropertyNameValue::Name()
{
    return data.first;
}

void ControlDeserializerPropertyNameValue::SetValue(Object *propholder)
{
    prop->SetInnerValue(NULL, propholder, data.second);
}

const std::wstring& ControlDeserializerPropertyNameValue::Value()
{
    return data.second;
}


//---------------------------------------------


ControlDeserializerPropertyCollection::ControlDeserializerPropertyCollection(const std::wstring &name, DesignSerializer *serializer, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type) : base(prop, freeprop, type), name(name), serializer(serializer)
{
}

ControlDeserializerPropertyCollection::~ControlDeserializerPropertyCollection()
{
}

const std::wstring& ControlDeserializerPropertyCollection::Name()
{
    return name;
}


//---------------------------------------------


ControlDeserializerPropertyArray::ControlDeserializerPropertyArray(const std::wstring &name, DesignSerializer *serializer, DesignProperty *prop, bool freeprop) : base(name, serializer, prop, freeprop, cdptArray)
{
}

ControlDeserializerPropertyArray::~ControlDeserializerPropertyArray()
{
}

void ControlDeserializerPropertyArray::AddValue(unsigned int index, const std::wstring &value)
{
    if (values.size() <= index)
        values.resize(index + 1);
    values[index] = value;
}

void ControlDeserializerPropertyArray::SetValue(Object *propholder)
{
    for (auto it = values.sbegin(); it != values.send(); ++it)
    {
        prop->SetInnerValue(NULL, propholder, it.base() - values.begin(), *it);
    }
}


//---------------------------------------------


ControlDeserializerPropertyList::ControlDeserializerPropertyList(const std::wstring &name, DesignSerializer *serializer, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type) : base(name, serializer, prop, freeprop, type)
{
}

ControlDeserializerPropertyList::~ControlDeserializerPropertyList()
{
    for (auto item : properties)
        delete item;
}

void ControlDeserializerPropertyList::SetValue(Object *propholder)
{
    Object *subpropholder = prop->SubHolder(propholder);

    if (!Name().empty())
    {
        auto nameprop = serializer->Find(L"Name");
        if (nameprop != nullptr)
            nameprop->SetValue(nullptr, subpropholder, Name());
        //else
        //    ShowMessageBox(L"Error in deserializer data: Name property value for object without a name property. Skipping...", L"Error", mbOk);
    }

    for (auto item : properties)
    {
        item->SetValue(subpropholder);
    }
}


//---------------------------------------------


GeneralControlDeserializerPropertyVector::GeneralControlDeserializerPropertyVector(const std::wstring &name, const type_info &elemtype, DesignSerializer *serializer, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type) : base(name, serializer, prop, freeprop, type), elemtype(elemtype)
{
}

GeneralControlDeserializerPropertyVector::~GeneralControlDeserializerPropertyVector()
{
}


//---------------------------------------------


ControlDeserializerPropertyClassVector::ControlDeserializerPropertyClassVector(const std::wstring &name, const type_info &elemtype, DesignSerializer *serializer, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type) : base(name, elemtype, serializer, prop, freeprop, type)
{
}

ControlDeserializerPropertyClassVector::~ControlDeserializerPropertyClassVector()
{
}

void ControlDeserializerPropertyClassVector::SetValue(Object *propholder)
{
    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        //if (ObjectTypeValues[elemtype] == otNative) // Native data types.
        //    prop->AddItem(propholder, (dynamic_cast<ControlDeserializerPropertyNameValue*>(*it))->Value());
        //else // Classes and sub controls.
        //{
            prop->AddItem(propholder);
            (*it)->SetValue(prop->SubHolder(propholder));
        //}
    }
}


//---------------------------------------------


ControlDeserializerPropertyNativeVector::ControlDeserializerPropertyNativeVector(const std::wstring &name, const type_info &elemtype, DesignProperty *prop, bool freeprop, ControlDeserializerPropertyType type) : base(name, elemtype, NULL, prop, freeprop, type)
{
}

ControlDeserializerPropertyNativeVector::~ControlDeserializerPropertyNativeVector()
{
}

void ControlDeserializerPropertyNativeVector::SetValue(Object *propholder)
{
    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        //if (ObjectTypeValues[elemtype] == otNative) // Native data types.
            prop->AddItem(propholder, (dynamic_cast<ControlDeserializerPropertyNameValue*>(*it))->Value());
        //else // Classes and sub controls.
        //{
            //prop->AddItem(propholder);
            //(*it)->SetValue(prop->SubHolder(propholder));
        //}
    }
}


//---------------------------------------------

/*
bool CutTo2(const std::wstring &str, std::wstring &str1, std::wstring &str2)
{
    int pos = 0;
    int wordlen = 0;
    int len = str.length();
    bool eqpassed = false;
    bool inquotes = false;
    int wrd = 0;
    while (pos < len)
    {
        while (pos < len && !inquotes && (str[pos] == L' ' || str[pos] == L'\t' || (eqpassed && str[pos] == L'"') || (wrd == 1 && str[pos] == L'=')))
        {
            if (str[pos] == L'=')
            {
                if (eqpassed)
                    return false;
                eqpassed = true;
            }
            else if (str[pos] == L'"')
                inquotes = true;

            pos++;
        }
        if (((wrd == 0 || !eqpassed) && (str[pos] < L'a' || str[pos] > L'z') && (str[pos] < L'A' || str[pos] > L'Z') && wrd == 0 && str[pos] != L'{' && str[pos] != L'}') || (eqpassed && !inquotes))
            return false;

        if (inquotes)
        {
            wordlen = len - pos - 1;

            while (wordlen >= 0 && (str[pos + wordlen] == L' ' || str[pos + wordlen] == L'\t'))
                wordlen--;

            if (wordlen < 0 || str[pos + wordlen] != L'"')
                return false;
            str2 = str.substr(pos, wordlen);
            return true;
        }

        wordlen = 1;
        while (pos + wordlen < len && str[pos + wordlen] != L' ' && str[pos + wordlen] != L'\t' && str[pos + wordlen] != L'=')
            wordlen++;
        eqpassed = pos + wordlen < len && str[pos + wordlen] == L'=';

        if (wrd == 0)
            str1 = str.substr(pos, wordlen);
        else if (wrd == 1)
            str2 = str.substr(pos, wordlen);
        else
            return false;

        pos += wordlen;
        wrd++;
    }
    if (wrd == 1)
        str2 = L"";

    return wrd > 0;
}
*/

void DeserializeFromStream(Object *parent, TokenStream &token, std::vector<ControlDeserializerItem*> &items, std::wstring containertype)
{
    ControlDeserializerItem *cd = NULL;
    ControlDeserializerPropertyList *pc = NULL;
    std::vector<ControlDeserializerItem*> itemstack;
    std::vector<ControlDeserializerPropertyCollection*> propstack;

    std::vector<std::pair<std::wstring, std::wstring>> eventfunc; // Vector of pair of event function values and their supposed type string. Used in checking whether the same function is used in incompatible events.

    //std::wstring str;
    //std::wstring str1;
    //std::wstring str2;

    bool prevtype = false; // Previous was a "type" token and name.
    bool prevsub = false; // Previous was a "subtype" token and property::type name.
    bool prevclass = false; // Previous was a "class" token and name.
    bool prevvector = false; // Previous line was a "vector" token, list elem type and name, followed by a = sign.
    bool prevarray = false; // Previous line was an "array" token followed by a = sign.
    bool inarray = false; // Listing elements of an array between curly brackets.
    bool elemnext = false;  // When listing elements of a vector and either the previous token was the vector opening bracket, or a comma marking another elem in the vector.
    bool neednameprop = false; // The property must have a name specified for it and it must be the first one in the property list.
    bool firstpropfound = false; // The next should be the first property of the given object, and it must specify the design name of the control and the name of the variable to be created.
    bool afteritem = false; // We have encountered a type inside another. This is true after finishing with a type, but set to false once a new type is started. It is an error to have this as true and new properties appear, because property values cannot come after the types.
    bool proponly = containertype == L"Project"; // Only deserialize properties and stop when something else found, "putting back" the last token to the stream (like it was only peeked).

    ReadTokenType rtt;

    bool container = !containertype.empty();
    if (container && (token.peek(rtt) != L"{" || rtt != rttToken)) // Opening bracket for form or container.
        throw TokenE(L"Missing \"{\"", token);

    try
    {
        while (container || token.read())
        {
            if (container || (token == rttUnquoted && token == L"type"))
            {
                if (!container && proponly) // No need to read types for the project.
                    break;
                if (prevtype) 
                    throw TokenE(L"Keyword \"type\" twice. Expecting \"{\"", token);
                if (prevsub) 
                    throw TokenE(L"Keyword \"type\' after \"subtype\". Expecting \"{\"", token);
                if (prevclass)
                    throw TokenE(L"Keyword \"type\" after \"class\". Expecting \"{\"", token);
                if (prevvector)
                    throw TokenE(L"Keyword \"type\" after \"vector\". Expecting \"{\"", token);
                if (prevarray)
                    throw TokenE(L"Keyword \"type\" after \"array\". Expecting \"{\"", token);
                if (inarray)
                    throw TokenE(L"Keyword \"type\" in array list", token);
                if (elemnext)
                    throw TokenE(L"Keyword \"type\" found when expecting \"vector\" elem.", token);
                if (!firstpropfound && !items.empty())
                    throw TokenE(L"Keyword \"type\" found before the \"Name\" property", token);

                if (pc)
                    throw TokenE(L"Keyword \"type\" found inside class property list", token);

                if (!container)
                {
                    token.read(rtmFullNamespace);
                    if (token != rttUnquoted)
                      throw TokenE(L"Keyword \"type\" must be followed with an unquoted control type name", token);
                }

                if (!container && token.empty())
                    throw TokenE(L"Unexpected end of file after keyword \"type\"", token);

                std::wstring dispname = container ? containertype : token.toString();
                const type_info *_type;
                try
                {
                    _type = &TypeInfoByDisplayName(dispname, true);
                }
                catch(...)
                {
                    throw TokenE(L"Unknown control type", token);
                }
                const type_info &type = *_type;

                if (!container && ObjectTypeByTypeInfo(type) == otTopLevel)
                    throw TokenE(L"Top level object cannot be placed inside controls.", token);

                container = false;

                //if (type == ctUnknown)
                //    throw TokenE(L"Unknown control type", token);

                if (ObjectTypeByTypeInfo(type) == otVisual && ((cd && !cd->serializer->ContainerControl()) || (!cd && parent && !parent->Serializer()->ContainerControl())))
                    throw TokenE(L"Control doesn't allow child controls", token);

                if ((ObjectTypeByTypeInfo(type) == otSubControl && !cd && !parent) || ((cd || parent) && !(!cd ? parent->Serializer() : cd->serializer)->ParentFor(TypeInfoByDisplayName(dispname, true))))
                    throw TokenE(L"Type of control not allowed here", token);

                cd = new ControlDeserializerItem(type, itemstack.empty() ? NULL : itemstack.back() );
                items.push_back(cd);
                itemstack.push_back(cd);
                firstpropfound = false;

                if (!proponly)
                {
                    DesignSerializer *serializer = SerializerByDisplayName(token.toString(), true);
                    if (!serializer)
                        throw TokenE(L"Invalid type", token);
                    auto nameprop = serializer->Find(L"Name");
                    neednameprop = nameprop != nullptr && nameprop->IsSerialized();
                }
                else
                    neednameprop = false;
                firstpropfound = !neednameprop;
                afteritem = false;
                prevtype = true;
                continue;
            }

            if (token == rttUnquoted && token == L"subtype")
            {
                if (proponly) // No need to read subtypes for property only objects.
                    break;
                if (!cd && !parent)
                    throw TokenE(L"Keyword \"subtype\" found outside control", token);
                if (prevsub) 
                    throw TokenE(L"Keyword \"subtype\" twice. Expecting \"{\"", token);
                if (prevtype)
                    throw TokenE(L"Keyword \"subtype\" after \"type\". Expecting \"{\"", token);
                if (prevclass)
                    throw TokenE(L"Keyword \"subtype\" after \"class\". Expecting \"{\"", token);
                if (prevvector)
                    throw TokenE(L"Keyword \"subtype\" after \"vector\". Expecting \"{\"", token);
                if (prevarray)
                    throw TokenE(L"Keyword \"subtype\" after \"array\". Expecting \"{\"", token);
                if (inarray)
                    throw TokenE(L"Keyword \"subtype\" in array list", token);
                if (elemnext)
                    throw TokenE(L"Keyword \"subtype\" found when expecting \"vector\" elem.", token);
                if (!firstpropfound && !items.empty())
                    throw TokenE(L"Keyword \"subtype\" found before the \"Name\" property", token);

                if (pc)
                    throw TokenE(L"Keyword \"subtype\" found inside class property list", token);

                token.read();
                if (token != rttUnquoted)
                    throw TokenE(L"Keyword \"subtype\" must be followed with an unquoted property name", token);
                if (token.empty())
                    throw TokenE(L"Unexpected end of file after keyword \"subtype\"", token);

                DesignSerializer *serializer = cd ? cd->serializer : parent->Serializer();
                if (!serializer)
                    throw TokenE(L"Trying to add a sub type to a control with no serialization capabilities", token);
                std::wstring propname = token.toString();
                DesignProperty *prop = serializer->Find(propname);

                if (!prop)
                    throw TokenE(L"Property name not valid for the current control", token);

                ReadTokenType ptype;
                if ((token.read() && (token != rttToken || token != L":")) || token.peek(ptype, rtmFullNamespace).empty() || (ptype == rttToken && token.peek() != L":") || (ptype != rttToken && ptype != rttUnquoted))
                    throw TokenE(L"Subtype property name must be followed by one or two colons", token);
                if (ptype == rttToken)
                    token.read();
                if (token.empty())
                    throw TokenE(L"Unexpected end of file after subtype property name", token);

                token.read(rtmFullNamespace);
                if (token != rttUnquoted)
                    throw TokenE(L"Keyword \"subtype property_name::\" must be followed by a type name", token);
                if (token.empty())
                    throw TokenE(L"Unexpected end of file after subtype property name", token);

                std::wstring dispname = container ? containertype : token.toString();
                const type_info *_type;
                try
                {
                    _type = &TypeInfoByDisplayName(dispname, true);
                }
                catch(...)
                {
                    throw TokenE(L"Unknown control type", token);
                }
                const type_info &type = *_type;
                container = false;

                if (ObjectTypeByTypeInfo(type) == otVisual && !serializer->ContainerControl())
                    throw TokenE(L"Control doesn't allow child controls", token);

                if (ObjectTypeByTypeInfo(type) != otSubControl || !serializer->ParentFor(type))
                    throw TokenE(L"Type of control not allowed here", token);

                if (cd && cd != itemstack.back())
                    throw L"Non correct stack";
                cd = new ControlDeserializerItem(type, cd ? cd : NULL, prop);
                items.push_back(cd);
                itemstack.push_back(cd);
                firstpropfound = false;

                serializer = SerializerByTypeInfo(type);
                if (!serializer)
                    throw TokenE(L"Invalid type", token);
                auto nameprop = serializer->Find(L"Name");
                neednameprop = nameprop != nullptr && nameprop->IsSerialized();
                //neednameprop = true;

                firstpropfound = !neednameprop;
                afteritem = false;
                prevsub = true;
                continue;
            }

            if (token == rttUnquoted && token == L"class")
            {
                if (proponly) // No need to read classes for property only objects.
                    break;
                if (!cd)
                    throw TokenE(L"Keyword \"class\" found outside control member list", token);
                if (prevclass)
                    throw TokenE(L"Keyword \"class\" twice. Expecting \"{\"", token);
                if (prevtype)
                    throw TokenE(L"Keyword \"class\" after \"type\". Expecting \"{\"", token);
                if (prevsub)
                    throw TokenE(L"Keyword \"class\" after \"subtype\". Expecting \"{\"", token);
                if (prevvector)
                    throw TokenE(L"Keyword \"class\" after \"vector\". Expecting \"{\"", token);
                if (prevarray)
                    throw TokenE(L"Keyword \"class\" after \"array\". Expecting \"{\"", token);
                if (inarray)
                    throw TokenE(L"Keyword \"class\" in array list", token);
                if (elemnext)
                    throw TokenE(L"Keyword \"class\" found when expecting \"vector\" elem.", token);
                if (!firstpropfound)
                    throw TokenE(L"Keyword \"class\" found before the \"Name\" property", token);
                if (afteritem)
                    throw TokenE(L"Keyword \"class\" found after control member list", token);

                if (!token.read(rtmFullNamespace))
                    throw TokenE(L"Unexpected end of file after keyword \"class\"", token);
                else if (token != rttUnquoted)
                    throw TokenE(L"Keyword \"class\" must be followed with an unquoted class name", token);

                std::wstring cname = token.toString();

                int propix = (pc ? pc->serializer : cd->serializer)->PropertyIndex(cname, dpuSerialized);
#ifndef SKIPERROR
                if (propix < 0)
                    throw TokenE(L"Invalid property name in class", token);

                const type_info &type = (pc ? pc->serializer : cd->serializer)->PropertyValueType(propix);
                ObjectTypes proptype = ObjectTypeByTypeInfo(type);
                if (proptype != otClass)
                    throw TokenE(L"Property type incorrectly marked as class.", token);

                DesignSerializer *serializer = SerializerByTypeInfo(type);
                ControlDeserializerPropertyList *prop = new ControlDeserializerPropertyList(cname, serializer, (pc ? pc->serializer : cd->serializer)->Properties(propix));
#else
                ControlDeserializerPropertyList *prop = new ControlDeserializerPropertyList(); // Constructor used, when property is invalid and must be deleted later.
#endif
                if (pc)
                {
                    pc->properties.push_back(prop);
                    prop->pos = --pc->properties.end();
                }
                else
                {
                    cd->properties.push_back(prop);
                    prop->pos = --cd->properties.end();
                }
                propstack.push_back(prop);
                pc = prop;

                neednameprop = false;
                firstpropfound = true;
                prevclass = true;
                continue;
            }

            if (token == rttUnquoted && token == L"vector")
            {
                if (proponly) // No need to read vectors for property only objects.
                    break;
                if (!cd)
                    throw TokenE(L"Keyword \"vector\" found outside control member list", token);
                if (prevvector)
                    throw TokenE(L"Keyword \"vector\" twice", token);
                if (prevtype)
                    throw TokenE(L"Keyword \"vector\" after \"type\"", token);
                if (prevsub)
                    throw TokenE(L"Keyword \"vector\" after \"subtype\"", token);
                if (prevclass)
                    throw TokenE(L"Keyword \"vector\" after \"class\"", token);
                if (prevarray)
                    throw TokenE(L"Keyword \"vector\" after \"array\"", token);
                if (inarray)
                    throw TokenE(L"Keyword \"vector\" in array list", token);
                if (elemnext)
                    throw TokenE(L"Keyword \"vector\" found when expecting \"vector\" elem.", token);
                if (!firstpropfound)
                    throw TokenE(L"Keyword \"vector\" found before the \"Name\" property", token);
                if (afteritem)
                    throw TokenE(L"Keyword \"vector\" found after control member list", token);

                if (!token.read(rtmFullNamespace))
                    throw TokenE(L"Unexpected end of file after keyword \"vector\"", token);
                else if (token != rttTemplateArg)
                    throw TokenE(L"Keyword \"vector\" must be followed by a vector elem type specifier between \"<\" and \">\" characters", token);

                std::wstring elemname = token.toString();

                const type_info *_elemtype;
                try
                {
                    _elemtype = &TypeInfoByDisplayName(elemname, true);
                }
                catch(...)
                {
                    throw TokenE(L"Unrecognized vector elem type", token);
                }
                const type_info &elemtype = *_elemtype;

                auto ot = ObjectTypeByTypeInfo(elemtype);
                if (ot != otNative && ot != otClass && ot != otSubControl && ot != otVector)
                    throw TokenE(L"Only sub types and native types can be stored in a vector", token);

                if (!token.read())
                    throw TokenE(L"Unexpected end of file after \"vector\" and elem specifier", token);
                else if (token != rttUnquoted)
                    throw TokenE(L"Keyword \"vector\" and elem specifier must be followed by a name for the property", token);
                std::wstring vname = token.toString();

                if (!token.read())
                    throw TokenE(L"Unexpected end of file after vector type name", token);
                if (token != rttToken || token != L"=")
                    throw TokenE(L"Expected \"=\" not found after vector type name", token);

                int propix = (pc ? pc->serializer : cd->serializer)->PropertyIndex(vname, dpuSerialized);

                ControlDeserializerPropertyList *prop;
#ifndef SKIPERROR
                if (propix < 0)
                    throw TokenE(L"Invalid property name in class", token);
                
                if (ot != otNative)
                {
                    DesignSerializer *serializer = SerializerByTypeInfo((pc ? pc->serializer : cd->serializer)->PropertyValueType(propix));
                    prop = new ControlDeserializerPropertyClassVector(vname, elemtype, serializer, (pc ? pc->serializer : cd->serializer)->Properties(propix));
                }
                else
                    prop = new ControlDeserializerPropertyNativeVector(vname, elemtype, (pc ? pc->serializer : cd->serializer)->Properties(propix));
#else
                // SKIPERROR not implemented, when implementing, after creating the property we have to find the last closing bracket belonging to it then going to the next token.
                ControlDeserializerPropertyVector *prop = new ControlDeserializerPropertyVector(); // Constructor used, when property is invalid and must be deleted later.
#endif

                if (pc)
                {
                    pc->properties.push_back(prop);
                    prop->pos = --pc->properties.end();
                }
                else
                {
                    cd->properties.push_back(prop);
                    prop->pos = --cd->properties.end();
                }
                propstack.push_back(prop);
                pc = prop;

                neednameprop = false;
                firstpropfound = true;
                prevvector = true;
                continue;
            }

            if (token == rttUnquoted && token == L"array")
            {
                if (proponly) // No need to read arrays for property only objects.
                    break;
                if (!cd)
                    throw TokenE(L"Keyword \"array\" found outside control member list", token);
                if (prevarray)
                    throw TokenE(L"Keyword \"array\" twice", token);
                if (inarray)
                    throw TokenE(L"Keyword \"array\" in array list", token);
                if (prevvector)
                    throw TokenE(L"Keyword \"array\" after \"vector\"", token);
                if (prevtype)
                    throw TokenE(L"Keyword \"array\" after \"type\"", token);
                if (prevsub)
                    throw TokenE(L"Keyword \"array\" after \"subtype\"", token);
                if (prevclass)
                    throw TokenE(L"Keyword \"array\" after \"class\"", token);
                if (elemnext)
                    throw TokenE(L"Keyword \"array\" found when expecting \"vector\" elem.", token);
                if (!firstpropfound)
                    throw TokenE(L"Keyword \"array\" found before the \"Name\" property", token);
                if (afteritem)
                    throw TokenE(L"Keyword \"array\" found after control member list", token);

                if (!token.read())
                    throw TokenE(L"Unexpected end of file after keyword \"array\"", token);
                else if (token != rttUnquoted)
                    throw TokenE(L"Keyword \"array\" must be followed by a valid property name", token);
                std::wstring propname = token.toString();

#ifndef SKIPERROR
                int propix = (pc ? pc->serializer : cd->serializer)->PropertyIndex(propname, dpuSerialized);

                const type_info &type = (pc ? pc->serializer : cd->serializer)->PropertyValueType(propix);
                ObjectTypes proptype = ObjectTypeByTypeInfo(type);

                if (proptype == otUnknown)
                    throw TokenE(L"Unrecognized array elem type", token);
                if (proptype != otNative)
                    throw TokenE(L"Only native types can be stored in an array", token);

                if (!token.read())
                    throw TokenE(L"Unexpected end of file after array type name", token);
                if (token != rttToken || token != L"=")
                    throw TokenE(L"Expected \"=\" not found after array type name", token);

                if (propix < 0)
                    throw TokenE(L"Invalid property name in class", token);

                DesignSerializer *serializer = SerializerByTypeInfo(type);
                ControlDeserializerPropertyArray *prop = new ControlDeserializerPropertyArray(propname, serializer, (pc ? pc->serializer : cd->serializer)->Properties(propix));
#else
                // SKIPERROR not implemented, when implementing, after creating the property we have to find the last closing bracket belonging to it then going to the next token.
                ControlDeserializerPropertyArray *prop = new ControlDeserializerPropertyArray(); // Constructor used, when property is invalid and must be deleted later.
#endif

                if (pc)
                {
                    pc->properties.push_back(prop);
                    prop->pos = --pc->properties.end();
                }
                else
                {
                    cd->properties.push_back(prop);
                    prop->pos = --cd->properties.end();
                }
                propstack.push_back(prop);
                //pc = prop;

                neednameprop = false;
                firstpropfound = true;
                prevarray = true;
                continue;
            }

            if ((token == rttString && elemnext) || ( token == rttToken && inarray && token != L"}") || token == L"{")
            {
                if (!prevtype && !prevsub && !prevclass && !prevvector && !elemnext && !prevarray && !inarray && token == rttToken)
                    throw TokenE(L"Unexpected \"{\"", token);
                if (token == L"{" && token != rttToken)
                    throw TokenE(L"Unquoted L\"{\" expected", token);

                if (elemnext) // Next is a vector elem, create its deserializer property from the "template" type we got earlier.
                {
                    GeneralControlDeserializerPropertyVector *vec = (GeneralControlDeserializerPropertyVector*)pc;
                    auto ot = ObjectTypeByTypeInfo(vec->elemtype);

                    if (ot != otNative && token == rttString)
                        throw TokenE(L"Unexpected value in vector of non native types.", token);

                    if (ot == otNative)
                    {

                        neednameprop = false;
                        if (token == rttToken && token == L",")
                            throw TokenE(L"Cannot start a vector with a comma", token);

                        while (elemnext)
                        {
                            if (token == rttToken && token == L",")
                                token.read();
                            if (token == rttToken && token == L"}")
                            {
                                token.unread();
                                break;
                            }
                            if (token != rttString)
                                throw TokenE(L"Only values between quotes can make up a vector", token);
                            std::wstring propval = token.toString();

                            ControlDeserializerPropertyNameValue *prop = new ControlDeserializerPropertyNameValue(std::wstring(), propval, NULL/*vec->prop->SubProperty(NULL, pc->properties.size())*/);
                            pc->properties.push_back(prop);

                            if (!token.read())
                                throw TokenE(L"Unexpected end of file in vector elem list", token);
                            if (token != rttToken)
                            {
                                if (token != L"," && token != L"}")
                                    throw TokenE(L"Only a list of values between quotes with comma separator can make up a vector", token);
                            }
                        }
                    }
                    else if (ot == otSubControl || ot == otClass)
                    {
                        neednameprop = ot == otSubControl;

                        DesignSerializer *serializer = SerializerByTypeInfo(vec->elemtype);
                        ControlDeserializerPropertyList *prop = new ControlDeserializerPropertyList(std::wstring(), serializer, vec->prop->SubProperty(NULL, pc->properties.size()), true);
                        pc->properties.push_back(prop);
                        prop->pos = --pc->properties.end();
                        propstack.push_back(prop);
                        pc = prop;
                        if (neednameprop)
                        {
                            auto nameprop = serializer->Find(L"Name");
                            neednameprop = nameprop != nullptr && nameprop->IsSerialized();
                        }
                        firstpropfound = !neednameprop;

                    }
                    else // otVector
                    {
                        neednameprop = false;
                        //DesignSerializer *serializer = propertymap[ControlTypeInfoNames[vec->elemtype]];
                        throw L"Not implemented!";
/*
                        ControlDeserializerPropertyVector *prop = new ControlDeserializerPropertyVector(std::wstring(), elemtype ???, serializer, vec->elemprop);
                        pc->properties.push_back(prop);
                        prop->pos = --pc->properties.end();
                        propstack.push_back(prop);
                        pc = prop;
*/
                    }
                    elemnext = false;
                    continue;
                }
                if (inarray)
                {
                    if (propstack.empty() || dynamic_cast<ControlDeserializerPropertyArray*>(propstack.back()) == NULL)
                        throw TokenE(L"Unexpected property type in array", token);
                    ControlDeserializerPropertyArray *arr = (ControlDeserializerPropertyArray*)propstack.back();

                    unsigned int index;
                    if (token != L"[" || token != rttToken)
                        throw TokenE(L"Expected opening square bracket before array index in property array", token);
                    if (!token.read())
                        throw TokenE(L"Unexpected end of file before property value", token);
                    if (token != rttUnquoted || !StrToInt(token.toString(), index))
                        throw TokenE(L"Expected array index between square brackets in property array", token);
                    if (!token.read())
                        throw TokenE(L"Unexpected end of file before property value", token);
                    if (token != L"]" || token != rttToken)
                        throw TokenE(L"Expected closing square bracket after array index in property array", token);


                    if (!token.read())
                        throw TokenE(L"Unexpected end of file before property value", token);
                    if (token != L"=" || token != rttToken)
                        throw TokenE(L"Expected \"=\" after array index", token);
                    if (!token.read())
                        throw TokenE(L"Unexpected end of file before property value", token);
                    if (token != rttString)
                        throw TokenE(L"Quoted property value expected", token);

                    arr->AddValue(index, token.toString());

                    continue;
                }

                firstpropfound = !neednameprop;
                prevtype = false;
                prevclass = false;
                prevsub = false;
                elemnext = prevvector;
                prevvector = false;
                inarray = prevarray;
                prevarray = false;
                continue;
            }

            if (token == L"}")
            {
                if (token == rttToken && !prevtype && !prevsub && !prevclass && !prevvector && !prevarray && !inarray && !elemnext && !cd && !pc && propstack.empty() && itemstack.empty())
                {
                    token.unread();
                    break;
                }
                if (token == rttToken && (prevtype || prevsub || prevclass || prevvector || prevarray || elemnext || !cd))
                    throw TokenE(L"Unexpected \"}\"", token);
                if (token != rttToken)
                    throw TokenE(L"Unquoted L\"}\" expected", token);
                if (neednameprop && !firstpropfound)
                    throw TokenE(L"Unexpected \"}\" before specifying the Name property", token);

                firstpropfound = true; // Allow any kind of property to come after a closed bracket, as it suggests that the name of the item currently deserializing already found its name.
                inarray = false;

                if (!propstack.empty())
                {
                    propstack.pop_back();
                    if (propstack.empty() || dynamic_cast<ControlDeserializerPropertyList*>(propstack.back()) == NULL)
                        pc = NULL;
                    else
                    {
                        pc = (ControlDeserializerPropertyList*)propstack.back();
                        if (pc->type == cdptClassVector || pc->type == cdptNativeVector)
                        {
                            if (!token.read())
                                throw TokenE(L"Unexpected end of line in vector", token);
                            if (token != rttToken)
                                throw TokenE(L"Unexpected character in vector", token);
                            if (token == L"}")
                                token.unread();
                            else if (token == L",")
                                elemnext = true;
                            else
                                throw TokenE(L"Unexpected token in vector", token);
                        }
                    }
                }
                else if (!itemstack.empty())
                {
                    itemstack.pop_back();
                    if (itemstack.empty())
                        cd = NULL;
                    else
                        cd = itemstack.back();
                    afteritem = true;
                }

                if (itemstack.empty() && propstack.empty() && !containertype.empty())
                    break;
                continue;
            }

            if (token == rttUnquoted && token == L"event") // Event found:
            {
                if (pc)
                    throw TokenE(L"Only controls can have event properties", token);
                if (!cd)
                    throw TokenE(L"Keyword \"event\" found outside control member list", token);
                if (prevclass)
                    throw TokenE(L"Keyword \"event\" after \"class\". Expecting event name and value", token);
                if (prevtype)
                    throw TokenE(L"Keyword \"event\" after \"type\". Expecting event name and value", token);
                if (prevsub)
                    throw TokenE(L"Keyword \"event\" after \"subtype\". Expecting event name and value", token);
                if (prevvector)
                    throw TokenE(L"Keyword \"event\" after \"vector\". Expecting event name and value", token);
                if (prevarray)
                    throw TokenE(L"Keyword \"event\" after \"array\". Expecting event name and value", token);
                if (inarray)
                    throw TokenE(L"Keyword \"event\" in array list", token);
                if (elemnext)
                    throw TokenE(L"Keyword \"event\" found when expecting \"vector\" elem", token);
                if (!firstpropfound)
                    throw TokenE(L"Keyword \"event\" found before the \"Name\" property", token);
                if (afteritem)
                    throw TokenE(L"Keyword \"event\" found after control member list", token);

                if (!token.read())
                    throw TokenE(L"Unexpected end of file after keyword \"event\"", token);

                std::wstring eventname = token.toString();

                if (eventname == L"Name")
                    throw TokenE(L"\"Name\" is not a valid event!", token);

                if (!token.read())
                    throw TokenE(L"Unexpected end of file before event function value", token);
                if (token != L"=" || token != rttToken)
                    throw TokenE(L"Expected \"=\" after event name", token);
                if (!token.read())
                    throw TokenE(L"Unexpected end of file before property value", token);
                if (token != rttString)
                    throw TokenE(L"Quoted event function value expected", token);

                std::wstring eventval = token.toString();

                int eventix = cd->serializer->EventIndex(eventname, dpuSerialized);
                if (eventix < 0)
                    throw TokenE(L"Event name not valid", token);

                if (eventix >= 0)
                {
                    auto it = std::find_if(eventfunc.begin(), eventfunc.end(), [&eventval](const std::pair<std::wstring, std::wstring> &val) { return val.first == eventval; });
                    std::wstring eventparam = cd->serializer->EventParam(eventix);
                    if (it != eventfunc.end())
                    {
                        if ((*it).second != eventparam)
                            throw TokenE(std::wstring(L"Event function named \"") + eventval + (L"\" found in an event incompatible with a previously found function definition."), token);
                    }
                    else
                        eventfunc.push_back(std::pair<std::wstring, std::wstring>(eventval, eventparam));
                    ControlDeserializerPropertyNameValue *prop = new ControlDeserializerPropertyNameValue(eventname, eventval, cd->serializer->Events(eventix));
                    cd->properties.push_back(prop);
                }
            }
            else // Normal property:
            {
                if (afteritem)
                    throw TokenE(L"Property member after control items list", token);
                if (token != rttUnquoted)
                    throw TokenE(L"Unexpected token", token);

                std::wstring propname = token.toString();

                if (neednameprop && firstpropfound && propname == L"Name")
                    throw TokenE(L"Property \"Name\" specified again", token);
                if (neednameprop && !firstpropfound && propname != L"Name")
                    throw TokenE(L"Expected \"Name\" as the first property", token);

                if (!token.read())
                    throw TokenE(L"Unexpected end of file before property value", token);
                if (token != L"=" || token != rttToken)
                    throw TokenE(L"Expected \"=\" after property name", token);
                if (!token.read())
                    throw TokenE(L"Unexpected end of file before property value", token);
                if (token != rttString)
                    throw TokenE(L"Quoted property value expected", token);

                std::wstring propval = token.toString();

                if (neednameprop && !firstpropfound)
                {
                    if (!ValidVarName(propval))
                        throw TokenE(L"Value of property \"Name\" must be a valid variable name", token);

                    firstpropfound = true;
                    if (pc)
                    {
                        pc->name = propval;
                        continue;
                    }
                }

                int propix = (pc ? pc->serializer : cd->serializer)->PropertyIndex(propname, dpuSerialized);
                if (propix < 0)
                {
                    // Skip invalid property name after showing a messagebox.
                    ShowMessageBox(L"Property '" + propname + L"' not found in serialized object. " + token.position_str(), L"Invalid property", mbOk);
                }
                else
                {
                    ControlDeserializerPropertyNameValue *prop = new ControlDeserializerPropertyNameValue(propname, propval, (pc ? pc->serializer : cd->serializer)->Properties(propix));
                    if (pc)
                        pc->properties.push_back(prop);
                    else
                        cd->properties.push_back(prop);
                }
            }
        }
        if (proponly) // No need to read types for the project.
            token.unread();
    }
    catch(TokenE&)
    {
        FreeDeserializerList(items);
        throw;
    }
    catch(...)
    {
        FreeDeserializerList(items);
        throw;
    }
}

void FreeDeserializerList(std::vector<ControlDeserializerItem*> &items)
{
    while (!items.empty())
    {
        ControlDeserializerItem *cd = items.back();
        items.pop_back();
        delete cd;
    }
}


//---------------------------------------------


DesignSerializer::DesignSerializer() : container(false), nvcontainer(false), createfunc(nullptr), defaultproperty(nullptr), userdefaultproperty(nullptr)
{
    properties.SetSerializer(this);
}

DesignSerializer::~DesignSerializer()
{
}

void DesignSerializer::SetCreateFunction(CreateFunction newcreatefunc)
{
    createfunc = newcreatefunc;
}

Object* DesignSerializer::CreateObject(Object *owner)
{
    if (createfunc)
        return createfunc(owner);
    return NULL;
}

int DesignSerializer::PropertiesIndex(int index, DesignPropertyUsageSet condition)
{
    int ix = -1;
    for (auto it = properties.begin(); it != properties.end() && index >= 0; ++it, ++ix)
    {
        DesignProperty *prop = *it;
        if ((condition.empty() || (condition.contains(dpuHidden) && !prop->IsListed() && !prop->IsSerialized() && !prop->IsExported()) || (!condition.contains(dpuHidden) && condition.contains(dpuListed) && prop->IsListed()) || (!condition.contains(dpuHidden) && condition.contains(dpuSerialized) && prop->IsSerialized()) || (!condition.contains(dpuHidden) && condition.contains(dpuExported) && prop->IsExported())))
            --index;
    }
    return ix;
}

int DesignSerializer::EventsIndex(int index, DesignPropertyUsageSet condition)
{
    int ix = -1;
    for (auto it = events.begin(); it != events.end() && index >= 0; ++it, ++ix)
    {
        DesignProperty *prop = *it;
        if ((condition.empty() || (condition.contains(dpuHidden) && !prop->IsListed() && !prop->IsSerialized() && !prop->IsExported()) || (!condition.contains(dpuHidden) && condition.contains(dpuListed) && prop->IsListed()) || (!condition.contains(dpuHidden) && condition.contains(dpuSerialized) && prop->IsSerialized()) || (!condition.contains(dpuHidden) && condition.contains(dpuExported) && prop->IsExported())))
            --index;
    }
    return ix;
}

DesignProperty* DesignSerializer::Properties(int index)
{
    return properties[index];
}

DesignProperty* DesignSerializer::Events(int index)
{
    return events[index];
}

const std::wstring& DesignSerializer::EventParam(int index)
{
    return eventserializers[index]->Type();
}

const std::wstring& DesignSerializer::Names(int index)
{
    return serializers[index]->Name();
}

void DesignSerializer::RenameSerializer(int index, const std::wstring &newname)
{
    serializers[index]->SetName(newname);
}

bool DesignSerializer::MustSerialize(Object *propholder)
{
    for (auto it = serializers.begin(); it != serializers.end(); ++it)
    {
        ISerializeItem *item = *it;
        if (item->MustSerialize(propholder))
            return true;
    }
    for (auto it = eventserializers.begin(); it != eventserializers.end(); ++it)
    {
        ISerializeEvent *item = *it;
        if (item->MustSerialize(propholder))
            return true;
    }
    return false;
}

extern Designer *designer;
void DesignSerializer::MakeDefault(int index)
{
    if (index < 0 || (int)properties.size() <= index)
        return;
    if (designer == nullptr)
        defaultproperty = properties[index];
    else
    {
        userdefaultproperty = properties[index];
        if (userdefaultproperty == defaultproperty)
            userdefaultproperty = nullptr;
    }
}

void DesignSerializer::MakeDefault(const std::wstring &propname)
{
    MakeDefault(PropertyIndex(propname));
}

void DesignSerializer::MakeDefault(DesignProperty *prop)
{
    MakeDefault(PropertyIndex(prop));
}

void DesignSerializer::ClearDefault()
{
    if (designer == nullptr)
        defaultproperty = nullptr;
    else
        userdefaultproperty = nullptr;
}

std::wstring DesignSerializer::UserDefaultProperty()
{
    if (!userdefaultproperty)
        return std::wstring();
    return userdefaultproperty->Name();
}

DesignProperty* DesignSerializer::DefaultProperty()
{
    if ((!userdefaultproperty || !userdefaultproperty->IsListed()) && (!defaultproperty || !defaultproperty->IsListed()))
        return nullptr;

    return userdefaultproperty && userdefaultproperty->IsListed() ? userdefaultproperty : defaultproperty;
}

DesignProperty* DesignSerializer::Find(const std::wstring &name, DesignPropertyUsageSet condition)
{
    int ix = PropertyIndex(name, condition);
    if (ix < 0)
        return NULL;
    return properties[ix];
}

const type_info& DesignSerializer::PropertyCreatorType(int index)
{
    return properties[index]->CreatorType();
}

const type_info& DesignSerializer::PropertyValueType(int index)
{
    return properties[index]->ValueType();
}

int DesignSerializer::PropertyCount(DesignPropertyUsageSet condition)
{
    int ix = 0;
    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        DesignProperty *prop = *it;
        if (condition == 0 || (condition.contains(dpuListed) && prop->IsListed()) || (condition.contains(dpuSerialized) && prop->IsSerialized()) || (condition.contains(dpuExported) && prop->IsExported()) || (condition.contains(dpuDerived) && prop->IsDerived()))
            ++ix;
    }
    return ix;
    //return properties.size();
}

const type_info& DesignSerializer::EventCreatorType(int index)
{
    return events[index]->CreatorType();
}

const type_info& DesignSerializer::EventValueType(int index)
{
    return events[index]->ValueType();
}

int DesignSerializer::EventCount(DesignPropertyUsageSet condition)
{
    int ix = 0;
    for (auto it = events.begin(); it != events.end(); ++it)
    {
        DesignProperty *prop = *it;
        if (condition == 0 || (condition.contains(dpuListed) && prop->IsListed()) || (condition.contains(dpuSerialized) && prop->IsSerialized()) || (condition.contains(dpuExported) && prop->IsExported()) || (condition.contains(dpuDerived) && prop->IsDerived()))
            ++ix;
    }
    return ix;
    //return events.size();
}

bool DesignSerializer::ContainerControl()
{
    return container;
}

bool DesignSerializer::NonVisualContainerControl()
{
    return nvcontainer;
}

bool DesignSerializer::ParentFor(const type_info& type)
{
    ObjectTypes ot = ObjectTypeByTypeInfo(type);
    if (ot == otVisual)
        return container;
    if (ot == otNonVisual)
        return nvcontainer || container;
    if (ot == otSubControl)
        return subchildtypes.find(type.name()) != subchildtypes.end();
    return false;
}

void DesignSerializer::SetContainerControl(bool iscontainer)
{
    container = iscontainer;
}

void DesignSerializer::SetNonVisualContainerControl(bool isnvcontainer)
{
    nvcontainer = isnvcontainer;
}

void DesignSerializer::AllowSubType(const type_info &type)
{
    subchildtypes.insert(type.name());
}

void DesignSerializer::DisallowSubType(const type_info &type)
{
    subchildtypes.erase(type.name());
}

void DesignSerializer::ClearAllowSubTypes()
{
    subchildtypes.clear();
}

void DesignSerializer::HideProperty(const std::wstring& name)
{
    int ix = PropertyIndex(name, 0);
    if (ix < 0)
        return;
    properties[ix]->Hide();
}

void DesignSerializer::HideEvent(const std::wstring& name)
{
    int ix = EventIndex(name, 0);
    if (ix < 0)
        return;
    events[ix]->Hide();
}

void DesignSerializer::MoveHere(const std::wstring& name)
{
    int ix = PropertyIndex(name, 0);
    if (ix < 0)
        return;
    properties.MoveToEnd(ix);
    auto is = serializers[ix];
    serializers.erase(serializers.begin() + ix);
    serializers.push_back(is);
}

void DesignSerializer::HideAll()
{
    for (auto it = properties.rbegin(); it != properties.rend(); ++it)
        (*it)->Hide();
    for (auto it = events.rbegin(); it != events.rend(); ++it)
        (*it)->Hide();
}

int DesignSerializer::PropertyIndex(DesignProperty *prop)
{
    for (int ix = properties.size() - 1; ix >= 0; --ix)
    {
        if (properties[ix] == prop)
            return ix;
    }
    return -1;
}

int DesignSerializer::PropertyIndex(const std::wstring& name, DesignPropertyUsageSet condition)
{
    return PropertyIndex<DesignProperty>(name, condition);
}

int DesignSerializer::EventIndex(const std::wstring& name, DesignPropertyUsageSet condition)
{
    return EventIndex<DesignProperty>(name, condition);
}

void DesignSerializer::ConstructExport(Indentation &indent, std::wiostream &stream, std::wiostream &sdelayed, std::wiostream &sevents, Object *parent, Object *control, DesignProperty *prop)
{
    std::wstring printedname = control->Name();

    if (!prop || !prop->ParentCreated())
        stream << indent << printedname << L" = new " << control->ClassName(true) << L"();" << std::endl;
    else
        prop->ConstructExport(indent, prop->Delayed() ? sdelayed : stream, parent, control, printedname);
    CppExport(indent, printedname, true, stream, sdelayed, sevents, control);
    if ((dynamic_cast<Control*>(control) != NULL || dynamic_cast<NonVisualControl*>(control) != NULL) && (!prop || !prop->ParentCreated()))
        stream << indent << printedname << L"->SetParent(" << (parent == 0 ? L"this" : parent->Name()) << L");" << std::endl;

    stream << std::endl;
}

void DesignSerializer::CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, std::wiostream &wsdelayed, std::wiostream &wsevents, Object *propholder)
{
    for (auto *s : serializers)
    {
        if (!s->MustExport(propholder))
            continue;
        s->CppExport(indent, prefix, pointerprefix, !s->Delayed() ? ws : wsdelayed, wsevents, propholder);
    }
    for (auto *es : eventserializers)
    {
        if (!es->MustExport(propholder))
            continue;
        es->CppExport(indent, prefix, pointerprefix, wsevents, propholder);
    }
}

void DesignSerializer::Serialize(Indentation &indent, std::wiostream &ws, Object *propholder)
{
    for (auto *s : serializers)
    {
        if (!s->MustSerialize(propholder))
            continue;
        s->Serialize(indent, ws, propholder);
    }
    for (auto *es : eventserializers)
    {
        if (!es->MustSerialize(propholder))
            continue;
        es->Serialize(indent, ws, propholder);
    }
}

void DesignSerializer::DeclareSerialize(Indentation &indent, std::wiostream &stream, Object *control, AccessLevels access)
{
    std::wstring name = control->Name();
    if (!name.length())
        return;
    if (access == control->AccessLevel())
        stream << indent << control->ClassName(true) << L" *" << name << L";" << std::endl;

    for (auto *s : serializers)
    {
        if (!s->MustExport(control))
            continue;
        s->DeclareSerialize(indent, stream, control, access);
    }
}

//void DesignSerializer::CollectEvents(std::vector<std::pair<void*, std::wstring>> &eventlist, Object *control)
//{
//    int ix = 0;
//    for (auto it = eventserializers.begin(); it != eventserializers.end(); ++it, ++ix)
//    {
//        if (!(*it)->MustCollect(control))
//            continue;
//        if (std::find_if(eventlist.begin(), eventlist.end(), [&eventlist, this, &control, ix](const std::pair<void*, std::wstring> &val) { return val.first == (dynamic_cast<IEventProperty*>(events[ix]))->Event(control); }) != eventlist.end())
//            continue;
//        eventlist.push_back(std::make_pair((dynamic_cast<IEventProperty*>(events[ix]))->Event(control), (*it)->ParamString()));
//    }
//}


//---------------------------------------------


}
/* End of NLIBNS */


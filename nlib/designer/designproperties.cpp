#include "stdafx_zoli.h"

#include "syscontrol.h"
#include "designproperties.h"
#include "designerdialogs.h"
#include "serializer.h"
#include "designerform.h"
#include "designer.h"

//---------------------------------------------


namespace NLIBNS
{


Rect cdp__cursorrect__g_zapp(0, 0, 0, 0); // Largest rectangle which can hold the image of any cursor in the system.

OpenDialog *imgpropopendialog = NULL;
SaveDialog *imgpropsavedialog = NULL;

//---------------------------------------------

ValuePair<Colors> ColorStrings[] = {
    make_ValuePair<Colors>((Colors)(INT_MAX - 1), L"Other..."),
    VALUEPAIR(clNone),

    VALUEPAIR(clBlack),
    VALUEPAIR(clBlue),
    VALUEPAIR(clCyan),
    VALUEPAIR(clFuchsia),
    VALUEPAIR(clGray),
    VALUEPAIR(clGreen),
    VALUEPAIR(clLime),
    VALUEPAIR(clMaroon),
    VALUEPAIR(clNavy),
    VALUEPAIR(clOlive),
    VALUEPAIR(clPurple),
    VALUEPAIR(clRed),
    VALUEPAIR(clSilver),
    VALUEPAIR(clTeal),
    VALUEPAIR(clYellow),
    VALUEPAIR(clWhite),

    VALUEPAIR(cl3DDkShadow),
    VALUEPAIR(cl3DHighlight),
    VALUEPAIR(cl3DLight),
    VALUEPAIR(cl3DShadow),
    VALUEPAIR(clActiveBorder),
    VALUEPAIR(clActiveCaption),
    VALUEPAIR(clAppWorkspace),
    VALUEPAIR(clBackground),
    VALUEPAIR(clBtnFace),
    VALUEPAIR(clBtnText),
    VALUEPAIR(clCaptionText),
    VALUEPAIR(clHighlight),
    VALUEPAIR(clHighlightText),
    VALUEPAIR(clGradientActiveCaption),
    VALUEPAIR(clGradientInactiveCaption),
    VALUEPAIR(clGrayText),
    VALUEPAIR(clHotlight),
    VALUEPAIR(clInactiveBorder),
    VALUEPAIR(clInactiveCaption),
    VALUEPAIR(clInactiveCaptionText),
    VALUEPAIR(clInfoBK),
    VALUEPAIR(clInfoText),
    VALUEPAIR(clMenu),
    VALUEPAIR(clMenubar),
    VALUEPAIR(clMenuHilight),
    VALUEPAIR(clMenuText),
    VALUEPAIR(clScrollbar),
    VALUEPAIR(clWindow),
    VALUEPAIR(clWindowFrame),
    VALUEPAIR(clWindowText),
};
//---------------------------------------------


void CreateImgPropOpenDialog()
{
    if (imgpropopendialog)
        return;
    FileDialog *fd = imgpropopendialog = new OpenDialog();
    fd->SetDefaultExtension(L"bmp");
    fd->SetTitle(L"Open image");
    fd->AddFilter(L"Images (*.bmp, *.png, *.jpg, *.jpeg, *.gif, *.ico, *.ttf)", L"*.bmp;*.png;*.jpg;*.jpeg;*.gif;*.ico;*.ttf");
    fd->AddFilter(L"Windows bitmap (*.bmp)", L"*.bmp");
    fd->AddFilter(L"Portable Network Graphics (*.png)", L"*.png");
    fd->AddFilter(L"JPEG (*.jpg, *.jpeg)", L"*.jpg;*.jpeg");
    fd->AddFilter(L"Graphics Interchange Format (*.gif)", L"*.gif");
    fd->AddFilter(L"Windows icons (*.ico)", L"*.ico");
    fd->AddFilter(L"Tagged Image File Format (*.ttf)", L"*.ttf");
}

void CreateImgPropSaveDialog()
{
    if (imgpropsavedialog)
        return;
    FileDialog *fd = imgpropsavedialog = new SaveDialog();
    fd->SetDefaultExtension(L"bmp");
    fd->SetTitle(L"Open image");
    fd->AddFilter(L"Images (*.bmp, *.png, *.jpg, *.jpeg, *.gif, *.ttf)", L"*.bmp;*.png;*.jpg;*.jpeg;*.gif;*.ttf");
    fd->AddFilter(L"Windows bitmap (*.bmp)", L"*.bmp");
    fd->AddFilter(L"Portable Network Graphics (*.png)", L"*.png");
    fd->AddFilter(L"JPEG (*.jpg, *.jpeg)", L"*.jpg;*.jpeg");
    fd->AddFilter(L"Graphics Interchange Format (*.gif)", L"*.gif");
    fd->AddFilter(L"Tagged Image File Format (*.ttf)", L"*.ttf");
}


//---------------------------------------------


void DrawThumbBitmap(Canvas *c, Bitmap *image, int statecount, const Rect &dest)
{
    if (!image)
    {
        c->SetAntialias(true);
        c->SetPen(Color(255, 0, 0), 2.5);
        c->Line(dest.left + 1.5F, dest.top + 1.5F, dest.right - 1.5F, dest.bottom - 1.5F);
        c->Line(dest.right - 1.5F, dest.top + 1.5F, dest.left + 1.5F, dest.bottom - 1.5F);
        c->SetAntialias(false);
        return;
    }
    int bw = max(1, image->Width() / statecount);
    int bh = image->Height();

    float div = min(1, max((float)dest.Width() / bw, (float)dest.Height() / bh));

    int nbh = bh;
    int nbw = bw;
    if ((float)bh / bw > (float)dest.Height() / dest.Width())
        nbh = min(bh, (float)bw * ((float)dest.Height() / dest.Width()));
    else
        nbw = min(bw, (float)bh * ((float)dest.Width() / dest.Height()));
    c->DrawF(image, (float)dest.left + max(0, (dest.Width() - bw * div) / 2), (float)dest.top + max(0, (dest.Height() - bh * div) / 2), min(dest.Width(), (float)bw * div), min(dest.Height(), (float)bh * div), (bw - nbw) / 2, (bh - nbh) / 2, nbw, nbh);
}


//---------------------------------------------


// Helper functions to avoid circular reference and make design properties know about form types.
DesignForm* CastToDesignForm(Object *obj)
{
    return dynamic_cast<DesignForm*>(obj);
}

DesignForm* CastToDesignForm(Form *form)
{
    return dynamic_cast<DesignForm*>(form);
}

DesignFormBase* CastToDesignFormBase(Object *obj)
{
    return dynamic_cast<DesignFormBase*>(obj);
}

DesignFormBase* CastToDesignFormBase(Form *form)
{
    return dynamic_cast<DesignFormBase*>(form);
}

const std::wstring& SerializerFormName(Form *form)
{
    return form->Name();
}

bool DesignFormIsObjectBase(DesignFormBase *form, Object *obj)
{
    return dynamic_cast<Object*>(form) == obj;
}

bool DesignFormIsForm(DesignFormBase *dform, Form *form)
{
    return dynamic_cast<Form*>(dform) == form;
}

bool DesignFormIsForm(DesignForm *dform, Form *form)
{
    return dynamic_cast<Form*>(dform) == form;
}

void CollectObjects(DesignFormBase *form, std::vector<std::pair<std::wstring, Object*>> &objectstrings, bool (*collector)(Object*), const std::wstring &objectname)
{
    form->CollectObjects(objectstrings, collector, objectname);
}

bool NameTaken(DesignFormBase *form, const std::wstring &val)
{
    return form->NameTaken(val);
}

bool FormNameTaken(DesignFormBase *form, const std::wstring &val)
{
    return form->FormNameTaken(val);
}

void GetTabActivatedControls(DesignForm *form, std::vector<std::pair<std::wstring, Control*> > &controlstrings, const std::wstring &controlname)
{
    form->GetTabActivatedControls(controlstrings, controlname);
}

std::wstring DesignFormEventFunction(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &name)
{
    auto e = form->GetEventFunction(eventtype, propholder, name);
    if (!e || e->func.empty())
        return std::wstring();
    return e->func;
}

void* DesignFormEvent(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &name)
{
    return form->GetEventFunction(eventtype, propholder, name);
}

std::wstring DesignFormEventFunctionByIndex(DesignFormBase *form, const std::wstring &eventtype, int index)
{
    return form->GetEventFunctionByIndex(eventtype, index);
}

void DesignFormSetEventFunction(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &name, const std::wstring &newvalue)
{
    form->SetEventFunction(eventtype, propholder, name, newvalue);
}

int DesignFormEventCountById(DesignFormBase *form, const std::wstring &eventtype)
{
    return form->GetEventCountById(eventtype);
}

int DesignFormEventFunctionIndex(DesignFormBase *form, const std::wstring &eventtype, Object *propholder, const std::wstring &eventfunction)
{
    return form->GetEventFunctionIndex(eventtype, propholder, eventfunction);
}

void DesignFormEditPopupMenu(DesignFormBase *form, PopupMenu *menu)
{
     form->EditPopupMenu(menu);
}

DesignProperty* SerializerFind(DesignSerializer *serializer, const std::wstring& pname)
{
    return serializer->Find(pname);
}

DesignProperty* SerializerProperties(DesignSerializer *serializer, int index)
{
    return serializer->Properties(index);
}

int SerializerPropertiesIndex(DesignSerializer *serializer, int index, DesignPropertyUsageSet condition)
{
    return serializer->PropertiesIndex(index, condition);
}

int SerializerPropertyCount(DesignSerializer *serializer, DesignPropertyUsageSet condition)
{
    return serializer->PropertyCount(condition);
}

int SerializerPropertyIndex(DesignSerializer *serializer, DesignProperty *prop)
{
    return serializer->PropertyIndex(prop);
}

const std::wstring& SerializerNames(DesignSerializer *serializer, int index)
{
    return serializer->Names(index);
}

int DesignerNextMemberIndex(std::wstring typestr)
{
    return designer->NextMemberIndex(typestr);
}

int DesignerExportToResource(std::vector<byte> &res)
{
    return designer->ExportToResource(res);
}


//---------------------------------------------


DesignProperty::DesignProperty(const std::wstring &name, const std::wstring &category) :
        name(name), category(category), priority(0), /*pointervalue(true),*/ usage(dpuListed | dpuSerialized | dpuExported), 
        parentcreate(false), reftype(pcrtFormDeclare), argsfunc(nullptr), owner(nullptr), propertystyle(psEditShared | psGuestEditable), sfunc(nullptr)
{
}

DesignProperty::DesignProperty()
{
}

DesignProperty::~DesignProperty()
{

}

void DesignProperty::SetOwner(DesignProperties *newowner)
{
    owner = newowner;
}

const std::wstring& DesignProperty::Category()
{
    return category;
}

const std::wstring& DesignProperty::Name()
{
    return name;
}

std::wstring DesignProperty::Value(Object *propholder)
{
    return std::wstring();
}

bool DesignProperty::Indexed()
{
    return false;
}

int DesignProperty::Index()
{
    throw L"This function must be overloaded!";
}

void DesignProperty::ChangeIndex(int newindex)
{
    throw L"This function must be overloaded!";
}

bool DesignProperty::HasPropertyStyle(PropertyStyles style)
{
    return propertystyle.contains(style);
}

void DesignProperty::DisableSharedEdit()
{
    propertystyle -= psEditShared;
}

void DesignProperty::SetImmediateUpdate(bool immediate)
{
    if (immediate)
        propertystyle << psImmediateUpdate;
    else
        propertystyle -= psImmediateUpdate;
}

int DesignProperty::Priority()
{
    return priority;
}

void DesignProperty::SetPriority(int newpriority)
{
    priority = newpriority;
}

void DesignProperty::DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
{
}

void DesignProperty::MeasureListItem(Object *propholder, MeasureItemParameters params)
{
}

void DesignProperty::DrawListItem(Object *propholder, DrawItemParameters params)
{
}

void DesignProperty::SetInnerValue(Form *parentform, Object *propholder, const std::wstring& val)
{
    if (propertystyle.contains(psInnerBinary))
    {
        std::vector<byte> v(val.length() / 2);
        unsigned int vallen = val.length();
        HexDecodeBytes(val.c_str(), vallen, &v[0], v.size());
        if (vallen * 2 != val.length())
            throw L"Invalid value in hex encoded property string";
        SetBinaryValue(parentform, propholder, v);
    }
    else
        SetValue(parentform, propholder, val);
}

void DesignProperty::SetInnerValue(Form *parentform, Object *propholder, int index, const std::wstring& val)
{
}

std::wstring DesignProperty::InnerValue(Object *propholder)
{
    if (propertystyle.contains(psInnerBinary))
    {
        std::vector<byte> val;
        BinaryValue(propholder, val);
        if (!val.empty())
            return HexEncodeW(&val[0], val.size());
        else
            return std::wstring();
    }
    else
        return Value(propholder);
}

bool DesignProperty::SetValue(Form *parentform, Object *propholder, const std::wstring& val)
{
    return false;
}

void DesignProperty::BinaryValue(Object *propholder, std::vector<byte> &result)
{
    ;
}

void DesignProperty::SetBinaryValue(Form *parentform, Object *propholder, std::vector<byte> &val)
{
    ;
}

void DesignProperty::ResourceValue(Object *propholder, std::vector<byte> &result)
{
    if (!propertystyle.contains(psBinaryResource))
        return;
    if (propertystyle.contains(psInnerBinary))
        BinaryValue(propholder, result);
}

bool DesignProperty::StoresBinaryResource(Object *propholder)
{
    return propertystyle.contains(psBinaryResource);
}

//void DesignProperty::Hide()
//{
//    hidden = true;
//}
//
//void DesignProperty::Show()
//{
//    hidden = false;
//}
//
//bool DesignProperty::Hidden()
//{
//    return hidden;
//}

bool DesignProperty::IsListed()
{
    return usage.contains(dpuListed);
}

bool DesignProperty::IsSerialized()
{
    return usage.contains(dpuSerialized);
}

bool DesignProperty::IsExported()
{
    return usage.contains(dpuExported);
}

bool DesignProperty::IsDerived()
{
    return usage.contains(dpuDerived);
}

bool DesignProperty::IsWritten()
{
    return IsSerialized() && IsExported();
}

bool DesignProperty::Delayed()
{
    return propertystyle.contains(psDelayedRestore);
}

void DesignProperty::Delay()
{
    propertystyle << psDelayedRestore;
}

void DesignProperty::MakeDefault()
{
    owner->Serializer()->MakeDefault(this);
}

void DesignProperty::DontList()
{
    usage -= dpuListed;
}

void DesignProperty::DontSerialize()
{
    usage -= dpuSerialized;
}

void DesignProperty::DontExport()
{
    usage -= dpuExported;
}

void DesignProperty::DontWrite()
{
    DontExport();
    DontSerialize();
}

void DesignProperty::DontDerive()
{
    usage -= dpuDerived;
}

void DesignProperty::Hide()
{
    usage = 0;
}

void DesignProperty::DoWrite()
{
    usage << dpuSerialized << dpuExported;
}

void DesignProperty::DoList()
{
    usage << dpuListed;
}

void DesignProperty::DoSerialize()
{
    usage << dpuSerialized;
}

void DesignProperty::DoExport()
{
    usage << dpuExported;
}

void DesignProperty::Derive()
{
    usage = dpuDerived;
}

int DesignProperty::SubCount(Object *propholder)
{
    return 0;
}

DesignProperty* DesignProperty::SubProperty(Object *propholder, int index)
{
    return NULL; // Dummy for not returning anything.
}

//Object* DesignProperty::SubItem(Object *propholder, int index)
//{
//    return NULL;
//}

int DesignProperty::ListCount(Object *propholder)
{
    return 0;
}

std::wstring DesignProperty::ListItem(Object *propholder, int index)
{
    return L"";
}

void* DesignProperty::ListValue(Object *propholder, int index)
{
    return NULL;
}

int DesignProperty::Selected(Object *propholder)
{
    return -1;
}

void DesignProperty::AddItem(Object *propholder)
{
}

void DesignProperty::AddItem(Object *propholder, const std::wstring &str)
{
}

Object* DesignProperty::SubHolder(Object *propholder)
{
    return propholder;
}

bool DesignProperty::SelectValue(Form *parentform, Object *propholder, void *val)
{
    int cnt = ListCount(propholder);
    for (int ix = 0; ix < cnt; ++ix)
        if (ListValue(propholder, ix) == val)
            return SetValue(parentform, propholder, ListItem(propholder, ix));

    return false;
}

bool DesignProperty::ClickEdit(Form *parentform, Object *propholder)
{
    return false;
}

std::wstring DesignProperty::ExportValue(Object *propholder)
{
    return Value(propholder);
}

std::wstring DesignProperty::ResourceExportValue(Object *propholder, int resourceid)
{
    return L"MAKEINTRESOURCE(" + IntToStr(resourceid) + L")";
}

void DesignProperty::SetParentCreation(ParentCreationReferenceType areftype, SerializeArgsFunc aargsfunc)
{
    argsfunc = aargsfunc;
    parentcreate = true;
    reftype = areftype;
}

void DesignProperty::SetParentCreationReferenceType(ParentCreationReferenceType areftype)
{
    parentcreate = true;
    reftype = areftype;
}

bool DesignProperty::ParentCreated()
{
    return parentcreate;
}

ParentCreationReferenceType DesignProperty::ReferenceType()
{
    return reftype;
}

std::wstring DesignProperty::ConstructSerializeArgs(Object *obj)
{
    if (!argsfunc)
        return std::wstring();
    return argsfunc(obj);
}

void DesignProperty::ConstructExport(Indentation &indent, std::wiostream &stream, Object *parent, Object *control, std::wstring &printedname)
{
    if (!parentcreate)
        return;

    stream << indent;
    if (reftype == pcrtLocalDeclare)
        stream << control->ClassName(true) << L" *";
    DesignSerializer *serializer = parent->Serializer();
    int ix = serializer->PropertyIndex(this);
    std::wstring name;
    if (ix >= 0)
        name = serializer->Names(ix);
    if (name.empty())
        throw L"No creation name given for the property with parent create!";

    if (reftype == pcrtLocalDeclare || reftype == pcrtFormDeclare)
    {
        std::wstring cname = control->Name();
        if (cname.empty())
        {
            if (reftype != pcrtLocalDeclare)
                throw L"No name for a locally declared control.";
            printedname = cname = L"temp_var00" + IntToStr((int)control);
        }
        stream << cname << L" = ";
    }

    stream << parent->Name() << L"->" << name << L"(" << ConstructSerializeArgs(control) << ");" << std::endl;
}

void DesignProperty::SetSerializerFunc(SerializerFunc asfunc)
{
    sfunc = asfunc;
}

bool DesignProperty::HasSerializerFunc()
{
    return sfunc != NULL;
}

void DesignProperty::CppExport(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum)
{
    if (!sfunc)
        throw L"Trying to export without setting a serializer function.";

    sfunc(indent, prefix, pointerprefix, ws, propholder, prop, resnum);
}

void DesignProperty::DeclaredNames(std::vector<std::pair<std::wstring, std::wstring>> &names, Object *propholder, AccessLevels access)
{
}

//void DesignProperty::DeclaredName(std::vector<std::pair<std::wstring, std::wstring>> &names, Object *propholder, Object *control)
//{
//    if (!parentcreate || declaretype)
//        return;
//    DesignSerializer *serializer = propholder->Serializer();
//    names.push_back(std::make_pair(control->ClassName(), control->Name()));
//
//}

bool DesignProperty::IsPointerValue()
{
    return true; //pointervalue;
}

//void DesignProperty::SetAsPointerValue(bool newpvalue)
//{
//    pointervalue = newpvalue;
//}


//---------------------------------------------


DesignProperties::DesignProperties() : serializer(nullptr)
{
}


DesignProperties::~DesignProperties()
{
    for (auto it = begin(); it != end(); it++)
        delete *it;
}

void DesignProperties::SetSerializer(DesignSerializer *aserializer)
{
    serializer = aserializer;
}

DesignSerializer* DesignProperties::Serializer()
{
    return serializer;
}


//---------------------------------------------


}
/* End of NLIBNS */


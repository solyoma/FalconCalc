#include "stdafx_zoli.h"
#include "syscontrol.h"
#include "buttons.h"
#include "generalcontrol.h"
#include "themes.h"

#include "designercontrols.h"
#include "designproperties.h"
#include "dialog.h"
#include "serializer.h"
#include "designerform.h"
#include "buttons.h"

#include "designer.h"

#define amClickActivate     (WM_APP + 1)

//---------------------------------------------


namespace NLIBNS
{


ButtonContainer::ButtonContainer(ButtonPanel *owner, const std::wstring &title) : base(), owner(owner), open(false), margin(3 * Scaling), btnheight(25 * Scaling), btnwidth(110 * Scaling) /*, cols(0), rows(0)*/
{
    SetText(title);

    InitControlList();
    glyphsize = themes->MeasureTreeviewGlyph(ttgsOpen);
    SetParentBackground(false);
    SetParentColor(false);

    SetDoubleBuffered(true);
}

ButtonContainer::~ButtonContainer()
{
}

void ButtonContainer::Clear()
{
    for (auto b : buttons)
        b->Destroy();
}

void ButtonContainer::InitHandle()
{
    base::InitHandle();
    //GetCanvas();
}

int ButtonContainer::TitleSize()
{
    return margin * 4 + glyphsize.cy;
}

void ButtonContainer::Paint(const Rect &updaterect)
{
    Rect cr = ClientRect();
    Canvas *c = GetCanvas();

    Rect hr = RectF(margin, margin, cr.right - margin, margin * 3.0 + glyphsize.cy);
    if (!updaterect.DoesIntersect(hr))
        return;

    c->SetBrush(hr, Color(255, 240, 240, 240), Color(255, 210, 210, 210), 90);
    c->SetAntialias(true);
    c->FillRoundRect(hr, margin * 2, margin * 2);
    c->SetPen(Color(255, 150, 150, 150));
    c->RoundFrameRect(hr, margin * 2, margin * 2);
    c->SetAntialias(false);

    themes->DrawTreeviewGlyph(c, GlyphRect(), open ? ttgsOpen : ttgsClosed);

    c->GetFont().SetBold(true);
    c->TextDraw(Rect(hr.left + margin * 3 + glyphsize.cx, margin * 1.5, hr.right - margin, hr.bottom - margin), hr.left + margin * 3 + glyphsize.cx, hr.top + margin * 0.3, Text());
}

void ButtonContainer::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    Rect r = GlyphRect(), r2 = Rect(0, r.top, 9999, r.bottom);
    if (PtInRect(&r, Point(x, y)) || (PtInRect(&r2, Point(x, y)) && vkeys.contains(vksDouble)))
    {
        Rect cr = WindowRect();
        open = !open;
        InvalidateRect(r);

        if (!buttons.empty())
        {
            if (!open)
            {
                if (buttons[0]->Visible())
                    for (auto b : buttons)
                        b->Hide();
            }
            else
            {
                if (!buttons[0]->Visible())
                    for (auto b : buttons)
                        b->Show();
            }
        }

        owner->ContentUpdated();
    }
}

Rect ButtonContainer::GlyphRect()
{
    return RectS(margin * 2, margin * 2, glyphsize.cx, glyphsize.cy);
}

void ButtonContainer::Update()
{
    for (auto b : buttons)
    {
        //b->SetImageLayout(owner->Captions() ? bipLeft : bipCenter, 6 * Scaling, 12 * Scaling);
        b->SetContentPosition(owner->Captions() ? bcpLeft : bcpCenter);
        b->SetShowText(owner->Captions());
        b->SetShowTooltip(!owner->Captions());
    }
}

void ButtonContainer::AddButton(const type_info &type)
{
    Rect cr = HandleCreated() ? ClientRect() : WindowRect();
    //if (rows == 0)
    //    rows = 1;

    ToolButton *fb = new ToolButton();
    buttons.push_back(fb);

    fb->Hide();

    fb->SetDoubleBuffered(true);

    fb->SetContentPosition(owner->Captions()? bcpLeft : bcpCenter);

    fb->SetTag((int)&type);
    fb->SetType(fbtCheckRadiobutton);

    fb->SetText(DisplayNameByTypeInfo(type, false));
    fb->SetShowText(owner->Captions());
    fb->SetSpacing(12 * Scaling);
    //fb->SetImageLayout(owner->Captions() ? bipLeft : bipCenter, 6 * Scaling, 12 * Scaling);
    fb->SetTooltipText(fb->Text());
    fb->SetShowTooltip(!owner->Captions());

    fb->SetShowText(owner->Captions());
    int imageindex = max(0, ImageIndexByTypeInfo(type));
    if (imageindex >= 0)
    {
        fb->Image()->SetImages(owner->Images());
        fb->Image()->SetImageIndex(imageindex);
    }

    fb->SetParent(this);

    fb->OnClick = CreateEvent(this, &ButtonContainer::btnclick);
    fb->OnDblClick = CreateEvent(this, &ButtonContainer::btndblclick);

    if (open)
        fb->Show();

    owner->ContentUpdated();
}

int ButtonContainer::MeasureHeight(int width, bool layout)
{
    bool cap = owner->Captions();

    int h = glyphsize.cx + margin * 3 +
                         + margin  /* Margin above buttons. */
                         + margin; /* Margin below buttons. */

    if (layout)
        InvalidateRect(Rect(0, 0, width, h - margin));

    if (!open)
        return h - margin;

    width -= margin * (cap ? 2 : 4);

    if (!layout)
    {
        int w;
        if (!cap)
        {
            w = btnheight;
            w = width / w;
        }
        else
        {
            w = min(btnwidth , width);
            if (w * 2 > width)
                w = width;
            w = width / w;
        }

        return h + ((buttons.size() + w - 1) / w) * btnheight;
    }


    HDWP hdwp = NULL;
    if (HandleCreated())
        hdwp = BeginDeferWindowPos(buttons.size());

    int left = 0;
    int w = min((cap ? btnwidth : btnheight), width);
    if (cap)
    {
        if (w * 2 > width)
            w = width;
        else
            w = width / (width / w);
    }
    for (auto b : buttons)
    {
        if (left + w > width)
        {
            left = 0;
            h += btnheight;
        }

        Rect r = RectS(left + margin * (cap ? 1 : 2), h - margin, w, btnheight);

        if (HandleCreated())
            hdwp = DeferWindowPos(hdwp, b->Handle(), NULL, r.left, r.top, r.Width(), r.Height(), SWP_BOUNDS);
        else
            b->SetBounds(r);

        left += w;
    }

    if (HandleCreated() && hdwp)
        EndDeferWindowPos(hdwp);

    return h + btnheight;
}

void ButtonContainer::ComputeAlignBounds(Rect &bounds)
{
    base::ComputeAlignBounds(bounds);
    bounds.bottom = bounds.top + MeasureHeight(bounds.Width(), true);
}

void ButtonContainer::UpdateImages()
{
    for (auto fb : buttons)
    {
        if (!fb->Image()->HasImage())
            continue;
        fb->Image()->SetImages(owner->Images());
        fb->Image()->SetImageIndex(fb->Image()->ImageIndex());
    }
}

void ButtonContainer::SetImageIndex(int buttonindex, int imageindex)
{
    if (buttonindex < 0 || buttonindex >= (int)buttons.size())
        throw L"Button index out of range.";

    buttons[buttonindex]->Image()->SetImages(owner->Images());
    buttons[buttonindex]->Image()->SetImageIndex(imageindex);
}

void ButtonContainer::btnclick(void *sender, EventParameters param)
{
    ToolButton *fb = (ToolButton*)sender;
    owner->ButtonClicked(fb);
}

void ButtonContainer::btndblclick(void *sender, MouseButtonParameters param)
{
    DesignForm *frm = dynamic_cast<DesignForm*>(designer->ActiveForm());
    if (frm)
        frm->PlaceControl(*(const type_info*)((ToolButton*)sender)->Tag());
    else
    {
        DesignContainerForm *cfrm = dynamic_cast<DesignContainerForm*>(designer->ActiveForm());
        if (cfrm)
            cfrm->PlaceControl(*(const type_info*)((ToolButton*)sender)->Tag());
    }

}

ToolButton* ButtonContainer::FindButton(int id)
{
    auto it = std::find_if(buttons.begin(), buttons.end(), [id](ToolButton *fb) { return fb->Tag() == id; });
    if (it == buttons.end())
        return nullptr;
    return *it;
}


//---------------------------------------------


int GetRegisteredControlCategoryIndex(const type_info &);

ButtonPanel::ButtonPanel() : base(), captions(true), images(NULL), lastfb(NULL), nvcontrol(NULL)
{
    InitControlList();
    SetColor(clWindow);
    NeedScrollbars();
    SetAutoSizeScroll(true);

    VScroll()->SetLineStep(15 * Scaling); 
}

ButtonPanel::~ButtonPanel()
{
    //Clear();
}

void ButtonPanel::Clear()
{
    for (auto c : containers)
        c->Destroy();
}

ButtonContainer* ButtonPanel::AddContainer(const std::wstring& captiontext)
{
    ButtonContainer *bc = new ButtonContainer(this, captiontext);
    bc->SetTop(containers.empty() ? 0 : (*(containers.end()-1))->Top()+1);
    bc->SetAlignment(alTop);
    bc->SetParent(this);
    containers.push_back(bc);
    return bc;
}

ButtonContainer* ButtonPanel::Containers(int index)
{
    return containers[index];
}

void ButtonPanel::AddButton(const type_info &type)
{
    Containers(GetRegisteredControlCategoryIndex(type))->AddButton(type);
}

Imagelist* ButtonPanel::Images()
{
    return images;
}

void ButtonPanel::SetImages(Imagelist *newimages)
{
    if (images == newimages)
        return;
    images = newimages;
    for (auto c : containers)
        c->UpdateImages();
}

bool ButtonPanel::Captions()
{
    return captions;
}

void ButtonPanel::SetCaptions(bool newcaptions)
{
    if (captions == newcaptions)
        return;
    captions = newcaptions;

    for (auto c : containers)
        c->Update();

    ContentUpdated();

}

void ButtonPanel::ContentUpdated()
{
    if (!HandleCreated())
        return;

    LayoutChildren(ClientRect(), true);
    ScrollResize();
}

void ButtonPanel::GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide)
{
    hw = 0;

    int h = 0;
    for (auto c : containers)
        h += c->MeasureHeight(uw);

    hh = max(0, h - uh);
}

void ButtonPanel::Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code)
{
    LayoutChildren(ClientRect(), true);
    base::Scrolled(kind, oldpos, pos, code);
}

void ButtonPanel::MeasureControlArea(Rect &clientrect)
{
    clientrect.top = -VPos();
}

void ButtonPanel::ButtonClicked(ToolButton *fb)
{
    if (lastfb != fb || (lastfb == fb && !fb->Down()))
    {
        if (lastfb)
        {
            if (lastfb != fb)
                lastfb->SetDown(false);
            lastfb = NULL;
        }
        if (lastfb == fb)
            return;
    }

    if (fb->Down())
    {
        if (OnUnpressButton)
            OnUnpressButton(this, EventParameters() );
        if (nvcontrol)
            UnregisterButtonPress(nvcontrol);
        lastfb = fb;
    }
}

bool ButtonPanel::ButtonPressed()
{
    return lastfb != NULL;
}

bool ButtonPanel::NVButtonPressed()
{
    return nvcontrol != NULL;
}

int ButtonPanel::PressedId()
{
    return lastfb ? lastfb->Tag() : -1;
}

NonVisualControl* ButtonPanel::PressedNVControl()
{
    return nvcontrol;
}

void ButtonPanel::PressButton(int id)
{
    if (lastfb && lastfb->Tag() != id)
    {
        lastfb->SetDown(false);
        lastfb = NULL;
    }

    if (id < 0)
        return;

    for (auto it = containers.begin(); it != containers.end(); ++it)
    {
        ToolButton *fb = (*it)->FindButton(id);
        if (fb)
        {
            if (OnUnpressButton)
                OnUnpressButton(this, EventParameters() );
            if (nvcontrol)
                UnregisterButtonPress(nvcontrol);

            lastfb = fb;
            fb->SetDown(true);
            break;
        }
    }
}

void ButtonPanel::RegisterButtonPress(NonVisualControl *control, NotifyEvent OnUnpress)
{
    if (lastfb || control != nvcontrol)
        UnpressButtons();

    nvcontrol = control;
    OnUnpressButton = OnUnpress;
}

void ButtonPanel::UnregisterButtonPress(NonVisualControl *control)
{
    if (nvcontrol != control)
        return;

    nvcontrol = NULL;
    OnUnpressButton = NULL;
}

void ButtonPanel::UnpressButtons()
{
    if (lastfb)
    {
        lastfb->SetDown(false);
        lastfb = NULL;
    }
    else if (nvcontrol)
    {
        if (OnUnpressButton)
            OnUnpressButton(this, EventParameters() );
        UnregisterButtonPress(nvcontrol);
    }
}


//---------------------------------------------


PropertyChoiceForm::PropertyChoiceForm() : lastitem(-1), mouseitem(-1)
{
    SetTopmost(true);
    SetBorderStyle(fbsNone);

    list = new Listbox();
    list->SetParent(this);
    list->SetAlignment(alClient);
    list->OnMouseMove = CreateEvent(this, &PropertyChoiceForm::listmousemove);
    list->OnMouseDown = CreateEvent(this, &PropertyChoiceForm::listmousedown);
    list->OnMouseUp = CreateEvent(this, &PropertyChoiceForm::listmouseup);
    list->OnKeyPush = CreateEvent(this, &PropertyChoiceForm::listkeypush);
    list->OnLoseFocus = CreateEvent(this, &PropertyChoiceForm::listlosefocus);
}

PropertyChoiceForm::~PropertyChoiceForm()
{
}

void PropertyChoiceForm::Destroy()
{
    list->Destroy();
    base::Destroy();
}

void PropertyChoiceForm::CreateClassParams(ClassParams &params)
{
    base::CreateClassParams(params);
    params.style << csDropShadow;
}

void PropertyChoiceForm::CreateWindowParams(WindowParams &params)
{
    base::CreateWindowParams(params);

    params.style -= wsChild;
    params.style -= wsCaption;
    params.style -= wsDlgFrame;
    params.style -= wsThickFrame;
    params.style -= wsBorder;
    params.style -= wsSysMenu;

    params.style << wsPopup;

    params.extstyle -= wsExClientEdge;
    params.extstyle -= wsExDlgModalFrame;
    params.extstyle -= wsExStaticEdge;
    params.extstyle -= wsExWindowEdge;

    params.extstyle << wsExToolWindow;
    params.extstyle << wsExNoActivate;
}

void PropertyChoiceForm::SetMouseitem(int newitem)
{
    if (mouseitem != newitem) {
        mouseitem = newitem;
        if (mouseitem >= 0)
            list->SetItemIndex(mouseitem);
    }
}

void PropertyChoiceForm::listlosefocus(void *sender, FocusChangedParameters param)
{
    Hide();
}

void PropertyChoiceForm::listmousemove(void *sender, MouseMoveParameters param)
{
    Rect r = list->ClientRect();

    if (!param.vkeys.contains(vksLeft) && (param.x < 0 || param.y < 0 || param.x >= r.Width() || param.y >= r.Height()))
        return;

    SetMouseitem( max(0, min(list->Items().Count() - 1, list->ItemAt(param.x, param.y) ) ) );
    /*
    SCROLLINFO info = {0};
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = SIF_POS;
    GetScrollInfo(list->Handle(), SB_VERT, &info);
    param.y += list->ItemHeight() * info.nPos;

    SetMouseitem(max(0, min( param.y / list->ItemHeight(), list->Items()->Count() - 1)));
    */
}

void PropertyChoiceForm::listmousedown(void *sender, MouseButtonParameters param)
{
    if (param.button != mbLeft)
        return;

    lastitem = list->ItemIndex();
}

void PropertyChoiceForm::listmouseup(void *sender, MouseButtonParameters param)
{
    if (lastitem < 0 || param.button != mbLeft || !Visible())
        return;

    if (OnPropertyChoice)
        OnPropertyChoice(this, PropertyChoiceParameters(lastitem));

    Hide();
}

void PropertyChoiceForm::listkeypush(void *sender, KeyPushParameters param)
{
    switch (param.key)
    {
    case VK_ESCAPE:
        Hide();
        break;
    case VK_RETURN:
        if (list->ItemIndex() >= 0 && OnPropertyChoice)
            OnPropertyChoice(this, PropertyChoiceParameters(list->ItemIndex()));
        Hide();
        break;
    }
}


//---------------------------------------------


PropertyData::PropertyData(const std::wstring& name, int level) : name(name), samevalue(true), open(false), level(level)
{
    ;
}

void PropertyData::AddProperty(Object *propowner, Object *propholder, DesignProperty *prop, bool isdefault)
{
    list.push_back(PropertyItem(propowner, propholder, prop, isdefault));
    if (samevalue && list.size() > 1)
        samevalue = (Value(0) == Value(list.size() - 1));
}

PropertyData PropertyData::CreateSubproperty(int subindex)
{
    DesignProperty* sub = SubProperty(subindex);
    PropertyData data(sub->Name(), Level() + 1);
    data.AddProperty(list[0].owner, list[0].item->SubHolder(list[0].holder), sub, false);

    for (auto oit = list.begin() + 1; oit != list.end(); ++oit)
    {
        DesignProperty* sub = (*oit).item->SubProperty((*oit).holder, subindex);
        data.AddProperty((*oit).owner, (*oit).item->SubHolder((*oit).holder), sub, false);
    }
    return data;
}

PropertyRowIdentityData PropertyData::Identity()
{
    return PropertyRowIdentityData(name, typeid(*list[0].item).name());
}

bool PropertyData::IsPropertyOfType(std::string proptypename)
{
    return proptypename == typeid(*list[0].item).name();
}

void PropertyData::PropertyRemoveDefault(int index)
{
    if (list[index].isdefault)
    {
        list[index].owner->Serializer()->ClearDefault();
        list[index].isdefault = false;
    }
}

void PropertyData::PropertySetToDefault(int index)
{
    list[index].owner->Serializer()->MakeDefault(list[index].item->Name());
    if (list[index].owner->Serializer()->DefaultProperty() == list[index].item)
        list[index].isdefault = true;
}

const std::wstring& PropertyData::Name()
{
    return name;
}

std::wstring PropertyData::ShortEventName()
{
    if (name.substr(0, 2) == L"On")
        return name.substr(2);
    return name;
}

unsigned int PropertyData::Level()
{
    return level;
}

bool PropertyData::Open()
{
    return open;
}

void PropertyData::SetOpen(bool newopen)
{
    open = newopen;
}

bool PropertyData::SameValue()
{
    return samevalue;
}

bool PropertyData::HasHolder(Object *propholder)
{
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if ((*it).holder == propholder)
            return true;
    }
    return false;
}

int PropertyData::OwnerCount()
{
    return list.size();
}

bool PropertyData::IsDefaultProperty(int index)
{
    return list[index].isdefault;
}

Object* PropertyData::PropOwner(int index)
{
    return list[index].owner;
}

Object* PropertyData::PropHolder(int index)
{
    return list[index].holder;
}

int PropertyData::Priority()
{
    return list[0].item->Priority();
}

const std::wstring& PropertyData::Category()
{
    return list[0].item->Category();
}

bool PropertyData::HasPropertyStyle(PropertyStyles style)
{
    return list[0].item->HasPropertyStyle(style);
}

int PropertyData::ListCount()
{
    return list[0].item->ListCount(list[0].holder);
}

std::wstring PropertyData::ListItem(int itemindex)
{
    return list[0].item->ListItem(list[0].holder, itemindex);
}

void* PropertyData::ListValue(int itemindex)
{
    return list[0].item->ListValue(list[0].holder, itemindex);
}

int PropertyData::Selected(int index)
{
    return list[index].item->Selected(list[index].holder);
}

int PropertyData::SubCount()
{
    return list[0].item->SubCount(list[0].holder);
}

DesignProperty* PropertyData::SubProperty(int subindex)
{
    return list[0].item->SubProperty(list[0].holder, subindex);
}

std::wstring PropertyData::Value(int index)
{
    return list[index].item->Value(list[index].holder);
}

bool PropertyData::IsDefault(int index)
{
    return list[index].item->IsDefault(list[index].holder);
}

bool PropertyData::HasDefault()
{
    for (unsigned int ix = 0; ix < list.size(); ++ix)
    {
        if (!list[ix].item->HasDefault())
            return false;
        if (ix > 0 && !list[0].item->DefaultsEqual(list[0].holder, list[ix].holder, list[ix].item))
            return false;
    }
    return true;
}

void PropertyData::Reset()
{
    for (unsigned int ix = 0; ix < list.size(); ++ix)
        list[ix].item->Reset(list[ix].holder);
}

bool PropertyData::IsDefault()
{
    for (unsigned int ix = 0; ix < list.size(); ++ix)
        if (!IsDefault(ix))
            return false;
    return true;
}

bool PropertyData::SetValue(Form *parentform, const std::wstring& value, bool afterselect)
{
    bool success = true;
    for (unsigned int ix = afterselect ? 1 : 0; ix < list.size(); ++ix)
        success = SetValue(parentform, ix, value);
    UpdateSamevalue();

    return success;
}

bool PropertyData::SetValue(Form *parentform, int index, const std::wstring& value)
{
    bool success = list[index].item->SetValue(parentform, list[index].holder, value);
    if (success)
        samevalue = false;
    return success;
}

bool PropertyData::SelectValue(Form *parentform, int index, void* value)
{
    return list[index].item->SelectValue(parentform, list[index].holder, value);
}

bool PropertyData::ClickEdit(Form *parentform, int index)
{
    return list[index].item->ClickEdit(parentform, list[index].holder);
}

void PropertyData::UpdateSamevalue()
{
    std::wstring value = Value(0);
    samevalue = true;
    for (unsigned int ix = 0; samevalue && ix < list.size(); ++ix)
    {
        if (value != Value(ix))
            samevalue = false;
    }
}

void PropertyData::DrawThumbImage(Canvas *c, const Rect &r, int imgindex)
{
    list[0].item->DrawThumbImage(list[0].holder, c, r, imgindex);
}

void PropertyData::MeasureListItem(MeasureItemParameters param)
{
    list[0].item->MeasureListItem(list[0].holder, param);
}

void PropertyData::DrawListItem(DrawItemParameters param)
{
    list[0].item->DrawListItem(list[0].holder, param);
}

//---------------------------------------------


PropertyListbox::PropertyListbox(PropertyListType listtype, ToolButton *abcbutton, ToolButton *catbutton) :
        listtype(listtype), showcat(false), colw(0.45f), colsizing(false), mousex(-1), mousey(-1), actionpos(-1),
        checkdown(false), checkhover(false), checkrow(-1), editchange(false), editdeleting(false),

        btnAbc(abcbutton), btnCat(catbutton)
{
    SetWantedKeyTypes(WantedKeyTypes() | wkTab);

    SetKind(lckOwnerDraw);

    SetParentColor(false);

    editor = new ControlEdit();
    editor->Hide();
    editor->SetAnchors(caTop | caLeft | caRight);
    editor->SetParent(this);
    editor->OnLeave = CreateEvent(this, &PropertyListbox::editorleave);
    //editor->OnLoseFocus = CreateEvent(this, &PropertyListbox::editorlosefocus);
    editor->OnKeyPush = CreateEvent(this, &PropertyListbox::editorkeypush);
    editor->OnTextChanged = CreateEvent(this, &PropertyListbox::editortextchanged);
    editor->OnDblClick = CreateEvent(this, &PropertyListbox::editordblclick);
    editor->OnDialogCode = CreateEvent(this, &PropertyListbox::editordlgcode);

    button = new ToolButton();
    button->Hide();
    button->SetDoubleBuffered(true);
    button->SetFlat(false);
    button->SetContentPosition(bcpCenter);
    button->SetParent(this);
    button->SetAnchors(caTop | caRight);
    button->SetText(L"6");
    button->GetFont().SetSize(10);
    button->GetFont().SetFamily(L"Webdings");
    button->GetFont().SetCharacterSet(fcsSymbol);
    button->OnMouseDown = CreateEvent(this, &PropertyListbox::buttonmousedown);
    button->OnMouseUp = CreateEvent(this, &PropertyListbox::buttonmouseup);

    listform = new PropertyChoiceForm();
    listform->OnPropertyChoice = CreateEvent(this, &PropertyListbox::listformpropertychoice);
    listform->SetBounds(RectS(0, 0, 200, 200)); // Set initial size for form to be able to measure difference between the list's client and window rectangle sizes.

    pmProp = new PopupMenu();
    pmProp->OnShow = CreateEvent(this, &PropertyListbox::pmpropshow);
    if (listtype == pltValues)
    {
        miEdit = pmProp->Add(L"Open Editor");
        miEdit->OnClick = CreateEvent(this, &PropertyListbox::pmpropedit);
        miDefault = pmProp->Add(L"Reset Value");
        miDefault->OnClick = CreateEvent(this, &PropertyListbox::pmpropdefault);
        pmProp->AddSeparator();
        miSetDefault = pmProp->Add(L"Make Default");
        miSetDefault->OnClick = CreateEvent(this, &PropertyListbox::pmpropsetdef);
    }
    else
    {
        miAssign = pmProp->Add(L"Assign Event");
        miAssign->OnClick = CreateEvent(this, &PropertyListbox::pmpropassign);
    }
    pmProp->AddSeparator();
    miAlpha = pmProp->Add(L"Alphabetic Order");
    miAlpha->SetGrouped(true);
    miAlpha->SetAutoCheck(true);
    miAlpha->SetChecked(true);
    miAlpha->OnClick = CreateEvent(this, &PropertyListbox::pmpropalpha);
    miCat = pmProp->Add(L"Order by Categories");
    miCat->SetGrouped(true);
    miCat->OnClick = CreateEvent(this, &PropertyListbox::pmpropcat);
    SetPopupMenu(pmProp);

    SetIntegralHeight(false);
}

PropertyListbox::~PropertyListbox()
{
}

void PropertyListbox::Destroy()
{
    Clear();
    base::Destroy();
}

void PropertyListbox::DeleteNotify(Object *object)
{
    base::DeleteNotify(object);

    auto it = std::find(ownerlist.begin(), ownerlist.end(), object);
    if (it == ownerlist.end())
        return;

    std::list<Object*> propowners;
    for (auto it2 = ownerlist.begin(); it2 != ownerlist.end(); ++it2)
    {
        if (*it2 == *it)
            continue;
        propowners.push_back(*it2);
    }
    SetProperties(propowners);
}

void PropertyListbox::InitHandle()
{
    base::InitHandle();

    Canvas *c = GetCanvas();
    Size s = c->MeasureText(L"yM");
    SetItemHeight(s.cy + int(5 * Scaling));

    editor->Handle();
}

LRESULT PropertyListbox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int row;
    Rect r;
    //LRESULT result;
    //INPUT inp;

    switch(uMsg)
    {
    case WM_WINDOWPOSCHANGED:
        Invalidate(false);
        break;
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
        mousex = GET_X_LPARAM(lParam);
        mousey = GET_Y_LPARAM(lParam);
        if (Cursor() != cSizeWE && !colsizing && !checkdown)
        {
            row = ItemAt(mousex, mousey);

            if (row >= 0 && row < Items().Count() && (int)Items().Data(row) >= 0)
            {
                r = EditRect(row, true);
                if (PtInRect(&r, Point(mousex, mousey)))
                {
                    base::WindowProc(uMsg, wParam, lParam);
                    PassMessage(WM_LBUTTONUP, 0, MAKELPARAM(mousex, mousey));
                    
                    if ((GetAsyncKeyState(!GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON) & ~(short)1) == 0)
                    {
                        ReleaseCapture();
                        return 0;
                    }

                    ShowEditor(row);

                    SendMessage(editor->Handle(), WM_LBUTTONDOWN, 0, MAKELPARAM(mousex - r.left, mousey - r.top));

                    return 0;
                }
            }
        }
        else
        {
            actionpos = RowRect(ItemIndex()).top + 1;
            lParam = MAKELPARAM(mousex, actionpos);
        }
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        mousex = GET_X_LPARAM(lParam);
        mousey = GET_Y_LPARAM(lParam);
        if (colsizing || checkdown)
            lParam = MAKELPARAM(lParam,  actionpos);
        break;
    case wmFormDeactivated:
        FinalizeEdit(true);
        break;
    case amClickActivate:
        if (propertylist.empty())
            break;

        PropertyData &data = propertylist[(int)lParam];
        row = DataRow((int)lParam);
        if (data.ClickEdit(ParentForm(), 0))
        {
            SetValue(row, data.Value(0), true);
            Modify();
        }
        InvalidateRect(RowRect(row));
        break;
    }
    return base::WindowProc(uMsg, wParam, lParam);
}

void PropertyListbox::CaptureChanged()
{
    colsizing = false;
    if (checkdown && checkhover)
    {
        Rect r = CheckboxRect(checkrow);
        InvalidateRect(r);
    }
    checkdown = false;
}

void PropertyListbox::SetProperties(Object *apropowner)
{
    if (!ownerlist.empty() && ownerlist.front() == ownerlist.back() && ownerlist.front() == apropowner)
        return;

    if (!apropowner)
    {
        Clear();
        return;
    }

    bool restoreselect = false;
    PropertyRowIdentity identity;
    if (ItemIndex() >= 0 && ItemIndex() < Items().Count())
    {
        identity = RowIdentity(ItemIndex());
        restoreselect = true;
    }

    Clear();

    ownerlist.push_back(apropowner);
    AddToNotifyList(apropowner, nrOwnership);

    //const char *ctrlname = typeid(*apropowner).name();

    //auto it = propertymap.find(ctrlname);

    DesignSerializer *serializer = SerializerByTypeInfo(typeid(*apropowner));
    if (serializer == NULL)
        throw L"Properties for type not initialized";
    

    DesignProperty *prop;
    std::wstring propname;

    bool guestcontrol = apropowner->ParentForm() && apropowner->ParentForm() != designer->ActiveForm();

    int cnt = listtype == pltValues ? serializer->PropertyCount(dpuListed) : serializer->EventCount(dpuListed);
    for (int ix = 0, pix = 0; pix != cnt; ++ix)
    {
        prop = listtype == pltValues ? serializer->Properties(ix) : serializer->Events(ix);
        if (!prop->IsListed())
            continue;
        ++pix;

        if (guestcontrol && !prop->HasPropertyStyle(psGuestEditable))
            continue;

        propname = prop->Name();

        PropertyData data(propname, 0);
        data.AddProperty(apropowner, apropowner, prop, prop == serializer->DefaultProperty());

        propertylist.push_back(data);
        Items().Add(propname, (void*)(propertylist.size() - 1));
    }

    Sort(showcat);

    if (restoreselect)
        Select(identity, true);

    if (ItemIndex() < 0)
        SelectDefault();
}

void PropertyListbox::SetProperties(const std::list<Object*> &propowners)
{
    if (ownerlist == propowners)
        return;

    if (propowners.empty())
    {
        Clear();
        return;
    }

    bool restoreselect = false;
    PropertyRowIdentity identity;
    if (ItemIndex() >= 0 && ItemIndex() < Items().Count())
    {
        identity = RowIdentity(ItemIndex());
        restoreselect = true;
    }

    Clear();

    ownerlist.assign(propowners.begin(), propowners.end());
    for (auto it = ownerlist.begin(); it != ownerlist.end(); ++it)
        AddToNotifyList(*it, nrOwnership);

    bool guestcontrol = false;

    DesignSerializer *serializer = NULL;
    for (auto obj : ownerlist)
    {
        DesignSerializer *tmp = SerializerByTypeInfo(typeid(*obj));
        if (tmp == NULL)
            throw L"Properties for type not initialized!";
        if (!serializer)
            serializer = tmp;
        if (!guestcontrol && obj->ParentForm() && obj->ParentForm() != designer->ActiveForm())
            guestcontrol = true;
    }


    std::vector<int> matches;
    int cnt = listtype == pltValues ? serializer->PropertyCount(dpuListed) : serializer->EventCount(dpuListed);
    for (int ix = 0, lix = 0; lix != cnt; ++ix)
    {
        DesignProperty *prop = listtype == pltValues ? serializer->Properties(ix) : serializer->Events(ix);
        if (!prop->IsListed())
            continue;
        ++lix;

        if ((guestcontrol && !prop->HasPropertyStyle(psGuestEditable)) || (propowners.front() != propowners.back() && !prop->HasPropertyStyle(psEditShared)))
            continue;

        bool match = true;

        for (auto oit = ++ownerlist.begin(); match && oit != ownerlist.end(); ++oit)
        {
            DesignSerializer *oserializer = SerializerByTypeInfo(typeid(*(*oit)));
            bool found = false;
            int ocnt = listtype == pltValues ? oserializer->PropertyCount(dpuListed) : oserializer->EventCount(dpuListed);
            for (int pix = 0, llix = 0; llix != ocnt; ++pix)
            {
                DesignProperty *pprop = listtype == pltValues ? oserializer->Properties(pix) : oserializer->Events(pix);
                if (!pprop->IsListed())
                    continue;
                ++llix;

                if (pprop->Name() == prop->Name())
                {
                    match = (pprop->CreatorType() == prop->CreatorType());
                    found = true;
                    break;
                }
            }
            if (!found)
                match = false;
        }

        if (match)
            matches.push_back(ix);
    }

    for (unsigned int mix = 0; mix < matches.size(); ++mix)
    {
        DesignProperty *mprop = listtype == pltValues ? serializer->Properties(matches[mix]) : serializer->Events(matches[mix]);
        std::wstring propname = mprop->Name();

        PropertyData data(propname, 0);
        data.AddProperty(*ownerlist.begin(), *ownerlist.begin(), mprop, mprop == serializer->DefaultProperty());

        for (auto oit = ++ownerlist.begin(); oit != ownerlist.end(); ++oit)
        {
            DesignSerializer *oserializer = SerializerByTypeInfo(typeid(*(*oit)));

            int ocnt = listtype == pltValues ? oserializer->PropertyCount(dpuListed) : oserializer->EventCount(dpuListed);
            for (int pix = 0, llix = 0; llix != ocnt; ++pix)
            {
                DesignProperty *pprop = listtype == pltValues ? oserializer->Properties(pix) : oserializer->Events(pix);
                if (!pprop->IsListed())
                    continue;
                ++llix;

                if (pprop->Name() != mprop->Name())
                    continue;

                data.AddProperty(*oit, *oit, pprop, pprop == oserializer->DefaultProperty());
                break;
            }
        }

        propertylist.push_back(data);
        Items().Add(propname, (void*)(propertylist.size() - 1));
    }

    Sort(showcat);

    if (restoreselect)
        Select(identity, true);

    if (ItemIndex() < 0)
        SelectDefault();
}

bool PropertyListbox::IsPropertyOwner(Object *searchowner)
{
    for (auto it = ownerlist.begin(); it != ownerlist.end(); ++it)
        if (*it == searchowner)
            return true;
    return false;
}

bool PropertyListbox::MainPropertyOwner(Object *searchowner)
{
    return !ownerlist.empty() && ownerlist.front() == searchowner;
}

DesignFormBase* PropertyListbox::PropertyOwnerForm()
{
    if (ownerlist.empty())
        return NULL;
    if (dynamic_cast<DesignFormBase*>(ownerlist.front()) != NULL)
        return (DesignFormBase*)ownerlist.front();
    return dynamic_cast<DesignFormBase*>(ownerlist.front()->ParentForm());
}

void PropertyListbox::Clear()
{
    FinalizeEdit(true);
    button->Hide();
    Items().Clear();
    propertylist.clear();
    for (auto it = ownerlist.begin(); it != ownerlist.end(); ++it)
        RemoveFromNotifyList(*it, nrNoReason);
    ownerlist.clear();
}

void PropertyListbox::MouseEnter()
{
}

void PropertyListbox::MouseLeave()
{
    if (checkdown && checkhover)
    {
        Rect r = CheckboxRect(checkrow);
        InvalidateRect(r);
    }
}

void PropertyListbox::SelChanged()
{
    base::SelChanged();

    int row = ItemIndex();
    if (row < 0 || row >= Items().Count() || (int)Items().Data(row) < 0)
    {
        button->Hide();
        return;
    }

    PlaceButton(row);
}

void PropertyListbox::PlaceButton(int row)
{
    int datapos;
    if (row < 0 || row >= Items().Count() || (datapos = (int)Items().Data(row)) < 0)
    {
        button->Hide();
        return;
    }
    PropertyData data = propertylist[datapos];
    if ((data.HasPropertyStyle(psReadonly) || data.ListCount() == 0) && !data.HasPropertyStyle(psEditButton))
    {
        button->Hide();
        return;
    }

    button->SetBounds(ButtonRect(row));
    if (data.HasPropertyStyle(psEditButton))
        button->SetText(L"@");
    else
        button->SetText(L"6");
    button->Show();
}

void PropertyListbox::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
{
    int datapos;
    if (!colsizing && !checkdown && !vkeys.contains(vksLeft) && !vkeys.contains(vksRight))
    {
        int d = abs(GetSystemMetrics(SM_CXDRAG));
        int w = ClientRect().Width() * colw;

        int row = TopRow() + y / ItemHeight();
        bool hascheck = false;

        if (row >= 0 && row < Items().Count() && (datapos = (int)Items().Data(row)) >= 0)
        {
            hascheck = propertylist[datapos].HasPropertyStyle(psCheckbox);
            if (checkhover && row != checkrow && checkrow >= 0 && checkrow < Items().Count())
            {
                Rect r = CheckboxRect(checkrow);
                checkhover = false;
                InvalidateRect(r);
            }
            checkrow = row;
        }

        if (x >= w - d && x < w + (hascheck ? int(2 * Scaling) : d) )
            SetCursor(cSizeWE);
        else
        {
            Rect r = EditRect(row, true);
            if (row >= 0 && row < Items().Count() && PtInRect(&r, Point(x, y)))
                SetCursor(cIBeam);
            else
                SetCursor(cDefault);
        }
    }
    else if (colsizing)
    {
        SetCursor(cSizeWE);

        Rect r = ClientRect();
        colw = min(0.9f, max(0.1f, float(x) / r.Width()));

        Invalidate(false);
    }
    else if (!checkdown && vkeys.contains(vksLeft))
        PlaceButton(ItemAt(x, y));

    if (checkdown || (!colsizing && !vkeys.contains(vksLeft) && !vkeys.contains(vksRight)))
    {
        bool hover = checkhover;
        int row = checkdown ? checkrow : TopRow() + mousey / ItemHeight();

        if (row >= 0 && row < Items().Count() && (datapos = (int)Items().Data(row)) >= 0)
        {
            PropertyData data = propertylist[datapos];
            if (data.HasPropertyStyle(psCheckbox) && !data.HasPropertyStyle(psReadonly))
            {
                Rect r = CheckboxRect(row);
                hover = PtInRect(&r, Point(mousex, mousey)) == TRUE;
                if (hover != checkhover)
                {
                    checkhover = hover;
                    InvalidateRect(r);
                }
            }
        }
    }

    base::MouseMove(x, y, vkeys);
}

void PropertyListbox::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    if (button != mbLeft)
        return;

    if (Cursor() == cSizeWE)
    {
        actionpos = RowRect(ItemIndex()).top + 1;
        colsizing = true;
        FinalizeEdit(true);
        MouseMove(x, y, vkeys);
        return;
    }

    if (checkhover)
    {
        Rect r = CheckboxRect(ItemAt(x, y));
        checkrow = TopRow() + y / ItemHeight();
        actionpos = y;
        FinalizeEdit(true);
        checkdown = true;
        InvalidateRect(r);
        return;
    }

    int row = ItemAt(x, y);
    int datapos;
    if (row >= 0 && row < Items().Count())
    {
        if ((datapos = (int)Items().Data(row)) >= 0)
        {
            PropertyData &data = propertylist[datapos];
            bool opened = false;
            if (data.SubCount() > 0)
            {
                Rect r = BoxRect(row);
                if (PtInRect(&r, Point(x, y)))
                {
                    if (data.Open())
                        CloseDataRow(row);
                    else
                        OpenDataRow(row);
                    opened = true;

                    // Futile attempt to prevent further mouse messages after opening and closing data rows.
                    //MSG msg;
                    //while (PeekMessage(&msg ,Handle(), WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE /*| PM_QS_INPUT | PM_QS_SENDMESSAGE | PM_QS_POSTMESSAGE*/))
                    //{
                    //    if (msg.message == WM_QUIT)
                    //    {
                    //        PostMessage(application->Handle(), WM_QUIT, 0, 0);
                    //        break;
                    //    }
                    //}

                    //INPUT input;
                    //input.type = INPUT_MOUSE;
                    //input.mi.dx = 0;
                    //input.mi.dy = 0;
                    //input.mi.mouseData = 0;
                    //input.mi.time = 0;
                    //input.mi.dwExtraInfo = 0;
                    //input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    //SendInput(1, &input, sizeof(INPUT));

                    //SetCapture(Handle());
                    //ReleaseCapture();

                    //BYTE ks[256];
                    //GetKeyboardState(ks);
                    //ks[VK_LBUTTON] = 1;
                    //SetKeyboardState(ks);

                }
            }
            if (!opened && data.HasPropertyStyle(psThumbImage) && data.HasPropertyStyle(psEditButton))
            {
                Rect r = ThumbRect(row);
                if (PtInRect(&r, Point(x, y)))
                {
                    PostMessage(Handle(), WM_LBUTTONUP, 0, MAKELPARAM(x, y));
                    PostMessage(Handle(), amClickActivate, 0, datapos);
                }
            }
        }
        else if (datapos >= -2)
        {
            Rect r = CategoryBoxRect(row);
            if (vkeys.contains(vksDouble) || PtInRect(&r, Point(x, y)))
            {
                if (datapos == -1)
                    CloseCategory(row);
                else
                    OpenCategory(row);
            }
        }
    }

    //PlaceButton(row);
}

void PropertyListbox::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    if (button != mbLeft)
        return;

    colsizing = false;

    int sel = ItemIndex();
    int datapos = sel >= 0 && sel < Items().Count() ? (int)Items().Data(sel) : -1;
    if (datapos >= 0 && checkhover && checkdown)
        SetValue(sel, propertylist[datapos].Value(0) != L"True" ? L"True" : L"False");
    checkdown = false;
}

bool PropertyListbox::SetValue(int row, const std::wstring& value, bool afterselect)
{
    int datapos = (int)Items().Data(row);
    PropertyData &prop = propertylist[datapos];
    InvalidateRow(row);
    if (!prop.SetValue(ParentForm(), value, afterselect))
        return false;
    Modify();
    return prop.SameValue();
}

void PropertyListbox::Modify()
{
    if (ownerlist.front()->DesignParent())
        ownerlist.front()->DesignParent()->Modify();
    else if (ownerlist.front()->SubOwner() && ownerlist.front()->SubOwner()->DesignParent())
        ownerlist.front()->SubOwner()->DesignParent()->Modify();
}

void PropertyListbox::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
{
    base::KeyPush(keycode, key, vkeys);

    int sel = ItemIndex();
    if (sel >= 0 && keycode == VK_TAB && (int)Items().Data(sel) >= 0)
    {
        ShowEditor(sel);
        editor->Focus();
        editor->SetSelStartAndLength(0, editor->Text().length());
    }
}

void PropertyListbox::KeyUp(WORD &keycode, VirtualKeyStateSet vkeys)
{
    base::KeyUp(keycode, vkeys);
}

bool PropertyListbox::HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures)
{
    if ((int)measures->itemID < 0)
        return true;

    int cnt = Items().Count();
    if ((int)measures->itemID >= cnt)
        return true;
        
    Canvas *c = GetCanvas();
    //c->SetPixelOffsetMode(pomHighQuality);

    int datapos = (int)Items().Data(measures->itemID);
    if (datapos < 0)
    {
        Rect r = measures->rcItem;
        c->SetBrush( (measures->itemState & ODS_SELECTED) == ODS_SELECTED ? MixColors(Color(220, 220, 220), clHighlight) : Color(220, 220, 220));
        c->GetFont().SetColor((measures->itemState & ODS_SELECTED) == ODS_SELECTED ? clHighlightText : clBlack);
        c->GetFont().SetBold(true);
        c->FillRect(r);

        c->SetBrush(Color(190, 190, 190));
        Gdiplus::GraphicsPath gp;
        gp.AddLine((int)r.left, (int)r.bottom, (int)r.left, (int)r.top);
        gp.AddLine((int)r.left + int(2 * Scaling) + int(8 * Scaling), (int)r.top, (int)r.left + int(2 * Scaling) + int(8 * Scaling) + ItemHeight() / 2.5, (int)r.top + ItemHeight() / 2);
        gp.AddLine((int)r.left + int(2 * Scaling) + int(8 * Scaling) + ItemHeight() / 2.5, (int)r.top + ItemHeight() / 2, (int)r.left + int(2 * Scaling) + int(8 * Scaling), (int)r.bottom);
        gp.CloseFigure();
        c->SetAntialias(true);
        c->FillPath(&gp);
        c->SetAntialias(false);
        std::wstring str = Items().Text(measures->itemID);
        if (str == L"")
            str = L"General";
        Size s = c->MeasureText(str);
        c->TextDraw(r, r.left + int(2 * Scaling) * 2 + int(16 * Scaling), r.top + (r.Height() - s.cy) / 2, str); 

        r = CategoryBoxRect(measures->itemID);
        DrawCategoryBox(r, datapos == -1);
        return true;
    }

    PropertyData &prop = propertylist[datapos];

    int firstdatapos = (int)measures->itemID > 0 ? (int)Items().Data(measures->itemID - 1) : -1;
    bool subfirst = (int)firstdatapos >= 0 && propertylist[firstdatapos].Level() < prop.Level();
    int lastdatapos = (int)measures->itemID < Items().Count() - 1 ? (int)Items().Data(measures->itemID + 1) : -1;
    bool sublast = (int)lastdatapos < 0 || ((prop.SubCount() && prop.Level() >= propertylist[lastdatapos].Level()) || (prop.Level() > 0 && prop.Level() > propertylist[lastdatapos].Level())) ;
    bool nextsubfirst = !subfirst && !sublast && lastdatapos >= 0 && (int)measures->itemID < Items().Count() - 1 && propertylist[lastdatapos].SubCount();

    Rect r1 = measures->rcItem;
    r1.right = r1.Width() * colw;
    Rect r2 = measures->rcItem;
    r2.left = r1.right;

    //r1.bottom--;
    //r2.bottom--;

    Color col1;
    Color col2;

    bool defprop = prop.OwnerCount() == 1 && prop.IsDefaultProperty(0);

    if ((measures->itemState & ODS_SELECTED) == ODS_SELECTED)
    {
        col1 = clHighlight;
        col2 = clHighlight;
        c->GetFont().SetColor(clHighlightText);
    }
    else
    {
        col1 = !subfirst && !prop.SubCount() ? Color(245, 245, 245) : Color(255, 255, 255);
        col2 = Color(255, 255, 255);
        c->GetFont().SetColor(clBlack);

        if (defprop)
            col1 = col1.Mix(col1 == Color(255, 255, 255) ? Color(225, 210, 130) : Color(255, 240, 150), 0.7);
    }


    // Decorative darker line on the left.
    c->SetBrush(Color(190, 190, 190));
    c->FillRect(Rect(r1.left, r1.top, r1.left + int(4 * Scaling), r1.bottom));
    r1.left += 4 * Scaling;

    c->SetPixelOffsetMode(pomHighQuality);
    // Property name column:
    if (prop.SubCount()) 
    {
        c->SetBrush(col1);
        c->FillRect(r1);
    }
    else if (sublast)
    {
        c->GradientRect(r1.Offset(0, 0, 0, -1), col1, MixColors(col1, Color(128, 128, 128), 0.8), lgmVertical);
    }
    else
    {
        c->GradientRect(r1.Offset(0, 0, 0, -1), col1, MixColors(col1, Color(200, 200, 200), 0.8), lgmVertical);
    }
    c->SetPixelOffsetMode(pomDefault);

    Rect cr = BoxRect(measures->itemID);
    if (!prop.SubCount())
        cr.right = cr.left;

    c->SetBrush(Color(190, 190, 190));
    if (!subfirst)
    {
        Rect fillrect = Rect(r1.left, r1.top, cr.left - int(2 * Scaling), r1.bottom);
        c->FillRect(fillrect);
    }
    else
    {
        Gdiplus::GraphicsPath path;
        int r = cr.left - int(2 * Scaling);
        int w = r - r1.left;
        path.AddLine(r1.left, r1.bottom, r1.left, r1.top);
        path.AddBezier(r1.left, r1.top, r1.left + w * 0.9, r1.top + r1.Height() * 0.2, r, r1.top + r1.Height() * 0.4,  r, r1.bottom);
        path.AddLine(r, r1.bottom, r1.left, r1.bottom);
        path.CloseFigure();
        c->SetAntialias(true);
        c->FillPath(&path);
        c->SetAntialias(false);
    }

    if (prop.SubCount())
        DrawBox(cr, prop.Open());

    std::wstring t = Items().Text(measures->itemID);


    Size s = c->MeasureText(L"My");
    c->TextDraw(r1, cr.left + cr.Width() + int(2 * Scaling) * 2, r1.top + (r1.Height() - s.cy) / 2, t);

    c->SetPen(sublast || nextsubfirst ? Color(200, 200, 200) : Color(240, 240, 240));
    c->Line(max(r1.left, cr.left - int(2 * Scaling)), r1.bottom - 1, r2.right, r1.bottom - 1);

    if (col1 == clHighlight)
    {
        col1 = Color(255, 250, 250, 250);
        col2 = Color(255, 255, 255, 255);
        c->GetFont().SetColor(clBlack);
    }

    // Value column:
    c->SetBrush(col2);
    --r2.bottom;
    c->FillRect(r2);

    if (((!prop.HasPropertyStyle(psReadonly) && prop.ListCount()) || prop.HasPropertyStyle(psEditButton)) && (measures->itemState & ODS_SELECTED) == ODS_SELECTED)
        r2.right -= button->WindowRect().Width();

    if (prop.SameValue())
    {
        if (!prop.IsDefault())
            c->GetFont().SetBold(true);

        t = prop.Value(0);
        c->TextDraw(r2, r2.left + int(2 * Scaling) * 2 + (prop.HasPropertyStyle(psThumbImage) ? ItemHeight() - int(Scaling) : 0)  + (prop.HasPropertyStyle(psCheckbox) ? int(13 * Scaling) + int(2 * Scaling): 0), r2.top + (r2.Height() - s.cy) / 2, t);
    }

    if (prop.HasPropertyStyle(psCheckbox))
    {
        Rect r = CheckboxRect(measures->itemID);
        DrawCheckbox(r, prop.Value(0) == L"True", !prop.HasPropertyStyle(psReadonly) && checkrow == (int)measures->itemID && checkhover, !prop.HasPropertyStyle(psReadonly) && checkrow == (int)measures->itemID && checkhover && checkdown, !prop.HasPropertyStyle(psReadonly));
    }

    if (prop.SameValue() && prop.HasPropertyStyle(psThumbImage))
    {
        Rect thumbrect = ThumbRect(measures->itemID); 
        auto state = c->SaveState();
        c->SetClip(thumbrect);
        prop.DrawThumbImage(c, thumbrect, INT_MAX);
        c->RestoreState(state);
    }

    if (button->Visible() || editor->Visible())
    {
        if (editor->Visible())
        {
            Rect r = EditRect(ItemIndex(), false);
            Rect r2 = editor->WindowRect();
            if (r != r2)
                editor->DelayedSetBounds(r);
        }
        if (button->Visible())
        {
            button->DelayedSetBounds(ButtonRect(ItemIndex()));
        }
    }

    return true;
}

void PropertyListbox::EraseBackground()
{
    Rect r = RowRect(Count() - 1);
    if (r.Empty())
        return;

    Canvas *c = GetCanvas();
    Rect cr = ClientRect();

    c->FillRect(cr.left, r.bottom, cr.right, cr.bottom);
}

Rect PropertyListbox::ButtonRect(int row)
{
    Rect r = RowRect(row);
    int siz = r.Height(); 
    int top = TopRow();
    if (top > row)
        r.top = -(top - row) * siz;
    return RectS(r.right - siz - 1, r.top - 1, siz + 1, siz + 1);
}

void PropertyListbox::ShowEditor(int row)
{
    int datapos;
    if (row < 0 || row >= Items().Count() || (datapos = (int)Items().Data(row)) < 0)
        return;

    editor->Hide();
    Rect r = EditRect(row, false);

    PropertyData &prop = propertylist[datapos];
    if (prop.SameValue())
        editor->SetText(prop.Value(0));
    else
        editor->SetText(L"");
    origtext = editor->Text();
    editor->SetModified(false);

    editor->SetBounds(r);
    Size s = GetCanvas()->MeasureText(L"My");
    editor->SetTopMargin((r.Height() - s.cy) / 2);
    //editor->SetTopMargin(2 * Scaling);
    editor->SetVisible(true);
}

Rect PropertyListbox::EditRect(int row, bool fullheight)
{
    int datapos;
    if (row < 0 || row >= Items().Count() || (datapos = (int)Items().Data(row)) < 0)
        return Rect(0,0,0,0);

    Rect r = RowRect(row);
    PropertyData &prop = propertylist[datapos];

    r.left = r.Width() * colw + int(2 * Scaling) + (prop.HasPropertyStyle(psThumbImage) ? ItemHeight() - int(Scaling) : 0) + (prop.HasPropertyStyle(psCheckbox) ? int(13 * Scaling) + int(2 * Scaling): 0);
    //Canvas *c = GetCanvas();
    
    if (!fullheight)
    {
        //Size s = c->MeasureText(L"My");
        r.bottom--;
        //r.top = r.top + (r.Height() - s.cy) / 2;
    }

    if ((!prop.HasPropertyStyle(psReadonly) && prop.ListCount()) || prop.HasPropertyStyle(psEditButton))
        r.right -= button->WindowRect().Width() - 1;
    return r;
}

Rect PropertyListbox::ThumbRect(int row)
{
    Rect r = RowRect(row);
    r.bottom--;

    r.left = r.left + r.Width() * colw;
    return RectS(r.left + int(2 * Scaling), r.top + int(Scaling), r.Height() - int(Scaling), ItemHeight() - int(Scaling) - int(2 * Scaling));
}

void PropertyListbox::FinalizeEdit(bool hide)
{
    if (!editor->Visible())
        return;

    if (!editor->Modified())
    {
        if (hide)
            editor->Hide();
        return;
    }
    editor->SetModified(false); // Duplicate, see below. Avoids execution to pass here twice before the function quits, as a result of calling SetValue might cause FinalizeEdit to be called in an infinite loop.

    int datapos = (int)Items().Data(ItemIndex());
    PropertyData &prop = propertylist[datapos];
    SetValue(ItemIndex(), editor->Text());

    if (hide)
        editor->Hide();
    else
    {
        if (prop.SameValue())
        {
            editor->SetText(prop.Value(0));
            editor->SetSelStartAndLength(0, editor->Text().size());
            origtext = editor->Text();
        }
        else
        {
            editor->SetText(L"");
            origtext = L"";
        }
        ReleaseCapture();
        editor->Show();
        editor->Focus();
    }

    editor->SetModified(false); // Mark the editor as unmodified for good.

    SelChanged();

    InvalidateRow(ItemIndex());
}

void PropertyListbox::RestoreEdit()
{
    if (!editor->Visible())
        return;

    if (editor->Text() != origtext)
        editor->SetText(origtext);

    int datapos = (int)Items().Data(ItemIndex());
    PropertyData &prop = propertylist[datapos];
    if (prop.HasPropertyStyle(psImmediateUpdate))
        FinalizeEdit(true);
    else
        editor->Hide();
}

void PropertyListbox::editorleave(void *sender, ActiveChangedParameters param)
{
    FinalizeEdit(true);
}

//void PropertyListbox::editorlosefocus(void *sender, FocusChangedParameters param)
//{
//    FinalizeEdit(true);
//}

int PropertyListbox::FirstEditableRow()
{
    int cnt = Items().Count();
    for (int ix = 0; ix != cnt; ++ix)
        if ((int)Items().Data(ix) >= 0)
            return ix;
    return -1;
}

int PropertyListbox::LastEditableRow()
{
    int cnt = Items().Count();
    for (int ix = cnt - 1; ix != -1; --ix)
        if ((int)Items().Data(ix) >= 0)
            return ix;
    return -1;
}

int PropertyListbox::NextEditableRow(int row)
{
    int cnt = Items().Count();
    if (row < 0 || row >= cnt)
        return -1;
    for (int ix = row + 1; ix != cnt; ++ix)
        if ((int)Items().Data(ix) >= 0)
            return ix;
    return -1;
}

int PropertyListbox::PrevEditableRow(int row)
{
    int cnt = Items().Count();
    if (row < 0 || row >= cnt)
        return -1;
    for (int ix = row - 1; ix != -1; --ix)
        if ((int)Items().Data(ix) >= 0)
            return ix;
    return -1;
}

void PropertyListbox::editorkeypush(void *sender, KeyPushParameters param)
{
    int cnt, i, j;
    
    if (param.keycode != 0)
    {
        if (param.keycode == VK_DELETE)
        {
            ScrollIntoView();

            int datapos = (int)Items().Data(ItemIndex());
            if (datapos < 0)
                return;

            PropertyData &prop = propertylist[datapos];

            if (prop.HasPropertyStyle(psReadonly))
                param.keycode = 0;
            else
                editdeleting = true;
            return;
        }

        switch (param.keycode)
        {
        case VK_PRIOR:
        case VK_NEXT:
        case VK_UP:
        case VK_DOWN:
            if (((param.keycode == VK_UP || param.keycode == VK_PRIOR) && ItemIndex() == FirstEditableRow()) || ((param.keycode == VK_DOWN || param.keycode == VK_NEXT) && ItemIndex() == LastEditableRow()))
            {
                param.keycode = 0;
                break;
            }

            cnt = Items().Count();
            FinalizeEdit(true);

            i = (param.keycode == VK_DOWN ? 1 : param.keycode == VK_UP ? -1 : param.keycode == VK_PRIOR ? -max(1, ClientHeight() / ItemHeight() - 1) : max(1, ClientHeight() / ItemHeight() - 1));
            if (i < 0)
            {
                j = PrevEditableRow(max(1, ItemIndex() + 1 + i));
                if (j < 0)
                    i = NextEditableRow(max(0, ItemIndex() + i));
                else
                    i = j;
            }
            else
            {
                j = NextEditableRow(min(cnt - 2, ItemIndex() - 1 + i));
                if (j < 0)
                    i = PrevEditableRow(min(cnt - 1, ItemIndex() + i));
                else
                    i = j;
            }

            if (i < 0)
            {
                param.keycode = 0;
                break;
            }

            SetItemIndex(i);
            SelChanged();
            if ((int)Items().Data(i) >= 0)
            {
                ShowEditor(i);
                editor->Focus();
                editor->SetSelStartAndLength(0, editor->Text().length());
            }
            param.keycode = 0;
            break;
        default:
            ScrollIntoView();
            break;
        }
    }
    else if (param.key != 0)
    {
        int datapos = (int)Items().Data(ItemIndex());
        if (datapos < 0)
            return;

        PropertyData &prop = propertylist[datapos];
        switch(param.key)
        {
        case VK_ESCAPE:
            RestoreEdit();
            param.key = 0;
            break;
        case VK_RETURN:
            FinalizeEdit(false);
            param.key = 0;
            break;
        case VK_BACK:
            if (prop.HasPropertyStyle(psReadonly))
                param.key = 0;
            else
                editdeleting = true;
            break;
        default:
            if (prop.HasPropertyStyle(psReadonly))
                param.key = 0;
        }
    }
}

void PropertyListbox::editortextchanged(void *sender, EventParameters param)
{
    if (!editor->Visible() || editchange)
        return;
    int datapos = (int)Items().Data(ItemIndex());
    if (datapos < 0)
        return;

    PropertyData &prop = propertylist[datapos];
    if (editor->Text() == origtext && !prop.HasPropertyStyle(psImmediateUpdate))
        return;

    unsigned int elen = editor->Text().length();
    if (elen > 0 && !editdeleting)
    {
        std::wstring etext = GenToLower(editor->Text());
        for (int ix = 0; ix < prop.ListCount(); ++ix)
        {
            std::wstring itemstr = prop.ListItem(ix);
            if (GenToLower(itemstr.substr(0, elen)) == etext)
            {
                if (itemstr.length() == elen)
                    break;

                editchange = true;
                editor->SetText(itemstr);
                editor->SetSelStartAndLength(elen, editor->Text().length() - elen);
                break;
            }
        }
        editchange = false;
    }
    editdeleting = false;

    if (prop.HasPropertyStyle(psImmediateUpdate))
        SetValue(ItemIndex(), editor->Text());
}

void PropertyListbox::editordblclick(void *sender, MouseButtonParameters param)
{
    int row = ItemIndex();
    int datapos;
    if (row < 0 || row >= Items().Count() || (datapos = (int)Items().Data(row)) < 0)
        return;

    PropertyData &prop = propertylist[datapos];

    if (prop.HasPropertyStyle(psReadonly) || prop.ListCount() == 0)
    {
        if (prop.HasPropertyStyle(psEditButton))
            PostMessage(Handle(), amClickActivate, 0, (LPARAM)datapos);
        return;
    }

    int sel = prop.Selected(0);
    int origsel = sel;

    do
    {
        if (sel == prop.ListCount() - 1)
            sel = 0;
        else
            sel++;
       if (SetValue(row, prop.ListItem(sel)))
           break;
    } while(sel != origsel);

    editor->SetText(prop.SameValue() ? prop.ListItem(sel) : L"");
    InvalidateRow(ItemIndex());
}

void PropertyListbox::editordlgcode(void *sender, DialogCodeParameters param)
{
    param.result |= dcWantAllKeys;
}

Rect PropertyListbox::BoxRect(int row)
{
    int datapos = (int)Items().Data(row);
    Rect r = RowRect(row);
    Size s = themes->MeasureTreeviewGlyph(ttgsOpen);
    int csizex = s.cx * Scaling;
    int csizey = s.cy * Scaling;
    int rleft = r.left + int(4 * Scaling) + int(2 * Scaling) + int(propertylist[datapos].Level() * 6 * Scaling);
    int rtop = r.top + (r.Height() - csizey) / 2;
    return Rect(rleft, rtop, rleft + csizex, rtop + csizey);
}

Rect PropertyListbox::CategoryBoxRect(int row)
{
    Rect r = RowRect(row);
    int csizex = 8 * Scaling;
    int csizey = 8 * Scaling;
    int rleft = r.left + int(2 * Scaling);
    int rtop = r.top + (r.Height() - csizey) / 2;
    return Rect(rleft, rtop, rleft + csizex, rtop + csizey);
}

Rect PropertyListbox::CheckboxRect(int row)
{
    Rect r = RowRect(row);
    return RectS(r.left + r.Width() * colw + int(2 * Scaling), r.top + (r.Height() - int(13 * Scaling)) / 2, 13 * Scaling, 13 * Scaling);
}

void PropertyListbox::DrawBox(const Rect &rect, bool open)
{
    Canvas *c = GetCanvas();
    themes->DrawTreeviewGlyph(c, rect, open ? ttgsOpen : ttgsClosed);
}

void PropertyListbox::DrawCategoryBox(const Rect &rect, bool open)
{
    Canvas *c = GetCanvas();
    c->SetPen( Color(92, 0, 0, 0) );
    c->FrameRect(rect);
    c->Line(rect.left + int(2 * Scaling), rect.top + rect.Height() / 2 - 1, rect.right - int(2 * Scaling) - 1, rect.top + rect.Height() / 2 - 1);
    c->Line(rect.left + int(2 * Scaling), rect.top + rect.Height() / 2, rect.right - int(2 * Scaling) - 1, rect.top + rect.Height() / 2 );
    if (!open)
    {
        c->Line(rect.left + rect.Width() / 2 - 1, rect.top + int(2 * Scaling), rect.left + rect.Width() / 2 - 1, rect.bottom - int(2 * Scaling) - 1);
        c->Line(rect.left + rect.Width() / 2, rect.top + int(2 * Scaling), rect.left + rect.Width() / 2, rect.bottom - int(2 * Scaling) - 1);
    }
}

void PropertyListbox::DrawCheckbox(const Rect &rect, bool checked, bool hovered, bool down, bool enabled)
{
    Canvas *c = GetCanvas();
    themes->DrawCheckbox(c, rect, !enabled ? (checked ? cbsCheckedDisabled : cbsUncheckedDisabled) : checked ? (down ? cbsCheckedPressed : hovered ? cbsCheckedHot : cbsCheckedNormal) : (down ? cbsUncheckedPressed : hovered ? cbsUncheckedHot : cbsUncheckedNormal) );
}

void PropertyListbox::OpenDataRow(int row)
{
    int datapos = (int)Items().Data(row);
    if (datapos < 0)
        return;

    PropertyData *prop = &propertylist[datapos];
    if (prop->Open())
        return;
    prop->SetOpen(true);

    int hidden = 0;
    BeginUpdate();
    for (int ix = 0; ix < prop->SubCount(); ++ix)
    {
        DesignProperty* sub = prop->SubProperty(ix);
        if (!sub->IsListed())
        {
            hidden++;
            continue;
        }

        PropertyData nprop = prop->CreateSubproperty(ix);
        propertylist.insert(propertylist.begin() + datapos + ix + 1 - hidden, nprop);
        prop = &propertylist[datapos]; // Expanding propertylist might invalidate data, though most of the time the value will stay the same.

        for (int iy = 0; iy < Items().Count(); ++iy)
        {
            int val = (int)Items().Data(iy);
            if (val > datapos + ix - hidden)
                Items().SetData(iy, (void*)(val + 1));
        }

        Items().Insert(row + ix + 1 - hidden, sub->Name(), (void*)(datapos + ix + 1 - hidden));

    }
    EndUpdate();
}

void PropertyListbox::CloseDataRow(int row)
{
    int datapos = (int)Items().Data(row);
    if (datapos < 0)
        return;

    PropertyData *prop = &propertylist[datapos];
    if (!prop->Open())
        return;

    if (editor->Visible())
        FinalizeEdit(true);

    prop->SetOpen(false);

    int len = propertylist.size(); // Items()->Count();
    int removed = 0;
    BeginUpdate();
    while (datapos + 1 < len)
    {
        PropertyData *prop2 = &propertylist[datapos + 1];
        if (prop->Level() == prop2->Level())
            break;
        propertylist.erase(propertylist.begin() + datapos + 1);
        prop = &propertylist[datapos]; // Removing an item might reposition the data in propertylist, so the variable must be updated.
        Items().Delete(row +  1);
        len--;
        removed++;
    }
    for (int ix = 0; ix < Items().Count(); ++ix)
    {
        int val = (int)Items().Data(ix);
        if (val > datapos)
            Items().SetData(ix, (void*)(val - removed));
    }
    EndUpdate();
}

void PropertyListbox::OpenCategory(int row)
{
    int datapos = (int)Items().Data(row);
    if (datapos != -2)
        return;
    FinalizeEdit(true);

    Items().SetData(row, (void*)-1);

    std::vector< std::vector<int> > poslist;
    std::wstring category = Items().Text(row);

    bool skip = false;
    // Find all items that belong to the given category.
    for (unsigned int ix = 0; ix < propertylist.size(); ++ix)
    {
        // ix is datapos in this only case.
        PropertyData &data = propertylist[ix];
        if (data.Level() == 0)
        {
            skip = data.Category() != category;
            if (!skip)
            {
                poslist.push_back( std::vector<int>() );
                std::vector<int> &vec = poslist.back();
                vec.push_back(ix);
            }
        }
        else if (!skip)
        {
            std::vector<int> &vec = poslist.back();
            vec.push_back(ix);
        }
    }

    QSort(0, poslist.size() - 1, poslist);

    BeginUpdate();
    // Fill list.
    int pos = 0;
    for (unsigned int ix = 0; ix < poslist.size(); ++ix)
    {
        std::vector<int> &sub = poslist[ix];
        for (auto yt = sub.begin(); yt != sub.end(); ++yt)
        {
            Items().Insert(row + pos + 1, propertylist[*yt].Name(), (void*)(*yt));
            pos++;
        }
    }
    EndUpdate();
}

void PropertyListbox::CloseCategory(int row)
{
    int datapos = (int)Items().Data(row);
    if (datapos != -1 || row == Items().Count() - 1)
        return;

    FinalizeEdit(true);

    Items().SetData(row, (void*)-2);

    int end = row;
    while (++end < Items().Count() && (int)Items().Data(end) >= 0)
        ;
    if (end == row + 1)
        return;
    row++;
    BeginUpdate();
    while (row != end--)
        Items().Delete(row);
    EndUpdate();
}

int PropertyListbox::DataRow(int datapos)
{
    for (int ix = Items().Count() - 1; ix >= 0; --ix)
        if ((int)Items().Data(ix) == datapos)
            return ix;
    return -1;
}

void PropertyListbox::buttonmousedown(void *sender, MouseButtonParameters param)
{
    if (param.button != mbLeft)
        return;

    if (listform->Visible())
    {
        listform->Hide();
        return;
    }

    int row = ItemIndex();
    int datapos = row >= 0 ? (int)Items().Data(row) : -1;
    if (datapos < 0)
        return;

    PropertyData &prop = propertylist[datapos];
    if (prop.HasPropertyStyle(psEditButton))
    {
        SendMessage(button->Handle(), WM_LBUTTONUP, 0, MAKELPARAM(1, 1));
        PostMessage(Handle(), amClickActivate, 0, datapos);
        return;
    }
    if (prop.HasPropertyStyle(psReadonly))
        return;

    Rect r = RowRect(row);
    r.left = r.Width() * colw;

    r = Rect( ClientToScreen(Point(r.left, r.top)), ClientToScreen(Point(r.right, r.bottom)) );

    listform->list->Handle();
    int ih = listform->list->ItemHeight();
    listform->SetTopLevelParent(ParentForm());

    listform->list->Items().Clear();

    if (prop.HasPropertyStyle(psDrawItem) || prop.HasPropertyStyle(psThumbImage))
    {
        listform->list->SetKind(lckOwnerDrawVariable);
        listform->list->OnMeasureItem = CreateEvent(this, &PropertyListbox::listformlistmeasureitem);
        listform->list->OnDrawItem = CreateEvent(this, &PropertyListbox::listformlistdrawitem);

        UINT uw = 0, uh = ih;
        listformlistmeasureitem(listform->list, MeasureItemParameters(listform->list->GetCanvas(), 0, uw, uh, 0));
        ih = uh;
    }
    else
    {
        listform->list->SetKind(lckNormal);
        listform->list->OnMeasureItem = NULL;
        listform->list->OnDrawItem = NULL;
    }

    listform->list->BeginUpdate();

    //std::wstring selvalue = data.samevalue ? list[0].item->Value(list[0].owner, list[0].holder) : L"";
    for (int ix = 0; ix < prop.ListCount(); ix++)
    {
        std::wstring s = prop.ListItem(ix);
        listform->list->Items().Add(s);
    }
    listform->list->EndUpdate();

    int ibh = listform->list->WindowRect().Height() - listform->list->ClientRect().Height();
    listform->SetBounds(RectS(r.left + int(Scaling), r.bottom, r.Width() - int(Scaling), (min(12, prop.ListCount()) * ih + ibh) ) );
    listform->SetHeight(listform->list->Height());

    // Don't allow the listbox to hang over the current monitor.
    Rect monrect = screen->MonitorFromWindow(listform)->FullArea();
    if (listform->Bottom() > monrect.Height())
        listform->SetTop(r.top - listform->Height());

    listform->SetVisible(true);
    int sel = prop.SameValue() ? prop.Selected(0) : -1;
    if (sel >= 0)
        listform->list->SetItemIndex(sel);
}

void PropertyListbox::buttonmouseup(void *sender, MouseButtonParameters param)
{
    if (param.button != mbLeft)
        return;

    if (listform->Visible())
        listform->list->Focus();
}

bool PropertyListbox::Active()
{
    return listform->Visible();
}

void PropertyListbox::listformpropertychoice(void *sender, PropertyChoiceParameters param)
{
    int datapos = (int)Items().Data(ItemIndex());
    if (datapos < 0)
        return;

    PropertyData &prop = propertylist[datapos];

    int sel = prop.SameValue() ? prop.Selected(0) : -1; // Currently selected item in the list which is about to be changed.
    if (sel >= 0 && sel == listform->list->ItemIndex())
        return;

    void* listvalue = prop.ListValue(listform->list->ItemIndex());
    if (prop.SelectValue(ParentForm(), 0, listvalue))
    {
        Modify();
        SetValue(ItemIndex(), prop.Value(0), true);
    }
    //std::wstring listitem = prop.ListItem(listform->list->Selected());
    //SetValue(Selected(), listitem, false);
}

void PropertyListbox::listformlistmeasureitem(void *sender, MeasureItemParameters param)
{
    int datapos = (int)Items().Data(ItemIndex());
    if (datapos < 0)
        return;

    PropertyData &prop = propertylist[datapos];

    if (prop.HasPropertyStyle(psDrawItem))
        prop.MeasureListItem(param);
    else if (prop.HasPropertyStyle(psThumbImage))
        param.height = ItemHeight() - 1;
}

void PropertyListbox::listformlistdrawitem(void *sender, DrawItemParameters param)
{
    int datapos = (int)Items().Data(ItemIndex());
    if (datapos < 0)
        return;

    PropertyData &prop = propertylist[datapos];

    Canvas *c = param.canvas; //((Listbox*)sender)->GetCanvas();
    auto state = c->SaveState();
    
    c->SelectStockBrush(param.state.contains(disSelected) ? sbHighlight : sbWindow);
    c->SelectStockPen(param.state.contains(disSelected) ? spHighlightText : spBtnText);
    c->GetFont().SetColor(param.state.contains(disSelected) ? clHighlightText : clBtnText);
    c->FillRect(param.rect);
    
    if (prop.HasPropertyStyle(psDrawItem))
    {
        prop.DrawListItem(param);
    }
    else if (prop.HasPropertyStyle(psThumbImage))
    {
        prop.DrawThumbImage(c, RectS(param.rect.left + int(2 * Scaling), param.rect.top + int(Scaling), ItemHeight() - 1, ItemHeight() - 1 - int(Scaling) * 2), param.index);
        param.rect.left += int(2 * Scaling) + ItemHeight() + 1;
        std::wstring t = prop.ListItem(param.index);
        Size s = c->MeasureText(t);
        c->TextDraw(param.rect, param.rect.left, param.rect.top + (param.rect.Height() - s.cy) / 2, t);
    }
    
    c->RestoreState(state);
}

void PropertyListbox::InvalidateRow(int row)
{
    Rect r = RowRect(row);
    InvalidateRect(r);

    int datapos = (int)Items().Data(row);
    if (datapos < 0)
        return;

    PropertyData &prop = propertylist[datapos];

    if (prop.Level() > 0)
    {
        PropertyData *prop2 = NULL;
        while(true)
        {
            prop2 = &propertylist[(int)Items().Data(--row)];
            if (prop2->Level() < prop.Level())
            {
                InvalidateRow(row);
                break;
            }
        }
    }
}

void PropertyListbox::InvalidateRow(Object *propholder, const std::wstring& propertyname)
{
    unsigned int level = 0;
    unsigned int holderlevel = 0;
    std::wstring match;
    std::wstring lastmatch;
    int cnt = Items().Count();

    for (int ix = 0; ix < cnt; ++ix)
    {
        int datapos = (int)Items().Data(ix);
        if (datapos < 0)
            continue;
        PropertyData &prop = propertylist[datapos];

        bool hasholder = prop.HasHolder(propholder);

        if (prop.Level() < holderlevel + level && !hasholder)
        {
            level = 0;
            holderlevel = 0;
            match = L"";
        }
        else if (!level && hasholder)
        {
            holderlevel = prop.Level();
            level = 1;
            match = L"";
        }
        else if (level && prop.Level() == holderlevel + level)
        {
            level++;
            match += lastmatch + L"/";
        }
        else if (hasholder)
        {
            for (; (int)(level + holderlevel) - 1 > (int)prop.Level(); --level)
                match = match.substr(0, match.rfind(L"/", match.length() - 2) + 1);
        }

        if (level)
        {
            lastmatch = prop.Name();
            if (match + lastmatch == propertyname)
            {
                InvalidateRow(ix);

                level = prop.Level();
                for (int iy = ix + 1; iy < cnt; ++iy)
                {
                    int datapos = (int)Items().Data(iy);
                    if (datapos < 0 || propertylist[datapos].Level() <= level)
                        break;

                    InvalidateRow(iy);
                }
                break;
            }
        }
    }
}

void PropertyListbox::InvalidateEventRows(const std::wstring& eventfuncname)
{
    if (listtype != pltEvents)
        return;
    for (int ix = 0; ix < Items().Count(); ++ix)
    {
        int datapos = (int)Items().Data(ix);
        if (datapos < 0)
            continue;
        PropertyData &prop = propertylist[datapos];
        if (prop.Value(0) == eventfuncname)
            InvalidateRow(ix);
    }
}

void PropertyListbox::EditProperty()
{
    int pos = ItemIndex();
    if ((pos < 0 && Count() > 0) || (pos >= 0 && pos < Count() && (int)Items().Data(pos) < 0))
    {
        int cnt = Items().Count();
        if (pos < 0 || pos >= cnt)
            pos = 0;
        while (pos < cnt && (int)Items().Data(pos) < 0)
            ++pos;
        if (pos == cnt)
            pos = -1;
        else
        {
            SetItemIndex(pos);
            SelChanged();
        }
    }

    if (pos < 0)
    {
        Focus();
        return;
    }

    PropertyData &prop = propertylist[(int)Items().Data(pos)];
    Focus();
    if (!editor || !editor->Focused())
    {
        ShowEditor(pos);
        editor->Focus();
        editor->SetSelStartAndLength(0, editor->Text().length());
    }
}

void PropertyListbox::EditProperty(const std::wstring& propertyname, bool activateeditor)
{
    int cnt = Items().Count();
    for (int ix = 0; ix < cnt; ++ix)
    {
        int datapos = (int)Items().Data(ix);
        if (datapos < 0)
            continue;
        PropertyData &prop = propertylist[datapos];

        if (prop.Name() != propertyname || prop.Level() != 0)
            continue;

        if (!Active())
            Focus();

        SetItemIndex(ix);
        SelChanged();

        if (activateeditor && prop.HasPropertyStyle(psEditButton))
            PostMessage(Handle(), amClickActivate, 0, datapos);
        else
        {
            ShowEditor(ix);
            editor->Focus();
            editor->SetSelStartAndLength(0, editor->Text().length());
        }
        break;
    }
}

void PropertyListbox::EditorKeyPress(WCHAR key)
{
    EditProperty();
    editor->Editor()->BeginUpdate();
    editor->SetText(std::wstring(1, key));
    editor->SetSelStartAndLength(editor->Text().length(), 0);
    editor->Editor()->EndUpdate();
    editor->Editor()->Invalidate();//RedrawWindow(editor->Handle(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

bool PropertyListbox::QSortCmp(int p1, int p2)
{
    PropertyData &d1 = propertylist[p1];
    PropertyData &d2 = propertylist[p2];

    if (d1.Priority() != d2.Priority())
        return d1.Priority() < d2.Priority();

    if (showcat)
    {
        int cmp = d1.Category().compare(d2.Category());
        if (cmp)
            return cmp > 0;
    }

    return d1.Name().compare(d2.Name()) > 0;
}

void PropertyListbox::QSort(int l, int r, std::vector< std::vector<int> > &poslist)
{
    if (l >= r)
        return;

    int compare = poslist[r].front();

    int lpos = l, rpos = r - 1;

    while (lpos <= rpos)
    {
        while (lpos <= rpos && !QSortCmp(poslist[lpos].front(), compare))
            ++lpos;
        // lpos on the first position which is not less than compare.

        while ((rpos > lpos && QSortCmp(poslist[rpos].front(), compare)) || rpos == lpos)
            --rpos;
        // rpos on the last position, which is less than compare.

        if (lpos < rpos)
        {
            std::vector<int> tmp = poslist[lpos];
            poslist[lpos] = poslist[rpos];
            poslist[rpos] = tmp;

            lpos++;
            rpos--;
        }
    }

    if (lpos != r)
    {
        std::vector<int> tmp = poslist[lpos];
        poslist[lpos] = poslist[r];
        poslist[r] = tmp;
    }

    QSort(l, lpos - 1, poslist);
    QSort(lpos + 1, r, poslist);

}

void PropertyListbox::Sort(bool showcategories)
{
    showcat = showcategories;
    std::vector<std::vector<int>> poslist;
    poslist.reserve(propertylist.size());


    bool restoreselect = false;
    PropertyRowIdentity identity;
    if (ItemIndex() >= 0 && ItemIndex() < Items().Count() && (int)Items().Data(ItemIndex()) >= 0)
    {
        identity = RowIdentity(ItemIndex());
        restoreselect = true;
    }

    BeginUpdate();

    // Create list of current items in the listbox. If an item is a subitem of another, it is not added separately, but instead to the vector of the main item.
    for (unsigned int ix = 0; ix < propertylist.size(); ++ix)
    {
        // ix is datapos in this only case.
        PropertyData &data = propertylist[ix];
        if (data.Level() == 0)
        {
            poslist.push_back( std::vector<int>() );
            std::vector<int> &vec = poslist.back();
            vec.push_back(ix);
        }
        else
        {
            std::vector<int> &vec = poslist.back();
            vec.push_back(ix);
        }
    }

    Items().Clear();

    QSort(0, poslist.size() - 1, poslist);

    // Fill list.
    for (auto it = poslist.begin(); it != poslist.end(); ++it)
    {
        std::vector<int> &sub = *it;
        for (auto yt = sub.begin(); yt != sub.end(); ++yt)
            Items().Add(propertylist[*yt].Name(), (void*)(*yt));
    }

    // Insert Categories
    if (showcat)
    {
        std::wstring cat = L"";
        int insertpos = 0;
        for (int ix = Items().Count() - 1; ix >= 0; --ix)
        {
            int datapos = (int)Items().Data(ix);
            PropertyData &prop = propertylist[datapos];

            if (prop.Level() > 0)
                continue;

            if (ix < Items().Count() - 1 && cat != prop.Category())
                Items().Insert(insertpos, cat, (void*)-1);
            cat = prop.Category();
            insertpos = ix;
        }
        Items().Insert(0, cat, (void*)-1);
    }

    if (restoreselect)
        Select(identity, true);

    EndUpdate();
}

PropertyRowIdentity PropertyListbox::RowIdentity(int row)
{
    PropertyRowIdentity identity;

    int datapos;
    if ((datapos = (int)Items().Data(row)) < 0)
    {
        identity.push_back(PropertyRowIdentityData(Items().Text(row)));
        return identity;
    }

    PropertyData *data = &propertylist[datapos];
    identity.push_back(data->Identity());
    unsigned int level = data->Level();
    while (data->Level())
    {
        data = &propertylist[--datapos];
        if (data->Level() < level)
        {
            level = data->Level();
            identity.insert(identity.begin(), data->Identity());
        }
    }
    return identity;
}

void PropertyListbox::Select(PropertyRowIdentity &identity, bool forceopen)
{
    unsigned int level = 0;
    for (int ix = 0; ix < Items().Count(); ++ix)
    {
        int datapos = (int)Items().Data(ix);
        if (datapos < 0)
        {
            if (level > 0 || !identity[0].category || identity[0].name != Items().Text(ix))
                continue;

            SetItemIndex(ix);
            return;
        }

        PropertyData *data = &propertylist[datapos];
        if (data->Level() != level || data->Name() != identity[level].name)
            continue;

        if (data->IsPropertyOfType(identity[level].proptypename))
        {
            if (level == identity.size() - 1)
            {
                SetItemIndex(ix);
                return;
            }
            level++;
            if (!data->Open() && !forceopen)
                return;
            OpenDataRow(ix);
        }
    }
}

void PropertyListbox::SelectDefault()
{
    if (ownerlist.empty())
        return;

    int nameix = -1;
    int textix = -1;
    int selix = -1;
    int cnt = Count();
    bool singleowner = ownerlist.size() == 1;
    for (int ix = 0; ix < cnt && selix == -1; ++ix)
    {
        int datapos = (int)Items().Data(ix);
        if (datapos < 0)
            continue;
        PropertyData *prop = &propertylist[datapos];
        if (prop->Level() != 0)
            continue;
        if (nameix == -1 && prop->Name() == L"Name")
            nameix = ix;
        if (textix == -1 && prop->Name() == L"Text")
            textix = ix;
        if (!singleowner && (textix >= 0))
            break;
        if (singleowner && prop->IsDefaultProperty(0))
            selix = ix;
    }

    if (selix >= 0)
        SetItemIndex(selix);
    else if (textix >= 0)
        SetItemIndex(textix);
    else if (nameix >= 0)
        SetItemIndex(nameix);
}

//Form* PropertyListbox::OwnerForm()
//{
//    FinalizeEdit(true);
//
//    Object *c = ownerlist.front();
//    if (dynamic_cast<Control*>(c))
//        return ((Control*)c)->ParentForm();
//    else if (dynamic_cast<NonVisualControl*>(c))
//        return ((NonVisualControl*)c)->ParentForm();
//    return NULL;
//}

void PropertyListbox::pmpropshow(void *sender, PopupMenuParameters param)
{
    int row;
    if (param.screenpos == Point(-1, -1))
        row = ItemIndex();
    else
        row = ItemAt(ScreenToClient(param.screenpos));
    int datapos = -1;
    if (row >= 0 && (datapos = (int)Items().Data(row)) >= 0)
    {
        PropertyData &data = propertylist[datapos];
        if (listtype == pltValues)
        {
            miSetDefault->SetEnabled(data.Level() == 0 && data.OwnerCount() == 1 && !data.IsDefaultProperty(0));
            miEdit->SetEnabled(data.HasPropertyStyle(psEditButton));
            miDefault->SetEnabled(data.HasDefault() && !data.IsDefault());
            miSetDefault->SetVisible(true);
            miEdit->SetVisible(true);
            miDefault->SetVisible(true);
        }
        else
        {
            miAssign->SetEnabled(ownerlist.size() == 1);
            miAssign->SetVisible(true);
        }
    }
    else
    {
        if (listtype == pltValues)
        {
            miSetDefault->SetVisible(false);
            miEdit->SetVisible(false);
            miDefault->SetVisible(false);
        }
        else
        {
            miAssign->SetVisible(false);
        }
    }

    pmProp->SetTag((row & 0xffff) | ((datapos << 16) & 0xffff0000));
    
    if (btnAbc->Down())
        miAlpha->SetChecked(true);
    else
        miCat->SetChecked(true);
}

void PropertyListbox::pmpropalpha(void *sender, EventParameters param)
{
    btnAbc->Click();
}

void PropertyListbox::pmpropcat(void *sender, EventParameters param)
{
    btnCat->Click();
}

void PropertyListbox::pmpropassign(void *sender, EventParameters param)
{
    int datapos = (pmProp->Tag() >> 16) & 0xffff;
    if (datapos < 0)
        return;
    PropertyData &data = propertylist[datapos];
    SetValue(pmProp->Tag() & 0xffff, ownerlist.front()->Name() + data.ShortEventName(), false);
}

void PropertyListbox::pmpropedit(void *sender, EventParameters param)
{
    int datapos = (pmProp->Tag() >> 16) & 0xffff;
    if (datapos < 0)
        return;
    PostMessage(Handle(), amClickActivate, 0, datapos);
}

void PropertyListbox::pmpropdefault(void *sender, EventParameters param)
{
    int datapos = (pmProp->Tag() >> 16) & 0xffff;
    int row = pmProp->Tag() & 0xffff;
    if (datapos < 0)
        return;
    PropertyData &prop = propertylist[datapos];
    prop.Reset();
    Modify();
    InvalidateRow(row);
    unsigned int level = prop.Level();
    int cnt = Items().Count();
    while (++row < cnt && (int)Items().Data(row) >= 0 && propertylist[(int)Items().Data(row)].Level() > level)
        InvalidateRow(row);
}

void PropertyListbox::pmpropsetdef(void *sender, EventParameters param)
{
    int datapos = (pmProp->Tag() >> 16) & 0xffff;
    int row = pmProp->Tag() & 0xffff;
    if (datapos < 0)
        return;

    int ix = 0;
    for (auto &p : propertylist)
    {
        if (p.IsDefaultProperty(0))
        {
            p.PropertyRemoveDefault(0);

            int cnt = Items().Count();
            for (int iy = 0; iy < cnt; ++iy)
            {

                int datapos = (int)Items().Data(iy);
                if (datapos == ix)
                {
                    InvalidateRow(iy);
                    break;
                }
            }
            break;
        }
        ++ix;
    }

    PropertyData &prop = propertylist[datapos];
    prop.PropertySetToDefault(0);
    InvalidateRow(row);
}


//---------------------------------------------


}
/* End of NLIBNS */

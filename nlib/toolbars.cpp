#include "stdafx_zoli.h"

#ifdef TOOLBARS_ENABLED

#include "toolbars.h"
#include "imagelist.h"
#include "themes.h"

#ifdef DESIGNING
#include "designer.h"
#include "serializer.h"
#include "property_controlbase.h"
#include "property_toolbars.h"

#include "property_imagelist.h"
#endif

namespace NLIBNS
{

#ifdef DESIGNING
    ValuePair<ToolbarButtonCheckStates> ToolbarButtonCheckStateStrings[] = {
            VALUEPAIR(tbbcsChecked),
            VALUEPAIR(tbbcsIndeterminate),
            VALUEPAIR(tbbcsUnchecked),
    };

    ValuePair<ToolbarButtonTypes> ToolbarButtonTypeStrings[] = {
            VALUEPAIR(tbctButton),
            VALUEPAIR(tbctCheckbox),
            VALUEPAIR(tbctDivider),
            VALUEPAIR(tbctDropDown),
            VALUEPAIR(tbctGroupButton),
            VALUEPAIR(tbctDropDownButton),
    };

    ValuePair<ToolbarButtonStyles> ToolbarButtonStyleStrings[] = {
            VALUEPAIR(tbtsAutoWidth),
            VALUEPAIR(tbtsNoPrefix),
            //VALUEPAIR(tbtsShowText, L"tbtsShowText"),
            VALUEPAIR(tbtsWrap),
    };

    Imagelist* ToolbarButton::DesignImagelist()
    {
        Imagelist *list = owner->Images();
        if (!list)
            list = owner->HotImages();
        if (!list)
            list = owner->DisabledImages();
        return list;
    }

    Object* ToolbarButton::SubOwner()
    {
        return owner;
    }

    void ToolbarButton::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->Add(L"SetCheckState", new ToolbarButtonCheckStatesDesignProperty<ToolbarButton>(L"CheckState", L"Behavior", &ToolbarButton::CheckState, &ToolbarButton::SetCheckState))->SetDefault(tbbcsUnchecked);
        serializer->Add(L"SetChecked", new BoolDesignProperty<ToolbarButton>(L"Checked", L"Behavior", &ToolbarButton::Checked, &ToolbarButton::SetChecked))->SetDefault(false)->DontWrite();
        serializer->Add(L"SetType", new ToolbarButtonTypesDesignProperty<ToolbarButton>(L"Type", L"Behavior", &ToolbarButton::Type, &ToolbarButton::SetType))->SetDefault(tbctButton);
        serializer->Add(L"SetEnabled", new BoolDesignProperty<ToolbarButton>(L"Enabled", L"Behavior", &ToolbarButton::Enabled, &ToolbarButton::SetEnabled))->SetDefault(true);
        serializer->Add(L"SetVisible", new BoolDesignProperty<ToolbarButton>(L"Visible", L"Behavior", &ToolbarButton::Visible, &ToolbarButton::SetVisible))->SetDefault(true);
        serializer->Add(L"SetMarked", new BoolDesignProperty<ToolbarButton>(L"Marked", L"Behavior", &ToolbarButton::Marked, &ToolbarButton::SetMarked))->SetDefault(false);
        serializer->Add(L"SetAutoWidth", new BoolDesignProperty<ToolbarButton>(L"AutoWidth", L"Behavior", &ToolbarButton::AutoWidth, &ToolbarButton::SetAutoWidth))->SetDefault(true);
        //serializer->Add(L"SetShowText", new BoolDesignProperty<ToolbarButton>(L"ShowText", L"Behavior", &ToolbarButton::ShowText, &ToolbarButton::SetShowText))->SetDefault(true);
        serializer->Add(L"SetNoPrefix", new BoolDesignProperty<ToolbarButton>(L"NoPrefix", L"Behavior", &ToolbarButton::NoPrefix, &ToolbarButton::SetNoPrefix))->SetDefault(false);
        serializer->Add(L"SetWrap", new BoolDesignProperty<ToolbarButton>(L"Wrap", L"Behavior", &ToolbarButton::Wrap, &ToolbarButton::SetWrap))->SetDefault(false);
        serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<ToolbarButton>(L"ImageIndex", L"Appearance", true, &ToolbarButton::DesignImagelist, &ToolbarButton::ImageIndex, &ToolbarButton::SetImageIndex))->SetDefault(-1);
        serializer->Add(L"SetText", new StringDesignProperty<ToolbarButton>(L"Text", L"Appearance", &ToolbarButton::Text, &ToolbarButton::SetText))->SetDefault(std::wstring())->SetImmediateUpdate(true);
        serializer->Add(L"SetWidth", new WordDesignProperty<ToolbarButton>(L"Width", L"Appearance", &ToolbarButton::Width, &ToolbarButton::SetWidth))->SetDefault(24 * Scaling);
    }
#endif
    ToolbarButton::ToolbarButton(Toolbar *owner, int id, const std::wstring &text, int width, int imgindex, ToolbarButtonCheckStates check, ToolbarButtonTypes type, ToolbarButtonStyleSet style, bool enabled, bool visible, bool marked) :
                owner(owner), id(id), width(width), text(text), check(check), type(type), style(style), state(0), imgindex(imgindex), control(nullptr)
    {
        if (enabled)
            state << itcsEnabled;
        if (!visible)
            state << itcsHidden;
        if (marked)
            state << itcsMarked;
    }

    ToolbarButton::~ToolbarButton()
    {
    }

    void ToolbarButton::Destroy()
    {
        base::Destroy();
    }

    Toolbar* ToolbarButton::Owner()
    {
        return owner;
    }

    int ToolbarButton::CommandId()
    {
        return id;
    }


    BYTE ToolbarButton::ButtonStateInner()
    {
        BYTE result = (int)check | (int)state | TBSTATE_ELLIPSES;
#ifdef DESIGNING
        if (owner->Designing())
            result &= ~itcsHidden;
#endif
        if (state.contains(ToolbarButton::itcsEnabled) && style.contains(tbtsWrap))
            return result | TBSTATE_WRAP;
        return result;
    }

    BYTE ToolbarButton::ButtonStyleInner()
    {
        BYTE st = (int)(style - tbtsWrap /*- ToolbarButtonStyles(owner->Kind() == tbkImageOnly ? tbtsShowText : 0)*/);


        if (owner->Kind() != tbkImageOnly)
            st |= tbbsShowText;

        switch (type)
        {
        case tbctButton:
            break;
        case tbctDropDown:
            st |= tbbsDropDown | tbbsWholeDropDown;
            break;
        case tbctDropDownButton:
            st |= tbbsDropDown;
            break;
        case tbctGroupButton:
            st |= tbbsCheckGroup;
            break;
        case tbctCheckbox:
            st |= tbbsCheck;
            break;
        case tbctDivider:
        case tbctControlOwner:
            st |= tbbsSeparator;
            break;
        default:
            break;
        }

        return st;
    }

    std::wstring ToolbarButton::Text()
    {
        return text;
    }

    void ToolbarButton::SetText(const std::wstring &newtext)
    {
        text = newtext;

        if (!owner->HandleCreated() || owner->kind == tbkImageOnly)
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_TEXT;
        if (newtext.empty())
            inf.pszText = (LPWSTR)-1;
        else
            inf.pszText = const_cast<wchar_t*>(&newtext[0]);
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    WORD ToolbarButton::Width()
    {
        return width;
    }

    void ToolbarButton::SetWidth(WORD newwidth)
    {
        width = max(1, newwidth);

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_SIZE;
        inf.cx = width;
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    ToolbarButtonCheckStates ToolbarButton::CheckState()
    {
        if (!owner->HandleCreated())
            return check;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsState & (int)tbbcsChecked) == tbbcsChecked)
        {
            check = tbbcsChecked;
            return tbbcsChecked;
        }
        if ((inf.fsState & (int)tbbcsIndeterminate) == tbbcsIndeterminate)
        {
            check = tbbcsIndeterminate;
            return tbbcsIndeterminate;
        }
        return tbbcsUnchecked;
    }

    void ToolbarButton::SetCheckState(ToolbarButtonCheckStates newstate)
    {
        check = newstate;
        if (type == tbctGroupButton && newstate == tbbcsChecked)
        {
            int index = owner->CommandIdToIndex(id);

            int p = index;
            while (p - 1 != -1 && owner->Buttons(p - 1)->type == tbctGroupButton)
                --p;
            int cnt = owner->ButtonCount();
            while (p != cnt && owner->Buttons(p)->type == tbctGroupButton)
            {
                if (p != index && owner->Buttons(p)->check == tbbcsChecked || owner->Buttons(p)->check == tbbcsIndeterminate)
                    owner->Buttons(p)->check = tbbcsUnchecked;
                ++p;
            }
        }

#ifdef DESIGNING
        if (owner->Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Checked");
#endif

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;

        inf.fsState = ButtonStateInner();
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    bool ToolbarButton::Checked()
    {
        return CheckState() == tbbcsChecked;
    }

    void ToolbarButton::SetChecked(bool newchecked)
    {
        SetCheckState(newchecked ? tbbcsChecked : tbbcsUnchecked);

#ifdef DESIGNING
        if (owner->Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"CheckState");
#endif
    }

    bool ToolbarButton::Indeterminate()
    {
        return CheckState() == tbbcsIndeterminate;
    }

    ToolbarButtonTypes ToolbarButton::Type()
    {
        return type;
    }

    void ToolbarButton::SetType(ToolbarButtonTypes newtype)
    {
        if (newtype == tbctControlOwner)
            throw L"This type cannot be set manually. Place a control on the toolbar instead.";

        if (control != nullptr)
            throw L"Cannot change the type of a button which holds a control.";

        type = newtype;

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STYLE /*| TBIF_LPARAM*/;

        inf.fsStyle = ButtonStyleInner();

        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    bool ToolbarButton::Enabled()
    {
        if (!owner->HandleCreated())
            return state.contains(itcsEnabled);

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsState & itcsEnabled) == itcsEnabled)
            state << itcsEnabled;
        else
            state -= itcsEnabled;

        return (inf.fsState & itcsEnabled) == itcsEnabled;
    }

    void ToolbarButton::SetEnabled(bool newenabled)
    {
        if (newenabled)
            state << itcsEnabled;
        else
            state -= itcsEnabled;

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        inf.fsState = ButtonStateInner();

        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    bool ToolbarButton::Visible()
    {
#ifdef DESIGNING
        if (owner->Designing())
            return !state.contains(itcsHidden);
#endif
        if (!owner->HandleCreated())
            return !state.contains(itcsHidden);

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);

        inf.dwMask = TBIF_STATE;

        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsState & itcsHidden) != 0)
            state << itcsHidden;
        else
            state -= itcsHidden;

        return (inf.fsState & itcsHidden) == 0;
    }

    void ToolbarButton::SetVisible(bool newvisible)
    {
#ifdef DESIGNING
        if (owner->Designing())
        {
            if (!newvisible) 
                state << itcsHidden;
            else
                state -= itcsHidden;
            return;
        }
#endif

        if (newvisible && state.contains(itcsHidden))
        {
            state -= itcsHidden;
            if (control != nullptr)
                control->SetVisible(true);
        }
        else if (!newvisible && !state.contains(itcsHidden))
        {
            if (control != nullptr)
                control->SetVisible(false);
            state << itcsHidden;
        }

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);

        inf.dwMask = TBIF_STATE;
        inf.fsState = ButtonStateInner();

        if (owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf) == 0)
            throw L"Couldn't change button visibility.";
    }

    bool ToolbarButton::Marked()
    {
        if (!owner->HandleCreated())
            return state.contains(itcsMarked);

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsState & itcsMarked) == itcsMarked)
            state << itcsMarked;
        else
            state -= itcsMarked;
        
        return (inf.fsState & itcsMarked) == itcsMarked;
    }

    void ToolbarButton::SetMarked(bool newmarked)
    {
        if (newmarked)
            state << itcsMarked;
        else
            state -= itcsMarked;

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        inf.fsState = ButtonStateInner();
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    bool ToolbarButton::AutoWidth()
    {
        if (!owner->HandleCreated())
            return style.contains(tbtsAutoWidth);

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STYLE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsStyle & tbtsAutoWidth) == tbtsAutoWidth)
            style << tbtsAutoWidth;
        else
            style -= tbtsAutoWidth;

        return (inf.fsStyle & tbtsAutoWidth) == tbtsAutoWidth;
    }

    void ToolbarButton::SetAutoWidth(bool newauto)
    {
        if (newauto)
            style << tbtsAutoWidth;
        else
            style -= tbtsAutoWidth;
        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STYLE;
        inf.fsStyle = ButtonStyleInner();
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    //bool ToolbarButton::ShowText()
    //{
    //    if (!owner->HandleCreated() || owner->Kind() == tbkImageOnly)
    //        return style.contains(tbtsShowText);

    //    TBBUTTONINFO inf;
    //    inf.cbSize = sizeof(TBBUTTONINFO);
    //    inf.dwMask = TBIF_STYLE;
    //    if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
    //        throw L"Error fetching requested toolbar button data.";

    //    if ((inf.fsStyle & tbtsShowText) == tbtsShowText)
    //        style << tbtsShowText;
    //    else
    //        style -= tbtsShowText;

    //    return (inf.fsStyle & tbtsShowText) == tbtsShowText;
    //}

    //void ToolbarButton::SetShowText(bool newshow)
    //{
    //    if (newshow)
    //        style << tbtsShowText;
    //    else
    //        style -= tbtsShowText;

    //    if (!owner->HandleCreated() || owner->Kind() == tbkImageOnly)
    //        return;

    //    TBBUTTONINFO inf;
    //    inf.cbSize = sizeof(TBBUTTONINFO);
    //    inf.dwMask = TBIF_STYLE;
    //    inf.fsStyle = ButtonStyleInner();
    //    owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    //}

    bool ToolbarButton::NoPrefix()
    {
        if (!owner->HandleCreated())
            return style.contains(tbtsNoPrefix);

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STYLE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsStyle & tbtsNoPrefix) == tbtsNoPrefix)
            style << tbtsNoPrefix;
        else
            style -= tbtsNoPrefix;

        return (inf.fsStyle & tbtsNoPrefix) == tbtsNoPrefix;
    }

    void ToolbarButton::SetNoPrefix(bool newnopref)
    {
        if (newnopref)
            style << tbtsNoPrefix;
        else
            style -= tbtsNoPrefix;
        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STYLE;
        inf.fsStyle = ButtonStyleInner();
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    bool ToolbarButton::Wrap()
    {
        if (!owner->HandleCreated())
            return style.contains(tbtsWrap);

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if ((inf.fsState & tbtsWrap) == tbtsWrap)
            style << tbtsWrap;
        else
            style -= tbtsWrap;

        return (inf.fsState & tbtsWrap) == tbtsWrap;
    }

    void ToolbarButton::SetWrap(bool newwrap)
    {
        if (newwrap)
            style << tbtsWrap;
        else
            style -= tbtsWrap;

        if (!owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_STATE;
        inf.fsState = ButtonStateInner();
        owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf);
    }

    int ToolbarButton::ImageIndex()
    {
        if (!owner->HandleCreated() || owner->Kind() == tbkTextOnly || type == tbctDivider)
            return imgindex;

        TBBUTTONINFO inf;
        memset(&inf, 0, sizeof(TBBUTTONINFO));
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_IMAGE;
        if (owner->PassMessage(TB_GETBUTTONINFO, id, (LPARAM)&inf) == -1)
            throw L"Error fetching requested toolbar button data.";

        if (inf.iImage == I_IMAGENONE)
            imgindex = -1;
        else
            imgindex = inf.iImage;

        return imgindex;
    }

    void ToolbarButton::SetImageIndex(int newindex)
    {
        imgindex = max(-1, newindex);

        if (owner->kind == tbkTextOnly || !owner->HandleCreated())
            return;

        TBBUTTONINFO inf;
        inf.cbSize = sizeof(TBBUTTONINFO);
        inf.dwMask = TBIF_IMAGE;

        if (imgindex == -1)
            inf.iImage = I_IMAGENONE;
        else
            inf.iImage = owner->Kind() != tbkTextOnly ? imgindex : I_IMAGENONE;

        if (owner->PassMessage(TB_SETBUTTONINFO, id, (LPARAM)&inf) == 0)
            throw L"Error changing toolbar button data.";
    }

    int ToolbarButton::Index()
    {
        return owner->ButtonIndex(this);
    }

    Rect ToolbarButton::Area()
    {
        if (!owner->HandleCreated())
            return Rect();

        Rect result;
        if (owner->PassMessage(TB_GETITEMRECT, owner->CommandIdToIndex(id), (LPARAM)&result) == FALSE)
            return Rect();

        return result;
    }

#ifdef DESIGNING

    ValuePair<ToolbarKinds> ToolbarKindStrings[] = {
            VALUEPAIR(tbkImageOnly),
            VALUEPAIR(tbkTextOnly),
            VALUEPAIR(tbkTextBelow),
            VALUEPAIR(tbkSideText),
    };

    Size Toolbar::DesignSize()
    {
        return Size(320, 28 * Scaling);
    }

    std::wstring ToolbarButtonSerializeArgs(Object *control)
    {
        ToolbarButton *tc = dynamic_cast<ToolbarButton*>(control);
        if (!tc)
            return std::wstring();
        return L"L\"" + EscapeCString(tc->Text()) + L"\", " + IntToStr(tc->Tag()) + L", " + IntToStr(tc->ImageIndex()) + L", " +
               EnumToEnumString(ToolbarButtonCheckStateStrings, tc->CheckState()) + L", " + EnumToEnumString(ToolbarButtonTypeStrings, tc->Type()) + L", " +
               (tc->AutoWidth() ? L"tbtsAutoWidth" : L"0") /*+ L" | " + (tc->ShowText() ? L"tbtsShowText" : L"0")*/ + L" | " + (tc->NoPrefix() ? L"tbtsNoPrefix" : L"0") + L" | " + (tc->Wrap() ? L"tbtsWrap" : L"0") + L", " +
               (tc->Enabled() ? L"true" : L"false") + L", " + (tc->Visible() ? L"true" : L"false") + L", " + (tc->Marked() ? L"true" : L"false");
    }


    void Toolbar::InitDesignerMenu(Point clientpos, std::vector<menu_item_data> &inserteditems)
    {
        inserteditems.push_back(menu_item_data(L"Add Button", 0, CreateEvent(this, &Toolbar::DoAddButton)));
        int index = ButtonAt(clientpos);
        if (index >= 0)
            inserteditems.push_back(menu_item_data(L"Delete Button", index, CreateEvent(this, &Toolbar::DoDeleteButton)));
    }

    void Toolbar::DesignSubSelected(Object *subobj)
    {
        if (dynamic_cast<ToolbarButton*>(subobj) != nullptr)
            DesignSelectButton(CommandIdToIndex(((ToolbarButton*)subobj)->id));
    }

    void Toolbar::DesignAddButton()
    {
        //AddButton(L"", 0, -1, tbbcsUnchecked, tbctButton, tbtsAutoWidth | tbtsShowText, true, true, false);

        ToolbarButton *item = AddButton(L"", 24 * Scaling, -1, tbbcsUnchecked, tbctButton, tbtsAutoWidth /*| tbtsShowText*/, true, true, false);
        item->SetName(L"ToolbarButton" + IntToStr(DesignParent()->NameNext(L"ToolbarButton")));
        DesignParent()->RegisterSubObject(this, item, item->Name());
    }

    void Toolbar::DoAddButton(void *sender, EventParameters param)
    {
        DesignAddButton();
        buttons.back()->SetText(buttons.back()->Name());

        DesignSelectButton(ButtonCount() - 1);
    }

    void Toolbar::DoDeleteButton(void *sender, EventParameters param)
    {
        DeleteButton(((MenuItem*)sender)->Tag());
    }

    bool Toolbar::NeedDesignerHittest(int x, int y, LRESULT hittest)
    {
        int btn = ButtonAt(x, y);
        if (btn < 0)
            return base::NeedDesignerHittest(x, y, hittest);
        return true;
    }

    bool Toolbar::DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
    {
        int btn = ButtonAt(x, y);
        if (btn < 0)
            return false;

        DesignSelectButton(btn);
        return true;
    }

    bool Toolbar::DesignKeyPush(DesignForm *form, WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
    {
        ToolbarButton *btn =  dynamic_cast<ToolbarButton*>(DesignParent()->SelectedObject().second);
        if (!designer || btn == nullptr || btn->Owner() != this)
            return false;

        if (keycode == VK_DELETE || keycode == VK_BACK)
        {
            btn->Destroy();
            return true;
        }
        if (!vkeys.contains(vksShift) && !vkeys.contains(vksCtrl) && (keycode == VK_LEFT || keycode == VK_RIGHT))
        {
            if ((keycode == VK_LEFT && btn->Index() == 0) || (keycode == VK_RIGHT && btn->Index() == buttons.size() - 1))
                return false;

            DesignForm *frm = (DesignForm*)DesignParent();

            if (keycode == VK_LEFT)
                btn = buttons[btn->Index() - 1];
            else
                btn = buttons[btn->Index() + 1];

            frm->SelectDesignControl(this, btn->Area(), btn, btn->CommandId());
            return true;
        }

        return false;
    }

    bool Toolbar::DesignTabNext(bool entering, bool backwards)
    {
        if (buttons.empty() || (entering && !backwards))
            return false;

        DesignForm *frm = (DesignForm*)DesignParent();
        ToolbarButton *btn;

        int index = -1;

        if (entering)
            index = buttons.size() - 1;

        if (index == -1)
        {
            btn = dynamic_cast<ToolbarButton*>(DesignParent()->SelectedObject().second);
            if (btn == nullptr)
            {
                if (!entering && backwards)
                    return false;

                index = 0;
            }
            else if (btn->Index() != 0 && backwards)
                index = btn->Index() - 1;
            else if (btn->Index() != buttons.size() - 1 && !backwards)
                index = btn->Index() + 1;
            else if (btn->Index() == 0 && backwards)
            {
                frm->SelectObject(this);
                return true;
            }
        }

        if (index != -1)
        {
            btn = buttons[index];
            frm->SelectDesignControl(this, btn->Area(), btn, btn->CommandId());
            return true;
        }

        return false;
    }

    void Toolbar::DesignSelectButton(int index)
    {
        if (index < 0)
            return;

        DesignForm *frm = (DesignForm*)DesignParent();
        frm->SelectDesignControl(this, buttons[index]->Area(), buttons[index], buttons[index]->CommandId());
    }

    ToolbarButton* Toolbar::DesignGetButtons(int index)
    {
        return buttons[index];
    }

    void Toolbar::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"Text");
        serializer->HideProperty(L"BorderStyle");
        serializer->HideProperty(L"AcceptInput");
        serializer->HideProperty(L"WantedKeyTypes");
        serializer->Find<ControlAlignmentsDesignProperty<Control>>(L"Alignment")->SetDefault(alTop);

        serializer->Add(L"SetKind", new ToolbarKindsDesignProperty<Toolbar>(L"Kind", L"Layout", &Toolbar::Kind, &Toolbar::SetKind))->SetDefault(tbkTextOnly);
        serializer->Add(L"SetTopDivider", new BoolDesignProperty<Toolbar>(L"TopDivider", L"Appearance", &Toolbar::TopDivider, &Toolbar::SetTopDivider))->SetDefault(false);
        //serializer->Add(L"SetMinButtonWidth", new WordDesignProperty<Toolbar>(L"MinButtonWidth", L"Appearance", &Toolbar::MinButtonWidth, &Toolbar::SetMinButtonWidth))->SetDefault(6 + 7 * Scaling);
        //serializer->Add(L"SetMaxButtonWidth", new WordDesignProperty<Toolbar>(L"MaxButtonWidth", L"Appearance", &Toolbar::MaxButtonWidth, &Toolbar::SetMaxButtonWidth))->SetDefault(16 + 7 * Scaling);
        serializer->Add(L"SetButtonWidth", new WordDesignProperty<Toolbar>(L"ButtonWidth", L"Appearance", &Toolbar::ButtonWidth, &Toolbar::SetButtonWidth))->SetDefault(16 + 7 * Scaling);
        serializer->Add(L"SetButtonHeight", new WordDesignProperty<Toolbar>(L"ButtonHeight", L"Appearance", &Toolbar::ButtonHeight, &Toolbar::SetButtonHeight))->SetDefault(16 + 6 * Scaling);
        serializer->Add(L"SetImages", new ImagelistDesignProperty<Toolbar>(L"Images", L"Appearance", &Toolbar::Images, &Toolbar::SetImages));
        serializer->Add(L"SetHotImages", new ImagelistDesignProperty<Toolbar>(L"HotImages", L"Appearance", &Toolbar::HotImages, &Toolbar::SetHotImages));
        serializer->Add(L"SetDisabledImages", new ImagelistDesignProperty<Toolbar>(L"DisabledImages", L"Appearance", &Toolbar::DisabledImages, &Toolbar::SetDisabledImages));

        serializer->Add(L"AddButton", new ToolbarButtonVectorDesignProperty<Toolbar>(L"Buttons", &Toolbar::ButtonCount, &Toolbar::DesignGetButtons, &Toolbar::DesignAddButton));
    }
#endif

    Toolbar::Toolbar() : kind(tbkTextOnly), topdiv(false), btnwidth(16 + 7 * Scaling), btnheight(16 + 6 * Scaling), /*minwidth(1), maxwidth(16 + 7 * Scaling),*/ images(nullptr), himages(nullptr), dimages(nullptr)
    {
        controlstyle << csInTabOrder;
        InitControlList();
        SetAlignment(alTop);
    }

    Toolbar::~Toolbar()
    {
    }

    void Toolbar::Destroy()
    {
        while (!buttons.empty())
            buttons[0]->Destroy();

        base::Destroy();
    }

    void Toolbar::CreateClassParams(ClassParams &params)
    {
        InitCommonControl(ICC_BAR_CLASSES);
        params.classname = TOOLBARCLASSNAME;
    }

    void Toolbar::CreateWindowParams(WindowParams &params)
    {
        base::CreateWindowParams(params);

        if (topdiv)
            params.style -= ccsNoDivider;
        else
            params.style << ccsNoDivider;

#ifdef DESIGNING
#endif

        params.style << ccsNoResize << ccsNoParentAlign;
        if (DoubleBuffered())
            params.extstyle << tbsExDoubleBuffer;
    }

    void Toolbar::InitHandle()
    {
        base::InitHandle();

        PassMessage(TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        PassMessage(TB_SETINDENT, 0/* new indentation */, 0);

        //DWORD dpad = PassMessage(TB_GETPADDING, 0, 0);
        //Size pad = Size(LOWORD(dpad), HIWORD(dpad));

        //Size imgsiz;
        //if (images)
        //    imgsiz = Size(images->Width(), images->Height());
        //else if (himages)
        //    imgsiz = Size(himages->Width(), himages->Height());
        //else if (dimages)
        //    imgsiz = Size(dimages->Width(), dimages->Height());
        //PassMessage(TB_SETBITMAPSIZE, 0, MAKELPARAM(imgsiz.cx, imgsiz.cy));

        if (kind != tbkTextOnly)
        {
            if (images)
                PassMessage(TB_SETIMAGELIST, 0, (LPARAM)images->Handle());
            if (himages)
                PassMessage(TB_SETIMAGELIST, 0, (LPARAM)himages->Handle());
            if (dimages)
                PassMessage(TB_SETIMAGELIST, 0, (LPARAM)dimages->Handle());
        }

        //PassMessage(TB_SETBUTTONWIDTH, 0, MAKELPARAM(minwidth, maxwidth)); // no change when calling this
        //PassMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(btnheight, btnwidth));

        WindowStyleSet style = GetWindowLongPtr(Handle(), GWL_STYLE);
        ExtendedWindowStyleSet extstyle = GetWindowLongPtr(Handle(), GWL_EXSTYLE);
        style << tbsTransparent /*<< tbsWrap*/;
        if (DoubleBuffered())
            extstyle << tbsExDoubleBuffer;
        else
            extstyle -= tbsExDoubleBuffer;
        extstyle << tbsExSeparateArrows;
        style -= tbsList;
        style -= tbsFlat;
        extstyle -= tbsExMixed;

        switch (kind)
        {
        case tbkTextOnly:
            style << tbsList;
            //extstyle << tbsExMixed;
            break;
        case tbkImageOnly:
            style << tbsFlat;
            style -= tbsTransparent;
            break;
        case tbkTextBelow:
            style << tbsFlat;
            break;
        case tbkSideText:
            style << tbsList;
            //extstyle << tbsExMixed;
            break;
        };

        PassMessage(TB_SETSTYLE, 0, (LPARAM)style);
        PassMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM)extstyle);

        if (!buttons.empty())
            DoInsertButton(-1, buttons.size());

        LRESULT pd = SendMessage(Handle(), TB_GETPADDING, 0, 0);
        Size pad(LOWORD(pd), HIWORD(pd));

        PassMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(btnwidth - pad.cx, btnheight - pad.cy));
    }

    void Toolbar::SaveWindow()
    {
        int cnt = ButtonCount();

        ButtonSize();

        base::SaveWindow();
    }

    LRESULT Toolbar::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //switch (uMsg)
        //{
        //case wmColorChanged:
        //    return 0;
        //}
        return base::WindowProc(uMsg, wParam, lParam);
    }

    bool Toolbar::HandleCommand(Control *parent, WPARAM wParam)
    {
        return false;
    }

    bool Toolbar::HandleNotify(Control *parent, LPARAM lParam, HRESULT &result)
    {
        result = 0;

        NMTOOLBAR *nm = (NMTOOLBAR*)lParam;
        if (nm->hdr.code == TBN_DELETINGBUTTON)
        {
            if (toolbarstate.contains(tsSwapping))
                return false;

            toolbarstate << tsButtonDeleting;

            int ix = CommandIdToIndex(nm->iItem);

            if (ix >= 0)
            {
                ToolbarButton *item = buttons[ix];
                if (!controlstate.contains(csDestroying) && !controlstate.contains(csRecreating) && item->control != nullptr)
                    item->control->Destroy();

                buttons.erase(buttons.begin() + ix);
                item->Destroy();
            }

            toolbarstate -= tsButtonDeleting;
            return true;
        }
        if (nm->hdr.code == NM_CUSTOMDRAW)
        {
            NMTBCUSTOMDRAW *tbcd = (NMTBCUSTOMDRAW*)lParam;
            NMCUSTOMDRAW &cd = tbcd->nmcd;

            if ((cd.dwDrawStage & CDDS_ITEM) != CDDS_ITEM && (cd.dwDrawStage & CDDS_PREPAINT) == CDDS_PREPAINT)
            {
                result = CDRF_NOTIFYITEMDRAW;
                return true;
            }
            //if ((cd.dwDrawStage & CDDS_ITEMPREPAINT) == CDDS_ITEMPREPAINT)
            //{
            //    result = CDRF_SKIPDEFAULT;
            //    return true;
            //}
            //else if ((cd.dwDrawStage & CDDS_ITEM) == CDDS_ITEM)
            //{

            //    ThemeButtonStates btnstate = tbsNormal;
            //    switch (cd.uItemState)
            //    {
            //    case CDIS_HOT:
            //    case CDIS_OTHERSIDEHOT:
            //        btnstate = tbsHot;
            //    case CDIS_CHECKED:
            //        if (btnstate == tbsHot)
            //            btnstate = tbsHotChecked;
            //        else
            //            btnstate = tbsChecked;
            //        break;
            //    case CDIS_DISABLED:
            //        btnstate = tbsDisabled;
            //        break;
            //    case CDIS_FOCUS:
            //    case CDIS_SELECTED:
            //        btnstate = tbsDefaulted;
            //        break;
            //    case CDIS_INDETERMINATE:
            //        break;

            //    }
            //    themes->DrawToolbarButton(GetCanvas(), cd.rc, btnstate);
            //    //, tbsHot, tbsChecked, tbsHotChecked, tbsPressed, tbsDefaulted, tbsDisabled
            //}
#ifdef DESIGNING
            else if (((cd.dwDrawStage & CDDS_ITEM) == CDDS_ITEM || (cd.dwDrawStage & CDDS_SUBITEM) == CDDS_SUBITEM))
            {
                if (!Designing() && (cd.dwDrawStage & CDDS_ITEMPREPAINT) == CDDS_ITEMPREPAINT)
                {
                    if ((cd.uItemState & CDIS_HOT) == CDIS_HOT)
                    {
                        ControlCanvas cc(cd.hdc);
                        cc.SetPen(spBlack);
                        cc.Line(cd.rc.left, cd.rc.top, cd.rc.right - 1, cd.rc.bottom - 1);
                    }
                    result = CDRF_SKIPDEFAULT;
                    return true;

                }
                else if (Designing())
                    cd.uItemState |= CDIS_HOT;
            }
#endif

        }

        return false;
    }

    void Toolbar::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        if (object == images)
        {
            images = NULL;
            if (HandleCreated())
                SendMessage(Handle(), TB_SETIMAGELIST, 0, 0);
            return;
        }
        else if (object == himages)
        {
            himages = NULL;
            if (HandleCreated())
                SendMessage(Handle(), TB_SETHOTIMAGELIST, 0, 0);
            return;
        }
        else if (object == dimages)
        {
            dimages = NULL;
            if (HandleCreated())
                SendMessage(Handle(), TB_SETDISABLEDIMAGELIST, 0, 0);
            return;
        }
        else if (dynamic_cast<ToolbarButton*>(object) != nullptr && ((ToolbarButton*)object)->Owner() == this)
        {
            ToolbarButton *btn = (ToolbarButton*)object;
            int index = btn->Index();

#ifdef DESIGNING
            if (Designing() && DesignParent() && DesignParent()->SelectedObject().second == object)
            {
                DesignForm *frm = (DesignForm*)DesignParent();
                if (index != buttons.size() - 1 || index > 0)
                {
                    int ix = index != buttons.size() - 1 ? index + 1 : index - 1;
                    frm->SelectDesignControl(this, buttons[ix]->Area(), buttons[ix], buttons[ix]->CommandId());
                }
                else
                    DesignParent()->SelectObject(this);
            }
#endif
            buttons.erase(buttons.begin() + index);
            if (!HandleCreated())
                return;

            if (PassMessage(TB_DELETEBUTTON, index, 0) == FALSE)
                throw L"Button index out of range.";
        }
    }

    void Toolbar::FontChanged(const FontData &data)
    {
        base::FontChanged(data);
        if (HandleCreated())
        {
            LRESULT pd = SendMessage(Handle(), TB_GETPADDING, 0, 0);
            Size pad(LOWORD(pd), HIWORD(pd));
            PassMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(btnwidth - pad.cx, btnheight - pad.cy));
        }
    }

    bool Toolbar::TopDivider() const
    {
        return topdiv;
    }

    void Toolbar::SetTopDivider(bool newtopdiv)
    {
        if (topdiv == newtopdiv)
            return;
        topdiv = newtopdiv;

        if (!HandleCreated())
            return;

        RecreateHandle();
        if (topdiv)
            SetWindowPos(Handle(), 0, 0, 0, 0, 0, SWP_FRAMEONLY);
    }

    Imagelist* Toolbar::Images()
    {
        if (!HandleCreated() || images == nullptr || kind == tbkTextOnly)
            return images;

        if ((HIMAGELIST)PassMessage(TB_GETIMAGELIST, 0, 0) != images->Handle())
        {
            RemoveFromNotifyList(images, nrSubControl);
            images = nullptr;
        }
        return images;
    }

    void Toolbar::SetImages(Imagelist* newimages)
    {
        if (images == newimages)
            return;
        if (images)
            RemoveFromNotifyList(images, nrSubControl);
        images = newimages;
        if (images)
            AddToNotifyList(images, nrSubControl);
        if (kind == tbkTextOnly || !HandleCreated())
            return;

        PassMessage(TB_SETIMAGELIST, 0, !images ? 0 : (LPARAM)images->Handle());
    }

    Imagelist* Toolbar::HotImages()
    {
        if (!HandleCreated() || himages == nullptr || kind == tbkTextOnly)
            return himages;

        if ((HIMAGELIST)PassMessage(TB_GETHOTIMAGELIST, 0, 0) != himages->Handle())
        {
            RemoveFromNotifyList(himages, nrSubControl);
            himages = nullptr;
        }
        return himages;
    }

    void Toolbar::SetHotImages(Imagelist* newimages)
    {
        if (himages == newimages)
            return;
        if (himages)
            RemoveFromNotifyList(himages, nrSubControl);
        himages = newimages;
        if (himages)
            AddToNotifyList(himages, nrSubControl);
        if (kind == tbkTextOnly || !HandleCreated())
            return;

        PassMessage(TB_SETHOTIMAGELIST, 0, !himages ? 0 : (LPARAM)himages->Handle());
    }

    Imagelist* Toolbar::DisabledImages()
    {
        if (!HandleCreated() || dimages == nullptr || kind == tbkTextOnly)
            return dimages;

        if ((HIMAGELIST)PassMessage(TB_GETDISABLEDIMAGELIST, 0, 0) != dimages->Handle())
        {
            RemoveFromNotifyList(dimages, nrSubControl);
            dimages = nullptr;
        }
        return dimages;
    }

    void Toolbar::SetDisabledImages(Imagelist* newimages)
    {
        if (dimages == newimages)
            return;
        if (dimages)
            RemoveFromNotifyList(dimages, nrSubControl);
        dimages = newimages;
        if (dimages)
            AddToNotifyList(dimages, nrSubControl);
        if (kind == tbkTextOnly || !HandleCreated())
            return;

        PassMessage(TB_SETDISABLEDIMAGELIST, 0, !dimages ? 0 : (LPARAM)dimages->Handle());
    }

    //WORD Toolbar::MinButtonWidth()
    //{
    //    return minwidth;
    //}

    //void Toolbar::SetMinButtonWidth(WORD newwidth)
    //{
    //    if (minwidth == newwidth)
    //        return;
    //    minwidth = newwidth;
    //    if (HandleCreated())
    //        RecreateHandle();
    //}

    //WORD Toolbar::MaxButtonWidth()
    //{
    //    return maxwidth;
    //}

    //void Toolbar::SetMaxButtonWidth(WORD newwidth)
    //{
    //    if (maxwidth == newwidth)
    //        return;
    //    maxwidth = newwidth;
    //    if (HandleCreated())
    //        RecreateHandle();
    //}

    Size Toolbar::ButtonSize()
    {
        if (HandleCreated())
        {
            DWORD s = SendMessage(Handle(), TB_GETBUTTONSIZE, 0, 0);
            btnwidth = LOWORD(s);
            btnheight = HIWORD(s);
        }

        return Size(btnwidth, btnheight);
    }

    void Toolbar::SetButtonSize(const Size &newsize)
    {
        if (newsize.cx == btnwidth && newsize.cy == btnheight)
            return;

        btnwidth = newsize.cx;
        btnheight = newsize.cy;

        if (HandleCreated())
        {
            LRESULT pd = SendMessage(Handle(), TB_GETPADDING, 0, 0);
            Size pad(LOWORD(pd), HIWORD(pd));
            SendMessage(Handle(), TB_SETBUTTONSIZE, 0, MAKELPARAM(newsize.cx - pad.cx, newsize.cy - pad.cy));
            //SendMessage(Handle(), TB_AUTOSIZE, 0, 0);
        }
    }

    WORD Toolbar::ButtonWidth()
    {
        return ButtonSize().cx;
    }

    void Toolbar::SetButtonWidth(WORD newwidth)
    {
        SetButtonSize(Size(newwidth, ButtonSize().cy));
    }

    WORD Toolbar::ButtonHeight()
    {
        return ButtonSize().cy;
    }

    void Toolbar::SetButtonHeight(WORD newheight)
    {
        SetButtonSize(Size(ButtonSize().cx, newheight));
    }

    int Toolbar::CommandIdToIndex(int id)
    {
        auto it = std::find_if(buttons.begin(), buttons.end(), [id](ToolbarButton *item) { return item->id == id; });
        if (it == buttons.end())
            return -1;
        return it - buttons.begin();
    }

    int Toolbar::IndexToCommandId(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            return -1;
        return buttons[index]->id;
    }

    int Toolbar::ButtonCount() const
    {
        return buttons.size();
    }

    ToolbarButton* Toolbar::Buttons(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Button index out of range.";
        return buttons[index];
    }

    ToolbarKinds Toolbar::Kind()
    {
        return kind;
    }

    void Toolbar::SetKind(ToolbarKinds newkind)
    {
        if (kind == newkind)
            return;
        kind = newkind;
        if (HandleCreated())
            RecreateHandle();
    }

    ToolbarButton* Toolbar::AddButton()
    {
        return AddButton(std::wstring(), 24 * Scaling, -1, tbbcsUnchecked, tbctButton, tbtsAutoWidth /*| tbtsShowText*/, true, true, false);
    }

    ToolbarButton* Toolbar::InsertButton(int index)
    {
        return InsertButton(index, std::wstring(), 24 * Scaling, -1, tbbcsUnchecked, tbctButton, tbtsAutoWidth /*| tbtsShowText*/, true, true, false);
    }

    ToolbarButton* Toolbar::InsertButton(int index, const std::wstring &text, int width, int imgindex, ToolbarButtonCheckStates check, ToolbarButtonTypes type, ToolbarButtonStyleSet style, bool enabled, bool visible, bool marked)
    {
        if (index < 0 || index > (int)buttons.size())
            index = (int)buttons.size();

        ToolbarButton *item = new ToolbarButton(this, GenerateCommandId(), text, width, imgindex, check, type, style, enabled, visible, marked);
        buttons.insert(buttons.begin() + index, item);

        AddToNotifyList(item, nrSubControl);
        if (HandleCreated())
            DoInsertButton(index, 1);

        return item;
    }

    ToolbarButton* Toolbar::AddButton(const std::wstring &text, int width, int imgindex, ToolbarButtonCheckStates check, ToolbarButtonTypes type, ToolbarButtonStyleSet style, bool enabled, bool visible, bool marked)
    {
        return InsertButton(-1, text, width, imgindex, check, type, style, enabled, visible, marked);
    }

    void Toolbar::DeleteButton(int index)
    {
        DeleteButtonInner(index, -1);
    }

    void Toolbar::DeleteButtonById(int id)
    {
        DeleteButtonInner(-1, id);
    }

    int Toolbar::ButtonAt(int x, int y)
    {
        if (!HandleCreated())
            return 0;
        Point p(x, y);
        int index = PassMessage(TB_HITTEST, 0, (LPARAM)&p);
        if (index >= 0)
            return index;

        if (-1 - index < (int)buttons.size() && buttons[-1 - index]->Area().Contains(x, y))
            return -1 - index;
        return -1;
    }

    int Toolbar::ButtonAt(Point pt)
    {
        return ButtonAt(pt.x, pt.y);
    }

    int Toolbar::ButtonIndex(ToolbarButton *btn)
    {
        auto it = std::find(buttons.begin(), buttons.end(), btn);
        if (it != buttons.end())
            return it - buttons.begin();
        return -1;
    }

    void Toolbar::DoInsertButton(int index, int count)
    {
        if (!HandleCreated())
            throw L"Cannot insert a button into a toolbar with no handle.";

        if (count == 0)
            return;

        if (count == 1 && index != -1)
        {
            ToolbarButton *d = buttons[index];

            TBBUTTON btn;
            memset(&btn, 0, sizeof(TBBUTTON));

            btn.idCommand = d->id;
            btn.iBitmap = (d->type == tbctDivider || d->type == tbctControlOwner) ? d->width : (d->imgindex == -1 || kind == tbkTextOnly) ? I_IMAGENONE : d->imgindex;
            if (kind == tbkImageOnly || d->text.empty())
                btn.iString = -1;
            else
                btn.iString = (INT_PTR)&d->text[0];

            btn.fsState = d->ButtonStateInner();
            btn.fsStyle = d->ButtonStyleInner();

            if (PassMessage(index == -1 ? TB_ADDBUTTONS : TB_INSERTBUTTON, index == -1 ? count : index, (LPARAM)&btn) == FALSE)
                throw L"Couldn't create toolbar button.";
        }
        else
        {
            if (index != -1)
                throw L"Only a single button can be inserted at a time.";

            std::vector<TBBUTTON> btn(count);

            int dataadded = 0;

            for (int ix = 0; ix < count; ++ix)
            {
                memset(&btn[ix], 0, sizeof(TBBUTTON));

                ToolbarButton *d = buttons[ix];

                btn[ix].idCommand = d->id;
                btn[ix].iBitmap = (d->type == tbctDivider || d->type == tbctControlOwner) ? d->width : (d->imgindex == -1 || kind == tbkTextOnly) ? I_IMAGENONE : d->imgindex;
                if (kind == tbkImageOnly || d->text.empty())
                    btn[ix].iString = -1;
                else
                    btn[ix].iString = (INT_PTR)&d->text[0];

                btn[ix].fsState = d->ButtonStateInner();
                btn[ix].fsStyle = d->ButtonStyleInner();
            }

            if (PassMessage(TB_ADDBUTTONS, count, (LPARAM)&btn[0]) == FALSE)
                throw L"Couldn't add toolbar buttons.";
        }
    }

    void Toolbar::MoveButton(int from, int to)
    {
        if (from < 0 || to < 0 || max(from, to) >= (int)buttons.size())
            throw L"Index out of range.";

        if (from == to)
            return;

        ToolbarButton *data = buttons[from];
        buttons.erase(buttons.begin() + from);
        buttons.insert(buttons.begin() + to, data);

        if (!HandleCreated())
            return;

        toolbarstate << tsSwapping;
        if (PassMessage(TB_MOVEBUTTON, from, to) == 0)
        {
            toolbarstate -= tsSwapping;
            throw L"Couldn't move button.";
        }
        toolbarstate -= tsSwapping;
    }

    void Toolbar::SwapButtons(int first, int second)
    {
        if (first == second)
            return;
        if (first > second)
            std::swap(first, second);

        if (first < 0 || second < 0 || max(first, second) >= (int)buttons.size())
            throw L"Index out of range.";

        ToolbarButton *data1 = buttons[first];
        ToolbarButton *data2 = buttons[second];
        buttons.erase(buttons.begin() + second);
        buttons.erase(buttons.begin() + first);
        buttons.insert(buttons.begin() + first, data2);
        buttons.insert(buttons.begin() + second, data1);

        if (!HandleCreated())
            return;

        toolbarstate << tsSwapping;

        if (PassMessage(TB_DELETEBUTTON, second, 0) == FALSE)
        {
            buttons.erase(buttons.begin() + second);
            buttons.erase(buttons.begin() + first);
            buttons.insert(buttons.begin() + first, data1);
            buttons.insert(buttons.begin() + second, data2);

            toolbarstate -= tsSwapping;
            throw L"Index out of range.";
        }
        if (PassMessage(TB_DELETEBUTTON, first, 0) == FALSE)
        {
            buttons.erase(buttons.begin() + second);
            buttons.erase(buttons.begin() + first);
            buttons.insert(buttons.begin() + first, data1);
            buttons.insert(buttons.begin() + second, data2);

            toolbarstate -= tsSwapping;
            DoInsertButton(second, 1);
            throw L"Index out of range.";
        }
        DoInsertButton(first, 1);
        DoInsertButton(second, 1);
        toolbarstate -= tsSwapping;
    }

    std::wstring Toolbar::ButtonText(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Text();
    }

    void Toolbar::SetButtonText(int index, const std::wstring &newtext)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetText(newtext);
    }

    ToolbarButtonCheckStates Toolbar::ButtonCheckState(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->CheckState();
    }

    void Toolbar::SetButtonCheckState(int index, ToolbarButtonCheckStates newstate)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetCheckState(newstate);
    }

    bool Toolbar::ButtonChecked(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->CheckState() == tbbcsChecked;
    }

    void Toolbar::SetButtonChecked(int index, bool newchecked)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetCheckState(tbbcsChecked);
    }

    bool Toolbar::ButtonIndeterminate(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return ButtonCheckState(index) == tbbcsIndeterminate;
    }

    ToolbarButtonTypes Toolbar::ButtonType(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Type();
    }

    void Toolbar::SetButtonType(int index, ToolbarButtonTypes newtype)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetType(newtype);
    }

    bool Toolbar::ButtonEnabled(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Enabled();
    }

    void Toolbar::SetButtonEnabled(int index, bool newenabled)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetEnabled(newenabled);
    }

    bool Toolbar::ButtonVisible(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Visible();
    }

    void Toolbar::SetButtonVisible(int index, bool newvisible)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetVisible(newvisible);
    }

    bool Toolbar::ButtonMarked(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Marked();
    }

    void Toolbar::SetButtonMarked(int index, bool newmarked)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetMarked(newmarked);
    }

    bool Toolbar::ButtonAutoWidth(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->AutoWidth();
    }

    void Toolbar::SetButtonAutoWidth(int index, bool newauto)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetAutoWidth(newauto);
    }

    //bool Toolbar::ButtonShowText(int index)
    //{
    //    if (index < 0 || index >= (int)buttons.size())
    //        throw L"Index out of range.";
    //    return buttons[index]->ShowText();
    //}

    //void Toolbar::SetButtonShowText(int index, bool newshow)
    //{
    //    if (index < 0 || index >= (int)buttons.size())
    //        throw L"Index out of range.";
    //    buttons[index]->SetShowText(newshow);
    //}

    bool Toolbar::ButtonNoPrefix(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->NoPrefix();
    }

    void Toolbar::SetButtonNoPrefix(int index, bool newnopref)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetNoPrefix(newnopref);
    }

    bool Toolbar::ButtonWrap(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Wrap();
    }

    void Toolbar::SetButtonWrap(int index, bool newwrap)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetWrap(newwrap);
    }

    Rect Toolbar::ButtonArea(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Area();
    }

    tagtype Toolbar::ButtonTag(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->Tag();
    }

    void Toolbar::SetButtonTag(int index, tagtype newtag)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetTag(newtag);
    }

    int Toolbar::ButtonImageIndex(int index)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        return buttons[index]->ImageIndex();
    }

    void Toolbar::SetButtonImageIndex(int index, int newindex)
    {
        if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";
        buttons[index]->SetImageIndex(newindex);
    }





    std::wstring Toolbar::ButtonTextById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Text();
    }

    void Toolbar::SetButtonTextById(int id, const std::wstring &newtext)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetText(newtext);
    }

    ToolbarButtonCheckStates Toolbar::ButtonCheckStateById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->CheckState();
    }

    void Toolbar::SetButtonCheckStateById(int id, ToolbarButtonCheckStates newstate)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetCheckState(newstate);
    }

    bool Toolbar::ButtonCheckedById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->CheckState() == tbbcsChecked;
    }

    void Toolbar::SetButtonCheckedById(int id, bool newchecked)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetCheckState(tbbcsChecked);
    }

    bool Toolbar::ButtonIndeterminateById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return ButtonCheckState(index) == tbbcsIndeterminate;
    }

    ToolbarButtonTypes Toolbar::ButtonTypeById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Type();
    }

    void Toolbar::SetButtonTypeById(int id, ToolbarButtonTypes newtype)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetType(newtype);
    }

    bool Toolbar::ButtonEnabledById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Enabled();
    }

    void Toolbar::SetButtonEnabledById(int id, bool newenabled)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetEnabled(newenabled);
    }

    bool Toolbar::ButtonVisibleById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Visible();
    }

    void Toolbar::SetButtonVisibleById(int id, bool newvisible)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetVisible(newvisible);
    }

    bool Toolbar::ButtonMarkedById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Marked();
    }

    void Toolbar::SetButtonMarkedById(int id, bool newmarked)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetMarked(newmarked);
    }

    bool Toolbar::ButtonAutoWidthById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->AutoWidth();
    }

    void Toolbar::SetButtonAutoWidthById(int id, bool newauto)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetAutoWidth(newauto);
    }

    //bool Toolbar::ButtonShowTextById(int id)
    //{
    //    int index = CommandIdToIndex(id);
    //    if (index < 0)
    //        throw L"Index out of range.";
    //    return buttons[index]->ShowText();
    //}

    //void Toolbar::SetButtonShowTextById(int id, bool newshow)
    //{
    //    int index = CommandIdToIndex(id);
    //    if (index < 0)
    //        throw L"Index out of range.";
    //    buttons[index]->SetShowText(newshow);
    //}

    bool Toolbar::ButtonNoPrefixById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->NoPrefix();
    }

    void Toolbar::SetButtonNoPrefixById(int id, bool newnopref)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetNoPrefix(newnopref);
    }

    bool Toolbar::ButtonWrapById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Wrap();
    }

    void Toolbar::SetButtonWrapById(int id, bool newwrap)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetWrap(newwrap);
    }

    Rect Toolbar::ButtonAreaById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Area();
    }

    tagtype Toolbar::ButtonTagById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->Tag();
    }

    void Toolbar::SetButtonTagById(int id, tagtype newtag)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetTag(newtag);
    }

    int Toolbar::ButtonImageIndexById(int id)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        return buttons[index]->ImageIndex();
    }

    void Toolbar::SetButtonImageIndexById(int id, int newindex)
    {
        int index = CommandIdToIndex(id);
        if (index < 0)
            throw L"Index out of range.";
        buttons[index]->SetImageIndex(newindex);
    }


    void Toolbar::DeleteButtonInner(int index, int id)
    {
        if (id >= 0)
        {
            index = CommandIdToIndex(id);
            if (index < 0)
                throw L"Invalid button id.";
        }
        else if (index < 0 || index >= (int)buttons.size())
            throw L"Index out of range.";

        buttons[index]->Destroy();
    }





}


#endif
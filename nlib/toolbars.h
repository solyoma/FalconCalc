#pragma once

#ifdef TOOLBARS_ENABLED

#include "syscontrol.h"

namespace NLIBNS
{

    class Imagelist;

    //enum ToolbarButtonStates : int {
    //        tbbsChecked = 0x01, tbbsEllipses = 0x40, tbbsEnabled = 0x04, tbbsHidden = 0x08,
    //        tbbsIndeterminate = 0x10, tbbsMarked = 0x80, tbbsPressed = 0x02, tbbsWrap = 0x20
    //};

    enum ToolbarButtonCheckStates : int {
            tbbcsUnchecked = 0x00, tbbcsChecked = 0x01, tbbcsIndeterminate = 0x10,
#ifdef DESIGNING
            tbbcsCount = 3
#endif
    };

    enum ToolbarButtonTypes : int {
            tbctButton, tbctDropDown, tbctDropDownButton, tbctCheckbox, tbctGroupButton, tbctDivider,
            tbctControlOwner /* Not valid to specify when setting type. Automatically set for the button if a control is placed on the toolbar */, 
#ifdef DESIGNING
            tbctCount = 6
#endif
    };

    // A mixture of the winapi toolbar button states and styles in a set that makes sense. 
    enum ToolbarButtonStyles : int {
            tbtsAutoWidth = 0x0010,
            //tbtsShowText = 0x0040,
            tbtsNoPrefix = 0x0020,
            // Unlike the previous values, wrap is only a style logically, and not considered to be one in the system. Its value does not match the windows header defines.
            tbtsWrap = 0x01,
#ifdef DESIGNING
            tbtsCount = 3
#endif
    };
    typedef uintset<ToolbarButtonStyles> ToolbarButtonStyleSet;

    class Toolbar;
    class ToolbarButton : public Object
    {
    private:
        typedef Object  base;
        Toolbar *owner;
        int id;

        enum InnerToolbarButtonState { itcsEnabled = 0x04, itcsHidden = 0x08, itcsMarked = 0x80 };
        typedef uintset<InnerToolbarButtonState> InnerToolbarButtonStateSet;

        WORD width;
        std::wstring text;
        ToolbarButtonCheckStates check;
        ToolbarButtonTypes type;
        ToolbarButtonStyleSet style;
        InnerToolbarButtonStateSet state;
        int imgindex;

        Control *control;

        ToolbarButton(Toolbar *owner, int id, const std::wstring &text, int width, int imgindex, ToolbarButtonCheckStates check, ToolbarButtonTypes type, ToolbarButtonStyleSet style, bool enabled, bool visible, bool marked);

        ToolbarButton() { throw L"?"; }
        ToolbarButton(const ToolbarButton &other) { throw L"?"; }
        ToolbarButton(ToolbarButton &&other) { throw L"?"; }
        ToolbarButton& operator=(const ToolbarButton &other) { throw L"?"; }
        ToolbarButton& operator=(ToolbarButton &&other) { throw L"?"; }

        BYTE ButtonStateInner(); // Returns the state that should be used in the system for the given toolbar button.
        BYTE ButtonStyleInner(); // Returns the style that should be used in the system for the given toolbar button.

        friend class Toolbar;
    protected:
        //virtual void TagChanged(tagtype oldtag);
        virtual ~ToolbarButton();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        Imagelist* DesignImagelist();

        virtual Object* SubOwner();
#endif
        virtual void Destroy();

        Toolbar* Owner();

        int CommandId();
        std::wstring Text();
        void SetText(const std::wstring &newtext);

        WORD Width(); // This width is the desired width of the button, which can be different from the real width on the control. Use Area() to retrieve the current size.
        void SetWidth(WORD newwidth);

        ToolbarButtonCheckStates CheckState();
        void SetCheckState(ToolbarButtonCheckStates newstate);
        bool Checked();
        void SetChecked(bool newchecked);
        bool Indeterminate();

        ToolbarButtonTypes Type();
        void SetType(ToolbarButtonTypes newtype);

        bool Enabled();
        void SetEnabled(bool newenabled);
        bool Visible();
        void SetVisible(bool newvisible);
        //bool TextEllipses();
        //void SetTextEllipses(bool newellipses);
        bool Marked();
        void SetMarked(bool newmarked);
        bool AutoWidth();
        void SetAutoWidth(bool newauto);
        //bool ShowText();
        //void SetShowText(bool newshow);
        bool NoPrefix();
        void SetNoPrefix(bool newnopref);
        bool Wrap();
        void SetWrap(bool newwrap);

        int ImageIndex();
        void SetImageIndex(int newindex);

        int Index();
        Rect Area();
    };

    enum ToolbarKinds {
            tbkImageOnly, tbkTextOnly, tbkTextBelow, tbkSideText,
#ifdef DESIGNING
            tbkCount = 4
#endif
    };

    class Toolbar : public SystemControl
    {
    private:
        typedef SystemControl base;

        ToolbarKinds kind;

        bool topdiv; // Allow the system to draw a visual divider on the top.
        WORD btnwidth; // Width of buttons on the toolbar. Single buttons might have different width.
        WORD btnheight; // Height of buttons on the toolbar.
        //WORD minwidth; // Minimum width of a button. No button can be smaller.
        //WORD maxwidth; // Maximum width of a button. If the text does not fit, ellipses will be shown. Set this to 0 to allow any width for buttons.

        Imagelist *images;  // List of images shown on the buttons.
        Imagelist *himages; // Hot images.
        Imagelist *dimages; // Disabled images.

        std::vector<ToolbarButton*> buttons; // Button controls on the toolbar.

        enum ToolbarStates { tsSwapping = 0x01, tsButtonDeleting = 0x02 };
        typedef uintset<ToolbarStates> ToolbarStateSet;
        ToolbarStateSet toolbarstate;

        void DoInsertButton(int index, int count); // Tells the system to add a button at the given index or at the end of the items list if index is -1. The items list is not changed.
        void DeleteButtonInner(int index, int id); // Deletes a button from the system and from the list of buttons.

        friend class ToolbarButton;
#ifdef DESIGNING
        void DesignAddButton();
        void DoAddButton(void *sender, EventParameters param);
        void DoDeleteButton(void *sender, EventParameters param);
        void DesignSelectButton(int index);
        ToolbarButton* DesignGetButtons(int index);

        virtual void DesignSubSelected(Object *subobj);
        //void DoButtonSelected(void *sender, DesignFormBase::SubObjectSelectParameters param);

        //friend class ToolbarButtonEditorList;
#endif
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual void SaveWindow();

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual bool HandleCommand(Control *parent, WPARAM wParam);
        virtual bool HandleNotify(Control *parent, LPARAM lParam, HRESULT &result);
        virtual void DeleteNotify(Object *object);

        virtual void FontChanged(const FontData &data);

#ifdef DESIGNING
        virtual bool NeedDesignerHittest(int x, int y, LRESULT hittest);
        virtual bool DesignMouseDown(DesignForm *form, short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual bool DesignKeyPush(DesignForm *form, WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);
        virtual bool DesignTabNext(bool entering, bool backwards);
#endif

        virtual ~Toolbar();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual Size DesignSize();

        virtual void InitDesignerMenu(Point clientpos, std::vector<menu_item_data> &inserteditems);
#endif
        Toolbar();
        virtual void Destroy();

        ToolbarKinds Kind();
        void SetKind(ToolbarKinds newkind);

        ToolbarButton* AddButton();
        ToolbarButton* InsertButton(int index);
        ToolbarButton* InsertButton(int index, const std::wstring &text, int width, int imgindex, ToolbarButtonCheckStates check, ToolbarButtonTypes type, ToolbarButtonStyleSet style, bool enabled, bool visible, bool marked);
        ToolbarButton* AddButton(const std::wstring &text, int width, int imgindex, ToolbarButtonCheckStates check, ToolbarButtonTypes type, ToolbarButtonStyleSet style, bool enabled, bool visible, bool marked);
        void DeleteButton(int index);
        void DeleteButtonById(int id);

        int ButtonAt(int x, int y);
        int ButtonAt(Point pt);
        int ButtonIndex(ToolbarButton *btn);

        bool TopDivider() const;
        void SetTopDivider(bool newtopdiv);

        Imagelist* Images();
        void SetImages(Imagelist* newimages);
        Imagelist* HotImages();
        void SetHotImages(Imagelist* newimages);
        Imagelist* DisabledImages();
        void SetDisabledImages(Imagelist* newimages);

        Size ButtonSize();
        void SetButtonSize(const Size &newsize);
        WORD ButtonWidth();
        void SetButtonWidth(WORD newwidth);
        WORD ButtonHeight();
        void SetButtonHeight(WORD newheight);

        //WORD MinButtonWidth();
        //void SetMinButtonWidth(WORD newwidth);
        //WORD MaxButtonWidth();
        //void SetMaxButtonWidth(WORD newwidth);

        int CommandIdToIndex(int id);
        int IndexToCommandId(int index);

        int ButtonCount() const;
        ToolbarButton* Buttons(int index);
        void SwapButtons(int first, int second);
        void MoveButton(int from, int to);

        std::wstring ButtonText(int index);
        void SetButtonText(int index, const std::wstring &newtext);
        ToolbarButtonCheckStates ButtonCheckState(int index);
        void SetButtonCheckState(int index, ToolbarButtonCheckStates newstate);
        bool ButtonChecked(int index);
        void SetButtonChecked(int index, bool newchecked);
        bool ButtonIndeterminate(int index);
        ToolbarButtonTypes ButtonType(int index);
        void SetButtonType(int index, ToolbarButtonTypes newtype);
        bool ButtonEnabled(int index);
        void SetButtonEnabled(int index, bool newenabled);
        bool ButtonVisible(int index);
        void SetButtonVisible(int index, bool newvisible);
        bool ButtonMarked(int index);
        void SetButtonMarked(int index, bool newmarked);
        bool ButtonAutoWidth(int index);
        void SetButtonAutoWidth(int index, bool newauto);
        //bool ButtonShowText(int index);
        //void SetButtonShowText(int index, bool newshow);
        bool ButtonNoPrefix(int index);
        void SetButtonNoPrefix(int index, bool newnopref);
        bool ButtonWrap(int index);
        void SetButtonWrap(int index, bool newwrap);
        Rect ButtonArea(int index);
        tagtype ButtonTag(int index);
        void SetButtonTag(int index, tagtype newtag);
        int ButtonImageIndex(int index);
        void SetButtonImageIndex(int index, int newindex);

        std::wstring ButtonTextById(int id);
        void SetButtonTextById(int id, const std::wstring &newtext);
        ToolbarButtonCheckStates ButtonCheckStateById(int id);
        void SetButtonCheckStateById(int id, ToolbarButtonCheckStates newstate);
        bool ButtonCheckedById(int id);
        void SetButtonCheckedById(int id, bool newchecked);
        bool ButtonIndeterminateById(int id);
        ToolbarButtonTypes ButtonTypeById(int id);
        void SetButtonTypeById(int id, ToolbarButtonTypes newtype);
        bool ButtonEnabledById(int id);
        void SetButtonEnabledById(int id, bool newenabled);
        bool ButtonVisibleById(int id);
        void SetButtonVisibleById(int id, bool newvisible);
        bool ButtonMarkedById(int id);
        void SetButtonMarkedById(int id, bool newmarked);
        bool ButtonAutoWidthById(int id);
        void SetButtonAutoWidthById(int id, bool newauto);
        //bool ButtonShowTextById(int id);
        //void SetButtonShowTextById(int id, bool newshow);
        bool ButtonNoPrefixById(int id);
        void SetButtonNoPrefixById(int id, bool newnopref);
        bool ButtonWrapById(int id);
        void SetButtonWrapById(int id, bool newwrap);
        Rect ButtonAreaById(int id);
        tagtype ButtonTagById(int id);
        void SetButtonTagById(int id, tagtype newtag);
        int ButtonImageIndexById(int id);
        void SetButtonImageIndexById(int id, int newindex);
    };

}


#endif
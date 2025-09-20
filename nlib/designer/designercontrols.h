#pragma once

#include "syscontrol.h"
#include "imagelist.h"

//---------------------------------------------


namespace NLIBNS
{


class ColorDialog;
class DesignFormBase;

struct PanelButtonClickParameters
{
    int id;
    bool down;
    PanelButtonClickParameters(int id, bool down) : id(id), down(down) { }
};
typedef Event<PanelButtonClickParameters> PanelButtonClickEvent;

class ButtonPanel;
class ButtonContainer : public Control
{
private:
    typedef Control base;

    ButtonPanel *owner;

    std::vector<ToolButton*> buttons;

    bool open;

    Size glyphsize;
    int margin;
    int btnheight; // Height of a control button.
    int btnwidth; // Width of a control button if control names are shown. Otherwise btnheight is used for the width of buttons.

    //int cols; // Number of buttons (columns) in a row.
    //int rows; // Number of rows of buttons.

    int MeasureHeight(int width, bool layout = false);
    int TitleSize();
    Rect GlyphRect();
    void Update(); // Change the buttons to show/hide their caption text.

    //void LayoutButtons(int width);
    //void Update();

    ToolButton* FindButton(int id);


    // Event functions:
    void btnclick(void *sender, EventParameters param);
    void btndblclick(void *sender, MouseButtonParameters param);
protected:
    virtual void InitHandle();

    virtual void Paint(const Rect &updaterect);

    virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

    ButtonContainer(ButtonPanel *owner, const std::wstring &title);
    virtual ~ButtonContainer();
    ButtonContainer(const ButtonContainer &orig) : base(orig) { }
    void UpdateImages();

    virtual void ComputeAlignBounds(Rect &bounds);

    friend ButtonPanel;
public:
    void AddButton(const type_info &type);
    void Clear();

    void SetImageIndex(int buttonindex, int imageindex);
};

class NonVisualControl;
class ButtonPanel : public ScrollableControl
{
private:
    typedef ScrollableControl base;

    std::vector<ButtonContainer*> containers;

    bool captions;

    Imagelist *images;

    void ContentUpdated();

    ToolButton *lastfb; // The last button pressed.
    void ButtonClicked(ToolButton *fb);

    NonVisualControl *nvcontrol;
    NotifyEvent OnUnpressButton;

    friend ButtonContainer;
protected:
    virtual void GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide);
    virtual void Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code);
    virtual void MeasureControlArea(Rect &clientrect);

    virtual ~ButtonPanel();
public:
    ButtonPanel();

    void Clear();

    using base::SetParent;
    using base::SetAlignment;
    using base::SetText;

    ButtonContainer* AddContainer(const std::wstring& captiontext);
    ButtonContainer* ButtonPanel::Containers(int index);
    void AddButton(const type_info &type);
    Imagelist* Images();
    void SetImages(Imagelist *newimages);

    bool Captions();
    void SetCaptions(bool newcaptions);

    bool ButtonPressed();
    int PressedId();
    bool NVButtonPressed();
    NonVisualControl* PressedNVControl();

    void PressButton(int id); // Presses the first button with the given ID. Releases any button that is pressed with a different id.
    void RegisterButtonPress(NonVisualControl *control, NotifyEvent OnUnpress); // Called when a non-visual control's button is pressed on some form.
    void UnregisterButtonPress(NonVisualControl *control); // Called when a non-visual control's button become unpressed.

    void UnpressButtons(); // Tell the panel to unpress any pressed buttons and notify registered control owners that their button should be unpressed.
};

class DesignProperties;
class DesignProperty;
class PropertyListbox;

struct PropertyChoiceParameters
{
    int index;

    PropertyChoiceParameters(int index) : index(index) { }
};
typedef Event<PropertyChoiceParameters> PropertyChoiceEvent;

// Listbox for an opening combobox on the property list form.
class PropertyChoiceForm : public Form
{
private:
    typedef Form base;

    Listbox *list;

    int lastitem; // Item the mouse was on before moving out of the listbox.
    int mouseitem; // Current item the mouse is on.
    void SetMouseitem(int newitem);

    void listmousemove(void *sender, MouseMoveParameters param);
    void listmousedown(void *sender, MouseButtonParameters param);
    void listmouseup(void *sender, MouseButtonParameters param);
    void listkeypush(void *sender, KeyPushParameters param);
    void listlosefocus(void *sender, FocusChangedParameters param);
protected:
    virtual void CreateClassParams(ClassParams &params);
    virtual void CreateWindowParams(WindowParams &params);

    virtual ~PropertyChoiceForm();
    friend class PropertyListbox;
public:
    PropertyChoiceForm();
    virtual void Destroy();

    PropertyChoiceEvent OnPropertyChoice;
};

// Structure holding information to identify a specific property or subproperty when the control, whose properties are being listed, changes.
struct PropertyRowIdentityData
{
    std::wstring name;
    std::string proptypename;
    bool category;

    PropertyRowIdentityData(const std::wstring& name, std::string proptypename) : name(name), proptypename(proptypename), category(false) { }
    PropertyRowIdentityData(const std::wstring& name) : name(name), category(true) { }
};
typedef std::vector<PropertyRowIdentityData> PropertyRowIdentity;

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum PropertyStyles : int;
#else
enum PropertyStyles;
#endif

/* Property for a row in the property listbox. One data can hold properties of multiple objects,
 * but all properties in this data have the same name and same base class. Most public functions
 * return the same as the functions of the properties with the same name. Some require an index
 * when the different properties can return or set different values.
 */
class PropertyData
{
private:
    // Struct of property and its object. There can be several for a given item in the listbox, when more controls are selected.
    struct PropertyItem
    {
        Object *owner; // Same as holder if this is a control's property, or the control whose sub-property is identified by item.
        Object *holder; // Pointer to a control or other object which is modified by the property.
        DesignProperty *item; // The property.
        bool isdefault; // Shows up as bold in the list of properties.

        PropertyItem(Object *owner, Object *holder, DesignProperty *item, bool isdefault) : owner(owner), holder(holder), item(item), isdefault(isdefault) {}
    };
    typedef std::vector<PropertyItem> PropertyVector;

    std::wstring name; // Name of the property.
    bool samevalue; // The value is the same for each item in the list of properties and their holders.
    bool open; // Has an open/close box and it is in its open state.
    unsigned int level; // Indentation level of property. 0 for properties that are part of the top control, and higher value for sub-properties that are properties of other properties.
    PropertyVector list; // Properties and their holders for a given row in the listbox.

    void UpdateSamevalue();
public:
    PropertyData(const std::wstring& name, int level);

    void AddProperty(Object *propowner, Object *propholder, DesignProperty *prop, bool isdefault);
    PropertyData CreateSubproperty(int subindex);

    // Own values.
    const std::wstring& Name();
    std::wstring ShortEventName(); // Returns the shortened name of the event that can be appended after control names to assign new events for them, where Name() is "On[ShortEventName]".
    unsigned int Level();
    bool Open();
    bool SameValue();
    void SetOpen(bool newopen);
    bool HasHolder(Object *propholder); // One property's property holder is the control or object passed in propholder.
    PropertyRowIdentityData Identity(); // Returns the identity of this property, which is used to find the same property in the property list when the listed control changes or the list is modified.
    bool IsPropertyOfType(std::string proptypename);

    void PropertyRemoveDefault(int index); // Remove the property's default status in the indexth owner's serializer.
    void PropertySetToDefault(int index); // Make the property default in the indexth owner's serializer.

    // Shared values in all classes of the same property. Calling the functions with the same name of the first property.
    const std::wstring& Category();
    int Priority();
    int OwnerCount();
    bool IsDefaultProperty(int index);
    Object* PropOwner(int index);
    Object* PropHolder(int index);
    bool HasPropertyStyle(PropertyStyles style);
    int ListCount();
    std::wstring ListItem(int itemindex);
    void* ListValue(int itemindex);
    int Selected(int index);
    int SubCount();
    DesignProperty* SubProperty(int subindex);
    std::wstring Value(int index); // The value of the indexth property data.
    bool IsDefault(int index); // Returns whether the current value is the default for the property at the given index.
    bool IsDefault(); // Returns whether the current value is the default for the property in all controls. (Checks defaults in a loop with IsDefault(int index).)
    bool HasDefault(); // Checks whether the property has a default value that can be restored with reset.
    void Reset(); // Changes the value of the property in all controls to their default value. Only works if the properties in all controls have a common default value.
    bool SetValue(Form *parentform, const std::wstring& value, bool afterselect = false); // Set the value of all items and return whether it succeeded.
    bool SetValue(Form *parentform, int index, const std::wstring& value); // Set the value of the indexth property data only. In turn setting "samevalue" to false.
    bool SelectValue(Form *parentform, int index, void* value); // Calls the function with the same name for the property, when a value is selected from the property's value listbox. Returns whether the value has changed.
    bool ClickEdit(Form *parentform, int index); // Calls the function with the same name for the property, when the property's line is double-clicked or the thumbnail image was clicked. Returns whether the value has changed.
    void DrawThumbImage(Canvas *c, const Rect &r, int imgindex);
    void MeasureListItem(MeasureItemParameters param);
    void DrawListItem(DrawItemParameters param);
};

// List of properties for controls and forms. Shows current properties of the selected control or controls.
enum PropertyListType : int { pltValues, pltEvents };

class PropertyListbox : protected Listbox
{
private:
    typedef Listbox base;

    PropertyListType listtype;

    std::list<Object*> ownerlist; // List of all controls who have properties listed.
    std::vector<PropertyData> propertylist; // List of shown properties.

    DesignFormBase *parentform; // The form which will be marked as modified when a property changes.

    bool showcat; // When true, categories are shown and the properties are ordered by them. Otherwise the properties are ordered alphabetically

    float colw; // Position of column separator.
    bool colsizing; // The column is being resized while the mouse button is pressed.

    short mousex; // Real mouse coordinates any time the list gets mouse messages.
    short mousey;
    short actionpos; // Saved y coordinate for mouse when resizing columns or checking boxes to keep the selected row intact.

    bool checkdown; // The mouse button was pressed on a checkbox.
    bool checkhover; // The mouse is hovering over the checkbox.
    int checkrow; // The row of the checkbox the mouse is on. This changes when the mouse moves if the button is unpressed, but stays on the same row if it is pressed.

    ControlEdit *editor;
    std::wstring origtext; // Save text in case the user presses escape and wants to reset the edited value.
    bool editchange; // The text is being changed while matching it to one result in the list of a property.
    bool editdeleting; // The text is modified after pressing the delete or backspace buttons. Make sure we don't autocomplete the text.

    ToolButton *button;

    PropertyChoiceForm *listform;

    bool SetValue(int row, const std::wstring& value, bool afterselect = false);

    void ShowEditor(int row);
    void PlaceButton(int row);
    Rect ButtonRect(int row);
    Rect EditRect(int row, bool fullheight); // Returns the rectangle of the edit box in the given row. If fullheight is true, the top and bottom coordinates of the rectangle will cover the full row height.
    Rect BoxRect(int row);
    Rect CategoryBoxRect(int row);
    Rect ThumbRect(int row);
    void DrawBox(const Rect &rect, bool open);
    void DrawCategoryBox(const Rect &rect, bool open);
    void DrawCheckbox(const Rect &rect, bool checked, bool hovered, bool down, bool enabled);
    Rect CheckboxRect(int row);
    void RestoreEdit(); // Restores the saved text in the edit box and hides the box.
    void OpenDataRow(int row);
    void CloseDataRow(int row);
    void OpenCategory(int row);
    void CloseCategory(int row);
    int DataRow(int datapos); // Returns the index of the row which shows the specified property data.

    int FirstEditableRow(); // First row in the listbox that doesn't show a category field but an editable property values. -1 if no such rows are present.
    int LastEditableRow(); // Last row in the listbox that doesn't show a category field but an editable property values. -1 if no such rows are present.
    int NextEditableRow(int row);
    int PrevEditableRow(int row);

    // Events for the editor:
    void editorleave(void *sender, ActiveChangedParameters param);
    //void editorlosefocus(void *sender, FocusChangedParameters param);
    void editorkeypush(void *sender, KeyPushParameters param);
    void editortextchanged(void *sender, EventParameters param);
    void editordblclick(void *sender, MouseButtonParameters param);
    void editordlgcode(void *sender, DialogCodeParameters param);

    void buttonmousedown(void *sender, MouseButtonParameters param);
    void buttonmouseup(void *sender, MouseButtonParameters param);

    void listformpropertychoice(void *sender, PropertyChoiceParameters param);
    void listformlistmeasureitem(void *sender, MeasureItemParameters param);
    void listformlistdrawitem(void *sender, DrawItemParameters param);
protected:
    virtual void InitHandle();
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void DeleteNotify(Object *object);

    virtual void CaptureChanged();
    virtual void MouseEnter();
    virtual void MouseLeave();
    virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
    virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
    virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

    virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);
    virtual void KeyUp(WORD &keycode, VirtualKeyStateSet vkeys);

    //virtual bool HandleMeasureItem(Control *parent, MEASUREITEMSTRUCT *measures);
    virtual bool HandleDrawItem(Control *parent, DRAWITEMSTRUCT *measures);
    virtual void EraseBackground();

    virtual void SelChanged();

    void QSort(int l, int r, std::vector< std::vector<int> > &poslist);
    bool QSortCmp(int p1, int p2); // Returns true if d1 is larger than d2 (that is there is a need to swap).

    ToolButton *btnAbc; // The button for sorting the property list alphabetically.
    ToolButton *btnCat; // The button for sorting the property list by category.

    PopupMenu *pmProp; // Popup menu for properties and events in the list box.
    // General menu items in the popup menu.
    MenuItem *miAlpha; // Alphabetic order.
    MenuItem *miCat; // Show categories.
    // Menu items for properties.
    MenuItem *miSetDefault; // Set property to be the default when double clicking a control.
    MenuItem *miEdit; // Edit property.
    MenuItem *miDefault; // Reset to default.
    // Menu items for events.
    MenuItem *miAssign; // Assign event.

    void pmpropshow(void *sender, PopupMenuParameters param);
    void pmpropalpha(void *sender, EventParameters param);
    void pmpropcat(void *sender, EventParameters param);
    void pmpropassign(void *sender, EventParameters param);
    void pmpropedit(void *sender, EventParameters param);
    void pmpropdefault(void *sender, EventParameters param);
    void pmpropsetdef(void *sender, EventParameters param);

    void Modify(); // Calls Modify of the currently edited form if there is one.

    virtual ~PropertyListbox();
public:
    PropertyListbox(PropertyListType listtype, ToolButton *abcbutton, ToolButton *catbutton);
    virtual void Destroy();

    void SetProperties(Object *propowner); // Set the object whose properties will be listed in the listbox. The parentform is the form which will be marked as modified when a property is changed.
    void SetProperties(const std::list<Object*> &propowners); // Set a list of objects whose properties will be listed in the listbox. The parentform is the form which will be marked as modified when a property is changed.
    bool IsPropertyOwner(Object *searchowner); // Returns true if the control being passed is one among those whose properties are being listed in the list.
    bool MainPropertyOwner(Object *searchowner); // Returns true if the control being passed is the first or only one whose properties are being listed in the list.

    DesignFormBase* PropertyOwnerForm(); // Returns the form owning the control currently edited.

    PropertyRowIdentity RowIdentity(int row); // Returns a list holding the identity for the given row, or false if the given row is not a property.
    void Select(PropertyRowIdentity &identity, bool forceopen);
    void SelectDefault(); // Selects the row of the default property for the current property owners. If the row is not shown, or multiple owners are displayed, tries to select the Text property, and if not found, selects the Name. If the name is not shown either, the first property row is selected.

    void Clear();

    void Sort(bool showcategories);

    bool Active(); // Returns true when the form that contains this control should appear activated. i.e. when the choice form is visible and active and the parent form is not.

    void FinalizeEdit(bool hide); // Sets the entered value as the new property, and hides the edit box if requested.

    void InvalidateRow(int row);
    void InvalidateRow(Object *propholder, const std::wstring& propertyname);
    void InvalidateEventRows(const std::wstring& eventfuncname);
    void EditProperty();
    void EditProperty(const std::wstring& propertyname, bool activateeditor);
    void EditorKeyPress(WCHAR key);

    using base::Parent;
    using base::SetParent;
    using base::SetAlignment;
    using base::IntegralHeight;
    using base::SetIntegralHeight;
    using base::BorderStyle;
    using base::SetBorderStyle;
    using base::IsVisible;
    using base::SetBounds;
    using base::SetAnchors;
    using base::GetFont;
    using base::SetTabOrder;
    using base::Focus;
};


}
/* End of NLIBNS */


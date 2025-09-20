#pragma once

#include "dialog.h"
#include "generalcontrol.h"

//---------------------------------------------


namespace NLIBNS
{


#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum ModalResults : int;
#else
enum ModalResults;
#endif

class Button;

class DesignerDialog
{
private:
    DialogModes mode;
public:
    DesignerDialog();

    DialogModes DialogMode();
    void SetDialogMode(DialogModes newdialogmode);

    ModalResults Show(Form *dialog, Form *topparent);
};

class ToolButton;
// Custom dialog for picking transparent pixel from a bitmap.
class TransparentColorPickerDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;

    class TransparentColorPicker : public Form
    {
    private:
        typedef Form    base;

        TransparentColorPickerDialog *owner;
    protected:
        void paintimage(void *sender, PaintParameters param);
        void plusbtnclick(void *sender, EventParameters param);
        void minusbtnclick(void *sender, EventParameters param);
        void formresize(void *sender, EventParameters param);
        void formkeypush(void *sender, KeyPushParameters param);
        void formkeyup(void *sender, KeyParameters param);
        void okbtnclick(void *sender, EventParameters param);

        void paintmousedown(void *sender, MouseButtonParameters param);
        void paintmouseup(void *sender, MouseButtonParameters param);
        void paintmousemove(void *sender, MouseMoveParameters param);

        int zoom;
        PointF center;

        bool dragkey; // The space key is down so the image can be dragged even when the color picker tool is selected.
        bool dragging;
        Point dragpos;

        bool picking;
        bool picked;

        Bitmap *bgpattern;

        void PickColor(int x, int y);

        virtual ~TransparentColorPicker();
    public:
        TransparentColorPicker(TransparentColorPickerDialog *owner);

        Bitmap *bmp;

        Paintbox *pbImage;

        Panel *pPick;

        Button *btnOk;
        ToolButton *btnHand;
        ToolButton *btnPick;
        ToolButton *btnPlus;
        ToolButton *btnMinus;
    };
public:
    TransparentColorPickerDialog();

    Point selpt; // Coordinates of the selected transparent pixel.
    Color selcol; // Selected color which will be transparent in the final image.
    bool allownone; // Set to true if it is allowed to have no color selected as a background for the bitmap.
    bool Selected(); // Returns true if a color was selected, false if there should be no transparent pixels on the bitmap.

    bool Show(Form *topparent, Bitmap *bmp);
};

struct ControlBitmap;
class BitmapSelectorDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;

    class BitmapSelector : public Form
    {
    private:
        typedef Form    base;

        BitmapSelectorDialog *owner;
        Bitmap *bgpattern;

        ControlBitmap *bmp;

        void okbtnclick(void *sender, EventParameters param);
        void cancelbtnclick(void *sender, EventParameters param);
        void clearbtnclick(void *sender, EventParameters param);
        void loadbtnclick(void *sender, EventParameters param);
        void savebtnclick(void *sender, EventParameters param);
        void transparentbtnclick(void *sender, EventParameters param);
        void pbpaint(void *sender, PaintParameters param);
        void formresize(void *sender, EventParameters param);
    protected:
        virtual ~BitmapSelector();
    public:
        BitmapSelector(BitmapSelectorDialog *owner, ControlBitmap *bmp);

        virtual void Show();

        Paintbox *pb;
        Button *btnClear;
        Button *btnLoad;
        Button *btnSave;
        Button *btnTransparent;
        Button *btnOk;
        Button *btnCancel;

    };

    ControlBitmap *bmp;
public:
    BitmapSelectorDialog();
    virtual ~BitmapSelectorDialog();

    bool Show(Form *topparent, const ControlBitmap &bmp);
    const ControlBitmap& GetBitmap();
};

class Imagelist;
class ImagelistEditorDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;

    class ImagelistEditor : public Form
    {
    private:
        typedef Form    base;
        typedef ImagelistEditorDialog::ImagelistEditor  self;

        ImagelistEditorDialog *owner;

        void okbtnclick(void *sender, EventParameters param);
        void cancelbtnclick(void *sender, EventParameters param);
        void loadbtnclick(void *sender, EventParameters param);
        void savebtnclick(void *sender, EventParameters param);
        void deletebtnclick(void *sender, EventParameters param);
        void clearbtnclick(void *sender, EventParameters param);
        void upbtnmousedown(void *sender, MouseButtonParameters param);
        void downbtnmousedown(void *sender, MouseButtonParameters param);
        void pbpaint(void *sender, PaintParameters param);
        void pbresize(void *sender, EventParameters param);
        void pbscroll(void *sender, ScrollParameters param);
        void pbkeypush(void *sender, KeyPushParameters param);
        void pbmousedown(void *sender, MouseButtonParameters param);
        void pbscrolloverflow(void *sender, ScrollOverflowParameters param);
        void pbenter(void *sender, ActiveChangedParameters param);
        void pbleave(void *sender, ActiveChangedParameters param);
        void pbdlgcode(void *sender, DialogCodeParameters param);

        int pos;
        int thumbw;
        int thumbh;
        int padding;
        int cols;
        int rows;
        bool stretch;

        Rect ImageRect(int index);

        void SetPos(int newpos);

        void UpdateButtons();
    protected:
        virtual ~ImagelistEditor();
    public:
        ImagelistEditor(ImagelistEditorDialog *owner, Imagelist *list);

        virtual void Show();

        Paintbox *pb;
        Button *btnDelete;
        Button *btnClear;
        Button *btnLoad;
        Button *btnSave;
        Button *btnOk;
        Button *btnCancel;
        Button *btnUp;
        Button *btnDown;

        Imagelist *list;
    };

    Imagelist *list;
public:
    ImagelistEditorDialog();
    virtual ~ImagelistEditorDialog();

    bool Show(Form *topparent, Imagelist *list);

    Imagelist* Images();
};

class PropertyListbox;
class PageControl;
class TabPage;

enum PropertyEditorDialogButtons : int {
            pedbAdd = 0x01,
            pedbAddSub = 0x02,
            pedbDelete = 0x04,
            pedbMove = 0x08,
            pedbMoveSub = 0x10
};

typedef uintset<PropertyEditorDialogButtons> PropertyEditorDialogButtonSet;
// Base class used in a PropertyEditorDialog for changing elements, with mostly virtual abstract functions. Override this to be able to provide elements for the dialog.
class PropertyEditorList
{
public:
    virtual ~PropertyEditorList() {}

    virtual PropertyEditorDialogButtonSet Buttons() = 0;

    virtual void Finished(bool canceled) = 0; // Called when the dialog for editing properties is closed. Canceled is true when the window wasn't closed by pressing the OK button.
    virtual int Count() = 0; // Return the number of items which are listed for their properties, but not counting the number of sub items.
    virtual void Move(int pos, int subpos, int diff) = 0; // Called when the up or down button was pressed to move an item around. Pos is the position of the selected item, subpos is -1 when the main item is selected, otherwise the index of the selected sub item within the selected item. diff is either -1 or 1 depending on whether the item was moved up or down.
    virtual int SubCount(int pos) = 0; // Must return the number of sub items for the item at the given position.
    virtual int Add() = 0; // Add a new item somewhere in the list and return the index to it.
    virtual int AddSub(int pos) = 0; // Add a new subitem for the item at the passed position.
    virtual std::wstring Texts(int pos, int subpos) = 0; // Return the text that should be shown for the passed item, or its subitem if subpos is not negative.
    virtual Object* Objects(int pos, int subpos) = 0; // Return the object which will be edited for the item at the given position, or its subitem if subpos is not negative.
    virtual void Delete(int pos, int subpos) = 0; // Delete the item at the given position, or its subitem if subpos is not negative.
    virtual bool CanDelete(int pos, int subpos) = 0; // Called by the dialog to disable or enable its delete button when something is selected.
};

class Listbox;
class PropertyEditorDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;
protected:
    class PropertyEditorForm : public Form
    {
    private:
        typedef Form    base;
        typedef PropertyEditorForm  self;

        PropertyEditorDialog *owner;
        PropertyEditorList *editlist;

        void UpdateButtons(); // Enables and disabled buttons depending on the current state of the editor.
        void ItemPos(int &pos, int &subpos); // Updates 'pos' and 'subpos' to the position of the currently selected item at 'pos', which must initially be the position in the listbox.
        int ListPos(int itemindex); // Return an item's position in the listbox.

        void onshow(void *sender, EventParameters param);
        void onhide(void *sender, EventParameters param);

        void btnokclick(void *sender, EventParameters param);
        void btncancelclick(void *sender, EventParameters param);
        void fbabcclick(void *sender, EventParameters param);

        void fbupdownmousedown(void *sender, MouseButtonParameters param);
        void fbaddclick(void *sender, EventParameters param);
        void fbaddsubclick(void *sender, EventParameters param);
        void fbdeleteclick(void *sender, EventParameters param);
        void lbitemschanged(void *sender, EventParameters param);
        void lbitemsdraw(void *sender, DrawItemParameters param);
        void lbitemskeypush(void *sender, KeyPushParameters param);
    protected:
        virtual ~PropertyEditorForm();
    public:
        PropertyEditorForm(PropertyEditorDialog *owner, PropertyEditorList *editlist);
        virtual void Destroy();

        Listbox *lbItems;
        ToolButton *fbUp;
        ToolButton *fbDown;
        ToolButton *fbAdd;
        ToolButton *fbAddSub;
        ToolButton *fbDelete;
        Button *btnOk;
        Button *btnCancel;
        PageControl *pcProp;
        TabPage *tpProperties;
        Panel *pProp;
        ToolButton *fbPropAbc;
        ToolButton *fbPropCat;
        Panel *pEvents;
        ToolButton *fbEventAbc;
        ToolButton *fbEventCat;
        PropertyListbox *lbProp;
        TabPage *tpEvents;
        PropertyListbox *lbEvents;
    };
public:
    PropertyEditorDialog();
    virtual ~PropertyEditorDialog();

    bool Show(Form *topparent, PropertyEditorList *editlist);
};

class StringGrid;
class FileExtensionDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;
protected:
    class FileExtensionForm : public Form
    {
    private:
        typedef Form    base;
        typedef FileExtensionForm  self;

        FileExtensionDialog *owner;

        //std::vector<std::pair<std::wstring, std::wstring>> filters;

        std::wstring origstr;
        bool handleedited; // filtersedited is called when the editor becomes hidden in the grid. Avoid updating anything if it is not necessary by setting handleedited to false.
        Point lastedited;

        void UpdateButtons(const Point &pt);
        bool EmptyRow(int row);

        void btnokclick(void *sender, EventParameters param);
        void btncancelclick(void *sender, EventParameters param);
        void btnupclick(void *sender, EventParameters param);
        void btndownclick(void *sender, EventParameters param);
        void btndelclick(void *sender, EventParameters param);

        void filtersbeginedit(void *sender, BeginCellEditParameters param);
        void filtersedited(void *sender, CellEditedParameters param);
        void filterseditkeypush(void *sender, KeyPushParameters param);
        void filtersmousedown(void *sender, MouseButtonParameters param);
        void filtersmousemove(void *sender, MouseMoveParameters param);
        void filtersenter(void *sender, ActiveChangedParameters param);
        void filterseditdlgcode(void *sender, DialogCodeParameters param);
    protected:
        virtual ~FileExtensionForm();
    public:
        FileExtensionForm(FileExtensionDialog *owner, const std::vector<std::pair<std::wstring, std::wstring>> &filters);
        void CopyFilters(std::vector<std::pair<std::wstring, std::wstring>> &dest);

        virtual void Show();

        //ListGrid *lgFilters;
        StringGrid *sgFilters;
        Panel *Panel1;
        Button *btnOk;
        Button *btnCancel;
        ToolButton *btnUp;
        ToolButton *btnDown;
        ToolButton *btnDel;
    };
public:
    FileExtensionDialog();
    virtual ~FileExtensionDialog();

    bool Show(Form *topparent, std::vector<std::pair<std::wstring, std::wstring>> &filters);
};

class IconSelectorDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;

    class IconSelector : public Form
    {
    private:
        typedef Form    base;
        typedef IconSelector    selftype;

        IconSelectorDialog *owner;

        OpenDialog *dlgOpen;
        SaveDialog *dlgSave;
        Paintbox *pb;
        Button *btnLoad;
        Button *btnSave;
        Button *btnClear;
        Button *btnOk;
        Button *btnCancel;

        IconData &icon;

        // Number of columns and rows of icon entries in the paintbox.
        int cols;
        int rows;
        // Previous number of columns of icon entries in the paintbox before resizing the window, so the paintbox can be invalidated if needed.
        int prevcols;

        int sel; // Selected icon entry in the list of icons.

        std::vector<Bitmap> icons; // Icon entries in the loaded icon file.
        bool GenerateBitmaps(IconData &data); // Creates the icon entries that can be shown in the paint box when loading icons and updates the buttons' enabled state.
    protected:
        virtual ~IconSelector();
    public:
        IconSelector(IconSelectorDialog *owner, IconData &icon);
        virtual void Destroy(); /* Call this instead of using delete. */

        void pbPaint(void *sender, PaintParameters param);
        void pbResize(void *sender, EventParameters param);
        void pbScroll(void *sender, ScrollParameters param);
        void pbScrollOverflow(void *sender, ScrollOverflowParameters param);
        void pbScrollAutoSized(void *sender, EventParameters param);
        void btnLoadClick(void *sender, EventParameters param);
        void btnSaveClick(void *sender, EventParameters param);
        void btnClearClick(void *sender, EventParameters param);
        void btnOkClick(void *sender, EventParameters param);
        void btnCancelClick(void *sender, EventParameters param);
    };

    IconData icon;
public:
    IconSelectorDialog();
    virtual ~IconSelectorDialog();

    bool Show(Form *topparent, const IconData &icon);

    IconData& GetIcon();
};

class Label;
class Memo;
class LineEditorDialog : public DesignerDialog
{
private:
    typedef DesignerDialog  base;

    class LineEditor : public Form
    {
    private:
        typedef Form    base;
        typedef LineEditor  selftype;

        LineEditorDialog *owner;

        Button *btnOk;
        Button *btnCancel;
        Label *lbCnt;
    protected:
        virtual ~LineEditor();
    public:
        LineEditor(LineEditorDialog *owner, const std::vector<std::wstring> &lines);

        Memo *mLines;
        void btnOkClick(void *sender, EventParameters param);
        void btnCancelClick(void *sender, EventParameters param);
        void mLinesChanged(void *sender, EventParameters param);
    };

    std::vector<std::wstring> lines;
public:
    LineEditorDialog();
    virtual ~LineEditorDialog();

    bool Show(Form *topparent, const std::vector<std::wstring> &lines);
    void GetLines(std::vector<std::wstring> &out);
};


// Creates a file extension dialog and returns true if it was closed with the OK button. The filters list is updated in that case.
bool ShowFileExtensionDialog(Form *topparent, std::vector<std::pair<std::wstring, std::wstring>> &filters);


}
/* End of NLIBNS */


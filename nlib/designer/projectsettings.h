#pragma once

//---------------------------------------------


namespace NLIBNS
{


class TfrmProjSettings : public Form
{
private:
    IconData icondata; // Data of the temporary icon used in the settings so the original is not overwritten if cancel was selected.
    Bitmap *icon; // The picture of the large system icon loaded into the project as the application icon.
    void GenerateIcon();

    int frmcreated; // Number of forms having the auto create property.
    void UpdateFormButtons(); // For handling form ordering and creation.

    bool formschanged;
    bool formdeleted;

N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
    void InitializeFormAndControls(); /* Control initializations. Do not remove. */
    FolderDialog *dlgFolder;
    Button *btnOk;
    Button *btnCancel;
    Label *Label1;
    Paintbox *pbAppIcon;
    Label *Label7;
    Button *btnChangeIcon;
    Label *Label8;
    Label *Label4;
    Button *btnPath;
    Label *Label6;
    Button *btnRes;
    Label *Label5;
protected:
    virtual ~TfrmProjSettings();
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
public:
    TfrmProjSettings();
    virtual void Destroy(); /* Call this instead of using delete. */
    
    ModalResults ShowModal();
    bool FormsChanged();
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
    PageControl *PageControl1;
    TabPage *TabPage1;
    Edit *edProjName;
    Listbox *lbForms;
    ToolButton *btnFormUp;
    ToolButton *btnFormDown;
    ToolButton *btnFormDel;
    Checkbox *cbFormCreate;
    TabPage *TabPage2;
    Edit *edPath;
    Edit *edRes;
    Edit *edSrcExt;
    Edit *edHeadExt;

    void btnPathClick(void *sender, EventParameters param);
    void btnOkClick(void *sender, EventParameters param);
    void btnCancelClick(void *sender, EventParameters param);
    void btnChangeIconClick(void *sender, EventParameters param);
    void pbAppIconPaint(void *sender, PaintParameters param);
    void btnFormUpClick(void *sender, EventParameters param);
    void btnFormDownClick(void *sender, EventParameters param);
    void cbFormCreateClick(void *sender, EventParameters param);
    void lbFormsChanged(void *sender, EventParameters param);
    void btnFormDelClick(void *sender, EventParameters param);
};

extern TfrmProjSettings *frmProjSettings;


}
/* End of NLIBNS */

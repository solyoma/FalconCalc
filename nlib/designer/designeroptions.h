#pragma once


//---------------------------------------------


namespace NLIBNS
{


    class TfrmOptions : public Form
    {
    private:
    N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
        void InitializeFormAndControls(); /* Control initializations. Do not remove. */
        FolderDialog *dlgFolder;
        PageControl *Pages;
        TabPage *pDesigner;
        Checkbox *cbLoadLast;
        Checkbox *cbOnTop;
        Checkbox *cbSavePos;
        TabPage *pCode;
        Groupbox *Groupbox1;
        Checkbox *cbTabIndent;
        Edit *edIndent;
        Label *Label1;
        Checkbox *cbPubTop;
        TabPage *pPaths;
        Label *Label2;
        Edit *edLibPath;
        Button *btnLibPath;
        Label *Label3;
        Edit *edHeaderPath;
        Button *btnHeaderPath;
        TabPage *pEditing;
        Label *Label4;
        Combobox *cbCtrlAccess;
        Button *btnOk;
        Button *btnCancel;
    protected:
        virtual ~TfrmOptions();
    N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
    public:
        TfrmOptions();
        virtual void Destroy();

        ModalResults ShowModal(); // Copies the settings and updates the controls before showing the form.

    N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
        void cbloadlastcheck(void *sender, CheckboxCheckParameters param);
        void cbontopcheck(void *sender, CheckboxCheckParameters param);
        void cbsaveposcheck(void *sender, CheckboxCheckParameters param);
        void cbtabindentcheck(void *sender, CheckboxCheckParameters param);
        void edindentkeydown(void *sender, KeyParameters param);
        void edindentkeypress(void *sender, KeyPressParameters param);
        void edindenttextchanged(void *sender, EventParameters param);
        void cbpubtopcheck(void *sender, CheckboxCheckParameters param);
        void btnlibpathclick(void *sender, EventParameters param);
        void btnheaderpathclick(void *sender, EventParameters param);
        void btnokclick(void *sender, EventParameters param);
        void btncancelclick(void *sender, EventParameters param);
    };

    extern TfrmOptions *frmOptions;


}
/* End of NLIBNS */

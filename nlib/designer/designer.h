#pragma once

#include "generalcontrol.h"
#include "filestream.h"

//---------------------------------------------


namespace NLIBNS
{

    // Settings for the designer.
    struct Settings
    {
    // Designer:
        std::wstring designer_lastproj; // File name of the last open project.
        bool designer_loadlast; // Load last open project on startup.
        bool designer_ontop; // Keep designer windows on top.
        bool designer_savepos; // Save designer window position and size.
    // Code generation:
        bool code_publictop; // Print "public:" first in class definition.
        bool code_tabindent; // Use tab character for indentation.
        int code_indentsize; // Size of indentation if not using the tab character.
    // Directories:
        std::wstring paths_library; // Library path.
        std::wstring paths_headers; // Header files path of library.
    // Control editing:
        AccessLevels control_access;
    };

    Settings GetSettings(); // Returns a copy of the current settings.


    class ButtonPanel;
    class PropertyListbox;
    class ColorDialog;
    class Menubar;
    class OpenDialog;
    class SaveDialog;
    class DesignFormBase;
    class PageControl;
    class TabPage;
    class FlatButton;
    class Combobox;

#ifdef TOOLBARS_ENABLED
    class Toolbar; // TOOLBAR TEST
#endif

    // Current project's data and settings.
    class Project : public Object
    {
    public:
        typedef std::pair<DesignFormBase*, unsigned int> FormDataItem;
    private:
        typedef Object  base;

        bool constructing; // Only true in the constructor.

        bool modified; // Tells the designer that the project hasn't been saved yet.
        std::wstring name; // Name of project which is used as the name as the exported project files.
        std::wstring srcpath; // Path that acts as root when outputting header and cpp files.
        std::wstring respath; // Path where files can be saved to be included in the rc files when RCDATA is not usable and the resource must be in an external file.
        std::wstring cppext; // Extension of source files. Default is cpp.
        std::wstring hext; // Extension of header files. Default is h.

        std::wstring path; // File name and path of the project when it was opened or saved. Empty if it hasn't been saved to disk.
        std::list<FormDataItem> forms; // Forms of the project.

        IconData icon;

        void FixFormCreationOrder(); // During construction when forms are read, their creation order is unchecked. The function fixes conflicting creation order values.
    protected:
        virtual ~Project();
    public:
        static void EnumerateProperties(DesignSerializer *serializer);
        void DesignSetAppIcon(IconData *data);

        Project(); // Creates an empty project with a single form.
        Project(const std::wstring& fname); // Opens the project specified by fname. Throws an exception if fname is not accessible or not valid.
        virtual void Destroy();

        const std::wstring& Name();
        const std::wstring& Path();

        bool Modified(); // Returns the value of modified.
        void Modify(); // Sets modified to true.
        bool Save(std::wstring fname); // Saves the project to the file specified by fname and resets modified.
        void ClearModified();

        const std::wstring& SourcePath();
        std::wstring ResolvedSourcePath(); // Returns the path where the source files are kept. If the stored path is relative, it is resolved starting from the path of the project file.
        const std::wstring& ResourcePath();
        std::wstring ResolvedResourcePath(); // Returns the path where the source files are kept. If the stored path is relative, it is resolved starting from the path of the project file.
        const std::wstring& CppExt();
        const std::wstring& HeaderExt();
        void SetName(const std::wstring &newname);
        void SetSourcePath(const std::wstring &newpath);
        void SetResourcePath(const std::wstring &newpath);
        void SetCppExt(const std::wstring &newext);
        void SetHeaderExt(const std::wstring &newext);
        int FormCount();
        DesignFormBase* Forms(unsigned int ix);
        DesignFormBase* FindForm(const std::wstring &formname); // Returns the form with the specified name.
        DesignFormBase* NewForm(); // Creates a new form in the project.
        DesignFormBase* NewContainer(); // Creates a new container in the project.
        int FormCreationOrder(DesignFormBase *form);
        void SetFormCreationOrder(DesignFormBase *form, unsigned int neworder);
        void ReorderForms(std::vector<std::wstring> &order); // Updates the forms creation order by a vector of their name.
        void FormDelete(unsigned int ix); // Destroys the form at the given index.
        void FormReferenced(DesignFormBase *main, DesignFormBase *guest); // Called when placing a non visual control from main on guest, to reorder the forms in the forms list.
        IconData* AppIcon();
        void SetAppIcon(IconData *data);
        void SetAppIcon(IconData &&data);
    };

    class Designer : public Form
    {
    private:
        typedef Form base;

        std::list<std::wstring> recents; // File name of recent projects.

        std::vector<std::pair<PropertyListbox*, PropertyListbox*>> editors;
        int miFormsCnt;

        std::map<std::wstring, int> memberindexes; // A list of type strings and numbers to be able to return a unique index for a type on a given form when generating the constructor. The map is cleared before a new form starts serialization, so the indexing is reset for each form.
        DesignFormBase *serializedform; // Form being exported to cpp files.
        DesignFormBase *editedform; // Form parent of control active in the editor, or the form itself if no controls are active.

        FILESTD wfstream resfile; // File to be opened when a resource file must be output for exported data.
        int nextresid; // The next resource id to be used when adding a new resource to the resource file.

        int changing; // Non-zero if the control list is being changed after calling BeginControlChange, and not calling EndControlChange the same number of times.

        void CleanUp(); // Removes the menu items from miForms until it has miFormsCount number of items.

        void OpenProject(const std::wstring &fname);
        void SaveFormSrc(DesignFormBase *form);

        void CreateFormMenu(DesignFormBase *form);

        void UpdateControlList();
        void SelectControl(std::wstring cname);

        void PagesTabChanged(int newtab);

        void selectactiveform(void *sender, AppSelectActiveFormParameters param);
    protected:
        std::vector<MenuItem*> formmenus; // Menu items for forms and containers.

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        OpenDialog *dlgOpenProj;
        SaveDialog *dlgSaveProj;
        SaveDialog *dlgSave;

        Menubar *mainmenu;

        MenuItem *miFile;
        MenuItem *miNewProj;
        MenuItem *miOpenProj;
        MenuItem *miSaveProj;
        MenuItem *miSaveProjAs;
        MenuItem *miSaveSrc;
        MenuItem *miCreateFiles;
        MenuItem *miRecents;
        MenuItem *miExit;

        MenuItem *miEdit;
        MenuItem *miEPropSwitch;
        MenuItem *miEEventSwitch;

        MenuItem *miView;
        MenuItem *miVSort;
        MenuItem *miVSortCat;
        MenuItem *miVSortAlpha;
        MenuItem *miVTab;
        MenuItem *miVProps;
        MenuItem *miVEvents;
        MenuItem *miVControls;
        MenuItem *miVLabels;

        MenuItem *miTools;
        MenuItem *miOptions;
        MenuItem *miProjSettings;

        MenuItem *miForms;
        MenuItem *miFormsAdd;
        MenuItem *miAddForm;
        MenuItem *miAddContainer;

        PageControl *pages;
        TabPage *tpProperties;
        TabPage *tpControls;
        TabPage *tpEvents;

        PropertyListbox *lbProp;
        PropertyListbox *lbEvents;
        Panel *pPropTop;
        FlatButton *btnPropAbc;
        FlatButton *btnPropCat;
        Combobox *cbControlList;

#ifdef TOOLBARS_ENABLED
        Toolbar *tbTest; // TOOLBAR TEST
#endif

        virtual void NameChangeNotify(Object *object, const std::wstring& oldname);
        virtual void ChangeNotify(Object *object, int changetype);

        void minewprojclick(void *sender, EventParameters param);
        void miopenprojclick(void *sender, EventParameters param);
        void misaveprojclick(void *sender, EventParameters param);
        void misaveprojasclick(void *sender, EventParameters param);
        void misavesrcclick(void *sender, EventParameters param);
        void micreatefilesclick(void *sender, EventParameters param);
        void miexitclick(void *sender, EventParameters param);

        void mirecentclick(void *sender, EventParameters param);

        void miepropswitchclick(void *sender, EventParameters param);
        void mieeventswitchclick(void *sender, EventParameters param);

        void mivchoiceclick(void *sender, EventParameters param);
        void mivlabelsclick(void *sender, EventParameters param);

        void mioptionsclick(void *sender, EventParameters param);
        void miprojsettingsclick(void *sender, EventParameters param);

        void sortdownchangedclick(void *sender, EventParameters param);

        void miaddformclick(void *sender, EventParameters param);
        void miaddcontainerclick(void *sender, EventParameters param);

        void miformitemclick(void *sender, EventParameters param);

        void formclose(void *sender, FormCloseParameters param);

        void pagestabchange(void *sender, TabChangeParameters param);

        void cbcontrollistkeypress(void *sender, KeyPressParameters param);
        void cbcontrollistchange(void *sender, EventParameters param);
        void cbcontrollistlosefocus(void *sender, FocusChangedParameters param);

        void _add(bool container);

        ModalResults saveas(); // Shows the save as dialog and if a project name is selected, saves the project. Returns false if cancel was pressed in the dialog.

        bool SaveVerify(); // Shows a message box in case the current project is modified and asks whether to save it.
        void FinalizeEdits(); // Cases the focused property list box to finalize any edits if it has an editor open.

        virtual ~Designer();
    public:
        Designer();
        virtual void Destroy();
        void Initialize();

        void SetTopmost(bool newtopmost);

        void UpdateCaption(); // Set the text on top of the designer form reflect the current loaded project and its modified state.
        void Modify(); // Calls the Modify() function of the project if it has been created.

        DesignFormBase* ActiveForm(); // Returns the form which holds the controls currently showing their properties in the designer.
        void SetActiveForm(DesignFormBase *form); // Selects the form as the new active form in the designer.
        void FormNameChanged(const std::wstring &newname);
        void ControlNameChanged(Object *control, const std::wstring &oldname, const std::wstring &newname);
        void ControlDeleted(Object *control, std::wstring name);
        void ControlAdded(Object *control, std::wstring name);
        void BeginControlChange();
        void EndControlChange();
        bool ControlsChanging();

        void DesignKeyPress(WCHAR key); // Selects a property and updates its text with the character. Only pass printable characters to key.

        void CreateForm();
        void CreateContainer();

        int FormCount();
        DesignFormBase* Forms(int index);
        void FillUp(); // Adds menu items in miForms that will correspond to the forms in the project. Calls CleanUp first. Use it when the order of forms has changed.

        bool MainPropertyOwner(Object *searchowner); // Returns true if the control being passed is the first among the controls which has their properties listed.
        bool IsPropertyOwner(Object *searchowner); // Returns true if the control is among the property owners whose properties are listed.
        void InvalidateRow(Object *propholder, const std::wstring& propertyname); // Invalidates the passed property's row in the property list.
        void InvalidateEventRows(const std::wstring& eventfuncname); // Invalidates all rows in the designer's event list that have the given function name.
        void SetProperties(Object *propowner); // Sets a control as the owner of the properties being listed in the property list. Tag is a custom number to be matched with the previously passed tag value. If the values are different, the displayed properties are updated even if the propowner hasn't changed.
        void SetProperties(const std::list<Object*> &propowners); // Sets a list of controls as the owners of the properties being listed in the property list. Tag is a custom number to be matched with the previously passed tag value. If the values are different, the displayed properties are updated even if the owner list hasn't changed.
        void EditEvent(); // Switches to the currently selected event to start editing.
        void EditProperty(); // Switches to the currently selected property to start editing.
        void EditProperty(const std::wstring& propertyname, bool activateeditor); // Focuses the property with the passed name. If activateeditor is true and the property has an edit button, it activates the editor, just like pressing the edit button would.

        bool FormNameTaken(const std::wstring &name); // Returns whether the passed string is used as the name of some form within the project.
        std::wstring FormClassName(const std::wstring &name); // Returns the class name of a form or container that will be used when the project is written to files.
        bool UnitNameTaken(const std::wstring &unitname); // Returns whether the passed string is used as the name of a unit within the project.
        std::wstring UnusedFormName(const std::wstring &base); // Returns a name of base + [number] that is not taken where number is the smallest possible positive number.

        void ReplaceDesignerBoxes(PropertyListbox *newprop, PropertyListbox *newevents); // Used when a window opens which holds property listboxes which should be used to represent properties currently being edited. Call RestoreDesignerBoxes() when the window closes or the listboxes are no longer functional.
        void RestoreDesignerBoxes(); // Restores the property listboxes used to represent currently being edited properties after a call to ReplaceDesignerBoxes().

        int NextMemberIndex(std::wstring typestr); // Returns a unique number for a given type on a given form which is at least 1 higher than the one previously returned or at least 1.
        int ExportToResource(std::vector<byte> &res); // Called by properties that must be exported to a resource file as well when exporting to cpp. Adds data to the resource file and returns the identifier number assigned to the new data in the rc file.
    
        ButtonPanel *bpControls;
    };


    extern Designer *designer;
    extern Project *project;


}
/* End of NLIBNS */

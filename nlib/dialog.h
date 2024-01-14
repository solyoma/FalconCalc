#pragma once

#ifdef __MINGW32__
#include <shlobj.h>
#else
#include <ShTypes.h>
#endif
#include "objectbase.h"
#include "events.h"
#include "canvas.h"

namespace NLIBNS
{


    class DesignSerializer;

    enum DialogModes : int {
            dmDefault, dmDisableParent, dmDisableBranch, dmDisableTree, dmDisableAppWindows,
#ifdef DESIGNING
            dmCount = 5
#endif
    };

    // Base class for native dialog window wrappers and for forms.
    class Dialog : public NonVisualControl
    {
    private:
        typedef NonVisualControl    base;

        DialogModes mode;
        std::vector<HWND> disabled; // List of forms disabled by this dialog before showing.
        //HWND active; // Handle to the active window before DisableForms was called.

    protected:
        virtual ~Dialog();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        Dialog();

        void DisableForms(Control *topparent); // Disables the forms in the application before showing the dialog. topparent is the parent form of the dialog window.
        void EnableForms();

        DialogModes DialogMode();
        void SetDialogMode(DialogModes newdialogmode);
    };

    class DialogControl : public Dialog
    {
    private:
        typedef Dialog  base;

    protected:
        using base::DisableForms;
        using base::EnableForms;

#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        DialogControl();
#endif
        virtual ~DialogControl() {}
    
    public:
        virtual bool Show(Form *topparent) = 0;
    };

    // System dialog for selecting a color.
    class ColorDialog : public DialogControl
    {
    private:
        typedef DialogControl   base;

        COLORREF customcolors[16];
        Color color; // Set before showing to initialize the color that is selected.
        bool expanded; // The color dialog is expanded to show the "advanced" controls.
    protected:
        virtual ~ColorDialog();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        ColorDialog();

        virtual bool Show(Form *topparent); // Shows the dialog with topparent as the parent for the dialog.

        Color GetColor();
        void SetColor(Color newcolor);
        bool Expanded();
        void SetExpanded(bool newexpanded);
    };


    enum FontDialogOptions : int {
            fntoApplyButton = CF_APPLY /* Show apply button. Only when hook present. */, 
            fntoColorStrikeoutUnderline = CF_EFFECTS /* Show color and font strikeout / underline controls. */,
            fntoFixedPitchOnly = CF_FIXEDPITCHONLY,
            // CF_FORCEFONTEXIST ? ChooseFont should indicate an error condition if the user attempts to select a font or style that is not listed in the dialog box.
            fntoInactiveFonts = CF_INACTIVEFONTS /* Only after windows 7. Show fonts that were hidden in the control panel.  */,
            fntoHideFontFace = CF_NOFACESEL /* Don't show the font name control. */,
            fntoNoCharset = CF_NOSCRIPTSEL /* Don't show the character set control */,
            fntoNoSimulations = CF_NOSIMULATIONS,
            fntoNoSize = CF_NOSIZESEL /* Don't show the font size control. */,
            fntoNoStyle = CF_NOSTYLESEL /* Don't show the font style control. */,
            fntoNoVectorFonts = CF_NOVECTORFONTS,
            fntoNoVertFonts = CF_NOVERTFONTS /* No vertically oriented fonts. */,
            fntoOnlyScalable = CF_SCALABLEONLY /* Only vector based fonts */,
            fntoOnlyScripts = CF_SCALABLEONLY /* From the documentation: allow selection of fonts for all non-OEM and Symbol character sets, as well as the ANSI character set */,
            fntoRestrictCharset = CF_SELECTSCRIPT /* Don't allow changing the character set, so only fonts with the character set of the initial font can be selected. */,
            fntoOnlyTrueType = CF_TTONLY,
#ifdef DESIGNING
            fntoCount = 15
#endif
    };
    typedef uintset<FontDialogOptions> FontDialogOptionSet;

    class Font;

    // System dialog for selecting a font.
    class FontDialog : public DialogControl
    {
    private:
        typedef DialogControl   base;

        class DialogFont : public OwnedFont
        {
        private:
            typedef OwnedFont base;

            FontDialog *owner;
            DialogFont(const DialogFont &copy) : base(FontData()) { throw L"Cannot copy dialog fonts"; }
        protected:
            virtual void DoChanged(const FontData &saveddata);
        public:
            DialogFont(FontDialog *owner);
            DialogFont(FontDialog *owner, const LOGFONT &lf);
            DialogFont(FontDialog *owner, const FontData &data);
        };

        void FontChanged(const FontData &origdata);

        DialogFont *font;
        FontDialogOptionSet options;
        int minsize;
        int maxsize;

        CHOOSEFONT cf; // Passed to the ChooseFont function. During dialog initialization, its lCustData value is updated by the callback function with the dialog's handle which can be used later to send it messages.

        friend class DialogFont;
    protected:
        virtual ~FontDialog();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        FontDialog();
        virtual bool Show(Form *topparent); // Shows the dialog with topparent as the parent for the dialog.

        Font& GetFont();
        void SetFont(const Font &newfont);
        FontDialogOptionSet Options();
        void SetOptions(FontDialogOptionSet newoptions);

        NotifyEvent OnApplyClicked;
    };

    enum FolderDialogOptions : int {
            fodoOnlyFSDir = 0x00000001, fodoDontGoBelowDomain = 0x00000002, fodoStatusArea = 0x00000004 /* Only for old style dialogs. */,
            fodoOnlyFSAncestors = 0x00000008, fodoEditBox = 0x00000010, fodoValidateEdit = 0x00000020 /* Only when fodoEditBox is included. */,
            fodoNewDialogStyle = 0x00000040, fodoIncludeURLs = 0x00000080 /* Only with fodoNewDialogStyle and fodoIncludeFiles. */, 
            fodoUsageHints = 0x00000100 /* Only with fodoNewDialogStyle if no fodoEditBox is present. */, fodoNoNewFolder = 0x00000200,
            fodoDontResolveShortcut = 0x00000400, fodoOnlyComputers = 0x00001000, fodoBrowseForPrinter = 0x00002000 /* When this is set, the root folder should be CSIDL_PRINTERS. */,
            fodoIncludeFiles = 0x00004000, fodoShowShared = 0x00008000 /* Only with fodoNewDialogStyle. */,
            fodoBrowseJunctions /* Only for Win 7 and above. */,
#ifdef DESIGNING
            fodoCount = 16
#endif
    };
    typedef uintset<FolderDialogOptions> FolderDialogOptionSet;
    class FolderDialog : public DialogControl
    {
    private:
        typedef DialogControl   base;

        std::wstring title;

        LPITEMIDLIST rootid;
        std::wstring root;

        LPITEMIDLIST initid;
        std::wstring initdir;

        LPITEMIDLIST IdFromPath(const std::wstring &path);
        LPITEMIDLIST FollowIdToTarget(LPITEMIDLIST orig);

        FolderDialogOptionSet options;

        LPITEMIDLIST resultid;
    protected:
        virtual ~FolderDialog();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        FolderDialog();

        virtual bool Show(Form *topparent); // Shows the dialog with topparent as the parent for the dialog.

        LPITEMIDLIST RootFolderId(); // The root PIDL of the browse dialog. The value is only valid if set by SetRootFolderId or SetRootFolder, and is destroyed when the dialog is destroyed. 
        void SetRootFolderId(LPITEMIDLIST newroot); // Set a folder to be the root of the browse dialog. The dialog frees the root id in its destructor with CoTaskMemFree (so it must have been allocated with CoTaskMemAlloc or an Sh** function call). Do not free it unless it is replaced by calling this function with NULL. If called with any other id, the original value is freed. The original id is freed if setting the root folder with SetRootFolder as well.
        const std::wstring& RootFolder(); // The root folder previously set with SetRootFolder. If SetRootFolderId is called with a non NULL value, this folder is invalidated.
        void SetRootFolder(const std::wstring &newroot); // Set the root folder of the browse dialog. Setting the folder invalidates the root id set with SetRootFolderId.

        LPITEMIDLIST InitialFolderId(); // The initially open path's PIDL of the browse dialog. The value is only valid if set by SetInitialFolderId or SetInitialFolder, and is destroyed when the dialog is destroyed. 
        void SetInitialFolderId(LPITEMIDLIST newpath); // Set a folder to be the initially open path in the browse dialog. The dialog frees the id in its destructor with CoTaskMemFree (so it must have been allocated with CoTaskMemAlloc or an Sh** function call). Do not free it unless it is replaced by calling this function with NULL. If called with any other id, the original value is freed. The original id is freed if setting the initial folder with SetInitialFolder as well.
        const std::wstring& InitialFolder(); // The initially open path previously set with SetInitialFolder. If SetInitialFolderId is called with a non NULL value, this folder is invalidated.
        void SetInitialFolder(const std::wstring &newpath); // Set the initially open path in the browse dialog. Setting the folder invalidates the initial path id set with SetInitialFolderId.

        const std::wstring& Title(); // Shown in the caption of the dialog.
        void SetTitle(const std::wstring& newtitle);

        FolderDialogOptionSet Options();
        void SetOptions(FolderDialogOptionSet newoptions);

        LPITEMIDLIST FolderId(); // The PIDL of the selected folder after Show returns true. The pidl is only valid before the dialog is destroyed or the Show function is called again.
        std::wstring Folder(); // Gives the display name of the selected folder after Show returns true, as retrieved from the id in FolderId. This only works if the retrieved folder id is part of the file system. Otherwise get the Id with FolderId().
    };


    enum FileDialogOptions : int {
            fdoMultiselect = OFN_ALLOWMULTISELECT, fdoCreatePrompt = OFN_CREATEPROMPT, fdoDontAddToRecent = OFN_DONTADDTORECENT, 
            fdoEnableSizing = OFN_ENABLESIZING, fdoFileMustExist = OFN_FILEMUSTEXIST,
            fdoForceShowHidden = OFN_FORCESHOWHIDDEN, fdoHideReadOnly = OFN_HIDEREADONLY, fdoDontFollowLinks = OFN_NODEREFERENCELINKS,
            fdoNoNetworkButton = OFN_NONETWORKBUTTON, fdoNoTestFileCreate = OFN_NOTESTFILECREATE /* Used to disable checks when opening something through a create-nonmodify network share. */,
            fdoNoValidate = OFN_NOVALIDATE, fdoOverwritePrompt = OFN_OVERWRITEPROMPT, fdoPathMustExist = OFN_PATHMUSTEXIST, fdoReadOnlyChecked = OFN_READONLY /* Returned too. */,
            fdoShareAware = OFN_SHAREAWARE /* Notifies the hook about network sharing violations. */, fdoShowHelp = OFN_SHOWHELP,
            /* Returned flags */
            fdoExtensionDifferent = OFN_EXTENSIONDIFFERENT, fdoNotReadOnly = OFN_NONETWORKBUTTON,
            /* Hooks */
            fdoUseHook = OFN_ENABLEHOOK, fdoIncludeNotify = OFN_ENABLEINCLUDENOTIFY,
            /* Templates */
            fdoUseTemplate = OFN_ENABLETEMPLATE, fdoUseTemplateHandle = OFN_ENABLETEMPLATEHANDLE,
            /* Always include */
            fdoExplorer = OFN_EXPLORER, fdoLongNames = OFN_LONGNAMES, fdoNoChangeDir = OFN_NOCHANGEDIR,
#ifdef DESIGNING
            fdoCount = 25
#endif
    };

    enum FileDialogOptionsEx : int {
            fdoxNoPlacesBar = OFN_EX_NOPLACESBAR,
#ifdef DESIGNING
            fdoxCount = 1
#endif
    };

    typedef uintset<FileDialogOptions> FileDialogOptionSet;
    typedef uintset<FileDialogOptionsEx> FileDialogOptionSetEx;

    // Base dialog class for system open and save file dialogs.
    class FileDialog : public DialogControl
    {
    private:
        typedef DialogControl   base;

        static wchar_t *filenamebuffer; /* A common buffer of the size of 32k for all file dialogs used when selecting multiple files. */
        static int instances; /* Number of dialogs sharing the filenamebuffer. When this decreases to 0 the buffer is freed. */

        std::vector<std::pair<std::wstring, std::wstring>> filters; /* Pairs of file name filter descriptions and patterns shown in the dialog. */
        int filterindex; /* Index of the default selected filter. */

        std::wstring initialpath; /* The initial file path to show when the dialog opens. */
        std::wstring initialname; /* The file name to be shown in the edit box when the dialog opens. */
        std::wstring filepath; /* The path to the file when the dialog closed. This is copied to the initial path if the corresponding setting is selected. */
        std::vector<std::wstring> filenames; /* Name of files selected before the user closed the dialog. */

        std::wstring defext; /* Default extension appended to file names that do not contain any extension. Shouldn't contain the '.' character. */

        std::wstring title; /* The window title shown on the top of the dialog. If this is empty, the default title is shown. */

        HINSTANCE templateinstance; /* The instance of the module containing the template or the data block of the template. */
        std::wstring templatename; /* Name of the template resource, when one is used to make the dialog. */
    protected:
        FileDialogOptionSet options;
        FileDialogOptionSetEx optionsex;

        virtual BOOL ShowDialog(OPENFILENAME &info) = 0;

        virtual ~FileDialog();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        std::vector<std::pair<std::wstring, std::wstring>>& DesignGetFilter();
#endif
        FileDialog();

        virtual bool Show(Form *topparent); /* Shows the dialog with topparent as the parent for the dialog. */

        int FilterCount(); /* The number of file name filters added to the dialog. */
        std::wstring FilterString(); /* Retreives the file filters converted to a single string. The elements in the filters and the filters themselves are separated with a | character. i.e. "File Type 1 (*.file)|*.file|File Type 2|*.file2" */
        void SetFilterString(const std::wstring &str); /* Updates the filters with a string of the following format: "File Type 1 (*.file)|*.file|File Type 2|*.file2" */
        std::pair<std::wstring, std::wstring> Filters(int ix); /* Retreives the filter with the given index. The first item in the pair is the filter's human readable description, the second is the actual filter in the format: "*.txt;*.doc" */
        void AddFilter(const std::wstring& filtername, const std::wstring& filetypes); /* Add a new filter to the dialog. The first item in the pair is the filter's human readable description, the second is the actual filter in the format: "*.txt;*.doc" */
        void SetFilter(int ix, const std::wstring& filtername, const std::wstring& filetypes); /* Change the filter with the given index. The first item in the pair is the filter's human readable description, the second is the actual filter in the format: "*.txt;*.doc" */
        void DeleteFilter(int ix); /* Removes a filter from the list of filters with the given index. */
        void ClearFilters(); /* Removes all filters. */

        int SelectedFilter(); /* The default filter's index which is selected when the dialog box opens. */
        void SetSelectedFilter(int newselectedfilter); /* Change the default filter which is selected when the dialog opens. */

        std::wstring InitialPath(); /* The initial path to show when the dialog opens. */
        void SetInitialPath(const std::wstring& newinitialpath); /* Set the initial path which is shown when the dialog opens. */
        const std::wstring& FilePath(); /* Path to the user selected file or files when the dialog closes. */
        int FileNameCount(); /* Number of file names the user selected before closing the dialog. If multiselection of files is disabled, this always returns one on success and zero on cancel or error. */
        const std::wstring& FileNames(int ix); /* Returns the indexth file name when multiselection was true. Index is ignored if multiselect wasn't enabled, and the single file is returned instead. Use FilePath() to get the path to the files. */
        const std::vector<std::wstring>& FileNames(); /* Returns the selected file names in a constant vector. When multiselect is not enabled, it can only have a single item. */
        std::wstring FileName(); /* Name of the selected file on success including its extension but not including the path. This returns the first file name when multiselect was enabled. */
        void SetFileName(const std::wstring& filename); /* Similar to the initial path, the filename will be shown when the dialog opens. */
        std::wstring FullFileName(); /* Returns the path + filename after a successful file selection. If multiple files were selected, only the first item is returned. */

        const std::wstring& DefaultExtension(); /* The default extension to be appended to the file names if they don't contain any extensions. */
        void SetDefaultExtension(const std::wstring& newdefaultextension); /* Change the default extension appended to file names that don't have an extension. */

        const std::wstring& Title(); /* The window title shown on the top of the dialog. If this is empty, the default title is shown. */
        void SetTitle(const std::wstring& newtitle); /* Changes the window title shown on the top of the dialog. */

        FileDialogOptionSet Options(); /* Gets the options that are used when opening the file dialog. */
        void SetOptions(FileDialogOptionSet newoptions); /* Sets the options that are used when opening the file dialog. */
        FileDialogOptionSetEx OptionsEx(); /* Gets the extended options that are used when opening the file dialog. */
        void SetOptionsEx(FileDialogOptionSetEx newoptions); /* Sets the extended options that are used when opening the file dialog. */

        std::pair<HINSTANCE, std::wstring> Template(); /* Returns the instance and resource name of the used template, or the template data block and an empty string if the template is not set as a module but instead as a data block. */
        void SetTemplate(HINSTANCE templatemodule, const std::wstring& templateresource); /* Sets the template module and template resource name. Setting templatemodule to 0 is the same as removing the template, while if the templateresource is an empty string, the module will act like it is a resource block instead. */
        void SetTemplate(HINSTANCE templateblock); /* Sets the data block of a preloaded template, and makes the dialog use that template. */
        void RemoveTemplate();
    };


    // Native open file dialog.
    class OpenDialog : public FileDialog
    {
    private:
        typedef FileDialog  base;
    protected:
        virtual BOOL ShowDialog(OPENFILENAME &info);

        virtual ~OpenDialog() {}
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        OpenDialog();
    };


    // Native save file dialog.
    class SaveDialog : public FileDialog
    {
    private:
        typedef FileDialog  base;
    protected:
        virtual BOOL ShowDialog(OPENFILENAME &info);

        virtual ~SaveDialog() {}
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
#endif
        SaveDialog();
    };


}
/* End of NLIBNS */


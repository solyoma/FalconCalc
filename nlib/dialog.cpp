#include "stdafx_zoli.h"
#include "application.h"
#include "dialog.h"
#include "generalcontrol.h"
#include "buttons.h"
#ifdef DESIGNING
#include "serializer.h"
#include "property_dialog.h"
#include "property_controlbase.h"
#include "property_canvas.h"
#endif
//---------------------------------------------

namespace NLIBNS
{


#ifdef DESIGNING

ValuePair<DialogModes> DialogModeStrings[] = {
    VALUEPAIR(dmDefault),
    VALUEPAIR(dmDisableParent),
    VALUEPAIR(dmDisableBranch),
    VALUEPAIR(dmDisableTree),
    VALUEPAIR(dmDisableAppWindows),
};


void Dialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->Add(L"SetDialogMode", new DialogModesDesignProperty<Dialog>(L"DialogMode", L"Behavior", &Dialog::DialogMode, &Dialog::SetDialogMode))->SetDefault(dmDefault);
}
#endif

Dialog::Dialog() : mode(dmDefault)
{
}

Dialog::~Dialog()
{
    EnableForms();
}

DialogModes Dialog::DialogMode()
{
    return mode;
}

void Dialog::SetDialogMode(DialogModes newdialogmode)
{
    if (mode == newdialogmode)
        return;
    mode = newdialogmode;
}

BOOL CALLBACK dialog_enumthreadwndproc__zapp(HWND hwnd, LPARAM lParam)
{
    std::vector<HWND> *disabled = (std::vector<HWND>*)lParam;
    if (IsWindowEnabled(hwnd) && IsWindowVisible(hwnd))
    {
        disabled->push_back(hwnd);
        EnableWindow(hwnd, false);
    }
    return true;
}

void Dialog::DisableForms(Control *parent)
{
    Control *next;
    std::vector<Control*> cstack;

    DialogModes dmode = mode == dmDefault ? application->DialogMode() : mode;

    if (!parent && (dmode == dmDisableParent || dmode == dmDisableBranch || dmode == dmDisableTree))
        return;

    switch (dmode)
    {
    case dmDisableParent:
        if (parent)
            dialog_enumthreadwndproc__zapp(parent->Handle(), (LPARAM)&disabled);
        break;
    case dmDisableAppWindows:
        EnumThreadWindows(MainThreadId, &dialog_enumthreadwndproc__zapp, (LPARAM)&disabled);
        break;
    case dmDisableBranch:
        while (parent && IsWindowVisible(parent->Handle()) && IsWindowEnabled(parent->Handle()))
        {
            dialog_enumthreadwndproc__zapp(parent->Handle(), (LPARAM)&disabled);
            parent = parent->TopLevelParent();
        }
        break;
    case dmDisableTree:
        while (parent && (next = parent->TopLevelParent()) != NULL)
            parent = next;
        cstack.push_back(parent);

        while (cstack.size())
        {
            next = cstack.back();
            dialog_enumthreadwndproc__zapp(next->Handle(), (LPARAM)&disabled);
            for (int ix = 0; ix < next->TopChildCount(); ++ix)
            {
                cstack.push_back(next->TopChild(ix));
            }
        }

        break;
    default:
        break;
    }
}

void Dialog::EnableForms()
{
    Control *c;
    for (auto it = disabled.begin(); it != disabled.end(); ++it)
    {
        if ((c = application->ControlFromHandle(*it)) != NULL)
        {
            if (!c->Parent()) // Only top level controls with a valid handle have no parent.
                EnableWindow(*it, true);
        }
        else // The disabled window is not a control in the library.
            EnableWindow(*it, true);
    }
}


//---------------------------------------------


#ifdef DESIGNING
void DialogControl::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
}

DialogControl::DialogControl() : base()
{
}


//---------------------------------------------


#endif
#ifdef DESIGNING
void ColorDialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->Add(L"SetColor", new ColorDesignProperty<ColorDialog>(L"Color", L"Settings", true, false, false, &ColorDialog::GetColor, &ColorDialog::SetColor))->MakeDefault();
    serializer->Add(L"SetExpanded", new BoolDesignProperty<ColorDialog>(L"Expanded", L"Settings", &ColorDialog::Expanded, &ColorDialog::SetExpanded))->SetDefault(false);
}
#endif

ColorDialog::ColorDialog() : color(clWhite), expanded(false)
{
    for (int i = 0; i < 16; i++)
        customcolors[i] = 0x00ffffff;
}

ColorDialog::~ColorDialog()
{
}

bool ColorDialog::Show(Form *topparent)
{
    CHOOSECOLOR cc;
    memset(&cc,0,sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = topparent ? topparent->Handle() : NULL;
    cc.rgbResult = color;
    cc.lpCustColors = customcolors;
    cc.Flags = CC_ANYCOLOR | CC_RGBINIT | (expanded ? CC_FULLOPEN : 0);
    DisableForms(topparent);
    if (!ChooseColor(&cc))
    {
        EnableForms();
        return false;
    }
    color = Color(cc.rgbResult);
    EnableForms();
    return true;
}

Color ColorDialog::GetColor()
{
    return color;
}

void ColorDialog::SetColor(Color newcolor)
{
    color = newcolor;
}

bool ColorDialog::Expanded()
{
    return expanded;
}

void ColorDialog::SetExpanded(bool newexpanded)
{
    expanded = newexpanded;
}


//---------------------------------------------


FontDialog::DialogFont::DialogFont(FontDialog *owner) : owner(owner)
{
}

FontDialog::DialogFont::DialogFont(FontDialog *owner, const LOGFONT &lf) : base(FontData(lf)), owner(owner)
{
}

FontDialog::DialogFont::DialogFont(FontDialog *owner, const FontData &lf) : base(lf), owner(owner)
{
}

void FontDialog::DialogFont::DoChanged(const FontData &saveddata)
{
    owner->FontChanged(saveddata);
}


//---------------------------------------------


#ifdef DESIGNING
ValuePair<FontDialogOptions> FontDialogOptionStrings[] = {
    VALUEPAIR(fntoApplyButton),
    VALUEPAIR(fntoColorStrikeoutUnderline),
    VALUEPAIR(fntoFixedPitchOnly),
    VALUEPAIR(fntoInactiveFonts),
    VALUEPAIR(fntoHideFontFace),
    VALUEPAIR(fntoNoCharset),
    VALUEPAIR(fntoNoSimulations),
    VALUEPAIR(fntoNoSize),
    VALUEPAIR(fntoNoStyle),
    VALUEPAIR(fntoNoVectorFonts),
    VALUEPAIR(fntoNoVertFonts),
    VALUEPAIR(fntoOnlyScalable),
    VALUEPAIR(fntoOnlyScripts),
    VALUEPAIR(fntoRestrictCharset),
    VALUEPAIR(fntoOnlyTrueType),
};

void FontDialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->Add(L"GetFont", new FontDesignProperty<FontDialog>(L"Font", L"Value", false, &FontDialog::GetFont, &FontDialog::SetFont))->MakeDefault();
    serializer->Add(L"SetOptions", new FontDialogOptionSetDesignProperty<FontDialog>(L"Options", L"Behavior", &FontDialog::Options, &FontDialog::SetOptions))->SetDefault(fntoColorStrikeoutUnderline);
    serializer->AddEvent<FontDialog, NotifyEvent>(L"OnApplyClicked", std::wstring());
}
#endif

FontDialog::FontDialog() : font(NULL), options(fntoColorStrikeoutUnderline), minsize(-1), maxsize(-1)
{
    font = new FontDialog::DialogFont(this);
}

FontDialog::~FontDialog()
{
    delete font;
}

#ifndef IDAPPLY
#define IDAPPLY_DEFINED_FOR_FONT_DIALOG
#define IDAPPLY    0x0402
#endif
static FontDialog *_shownfntdlg = 0;
static UINT_PTR CALLBACK FontDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    if (!_shownfntdlg)
        return 0;

    CHOOSEFONT *cf;

    switch (uiMsg)
    {
    case WM_INITDIALOG:
        cf = (CHOOSEFONT*)lParam;
        cf->lCustData = (LPARAM)hdlg;
        break;
    case WM_COMMAND:
        if (_shownfntdlg->OnApplyClicked && LOWORD(wParam) == IDAPPLY && HIWORD(wParam) == BN_CLICKED)
        {
            LOGFONT lf = {0};
            SendMessage(hdlg, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)&lf);
            Font fnt(lf);
            _shownfntdlg->SetFont(fnt);
            _shownfntdlg->OnApplyClicked(_shownfntdlg, EventParameters());
            return 1;
        }
        break; 
    }

    if (!_shownfntdlg->OnApplyClicked)
        return 0;
    return 0;
}
#ifdef IDAPPLY_DEFINED_FOR_FONT_DIALOG
#undef IDAPPLY_DEFINED_FOR_FONT_DIALOG
#undef IDAPPLY
#endif

bool FontDialog::Show(Form *topparent)
{
    memset(&cf, 0, sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = topparent ? topparent->Handle() : NULL;
    cf.hDC = 0;
    cf.Flags = (UINT)options | CF_ENABLEHOOK | CF_INITTOLOGFONTSTRUCT;
    if (minsize >= 0 && maxsize >= 0)
        cf.Flags |= CF_LIMITSIZE;

    LOGFONT lf(font->Data());
    cf.lpLogFont = &lf;
    cf.rgbColors = font->GetColor();
    cf.lpfnHook = &FontDialogHookProc;

    cf.nSizeMin = minsize;
    cf.nSizeMax = maxsize;

    _shownfntdlg = this;
    try
    {
        if (ChooseFont(&cf) != TRUE)
        {
            _shownfntdlg = NULL;
            return false;
        }
    }
    catch(...)
    {
        _shownfntdlg = NULL;
        throw;
    }

    _shownfntdlg = NULL;
    delete font;
    font = new DialogFont(this, lf);
    font->SetColor(cf.rgbColors);
    return true;
}

Font& FontDialog::GetFont()
{
    return *font;
}

void FontDialog::SetFont(const Font &afont)
{
    delete font;
    font = NULL;
    font = new DialogFont(this, afont.Data());

    // Doesn't work due to bug in windows.
    //if (_shownfntdlg) // The font is changed while the dialog is shown. Update the selected font by sending us a message.
    //{
    //    if (cf.lCustData)
    //    {
    //        LOGFONT lf = font->Data();
    //        SendMessage((HWND)cf.lCustData, WM_CHOOSEFONT_SETLOGFONT, 0, (LPARAM)&lf);
    //    }
    //}
}

void FontDialog::FontChanged(const FontData &origdata)
{
    if (font->Data() == origdata || !_shownfntdlg) // The font is already updated when this function is called by it. Only update if the original values were different and the dialog is shown at the moment.
        return;

    // Doesn't work due to bug in windows.
    //if (_shownfntdlg) // The font is changed while the dialog is shown. Update the selected font by sending us a message.
    //{
    //    if (cf.lCustData)
    //    {
    //        LOGFONT lf = font->Data();
    //        SendMessage((HWND)cf.lCustData, WM_CHOOSEFONT_SETLOGFONT, 0, (LPARAM)&lf);
    //    }
    //}
}

FontDialogOptionSet FontDialog::Options()
{
    return options;
}

void FontDialog::SetOptions(FontDialogOptionSet newoptions)
{
    options = newoptions;
}


//---------------------------------------------


#ifdef DESIGNING
ValuePair<FolderDialogOptions> FolderDialogOptionStrings[] = {
    VALUEPAIR(fodoBrowseForPrinter),
    VALUEPAIR(fodoBrowseJunctions),
    VALUEPAIR(fodoDontGoBelowDomain),
    VALUEPAIR(fodoDontResolveShortcut),
    VALUEPAIR(fodoEditBox),
    VALUEPAIR(fodoIncludeFiles),
    VALUEPAIR(fodoIncludeURLs),
    VALUEPAIR(fodoNewDialogStyle),
    VALUEPAIR(fodoNoNewFolder),
    VALUEPAIR(fodoOnlyComputers),
    VALUEPAIR(fodoOnlyFSAncestors),
    VALUEPAIR(fodoOnlyFSDir),
    VALUEPAIR(fodoShowShared),
    VALUEPAIR(fodoStatusArea),
    VALUEPAIR(fodoUsageHints),
    VALUEPAIR(fodoValidateEdit),
};

void FolderDialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->Add(L"SetRootFolder", new StringDesignProperty<FolderDialog>(L"RootFolder", L"Path", &FolderDialog::RootFolder, &FolderDialog::SetRootFolder))->SetDefault(std::wstring());
    serializer->Add(L"SetInitialFolder", new StringDesignProperty<FolderDialog>(L"InitialFolder", L"Path", &FolderDialog::InitialFolder, &FolderDialog::SetInitialFolder))->SetDefault(std::wstring());
    serializer->Add(L"SetTitle", new StringDesignProperty<FolderDialog>(L"Title", L"Appearance", &FolderDialog::Title, &FolderDialog::SetTitle))->SetDefault(L"Browse for folder");
    serializer->Add(L"SetOptions", new FolderDialogOptionSetDesignProperty<FolderDialog>(L"Options", L"Behavior", &FolderDialog::Options, &FolderDialog::SetOptions))->SetDefault(fodoOnlyFSDir | fodoDontGoBelowDomain | fodoOnlyFSAncestors | fodoNewDialogStyle | fodoBrowseJunctions);
}
#endif

FolderDialog::FolderDialog() : title(L"Browse for folder"), rootid(NULL), initid(NULL),
            options(fodoOnlyFSDir | fodoDontGoBelowDomain | fodoOnlyFSAncestors | fodoNewDialogStyle | fodoBrowseJunctions), resultid(NULL)
{
}

FolderDialog::~FolderDialog()
{
    if (rootid != NULL)
        CoTaskMemFree(rootid);
    if (resultid != NULL)
        CoTaskMemFree(resultid);
}

static int CALLBACK bffdproc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg != BFFM_INITIALIZED)
        return 0;
    FolderDialog *dialog = (FolderDialog*)lpData;

    if (dialog->InitialFolderId() != NULL || !dialog->InitialFolder().empty())
        SendMessage(hwnd, BFFM_SETSELECTION, dialog->InitialFolderId() != NULL ? FALSE : TRUE, dialog->InitialFolderId() != NULL ? (LPARAM)dialog->InitialFolderId() : (LPARAM)dialog->InitialFolder().c_str());

    return 0;
}

bool FolderDialog::Show(Form *topparent)
{
    if (resultid != NULL)
        CoTaskMemFree(resultid);
    resultid = NULL;

    BROWSEINFO binfo;
    binfo.hwndOwner = topparent ? topparent->Handle() : NULL;

    if (!rootid)
    {
        if ((rootid = IdFromPath(root)) == NULL)
            throw L"The given path is not a valid file system folder.";
    }
    binfo.pidlRoot = rootid;

    wchar_t dispname[MAX_PATH + 1]; // For retrieving the display name of the selected folder.
    binfo.pszDisplayName = dispname;
    binfo.lpszTitle = title.c_str();
    binfo.ulFlags = options;

    binfo.lpfn = &bffdproc;
    binfo.lParam = (LPARAM)this;

    resultid = SHBrowseForFolder(&binfo);
    DisableForms(topparent);
    if (resultid == 0)
    {
        EnableForms();
        return false;
    }

    if (!options.contains(fodoDontResolveShortcut) && !WinVerSupported(5, 1)) // Win 2000 and below.
    {
        LPITEMIDLIST p2 = FollowIdToTarget(resultid);
        if (p2)
        {
            CoTaskMemFree(resultid);
            resultid = p2;
        }
    }

    EnableForms();
    return true;
}

LPITEMIDLIST FolderDialog::FollowIdToTarget(LPITEMIDLIST orig)
{
    IShellFolder *folder;
    IShellLink *lnk;
    LPCITEMIDLIST relativefolder;
    if (SHBindToParent(orig, IID_IShellFolder, (void**)&folder, &relativefolder) != S_OK)
        return NULL;

    if (folder->GetUIObjectOf(NULL, 1, &relativefolder, IID_IShellLink, NULL, (void**)&lnk) != S_OK)
    {
        folder->Release();
        return NULL;
    }
    folder->Release();

    LPITEMIDLIST target;
    if (lnk->GetIDList(&target) != S_OK)
    {
        lnk->Release();
        return NULL;
    }
    lnk->Release();
    return target;
}

LPITEMIDLIST FolderDialog::IdFromPath(const std::wstring &path)
{
    IShellFolder *desk;
    if (SHGetDesktopFolder(&desk) != S_OK)
        return NULL;
    ULONG eaten = 0;
    ULONG attrib = 0;
    LPITEMIDLIST idl;
    if (desk->ParseDisplayName(NULL, NULL, const_cast<wchar_t*>(path.c_str()), &eaten, &idl, &attrib) != S_OK)
    {
        desk->Release();
        return NULL;
    }
    desk->Release();
    return idl;
}

LPITEMIDLIST FolderDialog::RootFolderId()
{
    return rootid;
}

void FolderDialog::SetRootFolderId(LPITEMIDLIST newroot)
{
    if (rootid)
        CoTaskMemFree(rootid);
    rootid = newroot;
    if (newroot)
        root = std::wstring();
}

const std::wstring& FolderDialog::RootFolder()
{
    return root;
}

void FolderDialog::SetRootFolder(const std::wstring &newroot)
{
    if (rootid)
        CoTaskMemFree(rootid);
    rootid = NULL;
    root = newroot;
}

LPITEMIDLIST FolderDialog::InitialFolderId()
{
    return initid;
}

void FolderDialog::SetInitialFolderId(LPITEMIDLIST newdir)
{
    if (initid)
        CoTaskMemFree(initid);
    initid = newdir;
    if (newdir)
        initdir = std::wstring();
}

const std::wstring& FolderDialog::InitialFolder()
{
    return initdir;
}

void FolderDialog::SetInitialFolder(const std::wstring &newdir)
{
    if (initid)
        CoTaskMemFree(rootid);
    initid = NULL;
    initdir = newdir;
}

const std::wstring& FolderDialog::Title()
{
    return title;
}

void FolderDialog::SetTitle(const std::wstring& newtitle)
{
    title = newtitle;
}

FolderDialogOptionSet FolderDialog::Options()
{
    return options;
}

void FolderDialog::SetOptions(FolderDialogOptionSet newoptions)
{
    options = newoptions;
}

LPITEMIDLIST FolderDialog::FolderId()
{
    return resultid;
}

std::wstring FolderDialog::Folder()
{
    if (!resultid)
        return std::wstring();
    wchar_t result[MAX_PATH + 1] = {0};
    SHGetPathFromIDList(resultid, result);

    return std::wstring(result);
}


//---------------------------------------------


#define _FILEDIALOG_FILENAMEBUFFER_MAXSIZE__ABL    (32*1024)
wchar_t* FileDialog::filenamebuffer = NULL; 
int FileDialog::instances = 0;

#ifdef DESIGNING
ValuePair<FileDialogOptions> FileDialogOptionStrings[] = {
    VALUEPAIR(fdoCreatePrompt),
    VALUEPAIR(fdoDontAddToRecent),
    VALUEPAIR(fdoDontFollowLinks),
    VALUEPAIR(fdoEnableSizing),
    VALUEPAIR(fdoExplorer),
    VALUEPAIR(fdoExtensionDifferent),
    VALUEPAIR(fdoFileMustExist),
    VALUEPAIR(fdoForceShowHidden),
    VALUEPAIR(fdoHideReadOnly),
    VALUEPAIR(fdoIncludeNotify),
    VALUEPAIR(fdoLongNames),
    VALUEPAIR(fdoMultiselect),
    VALUEPAIR(fdoNoChangeDir),
    VALUEPAIR(fdoNoNetworkButton),
    VALUEPAIR(fdoNoTestFileCreate),
    VALUEPAIR(fdoNotReadOnly),
    VALUEPAIR(fdoNoValidate),
    VALUEPAIR(fdoOverwritePrompt),
    VALUEPAIR(fdoPathMustExist),
    VALUEPAIR(fdoReadOnlyChecked),
    VALUEPAIR(fdoShareAware),
    VALUEPAIR(fdoShowHelp),
    VALUEPAIR(fdoUseHook),
    VALUEPAIR(fdoUseTemplate),
    VALUEPAIR(fdoUseTemplateHandle),
};

ValuePair<FileDialogOptionsEx> FileDialogOptionStringsEx[] = {
    VALUEPAIR(fdoxNoPlacesBar),
};

void FileDialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->Add(L"SetInitialPath", new FilePathDesignProperty<FileDialog>(L"InitialPath", L"File", &FileDialog::InitialPath, &FileDialog::SetInitialPath))->SetDefault(std::wstring());
    serializer->Add(L"SetFileName", new FileNameDesignProperty<FileDialog>(L"FileName", L"File", false, &FileDialog::FileName, &FileDialog::SetFileName))->SetDefault(L"");
    serializer->Add(L"SetDefaultExtension", new StringDesignProperty<FileDialog>(L"DefaultExtension", L"File", &FileDialog::DefaultExtension, &FileDialog::SetDefaultExtension))->SetDefault(std::wstring());
    serializer->Add(L"SetTitle", new StringDesignProperty<FileDialog>(L"Title", L"Appearance", &FileDialog::Title, &FileDialog::SetTitle));
    serializer->Add(L"SetOptions", new FileDialogOptionSetDesignProperty<FileDialog>(L"Options", L"Behavior", &FileDialog::Options, &FileDialog::SetOptions))->SetDefault(fdoExplorer | fdoLongNames | fdoEnableSizing | fdoDontFollowLinks);
    serializer->Add(L"SetOptionsEx", new FileDialogOptionSetExDesignProperty<FileDialog>(L"OptionsEx", L"Behavior", &FileDialog::OptionsEx, &FileDialog::SetOptionsEx))->SetDefault(0);
    serializer->Add(L"SetFilterString", new FileFilterDesignProperty<FileDialog>(L"Filters", L"Selection", &FileDialog::DesignGetFilter, &FileDialog::FilterString, &FileDialog::SetFilterString))->SetDefault(std::wstring())->MakeDefault();
}

std::vector<std::pair<std::wstring, std::wstring>>& FileDialog::DesignGetFilter()
{
    return filters;
}
#endif

FileDialog::FileDialog() : filterindex(0), templateinstance(NULL)
{
    if (instances++ == 0)
        filenamebuffer = new wchar_t[_FILEDIALOG_FILENAMEBUFFER_MAXSIZE__ABL];
    options << fdoExplorer << fdoLongNames << fdoEnableSizing << fdoDontFollowLinks;
}

FileDialog::~FileDialog()
{
    if (--instances == 0)
        delete[] filenamebuffer;
}

bool FileDialog::Show(Form *topparent)
{
    OPENFILENAME info = {0};
    info.lStructSize = sizeof(OPENFILENAME);
    info.hwndOwner = topparent ? topparent->Handle() : NULL;

    info.hInstance = NULL; // Used for templates later.

    int bufsize = 1;
    for (auto it = filters.begin(); it != filters.end(); ++it)
        bufsize += (*it).first.length() + 1 + (*it).second.length() + 1;
    if (bufsize > 1)
    {
        wchar_t *bufpos = new wchar_t[bufsize];
        info.lpstrFilter = bufpos;

        for (auto it = filters.begin(); it != filters.end(); ++it)
        {
            wcscpy(bufpos, (*it).first.c_str());
            bufpos += (*it).first.length() + 1;
            wcscpy(bufpos, (*it).second.c_str());
            bufpos += (*it).second.length() + 1;
        }
        *bufpos = 0;
    }

    info.nFilterIndex = filterindex + 1;

    memset(filenamebuffer, 0, sizeof(wchar_t) * _FILEDIALOG_FILENAMEBUFFER_MAXSIZE__ABL);
    if (initialname.length())
        wcscpy(filenamebuffer, initialname.c_str());
    info.lpstrFile = filenamebuffer;
    info.nMaxFile = _FILEDIALOG_FILENAMEBUFFER_MAXSIZE__ABL;

    info.lpstrInitialDir = initialpath.c_str();

    if (title.length() > 0)
        info.lpstrTitle = title.c_str();

    info.Flags = options;
    info.FlagsEx = optionsex;

    if (options.contains(fdoUseTemplate) || options.contains(fdoUseTemplateHandle))
    {
        info.hInstance = templateinstance;
        if (options.contains(fdoUseTemplateHandle))
            info.lpTemplateName = templatename.c_str();
    }

    DisableForms(topparent);
    /* SHOWING THE DIALOG */
    BOOL val = ShowDialog(info);
    /* ------------------ */
    EnableForms();

    delete[] info.lpstrFilter;

    DWORD err;
    if (val == FALSE && (err = CommDlgExtendedError()) != 0)
    {
        std::wstringstream wserr;
        wserr << L"An error occured while showing the file dialog! The error code is: " << val << " You can look up its meaning on the msdn page describing CommDlgExtendedError";
        std::wstring str = wserr.str();
        throw str.c_str();
    }

    if (val != 0)
    {
        // info.lpstrFile contains the selected file or files. If multiple files were selected, they are all listed in this
        // array with NULL separator character, and the first item is the path to all the files. For a single file,
        // it is a single continuous string with no separators, even if multiple selection was enabled.

        bool multi = options.contains(fdoMultiselect);
        wchar_t *namepos = info.lpstrFile + info.nFileOffset;
        if (multi && (info.nFileOffset == 0 || *(namepos - 1) != 0))
            multi = false;

        filenames.clear();
        std::wstringstream snames;
        if (!multi)
        {
            filepath = std::wstring(info.lpstrFile, info.nFileOffset);
            if (defext.length() > 0 && FindFileNameExtension(namepos) == 0)
            {
                snames << namepos << L'.' << defext;
                filenames.push_back(snames.str()); 
            }
            else
                filenames.push_back(namepos);
        }
        else
        {
            filepath = info.lpstrFile;
            while (*namepos != 0)
            {
                if (defext.length() > 0 && FindFileNameExtension(namepos) == 0)
                {
                    snames << namepos << L'.' << defext;
                    filenames.push_back(snames.str()); 
                    snames.clear();
                    snames.str(std::wstring());
                    snames.seekg(0);
                    snames.seekp(0);
                }
                else
                    filenames.push_back(namepos);
                namepos += wcslen(namepos) + 1;
            }
        }
        return true;
    }


    return false;
}

int FileDialog::FilterCount()
{
    return filters.size();
}

std::pair<std::wstring, std::wstring> FileDialog::Filters(int ix)
{
    if (ix < 0 || ix >= (int)filters.size())
        throw L"Filter index out of range!";

    return filters[ix];
}

std::wstring FileDialog::FilterString()
{
    std::wstringstream str;
    for (auto it = filters.begin(); it != filters.end(); ++it)
    {
        if (it != filters.begin())
            str << L"|";
        str << (*it).first << L"|" << (*it).second;
    }
    return str.str();
}

void FileDialog::SetFilterString(const std::wstring &str)
{
    filters.clear();

    int len = str.length();
    const wchar_t *c = str.c_str();
    int pos = 0;

    while (pos < len)
    {
        int start = pos;
        while (pos < len && c[pos] != L'|')
            ++pos;
        int start2 = ++pos;
        while (pos < len && c[pos] != L'|')
            ++pos;
        AddFilter(std::wstring(c + start, start2 - 1 - start), start2 < pos ? std::wstring(c + start2, pos - start2) : std::wstring());
        ++pos;
    }
}

void FileDialog::AddFilter(const std::wstring& filtername, const std::wstring& filetypes)
{
    filters.push_back( std::pair<std::wstring, std::wstring>(filtername, filetypes) );
}

void FileDialog::SetFilter(int ix, const std::wstring& filtername, const std::wstring& filetypes)
{
    if (ix < 0 || ix >= (int)filters.size())
        throw L"Filter index out of range!";

    std::pair<std::wstring, std::wstring> &val = filters[ix];
    val.first = filtername;
    val.second = filetypes;
}

void FileDialog::DeleteFilter(int ix)
{
    if (ix < 0 || ix >= (int)filters.size())
        throw L"Filter index out of range!";

    if (filterindex == ix)
        filterindex = 0;
    else if (filterindex > ix)
        filterindex--;
    filters.erase(filters.begin() + ix);
}

void FileDialog::ClearFilters()
{
    filters.clear();
}

int FileDialog::SelectedFilter()
{
    return filterindex;
}

void FileDialog::SetSelectedFilter(int newselectedfilter)
{
    if (newselectedfilter < 0 || newselectedfilter >= (int)filters.size())
        throw L"Filter index out of range!";

    filterindex = newselectedfilter;
}

std::wstring FileDialog::InitialPath()
{
    return initialpath;
}

void FileDialog::SetInitialPath(const std::wstring& newinitialpath)
{
    initialpath = newinitialpath;
}

const std::wstring& FileDialog::FilePath()
{
    return filepath;
}

int FileDialog::FileNameCount()
{
    if (options.contains(fdoMultiselect))
        return filenames.size();
    else
        return filenames.size() != 0 ? 1 : 0;
}

const std::wstring& FileDialog::FileNames(int ix)
{
    if (ix < 0 || ix >= (int)filenames.size())
        throw L"File name index out of range.";

    if (options.contains(fdoMultiselect))
        return filenames[ix];
    else
        return filenames[0];
}

const std::vector<std::wstring>& FileDialog::FileNames()
{
    return filenames;
}

std::wstring FileDialog::FileName()
{
    if (filenames.size() != 0)
        return filenames[0];
    return std::wstring();
}

void FileDialog::SetFileName(const std::wstring &filename)
{
    int pl = PathLength(filename);


    if (pl > 0)
    {
        initialpath = filename.substr(0, pl);
        filenames.clear();
        initialname = filename.substr(pl);
        filenames.push_back(initialname);

    }
    else
    {
        filenames.clear();
        filenames.push_back(filename);

        initialname = filename;
    }
}

std::wstring FileDialog::FullFileName()
{
    if (filenames.size() > 0)
        return filepath + filenames[0];
    else
        return std::wstring();
}

const std::wstring& FileDialog::DefaultExtension()
{
    return defext;
}

void FileDialog::SetDefaultExtension(const std::wstring& newdefaultextension)
{
    defext = newdefaultextension;
}

const std::wstring& FileDialog::Title()
{
    return title;
}

void FileDialog::SetTitle(const std::wstring& newtitle)
{
    title = newtitle;
}

FileDialogOptionSet FileDialog::Options()
{
    return options;
}

void FileDialog::SetOptions(FileDialogOptionSet newoptions)
{
    newoptions -= fdoUseTemplate;
    newoptions -= fdoUseTemplateHandle;
    options = newoptions;
    if (templateinstance != NULL)
        options << (templatename.length() > 0 ? fdoUseTemplate : fdoUseTemplateHandle);

}

FileDialogOptionSetEx FileDialog::OptionsEx()
{
    return optionsex;
}

void FileDialog::SetOptionsEx(FileDialogOptionSetEx newoptions)
{
    optionsex = newoptions;
}

std::pair<HINSTANCE, std::wstring> FileDialog::Template()
{
    return std::pair<HINSTANCE, std::wstring>(templateinstance, templatename);
}

void FileDialog::SetTemplate(HINSTANCE templatemodule, const std::wstring& templateresource)
{
    if (templatemodule == NULL)
    {
        RemoveTemplate();
        return;
    }

    if (templateresource.length() == 0)
    {
        SetTemplate(templatemodule);
        return;
    }

    templateinstance = templatemodule;
    templatename = templateresource;
    options -= fdoUseTemplateHandle;
    options << fdoUseTemplate;
}

void FileDialog::SetTemplate(HINSTANCE templateblock)
{
    templateinstance = templateblock;
    templatename = std::wstring();
    options -= fdoUseTemplate;
    options << fdoUseTemplateHandle;
}

void FileDialog::RemoveTemplate()
{
    templateinstance = NULL;
    templatename = std::wstring();
    options -= fdoUseTemplateHandle;
    options -= fdoUseTemplate;
}


//---------------------------------------------


#ifdef DESIGNING
void OpenDialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->Find<FileDialogOptionSetDesignProperty<FileDialog>>(L"Options")->IncludeInDefault(fdoFileMustExist | fdoPathMustExist);
}
#endif

OpenDialog::OpenDialog()
{
    options << fdoFileMustExist << fdoPathMustExist;
}

BOOL OpenDialog::ShowDialog(OPENFILENAME &info)
{
    return GetOpenFileName(&info);
}


//---------------------------------------------


#ifdef DESIGNING
void SaveDialog::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->Find<FileDialogOptionSetDesignProperty<FileDialog>>(L"Options")->IncludeInDefault(fdoOverwritePrompt | fdoHideReadOnly);
}
#endif

SaveDialog::SaveDialog()
{
    options << fdoOverwritePrompt << fdoHideReadOnly;
}

BOOL SaveDialog::ShowDialog(OPENFILENAME &info)
{
    return GetSaveFileName(&info);
}


//---------------------------------------------


}
/* End of NLIBNS */


#include "stdafx_zoli.h"
#include "resource.h"
#include "application.h"

#include "designer.h"
#include "serializer.h"
#include "designercontrols.h"
#include "designproperties.h"
#include "imagelist.h"
#include "buttons.h"
#include "designerdialogs.h"
#include "designerform.h"
#include "menu.h"
#include "resunzip.h"
#include "inifile.h"
#include "designeroptions.h"
#include "projectsettings.h"
#include "property_generalcontrol.h"
#include "graphiccontrol.h"
#include "utility.h"
#include "themes.h"

#include "designeroptions.h"

#ifdef TOOLBARS_ENABLED
#include "toolbars.h" // TOOLBAR TEST
#endif

//#define TESTING_SPARSE_LIST
#ifdef TESTING_SPARSE_LIST
// TODO: remove
#include "sparse_list.h"
#endif

#define  amClearMod     (WM_APP + 6)


//---------------------------------------------


int main(int argc, wchar_t *argv[])
{
    return NLIBNS::Application::GetInstance()->Run();
}


namespace NLIBNS
{


    Designer *designer = nullptr; // The main form of the designer interface containing the main property editor and control selector.
    Project *project = nullptr;

    Imagelist *ilControls = nullptr; // Images appearing on the control selector.
    Imagelist *ilButtons = nullptr; // Images appearing on buttons of the editor.
    Imagelist *ilAlign = nullptr; // Images in the alignment and size popup menu.

    void UpdateSettings(const Settings &newsettings); // Modifies the current settings.

    //---------------------------------------------


    extern ColorDialog *colordialog;
    extern TransparentColorPickerDialog *transparentcolordialog;
    extern BitmapSelectorDialog *bitmapselectordialog;
    extern ImagelistEditorDialog *imagelisteditordialog;

    void RunTypeInitializers();
    void RunControlInitializers();
}


void Initialize()
{
    using namespace NLIBNS;

    bool val1 = std::is_object<Paintbox>::value;
    bool val2 = std::is_scalar<Paintbox>::value;

    application->SetDialogMode(dmDisableAppWindows);
    //themes->SetThemed(false);

    ilControls = new Imagelist(19, 19, false, icd32bit);
    Bitmap dummy(19, 19);
    dummy.SetPen(clRed, 2.5);
    dummy.SetAntialias(true);
    dummy.Line(0, 0, 18, 18);
    dummy.Line(18, 0, 0, 18);
    ilControls->Add(&dummy);

    Bitmap bmp((HMODULE)NULL, MAKEINTRESOURCE(IDI_PICKERIMAGES));
    ilButtons = new Imagelist(bmp.Height(), bmp.Height(), false, icd32bit);
    ilButtons->Add(&bmp);

    bmp = Bitmap((HMODULE)NULL, MAKEINTRESOURCE(IDI_ALIGNIMAGES));
    ilAlign = new Imagelist(bmp.Height(), bmp.Height(), false, icd32bit);
    ilAlign->Add(&bmp);

    colordialog = new ColorDialog();
    transparentcolordialog = new TransparentColorPickerDialog();
    bitmapselectordialog = new BitmapSelectorDialog();
    imagelisteditordialog = new ImagelistEditorDialog();

    // Calling type and property creation before creating the designer. The designer should be null before these run.
    RunTypeInitializers();
    RunControlInitializers(); 

    designer = new Designer();
    designer->Initialize();
}


    //---------------------------------------------

namespace NLIBNS
{

    void Project::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);

        serializer->HideProperty(L"Name");
        serializer->HideProperty(L"AccessLevel");

        serializer->Add(std::wstring(), new StringDesignProperty<Project>(L"SourcePath", std::wstring(), &Project::SourcePath, &Project::SetSourcePath))->SetDefault(std::wstring());
        serializer->Add(std::wstring(), new StringDesignProperty<Project>(L"ResourcePath", std::wstring(), &Project::ResourcePath, &Project::SetResourcePath))->SetDefault(std::wstring());
        serializer->Add(std::wstring(), new StringDesignProperty<Project>(L"ProjectName", std::wstring(), &Project::Name, &Project::SetName))->SetDefault(std::wstring());
        serializer->Add(std::wstring(), new StringDesignProperty<Project>(L"CppExtension", std::wstring(), &Project::CppExt, &Project::SetCppExt))->SetDefault(L"cpp");
        serializer->Add(std::wstring(), new StringDesignProperty<Project>(L"HeaderExtension", std::wstring(), &Project::HeaderExt, &Project::SetHeaderExt))->SetDefault(L"h");
        serializer->Add(std::wstring(), new IconDesignProperty<Project>(L"AppIcon", std::wstring(), &Project::AppIcon, &Project::DesignSetAppIcon, NULL));

    }

    void Project::DesignSetAppIcon(IconData *data)
    {
        SetAppIcon(data);
    }

    Project::Project() : base(), constructing(true), modified(false), cppext(L"cpp"), hext(L"h")
    {
        project = this;

        try
        {
            NewForm();
            PostMessage(designer->Handle(), amClearMod, 0, 0);

            constructing = false;
        }
        catch(...)
        {
            project = nullptr;
            throw;
        }
    }

    Project::Project(const std::wstring& fname) : base(), constructing(true), modified(false), cppext(L"cpp"), hext(L"h")
    {
        project = this;

        try
        {
            FileStream ws(fname, std::ios_base::in);
            if (!ws)
                throw L"Couldn't open project file.";

            if (!ws.skipbom())
            {
                if (!ws.good())
                    throw L"Error reading in project file.";
                else
                    throw L"The project file must be in UTF-8 format with BOM.";
            }

            TokenStream token(ws);
            ReadTokenType rtt;

            DesignFormBase *form = NULL;
            try
            {
                if (!token.read() || token != rttUnquoted || token != L"project")
                    throw TokenE(L"Project line expected but not found", token);
                if (!token.read() || token == L"" || !ValidFileName(token.toString(), false))
                    throw TokenE(L"Invalid project name", token);

                std::vector<ControlDeserializerItem*> props; // Gets the properties of the project.
                DeserializeFromStream(NULL, token, props, L"Project");
                for (auto it = props.begin(); it != props.end(); ++it)
                {
                    for (auto pit = (*it)->properties.begin(); pit != (*it)->properties.end(); ++pit)
                        (*pit)->SetValue(this);
                }
                FreeDeserializerList(props);

                if ((token.peek(rtt) != L"type" || rtt != rttUnquoted) && (token.peek(rtt) != L"}" || rtt != rttToken))
                    throw TokenE(L"Expected form type or \"}\"", token);

                // Find all forms till the end;
                while (token.peek(rtt) == L"type" && rtt == rttUnquoted)
                {
                    token.read();  // Skip "type".
                    if (token.read() && rtt == rttUnquoted)
                    {
                        if (token == L"Form")
                        {
                            form = new DesignForm(L"", designer, designer->bpControls);
                            if (!form)
                                throw ECantAllocate(L"Couldn't allocate memory for form.");
                        }
                        else if (token == L"Container")
                        {
                            form = new DesignContainerForm(L"", designer, designer->bpControls);
                            if (!form)
                                throw ECantAllocate(L"Couldn't allocate memory for form.");
                        }
                        else
                            throw TokenE(L"Unexpected type", token);
                    }
                    else
                        throw TokenE(L"Expected container type name not found", token);

                    forms.push_back(FormDataItem(form, -1));
                    form->ReadFromStream(token);
                    form = nullptr;
                }

                FixFormCreationOrder();

                if (!token.read() || token != L"}")
                    throw TokenE(L"Excpected \"}\"", token);
            }
            catch(TokenE& e)
            {
                for (auto &formdata : forms)
                    formdata.first->Destroy();
                forms.clear();
                form = nullptr;
                throw e.what();
            }
            catch(ECantAllocate&)
            {
                ;
            }
            catch(...)
            {
                if (form)
                    form->Destroy();
                throw;
            }

            ws.close();

            for (auto &formdata : forms)
                if (formdata.first->IsDesignVisible())
                    formdata.first->Show();

            path = fname;
            PostMessage(designer->Handle(), amClearMod, 0, 0);;

            constructing = false;
        }
        catch(...)
        {
            project = nullptr;
            throw;
        }
    }

    Project::~Project()
    {
        project = nullptr;
        for (auto &formdata : forms)
            formdata.first->Destroy();
    }

    void Project::Destroy()
    {
        designer->SetProperties(NULL);
        base::Destroy();
    }

    std::wstring Project::ResolvedSourcePath()
    {
        std::wstring result = srcpath + L"\\";
        if (result.length() < 2 || (result.find(L"\\\\") != 0 && result[1] != L':'))
            result = AppendToPath(GetFilePath(path), result);
        return result;
    }

    std::wstring Project::ResolvedResourcePath()
    {
        std::wstring result = respath + L"\\";
        if (result.length() < 2 || (result.find(L"\\\\") != 0 && result[1] != L':'))
            result = AppendToPath(GetFilePath(path), result);
        return result;
    }

    const std::wstring& Project::Path()
    {
        return path;
    }

    void Project::ClearModified()
    {
        for (auto &formdata : forms)
            formdata.first->Modify(false);
        modified = false;
        designer->UpdateCaption();
    }

    bool Project::Save(std::wstring fname)
    {
        FileStream ws(fname, std::ios_base::out | std::ios_base::in | std::ios_base::trunc);
        if (!ws)
            return false;
        ws.writebom();

        Indentation indent(false, 4);
        ws << indent << L"project \"" << GetFileName(fname) << L"\"" << std::endl;
        ws << indent << L"{" << std::endl;
        ++indent;

        DesignSerializer *serializer = Serializer();
        serializer->Serialize(indent, ws, this);

        for (auto &formdata : forms)
            formdata.first->WriteToStream(indent, ws);
        --indent;
        ws << indent << L"}" << std::endl;
        ws.close();

        path = fname;
        ClearModified();

        return true;
    }

    bool Project::Modified()
    {
        return modified;
    }

    void Project::Modify()
    {
        if (modified)
            return;
        modified = true;
        designer->UpdateCaption();
    }

    const std::wstring& Project::Name()
    {
        return name;
    }

    const std::wstring& Project::SourcePath()
    {
        return srcpath;
    }

    const std::wstring& Project::ResourcePath()
    {
        return respath;
    }

    const std::wstring& Project::CppExt()
    {
        return cppext;
    }

    const std::wstring& Project::HeaderExt()
    {
        return hext;
    }

    void Project::SetName(const std::wstring &newname)
    {
        if (name == newname)
            return;
        if (!ValidFileName(newname, false))
        {
            ShowMessageBox(L"Name of the project can't include the path separator and characters not valid in windows file names.", L"Error", mbOk);
            return;
        }
        name = newname;
        Modify();
    }

    void Project::SetSourcePath(const std::wstring &newpath)
    {
        if (srcpath == newpath)
            return;
        srcpath = newpath;
        Modify();
    }

    void Project::SetResourcePath(const std::wstring &newpath)
    {
        if (respath == newpath)
            return;
        respath = newpath;
        Modify();
    }

    void Project::SetCppExt(const std::wstring &newext)
    {
        if (cppext == newext)
            return;
        cppext = newext;
        Modify();
    }

    void Project::SetHeaderExt(const std::wstring &newext)
    {
        hext = newext;
        Modify();
    }

    int Project::FormCount()
    {
        return forms.size();
    }

    DesignFormBase* Project::Forms(unsigned int ix)
    {
        if (ix >= forms.size())
            throw L"Index for form out of range.";

        for (auto &formdata : forms)
            if (formdata.second == ix)
                return formdata.first;

        throw L"Form creation order is not fixed.";
    }

    DesignFormBase* Project::FindForm(const std::wstring &formname)
    {
        for (auto formdata : forms)
            if (formdata.first->Name() == formname)
                return formdata.first;
        return nullptr;
    }

    DesignFormBase* Project::NewForm()
    {
        std::wstring fname = designer->UnusedFormName(L"Form");
        DesignFormBase *form = new DesignForm(fname, designer, designer->bpControls);
        forms.push_back(FormDataItem(form, forms.size()));
        form->SetShowPosition(fspParentMonitorCenter);
        form->Show();
        form->SetShowPosition(fspUnchanged);

        Modify();

        return form;
    }

    DesignFormBase* Project::NewContainer()
    {
        std::wstring fname = designer->UnusedFormName(L"Container");
        DesignFormBase *form = new DesignContainerForm(fname, designer, designer->bpControls);
        forms.push_back(FormDataItem(form, forms.size()));
        form->SetShowPosition(fspParentMonitorCenter);
        form->Show();
        form->SetShowPosition(fspUnchanged);

        Modify();

        return form;
    }

    IconData* Project::AppIcon()
    {
        return &icon;
    }

    void Project::SetAppIcon(IconData *data)
    {
        if (data)
            icon = *data;
        else
            icon = IconData();
        Modify();
    }

    void Project::SetAppIcon(IconData &&data)
    {
        icon = std::move(data);
        Modify();
    }

    int Project::FormCreationOrder(DesignFormBase *form)
    {
        for (auto &formdata : forms)
            if (formdata.first == form)
                return formdata.second;
        return -1;
    }

    void Project::SetFormCreationOrder(DesignFormBase *form, unsigned int neworder)
    {
        if (!constructing && neworder >= forms.size())
            neworder = forms.size() - 1;

        FormDataItem *data = nullptr;
        for (auto &formdata : forms)
        {
            if (formdata.first == form)
            {
                data = &formdata;
                break;
            }
        }

        if (!constructing) // The project is null during construction, which means we are in the middle of reading forms from stream. Their creation order will be fixed later.
        {
            for (auto &formdata : forms)
            {
                if (formdata.first == form)
                    continue;
                if (formdata.second >= neworder && formdata.second < data->second)
                    ++formdata.second;
                else if (formdata.second <= neworder && formdata.second > data->second)
                    --formdata.second;
            }
        }

        data->second = neworder;

        if (!constructing) // Order of menu items in the designer should not be updated while loading the project.
            designer->FillUp();
    }

    void Project::FixFormCreationOrder()
    {
        if (!constructing)
            throw L"The project can only fix its creation order during construction.";

        std::multimap<unsigned int, DesignFormBase*> order;
        for (auto &formdata : forms)
            order.insert(std::pair<unsigned int, DesignFormBase*>(formdata.second, formdata.first));

        int ix = 0;
        for (auto &orderdata : order)
            SetFormCreationOrder(orderdata.second, ix++);
    }

    void Project::ReorderForms(std::vector<std::wstring> &order)
    {
        if (order.size() != forms.size())
            throw L"New order list must contain all forms in the project.";

        for (int ix = 0; ix < (int)order.size(); ++ix)
        {
            bool changed = false;
            for (auto &formdata : forms)
            {
                if (formdata.first->Name() == order[ix])
                {
                    formdata.second = ix;
                    changed = true;
                    break;
                }
            }
            if (!changed)
                throw L"Invalid form name in ordered forms list.";
        }

        Modify();
    }

    void Project::FormDelete(unsigned int ix)
    {
        if (ix >= forms.size())
            return;

        for (auto it = forms.begin(); it != forms.end(); ++it)
        {
            auto formdata = *it;
            if (formdata.second == ix)
            {
                formdata.first->Destroy();
                it = forms.erase(it);
                formdata = *it;
                //continue;  Skip this because formdata is the next one already.
            }

            if (formdata.second > ix)
                --formdata.second;
        }

        Modify();
    }

    void Project::FormReferenced(DesignFormBase *main, DesignFormBase *guest)
    {
        if (main == guest)
            throw L"Referencing a form from itself is invalid.";

        auto it = forms.begin();
        while (it != forms.end() && it->first != main && it->first != guest)
            ++it;

        if (it == forms.end())
            throw L"Couldn't find neither the main nor the guest form in the forms list when trying to make a guest reference.";

        if (it->first == main) // Main coming before guest means that it is already correctly ordered, as the guest form has to be created later.
            return;

        auto it2 = std::next(it);
        while (it2 != forms.end() && it2->first != main)
            ++it2;

        if (it == forms.end())
            throw L"Couldn't find the main form in the forms list when trying to make a guest reference.";

        auto val = *it2;
        forms.erase(it2);
        forms.insert(it, val);

        ++it;
        while (it != forms.end())
        {
            DesignFormBase *frm = it->first;
            ++it;

            if (frm->GuestReferenceOn(val.first))
                FormReferenced(frm, val.first);
        }

        Modify();
    }


    //---------------------------------------------

    int ControlCategoryCount();
    const std::wstring& GetControlCategory(int index);
    int RegisteredControlCount();
    const type_info& GetRegisteredControlType(int index);

    Designer::Designer() : serializedform(NULL), editedform(NULL), changing(0)
    {
        mainmenu = new Menubar();
        SetMenu(mainmenu);
        IconFromResource(NULL, MAKEINTRESOURCE(IDI_MAINICON));

        miFile = mainmenu->Add(L"&File");
        miNewProj = miFile->Add(L"&New Project");
        miNewProj->SetShortcut(L"Ctrl+N");
        miNewProj->OnClick = CreateEvent(this, &Designer::minewprojclick);
        miFile->AddSeparator();
        miOpenProj = miFile->Add(L"&Open Project...");
        miOpenProj->SetShortcut(L"Ctrl+O");
        miOpenProj->OnClick = CreateEvent(this, &Designer::miopenprojclick);
        miSaveProj = miFile->Add(L"&Save Project");
        miSaveProj->SetShortcut(L"Ctrl+S");
        miSaveProj->SetEnabled(false);
        miSaveProj->OnClick = CreateEvent(this, &Designer::misaveprojclick);
        miSaveProjAs = miFile->Add(L"Save Project &As...");
        miSaveProjAs->SetShortcut(L"Ctrl+A");
        miSaveProjAs->OnClick = CreateEvent(this, &Designer::misaveprojasclick);
        miFile->AddSeparator();
        miSaveSrc = miFile->Add(L"Save Sour&ce Files...");
        miSaveSrc->SetShortcut(L"Ctrl+E");
        miSaveSrc->OnClick = CreateEvent(this, &Designer::misavesrcclick);
        miFile->AddSeparator();
        miCreateFiles = miFile->Add(L"Create Project Files...");
        miCreateFiles->OnClick = CreateEvent(this, &Designer::micreatefilesclick);
        miFile->AddSeparator();
        miRecents = miFile->Add(L"&Recent Projects");
        miRecents->SetEnabled(false);
        miFile->AddSeparator();
        miExit = miFile->Add(L"E&xit");
        miExit->SetShortcut(L"Alt+F4");
        miExit->OnClick = CreateEvent(this, &Designer::miexitclick);
        mainmenu->SetParent(this);

        miEdit = mainmenu->Add(L"&Edit");
        miEPropSwitch = miEdit->Add(L"Focus &Properties/Form");
        miEPropSwitch->SetShortcut(L"F11");
        miEPropSwitch->OnClick = CreateEvent(this, &Designer::miepropswitchclick);
        miEEventSwitch = miEdit->Add(L"Focus &Events/Form");
        miEEventSwitch->SetShortcut(L"F12");
        miEEventSwitch->OnClick = CreateEvent(this, &Designer::mieeventswitchclick);

        miView = mainmenu->Add(L"&View");
        miVSort = miView->Add(L"Property Sort");

        miVSortAlpha = miVSort->Add(L"Alphabetically");
        miVSortAlpha->SetShortcutText(L"F9");
        miVSortAlpha->SetTag(0);
        miVSortAlpha->SetChecked(true);
        miVSortAlpha->SetGrouped(true);
        miVSortAlpha->SetAutoCheck(true);
        miVSortAlpha->OnClick = CreateEvent(this, &Designer::sortdownchangedclick);
        miVSortCat = miVSort->Add(L"By Category");
        miVSortCat->SetShortcutText(L"F9");
        miVSortCat->SetShortcut(L"F9");
        miVSortCat->SetTag(1);
        miVSortCat->SetGrouped(true);
        miVSortCat->SetAutoCheck(true);
        miVSortCat->OnClick = CreateEvent(this, &Designer::sortdownchangedclick);
        miView->AddSeparator();
        miVTab = miView->Add(L"Edited Tabs");
        miVProps = miVTab->Add(L"Properties");
        miVProps->SetTag(0);
        miVProps->SetChecked(true);
        miVProps->SetGrouped(true);
        miVProps->SetShortcut(L"F5");
        miVProps->OnClick = CreateEvent(this, &Designer::mivchoiceclick);
        miVEvents = miVTab->Add(L"Events");
        miVEvents->SetGrouped(true);
        miVEvents->SetShortcut(L"F6");
        miVEvents->SetTag(1);
        miVEvents->OnClick = CreateEvent(this, &Designer::mivchoiceclick);
        miVControls = miVTab->Add(L"Controls");
        miVControls->SetGrouped(true);
        miVControls->SetShortcut(L"F7");
        miVControls->SetTag(2);
        miVControls->OnClick = CreateEvent(this, &Designer::mivchoiceclick);
        miView->AddSeparator();
        miVLabels = miView->Add(L"Control Button Labels");
        miVLabels->SetChecked(true);
        miVLabels->SetAutoCheck(true);
        miVLabels->OnClick = CreateEvent(this, &Designer::mivlabelsclick);

        miTools = mainmenu->Add(L"&Tools");
        miProjSettings = miTools->Add(L"&Project Settings...\tCtrl+R");
        miProjSettings->OnClick = CreateEvent(this, &Designer::miprojsettingsclick);
        miOptions = miTools->Add(L"&Options...");
        miOptions->SetShortcut(L"Ctrl+I");
        miOptions->OnClick = CreateEvent(this, &Designer::mioptionsclick);

        miForms = mainmenu->Add(L"For&ms");
        miFormsAdd = miForms->Add(L"&Add");
        miForms->AddSeparator();
        miAddForm = miFormsAdd->Add(L"New &Form");
        miAddForm->OnClick = CreateEvent(this, &Designer::miaddformclick);
        miAddContainer = miFormsAdd->Add(L"New &Container");
        miAddContainer->OnClick = CreateEvent(this, &Designer::miaddcontainerclick);

        Rect wa = screen->MonitorFromPoint(screencursor->Pos())->WorkArea();
        SetClientWidth(400 * Scaling);
        SetClientHeight(550 * Scaling);
        SetMinimumWidth(200 * Scaling);
        Rect wr = WindowRect();
        SetBounds(RectS(wa.left + wa.Width() - wr.Width() - 10 * Scaling, wa.top + (wa.Height() - wr.Height()) / 2, wr.Width(), wr.Height()));
        //SetMinimumHeight(63);

        SetText(L"Toolbox");

#ifdef TOOLBARS_ENABLED
        tbTest = new Toolbar();
        tbTest->SetParent(this);
        tbTest->SetKind(tbkTextOnly);
        tbTest->SetHeight(48);
        tbTest->AddButton(L"1", 24, 0, tbbcsUnchecked, tbctButton, tbbsShowText, true, true, false);
        tbTest->AddButton(L"2", 24, 0, tbbcsUnchecked, tbctDropDownButton, tbbsShowText, true, true, false);
        tbTest->AddButton(L"3", 24, 0, tbbcsUnchecked, tbctButton, tbbsShowText, true, true, false);
        tbTest->AddButton(L"4", 24, 0, tbbcsUnchecked, tbctDropDown, tbbsShowText, true, true, false);
        tbTest->AddButton(L"5", 24, 0, tbbcsUnchecked, tbctButton, tbbsShowText, true, true, false);
        tbTest->SetButtonHeight(12);
        tbTest->SetDoubleBuffered(true);
#endif

        pages = new PageControl();
        pages->SetAlignment(alClient);
        tpProperties = pages->AddTabPage(L"Properties");
        tpEvents = pages->AddTabPage(L"Events");
        tpControls = pages->AddTabPage(L"Controls");
        pages->OnTabChange = CreateEvent(this, &Designer::pagestabchange);
        pages->SetParent(this);

        bpControls = new ButtonPanel();
        bpControls->SetImages(ilControls);
        bpControls->SetAlignment(alClient);
        bpControls->SetParent(tpControls);

        for (int ix = 0; ix < ControlCategoryCount(); ++ix)
            bpControls->AddContainer(GetControlCategory(ix));

        for (int ix = 0; ix < RegisteredControlCount(); ++ix)
        {
            const type_info &type = GetRegisteredControlType(ix);
            bpControls->AddButton(type);
        }
        //ButtonContainer* cgen = bpControls->AddContainer(L"General controls");
        //ButtonContainer* csys = bpControls->AddContainer(L"System controls");
        //ButtonContainer* cnon = bpControls->AddContainer(L"Non-visual controls");

        //cgen->AddButton(typeid(Panel));
        //cgen->AddButton(typeid(Paintbox));
        //cgen->AddButton(typeid(FlatButton));
        //cgen->AddButton(typeid(Label));
        //cgen->AddButton(typeid(Bevel));
        //cgen->AddButton(typeid(StringGrid));

        //csys->AddButton(typeid(Button));
        //csys->AddButton(typeid(Checkbox));
        //csys->AddButton(typeid(Radiobox));
        //csys->AddButton(typeid(Groupbox));
        //csys->AddButton(typeid(Edit));
        //csys->AddButton(typeid(Memo));
        //csys->AddButton(typeid(UpDown));
        //csys->AddButton(typeid(Progressbar));
        //csys->AddButton(typeid(Listbox));
        //csys->AddButton(typeid(Combobox));
        //csys->AddButton(typeid(TabControl));
        //csys->AddButton(typeid(PageControl));
        //csys->AddButton(typeid(Listview));

        //cnon->AddButton(typeid(Imagelist));
        //cnon->AddButton(typeid(ColorDialog));
        //cnon->AddButton(typeid(FontDialog));
        //cnon->AddButton(typeid(OpenDialog));
        //cnon->AddButton(typeid(SaveDialog));
        //cnon->AddButton(typeid(FolderDialog));
        //cnon->AddButton(typeid(Menubar));
        //cnon->AddButton(typeid(PopupMenu));
        //cnon->AddButton(typeid(Timer));

        // The property editor list
        pPropTop = new Panel();
        pPropTop->SetBounds(RectS(0, 0, 1, 28 * Scaling));
        pPropTop->SetAlignment(alTop);
        pPropTop->SetText(L"");
        pPropTop->SetInnerBorderStyle(pbsNone);
        pPropTop->SetParent(tpProperties);
        pPropTop->SetParentBackground(false);

        btnPropAbc = new FlatButton();
        btnPropAbc->SetBounds(RectS(2 * Scaling, 2 * Scaling, 23 * Scaling, 23 * Scaling));
        btnPropAbc->SetDoubleBuffered(true);
        //btnPropAbc->SetText(L"Abc");
        btnPropAbc->SetType(fbtRadiobutton);
        btnPropAbc->SetGroupIndex(1);
        btnPropAbc->SetDown(true);
        btnPropAbc->OnDownChanged = CreateEvent(this, &Designer::sortdownchangedclick);
        btnPropAbc->SetTag(0);
        btnPropAbc->SetImagePosition(bipCenter);
        btnPropAbc->Image()->SetImages(ilButtons);
        btnPropAbc->Image()->SetImageIndex(7);
        btnPropAbc->SetTooltipText(L"Sort alphabetically");
        btnPropAbc->SetParent(pPropTop);

        btnPropCat = new FlatButton();
        btnPropCat->SetBounds(RectS(int(2 * Scaling) * 2 + int(23 * Scaling), 2 * Scaling, 23 * Scaling, 23 * Scaling));
        btnPropCat->SetDoubleBuffered(true);
        //btnPropCat->SetText(L"Cat");
        btnPropCat->SetType(fbtRadiobutton);
        btnPropCat->SetGroupIndex(1);
        btnPropCat->OnDownChanged = CreateEvent(this, &Designer::sortdownchangedclick);
        btnPropCat->SetTag(1);
        btnPropCat->SetImagePosition(bipCenter);
        btnPropCat->Image()->SetImages(ilButtons);
        btnPropCat->Image()->SetImageIndex(8);
        btnPropCat->SetTooltipText(L"Sort by category");
        btnPropCat->SetParent(pPropTop);

        cbControlList = new Combobox();
        cbControlList->SetBounds(RectS(int(2 * Scaling) * 3 + 2 * int(23 * Scaling), 2 * Scaling, 1, cbControlList->Height() ));
        cbControlList->SetWidth(pPropTop->Width() - cbControlList->Left() - int(2 * Scaling));
        cbControlList->SetAnchors(caLeft | caTop | caRight);
        cbControlList->SetType(ctDropdown);
        cbControlList->SetWantedKeyTypes(cbControlList->WantedKeyTypes() | wkEnter);
        cbControlList->SetParent(pPropTop);
        cbControlList->OnChanged = CreateEvent(this, &Designer::cbcontrollistchange);
        cbControlList->OnKeyPress = CreateEvent(this, &Designer::cbcontrollistkeypress);
        cbControlList->OnLoseFocus = CreateEvent(this, &Designer::cbcontrollistlosefocus);

        lbProp = new PropertyListbox(pltValues, btnPropAbc, btnPropCat);
        lbProp->SetAlignment(alClient);
        lbProp->SetBorderStyle(bsNone);
        lbProp->SetParent(tpProperties);

        lbEvents = new PropertyListbox(pltEvents, btnPropAbc, btnPropCat);
        lbEvents->SetAlignment(alClient);
        lbEvents->SetBorderStyle(bsNone);
        lbEvents->SetParent(tpEvents);

        SetWidth(Width() + 1);

        dlgOpenProj = new OpenDialog();
        dlgSaveProj = new SaveDialog();
        dlgSave = new SaveDialog();

        dlgOpenProj->AddFilter(L"Project file (*.aprj)", L"*.aprj");
        dlgOpenProj->AddFilter(L"All files (*.*)", L"*.*");
        dlgSaveProj->AddFilter(L"Project file (*.aprj)", L"*.aprj");
        dlgSaveProj->AddFilter(L"All files (*.*)", L"*.*");
        dlgSaveProj->SetDefaultExtension(L"aprj");
        dlgSave->AddFilter(L"C++ file and header (*.cpp, *.h)", L"*.cpp;*.h");
        dlgSave->AddFilter(L"All files (*.*)", L"*.*");
        dlgSave->SetDefaultExtension(L"cpp");

        OnClose = CreateEvent(this, &Designer::formclose);

        application->OnSelectActiveForm = CreateEvent(this, &Designer::selectactiveform);
    }

    Designer::~Designer()
    {
    }

    void Designer::Destroy()
    {
        Settings settings = GetSettings();

        IniFile ini(ExecutablePath + L"settings.ini");
        ini.SetBool(L"Designer", L"Control labels", miVLabels->Checked());
        ini.SetBool(L"Designer", L"Property alphabetic sorting", btnPropAbc->Down());

        ini.SetString(L"Designer", L"lastproj", settings.designer_lastproj);

        ini.SetBool(L"Designer", L"loadlast", settings.designer_loadlast);
        ini.SetBool(L"Designer", L"ontop", settings.designer_ontop);
        ini.SetBool(L"Designer", L"savepos", settings.designer_savepos);

        ini.SetBool(L"Code", L"publictop", settings.code_publictop);
        ini.SetBool(L"Code", L"tabindent", settings.code_tabindent);
        ini.SetInt(L"Code", L"indentsize", settings.code_indentsize);

        ini.SetString(L"Paths", L"library", settings.paths_library);
        ini.SetString(L"Paths", L"headers", settings.paths_headers);

        ini.SetInt(L"Control Editing", L"default access", settings.control_access);

        if (settings.designer_savepos)
            ini.SetRect(L"Designer", L"rect", WindowRect());
        else
            ini.DeleteKey(L"Designer", L"rect");

        ini.DeleteSection(L"Recent Projects");
        int num = 1;
        std::for_each(recents.begin(), recents.end(), [&ini, &num](const std::wstring &fname) {
            ini.SetString(L"Recent Projects", L"project" + IntToStr(num++), fname);
        });

        for (int ix = 0; ix < RegisteredControlCount(); ++ix)
        {
            const type_info &type = GetRegisteredControlType(ix);
            if (!SerializerByTypeInfo(type)->UserDefaultProperty().empty())
                ini.SetString(L"Default Properties", DisplayNameByTypeInfo(type, true), SerializerByTypeInfo(type)->UserDefaultProperty());
            else
                ini.DeleteKey(L"Default Properties", DisplayNameByTypeInfo(type, true));
        }
        if (!SerializerByTypeInfo(typeid(DesignForm))->UserDefaultProperty().empty())
            ini.SetString(L"Default Properties", DisplayNameByTypeInfo(typeid(DesignForm), true), SerializerByTypeInfo(typeid(DesignForm))->UserDefaultProperty());
        else
            ini.DeleteKey(L"Default Properties", DisplayNameByTypeInfo(typeid(DesignForm), true));
        if (!SerializerByTypeInfo(typeid(DesignContainerForm))->UserDefaultProperty().empty())
            ini.SetString(L"Default Properties", DisplayNameByTypeInfo(typeid(DesignContainerForm), true), SerializerByTypeInfo(typeid(DesignContainerForm))->UserDefaultProperty());
        else
            ini.DeleteKey(L"Default Properties", DisplayNameByTypeInfo(typeid(DesignContainerForm), true));

        ini.Flush();

        dlgOpenProj->Destroy();
        dlgSaveProj->Destroy();
        dlgSave->Destroy();

        if (project)
            project->Destroy();

        base::Destroy();
    }

    void Designer::Initialize()
    {
        IniFile ini(ExecutablePath + L"settings.ini");
        miVLabels->SetChecked(ini.GetBool(L"Designer", L"Control labels", true));
        btnPropCat->SetDown(!ini.GetBool(L"Designer", L"Property alphabetic sorting", true));

        mivlabelsclick(miVLabels, EventParameters());

        Settings settings = GetSettings();

        settings.designer_lastproj = ini.GetString(L"Designer", L"lastproj", std::wstring());

        settings.designer_loadlast = ini.GetBool(L"Designer", L"loadlast", true);
        settings.designer_ontop = ini.GetBool(L"Designer", L"ontop", false);
        SetTopmost(settings.designer_ontop);
        settings.designer_savepos = ini.GetBool(L"Designer", L"savepos", true);

        settings.code_publictop = ini.GetBool(L"Code", L"publictop", true);
        settings.code_tabindent = ini.GetBool(L"Code", L"tabindent", true);
        settings.code_indentsize = ini.GetInt(L"Code", L"indentsize", 4);

        settings.paths_library = ini.GetString(L"Paths", L"library", std::wstring());
        settings.paths_headers = ini.GetString(L"Paths", L"headers", std::wstring());

        settings.control_access = FixEnumValue((AccessLevels)ini.GetInt(L"Control Editing", L"default access", 0), alPublic, alPrivate, alPublic);

        if (!settings.designer_loadlast && !settings.designer_lastproj.empty())
        {
            recents.push_front(settings.designer_lastproj);
            settings.designer_lastproj = std::wstring();
        }

        if (settings.designer_savepos)
        {
            Rect posrect = ini.GetRect(L"Designer", L"rect", Rect());
            if (!posrect.Empty())
            {
                DisplayMonitor *m = screen->MonitorFromRect(posrect);
                Rect r = m->WorkArea().Intersect(posrect);
                int cy = GetSystemMetrics(SM_CYCAPTION);
                if (r.Width() > cy && r.Height() > cy)
                    SetBounds(posrect);
            }
        }

        std::wstring fname;
        int num = 1;
        while (!(fname = ini.GetString(L"Recent Projects", L"project" + IntToStr(num++), std::wstring())).empty())
            recents.push_back(fname);
        if (!recents.empty())
        {
            miRecents->SetEnabled(true);
            std::for_each(recents.begin(), recents.end(), [this](const std::wstring &fname) {
                MenuItem *mi = miRecents->Add(fname);
                mi->OnClick = CreateEvent(this, &Designer::mirecentclick);
            });
        }

        for (int ix = 0; ix < RegisteredControlCount(); ++ix)
        {
            const type_info &type = GetRegisteredControlType(ix);
            std::wstring defprop;
            ini.GetString(L"Default Properties", DisplayNameByTypeInfo(type, true), defprop, std::wstring());
            if (!defprop.empty())
                SerializerByTypeInfo(type)->MakeDefault(defprop);
        }
        std::wstring defprop;
        ini.GetString(L"Default Properties", DisplayNameByTypeInfo(typeid(DesignForm), true), defprop, std::wstring());
        if (!defprop.empty())
            SerializerByTypeInfo(typeid(DesignForm))->MakeDefault(defprop);
        ini.GetString(L"Default Properties", DisplayNameByTypeInfo(typeid(DesignContainerForm), true), defprop, std::wstring());
        if (!defprop.empty())
            SerializerByTypeInfo(typeid(DesignContainerForm))->MakeDefault(defprop);

        bool loaded = false;
        project = nullptr;

        Show();

        miFormsCnt = miForms->Count();

        try
        {
            if (settings.designer_loadlast && !settings.designer_lastproj.empty())
            {
                OpenProject(settings.designer_lastproj);
                loaded = true;
            }
        }
        catch(...)
        {
            if (project)
                project->Destroy();
            loaded = false;
            settings.designer_lastproj = std::wstring();
        }

        UpdateSettings(settings);

        if (!loaded)
            new Project();

        FillUp();
    }

    void Designer::FinalizeEdits()
    {
        lbProp->FinalizeEdit(true);
        lbEvents->FinalizeEdit(true);
    }

    void Designer::UpdateCaption()
    {
        if (!project || project->Path().empty())
            SetText(std::wstring(L"Toolbox") + (project && project->Modified() ? L" *" : L""));
        else
            SetText(std::wstring(L"Toolbox - ") + (project && project->Modified() ? L"*" : L"") + GetFileName(project->Path()));
    }

    void Designer::Modify()
    {
        if (!project)
            return;
        project->Modify();
    }

    void Designer::FillUp()
    {
        CleanUp();

        for (int ix = 0; ix < project->FormCount(); ++ix)
        {
            DesignFormBase *form = project->Forms(ix);
            CreateFormMenu(form);
            AddToNotifyList(form, nrOwnership);
        }
    }

    void Designer::CleanUp()
    {
        while (miForms->Count() != miFormsCnt)
            miForms->Delete(miFormsCnt);
        formmenus.clear();
    }

    void Designer::ReplaceDesignerBoxes(PropertyListbox *newprop, PropertyListbox *newevents)
    {
        editors.push_back(std::pair<PropertyListbox*, PropertyListbox*>(lbProp, lbEvents));
        lbProp = newprop;
        lbEvents = newevents;
    }

    void Designer::RestoreDesignerBoxes()
    {
        if (editors.empty())
            throw L"Restoring designer boxes when the listbox list is empty!";

        lbProp = editors.back().first;
        lbEvents = editors.back().second;
        editors.pop_back();
    }

    bool Designer::MainPropertyOwner(Object *searchowner)
    {
        return lbProp->MainPropertyOwner(searchowner);
    }

    bool Designer::IsPropertyOwner(Object *searchowner)
    {
        return lbProp->IsPropertyOwner(searchowner);
    }

    void Designer::InvalidateRow(Object *propholder, const std::wstring& propertyname)
    {
        lbProp->InvalidateRow(propholder, propertyname);
    }

    void Designer::InvalidateEventRows(const std::wstring& eventfuncname)
    {
        lbEvents->InvalidateEventRows(eventfuncname);
    }

    void Designer::EditEvent()
    {
        pages->SetActivePageIndex(1);
        lbEvents->EditProperty();
    }

    void Designer::EditProperty()
    {
        pages->SetActivePageIndex(0);
        lbProp->EditProperty();
    }

    void Designer::EditProperty(const std::wstring& propertyname, bool activateeditor)
    {
        pages->SetActivePageIndex(0);
        lbProp->EditProperty(propertyname, activateeditor);
    }

    void Designer::SetActiveForm(DesignFormBase *form)
    {
        if (form == editedform)
            return;

        //HWND currentactive = GetActiveWindow();

        changing = 0;

        if (editedform)
            editedform->DeactivateSelection();
        editedform = form;

        if (editedform)
        {
            editedform->ActivateSelection();
            std::wstring formname = ShortNamespaceName(editedform->ClassName(true)) + L"* " + editedform->Name();

            std::vector<std::pair<std::wstring, Object*>> objects;
            editedform->CollectObjects(objects);
            std::vector<std::wstring> list;
            list.push_back(formname);

            for (auto &obj : objects)
                list.push_back(ShortNamespaceName(obj.second->ClassName(true)) + L"* " + obj.first);

            std::sort(list.begin() + 1, list.end(), [](const std::wstring &str1, const std::wstring &str2) { return str1 < str2; });
            cbControlList->SetLines(list);
        }

        UpdateControlList();
    }

    void Designer::UpdateControlList()
    {
        if (!editedform)
        {
            cbControlList->Clear();
            cbControlList->SetText(L"[Empty]");
        }
        else
        {
            std::wstring formname = ShortNamespaceName(editedform->ClassName(true)) + L"* " + editedform->Name();

            std::pair<std::wstring, Object*> sel = editedform->SelectedObject();
            if (sel.second)
                cbControlList->SetText(ShortNamespaceName(sel.second->ClassName(true)) + L"* " + sel.first);
            else
            {
                if (sel.first != L"0")
                    cbControlList->SetText(sel.first + L" controls selected");
                else
                    cbControlList->SetText(formname);
            }
        }
        if (cbControlList->Focused())
            cbControlList->SetSelStartAndLength(cbControlList->Text().length(), 0);
    }

    void Designer::FormNameChanged(const std::wstring &newname)
    {
        // Remember if the form was selected in the control list.
        bool updatetext = cbControlList->ItemIndex() == 0;

        cbControlList->BeginUpdate();
        cbControlList->DeleteItem(0);
        cbControlList->InsertItem(0, ShortNamespaceName(editedform->ClassName(true)) + L"* " + editedform->Name());
        cbControlList->EndUpdate();
        cbControlList->Invalidate();

        if (updatetext)
            cbControlList->SetItemIndex(0);
    }

    void Designer::ControlNameChanged(Object *control, const std::wstring &oldname, const std::wstring &newname)
    {
        // Remember if the old name was selected in the control list.
        bool updatetext = cbControlList->Text() == ShortNamespaceName(control->ClassName(true)) + L"* " + oldname;

        cbControlList->BeginUpdate();
        if (!oldname.empty())
            ControlDeleted(control, oldname);
        if (!newname.empty())
            ControlAdded(control, newname);
        cbControlList->EndUpdate();
        cbControlList->Invalidate();

        if (updatetext && !newname.empty()) // Update the current text to reflect the name change.
            cbControlList->SetText(ShortNamespaceName(control->ClassName(true)) + L"* " + newname);
    }

    void Designer::ControlDeleted(Object *control, std::wstring name)
    {
        name = ShortNamespaceName(control->ClassName(true)) + L"* " + name;

        // Find the name to remove from the control list.
        int cnt = cbControlList->Count();
        int min = 1, max = cnt - 1;
        while (min < max)
        {
            int mid = (min + max) / 2;
            std::wstring str = cbControlList->ItemText(mid);
            if (str > name)
                max = mid - 1;
            else if (str < name)
                min = mid + 1;
            else
            {
                min = mid;
                break;
            }
        }
        cbControlList->DeleteItem(min);
    }

    void Designer::ControlAdded(Object *control, std::wstring name)
    {
        name = ShortNamespaceName(control->ClassName(true)) + L"* " + name;

        // Find the name to add to the control list.
        int cnt = cbControlList->Count();
        int min = 1, max = cnt - 1;
        while (min <= max)
        {
            int mid = (min + max) / 2;
            if (cbControlList->ItemText(mid) < name)
                min = mid + 1;
            else
                max = mid - 1;
        }
        cbControlList->InsertItem(min, name);
    }

    void Designer::BeginControlChange()
    {
        if (!changing)
            cbControlList->BeginUpdate();
        ++changing;
    }

    void Designer::EndControlChange()
    {
        if (!changing)
            return;

        --changing;
        if (!changing)
        {
            cbControlList->EndUpdate();
            cbControlList->Invalidate();
        }
    }

    bool Designer::ControlsChanging()
    {
        return changing > 0;
    }

    void Designer::DesignKeyPress(WCHAR key)
    {
        if (editedform == nullptr)
            return;

        Focus();
        if (pages->ActivePageIndex() == 0 || pages->ActivePageIndex() > 2)
            lbProp->EditorKeyPress(key);
        else
            lbEvents->EditorKeyPress(key);

    }

    void Designer::SetProperties(Object *propowner)
    {
        DesignFormBase *form = editedform; // = propowner ? propowner->DesignParent() : NULL;
        if (dynamic_cast<DesignFormBase*>(propowner) || propowner == nullptr)
            form = (DesignFormBase*)propowner;

        if (form == editedform)
            UpdateControlList();
        else
            SetActiveForm(form);

        lbProp->SetProperties(propowner);
        lbEvents->SetProperties(propowner);
    }

    void Designer::SetProperties(const std::list<Object*> &propowners)
    {
        DesignFormBase *form = editedform; //!propowners.empty() ? propowners.front()->DesignParent() : NULL;
        if (!propowners.empty() && dynamic_cast<DesignFormBase*>(propowners.front()))
            form = (DesignFormBase*)propowners.front();

        if (form == editedform)
            UpdateControlList();
        else
            SetActiveForm(form);

        lbProp->SetProperties(propowners);
        lbEvents->SetProperties(propowners);
    }

    void Designer::CreateFormMenu(DesignFormBase *form)
    {
        MenuItem *mnu = miForms->Add(form->Name());
        mnu->OnClick = CreateEvent(this, &Designer::miformitemclick);
        mnu->SetTag((int)form);
        mnu->SetChecked(form->Visible());
        formmenus.push_back(mnu);
    }

    void Designer::_add(bool container)
    {
        DesignFormBase *frm = !container ? project->NewForm() : project->NewContainer();
        CreateFormMenu(frm);
        AddToNotifyList(frm, nrOwnership);
    }

    std::wstring Designer::UnusedFormName(const std::wstring &base)
    {
        int num = 1;

        for (auto it = formmenus.begin(); it != formmenus.end(); it++)
        {
            DesignFormBase *form = (DesignFormBase*)(*it)->Tag();
            std::wstring cname = form->Name();

            int blen = base.length();
            int clen = cname.length();
            if (clen <= blen || cname[blen] < L'0' || cname[blen] > L'9' || wcsncmp(cname.c_str(), base.c_str(), blen) != 0)
                continue;

            int p = clen;
            while (p >= blen && cname[p - 1] >= L'0' && cname[p - 1] <= L'9')
                p--;
            if (p != blen)
                continue;
            int cnum;
            StrToInt(cname, cnum, p);
            num = max(num, cnum + 1);
        }

        return base + IntToStr(num);
    }

    void Designer::CreateForm()
    {
        _add(false);
    }

    void Designer::CreateContainer()
    {
        _add(true);
    }

    void Designer::ChangeNotify(Object *object, int changetype)
    {
        base::ChangeNotify(object, changetype);

        if (changetype == CHANGE_SHOW)
        {
            auto it = std::find_if(formmenus.begin(), formmenus.end(), [object](MenuItem *mnu){ return (Object*)mnu->Tag() == object; } );
            if (it == formmenus.end())
                return;

            MenuItem *mnu = *it;
            DesignFormBase *form = (DesignFormBase*)mnu->Tag();
            mnu->SetChecked(form->Visible());

            it = std::find_if(formmenus.begin(), formmenus.end(), [](MenuItem *mnu) { return mnu->Checked(); } );
            form = (DesignFormBase*)mnu->Tag();

            if (it == formmenus.end())
            {
                lbProp->SetProperties(NULL);
                lbEvents->SetProperties(NULL);
                SetActiveForm(NULL);
            }
            else if (!form->Visible())
                form->Focus();
        }
    }

    void Designer::NameChangeNotify(Object *object, const std::wstring& oldname)
    {
        auto it = std::find_if(formmenus.begin(), formmenus.end(), [object](MenuItem *mnu){ return (Object*)mnu->Tag() == object; } );
        if (it == formmenus.end())
            return;
        MenuItem *mnu = *it;
        DesignFormBase *form = (DesignFormBase*)mnu->Tag();
        mnu->SetText(form->Name());

        base::NameChangeNotify(object, oldname);
    }

    std::wstring Designer::FormClassName(const std::wstring &name)
    {
        return L"T" + name;
    }

    bool Designer::FormNameTaken(const std::wstring &name)
    {
        if (name.empty())
            return false;

        for (auto it = formmenus.begin(); it != formmenus.end(); ++it)
        {
            DesignFormBase *form = (DesignFormBase*)(*it)->Tag();
            if (form->Name() == name || form->ClassName(false) == name)
                return true;
        }
        return false;
    }

    bool Designer::UnitNameTaken(const std::wstring &unitname)
    {
        if (unitname.empty())
            return false;

        for (auto it = formmenus.begin(); it != formmenus.end(); ++it)
        {
            DesignFormBase *form = (DesignFormBase*)(*it)->Tag();
            if (form->UnitName() == unitname)
                return true;
        }
        return false;
    }

    LRESULT Designer::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_NCACTIVATE:
            if (lbProp && lbProp->Parent() == this && lbProp->Active())
                wParam = true;
            break;
        case amClearMod:
            if (project)
                project->ClearModified();
            break;
        }

        return base::WindowProc(uMsg, wParam, lParam);
    }

    void Designer::sortdownchangedclick(void *sender, EventParameters param)
    {
        Object *obj = (Object*)sender;

        bool showcat = dynamic_cast<FlatButton*>(obj) != nullptr ? (obj->Tag() == 1) == ((FlatButton*)obj)->Down() : obj->Tag() == 1;

        miVSortAlpha->SetShortcut(showcat ? L"F9" : L"");
        miVSortCat->SetShortcut(!showcat ? L"F9" : L"");

        miVSortCat->SetChecked(showcat);
        miVSortAlpha->SetChecked(!showcat);
        btnPropAbc->SetDown(!showcat);
        btnPropCat->SetDown(showcat);
        lbProp->Sort(showcat);
        lbEvents->Sort(showcat);
    }

    void Designer::miexitclick(void *sender, EventParameters param)
    {
        Close();
    }

    void Designer::mivlabelsclick(void *sender, EventParameters param)
    {
        bpControls->SetCaptions(((MenuItem*)sender)->Checked());
    }

    void Designer::mivchoiceclick(void *sender, EventParameters param)
    {
        pages->SetActivePageIndex(((MenuItem*)sender)->Tag());
        if (((MenuItem*)sender)->Tag() == 0)
            lbProp->Focus();
        else if (((MenuItem*)sender)->Tag() == 1)
            lbEvents->Focus();
    }

    void Designer::miepropswitchclick(void *sender, EventParameters param)
    {
        if (Active() && pages->ActivePage() == tpProperties)
        {
            if (editedform != nullptr)
                editedform->Focus();
        }
        else
        {
            Focus();
            EditProperty();
        }
    }

    void Designer::mieeventswitchclick(void *sender, EventParameters param)
    {
        if (Active() && pages->ActivePage() == tpEvents)
        {
            if (editedform != nullptr)
                editedform->Focus();
        }
        else
        {
            Focus();
            EditEvent();
        }
    }

    bool Designer::SaveVerify()
    {
        if (!project || !project->Modified())
            return true;

        ModalResults res = ShowMessageBox(L"The current project has unsaved changes. Would you like to save it?", L"Query", mbYesNoCancel);
        if (res == mrCancel)
            return false;
        if (res == mrNo)
            return true;

        if (project->Path().empty() || !project->Save(project->Path()))
        {
            while ((res = saveas()) == mrNo)
                ;
            if (res == mrCancel)
                return false;
        }
        return true;
    }

    void Designer::minewprojclick(void *sender, EventParameters param)
    {
        if (!SaveVerify())
            return;

        if (!project->Path().empty())
        {
            recents.push_front(project->Path());
            MenuItem *mi = miRecents->Insert(0, project->Path());
            mi->OnClick = CreateEvent(this, &Designer::mirecentclick);
            miRecents->SetEnabled(true);
        }

        project->Destroy();
        CleanUp();

        new Project();

        Settings settings = GetSettings();
        settings.designer_lastproj = std::wstring();
        UpdateSettings(settings);

        miSaveProj->SetEnabled(false);

        FillUp();

        UpdateCaption();
    }

    void Designer::miopenprojclick(void *sender, EventParameters param)
    {
        if (!SaveVerify())
            return;

        if (!dlgOpenProj->Show(this))
            return;

        OpenProject(dlgOpenProj->FullFileName());
    }

    void Designer::OpenProject(const std::wstring &fname)
    {
        std::wstring projpath = project ? project->Path() : std::wstring();
        if (project && !projpath.empty() && projpath == fname)
            return;

        if (project)
            project->Destroy();
        CleanUp();


        if (!projpath.empty())
        {
            recents.push_front(projpath);
            MenuItem *mi = miRecents->Insert(0, projpath);
            mi->OnClick = CreateEvent(this, &Designer::mirecentclick);
        }

        Settings settings = GetSettings();

        try
        {
            new Project(fname);
            settings.designer_lastproj = fname;
        }
        catch(const std::wstring& str)
        {
            if (project)
                project->Destroy();
            ShowMessageBox(L"Error opening project file: " + str, L"Error", mbOk);
            new Project();
            settings.designer_lastproj = std::wstring();

            miSaveProj->SetEnabled(false);

            UpdateCaption();
        }
        catch(const wchar_t *str)
        {
            if (project)
                project->Destroy();
            ShowMessageBox(std::wstring(L"Error opening project file: ") + str, L"Error", mbOk);
            new Project();
            settings.designer_lastproj = std::wstring();

            miSaveProj->SetEnabled(false);

            UpdateCaption();
        }
        catch(...)
        {
            if (project)
                project->Destroy();
            ShowMessageBox(L"Error occured while opening project file.", L"Error", mbOk);
            new Project();
            settings.designer_lastproj = std::wstring();

            miSaveProj->SetEnabled(false);

            UpdateCaption();
        }

        UpdateSettings(settings);

        MenuItem *mi = miRecents->Count() ? miRecents->Items(0) : nullptr;
        for (auto it = recents.begin(); it != recents.end(); ++it, mi = mi->NextItem())
        {
            if ((*it) == fname)
            {
                recents.erase(it);
                mi->Destroy();
                break;
            }
        }
        miRecents->SetEnabled(!recents.empty());

        miSaveProj->SetEnabled(!project->Path().empty());
        FillUp();

        UpdateCaption();
    }

    void Designer::misaveprojclick(void *sender, EventParameters param)
    {
        if (project->Path().empty() || !project->Save(project->Path()))
            misaveprojasclick(sender, param);
    }

    void Designer::misaveprojasclick(void *sender, EventParameters param)
    {
        while (saveas() == mrNo)
            ;
    }

    ModalResults Designer::saveas()
    {
        if (!dlgSaveProj->Show(this))
            return mrCancel;

        std::wstring projpath = project->Path();

        if (!project->Save(dlgSaveProj->FullFileName()))
            return mrNo;

        if (projpath != dlgSaveProj->FullFileName())
        {
            if (!projpath.empty())
            {
                recents.push_front(projpath);
                MenuItem *mi = miRecents->Insert(0, projpath);
                mi->OnClick = CreateEvent(this, &Designer::mirecentclick);
            }

            Settings settings = GetSettings();
            settings.designer_lastproj = project->Path();
            UpdateSettings(settings);

            if (!recents.empty())
            {
                auto it = ++recents.begin();
                MenuItem *mi = miRecents->Items(0)->NextItem();
                for ( ; it != recents.end(); ++it, mi = mi->NextItem())
                {
                    if ((*it) == project->Path())
                    {
                        recents.erase(it);
                        mi->Destroy();
                        break;
                    }
                }
            }

            miRecents->SetEnabled(!recents.empty());
        }

        miSaveProj->SetEnabled(!project->Path().empty());

        UpdateCaption();

        return mrOk;
    }

    void Designer::misavesrcclick(void *sender, EventParameters param)
    {
        if (project->SourcePath().empty())
        {
            ShowMessageBox(L"Create the project files or fill the project settings to specify a target folder where the source files are found.", L"Unknown target", mbOk);
            return;
        }

        std::wstring invalid;
        for (auto it = formmenus.begin(); it != formmenus.end(); ++it)
        {
            DesignFormBase *form = (DesignFormBase*)(*it)->Tag();
            if (form->UnitName().empty())
            {
                if (!invalid.empty())
                    invalid += L", ";
                invalid += form->Name();
            }
        }
        if (!invalid.empty())
        {
            ShowMessageBox(L"Specify a unit name for the following forms: " + invalid, L"Missing unit name", mbOk);
            return;
        }

        if (project->Modified() && ShowMessageBox(L"The project will be saved after the export.", L"Notice", mbOkCancel) != mrOk)
            return;

        if (!PathExists(project->ResolvedSourcePath()))
            CreateFolder(project->ResolvedSourcePath(), true);

        std::wstring fname = AppendToPath(project->ResolvedSourcePath(), L"nresource.rc");
        if (FileExists(fname))
            CopyFile(fname.c_str(), (fname + L".bak").c_str(), FALSE);
        resfile.open(fname, std::ios_base::out | std::ios_base::trunc);
        if (resfile.fail())
        {
            ShowMessageBox(L"Couldn't open nresource.rc at source file export path.", L"Error", mbOk);
            return;
        }

        if (!PathExists(project->ResolvedResourcePath()))
            CreateFolder(project->ResolvedResourcePath(), true);

        fname = AppendToPath(project->ResolvedResourcePath(), L"program.ico");
        if (FileExists(fname))
            CopyFile(fname.c_str(), (fname + L".bak").c_str(), FALSE);
        FILESTD fstream icofile(fname, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
        if (icofile.fail())
        {
            ShowMessageBox(L"Couldn't create program.ico at resource export path.", L"Error", mbOk);
            return;
        }

        nextresid = 0x7000;
        resfile << L"IDI_MAINICON ICON \"" << EscapeCString(project->ResolvedResourcePath()) << L"\\\\program.ico\"" << std::endl << std::endl;

        IconData &icon = *project->AppIcon();
        if (icon.filedata != NULL)
            icofile.write(icon.filedata, icon.filesize);
        else
        {
            HRSRC res = FindResource(NULL, MAKEINTRESOURCE(IDI_PRGDEFAULT), RT_RCDATA);
            if (!res)
            {
                ShowMessageBox(L"Default program icon resource missing.", L"Error", mbOk);
                return;
            }
            HGLOBAL glob = LoadResource(NULL, res);
            char *dat = (char*)LockResource(glob);
            icofile.write(dat, SizeofResource(NULL, res));
        }
        if (icofile.fail())
        {
            ShowMessageBox(L"Error writing icon file to resource export path.", L"Error", mbOk);
            return;
        }
        icofile.close();

        for (auto mi : formmenus)
        {
            DesignFormBase *form = (DesignFormBase*)mi->Tag();

            // TODO: only update project that was changed since last export. The Modified() member of forms can't be used as they are only temporary for the current session.
            // WARNING, the project file must be recreated anyway, or must be able to determine which data belongs to which form so those that don't get updated won't be deleted.
            SaveFormSrc(form);
        }

        // TODO: try/catch because even unsuccessful export must be followed by save.

        if (project->Modified())
            miSaveProj->Click();

        if (resfile.is_open())
            resfile.close();
    }

    void Designer::SaveFormSrc(DesignFormBase *form)
    {
        serializedform = form;

        // Find the path where the cpp files should be saved.
        std::wstring root = project->ResolvedSourcePath();

        std::wstring fname = AppendToPath(root, form->UnitName());
        std::wstring fhname = fname + L"." + project->HeaderExt();

        std::wstringstream header;
        std::wstringstream source;

        std::wstring fsname;
        {
            FileStream wsh(fhname, std::ios_base::in);
            if (wsh.fail() && FileExists(fhname))
            {
                ShowMessageBox(L"Couldn't open the header file for reading!", L"Error", mbOk);
                return;
            }
            wsh.clear();

            fsname = fname + L"." + project->CppExt();
            FileStream wscpp(fsname, std::ios_base::in);
            if (wscpp.fail() && FileExists(fsname))
            {
                ShowMessageBox(L"Couldn't open the source file for reading!", L"Error", mbOk);
                return;
            }
            wscpp.clear();

            if (wsh.is_open())
            {
                wsh.skipbom();
                header << wsh.rdbuf();
                wsh.close();
            }
            if (wscpp.is_open())
            {
                wscpp.skipbom();
                source << wscpp.rdbuf();
                wscpp.close();
            }
        }

        std::vector<DesignFormBase::EventListItem*> events;

        bool createheader = (int)header.tellp() == 0; // There is no header file yet or it has nothing written to it.
        bool createsource = (int)source.tellp() == 0; // There is no source file yet or it has nothing written to it.
        form->CollectEvents(events);

        memberindexes.clear();

        Settings settings = GetSettings();
        if (!createheader)
        {
            Indentation indent(settings.code_tabindent, settings.code_indentsize);
            try
            {
                header.seekp(0);
                header.seekg(0);
                form->HeaderUpdate(indent, header, events);
            }
            catch(...)
            {
                ShowMessageBox(L"Error occured while updating header file for form: " + form->Name(), L"Error", mbOk);
                return;
            }
        }
        else
        {
            Indentation indent(settings.code_tabindent, settings.code_indentsize);
            form->HeaderExport(indent, header, events, true);
        }

        if (!createsource)
        {
            Indentation indent(settings.code_tabindent, settings.code_indentsize);
            try
            {
                source.seekp(0);
                source.seekg(0);
                form->CppUpdate(indent, source, events);
            }
            catch(...)
            {
                ShowMessageBox(L"Error occured while updating source file for form: " + form->Name(), L"Error", mbOk);
                return;
            }
        }
        else
        {
            Indentation indent(settings.code_tabindent, settings.code_indentsize);
            source << indent << L"#include \"stdafx_zoli.h\"" << std::endl;
            //source << indent << L"#include \"controlbase.h\"" << std::endl;
            //source << indent << L"#include \"syscontrol.h\"" << std::endl;
            //source << indent << L"#include \"dialog.h\"" << std::endl;
            //source << indent << L"#include \"graphiccontrol.h\"" << std::endl;
            //source << indent << L"#include \"buttons.h\"" << std::endl;
            //source << std::endl;
            source << indent << L"#include \"" << GetFileName(fhname) << L"\"" << std::endl;
            source << std::endl;
            form->CppExport(indent, source, events);
        }

        {
            CopyFile(fhname.c_str(), (fhname + L".bak").c_str(), FALSE);
            FileStream wsh(fhname, std::ios_base::out | std::ios_base::trunc);
            if (wsh.fail())
            {
                ShowMessageBox(L"Couldn't open the header file for writing!", L"Error", mbOk);
                return;
            }

            CopyFile(fsname.c_str(), (fsname + L".bak").c_str(), FALSE);
            FileStream wscpp(fsname, std::ios_base::out | std::ios_base::trunc);
            if (wscpp.fail())
            {
                ShowMessageBox(L"Couldn't open the source file for writing!", L"Error", mbOk);
                return;
            }

            // TODO: replace message boxes with throws within a try catch block and a message box at the end

            header.seekg(0);
            wsh.writebom();
            wsh << header.rdbuf();
            wsh.close();

            source.seekg(0);
            wscpp.writebom();
            wscpp << source.rdbuf();
            wscpp.close();
        }

        form->FinalizeExport();
    }

    int Designer::NextMemberIndex(std::wstring typestr)
    {
        if (memberindexes.count(typestr) > 0)
            return ++memberindexes[typestr];

        memberindexes[typestr] = serializedform->NameNext(typestr);
        return memberindexes[typestr];
    }

    int Designer::ExportToResource(std::vector<byte> &res)
    {
        const char *hexnum[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };

        resfile << IntToStr(nextresid) << " RCDATA" << std::endl << "{" << std::endl;

        // Print the bytes from res, 256 on each line.
        int siz = res.size();
        int lpos = 0; // Line position in characters. Up to 256.
        int pos = 0; // Position in bytes of the data. Up to siz.
        byte *arr = &res[0];
        while (pos < siz)
        {
            if (lpos == 0)
                resfile << "\t\"";
            byte c = arr[pos++];
            if (c >= 32 && c != 128 && c != '"' && c != '\\' && c != 0x7f)
                resfile << (char)c;
            else
                resfile << "\\x" << hexnum[c / 16] << hexnum[c - ((c / 16) * 16)];
            if (++lpos == 256)
            {
                lpos = 0;
                resfile << "\"" << (pos < siz ? "," : "") << std::endl;
            }
        }
        if (lpos != 0)
            resfile << "\"" << std::endl;

        resfile << "}" << std::endl << std::endl;
        return nextresid++;
    }

    int Designer::FormCount()
    {
        return project->FormCount();
    }

    DesignFormBase* Designer::Forms(int index)
    {
        return project->Forms(index);
    }

    void Designer::PagesTabChanged(int newtab)
    {
        miVProps->SetChecked(newtab == 0);
        miVEvents->SetChecked(newtab == 1);
        miVControls->SetChecked(newtab == 2);
    }

    void Designer::miaddformclick(void *sender, EventParameters param)
    {
        CreateForm();
    }

    void Designer::miaddcontainerclick(void *sender, EventParameters param)
    {
        CreateContainer();
    }

    void Designer::miformitemclick(void *sender, EventParameters param)
    {
        MenuItem *mi = (MenuItem*)sender;
        auto it = std::find(formmenus.begin(), formmenus.end(), mi);
        if (it == formmenus.end())
            return;

        DesignFormBase *form = (DesignFormBase*)mi->Tag();
        if (form->Visible() != mi->Checked())
            return;

        form->SetVisible(!mi->Checked());
    }

    void Designer::micreatefilesclick(void *sender, EventParameters param)
    {
        ResourceUnzipper unzipper;
        unzipper.Unzip(IDR_VS2010PROJECT);
        std::vector<std::wstring> lines;
        int cnt = unzipper.Count();
        for (int ix = 0; ix < cnt; ++ix)
        {
            unzipper.Lines8(ix, lines);
            lines.clear();
        }
    }

    void Designer::mioptionsclick(void *sender, EventParameters param)
    {
        frmOptions = new TfrmOptions;
        frmOptions->SetTopLevelParent(this);
        ModalResults mr = frmOptions->ShowModal();
        frmOptions->Destroy();

        if (mr == mrOk)
        {
            Settings settings = GetSettings();
            SetTopmost(settings.designer_ontop);
        }
    }

    void Designer::SetTopmost(bool newtopmost)
    {
        base::SetTopmost(newtopmost);
        if (!newtopmost)
            NoTopmostRecursive();
    }

    void Designer::mirecentclick(void *sender, EventParameters param)
    {
        if (!SaveVerify())
            return;

        MenuItem *mi = (MenuItem*)sender;
        int ix = 0;
        auto it = recents.begin();
        for ( ; it != recents.end() && miRecents->Items(ix) != mi; ++it, ++ix)
            ;
        if (it == recents.end())
            return;

        OpenProject(*it);
    }

    void Designer::miprojsettingsclick(void *sender, EventParameters param)
    {
        frmProjSettings = new TfrmProjSettings;
        frmProjSettings->SetTopLevelParent(this);
        frmProjSettings->ShowModal();
        if (frmProjSettings->FormsChanged())
            FillUp();

        frmProjSettings->Destroy();
    }

    void Designer::formclose(void *sender, FormCloseParameters param)
    {
        FinalizeEdits();

        if (project->Modified())
        {
            auto r = ShowMessageBox(L"The project has unsaved changes. Would you like to save before quitting?", L"Query", mbYesNoCancel);
            if (r == mrCancel)
                param.action = caPreventClose;
            else if (r == mrYes)
            {
                std::wstring origprojname;
                if (!project->Path().empty())
                    project->Save(project->Path());

                if (project->Path().empty())
                {
                    while (saveas() == mrNo)
                        ;
                }
                if (project->Path().empty())
                    param.action = caPreventClose;
            }
        }
    }

    void Designer::pagestabchange(void *sender, TabChangeParameters param)
    {
        PagesTabChanged(param.tabindex);
        pPropTop->SetParent(param.tabindex == 0 ? tpProperties : tpEvents);
    }

    void Designer::SelectControl(std::wstring name) // Find an element in the controls list which matches the search string. If no match found, restore the control list's text.
    {
        if (editedform)
        {
            unsigned int p = name.find(L"*");
            if (p != std::wstring::npos) // The class name is specified for the control to select.
            {
                std::wstring search = trim(name.substr(0, p)) + L"* " + trim(name.substr(p + 1));

                int cnt = cbControlList->Count();
                int iix = -1; // Index of the first item in the list which would match without case sensitivity. Use this if there is no exact match.
                int mix = -1; // Index of item which exactly matches the search string.
                std::wstring isearch = GenToLower(search); // Lower case search string for case insensitive matches.
                for (int ix = 0; ix < cnt; ++ix)
                {
                    std::wstring str = cbControlList->ItemText(ix);
                    if (str == search) // Exact match.
                    {
                        mix = ix;
                        break;
                    }

                    if (iix == -1 && GenToLower(str) == isearch) // Case insensitive match.
                    {
                        isearch = str; // Update the lowercase search string to the one found in the control list. This value can be used instead of the original searched string if it was not found.
                        iix = ix;
                    }
                }
                if (mix == -1 && iix == -1) // No match found.
                {
                    UpdateControlList();
                    return;
                }
                if (mix == 0 || (mix == -1 && iix == 0)) // The first item is the form, which can be selected by passing SelectObject an empty string.
                    name = std::wstring();
                else
                {
                    name = mix != -1 ? search : isearch;
                    name = trim(name.substr(name.find(L"*") + 1));
                }
            }
            else // Only the name of the control is specified.
            {
                name = trim(name);
                std::wstring isearch = GenToLower(name);
                int cnt = cbControlList->Count();
                int mix = -1;
                int iix = -1;
                for (int ix = 0; ix < cnt; ++ix)
                {
                    std::wstring str = cbControlList->ItemText(ix);
                    str = trim(str.substr(str.find(L"*") + 1));
                    if (str == name)
                    {
                        mix = ix;
                        break;
                    }
                    if (iix == -1 && GenToLower(str) == name)
                    {
                        iix = ix;
                        isearch = str;
                    }
                }
                if (mix == -1 && iix == -1) // No match found.
                {
                    UpdateControlList();
                    return;
                }
                if (mix == 0 || (mix == -1 && iix == 0)) // The first item is the form, which can be selected by passing SelectObject an empty string.
                    name = std::wstring();
                else
                    name = mix != -1 ? name : isearch;
            }

            editedform->SelectObject(name);
        }

        UpdateControlList();
    }

    DesignFormBase* Designer::ActiveForm()
    {
        return editedform;
    }

    void Designer::cbcontrollistchange(void *sender, EventParameters param)
    {
        if (cbControlList->ItemIndex() >= 0)
            SelectControl(cbControlList->ItemText(cbControlList->ItemIndex()));
    }

    void Designer::cbcontrollistkeypress(void *sender, KeyPressParameters param)
    {
        if (param.key == VK_RETURN)
        {
            SelectControl(cbControlList->Text());
            param.key = 0;
            return;
        }
    }

    void Designer::cbcontrollistlosefocus(void *sender, FocusChangedParameters param)
    {
        UpdateControlList();
    }

    //void Designer::pmpropshow(void *sender, PopupMenuParameters param)
    //{
    //    PropertyListbox *box = (PropertyListbox*)param.popupcontrol;
    //    box->InitPopup(param.screenpos);
    //}
    //
    //void Designer::pmpropalpha(void *sender, EventParameters param)
    //{
    //    PropertyListbox *box = (PropertyListbox*)pmProp->Tag();
    //    box->btnAbc->Click();
    //}
    //
    //void Designer::pmpropcat(void *sender, EventParameters param)
    //{
    //    PropertyListbox *box = (PropertyListbox*)pmProp->Tag();
    //    box->btnCat->Click();
    //}

    void Designer::selectactiveform(void *sender, AppSelectActiveFormParameters param)
    {
        DesignContainerForm *cont = dynamic_cast<DesignContainerForm*>(param.newactive);
        if (cont == NULL)
            return;
        DesignForm *frm = dynamic_cast<DesignForm*>(cont->FormObj());
        if (frm && frm != param.oldactive)
            param.newactive = frm;

    }

    //---------------------------------------------


}
/* End of NLIBNS */

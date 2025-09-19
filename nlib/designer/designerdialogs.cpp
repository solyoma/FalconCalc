#include "stdafx_zoli.h"

#include "application.h"
#include "designerdialogs.h"
#include "designercontrols.h"
#include "designer.h"
#include "buttons.h"
#include "imagelist.h"
#include "images.h"
#include "graphiccontrol.h"
#include "gridbase.h"
#include "utility.h"
#include <fstream>

#ifdef __MINGW32__
#include "compfstream.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


    void CreateImgPropOpenDialog();
    void CreateImgPropSaveDialog();
    extern OpenDialog *imgpropopendialog;
    extern SaveDialog *imgpropsavedialog;

    TransparentColorPickerDialog *transparentcolordialog = NULL;
    BitmapSelectorDialog *bitmapselectordialog = NULL;
    ImagelistEditorDialog *imagelisteditordialog = NULL;

    extern Imagelist *ilButtons;

    //---------------------------------------------


    DesignerDialog::DesignerDialog() : mode(dmDefault)
    {
    }

    DialogModes DesignerDialog::DialogMode()
    {
        return mode;
    }

    void DesignerDialog::SetDialogMode(DialogModes newdialogmode)
    {
        mode = newdialogmode;
    }

    ModalResults DesignerDialog::Show(Form *dialog, Form *topparent)
    {
        dialog->SetDialogMode(DialogMode());
        dialog->SetTopLevelParent(topparent);
        return dialog->ShowModal();
    }


    //---------------------------------------------


    TransparentColorPickerDialog::TransparentColorPickerDialog() : selcol(clNone), allownone(true)
    {

    }

    bool TransparentColorPickerDialog::Show(Form *topparent, Bitmap *bmp)
    {
        if (!allownone && selcol == clNone)
            throw L"Set a selcol value when allownone is false!";

        TransparentColorPicker *picker = new TransparentColorPicker(this);
        picker->bmp = bmp;

        ModalResults mr = base::Show(picker, topparent);

        picker->Destroy();
        return mr == mrOk;
    }

    bool TransparentColorPickerDialog::Selected()
    {
        return selcol != clNone;
    }


    //---------------------------------------------


    TransparentColorPickerDialog::TransparentColorPicker::TransparentColorPicker(TransparentColorPickerDialog *owner) :
            owner(owner), zoom(1), center(0, 0), dragkey(false), dragging(false), picking(false), picked(false)
    {
        SetShowPosition(fspParentMonitorCenter);
        bgpattern = new Bitmap(16, 16);

        bgpattern->Clear(clWhite);
        bgpattern->SetBrush(Color(220, 220, 220));
        bgpattern->FillRect(0, 0, 8, 8);
        bgpattern->FillRect(8, 8, 16, 16);

        SetBounds(Rect(615, 350, 1098, 605));
        SetText(L"Transparent color picker");
        //GetFont()->SetFamily(L"Segoe UI");
        SetKeyPreview(true);

        OnResize = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::formresize);
        OnKeyPush = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::formkeypush);
        OnKeyUp = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::formkeyup);

        pbImage = new Paintbox();
        pbImage->SetBounds(Rect(27, 2, 465, 186));
        pbImage->SetText(L"pbImage");
        pbImage->SetBorderStyle(bsModern);
        pbImage->SetAnchors(caLeft | caTop | caRight | caBottom);
        //pbImage->GetFont()->SetFamily(L"Segoe UI");
        pbImage->SetTabOrder(0);
        pbImage->OnPaint = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::paintimage);
        pbImage->OnMouseDown = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::paintmousedown);
        pbImage->OnMouseUp = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::paintmouseup);
        pbImage->OnMouseMove = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::paintmousemove);
        pbImage->SetParent(this);

        btnOk = new Button();
        btnOk->SetBounds(Rect(196, 189, 276, 213));
        btnOk->SetText(L"Ok");
        btnOk->SetAnchors(caBottom);
        //btnOk->GetFont()->SetFamily(L"Segoe UI");
        btnOk->SetTabOrder(1);
        btnOk->OnClick = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::okbtnclick);
        btnOk->SetParent(this);

        btnHand = new FlatButton();
        btnHand->SetBounds(Rect(2, 4, 25, 27));
        btnHand->SetText(L"");
        //btnHand->GetFont()->SetFamily(L"Segoe UI");
        btnHand->SetTabOrder(2);
        btnHand->SetType(fbtRadiobutton);
        btnHand->SetImagePosition(bipCenter);
        btnHand->Image()->SetImages(ilButtons);
        btnHand->Image()->SetImageIndex(1);
        btnHand->SetParent(this);

        btnPick = new FlatButton();
        btnPick->SetBounds(Rect(2, 28, 25, 51));
        btnPick->SetText(L"");
        //btnPick->GetFont()->SetFamily(L"Segoe UI");
        btnPick->SetTabOrder(3);
        btnPick->SetType(fbtRadiobutton);
        btnPick->SetDown(true);
        btnPick->SetImagePosition(bipCenter);
        btnPick->Image()->SetImages(ilButtons);
        btnPick->Image()->SetImageIndex(0);
        btnPick->SetParent(this);

        btnPlus = new FlatButton();
        btnPlus->SetBounds(Rect(2, 57, 25, 80));
        btnPlus->SetText(L"");
        //btnPlus->GetFont()->SetFamily(L"Segoe UI");
        btnPlus->SetTabOrder(4);
        btnPlus->SetImagePosition(bipCenter);
        btnPlus->Image()->SetImages(ilButtons);
        btnPlus->Image()->SetImageIndex(2);
        btnPlus->OnClick = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::plusbtnclick);
        btnPlus->SetParent(this);

        btnMinus = new FlatButton();
        btnMinus->SetBounds(Rect(2, 81, 25, 104));
        btnMinus->SetText(L"");
        //btnMinus->GetFont()->SetFamily(L"Segoe UI");
        btnMinus->SetTabOrder(5);
        btnMinus->SetImagePosition(bipCenter);
        btnMinus->Image()->SetImages(ilButtons);
        btnMinus->Image()->SetImageIndex(3);
        btnMinus->OnClick = CreateEvent(this, &TransparentColorPickerDialog::TransparentColorPicker::minusbtnclick);
        btnMinus->SetParent(this);

        pPick = new Panel;
        pPick->SetBounds(RectS(2, 125, 23, 23));
        pPick->SetBorderStyle(bsModern);
        pPick->SetInnerBorderStyle(pbsNone);
        pPick->SetParentBackground(false);
        pPick->SetShowText(false);
        pPick->SetParent(this);

        if (owner->selcol != clNone)
        {
            pPick->SetColor(owner->selcol);
            picked = true;
        }
    }

    TransparentColorPickerDialog::TransparentColorPicker::~TransparentColorPicker()
    {
        delete bgpattern;
    }

    void TransparentColorPickerDialog::TransparentColorPicker::paintimage(void *sender, PaintParameters param)
    {
        Canvas *c = param.canvas;
    

        c->SetInterpolationMode(imNearestNeighbor);
        c->SetPixelOffsetMode(pomHalf);

        Rect r = pbImage->ClientRect();
        RectF bmpr = RectFS(-center.x * zoom, -center.y * zoom, bmp->Width() * zoom, bmp->Height() * zoom);
        bmpr.Move(r.left + (r.Width() - bmpr.Width()) / 2, r.top + (r.Height() - bmpr.Height()) / 2);

        r = bmpr;

        if (picked)
            c->SetColorKey(pPick->GetColor(), pPick->GetColor());

        //Gdiplus::TextureBrush tb(bgpattern->GetBitmap());
        c->SetClip(r);
        c->SetBrush(bgpattern);
        c->FillRect(param.updaterect);
        c->ResetClip();
        c->ExcludeClip(r);
        c->SetBrush(Color(160, 160, 160));
        c->FillRect(param.updaterect);
        c->ResetClip();
        c->Draw(bmp, r.left, r.top, r.Width(), r.Height(), 0, 0, bmp->Width(), bmp->Height());
    }

    void TransparentColorPickerDialog::TransparentColorPicker::PickColor(int x, int y)
    {
        Rect r = pbImage->ClientRect();
        RectF bmpr = RectFS(-center.x * zoom, -center.y * zoom, bmp->Width() * zoom, bmp->Height() * zoom);
        bmpr.Move(r.left + (r.Width() - bmpr.Width()) / 2, r.top + (r.Height() - bmpr.Height()) / 2);
        Rect r2 = Rect(bmpr.left, bmpr.top, bmpr.right, bmpr.bottom);
        if (!PtInRect(&r2, Point(x, y)))
        {
            if (!owner->allownone)
                return;

            picked = false;
            pPick->SetColor(clBtnFace);
            pbImage->Invalidate();
            owner->selpt = Point(-1, -1);
            owner->selcol = clNone;
            return;
        }
        x -= r2.left;
        y -= r2.top;
        x /= zoom;
        y /= zoom;
        Color c = bmp->Pixel(x, y);
        if (c != pPick->GetColor())
        {
            picked = true;
            pPick->SetColor(c);
            pbImage->Invalidate();
            owner->selpt = Point(x, y);
            owner->selcol = c;
        }
    }

    void TransparentColorPickerDialog::TransparentColorPicker::plusbtnclick(void *sender, EventParameters param)
    {
        if (zoom == 24)
            return;
        if (zoom < 3)
            ++zoom;
        else
            zoom *= 2;
        pbImage->Invalidate();
    }

    void TransparentColorPickerDialog::TransparentColorPicker::minusbtnclick(void *sender, EventParameters param)
    {
        if (zoom == 1)
            return;
        if (zoom > 3)
            zoom /= 2;
        else
            --zoom;
        pbImage->Invalidate();
    }

    void TransparentColorPickerDialog::TransparentColorPicker::formresize(void *sender, EventParameters param)
    {
        pbImage->Invalidate();
    }

    void TransparentColorPickerDialog::TransparentColorPicker::formkeypush(void *sender, KeyPushParameters param)
    {
        switch (param.keycode)
        {
        case VK_ADD:
            btnPlus->Click();
            break;
        case VK_SUBTRACT:
            btnMinus->Click();
            break;
        }

        switch (param.key)
        {
        case L'H':
            btnHand->Click();
            btnHand->SetDown(true);
            break;
        case L'P':
            btnPick->Click();
            btnPick->SetDown(true);
            break;
        case L' ':
            if (btnHand->Down())
                break;
            dragkey = true;
            btnHand->Click();
            btnHand->SetDown(true);
            break;
        }
    }

    void TransparentColorPickerDialog::TransparentColorPicker::formkeyup(void *sender, KeyParameters param)
    {
        if (param.keycode == VK_SPACE && dragkey)
        {
            btnPick->Click();
            btnPick->SetDown(true);
            dragkey = false;
        }
    }

    void TransparentColorPickerDialog::TransparentColorPicker::paintmousedown(void *sender, MouseButtonParameters param)
    {
        if (param.button != mbLeft)
        {
            if (param.button == mbRight)
                PickColor(-1, -1);
            return;
        }

        if (btnHand->Down())
        {
            dragging = true;
            dragpos = Point(param.x, param.y);
        }
        else
        {
            PickColor(param.x, param.y);
            picking = true;
        }
    }

    void TransparentColorPickerDialog::TransparentColorPicker::paintmouseup(void *sender, MouseButtonParameters param)
    {
        if (param.button != mbLeft)
            return;
        dragging = false;
        picking = false;
    }

    void TransparentColorPickerDialog::TransparentColorPicker::paintmousemove(void *sender, MouseMoveParameters param)
    {
        if (dragging)
        {
            center.Move( (float)(dragpos.x - param.x) / zoom, (float)(dragpos.y - param.y) / zoom);
            pbImage->Invalidate();
            dragpos = Point(param.x, param.y);
        }
        else if (picking)
            PickColor(param.x, param.y);

    }

    void TransparentColorPickerDialog::TransparentColorPicker::okbtnclick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }


    //---------------------------------------------


    BitmapSelectorDialog::BitmapSelectorDialog()
    {
        bmp = new ControlBitmap();
    }

    BitmapSelectorDialog::~BitmapSelectorDialog()
    {
        bmp->Free();
        delete bmp;
    }

    bool BitmapSelectorDialog::Show(Form *topparent, const ControlBitmap &abmp)
    {
        bmp->Free();
        abmp.CopyTo(*bmp);
        BitmapSelector *selector = new BitmapSelector(this, bmp);

        ModalResults mr = base::Show(selector, topparent);

        selector->Destroy();
        return mr == mrOk;
    }

    const ControlBitmap& BitmapSelectorDialog::GetBitmap()
    {
        return *bmp;
    }


    //---------------------------------------------


    BitmapSelectorDialog::BitmapSelector::BitmapSelector(BitmapSelectorDialog *owner, ControlBitmap *bmp) : owner(owner), bmp(bmp)
    {
        SetShowPosition(fspParentMonitorCenter);
        SetKeyPreview(true);

        bgpattern = new Bitmap(16, 16);
        bgpattern->Clear(clWhite);
        bgpattern->SetBrush(Color(220, 220, 220));
        bgpattern->FillRect(0, 0, 8, 8);
        bgpattern->FillRect(8, 8, 16, 16);

        OnResize = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::formresize);

        SetBounds(Rect(615, 350, 1061, 686));
        SetText(L"Load, save or change the bitmap...");
        //GetFont()->SetFamily(L"Segoe UI");
        SetMinimumWidth(329);
        SetMinimumHeight(229);

        pb = new Paintbox();
        pb->SetBounds(Rect(8, 8, 326, 291));
        pb->SetText(L"pb");
        pb->SetBorderStyle(bsModern);
        pb->SetAnchors(caLeft | caTop | caRight | caBottom);
        //pb->GetFont()->SetFamily(L"Segoe UI");
        pb->SetTabOrder(0);
        pb->SetParent(this);
        pb->OnPaint = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::pbpaint);

        btnClear = new Button();
        btnClear->SetBounds(Rect(333, 57, 421, 81));
        btnClear->SetText(L"Clear");
        btnClear->SetAnchors(caTop | caRight);
        //btnClear->GetFont()->SetFamily(L"Segoe UI");
        btnClear->SetTabOrder(1);
        btnClear->SetParent(this);
        btnClear->OnClick = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::clearbtnclick);

        btnLoad = new Button();
        btnLoad->SetBounds(Rect(333, 9, 421, 33));
        btnLoad->SetText(L"Load...");
        btnLoad->SetAnchors(caTop | caRight);
        //btnLoad->GetFont()->SetFamily(L"Segoe UI");
        btnLoad->SetTabOrder(2);
        btnLoad->SetParent(this);
        btnLoad->OnClick = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::loadbtnclick);

        btnSave = new Button();
        btnSave->SetBounds(Rect(333, 33, 421, 57));
        btnSave->SetText(L"Save...");
        btnSave->SetAnchors(caTop | caRight);
        //btnSave->GetFont()->SetFamily(L"Segoe UI");
        btnSave->SetTabOrder(3);
        btnSave->SetParent(this);
        btnSave->OnClick = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::savebtnclick);

        btnTransparent = new Button();
        btnTransparent->SetBounds(Rect(333, 81, 421, 105));
        btnTransparent->SetText(L"Transparent...");
        btnTransparent->SetAnchors(caTop | caRight);
        //btnTransparent->GetFont()->SetFamily(L"Segoe UI");
        btnTransparent->SetTabOrder(4);
        btnTransparent->SetParent(this);
        btnTransparent->OnClick = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::transparentbtnclick);

        btnOk = new Button();
        btnOk->SetBounds(Rect(333, 119, 421, 143));
        btnOk->SetText(L"OK");
        btnOk->SetAnchors(caTop | caRight);
        //btnOk->GetFont()->SetFamily(L"Segoe UI");
        btnOk->SetTabOrder(5);
        btnOk->SetParent(this);
        btnOk->OnClick = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::okbtnclick);

        btnCancel = new Button();
        btnCancel->SetBounds(Rect(333, 159, 421, 183));
        btnCancel->SetText(L"Cancel");
        btnCancel->SetAnchors(caTop | caRight);
        //btnCancel->GetFont()->SetFamily(L"Segoe UI");
        btnCancel->SetTabOrder(6);
        btnCancel->SetParent(this);
        btnCancel->SetCancel(true);
        btnCancel->OnClick = CreateEvent(this, &BitmapSelectorDialog::BitmapSelector::cancelbtnclick);
    }

    BitmapSelectorDialog::BitmapSelector::~BitmapSelector()
    {
        delete bgpattern;
    }

    void BitmapSelectorDialog::BitmapSelector::Show()
    {
        btnSave->SetEnabled(bmp->image != NULL);
        if (!bmp->image)
            btnTransparent->SetEnabled(false);
        else
        {
            ImageTypes type = bmp->image->GetImageType();
            btnTransparent->SetEnabled(type == itBMP || type == itTIFF || type == itUndefined || type == itMemoryBitmap);
        }

        base::Show();
    }

    void BitmapSelectorDialog::BitmapSelector::okbtnclick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }

    void BitmapSelectorDialog::BitmapSelector::cancelbtnclick(void *sender, EventParameters param)
    {
        SetModalResult(mrCancel);
    }

    void BitmapSelectorDialog::BitmapSelector::clearbtnclick(void *sender, EventParameters param)
    {
        btnSave->SetEnabled(false);
        btnTransparent->SetEnabled(false);
        bmp->Free();
        pb->Invalidate();
    }

    void BitmapSelectorDialog::BitmapSelector::loadbtnclick(void *sender, EventParameters param)
    {
        CreateImgPropOpenDialog();

        ControlBitmap cbmp;
        try
        {
            if (imgpropopendialog->Show(this))
            {
                Bitmap *tbmp;

                tbmp = new Bitmap(imgpropopendialog->FullFileName());
        
                cbmp.filename = imgpropopendialog->FileName();

                FILESTD ifstream f(imgpropopendialog->FullFileName(), std::ifstream::binary | std::ifstream::ate);

                cbmp.filesize = f.tellg();
                f.seekg(std::ios_base::beg);
                cbmp.filedata = new char[cbmp.filesize];
                f.read(cbmp.filedata, cbmp.filesize);
                f.close();

                ImageTypes type = tbmp->GetImageType();
                if (type == itBMP || type == itTIFF || type == itUndefined || type == itMemoryBitmap)
                {
                    transparentcolordialog->selcol = clNone;
                    transparentcolordialog->allownone = true;
                    transparentcolordialog->Show(this, tbmp);
                    if (transparentcolordialog->Selected())
                    {
                        cbmp.transparent = transparentcolordialog->selcol;
                        Bitmap *tmp = tbmp;
                        tbmp = new Bitmap(tmp->Width(), tmp->Height());
                        tbmp->SetColorKey(cbmp.transparent, cbmp.transparent);
                        tbmp->Draw(tmp, 0, 0);
                        tbmp->ResetColorKey();
                    }
                    else
                        cbmp.transparent = clNone;
                    btnTransparent->SetEnabled(true);
                }
                else
                    btnTransparent->SetEnabled(false);

                cbmp.image = tbmp;

                bmp->Free();
                *bmp = cbmp;
                btnSave->SetEnabled(true);
            }
        }
        catch(...)
        {
            cbmp.Free();
        }

        pb->Invalidate();
    }

    void BitmapSelectorDialog::BitmapSelector::savebtnclick(void *sender, EventParameters param)
    {
        CreateImgPropSaveDialog();

        imgpropsavedialog->SetFileName(GetFileName(bmp->filename));
        if (imgpropsavedialog->Show(this))
        {

            FILESTD ofstream f(imgpropsavedialog->FullFileName(), std::ifstream::binary);

            f.write(bmp->filedata, bmp->filesize);
        }
    }

    void BitmapSelectorDialog::BitmapSelector::transparentbtnclick(void *sender, EventParameters param)
    {
        Bitmap *nbmp = new Bitmap(bmp->filedata, bmp->filesize);
        transparentcolordialog->selcol = bmp->transparent;
        transparentcolordialog->allownone = true;
        transparentcolordialog->Show(this, nbmp);
        if (transparentcolordialog->Selected())
        {
            Bitmap *tbmp = NULL;
            try
            {
                tbmp = new Bitmap(nbmp->Width(), nbmp->Height());
                tbmp->SetColorKey(transparentcolordialog->selcol, transparentcolordialog->selcol);
                tbmp->Draw(nbmp, 0, 0);
                tbmp->ResetColorKey();
            }
            catch(...)
            {
                delete nbmp;
                delete tbmp;
                return;
            }

            delete bmp->image;
            bmp->image = tbmp;
            bmp->transparent = transparentcolordialog->selcol;

            pb->Invalidate();
        }
        delete nbmp;
    }

    void BitmapSelectorDialog::BitmapSelector::pbpaint(void *sender, PaintParameters param)
    {
        Canvas *c = param.canvas;
        //Gdiplus::TextureBrush tb(bgpattern->GetBitmap());

        Rect cr = pb->ClientRect(); 
        c->SetBrush(bgpattern);
        c->FillRect(pb->ClientRect());
        if (bmp->image)
        {
            Rect r = RectS(0, 0, bmp->image->Width(), bmp->image->Height());
            r.Move((cr.Width() - r.Width()) / 2, (cr.Height() - r.Height()) / 2);
            c->Draw(bmp->image, r.left, r.top);
        }
    }

    void BitmapSelectorDialog::BitmapSelector::formresize(void *sender, EventParameters param)
    {
        pb->Invalidate();
    }


    //---------------------------------------------


    ImagelistEditorDialog::ImagelistEditorDialog() : list(NULL)
    {

    }

    ImagelistEditorDialog::~ImagelistEditorDialog()
    {
        if (list)
            list->Destroy();
    }

    bool ImagelistEditorDialog::Show(Form *topparent, Imagelist *alist)
    {
        if (!alist)
            throw "No imagelist present!";

        if (list)
            list->Destroy();

        list = new Imagelist(*alist);

        ImagelistEditor *editor = new ImagelistEditor(this, list);

        ModalResults mr = base::Show(editor, topparent);

        editor->Destroy();
        return mr == mrOk;
    }

    Imagelist* ImagelistEditorDialog::Images()
    {
        return list;
    }


    //---------------------------------------------


    ImagelistEditorDialog::ImagelistEditor::ImagelistEditor(ImagelistEditorDialog *owner, Imagelist *list) :
            owner(owner), pos(0), list(list)
    {
        SetShowPosition(fspParentMonitorCenter);
        SetBounds(Rect(528, 352, 972, 665));
        SetText(L"Edit the images of the imagelist...");
        //GetFont()->SetFamily(L"Segoe UI");
        SetMinimumWidth(329);
        SetMinimumHeight(282);

        pb = new Paintbox();
        pb->SetBounds(Rect(8, 8, 348, 265));
        pb->SetText(L"pb");
        pb->SetAnchors(caLeft | caTop | caRight | caBottom);
        //pb->GetFont()->SetFamily(L"Segoe UI");
        pb->SetTabOrder(0);
        pb->SetParent(this);
        pb->SetAcceptInput(false);
        pb->SetBorderStyle(bsModern);
        pb->SetDoubleBuffered(true);
        pb->OnPaint = CreateEvent(this, &self::pbpaint);
        pb->OnResize = CreateEvent(this, &self::pbresize);
        pb->OnKeyPush = CreateEvent(this, &self::pbkeypush);
        pb->OnMouseDown = CreateEvent(this, &self::pbmousedown);
        pb->OnScrollOverflow = CreateEvent(this, &self::pbscrolloverflow);
        pb->OnEnter = CreateEvent(this, &self::pbenter);
        pb->OnLeave = CreateEvent(this, &self::pbleave);
        pb->OnDialogCode = CreateEvent(this, &self::pbdlgcode);
        pb->OnScroll = CreateEvent(this, &self::pbscroll);

        btnUp = new Button();
        btnUp->SetBounds(Rect(355, 9, 419, 33));
        btnUp->SetText(L"5");
        btnUp->SetEnabled(false);
        btnUp->SetAnchors(caTop | caRight);
        btnUp->GetFont().SetFamily(L"Webdings");
        btnUp->GetFont().SetSize(12);
        btnUp->SetTabOrder(1);
        btnUp->SetParent(this);
        btnUp->OnMouseDown = CreateEvent(this, &self::upbtnmousedown);

        btnDown = new Button();
        btnDown->SetBounds(Rect(355, 33, 419, 57));
        btnDown->SetText(L"6");
        btnDown->SetEnabled(false);
        btnDown->SetAnchors(caTop | caRight);
        btnDown->GetFont().SetFamily(L"Webdings");
        btnDown->GetFont().SetSize(12);
        btnDown->SetTabOrder(2);
        btnDown->SetParent(this);
        btnDown->OnMouseDown = CreateEvent(this, &self::downbtnmousedown);

        btnLoad = new Button();
        btnLoad->SetBounds(Rect(355, 57, 419, 81));
        btnLoad->SetText(L"Load...");
        btnLoad->SetAnchors(caTop | caRight);
        //btnLoad->GetFont()->SetFamily(L"Segoe UI");
        btnLoad->SetTabOrder(3);
        btnLoad->SetParent(this);
        btnLoad->OnClick = CreateEvent(this, &self::loadbtnclick);

        btnSave = new Button();
        btnSave->SetBounds(Rect(355, 81, 419, 105));
        btnSave->SetText(L"Save...");
        btnSave->SetEnabled(false);
        btnSave->SetAnchors(caTop | caRight);
        //btnSave->GetFont()->SetFamily(L"Segoe UI");
        btnSave->SetTabOrder(4);
        btnSave->SetParent(this);
        btnSave->OnClick = CreateEvent(this, &self::savebtnclick);

        btnDelete = new Button();
        btnDelete->SetBounds(Rect(355, 105, 419, 129));
        btnDelete->SetText(L"Delete");
        btnDelete->SetEnabled(false);
        btnDelete->SetAnchors(caTop | caRight);
        //btnDelete->GetFont()->SetFamily(L"Segoe UI");
        btnDelete->SetTabOrder(5);
        btnDelete->SetParent(this);
        btnDelete->OnClick = CreateEvent(this, &self::deletebtnclick);

        btnClear = new Button();
        btnClear->SetBounds(Rect(355, 129, 419, 153));
        btnClear->SetText(L"Clear");
        btnClear->SetEnabled(false);
        btnClear->SetAnchors(caTop | caRight);
        //btnClear->GetFont()->SetFamily(L"Segoe UI");
        btnClear->SetTabOrder(6);
        btnClear->SetParent(this);
        btnClear->OnClick = CreateEvent(this, &self::clearbtnclick);

        btnOk = new Button();
        btnOk->SetBounds(Rect(355, 171, 419, 195));
        btnOk->SetText(L"OK");
        btnOk->SetAnchors(caTop | caRight);
        //btnOk->GetFont()->SetFamily(L"Segoe UI");
        btnOk->SetTabOrder(7);
        btnOk->SetParent(this);
        btnOk->OnClick = CreateEvent(this, &self::okbtnclick);

        btnCancel = new Button();
        btnCancel->SetBounds(Rect(355, 211, 419, 235));
        btnCancel->SetText(L"Cancel");
        btnCancel->SetAnchors(caTop | caRight);
        //btnCancel->GetFont()->SetFamily(L"Segoe UI");
        btnCancel->SetTabOrder(8);
        btnCancel->SetParent(this);
        btnCancel->SetCancel(true);
        btnCancel->OnClick = CreateEvent(this, &self::cancelbtnclick);

        padding = pb->GetCanvas()->MeasureText(L"wqmL").cy;
        stretch = list->Width() > 64 || list->Height() > 64;
        if (stretch)
        {
            double div = min((double)64 / list->Width(), (double)64 / list->Height());
            thumbw = list->Width() * div + padding * 2;
            thumbh = list->Height() * div + padding * 2;
        }
        else
        {
            thumbw = min(64, list->Width()) + padding * 2;
            thumbh = min(64, list->Height()) + padding * 2;
        }
    }

    ImagelistEditorDialog::ImagelistEditor::~ImagelistEditor() {}

    void ImagelistEditorDialog::ImagelistEditor::Show()
    {
        pb->ScrollResize();
        UpdateButtons();
        base::Show();
    }

    Rect ImagelistEditorDialog::ImagelistEditor::ImageRect(int index)
    {
        Rect r = Rect(0, 0, thumbw, thumbh);
        r.Move((index % cols) * thumbw, (index / cols) * thumbh - thumbh * pb->VPos());
        return r;
    }

    void ImagelistEditorDialog::ImagelistEditor::SetPos(int newpos)
    {
        newpos = max(0, min(list->Count() - 1, newpos));
        if (newpos == pos)
            return;

        if (pb->VPos() * cols > newpos)
        {
            pb->SetVPos(newpos / cols);
        }
        else if (pb->VPos() * cols + rows * cols - 1 < newpos)
        {
            pb->SetVPos( newpos / cols - rows + 1);
        }
        else
        {
            pb->InvalidateRect(ImageRect(pos));
            pos = newpos;
            pb->InvalidateRect(ImageRect(pos));
        }

        UpdateButtons();
    }

    void ImagelistEditorDialog::ImagelistEditor::UpdateButtons()
    {
        btnUp->SetEnabled(pos > 0);
        btnDown->SetEnabled(pos < list->Count() - 1);
        btnSave->SetEnabled(list->Count() != 0);
        btnClear->SetEnabled(list->Count() != 0);
        btnDelete->SetEnabled(list->Count() != 0);
        pb->SetAcceptInput(list->Count() != 0);
    }

    void ImagelistEditorDialog::ImagelistEditor::okbtnclick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }

    void ImagelistEditorDialog::ImagelistEditor::cancelbtnclick(void *sender, EventParameters param)
    {
        SetModalResult(mrCancel);
    }

    void ImagelistEditorDialog::ImagelistEditor::loadbtnclick(void *sender, EventParameters param)
    {
        CreateImgPropOpenDialog();

        Bitmap *tmp = NULL, *tbmp = NULL;
        int newpos = pos;
        try
        {
            if (imgpropopendialog->Show(this))
            {
                tbmp = new Bitmap(imgpropopendialog->FullFileName());

                ImageTypes type = tbmp->GetImageType();
                if (list->Masked() || list->ColorDepth() == icd32bit)
                {
                    if (type == itBMP || type == itTIFF || type == itUndefined || type == itMemoryBitmap)
                    {
                        transparentcolordialog->selcol = list->Masked() ? list->MaskColor() : clNone;
                        transparentcolordialog->allownone = !list->Masked();
                        transparentcolordialog->Show(this, tbmp);
                        if (transparentcolordialog->Selected())
                        {
                            tmp = tbmp;
                            tbmp = new Bitmap(tmp->Width(), tmp->Height());

                            tbmp->SetColorKey(transparentcolordialog->selcol, transparentcolordialog->selcol);
                            if (list->Masked() || list->ColorDepth() != icd32bit)
                                tbmp->Clear(list->MaskColor());
                            tbmp->Draw(tmp, 0, 0);
                            tbmp->ResetColorKey();

                            delete tmp;
                            tmp = NULL;
                        }
                    }
                    else if (list->Masked() && (type == itPNG || type == itGIF))
                    {
                        tmp = tbmp;
                        tbmp = new Bitmap(tmp->Width(), tmp->Height());
                        tbmp->Clear(list->MaskColor());
                        tbmp->Draw(tmp, 0, 0);

                        delete tmp;
                        tmp = NULL;
                    }
                }

                newpos = list->Add(tbmp);

                delete tbmp;
                tbmp = NULL;
            }
        }
        catch(...)
        {
            delete tbmp;
            delete tmp;
        }

        if (newpos >= 0)
            pos = newpos;

        pb->ScrollResize();

        UpdateButtons();
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::savebtnclick(void *sender, EventParameters param)
    {
        CreateImgPropSaveDialog();

        if (imgpropsavedialog->Show(this))
        {
            Bitmap bmp(list->Width(), list->Height());
            list->Draw(&bmp, pos, 0, 0);
            bmp.Save(imgpropsavedialog->FullFileName());
        }
    }

    void ImagelistEditorDialog::ImagelistEditor::clearbtnclick(void *sender, EventParameters param)
    {
        list->Clear();
        pos = 0;
        pb->ScrollResize();

        UpdateButtons();
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::deletebtnclick(void *sender, EventParameters param)
    {
        list->Remove(pos);
        if (pos >= list->Count())
            pos = list->Count() - 1;
        pb->ScrollResize();
        UpdateButtons();
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::upbtnmousedown(void *sender, MouseButtonParameters param)
    {
        list->Move(pos, pos - 1);
        SetPos(pos - 1);
        UpdateButtons();
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::downbtnmousedown(void *sender, MouseButtonParameters param)
    {
        list->Move(pos, pos + 1);
        SetPos(pos + 1);
        UpdateButtons();
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::pbpaint(void *sender, PaintParameters param)
    {
        Canvas *c = param.canvas;
        Rect cr = pb->ClientRect();

        c->SetPen(Color(200, 200, 200));
        //c->SelectStockBrush(sbWindow);
        //c->FillRect(cr);

        Bitmap *tmp = NULL;
        //double div = 1.;
        if (stretch)
            tmp = new Bitmap(list->Width(), list->Height());

        int top = pb->VPos();
        int ix = top * cols;
        int cnt = list->Count();
        int y = 0;
        int x = 0;

        Color bg;

        Color origpc = list->PaintColor();
        CanvasTextAlignmentSet ts = c->TextAlignment();
        c->SetTextAlignment(ctaTop | ctaCenter);
        while (y < cr.Height() && ix < cnt)
        {
            bg = Color(pos == ix ? clHighlight : clWindow).ToRGB();
            if (!pb->Focused() && pos == ix)
                bg = MixColors(bg, clWindow);

            c->SetBrush(bg);
            c->FillRect(RectS(x, y, thumbw, thumbh));
            c->Line(x + thumbw - 1, y, x + thumbw - 1, y + thumbh - 1);
            c->Line(x + thumbw - 1, y + thumbh - 1, x, y + thumbh - 1);

            list->SetPaintColor(bg);

            c->GetFont().SetColor(pos == ix && pb->Focused() ? clHighlightText : clWindowText);
            c->TextDraw(RectS(x, y, thumbw - 1, padding + 2), x + thumbw / 2, y, IntToStr(ix) + L".");

            if (!stretch)
                list->Draw(c, ix, x + padding, y + padding * 1.3);
            else
            {
                tmp->Clear(bg);
                list->Draw(tmp, ix, 0, 0);
                c->Draw(tmp, x + padding, y + padding * 1.3, thumbw  - 2 * padding, thumbh - 2 * padding, 0, 0, tmp->Width(), tmp->Height());
            }

            if (x + thumbw * 2 > cr.right)
            {
                c->SelectStockBrush(sbWindow);
                c->FillRect(Rect(x + thumbw, y, cr.right, y + thumbh));
                x = 0;
                y += thumbw;
            }
            else
                x += thumbw;
            ++ix;
        }
        c->SetTextAlignment(ts);

        c->SelectStockBrush(sbWindow);
        c->FillRect(Rect(x, y, cr.right, y + thumbh));
        c->FillRect(Rect(0, y + thumbh, cr.right, cr.bottom));

        list->SetPaintColor(origpc);

        delete tmp;
    }

    void ImagelistEditorDialog::ImagelistEditor::pbresize(void *sender, EventParameters param)
    {
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::pbscroll(void *sender, ScrollParameters param)
    {
        pb->Invalidate();
    }

    void ImagelistEditorDialog::ImagelistEditor::pbkeypush(void *sender, KeyPushParameters param)
    {
        int i = 0;
        switch(param.keycode)
        {
        case VK_LEFT:
            SetPos(pos - 1);
            break;
        case VK_RIGHT:
            SetPos(pos + 1);
            break;
        case VK_UP:
            if (pos >= cols)
                SetPos(pos - cols);
            break;
        case VK_DOWN:
            if (pos + cols < list->Count())
                SetPos(pos + cols);
            break;
        case VK_PRIOR:
            while (i < rows && pos - cols * i >= cols)
                i++;
            SetPos(pos - cols * i);
            break;
        case VK_NEXT:
            while (i < rows && pos + cols * i < list->Count() - cols)
                i++;
            SetPos(pos + cols * i);
            break;
        case VK_HOME:
            if (param.vkeys.contains(vksCtrl))
                SetPos(0);
            else
                SetPos(pos - pos % cols);
            break;
        case VK_END:
            if (param.vkeys.contains(vksCtrl))
                SetPos(list->Count() - 1);
            else
                SetPos(pos + (cols - (pos % cols)) - 1);
            break;
        }
    }

    void ImagelistEditorDialog::ImagelistEditor::pbmousedown(void *sender, MouseButtonParameters param)
    {
        if (param.button != mbLeft || param.x >= cols * thumbw)
            return;

        int newpos = pb->VPos() * cols + (param.y / thumbh) * cols + param.x / thumbh;
        if (newpos > list->Count())
            return;
        SetPos(newpos);
    }

    void ImagelistEditorDialog::ImagelistEditor::pbscrolloverflow(void *sender, ScrollOverflowParameters param)
    {
        cols = max(1, param.visiblewidth / thumbw);
        rows = max(1, min(param.visibleheight / thumbh, (list->Count() + cols - 1) / cols));
        param.visiblewidth = cols;
        param.visibleheight = rows;
        param.hiddenwidth = 0;
        param.hiddenheight = max(0, (list->Count() - 1) / cols - rows + 1);
    }

    void ImagelistEditorDialog::ImagelistEditor::pbenter(void *sender, ActiveChangedParameters param)
    {
        pb->InvalidateRect(ImageRect(pos));
    }

    void ImagelistEditorDialog::ImagelistEditor::pbleave(void *sender, ActiveChangedParameters param)
    {
        pb->InvalidateRect(ImageRect(pos));
    }

    void ImagelistEditorDialog::ImagelistEditor::pbdlgcode(void *sender, DialogCodeParameters param)
    {
        param.result << dcWantArrows;
    }


    //---------------------------------------------


    PropertyEditorDialog::PropertyEditorDialog()
    {

    }

    PropertyEditorDialog::~PropertyEditorDialog()
    {
    }

    bool PropertyEditorDialog::Show(Form *topparent, PropertyEditorList *editlist)
    {
        PropertyEditorForm *editor = new PropertyEditorForm(this, editlist);

        ModalResults mr = base::Show(editor, topparent);

        editor->Destroy();
        return mr == mrOk;
    }


    //---------------------------------------------


    PropertyEditorDialog::PropertyEditorForm::PropertyEditorForm(PropertyEditorDialog *owner, PropertyEditorList *editlist) : owner(owner), editlist(editlist)
    {
        OnShow = CreateEvent(this, &self::onshow);
        OnHide = CreateEvent(this, &self::onhide);

        SetShowPosition(fspParentMonitorCenter);
        SetBounds(Rect(0, 0, 650, 500));
        SetText(L"Edit objects and properties");
        SetMinimumWidth(440);
        SetMinimumHeight(202);

        lbItems = new Listbox();
        lbItems->SetBounds(Rect(8, 32, 192, 430));
        lbItems->SetText(L"lbItems");
        lbItems->SetBorderStyle(bsNormal);
        lbItems->SetAnchors(caLeft | caTop | caBottom);
        lbItems->SetTabOrder(0);
        lbItems->SetIntegralHeight(false);
        lbItems->SetParent(this);
        lbItems->SetKind(lckOwnerDraw);
        lbItems->OnChanged = CreateEvent(this, &self::lbitemschanged);
        lbItems->OnDrawItem = CreateEvent(this, &self::lbitemsdraw);
        lbItems->OnKeyPush = CreateEvent(this, &self::lbitemskeypush);

        fbUp = new FlatButton();
        fbUp->SetBounds(Rect(195, 32, 220, 57));
        fbUp->SetTabOrder(1);
        fbUp->SetParent(this);
        fbUp->SetTag(-1);
        fbUp->SetImagePosition(bipCenter);
        fbUp->Image()->SetImages(ilButtons);
        fbUp->Image()->SetImageIndex(5);
        fbUp->OnMouseDown = CreateEvent(this, &self::fbupdownmousedown);

        fbDown = new FlatButton();
        fbDown->SetBounds(Rect(195, 60, 220, 85));
        fbDown->SetTabOrder(2);
        fbDown->SetParent(this);
        fbDown->SetTag(1);
        fbDown->SetImagePosition(bipCenter);
        fbDown->Image()->SetImages(ilButtons);
        fbDown->Image()->SetImageIndex(6);
        fbDown->OnMouseDown = CreateEvent(this, &self::fbupdownmousedown);

        fbAdd = new FlatButton();
        fbAdd->SetBounds(Rect(9, 5, 34, 30));
        fbAdd->SetTabOrder(3);
        fbAdd->SetParent(this);
        fbAdd->SetImagePosition(bipCenter);
        fbAdd->Image()->SetImages(ilButtons);
        fbAdd->Image()->SetImageIndex(2);
        fbAdd->OnClick = CreateEvent(this, &self::fbaddclick);

        fbAddSub = new FlatButton();
        fbAddSub->SetBounds(Rect(36, 5, 61, 30));
        fbAddSub->SetTabOrder(4);
        fbAddSub->SetParent(this);
        fbAddSub->SetImagePosition(bipCenter);
        fbAddSub->Image()->SetImages(ilButtons);
        fbAddSub->Image()->SetImageIndex(4);
        fbAddSub->OnClick = CreateEvent(this, &self::fbaddsubclick);

        fbDelete = new FlatButton();
        fbDelete->SetBounds(Rect(167, 5, 192, 30));
        fbDelete->SetTabOrder(5);
        fbDelete->SetParent(this);
        fbDelete->SetImagePosition(bipCenter);
        fbDelete->Image()->SetImages(ilButtons);
        fbDelete->Image()->SetImageIndex(3);
        fbDelete->OnClick = CreateEvent(this, &self::fbdeleteclick);

        pcProp = new PageControl();
        pcProp->SetBounds(Rect(224, 8, 626, 430));
        pcProp->SetText(L"pcProp");
        pcProp->SetAnchors(caLeft | caTop | caRight | caBottom);
        pcProp->SetTabOrder(6);
        pcProp->SetAcceptInput(false);
        pcProp->SetParent(this);

        tpProperties = pcProp->AddTabPage(L"Properties");
        tpEvents = pcProp->AddTabPage(L"Events");

        pProp = new Panel();
        pProp->SetBounds(Rect(0, 0, 198, 27));
        pProp->SetAlignment(alTop);
        pProp->SetTabOrder(0);
        pProp->SetShowText(false);
        pProp->SetInnerBorderStyle(pbsNone);
        pProp->SetParent(tpProperties);

        pEvents = new Panel();
        pEvents->SetBounds(Rect(0, 0, 198, 27));
        pEvents->SetAlignment(alTop);
        pEvents->SetTabOrder(0);
        pEvents->SetShowText(false);
        pEvents->SetInnerBorderStyle(pbsNone);
        pEvents->SetParent(tpEvents);

        fbPropAbc = new FlatButton();
        fbPropAbc->SetBounds(Rect(2, 1, 27, 26));
        fbPropAbc->SetTabOrder(0);
        fbPropAbc->SetGroupIndex(1);
        fbPropAbc->SetType(fbtRadiobutton);
        fbPropAbc->SetDown(true);
        fbPropAbc->SetImagePosition(bipCenter);
        fbPropAbc->Image()->SetImages(ilButtons);
        fbPropAbc->Image()->SetImageIndex(7);
        fbPropAbc->OnClick = CreateEvent(this, &self::fbabcclick);
        fbPropAbc->SetParent(pProp);

        fbPropCat = new FlatButton();
        fbPropCat->SetBounds(Rect(29, 1, 54, 26));
        fbPropCat->SetTabOrder(1);
        fbPropCat->SetGroupIndex(1);
        fbPropCat->SetType(fbtRadiobutton);
        fbPropCat->SetTag(1);
        fbPropCat->SetImagePosition(bipCenter);
        fbPropCat->Image()->SetImages(ilButtons);
        fbPropCat->Image()->SetImageIndex(8);
        fbPropCat->OnClick = CreateEvent(this, &self::fbabcclick);
        fbPropCat->SetParent(pProp);

        fbEventAbc = new FlatButton();
        fbEventAbc->SetBounds(Rect(2, 1, 27, 26));
        fbEventAbc->SetTabOrder(0);
        fbEventAbc->SetGroupIndex(1);
        fbEventAbc->SetType(fbtRadiobutton);
        fbEventAbc->SetDown(true);
        fbEventAbc->SetImagePosition(bipCenter);
        fbEventAbc->Image()->SetImages(ilButtons);
        fbEventAbc->Image()->SetImageIndex(7);
        fbEventAbc->OnClick = CreateEvent(this, &self::fbabcclick);
        fbEventAbc->SetParent(pEvents);

        fbEventCat = new FlatButton();
        fbEventCat->SetBounds(Rect(29, 1, 54, 26));
        fbEventCat->SetTabOrder(1);
        fbEventCat->SetGroupIndex(1);
        fbEventCat->SetType(fbtRadiobutton);
        fbEventCat->SetTag(1);
        fbEventCat->SetImagePosition(bipCenter);
        fbEventCat->Image()->SetImages(ilButtons);
        fbEventCat->Image()->SetImageIndex(8);
        fbEventCat->OnClick = CreateEvent(this, &self::fbabcclick);
        fbEventCat->SetParent(pEvents);

        lbProp = new PropertyListbox(pltValues, fbEventAbc, fbEventCat);
        lbProp->SetBounds(Rect(0, 27, 198, 226));
        lbProp->SetBorderStyle(bsNone);
        lbProp->SetAnchors(caLeft | caTop | caRight | caBottom);
        lbProp->SetAlignment(alClient);
        lbProp->SetTabOrder(1);
        lbProp->SetParent(tpProperties);

        lbEvents = new PropertyListbox(pltEvents, fbEventAbc, fbEventCat);
        lbEvents->SetBounds(Rect(0, 27, 198, 226));
        lbEvents->SetBorderStyle(bsNone);
        lbEvents->SetAnchors(caLeft | caTop | caRight | caBottom);
        lbEvents->SetAlignment(alClient);
        lbEvents->SetTabOrder(0);
        lbEvents->SetParent(tpEvents);

        btnOk = new Button();
        btnOk->SetBounds(Rect(241, 434, 321, 458));
        btnOk->SetText(lt.GetTranslationFor(FCT_OK));
        btnOk->SetAnchors(caBottom);
        btnOk->SetTabOrder(7);
        btnOk->SetParent(this);
        btnOk->OnClick = CreateEvent(this, &self::btnokclick);

        btnCancel = new Button();
        btnCancel->SetBounds(Rect(547, 434, 627, 458));
        btnCancel->SetText(lt.GetTranslationFor(FCT_CANCEL));
        btnCancel->SetAnchors(caBottom | caRight);
        btnCancel->SetTabOrder(8);
        btnCancel->SetParent(this);
        btnCancel->SetCancel(true);
        btnCancel->OnClick = CreateEvent(this, &self::btncancelclick);

        // Make lbItems hold data about the items in the editor. If the data is a positive number, it is the index of a main item,
        // negative numbers represent index of sub items. The main item is the closest positive number above in the list.
        int cnt = editlist->Count();
        for (int ix = 0; ix < cnt; ++ix)
        {
            lbItems->Items().Add(editlist->Texts(ix, -1), (void*)ix);
            int subcnt = editlist->SubCount(ix);
            for (int iy = 1; iy < subcnt + 1; ++iy)
                lbItems->Items().Add(editlist->Texts(ix, iy - 1), (void*)-iy);
        }
        if (cnt)
        {
            lbItems->SetItemIndex(0, true);
            lbitemschanged(lbItems, EventParameters());
        }

        UpdateButtons();
    }

    PropertyEditorDialog::PropertyEditorForm::~PropertyEditorForm()
    {
    }

    void PropertyEditorDialog::PropertyEditorForm::Destroy()
    {
        //int cnt = lbItems->ItemCount();
        //for (int ix = 0; ix < cnt; ++ix)
        //    delete ((std::pair<int, int>*)lbItems->ItemData(ix));
        base::Destroy();
    }

    void PropertyEditorDialog::PropertyEditorForm::UpdateButtons()
    {
        int sel = lbItems->ItemIndex();

        int pos = sel;
        int sub = -1;
        if (sel >= 0)
            ItemPos(pos, sub);

        PropertyEditorDialogButtonSet btn = editlist->Buttons();

        fbAdd->SetEnabled(btn.contains(pedbAdd));
        fbUp->SetEnabled(btn.contains(pedbMove) && sel > 0 && sub != 0);
        fbDown->SetEnabled(btn.contains(pedbMove) && sel >= 0 && sel < lbItems->Items().Count() - 1 && ((sub < 0 && pos < editlist->Count() - 1) || (sub >= 0 && sub < editlist->SubCount(pos) - 1)));
        fbDelete->SetEnabled(btn.contains(pedbDelete) && sel >= 0 && editlist->CanDelete(pos, sub));
        fbAddSub->SetEnabled(btn.contains(pedbAdd) && btn.contains(pedbAddSub) && sel >= 0);
    }

    void PropertyEditorDialog::PropertyEditorForm::ItemPos(int &pos, int &subpos)
    {
        if (pos < 0 || pos >= lbItems->Count())
            throw L"Item index out of bounds.";

        int dat = (int)lbItems->ItemData(pos);
        if (dat < 0)
        {
            pos = (int)lbItems->ItemData(pos + dat);
            subpos = -1 - dat;
        }
        else
        {
            pos = dat;
            subpos = -1;
        }
    }

    int PropertyEditorDialog::PropertyEditorForm::ListPos(int itemindex)
    {
        int ix = 0;
        int cnt = lbItems->Count();
        while(itemindex > 0)
        {
            ++ix;
            if (ix == cnt)
            {
                if (itemindex != 1)
                    throw L"Invalid parameter.";
                break;
            }
            if ((int)lbItems->ItemData(ix) >= 0)
                --itemindex;
        }
        return ix;
    }

    void PropertyEditorDialog::PropertyEditorForm::onshow(void *sender, EventParameters param)
    {
        designer->ReplaceDesignerBoxes(lbProp, lbEvents);
    }

    void PropertyEditorDialog::PropertyEditorForm::onhide(void *sender, EventParameters param)
    {
        designer->RestoreDesignerBoxes();
        editlist->Finished(ModalResult() != mrOk);
    }

    void PropertyEditorDialog::PropertyEditorForm::btnokclick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }

    void PropertyEditorDialog::PropertyEditorForm::btncancelclick(void *sender, EventParameters param)
    {
        SetModalResult(mrCancel);
    }

    void PropertyEditorDialog::PropertyEditorForm::fbabcclick(void *sender, EventParameters param)
    {
        FlatButton *fb = (FlatButton*)sender;
        lbProp->Sort(fb->Tag() == 1);
        lbEvents->Sort(fb->Tag() == 1);
        fbPropAbc->SetDown(fb->Tag() == 0);
        fbPropCat->SetDown(fb->Tag() == 1);
        fbEventAbc->SetDown(fb->Tag() == 0);
        fbEventCat->SetDown(fb->Tag() == 1);
    }

    void PropertyEditorDialog::PropertyEditorForm::fbupdownmousedown(void *sender, MouseButtonParameters param)
    {
        if (param.button != mbLeft)
            return;

        MouseRepeat(fbUp->OnMouseDown, (Control*)sender, param);

        int dif = ((FlatButton*)sender)->Tag();

        int sel = lbItems->ItemIndex();
        int pos = sel;
        int sub = -1;
        ItemPos(pos, sub);
        editlist->Move(pos, sub, dif);

        lbItems->BeginUpdate();
        if (sub < 0)
        {
            int movecnt = editlist->SubCount(pos + dif) + 1; // Number of the main item and sub items being moved.
            int difcnt = editlist->SubCount(pos) + 1; // Number of the main item and sub items of the item which was taking the moved item's place before.

            if (dif < 0)
                sel -= difcnt;

            int p = (int)lbItems->ItemData(sel), dat = p, s = -1;
            for (int ix = 0; ix < movecnt + difcnt; ++ix)
            {
                if (ix == (dif < 0 ? movecnt : difcnt))
                {
                    ++p;
                    s = -1;
                    dat = p;
                }
                else if (ix > 0)
                {
                    ++s;
                    dat = -s - 1;
                }

                lbItems->Items().SetText(sel + ix, editlist->Texts(p, s));
                lbItems->Items().SetData(sel + ix, (void*)dat);
            }

            if (dif > 0)
                sel += difcnt;
        }
        else
        {
            lbItems->Items().SetText(sel, editlist->Texts(pos, sub));
            lbItems->Items().SetText(sel + dif, editlist->Texts(pos, sub + dif));
            sel += dif;
        }

        lbItems->EndUpdate();
        lbItems->SetItemIndex(sel, true);

        UpdateButtons();
    }

    void PropertyEditorDialog::PropertyEditorForm::fbaddclick(void *sender, EventParameters param)
    {
        int ipos = editlist->Add();
        if (ipos < 0)
            return;

        int subcnt = editlist->SubCount(ipos);

        lbItems->BeginUpdate();

        int pos = ListPos(ipos);

        lbItems->Items().Insert(pos, editlist->Texts(ipos, -1), (void*)ipos);
        for (int ix = 0; ix < subcnt; ++ix)
            lbItems->Items().Insert(pos + ix + 1, editlist->Texts(ipos, ix), (void*)(-ix - 1));
        int cnt = lbItems->Count();
        int p = 0;
        for (int ix = pos + 1 + subcnt; ix < cnt; ++ix) // Update the text of everything below, just in case it depended on the order of items, and update the data stored
        {
            int dat = (int)lbItems->ItemData(ix);
            if (dat >= 0)
            {
                lbItems->SetItemData(ix, (void*)++dat);
                p = (int)lbItems->ItemData(ix); //p = dat;
                dat = -1;
            }
            else
                dat = -1 - dat;
            lbItems->Items().SetText(ix, editlist->Texts(p, dat));
        }

        lbItems->SetItemIndex(pos, true);
        lbItems->EndUpdate();

        UpdateButtons();
    }

    void PropertyEditorDialog::PropertyEditorForm::fbaddsubclick(void *sender, EventParameters param)
    {
        int sel = lbItems->ItemIndex();
        int pos = sel;
        int sub = -1;
        ItemPos(pos, sub);

        int newsub = editlist->AddSub(pos); 
        if (newsub < 0)
            return;

        int subcnt = editlist->SubCount(pos);

        lbItems->BeginUpdate();
        lbItems->Items().Insert(sel - (sub < 0 ? 0 : sub + 1) + subcnt, L"", (void*)(-subcnt));

        sel += newsub - sub;
        for (int ix = newsub; ix < subcnt; ++ix)
            lbItems->SetItemText(sel + ix - newsub, editlist->Texts(pos, ix));

        lbItems->SetItemIndex(sel, true);
        lbItems->EndUpdate();

        UpdateButtons();
    }

    void PropertyEditorDialog::PropertyEditorForm::fbdeleteclick(void *sender, EventParameters param)
    {
        int sel = lbItems->ItemIndex();
        int pos = sel;
        int sub = -1;
        ItemPos(pos, sub);

        int subcnt = editlist->SubCount(pos) + 1;

        lbItems->BeginUpdate();

        if (sub < 0)
        {
            editlist->Delete(pos, -1);
            for (int ix = 0; ix < subcnt; ++ix)
                lbItems->Items().Delete(sel);
            int cnt = lbItems->Count();
            int p = 0;
            for (int ix = sel; ix < cnt; ++ix)
            {
                int dat = (int)lbItems->ItemData(ix);
                if (dat >= 0)
                {
                    lbItems->SetItemData(ix, (void*)--dat); 
                    p = dat;
                    dat = -1;
                }
                else
                    dat = -1 - dat;
                lbItems->SetItemText(ix, editlist->Texts(p, dat));
            }
        }
        else
        {
            --subcnt;
            editlist->Delete(pos, sub);
            lbItems->Items().Delete(sel - sub - 1 + subcnt);
            --subcnt;
            for (int ix = sub; ix < subcnt; ++ix)
                lbItems->SetItemText(sel, editlist->Texts(pos, ix));
        }

        lbItems->SetItemIndex(sel, true);
        lbItems->EndUpdate();

        UpdateButtons();
    }

    void PropertyEditorDialog::PropertyEditorForm::lbitemschanged(void *sender, EventParameters param)
    {
        int sel = lbItems->ItemIndex();
        if (sel < 0)
        {
            lbProp->SetProperties(NULL);
            lbEvents->SetProperties(NULL);
            return;
        }

        int s = (int)lbItems->ItemData(sel);
        int p;
        if (s >= 0)
        {
            p = s;
            s = -1;
        }
        else
        {
            p = (int)lbItems->ItemData(sel + s);
            s = -1 - s;
        }
        auto dat = editlist->Objects(p, s);
        lbProp->SetProperties(dat);
        lbEvents->SetProperties(dat);

        UpdateButtons();
    }

    void PropertyEditorDialog::PropertyEditorForm::lbitemsdraw(void *sender, DrawItemParameters param)
    {
        Canvas *c = param.canvas;
        c->FillRect(param.rect);

        int sel = param.index;
        int pos = sel;
        int sub = -1;
        ItemPos(pos, sub);

        int indent = 4 * Scaling;
        if (sub >= 0)
            indent += param.rect.Height(); 

        std::wstring str = editlist->Texts(pos, sub);
        if (str.empty())
        {
            str = L"No text";
            c->GetFont().SetColor(c->GetBrush().GetColor().Mix(clWindowText));
        }
        Size s = c->MeasureText(str);
        c->TextDraw(param.rect, param.rect.left + indent, param.rect.top + (param.rect.Height() - s.cy) / 2, str);
    }

    void PropertyEditorDialog::PropertyEditorForm::lbitemskeypush(void *sender, KeyPushParameters param)
    {
        if (param.key >= 0x20)
        {
            if (pcProp->ActivePageIndex() == 0 || pcProp->ActivePageIndex() > 2)
                lbProp->EditorKeyPress(param.key);
            else
                lbEvents->EditorKeyPress(param.key);
            param.key = 0;
        }
    }


    //---------------------------------------------


    bool ShowFileExtensionDialog(Form *topparent, std::vector<std::pair<std::wstring, std::wstring>> &filters)
    {
        FileExtensionDialog *fed = new FileExtensionDialog();
        bool ok = fed->Show(topparent, filters);
        delete fed;
        return ok;
    }


    //---------------------------------------------


    FileExtensionDialog::FileExtensionDialog()
    {
    }

    FileExtensionDialog::~FileExtensionDialog()
    {
    }

    bool FileExtensionDialog::Show(Form *topparent, std::vector<std::pair<std::wstring, std::wstring>> &afilters)
    {
        FileExtensionForm *editor = new FileExtensionForm(this, afilters);

        ModalResults mr = base::Show(editor, topparent);
        if (mr == mrOk)
            editor->CopyFilters(afilters);

        editor->Destroy();
        return mr == mrOk;
    }


    //---------------------------------------------


    FileExtensionDialog::FileExtensionForm::FileExtensionForm(FileExtensionDialog *owner, const std::vector<std::pair<std::wstring, std::wstring>> &filters) : owner(owner), /*filters(filters),*/ handleedited(true)
    {
        SetShowPosition(fspParentMonitorCenter);
        SetBounds(Rect(615, 350, 1034, 661));
        SetText(L"File extension filters");
        SetMinimumWidth(196);
        SetMinimumHeight(140);

        sgFilters = new StringGrid();
        sgFilters->SetBounds(Rect(8, 8, 372, 236));
        sgFilters->SetAnchors(caLeft | caTop | caRight | caBottom);
        sgFilters->SetTabOrder(0);
        sgFilters->SetColCount(2);
        sgFilters->SetFixedColCount(0);
        sgFilters->SetRowCount(filters.size() + 2);
        for (auto it = filters.begin(); it != filters.end(); ++it)
        {
            sgFilters->SetString(0, (it - filters.begin()) + 1, (*it).first);
            sgFilters->SetString(1, (it - filters.begin()) + 1, (*it).second);
        }
        sgFilters->SetColWidth(0, 180);
        sgFilters->SetColWidth(1, 180);
        sgFilters->SetRowHeight(0, 21 * Scaling);
        sgFilters->SetDefColWidth(0, 180);
        sgFilters->SetDefColWidth(1, 180);
        sgFilters->SetColumnsResizable(true);
        sgFilters->SetDefaultColWidth(999999);
        sgFilters->SetString(0, 0, L"Description");
        sgFilters->SetString(1, 0, L"Filter");
        sgFilters->SetOptions(goVertLines | goHorzLines | goLinesFillVert | goLinesFillHorz);
        sgFilters->SetHLineStep(sgFilters->DefaultRowHeight() * 0.5);
        sgFilters->SetSelectionKind(gskNoSelect);
        sgFilters->SetAutoEdit(true);
        sgFilters->SetSmoothHorzScroll(true);
        sgFilters->SetParent(this);
        sgFilters->OnBeginCellEdit = CreateEvent(this, &self::filtersbeginedit);
        sgFilters->OnCellEdited = CreateEvent(this, &self::filtersedited);
        sgFilters->OnEditorKeyPush = CreateEvent(this, &self::filterseditkeypush);
        sgFilters->OnMouseDown = CreateEvent(this, &self::filtersmousedown);
        sgFilters->OnMouseMove = CreateEvent(this, &self::filtersmousemove);
        sgFilters->OnEnter = CreateEvent(this, &self::filtersenter);
        sgFilters->OnEditorDialogCode = CreateEvent(this, &self::filterseditdlgcode);

    //#ifdef DEBUG
    //    sgFilters->SetColumnDrag(true);
    //#endif

        btnUp = new FlatButton();
        btnUp->SetBounds(Rect(375, 7, 400, 32));
        btnUp->SetAnchors(caTop | caRight);
        btnUp->SetTabOrder(1);
        btnUp->SetParent(this);
        btnUp->SetImagePosition(bipCenter);
        btnUp->Image()->SetImages(ilButtons);
        btnUp->Image()->SetImageIndex(5);
        btnUp->OnClick = CreateEvent(this, &self::btnupclick);

        btnDown = new FlatButton();
        btnDown->SetBounds(Rect(375, 33, 400, 58));
        btnDown->SetAnchors(caTop | caRight);
        btnDown->SetTabOrder(2);
        btnDown->SetParent(this);
        btnDown->SetImagePosition(bipCenter);
        btnDown->Image()->SetImages(ilButtons);
        btnDown->Image()->SetImageIndex(6);
        btnDown->OnClick = CreateEvent(this, &self::btndownclick);

        btnDel = new FlatButton();
        btnDel->SetBounds(Rect(375, 59, 400, 84));
        btnDel->SetAnchors(caTop | caRight);
        btnDel->SetTabOrder(3);
        btnDel->SetParent(this);
        btnDel->SetImagePosition(bipCenter);
        btnDel->Image()->SetImages(ilButtons);
        btnDel->Image()->SetImageIndex(3);
        btnDel->OnClick = CreateEvent(this, &self::btndelclick);

        Panel1 = new Panel();
        Panel1->SetBounds(Rect(120, 242, 280, 268));
        Panel1->SetAnchors(caBottom);
        Panel1->SetTabOrder(4);
        Panel1->SetShowText(false);
        Panel1->SetInnerBorderStyle(pbsNone);
        Panel1->SetParent(this);

        btnOk = new Button();
        btnOk->SetBounds(Rect(0, 1, 80, 25));
        btnOk->SetText(L"Ok");
        btnOk->SetTabOrder(0);
        btnOk->SetParent(Panel1);
        btnOk->OnClick = CreateEvent(this, &self::btnokclick);

        btnCancel = new Button();
        btnCancel->SetBounds(Rect(80, 1, 160, 25));
        btnCancel->SetText(L"Cancel");
        btnCancel->SetTabOrder(1);
        btnCancel->SetParent(Panel1);
        btnCancel->SetCancel(true);
        btnCancel->OnClick = CreateEvent(this, &self::btncancelclick);

        //SetActiveControl(lgFilters);
    }

    FileExtensionDialog::FileExtensionForm::~FileExtensionForm()
    {
    }

    void FileExtensionDialog::FileExtensionForm::Show()
    {
        base::Show();
        sgFilters->EditCell(0, sgFilters->RowCount() - 1);
    }

    void FileExtensionDialog::FileExtensionForm::UpdateButtons(const Point &pt)
    {
        int cnt = sgFilters->RowCount() - 1;

        btnUp->SetEnabled(pt.y > 1 && pt.y < cnt);
        btnDown->SetEnabled(pt.y > 0 && pt.y < cnt - 1);
        btnDel->SetEnabled(pt.y > 0 && pt.y < cnt);
    }

    bool FileExtensionDialog::FileExtensionForm::EmptyRow(int row)
    {
        if (row < 1 || row >= sgFilters->RowCount())
            return true;
        Point pt = sgFilters->CellEdited();
        if (pt.y != row || !sgFilters->Editing())
            return sgFilters->String(0, row).empty() && sgFilters->String(1, row).empty();
        return (pt.x == 0 ? sgFilters->EditorText() : sgFilters->String(0, row)).empty() && (pt.x == 1 ? sgFilters->EditorText() : sgFilters->String(1, row)).empty();
    }

    void FileExtensionDialog::FileExtensionForm::filtersbeginedit(void *sender, BeginCellEditParameters param)
    {
        origstr = sgFilters->String(param.col, param.row);
        lastedited = Point(param.col, param.row);
        UpdateButtons(lastedited);
    }

    void FileExtensionDialog::FileExtensionForm::filtersedited(void *sender, CellEditedParameters param)
    {
        if (!handleedited)
            return;

        bool empty = param.text.empty() && (param.col == 0 ? sgFilters->String(1, param.row).empty() : sgFilters->String(0, param.row).empty());
        if (empty && param.row != sgFilters->RowCount() - 1)
        {
            param.allowupdate = false;
            sgFilters->DeleteRow(param.row);
        }
        else if (!empty && param.row == sgFilters->RowCount() - 1)
            sgFilters->SetRowCount(sgFilters->RowCount() + 1);
    }

    void FileExtensionDialog::FileExtensionForm::filterseditkeypush(void *sender, KeyPushParameters param)
    {
        int pos, len;
        Point p;
        bool b;

        handleedited = false;
        switch (param.keycode)
        {
        case VK_TAB:
            p = sgFilters->CellEdited();
            if (param.vkeys.contains(vksShift))
            {
                if (p.x == 0 && p.y == 1)
                {
                    TabNext(sgFilters, false)->Focus();
                    break;
                }

                if (p.x == 1)
                    sgFilters->EditCell(0, p.y);
                else
                {
                    b = EmptyRow(p.y);
                    if (b && p.y != sgFilters->RowCount() - 1)
                        sgFilters->DeleteRow(p.y);
                    sgFilters->EditCell(1, p.y - 1);
                }
                len = sgFilters->EditorText().length();
                sgFilters->SetEditorSelStartAndLength(0, len);
                param.keycode = 0;
            }
            else
            {
                b = EmptyRow(p.y);
                if (b && p.x == 1)
                {
                    if (p.y == sgFilters->RowCount() - 1)
                    {
                        TabNext(sgFilters, true)->Focus();
                        break;
                    }
                    sgFilters->DeleteRow(p.y);
                    sgFilters->EditCell(0, p.y);
                }
                else
                {
                    if (p.x == 1 && p.y == sgFilters->RowCount() - 1)
                        sgFilters->SetRowCount(sgFilters->RowCount() + 1);
                    sgFilters->EditCell(p.x == 0 ? 1 : 0, p.x == 0 ? p.y : p.y + 1);
                }
                len = sgFilters->EditorText().length();
                sgFilters->SetEditorSelStartAndLength(0, len);
                param.keycode = 0;
            }
            break;
        case VK_LEFT:
            p = sgFilters->CellEdited();
            sgFilters->EditorSelStartAndLength(pos, len);
            break;
        case VK_RIGHT:
            p = sgFilters->CellEdited();
            sgFilters->EditorSelStartAndLength(pos, len);
            break;
        case VK_UP:
            p = sgFilters->CellEdited();
            if (p.y == 0)
                break;

            if (p.y != sgFilters->RowCount() - 1 && EmptyRow(p.y))
                sgFilters->DeleteRow(p.y);

            sgFilters->EditCell(p.x, p.y - 1);
            param.keycode = 0;
            break;
        case VK_DOWN:
            p = sgFilters->CellEdited();
            b = EmptyRow(p.y);
            if (!b && p.y == sgFilters->RowCount() - 1)
                sgFilters->SetRowCount(sgFilters->RowCount() + 1);
            else if (p.y == sgFilters->RowCount() - 1)
                break;
            if (b)
                sgFilters->DeleteRow(p.y);
            sgFilters->EditCell(p.x, p.y + (b ? 0 : 1));

            param.keycode = 0;
            break;
        }

        handleedited = true;

        if (param.key == VK_ESCAPE)
        {
            param.key = 0;
            sgFilters->SetEditorText(origstr);
        }
        if (param.key == VK_TAB)
            param.key = 0;
    }

    void FileExtensionDialog::FileExtensionForm::filterseditdlgcode(void *sender, DialogCodeParameters param)
    {
        param.result << dcWantTab;
    }

    void FileExtensionDialog::FileExtensionForm::filtersmousedown(void *sender, MouseButtonParameters param)
    {
        GridPosition pos = sgFilters->PositionAt(param.x, param.y);
        if (pos.type == gptAfterCells)
        {
            if (pos.row <= 0)
                pos.row = sgFilters->RowCount() - 1;
            if (pos.col < 0)
                pos.col = 1;
        }

        if (pos.type == gptCell || pos.type == gptAfterCells)
            sgFilters->EditCell(pos.col, pos.row);
    }

    void FileExtensionDialog::FileExtensionForm::filtersmousemove(void *sender, MouseMoveParameters param)
    {
        GridPosition pos = sgFilters->PositionAt(param.x, param.y);
        if (pos.type != gptNowhere && pos.type != gptFixed && pos.type != gptVertEdge && sgFilters->MouseAction() == gmaNone)
        {
            sgFilters->SetCursor(cIBeam);
            screencursor->Set(cIBeam);
        }
        else
            sgFilters->SetCursor(cDefault);
    }

    void FileExtensionDialog::FileExtensionForm::filtersenter(void *sender, ActiveChangedParameters param)
    {
        if (!param.tabactivate || !param.other)
            ;//sgFilters->EditCell(lastedited.x, lastedited.y);
        else
        {
            if (param.other == TabLast())
               sgFilters->EditCell(0, 1);
            else
               sgFilters->EditCell(1, sgFilters->RowCount() - 1);
        }
    }

    void FileExtensionDialog::FileExtensionForm::btnokclick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }

    void FileExtensionDialog::FileExtensionForm::btncancelclick(void *sender, EventParameters param)
    {
        SetModalResult(mrCancel);
    }

    void FileExtensionDialog::FileExtensionForm::CopyFilters(std::vector<std::pair<std::wstring, std::wstring>> &dest)
    {
        dest.clear();
        int cnt = sgFilters->RowCount() - (EmptyRow(sgFilters->RowCount() - 1) ? 1 : 0);
        for (int ix = 1; ix < cnt; ++ix)
        {
            //ListGridRow *row = sgFilters->Rows(ix);
            dest.push_back(std::pair<std::wstring, std::wstring>(sgFilters->String(0, ix), sgFilters->String(1, ix)));
        }
    }

    void FileExtensionDialog::FileExtensionForm::btnupclick(void *sender, EventParameters param)
    {
        Point p = sgFilters->CellEdited();
        if (p.y == sgFilters->RowCount() - 1 || p.y == 0)
            return;
        int sel, len;
        sgFilters->EditorSelStartAndLength(sel, len);
        sgFilters->SwapRowData(p.y, p.y - 1);
        //sgFilters->MoveRow(p.y, p.y - 1);
        sgFilters->EditCell(p.x, p.y - 1);
        sgFilters->SetEditorSelStartAndLength(sel, len);
    }

    void FileExtensionDialog::FileExtensionForm::btndownclick(void *sender, EventParameters param)
    {
        Point p = sgFilters->CellEdited();
        if (p.y >= sgFilters->RowCount() - 2)
            return;
        int sel, len;
        sgFilters->EditorSelStartAndLength(sel, len);
        sgFilters->SwapRowData(p.y, p.y + 1);
        //sgFilters->MoveRow(p.y, p.y + 1);
        sgFilters->EditCell(p.x, p.y + 1);
        sgFilters->SetEditorSelStartAndLength(sel, len);
    }

    void FileExtensionDialog::FileExtensionForm::btndelclick(void *sender, EventParameters param)
    {
        Point p = sgFilters->CellEdited();
        if (p.y == sgFilters->RowCount() - 1)
            return;
        sgFilters->DeleteRow(p.y);
        sgFilters->EditCell(p.x, min(sgFilters->RowCount() - 1, p.y));
    }


    //---------------------------------------------


    IconSelectorDialog::IconSelectorDialog()
    {
    }

    IconSelectorDialog::~IconSelectorDialog()
    {
    }

    bool IconSelectorDialog::Show(Form *topparent, const IconData &aicon)
    {
        icon = aicon;
        IconSelector *selector = new IconSelector(this, icon);

        ModalResults mr = base::Show(selector, topparent);

        selector->Destroy();
        return mr == mrOk;
    }

    IconData& IconSelectorDialog::GetIcon()
    {
        return icon;
    }


    //---------------------------------------------


    IconSelectorDialog::IconSelector::IconSelector(IconSelectorDialog *owner, IconData &icon) : owner(owner), icon(icon), prevcols(-1), sel(-1)
    {
        /* Generated member initialization. Do not modify. */
        //SetLeft(805);
        //SetTop(473);
        SetText(L"Icon selector");
        SetMinimumWidth(430);
        SetMinimumHeight(160);
        SetShowPosition(fspParentMonitorCenter);
        SetBorderButtons(0);
        SetClientRect(Rect(0, 0, 414, 202));

        dlgOpen = new OpenDialog();
        dlgOpen->SetDefaultExtension(L"ico");
        dlgOpen->SetTitle(L"Load icon");
        dlgOpen->SetFilterString(L"Icons (*.ico)|*.ico");

        dlgSave = new SaveDialog();
        dlgSave->SetDefaultExtension(L"ico");
        dlgSave->SetTitle(L"Save icon");
        dlgSave->SetFilterString(L"Icons (*.ico)|*.ico");

        pb = new Paintbox();
        pb->SetBounds(Rect(5, 6, 409, 170));
        pb->SetBorderStyle(bsModern);
        pb->SetAnchors(caLeft | caTop | caRight | caBottom);
        pb->SetTabOrder(0);
        pb->SetAcceptInput(true);
        pb->SetParent(this);

        btnLoad = new Button();
        btnLoad->SetBounds(Rect(5, 174, 77, 198));
        btnLoad->SetText(L"Load...");
        btnLoad->SetAnchors(caLeft | caBottom);
        btnLoad->SetTabOrder(1);
        btnLoad->SetParent(this);

        btnSave = new Button();
        btnSave->SetBounds(Rect(151, 174, 223, 198));
        btnSave->SetText(L"Save...");
        btnSave->SetEnabled(false);
        btnSave->SetAnchors(caLeft | caBottom);
        btnSave->SetTabOrder(3);
        btnSave->SetParent(this);

        btnClear = new Button();
        btnClear->SetBounds(Rect(78, 174, 150, 198));
        btnClear->SetText(L"Clear");
        btnClear->SetEnabled(false);
        btnClear->SetAnchors(caLeft | caBottom);
        btnClear->SetTabOrder(2);
        btnClear->SetParent(this);

        btnOk = new Button();
        btnOk->SetBounds(Rect(264, 174, 336, 198));
        btnOk->SetText(L"OK");
        btnOk->SetAnchors(caRight | caBottom);
        btnOk->SetTabOrder(4);
        btnOk->SetParent(this);

        btnCancel = new Button();
        btnCancel->SetBounds(Rect(337, 174, 409, 198));
        btnCancel->SetText(L"Cancel");
        btnCancel->SetAnchors(caRight | caBottom);
        btnCancel->SetTabOrder(5);
        btnCancel->SetCancel(true);
        btnCancel->SetParent(this);

        pb->OnPaint = CreateEvent(this, &selftype::pbPaint);
        pb->OnResize = CreateEvent(this, &selftype::pbResize);
        pb->OnScroll = CreateEvent(this, &selftype::pbScroll);
        pb->OnScrollOverflow = CreateEvent(this, &selftype::pbScrollOverflow);
        pb->OnScrollAutoSized = CreateEvent(this, &selftype::pbScrollAutoSized);
        btnLoad->OnClick = CreateEvent(this, &selftype::btnLoadClick);
        btnSave->OnClick = CreateEvent(this, &selftype::btnSaveClick);
        btnClear->OnClick = CreateEvent(this, &selftype::btnClearClick);
        btnOk->OnClick = CreateEvent(this, &selftype::btnOkClick);
        btnCancel->OnClick = CreateEvent(this, &selftype::btnCancelClick);

        // Additional to the properties set in the designer:
        CreateHandle();
        //SetMinimumWidth(MinimumWidth() + GetSystemMetrics(SM_CXVSCROLL));
        SetMinimumWidth(MinimumWidth() + GetSystemMetrics(SM_CXVSCROLL));
        SetWidth(MinimumWidth());
        dlgOpen->SetInitialPath(GetFilePath(icon.filename));
        dlgSave->SetInitialPath(GetFilePath(icon.filename));
        pb->SetVLineStep(16);
        GenerateBitmaps(icon);
    }

    IconSelectorDialog::IconSelector::~IconSelector()
    {
    }

    void IconSelectorDialog::IconSelector::Destroy()
    {
    }

    void IconSelectorDialog::IconSelector::pbPaint(void *sender, PaintParameters param)
    {
        Canvas *c = param.canvas;
        c->SelectStockBrush(sbWindow);
        if (icons.empty())
        {
            c->FillRect(param.updaterect);
            return;
        }

        int pos = pb->VPos() / 80 * cols;
        int x = 0;
        int y = -(pb->VPos() % 80);

        Rect cr = pb->ClientRect();
        Rect ir; // Icon paint rectangle in the paint box.

        while (y < cr.Height() && pos < (int)icons.size())
        {
            ir = RectS(x, y, 80, 80);
            if (param.updaterect.DoesIntersect(ir))
            {
                c->SelectStockBrush(sel == pos ? sbHighlight : sbWindow);
                c->FillRect(ir);
                Bitmap &bmp = icons[pos];
                // Draw icon in 48x48.
                double div = min(1, min(48.0 / bmp.Width(), 48.0 / bmp.Height()));
                int draww = bmp.Width() * div;
                int drawh = bmp.Height() * div;

                c->Draw(&bmp, ir.left + (ir.Width() - draww) / 2, ir.top + (ir.Height() - drawh) / 2, draww, drawh, 0, 0, bmp.Width(), bmp.Height());

                c->SetAntialias(true);
                c->SetPen(Color(clHighlight).Mix(clWhite));
                c->RoundFrameRect(ir.Inflate(-4), 8, 8);
                c->SetAntialias(false);
            }
            ++pos;
            x += 80;
            if (x + 80 > cr.Width())
            {
                y += 80;
                x = 0;
            }
        }

        c->SelectStockBrush(sbWindow);
        Rect r = Rect(cols * 80, 0, cr.right, cr.bottom);
        if (!r.Empty())
            c->FillRect(r);
        r = Rect(x, y, cols * 80, y + 80).Intersect(param.updaterect);
        if (!r.Empty())
            c->FillRect(r);
        r = Rect(0, y + 80, cr.right, cr.bottom).Intersect(param.updaterect);
        if (!r.Empty())
            c->FillRect(r);
    }

    void IconSelectorDialog::IconSelector::pbResize(void *sender, EventParameters param)
    {

    }

    void IconSelectorDialog::IconSelector::btnClearClick(void *sender, EventParameters param)
    {
        icon.Clear();
        GenerateBitmaps(icon);
        pb->Invalidate();
    }

    void IconSelectorDialog::IconSelector::btnLoadClick(void *sender, EventParameters param)
    {
        if (dlgOpen->Show(this))
        {
            dlgOpen->SetInitialPath(GetFilePath(dlgOpen->FullFileName()));

            IconData loadicon;
            loadicon.filename = dlgOpen->FullFileName();

            FILESTD fstream file(loadicon.filename, std::ios_base::in | std::ios_base::binary);
            file.seekg(0, std::ios_base::end);
            loadicon.filesize = file.tellg();
            if (loadicon.filesize > 1024 * 1024 * 10)
            {
                ShowMessageBox(L"The icon's file size can be at most 10 MBs. Please select a smaller file.", L"Invalid file", mbOk);
                return;
            }
            file.seekg(0, std::ios_base::beg);
            loadicon.filedata = new char[loadicon.filesize];
            file.read(loadicon.filedata, loadicon.filesize);
        
            if (file.bad())
                ShowMessageBox(L"Couldn't read icon from file.", L"Error", mbOk);
            else
            {
                if (!GenerateBitmaps(loadicon))
                    ShowMessageBox(L"Couldn't extract icon entries from the icon file.", L"Error", mbOk);
                else
                {
                    icon = std::move(loadicon);
                    pb->Invalidate();
                }
            }
        }
    }

    void IconSelectorDialog::IconSelector::btnSaveClick(void *sender, EventParameters param)
    {
        if (dlgSave->Show(this))
        {
            dlgSave->SetInitialPath(GetFilePath(dlgSave->FullFileName()));
            FILESTD fstream file(dlgSave->FullFileName(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
            file.write(icon.filedata, icon.filesize);
            if (file.bad())
                ShowMessageBox(L"Couldn't write icon to file.", L"Error", mbOk);
        }
    }

    void IconSelectorDialog::IconSelector::pbScrollOverflow(void *sender, ScrollOverflowParameters param)
    {
        if (prevcols < 0)
            prevcols = cols;
        cols = max(icons.empty() ? 0 : 1, param.visiblewidth / 80);
        rows = max(icons.empty() ? 0 : 1, ((int)icons.size() + cols - 1) / cols);
        param.visiblewidth = cols;
        param.hiddenwidth = 0;
        param.visibleheight = min(param.visibleheight, rows * 80);
        param.hiddenheight = max(0, rows * 80 - param.visibleheight);
    }

    void IconSelectorDialog::IconSelector::pbScrollAutoSized(void *sender, EventParameters param)
    {
        if (prevcols != cols)
            pb->Invalidate();
        prevcols = -1;
    }

    void IconSelectorDialog::IconSelector::pbScroll(void *sender, ScrollParameters param)
    {
        pb->Invalidate();
    }

    void IconSelectorDialog::IconSelector::btnOkClick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }

    void IconSelectorDialog::IconSelector::btnCancelClick(void *sender, EventParameters param)
    {
        SetModalResult(mrCancel);
    }

    bool IconSelectorDialog::IconSelector::GenerateBitmaps(IconData &data)
    {
        if (data.filedata)
        {
            try
            {
                std::vector<IconEntry> entries;
                FillIconEntryVector(data.filedata, data.filesize, entries, false);

                std::vector<Bitmap> tmpicons;
                tmpicons.resize(entries.size());

                for (unsigned int ix = 0; ix < entries.size(); ++ix)
                {
                    char *dat = data.filedata + entries[ix].icondirentry.dataoffset;

                    // Check whether it is a png icon or not.
                    // PNG starts with 0x89 50 4E 47 0D 0A 1A 0A
                    //byte pngh[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
                    //if (memcmp(dat, pngh, 8) == 0) // Found a PNG header. Create a bitmap from it.
                    //{
                    //    tmpicons[ix] = Bitmap(dat, entries[ix].icondirentry.datasize);
                    //    continue;
                    //}

                    Icon tmp(CreateIconFromResourceEx((byte*)dat, entries[ix].icondirentry.datasize, TRUE, 0x00030000, entries[ix].icondirentry.width, entries[ix].icondirentry.height, 0));
                    tmp.ToBitmap(tmpicons[ix]);
                }

                icons.swap(tmpicons);
            }
            catch(...)
            {
                return false;
            }
        }
        else
            icons.clear();
        btnSave->SetEnabled(!icons.empty());
        btnClear->SetEnabled(!icons.empty());
        pb->ScrollResize();
        return true;
    }


    //---------------------------------------------


    LineEditorDialog::LineEditorDialog()
    {
    }

    LineEditorDialog::~LineEditorDialog()
    {
    }

    bool LineEditorDialog::Show(Form *topparent, const std::vector<std::wstring> &alines)
    {
        lines = alines;
        LineEditor *selector = new LineEditor(this, lines);

        ModalResults mr = base::Show(selector, topparent);
        if (mr == mrOk)
            selector->mLines->GetLines(lines);

        selector->Destroy();
        return mr == mrOk;
    }

    void LineEditorDialog::GetLines(std::vector<std::wstring> &out)
    {
        out = lines;
    }


    //---------------------------------------------


    LineEditorDialog::LineEditor::LineEditor(LineEditorDialog *owner, const std::vector<std::wstring> &lines) : owner(owner)
    {
        //SetLeft(740);
        //SetTop(445);
        SetShowPosition(fspParentMonitorCenter);
        SetText(L"String item list editor");
        SetParentFont(false);
        SetClientRect(Rect(0, 0, 434, 312));
        SetMinimumWidth(260);

        mLines = new Memo();
        mLines->SetBounds(Rect(8, 8, 427, 280));
        mLines->SetAnchors(caLeft | caTop | caRight | caBottom);
        mLines->SetTabOrder(0);
        mLines->SetWantedKeyTypes(wkArrows | wkEnter | wkOthers);
        mLines->SetParent(this);

        btnOk = new Button();
        btnOk->SetBounds(Rect(274, 284, 349, 307));
        btnOk->SetText(L"Ok");
        btnOk->SetAnchors(caRight | caBottom);
        btnOk->SetTabOrder(1);
        btnOk->SetParent(this);

        btnCancel = new Button();
        btnCancel->SetBounds(Rect(354, 284, 429, 307));
        btnCancel->SetText(L"Cancel");
        btnCancel->SetAnchors(caRight | caBottom);
        btnCancel->SetTabOrder(2);
        btnCancel->SetCancel(true);
        btnCancel->SetParent(this);

        lbCnt = new Label();
        lbCnt->SetBounds(Rect(16, 289, 49, 302));
        lbCnt->SetText(L"0 lines");
        lbCnt->SetAnchors(caLeft | caBottom);
        lbCnt->SetParent(this);

        btnOk->OnClick = CreateEvent(this, &selftype::btnOkClick);
        btnCancel->OnClick = CreateEvent(this, &selftype::btnCancelClick);
        mLines->OnTextChanged = CreateEvent(this, &selftype::mLinesChanged);

        // Added by hand
        mLines->SetLines(lines, true);
    }

    LineEditorDialog::LineEditor::~LineEditor()
    {
        ;
    }

    void LineEditorDialog::LineEditor::btnOkClick(void *sender, EventParameters param)
    {
        SetModalResult(mrOk);
    }

    void LineEditorDialog::LineEditor::btnCancelClick(void *sender, EventParameters param)
    {
        SetModalResult(mrCancel);
    }

    void LineEditorDialog::LineEditor::mLinesChanged(void *sender, EventParameters param)
    {
        int lcnt = mLines->LineCount();
        if (mLines->LineLength(lcnt - 1) == 0)
            --lcnt;
        lbCnt->SetText(IntToStr(lcnt) + L" lines");

    }


    //---------------------------------------------


}
/* End of NLIBNS */

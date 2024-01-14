#include "stdafx_zoli.h"
#include "canvas.h"
#include "imagelist.h"
#include "images.h"
#include "controlbase.h"

#include <GdiPlusImageCodec.h>
#ifdef DESIGNING
#include "property_images.h"
#include "property_imagelist.h"
#include "designproperties.h"
#include "designer.h"
//#include "designercontrols.h"
#include "serializer.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


        //Gdiplus::Bitmap *gbmp = bmp.GetBitmap();
        //CLSID cls;
        //GetEncoderClsid(giePNG, &cls);
        //gbmp->Save(L"D:\\test.png", &cls);


#ifdef DESIGNING


//---------------------------------------------


ControlBitmap::ControlBitmap() : owner(NULL), image(NULL), statecount(1), filedata(NULL), filesize(0), transparent(clNone)
{
}

bool ControlBitmap::operator==(const ControlBitmap& other) const
{
    return image == other.image;
}

void ControlBitmap::Free()
{
    delete image;
    image = NULL;
    delete[] filedata;
    filedata = NULL;
    filesize = 0;
    transparent = clNone;
    filename = std::wstring();
}

void ControlBitmap::CopyTo(ControlBitmap &other) const
{
    if (image)
    {
        other.filesize = filesize;
        other.filedata = new char[filesize];
        memcpy(other.filedata, filedata, sizeof(char) * filesize);
        other.image = Bitmap::CreateCopy(*image);
        other.filename = filename;
        other.transparent = transparent;
        other.statecount = statecount;
    }
    else
    {
        other.filesize = 0;
        other.filedata = NULL;
        other.image = NULL;
        other.filename = std::wstring();
        other.transparent = clNone;
    }
}


//---------------------------------------------


void ControlImage::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->HideProperty(L"Name");
    serializer->HideProperty(L"AccessLevel");
    serializer->HideProperty(L"Tag");

    serializer->Add(L"SetBitmap", new ControlBitmapDesignProperty<ControlImage>(L"Bitmap", std::wstring(), &ControlImage::GetControlBitmap, &ControlImage::SetControlBitmap))->SetDefault(&ControlImage::DefaultBitmap);
    serializer->Add(L"SetStateCount", new IntDesignProperty<ControlImage>(L"StateCount", std::wstring(), &ControlImage::StateCount, &ControlImage::SetStateCount))->SetDefault(1);
    serializer->Add(L"SetImages", new ImagelistDesignProperty<ControlImage>(L"Images", std::wstring(), &ControlImage::Images, &ControlImage::SetImages));
    serializer->Add(L"SetImageIndex", new ImagelistIndexDesignProperty<ControlImage>(L"ImageIndex", std::wstring(), false, &ControlImage::Images, &ControlImage::ImageIndex, &ControlImage::SetImageIndex))->SetDefault(0);
    serializer->Add(L"SetDisabledIndex", new ImagelistIndexDesignProperty<ControlImage>(L"DisabledIndex", std::wstring(), true, &ControlImage::Images, &ControlImage::DisabledIndex, &ControlImage::SetDisabledIndex))->SetDefault(-1);
    serializer->Add(L"SetPressedIndex", new ImagelistIndexDesignProperty<ControlImage>(L"PressedIndex", std::wstring(), true, &ControlImage::Images, &ControlImage::PressedIndex, &ControlImage::SetPressedIndex))->SetDefault(-1);
    serializer->Add(L"SetDownIndex", new ImagelistIndexDesignProperty<ControlImage>(L"DownIndex", std::wstring(), true, &ControlImage::Images, &ControlImage::DownIndex, &ControlImage::SetDownIndex))->SetDefault(-1);
    serializer->Add(L"SetFocusedIndex", new ImagelistIndexDesignProperty<ControlImage>(L"FocusedIndex", std::wstring(), true, &ControlImage::Images, &ControlImage::FocusedIndex, &ControlImage::SetFocusedIndex))->SetDefault(-1);
    serializer->Add(L"SetHoveredIndex", new ImagelistIndexDesignProperty<ControlImage>(L"HoveredIndex", std::wstring(), true, &ControlImage::Images, &ControlImage::HoveredIndex, &ControlImage::SetHoveredIndex))->SetDefault(-1);
}

ControlBitmap ControlImage::GetControlBitmap()
{
    ctrlimg.owner = this;
    ctrlimg.image = image;
    ctrlimg.statecount = statecount;

    return ctrlimg;
}

void ControlImage::SetControlBitmap(ControlBitmap newbmp)
{
    ctrlimg.Free();
    newbmp.CopyTo(ctrlimg);
    SetBitmap(ctrlimg.image, true);
}

ControlBitmap ControlImage::DefaultBitmap()
{
    return ControlBitmap();
}

#endif


ControlImage::ControlImage(Control *owner) :
        owner(owner), imgtype(citBitmap), transform(true), image(NULL), ownbmp(false), statecount(1), images(NULL), index(0),
        disabled(-1), pressed(-1), down(-1), focused(-1), hovered(-1)
{
    AddToNotifyList(owner, nrSubControl);
}

ControlImage::~ControlImage()
{
    if (ownbmp)
        delete image;
#ifdef DESIGNING
    ctrlimg.Free();
#endif
}

int ControlImage::Width() const
{
    if (image)
        return image->Width() / statecount;
    if (images)
        return images->Width();
    return 0;
}

int ControlImage::Height() const
{
    if (image)
        return image->Height();
    if (images)
        return images->Height();
    return 0;
}

bool ControlImage::HasImage() const
{
    return image || (images && images->Count() > 0);
}

bool ControlImage::Transform()
{
    return transform;
}

void ControlImage::SetTransform(bool newtransform)
{
    transform = newtransform;
    Changed();
}

ControlImageTypes ControlImage::ImageType()
{
    return imgtype;
}

Bitmap* ControlImage::GetBitmap()
{
    return image;
}

void ControlImage::SetBitmap(Bitmap *newbitmap, bool shared)
{
    if ((imgtype == citBitmap && !image && newbitmap && newbitmap->Width() == 0) || (!image && !newbitmap))
        return;

    if (ownbmp)
        delete image;
    image = newbitmap;
    ownbmp = !shared;
    if (image)
    {
        if (image->Width() == 0)
        {
            if (ownbmp)
                delete image;
            image = NULL;
        }
        else
        {
            imgtype = citBitmap;
            statecount = min(6, image->Width() / image->Height());
            SetImages(NULL);
#ifdef DESIGNING
            if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
                designer->InvalidateRow(this, L"StateCount");
            if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
                designer->InvalidateRow(this, L"Images");
#endif
        }
    }
    Changed();

}

int ControlImage::StateCount() const
{
    return statecount;
}

void ControlImage::SetStateCount(int newstatecount)
{
    newstatecount = min(6, max(newstatecount, 1));
    if (image)
    {
        statecount = min(newstatecount, image->Width());
        Changed();
    }
    else
        statecount = newstatecount;
}

Imagelist* ControlImage::Images()
{
    return images;
}

void ControlImage::SetImages(Imagelist *newimages)
{
    if ((imgtype == citImagelist && images == newimages) || (!images && !newimages))
        return;

    RemoveFromNotifyList(images, nrSubControl);
    images = newimages;
    if (newimages)
    {
        imgtype = citImagelist;
        if (ownbmp)
            delete image;
        image = NULL;
        index =    max( 0, index);
        disabled = max(-1, disabled);
        pressed =  max(-1, pressed);
        down =     max(-1, down);
        focused =  max(-1, focused);
        hovered =  max(-1, hovered);

        AddToNotifyList(images, nrSubControl);

#ifdef DESIGNING
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"Bitmap");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"ImageIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"DisabledIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"PressedIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"DownIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"FocusedIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"HoveredIndex");
#endif
    }
    Changed();
}

int ControlImage::ImageIndex() const
{
    return index;
}

void ControlImage::DeleteNotify(Object *object)
{
    base::DeleteNotify(object);
    if (object == images)
    {
        images = NULL;
        Changed();
#ifdef DESIGNING
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"Images");
#endif
    }
}

void ControlImage::ChangeNotify(Object *object, int changetype)
{
    base::ChangeNotify(object, changetype);
    if (object == images)
    {
#ifdef DESIGNING
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"ImageIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"DisabledIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"PressedIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"DownIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"FocusedIndex");
        if (owner->Designing() && designer && designer->MainPropertyOwner(owner))
            designer->InvalidateRow(this, L"HoveredIndex");
#endif
        Changed();
    }
}

void ControlImage::SetImageIndex(int newimageindex)
{
    newimageindex = max(0, newimageindex);
    if (index == newimageindex)
        return;
    index = newimageindex;
    if (images)
        Changed();
}

int ControlImage::DisabledIndex() const
{
    return disabled;
}

void ControlImage::SetDisabledIndex(int newdisabledindex)
{
    newdisabledindex = max(-1, newdisabledindex);
    if (disabled == newdisabledindex)
        return;
    disabled = newdisabledindex;
    if (images)
        Changed();
}

int ControlImage::PressedIndex() const
{
    return pressed;
}

void ControlImage::SetPressedIndex(int newpressedindex)
{
    newpressedindex = max(-1, newpressedindex);
    if (pressed == newpressedindex)
        return;
    pressed = newpressedindex;
    if (images)
        Changed();
}

int ControlImage::DownIndex() const
{
    return down;
}

void ControlImage::SetDownIndex(int newdownindex)
{
    newdownindex = max(-1, newdownindex);
    if (down == newdownindex)
        return;
    down = newdownindex;
    if (images)
        Changed();
}

int ControlImage::FocusedIndex() const
{
    return focused;
}

void ControlImage::SetFocusedIndex(int newfocusedindex)
{
    newfocusedindex = max(-1, newfocusedindex);
    if (focused == newfocusedindex)
        return;
    focused = newfocusedindex;
    if (images)
        Changed();
}

int ControlImage::HoveredIndex() const
{
    return hovered;
}

void ControlImage::SetHoveredIndex(int newhoveredindex)
{
    newhoveredindex = max(-1, newhoveredindex);
    if (hovered == newhoveredindex)
        return;
    hovered = newhoveredindex;
    if (images)
        Changed();
}

void ControlImage::Draw(Canvas *dest, int x, int y, ControlImageStates state)
{
#ifdef DESIGNING
    bool transform = this->transform && !owner->Designing();
#endif

    if (image)
    {
        int drawstate = (int)state;
        if (drawstate >= statecount)
            drawstate = 0;

        int drawwidth = max(1, image->Width() / statecount);

        if (statecount > (int)state || (!transform && state != cisDisabled) || state == cisNormal || state == cisDown || state == cisFocused)
        {
            dest->Draw(image, x, y, drawstate * drawwidth, 0, drawwidth, image->Height());
            return;
        }
        
        ColorMatrix cm;
        switch(state)
        {
        case cisDisabled:
            cm.Grayscale().TransformAlpha(0.4F);
            break;
        case cisPressed:
            cm.Darken(0.1F);
            break;
        case cisHovered:
            cm.Saturate(0.5F);
            break;
        default:
            break;
        }
        auto cstate = dest->SaveState();
        dest->SetColorMatrix(cm);
        dest->Draw(image, x, y, drawstate * drawwidth, 0, drawwidth, image->Height());
        dest->RestoreState(cstate);
    }
    else if (images && images->Count())
    {
        bool match = state == cisNormal;
        int imgcnt = images->Count();
        int drawindex = index;
        if (state == cisDisabled && disabled >= 0 && disabled < imgcnt)
            drawindex = disabled, match = true;
        else if (state == cisPressed && pressed >= 0 && pressed < imgcnt)
            drawindex = pressed, match = true;
        else if (state == cisDown && down >= 0 && down < imgcnt)
            drawindex = down, match = true;
        else if (state == cisFocused && focused >= 0 && focused < imgcnt)
            drawindex = focused, match = true;
        else if (state == cisHovered && hovered >= 0 && hovered < imgcnt)
            drawindex = hovered, match = true;

        if (drawindex < 0 && drawindex >= imgcnt)
            return;

        if (match || (!transform && state != cisDisabled) || state == cisNormal || state == cisDown || state == cisFocused)
        {
            images->Draw(dest, drawindex, x, y);
            return;
        }

        ColorMatrix cm;
        switch(state)
        {
        case cisDisabled:
            cm.Grayscale().TransformAlpha(0.5F);
            break;
        case cisPressed:
            cm.Darken(0.1F);
            break;
        case cisHovered:
            cm.Saturate(0.5F);
            break;
        default:
            break;
        }

        auto cstate = dest->SaveState();

        Bitmap bmp(images->Width(), images->Height());
        images->Draw(&bmp, drawindex, 0, 0);

        dest->SetColorMatrix(cm);
        dest->Draw(&bmp, x, y, 0, 0, bmp.Width(), bmp.Height());
        dest->RestoreState(cstate);
    }
}

Form* ControlImage::ParentForm() const
{
    return owner->ParentForm();
}


//---------------------------------------------


}
/* End of NLIBNS */


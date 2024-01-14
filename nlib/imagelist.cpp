#include "stdafx_zoli.h"
#include "canvas.h"
#include "imagelist.h"

#ifdef __MINGW32__
#include "commoncontrols.h"
#endif

#ifdef DESIGNING
#include "designer.h"
#include "property_controlbase.h"
#include "property_imagelist.h"
#include "serializer.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


#ifdef __MINGW32__
    // {46EB5926-582E-4017-9FDF-E8998DAA0950} -vstudio
    const IID IID_IImageList = { 0x46EB5926, 0x582E, 0x4017, { 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x09, 0x50 } };
#endif

#ifdef DESIGNING

    ValuePair< ImagelistColorDepths> ImagelistColorDepthStrings[] = {
        VALUEPAIR(icdDeviceDependent),
        VALUEPAIR(icdDefault),
        VALUEPAIR(icd4bit),
        VALUEPAIR(icd8bit),
        VALUEPAIR(icd16bit),
        VALUEPAIR(icd24bit),
        VALUEPAIR(icd32bit),
    };


    void Imagelist::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        serializer->Add(L"SetWidth", new IntDesignProperty<Imagelist>(L"Width", L"Image dimensions", &Imagelist::Width, &Imagelist::SetWidth))->SetDefault(16)->SetDefaultWrite(&Imagelist::HandleCreated);
        serializer->Add(L"SetHeight", new IntDesignProperty<Imagelist>(L"Height", L"Image dimensions", &Imagelist::Height, &Imagelist::SetHeight))->SetDefault(16)->SetDefaultWrite(&Imagelist::HandleCreated);
        serializer->Add(L"SetMasked", new BoolDesignProperty<Imagelist>(L"Masked", L"Images", &Imagelist::Masked, &Imagelist::SetMasked))->SetDefault(false)->SetDefaultWrite(&Imagelist::HandleCreated);
        serializer->Add(L"SetColorDepth", new ImagelistColorDepthsDesignProperty<Imagelist>(L"ColorDepth", L"Images", &Imagelist::ColorDepth, &Imagelist::SetColorDepth))->SetDefault(icd32bit)->SetDefaultWrite(&Imagelist::HandleCreated);
        serializer->Add(L"SetPaintColor", new ColorDesignProperty<Imagelist>(L"PaintColor", L"Drawing", true, true, false, &Imagelist::PaintColor, &Imagelist::SetPaintColor))->SetDefault(clNone)->SetDefaultWrite(&Imagelist::HandleCreated);
        serializer->Add(L"SetMaskColor", new ColorDesignProperty<Imagelist>(L"MaskColor", L"Drawing", true, false, false, &Imagelist::MaskColor, &Imagelist::SetMaskColor))->SetDefault(clFuchsia);
        serializer->Add(L"ReadFromResource", new ImagelistImagesDesignProperty(L"Images", L"Images"))->MakeDefault();
    }
#endif

    Imagelist::Imagelist() :
            handle(NULL), imgwidth(16), imgheight(16), masked(false), colordepth(icd32bit),
            count(0), paintcolor(clNone), maskcolor(clFuchsia)
    {
        InitCommonControl(ICC_LISTVIEW_CLASSES);
    }

    Imagelist::Imagelist(int imgwidth, int imgheight, bool masked, ImagelistColorDepths colordepth) :
             handle(NULL), imgwidth(imgwidth), imgheight(imgheight), masked(masked), colordepth(colordepth),
             count(0), paintcolor(clNone), maskcolor(clFuchsia)
    {
        InitCommonControl(ICC_LISTVIEW_CLASSES);
        imgwidth = max(1, min(256, imgwidth));
        imgheight = max(1, min(256, imgheight));

        //ImageList_SetBkColor(handle, CLR_NONE);
    }

    Imagelist::Imagelist(const Imagelist &orig) : handle(NULL)
    {
        *this = orig;
    }

    Imagelist::Imagelist(Imagelist &&orig) : handle(NULL)
    {
        *this = std::move(orig);
    }

    Imagelist& Imagelist::operator=(const Imagelist &orig)
    {
        DestroyHandle();

        imgwidth = orig.imgwidth;
        imgheight = orig.imgheight;
        masked = orig.masked;
        colordepth = orig.colordepth;
        count = orig.count;
        paintcolor = orig.paintcolor;
        maskcolor = orig.maskcolor;

        if (orig.HandleCreated())
        {
            //if (colordepth == icd32bit)
            //{
            //    other.CreateHandle();
            //    ImageList_SetBkColor(handle, CLR_NONE);

            //    for (int ix = 0; ix < count; ++ix)
            //    {
            //        Bitmap bmp(imgwidth, imgheight);
            //        Draw(&bmp, ix, 0, 0);
            //        other.Add(&bmp);
            //    }

            //    ImageList_SetBkColor(handle, paintcolor == clNone ? (COLORREF)CLR_NONE : (COLORREF)paintcolor);
            //}
            //else
                handle = ImageList_Duplicate(orig.handle);

            ImageList_SetBkColor(handle, paintcolor == clNone ? (COLORREF)CLR_NONE : (COLORREF)paintcolor);
        }
        Changed();

        return *this;
    }

    Imagelist& Imagelist::operator=(Imagelist &&orig)
    {
        std::swap(handle, orig.handle);
        std::swap(count, orig.count);

        std::swap(imgwidth, orig.imgwidth);
        std::swap(imgheight, orig.imgheight);
        std::swap(masked, orig.masked);
        std::swap(colordepth, orig.colordepth);

        std::swap(paintcolor, orig.paintcolor);
        std::swap(maskcolor, orig.maskcolor);

        orig.Changed();
        Changed();

        return *this;
    }

    void Imagelist::Changed()
    {
#ifdef DESIGNING
        if (designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"Images");
#endif

        base::Changed(0);
    }

    bool Imagelist::HandleCreated() const
    {
        return handle != 0;
    }

    void Imagelist::CreateHandle()
    {
        if (handle)
            return;
        handle = ImageList_Create(imgwidth, imgheight, (masked ? ILC_MASK : 0) | (UINT)colordepth, 8, 8);
        if (!handle)
            throw L"Couldn't create imagelist.";

        ImageList_SetBkColor(handle, paintcolor == clNone ? CLR_NONE : (COLORREF)paintcolor);
    }

    void Imagelist::DestroyHandle()
    {
        if (!handle)
            return;

        ImageList_Destroy(handle);
        handle = 0;
        count = 0;

        Changed();
    }

    Imagelist::~Imagelist()
    {
        DestroyHandle();
    }

    HIMAGELIST Imagelist::Handle()
    {
        if (!handle)
            CreateHandle();
        return handle;
    }

    int Imagelist::Count() const
    {
        return count;
    }

    //int Imagelist::Width()
    //{
    //    return imgwidth;
    //}
    //
    //int Imagelist::Height()
    //{
    //    return imgheight;
    //}

    int Imagelist::Width() const
    {
        return imgwidth;
    }

    int Imagelist::Height() const
    {
        return imgheight;
    }

    void Imagelist::SetSizes(int newwidth, int newheight)
    {
        newwidth = max(1, min(256, newwidth));
        newheight = max(1, min(256, newheight));

        if (newwidth == imgwidth && newheight == imgheight)
            return;

        count = 0;

        imgwidth = newwidth;
        imgheight = newheight;
        if (HandleCreated())
            ImageList_SetIconSize(handle, newwidth, newheight);

        Changed();
    }

    void Imagelist::SetWidth(int newwidth)
    {
        SetSizes(newwidth, imgheight);
    }

    void Imagelist::SetHeight(int newheight)
    {
        SetSizes(imgwidth, newheight);
    }

    ImagelistColorDepths Imagelist::ColorDepth()
    {
        return colordepth;
    }

    void Imagelist::SetColorDepth(ImagelistColorDepths newcolordepth)
    {
        if (colordepth == newcolordepth)
            return;
        colordepth = newcolordepth;
        DestroyHandle();
    }

    bool Imagelist::Masked()
    {
        return masked;
    }

    void Imagelist::SetMasked(bool newmasked)
    {
        if (masked == newmasked)
            return;
        masked = newmasked;
        DestroyHandle();
    }

    Color Imagelist::PaintColor()
    {
        return paintcolor;
    }

    void Imagelist::SetPaintColor(Color newpaintcolor)
    {
        if (paintcolor == newpaintcolor)
            return;
        paintcolor = newpaintcolor;

        if (HandleCreated())
            ImageList_SetBkColor(handle, paintcolor == clNone ? CLR_NONE : (COLORREF)paintcolor);

        Changed();
    }

    Color Imagelist::MaskColor()
    {
        return maskcolor;
    }

    void Imagelist::SetMaskColor(Color newmaskcolor)
    {
        if (newmaskcolor == clNone)
            return;
        newmaskcolor = newmaskcolor.Solid();
        if (maskcolor == newmaskcolor)
            return;
        maskcolor = newmaskcolor;
    }

    HBITMAP Imagelist::HandleFromBitmap(Bitmap *bmp)
    {
        return bmp->HandleCopy(masked || colordepth < icd32bit ? maskcolor : Color(0, 0, 0, 0));
    }

    HBITMAP CreateMaskFromHBITMAP(HBITMAP hbmp, Color maskcolor)
    {
        HDC dc = GetDC(0);
        if (!dc)
            return NULL;

        maskcolor = maskcolor.Solid().ToRGB();

        BITMAPINFO info;
        memset(&info, 0, sizeof(BITMAPINFO) );
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        if (!GetDIBits(dc, hbmp, 0, 1, NULL, &info, DIB_RGB_COLORS))
        {
            ReleaseDC(0, dc);
            return NULL;
        }

        int w = info.bmiHeader.biWidth;
        int h = abs(info.bmiHeader.biHeight);
        byte *bits = new byte[w * h * 4];
        info.bmiHeader.biCompression = BI_RGB;
        int r;
        if (!bits || (r = GetDIBits(dc, hbmp, 0, h, bits, &info, DIB_RGB_COLORS)) == ERROR_INVALID_PARAMETER || (!r && h > 0))
        {
            delete[] bits;
            ReleaseDC(0, dc);
            return NULL;
        }
    
        char *vinf = new char[sizeof(BITMAPINFOHEADER) +  sizeof(RGBQUAD) * 2];
        BITMAPINFO *info2 = (BITMAPINFO*)vinf;
        memset(info2->bmiColors, 0, sizeof(RGBQUAD));
        memset(info2->bmiColors + 1, 255, sizeof(RGBQUAD));
        info2->bmiColors[1].rgbReserved = 0;
        memcpy(&info2->bmiHeader, &info.bmiHeader, sizeof(BITMAPINFOHEADER));
        info2->bmiHeader.biBitCount = 1;

        void *bits2 = NULL;
        HBITMAP dibbmp = CreateDIBSection(dc, info2, DIB_RGB_COLORS, &bits2, NULL, 0);
        ReleaseDC(0, dc);
        delete[] vinf;
        if (!hbmp)
            return NULL;

        byte *bitspos = bits;
        byte *bits2pos = (byte*)bits2;
        byte val;
        for (int y = 0; y < h; ++y)
        {
            val = 0;
            for (int x = 0; x < w; ++x)
            {
                if (bitspos[x * 4] == maskcolor.R() &&  bitspos[x * 4 + 1] == maskcolor.G() && bitspos[x * 4 + 2] == maskcolor.B() && bitspos[x * 4 + 3] == 255)
                    val += (1 << (7 - (x % 8)));

                if (((x + 1) % 8) == 0)
                {
                    bits2pos[x / 8] = val;
                    val = 0;
                }
            }
            if (((w + 1) % 8) != 0)
                bits2pos[w / 8] = val;

            bitspos += w * 4;
            bits2pos += ((((w + 7) / 8) + 3) / 4) * 4;
        }

        GdiFlush();
        
        return dibbmp;
    }

    int Imagelist::Add(HBITMAP img, HBITMAP mask)
    {
        //if (!masked && mask != NULL)
        //    throw L"Cannot add bitmap with mask to a non-masked imagelist.";

        CreateHandle();

        int r = -1;
        if (!masked || mask)
            r = ImageList_Add(handle, img, mask);
        else
        {
            mask = CreateMaskFromHBITMAP(img, maskcolor);
            if (!mask)
                throw L"Couldn't create mask from image.";
            r = ImageList_Add(handle, img, mask);
            DeleteObject(mask);
        }
        if (r >= 0)
        {
            count = ImageList_GetImageCount(handle);

            Changed();
            return r;
        }
        throw L"Couldn't add image to imagelist.";
    }

    int Imagelist::Add(Bitmap *bmp)
    {
        HBITMAP hbmp = HandleFromBitmap(bmp);
        int r = -1;
        if (hbmp)
        {
            try
            {
                r = Add(hbmp, NULL);
                //CreateHandle();

                //if (masked)
                //    r = ImageList_AddMasked(handle, hbmp, maskcolor);
                //else
                //    r = ImageList_Add(handle, hbmp, NULL);
                //if (r >= 0)
                //    count = ImageList_GetImageCount(handle);
            }
            catch(...)
            {
                DeleteObject(hbmp);
                throw;
            }
            DeleteObject(hbmp);
        }
        if (r < 0)
            throw L"Couldn't add bitmap to imagelist.";
        return r;
    }

    int Imagelist::AddIcon(HICON icon)
    {
        CreateHandle();

        int r = ImageList_AddIcon(handle, icon);
        if (r >= 0)
        {
            count = ImageList_GetImageCount(handle);

            Changed();
            return r;
        }
        throw L"Couldn't add icon to imagelist.";
    }

    void Imagelist::Replace(int index, HBITMAP img, HBITMAP mask)
    {
        if (index < 0 || index >= count)
            throw L"Index out of range.";

        //if (!masked && mask != NULL)
        //    throw L"Cannot add bitmap with mask to a non-masked imagelist.";

        if (!masked || mask)
        {
            if (!ImageList_Replace(handle, index, img, mask))
                throw L"Couldn't replace image.";
        }
        else
        {
            int cnt = count;
            mask = CreateMaskFromHBITMAP(img, maskcolor);
            if (!mask)
                throw L"Couldn't create mask from image.";
            int r = ImageList_Add(handle, img, mask);
            DeleteObject(mask);
            if (r < 0)
                throw L"Couldn't add image.";

            while (cnt + 1 < count)
                Remove(r + 1);
            Remove(index);
            if (r > index)
                r--;

            Move(r, index);
        }

        count = ImageList_GetImageCount(handle);

        Changed();
    }

    void Imagelist::ReplaceIcon(int index, HICON icon)
    {
        if (index < 0 || index >= count)
            throw L"Index out of range.";

        if (!ImageList_ReplaceIcon(handle, index, icon))
            throw L"Couldn't replace image.";

        count = ImageList_GetImageCount(handle);

        Changed();
    }

    void Imagelist::Replace(int index, Bitmap *bmp)
    {
        if (index < 0 || index >= count)
            throw L"Index out of range.";

        HBITMAP hbmp = HandleFromBitmap(bmp);
        if (hbmp)
        {
            try
            {
                Replace(index, hbmp, NULL);
            }
            catch(...)
            {
                DeleteObject(hbmp);
                throw;
            }
            DeleteObject(hbmp);
        }
        else
            throw L"Couldn't add bitmap to imagelist.";

        count = ImageList_GetImageCount(handle);

        Changed();
    }

    void Imagelist::Remove(int index)
    {
        if (index < 0 || index >= count)
            throw L"Index out of range.";

        if (ImageList_Remove(handle, index))
            count = ImageList_GetImageCount(handle);
        else
            throw L"Couldn't remove the image from the imagelist.";

        Changed();
    }

    void Imagelist::Clear()
    {
        if (!count)
            return;

        if (ImageList_RemoveAll(handle))
            count = 0;
        else
            throw L"Couldn't clear the imagelist.";

        Changed();
    }

    void Imagelist::Move(int src, int dest)
    {
        if (src < 0 || dest < 0 || src >= count || dest >= count)
            throw L"Index out of range.";

        int delta = src < dest ? 1 : -1;
        while (src != dest)
        {
            if (!ImageList_Copy(handle, src + delta, handle, src, ILCF_SWAP))
                throw L"Couldn't move image from src to dest.";
            src += delta;
        }

        Changed();
    }

    HICON Imagelist::CreateIcon(int index)
    {
        if (index < 0 || index >= count)
            throw L"Index out of range.";

        CreateHandle();

        return ImageList_ExtractIcon(NULL, handle, index);
    }

    void Imagelist::Draw(Canvas *canvas, int index, int x, int y) const
    {
        if (index < 0 || index >= count)
            throw L"Index out of range.";

        if (canvas->CompatibleDC() || paintcolor != clNone)
        {
            HDC dc = canvas->GetDC();
            ImageList_Draw(handle, index, dc, x, y, ILD_NORMAL);
            canvas->ReturnDC();
        }
        else
        {
            HDC hdc = GetDC(0);

            HDC newdc = CreateCompatibleDC(hdc);

            byte *b = new byte[Width() * 4 * Height()];
            memset(b, 0, Width() * 4 * Height());
            BITMAPINFO inf;
            memset(&inf, 0, sizeof(BITMAPINFO) );
            inf.bmiHeader.biSize = sizeof(inf.bmiHeader);
            inf.bmiHeader.biWidth = Width();
            inf.bmiHeader.biHeight = Height();
            inf.bmiHeader.biPlanes = 1;
            inf.bmiHeader.biBitCount = 32;
            inf.bmiHeader.biCompression = BI_RGB;

            HBITMAP bmp = CreateDIBitmap(hdc, &inf.bmiHeader, CBM_INIT, b, &inf, DIB_RGB_COLORS);

            HBITMAP oldbmp = (HBITMAP)SelectObject(newdc, bmp);

            ImageList_Draw(handle, index, newdc, x, y, ILD_NORMAL);

            SelectObject(newdc, oldbmp);
            DeleteDC(newdc);

            GetDIBits(hdc, bmp, 0, Height(), b, &inf, DIB_RGB_COLORS);

            ReleaseDC(0, hdc);

            Bitmap gbmp(Width(), Height(), PixelFormat32bppPARGB);
            auto data = gbmp.LockBits(glmWriteOnly);
            byte *line = (byte*)data->Scan0;
            for (int iy = 0; iy < Height(); ++iy)
            {
                memcpy(line, b + (Height() - iy - 1) * 4 * Width(), 4 * Width());
                line += data->Stride;
            }
            gbmp.UpdateBits();
            delete[] b;

            canvas->Draw(&gbmp, x, y);

            DeleteObject(bmp);
        }
    }

    bool Imagelist::WriteToStream(IStream *stream)
    {
        if (!HandleCreated())
            return true;
        return ImageList_WriteEx(handle, ILP_NORMAL, stream) == S_OK;
    }

    bool Imagelist::ReadFromStream(IStream *stream)
    {
        IImageList *iimage;
        ImageList_ReadEx(ILP_NORMAL, stream, IID_IImageList, (void**)&iimage);
    
        if (!iimage)
            return false;

        if (HandleCreated())
            DestroyHandle();
        
        Imagelist *dummy = new Imagelist();
    
        if (dummy->HandleCreated())
            ImageList_Destroy(dummy->handle);
        dummy->handle = (HIMAGELIST)iimage;
        dummy->count = ImageList_GetImageCount(dummy->handle);
        COLORREF ref = ImageList_GetBkColor(dummy->handle);
        if (ref == CLR_NONE)
            dummy->paintcolor = clNone;
        else
            dummy->paintcolor = ref;
        ImageList_GetIconSize(dummy->handle, &dummy->imgwidth, &dummy->imgheight);

        dummy->masked = masked;
        dummy->colordepth = colordepth;

        if (dummy->count)
        {
            // Try to get the masked value and the color depth of the image inside the imagelist.
            IMAGEINFO inf = {0};
            if (ImageList_GetImageInfo(dummy->handle, 0, &inf) == 0)
            {
                // Failure
                iimage->Release();
                dummy->Destroy();
                return false;
            }

            dummy->masked = inf.hbmMask != NULL;
            HDC dc = GetDC(0);
            BITMAPINFO binf;
            memset(&binf, 0, sizeof(BITMAPINFO));
            binf.bmiHeader.biSize = sizeof(BITMAPINFO);
            GetDIBits(dc, inf.hbmImage, 0, 1, NULL, &binf, 0);
            ReleaseDC(0, dc);
            switch (binf.bmiHeader.biBitCount)
            {
            case 4:
                dummy->colordepth = icd4bit;
                break;
            case 8:
                dummy->colordepth = icd8bit;
                break;
            case 16:
                dummy->colordepth = icd16bit;
                break;
            case 24:
                dummy->colordepth = icd24bit;
                break;
            case 32:
                dummy->colordepth = icd32bit;
                break;
            };
        }

        dummy->maskcolor = maskcolor;
        *this = *dummy;

        dummy->handle = 0;
        iimage->Release();
        dummy->Destroy();

        return true;
    }

    void Imagelist::ReadFromResource(HMODULE module, const wchar_t *resname)
    {
        HRSRC res = FindResource(module, resname, RT_RCDATA);
        if (res == NULL)
            throw L"Can't find resource with this name";

        int datasize = SizeofResource(module, res);
        HGLOBAL lres = LoadResource(module, res);
        void *data = NULL;
        if (lres)
            data = LockResource(lres);
        if (!data)
            throw L"Couldn't load the resource.";

        HGLOBAL glob = GlobalAlloc(GMEM_MOVEABLE, datasize);
        if (!glob)
            throw L"Couldn't allocate memory for the resource image.";

        void* buff = GlobalLock(glob);
        if (!buff)
        {
            GlobalFree(glob);
            throw L"Couldn't lock memory for bitmap.";
        }

        memcpy(buff, data, datasize);
        GlobalUnlock(glob);

        IStream *stream = NULL;
        bool success = false;
        if (CreateStreamOnHGlobal(glob, FALSE, &stream) == S_OK)
        {
            success = ReadFromStream(stream);
            stream->Release();
        }

        GlobalFree(glob);

        if (!success)
            throw L"Couldn't load imagelist from stream.";
    }


    //---------------------------------------------



}
/* End of NLIBNS */


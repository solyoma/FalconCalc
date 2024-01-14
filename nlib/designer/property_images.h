#pragma once

#include "designproperties.h"
#include "imagelist.h"
#include "images.h"
#include "designerdialogs.h"
#include "utility.h"

//---------------------------------------------


namespace NLIBNS
{


extern OpenDialog *imgpropopendialog;
extern TransparentColorPickerDialog *transparentcolordialog;
extern BitmapSelectorDialog *bitmapselectordialog;

// Helper function for drawing thumb images of a larger image.
void DrawThumbBitmap(Canvas *c, Bitmap *image, int statecount, const Rect &dest);
void CreateImgPropOpenDialog();

// ControlImage property based on the class property.
template<typename PropertyHolder>
class GeneralControlImageDesignProperty : public GeneralClassDesignProperty<PropertyHolder, ControlImage*>
{
private:
    typedef GeneralClassDesignProperty<PropertyHolder, ControlImage*>  base;
public:
    GeneralControlImageDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) :    base(name, category, reader)
    {
        this->propertystyle << psThumbImage << psReadonly;
        this->propertystyle -= psEditShared;
    }

    virtual std::wstring Value(Object *propholder)
    {
        ControlImage *ci = this->CallGetter(propholder);

        if (ci->HasImage())
            return ci->GetBitmap() != NULL ? L"Bitmap" : L"Imagelist";
        else
            return L"Empty";
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        ControlImage *image = this->CallGetter(propholder);
        if ((image->ImageType() == citBitmap && image->GetBitmap() == NULL) || (image->ImageType() == citImagelist && (image->Images() == NULL || image->ImageIndex() >= image->Images()->Count())))
        {
            DrawThumbBitmap(c, NULL, 0, r);
            return;
        }

        if (image->ImageType() == citBitmap)
            DrawThumbBitmap(c, image->GetBitmap(), image->StateCount(), r);
        else if (image->ImageType() == citImagelist)
        {
            Bitmap bmp(image->Width(), image->Height());
            Imagelist *lst = image->Images();
            if (image->ImageIndex() >= lst->Count())
                return;
            lst->Draw(&bmp, image->ImageIndex(), 0, 0);
            DrawThumbBitmap(c, &bmp, 1, r);
        }
    }
};

template<typename PropertyHolder>
class ControlImageDesignProperty : public GeneralControlImageDesignProperty<PropertyHolder>
{
private:
    typedef GeneralControlImageDesignProperty<PropertyHolder>  base;
public:
    template<typename GetterProc>
    ControlImageDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter) : base(name, category, CreatePropertyReader<PropertyHolder, ControlImage*>(getter, nullptr))
    {}
};

// Bitmap property in ControlImages.
struct ControlBitmap;
template<typename PropertyHolder>
class GeneralControlBitmapDesignProperty : public GeneralDesignProperty<PropertyHolder, ControlBitmap>
{
private:
    typedef GeneralDesignProperty<PropertyHolder, ControlBitmap>  base;
protected:
public:
    GeneralControlBitmapDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
    {
        this->propertystyle << psReadonly << psThumbImage << psEditButton << psInnerBinary << psBinaryResource;
        this->propertystyle -= psEditShared;
    }

    virtual std::wstring Value(Object *propholder)
    {
        const ControlBitmap &bmp = this->CallGetter(propholder);
        if (!bmp.image)
            return L"Empty";
        std::wstringstream ws;
        ws << bmp.image->Width() << L"x" << bmp.image->Height();
        return ws.str();
    }

    // Returns the raw image data instead of the default [empty] or dimensions string returned by Value.
    virtual void BinaryValue(Object *propholder, std::vector<byte> &result)
    {
        const ControlBitmap &bmp = this->CallGetter(propholder);
        if (!bmp.image)
            return;

        uLongf sizemax = compressBound(bmp.filesize);
        int headersize = sizeof(WORD) + bmp.filename.length() * sizeof(wchar_t) + sizeof(unsigned int/*bmp.filesize*/) + sizeof(byte/*bmp.statecount*/) + sizeof(bool) + (bmp.transparent.EnumIndexed() ? sizeof(Colors) : sizeof(COLORREF));
        result.resize(headersize + sizemax);
        byte *arr = &result[0];
        *(WORD*)arr = (WORD)bmp.filename.length();
        arr += sizeof(WORD);
        memcpy(arr, bmp.filename.c_str(), bmp.filename.length() * sizeof(wchar_t));
        arr += bmp.filename.length() * sizeof(wchar_t);
        *(unsigned int*)arr = bmp.filesize;
        arr += sizeof(unsigned int); // unsigned int
        *arr = bmp.statecount;
        ++arr;
        *(bool*)arr = bmp.transparent.EnumIndexed();
        arr += sizeof(bool);
        if (bmp.transparent.EnumIndexed())
        {
            *(Colors*)arr = bmp.transparent.EnumValue();
            arr += sizeof(Colors);
        }
        else
        {
            *(COLORREF*)arr = bmp.transparent;
            arr += sizeof(COLORREF);
        }

        if (compress(arr, &sizemax, (byte*)bmp.filedata, bmp.filesize) != Z_OK)
            throw L"Couldn't compress bitmap data!";
        result.resize(headersize + sizemax);

        return;
    }

    virtual void SetBinaryValue(Form *parentform, Object *propholder, std::vector<byte> &val)
    {
        ControlBitmap cbmp;
        byte *arr = &val[0];

        cbmp.filename.resize(*(WORD*)arr);
        arr += sizeof(WORD);
        memcpy(const_cast<wchar_t*>(cbmp.filename.c_str()), arr, cbmp.filename.length() * sizeof(wchar_t));
        arr += cbmp.filename.length() * sizeof(wchar_t);
        cbmp.filesize = *(unsigned int*)arr;
        arr += sizeof(unsigned int);
        cbmp.statecount = *arr;
        ++arr;
        bool indexed = *(bool*)arr;
        arr += sizeof(bool);
        if (indexed)
        {
            cbmp.transparent = *(Colors*)arr;
            arr += sizeof(Colors);
        }
        else
        {
            cbmp.transparent = *(COLORREF*)arr;
            arr += sizeof(COLORREF);
        }

        cbmp.filedata = new char[cbmp.filesize];
        if (!cbmp.filedata)
            return;

        uLongf fsize = cbmp.filesize;
        int len = val.size() - (arr - &val[0]);
        bool error = uncompress((Bytef*)cbmp.filedata, &fsize, arr, len) != Z_OK;

        if (error)
        {
            delete[] cbmp.filedata;
            return;
        }

        Bitmap *tmp = new Bitmap(cbmp.filedata, cbmp.filesize);

        if (cbmp.transparent != clNone)
        {
            cbmp.image = new Bitmap(tmp->Width(), tmp->Height());
            cbmp.image->SetColorKey(cbmp.transparent, cbmp.transparent);
            cbmp.image->Draw(tmp, 0, 0);
            cbmp.image->ResetColorKey();
            delete tmp;
        }
        else
            cbmp.image = tmp;

        this->CallSetter(propholder, cbmp);
        cbmp.Free();
    }

    virtual void ResourceValue(Object *propholder, std::vector<byte> &result)
    {
        const ControlBitmap &bmp = this->CallGetter(propholder);
        if (!bmp.image)
            return;

        if (bmp.transparent == clNone)
        {
            result.resize(bmp.filesize);
            memcpy(&result[0], bmp.filedata, bmp.filesize);
            return;
        }

        VectorIStream *stream = new VectorIStream(result);
        bmp.image->Save(stream, giePNG);
        stream->Release();
    }

    virtual std::wstring ResourceExportValue(Object *propholder, int resourceid)
    {
#define NLIB_CONCATSTR(str1, str2)    str1 str2
        return NLIB_CONCATSTR(L"new ", NLIBNS_STRING L"Bitmap(NULL, ") + base::ResourceExportValue(propholder, resourceid) + L")";
#undef NLIB_CONCATSTR
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
    {
        return false;
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        CreateImgPropOpenDialog();
        ControlBitmap cbmp = this->CallGetter(propholder);

        if (cbmp.image == NULL)
        {
            try
            {
                if (imgpropopendialog->Show(parentform))
                {
                    Bitmap *bmp;

                    bmp = new Bitmap(imgpropopendialog->FullFileName());

                    cbmp.filename = imgpropopendialog->FileName();

                    {
                        FILESTD ifstream f(imgpropopendialog->FullFileName(), std::ifstream::binary | std::ifstream::ate);

                        cbmp.filesize = f.tellg();
                        f.seekg(std::ios_base::beg);
                        cbmp.filedata = new char[cbmp.filesize];
                        f.read(cbmp.filedata, cbmp.filesize);
                        f.close();
                    }

                    ImageTypes type = bmp->GetImageType();
                    if (type == itBMP || type == itTIFF || type == itUndefined || type == itMemoryBitmap)
                    {
                        transparentcolordialog->selcol = clNone;
                        transparentcolordialog->allownone = true;
                        transparentcolordialog->Show(parentform, bmp);
                        if (transparentcolordialog->Selected())
                        {
                            cbmp.transparent = transparentcolordialog->selcol;
                            Bitmap *tmp = bmp;
                            bmp = new Bitmap(tmp->Width(), tmp->Height());
                            bmp->SetColorKey(cbmp.transparent, cbmp.transparent);
                            bmp->Draw(tmp, 0, 0);
                            bmp->ResetColorKey();
                        }
                        else
                            cbmp.transparent = clNone;
                    }
                    cbmp.image = bmp;
                    this->CallSetter(propholder, cbmp);
                    cbmp.Free();

                    return true;
                }
                cbmp.Free();
            }
            catch(...)
            {
                cbmp.Free();
                return false;
            }
        }
        else
        {
            if (bitmapselectordialog->Show(parentform, cbmp))
            {
                this->CallSetter(propholder, bitmapselectordialog->GetBitmap());
                return true;
            }
        }

        return false;
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        const ControlBitmap &cbmp = this->CallGetter(propholder);
        DrawThumbBitmap(c, cbmp.image, cbmp.statecount, r);
    }

};

template<typename PropertyHolder>
class ControlBitmapDesignProperty : public GeneralControlBitmapDesignProperty<PropertyHolder>
{
private:
    typedef GeneralControlBitmapDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    ControlBitmapDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, ControlBitmap>(getter, setter))
    {}
};


}
/* End of NLIBNS */


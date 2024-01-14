#pragma once

#include "objectbase.h"
#include "designproperties.h"
#include "designerdialogs.h"
#include "utility.h"

//---------------------------------------------


namespace NLIBNS
{


extern ImagelistEditorDialog *imagelisteditordialog;

extern ValuePair<ImagelistColorDepths> ImagelistColorDepthStrings[];
template<typename PropertyHolder>
class ImagelistColorDepthsDesignProperty : public EnumDesignProperty<PropertyHolder, ImagelistColorDepths>
{
private:
    typedef EnumDesignProperty<PropertyHolder, ImagelistColorDepths>    base;
public:
    template<typename GetterProc, typename SetterProc>
    ImagelistColorDepthsDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, getter, setter, ImagelistColorDepthStrings)
    {}
};

template<typename PropertyHolder>
class GeneralImagelistDesignProperty : public GeneralControlByTypeDesignProperty<PropertyHolder, Imagelist>
{
private:
    typedef GeneralControlByTypeDesignProperty<PropertyHolder, Imagelist>    base;
public:
    GeneralImagelistDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader)
    {
    }
};

template<typename PropertyHolder>
class ImagelistDesignProperty : public GeneralImagelistDesignProperty<PropertyHolder>
{
private:
    typedef GeneralImagelistDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    ImagelistDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Imagelist*>(getter, setter))
    {}
};

void DrawThumbBitmap(Canvas *c, Bitmap *image, int statecount, const Rect &dest);
template<typename PropertyHolder>
class GeneralImagelistIndexDesignProperty : public GeneralDesignProperty<PropertyHolder, int>
{
private:
    typedef GeneralDesignProperty<PropertyHolder, int> base;
protected:
    typedef Imagelist* (PropertyHolder::*ImagelistGetter)();
private:
    bool allownegative;
    ImagelistGetter listgetter;

    int val; // Temporary value which has an address that can be returned.
public:
    GeneralImagelistIndexDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader, bool allownegative, ImagelistGetter listgetter) : base(name, category, reader),
            allownegative(allownegative), listgetter(listgetter)
    {
        this->propertystyle << psThumbImage;
    }

    virtual std::wstring Value(Object *propholder)
    {
        std::wstringstream str;
        str << this->CallGetter(propholder);
        return str.str();
    }

    virtual bool SetValue(Form *parentform, Object *propholder, const std::wstring& val)
    {
        try
        {
            int i;
            if (!StrToInt(val, i) || (i <= 0 && !allownegative))
                return false;
            i = max(allownegative ? -1 : 0, i);
            this->CallSetter(propholder, i);
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        if (index != INT_MAX && allownegative)
            --index;

        Imagelist *lst = ((dynamic_cast<PropertyHolder*>(propholder))->*listgetter)();
        if (lst && index == INT_MAX)
            index = this->CallGetter(propholder);

        if (!lst || index < 0 || index >= lst->Count())
        {
            DrawThumbBitmap(c, NULL, 1, r);
            return;
        }

        Bitmap bmp(lst->Width(), lst->Height());
        lst->Draw(&bmp, index, 0, 0);
        DrawThumbBitmap(c, &bmp, 1, r);
    }

    virtual int ListCount(Object *propholder)
    {
        Imagelist *lst = ((dynamic_cast<PropertyHolder*>(propholder))->*listgetter)();
        if (!lst)
            return 1;
        return max(1, lst->Count() + (allownegative ? 1 : 0));
    }

    virtual std::wstring ListItem(Object *propholder, int index)
    {
        if (allownegative && index == 0)
            return L"-1";
        Imagelist *lst = ((dynamic_cast<PropertyHolder*>(propholder))->*listgetter)();
        if (!lst || !lst->Count())
            return L"-1";

        return IntToStr(index - (allownegative ? 1 : 0));
    }

    virtual void* ListValue(Object *propholder, int index)
    {
        if (allownegative && index == 0)
            val = -1;
        else
        {
            Imagelist *lst = ((dynamic_cast<PropertyHolder*>(propholder))->*listgetter)();
            if (!lst || !lst->Count())
                val = allownegative ? -1 : 0;
            else
                val = index - (allownegative ? 1 : 0);
        }
        return &val;
    }

    virtual int Selected(Object *propholder)
    {
        int i = this->CallGetter(propholder);
        if (i < 0 || (!allownegative && i == 0))
            return 0;

        Imagelist *lst = ((dynamic_cast<PropertyHolder*>(propholder))->*listgetter)();
        if (!lst || lst->Count() <= i)
            return -1;

        return i + (allownegative ? 1 : 0);
    }

    virtual bool SelectValue(Form *parentform, Object *propholder, void *val)
    {
        int i = *(int*)val;
        if (i < 0)
        {
            if (!allownegative)
                return false;
            else
            {
                this->CallSetter(propholder, -1);
                return true;
            }
        }
        if (!allownegative && i == 0)
        {
            this->CallSetter(propholder, 0);
            return true;
        }

        Imagelist *lst = ((dynamic_cast<PropertyHolder*>(propholder))->*listgetter)();
        if (!lst || i >= lst->Count())
            return false;
        this->CallSetter(propholder, i);
        return true;
    }

};

template<typename PropertyHolder>
class ImagelistIndexDesignProperty : public GeneralImagelistIndexDesignProperty<PropertyHolder>
{
private:
    typedef GeneralImagelistIndexDesignProperty<PropertyHolder>    base;
public:
    template<typename GetterProc, typename SetterProc>
    ImagelistIndexDesignProperty(const std::wstring &name, const std::wstring &category, bool allownegative, typename base::ImagelistGetter listgetter, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, int>(getter, setter), allownegative, listgetter)
    {}
};


// Property for editing images in an imagelist.
class ImagelistImagesDesignProperty : public GeneralDesignProperty<Imagelist, byte*>
{
private:
    typedef GeneralDesignProperty<Imagelist, byte*>    base;
public:
    ImagelistImagesDesignProperty(const std::wstring &name, const std::wstring &category) : base(name, category, NULL)
    {
        this->propertystyle << psReadonly << psEditButton << psInnerBinary << psBinaryResource;
        this->propertystyle -= psEditShared;
    }

    virtual bool IsDefaultWrite(Object *propholder)
    {
        Imagelist *list = dynamic_cast<Imagelist*>(propholder);
        if (!list || !list->HandleCreated())
            return true;
        return base::IsDefaultWrite(propholder);
    }
    
    virtual std::wstring Value(Object *propholder)
    {
        Imagelist *list = dynamic_cast<Imagelist*>(propholder);
        int cnt = list->Count();
        if (!cnt)
            return L"Empty";

        std::wstringstream ss;
        ss << L"Count: " << cnt;
        return ss.str();
    }

    virtual void BinaryValue(Object *propholder, std::vector<byte> &result)
    {
        Imagelist *list = dynamic_cast<Imagelist*>(propholder);
        if (!list->HandleCreated())
            throw L"No handle for imagelist.";
        IStream *stream;

        if (CreateStreamOnHGlobal(NULL, true, &stream) != S_OK)
            return;

        list->WriteToStream(stream);
        //ImageList_WriteEx(list->Handle(), ILP_NORMAL, stream);

        HGLOBAL mem;
        if (GetHGlobalFromStream(stream, &mem) != S_OK)
        {
            stream->Release();
            return;
        }
        SIZE_T ss = GlobalSize(mem); // Size of all the data in the imagelist.
        char *c = (char*)GlobalLock(mem); // The imagelist data.

        uLongf ds = compressBound(ss);
        int headersize = sizeof(ss);
        result.resize(headersize + ds);
        byte *arr = &result[0];
        *(SIZE_T*)arr = ss;
        arr += sizeof(SIZE_T);

        bool error = compress(arr, &ds, (Bytef*)c, ss) != Z_OK;

        GlobalUnlock(mem);
        stream->Release();

        if (error)
            throw L"Error compressing imagelist data";

        result.resize(headersize + ds);

        return;
    }

    virtual void SetBinaryValue(Form *parentform, Object *propholder, std::vector<byte> &val)
    {
        byte *arr = &val[0];
        SIZE_T ss = *(SIZE_T*)arr;
        arr += sizeof(ss);

        HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, ss);
        if (!mem)
            return;

        char *glob = (char*)GlobalLock(mem);
        uLongf fsize = ss;
        bool error = uncompress((Bytef*)glob, &fsize, arr, val.size() - sizeof(ss)) != Z_OK;
        GlobalUnlock(mem);

        if (error)
        {
            GlobalFree(mem);
            return;
        }

        IStream *stream;
        CreateStreamOnHGlobal(mem, false, &stream);
            
        Imagelist *list = dynamic_cast<Imagelist*>(propholder);

        list->ReadFromStream(stream);

        //IImageList *iimage;
        //ImageList_ReadEx(ILP_NORMAL, stream, IID_IImageList, (void**)&iimage);
        stream->Release();
        GlobalFree(mem);
    }

    virtual void ResourceValue(Object *propholder, std::vector<byte> &result)
    {
        VectorIStream *stream = new VectorIStream(result);
        Imagelist *list = dynamic_cast<Imagelist*>(propholder);
        list->WriteToStream(stream);
        stream->Release();
    }

    virtual std::wstring ResourceExportValue(Object *propholder, int resourceid)
    {
        return L"NULL, " + base::ResourceExportValue(propholder, resourceid);
    }

    virtual bool ClickEdit(Form *parentform, Object *propholder)
    {
        Imagelist *list = dynamic_cast<Imagelist*>(propholder);
        if (imagelisteditordialog->Show(parentform, list))
        {
            *list = std::move(*imagelisteditordialog->Images()); //->CopyTo(*list, true);
            return true;
        }
        return false;
    }

};


}
/* End of NLIBNS */


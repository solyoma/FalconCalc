#include "stdafx_zoli.h"

#include "comdata.h"
#include "controlbase.h"
#include "events.h" // For keyboard state enum.


//---------------------------------------------


namespace NLIBNS
{


BasicDataObject::BasicDataObject() : refcnt(1)/*, formatcnt(0), formats(NULL), stgmed(NULL), mustrelease(NULL)*/
{
}

BasicDataObject::~BasicDataObject()
{
    //for (int ix = 0; ix < formatcnt; ++ix)
    //    if (mustrelease[ix])
    //        ReleaseStgMedium(stgmed + ix);
    //delete[] formats;
    //delete[] stgmed;
    //delete[] mustrelease;
    for (auto it = items.begin(); it != items.end(); ++it)
        ReleaseStgMedium(&(*it).medium);
}

/* IUnknown implementation */

HRESULT __stdcall BasicDataObject::QueryInterface (REFIID iid, void **ppvObject)
{
    if (iid == IID_IUnknown || iid == IID_IDataObject)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG  __stdcall BasicDataObject::AddRef()
{
    InterlockedIncrement(&refcnt);
    return refcnt;
}

ULONG  __stdcall BasicDataObject::Release()
{
    LONG nrefcnt = InterlockedDecrement(&refcnt);
    if (0 == nrefcnt)
    {
        delete this;
    }
    return nrefcnt;
}

/* End of IUnknown. */

std::vector<BasicDataObject::DataItem>::iterator BasicDataObject::FormatPos(FORMATETC *fmt)
{
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        const DataItem &item = *it;
        if ((fmt->tymed & item.format.tymed) != 0 && fmt->cfFormat == item.format.cfFormat && fmt->dwAspect == item.format.dwAspect)
            return it;
    }

    return items.end();

    //for (int ix = 0; ix < formatcnt; ++ix)
    //{
    //    if ((fmt->tymed & formats[ix].tymed) != 0 && fmt->cfFormat == formats[ix].cfFormat && fmt->dwAspect == formats[ix].dwAspect)
    //        return ix;
    //}
    //return -1;
}

HRESULT __stdcall BasicDataObject::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease)
{
    if (!pFormatEtc)
        return DV_E_FORMATETC;
    if (!pMedium)
        return E_INVALIDARG;
    if (pFormatEtc->tymed != pMedium->tymed)
        return DV_E_TYMED;

    //FORMATETC *formattmp = formats;
    //STGMEDIUM *stgmedtmp = stgmed;
    //bool *mustreleasetmp = mustrelease;
    try
    {
        auto it = FormatPos(pFormatEtc);
        if (it != items.end())
            ReleaseStgMedium(&(*it).medium);

        DataItem item;
        item.format = *pFormatEtc;
        if (!fRelease)
            CopyStgMedium(pMedium, &item.medium);
        else
            item.medium = *pMedium;

        if (it == items.end())
            items.push_back(item);
        else
            *it = item;

        //int ix = FormatIndex(pFormatEtc);
        //if (ix < 0)
        //{
        //    ++formatcnt;
        //    formats = new FORMATETC[formatcnt];
        //    stgmed = new STGMEDIUM[formatcnt];
        //    mustrelease = new bool[formatcnt];

        //    memcpy(formats, formattmp, sizeof(FORMATETC) * (formatcnt - 1));
        //    memcpy(stgmed, stgmedtmp, sizeof(STGMEDIUM) * (formatcnt - 1));
        //    memcpy(mustrelease, mustreleasetmp, sizeof(bool) * (formatcnt - 1));
        //    ix = formatcnt - 1;
        //}
        //else
        //    ReleaseStgMedium(stgmed + ix);

        //if (!fRelease)
        //    CopyStgMedium(pMedium, stgmed + ix);
        //else
        //    stgmed[ix] = *pMedium;

        //formats[ix] = *pFormatEtc;
        // //stgmed[ix] = *pMedium;
        //mustrelease[ix] = fRelease != TRUE;

        // //if (!fRelease)
        // //{
        // //    if (CopyMedium(pMedium, &stgmed[ix]) != S_OK)
        // //        throw "Error copying medium!";

        // //    stgmed[ix].pUnkForRelease = NULL;
        // //}

        //delete[] formattmp;
        //delete[] stgmedtmp;
        //delete[] mustreleasetmp;
    }
    catch(...)
    {
        //if (formattmp != formats)
        //{
        //    delete[] formats;
        //    formats = formattmp;
        //}
        //if (stgmed != stgmedtmp)
        //{
        //    delete[] stgmed;
        //    stgmed = stgmedtmp;
        //}
        //if (mustrelease != mustreleasetmp)
        //{
        //    delete[] mustrelease;
        //    mustrelease = mustreleasetmp;
        //}
        //formatcnt--;
        return E_FAIL;
    }

    return S_OK;
}

HRESULT __stdcall BasicDataObject::QueryGetData(FORMATETC *pFormatEtc)
{
    return FormatPos(pFormatEtc) == items.end() ? DV_E_FORMATETC : S_OK;
}

HRESULT __stdcall BasicDataObject::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pmedium)
{
    auto it = FormatPos(pFormatEtc);
 
    if (it == items.end())
        return DV_E_FORMATETC;
 
    //pmedium->tymed = (*it).format.tymed;
    //pmedium->pUnkForRelease = 0;
 
    HRESULT r = CopyStgMedium(&(*it).medium, pmedium);
    return r;
}

//HRESULT BasicDataObject::CopyMedium(STGMEDIUM *src, STGMEDIUM *dest)
//{
//    SIZE_T siz;
//    LPVOID mem;
//    LPVOID nmem;
//    LPMETAFILEPICT shm;
//    LPMETAFILEPICT dhm;
//    BYTE *bits;
//
//    switch(src->tymed)
//    {
//    /* TODO implement other TYMED_ consts */
//    case TYMED_HGLOBAL:
//        siz = GlobalSize(src->hGlobal);
//        dest->hGlobal = GlobalAlloc(GMEM_MOVEABLE, siz);
//        if (!dest->hGlobal)
//            return E_OUTOFMEMORY;
//
//        mem = GlobalLock(src->hGlobal);
//        nmem = GlobalLock(dest->hGlobal);
//
//        memcpy(nmem, mem, siz);
//        GlobalUnlock(src->hGlobal);
//        GlobalUnlock(dest->hGlobal);
//        return S_OK;
//    case TYMED_GDI: /* HBITMAP */
//        dest->hBitmap = (HBITMAP)CopyImage(src->hBitmap, IMAGE_BITMAP, 0, 0, 0);
//        if (dest->hBitmap)
//            return S_OK;
//        return E_UNEXPECTED;
//    case TYMED_ENHMF:
//        siz = GetEnhMetaFileBits(src->hEnhMetaFile, 0, NULL);
//        bits = new BYTE[siz];
//        if (GetEnhMetaFileBits(src->hEnhMetaFile, siz, bits) != siz)
//        {
//            delete[] bits;
//            return E_OUTOFMEMORY;
//        }
//        dest->hEnhMetaFile = SetEnhMetaFileBits(siz, bits);
//        delete[] bits;
//        if (dest->hEnhMetaFile)
//            return S_OK;
//        return E_UNEXPECTED;
//    case TYMED_MFPICT:
//        shm = (LPMETAFILEPICT)GlobalLock(src->hMetaFilePict);
//        siz = GetMetaFileBitsEx(shm->hMF, 0, NULL);
//        bits = new BYTE[siz];
//        if (!bits || GetMetaFileBitsEx(shm->hMF, siz, bits) != siz)
//        {
//            delete[] bits;
//            GlobalUnlock(src->hMetaFilePict);
//            return E_OUTOFMEMORY;
//        }
//
//        dest->hMetaFilePict = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
//        if (!dest->hMetaFilePict)
//        {
//            delete[] bits;
//            GlobalUnlock(src->hMetaFilePict);
//            return E_OUTOFMEMORY;
//        }
//
//        dhm = (LPMETAFILEPICT)GlobalLock(dest->hMetaFilePict);
//        *dhm = *shm;
//        GlobalUnlock(src->hMetaFilePict);
//        dhm->hMF = SetMetaFileBitsEx(siz, bits);
//        GlobalUnlock(dest->hMetaFilePict);
//        delete[] bits;
//
//        return S_OK;
//    default:
//        return DV_E_FORMATETC;
//    }
//}

//void BasicDataObject::ReleaseMedium(STGMEDIUM *med)
//{
//    med->pUnkForRelease = NULL;
//    ReleaseStgMedium(med);
//    //switch(med->tymed)
//    //{
//    // /* TODO implement other TYMED_ consts */
//    //case TYMED_HGLOBAL:
//    //    GlobalFree(med->hGlobal);
//    //    break;
//    //}
//}

HRESULT __stdcall BasicDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
    if (dwDirection != DATADIR_GET)
    {
        ppEnumFormatEtc = NULL;
        return E_NOTIMPL;
    }


    FORMATETC *formats = new FORMATETC[items.size()];
    if (!formats)
        return E_OUTOFMEMORY;

    int ix = 0;
    for (auto it = items.begin(); it != items.end(); ++it, ++ix)
    {
        formats[ix] = (*it).format;
        if ((*it).format.ptd)
        {
            formats[ix].ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
            *formats[ix].ptd = *(*it).format.ptd;
        }
    }
    *ppEnumFormatEtc = new BasicEnumFormatEtc(formats, items.size());

    return S_OK;
}

HRESULT BasicDataObject::DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
 
HRESULT BasicDataObject::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
 
HRESULT BasicDataObject::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT BasicDataObject::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
    // TODO implement this when IStream or IStorage will be supported later.
    return DATA_E_FORMATETC;
}

HRESULT BasicDataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
    pFormatEtcOut->ptd = NULL;
    return E_NOTIMPL;
}

//IEnumFORMATETC* BasicDataObject::CreateEnumFormatEtc(FORMATETC *formats, int formatcnt)
//{
//    return new BasicEnumFormatEtc(formats, formatcnt);
//}

//---------------------------------------------


GlobalDataObject::GlobalDataObject() : base()
{
}

void GlobalDataObject::AddFormat(CLIPFORMAT format, HGLOBAL alloced, bool shared)
{
    FORMATETC fmt = { format, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM med = {0};
    med.tymed = TYMED_HGLOBAL;
    med.hGlobal = alloced;

    SetData(&fmt, &med, !shared);
}


//---------------------------------------------


TextDataObject::TextDataObject(const std::wstring &texttoclipboard)
{
    HGLOBAL text = GlobalAlloc(GMEM_MOVEABLE, (texttoclipboard.length() + 1) * sizeof(wchar_t));
    if (text == NULL)
        throw L"Out of memory.";
    wchar_t *wch = (wchar_t*)GlobalLock(text);
    if (wch == NULL)
    {
        GlobalFree(text);
        throw L"Out of memory.";
    }
    wcscpy(wch, texttoclipboard.c_str());
    GlobalUnlock(text);

    AddFormat(CF_UNICODETEXT, text);

    std::string str = WideToANSI(texttoclipboard);
    text = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
    if (text == NULL)
        throw L"Out of memory.";

    char *ch = (char*)GlobalLock(text);
    strcpy(ch, str.c_str());
    GlobalUnlock(text);

    AddFormat(CF_TEXT, text);
}

TextDataObject::TextDataObject(const std::string &texttoclipboard)
{
    HGLOBAL text = GlobalAlloc(GMEM_MOVEABLE, texttoclipboard.length() + 1);
    if (text == NULL)
        throw L"Out of memory.";
    char *ch = (char*)GlobalLock(text);
    if (ch == NULL)
    {
        GlobalFree(text);
        throw L"Out of memory.";
    }
    strcpy(ch, texttoclipboard.c_str());
    GlobalUnlock(text);

    AddFormat(CF_TEXT, text);
}

TextDataObject::TextDataObject(const wchar_t *texttoclipboard, int textlen)
{
    HGLOBAL text = GlobalAlloc(GMEM_MOVEABLE, (textlen + 1) * sizeof(wchar_t));
    if (text == NULL)
        throw L"Out of memory.";
    wchar_t *wch = (wchar_t*)GlobalLock(text);
    if (wch == NULL)
    {
        GlobalFree(text);
        throw L"Out of memory.";
    }
    wcsncpy(wch, texttoclipboard, textlen);
    wch[textlen] = 0;
    GlobalUnlock(text);

    AddFormat(CF_UNICODETEXT, text);

    char* str = WideToANSI(texttoclipboard, textlen);
    text = NULL;
    if (str)
        text = GlobalAlloc(GMEM_MOVEABLE, textlen + 1);
    if (text == NULL)
    {
        delete[] str;
        throw L"Out of memory.";
    }

    char *ch = (char*)GlobalLock(text);
    if (ch == NULL)
    {
        delete[] str;
        throw L"Out of memory.";
    }

    strncpy(ch, str, textlen);
    ch[textlen] = 0;
    GlobalUnlock(text);
    delete[] str;

    AddFormat(CF_TEXT, text);
}

TextDataObject::TextDataObject(const char *texttoclipboard, int textlen)
{
    HGLOBAL text = GlobalAlloc(GMEM_MOVEABLE, textlen + 1);
    if (text == NULL)
        throw L"Out of memory.";
    char *ch = (char*)GlobalLock(text);
    if (ch == NULL)
    {
        GlobalFree(text);
        throw L"Out of memory.";
    }
    strncpy(ch, texttoclipboard, textlen);
    ch[textlen] = 0;
    GlobalUnlock(text);

    AddFormat(CF_TEXT, text);
}


//---------------------------------------------


BasicEnumFormatEtc::BasicEnumFormatEtc(FORMATETC *formats, int formatcnt) : refcnt(1), formats(formats), formatcnt(formatcnt), enumpos(0)
{
    //formats = new FORMATETC[formatcnt];

    //for (int ix = 0; ix < formatcnt; ++ix)
    //{
    //    formats[ix] = aformats[ix];
    //    if (aformats[ix].ptd)
    //    {
    //        formats[ix].ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
    //        *formats[ix].ptd = *aformats[ix].ptd;
    //    }
    //}
}

BasicEnumFormatEtc::~BasicEnumFormatEtc()
{
    for (int ix = 0; ix < formatcnt; ++ix)
        CoTaskMemFree(formats[ix].ptd);
    delete[] formats;
}

/* IUnknown implementation */

HRESULT __stdcall BasicEnumFormatEtc::QueryInterface (REFIID iid, void **ppvObject)
{
    if (iid == IID_IUnknown || iid == IID_IEnumFORMATETC)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG  __stdcall BasicEnumFormatEtc::AddRef()
{
    InterlockedIncrement(&refcnt);
    return refcnt;
}

ULONG  __stdcall BasicEnumFormatEtc::Release()
{
    LONG nrefcnt = InterlockedDecrement(&refcnt);
    if (0 == nrefcnt)
    {
        delete this;
    }
    return nrefcnt;
}

/* End of IUnknown. */

HRESULT BasicEnumFormatEtc::Reset()
{
    enumpos = 0;
    return S_OK;
}

HRESULT BasicEnumFormatEtc::Skip(ULONG celt)
{
    enumpos += celt;
    return enumpos <= formatcnt ? S_OK : S_FALSE;
}

HRESULT BasicEnumFormatEtc::Clone(IEnumFORMATETC **ppEnumFormatEtc)
{
    FORMATETC *cpy = new FORMATETC[formatcnt];

    for (int ix = 0; ix < formatcnt; ++ix)
    {
        cpy[ix] = formats[ix];
        if (formats[ix].ptd)
        {
            cpy[ix].ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
            *cpy[ix].ptd = *formats[ix].ptd;
        }
    }
    *ppEnumFormatEtc = new BasicEnumFormatEtc(formats, formatcnt);

    if (!*ppEnumFormatEtc)
        return E_OUTOFMEMORY;

    (*(BasicEnumFormatEtc**)ppEnumFormatEtc)->enumpos = enumpos;
    return S_OK;
}

HRESULT BasicEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtc, ULONG *pceltFetched)
{
    ULONG copied = 0;

    if (celt > 1 && pceltFetched == NULL)
        return S_FALSE;
 
    while(enumpos < formatcnt && copied < celt)
    {
        pFormatEtc[copied] = formats[enumpos];
        if (formats[enumpos].ptd)
        {
            pFormatEtc[copied].ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
            if (pFormatEtc[copied].ptd == NULL) // Failure to allocate memory.
                break;
            *pFormatEtc[copied].ptd = *formats[enumpos].ptd;
        }

        copied++;
        enumpos++;
    }
 
    if (pceltFetched)
        *pceltFetched = copied;
 
    return copied == celt ? S_OK : S_FALSE;
}


//---------------------------------------------


BasicDropSource::BasicDropSource() : refcnt(1)
{
}

BasicDropSource::~BasicDropSource()
{
}

/* IUnknown implementation */

HRESULT __stdcall BasicDropSource::QueryInterface (REFIID iid, void **ppvObject)
{
    if (iid == IID_IUnknown || iid == IID_IDropSource)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG  __stdcall BasicDropSource::AddRef()
{
    InterlockedIncrement(&refcnt);
    return refcnt;
}

ULONG  __stdcall BasicDropSource::Release()
{
    LONG nrefcnt = InterlockedDecrement(&refcnt);
    if (0 == nrefcnt)
    {
        delete this;
    }
    return nrefcnt;
}

/* End of IUnknown. */

HRESULT __stdcall BasicDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if(fEscapePressed == TRUE)
        return DRAGDROP_S_CANCEL;  
    if((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0)
        return DRAGDROP_S_DROP;
    return S_OK;
}

HRESULT __stdcall BasicDropSource::GiveFeedback(DWORD dwEffect)
{   
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


//---------------------------------------------


BasicDropTarget::BasicDropTarget(Control *owner) : refcnt(1), owner(owner), helper(NULL)
{
}

BasicDropTarget::~BasicDropTarget()
{
    ClearEnumFormats();
    SetHelper(false);
}

/* IUnknown implementation */

HRESULT __stdcall BasicDropTarget::QueryInterface (REFIID iid, void **ppvObject)
{
    if (iid == IID_IUnknown || iid == IID_IDropTarget)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG __stdcall BasicDropTarget::AddRef()
{
    InterlockedIncrement(&refcnt);
    return refcnt;
}

ULONG __stdcall BasicDropTarget::Release()
{
    LONG nrefcnt = InterlockedDecrement(&refcnt);
    if (0 == nrefcnt)
    {
        delete this;
    }
    return nrefcnt;
}

/* End of IUnknown. */

HRESULT __stdcall BasicDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    formatindex = DataFormatIndex(pDataObject, *pdwEffect);

    if (formatindex < 0)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    UINT vkeys = VirtualKeysFromKeyState(grfKeyState);
    Point p(pt.x, pt.y);
    *pdwEffect = owner->DragEnter(formatindex, vkeys, p, *pdwEffect) & (DragDropEffects)*pdwEffect;

    if (helper)
        helper->DragEnter(owner->Handle(), pDataObject, &p, *pdwEffect);

    return S_OK;
}

HRESULT __stdcall BasicDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (formatindex < 0)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    UINT vkeys = VirtualKeysFromKeyState(grfKeyState);
    Point p(pt.x, pt.y);
    *pdwEffect = owner->DragMove(formatindex, vkeys, p, *pdwEffect) & (DragDropEffects)*pdwEffect;

    if (helper)
        helper->DragOver(&p, *pdwEffect);

    return S_OK;
}

HRESULT __stdcall BasicDropTarget::DragLeave()
{
    owner->DragLeave();

    if (helper)
        helper->DragLeave();

    return S_OK;
}

HRESULT __stdcall BasicDropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (formatindex < 0)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    UINT vkeys = VirtualKeysFromKeyState(grfKeyState);
    Point p(pt.x, pt.y);
    *pdwEffect = owner->DragDrop(formatindex, vkeys, p, *pdwEffect, pDataObject);

    if (helper)
        helper->Drop(pDataObject, &p, *pdwEffect);

    return S_OK;
}

void BasicDropTarget::AddEnumFormat(DragDropEffectSet dropeffects, CLIPFORMAT cf, StorageMediumTypes medtype, DataViewAspects aspect)
{
    EnumFormatItem *it = new EnumFormatItem();
    it->cf = cf;
    it->aspect = aspect;
    it->medtype = medtype;
    it->dropeffects = dropeffects;
    formats.push_back(it);
}

int BasicDropTarget::EnumFormatCount()
{
    return formats.size();
}

void BasicDropTarget::DeleteEnumFormat(int ix)
{
    auto it = formats.begin() + ix;
    delete *it;
    formats.erase(it);
}

void BasicDropTarget::ClearEnumFormats()
{
    while (formats.size())
        DeleteEnumFormat(formats.size() - 1);
}

void BasicDropTarget::EnumFormat(int ix, DragDropEffectSet &dropeffects, CLIPFORMAT &cf, StorageMediumTypes &medtype, DataViewAspects &aspect)
{
    EnumFormatItem *it = formats[ix];
    cf = it->cf;
    aspect = it->aspect;
    medtype = it->medtype;
    dropeffects = it->dropeffects;
}

int BasicDropTarget::DataFormatIndex(IDataObject *data, DragDropEffectSet dropeffects)
{
    FORMATETC fmt;
    fmt.ptd = 0;
    fmt.lindex = -1;
    //fmt.tymed;
    //fmt.dwAspect;
    //fmt.cfFormat;

    int ix = 0;
    for (auto it = formats.begin(); it != formats.end(); ++it, ++ix)
    {
        if (!(((*it)->dropeffects & dropeffects) & DragDropEffectSet(ddeCopy | ddeMove | ddeLink)))
            continue;

        fmt.cfFormat = (*it)->cf;
        fmt.dwAspect = (*it)->aspect;
        fmt.tymed = (*it)->medtype;
        if (data->QueryGetData(&fmt) == S_OK)
            return ix;
    }

    return -1;
}

UINT BasicDropTarget::VirtualKeysFromKeyState(DWORD keystate)
{
    unsigned int vkeys = 0;
    if ((keystate & MK_CONTROL) == MK_CONTROL)
        vkeys |= vksCtrl;
    if ((keystate & MK_LBUTTON) == MK_LBUTTON)
        vkeys |= vksLeft;
    if ((keystate & MK_RBUTTON) == MK_RBUTTON)
        vkeys |= vksRight;
    if ((keystate & MK_MBUTTON) == MK_MBUTTON)
        vkeys |= vksMiddle;
    if ((keystate & MK_SHIFT) == MK_SHIFT)
        vkeys |= vksShift;
    if ((keystate & MK_ALT) == MK_ALT)
        vkeys |= vksAlt;
    return vkeys;
}

bool BasicDropTarget::Helper()
{
    return helper != NULL;
}

void BasicDropTarget::SetHelper(bool add)
{
    if (add == (helper != NULL))
        return;
    if (!add)
    {
        helper->Release();
        helper = NULL;
    }
    else
    {
        int res;
        if ((res = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_IDropTargetHelper, (void**)&helper)) != S_OK)
        {
            helper = NULL;
            return;
        }
    }
}


//---------------------------------------------




//---------------------------------------------


DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects)
{
    BasicDropSource *dsrc = new BasicDropSource;
    DWORD effect;
    HRESULT r = DoDragDrop(obj, dsrc, allowedeffects, &effect);
    if (r == DRAGDROP_S_DROP)
        return (DragDropEffects)effect;
    return ddeNone;
}

DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects, Bitmap *bmp, Point dragpoint)
{
    //HBITMAP hbmp = bmp->HandleCopy(Color(0, 0, 0, 0));

    // Because there is a bug in 64bit windows, if a drag image is shared between 32 and 64bit programs,
    // there is some strange artefact resulting from the 64bit version of the SHDRAGIMAGE is 8 bytes larger.
    // Let's add extra pixels to the drag image to ensure the image at least doesn't look corrupted (while
    // in reality it always is when used between 32bit and 64bit programs.)
    int hw = bmp->Width();
    int hh = bmp->Height();
    hw += 64 - (hw % 64) + 2;
    hh += (16 - (hh % 16)) + 1;

    auto lock = bmp->LockBits(glmReadOnly, PixelFormat32bppARGB);
    char *line = (char*)lock->Scan0;
    char *bits = new char[hw * hh * 4];
    memset(bits, 0, hw * hh * 4);

    for (int ix = 0; ix < bmp->Height(); ++ix)
    {
        memcpy(bits + (ix) * (hw * 4) + 2 * 4, line, 4 * bmp->Width());
        line += lock->Stride;
    }
    bmp->UpdateBits();

    HDC dc = GetDC(0);
    HBITMAP hbmp = CreateCompatibleBitmap(dc, hw, hh);
    BITMAPINFO binf;
    binf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    binf.bmiHeader.biWidth = hw;
    binf.bmiHeader.biHeight = - hh;
    binf.bmiHeader.biPlanes = 1;
    binf.bmiHeader.biBitCount = 32;
    binf.bmiHeader.biCompression = BI_RGB;
    binf.bmiHeader.biSizeImage = 0;
    binf.bmiHeader.biXPelsPerMeter = 0;
    binf.bmiHeader.biYPelsPerMeter = 0;
    binf.bmiHeader.biClrUsed = 0;
    binf.bmiHeader.biClrImportant = 0;

    SetDIBits(dc, hbmp, 0, hh, bits, &binf, 0);
    delete[] bits;
    ReleaseDC(0, dc);

    DragDropEffects r = BeginDragDropOperation(obj, allowedeffects, hw, hh, hbmp, dragpoint);
    DeleteObject(hbmp);
    return r;
}

DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects, int dragimagewidth, int dragimageheight, HBITMAP dragimage, Point dragpoint)
{
    IDragSourceHelper *helper;
    int res;
    if ((res = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_IDragSourceHelper, (void**)&helper)) != S_OK)
        return BeginDragDropOperation(obj, allowedeffects);

    SHDRAGIMAGE sh;
    sh.sizeDragImage.cx = dragimagewidth;
    sh.sizeDragImage.cy = dragimageheight;
    sh.ptOffset = dragpoint;
    sh.crColorKey = CLR_NONE;
    sh.hbmpDragImage = dragimage;

    helper->InitializeFromBitmap(&sh, obj);

    BasicDropSource *dsrc = new BasicDropSource;
    DWORD effect;
    HRESULT r = DoDragDrop(obj, dsrc, allowedeffects, &effect);

    helper->Release();

    if (r == DRAGDROP_S_DROP)
        return (DragDropEffects)effect;
    return ddeNone;
}

DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects, HWND imageowner)
{
    IDragSourceHelper *helper;
    int res;
    if ((res = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_IDragSourceHelper, (void**)&helper)) != S_OK)
        return BeginDragDropOperation(obj, allowedeffects);

    helper->InitializeFromWindow(imageowner, NULL, obj);

    BasicDropSource *dsrc = new BasicDropSource;
    DWORD effect;
    HRESULT r = DoDragDrop(obj, dsrc, allowedeffects, &effect);

    helper->Release();

    if (r == DRAGDROP_S_DROP)
        return (DragDropEffects)effect;
    return ddeNone;
}

void FillFormatEtc(FORMATETC &etc, CLIPFORMAT cf, StorageMediumTypes medtype, DataViewAspects aspect)
{
    etc.cfFormat = cf;
    etc.dwAspect = aspect;
    etc.lindex = -1;
    etc.ptd = NULL;
    etc.tymed = medtype;
}

bool DataObjectContainsFormat(IDataObject *obj, CLIPFORMAT cf, StorageMediumTypes medtype, DataViewAspects aspect)
{
    FORMATETC etc = { cf, 0, aspect, -1, medtype };
    return obj->QueryGetData(&etc) == S_OK;
}


//---------------------------------------------


}
/* End of NLIBNS */


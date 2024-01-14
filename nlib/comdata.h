#pragma once
#include <ShlObj.h>


namespace NLIBNS
{


/*#ifdef __MINGW32__
        STDAPI CopyStgMedium(const STGMEDIUM * pcstgmedSrc,  
                                   STGMEDIUM * pstgmedDest); 
#endif*/

    enum DataViewAspects : DWORD {
            dvaContent = DVASPECT_CONTENT,
            dvaThumbnail = DVASPECT_THUMBNAIL,
            dvaIcon = DVASPECT_ICON,
            dvaDocprint = DVASPECT_DOCPRINT,
    };

    enum StorageMediumTypes : DWORD {
            smtHGlobal = TYMED_HGLOBAL,
            smtFile = TYMED_FILE,
            smtIStream = TYMED_ISTREAM,
            smtIStorage = TYMED_ISTORAGE,
            smtGDI = TYMED_GDI,
            smtMetafile = TYMED_MFPICT,
            smtEnhancedMetafile = TYMED_ENHMF,
            smtNone = TYMED_NULL,
    };

#ifdef __MINGW32__
    enum DragDropEffects : unsigned int {
#else
    enum DragDropEffects {
#endif
            ddeNone = 0, //DROPEFFECT_NONE,
            ddeCopy = 1, //DROPEFFECT_COPY,
            ddeMove = 2, //DROPEFFECT_MOVE,
            ddeLink = 4, //DROPEFFECT_LINK,
            // Above this all values are additional states not effects of the dropping.
            ddeScroll = 0x80000000 //DROPEFFECT_SCROLL,
    };
    typedef uintset<DragDropEffects> DragDropEffectSet;


    class BasicDataObject : public IDataObject
    {
    public:
        HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
        ULONG  __stdcall AddRef();
        ULONG  __stdcall Release();

        HRESULT __stdcall GetData(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
        HRESULT __stdcall GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
        HRESULT __stdcall QueryGetData(FORMATETC *pFormatEtc);
        HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut);
        HRESULT __stdcall SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease);
        HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
        HRESULT __stdcall DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink*, DWORD*);
        HRESULT __stdcall DUnadvise(DWORD dwConnection);
        HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppEnumAdvise);

        BasicDataObject();
        virtual ~BasicDataObject();
    private:
        LONG refcnt;

        //LONG formatcnt;

        struct DataItem
        {
            FORMATETC format;
            STGMEDIUM medium;
        };
        // Arrays of size formatcnt:
        //FORMATETC *formats; // The format structures in the data object.
        //STGMEDIUM *stgmed; // The data in the data object.
        //bool *mustrelease; // The data must be released by ourselves when freeing memory.

        std::vector<DataItem> items;

        std::vector<DataItem>::iterator FormatPos(FORMATETC *fmt);
        //HRESULT CopyMedium(STGMEDIUM *src, STGMEDIUM *dest);
        //void ReleaseMedium(STGMEDIUM *med);
        //IEnumFORMATETC* CreateEnumFormatEtc(FORMATETC *formats, int formatcnt);
    };

    // Class for copying and pasting data allocated with HGLOBAL in any given clipboard format.
    class GlobalDataObject : public BasicDataObject
    {
    private:
        typedef BasicDataObject    base;
    public:
        GlobalDataObject();
        void AddFormat(CLIPFORMAT format, HGLOBAL alloced, bool shared = false); // Pass the clipboard format and an allocated HGLOBAL value. If shared is false, do not free the HGLOBAL after the call as it will be now owned by the data object.
    };

    // Class for placing and reading textual data from the clipboard or with drag and drop operations.
    class TextDataObject : public GlobalDataObject
    {
    private:
        typedef GlobalDataObject    base;
    public:
        TextDataObject(const std::wstring &texttoclipboard);
        TextDataObject(const std::string &texttoclipboard);
        TextDataObject(const wchar_t *texttoclipboard, int textlen);
        TextDataObject(const char *texttoclipboard, int textlen);
    };

    class BasicEnumFormatEtc : public IEnumFORMATETC
    {
    public:
        HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
        ULONG  __stdcall AddRef();
        ULONG  __stdcall Release();

        HRESULT __stdcall Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched);
        HRESULT __stdcall Skip(ULONG celt);
        HRESULT __stdcall Reset();
        HRESULT __stdcall Clone(IEnumFORMATETC **ppEnumFormatEtc);

        BasicEnumFormatEtc(FORMATETC *formats, int formatcnt);
        virtual ~BasicEnumFormatEtc();
    private:
        LONG refcnt;

        FORMATETC *formats;
        int formatcnt;
        int enumpos; // Current position while enumerating.
    };

    class BasicDropSource : public IDropSource
    {
    public:
        HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
        ULONG  __stdcall AddRef();
        ULONG  __stdcall Release();

        virtual HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
        virtual HRESULT __stdcall GiveFeedback(DWORD dwEffect);

        BasicDropSource();
        virtual ~BasicDropSource();
    private:
        LONG refcnt;

    };

    class Control;
    class BasicDropTarget : public IDropTarget
    {
    public:
        HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
        ULONG  __stdcall AddRef();
        ULONG  __stdcall Release();

        HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
        HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
        HRESULT __stdcall DragLeave();
        HRESULT __stdcall Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

        BasicDropTarget(Control *owner);
        virtual ~BasicDropTarget();

        void AddEnumFormat(DragDropEffectSet dropeffects, CLIPFORMAT cf, StorageMediumTypes medtype, DataViewAspects aspect); // Register a format which the drag drop control accepts.
        int EnumFormatCount();
        void DeleteEnumFormat(int ix);
        void ClearEnumFormats();
        void EnumFormat(int ix, DragDropEffectSet &dropeffects, CLIPFORMAT &cf, StorageMediumTypes &medtype, DataViewAspects &aspect);

        bool Helper(); // Does this object use an IDropTargetHelper to show the drag drop image?
        void SetHelper(bool add); // Set whether you want to show the drag drop image when the mouse is over the owner control in a drag drop operation.
    private:
        LONG refcnt;

        Control *owner; // Drop target control which will accept drag-drop input.
        IDropTargetHelper *helper; // Helper object for showing a drag drop image.

        struct EnumFormatItem
        {
            CLIPFORMAT cf;
            DataViewAspects aspect;
            StorageMediumTypes medtype;
            DragDropEffectSet dropeffects;
        };
        std::vector<EnumFormatItem*> formats;
        int formatindex; // The index of the format currently being dragged over our control if dataneeded is true.

        int DataFormatIndex(IDataObject *data, DragDropEffectSet dropeffects);
        UINT VirtualKeysFromKeyState(DWORD keystate);
    };

    class Bitmap;
    DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects);
    DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects, Bitmap *bmp, Point dragpoint);
    DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects, int dragimagewidth, int dragimageheight, HBITMAP dragimage, Point dragpoint);
    DragDropEffects BeginDragDropOperation(BasicDataObject *obj, DragDropEffectSet allowedeffects, HWND imageowner);

    void FillFormatEtc(FORMATETC &etc, CLIPFORMAT cf, StorageMediumTypes medtype = smtHGlobal, DataViewAspects aspect = dvaContent); // Fills a formatetc struct with the given media type information.
    bool DataObjectContainsFormat(IDataObject *obj, CLIPFORMAT cf, StorageMediumTypes medtype = smtHGlobal, DataViewAspects aspect = dvaContent); // Returns true if the passed data object contains the aspect of the clipboard format in the given medium.


}
/* End of NLIBNS */


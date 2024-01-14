#include "stdafx_zoli.h"
#include "utility.h"
#include "controlbase.h"

#ifdef DESIGNING
#include "designproperties.h"
#include "serializer.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


static bool registered = false;
static std::map<HWND, MessageWindowBase*> handles;


//---------------------------------------------


static LRESULT CALLBACK MsgFormWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto mapit = handles.find(hwnd);
    if (mapit == handles.end() || mapit->first != hwnd)
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    else
        return mapit->second->PassMessage(uMsg, wParam, lParam);
}

MessageWindowBase::MessageWindowBase() : handle(NULL)
{
}

MessageWindowBase::~MessageWindowBase()
{
    DestroyHandle();
}

void MessageWindowBase::CreateHandle()
{
    if (!registered)
    {
        registered = true;
        WNDCLASSEX classex;
        classex.cbSize = sizeof(WNDCLASSEXW);
        classex.style = CS_CLASSDC;
        classex.lpfnWndProc = &MsgFormWndProc;
        classex.cbClsExtra = 0;
        classex.cbWndExtra = 0;
        classex.hInstance = hInstance;
        classex.hIcon = NULL;
        classex.hIconSm = NULL;
        classex.hCursor = NULL;
        classex.hbrBackground = NULL;
        classex.lpszMenuName = NULL;
        classex.lpszClassName = L"MessageOnlyForm";
        if (!RegisterClassEx(&classex))
            throw L"Couldn't register a message only form!";
    }

    handle = CreateWindowEx(WS_EX_NOACTIVATE, L"MessageOnlyForm", L"", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    handles[handle] = this;
}

/**
 * After the call to this function, HandleCreated() will return \a false until Handle() is called
 * again to re-create the handle for the object.
 */
void MessageWindowBase::DestroyHandle()
{
    if (!handle)
        return;

    handles.erase(handle);
    DestroyWindow(handle);
    handle = NULL;
}

/**
 * If the handler of a message is written in a way that it doesn't use the handle for the object,
 * calling PassMessage() can avoid creating the handle.
 * \param uMsg The message identifier.
 * \param wParam The \a WPARAM part of the message.
 * \param lParam the \a LPARAM part of the message.
 * \sa HandleMessage(), Handle()
 */
LRESULT MessageWindowBase::PassMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMessage(uMsg, wParam, lParam);
}

/**
 * In case there was no handle created before the call, the function creates it.
 * \return The handle created by the system.
 * \sa HandleCreated(), DestroyHandle()
 */
HWND MessageWindowBase::Handle()
{
    if (!handle)
        CreateHandle();
    return handle;
}

/**
 * Call this function if creating the handle for the object can be avoided. PassMessage() can pass
 * messages directly to the object without using the program's message loop, in which case a handle
 * might not be needed.
 * \return Whether a handle has been created for the object.
 * \sa Handle()
 */
bool MessageWindowBase::HandleCreated()
{
    return handle != NULL;
}

//---------------------------------------------

sparse_list<Timer*>::size_type Timer::TimerMessageWindow::nextid = 0;

LRESULT Timer::TimerMessageWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_TIMER && wParam > 0 && wParam <= timers.size())
    {
        if (timers.is_set(wParam - 1))
            timers[wParam - 1]->Time();
    }

    return 0;
}

UINT_PTR Timer::TimerMessageWindow::AddTimer(Timer *timer)
{
    UINT_PTR result;

    auto it = timers.spos(nextid).base();
    sparse_list<Timer*>::size_type startid;
    if (it == timers.end() || (startid = (it - timers.begin())) < nextid)
    {
        result = it == timers.end() ? 0 : startid + 1;
        if (result != timers.size())
            timers[result] = timer;
        else
            timers.push_back(timer);
    }
    else
    {
        if (nextid == timers.max_size() - 1)
            result = 0;
        else
            result = nextid + 1;
        while (timers.is_set(result))
        {
            if (result == timers.max_size() - 1)
                result = 0;
            else
                ++result;
            if (result == startid)
                throw L"All timer positions are taken!";
        }
        if (result != timers.size())
            timers[result] = timer;
        else
            timers.push_back(timer);
    }

    if (result != timers.max_size() - 1)
        nextid = result + 1;
    else
        nextid = 0;

    return result + 1;
}

void Timer::TimerMessageWindow::RemoveTimer(UINT_PTR id)
{
    timers.unset(id - 1);
}


//---------------------------------------------


Timer::TimerMessageWindow Timer::handler;

#ifdef DESIGNING
void Timer::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->Add(L"SetEnabled", new BoolDesignProperty<Timer>(L"Enabled", L"Control", &Timer::Enabled, &Timer::SetEnabled));
    serializer->Add(L"SetInterval", new UnsignedIntDesignProperty<Timer>(L"Interval", L"Control", &Timer::Interval, &Timer::SetInterval))->MakeDefault();

    serializer->AddEvent<Control, NotifyEvent>(L"OnTime", L"Control");
}
#endif
Timer::Timer() : enabled(false), interval(1000)
{
    id = handler.AddTimer(this);
}

Timer::~Timer()
{
    handler.RemoveTimer(id);
}

/**
 * Only timers that are enabled receive the WM_TIMER message from the system and call the OnTime event.
 * \sa SetEnabled()
 */
bool Timer::Enabled()
{
    return enabled;
}

/**
 * Only timers that are enabled receive the WM_TIMER message from the system and call the OnTime event.
 * \sa Enabled()
 */
void Timer::SetEnabled(bool newenabled)
{
    if (enabled == newenabled)
        return;
    enabled = newenabled;
#ifdef DESIGNING
    if (Designing())
        return;
#endif

    if (!enabled)
    {
        if (handler.HandleCreated())
            KillTimer(handler.Handle(), id);
        return;
    }

    SetTimer(handler.Handle(), id, interval, NULL);
}

/**
 * The system sends a WM_TIMER message to the timer in the program's message loop, so the timer might not
 * be precise to the exact value. If the program stops handling messages temporarily for some reason, it is possible
 * that several timer messages arrive at once when the message loop is read again.
 * \sa SetInterval()
 */
unsigned int Timer::Interval()
{
    return interval;
}

/**
 * The system sends a WM_TIMER message to the timer in the program's message loop, so the timer might not
 * be precise to the exact value. If the program stops handling messages temporarily for some reason, it is possible
 * that several timer messages arrive at once when the message loop is read again.
 * \sa Interval()
 */
void Timer::SetInterval(unsigned int newinterval)
{
    if (interval == newinterval)
        return;
    interval = newinterval;
#ifdef DESIGNING
    if (Designing())
        return;
#endif
    if (enabled)
        SetTimer(handler.Handle(), id, interval, NULL);
}

void Timer::Time()
{
    if (OnTime)
        OnTime(this, EventParameters());
}

// Event Documentation:

/** \var Timer::OnTime
 * Set the OnTime event to execute code at given intervals while the timer is enabled.
 * \sa Enabled(), SetEnabled(), Interval(), SetInterval()
 */



//---------------------------------------------


/**
 * Creates a stream with a vector for storage. The vector is updated on Write() and SetSize().
 * If you want to create a read-only IStream, use MemoryIStream instead.
 * \param buf A vector used as the storage for the stream.
 */
VectorIStream::VectorIStream(std::vector<byte> &buf) : refcnt(1), buf(buf), pos(0)
{
}

VectorIStream::~VectorIStream()
{
}

HRESULT STDMETHODCALLTYPE VectorIStream::QueryInterface(REFIID riid, void **ppvObject)
{
    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    if (riid == IID_IStream)
    {
        *ppvObject = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    if (riid == IID_ISequentialStream)
    {
        *ppvObject = static_cast<ISequentialStream*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE VectorIStream::AddRef()
{
    InterlockedIncrement(&refcnt);
    return refcnt;
}

ULONG STDMETHODCALLTYPE VectorIStream::Release()
{
    LONG nrefcnt = InterlockedDecrement(&refcnt);
    if (0 == nrefcnt)
    {
        delete this;
    }
    return nrefcnt;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    if (!pv || !pcbRead)
        return STG_E_INVALIDPOINTER;
    ULONG ccb = cb; // Save in case pcbRead points to cb.

    *pcbRead = min(ccb, max(0, buf.size() - pos));

    if (*pcbRead)
    {
        byte *arr = &buf[0];
        if ((unsigned int)pv + *pcbRead > (unsigned int)arr + pos && (unsigned int)pv < (unsigned int)arr + pos + *pcbRead) // Read and write memory areas overlap. Use memmove for safety.
            memmove(pv, arr + pos, *pcbRead);
        else
            memcpy(pv, arr + pos, *pcbRead);

        pos += *pcbRead;
    }

    return *pcbRead < ccb ? S_FALSE : S_OK;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
    ULONG ccb = cb; // Save in case pcbWritten points to cb.
    if (pcbWritten)
        *pcbWritten = 0;

    if (!pv)
        return STG_E_INVALIDPOINTER;

    if (!ccb)
        return S_OK;

    unsigned int newsize = max(pos + ccb, buf.size());
    if (newsize > buf.size())
    {
        if (newsize > buf.max_size())
            newsize = buf.max_size();
        try
        {
            buf.resize(newsize);
        }
        catch(...)
        {
            return STG_E_CANTSAVE;
        }
    }

    unsigned int writecnt = min(ccb, max(0, newsize - pos));

    byte *arr = &buf[0];
    if ((unsigned int)pv + writecnt > (unsigned int)arr + pos && (unsigned int)pv < (unsigned int)arr + pos + writecnt) // Read and write memory areas overlap. Use memmove for safety.
        memmove(arr + pos, pv, writecnt);
    else
        memcpy(arr + pos, pv, writecnt);

    if (pcbWritten)
        *pcbWritten = writecnt;

    pos += writecnt;

    if (ccb != writecnt)
        return STG_E_MEDIUMFULL;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    ULONGLONG newpos;
    HRESULT error = S_OK;
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        newpos = (ULONGLONG)dlibMove.QuadPart;
        break;
    case STREAM_SEEK_CUR:
        if (-dlibMove.QuadPart > pos)
            error = STG_E_INVALIDFUNCTION;
        else
            newpos = pos + dlibMove.QuadPart;
        break;
    case STREAM_SEEK_END:
        if (-dlibMove.QuadPart > buf.size())
            return STG_E_INVALIDFUNCTION;
        else
            newpos = buf.size() + dlibMove.QuadPart;
        break;
    default:
        error = STG_E_INVALIDFUNCTION;
    }

    if (error == S_OK && (unsigned int)newpos == newpos && newpos <= buf.max_size())
    {
        pos = (unsigned int)newpos;
        if (plibNewPosition)
        {
            plibNewPosition->HighPart = 0;
            plibNewPosition->QuadPart = pos;
        }
    }
    else
        error = STG_E_INVALIDFUNCTION;
    return error;
}

HRESULT STDMETHODCALLTYPE VectorIStream::SetSize(ULARGE_INTEGER libNewSize)
{
    if (libNewSize.HighPart != 0 || libNewSize.LowPart > buf.max_size())
        return STG_E_INVALIDFUNCTION;

    try
    {
        buf.resize(libNewSize.LowPart);
    }
    catch(...)
    {
        return STG_E_MEDIUMFULL;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE VectorIStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    if (!pstm)
        return STG_E_INVALIDPOINTER;

    ULONG copycnt = min(cb.LowPart, max(0, buf.size() - pos));
    ULONG copied = 0;

    HRESULT res = pstm->Write(&buf[pos], copycnt, &copied);

    pos += copied;

    if (pcbRead)
    {
        pcbRead->HighPart = 0;
        pcbRead->LowPart = copied;
    }
    if (pcbWritten)
    {
        pcbWritten->HighPart = 0;
        pcbWritten->LowPart = copied;
    }

    if (res == STG_E_ACCESSDENIED || res == STG_E_CANTSAVE || res == STG_E_WRITEFAULT)
        return STG_E_INVALIDPOINTER;

    return res;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Commit(DWORD grfCommitFlags)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Revert()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VectorIStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return STG_E_INVALIDFUNCTION;
}

HRESULT STDMETHODCALLTYPE VectorIStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return STG_E_INVALIDFUNCTION;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if (!pstatstg)
        return STG_E_INVALIDPOINTER;
    pstatstg->pwcsName = NULL;
    if (grfStatFlag != STATFLAG_DEFAULT && grfStatFlag != STATFLAG_NONAME)
        return STG_E_INVALIDFLAG;
    if (grfStatFlag == STATFLAG_DEFAULT)
    {
        pstatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR) * 14);
        if (!pstatstg->pwcsName)
            return STG_E_INSUFFICIENTMEMORY;
        wcscpy(pstatstg->pwcsName, L"VectorIStream");
    }
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.HighPart = 0;
    pstatstg->cbSize.LowPart = buf.size();
    memset(&pstatstg->mtime, 0, sizeof(FILETIME));
    memset(&pstatstg->ctime, 0, sizeof(FILETIME));
    memset(&pstatstg->atime, 0, sizeof(FILETIME));
    pstatstg->grfLocksSupported = 0;
    pstatstg->grfMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    pstatstg->clsid = CLSID_NULL;
    pstatstg->grfStateBits = 0;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE VectorIStream::Clone(IStream **ppstm)
{
    if (!ppstm)
        return STG_E_INVALIDPOINTER;
    *ppstm = new VectorIStream(buf);
    if (!*ppstm)
        return STG_E_INSUFFICIENTMEMORY;
    ((VectorIStream*)ppstm)->pos = pos;

    return S_OK;
}

//---------------------------------------------


/**
 * Creates a stream with a memory buffer for storage. The buffer is used as if it were read-only.
 * If you want to create a read-write IStream, use VectorIStream instead.
 * \param buf A memory buffer holding data to be read.
 * \param length The size of buf in bytes.
 */
MemoryIStream::MemoryIStream(void *buf, int length) : refcnt(1), buf(buf), pos(0), length(length)
{
}

MemoryIStream::~MemoryIStream()
{
}

HRESULT STDMETHODCALLTYPE MemoryIStream::QueryInterface(REFIID riid, void **ppvObject)
{
    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    if (riid == IID_IStream)
    {
        *ppvObject = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    if (riid == IID_ISequentialStream)
    {
        *ppvObject = static_cast<ISequentialStream*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE MemoryIStream::AddRef()
{
    InterlockedIncrement(&refcnt);
    return refcnt;
}

ULONG STDMETHODCALLTYPE MemoryIStream::Release()
{
    LONG nrefcnt = InterlockedDecrement(&refcnt);
    if (0 == nrefcnt)
    {
        delete this;
    }
    return nrefcnt;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    if (!pv || !pcbRead)
        return STG_E_INVALIDPOINTER;
    ULONG ccb = cb; // Save in case pcbRead points to cb.

    *pcbRead = min(ccb, max(0, length - pos));

    if (*pcbRead)
    {
        byte *arr = (byte*)buf;
        if ((unsigned int)pv + *pcbRead > (unsigned int)arr + pos && (unsigned int)pv < (unsigned int)arr + pos + *pcbRead) // Read and write memory areas overlap. Use memmove for safety.
            memmove(pv, arr + pos, *pcbRead);
        else
            memcpy(pv, arr + pos, *pcbRead);

        pos += *pcbRead;
    }

    return *pcbRead < ccb ? S_FALSE : S_OK;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
    return STG_E_ACCESSDENIED;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    ULONGLONG newpos;
    HRESULT error = S_OK;
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        newpos = (ULONGLONG)dlibMove.QuadPart;
        break;
    case STREAM_SEEK_CUR:
        if (-dlibMove.QuadPart > pos)
            error = STG_E_INVALIDFUNCTION;
        else
            newpos = pos + dlibMove.QuadPart;
        break;
    case STREAM_SEEK_END:
        if (-dlibMove.QuadPart > length)
            return STG_E_INVALIDFUNCTION;
        else
            newpos = length + dlibMove.QuadPart;
        break;
    default:
        error = STG_E_INVALIDFUNCTION;
    }

    if (error == S_OK && (unsigned int)newpos == newpos && newpos <= length)
    {
        pos = (unsigned int)newpos;
        if (plibNewPosition)
        {
            plibNewPosition->HighPart = 0;
            plibNewPosition->QuadPart = pos;
        }
    }
    else
        error = STG_E_INVALIDFUNCTION;
    return error;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::SetSize(ULARGE_INTEGER libNewSize)
{
    //if (libNewSize.HighPart != 0 || libNewSize.LowPart > buf.max_size())
        return STG_E_INVALIDFUNCTION;

    //try
    //{
    //    buf.resize(libNewSize.LowPart);
    //}
    //catch(...)
    //{
    //    return STG_E_MEDIUMFULL;
    //}

    //return S_OK;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    if (!pstm)
        return STG_E_INVALIDPOINTER;

    ULONG copycnt = min(cb.LowPart, max(0, length - pos));
    ULONG copied = 0;

    HRESULT res = pstm->Write(((byte*)buf + pos), copycnt, &copied);

    pos += copied;

    if (pcbRead)
    {
        pcbRead->HighPart = 0;
        pcbRead->LowPart = copied;
    }
    if (pcbWritten)
    {
        pcbWritten->HighPart = 0;
        pcbWritten->LowPart = copied;
    }

    if (res == STG_E_ACCESSDENIED || res == STG_E_CANTSAVE || res == STG_E_WRITEFAULT)
        return STG_E_INVALIDPOINTER;

    return res;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Commit(DWORD grfCommitFlags)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Revert()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return STG_E_INVALIDFUNCTION;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return STG_E_INVALIDFUNCTION;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if (!pstatstg)
        return STG_E_INVALIDPOINTER;
    pstatstg->pwcsName = NULL;
    if (grfStatFlag != STATFLAG_DEFAULT && grfStatFlag != STATFLAG_NONAME)
        return STG_E_INVALIDFLAG;
    if (grfStatFlag == STATFLAG_DEFAULT)
    {
        pstatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR) * 14);
        if (!pstatstg->pwcsName)
            return STG_E_INSUFFICIENTMEMORY;
        wcscpy(pstatstg->pwcsName, L"MemoryIStream");
    }
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.HighPart = 0;
    pstatstg->cbSize.LowPart = length;
    memset(&pstatstg->mtime, 0, sizeof(FILETIME));
    memset(&pstatstg->ctime, 0, sizeof(FILETIME));
    memset(&pstatstg->atime, 0, sizeof(FILETIME));
    pstatstg->grfLocksSupported = 0;
    pstatstg->grfMode = FILE_SHARE_READ;
    pstatstg->clsid = CLSID_NULL;
    pstatstg->grfStateBits = 0;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE MemoryIStream::Clone(IStream **ppstm)
{
    if (!ppstm)
        return STG_E_INVALIDPOINTER;
    *ppstm = new MemoryIStream(buf, length);
    if (!*ppstm)
        return STG_E_INSUFFICIENTMEMORY;
    ((MemoryIStream*)ppstm)->pos = pos;

    return S_OK;
}


//---------------------------------------------


struct MouseRepeatData
{
    bool mousedown;

    MouseButtonEvent handler;

    int timerhandle;
    HHOOK thishook;

    int mouseskip;
    long lasttime;

    Control *sender;
    short x;
    short y;
    MouseButtons button;
    VirtualKeyStateSet vkeys;
    Rect activearea;

    MouseRepeatData() : mousedown(true), timerhandle(0), thishook(NULL), mouseskip(0), lasttime(0), x(0), y(0), button(mbLeft), vkeys(0) {}
    void Update(Control *asender, MouseButtonParameters &aparam)
    {
        sender = asender;
        x = aparam.x;
        y = aparam.y;
        button = aparam.button;
        vkeys = aparam.vkeys;
    }
};
static MouseRepeatData repeatdata;

static LRESULT __stdcall MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && ((wParam == WM_LBUTTONUP && repeatdata.button == mbLeft) || (wParam == WM_RBUTTONUP && repeatdata.button == mbRight)))
    {
        repeatdata.mousedown = false;
        //if (repeatdata.timerhandle != 0)
        //    KillTimer(NULL, repeatdata.timerhandle);
        //repeatdata.timerhandle = 0;
        //repeatdata.handler = nullptr;
    }
    return CallNextHookEx(repeatdata.thishook, nCode, wParam, lParam);
}

static void __stdcall timeprc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (repeatdata.mouseskip > 0)
        --repeatdata.mouseskip;
    else
    {
        if (repeatdata.sender != nullptr)
        {
            Point p;
            GetCursorPos(&p);
            p = repeatdata.sender->ScreenToClient(p);
            repeatdata.x = p.x;
            repeatdata.y = p.y;
        }

        MouseRepeat(repeatdata.handler, repeatdata.sender, MouseButtonParameters(repeatdata.x, repeatdata.y, repeatdata.button, repeatdata.vkeys), &repeatdata.activearea);
    }
}

/**
 * The handler is only called again if the mouse button is still pressed. The function must be called in
 * each call of the handler to generate the next event. The paramters are the event handler, and the
 * handler parameters when it was called.
 * \param handler A handler created with CreateEvent(). This can be the same handler that was set as the OnMouseDown event for a control.
 * \param sender The control which is clicked. Set it to NULL if the sender was not a control, but in that case MouseStopRepeat() must be called if the capture changes or the mouse button is released.
 * \param param The param parameter passed to the handler, which calls the MouseRepeat() function.
 * \param activearea A rectangle in the same units as the param's x and y values. If the x and y coordinates in param are outside this area, the handler won't be called again. This can be set to NULL if checking the area is not necessary. The rectangle is copied, so the original does not need to be reserved.
 * \sa MouseStopRepeat()
 */
void MouseRepeat(MouseButtonEvent handler, Control *sender, MouseButtonParameters param, Rect *activearea)
{
    static bool skip = false;

    repeatdata.Update(sender, param);

    if ((sender && (!sender->Enabled() || !sender->HandleCreated() || GetCapture() != sender->Handle())) || (!repeatdata.vkeys.contains(vksLeft) && !repeatdata.vkeys.contains(vksRight)))
        repeatdata.mousedown = false;

    if (!repeatdata.handler || !repeatdata.mousedown)
    {
        if (!repeatdata.mousedown)
        {
            if (repeatdata.timerhandle != 0)
                KillTimer(NULL, repeatdata.timerhandle);
            repeatdata.timerhandle = 0;
            repeatdata.mousedown = true;
            if (repeatdata.thishook)
                UnhookWindowsHookEx(repeatdata.thishook);
            repeatdata.thishook = NULL;
            repeatdata.handler = nullptr;
            //AppActivateEvent(NULL, d1.prc, false, false);
            return;
        }

        repeatdata.mouseskip = 5;

        //AppActivateEvent(NULL, d1.prc, false, true);

        repeatdata.handler = handler;
        if (activearea)
            repeatdata.activearea = *activearea;
        else if (sender != nullptr)
            repeatdata.activearea = Rect(0, 0, sender->Width(), sender->Height());
        else
            repeatdata.activearea = Rect();
        repeatdata.mousedown = true;
        repeatdata.thishook = SetWindowsHookEx(WH_MOUSE, (LRESULT (__stdcall *)(int, WPARAM, LPARAM))&MouseProc, NULL, MainThreadId);
        repeatdata.lasttime = GetTickCount();
        repeatdata.timerhandle = SetTimer(NULL, 0, 50, (void (__stdcall *)(HWND, UINT, UINT_PTR, DWORD))&timeprc);
        return;
    }

    if (activearea)
        repeatdata.activearea = *activearea;
    else
        repeatdata.activearea = Rect();

    if (sender != nullptr)
    {
        Point p;
        GetCursorPos(&p);
        p = sender->ScreenToClient(p);
        repeatdata.x = p.x;
        repeatdata.y = p.y;
    }
    if (skip || (!repeatdata.activearea.Empty() && !PtInRect(&repeatdata.activearea, Point(repeatdata.x, repeatdata.y))))
    {
        if (!skip && activearea != nullptr)
            repeatdata.activearea = *activearea;
        return;
    }

    skip = true;
    repeatdata.handler(sender, param);
    skip = false;
}

/**
 * \sa MouseRepeat()
 */
void MouseStopRepeat()
{
    if (repeatdata.timerhandle != 0)
        KillTimer(NULL, repeatdata.timerhandle);
    repeatdata.timerhandle = 0;
    repeatdata.mousedown = false;
    if (repeatdata.thishook)
        UnhookWindowsHookEx(repeatdata.thishook);
    repeatdata.thishook = NULL;
    repeatdata.handler = nullptr;
}


//---------------------------------------------



}
/* End of NLIBNS */


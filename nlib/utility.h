#pragma once
#include "objectbase.h"
#include "events.h"


namespace NLIBNS
{


/// Base class for windowless message handlers.
/**
 * Override the protected HandleMessage() method to handle messages sent to the Handle(), or pass
 * messages via PassMessage() even if no handle was created for the object.
 */
class MessageWindowBase
{
private:
    HWND handle;

    void CreateHandle();

    //friend LRESULT CALLBACK MsgFormWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
    /// Handles a single message sent to the object.
    /**
     * Override in derived classes to handle messages sent or passed to the object.
     * Make sure that messages that might be passed without a created handle can be handled
     * correctly.
     * \param uMsg The message identifier.
     * \param wParam The \a WPARAM part of the message.
     * \param lParam the \a LPARAM part of the message.
     * \sa PassMessage();
     */
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
public:
    MessageWindowBase(); ///< Constructor.
    virtual ~MessageWindowBase(); ///< Destructor.

    LRESULT PassMessage(UINT uMsg, WPARAM wParam, LPARAM lParam); ///< Pass a message directly to the object without using the program's message loop.

    bool HandleCreated(); ///< Returns \a true if a handle for the object has been created.
    HWND Handle(); ///< Returns the handle created for the message window.
    void DestroyHandle(); ///< Destroys the handle of the object if it has been created.
};

/// Class for managing system timers.
/**
 * Create a Timer object if the basic system timer is good enough for timing events.
 * Handles WM_TIMER message sent via the program's message loop. The timer is initially not enabled,
 * but it can be turned on by calling SetEnabled(). The interval for calling the [OnTime](@ref Timer::OnTime) event can be
 * changed via SetInterval().
 *
 * \nosubgrouping
 */
class Timer : public NonVisualControl
{
private:
    typedef NonVisualControl base;

    class TimerMessageWindow : public MessageWindowBase
    {
    private:
        sparse_list<Timer*> timers;
        static sparse_list<Timer*>::size_type nextid;
    protected:
        virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    public:
        UINT_PTR AddTimer(Timer *timer);
        void RemoveTimer(UINT_PTR id);
    };

    static TimerMessageWindow handler;

    bool enabled;
    unsigned int interval;

    UINT_PTR id;

    void Time();

    friend class TimerMessageWindow;
protected:
    virtual ~Timer(); ///< \copydoc Object::~Object()
public:
#ifdef DESIGNING
    static void EnumerateProperties(DesignSerializer *serializer);
#endif
    Timer(); ///< Constructor.

    bool Enabled(); ///< Indicates whether the timer is started or not.
    void SetEnabled(bool newenabled); ///< Starts or stops the timer.
    unsigned int Interval(); ///< The approximate millisecond interval for the timer.
    void SetInterval(unsigned int newinterval); ///< Changes the interval of timer events.

    /// \name Events
    ///@{
    NotifyEvent OnTime; ///< %Event called at timer intervals.
    ///@}
};

/// Implements a read-write IStream using std::vector<unsigned char> as the storage for the stream.
/**
 * Pass a vector in the constructor which will be used in read and write operations.
 * The vector is resized as needed. See documentation of the IStream interface on MSDN.
 * \sa MemoryIStream
 */
class VectorIStream : public IStream
{
private:
    typedef IStream base;
    LONG refcnt;

    std::vector<byte> &buf;
    unsigned int pos; // Current read/write pointer position.
public:
    VectorIStream(std::vector<byte> &buf); ///< Creates a read-write IStream.
    virtual ~VectorIStream();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void  **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
    virtual HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
    virtual HRESULT STDMETHODCALLTYPE Revert();
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);
};

/// Implements a read-only IStream using a memory buffer as the storage for the stream.
/**
 * Pass the buffer and its size in the constructor which will be used in read operations.
 * The buffer is used as if it were read-only. See documentation of the IStream interface on MSDN.
 * \sa VectorIStream
 */
class MemoryIStream : public IStream
{
private:
    typedef IStream base;
    LONG refcnt;

    void *buf;
    unsigned int pos;
    unsigned int length;
public:
    MemoryIStream(void *buf, int length); ///< Creates a read-only IStream.
    virtual ~MemoryIStream();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void  **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
    virtual HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
    virtual HRESULT STDMETHODCALLTYPE Revert();
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);
};

/// Call as the first instruction in a MouseDown handler to start a timer that calls the handler again after the system mouse repeat time, simulating repeated mouse presses.
void MouseRepeat(MouseButtonEvent handler, Control *sender, MouseButtonParameters param, Rect *activearea = NULL);

/// Prevent the next simulated mouse press after a MouseRepeat call, before the event is generated.
void MouseStopRepeat();


}
/* End of NLIBNS */


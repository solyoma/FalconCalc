#include "stdafx_zoli.h"
#include <locale>
#include "application.h"
#include "registry.h"
#include "kbd64.h"

#include "comdata.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <float.h>


#ifdef DESIGNING
namespace std
{

    template<typename chartype>
    base_streamformat<chartype> streamstate(const basic_ios<chartype> &stream)
    {
        base_streamformat<chartype> fmt;
        fmt.flags = stream.flags();
        fmt.fillwidth = stream.width();
        fmt.fillchar = stream.fill();
        return fmt;
    }

    streamformat streamstate(const basic_ios<char> &stream)
    {
        return streamstate<char>(stream);
    }
    wstreamformat streamstate(const basic_ios<wchar_t> &stream)
    {
        return streamstate<wchar_t>(stream);
    }

    template<typename chartype>
    void streamstate(basic_ios<chartype> &stream, const base_streamformat<chartype> &format)
    {
        stream.flags(format.flags);
        stream.width(format.fillwidth);
        stream.fill(format.fillchar);
    }

    void streamstate(basic_ios<char> &stream, const streamformat &format)
    {
        streamstate<char>(stream, format);
    }
    void streamstate(basic_ios<wchar_t> &stream, const wstreamformat &format)
    {
        streamstate<wchar_t>(stream, format);
    }

    template<typename chartype>
    bool tryget(basic_istream<chartype> &stream, const chartype *str, int strlen, bool putback)
    {
        chartype c;
        int pos = 0;
        while (pos < strlen)
        {
            stream.get(c);
            if (c != str[pos])
            {
                if (putback)
                {
                    stream.putback(c);
                    std::putback(stream, str, pos);
                }

                return false;
            }
            pos++;
        }

        return true;
    }

    bool tryget(basic_istream<char> &stream, const char *str, bool putback)
    {
        return tryget<char>(stream, str, strlen(str), putback);
    }

    bool tryget(basic_istream<wchar_t> &stream, const wchar_t *str, bool putback)
    {
        return tryget<wchar_t>(stream, str, wcslen(str), putback);
    }

    bool tryget(basic_istream<char> &stream, const char *str, int slen, bool putback)
    {
        return tryget<char>(stream, str, slen, putback);
    }

    bool tryget(basic_istream<wchar_t> &stream, const wchar_t *str, int slen, bool putback)
    {
        return tryget<wchar_t>(stream, str, slen, putback);
    }

    template<typename chartype>
    void putback(basic_istream<chartype> &stream, const chartype *str, int slen)
    {
        int pos = slen;
        while (pos > 0)
        {
            --pos;
            stream.putback(str[pos]);
        }
    }

    void putback(basic_istream<char> &stream, const string &str)
    {
        putback<char>(stream, str.c_str(), str.length());
    }

    void putback(basic_istream<wchar_t> &stream, const wstring &str)
    {
        putback<wchar_t>(stream, str.c_str(), str.length());
    }

    void putback(basic_istream<char> &stream, const char *str, int slen)
    {
        putback<char>(stream, str, slen);
    }

    void putback(basic_istream<wchar_t> &stream, const wchar_t *str, int slen)
    {
        putback<wchar_t>(stream, str, slen);
    }

};


#endif


namespace NLIBNS
{

    Color Color::colors[clArraySize];


    //---------------------------------------------


    bool SetContains(UINT uset, UINT val)
    {
        if (val == 0)
            return uset == 0;
        return ((uset & val) == val);
    }


    //---------------------------------------------


    UINT VirtualKeysFromWParam(WPARAM wParam)
    {
        unsigned int vkeys = 0;
        if ((wParam & MK_CONTROL) == MK_CONTROL)
            vkeys |= vksCtrl;
        if ((wParam & MK_LBUTTON) == MK_LBUTTON)
            vkeys |= vksLeft;
        if ((wParam & MK_RBUTTON) == MK_RBUTTON)
            vkeys |= vksRight;
        if ((wParam & MK_MBUTTON) == MK_MBUTTON)
            vkeys |= vksMiddle;
        if ((wParam & MK_SHIFT) == MK_SHIFT)
            vkeys |= vksShift;
        if ((GetKeyState(VK_MENU) & (1 << 15)) != 0)
            vkeys |= vksAlt;
        return vkeys;
    }

    WPARAM WParamFromVirtualKeys(VirtualKeyStateSet vkeys)
    {
        WPARAM res = 0;
        if (vkeys.contains(vksCtrl))
            res |= MK_CONTROL;
        if (vkeys.contains(vksLeft))
            res |= MK_LBUTTON;
        if (vkeys.contains(vksRight))
            res |= MK_RBUTTON;
        if (vkeys.contains(vksMiddle))
            res |= MK_MBUTTON;
        if (vkeys.contains(vksShift))
            res |= MK_SHIFT;
        return res;
    }

    VirtualKeyStateSet PressedVirtualKeys()
    {
        VirtualKeyStateSet vkeys;
        if ((GetKeyState(VK_LBUTTON) & (1 << 15)) != 0)
            vkeys |= vksLeft;
        if ((GetKeyState(VK_RBUTTON) & (1 << 15)) != 0)
            vkeys |= vksRight;
        if ((GetKeyState(VK_MBUTTON) & (1 << 15)) != 0)
            vkeys |= vksMiddle;
        if ((GetKeyState(VK_CONTROL) & (1 << 15)) != 0)
            vkeys |= vksCtrl;
        if ((GetKeyState(VK_SHIFT) & (1 << 15)) != 0)
            vkeys |= vksShift;
        if ((GetKeyState(VK_MENU) & (1 << 15)) != 0)
            vkeys |= vksAlt;
        return vkeys;
    }

    MouseButtons ButtonFromMsg(UINT uMsg)
    {
        return uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_LBUTTONDBLCLK ? mbLeft : uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_RBUTTONDBLCLK ? mbRight : uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP || uMsg == WM_MBUTTONDBLCLK ? mbMiddle : mbError;
    }

    MouseButtons NCButtonFromMsg(UINT uMsg)
    {
        return uMsg == WM_NCLBUTTONDOWN || uMsg == WM_NCLBUTTONUP || uMsg == WM_NCLBUTTONDBLCLK ? mbLeft : uMsg == WM_NCRBUTTONDOWN || uMsg == WM_NCRBUTTONUP || uMsg == WM_NCRBUTTONDBLCLK ? mbRight : uMsg == WM_NCMBUTTONDOWN || uMsg == WM_NCMBUTTONUP || uMsg == WM_NCMBUTTONDBLCLK ? mbMiddle : mbError;
    }

    UINT GenerateCommandId()
    {
        static UINT id = 0;
        return ++id;
    }

    //---------------------------------------------


    Rect::Rect(LONG left, LONG top, LONG right, LONG bottom)
    {
        base::left = left;
        base::top = top;
        base::right = right;
        base::bottom = bottom;
    }

    Rect RectS(LONG left, LONG top, LONG width, LONG height)
    {
        Rect r(left, top, left + width, top + height);
        return r;
    }

    Rect RectS(Point topleft, LONG width, LONG height)
    {
        Rect r(topleft.x, topleft.y, topleft.x + width, topleft.y + height);
        return r;
    }

    Rect::Rect(const RECT &orig)
    {
        *this = orig;
    }

    Rect::Rect(const Point &topleft, const Point &bottomright)
    {
        base::left = topleft.x;
        base::top = topleft.y;
        base::right = bottomright.x;
        base::bottom = bottomright.y;
    }

    Rect& Rect::operator=(const RECT &r)
    {
        left = r.left;
        top = r.top;
        bottom = r.bottom;
        right = r.right;
        return *this;
    }

    Rect::Rect(const Gdiplus::Rect &orig)
    {
        *this = orig;
    }

    Rect& Rect::operator=(const Gdiplus::Rect &r)
    {
        left = r.GetLeft();
        top = r.GetTop();
        bottom = r.GetBottom();
        right = r.GetRight();
        return *this;
    }

    Rect::Rect(const RectF &orig)
    {
        *this = orig;
    }

    Rect& Rect::operator=(const RectF &r)
    {
        left = r.left;
        top = r.top;
        bottom = r.bottom;
        right = r.right;
        return *this;
    }

    Rect::Rect(const Gdiplus::RectF &orig)
    {
        *this = orig;
    }

    Rect& Rect::operator=(const Gdiplus::RectF &r)
    {
        left = r.GetLeft();
        top = r.GetTop();
        bottom = r.GetBottom();
        right = r.GetRight();
        return *this;
    }

    LONG Rect::Width() const
    {
        if ((UINT)left == CW_USEDEFAULT && (UINT)right == CW_USEDEFAULT)
            return (LONG)CW_USEDEFAULT;
        return right - left;
    }
    LONG Rect::Height() const
    {
        if ((UINT)top == CW_USEDEFAULT && (UINT)bottom == CW_USEDEFAULT)
            return (LONG)CW_USEDEFAULT;
        return bottom - top;
    }

    bool Rect::Empty() const
    {
        return top >= bottom || left >= right;
    }

    Rect::operator Gdiplus::Rect() const
    {
        return Gdiplus::Rect(left, top, Width(), Height());
    }

    Rect Rect::Offset(const Point &pt) const
    {
        return Rect(left + pt.x, top + pt.y, right + pt.x, bottom + pt.y);
    }

    Rect Rect::Offset(int x, int y) const
    {
        return Rect(left + x, top + y, right + x, bottom + y);
    }

    Rect Rect::Offset(int dleft, int dtop, int dright, int dbottom) const
    {
        return Rect(left + dleft, top + dtop, right + dright, bottom + dbottom);
    }

    Rect& Rect::Move(const Point &pt)
    {
        left += pt.x;
        top += pt.y;
        right += pt.x;
        bottom += pt.y;
        return *this;
    }

    Rect& Rect::Move(int x, int y)
    {
        left += x;
        top += y;
        right += x;
        bottom += y;
        return *this;
    }

    Rect& Rect::Move(int dleft, int dtop, int dright, int dbottom)
    {
        left += dleft;
        top += dtop;
        right += dright;
        bottom += dbottom;
        return *this;
    }

    Rect Rect::Intersect(const Rect &other) const
    {
        return Rect(max(left, other.left), max(top, other.top), min(right, other.right), min(bottom, other.bottom));
    }

    Rect& Rect::Boolean(const Rect &other)
    {
        left = max(left, other.left);
        top = max(top, other.top);
        right = min(right, other.right);
        bottom = min(bottom, other.bottom);

        return *this;
    }

    bool Rect::DoesIntersect(const Rect &other) const
    {
        return max(left, other.left) < min(right, other.right) && max(top, other.top) < min(bottom, other.bottom);
    }

    Rect Rect::Inflate(int val) const
    {
        return Rect(left - val, top - val, right + val, bottom + val);
    }

    Rect Rect::Inflate(int xval, int yval) const
    {
        return Rect(left - xval, top - yval, right + xval, bottom + yval);
    }

    Rect Rect::Inflate(int dleft, int dtop, int dright, int dbottom) const
    {
        return Rect(left - dleft, top - dtop, right + dright, bottom + dbottom);
    }

    Rect& Rect::Expand(int val)
    {
        InflateRect(this, val, val);
        return *this;
    }

    Rect& Rect::Expand(int xval, int yval)
    {
        InflateRect(this, xval, yval);
        return *this;
    }

    Rect& Rect::Expand(int dleft, int dtop, int dright, int dbottom)
    {
        left -= dleft;
        top -= dtop;
        right += dright;
        bottom += dbottom;
        return *this;
    }

    Point Rect::TopLeft() const
    {
        return Point(left, top);
    }

    Point Rect::BottomRight() const
    {
        return Point(right, bottom);
    }

    bool Rect::Contains(int x, int y) const
    {
        return PtInRect(this, Point(x, y)) != FALSE;
    }

    bool Rect::Contains(Point pt) const
    {
        return PtInRect(this, pt) != FALSE;
    }


    //---------------------------------------------


    RectF::RectF(float left, float top, float right, float bottom) : left(left), top(top), right(right), bottom(bottom)
    {
    }

    RectF RectFS(float left, float top, float width, float height)
    {
        RectF r(left, top, left + width, top + height);
        return r;
    }

    RectF RectFS(PointF topleft, float width, float height)
    {
        RectF r(topleft.x, topleft.y, topleft.x + width, topleft.y + height);
        return r;
    }



    RectF::RectF(const Rect &orig)
    {
        *this = orig;
    }

    RectF::RectF(const RECT &orig)
    {
        *this = orig;
    }

    RectF& RectF::operator=(const Rect &r)
    {
        left = r.left;
        top = r.top;
        bottom = r.bottom;
        right = r.right;
        return *this;
    }

    RectF& RectF::operator=(const RECT &r)
    {
        left = r.left;
        top = r.top;
        bottom = r.bottom;
        right = r.right;
        return *this;
    }

    RectF::RectF(const Gdiplus::RectF &orig)
    {
        *this = orig;
    }

    RectF& RectF::operator=(const Gdiplus::RectF &r)
    {
        left = r.GetLeft();
        top = r.GetTop();
        bottom = r.GetBottom();
        right = r.GetRight();
        return *this;
    }

    float RectF::Width() const
    {
        if (left == CW_USEDEFAULT && right == CW_USEDEFAULT)
            return CW_USEDEFAULT;
        return right-left;
    }
    float RectF::Height() const
    {
        if (top == CW_USEDEFAULT && bottom == CW_USEDEFAULT)
            return CW_USEDEFAULT;
        return bottom-top;
    }

    bool RectF::Empty() const
    {
        return top >= bottom && left >= right;
    }

    RectF::operator Gdiplus::RectF() const
    {
        return Gdiplus::RectF(left, top, Width(), Height());
    }

    RectF::operator Rect() const
    {
        return Rect(left, top, right, bottom);
    }

    RectF RectF::Offset(float x, float y) const
    {
        return RectF(left + x, top + y, right + x, bottom + y);
    }

    RectF RectF::Offset(const PointF &pt) const
    {
        return RectF(left + pt.x, top + pt.y, right + pt.x, bottom + pt.y);
    }

    RectF RectF::Offset(float dleft, float dtop, float dright, float dbottom) const
    {
        return RectF(left + dleft, top + dtop, right + dright, bottom + dbottom);
    }

    RectF& RectF::Move(float x, float y)
    {
        left += x;
        top += y;
        right += x;
        bottom += y;
        return *this;
    }

    RectF& RectF::Move(const PointF &pt)
    {
        left += pt.x;
        top += pt.y;
        right += pt.x;
        bottom += pt.y;
        return *this;
    }

    RectF RectF::Intersect(const RectF &other) const
    {
        return RectF(max(left, other.left), max(top, other.top), min(right, other.right), min(bottom, other.bottom));
    }

    bool RectF::DoesIntersect(const RectF &other) const
    {
        return max(left, other.left) < min(right, other.right) && max(top, other.top) < min(bottom, other.bottom);
    }

    RectF RectF::Inflate(float val) const
    {
        return RectF(left - val, top - val, right + val, bottom + val);
    }

    RectF RectF::Inflate(float xval, float yval) const
    {
        return RectF(left - xval, top - yval, right + xval, bottom + yval);
    }

    RectF& RectF::Expand(float val)
    {
        left -= val;
        top -= val;
        right += val;
        bottom += val;
        return *this;
    }

    RectF& RectF::Expand(float xval, float yval)
    {
        left -= xval;
        top -= yval;
        right += xval;
        bottom += yval;
        return *this;
    }

    PointF RectF::TopLeft() const
    {
        return PointF(left, top);
    }

    PointF RectF::BottomRight() const
    {
        return PointF(right, bottom);
    }

    bool RectF::Contains(float x, float y) const
    {
        return x >= left && x <= right && y >= top && y <= bottom;
    }

    bool RectF::Contains(PointF pt) const
    {
        return pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom;
    }


    //---------------------------------------------


    bool operator==(const Rect &a, const Rect &b)
    {
        return memcmp(&a,&b,sizeof(Rect)) == 0;
    }

    bool operator!=(const Rect &a, const Rect &b)
    {
        return memcmp(&a,&b,sizeof(Rect)) != 0;
    }

    bool operator==(const RectF &a, const RectF &b)
    {
        return memcmp(&a,&b,sizeof(RectF)) == 0;
    }

    bool operator!=(const RectF &a, const RectF &b)
    {
        return memcmp(&a,&b,sizeof(RectF)) != 0;
    }

    bool operator==(const RECT &a, const RECT &b)
    {
        return memcmp(&a,&b,sizeof(RECT)) == 0;
    }

    bool operator!=(const RECT &a, const RECT &b)
    {
        return memcmp(&a,&b,sizeof(RECT)) != 0;
    }

    bool operator==(const Size &a, const Size &b)
    {
        return a.cx == b.cx && a.cy == b.cy;
    }

    bool operator!=(const Size &a, const Size &b)
    {
        return a.cx != b.cx || a.cy != b.cy;
    }

    bool operator==(const SIZE &a, const SIZE &b)
    {
        return a.cx == b.cx && a.cy == b.cy;
    }

    bool operator!=(const SIZE &a, const SIZE &b)
    {
        return a.cx != b.cx || a.cy != b.cy;
    }

    bool operator==(const Point &a, const Point &b)
    {
        return memcmp(&a,&b,sizeof(Point)) == 0;
    }

    bool operator!=(const Point &a, const Point &b)
    {
        return memcmp(&a,&b,sizeof(Point)) != 0;
    }

    bool operator==(const PointF &a, const PointF &b)
    {
        return memcmp(&a,&b,sizeof(PointF)) == 0;
    }

    bool operator!=(const PointF &a, const PointF &b)
    {
        return memcmp(&a,&b,sizeof(PointF)) != 0;
    }

    bool operator==(const POINT &a, const POINT &b)
    {
        return memcmp(&a,&b,sizeof(POINT)) == 0;
    }

    bool operator!=(const POINT &a, const POINT &b)
    {
        return memcmp(&a,&b,sizeof(POINT)) != 0;
    }


    //---------------------------------------------


    Size::Size(LONG cx, LONG cy)
    {
        base::cx = cx;
        base::cy = cy;
    }

    Size::Size(const SIZE &orig)
    {
        *this = orig;
    }

    Size& Size::operator=(const SIZE &s)
    {
        base::cx = s.cx;
        base::cy = s.cy;
        return *this;
    }

    //---------------------------------------------


    Point::Point(LONG x, LONG y)
    {
        base::x = x;
        base::y = y;
    }

    Point::Point(const POINT &p) : base(p)
    {
    }

    Point::Point(const POINTS &p)
    {
        x = p.x;
        y = p.y;
    }

    Point::operator Gdiplus::Point()
    {
        return Gdiplus::Point(x, y);
    }

    Point& Point::Move(int deltax, int deltay)
    {
        x += deltax;
        y += deltay;

        return *this;
    }

    Point& Point::Move(const Point &pt)
    {
        x += pt.x;
        y += pt.y;

        return *this;
    }

    Point Point::Offset(int deltax, int deltay) const
    {
        return Point(x + deltax, y + deltay);
    }

    Point Point::Offset(const Point &pt) const
    {
        return Point(x + pt.x, y + pt.y);
    }

    Point Point::operator-(const Point &other) const
    {
        return Point(x - other.x, y - other.y);
    }

    Point Point::operator+(const Point &other) const
    {
        return Point(x + other.x, y + other.y);
    }

    Point& Point::operator-=(const Point &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Point& Point::operator+=(const Point &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    void Point::Get(int &ax, int &ay) const
    {
        ax = x;
        ay = y;
    }


    //---------------------------------------------


    PointF::PointF() : x(0), y(0) {}

    PointF::PointF(float x, float y) : x(x), y(y) {}

    PointF::PointF(const Point &p) : x(p.x), y(p.y) {}
    PointF::PointF(const POINT &p) : x(p.x), y(p.y) {}
    PointF::PointF(const Gdiplus::PointF &p) : x(p.X), y(p.Y) {}
    PointF& PointF::operator=(const PointF &p)
    {
        x = p.x;
        y = p.y;
        return *this;
    }

    PointF& PointF::operator=(const Point &p)
    {
        x = p.x;
        y = p.y;
        return *this;
    }

    PointF& PointF::operator=(const POINT &p)
    {
        x = p.x;
        y = p.y;
        return *this;
    }

    PointF& PointF::operator=(const Gdiplus::PointF &p)
    {
        x = p.X;
        y = p.Y;
        return *this;
    }

    PointF::operator Gdiplus::PointF()
    {
        return Gdiplus::PointF(x, y);
    }

    PointF& PointF::Move(float deltax, float deltay)
    {
        x += deltax;
        y += deltay;

        return *this;
    }

    PointF& PointF::Move(const PointF &pt)
    {
        x += pt.x;
        y += pt.y;

        return *this;
    }

    PointF PointF::Offset(float deltax, float deltay) const
    {
        return PointF(x + deltax, y + deltay);
    }

    PointF PointF::Offset(const PointF &pt) const
    {
        return PointF(x + pt.x, y + pt.y);
    }


    PointF PointF::operator-(const PointF &other) const
    {
        return PointF(x - other.x, y - other.y);
    }

    PointF PointF::operator+(const PointF &other) const
    {
        return PointF(x + other.x, y + other.y);
    }

    PointF& PointF::operator-=(const PointF &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    PointF& PointF::operator+=(const PointF &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    void PointF::Get(float &ax, float &ay) const
    {
        ax = x;
        ay = y;
    }


    //---------------------------------------------


    Size BaseToPixel(float basewidth, float baseheight)
    {
        double dw = basewidth * Scaling * 1.498;
        double dh = baseheight * Scaling * 1.643;
        return Size(int(dw + 0.5), int(dh + 0.5));
    }


    //---------------------------------------------

    Color::Color() : indexed(true)
    {
        data.colval = clBlack;
    };

    Color::Color(const COLORREF &color) : indexed(false)
    {
        *this = color;
    }

    Color::Color(const RGBQUAD &color) : indexed(false)
    {
        *this = color;
    }

    Color::Color(Gdiplus::Color color) : indexed(false)
    {
        *this = color;
    }

    Color::Color(const Color &other) : indexed(false)
    {
        *this = other;
    }

    Color::Color(byte A, byte R, byte G, byte B) : indexed(false)
    {
        data.color = (A << 24) | (R << 16) | (G << 8) | B;
    }

    Color::Color(byte R, byte G, byte B) : indexed(false)
    {
        data.color = 0xff000000 | (R << 16) | (G << 8) | B;
    }

    Color::Color(Colors col) : indexed(true)
    {
        data.colval = col;
    }

    Color::operator COLORREF() const
    {
        if (!indexed)
            return RGB(data.R, data.G, data.B);
        return data.colval == clNone ? Color(0, 0, 0) : colors[data.colval];
    }

    Color::operator Gdiplus::Color() const
    {
        if (!indexed)
            return Gdiplus::Color(data.color);
        return data.colval == clNone ? Color(0, 0, 0, 0) : colors[data.colval];
    }

    Color::operator RGBQUAD() const
    {
        RGBQUAD val;
        if (!indexed)
        {
            memcpy(&val, &data, sizeof(RGBQUAD));
            val.rgbReserved = 0;
        }
        else if (data.colval == clNone)
            memset(&val, 0, sizeof(RGBQUAD));
        else
            return colors[data.colval];
        return val;
    }

    DWORD Color::ToDWORD() const
    {
        if (!indexed)
            return data.color;
        if (data.colval == clNone)
            return 0;
        return colors[data.colval].data.color;
    }

    bool Color::operator==(const Color &other) const
    {
        return indexed == other.indexed && (indexed ? data.colval == other.data.colval : data.color == other.data.color);
    }

    bool Color::operator==(const COLORREF &other) const
    {
        return (!indexed && data.A == 0xff && (other == RGB(data.R, data.G, data.B))) || (indexed && ToRGB() == other);
    }

    bool Color::operator==(const RGBQUAD &other) const
    {
        return (!indexed && data.A == 0xff && (memcmp(&other, &data, 3) == 0)) || (indexed && ToRGB() == other);
    }

    bool Color::operator==(const Gdiplus::Color &other) const
    {
        return !indexed && data.color == other.GetValue();
    }

    bool Color::operator==(const Colors &col) const
    {
        return indexed && data.colval == col;
    }

    bool Color::operator!=(const Color &other) const
    {
        return indexed != other.indexed || (indexed ? data.colval != other.data.colval : data.color != other.data.color);
    }

    bool Color::operator!=(const COLORREF &other) const
    {
        return (indexed || data.A != 0xff || (other != RGB(data.R, data.G, data.B))) && (!indexed || ToRGB() != other);
    }

    bool Color::operator!=(const RGBQUAD &other) const
    {
        return (indexed || data.A != 0xff || (memcmp(&other, &data, 3) != 0)) && (!indexed || ToRGB() != other);
    }

    bool Color::operator!=(const Gdiplus::Color &other) const
    {
        return indexed || data.color != other.GetValue();
    }

    bool Color::operator!=(const Colors &col) const
    {
        return !indexed || data.colval != col;
    }

    Color& Color::operator=(const Color &other)
    {
        data.color = other.data.color;
        indexed = other.indexed;
        return *this;
    }

    Color& Color::operator=(const COLORREF &other)
    {
        data.color = 0xff000000 | ((other & 0xff) << 16) | (other & 0x00ff00) | ((other & 0xff0000) >> 16);
        indexed = false;
        return *this;
    }

    Color& Color::operator=(const RGBQUAD &other)
    {
        memcpy(&data, &other, 3);
        data.A = 255;
        indexed = false;
        return *this;
    }

    Color& Color::operator=(const Gdiplus::Color &other)
    {
        data.color = other.GetValue();
        indexed = false;
        return *this;
    }

    Color& Color::operator=(const Colors &col)
    {
        data.colval = col;
        indexed = true;
        return *this;
    }

    HSVColor::HSVColor() : a(255), h(0), s(1.f), v(1.f)
    {
        ;
    }

    HSVColor::HSVColor(float h, float s, float v) : a(255), h(h), s(s), v(v)
    {
        h -= int(h);
        if (h < 0)
            h += 1;
        s = clamp(s, 0.f, 1.f);
        v = clamp(v, 0.f, 1.f);
    }

    HSVColor::HSVColor(byte a, float h, float s, float v) : a(a), h(h), s(s), v(v)
    {
        a = clamp(a, 0, 255);
        h -= int(h);
        if (h < 0)
            h += 1;
        s = clamp(s, 0.f, 1.f);
        v = clamp(v, 0.f, 1.f);
    }

    HSVColor::HSVColor(const HSVColor &other) : a(other.a), h(other.h), s(other.s), v(other.v)
    {
        ;
    }

    HSVColor::HSVColor(const Color &other)
    {
        *this = other;
    }

    HSVColor::HSVColor(Gdiplus::Color color)
    {
        *this = color;
    }

    HSVColor::HSVColor(Colors col)
    {
        *this = col;
    }

    HSVColor::operator Color() const
    {
        float sval = (1.f - s) * v;

        float hval =  h * 6.f;
        int range60 = int(hval);
        hval -= range60;

        byte r;
        byte g;
        byte b;

        switch (range60)
        {
        case 1:
            g = byte(v * 255.f);
            b = byte(sval * 255.f);
            r = byte((v - hval * v * s) * 255);
            break;
        case 2:
            g = byte(v * 255.f);
            r = byte(sval * 255.f);
            b = byte((v - v * s + hval * v * s) * 255.f);
            break;
        case 3:
            b = byte(v * 255.f);
            r = byte(sval * 255.f);
            g = byte((v - hval * v * s) * 255);
            break;
        case 4:
            b = byte(v * 255.f);
            g = byte(sval * 255.f);
            r = byte((v - v * s + hval * v * s) * 255.f);
            break;
        case 5:
            r = byte(v * 255.f);
            g = byte(sval * 255.f);
            b = byte((v - hval * v * s) * 255); //byte(v - hval * (v - sval));
            break;
        default:
            r = byte(v * 255.f);
            b = byte(sval * 255.f);
            g = byte((v - v * s + hval * v * s) * 255.f); //byte(sval + hval * (v - sval));
            break;
        }

        return Color(a, r, g, b);
    }

    HSVColor& HSVColor::operator=(const Color &other)
    {
        a = other.A();
        float r = float(other.R()) / 255.f + 1 / 1024.f;
        float g = float(other.G()) / 255.f + 1 / 1024.f;
        float b = float(other.B()) / 255.f + 1 / 1024.f;

        if (r > g)
        {
            if (g > b) /* 0: r > g > b */
            {
                v = r;
                s = 1.f - b / r;
                h = ((g - b) / (r - b)) / 6.f;
            }
            else if (r > b) /* 5: r > b >= g */
            {
                v = r;
                s = 1.f - g / r;
                h = (6.f - (b - g) / (r - g)) / 6.f;
            }
            else /* 4: b >= r > g */
            {
                v = b;
                s = 1.f - g / b;
                h = (4.f + (r - g) / (b - g)) / 6.f;
            }
        }
        else /* g >= r */
        {
            if (r > b) /* 1: g >= r > b */
            {
                v = g;
                s = 1.f - b / g;
                h = (2.f - (r - b) / (g - b)) / 6.f;
            }
            else if (g > b) /* 2: g > b >= r */
            {
                v = g;
                s = 1.f - r / g;
                h = (2.f + (b - r) / (g - r)) / 6.f;
            }
            else /* 3: b >= g >= r */
            {
                v = b;
                if (b == r)
                {
                    s = 0;
                    h = 0;
                }
                else
                {
                    s = 1 - r / b;
                    h = (4.f - (g - r) / (b - r)) / 6.f;
                }
            }
        }
        return *this;
    }

    HSVColor& HSVColor::operator=(const HSVColor &other)
    {
        a = other.a;
        h = other.h;
        s = other.s;
        v = other.v;

        return *this;
    }

    bool HSVColor::operator==(const Gdiplus::Color &other) const
    {
        return *this == HSVColor(Color(other));
    }

    bool HSVColor::operator==(const Colors &col) const
    {
        if (col == clNone)
            return false;
        return *this == HSVColor(Color::colors[col]);
    }

    bool HSVColor::operator!=(const Gdiplus::Color &other) const
    {
        return *this != HSVColor(Color(other));
    }

    bool HSVColor::operator!=(const Colors &col) const
    {
        if (col == clNone)
            return true;
        return *this != HSVColor(Color::colors[col]);
    }

    HSVColor& HSVColor::operator=(const Gdiplus::Color &other)
    {
        return *this = Color(other);
    }

    HSVColor& HSVColor::operator=(const Colors &col)
    {
        if (col == clNone)
            return *this;
        return *this = Color::colors[col];
    }

    bool HSVColor::operator==(const HSVColor &other) const
    {
        return a == other.a && fabs(h - other.h) < 1e-20f && fabs(s - other.s) < 1e-20f && fabs(v - other.v) < 1e-20f;
    }

    bool HSVColor::operator!=(const HSVColor &other) const
    {
        return !(*this == other);
    }

    byte HSVColor::A() const
    {
        return a;
    }

    float HSVColor::H() const
    {
        return h;
    }

    float HSVColor::S() const
    {
        return s;
    }

    float HSVColor::V() const
    {
        return v;
    }

    void HSVColor::SetA(byte newa)
    {
        a = clamp(newa, 0, 255);
    }

    void HSVColor::SetH(float newh)
    {
        h = newh - int(newh);
        if (h < 0)
            h += 1;
    }

    void HSVColor::SetS(float news)
    {
        s = clamp(news, 0.f, 1.f);
    }

    void HSVColor::SetV(float newv)
    {
        v = clamp(newv, 0, 1.f);
    }

    HSVColor HSVColor::Mix(const HSVColor &other, double ratio) const
    {
        HSVColor tmp;
        tmp.a = byte(a * ratio + other.a * (1 - ratio));
        tmp.h = WORD(h * ratio + other.h * (1 - ratio));
        tmp.s = byte(s * ratio + other.s * (1 - ratio));
        tmp.v = byte(v * ratio + other.v * (1 - ratio));

        return tmp;
    }


#ifdef DESIGNING
    std::ostream& operator<<(std::ostream& ostr, const Color &c)
    {
        //auto ss = std::streamstate(ostr);

        if (c.EnumIndexed())
            ostr << "Color(" << (int)c.EnumValue() << ")";
        else
            ostr << "Color(" << c.A() << "," << c.R() << "," << c.G() << "," << c.B() << ")";

        //std::streamstate(ostr, ss);

        return ostr;
    }

    std::wostream& operator<<(std::wostream& ostr, const Color &c)
    {
        //auto ss = std::streamstate(ostr);

        if (c.EnumIndexed())
            ostr << L"Color(" << (int)c.EnumValue() << L")";
        else
            ostr << L"Color(" << c.A() << L"," << c.R() << L"," << c.G() << "," << c.B() << L")";

        //std::streamstate(ostr, ss);

        return ostr;
    }

    std::istream& operator>>(std::istream& istr, Color &c)
    {
        if (!std::tryget(istr, "Color(", 6, true))
        {
            istr.setstate(std::ios_base::badbit);
            return istr;
        }

        std::stringstream str;
        str << "Color(";
        int numcnt = 0;
        int num[4] = { 0, 0, 0, 0 };
        char ch = 0;
        bool lastcomma = true;
        bool lastneg = false;
        while (ch != ')')
        {
            istr.get(ch);
            str << ch;

            if (ch != ')')
            {

                if (ch == '-' && (!lastcomma || numcnt > 0)) // Only the first number can be negative, in which case it means the clNone color value.
                {
                    std::putback(istr, str.str());
                    istr.setstate(std::ios_base::badbit);
                    return istr;
                }

                if ((ch >= '0' && ch <= '9') || ch == '-')
                {
                    if (lastcomma)
                    {
                        if (lastneg)
                        {
                            num[numcnt-1] *= -1;
                            lastneg = false;
                        }

                        numcnt++;
                        lastcomma = false;
                    }
                    if (ch == '-')
                        lastneg = true;
                    else if (numcnt <= 4)
                    {
                        num[numcnt-1] *= 10;
                        num[numcnt-1] += ch - '0';
                    }
                }

                if (((ch < '0' || ch > '9') && (ch != ',' || lastcomma) && ch != '-') || num[numcnt-1] > 255 || numcnt > 4)
                {
                    std::putback(istr, str.str());
                    istr.setstate(std::ios_base::badbit);
                    return istr;
                }

                if (ch == ',')
                    lastcomma = true;
            }
        }
        if (lastneg && numcnt)
            num[numcnt-1] *= -1;

        if (numcnt != 1 && numcnt != 3 && numcnt != 4)
        {
            std::putback(istr, str.str());
            istr.setstate(std::ios_base::badbit);
            return istr;
        }

        if (numcnt == 1)
            c = (Colors)num[0];
        else if (numcnt == 3)
            c = Color(num[0], num[1], num[2]);
        else
            c = Color(num[0], num[1], num[2], num[3]);

        return istr;
    }

    std::wistream& operator>>(std::wistream& istr, Color &c)
    {
        if (!std::tryget(istr, L"Color(", 6, true))
        {
            istr.setstate(std::ios_base::badbit);
            return istr;
        }

        std::wstringstream str;
        str << L"Color(";
        int numcnt = 0;
        int num[4] = { 0, 0, 0 };
        wchar_t ch = 0;
        bool lastcomma = true;
        bool lastneg = false;
        while (ch != L')')
        {
            istr.get(ch);
            str << ch;

            if (ch != L')')
            {

                if (ch == '-' && (!lastcomma || numcnt > 0)) // Only the first number can be negative, in which case it means the clNone color value.
                {
                    std::putback(istr, str.str());
                    istr.setstate(std::ios_base::badbit);
                    return istr;
                }

                if ((ch >= L'0' && ch <= L'9') || ch == L'-')
                {
                    if (lastcomma)
                    {
                        if (lastneg)
                        {
                            num[numcnt-1] *= -1;
                            lastneg = false;
                        }

                        numcnt++;
                        lastcomma = false;
                    }
                    if (ch == L'-')
                        lastneg = true;
                    else if (numcnt <= 4)
                    {
                        num[numcnt-1] *= 10;
                        num[numcnt-1] += ch - L'0';
                    }
                }

                if (((ch < L'0' || ch > L'9') && (ch != L',' || lastcomma) && ch != L'-') || num[numcnt-1] > 255 || numcnt > 4)
                {
                    std::putback(istr, str.str());
                    istr.setstate(std::ios_base::badbit);
                    return istr;
                }

                if (ch == L',')
                    lastcomma = true;
            }
        }
        if (lastneg && numcnt)
            num[numcnt-1] *= -1;

        if (numcnt != 1 && numcnt != 3 && numcnt != 4)
        {
            std::putback(istr, str.str());
            istr.setstate(std::ios_base::badbit);
            return istr;
        }

        if (numcnt == 1)
            c = (Colors)num[0];
        else if (numcnt == 3)
            c = Color(num[0], num[1], num[2]);
        else
            c = Color(num[0], num[1], num[2], num[3]);

        return istr;
    }
#endif

    bool Color::EnumIndexed() const
    {
        return indexed;
    }

    Colors Color::EnumValue() const
    {
        if (!indexed)
            throw L"Color is not indexed! Cannot return an index!";

        return data.colval;
    }

    byte Color::R() const
    {
        if (indexed)
        {
            if (data.colval == clNone)
                return 0;
            return colors[data.colval].R();
        }
        return data.R;
    }

    byte Color::G() const
    {
        if (indexed)
        {
            if (data.colval == clNone)
                return 0;
            return colors[data.colval].G();
        }
        return data.G;
    }

    byte Color::B() const
    {
        if (indexed)
        {
            if (data.colval == clNone)
                return 0;
            return colors[data.colval].B();
        }
        return data.B;
    }

    byte Color::A() const
    {
        if (indexed)
        {
            if (data.colval == clNone)
                return 0;
            return colors[data.colval].A();
        }
        return data.A;
    }

    void Color::ConvertToRGB()
    {
        *this = ToRGB();
    }

    Color& Color::SetR(byte newR)
    {
        ConvertToRGB();
        data.R = newR;
        return *this;
    }

    Color& Color::SetG(byte newG)
    {
        ConvertToRGB();
        data.G = newG;
        return *this;
    }

    Color& Color::SetB(byte newB)
    {
        ConvertToRGB();
        data.B = newB;
        return *this;
    }

    Color& Color::SetA(byte newA)
    {
        ConvertToRGB();
        data.A = newA;
        return *this;
    }

    Color Color::ToRGB() const
    {
        if (!indexed)
            return *this;

        return data.colval == clNone ? Color(0, 0, 0, 0) : colors[data.colval];
    }

    Color Color::Solid() const
    {
        if (indexed)
            return *this;

        return Color(data.R, data.G, data.B);
    }

    bool operator==(const COLORREF &other, const Color &color)
    {
        return color == other;
    }

    bool operator==(const Gdiplus::Color &other, const Color &color)
    {
        return color == other;
    }

    bool operator==(const Colors &col, const Color &color)
    {
        return color == col;
    }

    bool operator!=(const COLORREF &other, const Color &color)
    {
        return color != other;
    }

    bool operator!=(const Gdiplus::Color &other, const Color &color)
    {
        return color != other;
    }

    bool operator!=(const Colors &col, const Color &color)
    {
        return color != col;
    }

    Color MixColors(Color a, Color b, double ratio)
    {
        if (a.EnumIndexed())
            a = a.ToRGB();
        if (b.EnumIndexed())
            b = b.ToRGB();
        ratio = min(1, max(0, ratio));

        return Color(a.A() * ratio + b.A() * (1 - ratio), a.R() * ratio + b.R() * (1 - ratio), a.G() * ratio + b.G() * (1 - ratio), a.B() * ratio + b.B() * (1 - ratio));
    }

    Color Color::Mix(Color c, double ratio) const
    {
        return MixColors(*this, c, ratio);
    }


    //---------------------------------------------


    const Gdiplus::ColorMatrix identitymatrix = { {
            { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f } ,
            { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f } ,
            { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f } ,
            { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f } ,
            { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } } };
    const Gdiplus::ColorMatrix grayscalematrix = { {
            { 0.299f, 0.299f, 0.299f,    0.0f,    0.0f } ,
            { 0.587f, 0.587f, 0.587f,    0.0f,    0.0f } ,
            { 0.114f, 0.114f, 0.114f,    0.0f,    0.0f } ,
            {   0.0f,   0.0f,   0.0f,    1.0f,    0.0f } ,
            {   0.0f,   0.0f,   0.0f,    0.0f,    1.0f } } };

    ColorMatrix::ColorMatrix()
    {
        *this = identitymatrix;
    }

    ColorMatrix::ColorMatrix(const Gdiplus::ColorMatrix &other)
    {
        memcpy(m, other.m, sizeof(Gdiplus::REAL) * 5 * 5);
    }

    ColorMatrix::ColorMatrix(Gdiplus::REAL data[5][5])
    {
        memcpy(m, data, sizeof(Gdiplus::REAL) * 5 * 5);
    }

    ColorMatrix& ColorMatrix::operator=(const Gdiplus::ColorMatrix &other)
    {
        memcpy(m, other.m, sizeof(Gdiplus::REAL) * 5 * 5);
        return *this;
    }

    ColorMatrix ColorMatrix::operator*(const Gdiplus::ColorMatrix &other) const
    {
        Gdiplus::REAL m2[5][5];
        memset(&m2, 0, sizeof(Gdiplus::REAL) * 5 * 5);
        for (int a = 0; a < 5; ++a)
            for (int b = 0; b < 5; ++b)
                for (int i = 0, j = 0; i < 5; ++i, ++j)
                    m2[a][b] += m[a][i] * other.m[j][b];

        return m2;
    }

    ColorMatrix ColorMatrix::operator+(const Gdiplus::ColorMatrix &other) const
    {
        Gdiplus::REAL m2[5][5];
        for (int i = 0; i < 5; ++i)
           for (int j = 0; j < 5; ++j)
               m2[i][j] = m[i][j] + other.m[i][j];

       return m2;
    }

    ColorMatrix& ColorMatrix::operator*=(const Gdiplus::ColorMatrix &other)
    {
        *this = (*this * other);
        return *this;
    }

    ColorMatrix& ColorMatrix::operator+=(const Gdiplus::ColorMatrix &other)
    {
        *this = (*this + other);
        return *this;
    }

    ColorMatrix& ColorMatrix::operator=(const ColorMatrix &other)
    {
        memcpy(m, other.m, sizeof(Gdiplus::REAL) * 5 * 5);
        return *this;
    }

    ColorMatrix ColorMatrix::operator*(const ColorMatrix &other) const
    {
        Gdiplus::REAL m2[5][5];
        memset(&m2, 0, sizeof(Gdiplus::REAL) * 5 * 5);
        for (int a = 0; a < 5; ++a)
            for (int b = 0; b < 5; ++b)
                for (int i = 0, j = 0; i < 5; ++i, ++j)
                    m2[a][b] += m[a][i] * other.m[j][b];

        return m2;
    }

    ColorMatrix ColorMatrix::operator+(const ColorMatrix &other) const
    {
        Gdiplus::REAL m2[5][5];
        for (int i = 0; i < 5; ++i)
           for (int j = 0; j < 5; ++j)
               m2[i][j] = m[i][j] + other.m[i][j];

       return m2;
    }

    ColorMatrix& ColorMatrix::operator*=(const ColorMatrix &other)
    {
        *this = (*this * other);
        return *this;
    }

    ColorMatrix& ColorMatrix::operator+=(const ColorMatrix &other)
    {
        *this = (*this + other);
        return *this;
    }

    ColorMatrix::operator Gdiplus::ColorMatrix() const
    {
        //Gdiplus::ColorMatrix cm;
        //memcpy(cm.m, m, sizeof(Gdiplus::REAL) * 5 * 5);
        //return cm;
        return *static_cast<Gdiplus::ColorMatrix*>((void*)this);
    }

    ColorMatrix::operator Gdiplus::ColorMatrix&()
    {
        //Gdiplus::ColorMatrix cm;
        //memcpy(cm.m, m, sizeof(Gdiplus::REAL) * 5 * 5);
        //return cm;
        return *static_cast<Gdiplus::ColorMatrix*>((void*)this);
    }

    ColorMatrix& ColorMatrix::TransformAlpha(float transform)
    {
        m[3][3] = min(1., max(0., m[3][3] * transform));
        return *this;
    }

    ColorMatrix& ColorMatrix::SetAlpha(float transform)
    {
        m[3][3] = min(1., max(0., transform));
        return *this;
    }

    ColorMatrix& ColorMatrix::Grayscale()
    {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = grayscalematrix.m[i][j];
        return *this;
    }

    ColorMatrix& ColorMatrix::Darken(float transform)
    {
        transform = min(1., max(0., transform));
        for (int i = 0; i < 3; ++i)
            m[4][i] -= transform;
        return *this;
    }

    ColorMatrix& ColorMatrix::Brighten(float transform)
    {
        transform = min(1., max(0., transform));
        for (int i = 0; i < 3; ++i)
            m[i][4] += transform;
        return *this;
    }

    ColorMatrix& ColorMatrix::Saturate(float transform)
    {
        transform = 1. + min(1., max(0., transform)) * 3;
        float tc = 1 - transform;
        float r = 0.3086 * tc, g = 0.6094 * tc, b = 0.0820 * tc;
        m[0][0] = r + transform;
        m[0][1] = r;
        m[0][2] = r;

        m[1][0] = g;
        m[1][1] = g + transform;
        m[1][2] = g;

        m[2][0] = b;
        m[2][1] = b;
        m[2][2] = b + transform;

        return *this;
    }

    ColorMatrix& ColorMatrix::Desaturate(float transform)
    {
        transform = 1. - min(1., max(0., transform));
        float tc = 1 - transform;
        float r = 0.3086 * tc, g = 0.6094 * tc, b = 0.0820 * tc;
        m[0][0] = r + transform;
        m[0][1] = r;
        m[0][2] = r;

        m[1][0] = g;
        m[1][1] = g + transform;
        m[1][2] = g;

        m[2][0] = b;
        m[2][1] = b;
        m[2][2] = b + transform;

        return *this;
    }

    ColorMatrix& ColorMatrix::SetContrast(float newcontrast)
    {
        newcontrast = max(0., newcontrast);
        for (int i = 0; i < 3; ++i)
            m[i][i] = newcontrast;
        if (newcontrast < 0.5F)
            return Brighten(0.5 - newcontrast);
        else
            return Darken(newcontrast - 0.5);
    }


    //---------------------------------------------


    Matrix::Matrix()
    {
    }

    Matrix::Matrix(const Matrix &other)
    {
        *this = other;
    }

    Matrix::Matrix(const Gdiplus::Matrix &other)
    {
        *this = other;
    }

    Matrix& Matrix::operator=(const Matrix &other)
    {
        return (*this = (const Gdiplus::Matrix&)other);
    
    }

    Matrix& Matrix::operator=(const Gdiplus::Matrix &other)
    {
        Gdiplus::REAL elems[6];
        other.GetElements(elems);
        SetElements(elems[0], elems[1], elems[2], elems[3], elems[4], elems[5]);
        return *this;
    }

    Matrix& Matrix::SetElements(Gdiplus::REAL m11, Gdiplus::REAL m12, Gdiplus::REAL m21, Gdiplus::REAL m22, Gdiplus::REAL dx, Gdiplus::REAL dy)
    {
        Gdiplus::Matrix::SetElements(m11, m12, m21, m22, dx, dy);
        return *this;
    }

    void Matrix::GetElements(Gdiplus::REAL &m11, Gdiplus::REAL &m12, Gdiplus::REAL &m21, Gdiplus::REAL &m22, Gdiplus::REAL &dx, Gdiplus::REAL &dy)
    {
        Gdiplus::REAL elems[6];
        Gdiplus::Matrix::GetElements(elems);
        m11 = elems[0];
        m12 = elems[1];
        m21 = elems[2];
        m22 = elems[3];
        dx = elems[4];
        dy = elems[5];
    }

    Matrix::operator Gdiplus::Matrix&()
    {
        return *static_cast<Gdiplus::Matrix*>((void*)this);
    }

    bool Matrix::operator==(const Matrix &other)
    {
        return Equals(&other) != FALSE;
    }

    bool Matrix::operator==(const Gdiplus::Matrix &other)
    {
        return Equals(&other) != FALSE;
    }

    Matrix& Matrix::Multiply(const Matrix &matrix, bool prepend)
    {
        Gdiplus::Matrix::Multiply(&matrix, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }

    Matrix& Matrix::Multiply(const Gdiplus::Matrix &matrix, bool prepend)
    {
        Gdiplus::Matrix::Multiply(&matrix, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }

    Matrix& Matrix::Rotate(Gdiplus::REAL angle, bool prepend)
    {
        Gdiplus::Matrix::Rotate(angle, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }

    Matrix& Matrix::RotateAt(Gdiplus::REAL angle, PointF center, bool prepend)
    {
        Gdiplus::Matrix::RotateAt(angle, center, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }

    Matrix& Matrix::Scale(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend)
    {
        Gdiplus::Matrix::Scale(x, y, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }

    Matrix& Matrix::Translate(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend)
    {
        Gdiplus::Matrix::Translate(x, y, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }

    Matrix& Matrix::Shear(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend)
    {
        Gdiplus::Matrix::Shear(x, y, prepend ? Gdiplus::MatrixOrderPrepend : Gdiplus::MatrixOrderAppend);
        return *this;
    }


    //---------------------------------------------


    extern ConstValue<Application, DWORD> LOCALE_ENGLISH_DEFAULT;
    extern ConstValue<Application, DWORD> gen_lcid;

    void GenLower(std::wstring &str)
    {
        wchar_t *dest = new wchar_t[str.size()];
        LCMapString(gen_lcid, LCMAP_LOWERCASE, str.c_str(), str.size(), dest, str.size());
        str = std::wstring(dest,str.size());
        delete[] dest;
        // //std::use_facet< std::ctype<wchar_t> >(std::locale("invariant")).tolower(&str[0], &str[0] + str.size());
        //std::transform(str.begin(), str.end(), str.begin(), std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>), std::locale("C")));
    }

    void GenUpper(std::wstring &str)
    {
        wchar_t *dest = new wchar_t[str.size()];
        LCMapString(gen_lcid, LCMAP_UPPERCASE, str.c_str(), str.size(), dest, str.size());
        str = std::wstring(dest,str.size());
        delete[] dest;
        // //std::use_facet< std::ctype<wchar_t> >(std::locale("invariant")).tolower(&str[0], &str[0] + str.size());
        //std::transform(str.begin(), str.end(), str.begin(), std::bind2nd(std::ptr_fun(&std::toupper<wchar_t>), std::locale("C")));
    }

    std::wstring GenToLower(const std::wstring &str)
    {
        std::wstring result;
        wchar_t *dest = new wchar_t[str.size()];
        LCMapString(gen_lcid, LCMAP_LOWERCASE, str.c_str(), str.size(), dest, str.size());
        result = std::wstring(dest,str.size());
        delete[] dest;
        //result.resize(str.size());
        // //std::use_facet< std::ctype<wchar_t> >(std::locale("invariant")).tolower(&str[0], &str[0] + str.size());
        //std::transform(str.begin(), str.end(), result.begin(), std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>), std::locale("C")));
        return result;
    }

    std::wstring GenToUpper(const std::wstring &str)
    {
        std::wstring result;
        wchar_t *dest = new wchar_t[str.size()];
        LCMapString(gen_lcid, LCMAP_UPPERCASE, str.c_str(), str.size(), dest, str.size());
        result = std::wstring(dest,str.size());
        delete[] dest;
        //result.resize(str.size());
        // //std::use_facet< std::ctype<wchar_t> >(std::locale("invariant")).tolower(&str[0], &str[0] + str.size());
        //std::transform(str.begin(), str.end(), result.begin(), std::bind2nd(std::ptr_fun(&std::toupper<wchar_t>), std::locale("C")));
        return result;
    }

    template<typename C>
    static void _splitstring(const std::wstring &str, const std::wstring &delim, C &split, bool skipempty)
    {
        int start = 0;
        int end = 0;
        int len = str.size();

        if (str.empty())
            return;

        const wchar_t *pos = &str[0];

        while (end != len)
        {
            while (end != len && delim.find(pos[end]) == std::wstring::npos)
                end++;

            if (!skipempty || end != start)
                split.push_back(std::wstring(pos + start, end - start));

            while (end != len && delim.find(pos[end]) != std::wstring::npos)
                end++;
            start = end;
        }
    }

    void splitstring(const std::wstring &str, const std::wstring &delim, std::vector<std::wstring> &split, bool skipempty)
    {
        _splitstring(str, delim, split, skipempty);
    }

    void splitstring(const std::wstring &str, const std::wstring &delim, std::list<std::wstring> &split, bool skipempty)
    {
        _splitstring(str, delim, split, skipempty);
    }

    template<typename C>
    static void _splitstring(const std::wstring &str, wchar_t delim, C &split, bool skipempty)
    {
        if (str.empty())
            return;

        size_t pos = 0;
        size_t prevpos;

        while (prevpos = pos, (pos = str.find_first_of(delim, pos)) != std::wstring::npos)
        {
            if (!skipempty || pos != prevpos)
                split.push_back(str.substr(prevpos, pos - prevpos));
            ++pos;
        }

        split.push_back(str.substr(prevpos));
    }

    void splitstring(const std::wstring &str, wchar_t delim, std::vector<std::wstring> &split, bool skipempty)
    {
        _splitstring(str, delim, split, skipempty);
    }

    void splitstring(const std::wstring &str, wchar_t delim, std::list<std::wstring> &split, bool skipempty)
    {
        _splitstring(str, delim, split, skipempty);
    }

    bool StrToInt(const std::wstring &str, int &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = (len == std::wstring::npos ? slen - pos : min(len, slen - pos)) - 1;
        if (len < 0)
            return false;

        slen = len + 1;

        result = 0;
        unsigned int p = 1;
        wchar_t ch;
        for ( ; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];
            if (!len && ch == L'-')
            {
                if (slen == 1)
                    return false;
                result *= -1;
            }
            else
            {
                if (ch < L'0' || ch > L'9')
                   return false;
                result += (ch - L'0') * p;
                p *= 10;
            }
        }

        return true;
    }

    bool StrToInt(const std::wstring &str, long &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = (len == std::wstring::npos ? slen - pos : min(len, slen - pos)) - 1;
        if (len < 0)
            return false;

        slen = len + 1;

        result = 0;
        unsigned long p = 1;
        wchar_t ch;
        for ( ; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];
            if (!len && ch == L'-')
            {
                if (slen == 1)
                    return false;
                result *= -1;
            }
            else
            {
                if (ch < L'0' || ch > L'9')
                   return false;
                result += (ch - L'0') * p;
                p *= 10;
            }
        }

        return true;
    }

    bool StrToInt(const std::wstring &str, unsigned int &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = (len == std::wstring::npos ? slen - pos : min(len, slen - pos)) - 1;
        if (len < 0)
            return false;

        result = 0;
        unsigned int p = 1;
        wchar_t ch;
        for ( ; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];
            if (ch < L'0' || ch > L'9')
                return false;
            result += (ch - L'0') * p;
            p *= 10;
        }

        return true;
    }

    bool StrToInt(const std::wstring &str, short &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = (len == std::wstring::npos ? slen - pos : min(len, slen - pos)) - 1;
        if (len < 0)
            return false;

        slen = len + 1;

        result = 0;
        unsigned short p = 1;
        wchar_t ch;
        for ( ; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];
            if (!len && ch == L'-')
            {
                if (slen == 1)
                    return false;
                result *= -1;
            }
            else
            {
                if (ch < L'0' || ch > L'9')
                   return false;
                result += (ch - L'0') * p;
                p *= 10;
            }
        }

        return true;
    }

    bool StrToInt(const std::wstring &str, unsigned short &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = (len == std::wstring::npos ? slen - pos : min(len, slen - pos)) - 1;
        if (len < 0)
            return false;

        result = 0;
        unsigned short p = 1;
        wchar_t ch;
        for ( ; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];
            if (ch < L'0' || ch > L'9')
                return false;
            result += (ch - L'0') * p;
            p *= 10;
        }

        return true;
    }

    bool StrToFloat(const std::wstring &str, double &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = len == std::wstring::npos ? slen - pos : min(len, slen - pos);
        if (len <= 0)
            return false;
        wchar_t ch = str[pos];
        if (ch == L' ' || ch == L'\n' || ch == L'\r' || ch == '\t')
            return false;
        const wchar_t *c = str.c_str() + pos;
        wchar_t *endp;
        result = wcstod(c, &endp);

        if (endp != c + len)
            return false;

        return true;
    }

    bool StrHexToInt(const std::wstring &str, int &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = len == std::wstring::npos ? slen - pos : min(len, slen - pos);
        if (len >= 2 && str[pos] == L'0' && (str[pos + 1] == L'x' ||str[pos + 1] == L'X'))
            len -= 2, pos += 2;
        if (len <= 0)
            return false;

        result = 0;
        int p = 1;
        wchar_t ch;
        for (--len; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];

            if (ch >= L'0' && ch <= L'9')
                result += (ch - L'0') * p;
            else if (ch >= L'a' && ch <= L'f')
                result += (ch - L'a' + 10) * p;
            else if (ch >= L'A' && ch <= L'F')
                result += (ch - L'A' + 10) * p;
            else
                return false;
            p = p << 4;
        }

        return true;
    }

    bool StrHexToInt(const std::wstring &str, long &result, std::wstring::size_type pos, std::wstring::size_type len)
    {
        int slen = str.length();
        len = len == std::wstring::npos ? slen - pos : min(len, slen - pos);
        if (len >= 2 && str[pos] == L'0' && (str[pos + 1] == L'x' ||str[pos + 1] == L'X'))
            len -= 2, pos += 2;
        if (len <= 0)
            return false;

        result = 0;
        int p = 1;
        wchar_t ch;
        for (--len; len != std::wstring::npos; --len)
        {
            ch = str[pos + len];

            if (ch >= L'0' && ch <= L'9')
                result += (ch - L'0') * p;
            else if (ch >= L'a' && ch <= L'f')
                result += (ch - L'a' + 10) * p;
            else if (ch >= L'A' && ch <= L'F')
                result += (ch - L'A' + 10) * p;
            else
                return false;
            p = p << 4;
        }

        return true;
    }

    std::wstring IntToStr(int val)
    {
        //std::wstringstream ss;
        //ss << val;
        //return ss.str();

        wchar_t dest[12];
        _itow(val, dest, 10);
        return dest;
    }

    std::wstring FloatToStr(double val)
    {
        //std::wstringstream ss;
        //ss << val;
        //return ss.str();

        char *c = setlocale(LC_NUMERIC, nullptr);
        if (strcmp(c, "C") != 0)
            c = _strdup(setlocale(LC_NUMERIC, "C"));
        else
            c = nullptr;

        wchar_t fstr[3 + FLT_MANT_DIG - FLT_MIN_EXP + 1 + /* safety first */ 64];
        swprintf(fstr, L"%f", val);

        if (c != nullptr)
        {
            setlocale(LC_NUMERIC, c);
            free(c);
        }

        return fstr;
    }

    static const std::pair<const wchar_t*, char> shortcutcodes[] = {
            std::make_pair(L"BkSp", VK_BACK), /* 0x08 */
            std::make_pair(L"BackSpace", VK_BACK), /* 0x08 */
            std::make_pair(L"Tab", VK_TAB), /* 0x09 */
            std::make_pair(L"Enter", VK_RETURN), /* 0x0d */
            std::make_pair(L"Return", VK_RETURN), /* 0x0d */
            std::make_pair(L"Pause", VK_PAUSE), /* 0x13 */
            std::make_pair(L"Esc", VK_ESCAPE), /* 0x1B */
            std::make_pair(L"Escape", VK_ESCAPE), /* 0x1B */
            std::make_pair(L"Space", VK_SPACE), /* 0x20 */
            std::make_pair(L"PgUp", VK_PRIOR), /* 0x21 */
            std::make_pair(L"PageUp", VK_PRIOR), /* 0x21 */
            std::make_pair(L"PgDn", VK_NEXT), /* 0x22 */
            std::make_pair(L"PageDown", VK_NEXT), /* 0x22 */
            std::make_pair(L"End", VK_END), /* 0x23 */
            std::make_pair(L"Home", VK_HOME), /* 0x24 */
            std::make_pair(L"Left", VK_LEFT), /* 0x25 */
            std::make_pair(L"Up", VK_UP), /* 0x26 */
            std::make_pair(L"Right", VK_RIGHT), /* 0x27 */
            std::make_pair(L"Down", VK_DOWN), /* 0x28 */
            std::make_pair(L"Ins", VK_INSERT), /* 0x2d */
            std::make_pair(L"Insert", VK_INSERT), /* 0x2d */
            std::make_pair(L"Del", VK_DELETE), /* 0x2E */
            std::make_pair(L"Delete", VK_DELETE), /* 0x2E */
            std::make_pair(L"NumPad0", VK_NUMPAD0), /* 0x60 */
            std::make_pair(L"NumPad 0", VK_NUMPAD0), /* 0x60 */
            std::make_pair(L"NumPad1", VK_NUMPAD1), /* 0x61 */
            std::make_pair(L"NumPad 1", VK_NUMPAD1), /* 0x61 */
            std::make_pair(L"NumPad2", VK_NUMPAD2), /* 0x62 */
            std::make_pair(L"NumPad 2", VK_NUMPAD2), /* 0x62 */
            std::make_pair(L"NumPad3", VK_NUMPAD3), /* 0x63 */
            std::make_pair(L"NumPad 3", VK_NUMPAD3), /* 0x63 */
            std::make_pair(L"NumPad4", VK_NUMPAD4), /* 0x64 */
            std::make_pair(L"NumPad 4", VK_NUMPAD4), /* 0x64 */
            std::make_pair(L"NumPad5", VK_NUMPAD5), /* 0x65 */
            std::make_pair(L"NumPad 5", VK_NUMPAD5), /* 0x65 */
            std::make_pair(L"NumPad6", VK_NUMPAD6), /* 0x66 */
            std::make_pair(L"NumPad 6", VK_NUMPAD6), /* 0x66 */
            std::make_pair(L"NumPad7", VK_NUMPAD7), /* 0x67 */
            std::make_pair(L"NumPad 7", VK_NUMPAD7), /* 0x67 */
            std::make_pair(L"NumPad8", VK_NUMPAD8), /* 0x68 */
            std::make_pair(L"NumPad 8", VK_NUMPAD8), /* 0x68 */
            std::make_pair(L"NumPad9", VK_NUMPAD9), /* 0x69 */
            std::make_pair(L"NumPad 9", VK_NUMPAD9), /* 0x69 */
            std::make_pair(L"*", VK_MULTIPLY), /* 0x6a */
            std::make_pair(L"Mul", VK_MULTIPLY), /* 0x6a */
            std::make_pair(L"Multiply", VK_MULTIPLY), /* 0x6a */
            std::make_pair(L"+", VK_ADD), /* 0x6b */
            std::make_pair(L"Add", VK_ADD), /* 0x6b */
            std::make_pair(L"-", VK_SUBTRACT), /* 0x6d */
            std::make_pair(L"Subtract", VK_SUBTRACT), /* 0x6d */
            std::make_pair(L".", VK_DECIMAL), /* 0x6e */
            std::make_pair(L"Decimal", VK_DECIMAL), /* 0x6e */
            std::make_pair(L"/", VK_DIVIDE), /* 0x6f */
            std::make_pair(L"Divide", VK_DIVIDE), /* 0x6f */
            std::make_pair(L"F1", VK_F1), /* 0x70 */
            std::make_pair(L"F2", VK_F2), /* 0x71 */
            std::make_pair(L"F3", VK_F3), /* 0x72 */
            std::make_pair(L"F4", VK_F4), /* 0x73 */
            std::make_pair(L"F5", VK_F5), /* 0x74 */
            std::make_pair(L"F6", VK_F6), /* 0x75 */
            std::make_pair(L"F7", VK_F7), /* 0x76 */
            std::make_pair(L"F8", VK_F8), /* 0x77 */
            std::make_pair(L"F9", VK_F9), /* 0x78 */
            std::make_pair(L"F10", VK_F10), /* 0x79 */
            std::make_pair(L"F11", VK_F11), /* 0x7a */
            std::make_pair(L"F12", VK_F12), /* 0x7b */
            std::make_pair(L"F13", VK_F13), /* 0x7c */
            std::make_pair(L"F14", VK_F14), /* 0x7d */
            std::make_pair(L"F15", VK_F15), /* 0x7e */
            std::make_pair(L"F16", VK_F16), /* 0x7f */
            std::make_pair(L"F17", VK_F17), /* 0x80 */
            std::make_pair(L"F18", VK_F18), /* 0x81 */
            std::make_pair(L"F19", VK_F19), /* 0x82 */
            std::make_pair(L"F20", VK_F20), /* 0x83 */
            std::make_pair(L"F21", VK_F21), /* 0x84 */
            std::make_pair(L"F22", VK_F22), /* 0x85 */
            std::make_pair(L"F23", VK_F23), /* 0x86 */
            std::make_pair(L"F24", VK_F24), /* 0x87 */
            std::make_pair(L"NumLock", VK_NUMLOCK), /* 0x90 */
            std::make_pair(L"\\", '\\'),
            std::make_pair(L"BacksLash", '\\'),
            std::make_pair(L"=", '='),
            std::make_pair(L"Equal", '='),
            std::make_pair(L"`", '`'),
            std::make_pair(L"Grave accent", '`'),
            std::make_pair(L"[", '['),
            std::make_pair(L"]", ']'),
            std::make_pair(L";", ';'),
            std::make_pair(L"Semicolon", ';'),
            std::make_pair(L",", ','),
            std::make_pair(L"Comma", ','),
    };

    static void AppendShortcutStr(std::wstring &str, const wchar_t *appended)
    {
        if (!str.empty())
            str += L"+";
        str += appended;
    }

    static void AppendShortcutChar(std::wstring &str, char appended)
    {
        if (!str.empty())
            str += L"+";
        str += appended;
    }

    std::wstring ShortcutToStr(WORD shortcut)
    {
        if (shortcut == 0)
            return std::wstring();
        std::wstring result;
        if (((shortcut >> 8) & 1) == 1)
            AppendShortcutStr(result, L"Ctrl");
        if (((shortcut >> 9) & 1) == 1)
            AppendShortcutStr(result, L"Shift");
        if (((shortcut >> 10) & 1) == 1)
            AppendShortcutStr(result, L"Alt");

        if ((shortcut & 0xff) != 0)
        {
            if (((shortcut & 0xff) >= L'A' && (shortcut & 0xff) <= L'Z') || ((shortcut & 0xff) >= L'0' && (shortcut & 0xff) <= L'9'))
                AppendShortcutChar(result, (shortcut & 0xff));

            int cnt = sizeof(shortcutcodes) / sizeof(std::pair<const wchar_t*, int>);
            for (int ix = 0; ix < cnt; ++ix)
            {
                if (shortcutcodes[ix].second == (shortcut & 0xff))
                {
                    AppendShortcutStr(result, shortcutcodes[ix].first);
                    break;
                }
            }
        }

        return result;
    }

    bool StrToShortcut(const std::wstring &str, WORD &result)
    {
        result = 0;

        int strpos = 0;
        int ppos = 0;
        int len = str.length();
        while (strpos < len && strpos != (int)std::wstring::npos)
        {
            while (strpos < len && str[strpos] == L' ')
                ++strpos;
            ppos = str.find(L'+', strpos + 1);
            int p = (ppos == (int)std::wstring::npos) ? len - 1 : ppos - 1;
            while (p >= strpos && str[p] == L' ')
                --p;
            if (p < strpos)
                return false;
            std::wstring s = GenToLower(str.substr(strpos, p - strpos + 1));
            if (s == L"ctrl" || s == L"control")
                result |= (1 << 8);
            else if (s == L"shift")
                result |= (1 << 9);
            else if (s == L"alt")
                result |= (1 << 10);
            else if ((result & 0xff) != 0)
                return false;
            else if (strpos == p && s[0] >= L'a' && s[0] <= L'z')
                result |= (s[0] - L'a') + 0x41;
            else if (strpos == p && s[0] >= L'0' && s[0] <= L'9')
                result |= (s[0] - L'0') + 0x30;
            else
            {
                int cnt = sizeof(shortcutcodes) / sizeof(std::pair<const wchar_t*, int>);
                for (int ix = 0; ix < cnt; ++ix)
                {
                    if (GenToLower(shortcutcodes[ix].first) == s)
                    {
                        result |= shortcutcodes[ix].second;
                        break;
                    }
                }
            }

            strpos = ppos + (ppos == (int)std::wstring::npos ? 0 : 1);
        }
        return true;
    }

#ifdef DESIGNING
    void HexEncode(const std::wstring &str, std::ostream &out)
    {
        auto ss = std::streamstate(out);
        out << std::hex << std::uppercase << std::right << std::setfill('0');
        unsigned int len = str.length();
        for (unsigned int ix = 0; ix < len; ++ix)
        {
            const wchar_t c = str[ix];
            out << std::setw(2) << int((byte)(c & 0xff));
            out << std::setw(2) << int((byte)((c & 0xff00) >> 8));
        }

        std::streamstate(out, ss);
    }

    void HexEncode(const void *str, unsigned int bytelen, std::ostream &out)
    {
        auto ss = std::streamstate(out);

        out << std::hex << std::uppercase << std::right << std::setfill('0');
        for (unsigned int ix = 0; ix < bytelen; ++ix)
            out << std::setw(2) << (int)((byte*)str)[ix];

        std::streamstate(out, ss);
    }

    std::string HexEncode(const std::wstring &str)
    {
        std::stringstream s;
        HexEncode(str, s);
        return s.str();
    }

    std::string HexEncode(const std::string &str)
    {
        return HexEncode(str.c_str(), str.length());
    }

    std::string HexEncode(const void *str, unsigned int bytelen)
    {
        std::stringstream s;
        HexEncode(str, bytelen, s);
        return s.str();
    }

    void HexEncodeW(const std::wstring &str, std::wostream &out)
    {
        auto ss = std::streamstate(out);
        out << std::hex << std::uppercase << std::right << std::setfill(L'0');
        unsigned int len = str.length();
        for (unsigned int ix = 0; ix < len; ++ix)
        {
            const wchar_t c = str[ix];
            out << std::setw(2) << int((byte)(c & 0xff));
            out << std::setw(2) << int((byte)((c & 0xff00) >> 8));
        }

        std::streamstate(out, ss);
    }

    void HexEncodeW(const void *str, unsigned int bytelen, std::wostream &out)
    {
        auto ss = std::streamstate(out);

        out << std::hex << std::uppercase << std::right << std::setfill(L'0');
        for (unsigned int ix = 0; ix < bytelen; ++ix)
            out << std::setw(2) << (int)((byte*)str)[ix];

        std::streamstate(out, ss);
    }

    std::wstring HexEncodeW(const std::wstring &str)
    {
        std::wstringstream s;
        HexEncodeW(str, s);
        return s.str();
    }

    std::wstring HexEncodeW(const std::string &str)
    {
        return HexEncodeW(str.c_str(), str.length());
    }

    std::wstring HexEncodeW(const void *str, unsigned int bytelen)
    {
        std::wstringstream s;
        HexEncodeW(str, bytelen, s);
        return s.str();
    }

    static int HexDecodeHelper(WORD c)
    {
        if (c >= 0x30 && c <= 0x39)
            return c - 0x30;
        else if (c >= 0x61 && c <= 0x66)
            return c - 0x61 + 10;
        else if (c >= 0x41 && c <= 0x46)
            return c - 0x41 + 10;
        else
            return -1;
    }

    template<typename chartype>
    bool HexDecodeChar(const chartype str[2], byte &ch)
    {
        int a = HexDecodeHelper(str[0]);
        int b = HexDecodeHelper(str[1]);
        if (a < 0 || b < 0)
            return false;
        ch = a * 16 + b;
        return true;
    }

    bool HexDecodeChar(const wchar_t str[2], byte &ch)
    {
        return HexDecodeChar<wchar_t>(str, ch);
    }

    bool HexDecodeChar(const char str[2], byte &ch)
    {
        return HexDecodeChar<char>(str, ch);
    }

    template<typename sourcechartype, typename destchartype>
    bool HexDecodeStringToken(const sourcechartype *src, unsigned int &srclen, sourcechartype delimiter, std::basic_ostream<destchartype> &dest)
    {
        destchartype ch;
        unsigned int destsize = sizeof(destchartype);
        byte b;
        for (unsigned int ix = 0; ix < srclen; ix += destsize * 2)
        {
            ch = 0;
            for (unsigned int iy = 0; iy < destsize; ++iy)
            {
                 if (!HexDecodeChar( src + ix + iy * 2, b))
                 {
                     srclen = ix;
                     return false;
                 }
                 ch |= b << (8 * iy);
            }

            if (ch == delimiter)
            {
                srclen = ix + destsize * 2;
                return true;
            }
            dest << ch;
        }
        return true;
    }

    bool HexDecodeStringToken(const wchar_t *src, unsigned int &srclen, wchar_t delimiter, std::wostream &dest)
    {
        return HexDecodeStringToken<wchar_t, wchar_t>(src, srclen, delimiter, dest);
    }

    bool HexDecodeStringToken(const char *src, unsigned int &srclen, char delimiter, std::wostream &dest)
    {
        return HexDecodeStringToken<char, wchar_t>(src, srclen, delimiter, dest);
    }

    bool HexDecodeStringToken(const wchar_t *src, unsigned int &srclen, wchar_t delimiter, std::ostream &dest)
    {
        return HexDecodeStringToken<wchar_t, char>(src, srclen, delimiter, dest);
    }

    bool HexDecodeStringToken(const char *src, unsigned int &srclen, char delimiter, std::ostream &dest)
    {
        return HexDecodeStringToken<char, char>(src, srclen, delimiter, dest);
    }

    template<typename chartype>
    bool HexDecodeBytes(const chartype *src, unsigned int &srclen, byte *dest, unsigned int destlen)
    {
        unsigned int ix = 0;
        byte b;
        for ( ; ix * 2 < srclen && ix < destlen; ++ix)
        {
            if (!HexDecodeChar(src + ix * 2, b))
            {
                srclen = ix;
                return false;
            }
            dest[ix] = b;
        }
        srclen = ix;
        return true;
    }

    bool HexDecodeBytes(const char *src, unsigned int &srclen, byte *dest, unsigned int destlen)
    {
        return HexDecodeBytes<char>(src, srclen, dest, destlen);
    }

    bool HexDecodeBytes(const wchar_t *src, unsigned int &srclen, byte *dest, unsigned int destlen)
    {
        return HexDecodeBytes<wchar_t>(src, srclen, dest, destlen);
    }
#endif

    const wchar_t *_trim_characters = L" \t\f\v\n\r";
    std::wstring trim(std::wstring str, bool fromstart, bool fromend)
    {
        if (fromend)
        {
            size_t e = str.find_last_not_of(_trim_characters);
            if (e == std::wstring::npos)
                return std::wstring();
            str.erase(e + 1);
        }
        if (fromstart)
        {
            size_t e = str.find_first_not_of(_trim_characters);
            if (e == std::wstring::npos)
                return std::wstring();
            str.erase(0, e);
        }

        return str;
    }

    std::wstring trim(std::wstring str, const wchar_t *trim_characters, bool fromstart, bool fromend)
    {
        if (fromend)
        {
            size_t e = str.find_last_not_of(trim_characters);
            if (e == std::wstring::npos)
                return std::wstring();
            str.erase(e + 1);
        }
        if (fromstart)
        {
            size_t e = str.find_first_not_of(trim_characters);
            if (e == std::wstring::npos)
                return std::wstring();
            str.erase(0, e);
        }

        return str;
    }

    std::wstring EscapeCString(const std::wstring &str)
    {
        std::wstringstream ws;
        size_t pos = 0;
        size_t start = 0;
        while (start < str.length() && (pos = str.find_first_of(L"\\\"", start)) != std::wstring::npos)
        {
            ws << str.substr(start, pos - start) << L"\\" << str[pos];
            start = pos + 1;
        }

        ws << str.substr(start);

        return ws.str();
    }

    //---------------------------------------------


#ifndef __MINGW32__
    int round(double d)
    {
        return d < 0 ? (int)(d - 0.5) : (int)(d + 0.5);
    }
#endif


    //---------------------------------------------


    int PathLength(wchar_t *str, bool unixslash)
    {
        wchar_t *p = wcsrchr(str, unixslash ? L'/' : L'\\');
        if (!p)
            return 0;
        return (p - str) + 1;
    }

    int PathLength(const std::wstring &str, bool unixslash)
    {
        size_t pos = str.rfind(unixslash ? L'/' : L'\\');
        if (pos == std::wstring::npos)
            return 0;
        return pos + 1;
    }

    int FindFileNameExtension(wchar_t *str, bool unixslash)
    {
        int startpos = PathLength(str, unixslash);
        str += startpos;

        wchar_t *p = wcsrchr(str, L'.');
        if (!p)
            return 0;
        return (p - str) + 1 + startpos;
    }

    int FindFileNameExtension(const std::wstring &str, bool unixslash)
    {
        int startpos = PathLength(str, unixslash);

        size_t pos = str.rfind(L'.');
        if (pos == std::wstring::npos || (int)pos < startpos)
            return 0;
        return pos + 1;
    }

    std::wstring GetFileName(const std::wstring &str, bool unixslash)
    {
        return str.substr(PathLength(str, unixslash));
    }

    std::wstring GetFilePath(const std::wstring &str, bool unixslash)
    {
        return str.substr(0, PathLength(str, unixslash));
    }

    std::wstring GetFileExtension(const std::wstring &str, bool unixslash)
    {
        int extpos = FindFileNameExtension(str, unixslash);
        if (!extpos)
            return std::wstring();
        return str.substr(extpos);
    }

    bool ValidVarName(const std::wstring &str)
    {
        int s = str.length();
        if (!s)
            return false;
        if ((str[0] < L'a' || str[0] > L'z') && (str[0] < L'A' || str[0] > L'Z') && str[0] != L'_')
            return false;
        for (int ix = 1; ix < s; ++ix)
            if ((str[ix] < L'a' || str[ix] > L'z') && (str[ix] < L'A' || str[ix] > L'Z') && str[ix] != L'_' && (str[ix] < L'0' || str[ix] > L'9'))
                return false;
        return true;
    }

    bool ValidFileName(const std::wstring &ss, bool containspath)
    {
        int s = ss.length();
        if (!s)
            return false;
        int ps = 0;
        if (containspath)
        {
            ps = PathLength(ss);
            if (ps == s || !ValidFilePath(ss.substr(0, ps)))
                return false;
        }
        if (ss[s - 1] == L'.' || ss[s - 1] == L' ' || ss[0] == L' ')
            return false;

        std::wstring str = GenToLower(ps ? ss.substr(ps) : ss);
        s -= ps;

        const wchar_t *reserved[] = { /* 3 character */ L"con", L"prn", L"aux", L"nul", /* 4 character */ L"com1", L"com2", L"com3", L"com4", L"com5", L"com6", L"com7", L"com8", L"com9", L"lpt1", L"lpt2", L"lpt3", L"lpt4", L"lpt5", L"lpt6", L"lpt7", L"lpt8", L"lpt9" };
        for (unsigned int ix = 0; ix < sizeof(reserved) / sizeof(char*); ++ix)
            if (str == reserved[ix] || (ix < 4 && s > 3 && str.substr(0, 3) == reserved[ix] && str[3] == L'.') || (ix >= 4 && s > 4 && str.substr(0, 4) == reserved[ix] && str[4] == L'.'))
                return false;

        const wchar_t *reservedchars = L"<>:\"/\\|?*";

        if (str.find_first_of(reservedchars, 0) != std::wstring::npos)
            return false;

        bool onlyperiod = true;
        for (int ix = 0; ix < s; ++ix)
        {
            if (str[ix] != L'.')
                onlyperiod = false;
            if (str[ix] < 32)
                return false;
        }

        if (onlyperiod)
            return false;

        return true;
    }

    bool ValidFilePath(const std::wstring &str)
    {
        int s = str.length();
        bool unc = s > 2 && str[0] == L'\\' && str[1] == L'\\';
        bool longunc = unc && s > 4 && str[2] == L'?' && str[3] == L'\\';
        if (s > MAX_PATH)
        {
            if (!longunc || s >= 32767)
                return false;
        }

        int pos = longunc ? 4 : unc ? 2 : 0;

        // Skip the drive letter.
        //bool driveletter = false;
        if (pos < s - 1 && ((str[pos] >= 'a' && str[pos] <= 'z') || (str[pos] >= 'A' && str[pos] <= 'Z')) && str[pos + 1] == L':')
        {
            pos += 2;
            if (pos < s && str[pos] == L'\\')
                ++pos;
            //driveletter = true;
        }

        int prev = pos;
        while (pos < s)
        {
            while (pos < s && str[pos] != L'\\')
                ++pos;
            if (pos == prev)
                return false;
            if ((pos - prev != 1 || str[prev] != L'.') && (pos - prev != 2 || (str[prev] != L'.' && str[prev + 1] != L'.')) && !ValidFileName(str.substr(prev, pos - prev), false))
                return false;
            ++pos;
            prev = pos;
        }
        return true;
    }

    std::wstring ShortenRelativePath(const std::wstring &str, bool devicepath)
    {
        int s = str.length();
        if (s == 0 || (s >= 4 && str[0] == L'\\' && str[1] == L'\\' && str[2] == L'?' && str[3] == L'\\')) // Return if path is an absolute path starting with "\\?\"
            return str;

        if (devicepath && str.compare(0, 4, L"\\\\.\\") != 0) // Check if path starts with \\.\ for device paths. Set devicepath to false if that string is not found.
            devicepath = false;

        // Get number of \ characters in path to determine the maximum number of folders and other parts.
        int partcnt = 0;
        size_t pos = (size_t)-1;
        do
        {
            ++pos;
            ++partcnt;
            pos = str.find(L'\\', pos);
        } while (pos != std::wstring::npos);
        if (partcnt == 1) // Only a single part which means there are no path separators in the passed string at all. Nothing to do.
            return str;

        int *positions = new int[partcnt]; // The \ character after the given part or the length of the string for the last part.
        bool *needed = new bool[partcnt]; // Set depending on whether the given part of path will be kept or thrown out.
        int firstneeded = !devicepath ? -1 : 2; // Index of first part that can be removed when a ".." part is encountered after it.
        pos = -1;
        for (int ix = 0; ix < partcnt; ++ix)
        {
            ++pos;
            if (ix < partcnt - 1)
                pos = str.find(L'\\', pos);
            else
                pos = s;
            positions[ix] = pos;

            if (devicepath && ix < 3)
            {
                needed[ix] = false;
                continue;
            }

            int partlen = pos - (ix > 0 ? positions[ix - 1] + 1 : 0);
            needed[ix] = (partlen > 2) || ix == 0 || (partlen == 2 && (str[pos - 1] != L'.' || str[pos - 2] != L'.')) || (partlen == 1 && str[pos - 1] != L'.');

            if (partlen == 2 && !needed[ix]) // Check if there's a part before the current ..\ that can be excluded.
            {
                int iy = ix;
                while (--iy > firstneeded)
                {
                    if (!needed[iy])
                        continue;
                    if (positions[iy] - (iy ? positions[iy - 1] : 0) != 2 || str[iy != 0 ? positions[iy - 1] + 1 : 0] != L'.' || str[iy != 0 ? positions[iy - 1] + 2 : 1] != L'.')
                    {
                        needed[iy] = false;
                        break;
                    }
                }
                if (iy == firstneeded)
                    needed[ix] = true;
            }
            if (firstneeded == ix - 1 && needed[ix] && (!partlen || ((partlen == 1 && str[pos - 1] == L'.') || (partlen == 2 && str[pos - 1] == L'.' && str[pos - 2] == L'.'))))
                firstneeded = ix;
        }

        std::wstring path;

        if (devicepath)
            path = L"\\\\.\\";

        firstneeded = 0;
        for (int ix = 0; ix < partcnt; ++ix)
        {
            if (!needed[ix])
                continue;
            if (!ix)
            {
                path += str.substr(0, positions[ix]);
                firstneeded = 1;
            }
            else if (!firstneeded)
            {
                path += str.substr(positions[ix - 1] + 1, positions[ix] - positions[ix - 1] - 1);
                firstneeded = 1;
            }
            else
                path += str.substr(positions[ix - 1], positions[ix] - positions[ix - 1]);
        }

        delete[] positions;
        delete[] needed;

        if (str.length() && str.back() == L'\\' && path.length() && path.back() != L'\\')
            path += L'\\';
        return path;
    }

    bool FileExists(const std::wstring &str)
    {
        if (str.empty())
            return false;

        struct _stat stat;
        if (_wstat(str.c_str(), &stat) < 0 || (stat.st_mode & _S_IFREG) != _S_IFREG)
            return false;
        return true;
    }

    bool PathExists(std::wstring str)
    {
        if (str.empty())
            return false;

        if (str.back() == L'\\')
        {
            bool removeper = true;
            if (str.compare(0, 2, L"\\\\") == 0)  // UNC paths, if the path is \\Server\\Share\ without folders, the ending \ must be retained.
            {
                if (str.length() == 2)
                    return false;

                size_t p = str.find(L"\\", 2);
                if (p != str.length() - 1 && str.find(L"\\", p + 1) == str.length() - 1)
                    removeper = false;
            }
            else if (str.length() == 3 && ((str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z')) && str[1] == L':')
                removeper = false;

            if (removeper)
                str.resize(str.length() - 1);
        }

        struct _stat stat;
        if (_wstat(str.c_str(), &stat) < 0 || (stat.st_mode & _S_IFDIR) != _S_IFDIR)
            return false;
        return true;
    }

    bool CreateFolder(std::wstring path, bool recursive)
    {
        if (!recursive)
            return CreateDirectory(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;

        std::wstring prefix;
        if (path.compare(0, 2, L"\\\\") == 0)
        {
            prefix = L"\\\\";
            path.erase(0, 2);
        }

        std::list<std::wstring> parts;
        splitstring(path, L'\\', parts, true);

        if (parts.empty())
            return false;

        if (!prefix.empty())
        {

            bool qm = parts.front() == L"?";
            if (qm)
            {
                prefix += L"?\\";
                parts.pop_front();

                if (parts.empty())
                    return false;

                if (parts.front() == L"GLOBALROOT")
                {
                    prefix += L"GLOBALROOT\\";
                    parts.pop_front();
                    if (parts.empty())
                        return false;
                }
                if (parts.front() == L"UNC")
                {
                    prefix += L"UNC\\";
                    parts.pop_front();
                    if (parts.empty())
                        return false;
                }
            }
            else if (parts.front() == L".") // Devices not supported.
                return false;
        }

        if (parts.front().back() != L':') // Presumed server name and path.
        {
            prefix += parts.front();
            prefix += L"\\";
            parts.pop_front();

            if (parts.empty())
                return false;

            prefix += parts.front();
            prefix += L"\\";
            parts.pop_front();

            if (parts.empty())
                return true;
        }
        if (parts.front().back() == L':') // Presumed drive path.
        {
            prefix += parts.front();
            prefix += L"\\";
            parts.pop_front();

            if (parts.empty())
                return true;
        }

        while (!parts.empty())
        {
            prefix += parts.front();
            if (CreateDirectory(prefix.c_str(), NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
                return false;
            prefix += L"\\";
            parts.pop_front();
        }
        return true;
    }

    std::wstring AppendToPath(const std::wstring &pathstr, const std::wstring &filestr, bool shortenrelative)
    {
        std::wstring result = pathstr;
        if (pathstr.empty() || pathstr.back() != L'\\')
            result += L'\\';
        result += filestr;
        if (shortenrelative)
            return ShortenRelativePath(result);
        else
            return result;
    }


    //---------------------------------------------

    // Helper for WinVerSupported functions. Returns -1 if the passed major and minor is less than the current running OS version. 1 if it is more.
    int N_WinVer_Sup(int major, int minor)
    {
        if (major < (int)Win32MajorVersion)
            return -1;
        if (major > (int)Win32MajorVersion)
            return 1;
        if (minor < (int)Win32MinorVersion)
            return -1;
        if (minor > (int)Win32MinorVersion)
            return 1;
        return 0;
    }

    bool WinVerSupported(int major, int minor)
    {
        return N_WinVer_Sup(major, minor) <= 0;

    }

    bool WinVerSupported(int minmajor, int minminor, int maxmajor, int maxminor)
    {
        return N_WinVer_Sup(minmajor, minminor) <= 0 && N_WinVer_Sup(maxmajor, maxminor) >= 0;
    }


    //---------------------------------------------


    char* ANSIToUTF8(const char* str, int &len)
    {
        wchar_t* wc = ANSIToWide(str, len);
        if (!len)
            return NULL;
        char *c = WideToUTF8(wc, len);
        delete[] wc;
        return c;
    }

    char* WideToUTF8(const wchar_t *str, int &len)
    {
        int newlen = WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
        if (!newlen)
        {
            len = 0;
            return NULL;
        }

        char *c2 = new char[newlen];
        len = WideCharToMultiByte(CP_UTF8, 0, str, len, c2, newlen, NULL, NULL);
        return c2;
    }

    wchar_t* UTF8ToWide(const char *str, int &len)
    {
        int newlen = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
        if (!newlen)
        {
            len = 0;
            return NULL;
        }
        wchar_t *c2 = new wchar_t[newlen];
        len = MultiByteToWideChar(CP_UTF8, 0, str, len, c2, newlen);
        return c2;
    }

    char* UTF8ToANSI(const char *str, int &len)
    {
        wchar_t *wc = UTF8ToWide(str, len);
        if (!len)
            return NULL;
        char *c = WideToANSI(wc, len);
        delete[] wc;
        return c;
    }

    char* WideToANSI(const wchar_t *str, int &len)
    {
        int newlen = WideCharToMultiByte(CP_ACP, 0, str, len, NULL, 0, 0, 0);
        if (!newlen)
        {
            len = 0;
            return NULL;
        }
        char *newstr = new char[newlen];
        len = WideCharToMultiByte(CP_ACP, 0, str, len, newstr, newlen, 0, 0);
        return newstr;
    }

    wchar_t* ANSIToWide(const char *str, int &len)
    {
        int newlen = MultiByteToWideChar(CP_ACP, 0, str, len, NULL, 0);
        if (!newlen)
        {
            len = 0;
            return NULL;
        }
        wchar_t *newstr = new wchar_t[newlen];
        len = MultiByteToWideChar(CP_ACP, 0, str, len, newstr, newlen);
        return newstr;
    }

    std::string WideToANSI(const std::wstring &str)
    {
        int len = str.length();
        int newlen = WideCharToMultiByte(CP_ACP, 0, str.c_str(), len, NULL, 0, 0, 0);
        if (!newlen)
            return std::string();
        std::string result(newlen, 'x');
        WideCharToMultiByte(CP_ACP, 0, str.c_str(), len, const_cast<char*>(result.c_str()), newlen, 0, 0);
        return result;
    }

    std::wstring ANSIToWide(const std::string &str)
    {
        int len = str.length();
        int newlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), len, NULL, 0);
        if (!newlen)
            return std::wstring();
        std::wstring result(newlen, 'x');
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), len, const_cast<wchar_t*>(result.c_str()), newlen);
        return result;
    }

    std::string ANSIToUTF8(const std::string &str)
    {
        return WideToUTF8(ANSIToWide(str));
    }

    std::string UTF8ToANSI(const std::string &str)
    {
        return WideToANSI(UTF8ToWide(str));
    }

    std::string WideToUTF8(const std::wstring &str)
    {
        int len = str.length();
        int newlen = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), len, NULL, 0, 0, 0);
        if (!newlen)
            return std::string();
        std::string result(newlen, 'x');
        WideCharToMultiByte(CP_UTF8, 0, str.c_str(), len, const_cast<char*>(result.c_str()), newlen, 0, 0);
        return result;
    }

    std::wstring UTF8ToWide(const std::string &str)
    {
        int len = str.length();
        int newlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), len, NULL, 0);
        if (!newlen)
            return std::wstring();
        std::wstring result(newlen, 'x');
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), len, const_cast<wchar_t*>(result.c_str()), newlen);
        return result;
    }

    int UTF8Bytes(const char *str, int strlen)
    {
        int len = 0;
        int chlen;
        while (strlen > 0 || (strlen == -1 && *str))
        {
            if ((*str & 0x80) == 0)
            {
                ++len;
                continue;
            }
            if (*str & 0x40) // At least 2 code points.
            {
                if (*str & 0x20) // At least 3 code points.
                {
                    if (*str & 0x10) // At least 4 code points.
                    {
                        if (*str & 0x08) // More than 4 code points. Stop here.
                            break;
                        chlen = 3;
                    }
                    else
                        chlen = 2;
                }
                else
                    chlen = 1;

                for (int chpos = chlen; chpos > 0; --chpos)
                {
                    ++str;
                    if (strlen != -1)
                        --strlen;
                    if (!strlen || (strlen == -1 && !*str)) // End of buffer in the middle of a code point.
                        break;

                    if (!(*str & 0x80) || (*str & 0x40)) // Invalid character in the middle of a code point.
                        break;
                }
                len += chlen;
            }
            else // UTF8 code points must start with 11XX 0000, which is 0x80 & 0x40.
                break;
        }
        return len;
    }

    int UTF8Length(const char *str, int strlen)
    {
        int len = 0;
        int chlen;
        while (strlen > 0 || (strlen == -1 && *str))
        {
            if ((*str & 0x80) == 0)
            {
                ++len;
                continue;
            }
            if (*str & 0x40) // At least 2 code points.
            {
                if (*str & 0x20) // At least 3 code points.
                {
                    if (*str & 0x10) // At least 4 code points.
                    {
                        if (*str & 0x08) // More than 4 code points. Stop here.
                            break;
                        chlen = 3;
                    }
                    else
                        chlen = 2;
                }
                else
                    chlen = 1;

                while (chlen--)
                {
                    ++str;
                    if (strlen != -1)
                        --strlen;
                    if (!strlen || (strlen == -1 && !*str)) // End of buffer in the middle of a code point.
                        break;

                    if (!(*str & 0x80) || (*str & 0x40)) // Invalid character in the middle of a code point.
                        break;
                }
                ++len;
            }
            else // UTF8 code points must start with 11XX 0000, which is 0x80 & 0x40.
                break;
        }
        return len;
    }


    //---------------------------------------------


    bool CopyToClipboard(const std::string &str)
    {
        TextDataObject *dat = NULL;
        try
        {
            dat = new TextDataObject(str);
        }
        catch(...)
        {
            return false;
        }

        HRESULT res;
        if ((res = OleSetClipboard(dat)) != S_OK)
        {
            dat->Release();
            return false;
        }

        OleFlushClipboard();
        dat->Release();
        return true;

        //return CopyToClipboard(str.c_str(), str.length());
    }

    bool CopyToClipboard(const std::wstring &str)
    {
        TextDataObject *dat = NULL;
        try
        {
            dat = new TextDataObject(str);
        }
        catch(...)
        {
            return false;
        }
        HRESULT res;
        if ((res = OleSetClipboard(dat)) != S_OK)
        {
            dat->Release();
            return false;
        }

        OleFlushClipboard();
        dat->Release();
        return true;

        //return CopyToClipboard(str.c_str(), str.length());
    }

    bool CopyToClipboard(const char *str, unsigned int len)
    {
        TextDataObject *dat = NULL;
        try
        {
            dat = new TextDataObject(str, len);
        }
        catch(...)
        {
            return false;
        }
        HRESULT res;
        if ((res = OleSetClipboard(dat)) != S_OK)
        {
            dat->Release();
            return false;
        }

        OleFlushClipboard();
        dat->Release();
        return true;

        //if (!OpenClipboard(application->Handle()))
        //    return false;
        //if (!EmptyClipboard())
        //{
        //    CloseClipboard();
        //    return false;
        //}

        //HGLOBAL alloced;
        //bool nulladded;

        //nulladded = len && str[len-1] == 0;
        //alloced = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, len + (nulladded ? 0 : 1));
        //if (!alloced)
        //{
        //    CloseClipboard();
        //    return false;
        //}

        //char *locked = (char *)GlobalLock(alloced);
        //if (!locked)
        //{
        //    GlobalFree(alloced);
        //    CloseClipboard();
        //    return false;
        //}

        //strncpy(locked, str, len);
        //if (!nulladded)
        //    locked[len] = 0;
        //GlobalUnlock(alloced);

        //SetClipboardData(CF_TEXT, alloced);

        //CloseClipboard();
        //return true;
    }

    bool CopyToClipboard(const wchar_t *str, unsigned int len)
    {
        TextDataObject *dat = NULL;
        try
        {
            dat = new TextDataObject(str, len);
        }
        catch(...)
        {
            return false;
        }

        HRESULT res;
        if ((res = OleSetClipboard(dat)) != S_OK)
        {
            dat->Release();
            return false;
        }

        OleFlushClipboard();
        dat->Release();
        return true;

        //if (!OpenClipboard(application->Handle()))
        //    return false;
        //if (!EmptyClipboard())
        //{
        //    CloseClipboard();
        //    return false;
        //}

        //HGLOBAL alloced;
        //bool nulladded;

        //nulladded = len && str[len-1] == 0;
        //alloced = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, (len + (nulladded ? 0 : 1)) * sizeof(wchar_t));
        //if (!alloced)
        //{
        //    CloseClipboard();
        //    return false;
        //}

        //wchar_t *locked = (wchar_t *)GlobalLock(alloced);
        //if (!locked)
        //{
        //    GlobalFree(alloced);
        //    CloseClipboard();
        //    return false;
        //}

        //wcsncpy(locked, str, len);
        //if (!nulladded)
        //    locked[len] = 0;
        //GlobalUnlock(alloced);

        //SetClipboardData(CF_UNICODETEXT, alloced);

        //int newlen = WideCharToMultiByte(CP_ACP, NULL, str, len, NULL, 0, 0, 0);
        //alloced = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, (len + (nulladded ? 0 : 1)) );
        //if (!alloced)
        //{
        //    CloseClipboard();
        //    return false;
        //}

        //char *newstr = (char*)GlobalLock(alloced);
        //if (!newstr)
        //{
        //    GlobalFree(alloced);
        //    CloseClipboard();
        //    return false;
        //}

        //newlen = WideCharToMultiByte(CP_ACP, NULL, str, len, newstr, newlen, 0, 0);
        //if (!nulladded)
        //    newstr[newlen] = 0;
        //GlobalUnlock(alloced);
        //SetClipboardData(CF_TEXT, alloced);

        //CloseClipboard();
        //return true;
    }

    bool PasteFromClipboard(std::string &str)
    {
        char *c = NULL;
        int len = 0;
        if (PasteFromClipboard(c, len))
        {
            str = c;
            delete[] c;
            return true;
        }
        else
            return false;
    }

    bool PasteFromClipboard(std::wstring &str)
    {
        wchar_t *c = NULL;
        int len = 0;
        if (PasteFromClipboard(c, len))
        {
            str = c;
            delete[] c;
            return true;
        }
        else
            return false;
    }

    bool PasteFromClipboard(char* &dest, int &len)
    {
        bool nonuni;
        if (!(nonuni = (IsClipboardFormatAvailable(CF_TEXT) != FALSE)) && !IsClipboardFormatAvailable(CF_UNICODETEXT))
            return false;

        if (!OpenClipboard(application->Handle()))
            return false;

        HGLOBAL alloced = GetClipboardData(nonuni ? CF_TEXT : CF_UNICODETEXT);
        if (!alloced)
        {
            CloseClipboard();
            return false;
        }

        if (nonuni)
        {
            char *locked = (char*)GlobalLock(alloced);
            if (!locked)
            {
                GlobalUnlock(alloced);
                CloseClipboard();
                return false;
            }

            len = strlen(locked);
            dest = new char[len + 1];
            strcpy(dest, locked);
        }
        else
        {
            wchar_t *locked = (wchar_t*)GlobalLock(alloced);
            if (!locked)
            {
                GlobalUnlock(alloced);
                CloseClipboard();
                return false;
            }

            len = wcslen(locked) + 1;
            dest = WideToANSI(locked, len);
        }

        GlobalUnlock(alloced);

        CloseClipboard();
        return true;
    }

    bool PasteFromClipboard(wchar_t* &dest, int &len)
    {
        bool uni;
        if (!(uni = (IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE)) && !IsClipboardFormatAvailable(CF_TEXT))
            return false;

        if (!OpenClipboard(application->Handle()))
            return false;

        HGLOBAL alloced = GetClipboardData(uni ? CF_UNICODETEXT : CF_TEXT);
        if (!alloced)
        {
            CloseClipboard();
            return false;
        }

        if (!uni)
        {
            char *locked = (char*)GlobalLock(alloced);
            if (!locked)
            {
                GlobalUnlock(alloced);
                CloseClipboard();
                return false;
            }

            len = strlen(locked);
            dest = ANSIToWide(locked, len);
        }
        else
        {
            wchar_t *locked = (wchar_t*)GlobalLock(alloced);
            if (!locked)
            {
                GlobalUnlock(alloced);
                CloseClipboard();
                return false;
            }

            len = wcslen(locked);
            dest = new wchar_t[len + 1];
            wcscpy(dest, locked);
        }

        GlobalUnlock(alloced);

        CloseClipboard();
        return true;
    }


    //---------------------------------------------


    int CreateCombinedRgn(HRGN &rgn, const Rect &r1, const Rect &r2, RegionCombineModes mode)
    {
        rgn = CreateRectRgnIndirect(&r1);
        HRGN rgn2 = CreateRectRgnIndirect(&r2);
        int res = CombineRgn(rgn, rgn, rgn2, (int)mode);
        DeleteObject(rgn2);
        return res;
    }

    int CombineRgnWithRgn(HRGN dest, HRGN rgn1, HRGN rgn2, RegionCombineModes mode)
    {
        return CombineRgn(dest, rgn1, rgn2, (int)mode);
    }

    int CombineRgnWithRect(HRGN dst, HRGN rgn, const Rect &r, RegionCombineModes mode)
    {
        HRGN rgn2 = CreateRectRgnIndirect(&r);
        int res = CombineRgn(dst, rgn, rgn2, (int)mode);
        DeleteObject(rgn2);
        return res;
    }

    int CombineRectWithRgn(HRGN dst, const Rect &r, HRGN rgn, RegionCombineModes mode)
    {
        HRGN rgn2 = CreateRectRgnIndirect(&r);
        int res = CombineRgn(dst, rgn2, rgn, (int)mode);
        DeleteObject(rgn2);
        return res;
    }

    int OffsetRgn(HRGN rgn, const Point &pt)
    {
        return OffsetRgn(rgn, pt.x, pt.y);
    }

    HRGN CreateRgnCopy(HRGN orig)
    {
        HRGN rgn = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(rgn, orig, rgn, RGN_COPY);
        return rgn;
    }

    HRGN CreateRectRgnIndirect(const Rect &r)
    {
        return ::CreateRectRgnIndirect(&r);
    }

    bool RgnIntersectsRgn(HRGN rgn1, HRGN rgn2)
    {
        HRGN test = CreateRectRgn(0, 0, 0, 0);
        int res = CombineRgn(test, rgn1, rgn2, RGN_AND);
        DeleteObject(test);
        return res != NULLREGION && res != ERROR;
    }

    bool RgnIntersectsRect(HRGN rgn, const Rect &r)
    {
        HRGN test = CreateRectRgnIndirect(&r);
        int res = CombineRgn(test, test, rgn, RGN_AND);
        DeleteObject(test);
        return res != NULLREGION && res != ERROR;
    }


    //---------------------------------------------


    Size BitmapSize(HBITMAP hbmp)
    {
        if (!hbmp)
            return Size(-1, -1);

        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        HDC dc = GetDC(0);
        int result = GetDIBits(dc, hbmp, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
        if (result == FALSE || result == ERROR_INVALID_PARAMETER)
        {
            if (dc)
                ReleaseDC(0, dc);
            return Size(-1, -1);
        }
        if (dc)
            ReleaseDC(0, dc);

        return Size(bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight);
    }


    //---------------------------------------------

    static HKL kbdlayout = 0;
    static KBDTABLES *kbdtables = NULL;
    static KBDTABLES64 *kbdtables64 = NULL;

    void LoadKeyboardLayoutData()
    {
        HKL currentlayout = GetKeyboardLayout(0);
        if (kbdlayout != currentlayout)
        {
            wchar_t layoutname[KL_NAMELENGTH + 1];
            GetKeyboardLayoutName(layoutname);
            kbdlayout = currentlayout;

            Registry reg;
            if (!reg.OpenKey(HKEY_LOCAL_MACHINE, std::wstring(L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\") + layoutname, rraValue, rwaNone))
                return;
            std::wstring dll = reg.ReadString(L"Layout File");
            reg.CloseKey();

            if (dll.empty())
                return;

            HMODULE kbrddll = LoadLibrary(dll.c_str());
            if (kbrddll == NULL)
                return;

            if (WindowsIs64Bit)
            {
                LayerDescriptor64 KbdLayerDescriptor = (LayerDescriptor64)GetProcAddress(kbrddll, "KbdLayerDescriptor");
                if (KbdLayerDescriptor != NULL)
                    kbdtables64 = (KBDTABLES64*)KbdLayerDescriptor();
                else
                    kbdtables64 = NULL;
            }
            else
            {
                LayerDescriptor KbdLayerDescriptor = (LayerDescriptor)GetProcAddress(kbrddll, "KbdLayerDescriptor");
                if (KbdLayerDescriptor != NULL)
                    kbdtables = KbdLayerDescriptor();
                else
                    kbdtables = NULL;
            }
        }
    }

    bool KeyboardWithAltGr()
    {
        LoadKeyboardLayoutData();
        if (WindowsIs64Bit)
            return kbdtables64 != NULL && (kbdtables64->fLocaleFlags & 0x01) == 0x01;
        else
            return kbdtables != NULL && (kbdtables->fLocaleFlags & 0x01) == 0x01;
    }


    //---------------------------------------------

}
/* End of NLIBNS */


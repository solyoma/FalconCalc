#pragma once

#include "wexception.h"


#define N_PUBLIC     public
#define N_PROTECTED  protected
#define N_PRIVATE    private

// Empty definitions to help identifying parts in the code to be updated when saving the cpp and h files.
#ifdef DESIGNING
#define N_INITIALIZATION_START
#define N_INITIALIZATION_END

#ifndef IDI_MAINICON
#define IDI_MAINICON      107
#endif
#endif

#define GET_X_LPARAM(p) ((int)((short)LOWORD(p)))
#define GET_Y_LPARAM(p) ((int)((short)HIWORD(p)))


// used in SetWindowPos calls
#define SWP_BOUNDS          (SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE)
#define SWP_SIZEONLY        (SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE)
#define SWP_POSITIONONLY    (SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE)
#define SWP_ZORDERONLY      (SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE)
#define SWP_NOPOSITION      (SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE)
#define SWP_FRAMEONLY       (SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED)



#ifdef DESIGNING
namespace std
{
    template<typename chartype>
    struct base_streamformat
    {
        std::ios_base::fmtflags flags;
        chartype fillchar;
        int fillwidth;
    };
    typedef base_streamformat<char> streamformat;
    typedef base_streamformat<wchar_t> wstreamformat;

    streamformat streamstate(const basic_ios<char> &stream);
    wstreamformat streamstate(const basic_ios<wchar_t> &stream);

    void streamstate(basic_ios<char> &stream, const streamformat &format);
    void streamstate(basic_ios<wchar_t> &stream, const wstreamformat &format);

    // Tries reading a specific string and returns true if the string was found. If the string is not found and putback is true, the read characters are put back to the stream before returning false.
    bool tryget(basic_istream<char> &stream, const char *str, bool putback = true);
    bool tryget(basic_istream<wchar_t> &stream, const wchar_t *str, bool putback = true);
    bool tryget(basic_istream<char> &stream, const char *str, int slen, bool putback = true);
    bool tryget(basic_istream<wchar_t> &stream, const wchar_t *str, int slen, bool putback = true);

    void putback(basic_istream<char> &stream, const string &str);
    void putback(basic_istream<wchar_t> &stream, const wstring &str);
    void putback(basic_istream<char> &stream, const char *str, int slen);
    void putback(basic_istream<wchar_t> &stream, const wchar_t *str, int slen);
};
#endif


namespace NLIBNS
{

    // Returns a value of an enum type, which is between min and max. If val is between min and max, it is returned, otherwise default is returned.
    template<typename EnumType>
    EnumType FixEnumValue(EnumType val, EnumType min, EnumType max, EnumType default)
    {
        if ((unsigned int)min > (unsigned int)max) // Signed enum type, where min was negative.
        {
            if ((int)val < (int)min || (int)val > (int)max)
                return default;
        }
        else
        {
            if ((unsigned int)val < (unsigned int)min || (unsigned int)val > (unsigned int)max)
                return default;
        }

        return val;
    }

    //namespace std = ::std;

    typedef LRESULT (CALLBACK *PWndProc)(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    enum ModalResults : int {
            mrNone = 0, mrOk = 1, mrCancel = 2, mrAbort = 3, mrRetry = 4, mrIgnore = 5,
            mrYes = 6, mrNo = 7, mrTryAgain = 10, mrContinue = 11, mrDefault = 99
    };

    class Application;
    class Screen;

    template<typename T>
    class uintset
    {
        typedef uintset<T>  selftype;
        UINT data;
    public:
        uintset() : data(0)
        {
        }

        uintset(UINT data) : data(data)
        {
        }

        uintset(const selftype &other)
        {
            *this = other;
        }

        uintset(selftype &&other) noexcept
        {
            *this = std::move(other);
        }

        selftype& operator=(const selftype &other) noexcept
        {
            data = other.data;
            return *this;
        }

        selftype& operator=(selftype &&other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        bool operator!=(const selftype &other) const
        {
            return data != other.data;
        }

        bool operator!() const
        {
            return data == 0;
        }

        bool empty() const
        {
            return data == 0;
        }

        bool operator<(const selftype &set2) const // Has some elments from set2 and no others, but does not equal to set2.
        {
            return (data & set2.data) == data && data != set2.data;
        }

        bool operator<=(const selftype &set2) const // Has some elments from set2 or equals to set2.
        {
            return (data & set2.data) == data;
        }

        bool operator>(const selftype &set2) const // Has all elements of set2 and additional elements.
        {
            return (data & set2.data) == set2.data && data != set2.data;
        }

        bool operator>=(const selftype &set2) const // Has all elments of set2 or more.
        {
            return (data & set2.data) == set2.data;
        }

        selftype& operator<<(T val)
        {
            data |= val;
            return *this;
        }

        selftype& operator<<(const selftype &set2)
        {
            data |= set2.data;
            return *this;
        }

        //selftype operator<<(T val) const
        //{
        //    return data | val;
        //}

        //selftype operator<<(const selftype &set2) const
        //{
        //    return data | set2.data;
        //}

        selftype& operator|=(const selftype &set2)
        {
            data |= set2.data;
            return *this;
        }

        selftype& operator&=(const selftype &set2)
        {
            data &= set2.data;
            return *this;
        }

        selftype& operator|=(T val)
        {
            data |= val;
            return *this;
        }

        selftype& operator&=(T val)
        {
            data &= val;
            return *this;
        }

        selftype operator|(const selftype &set2) const
        {
            return data | set2.data;
        }

        selftype operator&(const selftype &set2) const
        {
            return data & set2.data;
        }

        selftype operator|(T val) const
        {
            return data | val;
        }

        selftype operator&(T val) const
        {
            return data & val;
        }

        selftype& operator-=(T val)
        {
            data &= ~val;
            return *this;
        }

        selftype& operator-=(const selftype &set2)
        {
            data &= ~set2.data;
            return *this;
        }

        selftype operator-(T val) const
        {
            selftype tmp = *this;
            tmp.data &= ~val;
            return tmp;
        }

        selftype operator-(const selftype &set2) const
        {
            selftype tmp = *this;
            tmp.data &= ~set2.data;
            return tmp;
        }

        selftype& operator+=(T val)
        {
            data |= val;
            return *this;
        }

        selftype& operator+=(const selftype &set2)
        {
            data |= set2.data;
            return *this;
        }

        selftype operator+(T val) const
        {
            selftype tmp = *this;
            tmp.data |= val;
            return tmp;
        }

        selftype operator+(const selftype &set2) const
        {
            selftype tmp = *this;
            tmp.data |= set2.data;
            return tmp;
        }

        bool contains(T val) const
        {
            if (val == 0)
                return data == 0;
            return (data & val) == (UINT)val;
        }

        operator UINT() const
        {
            return data;
        }
    };

    bool SetContains(UINT uset, UINT val); // Returns ((uset & val) == val).

    template<class FRIEND, typename TYPE>
    class ConstValue
    {
    private:
        TYPE value;

        ConstValue(const ConstValue &copy) { }

        ConstValue<FRIEND, TYPE>& operator=(TYPE newval) { value = newval; return *this; }

        friend FRIEND;
    public:
        ConstValue() { }
        operator const TYPE&() { return value; }
    };

    template<class FRIEND, typename TYPE>
    class ConstValue<FRIEND, TYPE*>
    {
    private:
        typedef TYPE* PTYPE;
        PTYPE value;

        ConstValue(const ConstValue &copy) { }

        ConstValue<FRIEND, PTYPE>& operator=(PTYPE newval) { value = newval; return *this; }

        friend FRIEND;
    public:
        ConstValue() { }
        operator PTYPE() { return value; }
        PTYPE operator-> () { return value; }
    };

    template<class FRIEND, typename TYPE>
    class ConstVector : private std::vector<TYPE>
    {
    public:
        typedef typename std::vector<TYPE>::const_iterator           const_iterator;
        typedef typename std::vector<TYPE>::const_reverse_iterator   const_reverse_iterator;
    private:
        typedef std::vector<TYPE>   base;
        friend FRIEND;
    public:
        ConstVector() {}

        const_iterator begin() const { return base::begin(); }
        const_iterator end() const { return base::end(); }
        const_reverse_iterator rbegin() const { return base::rbegin(); }
        const_reverse_iterator rend() const { return base::rend(); }
        const TYPE& operator[](int n) const { return base::operator[](n); }
        const TYPE& at(int n ) const { return base::at(n); }
        const TYPE& front() const { return base::front(); }
        const TYPE& back() const { return base::back(); }
        using base::size;
        using base::empty;
    };

    template<class FRIEND>
    class ConstWString : private std::wstring
    {
    public:
        typedef std::wstring::const_iterator    const_iterator;
        typedef std::wstring::const_reverse_iterator    const_reverse_iterator;
    private:
        typedef ConstWString<FRIEND>    selftype;
        typedef std::wstring    base;
        friend FRIEND;

        selftype& operator=(const wchar_t *other) { base::operator=(other); return *this; }
        selftype& operator=(const std::wstring &other) { base::operator=(other); return *this; }
        selftype& operator=(const selftype &other) { base::operator=((std::wstring)other); return *this; }
    public:
        ConstWString() {}
        const wchar_t& operator[](int index) const { return base::operator[](index); }
        const wchar_t& at(int index) const { return base::at(index); }
        std::wstring operator+(const wchar_t *other) const { return *(std::wstring*)this + other; }
        std::wstring operator+(const std::wstring& other) const { return *(std::wstring*)this + other; }
        operator std::wstring() const { return *(std::wstring*)this + L""; }
        operator const std::wstring&() const { return *this; }
        const_iterator begin() const { return base::begin(); }
        const_iterator end() const { return base::end(); }
        const_reverse_iterator rbegin() const { return base::rbegin(); }
        const_reverse_iterator rend() const { return base::rend(); }
        using base::length;
        using base::copy;
        using base::substr;
        using base::empty;
        using base::c_str;
        using base::find;
        using base::rfind;
        using base::find_first_of;
        using base::find_last_of;
        using base::find_first_not_of;
        using base::find_last_not_of;
        using base::compare;
    };

    struct Size : public SIZE
    {
    private:
        typedef SIZE base;
    public:
        inline Size()
        {
            base::cx = 0;
            base::cy = 0;
        }

        Size(LONG cx, LONG cy);
        Size(const SIZE &orig);
        Size& operator=(const SIZE &s);
    };

    struct Point;
    struct RectF;
    struct Rect : public RECT
    {
    private:
        typedef RECT base;
    public:
        inline Rect()
        {
            base::left = 0;
            base::top = 0;
            base::right = 0;
            base::bottom = 0;
        }

        Rect(LONG left, LONG top, LONG right, LONG bottom);

        Rect(const RECT &orig);
        Rect& operator=(const RECT &r);

        explicit Rect(const RectF &orig);
        Rect& operator=(const RectF& r);

        explicit Rect(const Gdiplus::Rect &orig);
        Rect& operator=(const Gdiplus::Rect &r);

        explicit Rect(const Gdiplus::RectF &orig);
        Rect& operator=(const Gdiplus::RectF &r);

        operator RECT() const;

        Rect(const Point &topleft, const Point &bottomright);

        Rect Offset(const Point &pt) const; // Returns a new rectangle which has its sides offset by x and y.
        Rect Offset(int x, int y) const; // Returns a new rectangle which has its sides offset by x and y.
        Rect Offset(int dleft, int dtop, int dright, int dbottom) const; // Returns a new rectangle which has its sides offset by the specified values.
        Rect& Move(const Point &pt); // Moves this rectangle by x and y and returns the rectangle.
        Rect& Move(int x, int y); // Moves this rectangle by x and y and returns the rectangle.
        Rect& Move(int dleft, int dtop, int dright, int dbottom); // Moves the sides of the rectangle by the arguments and returns the rectangle.
        Rect Intersect(const Rect &other) const; // Returns a rectangle with the area where the two rectangles intersect.
        Rect& Boolean(const Rect &other); // Changes the rectangle to contain the two rectangles' intersection.
        bool DoesIntersect(const Rect &other) const; // Returns whether there are points that lie inside both rectangles.

        Rect Inflate(int val) const; // Creates a new rectangle from this one with its sides increased by val on each side. Val can be negative.
        Rect Inflate(int xval, int yval) const; // Creates a new rectangle from this one with its sides increased by xval and yval. Vals can be negative.
        Rect Inflate(int dleft, int dtop, int dright, int dbottom) const; // Creates a new rectangle from this one with its sides increased by the arguments. If any are negative, those sizes of the rectangle are moved inside.
        Rect& Expand(int val); // Increases the size of the rectangle by val on each side. Val can be negative.
        Rect& Expand(int xval, int yval); // Increases the size of the rectangle by xval and yval. Val can be negative.
        Rect& Expand(int dleft, int dtop, int dright, int dbottom); // Expands the sizes of the rectangle by the arguments. If any are negative, those sizes of the rectangle are moved inside.

        LONG Width() const;
        LONG Height() const;
        bool Empty() const;

        Point TopLeft() const;
        Point BottomRight() const;

        bool Contains(int x, int y) const;
        bool Contains(Point pt) const;

        operator Gdiplus::Rect() const;
    };

    struct PointF;
    struct RectF
    {
    public:
        float left;
        float top;
        float right;
        float bottom;

        float Width() const;
        float Height() const;
        bool Empty() const;

        inline RectF() : left(0.F), top(0.F), right(0.F), bottom(0.F)
        {
        }

        RectF(float left, float top, float right, float bottom);

        RectF(const Rect &orig);
        RectF& operator=(const Rect &r);
        RectF(const RECT &orig);
        RectF& operator=(const RECT &r);
        RectF(const Gdiplus::RectF &orig);
        RectF& operator=(const Gdiplus::RectF &r);

        RectF Offset(const PointF &pt) const; // Returns a new rectangle which has its sides offset by x and y.
        RectF Offset(float x, float y) const; // Returns a new rectangle which has its sides offset by x and y.
        RectF Offset(float dleft, float dtop, float dright, float dbottom) const; // Returns a new rectangle which has its sides offset by the specified values.
        RectF& Move(float x, float y); // Moves this rectangle by x and y and returns the rectangle.
        RectF& Move(const PointF &pt); // Moves this rectangle by x and y and returns the rectangle.
        RectF Intersect(const RectF &other) const; // Returns a rectangle with the area where the two rectangles intersect.
        bool DoesIntersect(const RectF &other) const; // Returns whether there are points that lie inside both rectangles.

        RectF Inflate(float val) const; // Creates a new rectangle from this one with its sides increased by val on each side. Val can be negative.
        RectF Inflate(float xval, float yval) const; // Creates a new rectangle from this one with its sides increased by xval and yval. Vals can be negative.
        RectF& Expand(float val); // Increases the size of the rectangle by val on each side. Val can be negative.
        RectF& Expand(float xval, float yval); // Increases the size of the rectangle by xval and yval. Val can be negative.

        PointF TopLeft() const;
        PointF BottomRight() const;

        bool Contains(float x, float y) const;
        bool Contains(PointF pt) const;

        operator Rect() const;
        operator Gdiplus::RectF() const;
    };

    struct Point : public POINT
    {
    private:
        typedef POINT base;
    public:
        inline Point()
        {
            base::x = 0;
            base::y = 0;
        }
        Point(LONG x, LONG y);
        Point(const POINT &p);
        Point(const POINTS &p);

        Point& Move(int deltax, int deltay);
        Point& Move(const Point &pt);
        Point Offset(int deltax, int deltay) const;
        Point Offset(const Point &pt) const;

        Point operator-(const Point &other) const;
        Point operator+(const Point &other) const;
        Point& operator-=(const Point &other);
        Point& operator+=(const Point &other);

        void Get(int &x, int &y) const; // Returns the x and y coordinates to avoid creating just another point when separate variables are already present.

        operator Gdiplus::Point();
    };

    struct PointF
    {
        float x;
        float y;

        PointF();
        PointF(float x, float y);
        PointF(const Point &p);
        PointF(const POINT &p);
        PointF(const Gdiplus::PointF &p);
        PointF& operator=(const PointF &p);
        PointF& operator=(const Point &p);
        PointF& operator=(const POINT &p);
        PointF& operator=(const Gdiplus::PointF &p);

        PointF& Move(float deltax, float deltay);
        PointF& Move(const PointF &pt);
        PointF Offset(float deltax, float deltay) const;
        PointF Offset(const PointF &pt) const;

        PointF operator-(const PointF &other) const;
        PointF operator+(const PointF &other) const;
        PointF& operator-=(const PointF &other);
        PointF& operator+=(const PointF &other);

        void Get(float &x, float &y) const; // Returns the x and y coordinates to avoid creating just another point when separate variables are already present.

        operator Gdiplus::PointF();
    };

    struct ColorMatrix : public Gdiplus::ColorMatrix
    {
        ColorMatrix();
        ColorMatrix(const Gdiplus::ColorMatrix &other);
        ColorMatrix(Gdiplus::REAL data[5][5]);
        ColorMatrix& operator=(const Gdiplus::ColorMatrix &other);
        ColorMatrix operator*(const Gdiplus::ColorMatrix &other) const;
        ColorMatrix operator+(const Gdiplus::ColorMatrix &other) const;
        ColorMatrix& operator*=(const Gdiplus::ColorMatrix &other);
        ColorMatrix& operator+=(const Gdiplus::ColorMatrix &other);
        ColorMatrix& operator=(const ColorMatrix &other);
        ColorMatrix operator*(const ColorMatrix &other) const;
        ColorMatrix operator+(const ColorMatrix &other) const;
        ColorMatrix& operator*=(const ColorMatrix &other);
        ColorMatrix& operator+=(const ColorMatrix &other);
        operator Gdiplus::ColorMatrix() const;
        operator Gdiplus::ColorMatrix&();

        ColorMatrix& TransformAlpha(float transform); /* Multiplies the alpha blending modifier value by the value in transform. */
        ColorMatrix& SetAlpha(float newalpha); /* Sets the alpha blending value in this matrix. */
        ColorMatrix& Grayscale(); /* Creates a grayscale converter matrix without touching the alpha value. */
        ColorMatrix& Darken(float transform); /* Decreases brightness. */
        ColorMatrix& Brighten(float transform); /* Increases brigthness. */
        ColorMatrix& Saturate(float transform); /* Increases the color saturation. 1 makes all colors as saturated as possible. */
        ColorMatrix& Desaturate(float transform); /* Decreases the color saturation. 1 makes all colors gray. */
        ColorMatrix& SetContrast(float newcontrast); /* Modifies the image to give it a different contrast. */
    };

    struct Matrix : public Gdiplus::Matrix
    {
        Matrix();
        Matrix(const Matrix &other);
        Matrix(const Gdiplus::Matrix &other);

        Matrix& SetElements(Gdiplus::REAL m11, Gdiplus::REAL m12, Gdiplus::REAL m21, Gdiplus::REAL m22, Gdiplus::REAL dx, Gdiplus::REAL dy);
        void GetElements(Gdiplus::REAL &m11, Gdiplus::REAL &m12, Gdiplus::REAL &m21, Gdiplus::REAL &m22, Gdiplus::REAL &dx, Gdiplus::REAL &dy);
        Matrix& operator=(const Gdiplus::Matrix &other);
        Matrix& operator=(const Matrix &other);
        operator Gdiplus::Matrix&();
        bool operator==(const Matrix &other);
        bool operator==(const Gdiplus::Matrix &other);
        Matrix& Multiply(const Matrix &matrix, bool prepend);
        Matrix& Multiply(const Gdiplus::Matrix &matrix, bool prepend);
        Matrix& Rotate(Gdiplus::REAL angle, bool prepend);
        Matrix& RotateAt(Gdiplus::REAL angle, PointF center, bool prepend);
        Matrix& Scale(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend);
        Matrix& Translate(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend);
        Matrix& Shear(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend);
    };

    extern const Gdiplus::ColorMatrix identitymatrix;
    extern const Gdiplus::ColorMatrix alphamatrix;
    extern const Gdiplus::ColorMatrix grayscalematrix;

    Rect RectS(LONG left, LONG top, LONG width, LONG height);
    RectF RectFS(float left, float top, float width, float height);
    Rect RectS(Point topleft, LONG width, LONG height);
    RectF RectFS(PointF topleft, float width, float height);

    bool operator==(const Rect &a, const Rect &b);
    bool operator!=(const Rect &a, const Rect &b);
    bool operator==(const RectF &a, const RectF &b);
    bool operator!=(const RectF &a, const RectF &b);

    bool operator==(const RECT &a, const RECT &b);
    bool operator!=(const RECT &a, const RECT &b);

    bool operator==(const Size &a, const Size &b);
    bool operator!=(const Size &a, const Size &b);
    bool operator==(const SIZE &a, const SIZE &b);
    bool operator!=(const SIZE &a, const SIZE &b);

    bool operator==(const Point &a, const Point &b);
    bool operator!=(const Point &a, const Point &b);
    bool operator==(const PointF &a, const PointF &b);
    bool operator!=(const PointF &a, const PointF &b);
    bool operator==(const POINT &a, const POINT &b);
    bool operator!=(const POINT &a, const POINT &b);

    Size BaseToPixel(float basewidth, float baseheight);


    enum Colors {
        clNone = -1 /* When converted to aRGB, this returns Color(0, 0, 0, 0) */,
        clScrollbar = 0, clBackground = 1, /*clDesktop = 1, */clActiveCaption = 2, clInactiveCaption = 3, clMenu = 4,
        clWindow = 5, clWindowFrame = 6, clMenuText = 7, clWindowText = 8, clCaptionText = 9, clActiveBorder = 10,
        clInactiveBorder = 11, clAppWorkspace = 12, clHighlight = 13, clHighlightText = 14, /*cl3DFace = 15, */clBtnFace = 15,
        cl3DShadow = 16, /*clBtnShadow = 16, */clGrayText = 17, clBtnText = 18, clInactiveCaptionText = 19, /*clBtnHighlight = 20,*/
        cl3DHighlight = 20, cl3DDkShadow = 21, cl3DLight = 22, clInfoText = 23, clInfoBK = 24, clHotlight = 26,
        clGradientActiveCaption = 27, clGradientInactiveCaption = 28, clMenuHilight = 29, clMenubar = 30,
        clBlack = 31, clWhite = 32, clRed = 33, clGreen = 34, clBlue = 35, clYellow = 36, clFuchsia = 37, clCyan = 38,
        clGray = 39, clSilver = 40, clLime = 41, clMaroon = 42, clNavy = 43, clOlive = 44, clTeal = 45, clPurple = 46, clArraySize = 47,
#ifdef DESIGNING
        clCount = 48
#endif
    };

    class Color
    {
        union
        {
            Colors colval;

            /* For non-enum colors: */
            DWORD color;
            struct {
                byte B;
                byte G;
                byte R;
                byte A;
            };
        } data;
        bool indexed;

        void ConvertToRGB(); // Converts color to RGB if it was indexed.
    public:
        Color();
        Color(const COLORREF &color);
        Color(const RGBQUAD &color);
        Color(Gdiplus::Color color);
        Color(const Color &other);
        Color(byte A, byte R, byte G, byte B);
        Color(byte R, byte G, byte B);
        Color(Colors col);

        static Color colors[clArraySize];
        Color Mix(Color c, double ratio = 0.5) const; // Mix this with another color using the formula (ratio * *this + (1 - ratio) * c).

        operator COLORREF() const;
        operator Gdiplus::Color() const;
        operator RGBQUAD() const;

        DWORD ToDWORD() const;

        bool EnumIndexed() const; // If instead of having an R G B, the color uses an index from the Colors enum, this returns true.
        Colors EnumValue() const; // Returns the enum value of this color if it is indexed. Otherwise raises an exception.
        byte R() const; // Returns the red component of the color, but only if it is not indexed. Otherwise convert either to COLORREF or Gdiplus::Color to get the R from the enum.
        byte G() const; // Returns the green component of the color, but only if it is not indexed. Otherwise convert either to COLORREF or Gdiplus::Color to get the G from the enum.
        byte B() const; // Returns the blue component of the color, but only if it is not indexed. Otherwise convert either to COLORREF or Gdiplus::Color to get the B from the enum.
        byte A() const; // Returns the alpha component of the color, but only if it is not indexed. Otherwise convert to Gdiplus::Color to get the A from the enum.
        Color ToRGB() const; // If the color uses an enum value instead of RGB, it returns a new color converted to the RGB equivalent.
        Color Solid() const; // Creates a color which has the same RGB values, but its alpha is set to 255.

        Color& SetR(byte newR); // Changes the red component to the new value and returns the color. If the color was indexed, it is first converted to RGB.
        Color& SetG(byte newG); // Changes the green component to the new value and returns the color. If the color was indexed, it is first converted to RGB.
        Color& SetB(byte newB); // Changes the blue component to the new value and returns the color. If the color was indexed, it is first converted to RGB.
        Color& SetA(byte newA); // Changes the alpha component to the new value and returns the color. If the color was indexed, it is first converted to RGB.

        bool operator==(const Color &other) const; // Two colors are equal if they are either both having the same Colors enum value, or the same RGB. If one value is from Colors while the other has the equivalent RGB, this returns false.
        bool operator==(const COLORREF &other) const;
        bool operator==(const RGBQUAD &other) const;
        bool operator==(const Gdiplus::Color &other) const;
        bool operator==(const Colors &col) const;
        bool operator!=(const Color &other) const; // Two colors are equal if they are either both having the same Colors enum value, or the same RGB. If one value is from Colors while the other has the equivalent RGB, this returns true.
        bool operator!=(const COLORREF &other) const;
        bool operator!=(const RGBQUAD &other) const;
        bool operator!=(const Gdiplus::Color &other) const;
        bool operator!=(const Colors &col) const;
        Color& operator=(const Color &other);
        Color& operator=(const COLORREF &other);
        Color& operator=(const RGBQUAD &other);
        Color& operator=(const Gdiplus::Color &other);
        Color& operator=(const Colors &col);
    };

    class HSVColor
    {
    public:
        HSVColor();
        HSVColor(float h, float s, float v);
        HSVColor(byte a, float h, float s, float v);
        HSVColor(const HSVColor &other);
        HSVColor(const Color &other);
        HSVColor(Gdiplus::Color color);
        HSVColor(Colors col);

        operator Color() const;
        bool operator==(const HSVColor &other) const;
        bool operator==(const Gdiplus::Color &other) const;
        bool operator==(const Colors &col) const;
        bool operator!=(const HSVColor &other) const;
        bool operator!=(const Gdiplus::Color &other) const;
        bool operator!=(const Colors &col) const;
        HSVColor& operator=(const Color &other);
        HSVColor& operator=(const HSVColor &other);
        HSVColor& operator=(const Gdiplus::Color &other);
        HSVColor& operator=(const Colors &col);

        byte A() const; // Value between 0 and 255.
        float H() const; // Value between 0 and 1.
        float S() const; // Value between 0 and 1.
        float V() const; // Value between 0 and 1.

        void SetA(byte newa);
        void SetH(float newh);
        void SetS(float news);
        void SetV(float newv);

        HSVColor Mix(const HSVColor &other, double ratio = 0.5) const;
    private:
        byte a;
        float h;
        float s;
        float v;
    };


#ifdef DESIGNING
    std::ostream& operator<<(std::ostream& ostr, const Color &c);
    std::wostream& operator<<(std::wostream& ostr, const Color &c);
    std::istream& operator>>(std::istream& istr, Color &c);
    std::wistream& operator>>(std::wistream& istr, Color &c);
#endif

    bool operator==(const COLORREF &other, const Color &color);
    bool operator==(const Gdiplus::Color &other, const Color &color);
    bool operator==(const Colors &col, const Color &color);
    bool operator!=(const COLORREF &other, const Color &color);
    bool operator!=(const Gdiplus::Color &other, const Color &color);
    bool operator!=(const Colors &col, const Color &color);
    Color MixColors(Color a, Color b, double ratio = 0.5); // Mix two colors using the formula (ratio * a + (1 - ratio) * b).


    enum VirtualKeyStates : int {
            vksLeft = 1,
            vksMiddle = 2,
            vksRight = 4,
            vksShift = 8,
            vksCtrl = 16,
            vksAlt = 32,
            vksDouble = 64
    };
    typedef uintset<VirtualKeyStates> VirtualKeyStateSet;

    enum MouseButtons : int {
            mbNone = 0,
            mbLeft = 1,
            mbMiddle = 2,
            mbRight = 3,
            mbError = 4
    };

    UINT VirtualKeysFromWParam(WPARAM wParam);
    WPARAM WParamFromVirtualKeys(VirtualKeyStateSet vkeys);
    VirtualKeyStateSet PressedVirtualKeys();
    MouseButtons ButtonFromMsg(UINT uMsg);
    MouseButtons NCButtonFromMsg(UINT uMsg);

    UINT GenerateCommandId(); // Returns a unique command identifier, which is currently not used in the interface. Always returns a number greater than 0. The numbers come in increasing order. Use this function if you need a new unique command id, or specify an id with a large value.

    enum CtrlKeys { ckA = 1, ckB, ckC, ckD, ckE, ckF, ckG, ckH, ckI, ckJ, ckK, ckL, ckM, ckN, ckO, ckP, ckQ, ckR, ckS, ckT, ckU, ckV, ckW, ckX, ckY, ckZ };

    struct MessageStruct
    {
        HWND hwnd;
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
        LRESULT result;
    };


    void GenLower(std::wstring &string); // Converts the wstring with an invariant culture to lower case.
    void GenUpper(std::wstring &string); // Converts the wstring with an invariant culture to upper case.
    std::wstring GenToLower(const std::wstring &string); // Converts the wstring with an invariant culture to lower case.
    std::wstring GenToUpper(const std::wstring &string); // Converts the wstring with an invariant culture to upper case.

    void splitstring(const std::wstring &str, const std::wstring &delim, std::vector<std::wstring> &split, bool skipempty = true);
    void splitstring(const std::wstring &str, const std::wstring &delim, std::list<std::wstring> &split, bool skipempty = true);
    void splitstring(const std::wstring &str, wchar_t delim, std::vector<std::wstring> &split, bool skipempty = true);
    void splitstring(const std::wstring &str, wchar_t delim, std::list<std::wstring> &split, bool skipempty = true);
    bool StrToInt(const std::wstring &str, int &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos); // Converts string to int, starting from the character specified in pos, using at most len characters. Returns true if the string is not empty and all characters are used to create a valid number. If the returned value is false, the result is not valid. The conversion does not use the current locale.
    bool StrToInt(const std::wstring &str, long &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos); // Converts string to long, starting from the character specified in pos, using at most len characters. Returns true if the string is not empty and all characters are used to create a valid number. If the returned value is false, the result is not valid. The conversion does not use the current locale.
    bool StrToInt(const std::wstring &str, unsigned int &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos); // Converts string to unsigned int, starting from the character specified in pos, using at most len characters. Returns true if the string is not empty and all characters are used to create a valid positive number. If the returned value is false, the result is not valid. The conversion does not use the current locale.
    bool StrToInt(const std::wstring &str, short &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos); // Converts string to short, starting from the character specified in pos, using at most len characters. Returns true if the string is not empty and all characters are used to create a valid positive number. If the returned value is false, the result is not valid. The conversion does not use the current locale.
    bool StrToInt(const std::wstring &str, unsigned short &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos); // Converts string to unsigned short, starting from the character specified in pos, using at most len characters. Returns true if the string is not empty and all characters are used to create a valid positive number. If the returned value is false, the result is not valid. The conversion does not use the current locale.
    bool StrToFloat(const std::wstring &str, double &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos);
    bool StrHexToInt(const std::wstring &str, int &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos);
    bool StrHexToInt(const std::wstring &str, long &result, std::wstring::size_type pos = 0, std::wstring::size_type len = std::wstring::npos);
    std::wstring IntToStr(int val); // Converts an integer to string. The conversion does not use the current locale.
    std::wstring FloatToStr(double val); // Converts a float to string. The conversion does not use the current locale.
    std::wstring EscapeCString(const std::wstring &str);

    extern const wchar_t *_trim_characters; // Newline, space and other whitespace characters removed in Trim() by default.
    std::wstring trim(std::wstring str, bool fromstart = true, bool fromend = true); // Removes spaces, tab and newline characters from the beginning and end of the string.
    std::wstring trim(std::wstring str, const wchar_t *trim_characters, bool fromstart = true, bool fromend = true); // Removes all characters found in the trim_characters argument from the beginning and end of the string (as specified).

#ifndef __MINGW32__
    int round(double d);
#endif

    std::wstring ShortcutToStr(WORD shortcut); // Converts shortcut value to string that can be shown in menus. Shortcuts are saved as a number that can be converted to a key combination. Passing an invalid value returns the recognizable part only.
    bool StrToShortcut(const std::wstring &str, WORD &result); // Converts a string shortcut to an integer value. Returns true if the conversion was successful.

#ifdef DESIGNING
    // Encode character strings to hex string representation. Passing a stream object is more efficient if the stream can be used in other operations.
    void HexEncode(const std::wstring &str, std::ostream &out);
    std::string HexEncode(const std::wstring &str);
    void HexEncode(const std::string &str, std::ostream &out);
    void HexEncode(const void *str, unsigned int bytelen, std::ostream &out);
    std::string HexEncode(const std::string &str);
    std::string HexEncode(const void *str, unsigned int bytelen);

    void HexEncodeW(const std::wstring &str, std::wostream &out);
    std::wstring HexEncodeW(const std::wstring &str);
    void HexEncodeW(const std::string &str, std::wostream &out);
    void HexEncodeW(const void *str, unsigned int bytelen, std::wostream &out);
    std::wstring HexEncodeW(const std::string &str);
    std::wstring HexEncodeW(const void *str, unsigned int bytelen);

    bool HexDecodeChar(const wchar_t str[2], byte &ch); // Returns a single byte representation of a hexa string of 2 characters.
    bool HexDecodeChar(const char str[2], byte &ch); // Returns a single byte representation of a hexa string of 2 characters.

    bool HexDecodeStringToken(const wchar_t *src, unsigned int &srclen, wchar_t delimiter, std::wostream &dest); // Decodes an array of [0-9a-fA-F] characters into bytes, then two bytes into wchar_t, which is finally added at the end of dest, until the delimiter or an invalid character is found. The delimiter character is not added. The array contains the delimiter character in hexa code, but the passed delimiter is the character after decoding. strlen is changed to the number of passed characters in the array (delimiter included). If the function encounters an invalid character, it returns false.
    bool HexDecodeStringToken(const char *src, unsigned int &srclen, char delimiter, std::wostream &dest); // Decodes an array of [0-9a-fA-F] characters into bytes, then two bytes into char, which is finally added at the end of dest, until the delimiter or an invalid character is found. The delimiter character is not added. The array contains the delimiter character in hexa code, but the passed delimiter is the character after decoding. strlen is changed to the number of passed characters in the array (delimiter included). If the function encounters an invalid character, it returns false.
    bool HexDecodeStringToken(const wchar_t *src, unsigned int &srclen, wchar_t delimiter, std::ostream &dest); // Decodes an array of [0-9a-fA-F] characters into bytes, then two bytes into wchar_t, which is finally added at the end of dest, until the delimiter or an invalid character is found. The delimiter character is not added. The array contains the delimiter character in hexa code, but the passed delimiter is the character after decoding. strlen is changed to the number of passed characters in the array (delimiter included). If the function encounters an invalid character, it returns false.
    bool HexDecodeStringToken(const char *src, unsigned int &srclen, char delimiter, std::ostream &dest); // Decodes an array of [0-9a-fA-F] characters into bytes, then two bytes into char, which is finally added at the end of dest, until the delimiter or an invalid character is found. The delimiter character is not added. The array contains the delimiter character in hexa code, but the passed delimiter is the character after decoding. strlen is changed to the number of passed characters in the array (delimiter included). If the function encounters an invalid character, it returns false.

    bool HexDecodeBytes(const char *src, unsigned int &srclen, byte *dest, unsigned int destlen); // Decodes an array of at most srclen length of [0-9a-fA-F] characters into an array of bytes. Changes srclen to the number of bytes read. If the function encounters an invalid character, it returns false.
    bool HexDecodeBytes(const wchar_t *src, unsigned int &srclen, byte *dest, unsigned int destlen); // Decodes an array of at most srclen length of [0-9a-fA-F] characters into an array of bytes. Changes srclen to the number of bytes read. If the function encounters an invalid character, it returns false.
#endif

    int PathLength(wchar_t *str, bool unixslash = false); /* Returns the number of characters up to and including the last backslash, or slash if unixslash is true. */
    int PathLength(const std::wstring &str, bool unixslash = false); /* Returns the number of characters up to and including the last backslash, or slash if unixslash is true. */
    int FindFileNameExtension(wchar_t *str, bool unixslash = false); /* Returns the position of the starting character of the file name extension, or 0 if the file name does not have an extension. Unixtype specifies whether the backslash or slash character is used as the path separator in the file name. */
    int FindFileNameExtension(const std::wstring &str, bool unixslash = false); /* Returns the position of the starting character of the file name extension, or 0 if the file name does not have an extension. Unixtype specifies whether the backslash or slash character is used as the path separator in the file name. */
    std::wstring GetFileName(const std::wstring &str, bool unixslash = false); /* Returns the file name portion of a full path and file name. */
    std::wstring GetFilePath(const std::wstring &str, bool unixslash = false); /* Returns the file path portion of a full path and file name. */
    std::wstring GetFileExtension(const std::wstring &str, bool unixslash = false); /* Returns the extension part of a full path and file name. */

    bool ValidVarName(const std::wstring &name);
    bool ValidFileName(const std::wstring &path, bool containspath); // Returns true if the passed string is a valid unicode file name when creating a file.
    bool ValidFilePath(const std::wstring &path); // Returns whether the passed string is a valid unicode file path under Windows.
    std::wstring ShortenRelativePath(const std::wstring &path, bool devicepath = false); // Removes unnecessary paths like .\, removes directory names before ..\ etc. Starting .\ and a series of starting ..\ are not removed. Be aware that if the path is like "C:\..\something", the drive will be removed so make sure the path is valid. The function does not touch paths starting with "\\?\". Device paths start with \\.\ but those are only handled if devicepath is set to true when calling this function. Setting devicepath for strings that do not start with \\.\ will have no effect.
    bool FileExists(const std::wstring &path); // Returns whether the file at the passed path exists and is a file not a folder.
    bool PathExists(std::wstring path); // Returns whether the passed path exists and it is a folder not a file.
    bool CreateFolder(std::wstring path, bool recursive); // Creates a folder at the specified location. If recursive is false and the path where the new folder should be created does not exists, the function will fail. Returns whether the call was successful. The call is successful even if the given folder already exists.
    std::wstring AppendToPath(const std::wstring &path, const std::wstring &file, bool shortenrelative = true); // Creates a full path and file name from the passed strings, adding a \ character to the path if necessary and shortening the path by resolving relative parts like .\ and ..\ if shortenrelative is true.

    // WARNING: If the functions returning character arrays return valid values, those must be freed with delete[] by the caller.
    char* ANSIToUTF8(const char *str, int &len);
    char* WideToUTF8(const wchar_t *str, int &len);
    wchar_t* UTF8ToWide(const char *str, int &len);
    char* UTF8ToANSI(const char *str, int &len);
    char* WideToANSI(const wchar_t *str, int &len);
    wchar_t* ANSIToWide(const char *str, int &len);
    std::string WideToANSI(const std::wstring &str);
    std::wstring ANSIToWide(const std::string &str);
    std::string ANSIToUTF8(const std::string &str);
    std::string UTF8ToANSI(const std::string &str);
    std::string WideToUTF8(const std::wstring &str);
    std::wstring UTF8ToWide(const std::string &str);

    int UTF8Bytes(const char *str, int strlen = -1); /* Returns the number of useful leading bytes in str that can be part of a well formed utf8 string which contains at most 4 byte long code points. When strlen is -1, str is considered to be a null terminated string. */
    int UTF8Length(const char *str, int strlen = -1); /* Number of valid code points in the passed UTF8 buffer. When strlen is -1, str is considered to be a null terminated string. */

    bool CopyToClipboard(const std::string &str);
    bool CopyToClipboard(const std::wstring &str);
    bool CopyToClipboard(const char *str, unsigned int len); /* You don't have to include the 0 character in len, but if you do, the function won't need to add it. */
    bool CopyToClipboard(const wchar_t *str, unsigned int len); /* You don't have to include the 0 character in len, but if you do, the function won't need to add it. */

    bool PasteFromClipboard(std::string &str);
    bool PasteFromClipboard(std::wstring &str);
    bool PasteFromClipboard(char* &dest, int &len); /* dest must be deleted with delete[] by the user. len specifies the length of the allocated array. */
    bool PasteFromClipboard(wchar_t* &dest, int &len); /* dest must be deleted with delete[] by the user. len specifies the length of the allocated array. */

    extern ConstValue<Application, int> ArgsCnt;
    extern ConstVector<Application, std::wstring> ArgsVar;
    extern ConstWString<Application> ExecutablePath;
    extern ConstValue<Application, DWORD> Win32MajorVersion;
    extern ConstValue<Application, DWORD> Win32MinorVersion;
    extern ConstValue<Application, DWORD> MainThreadId;
    extern ConstValue<Application, HINSTANCE> hInstance;
    extern ConstValue<Application, bool> WindowsIs64Bit;
    extern ConstValue<Screen, int> LogPixelsX;
    extern ConstValue<Screen, int> LogPixelsY;
    extern ConstValue<Screen, float> Scaling;

    bool WinVerSupported(int major, int minor); // Returns whether the running operating system has at least the given version number.
    bool WinVerSupported(int minmajor, int minminor, int maxmajor, int maxminor); // Returns whether the running operating system is between the min and max version numbers.

    enum RegionCombineModes { rcmAnd = RGN_AND, rcmCopy = RGN_COPY, rcmDiff = RGN_DIFF, rcmOr = RGN_OR, rcmXOr = RGN_XOR };
    int CreateCombinedRgn(HRGN &dest, const Rect &r1, const Rect &r2, RegionCombineModes mode); // Dest is created in the function.
    int CombineRgnWithRgn(HRGN dest, HRGN rgn1, HRGN rgn2, RegionCombineModes mode); // Dest must exist.
    int CombineRgnWithRect(HRGN dest, HRGN rgn, const Rect &r, RegionCombineModes mode); // Dest must exist.
    int CombineRectWithRgn(HRGN dest, const Rect &r, HRGN rgn, RegionCombineModes mode); // Dest must exist.
    int OffsetRgn(HRGN rgn, const Point &pt);
    HRGN CreateRgnCopy(HRGN orig);
    HRGN CreateRectRgnIndirect(const Rect &r);
    bool RgnIntersectsRgn(HRGN rgn1, HRGN rgn2);
    bool RgnIntersectsRect(HRGN rgn, const Rect &r);

    Size BitmapSize(HBITMAP hbmp); // Returns the dimensions of the hbmp or Size(-1, -1) on invalid argument. Height can be negative for top-down bitmaps.

    enum MessageBoxButtons { mbAbortRetryIgnore = 0x00000002, mbCancelTryContinue = 0x00000006, mbHelp = 0x00004000, mbOk = 0x00000000,
                             mbOkCancel = 0x00000001, mbRetryCancel = 0x00000005, mbYesNo = 0x00000004, mbYesNoCancel = 0x00000003 };
    enum MessageBoxIcons {  miNone = 0, miExclamation = 0x00000030, miInformation = 0x00000040, miQuestion = 0x00000020, miStop = 0x00000010 };
    enum MessageBoxDefaultButtons { mdb1 = 0, mdb2 = 0x00000100, mdb3 = 0x00000200, mdb4 = 0x00000300 };

    /// Shows a system message box with a custom title, message and selectable buttons.
    ModalResults ShowMessageBox(const std::wstring& message, const std::wstring& caption, MessageBoxButtons buttons, MessageBoxIcons icon = miNone, MessageBoxDefaultButtons def = mdb1);

    template<typename Iter>
    std::reverse_iterator<Iter> make_reverse_iterator(Iter iter)
    {
        return std::reverse_iterator<Iter>(iter);
    }

    bool KeyboardWithAltGr();

    template<typename TVAL, typename TMIN, typename TMAX>
    TVAL clamp(TVAL val, TMIN minval, TMAX maxval)
    {
        return min(max(val, minval), maxval);
    }

}
/* End of NLIBNS */


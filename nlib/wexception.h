#pragma once


namespace NLIBNS
{


// Base class for all exceptions thrown by the library.
class Exception
{
private:
    std::wstring text;
public:
    Exception();
    Exception(const Exception &e);

    Exception(const std::wstring &text);
    Exception(const wchar_t* ctext);

    virtual ~Exception();

    virtual std::wstring what();
};

class ECantAllocate : public Exception
{
private:
    typedef Exception base;
public:
    ECantAllocate(const std::wstring &text);
    ECantAllocate(const wchar_t* ctext);
};

class EOutOfRange : public Exception
{
private:
    typedef Exception base;
    bool rangeset;
    int valrange;
public:
    EOutOfRange(const std::wstring &text) : base(text), rangeset(false) {}
    EOutOfRange(const wchar_t* ctext) : base(ctext), rangeset(false) {}

    EOutOfRange(const std::wstring &text, int outofrangevalue);
    EOutOfRange(const wchar_t* ctext, int outofrangevalue);

    virtual std::wstring what();
};


}
/* End of NLIBNS */


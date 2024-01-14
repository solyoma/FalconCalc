#include "stdafx_zoli.h"
#include "wexception.h"


//---------------------------------------------


namespace NLIBNS
{


Exception::Exception() {};

Exception::Exception(const Exception &e) : text(e.text)
{}

Exception::Exception(const std::wstring &text) : text(text)
{}

Exception::Exception(const wchar_t* ctext) : text(ctext)
{}

Exception::~Exception()
{
}

std::wstring Exception::what()
{
    return text;
}


//---------------------------------------------


ECantAllocate::ECantAllocate(const std::wstring &text) : base(text)
{}

ECantAllocate::ECantAllocate(const wchar_t* ctext) : base(ctext)
{}


//---------------------------------------------


EOutOfRange::EOutOfRange(const std::wstring &text, int outofrangevalue) : base(text), rangeset(true), valrange(outofrangevalue)
{}

EOutOfRange::EOutOfRange(const wchar_t* ctext, int outofrangevalue) : base(ctext), rangeset(true), valrange(outofrangevalue)
{}

std::wstring EOutOfRange::what()
{
    if (!rangeset)
        return base::what();

    return base::what() + L" (Out of range value: " + IntToStr(valrange) + L")";
}

    
//---------------------------------------------


}
/* End of NLIBNS */


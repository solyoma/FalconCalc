#include "stdafx_zoli.h"
#include "FileStream.h"


//---------------------------------------------

namespace NLIBNS
{


fs_codecvt_noconv::fs_codecvt_noconv(bool textmode, size_t refs) : base(refs), textmode(textmode)
{
}

fs_codecvt_noconv::~fs_codecvt_noconv()
{
    ;
}

bool fs_codecvt_noconv::do_always_noconv() const throw()
{
    return !textmode;
}

int fs_codecvt_noconv::do_encoding() const throw()
{
    return !textmode ? 2 : 0;
}

auto fs_codecvt_noconv::do_in(stateT& state, const externT* from, const externT* from_end, const externT*& from_next, internT* to, internT* to_limit, internT*& to_next ) const -> result
{
    if (!textmode)
        return base::noconv;

    from_next = from;
    to_next = to;
    for ( ; from_next + 1 < from_end && to_next < to_limit ; from_next += 2, ++to_next)
    {
        if (*from_next != '\r' || *(from_next + 1) != 0)
        {
            *to_next = *from_next | (*(from_next + 1) << 8);
            continue;
        }
        if (from_next + 4 > from_end)
            break;
        if (*(from_next + 2) != '\n' || *(from_next + 3) != 0) // Expecting \n after \r.
            return error;
        *to_next = L'\n';
        from_next += 2; // Skip \r.
    }
    if (from_next != from_end)
        return partial;
    return ok;
}

int fs_codecvt_noconv::do_length(stateT& state, const externT* from, const externT* from_end, size_t maxsiz) const
{
    // Measure converted length of sequence in [from, from_end). In text mode, count the extra \r characters.
    int extra = 0;
    if (textmode)
    {
        internT *p = (internT*)from;
        while ((externT*)p + 2 <= from_end) // Adding 2 to p would result in incorrect behavior if externT wouldn't be char.
        {
            if (*p == L'\n')
                ++extra;
            ++p;
        }
    }

    return min(maxsiz, (size_t)((from_end - from) / 2) - extra);
}

int fs_codecvt_noconv::do_max_length() const throw()
{
    return !textmode ? 2 : 4;
}

auto fs_codecvt_noconv::do_out(stateT& state, const internT* from, const internT* from_end, const internT*& from_next, externT* to, externT* to_limit, externT*& to_next) const -> result
{
    if (!textmode)
        return base::noconv;

    from_next = from;
    to_next = to;
    for( ; from_next < from_end && to_next + 1 < to_limit; ++from_next, to_next += 2)
    {
        if (*from_next != L'\n')
        {
            *to_next = *(externT*)from_next;
            *(to_next + 1) = *((externT*)from_next + 1);
            continue;
        }

        if (to_next + 3 < to_limit)
        {
            *to_next = '\r';
            *(++to_next) = 0;
            *(++to_next) = '\n';
            *(to_next + 1) = 0;
        }
        else
            break;
    }
    if (from_next != from_end)
        return partial;
    return ok;
}

auto fs_codecvt_noconv::unshift(stateT& state, externT* to, externT* to_limit, externT*& to_next ) const -> result
{
    to_next = to;
    return base::noconv;
}


//---------------------------------------------


fs_codecvt_ansi::fs_codecvt_ansi(bool textmode, size_t refs) : base(refs), textmode(textmode)
{
}

fs_codecvt_ansi::~fs_codecvt_ansi()
{
    ;
}

bool fs_codecvt_ansi::do_always_noconv() const throw()
{
    return false;
}

int fs_codecvt_ansi::do_encoding() const throw()
{
    return !textmode ? 1 : 0;
}

auto fs_codecvt_ansi::do_in(stateT& state, const externT* from, const externT* from_end, const externT*& from_next, internT* to, internT* to_limit, internT*& to_next ) const -> result
{
    from_next = from;
    to_next = to;
    for ( ; from_next < from_end && to_next < to_limit ; ++from_next, ++to_next)
    {
        if (*from_next != '\r')
        {
            *to_next = *from_next;
            continue;
        }
        if (from_next + 2 > from_end)
            break;
        if (*(from_next + 1) != '\n') // Expecting \n after \r.
            return error;
        *to_next = L'\n';
        ++from_next; // Skip \r.
    }
    if (from_next != from_end)
        return partial;
    return ok;
}

int fs_codecvt_ansi::do_length(stateT& state, const externT* from, const externT* from_end, size_t maxsiz) const
{
    // Measure converted length of sequence in [from, from_end). In text mode, count the extra \r characters.
    int extra = 0;
    if (textmode)
    {
        const externT *p = from;
        while (p + 1 <= from_end)
        {
            if (*p == '\n')
                ++extra;
            ++p;
        }
    }

    return min(maxsiz, (size_t)(from_end - from) - extra);
}

int fs_codecvt_ansi::do_max_length() const throw()
{
    return !textmode ? 1 : 2;
}

auto fs_codecvt_ansi::do_out(stateT& state, const internT* from, const internT* from_end, const internT*& from_next, externT* to, externT* to_limit, externT*& to_next) const -> result
{
    from_next = from;
    to_next = to;
    for( ; from_next < from_end && to_next < to_limit; ++from_next, ++to_next)
    {
        if (*from_next != L'\n')
        {
            if (*from_next < 256)
                *to_next = *(externT*)from_next;
            else
                *to_next = '?';
            continue;
        }

        if (to_next + 1 < to_limit)
        {
            *to_next = '\r';
            *(++to_next) = '\n';
        }
        else
            break;
    }
    if (from_next != from_end)
        return partial;
    return ok;
}

auto fs_codecvt_ansi::unshift(stateT& state, externT* to, externT* to_limit, externT*& to_next ) const -> result
{
    to_next = to;
    return base::noconv;
}


//---------------------------------------------


fs_codecvt_utf8::fs_codecvt_utf8(bool textmode, size_t refs) : base(refs), textmode(textmode)
{
}

fs_codecvt_utf8::~fs_codecvt_utf8()
{
    ;
}

bool fs_codecvt_utf8::do_always_noconv() const throw()
{
    return false;
}

int fs_codecvt_utf8::do_encoding() const throw()
{
    return -1;
}

auto fs_codecvt_utf8::do_in(stateT& state, const externT* from, const externT* from_end, const externT*& from_next, internT* to, internT* to_limit, internT*& to_next ) const -> result
{
    from_next = from;
    to_next = to;

    int codesize;
    externT mask;

    for ( ; from_next < from_end && to_next < to_limit ; ++from_next, ++to_next)
    {
        if ((unsigned char)*from_next < 0x80)
        {
            if (textmode && *from_next == '\r')
            {
                if (from_next + 1 < from_end)
                {
                    if (*(from_next + 1) != '\n')
                        return error;
                    ++from_next;
                }
                else
                    return partial;
            }

            *to_next = *from_next;
            continue;
        }

        if ((*from_next & 0x40) != 0x40)
            return error; // Invalid at front of code point. Code point middle byte: 10xx xxx.
        codesize = 2;
        if (*from_next & 0x20)
        {
            if (*from_next & 0x10)
            {
                if (*from_next & 0x08)
                {
                    if (*from_next & 0x04)
                    {
                        if (*from_next & 0x02)
                            return error;
                        ++codesize;
                        mask = 0x03;
                    }
                    else
                        mask = 0x07;
                    ++codesize;
                }
                else
                    mask = 0x0f;
                ++codesize;
            }
            else
                mask = 0x1f;
            ++codesize;
        }
        else
            mask = 0x3f;

        if (codesize > 3) // More than 2 extra means 111Y YY0X which is at least 21 bits, more than the 16 our wchar_t can handle.
            return error;

        if (from_next + codesize > from_end)
            break;

        int ch = 0;
        for (int ix = 1; ix < codesize; ++ix)
        {
            if ((*(from_next + ix) & 0xC0) != 0x80)
                return error;
            ch |= int((unsigned char)*(from_next + ix) & 0x7f) << ((codesize - ix - 1) * 6);
        }
        ch |= (int((unsigned char)*from_next & mask) << ((codesize - 1) * 6));
        *to_next = (internT)ch;
        from_next += codesize - 1;
    }
    if (from_next != from_end)
        return partial;
    return ok;
}

int fs_codecvt_utf8::do_length(stateT& state, const externT* from, const externT* from_end, size_t maxsiz) const
{
    stateT st = state;

    // Measure converted length of sequence in [from, from_end).
    const externT *fnext;
    internT ch;
    internT *tnext;

    int len = 0;
    result r;
    do
    {
        r = do_in(st, from, from_end, fnext, &ch, &ch + 1, tnext);
        if (tnext == &ch + 1)
            ++len;
        else if (r == partial)
            break;
        if (r == error)
            break;
        from = fnext;
    } while(from != from_end);
    return len;
}

int fs_codecvt_utf8::do_max_length() const throw()
{
    return 9; // With BOM.
}

auto fs_codecvt_utf8::do_out(stateT& state, const internT* from, const internT* from_end, const internT*& from_next, externT* to, externT* to_limit, externT*& to_next) const -> result
{
    from_next = from;
    to_next = to;

    for( ; from_next < from_end && to_next < to_limit; ++from_next, ++to_next)
    {
        if ((unsigned short)*from_next < 0x80)
        {
            if (textmode && *from_next == L'\n')
            {
                if (to_next + 1 < to_limit)
                {
                    *to_next = L'\r';
                    ++to_next;
                }
                else
                    break;
            }
            *to_next = (externT)*from_next;
            continue;
        }

        if ((unsigned short)*from_next < 0x800) // 2 bytes long character.
        {
            if (to_next + 1 < to_limit)
            {
                *to_next = (internT)((*from_next >> 6) & 0x1f) | 0xc0;
                *(++to_next) = (internT)(*from_next & 0x3f) | 0x80;
            }
            else
                break;
        }
        else if ((unsigned int)*from_next < 0x00010000) // 3 bytes long character.
        {
            if (to_next + 2 < to_limit)
            {
                *to_next = (internT)((*from_next >> 12) & 0x0f) | 0xe0;
                *(++to_next) = (internT)((*from_next >> 6) & 0x3f) | 0x80;
                *(++to_next) = (internT)(*from_next & 0x3f) | 0x80;
            }
            else
                break;
        }
        else
            return error; // wchar_t can never get here, but return error just in case.
    }
    if (from_next != from_end)
        return partial;
    return ok;
}

auto fs_codecvt_utf8::unshift(stateT& state, externT* to, externT* to_limit, externT*& to_next ) const -> result
{
    to_next = to;
    return base::noconv;
}


//---------------------------------------------


FileStream::FileStream() : base(), enc(feDefault)
{
}

FileStream::FileStream(const wchar_t *filename, std::ios_base::openmode mode, FileStreamEncoding encoding) : base(), enc(feDefault)
{
    inner_setencoding(encoding, (mode & std::ios_base::binary) == 0);
    base::open(filename, (enc == feDefault ? mode : mode | std::ios_base::binary));
}

FileStream::FileStream(const std::wstring &filename, std::ios_base::openmode mode, FileStreamEncoding encoding) : base(), enc(feDefault)
{
    inner_setencoding(encoding, (mode & std::ios_base::binary) == 0);
    base::open(filename.c_str(), (enc == feDefault ? mode : mode | std::ios_base::binary));
}

void FileStream::inner_setencoding(FileStreamEncoding newenc, bool _textmode)
{
    if (enc == newenc && textmode == _textmode)
        return;

    enc = newenc;
    textmode = _textmode;

    switch (enc)
    {
    case feUnicode:
        imbue(std::locale(getloc(), new fs_codecvt_noconv(textmode)));
        break;
    case feUTF8:
        imbue(std::locale(getloc(), new fs_codecvt_utf8(textmode)));
        break;
    case feANSI:
        imbue(std::locale(getloc(), new fs_codecvt_ansi(textmode)));
        break;
    default:
        break;
    }
}

void FileStream::open(const wchar_t *filename, std::ios_base::openmode mode, FileStreamEncoding encoding)
{
    if (!is_open()) // If the file is_open(), the call to open() will fail, so only set the encoding if this won't change the open file handling.
        inner_setencoding(encoding, (mode & std::ios_base::binary) == 0);
    base::open(filename, (enc == feDefault ? mode : mode | std::ios_base::binary));
}

void FileStream::open(const std::wstring &filename, std::ios_base::openmode mode, FileStreamEncoding encoding)
{
    if (!is_open()) // If the file is_open(), the call to open() will fail, so only set the encoding if this won't change the open file handling.
        inner_setencoding(encoding, (mode & std::ios_base::binary) == 0);
    base::open(filename.c_str(), (enc == feDefault ? mode : mode | std::ios_base::binary));
}

FileStreamEncoding FileStream::encoding()
{
    return enc;
}

bool FileStream::skipbom()
{
    if (!good())
        return false;

    if (enc != feUnicode && enc != feUTF8)
    {
        setstate(failbit);
        return false;
    }


    wchar_t isbom = get();
    if (!good())
    {
        if (!bad())
            clear();
        return false;
    }

    if (isbom == 0xfeff)
        return true;
    putback(isbom);
    return false;
}

void FileStream::writebom()
{
    if (enc != feUnicode && enc != feUTF8)
    {
        setstate(failbit);
        return;
    }
    wchar_t BOM = 0xfeff;
    write(&BOM, 1);
}


//---------------------------------------------


}
/* End of NLIBNS */


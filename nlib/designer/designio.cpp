#include "stdafx_zoli.h"

#include "designio.h"
#include "designer.h"

//---------------------------------------------


namespace NLIBNS
{


    Indentation::Indentation(bool usetab, int size) : level(0), usetab(usetab), size(size) {}
    //Indentation::Indentation(int level) : level(level), usetab(false), size(4)
    //{
    //    if (level < 0)
    //        throw EOutOfRange(L"Cannot initialize indentation with negative number.", level);
    //}
    Indentation::Indentation(const Indentation &other) : level(other.level), usetab(other.usetab), size(other.size) {}

    Indentation& Indentation::operator=(const Indentation &other)
    {
        level = other.level;
        return *this;
    }

    Indentation& Indentation::operator=(int newlevel)
    {
        if (newlevel < 0)
            throw EOutOfRange(L"Cannot initialize indentation with negative number.", newlevel);
        level = newlevel;
        return *this;
    }

    bool Indentation::operator==(int val)
    {
        return level == val;
    }

    bool Indentation::operator==(const Indentation &other)
    {
        return level == other.level;
    }

    Indentation& Indentation::operator++()
    {
        ++level;
        return *this;
    }

    Indentation Indentation::operator++(int)
    {
        ++level;
        return *this;
    }

    Indentation& Indentation::operator--()
    {
        if (level == 0)
            throw EOutOfRange(L"Indentation level is 0. Cannot decrement.");
        --level;
        return *this;
    }

    Indentation Indentation::operator--(int)
    {
        if (level == 0)
            throw EOutOfRange(L"Indentation level is 0. Cannot decrement.");
        --level;
        return *this;
    }

    std::wstring Indentation::wstr() const
    {
        if (usetab)
            return L"\t";
        else
            return std::wstring(size, L' ');
    }

    std::string Indentation::str() const
    {
        if (usetab)
            return "\t";
        else
            return std::string(size, ' ');
    }

    std::wiostream& operator<<(std::wiostream &stream, const Indentation &indent)
    {
        int ppos = stream.tellp();
        int gpos = stream.tellg();
        if (ppos > 0)
        {
            auto state = stream.rdstate();
            stream.seekg(ppos - 1);
            int c = stream.peek();
            stream.seekg(gpos);
            stream.clear(state);

            if (c != L'\r' && c != L'\n')
                return stream;
        }

        int l = indent.level;
        while (l--)
            stream << indent.wstr();
        return stream;
    }

    std::iostream& operator<<(std::iostream &stream, const Indentation &indent)
    {
        int ppos = stream.tellp();
        int gpos = stream.tellg();
        if (ppos > 0)
        {
            auto state = stream.rdstate();
            stream.seekg(ppos - 1);
            int c = stream.peek();
            stream.seekg(gpos);
            stream.clear(state);

            if (c != '\r' && c != '\n')
                return stream;
        }

        int l = indent.level;
        while (l--)
            stream << indent.str();
        return stream;
    }

    std::wostream& operator<<(std::wostream &stream, const Indentation &indent)
    {
        int l = indent.level;
        while (l--)
            stream << indent.wstr();
        return stream;
    }

    std::ostream& operator<<(std::ostream &stream, const Indentation &indent)
    {
        int l = indent.level;
        while (l--)
            stream << indent.str();
        return stream;
    }


    //---------------------------------------------


    TokenStream::TokenStream(std::wistream &stream) : linenum(0), colpos(0), type(rttNone), peektype(rttNone), peeked(false), stream(stream)
    {

    }

    TokenStream::TokenStream(const TokenStream& other) : stream(other.stream)
    {
        linenum = other.linenum;
        colpos = other.colpos;
        type = other.type;
        token = other.token;
        peektype = other.peektype;
        peektoken.str(other.peektoken.str());
        peektoken.clear(other.peektoken.rdstate());
        pmode = other.pmode;
        peeked = other.peeked;
    }

    bool TokenStream::empty()
    {
        return type == rttNone;
    }

    //   words: /([a-zA-Z0-9_.-])/
    //   strings: /"([^\"]*(?:\\)*(?:\")*)*"/ (that is, between quotes. \ is the escape character for \ or ") The quote characters from the edges are dropped.
    //   template arguments: same as strings but between <> instead of quotes, and there is no escape character.
    //   Spaces, tabs and newline characters not in the above are skipped. Anything else is a single character token.

    bool TokenStream::read(ReadTokenMode mode)
    {
        token = peek(mode);
        type = peektype;
        peeked = false;
        return type != rttNone;
    }

    void TokenStream::unread()
    {
        if (peeked || peektype == rttNone)
            return;
        peeked = true;
    }

    std::wstring TokenStream::peek(ReadTokenMode mode)
    {
        return peek(peektype, mode);
    }

    std::wstring TokenStream::peek(ReadTokenType &ptype, ReadTokenMode mode)
    {
        if (peeked)
        {
            if ((mode != rtmUnspecified || pmode == rtmUnspecified) && mode != pmode)
                throw L"Cannot peek with a different mode.";

            ptype = peektype;
            return peektoken.str();
        }

        wchar_t c[3];
        peektoken.str(std::wstring());
        peektoken.clear();
        ptype = rttNone;
        pmode = mode;
        nmscpcnt = 0;

        int read = 0;
        while ((read = nextchar(c, ptype, mode)))
        {
            for (int ix = 0; ix < read; ++ix)
                peektoken << c[ix];
            if (ptype == rttToken) // Tokens are single character.
                break;
        }
        if (c[0] == 0 && ptype == rttNone)
        {
            peektoken.str(std::wstring());
            peektoken.clear();
        }
        else
            peeked = true;
        peektype = ptype;
        return peektoken.str();
    }

    int TokenStream::nextchar(wchar_t c[3], ReadTokenType &rtype, ReadTokenMode mode)
    {
        if (!stream.good())
        {
            c = 0;
            return 0;
        }

        if (rtype == rttNone) // Find the first character of a new token.
        {
            do
            {
                stream.get(c[0]);
                if (!stream.good())
                {
                    c[0] = 0;
                    return 0;
                }

                if (rtype != rttNone && (c[0] == L'\n' || c[0] == L'\r'))
                {
                    c[0] = 0;
                    return 0;
                }
                if (c[0] == L'\n')
                {
                    ++linenum;
                    colpos = 0;
                    continue;
                }

                ++colpos;
                if (c[0] == L'"')
                {
                    rtype = rttString;
                    return nextchar(c, rtype, mode);
                }
                else if (c[0] == L'<')
                {
                    rtype = rttTemplateArg;
                    return nextchar(c, rtype, mode);
                }
            } while (c[0] == L' ' || c[0] == L'\t' || c[0] == L'\r' || c[0] == L'\n');

            if ((c[0] >= L'a' && c[0] <= L'z') || (c[0] >= L'A' && c[0] <= L'Z') || (c[0] >= L'0' && c[0] <= L'9') || c[0] == L'_' || c[0] == L'.' || c[0] == L'-')
                rtype = rttUnquoted;
            else
                rtype = rttToken;
            return 1;
        }

        // String or template arg
        bool escape = false;
    
        while(true)
        {
            stream.get(c[0]);
            if (!stream.good()) // No closing quote or >.
            {
                c[0] = 0;
                if (rtype != rttUnquoted)
                    rtype = rttNone;
                return 0;
            }

            if (rtype == rttString)
            {
                if (c[0] == L'"')
                    return escape ? 1 : 0;
                if (rtype == rttString && c[0] == L'\\')
                {
                    if (escape)
                        return 1;
                    escape = true;
                    continue;
                }
            }

            if (rtype == rttTemplateArg && c[0] == L'>')
                return 0;

            if (rtype == rttUnquoted && !((c[0] >= L'a' && c[0] <= L'z') || (c[0] >= L'A' && c[0] <= L'Z') || (c[0] >= L'0' && c[0] <= L'9') || c[0] == L'_' || c[0] == L'.' || c[0] == L'-'))
            {
                if (mode == rtmUnspecified || c[0] != L':' || (mode == rtmSingleNamespace && nmscpcnt == 1))
                {
                    stream.unget();
                    return 0;
                }
                stream.get(c[1]); // Trying to get a second colon after the namespace identifier.
                if (!stream.good() || c[1] != L':')
                {
                    stream.clear();
                    stream.seekg(-2, std::ios_base::cur);
                    return 0;
                }
                stream.get(c[2]); // Trying to get the unquoted token character after the double colon to make the previous string a valid namespace token.
                if (!stream.good() || !((c[2] >= L'a' && c[2] <= L'z') || (c[2] >= L'A' && c[2] <= L'Z') || (c[2] >= L'0' && c[2] <= L'9') || c[2] == L'_' || c[2] == L'.' || c[2] == L'-'))
                {
                    stream.clear();
                    stream.seekg(-3, std::ios_base::cur);
                    return 0;
                }
                ++nmscpcnt;
                return 3;
            }

            break;
        }

        return 1;
    }

    ReadTokenType TokenStream::tokentype()
    {
        return type;
    }

    const std::wstring& TokenStream::toString()
    {
        return token;
    }

    bool TokenStream::toInt(int& i)
    {
        if (type != rttString || token.length() == 0)
            return false;
        return StrToInt(token, i);
    }

    bool TokenStream::operator==(const std::wstring &str)
    {
        return token == str;
    }

    bool TokenStream::operator!=(const std::wstring &str)
    {
        return token != str;
    }

    bool TokenStream::operator==(ReadTokenType rtype)
    {
        return type == rtype;
    }

    bool TokenStream::operator!=(ReadTokenType rtype)
    {
        return type != rtype;
    }

    int TokenStream::linenumber()
    {
        return linenum;
    }

    int TokenStream::column()
    {
        return colpos;
    }

    std::wstring TokenStream::position_str()
    {
        return std::wstring(L"Line: ") + IntToStr(linenum + 1) + L" Column: " + IntToStr(colpos + 1);
    }


    //---------------------------------------------


    std::wstring TokenE::what()
    {
        std::wstringstream ss;
        ss << base::what() << L" on line " << token.linenumber() << L" at character " << token.column() << L".";

        if (!token.empty())
            ss << L" Last read token: \"" << token.toString() << L"\".";

        if (token.peektype != rttNone)
            ss << L" Last peeked token: \"" << token.peektoken << L"\".";

        return ss.str();
    }


    //---------------------------------------------


    SourceStream::SourceStream(std::wistream &stream) : stream(stream)
    {
        rewind();
    }

    void SourceStream::rewind()
    {
        line.clear();
        prevline.clear();
        prefix.clear();
        prevprefix.clear();
        stream.clear();
        stream.seekg(0);
        linenum = -1;
        prevlinenum = -2;
        chnum = 0;
        wlen = 0;
        blevel = 0;
        fetchagain = false;
    }

    void SourceStream::fforward()
    {
        rewind();
        while (get(true))
            ;
    }

    bool SourceStream::skip_to(const std:: wstring &str, int level)
    {
        if (str.empty())
            return false;
        while (get(str[0] != L'#') && (last() != str || (level >= 0 && blevel != level)))
            ;
        return !eof();
    }

    bool SourceStream::eof() const
    {
        return linenum >= 0 && wlen == 0 && prefix.empty();
    }

    void SourceStream::unget()
    {
        if (fetchagain || linenum == -1)
            return;

        fetchagain = true;

        if (linenum != prevlinenum)
            std::swap(prevline, line);
        std::swap(prevlinenum, linenum);
        std::swap(prevchnum, chnum);
        std::swap(prevwlen, wlen);
        std::swap(prevblevel, blevel);
        std::swap(prevprefix, prefix);
        std::swap(prevlinepref, linepref);
        std::swap(prevchpref, chpref);

    }

    void SourceStream::append()
    {
        prefix = prevprefix + prevline.substr(prevchnum, prevwlen) + line.substr(chnum, wlen);
        chnum += wlen;
        wlen = 0;
        
    }

    bool SourceStream::get(bool skipdirectives)
    {
        if (fetchagain)
        {
            if (linenum != prevlinenum)
                std::swap(prevline, line);
            std::swap(prevlinenum, linenum);
            std::swap(prevchnum, chnum);
            std::swap(prevwlen, wlen);
            std::swap(prevblevel, blevel);
            std::swap(prevprefix, prefix);
            std::swap(prevlinepref, linepref);
            std::swap(prevchpref, chpref);
            fetchagain = false;
            return !eof();
        }

        if (linenum != prevlinenum)
            prevline = line;
        prevlinenum = linenum;
        prevchnum = chnum;
        prevwlen = wlen;
        prevblevel = blevel;
        prevprefix = prefix;
        prevlinepref = linepref;
        prevchpref = chpref;


        bool singlecomment = false; // Single comment line or compiler directive line.

        linepref = -1;
        chpref = -1;

        chnum += wlen; // Skip to the end of the last word.
        wlen = 0;
        prefix.clear();

        do
        {
            // Get lines if we are at the beginning of the stream or at the end of the last line.
            while (chnum == (int)line.length() && stream.good())
            {
                nextline();
                if (line.empty())
                    singlecomment = false;
            }

            if (stream.fail()) // Eof with nothing to read or some error.
            {
                if (linenum == -1 || linepref == -1)
                {
                    if (linenum == -1)
                    {
                        linenum = 0;
                        chnum = 0;
                        wlen = 0;
                    }
                    linepref = linenum;
                    chpref = chnum;
                }
                return false;
            }

            if (!singlecomment)
            {
                if (skipdirectives && chnum == 0) // Starting from a new line. See if it starts with some compiler pragma or definition etc..
                {
                    skipspace();
                    if (chnum < (int)line.length() && line[chnum] == L'#') // Line with compiler directive found.
                        singlecomment = true;
                }
                else
                    skipspace();

                if (stream.fail()) // Eof with nothing to read or some error.
                    return false;

                if (!singlecomment && chnum < (int)line.length() - 1 && line[chnum] == L'/' && line[chnum + 1] == L'/')
                    singlecomment = true;
                else if (!singlecomment && chnum == (int)line.length() - 1 && line[chnum] == L'\\') // Skip if line ends with \ character which would mean continuation in next line, because we haven't started reading a word yet.
                    ++chnum;
            }

            if (singlecomment) // Skip comment line. Skip the following lines as well if this ends in '\' to handle multi line defines and comments.
            {
                if (line.empty() || line.back() != L'\\')
                    singlecomment = false;

                chnum = line.length();
            }
        } while (chnum == (int)line.length());

        // We are at the first character of the next word. Save the position.
        linepref = linenum;
        chpref = chnum;

        wchar_t ch = nextchar();
        if (ch == 0)
            return false;

        if (ch == L'{' || ch == L'}')
        {
            if (ch == L'{')
                ++blevel;
            else
                blevel = max(0, blevel - 1); // Ignore errors of too many closing curly brackets.
            return true;
        }

        if (ch == L':')
        {
            ch = nextchar();
            if (ch != 0 && ch != L':')
                --wlen;

            return true;
        }

    //Matching words have the following format. Anything else counts as a single character word.
    //#[_a-zA-Z][_a-zA-Z0-9]*
    //[_a-zA-Z0-9]".*"[_a-zA-Z0-9]
    //[_a-zA-Z0-9]'?'[_a-zA-Z0-9]
    //(0-9|).[0-9_a-zA-Z0-9]
    //c++11 string format [_a-zA-Z0-9]R"(.*)"[_a-zA-Z0-9]
    //c++11 string format [_a-zA-Z0-9]R"delimiter(.*)delimiter"[_a-zA-Z0-9]

        if (!skipdirectives && ch == L'#')
        {
            ch = nextchar();
            if (ch && ch != L'_' && (ch < L'a' || ch > L'z') && (ch < L'A' || ch > L'Z'))
                --wlen;
            else while ((ch = nextchar()) != 0)
            {
                if (ch != L'_' && (ch < L'a' || ch > L'z') && (ch < L'A' || ch > L'Z') && (ch < L'0' || ch > L'9'))
                {
                    --wlen;
                    break;
                }
            }
        }
        else if ((ch >= L'0' && ch <= L'9') || ch == L'.') // Handle next word as a number literal.
        {
            int dotfound = false; // The . character in the number has been found once.
            wchar_t prevch;
            do
            {
                prevch = ch;
                ch = nextchar();
                if (!ch || (prevch == L'.' && ((prefix.empty() && wlen == 2) || (prefix.length() == 1 && wlen == 1)) && (ch < L'0' || ch > L'9'))) // Found a non number starting with a . character. Don't treat this as a number if it is not followed by one.
                {
                    if (ch)
                        --wlen;
                    break;
                }
                if ((ch < L'0' || ch > L'9') && (ch < L'a' || ch > L'z') && (ch < L'A' || ch > L'Z') && ch != L'.' && ch != L'_')
                {
                    --wlen;
                    break;
                }
                if (ch == L'.')
                {
                    if (dotfound)
                        throw L"Invalid number format.";
                    else
                        dotfound = true;
                }
            } while (true);
        }
        else if ((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z') || ch == L'_' || ch == L'"' || ch == L'\'') // Handle this as a general word in the source code, which could be a string as well.
        {
            std::wstring strdelim; // Delimiter for C++11 format strings that can look like XXR"delimiter(anything even ( ) and " chars)delimiter".
            bool bracketdelim = false; // True for C++11 format strings that either have a delimiter or are between XXR"( )".
            wchar_t prevch; // ch read in the previous nextchar() call.
            bool stringpassed = false; // We have encountered an opening quote for a string literal.
            wchar_t quotechar = 0; // The character starting the string or character literal. If this is 0 we are not inside a string.
            int litlen = 0; // The length of the string literal. This must be 1 for character literals.
            do
            {
                prevch = ch;
                ch = nextchar();
                if (!ch || ((ch < L'0' || ch > L'9') && (ch < L'a' || ch > L'z') && (ch < L'A' || ch > L'Z') && ch != L'_' && ch != L'"' && ch != L'\'' && prevch != L'"' && prevch != L'\''))
                {
                    if (ch)
                        --wlen;
                    break;
                }

                if (ch == L'"' || ch == L'\'' || prevch == L'"' || prevch == L'\'')
                {
                    if (stringpassed && quotechar == 0)
                        throw L"Second opening quote in string literal is not valid C++.";
                    stringpassed = true;
                    if (prevch == L'"' || prevch == L'\'')
                    {
                        ch = prevch;
                        --wlen;
                    }
                    quotechar = ch;
                }
                if (prevch == L'R' && ch == L'"')
                {
                    if ((ch = nextchar()) == L'"')
                        throw L"Invalid raw string literal.";
                    else
                        bracketdelim = true;

                    strdelim.reserve(16);
                    while ((ch = nextchar()) != 0 && ch != L'(' && ch != L')' && ch != L'"' && ch != L'\\')
                    {
                        if (strdelim.length() == 16)
                            throw L"Delimiter in raw string literal cannot be more than 16 characters.";
                        strdelim += ch;
                    }
                    if (ch != L'(')
                        throw L"Invalid raw string literal. Opening bracket after delimiter missing.";
                }

                int delimpos = -1; // Position in the ending delimiter for c++11 raw strings. If this is negative, the ending bracket hasn't been found yet.
                while(quotechar != 0)
                {
                    prevch = ch;
                    ch = nextchar();
                    if (!ch)
                        throw L"End of stream in the middle of a string literal.";

                    if (delimpos >= 0)
                    {
                        if (delimpos == (int)strdelim.length())
                        {
                            if (ch != L'"')
                                throw L"C++11 raw string delimiter must be followed by a closing quotation mark.";
                            quotechar = 0;
                            continue;
                        }

                        // We are currently after a closing bracket trying to find the string delimiter that ends the string.
                        if (strdelim[delimpos] != ch) // This is not the delimiter. Start over by looking for the closing bracket.
                            delimpos = -1;
                        else // Character matches current position in the delimiter string.
                        {
                            ++delimpos;
                            continue;
                        }
                    }
                        
                    if (!bracketdelim)
                    {
                        if (prevch == L'\\') // Anything in a string can be escaped with the \ character.
                            ch = 0; // Make ch 0 so even if it was a \ character, prevch won't remember that to avoid using the second \ in \\.
                        else if (ch == quotechar) // End of string.
                        {
                            if (litlen != 1 && quotechar == L'\'')
                                throw L"Character literal must be exactly 1 character long.";
                            quotechar = 0;
                            ch = 0;
                        }
                        else
                            ++litlen;
                    }
                    else if (bracketdelim && ch == L')') // Only the closing bracket + delimiter counts. If there is no closing quote after bracket and delimiter found, it is an error.
                        delimpos = 0;
                }
            } while (true);
        }
        // else... In any other case wlen is 1 and ch is a single character which is not part of any word or string literal..

        return true;

    }

    wchar_t SourceStream::nextchar()
    {
        int pos;
        int len;
        while (true)
        {
            pos = chnum + wlen;
            len = line.length();
            if (pos == len)
                return 0;
            if (pos < len - 1 || line[pos] != L'\\')
            {
                ++wlen;
                return line[pos];
            }
            prefix += line.substr(chnum, wlen);
            nextline();
            wlen = 0;
        }
    }

    void SourceStream::nextline()
    {
        if (fetchagain && linenum != prevlinenum)
        {
            line = prevline;
            fetchagain = false;
        }
        else
            std::getline(stream, line);

        chnum = 0;
        ++linenum;
    }

    void SourceStream::skipspace()
    {
        int len = line.length();
        wchar_t ch;
        bool commentblock = false;
        while (chnum < len || !stream.fail())
        {
            if (chnum == len)
            {
                nextline();
                len = line.length();
                continue;
            }
            ch = line[chnum];

            if (!commentblock)
            {
                if (ch == L' ' || ch == L'\t')
                    ++chnum;
                else if (chnum < len - 1 && ch == L'/' && line[chnum + 1] == L'*') // Comment blocks must be skipped here as well.
                {
                    chnum += 2;
                    commentblock = true;
                }
                else
                    break;
            }
            else
            {
                ++chnum;
                if (chnum == len || ch != L'*' || line[chnum] != L'/')
                    continue;
                // End of comment block found.
                ++chnum;
                commentblock = false;
            }
        }
    }

    bool SourceStream::emptyforward() const
    {
        if (eof())
            return false;

        int ix = 0;
        int len = line.length();
        while (chnum + wlen + ix < len && (line[chnum + wlen + ix] == L' ' || line[chnum + wlen + ix] == L'\t'))
            ++ix;
        return chnum + wlen + ix == len;
    }

    void SourceStream::skip_after(bool skipnextempty)
    {
        //int ix = 0;
        //int len = line.length();
        prefix += line.substr(chnum, wlen);
        chnum += wlen;
        wlen = 0;

        //// Check if we are the last word in the line.
        //while (chnum + ix < len && (line[chnum + ix] == L' ' || line[chnum + ix] == L'\t'))
        //    ++ix;
        //if (chnum + ix < len) // There was something after the current position in this line.
        //    return;
        if (!emptyforward())
            return;
        nextline();
        if (emptyforward())
            nextline();
    }

    auto SourceStream::position() -> Position
    {
        Position pos(linepref, chpref);
        return pos;
    }

    auto SourceStream::position_after() -> Position
    {
        //if (fetchagain)
        //    return position();
        Position pos(linenum, chnum + wlen);
        return pos;
    }

    auto SourceStream::wordposition() -> Size
    {
        return Size(Position(linepref, chpref), Position(linenum, chnum + wlen));
    }

    std::wstring SourceStream::last()
    {
        //if (fetchagain)
        //    return std::wstring();

        if (!wlen)
            return prefix;
        return prefix + line.substr(chnum, wlen);
    }

    int SourceStream::level()
    {
        return blevel;
    }

    //int SourceStream::linepos()
    //{
    //    return linenum;
    //}
    //
    //int SourceStream::chpos()
    //{
    //    return chnum;
    //}

    void SourceStream::record_update(const std::wstring &str)
    {
        record_update(position(), position_after(), str);
    }

    void SourceStream::record_update(const Position &from, const Position &to, const std::wstring &str)
    {
        record_insert(from, str);
        record_remove(from, to);
    }

    void SourceStream::record_update(const Size &pos, const std::wstring &str)
    {
        record_update(pos.first, pos.second, str);
    }

    void SourceStream::record_remove(const Position &pos)
    {
        record_remove(pos, position());
    }

    void SourceStream::record_insert_lines(std::wistream &s, bool after, int priority)
    {
        record_insert_lines(s, after ? position_after() : position(), priority);
    }

    void SourceStream::record_insert_lines(std::wistream &s, const Position &pos, int priority)
    {
        std::wstring str;
        bool first = true;
        //bool haseol = false;
        while (s.good())
        {
            std::getline(s, str);
            if (!s.fail())
            {
                if (!first)
                    record_insert(pos, L"\n", priority);
                record_insert(pos, str, priority);
                first = false;
            }
            else if (!first)
                record_insert(pos, L"\n", priority);
        }
    }

    void SourceStream::record_remove(const Position &from, const Position &to)
    {
        if (record.empty() || record.back().from <= from)
        {
            record.push_back(UpdateRec(from, to, 0));
            return;
        }
        auto it = record.rbegin();
        while (it->from > from)
            ++it;
        record.insert(it.base(), UpdateRec(from, to, 0));
    }

    void SourceStream::record_remove(const Size &pos)
    {
        record_remove(pos.first, pos.second);
    }

    void SourceStream::record_insert(const Position &at, const std::wstring &str, int priority)
    {
        if (record.empty() || record.back().from < at || (record.back().from == at && record.back().priority <= priority && !record.back().remove))
        {
            record.push_back(UpdateRec(at, str, priority));
            return;
        }

        auto it = record.rbegin();
        while (it != record.rend() && (it->from > at || (it->from == at && (it->priority > priority || it->remove))))
            ++it;
        record.insert(it.base(), UpdateRec(at, str, priority));

    }

    void SourceStream::update(std::wstringstream &s)
    {
        std::wstring str;
        rewind();
        s.clear();
        s.str(L"");
        linenum = -1;
        bool firstline = true;
        while (!stream.fail() || !record.empty())
        {
            if (!stream.fail())
            {
                std::getline(stream, str);
                chnum = 0;

                if (!firstline)
                    s << std::endl;
                else
                    firstline = false;

                if (stream.fail() && record.empty()) // End of stream or some error occured.
                    break;
            }

            ++linenum;

            while (!record.empty() && record.front().from.linepref < linenum)
                record.pop_front();

            if (record.empty() || record.front().from.linepref > linenum)
            {
                if (stream.fail()) // Nothing in the stream and the inserts in the records would come after the last line.
                {
                    if (!record.empty() && record.front().from.linepref == linenum + 1)
                        s << std::endl;
                    else
                        record.clear();
                }
                else
                    s << str;
                continue;
            }

            while (!record.empty() && record.front().from.linepref == linenum)
            {
                UpdateRec rec(record.front());
                record.pop_front();

                if (rec.from.chpref > chnum)
                {
                    s << str.substr(chnum, rec.from.chpref - chnum);
                    chnum = rec.from.chpref;
                }
                if (chnum != rec.from.chpref) // Ignore invalid records if an insertion or removal was added incorrectly.
                    continue; 
                if (!rec.remove)
                {
                    s << rec.str;
                    continue;
                }

                if (rec.to.linepref < rec.from.linepref || (rec.to.linepref == rec.from.linepref && rec.to.chpref < rec.from.chpref)) // Invalid record where the end position is less than the start position.
                    continue;

                if (rec.to.linepref == linenum)
                {
                    chnum = rec.to.chpref;
                    continue;
                }

                while (linenum != rec.to.linepref && !stream.fail())
                {
                    std::getline(stream, str);
                    ++linenum;
                }
                chnum = rec.to.chpref;
            }

            if (!stream.fail() && chnum < (int)str.length())
                s << str.substr(chnum);
        }
    }


    //---------------------------------------------


}
/* End of NLIBNS */


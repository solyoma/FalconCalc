//---------------------------------------------------------------------------
#include "stdafx_zoli.h"
#ifndef QTSA_PROJECT
	#include "stdafx_lc.h"
#endif
#include <locale>
#include <string>
#include <sstream>

#if defined _MSC_VER && defined __cplusplus_cli
	#include <msclr\marshal_cppstd.h>
#endif
#include "numdefs.h"
#include "numconsts.h"
#include "numconv.h"

using std::hex;
using std::oct;
using std::uppercase;
/*===============================================
 *
 *----------------------------------------------*/
RealNumber StringToNumber(STRING &str)
{
#ifdef QTSA_PROJECT
	std::string STR;
	STR = str.toLatin1();
#else 
	#define STR str
#endif
   	std::locale loc = std::cout.getloc();
    STRINGSTREAM ss;
    ss.precision(40);

    if(STR.length() > 1 &&  STR[0] == '0')
    {
        bool overflow = false;
        if(STR[1] == 'x' || STR[1] == 'X')
        {
            STR = STR.substr(2);
            if(STR.length() > sizeof(long long)*2)
                overflow = true;
            ss << hex;
        }
        else if( isdigit(STR[1],loc) )
        {
            if(STR.length() > 22)
                overflow = true;
            ss << oct;
            }
        else    // decimal
        {
            ss << STR;
            RealNumber val;
            ss >> val;
            return val;
        }
        long long l;
        ss << STR;
        ss >> l;
        if(overflow)
            return INF();
        return (RealNumber) l;
    }
    ss << STR;
    RealNumber val;
    ss >> val;
#ifndef QTSA_PROJECT
	#undef STR
#endif
    return val;
}

/*===============================================
 * TASK: Round a decimal string to a given number of 
 *       decimal digits
 * EXPECTS: s: decimal string w.o. exponent
 *			num_digits: required number of decimal digits or -1
 * RETURNS: decimal string rounded to 'num_digits' decimal digits
 *          or original string
 * REMARKS: internal function only
 *          if decimal digits required (num_digits > 0), but
 *          there are fewer present the number is padded by 0s
 *----------------------------------------------*/
static STRING _RoundStr(STRING s, int num_digits)
{
	if(num_digits < 0)	// general format: leave all decimal digits
		return s;

	int posd = s.find_first_of(numeric::constants.decpoint);// position of decimal point in string or npos

	if(posd == s.npos)		// no decimal point 
	{
		if (num_digits == 0)	// and none required
			return s;
							// but it should be so append
		s += PCHAR(".");
		while(num_digits--)	// and add fill digits
				s += PCHAR("0");
		return s;
	}

	_CHART sign = 0;

			// there is a decimal point in the number
	if(s[0] == '+' || s[0] == '-')
	{
			sign = s[0];
			s = s.substr(1);	// cut sign
	}

	int len = s.length() -1,	// index of last digit in string ( > 0)
		prev = len-1,			// index of previous digit
		nd = len - posd; // count of decimal digits in number string
	
	while(nd < num_digits)		// add trailing '0's when needed
			s += PCHAR("0"), ++nd;		
	//if (nd == num_digits)		// in this case we are ready
	//	return s;

	int carry = 0;
									// round up from the end	
	while(prev >= posd + num_digits || (prev >= 0 && carry) )
	{
		if(s[prev] == numeric::constants.decpoint) // only happens when there was a carry
			--prev;								   // skip decimal point to previous character

		if(s[len] > '4' || carry)
		{
			if(prev >= 0)
			{
				if(s[prev] == '9')
					s[prev] = '0', carry = 1;
				else
					s[prev] = s[prev]+1, carry = 0;		// digit codes are continuous
			}
		}
		--prev;
		--len;
	}
	if(carry)
		s = PCHAR("1")+s, ++posd;
	posd += num_digits + (num_digits > 0 ? 1 : 0);			// new length of string: add length of dec. digits
	s = s.substr(0, posd);    //  +carry);	// when there was a carry

	if (sign == '-')		// drop positive sign
	{						 
		s = PCHAR("-") + s;	// keep negative one and
		if (posd != string::npos)
			++posd;			// and set posd to new position
	}
	return s;
}

/*===============================================
 * TASK: Format positive number for mode dfSci or dfExp
 * EXPECTS: sres fields are set
 *
 * RETURNS: converted number have the greatest precision
 * REMARKS: internal function only
 *----------------------------------------------*/
static STRING _FormatSciOrEng(STRING &s, int &exp, size_t &posd, DEC_FORM df)
{
// first create a normalized number
	if(posd != s.npos)	// then get decimal point out of string
	{					// and correct exponent
		s = s.substr(0, posd) + s.substr(posd+1);
		int i;
		for(i = 0; s[i] == '0'; ++i)
			--exp;
		if(i==s.length() )
			return s = '0';
		if(i)
		{
			s = s.substr(i); // cut leading 0s
			posd = 1;
		}
	}
	else
		posd = s.length();
			// Sci format:
	exp +=posd-1;
	posd = 1;
	
	if(df == dfEng)		// for engineering format
	{
		int shift = exp % 3;
		if(shift <0)	// then exp < 0 but the mantissa > 0
			shift += 3;	// therefore shift to the right
		exp -= shift;
		posd += shift;
	}

	s = s.substr(0,posd) + PCHAR(".")+s.substr(posd);
	return s;
}
// -------------------- OUTPUT functions -------------------

/*===============================================
 * TASK: convert decimal number in 'sres' to string
 * EXPECTS: sres fields are set
 *
 * RETURNS: converted number have the greatest precision
 * REMARKS: internal function only
 *----------------------------------------------*/
static STRING _NumberToDecString(STRING_RESULT &sres, int dummy)
{
#ifndef _MSC_VER
//__BORLANDC__
    #define abs(a) fabs(a)
#endif
	if(sres.value == 0.0)
		return _RoundStr(PCHAR("0"), sres.num_digits);

    std::locale loc = std::cout.getloc();
    _CHART decpoint = numeric::constants.decpoint;
	int exp=0;		// exponent of number
	size_t posd,	// position of decimal point
		   pose;	// position of exponent

    STRINGSTREAM ss;
	STRING s;

	ss.unsetf ( std::ios::floatfield );
    ss.precision(32);			// all significant digits w.o. trailing zeros
	ss << sres.value;
	ss >> s;					// s contains all valid digits and may contain an exponent and/or decimal point too
	if(sres.decFormat == dfGeneral && sres.num_digits < 0)		// then we are already OK
				return s;
	 // either format is not general, or number of decimal digits is given
	pose = s.find_first_of('e');	// exponent in number
	if(pose != s.npos)
	{
		ss.clear();
		ss.str(STRING(PCHAR("")));
		ss << s.substr(pose+1);
		ss >> exp;
		s = s.substr(0, pose);	// mantissa
	}
	posd = s.find_first_of(decpoint);
	wchar_t sign = s[0] == '-' || s[0] == '+' ? s[0] : 0;
	if (sign)
	{
		s = s.substr(1);	// drop sign
		if (posd != string::npos)
			--posd;				// and modify position of decimal point to reflect this
	}
	// at this point 's' contains a positive mantissa w. or w.o a decimal point and 'exp' is the exponent
	// but if there is a decimal point it may be anywhere

	STRING wstmp;			// used to check rounding
	int posd1 = 0;			// position of decimal point in wstmp;
	bool doexp =false;		// must use exponent even when it is 0 (sci or eng notation)
	switch(sres.decFormat)             
    {
		case dfGeneral:	s = _RoundStr(s, sres.num_digits);
						break;

		case dfEng:
		case dfSci:	   _FormatSciOrEng(s,exp,posd,sres.decFormat); // first pass before rounding modifies 's'
						wstmp = _RoundStr(s, sres.num_digits);     
						posd1 = wstmp.find_first_of(decpoint);
						if(posd1 == string::npos || posd1 == posd+1)  //rounding may have caused a carry: e.g. 9.99 -> 10.0
						{
							if (posd1 == posd + 1)
								++posd;
							else
								posd = posd1;	
							_FormatSciOrEng(wstmp,exp, posd, sres.decFormat); // second pass before rounding
							wstmp = _RoundStr(wstmp, sres.num_digits);     
						}
						s = wstmp;
						doexp = true;
					   break;
		default:		// dfFixed
			break;
	}
	if(sign == '-')
		s = PCHAR("-") + s;
	if(exp || doexp)
	{
		ss.clear();
		ss.str(PCHAR(""));
		ss << exp;
		s += PCHAR("e")+ss.str();
	}
	return s;

}

/*============================================================
 * TASK : modify floating decimal number with exponent of 10
 *       in ws used in GraphicText output
 *       Example: 1.2e-013 => 1.2 <wchar_t(183)> 10^{-13}
 *-----------------------------------------------------------*/
static STRING _BeautifyDec(STRING s, BEATUFY_MODE mode = bmoHtml)
{
	int pos = s.find_first_of(_CHART('e'));
	if(pos != STRING::npos)
	{
		STRING ws = s.substr(0,pos), // mantissa
			    sign;
		size_t i = pos+1;
		if(s[i] == _CHART('-') || s[i] == _CHART('+') )	// may start with a sign
		{
			if(s[i] == L'-')	// drop
				sign = PCHAR("-");
			++i;
		}
		// skip leading 0's in exponent
		for(  ; i < s.length()-1 && (s[i] == _CHART('0')); ++i )
			;
		s = s.substr(i);  //exponent
		if(mode == bmoGraphText)
			s = ws + _CHART(183) + PCHAR("10^{") + sign + s + PCHAR("}");
		else if (mode == bmoHtml)
			s = ws + _CHART(183) + PCHAR("10<sup>") + sign + s + PCHAR("</sup>");
		else // if (mode == bmoTEX)
			s = ws + PCHAR(" \\cdot 10^{") + sign + s + PCHAR("}");
	}
	return s;
}

/*============================================================
 * TASK: convert number to decimal number with the required 
 *       precision and form in 'r'
 * EXPECTS: r is set up
 * RETURNS: string representation of number
 *-----------------------------------------------------------*/
static STRING _NumberToDec(STRING_RESULT &r)
{
	STRING s;
   	std::locale loc = std::cout.getloc();
	_CHART decpoint = numeric::constants.decpoint;

	s = _NumberToDecString(r, 0);
    if(r.decFormat == dfEng)    // then we got sci format in 's'
    {
        size_t posE = s.find_first_of(_CHART('e')), // must be present in SCI format STRING
               posD = s.find_first_of(decpoint);    // this may be missing
        long n = _wtol(s.substr(posE+1).c_str());    // exponent
        int shift = n % 3;
        if(shift) // then modify STRING AND exponent
        {
            n -= shift; // new exponent

        }
    }
	if(r.chThousandSep) // reformat STRING
	{
		size_t pos = s.find_first_of(decpoint); // only reformat integer part
		STRING sd;                             // left of the decimal point
		if(pos == STRING::npos)           // or if no decimal point then
			pos = s.find_first_of('e');			// before the exponent

		if(pos != STRING::npos)
		{
			sd = s.substr(pos);					// decimal digits
			s = s.substr(0,pos);				// integer part
		}
		size_t j = 0;
		for(int i = (int)s.length()-1; i >=0; --i)
		{
			sd = s[i] + sd;
			if( (++j % 3) == 0 && (i != 0) && (s[i-1] != '-') )
				sd = r.chThousandSep + sd;
		}
		s = sd;
	}
	return s;
}

/*============================================================
 *
 *-----------------------------------------------------------*/
static STRING _NibbleToHDigit(unsigned ch)
{
	const STRING digits=PCHAR("0123456789ABCDEF");
	return digits.substr(ch,1);
}

static STRING _ByteToHexString(_CHART ch)
{
	return _NibbleToHDigit(ch >> 4) + _NibbleToHDigit(ch & 0xF);
}

static STRING _ShowAs(STRING &snum, STRING_RESULT &r)
{
	if(snum[0] == 'T')	// C>f. Too long
		return snum;

	size_t shLen;		// asBytes: 2. asWords 4, asDWords: 8
	size_t len = snum.length();
	switch(r.showAs)
	{
		default:
		case shaNormal: return PCHAR("0x") + snum; // little/big endiannes doesn't matter
		case shaBytes: shLen = 2;
					  break;
		case shaWords: shLen = 4;
					  break;
		case shaDWords: shLen = 8;
					  break;
	}
	size_t dl = (len % shLen);
    dl = dl ? shLen - dl : 0; // length to be filled by '0's
	for(size_t i = 0; i < dl; ++i)
		snum = PCHAR("0") + snum;
	// now the STRING length is divisable with the unit size
	len += dl; // get new STRING length
	STRING s1;

    dl = shLen >> 1;    // re-use as number of bytes in shLen unit
	for(size_t i = 0; i < len; i += shLen)
	{
		if(r.endian == heLittle) // then the STRING grows from right to left
		{
			if(i && i != len)
					s1 =PCHAR(" ") + s1;
            for(size_t j = 0; j < dl; ++j)
    			s1 = snum.substr(i+2*j, 2) + s1;
		}
		else                     // then the STRING grows from left to right
		{
			if(i && i != len)	// not the last 2 digits
				s1 += PCHAR(" ");
			s1 += snum.substr(i,shLen);
		}
	}
	return s1;
}
static STRING _NumberToHex(STRING_RESULT &r)
{
    RealNumber val = r.value;	//save value 
    bool negative = false;
	if(r.hexSign && val < 0)
	{
		negative = true;
		r.value = -val;
	}
    else
        negative = false;

	size_t charsInUnit = r.num_digits,
		   size        = sizeof(float);		// single prec IEEE
	float d = (float)val;              // truncate if necessary
	unsigned char *pl;
	STRING s;

	if(r.hexFormat == hfNormal )
	{
		STRINGSTREAM ss;
		long long ll = (long long)r.value;
		ss << hex << uppercase << ll;
		ss >> s;
		s = _ShowAs(s,r);
        if(negative)
            s = PCHAR("-")+s;
		r.value = val;		// restore value
        return s;
	}
	else
	{
		switch(r.hexFormat)
		{
		case hfSingle: pl = (unsigned char *)&d;
			break;
		case hfDouble: pl = (unsigned char *)&val;
					   size = sizeof(RealNumber);
			break;

		default: break;
		}

		unsigned char ch = *pl++;  // little endian on INTEL platform
		s = _ByteToHexString(ch);
		if(r.showAs != shaNormal) // then create a big endian
		{						 // representation
			for(size_t i = 1; i < size; ++i)
			{
				ch = *pl++;
				s = _ByteToHexString(ch) + s;
			}
			return _ShowAs(s, r); // and show as big or little endian
		}
		else // show as big or little endian STRING
		{
			for(size_t i = 1; i < size; ++i)
			{
				ch = *pl++;
				if(r.endian == heBig)
					s = _ByteToHexString(ch) + s;
				else
					s = s + _ByteToHexString(ch);
			}
		}
	}
    if(negative)
        s = PCHAR("-")+s;

	r.value = val;		// restore value
	return s;
}

static STRING _NumberToOct(STRING_RESULT &r)
{
    RealNumber val = r.value;
    bool negative = false;
	if(r.hexSign && val < 0)
	{
		negative = true;
		val = -val;
	}
    else
        negative = false;
	STRINGSTREAM ss;
	ss << PCHAR("0") << oct << (long long) r.value;
	STRING s;
	ss >> s;
    if(s[0] == '-' || s[0] == '+' || isdigit(s[0]))
    {
        if(r.chThousandSep )
        {
            size_t j = 0;
            STRING sd;
            for(int i = (int)s.length()-1; i >=0; --i)
            {
                sd = s[i] + sd;
                if( (++j % 3) == 0 && (i != 0) && (s[i-1] != '-') )
                    sd = L' ' + sd;  // r.chThousandSep + sd;
            }
            s = sd;
        }
        if(r.chThousandSep && (s.length() > 4 && ((s.length() % 4) != 3)))
            s = STRING(3 - (s.length() % 4), '0') + s;
        if(negative)
            s = PCHAR("-")+s;
    }
    return s;
}

// convert to unsigned binary manually (no predefined conversion)
static STRING _NumberToBin(STRING_RESULT &r)
{
    RealNumber val = r.value;
    bool negative = false;
	if(r.hexSign && val < 0)
	{
		negative = true;
		val = -val;
	}
    else
        negative = false;
    STRING s;
	if(abs(val) < 1)
		return (STRING)PCHAR("0");
	if(abs(val) > 9223372036854775807.0) // (RealNumber)0x7FFFFFFFFFFFFFFFL)
		return (STRING)PCHAR("Too long");
	unsigned long long ll = (unsigned long long)floorl(val); // may be problems with big numbers
    int i = 0;
    do
    {
        s = ((ll & 1) ? PCHAR("1") : PCHAR("0")) + s;
        ++i;
        if(r.chThousandSep && (i%4 == 0) )
            s = PCHAR(" ") + s;
        ll >>= 1;
    } while(ll);
    if(r.chThousandSep && (s.length() > 5 && ((s.length() % 5) != 4)))
        s = STRING(4 - (s.length() % 5), '0') + s;
    i = r.chThousandSep ? 56 : 48;     // 64 binary digits - 8 bytes
    if(r.chThousandSep &&  (int)s.length() > i)
        return (STRING)PCHAR("Too long");
    s = PCHAR("#")+ s;
    if(negative && (s[0] == '-' || s[0] == '+' || isdigit(s[0])) )
        s = PCHAR("-")+s;
    return s;
}

STRING STRING_RESULT::ToString(RealNumber val)
{
	if(val != val )	// Nan() ?
		;
	else
		value = val;

	switch(type)
	{
		case stDecimal:
			return _NumberToDec(*this);
		case stDecBeautified:
			return value == 0.0 ? PCHAR("0") : _BeautifyDec(_NumberToDec(*this), mode);

		case stHex:
			return _NumberToHex(*this);
		case stOct:
			return _NumberToOct(*this);
		case stBin:
			return _NumberToBin(*this);
		default: break;
	}
	return PCHAR("");
}

#ifdef _MSC_VER
# ifdef __cplusplus_cli
    STRING SystemStringToSTLString(System::STRING^ S)
	{
		STRING s;
		msclr::interop::marshal_context context;
		s = context.marshal_as<STRING>(S);
		return s;
	}
    System::STRING^ STLStringToSystemString(STRING s)
	{
 	    msclr::interop::marshal_context context;
		return context.marshal_as<System::STRING^>(s);
	}
    System::STRING^ STLStringToSystemString(const _CHART*s)
	{
		 msclr::interop::marshal_context context;
		return context.marshal_as<System::STRING^>(s);
	}
# elif defined _MFC_VER
#   if 1
STRING SystemStringToSTLString(const LPTSTR str)	
{	
        char buf[1024];
        wcstombs(buf, str, wcslen(str)+1);
        STRING s(buf);
        return s;
}
STRING SystemStringToSTLString(STRING S)	{	return S; }
LPTSTR  STLStringToSystemString(STRING s, LPTSTR buf)	
{	
	mbstowcs(buf, s.c_str(), s.length()+1);
	return buf; 
}
LPTSTR  STLStringToSystemString(const _CHART*s, LPTSTR buf)	
{
	mbstowcs(buf, s, wcslen(buf)+1);
	return buf; 
}
#   else
    STRING SystemStringToSTLString(CString S)
	{
        char buf[1024];
        wcstombs(buf, S, S.Len()+1);
        STRING s(buf);
        return s;
	}
    CString STLStringToSystemString(STRING s)
	{
        wchar_t wbuf[1024];
        mbstowcs(wbuf, s.c_str(), s.length()+1);
        CString us = wbuf;
        return us;
	}
    CString STLStringToSystemString(const _CHART*s)
	{
        wchar_t wbuf[1024];
        mbstowcs(wbuf, s, strlen(s)+1);
        CString us = wbuf;
        return us;
	}
#   endif
# endif
#elif defined(__BORLANDC__)
    STRING SystemStringToSTLString(UnicodeString us)
    {
//        wchar_t buf[1024];
//        wcstombs(buf, us.w_str(), us.Length()+1);
//        STRING s(buf);
//        return s;
        return us.w_str();
    }
    UnicodeString STLStringToSystemString(STRING s)
    {
//        wchar_t wbuf[1024];
//        mbstowcs(wbuf, s.c_str(), s.length()+1);
//        UnicodeString us = s; // wbuf;
        return s.c_str(); //us;
    }
    UnicodeString StringToSystemString(const _CHART* s)
    {
//        wchar_t wbuf[1024];
//        mbstowcs(wbuf, s, strlen(s)+1);
        UnicodeString us = s;
        return us;
    }
	#pragma package(smart_init)
#endif
//---------------------------------------------------------------------------

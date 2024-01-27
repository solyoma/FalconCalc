#pragma once

#ifndef _SMARTSTRING_H
	#define _SMARTSTRING_H

#include <iostream>
#include <limits>
#include <locale>
#include <codecvt>
#include <string>
#include <vector>
#include <iterator>
#include <cstddef>

namespace SmString {

typedef std::string UTF8String;
typedef std::u16string String;

class SCharT
{
	char16_t _unicode;

	int _Utf8CodeLengthFrom(const UTF8String& u8str, size_t pos);  // -1: error
	int _FromString(const UTF8String& u8str, size_t pos);

	friend class SmartString;	//unicode string of SChars
public:
	SCharT() = default;
	explicit SCharT(char ch)	: _unicode((unsigned char) ch)	{}
	explicit SCharT(int ch)		: _unicode((unsigned) ch)		{}
	explicit SCharT(long ch)	: _unicode((char16_t) ch)		{}
	SCharT(char16_t sch): _unicode(sch)				{}
	explicit SCharT(wchar_t wch) : _unicode((char16_t)wch)		{}
	explicit SCharT(UTF8String& s, int pos = 0)	 // unicode character from utf8 encoded std::string
	{
		_FromString(s, pos);
	}

	//explicit operator String() const
	//{
	//	return String(1, _unicode);
	//}

	operator char16_t() const
	{
		return _unicode;
	}

	SCharT& operator=(size_t n) { _unicode=(char16_t)n; return *this; }
	SCharT& operator=(int n) { _unicode=(char16_t)n; return *this; }
	SCharT& operator+=(const SCharT n) { _unicode+=n._unicode; return *this; }
	SCharT& operator-=(const SCharT n) { _unicode-=n._unicode; return *this; }
	SCharT  operator+ (const SCharT n) const { SCharT sch=*this; sch._unicode += n._unicode; return sch; }
	SCharT  operator- (const SCharT n) const { SCharT sch=*this; sch._unicode -= n._unicode; return sch; }

	SCharT operator++() { ++_unicode; return *this; }
	SCharT operator++(int)	{ SCharT p(_unicode); ++_unicode; return p; }
	SCharT operator--()		{ --_unicode; return *this;}
	SCharT operator--(int)	{ SCharT p(_unicode); --_unicode; return p; }

	bool operator<(const SCharT& sch)  const { return _unicode < sch._unicode; }
	bool operator>(const SCharT& sch)  const { return _unicode > sch._unicode; }
	bool operator<=(const SCharT& sch) const { return _unicode <= sch._unicode; }
	bool operator>=(const SCharT& sch) const { return _unicode >= sch._unicode; }
	bool operator!=(const SCharT& sch) const { return _unicode != sch._unicode; }
	bool operator==(const SCharT& sch) const { return _unicode == sch._unicode; }

	bool IsDigit() const { return std::isdigit(_unicode, std::cout.getloc()); }
	bool IsAlnum() const { return std::isalnum(_unicode, std::cout.getloc()); }
	bool IsAlpha() const { return std::isalpha(_unicode, std::cout.getloc()); }


	constexpr char16_t Unicode() const { return _unicode; }
	constexpr explicit operator int() { return (int)(unsigned int)_unicode; }

	UTF8String ToUtf8String() const;

	SCharT ToUpper() { _unicode = std::toupper(_unicode, std::cout.getloc()); return *this;}
	SCharT ToLower() { _unicode = std::tolower(_unicode, std::cout.getloc()); return *this;}
	SCharT ToUpper(std::locale loc) { _unicode = std::toupper(_unicode, loc); return *this;}
	SCharT ToLower(std::locale loc) { _unicode = std::tolower(_unicode, loc); return *this;}
};

using UTF8Pos = size_t;			// when used for unicode position (no unicode character in string: same as position

// An UTF16 String class with some convenience functions not present in the STL
// wchar_t's size is 16 bit in Windows and 32 bit in Linux, but this class is always uses 16 bit
// Besides the usual concatenation and length functions it contains
// left(), right(), mid() with an optional fill character
// normal and regular expression search and toUpper() and toLower() functions
// it can contain utf-8 characters and convert to wide string
//
// The length() function returns the length in code points, the size() function returns
// the number of bytes that makes up the string
class SmartString : public String
{
public:
	using Iterator = String::iterator;		// std::basic_string<SCharT>::iterator;
	using Const_Iterator = String::const_iterator;// std::basic_string<SCharT>::const_iterator;
public:
	SmartString() : String() {}
	explicit SmartString(const UTF8String s);
	explicit SmartString(const char* pcstr);
	explicit SmartString(const std::wstring ws);
	explicit SmartString(const wchar_t* pws) :SmartString(std::wstring(pws)) {}
	SmartString(const SmartString& o) :String(o) {}
	SmartString(const SmartString& o, size_t pos, size_t len) { *this = mid(pos, len); }
	SmartString(const String &s) : String(s){}
	SmartString(SCharT ch) : SmartString(1, ch){}
	SmartString(const SCharT* ps) : String((char16_t*)ps) {}
	SmartString(const SCharT* ps, size_t len) :SmartString(ps) {}
	SmartString(size_t len, SCharT ch) : String(len, ch)  { }
	SmartString(String::iterator from, String::iterator to) : String(from, to) { }
	~SmartString() {}

	SmartString& operator=(const SmartString s);
	SmartString& operator=(const UTF8String s);
	SmartString& operator=(const std::wstring ws);
	SmartString& operator=(const char *pcs);
	SmartString& operator=(const wchar_t *pwcs);

	SmartString& operator +=(const SmartString s);
	SmartString& operator +=(const UTF8String s);
	SmartString& operator +=(SCharT ch);
	SmartString& operator +=(const std::wstring ws);

	bool operator==(const SmartString& s);
	bool operator!=(const SmartString& s);

	SCharT at(size_t pos, SCharT defch = SCharT(0) ) const;

	SmartString left( UTF8Pos n, SCharT fillChar = SCharT(-1)) const; // may extend the string to the right
	SmartString right(UTF8Pos n, SCharT fillChar = SCharT(-1)) const;	// may extend the string to the left
	// similar to substr but may extend the string with a given character too
	SmartString mid(UTF8Pos pos, size_t n = String::npos, SCharT fillChar = SCharT(-1) ) const;
	int indexOf(const SCharT ch, size_t pos = 0) const;			// start from 'pos'
	int indexOf(const SmartString ns, size_t pos = 0) const;	// only first occurance
	int indexOfRegex(const SmartString regexpString, size_t pos = 0) const;		// only first occurance
	void toUpper();
	void toLower();
	SmartString asUpperCase() const { SmartString s = *this; s.toUpper(); return s; }
	SmartString asLowerCase() const { SmartString s = *this; s.toLower(); return s; }

	void FromUtf8String(const UTF8String& ws);
	UTF8String ToUtf8String() const;
	void FromWideString(const std::wstring& ws);
	std::wstring ToWideString() const;

	void lTrim();
	void rTrim();
	void Trim();

	std::vector<SmartString> Split(const SCharT ch, bool keepEmpty);
	std::vector<SmartString> SplitRegex(const SCharT ch, bool keepEmpty);	// TODO

	void Reverse(); // order of characters
};
// literal operator for string constants like "1234"ss

const SmartString operator"" _ss(const char* ps, size_t len);

 // end namesapce SmString
}

std::ostream& operator<<(std::ostream& ofs, SmString::SmartString ss);

#endif	// _SMARTSTRING_H

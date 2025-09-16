#include <stdint.h>
#include <regex>
#include <cwchar>
#include <iostream>
#include <fstream>

#ifdef QTSA_PROJECT
	#include <QString>
#endif
#include "SmartString.h"


namespace SmString {

	static bool __bCaseSens = false;
	static int _sorter(const void* left, const void* right)
	{
		return __bCaseSens == SmartString::CaseSens::csCaseSensitive ?
									(SmartString*)left < (SmartString*)right :
									((SmartString*)left)->asLowerCase() < ((SmartString*)right)->asLowerCase();

	}


	// ************************** SCharT *********************

	int SCharT::_Utf8CodeLengthFrom(const UTF8String& u8str, size_t pos)
	{
		size_t c, siz = u8str.size(), x=(unsigned)pos;
		int len = -1;

		if ((unsigned)pos >= siz)
			return -1;

		c = (unsigned char)u8str.at(pos);

		if (c < 0x80) len = 1; // 0bbbbbbb
		else if ((c & 0xE0) == 0xC0) len = 2; // 110bbbbb
		else if (c == 0xed && x < (siz - 1) && ((unsigned char)u8str.at(x + 1) & 0xa0) == 0xa0) // invalid UTF8
			return -1;  //U+d800 to U+dfff
		else if ((c & 0xF0) == 0xE0) len = 3; // 1110bbbb
		else if ((c & 0xF8) == 0xF0) len = 4; // 11110bbb
		//else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
		//else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
		else
			return -1;
		// check all bytes for an utf8 code point

		size_t i = pos;
		for (size_t j = 0; j < (unsigned)len - 1 && i < siz; j++)  // n bytes matching 10bbbbbb follow ?
		{
			if ((++i == siz) || (((unsigned char)u8str.at(i)) & 0xC0) != 0x80)
				return -1;
		}
		return len;
	}

	int SCharT::_FromString(const UTF8String& u8str, size_t pos)
	{
		int len = _Utf8CodeLengthFrom(u8str, pos);
		if (len < 0)
			return len;
		if (len == 1)
			_unicode = u8str.at(pos);
		else
		{
			UTF8String ustr = u8str.substr(pos, len);
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			std::wstring wstr = converter.from_bytes(ustr);
			_unicode = wstr[0];
		}
		return len;
	}

	UTF8String SCharT::toUtf8String() const
	{
		wchar_t wch = _unicode;
		if (wch < 0x7F)
			return UTF8String(1, char(wch));

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return  converter.to_bytes(wch);
	}



	//*************************** SmartString ****************

	UTF8String SmartString::toUtf8String() const
	{								
		UTF8String s;
		for (size_t i = 0; i < length(); ++i)
		{
			s += SCharT((*this)[i]).toUtf8String();
		}
		return s;
	}

	std::wstring SmartString::ToWideString() const
	{
		std::wstring ws;
		ws.resize(length());
		for (size_t i = 0; i < size(); ++i)
			ws[i] = SCharT((*this)[i]).Unicode();
		return ws;
	}

#ifdef QTSA_PROJECT
	void SmartString::FromQString(const QString& qs)
	{
		*this = SmartString(qs);
	}

	QString SmartString::toQString() const
	{
		QString qs;
		qs.resize(length());
		for (size_t i = 0; i < size(); ++i)
			qs[i] = QChar((*this)[i]);
		return qs;
	}
#endif

	void SmartString::FromWideString(const std::wstring& ws)
	{
		resize(ws.length());
		for (size_t i = 0; i < size(); ++i)
			(*this)[i] = ws[i];
	}

	void SmartString::FromUtf8String(const UTF8String& us)
	{
		clear();
		SCharT sch;
		int l;
		for (size_t i = 0; i < us.length(); ++i)
		{
			l = sch._FromString(us, i) - 1;
			push_back(sch);
			if (l > 0)
				i += l;
		}
	}

	void SmartString::LTrim()
	{
		erase(begin(), std::find_if(begin(), end(), [](SCharT ch) {return !std::isspace((wchar_t)ch.Unicode(), std::cout.getloc()); }));
	}

	void SmartString::RTrim()
	{
		erase(std::find_if(rbegin(), rend(), [](SCharT ch) {return !std::isspace((wchar_t)ch.Unicode(), std::cout.getloc()); }).base(), end());
	}

	void SmartString::Trim()
	{
		LTrim();
		RTrim();
	}

	void SmartString::RemoveWhiteSpace()
	{
		erase(std::remove_if(begin(), end(), [](SCharT ch) {return std::isspace((wchar_t)ch.Unicode(), std::cout.getloc()); }), end());
	}

	SmartStringVector SmartString::Split(const SCharT ch, bool keepEmpty) const
	{
		SmartStringVector sv;
		size_t pos0 = 0;
		int pos;
		while ((pos = indexOf(ch,pos0)) >=0) 
		{
			if((size_t)pos != pos0 || keepEmpty)
				sv.push_back(mid(pos0, (size_t)pos - pos0));
			pos0 = (size_t)++pos;
		}
		if(pos0 < length() || (pos0 == length() && keepEmpty))
			sv.push_back(mid(pos0));

		return sv; ;
	}

	SmartStringVector SmartString::SplitRegex(const SmartString regexStr, bool keepEmpty) const
	{
		std::wstring ws = ToWideString(),
					 rx = regexStr.ToWideString();
		std::wsmatch matches;
		std::wregex re(rx);
		std::wsregex_token_iterator iti(ws.cbegin(), ws.cend(), re, -1);
		std::wsregex_token_iterator end;

		SmartStringVector sv;
		SmartString ss;
		while (iti != end) 
		{
			ss = SmartString(iti->str());
			if(ss.size() || keepEmpty)
				sv.push_back(ss);
			++iti;
		}
		return sv;
	}

	SmartString::SmartString(const char* pcstr)
	{
		UTF8String s(pcstr);
		*this = s;
	}

#ifdef QTSA_PROJECT
	SmartString::SmartString(const QString& qs)
	{
		resize(qs.length());
		for (size_t i = 0; i < length(); ++i)
			(*this)[i] = qs[i].unicode();
	}
#endif

	SmartString::SmartString(const UTF8String &s)
	{
		SCharT sch;
		int l = 0;
		for (size_t i = 0; i < s.length(); ++i)
		{
			l = sch._FromString(s, i) - 1;
			/*String::*/ push_back(sch);
			if (l > 0)
				i += l;
		}
	}

	SmartString::SmartString(std::wstring ws)
	{
		resize(ws.length());
		for (size_t i = 0; i < length(); ++i)
			(*this)[i] = ws[i];
	}

	SmartString& SmartString::operator=(const std::wstring ws)
	{
		FromWideString(ws);
		return *this;
	}
	SmartString& SmartString::operator=(const char* pcs)
	{
		std::string s(pcs);
		FromUtf8String(s);
		return *this;
	}
	SmartString& SmartString::operator=(const wchar_t* pwcs)
	{
		std::wstring ws(pwcs);
		FromWideString(ws);
		return *this;
	}
#ifdef QTSA_PROJECT
	SmartString& SmartString::operator=(const QString qs)
	{
		return *this = qs.toStdU16String();
	}
#endif
	SmartString& SmartString::operator=(const SmartString s)
	{
		if(this != &s)
			*(String*)this = (const String&)s;
		return *this;
	}
	SmartString& SmartString::operator=(const UTF8String s)
	{
		FromUtf8String(s);
		return *this;
	}
	SmartString& SmartString::operator+=(SCharT ch)
	{
		String::operator+=(String(1, ch));
		return *this;
	}

	SmartString& SmartString::operator+=(const SmartString s)
	{
		size_t pos = length();
		if (s.length())
		{
			resize(size() + s.length());
			for (size_t i = 0; i < s.length(); ++i)
				(*this)[pos++] = s[i];
		}
		return *this;
	}

	SmartString& SmartString::operator+=(const UTF8String s)
	{
		//std::wstring ws = SmartString(s).ToWideString();
		//return operator+=(ws);
		return operator+=(SmartString(s));
	}

	SmartString& SmartString::operator+=(const std::wstring ws)
	{
		operator+=(SmartString(ws));
		return *this;
	}

	SCharT SmartString::at(size_t pos, SCharT defch) const
	{
		if (pos >= size())
			return defch;
		return String::at(pos);
	}

	SmartString SmartString::left(UTF8Pos n, SCharT fillChar) const // may extend the string to the right
	{
		if (n == npos || n == length())	// full string or outside of string
			return *this;

		String s;
		if (n < length())
			s = substr(0, n);
		else if (fillChar >= SCharT(0) )
			s = *(String*)this + String(n - length(), fillChar);

		return s;
	}
	SmartString SmartString::right(UTF8Pos n, SCharT fillChar) const	// may extend the string to the left
	{
		if (n > length())	// must extend string to the left with fillchar
		{
			if (fillChar <= SCharT(0))
				return *this;
			n -= length();
			return SmartString(n, fillChar) + *this;
		}
				// else	just get the n unicode character
		return substr(n);
	}

	bool SmartString::operator==(const SmartString& s) const
	{
		return String(*this) == String(s);
	}

	bool SmartString::operator!=(const SmartString& s) const
	{
		return String(*this) != String(s);
	}

	int SmartString::CompareWith(const SmartString s, CaseSens caseSensitivity) const
	{
		SmartString left(*this), right(s);
		if (caseSensitivity == csCaseInsensitive)
		{
			left.toLower();
			right.toLower();
		}
		if (String(left) < String(right))
			return -1;
		if (String(left) == String(right))
			return 0;
		return 1;
	}

	// similar to substr but may extend the string with a given character 
	SmartString SmartString::mid(UTF8Pos pos, size_t n, SCharT fillChar) const
	{
		if (!pos && n == npos)
			return *this;

		if (n == npos)				//  special case: no fill!
			return substr(pos);

		size_t limit = pos + n;	// first character after the string
		if (limit <= length())
			return substr(pos,n);  // xxxx xxXx

		return substr(pos) + SmartString(limit-length(), fillChar);
	}
	int SmartString::indexOf(const SCharT ch, size_t pos) const		// start from 'pos'
	{
		size_t i = find_first_of(ch, pos);
		return (i == String::npos) ? -1 : (int)i;
	}
	int SmartString::indexOf(const SmartString ns, size_t pos) const		// only first occurance
	{
		size_t ix = pos, len = length();
		size_t is = 0, nslen=ns.length();
		for (size_t i = pos; i < len && i + is < nslen; ++i)
		{
			if ((*this)[i] != ns[is])
			{
				is = 0;
				continue;
			}
			else if (is == 0)
				ix = i;
			if (++is == nslen)
				return (int)ix;
		}
		return -1;
	}
	int SmartString::indexOfRegex(const SmartString regexString, size_t pos) const		// only first occurance
	{
#if 1
		std::wstring ws = mid(pos).ToWideString(),
					 rx = regexString.ToWideString();
		std::wsmatch matches;
		std::wregex re(rx);
		if (std::regex_search(ws, matches, re))
			return (int)matches.position(0);
		else
			return -1;
#else
#ifdef _MSC_VER
		using regex_schr_traits = std::_Regex_traits <SCharT>;
#else
		using regex_schr_traits = std::regex_traits <SCharT>;
#endif

		regex_schr_traits tr;
		std::basic_regex <SCharT, regex_schr_traits> re(regexString.data());
		std::match_results<SCharT> matches;
		SmartString s = mid(pos);
		if (std::regex_search(s, matches, re))
			return (int)matches.position(0);
		else
			return -1;
#endif
	}
	SmartString& SmartString::toUpper()
	{
		std::locale loc = std::cout.getloc();
		for (auto& ch : *this)
			ch = (char16_t)std::toupper((wchar_t)ch, loc);
		return *this;
	}
	SmartString& SmartString::toLower()
	{
		std::locale loc = std::cout.getloc();
		//std::locale loc("HU_hu");
		for (auto& ch : *this)
			ch = (char16_t)std::tolower((wchar_t)ch, loc);
		return *this;
	}

	void SmartString::Reverse() // order of characters
	{								// but characters in chunks are kept in the original order
		std::reverse(begin(), end()); // in algorithm
	}
	// end of namespace SmString

// literal operator for string constants like "1234"_ss (literal operators w.o. _ at the front are not to be used)

	const SmartString operator""_ss(const char* ps, size_t len) 
	{ 
		return SmartString(ps); 
	}

	SmartStringVector::SmartStringVector(const SmartString s, const SCharT ch, bool keepEmpty, bool trim=false)
	{
		*this = s.Split(ch, keepEmpty);
		if (trim)
			for (size_t i = 0; i < size(); ++i)
				(*this)[i].Trim();
	}
	SmartStringVector::SmartStringVector(const SmartString s, SmartString regex, bool keepEmpty, bool trim=false)
	{
		*this = s.SplitRegex(regex, keepEmpty);
		if (trim)
			for (size_t i = 0; i < size(); ++i)
				(*this)[i].Trim();
	}
	void SmartStringVector::SetSorted(bool setSorted)
	{
		_isSorted = setSorted;
		if (setSorted)
			_Sort();
	}
	void SmartStringVector::Add(SmartString s)	// no matter if it is sorted
	{
		int ix = IndexOf(s);
		if (ix == 0)
			return;
		if (ix > 0)
			Delete(ix);
		insert(begin(), s);
	}

	bool SmartStringVector::LoadFromFile(SmartString name)
	{
		std::ifstream ifs(name.toUtf8String(), std::ios_base::in);
		if(ifs.fail())
			return false;
		std::string line;
		bool sorted = _isSorted;
		sorted = false;
		while (std::getline(ifs, line))
			push_back(SmartString(line) );
		SetSorted(sorted);	// sort if needed
		return true;
	}

	bool SmartStringVector::SaveToFile(SmartString name)
	{
		std::ofstream ofs(name.toUtf8String(), std::ios_base::out);
		if(ofs.fail())
			return false;
		for (auto& s : *this)
			ofs << s << "\n";
		return true;
	}

	int SmartStringVector::IndexOf(const SmartString s)
	{
		SmartString::CaseSens caseSens = _caseSens;

		for (size_t i = 0; i < size(); ++i)
			if (!s.CompareWith((*this)[i], caseSens))
				return i;
		return -1;
	}

	std::vector<std::wstring> SmartStringVector::ToWstringVector() const
	{
		std::vector<std::wstring> ws;
		for (auto s : *this)
			ws.push_back(s.ToWideString());
		return ws;
	}

	void SmartStringVector::_Sort()
	{		   // do not sort the first item
		__bCaseSens = _caseSens == SmartString::csCaseSensitive;
		qsort((void*) & this[1], size(), sizeof(SmartString), _sorter);
	}
}
// output operator
std::ostream& operator<<(std::ostream& ofs, SmString::SmartString ss)
{
	ofs << ss.toUtf8String();
	return ofs;
}


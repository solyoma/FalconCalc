#pragma once
#ifndef _COMMON_H
	#define _COMMON_H
// include in windows project before stdafx_lc.h
// and exclude tstdafx_lc.h for Qt projects

#include <vector>
#include <string>
#include <algorithm>

#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <iostream>

#include <memory>
#include <math.h>

#define pow10(a)	pow((RealNumber)10,((RealNumber)a))


enum AppLanguage { lanNotSet, lanEng, lanHun };

//-------------- trim leading and trailing delimiters from a string or wstring 


// ------------- returns trimmed string but leave the original intact
inline std::wstring trim_right_copy(const std::wstring& s, const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return s.substr(0, s.find_last_not_of(delimiters) + 1);
}

inline std::wstring trim_left_copy(const std::wstring& s, const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return s.substr(s.find_first_not_of(delimiters));
}

inline std::wstring trim_copy(const std::wstring& s, const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return trim_left_copy(trim_right_copy(s, delimiters), delimiters);
}


// ----------    trim wstring in place and returns trimmed string
inline std::wstring& trim_right_inplace(std::wstring& s, const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return s.erase(s.find_last_not_of(delimiters) + 1);
}

inline std::wstring& trim_left_inplace(std::wstring& s, const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return s.erase(0, s.find_first_not_of(delimiters));
}

inline std::wstring& trim(std::wstring& s, const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return trim_left_inplace(trim_right_inplace(s, delimiters), delimiters);
}

#endif // _COMMON_H
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

//extern const char* STATE_ID_STRING	;
//extern const char* DAT_ID_STRING		;
//extern const char* FalconCalc_HIST_FILE;
//extern const char* FalconCalc_Dat_FILE;
//extern const char* FalconCalc_State_FILE;
//
//extern const int   VERSION_INT;
//extern const char* VERSION_STRING;
constexpr const char* STATE_ID_STRING = "FalconCalc State File V";
constexpr const char* DAT_ID_STRING = "FalconCalc Data File V";
constexpr const char* FalconCalc_Hist_File = "FalconCalc.hist";
constexpr const char* FalconCalc_Dat_File = "FalconCalc.dat";
constexpr const char* FalconCalc_State_File = "FalconCalc.cfg";

constexpr const int   VERSION_INT = 0x000900;
constexpr const char* VERSION_STRING = "0.9.0";

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
#pragma once

#ifndef _defines_H
#define _defines_H

#include <iostream>
#include <limits>
#include <cmath>
#include <locale>
#include <string>
#include <vector>

#pragma message("*** defines.h included")

#ifndef QTSA_PROJECT
	#include "SmartString.h"
#endif


#if defined  _UNICODE
		using FSTRING		=	std::wstring		 ;
		using STRINGSTREAM	=	std::wstringstream	 ;
		using ISTREAM		=	std::wistream		 ;
		using IFSTREAM		=	std::wifstream		 ;
		using OSTREAM		=	std::wostream		 ;
		using OFSTREAM		=	std::wofstream		 ;
		#define COUT			std::wcout
		#define CERR			std::wcerr
		#define CIN				std::wcin
		#define PCHAR(a)		L##a
		#define TO_STRING		to_wstring
# else
		#define FSTRING			std::string
		#define STRINGSTREAM	std::stringstream
		#define ISTREAM			std::istream
		#define IFSTREAM		std::ifstream
		#define OSTREAM			std::ostream
		#define OFSTREAM		std::ofstream
		#define COUT			std::cout
		#define CERR			std::cerr
		#define CIN				std::cin
		#define PCHAR(a)		a
		#define TO_STRING		to_string
#endif
#endif /* _defines_H */

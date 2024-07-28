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

# ifdef QTSA_PROJECT
	#include <QString> 

	using SmartString = QString;
	using SCharT = QChar;
	typedef std::string UTF8String;
	typedef std::u16string String;
#define TO_STRING(a)	QString().setNum(a)
# else
	#include "SmartString.h"
#   if defined  _UNICODE
		#define TO_STRING		std::to_wstring
#   else
		#define TO_STRING		std::to_string
#   endif

#endif

#endif /* _defines_H */

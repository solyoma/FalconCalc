#pragma once
#ifndef _STDAFX_LC_H
	#define _STDAFX_LC_H
// SA -------------
// SA----------------------
//#define _OLD_CODE
//#ifdef _OLD_CODE
//
//
//	#ifndef RealNumber
//		#define MYREAL double
//		#define RealNumber long double
//	#endif
//
//	#ifndef LONG_LONG
//		#define LONG_LONG int64_t
//	#endif
//	#include "numconsts.h"
//	#include "numconv.h"
//
//#else
//	#include "LongNumber.h"
//	using namespace LongNumber;
//	using _CHART = LongNumber::SCharT;
//	#ifndef _CHART
//		#define _CHART wchar_t
//	#endif
//#endif
	#ifndef _CHART
		#define _CHART wchar_t
	#endif
	namespace FalconCalc {}
	namespace nlib {}
// #include "calculate.h"


#include <vector>
#include <string>
#include <algorithm>

// /SA -------------
#ifdef _MSC_VER
#if defined(DESIGNING) || defined(LIBBUILD)
#pragma warning (disable : 4244)
#pragma warning (disable : 4250)
#pragma warning (disable : 4996)
#endif // DESIGNING
#include <SDKDDKVer.h>
#define EXIF_GUID_SUPPORTED
#else
#define _WIN32_IE  0x0610
#define _WIN32_WINNT 0x0610
#define WINVER 0x0610
#endif // _MSC_VER

#include <windows.h>

// in mingw
#ifdef __MINGW32__
#include <stdio.h>
#endif

#if defined(DESIGNING) || defined(LIBBUILD)
// C RunTime Header Files
#include <tchar.h>
#include <CommCtrl.h>
#ifdef _MSC_VER
#include <commoncontrols.h>
#endif

#include <string>
#include <sstream>
#include <algorithm>

#include <iomanip>
#endif // DESIGNING, LIBBUILD

#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <iostream>

#include <GdiPlus.h>

#include <memory>

#include <math.h>


#ifdef __MINGW32__
using std::type_info;
#include <limits.h>

#include "mingwdef.h"

#define FILESTD   wstd::
#else
#define FILESTD   std::
#endif

#include "general.h"

#ifdef __MINGW32__
#include "compfstream.h"
#endif

#ifdef __MINGW32__
#include <WinUser.h>
#include <Uxtheme_fixed.h>
#include <tmschema.h>
#else
#include <Uxtheme.h>
#include <vssym32.h>
#endif


#ifndef DESIGNING
#include "application.h"
#include "inifile.h"
#include "canvas.h"
#include "comdata.h"
#include "themes.h"
#include "objectbase.h"
#include "windowfunctions.h"
#include "controlbase.h"
#include "screen.h"
#include "imagelist.h"
#include "images.h"
#include "events.h"
#include "syscontrol.h"
#include "generalcontrol.h"
#include "graphiccontrol.h"
#include "buttons.h"
#include "dialog.h"
#include "filestream.h"
#include "gridbase.h"
#include "menu.h"
#include "wexception.h"
#endif

#include "TStringList.h"
#include "clipboard.h"


#ifdef _MSC_VER
   // missing functions:
  #define pow10(a)	pow((RealNumber)10,((RealNumber)a))


  #ifndef __cplusplus_cli
		#ifndef _SECURE_ATL
		#define _SECURE_ATL 1
		#endif

		#ifndef VC_EXTRALEAN
		#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
		#endif
     #ifndef _N_LIB
		#include "targetver.h"

		#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
     #endif
	 #ifdef _MFC_VER
		// turns off MFC's hiding of some common and often safely ignored warning messages
		#define _AFX_ALL_WARNINGS

		#include <afxwin.h>         // MFC core and standard components
		#include <afxext.h>         // MFC extensions


		#include <afxdisp.h>        // MFC Automation classes



		#ifndef _AFX_NO_OLE_SUPPORT
		#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
		#endif
		#ifndef _AFX_NO_AFXCMN_SUPPORT
		#include <afxcmn.h>             // MFC support for Windows Common Controls
		#endif // _AFX_NO_AFXCMN_SUPPORT

		#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
    #endif	
  #endif
    #ifdef _UNICODE
	    #if defined _M_IX86
    		#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
    	#elif defined _M_X64
    		#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
    	#else
    		#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
    	#endif
    #endif
#endif

extern  Clipboard *MyClipboard;  // in main.cpp

//-------------- trim leading and trailing delimiters from a string or wstring 

	// ------------- returns trimmed string but leave the original intact
inline std::wstring trim_right_copy(  const std::wstring& s,  const std::wstring& delimiters = L" \f\n\r\t\v" )
{
  return s.substr( 0, s.find_last_not_of( delimiters ) + 1 );
}

inline std::wstring trim_left_copy(  const std::wstring& s,  const std::wstring& delimiters = L" \f\n\r\t\v" )
{
  return s.substr( s.find_first_not_of( delimiters ) );
}

inline std::wstring trim_copy(  const std::wstring& s,  const std::wstring& delimiters = L" \f\n\r\t\v" )
{
  return trim_left_copy( trim_right_copy( s, delimiters ), delimiters );
}


	// ----------    trim wstring in place and returns trimmed string
inline std::wstring& trim_right_inplace( std::wstring&  s,  const std::wstring& delimiters = L" \f\n\r\t\v" )
{
  return s.erase( s.find_last_not_of( delimiters ) + 1 );
}

inline std::wstring& trim_left_inplace(  std::wstring&  s,  const std::wstring& delimiters = L" \f\n\r\t\v" )
{
  return s.erase( 0, s.find_first_not_of( delimiters ) );
}

inline std::wstring& trim( std::wstring& s,  const std::wstring& delimiters = L" \f\n\r\t\v" )
{
  return trim_left_inplace( trim_right_inplace( s, delimiters ), delimiters );
}

using namespace FalconCalc;

#endif
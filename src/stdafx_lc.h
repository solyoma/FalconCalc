#pragma once
#ifndef _STDAFX_LC_H
	#define _STDAFX_LC_H
	#ifndef _CHART
		#define _CHART wchar_t
	#endif
	namespace FalconCalc {}
	namespace nlib {}

// /SA -------------
#ifdef _MSC_VER
	#if defined(DESIGNING) || defined(LIBBUILD)
	#pragma warning (disable : 4244)
	#pragma warning (disable : 4250)
	#pragma warning (disable : 4996)
	#endif // DESIGNING
	#include <SDKDDKVer.h>
	#define EXIF_GUID_SUPPORTED
	#include <windows.h>
#elif defined(__MINGW32__)
	#define _WIN32_IE  0x0610
	#define _WIN32_WINNT 0x0610
	#define WINVER 0x0610
	#include <windows.h>
#endif // _MSC_VER


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

#include <GdiPlus.h>

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
#elif defined _MSC_VER
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

#include "SmartString.h"
#include "clipboard.h"


#ifdef _MSC_VER
   // missing functions:

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
	 #ifdef _MFCT_VER
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

using namespace FalconCalc;

#endif
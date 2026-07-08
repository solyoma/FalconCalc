//---------------------------------------------------------------------------
#ifndef numconv_H
	#define numconv_H

#if defined(__BORLANDC__)
    #include <vcl.h>
    #pragma hdrstop
#elif defined(_MSC_VER) && defined __cplusplus_cli
	#include <msclr\marshal_cppstd.h>
#endif

#include "numdefs.h"

#include <iostream>
#include <sstream>

RealNumber StringToNumber(STRING &str);

//inline RealNumber NaN() { return std::numeric_limits<RealNumber>::quiet_NaN(); }
inline RealNumber INF(int sign=1) { return sign*std::numeric_limits<RealNumber>::infinity(); }

enum SHOW_TYPE { stDecimal, stDecBeautified, stHex, stOct, stBin};		// base 10, 16, 8 and 2
enum BEATUFY_MODE { bmoGraphText, 			// 3.1e-5 => 3.1 dot 10^{-5}
					bmoHtml, 				// 3.1e-5 => 3.1 dot 10<sup>-5</sup>
					bmoTEX, 				// 3.1e-5 => 3.1 \cdot 10^{-5}
					bmoNone					// 3.1e-5 =>  3.1e-5
				  };
enum DEC_FORM { dfGeneral, dfFixed, dfSci, dfEng };		// general:either fixed or sci, fix: XX.YY, sci: XX.YYE+ZZZ, eng: XXX.yEzzz
enum HEX_FORM { hfNormal, hfSingle, hfDouble };			// hex. format: normal, as IEEE single, as IEEE double
enum HEX_ENDIAN { heBig, heLittle};                     // endiannes (intel: little, motorolla: big)
enum SHOW_HEX_AS { shaNormal, shaBytes, shaWords, shaDWords };
enum RES_VALID {rvOk, rvInvalid, rvDef };            // validity of result: OK, invalid, function, etc definition

struct STRING_RESULT
{
	RealNumber value;		// display this
    RES_VALID valid;        // set to 'rvOk' may be modified in the function calculating 'value'
	SHOW_TYPE type;
	BEATUFY_MODE mode;
	DEC_FORM decFormat;
	HEX_FORM hexFormat;
	HEX_ENDIAN endian;
	SHOW_HEX_AS showAs;
	bool hexSign;			// negative hex/oct/bin numbers printed the samas positive one, but with a - sign
	int  num_digits;	    // including integer and fractional part, -1: use default precision
	_CHART chThousandSep;	// thousand separator. 0 : nothing, other: character
                      // ::NaN() because some systems has a NaN() function
	STRING_RESULT() : value( ::NaN ), valid(rvOk), type(stDecimal), mode(bmoGraphText),
		              decFormat(dfGeneral), hexFormat(hfNormal),
					  endian(heBig), showAs(shaNormal), hexSign(false),
					  num_digits(-1), chThousandSep(0) {}
	STRING ToString(RealNumber val=::NaN);
};

#ifdef _MSC_VER
# ifdef __cplusplus_cli
	std::STRING SystemStringToSTLString(System::STRING ^s);
	System::STRING^ STLStringToSystemString(std::STRING s);
	System::STRING^ StringToSystemString(const wchar_t *s);
# elif defined _MFC_VER
#   if 1
	std::STRING SystemStringToSTLString(LPTSTR str);
//	std::STRING SystemStringToSTLString(std::STRING S);
	LPTSTR STLStringToSystemString(std::STRING s, LPTSTR buf);
	LPTSTR STLStringToSystemString(const wchar_t *s, LPTSTR buf);
#   else
	std::STRING SystemStringToSTLString(CString S);
	CString STLStringToSystemString(std::STRING s);
	CString STLStringToSystemString(const wchar_t *s);
#   endif
#elif defined _N_LIB
	// no SystemStrings !
# endif
#elif defined(__BORLANDC__)
	std::STRING SystemStringToSTLString(UnicodeString us);
	UnicodeString STLStringToSystemString(std::STRING s);
	UnicodeString StringToSystemString(const wchar_t*s);
#endif
//---------------------------------------------------------------------------
#endif

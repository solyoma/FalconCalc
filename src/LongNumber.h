#pragma once
#ifndef _LONGNUMBER_H
	#define _LONGNUMBER_H

#include <iostream>
#include <limits>
#include <algorithm>
#undef set
#include <set>
#include <map>

#include "defines.h"

namespace LongNumber {
	//using namespace SmString;				// SmString::SmartString.h
	constexpr const LENGTH_TYPE MaxAllowedDigits = 75;	// !!! Modify these  - no number string can have more digits than this + LengthOverflow
	constexpr const LENGTH_TYPE LengthOverFlow = 2;		// add this to the _maxLength of a RealNumber to get the real maximum string length
	constexpr const LENGTH_TYPE MaxAbsExponent = 1024;	// absolute value of maximum exponent
	constexpr const LENGTH_TYPE TrigAccuracy = 70;		// max this many digits used for trigonometric functions. must be <= MaxAllowedDigits

	extern const SmString::SmartString NAN_STR;
	extern const SmString::SmartString INF_STR;

	extern const SmString::SCharT chZero;				// = (SmString::SCharT)'0';
	extern const SmString::SCharT chOne;				// = (SmString::SCharT)'1';

#ifdef max
	#undef max
#endif
	constexpr const LENGTH_TYPE MaxExponent = std::numeric_limits<int>::max();


	enum class DisplayBase {
		rnb10,				// base 10
		// the ones below are only applied to the integer part of the 
		// number
		rnbHex,				// base 16
		rnbOct,				// base 8
		rnbBin,				// base 2
		rnbText				// each byte is one UTF-16 character
	};

	enum class NumberFormat {
		rnfGeneral,			// either rnfNormal or rnfSci depending on 
							// decDigits and the number
		rnfNormal,			// for decimal numbers: fix dot format with no exponent and either 
							//   no zero leading digit in integer part if any non-zero
							//   digit occures there (0.8 OK, but 08.3 is not)  
							// switches to sci format if it won't fit into displayed digits, 
							// for binary, hex and octal strings: all digits 
							//   optionally w. format specifier
		rnfSci, 			// format: -1.234E567	or any subformat below
		rnfEng,  			// like Sci but exponent is always a multiple of +/- 3
		rnfText				// ASCII or UTF-8, etc
	};
	enum class ExpFormat {
		rnsfGraph,			// exponent after ^, like -1.234^{567}
		rnsfSciHTML,		// format for html : -1.234x10<sup>-567</sup>
		rnsfSciTeX,			// format for TeX: -1.234\cdot10^{-567}
		rnsfE,				// exponent string is 'E'+ [+|-] + exp digits
		rnsfUTF8			// uses UTF-8 superscripts ⁰¹²³⁴⁵⁶⁷⁸⁹ with superscript - sign
	};
	enum class HexFormat {
		rnHexNormal, 		// -1234567890ABCDEF OR EDCBA09876543210	
		rnHexByte, 			// FE DC BA 09 87 65 43 21
		rnHexWord, 			// FEDC AB09 90AB CDEF
		rnHexDWord			// 12345678 90ABCDEF
	};
	enum class IEEEFormat
	{
		rntHexNotIEEE,
		rntHexIEEE754Single,	// 	1.0 => 3F800000
		rntHexIEEE754Double		//  1.0 => 3FF0000000000000
	};
	enum class TextFormat
	{
		rntfAnsi,			// each character pair in hexadecimal representation is one letter
		rntfUtf8,
		rntfUtf16
	};
	enum class SignOption 
	{ 
		soNormal,					// only use when positive
		soLeaveSpaceForPositive,	// leave space for positive
		soAlwaysShow				// show positive sign as well
	};


	// ---------
	enum class AngularUnit { auDeg, auRad, auGrad, auTurn };	// 1 turn = 2π

	/*
	*	mainFormat, displWidth and decDigits together dictates what is visible and how:
	*   - When displayWidth == 0 only decDigits matters
	*   - When displaywidth > 0 but smaller than the minimum width for the number
	*		(e.g. displayWidth = 4, number: 12345 or 123E2) than displayWidth number of
	*		'#' characters are shown instead of the number.
	*		are shown
	*	- When displayWidth > 0 and the length of the output string (including sign, exponents,
	*		number prefixes, separator characters) is greater than displWidth then
	*		* with decimal numbers the output is rounded
	*		* all other cases the message 'too long' is shown, unless displayWidth < the length
	*			of that string in which case the previous rule applies
	*/
	struct DisplayFormat
	{
		DisplayBase base = DisplayBase::rnb10;
		// for decimal display
		NumberFormat mainFormat = NumberFormat::rnfGeneral;
		ExpFormat  expFormat = ExpFormat::rnsfE;

		int decDigits = -1;						// only for base 10 format.
												// > 0	:This many digits required after the decimal point.
												// (if displWidth allows it). 
												// > 0	: Trailing 0 characters after the decimal point may be present
												//	 0	: only integer part, no decimal digits
												// -1	: all digits stored is displayed (if displWidth allows it)
												// > _numberstring.length(): add trailing zeros

		int displWidth = -1;					// display width in characters, -1: unlimited
												// when > 0 and the number string is shorter than this then the number is
												//		right aligned, otherwise it is left aligned. 
												//	the displayed number is determined by 'decDigits' and this
		SignOption signOption = SignOption::soNormal;
		SmString::SmartString strThousandSeparator;		// for decimal format. Empty: no separator
		bool useFractionSeparator = false;		// when true use a space character
		LENGTH_TYPE nFormatSwitchIntLength = 40,		// for decimal display: if the exponent is
												// larger than this 
			   nFormatSwitchFracLength = 40;	// or -exp larger than this
												// display will change to Sci automatically
												// for non base 10 display the maximum length
												// of the string in that base
// only hex or binary display
		bool bSignedBinOrHex = false;			// true: negative numbers are preceeded by a negative
		// sign and the corresponding hex digit, where the
		// highest bit is 0. false: for negative numbers
		// the highest bit is 1
		bool useNumberPrefix = false;			// '#' for binary, '0x' for hexadecimal, not used for octal numbers which always start with a '0'
		// only for hexadecimal display
		HexFormat hexFormat = HexFormat::rnHexNormal;
		IEEEFormat trippleE = IEEEFormat::rntHexNotIEEE;
		bool littleEndian = false;				// reverse order of bytes words or double words. Only used for rnHexWord and rnHexDWord
		AngularUnit angUnit = AngularUnit::auDeg;    // used when calculating sine, cosine, tangent, cotangent
	};

	enum class Base { dec, hex, oct, bin };

	enum class EFlag {
		// rnfOk,		OK when flags are empty
		rnfUnderflow = 1,
		rnfOverflow  = 2,
		rnfDivBy0    = 4,
		rnfInvalid   = 8,
		rnfMalformed	// e.g more than 1 decimal point or exponent or invalid number for base
	};

	// ---------
	enum class LogicalOperator { lopOr, lopXOr, lopAnd };

	//==============================================================================
	// constants
	//	values has the accuracy set when RealNumber accuracy changes in SetMaxLength
	class RealNumber;
	extern RealNumber half, 	// 1/2
		NaN, Inf							// specials
		;

	// physical constants
	struct Constant;

	extern Constant
		e,
		pi,											// π 
		twoPi, piP2,								// 2π, π/2
		sqrt2, sqrt3, sqrt3P2, ln10, ln2,			// √2, √3, 3√(π/2), log(10),log(2)
		rsqrt2, rln10, rln2,						// reciprocals: 1/√2, 1/log(10), 1/log(2)
		log2e,
		log10e,
		lge,
		ln2,
		piP2,
		piP4,
		rpi2,
		rpi,
		sqpi,
		sqrt2,
		rsqrt2,

		a0,			// Bohr radius
		au,			// astronomical unit
		c,			// speed of light in vacuum
		eps0,		// vacuum electric permittivity
		G,			// Newtonian constant of gravitation
		fsc,		// fine-structure constant (e^2/ (2 eps0 h c )
		gf,			// average g on Earth (9.81 m/s^2)
		h,			// Planck constant
		hbar,		// reduced Planck constant
		kb,			// Boltzmann constant
		kc,			// Coulomb constant (1/ (4 pi eps0)
		la,			// Avogadro constant
		me,			// electron mass
		mf,			// mass of the Earth
		mp,			// proton mass
		ms,			// mass of the Sun
		mu0,		// vacuum magnetic permeability 
		qe,			// elementary charge
		rfsc,		// reciprocal of afs (approx 137) 
		rf,			// radius of the Earth
		rg,			// molar gas constant R (approx 8.31 J/mol/K)
		rs,			// radius of the Sun
		sb,			// Stefan–Boltzmann constant
		u			// atomic mass unit
		;
	//-------------------- end constants ----------------------------------------------------------

	using LDouble = long double;

	SmString::SCharT ByteToMyCharT(uint16_t digit);
	uint16_t MyCharTToByte(SmString::SCharT ch);

	/*=============================================================
	 * REAL_NUMBERS are arbitrary long floating point numbers
	 * in base 10. They are stored as a decimal number string with a maximum
	 * 		length of 'MaxAllowedDigits', a number sign
	 *		and a signed integer exponent.
	 * The intenal number string contains no decimal point never
	 *	starts with a '0' digit. Otherwise it must be no longer
	 *  than the non zero digits in the number or '_maxLength'
	 *
	 *	Examples ( with _maxlength >= 4 and the stored string is '1234')
	 *	exponent  sign				 number
	 * 		 0		 1     		 1.234 E-1 (=0.1234)
	 * 		 1		 1     		 1.234 E0  (=1.234)
	 *		99		 1			 1.234 E98
	 *		-3		-1			-1.234 E-4 (=-0.0001234)
	 *		-99		-1			-1.234 E-100
	 *
	 * Rwo real numbers may have different _maxLength and _maxExponent
	 * values. If there is an operation beteen them the result will have
	 * the larger of these.
	 *
	 * Any arithmetic operations between two REAL_NUMBERs may result in
	 * a RealNumber with fewer or more mpm zero digits than the
	 * ones in the operation, but the total number of digits may never
	 * exeed _maxLength (which has a limit of 'MaxAllowedDigits').
	 *
	 * During arithmetic operations, however, the intermediate string
	 * may be as long as 2 x '_maxLength'.
	 *
	 * '_precision' is the length of the actual numeric string
	 * Minimum precision is 0
	 * For the mathematical constants use the appropriate functions
	 * so that the values have the same accuracy as the one set.
	 *------------------------------------------------------------*/
	class RealNumber
	{					// first part of private members: *second part after the public ones
	private:
		int _sign = 1;				// number sign: +1 or -1
		SmString::SmartString _numberString;	// normalized (original or calculated) number as string
		int _exponent = 0;	// ((10's exponent of number) + 1). If positive # of integer digits
		// for calculations the number is used in normalized form
		// example: 12345.678E-9 => mantissa: 0.12345678, 10's exponent: -5 (=-9+4), stored as: 
		//							=> _numberString: "12345678", _exponent : -4 
		//		  : number = 0		=> _numberString = "", _exponent = 0
		//		  : number = 0.123	=> _numberString = "123", _exponent = 0
		//		  : number = 12345.678E6 => _numberString = 12345678, _exponent=11

		static LENGTH_TYPE _maxExponent; // absolute value of largest possible exponent of 10 in number
		static LENGTH_TYPE _maxLength;	//	# of bytes this number may occupy, now === # of digits+LengthOverFlow
		static LENGTH_TYPE _maxTrigLength;	//	# of bytes trigonometry calculations can use
		// other private members like _leadingZeros, etc are below the 'public:' parts
	public:
		using EFlagSet = std::set<EFlag>;

	public:	// constructors
		// create a RealNumber with the actual _maxLength and _maxExponent
		RealNumber()
		{
			_GetDecPoint(); // sets _decPoint for locale	 
		}

		RealNumber(const RealNumber& rn) :
			_sign(rn._sign), _numberString(rn._numberString), _exponent(rn._exponent),_eFlags(rn._eFlags)
		{
			_GetDecPoint();
			if (_numberString.length() > MaxAllowedDigits)
				RoundNumberString(_numberString, MaxAllowedDigits);

			_isNormalized = true;
		}
		explicit RealNumber(const char* s) : RealNumber(SmString::SmartString(s)) {}
		explicit RealNumber(const SmString::UTF8String& s) : RealNumber(SmString::SmartString(s)) {}
		explicit RealNumber(const std::wstring& ws) : RealNumber(SmString::SmartString(ws)) {}
		explicit RealNumber(const SmString::SmartString& s) : _numberString(s), _exponent(0)
		{
			_GetDecPoint();
			_FromNumberString();
		}
		// next constructor: 'digits' has no decimal point or sign or exponent!
		explicit RealNumber(const SmString::SmartString& digits, int sign, int exponent) :
			_sign(sign), _numberString(digits), _exponent(exponent)
		{
			_GetDecPoint();
			if (_numberString.length() > MaxAllowedDigits)
				RoundNumberString(_numberString, MaxAllowedDigits);
		}
		explicit RealNumber(int lval) :_exponent(0) { _FromLongLong(lval); }
		explicit RealNumber(unsigned lval) :_exponent(0) { _FromLongLong(lval); }
		explicit RealNumber(int64_t lval) :_exponent(0) { _FromLongLong(lval); }
		explicit RealNumber(double dval) :_exponent(0) { _FromDouble(dval); }
		explicit RealNumber(long double dval) :_exponent(0) { _FromDouble(dval); }

	public:
		// special values
		static const RealNumber RN_0, RN_1, RN_2, RN_3, RN_4, RN_5, RN_6, RN_7, RN_8, RN_9, RN_10, 
								RN_11, RN_12, RN_13, RN_14, RN_15, RN_16,
								RN_30, RN_45, RN_60, RN_90, RN_180, RN_270, RN_360;
		static RealNumber epsilon;
		static RealNumber trigEpsilon;

	public:
		// static functions
		static LENGTH_TYPE MaxLength() { return _maxLength; }
		static SmString::SCharT DecPoint() { return _decPoint; }
		static inline void SetMaxExponent(LENGTH_TYPE maxExp)
		{
			_maxExponent = maxExp;
		}
		static LENGTH_TYPE SetMaxLength(LENGTH_TYPE mxl);	// returns original
		static LENGTH_TYPE SetMaxTrigLength(LENGTH_TYPE mxtrl);	// returns original
		static RealNumber NumberLimit(bool smallest = false) { RealNumber r; r._exponent = (int)r._maxExponent * (smallest ? 1 : -1); r._numberString = "1"; return r; }
		static RealNumber TenToThePowerOf(int exp) /* exp is normal 10's exponent*/ 
		{ 
			RealNumber x(RN_1); 
			x._exponent = exp + 1; 
			return x; 
		}

	public:	// operators
		RealNumber operator=(const RealNumber& rn);
		RealNumber operator=(RealNumber&& rn) noexcept;
		RealNumber operator=(const SmString::SmartString& rn);
		RealNumber operator=(double value);
		RealNumber operator+() const		// unary +
		{
			return *this;
		}
		RealNumber operator-() const		// unary -
		{
			RealNumber res(*this);
			res._sign = -res._sign;
			return res;
		}

		bool operator==(const RealNumber& rn) const;
		bool operator!=(const RealNumber& rn) const;
		bool operator<(const RealNumber& rn)  const;
		bool operator<=(const RealNumber& rn) const;
		bool operator>(const RealNumber& rn)  const;
		bool operator>=(const RealNumber& rn) const;

		RealNumber& operator++();	// prefix 
		RealNumber operator++(int);	// postfix
		RealNumber& operator--();	// prefix
		RealNumber operator--(int);	// postfix

		RealNumber operator+(const RealNumber& ld) const { return _Add(ld, false); }	// this and ld may have different precisions set!
		RealNumber operator-(const RealNumber& ld) const { return _Subtract(ld); }
		RealNumber operator*(const RealNumber& ld) const { return _Multiply(ld); }
		RealNumber operator/(const RealNumber& ld) const { return _Divide(ld); }

		RealNumber operator+=(const RealNumber& ld) { return *this = _Add(ld); }	// this and ld may have different precisions set!
		RealNumber operator-=(const RealNumber& ld) { return *this = _Subtract(ld); }
		RealNumber operator*=(const RealNumber& ld) { return *this = _Multiply(ld); }
		RealNumber operator/=(const RealNumber& ld) { return *this = _Divide(ld); }

		// these four are 'recursive on all control paths, function will cause runtime stack overflow'
		//template <typename T> RealNumber operator+(const T ld) { RealNumber  lTmp(ld); return *this + lTmp; }
		//template <typename T> RealNumber operator-(const T ld) { RealNumber  lTmp(ld); return *this - lTmp; }
		//template <typename T> RealNumber operator*(const T ld) { RealNumber  lTmp(ld); return *this * lTmp; }
		//template <typename T> RealNumber operator/(const T ld) { RealNumber  lTmp(ld); return *this / lTmp; }

		template <typename T> RealNumber operator+=(const T ld) { RealNumber lTmp(ld); return *this += lTmp; }
		template <typename T> RealNumber operator-=(const T ld) { RealNumber lTmp(ld); return *this -= lTmp; }
		template <typename T> RealNumber operator*=(const T ld) { RealNumber lTmp(ld); return *this *= lTmp; }
		template <typename T> RealNumber operator/=(const T ld) { RealNumber lTmp(ld); return *this /= lTmp; }

		RealNumber operator^(const RealNumber& power) const	// not XOR, but power function
		{
			return Pow(power);
		}
		RealNumber operator^(int power) const	// not XOR, but power function
		{
			return Pow(power);
		}
		RealNumber operator|(const RealNumber& other) const { return _LogOpWith(other, LogicalOperator::lopOr); }
	public:
		RealNumber SetError(EFlag ef) { _eFlags.insert(ef); return *this; }
		void SetNaN();
		bool Errors(EFlagSet* oflags = nullptr)  const
		{
			if (oflags)
				*oflags = _eFlags;

			return _eFlags.size();
		}

		RealNumber Or(const RealNumber& ra) const { return _LogOpWith(ra, LogicalOperator::lopOr); }
		RealNumber XOr(const RealNumber& ra) const { return _LogOpWith(ra, LogicalOperator::lopXOr); }
		RealNumber And(const RealNumber& ra) const { return _LogOpWith(ra, LogicalOperator::lopAnd); }
		RealNumber Square() const { return *this * *this; }
		RealNumber Squared() const
		{
			RealNumber r(*this);
			r.Square();
			return r;
		}
		RealNumber Pow(int power) const
		{
			return _PowInt(RealNumber(power));
		}

		RealNumber Pow(const RealNumber& power) const;

		constexpr const SmString::SmartString& Mantissa() const { return _numberString; } // a.k.a. significand
		constexpr int Exponent() const	// real 10's exponent as _exponent = (real exponent of 10) + 1
		{
			return _exponent - 1;
		}
		LENGTH_TYPE Precision() const { return _numberString.length(); }
		LENGTH_TYPE LargestExponent() const { return _maxExponent; }

		RealNumber Round(  int toThisManyDecimalPlaces				,int cntIntDigits=-1);		 // returns the rounded number rounded to the given decimal places
		RealNumber RoundToDigits(int toThisManySignificantDigits	,int cntIntDigits=-1);		 // returns the number rounded to the given significant digits which includes the integer part as well
		RealNumber RoundedToDigits(int toThisManySignificantDigits	,int cntIntDigits=-1) const; // similar to 'Round()' but a copy is rounded, not the original number
		RealNumber Rounded(int toThisManyDecimalPlaces				,int cntIntDigits=-1) const; // similar to 'RoundToDigits()' but for a copy
		// in place rounding of a string of numbers (and possibly a single decimal point) to max 'maxLength' digits. Returns increment in the exponent.
		// examples: strNumber = "129",   for maxLength >= 3 - nothing happens, returns 0, for 'maxLength=2' rounds to "13" and returns 1
		//			 strNumber = "9.995", for maxLength >= 4 - nothing happens, returns 0, for 'maxLength=2' rounds to "10" (== 10.000) and returns 1
		static int RoundNumberString(SmString::SmartString& strNumber, int maxLength); 

		SmString::SmartString ToBinaryString(const DisplayFormat& format);
		SmString::SmartString ToOctalString(const DisplayFormat& format);
		SmString::SmartString ToHexString(const DisplayFormat& format);
		SmString::SmartString ToDecimalString(const DisplayFormat& format);
		// SmString::SmartString ToTextString(const DisplayFormat& format, TextFormat textFormat);
		SmString::SmartString ToSmartString() const;
		SmString::UTF8String  toUtf8String() const;	   // because Qt uses this type of function names
		std::wstring ToWideString() const;
//#if defined _DEBUG	|| defined DEBUG
//		std::wstring numstringtowstr()
//		{
//			std::wstring ws;
//			for (auto& a : _numberString)
//				ws += wchar_t(a.unicode());
//			return ws;
//		}
//#endif


		SmString::SmartString ToString(const DisplayFormat& format);
		SmString::SmartString ToString()					// format as decimal string with default settings
		{
			DisplayFormat format;
			return ToString(format);
		}

		LDouble ToLongDouble();	// may return +/- Inf if number is infinite or doesn't fit into a (long) double and NaN if number is Nan
		int64_t ToInt64() const;

		RealNumber Int() const;		// discard fractional part
		RealNumber Frac() const;	// discard integer part

		bool ConvertToInteger();	// returns false for Nan's and too big numbers
		RealNumber Div(const RealNumber& divisor, RealNumber& remainder) const { return _Div(divisor, remainder); } // remainder may be nullptr

		inline EFlagSet EFlags() const { return _eFlags; }
		inline void SetEFlag(EFlag ef) { _eFlags.insert(ef); }
		inline constexpr int  Sign()   const { return _sign; }
		inline constexpr bool IsPositive()   const { return _sign == 1; }
		inline constexpr bool IsNegative()   const { return _sign == -1; }
		inline RealNumber &SetSign(int sign) { _sign = sign; return *this; }
		inline RealNumber &ToAbs() { _sign = 1; return *this;  return *this; }


		RealNumber Abs() const 
		{ 
			if(_sign == -1) 
			{
				RealNumber rn(*this); 
				rn.ToAbs(); 
				return rn;
			}
			return *this;
		}
		inline bool IsNaN()  const noexcept { return _numberString.at(0) == SmString::SCharT('N'); };
		inline int  IsInf()  const noexcept { return _numberString.at(0) == SmString::SCharT('I'); };	// +Inf or -Inf
		inline int  IsTooLong()  const noexcept { return _numberString.at(0) == SmString::SCharT('T'); };	// +Inf or -Inf
		inline bool IsPure10Power() const noexcept { return _numberString == SmString::SmartString(chOne); }

		inline bool IsNull() const
		{
			return _numberString.empty() || (_numberString.find_first_not_of(chZero) == SmString::SmartString::npos);
		}
		inline constexpr bool IsPmOne() const noexcept  // +- 1 ?
		{
			return _exponent == 1 && _numberString == SmString::SmartString("1");
		}
		inline bool IsInt() const noexcept
		{
			return IsValid() && (IsNull() || _exponent >= (int)_numberString.length() );
		}
		inline bool IsOdd() const noexcept
		{
			SmString::SCharT ch = _numberString.at(_exponent - 1);
			return (ch == SmString::SCharT('1') || ch == SmString::SCharT('3') || ch == SmString::SCharT('5') || ch == SmString::SCharT('7') || ch == SmString::SCharT('9'));
		}
		inline bool IsEven() const noexcept
		{
			return !IsOdd();
		}
		inline bool IsValid() const noexcept
		{
			return !IsNaN() && !IsInf();
		}

		// ******* private *************

	private:
		int _leadingZeros = 0;			// used in operations
		bool _isNormalized = false;

		static SmString::SCharT _decPoint;	// get from locale
		EFlagSet _eFlags;

		struct _DisplData				   // friend of RealNumber
		{
			// DEBUG
			RealNumber* pRn = nullptr;	   // display this number
			// /DEBUG

			DisplayFormat fmt;			   // in this format

			int		nLeadingDecimalZeros = 0;	// which are not stored in pRn's _numberString, but comes before it
			int		cntThousandSeparators = 0;	// any string
			int		exp=0;						// so we don't change _exp
			bool	numberIsZero = false;

			LENGTH_TYPE	nIntegerDigits = 0;			// in pRn->_numberString  ( if > _numberstring.length() logically right extended by '0's when needed on display)
			LENGTH_TYPE	cntDecDigitsToDisplay = 0;	// rounding position in pRn->_numberString
			LENGTH_TYPE	cntDecDigitsInNumberString = 0;	// total number for decimal part in pRn->_numberString
			LENGTH_TYPE	expLen = 0;					// length of exponent string
			LENGTH_TYPE	expW = 0;					// display length of same (TODO: using pixel units)

			int nRoundPos = -1;					// relative to pRn->_numberString, including integer part! -1: no rounding 

			LENGTH_TYPE	nWSign = 0;					// 0 or 1
			LENGTH_TYPE nWIntegerPart = 1;			// width of whole integer part of formatted string including thousand separators, but w.o. dec. point or sign
			LENGTH_TYPE nWDisplayW = LENGTH_TYPE(-1);		// display width in characters, -1: unlimited
			LENGTH_TYPE	nWFractionalPart = 0;		// length of visible part of fractional digits + 1 for decimal point
			LENGTH_TYPE	nWDecPoint = 0;				// or 1

			SmString::SmartString strExponent;
			SmString::SmartString strRounded;				// rounded string of digits from pRn->_numberString may be empty

			void Setup(const DisplayFormat& format, RealNumber& rN);
			SmString::SmartString FormatExponent();		// normal: smaller numbers at upper index position, TEX: with ^, HTML with <sup>...</sup>, else Exxx

			void FixNumberFormatAndSetExponent(); // for general format or too large/small numbers
			int RequiredSpaceforIntegerDigits();  // calc. nWIntegerPart
			bool PrepareRoundedString();		  // calc. nWFractionalPart including decimal separators
			bool Round();						// from pRn->_numberString to strRounded	
		};
		_DisplData _dsplD;

	private:	  // these are modified every time _maxLength is changed
		inline void _GetDecPoint() noexcept
		{
			_decPoint = ::std::use_facet<::std::numpunct<wchar_t> >(::std::cout.getloc()).decimal_point();
		}
		long _AddExponents(long oneExp, long otherExp, EFlagSet& efs) const;
		void _AddExponents(int otherExp);
		void _FromNumberString();		// from _numberString
		void _FromLongLong(const int64_t val)
		{
			_numberString = std::to_wstring(val);
			_FromNumberString();
		}
		void _FromDouble(const LDouble ld)
		{
			_numberString = std::to_wstring(ld);
			_FromNumberString();
		}
	private:
		static void _RedefineEpsilon();
		static void _RedefineTrigEpsilon();
		RealNumber _Add(const RealNumber& ra, bool negateRa = false) const;
		RealNumber _Subtract(const RealNumber& ra) const;
		RealNumber _Multiply(const RealNumber& ra) const;
		bool __HandleSpecialDivisions(RealNumber& dividend, const RealNumber& divisor) const;
		RealNumber _Divide(const RealNumber& ra) const;
		RealNumber _Div(const RealNumber& divisor, RealNumber& remainder) const; // rmainder may be the same variable as divisor
		RealNumber _LogOpWith(const RealNumber& ra, LogicalOperator lop) const;
	private:
		inline SmString::SCharT _CharAt(int position)  const // in _numberString from logical position
		{
			int ix = (int)_numberString.length() + _leadingZeros;
			return (position < ix && position >= _leadingZeros) ? (SmString::SCharT)_numberString[position - _leadingZeros] : chZero;
		}

		SmString::SmartString _AsSmartString() const;

		static void _RescaleConstants(int maxLength);

		void _Normalize();	// from a non-normalized numberstring, _sign, _exponent create a normalized one (unless normalized already)
		void _ShiftSmartString(LENGTH_TYPE byThisAmount);	// logical shift to the right when arg. is positive, real shift to the left when it is negative
		void _ShiftSmartString(RealNumber& rn, int byThisAmount);	// to the right when arg. is positive, to the left when it is negative
		void _SetNull();
		void _SetInf();
		void _SetTooLong();
								// allocates may change nIntDigits: stores the position in the result string
		SmString::SmartString _IntegerPartToString(const SmString::SmartString &sNumber, int sign, const DisplayFormat &format, LENGTH_TYPE &nIntDigits, LENGTH_TYPE chunkSize, bool prefixToAllChunks) const;
		SmString::SmartString _DecimalPartToString(const SmString::SmartString& sNumber, _DisplData &disp, LENGTH_TYPE& nIntDigits) const;
		SmString::SmartString _RoundNumberString(SmString::SmartString& s, int& cntIntegerDigits /* == _exponent + 1 */, int roundingPosition, int cntLeadingZeros) const;

		SmString::SCharT _AddDigits(SmString::SCharT digit1, SmString::SCharT digit2, int& carry) const;
		SmString::SCharT _SubtractDigits(SmString::SCharT digit1, SmString::SCharT digit2, int& borrow) const;
		// these perform the additions and subtractions of pre-prepared decimal strings
		// of possibly different length. The difference in exponents already taken into account
		// so one of the numbers (but not these strings) may start with 0 digits at the front
		// Result will be placed into the first argument's string
		// Returns the number with which the exponent should be changed,
		//  < 0 for left shift, > 0 for right shift
		void _AddStrings(RealNumber& left, RealNumber& right) const;	// sign of result is sign of left summand
		void _SubtractStrings(RealNumber& minuend, RealNumber& subtrahend)const;	// always set the larger absolute value number as 'left'
		void _MultiplyTheStrings(RealNumber& left, RealNumber& right) const;	// result will be as long as the sum of the significant digits
		void _DivideInternal(RealNumber& left, RealNumber& right, RealNumber* pRemainder = nullptr) const;
		void _CorrectResult(RealNumber& left, SmString::String& result, int trailingchZeros, int carry) const;
		inline SmString::SmartString _SignString(const DisplayFormat format) const
		{
			return SmString::SmartString(_sign < 0 ? "-" :
				(format.signOption == SignOption::soAlwaysShow ? "+" :
					(format.signOption == SignOption::soLeaveSpaceForPositive ? " " : "")));
		}

		int _PosExpInNumberString(const DisplayFormat fmt, const SmString::SmartString& s, int fromPos) const;

		// conversion
		SmString::SmartString _ToBase(int base, LENGTH_TYPE maxmaxLength = 32) const;	// number must be smallar than the allowed digits and base must be >1 && <= 16
		// exponentiation
		RealNumber _PowInt(const RealNumber& AnInteger) const;
	};

	/*=============================================================
	 * Functions
	 *------------------------------------------------------------*/
	inline RealNumber abs(const RealNumber r) { return r.Abs(); }
	inline RealNumber floor(const RealNumber r) { return r.Int(); }
	inline RealNumber ceil(const RealNumber r) { return (r + RealNumber::RN_1).Int(); }
	inline RealNumber frac(const RealNumber r) { return r.Frac(); }
	inline RealNumber round(int decDigits, const RealNumber r) { return r.Rounded(decDigits); }
	inline RealNumber fmod(const RealNumber x, const RealNumber& y) 
	{ 
		if (x < y)
			return x;
		RealNumber xi(x);
		int sx = x.Sign(), sy = y.Sign();
		return xi - (xi / y).Int() * y; 
	}
	RealNumber fact(const RealNumber n);
	RealNumber RadToAu(RealNumber r, AngularUnit au);

	inline RealNumber sqrt(RealNumber r);							// calculate the result up till _maxLength
	RealNumber sqrtA(RealNumber r, int cntDigitsAccuracy);	// calculate the result up till 1e-<cntDigitsAccuracy> or. when == -1 to  _maxLength
	RealNumber pow(RealNumber base, RealNumber power);			// base ^power
	RealNumber root(int n, RealNumber base);				// n√base		// see also root3
	RealNumber root3(RealNumber base);						// ³√base

	// transcendent functions
	RealNumber exp(RealNumber exponent);						// e^x
	RealNumber ln(RealNumber num);								// ln(x)
	RealNumber log(RealNumber num, RealNumber& base);		// logarithm in base 'base'
	RealNumber log10(RealNumber num);							// or lg, base = 10
	RealNumber log2(RealNumber  num);							// base = 2

	// trigonometric functions
	RealNumber sin(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// sine
	RealNumber csc(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// cosecant = 1/sine
	RealNumber cos(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// cosine
	RealNumber sec(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// secant = 1/cosine
	RealNumber tan(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// tangent
	RealNumber cot(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// cotangent
	// inverse trigonometric functions
	RealNumber asin(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// sine
	RealNumber acos(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// cosine
	RealNumber atan(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// tangent
	RealNumber acot(RealNumber r, AngularUnit angu = AngularUnit::auDeg);		// cotangent
	// hyperbolic functions
	RealNumber sinh(RealNumber r);			// hyperbolic sine
	RealNumber csch(RealNumber r);			// hyperbolic cosecant = 1/sinh
	RealNumber cosh(RealNumber r);			// hyperbolic cosine
	RealNumber sech(RealNumber r);			// hyperbolic secant = 1/cosh
	RealNumber tanh(RealNumber r);			// hyperbolic tangent
	RealNumber coth(RealNumber r);			// hyperbolic cotangent
	// inverse hyperbolic functions
	RealNumber asinh(RealNumber r);		// hyperbolic sine
	RealNumber acosh(RealNumber r);		// hyperbolic cosine
	RealNumber atanh(RealNumber r);		// hyperbolic tangent
	RealNumber acoth(RealNumber r);		// hyperbolic cotangent

	/*=============================================================
	 * Mathematical or physical built-in constants
	 * but can be used for variables too
	 *------------------------------------------------------------*/
	struct Constant
	{
		friend class ConstantsMap;

		SmString::SmartString name;		// used from outside
		SmString::SmartString unit;		// e.g. kg
		SmString::SmartString desc;		// description
		BuiltinDescId binDesc = DSC_NoDescription;	// description ID for builtins
		RealNumber value;		// re-scaled value

		Constant() {}
		// built-in constants
		explicit Constant(const wchar_t* cname, const RealNumber cvalue, const wchar_t* cunit, const BuiltinDescId binDesc, const RealNumber* pBaseValue) :
			value(cvalue), _pBaseValue(pBaseValue), _builtin(true), _set(true), binDesc(binDesc)
		{
			name.FromWideString(cname);
			unit.FromWideString(cunit);
			value.RoundToDigits(LongNumber::RealNumber::MaxLength());
		}
#ifndef _MSC_VER_SA
		// hasznalja
		explicit Constant(const char16_t* cname, const RealNumber cvalue, const char16_t* cunit, BuiltinDescId binDesc, const RealNumber* pBaseValue, const char16_t *cdesc= nullptr) :
			value(cvalue), _pBaseValue(pBaseValue), _builtin(true), _set(true), binDesc(binDesc)
		{
			name = SmartString(cname);
			unit = SmartString(cunit);
			if(cdesc)
				desc = SmartString(cdesc);
			value.RoundToDigits(MaxAllowedDigits);
		}
#endif

		explicit Constant(const SmString::String name, const RealNumber cvalue, const SmString::String unit, const SmString::String desc, const RealNumber* pBaseValue) :
			name(name), value(cvalue), unit(unit), binDesc(binDesc), _pBaseValue(pBaseValue), desc(desc), _builtin(true), _set(true) 
		{
			value.RoundToDigits(MaxAllowedDigits);
		}

		explicit Constant(const SmString::String name, const RealNumber cvalue, const SmString::String unit, const BuiltinDescId binDesc, const RealNumber* pBaseValue) :
			name(name), value(cvalue), unit(unit), binDesc(binDesc), _pBaseValue(pBaseValue), _builtin(true), _set(true) 
		{
			value.RoundToDigits(MaxAllowedDigits);
		}

		// for user defined constants (variables) definition value will be the base value
		explicit Constant(const wchar_t* cname, const RealNumber cvalue, const wchar_t* cunit, const wchar_t *pdesc) :
			value(cvalue), _set(true)
		{
			name.FromWideString(cname);
			unit.FromWideString(cunit);
			desc.FromWideString(pdesc);
			_pBaseValue = new RealNumber(value);		 // before possible rounding
			value.RoundToDigits(MaxAllowedDigits);
		}

		explicit Constant(const SmString::String name, const RealNumber cvalue, const SmString::String unit, const SmString::String desc) :
			name(name), value(cvalue), unit(unit), desc(desc) 
		{
			value.RoundToDigits(MaxAllowedDigits);
		}
		// for both
		Constant(const Constant& co) : name(co.name), value(co.value), unit(co.unit), desc(co.desc), binDesc(co.binDesc), _pBaseValue(co._pBaseValue), 
										_builtin(co._builtin), _set(co._set) 
		{
			value.RoundToDigits(MaxAllowedDigits);
		}

		virtual ~Constant()
		{
			if (!_builtin)
				delete _pBaseValue;
		}

		Constant& operator=(const Constant& other) 
		{ 
			name = other.name; 
			value = other.value; 
			unit = other.unit; 
			desc = other.desc; 
			binDesc = other.binDesc; 
			return *this; 
		}
		Constant& operator=(const SmString::SmartString sms) { name = sms; return *this; }
		Constant& operator=(const RealNumber val) { value = val; return *this; }
		operator const SmString::SmartString& () const { return name; }	// CAN'T use these to mdify content
		operator const RealNumber& ()  const 
		{ 
			return value; 
		}
		operator RealNumber() const { return value; }

		void SetAsBuiltIn()
		{
			if (!_set)
			{
				_set = true;
				_builtin = true;
			}
		}

		bool IsBuiltin() const { return _builtin; }

		void Rescale(int newMaxLength)
		{
			if (_pBaseValue)
				value = _pBaseValue->RoundedToDigits(newMaxLength);
		}
	private:
		const RealNumber* _pBaseValue = nullptr; // NULL or pointer to definition (now 102 decimal digits long) value, which can be rescaled
		bool _builtin = false;
		bool _set = false;
	};

	/*=============================================================
	 * this map contains pointers to constants
	 *	builtin constants exist already and not duplicated
	 * user defined constants added using the Add() function
	 * they will be freed
	 *------------------------------------------------------------*/
	class  ConstantsMap : public std::map<SmString::String, Constant*>
	{
	public:
		ConstantsMap();
		virtual ~ConstantsMap();

		// user defined constants only
		void Add(const wchar_t* cname, const RealNumber cvalue, const wchar_t* cunit, const wchar_t *cdesc);
		void Add(const SmString::String cname, const RealNumber cvalue, const SmString::String cunit, const String cdesc);
		void Rescale(int newMaxLength);
	private:
		void _AddBuiltIn(Constant& c);	// set as builtin
	};

	extern ConstantsMap constantsMap;
	// end of namespace LongNumber
}

#endif

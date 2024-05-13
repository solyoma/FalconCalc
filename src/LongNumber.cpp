#include <cassert>
#include <regex>
#include "LongNumber.h"

#ifdef _DEBUG		// DEBUG

#include <cstdarg>

int debugPrint(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int res = vprintf(format, args);
	va_end(args);
	return res;
}

#else
#define debugPrint
#endif// /DEBUG

//////////////////////////////////////////////////////////////
namespace LongNumber {
//////////////////////////////////////////////////////////////
	const SCharT chZero = (SCharT)'0';
	const SCharT chOne = (SCharT)'1';
	const SmartString NAN_STR("NaN");
	const SmartString INF_STR("Inf");


	const SCharT chSpace = SCharT(' ');
	static RealNumber epsilon(SmartString("1"), 1, -(int)RealNumber::MaxLength());
	// atan(0.2)  used in invert trigonometric function calculations
	const RealNumber atanOfDot2 = RealNumber(".197395559849880758370049765194790293447585103787852101517688940241033969978243785732697828037288044112");

	static void RedefineEpsilon()
	{
		epsilon = RealNumber(SmartString("1"), 1, -(int)RealNumber::MaxLength());
	}

SCharT ByteToMyCharT(uint8_t digit)	// 0 <= digit <= 32
{
	if (digit < 10)
		return SCharT('0' + digit);
	if (digit >= 10)
		return SCharT('A' + (digit - 10) );
	return SCharT(0);
}

uint8_t MyCharTToByte(SCharT ch)
{
	if (ch < SCharT('0'))
		return 0;
	if (ch <= SCharT('9'))
		return ch.Unicode() - '0';
	return 10 + (ch.Unicode() - 'A');
}

SmartString Utf8FromWideString(const std::wstring& ws)
{
	SmartString r;
	r.FromWideString(ws);
	return r;
}

SmartString ToHexByte(size_t byte)
{
	SmartString res(2, chZero);
	res[0] = ByteToMyCharT((byte & 0xF0) >> 4);
	res[1] = ByteToMyCharT((byte & 0x0F));
	return res;
};
//--------------------------------------
SCharT RealNumber::_decPoint;
size_t RealNumber::_maxExponent = 1024;	// absolute value of largest possible exponent of 10 in number
size_t RealNumber::_maxLength = 100;	// maximum length of string

const RealNumber RealNumber::RN_0("0"), RealNumber::RN_1("1"), RealNumber::RN_2("2"), RealNumber::RN_3("3"), RealNumber::RN_4("4"), 
				RealNumber::RN_5("5"), RealNumber::RN_6("6"), RealNumber::RN_7("7"), RealNumber::RN_8("8"), RealNumber::RN_9("9"), RealNumber::RN_10("10"),
				RealNumber::RN_16("16");

// constants and Functions that can be used with REAL_NUMBERs
static const RealNumber rnNull("0"),
						rnHalf("0.5"),
			// the following definitions define the main math cconstants' base value with 102 decimal places
			// The the constants used in calculations is given by the same name, just without the 'rn' or 'rn_' prefix and have
			// as many digits as set, in RealNumber::SetMaxLength(new_length) 
			//					   1		 2 		   3		 4		   5		 6		   7		 8		   9		10
			//			0 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012				  
				  rnPi("3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067980"), // π
				 rn2Pi("6.283185307179586476925286766559005768394338798750211641949889184615632812572417997256069650684234135960"),	// 2π
				rnPiP2("1.570796326794896619231321691639751442098584699687552910487472296153908203143104499314017412671058533990"),	// π/2
				  rnE ("2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427427"),	// e
			   rnSqrt2("1.414213562373095048801688724209698078569671875376948073176679737990732478462107038850387534327641572735"),	// √2
			  rnPSqrt2("0.707106781186547524400844362104849039284835937688474036588339868995366239231053519425193767163820786367"),	// 1/√2
			   rnSqrt3("1.732050807568877293527446341505872366942805253810380628055806979451933016908800037081146186757248575675"),	// √3
			 rnSqrt3P2("0.866025403784438646763723170752936183471402626905190314027903489725966508454400018540573093378624287837"),	// 1 / (3√π)
				 rnLn2("0.693147180559945309417232121458176568075500134360255254120680009493393621969694715605863326996418687542"),	// ln(2)
				rnLn10("2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298341"), // ln(10)
				 pln10("0.434294481903251827651128918916605082294397005803666566114453783165864649208870774729224949338431748318"),	// 1/ln(10)
				  pln2("1.442695040888963407359924681001892137426645954152985934135449406931109219181185079885526622893506344497"),	// 1/ln(2)

				 rnRpi("0.318309886183790671537767526745028724068919291480912897495334688117793595268453070180227605532506171912"),	// 1/π
				rnPiP4("0.785398163397448309615660845819875721049292349843776455243736148076954101571552249657008706335529266995"), // π/4
				rnRPi2("0.636619772367581343075535053490057448137838582961825794990669376235587190536906140360455211065012343824"), // 2/π
				rnSqpi("1.772453850905516027298167483341145182797549456122387128213807789852911284591032181374950656738544665415"),	// 1/√π
			  rnLogB2E("0.301029995663981195213738894724493026768189881462108541310427461127108189274424509486927252118186172040"),	// log_2(e)
			     rnLgE("0.434294481903251827651128918916605082294397005803666566114453783165864649208870774729224949338431748318"), // lg(e)
				 rnLge = rnLgE,

					rnNaN(NAN_STR), rnInf(INF_STR),
// physical constants		// they are not rescaled in RealNumber::SetMaxLength()
			rn_au		("149597870700"),		// astronomival unit - definition (m)
			rn_c		("299792458"),			// speed of light in vacuum	 m⋅s-1
			rn_eps0		("8.8541878128E-12"),	// vacuum electric permittivity	(F/m)
			rn_fsc		("7.297352569311E-3"),	// fine-structure constant (e^2/ (2 εo h c )
			rn_G		("6.6743015E-11"),		// Newtonian constant of gravitation (m^3/(kg s^2))
			rn_gf		("9.81"),				// average g on Earth (9.81 m/s^2)
			rn_h		("6.62607015E-34"),		// Planck constant  (J⋅s)
			rn_hbar		("1.054571817E-34"),	// reduced Planck constant
			rn_kb		("1.380649E-23"),		// Boltzmann constant (J/K)
			rn_kc		("8.987551792314E9"),	// Coulomb constant (1/ (4 π εo)
			rn_la		("6.02214076E23"),		// Avogadro constant	(1/mol)
			rn_me		("9.109383701528E-31"),	// electron mass
			rn_mf		("5.972168E24"),		// mass of the Earth (kg)
			rn_mp		("1.6726219236951E-27"),// proton mass
			rn_ms		("1.98849E30"),			// mass of the Sun	(kg)
			rn_mu0		("1.25663706212E-6"),	// vacuum magnetic permeability (N/A^2)
			rn_qe		("1.602176634E-19"),	// elementary charge (C)
			rn_pfsc		("137.03599908421"),	// reciprocal of afs (approx 137) 
			rn_rf		("6.378137E6"),			// radius of the Earth at the equator(m)
			rn_rg		("8.31446261815324"),	// molar gas constant R (J/mol/K)
			rn_rs		("6.957E8"),			// radius of the Sun (m)
			rn_sb		("5.670374419E-8"),		// Stefan–Boltzmann constant (W/(m^2 K^4))
			rn_u		("1.6605390666050E-27");// atomic mass unit (m(C12)/12  kg)
//-----------------------------------------------------------------------------------------------
// seen from outside
RealNumber	zero  		(rnNull), 
			half 		(rnHalf), 
			NaN 		(rnNaN),
			Inf 		(rnInf);
Constant
	//				 name     	value		 unit				  explanation
				// math
			e		{ u"e"		, rnE		, u"-"				, u"Euler's number"										,&rnE	 		},
			pi 		{ u"pi"		, rnPi		, u"-"				, u"π - half the circumference of a unit circle"		,&rnPi	 		},
			rpi		{ u"rpi"	, rnRpi		, u"-"				, u"1 / π, RealNumber::RN_1 / pi)"						,&rnRpi	 		},
			twoPi 	{ u"twoPi"	, rn2Pi		, u"-"				, u"2π"													,&rn2Pi	 		},
			piP2 	{ u"piP2"	, rnPiP2	, u"-"				, u"π/2"												,&rnPiP2		},
			piP4	{ u"piP4"	, rnPiP4	, u"-"				, u"π / 4, half * piP2)"								,&rnPiP4		},
			rpi2	{ u"rpi2"	, rnRPi2	, u"-"				, u"2 / π, RealNumber::RN_2 / pi)"						,&rnRPi2		},
			sqpi	{ u"sqpi"	, rnSqpi	, u"-"				, u"√π, sqrt(pi))"										,&rnSqpi		},
			sqrt2 	{ u"sqrt2"	, rnSqrt2	, u"-"				, u"√2"													,&rnSqrt2		},
			rsqrt2 	{ u"rsqrt2"	, rnPSqrt2	, u"-"				, u"1/√2"												,&rnPSqrt2		},
			sqrt3 	{ u"sqrt3"	, rnSqrt3	, u"-"				, u"√3"													,&rnSqrt3		},
			sqrt3P2 { u"sqrt3P2", rnSqrt3P2 , u"-"				, u"3√pi/2"												,&rnSqrt3P2  	}, 
			ln10 	{ u"ln10"	, rnLn10	, u"-"				, u"ln(10)"												,&rnLn10		},
			ln2		{ u"ln2"	, rnLn2		, u"-"				, u"natural logarithm of 2, ln2)"						,&rnLn2			},
			rln10  	{ u"rln10"	, pln10		, u"-"				, u"1/ln(10)"											,&pln10			},
			rln2	{ u"rln2"	, pln2		, u"-"				, u"1/ln(2)"											,&pln2			},
			log2e	{ u"log2e"	, rnLogB2E	, u"-"				, u"base 2 logarithm of e, log2(e))"					,&rnLogB2E		},
			lg10e	{ u"log10e"	, rnLgE		, u"-"				, u"base 10 logarithm of e, log10(e))"					,&rnLgE			},
			lge		{ u"lge"	, rnLge		, u"-"				, u"base 10 logarithm of e, log10(e))"					,&rnLge			},
	// physics 
			fsc		{ u"fsc"	, rn_fsc	, u"[-]"			, u"fine - structure constant"							, nullptr		},
			au		{ u"au"		, rn_au		, u"[m]"			, u"astronomical unit - exact value"					, nullptr		},
			c		{ u"c"		, rn_c		, u"[m/s]"			, u"speed of light in vacuum - exact value"				, nullptr		},
			eps0	{ u"eps0"	, rn_eps0	, u"[F/m=As/Vm]"	, u"vacuum electric permittivity - exact value"			, nullptr		},
			G		{ u"g"		, rn_G		, u"[m²/kg²s²]"		, u"Newtonian constant of gravitation"					, nullptr		},
			gf		{ u"gf"		, rn_gf		, u"[m/s²]"			, u"average g on Earth"									, nullptr		},
			h		{ u"h"		, rn_h		, u"[Js]"			, u"Planck constant"									, nullptr		},
			hbar	{ u"hbar"	, rn_hbar	, u"[Js]"			, u"reduced Planck constant (h/2π)"						, nullptr		},
			kb		{ u"kb"		, rn_kb		, u"[J/K]"			, u"Boltzmann constant"									, nullptr		},
			kc		{ u"kc"		, rn_kc		, u"[N m²/C²]"		, u"= 1/4πεo Coulomb constant"							, nullptr		},
			la		{ u"la"		, rn_la		, u"[1/mol]"		, u"Avogadro constant - exact value"					, nullptr		},
			me		{ u"me"		, rn_me		, u"[kg]"			, u"electron mass"										, nullptr		},
			mf		{ u"mf"		, rn_mf		, u"[kg]"			, u"mass of the Earth"									, nullptr		},
			mp		{ u"mp"		, rn_mp		, u"[kg]"			, u"proton mass"										, nullptr		},
			ms		{ u"ms"		, rn_ms		, u"[kg]"			, u"mass of the Sun"									, nullptr		},
			mu0		{ u"mu"		, rn_mu0	, u"[N/A²=Vs/m²]"	, u"4π*10ˉ⁷ vacuum magnetic permeability"				, nullptr		},
			qe		{ u"qe"		, rn_qe		, u"[C]"			, u"elementary charge"									, nullptr		},
			rfsc	{ u"rafs"	, rn_pfsc	, u"[-]"			, u"reciprocal of the fine structure constant (approx 137)", nullptr	},
			rf		{ u"rf"		, rn_rf		, u"[m]"			, u"radius of the Earth"								, nullptr		},
			rg		{ u"rg"		, rn_rg		, u"[J/mol K]"		, u"molar gas constant R"								, nullptr		},
			rs		{ u"rs"		, rn_rs		, u"[m]"			, u"radius of the Sun"									, nullptr		},
			sb		{ u"sb"		, rn_sb		, u"[W/m² K⁴]"		, u"Stefan–Boltzmann constant"							, nullptr		},
			u		{ u"u"		, rn_u		, u"[kg]"			, u"atomic mass unit (m[C12]/12)"						, nullptr		};


ConstantsMap constantsMap;


ConstantsMap::ConstantsMap()
{
	_Add(e);
	_Add(pi);
	_Add(rpi);
	_Add(twoPi);
	_Add(piP2);
	_Add(piP4);
	_Add(rpi2);
	_Add(sqpi);
	_Add(sqrt2);
	_Add(rsqrt2);
	_Add(sqrt3);
	_Add(sqrt3P2);
	_Add(ln10);
	_Add(ln2);
	_Add(rln10);
	_Add(rln2);
	_Add(log2e);
	_Add(lg10e);
	_Add(lge);
	_Add(fsc);
	_Add(au);
	_Add(c);
	_Add(eps0);
	_Add(G);
	_Add(gf);
	_Add(h);
	_Add(hbar);
	_Add(kb);
	_Add(kc);
	_Add(la);
	_Add(me);
	_Add(mf);
	_Add(mp);
	_Add(ms);
	_Add(mu0);
	_Add(qe);
	_Add(rfsc);
	_Add(rf);
	_Add(rg);
	_Add(rs);
	_Add(sb);
	_Add(u);
}

ConstantsMap::~ConstantsMap()	 // delete new values
{
	for(auto it = begin(); it != end(); ++it)
		if (!it->second->_builtin)		// then the constant is allocated 
			delete it->second;			// it deletes the new variable inside it too
}

void ConstantsMap::Add(const wchar_t* cname, const RealNumber cvalue, const wchar_t* cunit, const wchar_t* cdesc)
{
	Constant* pc = new Constant(cname, cvalue, cunit, cdesc);
	insert(end(), std::make_pair(pc->name, pc) ) ;
}
void ConstantsMap::Add(const String cname, const RealNumber cvalue, const String cunit, const String cdesc)
{
	insert(end(), { cname, new Constant(cname, cvalue, cunit, cdesc, nullptr)});
}
void ConstantsMap::_Add( Constant& c)	// existing builtin constant
{
	c._builtin = true;
	insert(end(), { c.name, &c });
}

void ConstantsMap::Rescale(int nmx)
{
	for (auto it = begin(); it != end(); ++it)
		it->second->Rescale(nmx);	// only re-scales if pBaseValue is set
}

//-------------- RealNumber ------------------------

size_t RealNumber::SetMaxLength(size_t mxl)	// returns original
{
	if (mxl > MaxAllowedDigits)
		mxl = MaxAllowedDigits;

	size_t res = _maxLength;
	_maxLength = mxl;
	_RescaleConstants((int)mxl);
	RedefineEpsilon();
	return res;
}


void RealNumber::_RescaleConstants(int maxLength)
{	// zero to 10 has just one valid digit, no need to modify them, ever
	// constants		
	constantsMap.Rescale(maxLength);
}

RealNumber RealNumber::operator=(const RealNumber& rn) 
{ 
	if (&rn != this)	// else same variables: nothing to do
	{
		_numberString = rn._numberString;
		_sign = rn._sign;
		_exponent = rn._exponent;
		_maxExponent = rn._maxExponent;
	}
	return *this; 
}

RealNumber RealNumber::operator=(RealNumber&& rn) noexcept
{
	if (&rn != this)	// else same variables: nothing to do
	{
		_numberString = rn._numberString;
		_sign = rn._sign;
		_exponent = rn._exponent;
		_maxExponent = rn._maxExponent;
		rn._SetNull();
	}
	return *this;
}

RealNumber RealNumber::operator=(const SmartString& rn)
{
	_numberString = rn;			// using the actual locale
	_FromNumberString();
	return *this;
}

RealNumber RealNumber::operator=(double value) // usually will loose accuracy as double is just 64bits long
{
	// special case for 0
	if (!value)
	{
		_exponent = 0;
		_sign = 1;
		_numberString = chZero;
		return *this;
	}

	_sign = value > 0 ? 1 : -1;
	value = std::abs(value);
	_numberString = std::to_string(value);	// will have a decimal point or equivalent
	//value = qAbs(value);
	//QString s;
	//s.setNum(value, 'E', 15);

	size_t xExp = _numberString.find_first_of(SCharT('E')); // < > 0, 
	_exponent = std::stol(_numberString.mid(xExp + 1).ToWideString()) + 1;	// format is 9.999999999999999E00 and we use 0.9999999999999999

	size_t precision = xExp - _decPoint.Unicode();	
	assert(precision > 1);
	// format X.YYYYYY
	for (size_t i =  2; i < xExp; ++i)
		_numberString[i - 1] = _numberString.at(i);
	_numberString.erase(precision);	// drop exponent and duplicated last digit 
	return *this;
}

RealNumber& RealNumber::operator++()
{
	if (IsNaN() || IsInf())
		return *this;
	RealNumber r("1");
	operator+=(r);
	return *this;
}

RealNumber RealNumber::operator++(int)
{
	RealNumber r(*this);
	(void)operator++();
	return r;
}

RealNumber& RealNumber::operator--()
{
	if (IsNaN() || IsInf())
		return *this;
	RealNumber r("1");
	operator-=(r);
	return *this;
}

RealNumber RealNumber::operator--(int)
{
	RealNumber r(*this);
	(void)operator--();
	return r;
}

bool RealNumber::operator==(const RealNumber& rn) const
{ 														   // works when both 'Inf' too
	return !IsNaN() && !rn.IsNaN() &&  _sign == rn._sign &&_numberString == rn._numberString && _exponent == rn._exponent  ; 
}

bool RealNumber::operator!=(const RealNumber& rn) const { return !operator==(rn); }
bool RealNumber::operator<(const RealNumber& rn) const
{
	if ( (IsNaN() || rn.IsNaN()) || (_sign > rn._sign) || (_sign == rn._sign && IsInf() && !rn.IsInf())	)
		return false;

	// sign <= rn._sign
	if( (_sign < rn._sign) || (!IsInf() && rn.IsInf()) || (_sign > 0 && (IsNull() || _exponent < rn._exponent)) || (_sign < 0 && _exponent > rn._exponent))
		return true;
	// sign == rn_sign
	if( (IsNull() && rn._sign > 0) || (_sign > 0 && _exponent > rn._exponent) || (_sign < 0 && _exponent < rn._exponent))
		return false;

	size_t i;	// exponents and sign are the same so digits matter only
	size_t len1 = _numberString.length(),
		   len2 = rn._numberString.length();
	for (i = 0; i < std::max(_leadingZeros + len1, rn._leadingZeros + len2); ++i)
	{
		SCharT ch = _CharAt((int)i), chrn = rn._CharAt((int)i);
		if (ch != chrn)
		{
			if ((_sign > 0 && ch < chrn) || (_sign < 0 && ch > chrn))
				return true;
			return false;
		}
	}
	return false;
}
bool RealNumber::operator<=(const RealNumber& rn) const
{
	return operator==(rn) || !operator>(rn);
}
bool RealNumber::operator>(const RealNumber& rn) const 
{
	return !operator==(rn) && !operator<(rn);
}
bool RealNumber::operator>=(const RealNumber& rn) const
{
	return operator==(rn) || !operator<(rn);
}

/*=============================================================
 * TASK   : change the number of decimal places in the number
 * PARAMS :	countOfDecimalDigits : this many digits after the
 *				decimal point when the number is used
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: if countOfDecimalDigits >= the number of decimal 
 *									  digits, nothing happens
 *			else the number is rounded to have exactly
 *						countOfDecimalDigits decimal places
 *------------------------------------------------------------*/

RealNumber RealNumber::Round(int countOfDecimalPlaces)
{
	/*
	*   number 
	*	Before rounding:
	*		number	c.D.Digits cc.S.Digits => string  precision _exponent
	*		99.98								9998,    4,         2
	*   After rounding:
	*		number	c.D.Digits cc.S.Digits => string  precision _exponent
	*		99.9800		 4			6		   9998     4,			2
	*		99.98		 2			4		   9998		4,			2
	*		100.0		 1			4	       1        1			3
	*/
	int cntIntegerDigits		= _exponent > 0 ? _exponent + 1 :0,
		cntLeadingDecimalZeros	= _exponent < 0 ? -_exponent : 0;
	_numberString = _RoundNumberString(_numberString,  cntIntegerDigits,  countOfDecimalPlaces, cntLeadingDecimalZeros);
	return *this;
}
RealNumber RealNumber::Rounded(int countOfDecimalPlaces) const
{
	RealNumber r(*this);
	return r.Round(countOfDecimalPlaces);
}

RealNumber RealNumber::RoundToDigits(int countOfSignificantDigits)
{
	int intLenIn = _exponent, intLenOut = intLenIn, roundPos=countOfSignificantDigits - 1; // position in string
	_RoundNumberString(_numberString,  intLenOut, roundPos, 0);
	if (intLenOut > intLenIn)
		++_exponent;
	return *this;
}
RealNumber RealNumber::RoundedToDigits(int countOfSignificantDigits) const
{
	RealNumber r(*this);
	return r.RoundToDigits(countOfSignificantDigits);
}

SmartString RealNumber::_ToBase(int base, size_t maxNumDigits) const	// base must be >1 && <= 16
{	
	if (IsNaN())
		return NAN_STR;

	// expects remainder to be <one (binary, octal, hexadecimal) < 16
	RealNumber tmp(*this),
				divisor = (base == 16 ? RN_16 : base == 8 ? RN_8 : base == 2 ? RN_2 : RN_10),
				remainder;
	SmartString convertedValue;

	tmp._sign = 1;	// always positive
	while (!tmp.IsNull() )
	{
		tmp = tmp._Div(divisor, remainder);
		int n = (int)remainder.ToInt64();
		convertedValue = String(1,ByteToMyCharT(n)) + convertedValue;
	}
	if (convertedValue.length() > maxNumDigits)
		convertedValue = SmartString((_sign < 0 ? "-": "")) + SmartString(INF_STR);

	return convertedValue;
}

/*=============================================================
 * TASK   :	return formatted integer part of sNumber for display
 * PARAMS :	sNumber - any bin,oct,dec or hex string
 *			fmt - gives the number format, the thousand separator
 *				  and if number prefix must be used
 *			nIntDigits - this many digits in sNumber are integer
 *				digits. May be larger than the length of 'sNumber'
 *				when 0: a single '0' character string is return
 *			chunkSize - this many digits are in one group
 *			prfxToAll - apply a number prefix to all group not just
 *				the first one
 * EXPECTS:	sNumber's format is the same as given in 'fmt'
 * GLOBALS: _sign
 * RETURNS:	number string for the integer part of the number 
 *			(includs delimiter.
 * REMARKS:	- returned string may end with '0's
 *------------------------------------------------------------*/
SmartString RealNumber::_IntegerPartToString(const SmartString& sNumber, int sign, const DisplayFormat& format, size_t &nIntDigits, size_t chunkSize, bool prfxToAll) const
{
	int len = sNumber.length(); 

	int sepLen = (int)format.strThousandSeparator.length(),
		nTrailingZeros = (int)nIntDigits > len ? nIntDigits - len : 0,
		cntDelimiters = nIntDigits <= 3 ? 0 : ((nIntDigits - 1) / chunkSize);
	bool useSign = false;

	// len  		0 1 2 3 4 5 6 7 8 9
	// len/3		0 0 0 1 1 1 2 2 2 3
	// len %3		0 1 2 0 1 2 0 1 2 0
	// (len-1)/3	0 0 0 0 1 1 1 2 2 2
	// (len)%4	 	0 1 2 3 0 1 2 3 0 1
	// sep cnt		0 0 0 0 1 1 1 2 2 2 
	// sep pos		- - - - 1 2	3
	// sign?
	if ((format.base == DisplayBase::rnb10 && (format.signOption != SignOption::soNormal || sign < 0)) ||
		((format.base == DisplayBase::rnbBin || format.base == DisplayBase::rnbHex) && sign < 0 && format.bSignedBinOrHex))
	{
		useSign = true;
	}
	SmartString sPrefix, sDelim = chSpace;

	switch (format.base)
	{
		case DisplayBase::rnbBin: sPrefix = SmartString(1, u'#'); break;
		case DisplayBase::rnbOct: sPrefix = SmartString(1, u'0'); break;
		case DisplayBase::rnbHex: sPrefix = SmartString("0x")   ; break;
		default:sDelim = format.strThousandSeparator;  break;	// rnb10
	}
	SmartString res;
	if (useSign)
	{
		res = _SignString(format);
	}

	if ( (format.base == DisplayBase::rnbOct && nIntDigits) || (format.useNumberPrefix && !sPrefix.empty()) )
		res += sPrefix;
	if (!nIntDigits)
		res += SmartString('0');
	else if (!sepLen)		// no separators: just prepare whole integer part of number
		res += sNumber.left(nIntDigits, chZero);
	else		// separate 'chunkSize' digit groups by separator and optionally by sPrefix too
	{
		int j = 0;
		while (j < (int)nIntDigits && j < (int) (nIntDigits % chunkSize))
			res += sNumber.at(j++, chZero);
		size_t dp = 0; // delimiter position
		while (j < (int)nIntDigits)
		{
			if (j && (dp % chunkSize) == 0)
			{
				res += sDelim;
				if (prfxToAll)
					res += sPrefix;
			}
			++dp;
			res += sNumber.at(j++, chZero);
		}
	}
	return res;
}

SmartString RealNumber::_DecimalPartToString(const SmartString& roundedString, const DisplayFormat& fmt, size_t& nIntegerDigits, int cntLeadingZeros) const
{
	size_t len = roundedString.length();  // no leading zeros in numberString!
	SmartString res;
	if (fmt.decDigits < 0 && len <= nIntegerDigits && !cntLeadingZeros)	// no decimal part
		return res;

	res += _decPoint;

	int n = nIntegerDigits;		// index in roundedString

	int dd = fmt.decDigits < 0 ? len - n + cntLeadingZeros : fmt.decDigits; // # of digits to be shown
	if (cntLeadingZeros && fmt.decDigits > cntLeadingZeros)
		cntLeadingZeros = fmt.decDigits;
	if (cntLeadingZeros)
	{
		res += SmartString(cntLeadingZeros, chZero);
		dd -= cntLeadingZeros;
	}

	int dp = 1;
	while (dd--)
	{
		if (fmt.useFractionSeparator && ((dp++ % 3) == 0))
			res += chSpace;
		res += roundedString.at(n++, chZero);
	}

	if (fmt.decDigits < 0)	// remove trailing zeros from fractional part
	{
		res.erase(std::find_if(res.rbegin(), res.rend(), [](SCharT ch) {return ch != chZero; }).base(), res.end());
		if (res.at(res.length() - 1, chZero) == _decPoint)
			res.pop_back();	// decimal point
	}
	return res;
}

 /*=============================================================
  * TASK   : show integer number in binary form
  * PARAMS : 'format' - describes the display format
  * EXPECTS: the number is an integer
  * GLOBALS:
  * RETURNS: formatted binary string w. or. w.o. '#' prefix
  *			 which may also be 'Inf' and 'NaN' in which case 
  *			'_eFlags' will contain 'rnfUnderflow' for negative and
  *			'rnfOverFlow' error for positive numbers (TODO)
  * REMARKS:  - if 'format.thousandSeparator' is set
  * 		 		use a space as separator between every 4 binary digits
  *					In this case there may even be leading zeros.
  * 		  - if 'format.bSignedBinOrHex' is true show the 2's complement
  * 		 		for negative numbers with as many digits as  
  * 		 		'format.nFormatSwitchLength'+1
  * 		 		In this case never use a sign before the number.
  * 		 		If not set and the number is negative, use a negative sign
  *			  - signs are always put before the number prefix
  * 		  - if 'format.mustUseSign' is true and 'format.bSigngnedBinOrHex' is not
  * 		 		set put a '+' sign before the prefix for positive numbers
  *------------------------------------------------------------*/
SmartString RealNumber::ToBinaryString(const DisplayFormat& format) const
{
	SmartString bin = _ToBase(2, _maxLength+LengthOverFlow);

	if (IsInf() || IsNaN())
		return _numberString;

	if ( (bin == u"Inf" && !IsInf()) || (bin == u"NaN" && !IsNaN()))
		return u"Too long";

	size_t lenb = bin.length();

	auto addone = [&bin](int i, int &carry)
	{	// 1's complement and add carry

		bin[i] = bin.at(i) == chZero ? chOne : chZero; // on's complement
		if (carry)
		{
			bin[i] = bin.at(i) == chZero ? chOne : chZero;
			if (bin.at(i) == chOne)
				carry = 0;
		}
	};
	if(_eFlags.count(EFlag::rnfInvalid))	
		bin = INF_STR;

	if (bin == INF_STR)
	{
		// const ! _eFlags.insert(_sign < 0 ? EFlag::rnfOverflow : EFlag::rnfOverflow);
		bin = _SignString(format) + bin;
		return bin;
	}
	if (bin == NAN_STR)
		return bin;

	if (_sign < 0 && !format.bSignedBinOrHex)	// then use 2's complement
	{
			// 2's complement
		int carry = 1;
		for (int i = (int)lenb - 1; i >= 0; --i)
			addone(i, carry);
	}

	return _IntegerPartToString(bin, 1, format, lenb, 4, false);
}
/*=============================================================
  * TASK   : show integer number in octal base
  * PARAMS : 'format' - describes the display format
  * EXPECTS: the number is an integer
  * GLOBALS:
  * RETURNS: formatted octal string that always starts with a 0
  *			 which may also be 'Inf' and 'NaN' in which case
  *			'_eFlags' will contain 'rnfUnderflow' for negative and
  *			'rnfOverFlow' error for positive numbers (TODO)
  * REMARKS:  - if 'format.thousandSeparator' is set
  * 		 		use a space as separator between every 3 octal digits
  * 		  - if 'format.bSignedBinOrHex' is true show the 8's complement
  * 		 		for negative numbers with as many digits as
  * 		 		'format.nFormatSwitchLength'+1
  * 		 		In this case never use a sign before the number.
  * 		 		If not set and the number is negative, use a negative sign
  * 		  - if 'format.mustUseSign' is true and 'format.bSigngnedBinOrHex' is not
  * 		 		set put a '+' sign before the number for positive numbers
 *------------------------------------------------------------*/
SmartString RealNumber::ToOctalString(const DisplayFormat& format) const
{
	if (IsInf() || IsNaN())
		return _numberString;

	SmartString oct = _ToBase(8, _maxLength+LengthOverFlow);	// w.o. leading '0'
	//oct = SmartString("0")+ oct;
	size_t leno = oct.length();

	if ( (oct == u"Inf" && !IsInf()) || (oct == u"NaN" && !IsNaN()))
		return u"Too long";

	if (_eFlags.count(EFlag::rnfInvalid))
		oct = INF_STR;

	if (oct == INF_STR)	  // it may have sign when sign is required
	{
		// const ! _eFlags.insert(_sign < 0 ? EFlag::rnfOverflow : EFlag::rnfOverflow);
		oct = SmartString(_sign < 0 ? "-" :
			(format.signOption == SignOption::soAlwaysShow ? "+" :
				(format.signOption == SignOption::soLeaveSpaceForPositive ? " " : ""))) + oct;
		return oct;
	}
	if (oct == NAN_STR)
		return oct;

	size_t n = leno;	// we need 'n' places in displayable result
						// including a single space for "thousand separator"

	return _IntegerPartToString(oct, _sign, format,leno,3,false);
}

/*=============================================================
  * TASK   : show integer number in hexadecimal base
  * PARAMS : 'format' - describes the display format
  * EXPECTS: the number is an integer
  * GLOBALS:
  * RETURNS: formatted hex. string that may or may not start with 0x
  *			 which may also be 'Inf' and 'NaN' in which case
  *			'_eFlags' will contain 'rnfUnderflow' for negative and
  *			'rnfOverFlow' error for positive numbers (TODO)
  * REMARKS:  - if format is not IEEEFormat::rntHexNotIEEE
  *					and the number is larger/smaller than the
  *					largest/smallest IEEE number (float or double)
  *					then "BADSmartString("is printed
  *			  - NumberFormat::LittleEndian is only used when
  *					the format is not HexFormat::rnHexNormal
  *					In all other cases it reveses the byte order
  *			  - if 'format.thousandSeparator' is set
  * 		 		use a space as separator between every 2 hexa digits
  * 		  - if 'format.bSignedBinOrHex' is true show the 16's complement
  * 		 		for negative numbers with as many digits as
  * 		 		'format.nFormatSwitchLength'+1
  * 		 		In this case never use a sign before the number.
  * 		 		If not set and the number is negative, use a negative sign
  *			  - signs are always put before the number prefix
  * 		  - if 'format.mustUseSign' is true and 'format.bSigngnedBinOrHex' is not
  * 		 		set put a '+' sign before the number for positive numbers
 *------------------------------------------------------------*/
SmartString RealNumber::ToHexString(const DisplayFormat &format) const
{
	if (IsInf() || IsNaN())
		return _numberString;

	SmartString hex;
	if (format.trippleE != IEEEFormat::rntHexNotIEEE)
	{
		LDouble ld = ToLongDouble();
		float f = (float)ld;
		double d = (double)ld;
		char* p = (char*)&d;
		size_t size = 8;	// in bytes for IEEE754Double
		if (format.trippleE == IEEEFormat::rntHexIEEE754Single)
		{
			f = (float)ld;
			p = (char*)&f;
			size = 4;
		}

		for (size_t i = 0; i < size; ++i)
			hex += ToHexByte(*(p++));
	}
	else
		hex = _ToBase(16, _maxLength+LengthOverFlow);

	if ( (hex == u"Inf" && !IsInf()) || (hex == u"NaN" && !IsNaN()))
		return u"Too long";

	if (hex.length() & 1)	// hex string must always have even number of characters
		hex = "0"_ss + hex; // the leading 0 will be dropped if the number has
							// a prefix and not LSB order is used
	// lambda
	auto addone = [&hex](int i, int& carry)
	{	
		// 15's complement and add carry, used when 'format.mustUseSign'
		uint8_t n = MyCharTToByte(hex.at(i));
		n = 15 - i;
		if (carry)
		{
			n += + 1;
			if (n == 16)
				n = 0;
			else
				carry = 0;
		}
		hex[i] = ByteToMyCharT(n);
	};
	// /lambda

	if (_eFlags.count(EFlag::rnfInvalid))
		hex = INF_STR;

	size_t lenh = hex.length();
	int sign = _sign;
	if (sign < 0 && !format.bSignedBinOrHex)	// then use 16's complement
	{
		// 16's complement
		int carry = 1;
		for (int i = (int)lenh - 1; i >= 0; --i)
			addone(i, carry);
		sign = 1;								// complement with positive sign
	}

	if (hex == INF_STR)
	{
		// const ! _eFlags.insert(_sign < 0 ? EFlag::rnfOverflow : EFlag::rnfOverflow);
		hex = _SignString(format) + hex;
		return hex;
	}
	if (hex == NAN_STR)
		return hex;

	// hex is not inf or nan
					 // hex always have an even number of digits here and may start with a '0' character
	if (format.littleEndian)
	{
		for (size_t i = 0; i < lenh / 2; i += 2)
		{
			std::swap(hex[i], hex[lenh - i - 2]);
			std::swap(hex[i + 1], hex[lenh - i - 2 + 1]);
		}
	};

	size_t lenhMod4 = lenh % 4,	
		   lenhMod8 = lenh % 8;

	size_t chunkLength = _maxLength + LengthOverFlow;

	if (format.hexFormat == HexFormat::rnHexNormal)	// not using the strThousandSeparator
	{
		if (format.useNumberPrefix)
		{
			if (hex[0] == SCharT('0'))
			{
				hex.erase(0, 1);
				--lenh;
			}
		}
	}
	else // not normal: there are many possibilities, 
	{	 //	but 'hex' always has an even number of characters										
		switch (format.hexFormat)
		{
			case HexFormat::rnHexByte:  chunkLength = 2; 		  // 2BDC546291F4B => 02 BD C5 46 29 1F 4B
				break;
			case HexFormat::rnHexWord:  chunkLength = 4; 
				if (lenhMod4)									  // prefix with '0's to have chunks of 4 digits
					hex = SmartString(4 - lenhMod4, SCharT('0')) + hex;
				break;
			case HexFormat::rnHexDWord: chunkLength = 8; 
				if (lenhMod8)
					hex = SmartString(8 - lenhMod8, SCharT('0')) + hex;
				break;
			default: break;
		}
	}

	bool bPrefixToAllChunks = chunkLength <= 8;
	lenh = hex.length();
	return _IntegerPartToString(hex, sign, format, lenh, chunkLength, bPrefixToAllChunks);
}

/*=============================================================================
 * TASK   : create a string representation of the RealNumber
 * PARAMS :	format (I)			 : how to display - se Remarks below
 * EXPECTS:	mainFormat is rnfNormal, rnfSci or rnfEng. No text format allowed
 * GLOBALS:
 * RETURNS: number as a SmartString  that may contain exponent after an 'E'
 * REMARKS:	- the result may contain thousand and decimal separators
 *			- if the format is 'rnfNormal' or 'rnfGeneral' and
 *				the integer part (sign+digits+separators+decimal point) 
 *				is already longer than 'displWidth, the format changes to Sci,
 *				otherwise the fractional part is reduced and rounded.
 *				This may strip the fractional part completely.
 *			- if even the Sci format string with no fractional digit but with
 *				the exponent string (e.g. E123, \cdot10^{123} or <sup>123</sup>)
 *				is wider a string of '#' characters is displayed
 *			- used fields of format
 *				format.strThousandSeparator	: if not empty put this string between groups of 3 digits
 *				format.decDigits 			: # of digits after the decimal point -1: all
 *				format.displWidth			: width in characters, 0: doesn't matter
 *				format.nFformatSwitchLength	: switch to scientific notation if the length
 *									of the converted string w.o. the
 *									exponent is greater than this
 *				format.useFractionSeparator : use separator character right of the decimal 
 *				format.mainFormat			: normal or sci/eng
 *				format.expFormat			: 
 *			- '_exponent' points to the position of the decimal sign
 *				when < 0  the decimal point is left of _numberString
 *----------------------------------------------------------------------------*/
SmartString RealNumber::ToDecimalString(const DisplayFormat &format) const
{
	if (IsNaN())
		return _numberString;
	SmartString signStr = _SignString(format);
	if (IsInf())
		return 	signStr + _numberString;
		// special case for 0, which reamins 0 even for Sci and Eng modes
	if (format.base == DisplayBase::rnb10 && !_exponent && (_numberString.empty() || (_numberString.length() == 1 && _numberString.at(0, chZero) == chZero)))
	{
		if (format.decDigits > 0)
			return SmartString("0."_ss) + SmartString(format.decDigits, chZero);
		else
			return SmartString(chZero);
	}
	
		// create a decimal string in the required format for the number without rounding
		// including optional sign, decimal point,thousand and decimal separators, exponent
		// but use just as many digits as format.decDigits and format. displayWidth allow

	int exp = _exponent;			// position of the decimal point. 
									// Example #1 exp = 0, _numberstring = 123 => number  = .123
									// Example #2 exp = 1, _numberstring = 123 => number  = 1.23

	DisplayFormat fmt = format;		// may change

			 // set/change formats when needed
	size_t	nIntegerDigits = exp > 0 ? exp : 0;	// in _numberString (logically right extended by '0's when needed)
	size_t  nWidthIntegerPart = nIntegerDigits;	// width of whole integer part of formatted string w.o. sign
	size_t  nDisplayWidth = fmt.displWidth <= 0 ? size_t(-1) : fmt.displWidth;

	int nTmp = (int)_numberString.length() - (int)nIntegerDigits;
	size_t  nReqdDecDigits = fmt.decDigits < 0 ? (nTmp <= 0 ? 0 : (size_t)nTmp) : fmt.decDigits;	// this many decimal digits
	size_t nWidthFractionalPart = 0;

	int cntThousandSeparators = (fmt.strThousandSeparator.empty() ? 0 : ((nWidthIntegerPart > 3 ? nWidthIntegerPart / 3 : 0) - 1 + (nWidthIntegerPart % 3 ? 2 - (nWidthIntegerPart % 3) : 0)));
	if(cntThousandSeparators > 0)
			nWidthIntegerPart = nWidthIntegerPart + cntThousandSeparators * fmt.strThousandSeparator.length();
	size_t nSizeSign = fmt.signOption != SignOption::soNormal || _sign < 0 ? 1 : 0;

	SmartString strExponent;
	size_t expLen = 0;
	// ---- lambda ----------
	auto requriedSpaceForFraction = [&]()->int	   // incl decimal point
		{
			return nWidthFractionalPart = nReqdDecDigits + (fmt.useFractionSeparator ? ((nReqdDecDigits - 1) / 3) : 0) + (nReqdDecDigits ? 1 : 0);
		};
	auto calculateExponentAndIntegerDigits = [&]()
		{
			// 10's exp:    5  4  3  2  1  0	-1 -2 -3 -4 
			// new Texp:	3  3  3  0  0  0	-3 -3 -3 -6
			// int digits:	3  2  1  3	2  1	 3  2  1  0
			// Texp %3:	    2  1  0  2	1  0	-1 -2  0 -1
			// Texp/3:		1  1  1	 0	0  0	 0	0 -1 -1
			// 
			// exp			6  5  4  3  2  1	 0 -1 -2 -3 
			// new exp:		4  4  4  1	1  1	-2 -2 -2 -5
			// int digits:	3  2  1  3	2  1	 3  2  1  0
			// exp/3:		1  1  1	 0	0  0	 0	 0 -1
			// exp%3        2  1  0	 2	1  0	-1 -2  0
			exp = _exponent;
			if (fmt.mainFormat == NumberFormat::rnfNormal || fmt.mainFormat == NumberFormat::rnfGeneral)
				nIntegerDigits = exp > 0 ? exp : 0;
			else if (fmt.mainFormat == NumberFormat::rnfSci)
				nIntegerDigits = exp > 0 ? 1 : 0;
			else // fmt.mainFormat == NumberFormat::rnfEng
			{
				nIntegerDigits = (exp < 0 ? (3 + (exp + 1) % 3) : (exp % 3 + 1));		// 1,2, or 3
				exp = (exp < 0 ? ((exp + 1) / 3 - 1) : (exp / 3)) * 3;
			}
			--exp;	// real 10's exponent
			strExponent = _FormatExponent(fmt, exp);
			if (fmt.mainFormat == NumberFormat::rnfSci || fmt.mainFormat == NumberFormat::rnfEng)
				expLen = strExponent.length();
			++exp;
		};

	auto requiredSpaceforIntegerDigits = [&]() -> int// how many characters are needed for the integer digits 
		{											 // 'nIntegerDigits' characters from _numerString are the integer part
													 // w.o. separators, sign or decimal point
			if (fmt.mainFormat == NumberFormat::rnfGeneral || fmt.mainFormat == NumberFormat::rnfNormal) // then use rnfNormal
			{
				nWidthIntegerPart = nIntegerDigits;	// width of whole integer part of formatted string w.o. sign
				cntThousandSeparators = (fmt.strThousandSeparator.empty() ? 0 : ((nWidthIntegerPart > 3 ? nWidthIntegerPart / 3 : 0) - 1 + (nWidthIntegerPart % 3 ? 2 - (nWidthIntegerPart % 3) : 0)));
				if (cntThousandSeparators > 0)
					nWidthIntegerPart = nWidthIntegerPart + cntThousandSeparators * fmt.strThousandSeparator.length();
			}
			else if (fmt.mainFormat == NumberFormat::rnfEng)
				nWidthIntegerPart = std::abs(exp % 3) + 1;						// ENG		fmt.mainFormat == NumberFormat::rnfEng
			// else Sci format, nWidthIntegerPart is already 1
			else // rnfSci
				nWidthIntegerPart = 1;
			nWidthIntegerPart = nWidthIntegerPart ? nWidthIntegerPart : 1;		// on display: always use an integer part
			return nWidthIntegerPart;
		};
	// ---- /lambda ----------

	// try to fit the displayed number into nDisplayWidth using the actual values of
	// fmt, exp, nIntegerDigits, nReqdDecDigit
	// if it doesn't fit 

	// general format is either normal or sci
	(void)calculateExponentAndIntegerDigits();
	(void) requiredSpaceforIntegerDigits();

	if (fmt.mainFormat == NumberFormat::rnfGeneral || fmt.mainFormat == NumberFormat::rnfNormal)
	{
		// too many integer digits or leading zeros or too large/small number?
		if( (size_t)std::abs(exp) > fmt.nFormatSwitchLength)
		{
			fmt.mainFormat = NumberFormat::rnfSci;
			nReqdDecDigits = fmt.decDigits < 0 ? (_numberString.empty() ? 0 : _numberString.length() - 1) : _numberString.length();
			(void)calculateExponentAndIntegerDigits();
			(void)requiredSpaceforIntegerDigits();
		}
		else
			fmt.mainFormat = NumberFormat::rnfNormal;
	}
		// get exponent string
	--exp;	// real 10's exponent
//	strExponent = _FormatExponent(fmt, exp); 

	// format could have been modified to sci from general or normal
	// now check fractional part including decimal point and delimiters

	(void)requriedSpaceForFraction();	// get nWidthFractionalPart

	// round number to given fractional digits
	// The rounding may increase the length of the number string

	++exp;
	SmartString roundedString;		// rounded decimal string w.o. delimiters, decimal point or exponent leading or trailing zeros
	SmartString ns = _numberString;

	int nIntegerDigitsAfter;
	int nLeadingDecimalZeros = exp >= 0 ? 0 : -exp;
	int nrp = nLeadingDecimalZeros + nReqdDecDigits;
	roundedString = _RoundNumberString(ns, nIntegerDigitsAfter = nIntegerDigits, nrp, nLeadingDecimalZeros);
	if (nIntegerDigits != nIntegerDigitsAfter)
	{
		if (fmt.mainFormat == NumberFormat::rnfSci)	// SCI
			++exp;									// but nIntegerDigits doesn't change
		else if (fmt.mainFormat == NumberFormat::rnfEng)
		{
			int nPrevExpSize = strExponent.length();
					// when 0.999 changed to 1.000 nIntLength won't change, we need 10s exponent here so do not increase 'exp' yet
			calculateExponentAndIntegerDigits();		// there can be no thousand delimiters in SCI or ENG numbers
		}
	}
	// now create the number string
	size_t nResultLength = nSizeSign + nWidthIntegerPart + nLeadingDecimalZeros + nWidthFractionalPart + expLen; // DEBUG line
	SmartString result;
	int sign = nWidthFractionalPart == 0 && roundedString.empty() ? 1 : _sign;
	result += _IntegerPartToString(roundedString, sign, fmt, nIntegerDigits, 3, false);
	result += _DecimalPartToString(roundedString, fmt, nIntegerDigits, nLeadingDecimalZeros);
	if (expLen)
		result += strExponent;

	return result;
}

SmartString RealNumber::ToString(const DisplayFormat& format, TextFormat textFormat) const
{
	if (format.mainFormat == NumberFormat::rnfText)
		return ToTextString(format, textFormat);

	SmartString sres;
	switch (format.base)
	{
		default:
		case DisplayBase::rnb10:
			sres = ToDecimalString(format); break;
		case DisplayBase::rnbHex:
			sres = ToHexString(format); break;
		case DisplayBase::rnbOct:
			sres = ToOctalString(format); break;
		case DisplayBase::rnbBin:
			sres = ToBinaryString(format); break;
	}

	// sres may contain delimiters in the decimal part

	if (format.displWidth >=0 && sres.length() > (size_t)format.displWidth)
	{
		DisplayFormat fmt = format;
		int posD, posE;	// set in sres.length() > fmt.displWidth
		// if possible shorten fractional part and modify sres
		// if not fills 'sres' with '#' characters
		// only called when the number format is SCI or ENG
		// ENG is not switched back to SCI !
		auto ShortenFractPart = [&]() -> bool
			{
				int lenI = posD < 0 ? 0 : posD,					// length of integer part w. sign and w.o. decimal point and delimiters
					pos2 = posE >= 0 ? posE : sres.length(),	// after the fractional part may include delimiters
					lenF = pos2 - lenI - 1,						// length of fractional part w. decimal delimiters
					lenO = lenI + (sres.length() - pos2) + 1,	// length of all non-fractional part w.o. exponent
					nLeadingZerosInFraction = _exponent < 0 ? - _exponent : 0;
				// see if the fractional part can be rounded so that the whole number fit
				// examples:  displWidth == 4, SCI 
				//	#		_exp 	| #string		length	 pos1   pos2   	result	  is OKround 
				//	8765     10		| 8.765E9		 7		  1		 5		 9E9		+	
				//	9765     10		| 9.765E9		 7		  1		 5		 1E10		+	
				//	9765     20		| 9.765E19		 8		  1		 5		 1E20		+	
				//	9765     100	| 9.765E99		 8		  1      5		 1E100		-	
				if (fmt.displWidth - lenO < 0)		// less than no room for fractional part :)
					return false;
				
				int flenF = fmt.displWidth - lenO,	// this many space for fractional decimal digits invl. leading 0s and separators
					cntSep = fmt.useFractionSeparator ? (flenF - 1) / 3 : 0; // which contains this many separators
										// which leaves
				lenF = flenF - cntSep;// -nLeadingZerosInFraction;	// decimal characters in fraction
				if (lenI && ( posD < 0 || sres[0] != chZero) )
					--lenF;
				RealNumber r(sres);
				r.Round(lenF);
				sres = r.ToDecimalString(fmt);
				return (int)sres.length() <= fmt.displWidth;
			};

		if (format.base == DisplayBase::rnb10)
		{
			if ((int)sres.length() > fmt.displWidth) // Too Long To Display
			{
				posD = sres.indexOf(DecPoint());				
				posE = _PosExpInNumberString(fmt, sres, posD + 1);

				if (ShortenFractPart())
					return sres;

				if (fmt.mainFormat != NumberFormat::rnfSci &&
					fmt.mainFormat != NumberFormat::rnfEng)
				{		 // try again in SCI mode
					fmt.mainFormat = NumberFormat::rnfSci;
					sres = ToDecimalString(fmt);		
					if ((int)sres.length() > fmt.displWidth)		// still too long: can't display
					{
						if (!ShortenFractPart())
							sres = SmartString(fmt.displWidth, u'#');
					}
				}
				else if (!ShortenFractPart())	// already sci or eng and too long
						sres = SmartString(format.displWidth, u'#');
				return sres;
			}
		}
		else
		{
			if (format.displWidth < (int)String(u"Too long").length())
				sres = SmartString(format.displWidth, u'#');
			else
				sres = SmartString("Too Long");
		}
	}

	return sres;
}

SmartString RealNumber::_FormatExponent(const DisplayFormat fmt, int exp) const
{
	SmartString s = SmartString(std::to_string(exp));
	switch (fmt.expFormat)
	{
		case ExpFormat::rnsfE:
			s = SmartString("E")+ s;
			break;
		case ExpFormat::rnsfSciHTML:
			s = SmartString("x10<sup>")+ s + SmartString("</sup>");
			break;
		case ExpFormat::rnsfSciTeX:
			s = SmartString(u"\\cdot10^{") + s + u"}";	// or "\\cdot10^("+s+")"
			break;
		case ExpFormat::rnsfGraph:
		default:
			s = SmartString(1, SCharT(183)) + u"10^{" + s + u"}";
			break;
	}

	return s;
}

int RealNumber::_PosExpInNumberString(const DisplayFormat fmt, const SmartString& s, int fromPos) const
{
	size_t posE;
	switch (fmt.expFormat)
	{
		case ExpFormat::rnsfSciHTML:
			posE = s.indexOf(u'x', fromPos + 1);
			break;
		case ExpFormat::rnsfSciTeX:
			posE = s.indexOf(u'\\', fromPos + 1);
			break;
		case ExpFormat::rnsfGraph:
			posE = s.indexOf(SCharT(183), fromPos + 1);
			break;
		default: // E
			posE = s.indexOf(u'E', fromPos + 1);
			break;
	}
	return posE;
}


SmartString RealNumber::ToTextString(const DisplayFormat& format, TextFormat dtf) const
{
	DisplayFormat fmt(format);
	fmt.useNumberPrefix = false;
	fmt.hexFormat = HexFormat::rnHexNormal;
	fmt.bSignedBinOrHex = false;

	RealNumber r(*this);
	r._exponent = (int)_numberString.length();	// make integer number

	// use all digits even with non-integers and forget exponent
	SmartString strh(r.Round((int)_maxLength)._ToBase(16, _maxLength)),
				str;
	size_t len = strh.length(), start=0;
	SCharT ch;
	if (len & 1)	// odd number of digits
	{
		start = 1;
		ch = (char)0xFF;
		str.push_back(ch);
	}
	for (size_t i = start; i < len; i += 2)
	{
		ch = strh[i];
		ch = (MyCharTToByte(SCharT(strh[i])) << 4) + MyCharTToByte(SCharT(strh[i + 1]));
		str.push_back(ch);
	}
	switch (dtf)
	{ 
		case TextFormat::rntfUtf8:
			//!! TODO
			break;
		case TextFormat::rntfUtf16:
			//!! TODO
			break;
		default:
			break;
	}
	return str;
}

LDouble RealNumber::ToLongDouble()	const
{
	if (IsNaN())
		return std::nanl("");
	if (IsInf())
		return _sign * INFINITY;
	// overflow?
	static RealNumber mx(LDBL_MAX), mn(LDBL_MIN);
	if (mx < *this)
		return std::nanl("");
	if( mn > *this)
		return 0.0;

	DisplayFormat format;
	format.mainFormat = NumberFormat::rnfSci;
	format.decDigits = 18;
	format.nFormatSwitchLength = 16;

	return strtod(ToDecimalString(format).ToUtf8String().c_str(),nullptr );
}

int64_t RealNumber::ToInt64() const
{
	// no call for ToString() as we don't need it
	RealNumber r = Int();
	static RealNumber lx((int64_t)LLONG_MAX) ;
	if (r.Abs() >= lx)
		throw("Number can't fit in a 64 bit integer");
	if (r._exponent > (int)r._numberString.length())
		r._numberString += SmartString(r._exponent - (int)r._numberString.length(), u'0');
	return std::strtoll(r._numberString.ToUtf8String().c_str(), nullptr, 10);
}

RealNumber RealNumber::Int()	const
{
	if(_exponent <= 0)
		return RealNumber(zero);
	if (_exponent > (int)(_maxLength + LengthOverFlow) )
		return RealNumber(INF_STR);
	RealNumber r(*this);
	if(_exponent <= (int)_numberString.length())
		r._numberString = _numberString.left(_exponent, chZero);

	return r;
}

bool RealNumber::ConvertToInteger()
{
	bool result = true;
	if (IsNaN() || IsInf())
		return false;
	if (_exponent <= 0)
	{
		_numberString.clear();
		_exponent = 1;
	}
	else if (_exponent <= (int)_numberString.length())
	{
		_numberString.erase(_exponent);
	}
	else if (_exponent >= (int)(_maxLength + LengthOverFlow) )
	{
		_numberString = INF_STR;
		result = false;
	}
	return result;
}

RealNumber RealNumber::Frac()	const
{
	if (_exponent <= 0)
		return RealNumber(*this);
	if (_exponent > (int)(_maxLength+LengthOverFlow) )
		return RealNumber(INF_STR);
	RealNumber r(*this);
	if (r._exponent >= (int)Precision())
			r._SetNull();
	else
	{
		r._numberString.erase(0, r._exponent);
		r._exponent = 0;
	}
	return r;
}

RealNumber RealNumber::_PowInt(const RealNumber& power) const // integer powers only but not 0 or 1
{
	RealNumber x(*this);
	if (power.IsEven())
		x._sign = 1;

	RealNumber n(power);

	if (power== RN_1 )			// when "+1"
		return x;
	else if(n._sign < 0)		// when negative
	{
		x = RN_1 / x;
		n._sign = -n._sign;
	}
	// special case:  (10^x)^power = 10^(x*power)
	if (_numberString == SmartString("1"))	
	{
		return TenToThePowerOf((_exponent - 1) * std::stoi(power._numberString.ToUtf8String()));
	}
	/* "exponentiate by squaringSmartString("algorithm
	* wikipedia
	*   if n < 0 then
	*		x := 1 / x;
	*		n := -n;
	*	if n = 0 then return 1
	*	y := 1;
	*	while n > 1 do
	*		if n is odd then
	*			y := x * y;
	*		x := x * x;
	*		n := floor(n / 2);
	*	return x * y
	  */
	RealNumber y(RN_1);
	if (n.Sign() < 0)
	{
	}
	while (n > RN_1)
	{
		if (n.IsOdd())
			y = x * y;
		x = x * x;
		n = (n / RN_2).Int();	// == floor
	}
	return x * y;
}
RealNumber RealNumber::Pow(const RealNumber &power) const
{
	if (!IsValid())
		return *this;
	if (!power.IsValid())
		return power;
	if (power.IsNull())
		return IsNull() ? NaN : RN_1;	// 0^0 is undefined
	if (IsNull())							// 0^x = 0
		return *this;

	if (power == RN_1)				// x^1 = x
		return *this;
	if (power == -RN_1)			// x^(-1) = 1/x
		return RN_1 / *this;

	if (_sign < 0 && (power < RealNumber::RN_1))	// maybe if we could detect r = 1/n, where n is odd
		return NaN;					// then we could return the corresponding root //!!TODO

	RealNumber	rnIntPart  = power.Int(),
				rnFracPart = power.Frac();


	if (!rnIntPart.IsNull())
		rnIntPart = _PowInt(rnIntPart);
	else
		rnIntPart = RN_1;
	// fractional part of exponent
	if (rnFracPart.IsNull())
		rnFracPart = RN_1;
	else if (*this != rnE)
		rnFracPart = exp(rnFracPart * ln(*this));
	else		// e^x, where  -1<x<1
		rnFracPart = exp(rnFracPart);
	return rnIntPart * rnFracPart;
}

/*=============================================================
 * TASK   : Add 2 numbers and return the result
 * EXPECTS:
 * PARAMS :	*this,	- left summand
 *			rnOther - right summand  these may have different precisions
 *			negateOther - used for subtraction
 * GLOBALS:
 * RETURNS: resulting number may contain fewer or more significant digits
 * REMARKS:	- if (after possible sign correction) the sign of the two numbers
 *			differ this will subtract the smaller absolute value 
 *			number from the larger absolute value number then
 *			corrects the sign of the result
 *------------------------------------------------------------*/
RealNumber RealNumber::_Add(const RealNumber& rnOther, bool negateOther) const
{
	if (IsNaN())
		return *this;
	if (rnOther.IsNaN())
		return rnOther;

	RealNumber left(*this);	
	RealNumber right(rnOther);	

	if (IsInf() || rnOther.IsInf())
	{
		if (IsInf() && rnOther.IsInf())
			return _sign > 0 ? left : (right._sign > 0 ? right : left);
		else if (!IsInf())
			return right;
		else
			return left;
	}

	if(negateOther)
		right._sign = - right._sign;

	int diff = _exponent - rnOther._exponent;
	if (diff > (int)(_maxLength + LengthOverFlow))
	{			
		left._eFlags.insert( EFlag::rnfUnderflow);
		return left;
	}
	else if(diff < -(int)(_maxLength + LengthOverFlow))
	{
		right._eFlags.insert( EFlag::rnfUnderflow);
		return right;
	}

	if (diff > 0)	// exponent of left is larger
		right._ShiftSmartString(diff);
	else if (diff < 0)
		left._ShiftSmartString(-diff);

	if (left._sign == right._sign)	// both the same sign
	{
		_AddStrings(left, right); 
		return left;
	}
	else	// subtraction: always subtract the smaller absolute valued number from the larger absolute valued one
	{
		int lsign = left._sign,	// left and right signs will be opposite
			rsign = right._sign;

		left._sign = right._sign = 1;

		if (right == left)
		{
			left._SetNull();
			return left;
		}
		else if (right < left)  
		{
			_SubtractStrings(left, right);
			left._sign = lsign;
			return left;
		}
		else					// negative + positive
		{
			_SubtractStrings(right, left);
			right._sign = rsign;
			return right;
		}
	}
}

RealNumber RealNumber::_Subtract(const RealNumber& rnOther) const // from this number
{
	return _Add(rnOther, true);	// minuend - subtrahend = minuend + (-subtrahend)
}

RealNumber RealNumber::_Multiply(const RealNumber& rnOther) const
{
	RealNumber left(*this);	// adjust number to larger precision
	left._sign *= rnOther._sign;
	if (rnOther.IsPure10Power())
	{
		left._exponent += rnOther._exponent - 1;
		return left;
	}

	RealNumber right(rnOther);	// i.e. when parameter +- diff is positive

	if (IsNaN() || ( IsNull() && !rnOther.IsInf()) )
		return left;
	if (rnOther.IsNaN() || (!IsInf() && rnOther.IsNull()))
		return right;

	if ((IsNull() && rnOther.IsInf()) || (IsInf() && rnOther.IsNull()) )
	{
		left._SetNaN();
		return left;
	}

	if (IsInf() || rnOther.IsInf())
	{
		if (IsInf() && rnOther.IsInf())
		{
			left._sign = _sign == rnOther._sign ? 1 : -1;
			return left;
		}
		else if (!IsInf())	
			return right;
		else
			return left;
	}
	// here neither of the numbers is Nan or Inf or 0
	// and both have the same precision

	left._AddExponents(right._exponent);
	_MultiplyStrings(left, right);
	return left;
}

bool RealNumber::__IsDivideByZero(RealNumber& dividend, const RealNumber& divisor) const
{
	if (divisor.IsNull())		// division by 0
	{
		dividend._eFlags.insert( EFlag::rnfDivBy0);
		if (IsNaN() || IsNull() || IsInf())
		{
			dividend._SetNaN();
			return true;
		}
		dividend._SetInf();
		return true;
	}

	if (divisor.IsInf())	// divide by infinity
	{
		dividend._SetNull();
		return true;
	}
	if (divisor.IsNaN())
	{	dividend._SetNaN();
		dividend._eFlags.insert( EFlag::rnfInvalid);
		return true;
	}
	return false;
}

RealNumber RealNumber::_Divide(const RealNumber& rnOther) const
{
	RealNumber left(*this);	// adjust number to larger precision
	left._sign *= rnOther._sign;
	if (rnOther.IsPure10Power())
	{
		left._exponent -= rnOther._exponent-1;
		return left;
	}

	RealNumber right(rnOther);	// i.e. when parameter +- diff is positive

	if (__IsDivideByZero(left, rnOther))	// handle null, inf, nan
		return left;

	_DivideInternal(left, right);
	return left;
}

RealNumber RealNumber::_Div(const RealNumber& divisor, RealNumber& remainder) const
{
	RealNumber rem;
	rem._SetNull();
	// RealNumber d(this->Int()), dv(divisor.Int());	// result, divident, divisor
	RealNumber d(*this), dv(divisor);	// result, divident, divisor

	if (__IsDivideByZero(d, dv))
		return d;
	int sl = d._sign, sr = dv._sign;

	d._sign = dv._sign = 1;

	if (d < dv)	// integer division when dividend is smaller than divisor
	{
		d._sign = sl;
		rem = d;
		d._SetNull();
	}
	else if (d == dv)
	{
		d = RealNumber("1");
		d._sign = sl * sr;
	}
	else
	{	// signs are not yet set back
		_DivideInternal(d, dv, &rem);	// d = result * dv + rem
		rem._sign = sl;
		d._sign = sl * sr;
	}

	remainder = rem;

	return d;
}

RealNumber RealNumber::_LogOpWith(const RealNumber& ra, LogicalOperator lop) const	// only integer part!
{
	int sign = _sign * ra._sign;
	if (IsNaN() || ra.IsNaN() )
		return NaN;
	if (IsInf() || ra.IsInf())
		return Inf * RealNumber(sign);

	SmartString ssLeft = this->Int().Abs()._ToBase(16, RealNumber::MaxLength()),
				ssRight = ra.Int().Abs()._ToBase(16, RealNumber::MaxLength());

	int diff = _exponent - ra._exponent;
	if (diff > (int)(_maxLength + LengthOverFlow))
	{
		RealNumber left(*this);
		left._eFlags.insert(EFlag::rnfUnderflow);
		return left;
	}
	else if (diff < -(int)(_maxLength + LengthOverFlow))
	{
		RealNumber right(ra);
		right._eFlags.insert(EFlag::rnfUnderflow);
		return right;
	}

	size_t	lenLeft = ssLeft.length(),
			lenRight = ssRight.length(),
			bigger = lenLeft > lenRight? lenLeft: lenRight;

	auto __toSize = [](SmartString& s, size_t size)
		{
			if (s.length() < size)
				s = SmartString(size - s.length(), chZero) + s;
		};
	auto __fromHexChar = [](SCharT ch)->int16_t
		{
			int res = ch.Unicode();
			if (res < '9')
				res = res & 0x0F;
			else
				res = (res - 'A') + 0xA;
			return res;
		};
	auto __toHexChar = [](int16_t ch)->SCharT
		{
			if (ch < 0xA)
				return SCharT('0' + ch);
			else
				return SCharT('A' + (ch- 0xA) );
		};
	auto __or = [](int16_t l, int16_t r)	// of two hexadecimal character digits
		{
			return l | r;
		};
	auto __and = [](int16_t l, int16_t r)	// of two hexadecimal character digits
		{
			return l & r;
		};
	auto __xor = [](int16_t l, int16_t r)	// of two hexadecimal character digits
		{
			return l ^ r;
		};

	__toSize(ssLeft, bigger);
	__toSize(ssRight, bigger);
	switch (lop)		// all characters in _numberString are ASCII decimal places
	{
		case LogicalOperator::lopOr:
			for (size_t i = 0; i < bigger; ++i)
				ssLeft[i] = __toHexChar( __or(__fromHexChar(ssLeft[i]), __fromHexChar(ssRight[i])) );
			break;
		case LogicalOperator::lopXOr:
			for (size_t i = 0; i < bigger; ++i)
				ssLeft[i] = __toHexChar( __xor(__fromHexChar(ssLeft[i]), __fromHexChar(ssRight[i])) );
			break;
		case LogicalOperator::lopAnd:
			for (size_t i = 0; i < bigger; ++i)
				ssLeft[i] = __toHexChar( __and(__fromHexChar(ssLeft[i]), __fromHexChar(ssRight[i])) );
			break;
	}
	ssLeft.erase(ssLeft.begin(), std::find_if(ssLeft.begin(), ssLeft.end(), [](SCharT ch) { return ch != chZero; }));
	ssLeft = u"0x" + ssLeft;
	RealNumber rln(ssLeft);
	rln._sign = sign;
	return rln;
}

/*=============================================================
 * TASK   : From a shifted number string and exponent
 *			create a proper normalized number
 * PARAMS :	none
 * EXPECTS: _isNormalized
 * GLOBALS:
 * RETURNS: nothing
 * REMARKS: non-normalized strings are created when real numbers
 *			are shifted to the right introducing leading zeros
 *			which may or may not be present in the string
 *------------------------------------------------------------*/
void RealNumber::_Normalize()
{
	if (_isNormalized)
		return;
	std::locale loc = std::cout.getloc();
	size_t n=0;
	for (size_t i = 0; i < _numberString.length() && _numberString[i] == chZero; ++i)
		++n;
	if (n)
	{
		_numberString.erase(0, n);
		_exponent -= (int)n;
	}
	if (_leadingZeros)
	{
		_exponent -= _leadingZeros;
		_leadingZeros = 0;
	}
	_isNormalized = true;
}

long RealNumber::_AddExponents(long oneExp, long otherExp, EFlagSet& efs) const
{
	int64_t ex = (int64_t)oneExp + (int64_t)otherExp;
	int64_t	dx = std::abs(ex);
	if (dx >= (int)_maxExponent)	// as _maxExponent is just an integer this always work
	{
		efs.insert(ex > 0 ? EFlag::rnfOverflow : EFlag::rnfUnderflow);
		return oneExp;
	}
	else
		return (long)ex;
}
void RealNumber::_AddExponents(int otherExp)
{
	long ex = _AddExponents(_exponent, otherExp, _eFlags);
	if (_eFlags.count(EFlag::rnfOverflow) || _eFlags.count(EFlag::rnfUnderflow))
		_SetInf();
	else
		_exponent = ex;
}

/*=============================================================
 * TASK:	take number string and convert to inner format
 * EXPECTS: inps: all uppercase number string
 *			- precision, _maxExponent set
 * PARAMS:	
 * GLOBALS:	_numberString, _sign, _exponent
 * RETURNS: nothing, 
 *				- number set in class members
 *				- _eFlags are also set
 * REMARKS:	- string may contain binary numbers starting w. '#' or
 *			  octal numbers starting by 0 with no decimal point or exponent or
 *			  floating point decimal number
 *			- only use this when the number is set from string or 
 *			  double as no check is made if it was already normalized
 *			- _numberString may not yet precision long
 *			- quetly discards non numeric or invalid parts of the string
 *			- maxExponent may be enlarged if the exponent given in string is larger,
 *			  however max possible exponent is 0x7fffffffl
 *------------------------------------------------------------*/
void RealNumber::_FromNumberString()
{
	_numberString.Trim();
	_numberString.toUpper();

	int sgnPos = 1;
	_sign = 1;
	if (_numberString.empty())		// number is 0, exponent and others don't count
		return;

	if (_numberString[0] == SCharT('-'))
		_sign = -1;
	else if (_numberString[0] != SCharT('+'))
		sgnPos = 0;

	if (sgnPos)
		_numberString.erase(0, 1);

	if (_numberString.empty())		// only sign character was present
	{
		_SetNaN();
		_eFlags.insert(EFlag::rnfMalformed);
		return;
	}

	if (_numberString == NAN_STR || _numberString == INF_STR)
		return;

	int positionOfError=0;	// if there's an error in the number string

	// exponent 
	_exponent = 0;
	int posE = _numberString.indexOf(SCharT('E'));	// if a string has an exponent it must be decimal

	if (posE > 0)
	{
			SmartString sExp = _numberString.mid((size_t)posE + 1);	// separate exponent and number
			_numberString.erase(posE, std::string::npos);
			// only sign and decimal places are allowed in exponent
			positionOfError = sExp.indexOfRegex(SmartString("[^-+0-9]"));
			if (positionOfError >= 0)
			{
				sExp = sExp.left(positionOfError);
				_eFlags.insert(EFlag::rnfMalformed);
				_SetNaN();
				return;
			}
			_exponent = std::stoi(sExp.ToUtf8String());
	}
	else if (posE == 0)
	{
		positionOfError = 0;
		_eFlags.insert(EFlag::rnfMalformed);
		_SetNaN();
		return;
	}

	std::smatch matches;
	std::regex re;
	SmartString pattern;	// regular expression as a string for the decimal (not exponent or sign) part

	Base base = Base::dec;	// number is decimal, hexadecimal, octal or binary, but decimal is the default

	// get number base here (no sign in string)
	if (_numberString.at(0) == chZero)			// hex:0x012, oct:067, dec:001.012300, dec:.12300, dec: 0123E-1
	{
		if (_numberString.at(1) == SCharT('X'))
		{
			_numberString.erase(0, 2);
			pattern = SmartString("[^0-9A-F]");			// even a decimal point means malformed hexadecimal string
			base = Base::hex;
		}
		else if (/*posE > 0 &&*/ (_numberString.indexOf(_decPoint) >= 0 || (_exponent && _exponent < (int)_numberString.length()) ))// explicit and implicit (e.g. 1E-3) decimal point
		{																										 // decimal and not octal string
			pattern = SmartString("[^0-9") + _decPoint.Unicode() + SmartString("]");
			
			size_t n = _numberString.find_last_of(_decPoint);
						// remove leading zeros from integer part, trailing zeros are removed after exponent set
			_numberString.erase(_numberString.begin(), std::find_if(_numberString.begin(), _numberString.end(), [](SCharT ch) {return ch != chZero; }));
		}
		else
		{
			pattern = SmartString("[^0-7]");				  // otherwise they are octal, so we do not need the leading 0 nor any other 0 char. after it
			_numberString.erase(_numberString.begin(), std::find_if(_numberString.begin(), _numberString.end(), [](SCharT ch) {return ch != chZero; }));
			base = Base::oct;									  // and the starting zero is just a prefix
		}
	}
	else if (_numberString.at(0) == SCharT('#'))					  // binary string
	{
		pattern = SmartString("[^01]");
		_numberString.erase(0, 1); // then erase any leading zeros
		_numberString.erase(_numberString.begin(), std::find_if(_numberString.begin(), _numberString.end(), [](SCharT ch) {return ch != chZero; }));
		base = Base::bin;
	}
	else		  // decimal string which did not start with a zero, but may end with zeros
	{
		pattern = SmartString("[^0-9") + _decPoint.Unicode() + SmartString("]");
		
	}

	// here _exponent is set from explicit value and removed from the string (e.g 12345.678E-3 -> _numberString = 12345.678, _exponent = -3)
	int posDp = -1;					// position of decimal point
	int cntLeadingZeros = 0;		// in converted number. may be non 0 only for numbers that are less than 1 (e.g. 0.0001)
	if(base == Base::dec)
	{
		posDp = _numberString.indexOf( _decPoint);
		if (posDp < 0)					// integer number w. possible explicit exponent
			_exponent += (int)_numberString.length();
		else							// numbers with fractional part
		{
			int pos2ndDecPoint = _numberString.indexOf(_decPoint, posDp+1); // 2nd dec. point or
			positionOfError = _numberString.indexOfRegex(pattern);			// any non-allowed character?
			if (positionOfError >= 0 || pos2ndDecPoint >=0)
			{
				_numberString = _numberString.left(positionOfError);
				_eFlags.insert(EFlag::rnfMalformed);
			}
			_exponent += posDp;

			if (std::abs(_exponent) > (int)_maxExponent)
			{
				_exponent = (int)_maxExponent;
				_eFlags.insert(EFlag::rnfOverflow);
			}
			// remove decimal point
			_numberString.erase(posDp, 1);	// however the number now may have leading zeros
			if (posDp == 0)					// dec. point as first character (exmpl: .00123 -> 00123)
			{
				auto it = std::find_if(_numberString.begin(), _numberString.end(), [](SCharT ch) {return ch != chZero; });
				if( (cntLeadingZeros = it - _numberString.begin()) > 0)
					_exponent -= cntLeadingZeros;
				_numberString.erase(_numberString.begin(), it);
			}
		}
		// remove trailing zeros
		_numberString.erase(std::find_if(_numberString.rbegin(), _numberString.rend(), [](SCharT ch) {return ch != SCharT('0');}).base(), _numberString.end());
	}

		// -----------------------------------------------------
		// from here _numberString may only contain 
		// decimal string - with digits and possibly a decimal point
		// hexadecimal strings - digits 0-9,A-F
		// octal string - with digits 0-7, 
		// or binary string  -of 0 and 1
		// characters
		// -----------------------------------------------------
	
	
	//------------------------ lambda for common part for integer conversion from non-decimal bases
	auto _convert = [&](const RealNumber &multiplier)
	{
		RealNumber number = zero;
		int ch;
		for (int i = 0; i < (int)_numberString.length(); ++i)
		{
			ch = MyCharTToByte(_numberString.at(i));
			number = number * multiplier + RealNumber(ch);
		}
		_numberString = number._numberString;
		_exponent = number._exponent;
	};

	if (base == Base::hex)	// decimal base already handled
	{
			_convert(RN_16);
	}
	else if(base == Base::bin)
	{
		_convert(RN_2);
	}
	else if(base == Base::oct)
		_convert(RN_8);

	// copy number
	size_t len = _numberString.size();		// can be 0
	if (len < 1)
		len = 1;

	if (_numberString.length() > MaxAllowedDigits)
	{
		int cntDecDigits = MaxAllowedDigits,
			cntIntDigits = 0;
		_RoundNumberString(_numberString, cntIntDigits, cntDecDigits, cntLeadingZeros);
		if (cntIntDigits)	// increased because of rounding
			if (++_exponent > MaxExponent)
			{
				_SetNaN();
				_eFlags.insert(_sign > 0 ? EFlag::rnfOverflow : EFlag::rnfUnderflow);
			}
	}
	_isNormalized = true;
}

/*=============================================================
 * TASK   : Logically not physically shifts number string  
 *			to the right by setting _leadingZeros and modifying
 *			_exponent
 * PARAMS :	byThisAmount => logically shifts number string right
 * EXPECTS:	number string length is same as precision
 * GLOBALS:	_leadingZeros, _exponent, _numberString
 * RETURNS: nothing
 * REMARKS: - right shift is just logical, all digits in
 *			  _numberString is kept
 *			- result string will be at least 1 character long
 * Example:   98765 (E3), byThisAmount = 5 => .....98765 (E8)
 *            00098765 (E3) byThisAmount = -3 => 98765 (E0)
 *------------------------------------------------------------*/
void RealNumber::_ShiftSmartString(size_t byThisAmount)
{
	_leadingZeros = (int)byThisAmount;
	_exponent += (int)byThisAmount;
}

void RealNumber::_ShiftSmartString(RealNumber& rn, int byThisAmount)
{
	return rn._ShiftSmartString(byThisAmount);
}

/*=============================================================
 * TASK   : round a number string of base 10 digits
 *			The number string may be shorter than the rounding position
 *			
 * PARAMS :	numString (I)	: number string to be rounded. It has no
 *							  decimal point, sign or exponent. 
 *							  May even be empty. It does not contain 
 *							  leading or trailing zeros
 *							  Usually this is '_numberString'
 *							- May be shorter than 'cntIntDigits'+'cntDecDigits'
 *			intLen (IO)	
 *							- This many digits of numString are in the integer part
 *							  that is the exponent of the number (position of decimal point)
 *							- May be increased by 1 after rounding if all rounded digits are
 *								9s In that case all digits, but the first one will be 0
 *							- when zero then there's no integer part, all digits in 
 *								'numString' are decimal digits. 'leadingZeros' may be > 0
 *			rPos (I) : virtual length of the fractional part of the rounded number 
 *							after the decimal point OR -1
 *							- if >= 0 rounding occurs startig at the 
 *								'intLen' + 'rPos' real/virtual position
 *							- if < 0 nothing happens (no rounding)
 *			leadingZeros   : when not 0 the leading zeros in the decimal digits
 * EXPECTS:
 * GLOBALS:
 * RETURNS:  the rounded number string without leading and trailing zeros
 * REMARKS: - may add 1 to 'intLen' if there's an overflow for the integer digits
 *			  In that case all but the first logical digit is 0 and the result is "1"
 *			- if the string is shorter than 'intLen' + 'leadingZeros' + 'rpos'
 *				then there is nothing to do
 *			- trailing zero characters are discarded as a result the
 *				returned string may be empty
 *------------------------------------------------------------*/
SmartString RealNumber::_RoundNumberString(SmartString &numString, int &exponent, int rPos, int leadingZeros) const
{
	if (rPos < 0 || numString.at(0).IsAlpha()) 		// use all digits or string (NaN, Inf)
		return numString;

		// index of the digit to round up the string with
	int	roundPos = exponent - leadingZeros + rPos;						// may be larger than length of 'numString'
	if (roundPos >= (int)numString.length())									// after the real string
		return numString;

	SCharT
		one  = SCharT('1'),
		zero = SCharT('0'),
		five = SCharT('5'),
		nine = SCharT('9');

	int carry = numString.at(roundPos, zero) >= five ? 1 : 0;
	SmartString res = numString.left(roundPos);
	if(leadingZeros < roundPos && roundPos < leadingZeros + (int)res.length())
		res.erase(roundPos);
	if (!carry)
		return res;

	SCharT ch=zero;
	while (roundPos-- && carry)
	{
		ch = numString.at(roundPos, chZero).Unicode() + carry;
		if (ch > nine)
			res.pop_back();
		else
		{
			carry = 0;
			res[roundPos] = ch;
		}
	}

	if (carry) // then 'res' is empty, and we have an overflow of '1'
	{
		res = SmartString(one);
		++exponent;
	}
	return numString = res;
}

void RealNumber::_SetNull()
{
	_exponent = 0;
	_numberString = "";
}

void RealNumber::_SetNaN()
{
	_numberString = NAN_STR;
}
void RealNumber::_SetInf()
{
	_numberString = INF_STR;
}

SCharT RealNumber::_AddDigits(SCharT digit1, SCharT digit2, int& carry)	 const
{
	SCharT d0 = chZero;
	SCharT d1 = digit1 - d0,
		   d2 = digit2 - d0;
	d1 += SCharT(carry);

	if ((d1 += d2).Unicode() > 9)
	{
		carry = d1.Unicode() / 10;
		d1 = d1.Unicode() % 10;
	}
	else
		carry = 0;
	return SCharT(d1+d0);
}

SCharT RealNumber::_SubtractDigits(SCharT digit1, SCharT digit2, int& borrow) const
{
	SCharT d0 = chZero;
	SCharT d1 = digit1 - d0,
		   d2 = digit2 - d0;
	d2 = d2.Unicode() + borrow;
					
	if (d1 < d2)
	{
		d1 = d1.Unicode() + 10;
		borrow = 1;
	}
	else
		borrow = 0;
	return SCharT(d1 - d2 + d0);
}

/*=============================================================
 * TASK   : Add the two strings of two RealNumber
 * EXPECTS:	- neither number is infinite or NaN
 *			- both has the same sign
 *			- the string of the smaller number might
 *			  be logically 'shifted' to the right to have the
 *			  same exponent as that of the larger 
 *			- the number strings may have different physical lengths
 *			- the difference in exponents was smaller than 
 *			  the constant _maxLength
 * PARAMS :	left - the larger absolute velue number
 *			right -the smaller absolute valued number
 * GLOBALS: 
 * RETURNS:	nothing
 * REMARKS:	- returns the result in the 'left' argument
 *			- the result string and exponent are adjusted and
 *				normalized
 *			- the result string and exponent are adjusted and
 *				normalized
 *------------------------------------------------------------*/
void RealNumber::_AddStrings(RealNumber &left, RealNumber &right) const
{
	size_t	ll = left.Precision() + left._leadingZeros,
			lr = right.Precision() + right._leadingZeros,
			l = std::max(ll, lr);

	if (l > _maxLength + LengthOverFlow)
		l = _maxLength + LengthOverFlow;
	String result(l,chZero);

	int trailingchZeros = 0;
	bool bTrailing = true;
	int carry = 0;
	for (int i =  (int)l - 1; i >= 0; --i)
	{
		result[i] = _AddDigits(left._CharAt(i), right._CharAt(i), carry);
		if (bTrailing && result.at(i) == chZero)
			++trailingchZeros;
		bTrailing = false;
	}
	_CorrectResult(left, result, trailingchZeros, carry);
}
/*=============================================================
 * TASK   : subtract right from left
 * EXPECTS: - left and right are not equal
 *			- left has the larger absolute value
 *			- both has the same sign
 *			- the smaller one is 'shifted' to have
 *			  the same exponent
 * PARAMS : minuend, subtrahend: REAL_NUMBERS
 * GLOBALS:
 * RETURNS: nothing
 * REMARKS: - returns the result in 'minuend'
 *			- the result string and exponent are adjusted and
 *				normalized
 *------------------------------------------------------------*/
void RealNumber::_SubtractStrings(RealNumber& minuend, RealNumber& subtrahend) const
{		// always the smaller from the larger
	size_t	ll = minuend.Precision() + minuend._leadingZeros,
			lr = subtrahend.Precision() + subtrahend._leadingZeros,
			l = std::max(ll, lr);

	String result(l,chZero);

	int trailingchZeros = 0;
	int borrow = 0;
	bool bTrailing = true;
	//int l = _numberString.length();
	for (int i =  (int)l - 1; i >= 0; --i)
	{
		if ((result[i] = _SubtractDigits(minuend._CharAt(i), subtrahend._CharAt(i), borrow)) == chZero && bTrailing)
			++trailingchZeros;
		bTrailing = false;
	}
	_CorrectResult(minuend, result, trailingchZeros, 0);	// no borrow as minuend was larger than subtrahend
}

/*=============================================================
 * TASK   : multiply two strings
 * EXPECTS:	- neither number is infinite or NaN
 *			- strings may have different sizes
 * PARAMS :	left, right: REAL_NUMBERS to be multiplied
 * GLOBALS:
 * RETURNS: none
 * REMARKS:	- the result is returned in 'left'
 *------------------------------------------------------------*/
void RealNumber::_MultiplyStrings(RealNumber& left, RealNumber& right) const
{
	size_t	ll = left. Precision() + left._leadingZeros,
			lr = right.Precision() + right._leadingZeros,
		l = ll + lr - 1 + LengthOverFlow;	// for overflow
	unsigned chZ = chZero.Unicode();

	SmartString result(l, SCharT(0) ); // not chZero till the end (?NOT SmartString?)
	unsigned digLeft, digRight, digResult, digTmp, fdaTmp;	// not numeric character, but the numbers themselves
	for (size_t i = 0; i < lr; ++i)		// multiply left starting at the most significant digit of right
	{
		digRight = right._CharAt((int)i).Unicode() - chZ;
		int fda=0;	// overflow of accu and on result
		for (int j = (int)ll - 1; j >= 0; --j)						  // multiply by one digit from the right
		{														  // and add it to partial result, which gives
			digLeft = left._CharAt(j).Unicode() - chZ;					  // intermediate result
			digResult = digLeft * digRight + fda;
			fda = digResult >= 10 ? digResult / 10 : 0;
			digTmp = result.at((int)(LengthOverFlow + i) + j).Unicode() + digResult - fda * 10;
			fdaTmp = digTmp >= 10 ? digTmp / 10 : 0;
			fda += fdaTmp;
			result[LengthOverFlow + i + j] = char16_t(digTmp - fdaTmp * 10);
		}
			// deal with overflow
		for (int j = (int)(LengthOverFlow + i - 1); j >= 0 && fda; --j)
		{
			digResult = result.at(j).Unicode() + fda;
			fda = digResult >= 10 ? digResult / 10 : 0;

			result[j] = char16_t(digResult - fda * 10);
		}
	}
	// calculate count of leading zeros
	size_t cntZeros = 0;
	while (result.at(cntZeros).Unicode() == 0)
		++cntZeros;
	left._exponent += (int)(LengthOverFlow - cntZeros)-1;	// overflow in decimal places changes exponent
	if (cntZeros)			   // remove them
	{
		size_t i = cntZeros;
		for(size_t j = 0; i < l; ++i, ++j)
			result[j] = result.at(i) + chZero;
		l -= cntZeros;
		result.erase(l, String::npos);
	}

	result.erase(std::find_if(result.rbegin(), result.rend(), [](SCharT ch) {return ch != chZero; }).base(), result.end());
//	size_t maxLength = _maxLength + LengthOverFlow;
	if (result.length() > _maxLength)
	{
		int tensOverflow = 0;
		int cntLeadingZeros = 0,
			roundPos = (int)_maxLength;	// round from here

		_RoundNumberString(result, tensOverflow, roundPos, cntLeadingZeros);	
		if(roundPos < (int)result.size())
			result.erase(roundPos, std::string::npos);
		if (tensOverflow)
			++left._exponent;
	}
	left._numberString = result;
}

/*=============================================================
 * TASK   :	 divide the string representation of two
 *			 0 normalized  numbers independent of
 *			 the signs of the arguments, which must be adjusted
 *			 later
 * PARAMS :	left  - dividend
 *			right - divisor
 *			pRemainder -  pointer to remainder or nullptr
 *					set pRemainder if you want integer divison
 *					with remainder AND both left and right
 *					are integers
 * EXPECTS:	exponent of left is set to the difference of
 *					exponents after this function
 *			If pRemainder is not null both left and right 
 *				have to be integer (with an exponent > 0) 
 *				but no check is made to ensure it
 * GLOBALS:
 * RETURNS: none: result in left
 * REMARKS: - left's exponent is increased if the
 *				ratio is larger than one, otherwise it is
 *				not modified
 *			- pRemainder may point to either left or right
 *			- signs for the result (and remainder) are set
 *				outside of this fucnction
 *			- algorithm 
 *					notation: 
 *						physical digit- a digit character from a number string
 *										never a leading zero, but can be a trailing one
 *						logical digit - either a physical digit ('left' or 'dividend')
 *										or a zero digit after the string all used up
 * 
 *				0. when pRemainder != nullptr and 'left' is smaller than or equal to 'right'
 *					we are already done just set '*pRemainder' to either 'left' or 0
 *					and quotient to 0 or 1 respectively.
 *				So from now on either 'pRemainder' is NULL or 'left' was larger than 'right'
 *				1. Create dividend from 'left' with at least as many logical digits as in the divisor.
 *				2. Increase the number of logical digits of the 'dividend' from 'left' until
 *						a) (when prRmainder == nullptr) - dividend is larger than the divisor
 *						b) (with pRemainder) - either a) is true or all (logical) integer digits	
 *							are used up, in which case we return with the 'dividend' as a remainder;
 *						loop #1
 *				3. Get the digit of the 'quotient' by either 
 *						a) subtracting the 'divisor' from the 'dividend' as many times (max 9) as possible
 *							before the result becomes negative. This changes
 *							'dividend' possibly leaving a series of leading zeros in it.
 *						b) getting the first guess of the next digit of quotient by dividing the integer 
 *							from the first or first 2 digit from 'dividend'
 *							with the integer value of the first digit from the divisor.
 *							If the divisor multiplied by this number is larger than the
 *							'dividend' decrease the quotient and multiply it again with the divisor.
 *							Subtract the result of this multiplication from the quotient
 *				4. Discard leading zeros from 'dividend'.
 *						loop #2
 *				5. If 'pRemainder' and all integer digit from left is used up set '*pRemainder'
 *					 from 'dividend', put the quotient into 'left', and return,
 *				6. Get the next (logical) digit from 'left' and append it to 'dividend'
 *				7. If 'dividend' is still smaller than 'divisor' then append a zero digit to it
 *					snd repeat from 5,else repeat from 3
 *------------------------------------------------------------*/
void RealNumber::_DivideInternal(RealNumber& left, RealNumber& right, RealNumber* pRemainder) const
{	
	if (left.IsNaN() || right.IsNaN())
		return left._SetNaN();
				   // simplest case for integer division and remainder
	if (pRemainder)
	{
		pRemainder->_SetNull();
		if (left == right)
		{
			left = RealNumber("1");
			return;
		}
		if (left < right)
		{
			*pRemainder = left;
			left._SetNull();
			return;
		}
	}
																		// int div example "123000/3210"
	size_t  lp = left. Precision(),										// str: "123", exp:10, lp:3
			rp = right.Precision()//,										// str: "321", exp: 4, rp:3
		    /*l  = lp+rp*/;													// l: 6
	size_t maxLength = _maxLength + LengthOverFlow;
	SmartString quotient(maxLength, chZero);				// q: 0000...0 - not as string
	EFlagSet efs;
	long exponentOfResult = _AddExponents(1 + left._exponent, - right._exponent, efs);	   // 1+ : compensation for number representation (0=> number < 1)
	if (!efs.empty())
	{
		left._SetInf();
		return;
	}
	// at start divident will have as many digits as the divisor. It does not start with
	// a '0' character, but may contain trailing zeros
	SmartString dividend = left._numberString.left(rp, chZero),			// d: "1230 changes during execution
				divisor  = right._numberString;							// dv:"321"
	if (dividend.compare(divisor) < 0)									// then first quotient digit would be a 0
	{																	// which decreases the exponent
		exponentOfResult = _AddExponents(exponentOfResult, -1, efs);
		if (!efs.empty())
		{
			left._SetInf();
			return;
		}
	}

	size_t iq = 0;		// index in quotient, finish division if dividend is zero (or empty) and iq > lp

	size_t il = rp;		// logical index in 'left._numberString'				//  il:3
						// of the mext digit to expand dividend with
						// when divisor is larger than the part
						// in dividend

	//************* Lambdas
	// extends divident after deleting the leading zeros in it with any number of digits from 'left'
	// until it has as many digits as the divisor has
	// Returns: false if divident can't be extended, true if it was extended
	// When called first time when 'with a 'dividend' has the same length as 'divisor'
	enum ResultT { rtOk, rtNo, rtError };
	auto extendDividend = [&]() -> ResultT
	{
		if (dividend[0] == chZero)	// erase leading zeros
			dividend.erase(dividend.begin(), std::find_if(dividend.begin(), dividend.end(), [](SCharT ch) {return ch != chZero; }));

		size_t len = dividend.length();
			   // maxLength = _maxLength + LengthOverFlow already defined outside;

		bool canHaveZeroQuotient = len < rp;
		// if after the leading zeros removed dividend is shorter than divisor and 'il' then add one digit from 'left' if it is possible
		// this does not add a 0 digit to the quotient, because this is a natural extension
		if (iq < maxLength  && len < rp && (!pRemainder || (pRemainder && il < (size_t)left._exponent)) )
		{
			dividend += left._numberString.at((int)il++,chZero); // no need to check parameter here
			++len;
		}
		// now either dividend is >= divisor or must extend dividend with one digit from 'left'
		// bot now each new digit introduces a 0 into the result
		while (iq < maxLength && len < rp && (!pRemainder || (pRemainder && il < (size_t)left._exponent)) )
		{
			dividend += left._numberString.at((int)il++,chZero); // no need to check parameter here
			++len;
			quotient[iq++] = chZero;
		}
			// if no more space or dividend is zero and 'il' > 'lp'
		if (iq >= maxLength || (il > lp && (dividend.empty() || dividend.find_first_not_of(chZero) == String::npos)))
			return rtNo;
		if (pRemainder && il >= (size_t)left._exponent && len < rp)
			return rtNo;
		if (len != rp)
			return len > rp ? rtOk : rtNo;

		SCharT tmp;
		int borrow = 0;
		for (int i = (int)len-1; i >= 0; --i)
			tmp = _SubtractDigits(dividend.at(i,chZero), divisor.at(i,chZero), borrow);
		if (borrow)	// then we need to extend the divident if it is possible
		{
			if(canHaveZeroQuotient)
				quotient[iq++] = chZero;
			if (iq < maxLength && (!pRemainder || (pRemainder && il < (size_t)left._exponent)) )
			{
				dividend += left._numberString.at((int)il++,chZero); // no need to check parameter here
				return rtOk;
			}
			return rtNo;
		}
		return rtOk;
	};
	auto divisorSmallerThanOrEqualToDividend = [&]()->bool	// divisor's first digit is never '0'
	{	// dividend may start with 0	
		size_t dlen = dividend.length();
		size_t is = 0;
		for (; is < dlen; ++is)
			if (dividend.at((int)is,chZero) != chZero)
				break;
		if (dlen -is != rp)
			return (dlen-is > rp);

		SCharT tmp;
		int borrow = 0;
		for (int i =  (int)dlen - 1, j = (int)rp -1; j >= 0; --i, --j)
			tmp = _SubtractDigits(dividend.at(i,chZero), divisor.at(j,chZero), borrow);
		return !borrow;
	};
	auto subtractDivisorFromDividend = [&]()
	{	
		size_t len = dividend.length();
		size_t extension = len > rp ? 1 : 0;
		int borrow = 0;
		for (int i =  (int)len - 1, j = (int)rp -1; j >= 0; --i, --j)
			dividend[i] = _SubtractDigits(dividend.at(i,chZero), divisor.at(j,chZero), borrow);
		if (extension)									  // 666|6 - 700*1 => 596|6 - 700*2 => ... 106|6 - 700 => 036|6 
			dividend[0] = dividend[0] - borrow;
	};
	//*************** Main division loop							
	ResultT res;
	while ((res = extendDividend()) == rtOk)	
	{			// 'dividend' has the same number of digits or one digit longer than 'divisor'
		SCharT q = chZero;		// quotient for one division
		while (divisorSmallerThanOrEqualToDividend())
		{
			subtractDivisorFromDividend();
			++q;
		}
		quotient[iq++] = q;	// remainder is left in 'dividend'
	}
	//************** Result 
	/*
		1000/2 => "1",exp=4 / "2", exp=1 => 
	*/
	if (pRemainder && res == rtError)	// number longer than _maxLength
	{
		left._numberString = NAN_STR;
		return;
	}
	
	// remove trailing zero bytes and characters (cannot combine these in one expression as we need to set left's exponent for 
	// integer modulus division
	quotient.erase(std::find_if(quotient.rbegin(), quotient.rend(), [](SCharT ch) {return ch.Unicode(); }).base(), quotient.end());	// zero bytes
	left._exponent = exponentOfResult;
	quotient.erase(std::find_if(quotient.rbegin(), quotient.rend(), [](SCharT ch) {return ch != chZero; }).base(), quotient.end()); // zero characters
	left._numberString = quotient;

	if (pRemainder)	// remainder is in 'dividend' which does not start with 0
	{				// remove trailing zeros
		dividend.erase(std::find_if(dividend.rbegin(), dividend.rend(), [](SCharT ch) {return ch != chZero; }).base(), dividend.end());
		pRemainder->_exponent		= (int)dividend.length();
		pRemainder->_numberString	= dividend;
	}
}

void RealNumber::_CorrectResult(RealNumber &left, String &result, int trailingchZeros, int carry) const
{
	size_t l = result.length() - trailingchZeros;
	size_t maxLength = _maxLength + LengthOverFlow;
	if (l > maxLength)
		l = maxLength;
	result.erase(l, result.npos);	// may erase all characters from string

	if (carry)
	{
		result = result.insert(0,1,SCharT(carry + chZero.Unicode())); // single digit
		++l;
		if (l > maxLength)
		{
			l = maxLength;
			result.pop_back();
		}
		++left._exponent;	// 1 plus digits at front
	}
	size_t leadingZeros;
	for (leadingZeros = 0; leadingZeros < l  && result.at(leadingZeros) == chZero; ++leadingZeros)
		;

	if (leadingZeros)
	{
		result.erase(0, leadingZeros);
		left._exponent -= (int)leadingZeros;
		l -= leadingZeros;
	}
	left._numberString = result;
	if (left.Precision() > maxLength)
		left.RoundToDigits((int)maxLength);
	left._leadingZeros = 0;
}

RealNumber fact(const RealNumber n)
{
	if (n.IsNegative() || n.IsInt() == false)
		return NaN;

	if (n > RealNumber(1000))	// at 1000 the error is 3.46925154994 E-6 %
	{
		return sqrt(RealNumber::RN_2 * pi * n) * (n / e) ^ n * (RealNumber::RN_1 + RealNumber::RN_1 / RealNumber(12) / n);
	}

	RealNumber res = n,
		nn = n;
	while (--nn != zero)
		res *= nn;
	return res;
}

RealNumber RadToAu(RealNumber r, AngularUnit au)
{
	if(r.IsNaN() || r. IsInf())
		return r;
	static RealNumber RN_360("360"), RN_400("400");
	switch (au)
	{
			case AngularUnit::auRad:
				return r;
			case AngularUnit::auDeg:
				return r / twoPi * RN_360;
			case AngularUnit::auGrad:
				return r / twoPi * RN_400;
			case AngularUnit::auTurn:
				return r / twoPi;
	}
	return NaN;
}

inline RealNumber sqrt(RealNumber r)
{
	return sqrtA(r, -1);
}

RealNumber sqrtA(RealNumber r, int accuracy)
{
	if (!r.IsValid() )
		return r;
	if (r.Sign() < 0)
		return NaN;

	if (accuracy < 0)
		accuracy = (int)(MaxAllowedDigits + LengthOverFlow);
	//Newton's method
	RealNumber	rootn("1"), // was r),	// n-th iteration step
				rootnp1,			// n plus 1th step
				epsilon(SmartString("1"), 1, -accuracy);
	
	// start with half the integer digits
//	rootn /= RealNumber("1");
	bool loop = true;
	int iter = 0;

	// Debug 7 lines
	// DisplayFormat format, fmtSci;
	// format.mainFormat = LongNumber::NumberFormat::rnfNormal;
	// format.decDigits = 4;
	// fmtSci = format;
	// fmtSci.nFormatSwitchLength = 4;
	// SmartString sr = r.ToDecimalString(format),
	//			sp, sn,snp1, sd;
	// /DEBUG

	while(loop)
	{
		rootnp1 = (rootn + r / rootn) * half;
		// DEBUG
		// LongNumber::DisplayFormat format, fmtSci;
		// fmtSci.mainFormat = LongNumber::NumberFormat::rnfSci;
		// fmtSci.decDigits = 16;
		// SmartString sp, sn, snp1, sd;
		// sp = (r / rootn).ToDecimalString(format);
		// sn = rootn.ToDecimalString(format);
		// snp1 = rootnp1.ToDecimalString(format);
		// std::cout << "\titer: SmartString("<< iter << "\t(rn+r/rn)/2:(SmartString("<< sn << "+"<< sp << ")/2=SmartString("<< snp1 << "\n";
		// RealNumber diff(rootn - rootnp1);
		// sd = diff.ToDecimalString(fmtSci);
		// std::cout << "\t\tdiff:SmartString("<< sd << ")\n";
		// /DEBUG
		loop = (rootn - rootnp1).Abs() > epsilon && iter++ < 1000;
		rootn = rootnp1;
	}

	return rootnp1;
}

RealNumber pow(RealNumber base, RealNumber power)
{
	return base.Pow(power);
}

RealNumber root(RealNumber num, int r)
{
	if (num.Sign() < 0 && !(r & 1))
		return NaN;
	if (r == 2)
		return sqrt(num);
	return exp( ln(num)/RealNumber(r) );
}

// transcendent functions
RealNumber exp(RealNumber power)						// e^x = e^(int(x)) x e^(frac(x))
{
	RealNumber	rnIntPart = power.Int(),
				rnFracPart = power.Frac();
	rnIntPart = ((RealNumber&)e).Pow(rnIntPart);	// this will not call exp()
	// e^x = 1 + sum_1^inf(x^n/n$)
	RealNumber x(rnFracPart), res(RealNumber::RN_1), resp(zero), xn, fact(RealNumber::RN_1);
		
	int n = 1;
	while ((res - resp).Abs() > epsilon || n < (int)RealNumber::MaxLength()) 
	{
		resp = res;
		res += x / fact;
		x = x * rnFracPart;
		fact = fact * RealNumber(++n);
	}
	return res * rnIntPart;
}

RealNumber ln(RealNumber num)
{
	if (num.Sign() < 0 || !num.IsValid())
		return NaN;
	if (num.IsNull())
		return Inf;
	if (num == e)
		return RealNumber::RN_1;
	// if x = a * 10^y, where  0 < a < 1 =>  ln(x) = y*ln10 + ln(a) 
	RealNumber x(num);
	int expnt = x.Exponent();
	RealNumber	rnIntPart = RealNumber(expnt) * rnLn10; // can be +/-
	x /= RealNumber::TenToThePowerOf(expnt);	// now x is between 0 and 1
	// use the Newtons' method (https://en.wikipedia.org/wiki/Natural_logarithm#High_precision)
	// Solve exp(y/2)-exp(-y/2) = 0, where y = ln x
	// 								  x - exp(y_n)
	//	y := lnx y_(n+1) = y_n + 2 * -------------
	//								  x	+ exp(y_n)
	int iter = 0;
	RealNumber yn(1), ynp1(0), expy,
				epsilon(SmartString("1"), 1, -(int)RealNumber::MaxLength() + 2 );
	while ((yn - ynp1).Abs() > epsilon && iter++ < 1000)
	{
		yn = ynp1;
		expy = exp(yn);
		ynp1 = yn + RealNumber(2) * (x - expy) / (x + expy);
	}
	yn = yn + rnIntPart;
	return yn;
}								// log_base(x)
RealNumber log(RealNumber x, RealNumber &base)	// logarithm in base 'base'
{
	if (base <= zero)
		return NaN;
	if (base == e)
		return ln(x);

	if(base.IsInt())
	{
		if(base == RealNumber::RN_2)
			return log2(x);
		if(base == RealNumber::RN_10)
			return log10(x);
	}
	return ln(x) / ln(base);
}

RealNumber log10(RealNumber num)					// or lg, base = 10
{
	return ln(num) * pln10;
}

RealNumber log2(RealNumber num)
{
	return ln(num) * pln2;
}

// trigonometric functions
static inline RealNumber deg2rad(RealNumber deg) 
{ 
	return deg / RealNumber("180") * pi;
}
// ------------- flag to disallow recursive call with units in radian
static bool _sinCalcOn = false;

static RealNumber _sin(RealNumber r)		// sine	(DEG)	0<= r <= 360 =>  0 <= _sin <= 1
{ 
	r = deg2rad(r);
	// from https://github.com/nlitsme/gnubc/blob/master/bc/libmath.b
	// Sin(x)  uses the standard series:
	//sin(x) = x - x^3/3! + x^5/5! - x^7/7! ...
	int z = (int)RealNumber::MaxLength();// , m = 0;

	RealNumber::SetMaxLength( (int)(1.1 * z) + 2);

	RealNumber b, e, i, n, s, v;

	  /* precondition r. */

	v = piP2.value / RealNumber::RN_2;	// π/4
	//	scale = 0 // to get integer part only
	RealNumber remainder;
	n = (r / v + RealNumber::RN_2).Div(RealNumber::RN_4, remainder);	// remainder just to ensure integer divison
								// n = [(4r/π+2)/4] = [(r+π/2)/π], e.g. when r = π/4 => [(1/4+1/2)]=0
								// n >0 when r >= π/2
	if(n.IsNull())
		r = r - RealNumber::RN_4 * n * v;		// move r into [0,π/2)
	if (n.IsOdd())	//		if (n % 2) x = -x		  for angles in quarters 3 or 4
		r.SetSign(-r.Sign());

			/* Do the loop. */
	RealNumber 	epsilon(SmartString("1"), 1, -(z + 2));

	RealNumber::SetMaxLength(z + 2);
	v = e = r;					  // v == sin(r) = r, e_{1} = r (actual power of r, n = 1) / i!
	s = -r * r;					  // for integer powers of r get -r^2
	i = RealNumber::RN_3;		  // i will be n!, first non linear term is x^3/3!
	while(true)
	{
		e *= s / (i * (i - RealNumber::RN_1));	  // e_{n+1} = e_{n} * (-r^2) / ( (i *(i-1)) * (i-2)! ) 
		if (e.Abs() <= epsilon)// x^(2n+1)/(2n+1)! < accuracy
		{
			RealNumber::SetMaxLength(z);
			if (r.Abs() < RealNumber("1e-40"))		// max accuracy for sine
				r = rnNull;
			return v;
		}
		v += e;					  // sum
		i += RealNumber::RN_2;	  // for (i+2)!
	}
			   // never comes here
	return RealNumber();
}
RealNumber sin (RealNumber r, AngularUnit au)		// sine
{

	int sign = r.Sign();	
	r.ToAbs();				// calculate sign of |r|

	RealNumber rn360 = RealNumber("360"),
				epsilon = RealNumber("1e-40");

	switch (au)
	{
		case AngularUnit::auDeg:
		{
			RealNumber rn180 = RealNumber("180");
			RealNumber rn90  = RealNumber("90");

			r.Div(rn360, r);			 // |r| is < 360
			// sine: 		+  | +
			//			 ------|------
			//				-  | -
			if (r >= rn180)
			{
				sign = -sign;
				r -= rn180;
			}
			if (r > rn90)					// sin(90+alpha)=cos(alpha)=sin(90-alpha), if alpha < 180
				r = r - rn90;

			// now r is in 0<= r <= 90
			if (r < epsilon)
				return RealNumber::RN_1;
			else if (r == RealNumber("30"))
			{
				r = half;
				return r.SetSign(sign);
			}
			else if (r == RealNumber("45"))
			{
				r = rsqrt2.value;
				return r.SetSign(sign);
			}
			else if (r == RealNumber("60"))
			{
				r = sqrt3P2.value;
				return r.SetSign(sign);
			}
			else if (r == rn90)
			{
				r = RealNumber::RN_1;
				return r.SetSign(sign);
			}
			else
				return _sin(r).SetSign(sign);		   // 0 <= r <= 90
			break;
		}
		case AngularUnit::auRad:
			r /= twoPi.value;
			r = r.Frac();				// 0 <= r <= 1
			// [[fallthrough]];			// from C++17
		case AngularUnit::auTurn:		// full circle 1 turn
			return sin(r*rn360);		// must change to DEG, otherwise infinite loop!
			break;

		case AngularUnit::auGrad:			// full circle 400 Grad
			return sin(rn360 / RealNumber("400") * r);
			break;
	}
	return RealNumber();
}

RealNumber csc(RealNumber r, AngularUnit au)		// cosecant = 1/sine
{
	return RealNumber::RN_1 / sin(r,au);
}

RealNumber cos(RealNumber r, AngularUnit au)		// cosine
{
	RealNumber rn360 = RealNumber("360"),
				epsilon = RealNumber("1e-40");
	// cos (- alpha) = cos(alpha) : argument always positive
	r.ToAbs();				

	switch (au)
	{
		case AngularUnit::auDeg:
		{
			RealNumber rn270 = RealNumber("270");
			RealNumber rn180 = RealNumber("180");
			RealNumber rn90 = RealNumber("90");

			(void)r.Div(rn360, r);	 // |r| is < 360
			// sine: 		-  | +
			//			 ------|------
			//				-  | +
			if (r <= rn90+epsilon)
				return sin(rn90 - r).SetSign(1);
			if (r <= rn180+epsilon)
				return sin(r - rn90).SetSign(-1);
			if (r <= rn270+epsilon)
				return sin(r - rn180).SetSign(-1);
			return sin(rn270 - r).SetSign(1);		// > 270
		}

		case AngularUnit::auRad:
			r /= twoPi.value;
			r = r.Frac();		// 0<= r <= 1 => number of "turns"
			// [[fallthrough]]; // From C++17
		case AngularUnit::auTurn:			// full circle 1 turn
			return cos(r * rn360);

		case AngularUnit::auGrad:			// full circle 400 Grad
			return cos(rn360 / RealNumber("400") * r);
			break;
	}
	return RealNumber();
}

RealNumber sec(RealNumber r, AngularUnit au)		// secant = 1/cosine
{
	return RealNumber::RN_1 / cos(r,au);
}

RealNumber tan(RealNumber r, AngularUnit au)		// tangent
{
	RealNumber rsin = sin(r, au);

	return rsin/cos(r, au);
}

RealNumber cot(RealNumber r, AngularUnit au)		// cotangent
{
	RealNumber rcos = cos(r,au);

	return rcos/sin(r,au);
}

// inverse trigonometric functions
RealNumber asin(RealNumber r, AngularUnit au)		// sine
{
	// fast answers
	if (r.Abs() > RealNumber::RN_1)
		return NaN;
	if (r.IsNull())
		return zero;
	if (r.Abs() == RealNumber::RN_1)
	{
		int sign = r.Sign();
		r = piP2.value;
		return RadToAu(r.SetSign(sign), au);
	}
	// slow answer
	return atan(r/sqrt(RealNumber::RN_1 - r*r), au );
}

RealNumber acos(RealNumber r, AngularUnit au)		// cosine
{
	return RadToAu(piP2.value - asin(r, AngularUnit::auRad), au);
}

RealNumber atan(RealNumber r, AngularUnit au)		// arcus tangent
{
	// From: https://github.com/nlitsme/gnubc/blob/master/bc/libmath.b
	// Using the formula:
	// atan(x) = atan(c) + atan((x - c) / (1 + xc)) for a small c(.2 here)
	//	For under .2, use the series :
	// atan(x) = x - x ^ 3 / 3 + x ^ 5 / 5 - x ^ 7 / 7 + ...
	// Changes by A.S.
  /* a is the value of atan(.2) if it is needed. */
  /* f is the value to multiply by a in the return. */
  /* e is the value of the current term in the series. */
  /* v is the accumulated value of the series. */
  /* m is 1 or -1 depending on x (-x -> -1).  not used (A.S.)  */
  /* i is the denominator value for series element. */
  /* n is the numerator value for the series element. */
  /* s is -x*x. */
  /* z is the saved user's scale. */

	// result must be converted to the selected units
  /* Special cases for fast answers */
	if (r.IsNaN())
		return r;
	int sign = r.Sign();

		// trivial results
	if (r.IsInf())		// multiple of 90 degrees
	{
		switch(au)
		{
			case AngularUnit::auRad:
				return piP2.value.Rounded((int)(RealNumber::MaxLength() + LengthOverFlow)).SetSign(sign);
			case AngularUnit::auDeg:
				return RealNumber("90").SetSign(sign);
			case AngularUnit::auGrad:
				return RealNumber("100").SetSign(sign);
			case AngularUnit::auTurn:
				return (RealNumber("0.25")).SetSign(sign);
				}
	}
	if (r == RealNumber::RN_1)
	{
		switch(au) 
		{
			case AngularUnit::auRad:
				return (piP2.value * half).RoundedToDigits((int)(RealNumber::MaxLength() + LengthOverFlow)).SetSign(sign);
			case AngularUnit::auDeg:
				return RealNumber("45").SetSign(sign);
			case AngularUnit::auGrad: 
				return RealNumber("50").SetSign(sign);
			case AngularUnit::auTurn: 
				return (RealNumber::RN_1/RealNumber(8)).SetSign(sign);
		}
	}	

			// aDot2 ==  atan(.2)
	RealNumber aOfDot2 = atanOfDot2.RoundedToDigits( (int)(RealNumber::MaxLength() + LengthOverFlow));
	RealNumber dot2 = RealNumber(".2");
	if (r == dot2)
		return RadToAu(r, au);

	r.SetSign(1);	// make it positive always

	RealNumber a, f, e, v=r, i, n, s;
	size_t z = RealNumber::MaxLength();
	// Calculate atan of a known number (0.2)
	if (r > aOfDot2)
	{
		(void)RealNumber::SetMaxLength(z + 5);
		a = aOfDot2;
	}

	/* Precondition r. */
	RealNumber::SetMaxLength(z + 3);
	while (r > dot2) 
	{
		f += RealNumber::RN_1;
		r = (r - dot2) / (RealNumber::RN_1 + r * dot2);
	}
	/* Initialize the series. */
	v = n = r;
	s = -r * r;

	/* Calculate the series. infinite loop */
	for (i = RealNumber::RN_3; true; i += RealNumber::RN_2)
	{
		e = (n *= s) / i;
		int esign = e.Sign();
		e.SetSign(1);		// faster then Abs and no space requirement
		if (e <= epsilon) 
		{
			RealNumber::SetMaxLength(z);
			r = (f * a + v);
			return RadToAu(r, au).SetSign(sign);
		}
		e.SetSign(esign);
		v += e;
	}
	return RealNumber();
}

RealNumber acot(RealNumber r, AngularUnit au)		// cotangent
{
	r = piP2.value - r;
	return atan(r, au);
}

// hyperbolic functions

RealNumber sinh(RealNumber r)			// hyperbolic sine
{
	RealNumber rn = exp(r);
	return RealNumber( (rn - rn.Pow(-1))/RealNumber::RN_2);
}

RealNumber csch(RealNumber r)			// hyperbolic cosecant = 1/sinh
{
	return RealNumber(RealNumber::RN_1/sinh(r));
}

RealNumber cosh(RealNumber r)			// hyperbolic cosine
{
	RealNumber rn = exp(r);
	return RealNumber( (rn + rn.Pow(-1))/RealNumber::RN_2);
}

RealNumber sech(RealNumber r)			// hyperbolic secant = 1/cosh
{
	return RealNumber(RealNumber::RN_1/cosh(r));
}

RealNumber tanh(RealNumber r)			// hyperbolic tangent
{
	RealNumber rp = exp(r), rr = RealNumber::RN_1 / rp;
	return RealNumber((rp - rr)/(rp + rr));
}

RealNumber coth(RealNumber r)			// hyperbolic cotangent
{
	RealNumber rp = exp(r), rr = RealNumber::RN_1 / rp;
	return RealNumber((rp + rr)/(rp - rr));
}

// inverse hyperbolic functions
RealNumber asinh(RealNumber r)		// area hyperbolic sine
{
	RealNumber x2 = sqrt(r * r + RealNumber::RN_1);
	return ln(r + x2);
}

RealNumber acosh(RealNumber r)		// area hyperbolic cosine only the positive signed RealNumber::RN_1
{
	RealNumber x2 = r * r - RealNumber::RN_1;
	if (x2.IsNegative())
		return NaN;
	x2 = sqrt(x2);

	return ln(r + x2);
}

RealNumber atanh(RealNumber r)		// area hyperbolic tangent
{
	if (r.Abs() >= RealNumber::RN_1)
		return NaN;

	return half * ln( (RealNumber::RN_1 + r) / (RealNumber::RN_1 - r) );
}

RealNumber acoth(RealNumber r)		// hyperbolic cotangent
{
	if (r.Abs() <= RealNumber::RN_1)
		return NaN;

	return half * ln( (RealNumber::RN_1 + r) / (RealNumber::RN_1 - r) );
}
//////////////////////////////////////////////////////////////
// end of namespace LongNumber
//////////////////////////////////////////////////////////////
}
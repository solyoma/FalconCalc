#pragma once

#ifndef _defines_H
#define _defines_H

#ifndef QTSA_PROJECT
typedef size_t LENGTH_TYPE;
#define GEQ0(i) (i < 0x80000000)
#else
#include <QString>
typedef int LENGTH_TYPE;
#define GEQ0(i) (i >= 0)
#endif

enum BuiltinDescId {
	DSC_NoDescription = 0,	// for user defined variables and functions
	// all others are for built-in constants descriptions
	// mathematical constants
	DSC_descriptionForE,		// u"Euler's number"								
	DSC_descriptionForLg10e,	// u"base 10 logarithm of e
	DSC_descriptionForLge,		// u"base 10 logarithm of e
	DSC_descriptionForLn10,		// u"natural logarithm of 10
	DSC_descriptionForLn2,		// u"natural logarithm of 2
	DSC_descriptionForLog2e,	// u"base 2 logarithm of e
	DSC_descriptionForPi,		// u"π - half the circumference of a unit circle" 
	DSC_descriptionForPiP2,		// u"π/2"											
	DSC_descriptionForPiP4,		// u"π / 4
	DSC_descriptionForRln10,	// u"1/ln(10)"											
	DSC_descriptionForRln2,		// u"1/ln(2)"											
	DSC_descriptionForRpi,		// u"1 / π
	DSC_descriptionForRpi2,		// u"2 / π
	DSC_descriptionForRsqrt2,	// u"1/√2"										
	DSC_descriptionForSqpi,		// u"√π
	DSC_descriptionForSqrt2,	// u"√2"											
	DSC_descriptionForSqrt3,	// u"√3"											
	DSC_descriptionForSqrt3P2,	// u"sqrt3P2"
	DSC_descriptionForTwoPi,	// u"2π"											
	// physics,	
	DSC_descriptionForAu,		// u"astronomical unit (exact value)"					
	DSC_descriptionForC,		// u"speed of light in vacuum (exact value)"				
	DSC_descriptionForEps0,		// u"εo
	DSC_descriptionForFsc,		// u"fine-structure constant (about 1/137)"				
	DSC_descriptionForG,		// u"Newtonian constant of gravitation"					
	DSC_descriptionForGf,		// u"average g on Earth"									
	DSC_descriptionForH,		// u"Planck constant"									
	DSC_descriptionForHbar,		// u"reduced Planck constant (h/2π)"						
	DSC_descriptionForKb,		// u"Boltzmann constant"									
	DSC_descriptionForKc,		// u"= 1/4πεo Coulomb constant"							
	DSC_descriptionForLa,		// u"Avogadro constant (exact value)"					
	DSC_descriptionForMe,		// u"electron mass"										
	DSC_descriptionForMf,		// u"mass of the Earth"									
	DSC_descriptionForMp,		// u"proton mass"										
	DSC_descriptionForMs,		// u"mass of the Sun"									
	DSC_descriptionForMu0,		// u"4π*10ˉ⁷ vacuum magnetic permeability"				
	DSC_descriptionForQe,		// u"elementary charge"									
	DSC_descriptionForRf,		// u"radius of the Earth"								
	DSC_descriptionForRfsc,		// u"reciprocal of the fine structure constant (approx 137)"
	DSC_descriptionForRg,		// u"molar gas constant R"								
	DSC_descriptionForRs,		// u"radius of the Sun"									
	DSC_descriptionForSb,		// u"Stefan–Boltzmann constant"							
	DSC_descriptionForU,		// u"atomic mass unit (=(mass of C12)/12)"				
  // functions
	DSC_FuncACos,					// "inverse of cosine " 
	DSC_FuncACosH,					// "inverse of hyperbolic cosine"
	DSC_FuncACotH,					// "inverse of hyperbolic cotangent"
	DSC_FuncASin,					// "inverse of sine"
	DSC_FuncASinH,					// "inverse of hyperbolic sine"
	DSC_FuncATan,					// "inverse of tangent" 
	DSC_FuncATanH,					// "inverse of hyperbolic tangent"
	DSC_FuncAbs,					// "absolute value"
	DSC_FuncCos,					// "cosine"
	DSC_FuncCosH,					// "hyperbolic cosine"
	DSC_FuncCot,					// "cotangent"
	DSC_FuncCotH,					// "hyperbolic cotangent"
	DSC_FuncExp,					// "power of e"
	DSC_FuncFact,					// "factorial"
	DSC_FuncFrac,					// "fractional part"
	DSC_FuncInt,					// "integer part"
	DSC_FuncLg,						// "base 10 logarithm"
	DSC_FuncLn,						// "natural logarithm"
	DSC_FuncLog2,					// "base 2 logarithm"
	DSC_FuncPow,					// "pow(x,y)=x^y"
	DSC_FuncRoot3,					// "cubic root of x"
	DSC_FuncRound,					// "round y to x decimals"
	DSC_FuncSign,					// "sign of number"
	DSC_FuncSin,					// "sine"
	DSC_FuncSinH,					// "hyperbolic sine"
	DSC_FuncSqrt,					// "square root"
	DSC_FuncTan,					// "tangent"
	DSC_FuncTanH,					// "hyperbolic tangent"
	DSC_FuncTrunc,					// "truncate to integer"
	DSC_FuncXRY,					// "x-th root of y"
};

#ifdef QTSA_PROJECT
	extern QString GetBuiltinTextForId(BuiltinDescId bdId); // in VariablesFunctionsDialog.cpp
#endif


#endif /* _defines_H */

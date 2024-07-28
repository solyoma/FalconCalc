#ifndef QTSA_PROJECT
    namespace nlib {}
    using namespace nlib;
#endif

#include "SmartString.h"
using namespace SmString;

namespace LongNumber {}
using namespace LongNumber;

#include "LongNumber.h"

#include "calculate.h"

using namespace std;


constexpr auto VERSION_STRING = L"FalconCalc V1.1";


#include <math.h>

int _matherr (struct _exception *a)
{
    if (a->type == OVERFLOW)
    {
//        if (!strcmp(a->name,"pow"_ss) ||
//            !strcmp(a->name,"powl"_ss) ||
//            !strcmp(a->name,"exp"_ss) ||
//            !strcmp(a->name,"expl"_ss))
        {
#ifdef _MSC_VER
#else
            throw "overflow error"_ss;
#endif
        }
    }
    else if(a->type == DOMAIN)
    {
#ifdef _MSC_VER
#else
        throw "domain error"_ss;
#endif
    }
    return 1;
}

bool IsAlpha(wchar_t ch, locale loc);    // In ,wcommon.cpp, needed for names in one locale when working in another localse
bool IsAlnum(wchar_t ch, locale loc);    // In ,wcommon.cpp, needed for names in one locale when working in another localse


using namespace SmString;
using namespace LongNumber;
using namespace FalconCalc;
                     // static so each engine has the same constants and variables
VariableTable LittleEngine::variables;
FunctionTable LittleEngine::functions;

extern FalconCalc::LittleEngine* lengine;

/*==============================================================
 * NAMESPACE FalconCalc
 *-------------------------------------------------------------*/
namespace FalconCalc {

const SCharT schCommentDelimiter = SCharT(':');
const SmartString ssCommentDelimiterString(1,schCommentDelimiter);


std::map<Trigger_Type, SmartString> triggerMap
{
    {Trigger_Type::BUILTIN_FUNCTIONS_CANNOT_BE_REDEFINED,"Builtin functions cannot be redefined"_ss},
    {Trigger_Type::BUILTIN_VARIABLES_CANNOT_BE_REDEFINED,"Builtin variables cannot be redefined"_ss},
    {Trigger_Type::CLOSING_QUOTE_NOT_FOUND,"Closing quote not found"_ss},
    {Trigger_Type::DIVISON_BY_0,"Divison by 0"_ss},
    {Trigger_Type::EITHER_THE_SEPARATOR_WAS_MISPLACED_OR_PARENTHESIS_WERE_MISMATCHED,"Either the separator was misplaced or parenthesis were mismatched"_ss},
    {Trigger_Type::EXPRESSION_ERROR,"Expression error"_ss},
    {Trigger_Type::FUNCTION_DEFINITION_MISSING_RIGHT_BRACE,"Function definition missing right brace"_ss},
    {Trigger_Type::FUNCTION_MISSING_OPENING_BRACE,"Function missing opening brace"_ss},
    {Trigger_Type::ILLEGAL_AT_LINE_END,"Illegal at line end"_ss},
    {Trigger_Type::ILLEGAL_BINARY_NUMBER,"Illegal binary number"_ss},
    {Trigger_Type::ILLEGAL_CHARACTER_NUMBER,"Illegal character number"_ss},
    {Trigger_Type::ILLEGAL_HEXADECIMAL_NUMBER,"Illegal hexadecimal number"_ss},
    {Trigger_Type::ILLEGAL_NUMBER_No1,"Illegal number #1"_ss},
    {Trigger_Type::ILLEGAL_NUMBER_No2,"Illegal number #2"_ss},
    {Trigger_Type::ILLEGAL_OCTAL_NUMBER,"Illegal octal number"_ss},
    {Trigger_Type::ILLEGAL_OPERATOR_AT_LINE_END,"Illegal operator at line end"_ss},
    {Trigger_Type::INVALID_CHARACTER_IN_FUNCTION_DEFINITION,"Invalid character in function definition"_ss},
    {Trigger_Type::INVALID_FUNCTION_DEFINITION,"Invalid function definition"_ss},
    {Trigger_Type::MISMATCHED_PARENTHESIS,"Mismatched parenthesis"_ss},
    {Trigger_Type::MISSING_BINARY_NUMBER,"Missing binary number"_ss},
    {Trigger_Type::NO_FUNCTION_ARGUMENT,"No function argument"_ss},
    {Trigger_Type::RECURSIVE_FUNCTIONS_ARE_NOT_ALLOWED,"Recursive functions are not allowed"_ss},
    {Trigger_Type::STACK_ERROR,"Stack error"_ss},
    {Trigger_Type::SYNTAX_ERROR,"Syntax error"_ss},
    {Trigger_Type::UNKNOWN_FUNCTION_IN_EXPRESSION,"Unknown function in expression"_ss},
    {Trigger_Type::VARIABLE_DEFINITION_MISSING,"Variable definition missing"_ss}
};
		
void Trigger(Trigger_Type tt)
{
		throw tt;
}

// Class MathOperator
map<SmartString, OP> MathOperator::ops; // operator table
bool MathOperator::bOpsOk = false;
/*==========================================================
 * TASK: set up static table of in-built operators
 * EXPECTS: nothing
 * RETURNS: nothing, 'ops' table is set up
 * REMARKS: if modified must check/modify 'LittleEngine::InfixToPostFix()'
 *---------------------------------------------------------*/
void MathOperator::Setup()
{
	if(bOpsOk)
		return;

	ops["*invalid*"_ss].oper       = opINVALID;

	ops["("_ss].oper       = opOpenBrace;
	ops["("_ss].precedence          = -1; // special case  !

	ops[")"_ss].oper       = opCloseBrace;
	ops[")"_ss].precedence          = -1;

	ops["or"_ss].oper       = opOR;
	ops["or"_ss].precedence          = 0;

	ops["|"_ss].oper        = opOR;
	ops["|"_ss].precedence           = 0;

	ops["xor"_ss].oper      = opXOR;
	ops["xor"_ss].precedence         = 1;

	ops["&"_ss].oper        = opAND;
	ops["&"_ss].precedence           = 2;

	ops["and"_ss].oper      = opAND;
	ops["and"_ss].precedence         = 2;

	ops["=="_ss].oper       = opEQ;            // result 0 or 1
	ops["=="_ss].precedence          = 3;

	ops["!="_ss].oper       = opNEQ;
	ops["!="_ss].precedence          = 3;

	ops["<"_ss].oper        = opLT;			// result: 0 or 1
	ops["<"_ss].precedence           = 4;

	ops["<="_ss].oper       = opLE;
	ops["<="_ss].precedence          = 4;

	ops[">"_ss].oper        = opGT;
	ops[">"_ss].precedence           = 4;

	ops[">="_ss].oper       = opGE;
	ops[">="_ss].precedence          = 4;

	ops["<<"_ss].oper       = opSHL;
	ops["<<"_ss].precedence          = 5;

	ops["shl"_ss].oper      = opSHL;
	ops["shl"_ss].precedence         = 5;

	ops[">>"_ss].oper       = opSHR;
	ops[">>"_ss].precedence          = 5;

	ops["shr"_ss].oper      = opSHR;
	ops["shr"_ss].precedence         = 5;

	ops["+"_ss].oper        = opPLUS;
	ops["+"_ss].precedence           = 6;

	ops["-"_ss].oper        = opMINUS;
	ops["-"_ss].precedence           = 6;

	ops["*"_ss].oper        = opMUL;
	ops["*"_ss].precedence           = 7;

	ops["/"_ss].oper        = opDIV;
	ops["/"_ss].precedence           = 7;

	ops["%"_ss].oper        = opMOD;
	ops["%"_ss].precedence           = 7;

	ops["mod"_ss].oper        = opMOD;
	ops["mod"_ss].precedence         = 7;

	ops["_"_ss].oper        = opUMIN;				// unary '-' on stack
	ops["_"_ss].precedence           = 8;
	ops["_"_ss].right_associative    = true;

	ops["!"_ss].oper        = opNOT;
	ops["!"_ss].precedence           = 8;
	ops["!"_ss].right_associative    = true;

	ops["not"_ss].oper      = opNOT;
	ops["not"_ss].precedence         = 8;
	ops["not"_ss].right_associative  = true;

	ops["~"_ss].oper = opCompl;
	ops["~"_ss].precedence = 8;
	ops["~"_ss].right_associative = true;

	ops["^"_ss].oper        = opPOW;
	ops["^"_ss].precedence           = 9;
	ops["^"_ss].right_associative    = true;

    ops["="_ss].oper        = opLET;				// in 'x = 3' or 'f(x,y) = x * y'
 	ops["="_ss].precedence           = 11;
	ops["="_ss].right_associative    = true;

	bOpsOk = true;
}


/*==========================================
Class Token
 *-----------------------------------------*/

/*==========================================
 * TASK: scans 'text' from pos for non alpha
 *       operators and sets 'data' accordingly
 * EXPECTS: 'text' and 'pos' <= text.length()
 * RETURNS: pos now points after the SmartString of operator
 *         'data' is set up
  *----------------------------------------*/
void Token::_GetOperator(const SmartString &text, unsigned &pos)
{
	SCharT c = text[pos++],
		  cn = SCharT(pos >= text.length() ? 0 : text[pos] );

	SmartString s;
	s = c;

	switch(c)
	{
	    case '<' : if(!cn) // no more character in line
					    Trigger(Trigger_Type::ILLEGAL_AT_LINE_END);
				    switch(cn)
				    {
					    case '<' : name = u"<<"; break;
					    case '=' : name = u"<="; break;
					    default  : name = u"<";  break;
				    };
				    break;
	    case '>' : if(!cn) // no more character in line
					    Trigger(Trigger_Type::ILLEGAL_OPERATOR_AT_LINE_END);
				    switch(cn)
				    {               
					    case '>' : name = u">>"; break;
                        case '=':  name = u">="; break;
                        default:   name = u">";  break;
				    };
				    break;
	    case '!':  if(!cn) // no more character in line
					    Trigger(Trigger_Type::ILLEGAL_OPERATOR_AT_LINE_END);
				    switch(cn)
				    {
					    case '=' : name =u"!="; break;
                        default  : name = u"!"; break;
				    };
				    break;
	    case '=' :  if(!cn) // no more character in line
					    Trigger(Trigger_Type::ILLEGAL_OPERATOR_AT_LINE_END);
				    switch(cn)
				    {
					    case '=' : name = u"=="; break;
					    default  : name = u"=";  break;
				    };
				    break;
	    case '~' :  if (!cn) // no more character in line
					    Trigger(Trigger_Type::ILLEGAL_OPERATOR_AT_LINE_END);
                    name = u"~"; 
                    break;
	    default:  *this = s;
			      return;
	}
    data = MathOperator::Op(name);
    type = tknOperator;
}

/*==========================================
 * TASK: scans 'text' from pos for decimal digits
 *       using current locale
 * EXPECTS: 'text' and 'pos' <= text.length()
 * RETURNS: pos now points after the SmartString of digits
  *----------------------------------------*/
void Token::_GetDecDigits(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	while(pos < text.length() && isdigit(text[pos], loc) )
		++pos;
}

/*========================================================
 * TASK: read a decimal number from 'text'. Decimal numbers are
 *       any text in the format ([] means optional arguments)
 * [decimal digit(s)][decimal point][decimal digit(s)][e[+-][decimal number]]
 *       where [decimal point] is determined by the current locale
 *       and either RealNumber::RN_1 of the [decimal digit(s)] must be present
 * EXPECTS: 'text' is lowercase, starts with number or
 *        [decimal point] (no unary + or -)
 *        and if 'text' starts with decimal point it contains at
 *        least RealNumber::RN_1 additional digit
 * RETURNS: nothing, type and number SmartString is set in 'data' member
 *         'pos' is positioned after the number SmartString
 *-----------------------------------------------------------*/
void Token::_GetDecimalNumber(const SmartString &text, unsigned &pos)
{

    const SCharT decpoint = RealNumber::DecPoint();

	int startpos = pos;
	int nIntP  = 0,
        nFracP = 0, //           fractional
		nExpP  = 0; //           exponent part starting after the 'e'
	nIntP  = pos;         // start of integer part of number
	_GetDecDigits(text, pos);
	nIntP = pos - nIntP;      // length of integer part of number

	if(text[pos] == decpoint)	// decimal point: get fractions
	{
		nFracP = ++pos;
		_GetDecDigits(text, pos);
		nFracP = pos - nFracP;
	}
	bool bExp = (text[pos] == 'e' || (text[pos] == 'E'));
	if(bExp)
	{
		if(text[++pos] == '+' || text[pos] == '-')
			++pos;
		nExpP = pos;
		_GetDecDigits(text, pos);
		nExpP = pos - nExpP;
	}
	 // a number from a single decimal point or with no exponent after the 'e'
	 // is illegal
	if( (!nIntP && ! nFracP)  || (bExp && ! nExpP))
		Trigger(Trigger_Type::ILLEGAL_NUMBER_No1);
	type = tknNumber;
	name = text.substr(startpos, pos - startpos);
    val = RealNumber(name);
}

/*========================================================
 * TASK: read a hexadecimal number from 'text'.
 * EXPECTS: 'text' is lowercase, 'pos' points after the '0x'
 * RETURNS: nothing, type and number SmartString is set in 'data' member
 *         'pos' is positioned after the number SmartString
 *-----------------------------------------------------------*/
void Token::_GetHexNumber(const SmartString &text, unsigned &pos)
{
    unsigned startpos = pos;
    pos += 2;
	locale loc = cout.getloc();
	while(pos < text.length() && isxdigit(text[pos],loc) )
		++pos;
	if(pos == startpos+2)
		Trigger(Trigger_Type::ILLEGAL_HEXADECIMAL_NUMBER);
	type = tknNumber;
	name = text.substr(startpos, pos - startpos);
    val = RealNumber(name);
}

/*========================================================
 * TASK: read an octal number from 'text'.
 * EXPECTS: 'text' is lowercase, 'pos' points after the '0'
 * RETURNS: nothing, type and number SmartString is set in 'data' member
 *         'pos' is positioned after the number SmartString
 *-----------------------------------------------------------*/
void Token::_GetOctNumber(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	unsigned startpos = pos++;
	while(pos < text.length() && isdigit(text[pos],loc))
	{
		if(text[pos] > '7')
				Trigger(Trigger_Type::ILLEGAL_OCTAL_NUMBER);
		++pos;
	}
	type = tknNumber;
	if(pos != startpos+1)
		name = text.substr(startpos, pos - startpos);
    val = RealNumber(name);
}

/*========================================================
 * TASK: read a binary number from 'text'.
 * EXPECTS: 'text' is lowercase, 'pos' points after the '#'
 * RETURNS: nothing, type and number SmartString is set in 'data' member
 *         'pos' is positioned after the number SmartString
 *-----------------------------------------------------------*/
void Token::_GetBinaryNumber(const SmartString &text, unsigned &pos)
{
    auto triggerError = []()
        {
            Trigger(Trigger_Type::ILLEGAL_BINARY_NUMBER);
        };
    locale loc = cout.getloc();
    unsigned startpos = pos++;    // skip '#'
    while (pos < text.length() && isdigit(text[pos], loc))
    {
        if (text[pos] > '1')
            triggerError();
        ++pos;
    }
    type = tknNumber;
    if (pos == startpos+1)
        triggerError();
    name = text.substr(startpos, pos - startpos);
    val = RealNumber(name);
}
/*========================================================
 * TASK: read a  number from character SmartString 'text'.
 * EXPECTS: - 'text' contain character string between single quotes
 *          - 'pos' at input: points to after the opening single quote
 * RETURNS: nothing, type and number SmartString is set in 'data' member
 *         'pos' is positioned after the closing single quote
 * REMARKS: - Each character is 16 bit long UTF16 character
 *          - The ersult is a number where each character has its own 16 bits
 *          - The character SmartString is considered a BIG_ENDIAN
 *              number
 *-----------------------------------------------------------*/
void Token::_GetNumberFromQuotedString(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	unsigned startpos = pos;
    RealNumber lval = RealNumber::RN_0, r10k = RealNumber(0x10000);
	while(pos < text.length() && (text[pos] != '\'' || (pos > 0 && text[pos-1] == '\\')) )
    {
		SCharT ch = text[pos];
        lval = lval * r10k + RealNumber(ch.Unicode()) ;
		++pos;
    }
	if(pos == startpos)
		Trigger(Trigger_Type::ILLEGAL_CHARACTER_NUMBER);
    if(pos == text.length() )
        Trigger(Trigger_Type::CLOSING_QUOTE_NOT_FOUND);

	type = tknCharacter;
	name = "'"_ss + text.mid(startpos, pos - startpos) + "'"_ss;
    val  = lval;
    ++pos;
}

/*========================================================
 * TASK: read a word starting with an alpha character
 *       and containing alphanumeric characters or underscores
 *       categorize it and stores in 'data'
 *       A word may contain any alphanumeric characters and underscores
 *       but may not start with '_'
 * EXPECTS: 'text' is lowercase, 'pos' points to first character which
 *       is either an alpha character or an underscore
 * RETURNS: nothing, type and word is set in 'data' member
 *         'body' is the name of the variable/function/operator
 *         'pos' is positioned after the number SmartString
 *-----------------------------------------------------------*/
void Token::_GetVarOrFuncOrOperator(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	int startpos = pos;
	while(pos < text.length() && (IsAlnum(text[pos],loc) || text[pos] == '_'))
		++pos;
	SmartString s = text.substr(startpos, pos - startpos);
    if (s.length() == 1 && s == u"π")
        s = u"pi";
	name = s;  //  set name
    while(pos < text.length() && isspace((wchar_t)text[pos],loc))
        ++pos; // skip whitespace because of function definitions
    data = MathOperator::Op(s );
	if( data.oper != opINVALID )
		type = tknOperator;
	else if(pos < text.length() && text[pos] == '(') // function
    {
		type = tknFunction;
        ++pos;                 // skip '('
    }
	else
		type = tknVariable;
 }

void Token::FromText(const SmartString &text, unsigned &pos)
{
    val = 0;
    type = tknUnknown;
	locale loc = cout.getloc();

	// skip whitespace
	unsigned len = text.length();
	if(pos >= len || text[pos] == schCommentDelimiter) 
    {
        type = tknEOL;
		return;
    }

    SCharT decpoint = RealNumber::DecPoint();
	SmartString sErr = "Illegal operator"_ss;

	while(pos < len && isspace((wchar_t)text[pos], loc))
		++pos;
	SCharT  c = text[pos++],
			cn = (pos >= len ? SCharT(0) : SCharT(text[pos])); // look ahead
    --pos;  // go back to start of token

    if(c == SCharT('\'') )   // character SmartString
        _GetNumberFromQuotedString(text, ++pos);
	else if(isdigit(c,loc) || c == decpoint || c == SCharT('#'))		// token is a decimal, hexadecimal, octal or binary number
	{
		bool bDecpF = (c == decpoint),							        // decimal point found ?
				bHexF = (c == SCharT('0') && (cn == SCharT('x') )),		// hex number?
				bOctF = (c == SCharT('0') && cn && isdigit(cn,loc)),
				bBinF = (c == SCharT('#'));

		if(bHexF)                       // starts with 0x and ends when any non hex. digit character found
			_GetHexNumber(text, pos);    // 0x....
		else if(bOctF)
			_GetOctNumber(text, pos);    // 0....
		else if(bBinF)                  // #...
		{
			if(!cn) // then EOL and number is a single binary type character
				Trigger(Trigger_Type::MISSING_BINARY_NUMBER);
			_GetBinaryNumber(text, pos);  // starting after the '#' type character
		}
		else // decimal number
		{
			if((!cn && bDecpF) || (bDecpF && !isdigit(cn,loc)))	// then EOL and number is a single decimal point
				Trigger(Trigger_Type::ILLEGAL_NUMBER_No2);
			_GetDecimalNumber(text, pos); // start at the first number/decimal point
		}
	}
    else if (IsAlpha(c,loc)) // variable, function or text operator (e.g. 'or')
        _GetVarOrFuncOrOperator(text, pos);
	else // a possible operator character
		_GetOperator(text, pos);
}
/*===================================================
 * TASK: construct a token from text
 * EXPECTS: 'text' SmartString
 *          'pos': position in 'text' where the tex of the token starts
 *           text to only contain valid characters and only lowercase letters!
 *--------------------------------------------------*/
Token::Token( const SmartString &text, unsigned &pos) : type(tknUnknown), val(0)
{
    FromText(text, pos);
}


/*==========================================
Class LittleEngine
 *-----------------------------------------*/

//FunctionTable LittleEngine::functions;
//VariableTable LittleEngine::variables;
unsigned LittleEngine::numBuiltinVars  = 0,
         LittleEngine::numBuiltinFuncs = 0;

bool LittleEngine::builtinsOk=false;

inline static RealNumber Sign(RealNumber r) { return r.Sign() > 0 ? RealNumber::RN_1:-RealNumber::RN_1; }

static RealNumber Sin(RealNumber  r) { return sin (r, lengine->AngleUnit()); }
static RealNumber Csc(RealNumber  r) { return csc (r, lengine->AngleUnit()); }
static RealNumber Cos(RealNumber  r) { return cos (r, lengine->AngleUnit()); }
static RealNumber Sec(RealNumber  r) { return sec (r, lengine->AngleUnit()); }
static RealNumber Tan(RealNumber  r) { return tan (r, lengine->AngleUnit()); }
static RealNumber Cot(RealNumber  r) { return cot (r, lengine->AngleUnit()); }
static RealNumber Asin(RealNumber r) { return asin(r, lengine->AngleUnit()); }
static RealNumber Acos(RealNumber r) { return acos(r, lengine->AngleUnit()); }
static RealNumber Atan(RealNumber r) { return atan(r, lengine->AngleUnit()); }
static RealNumber Acot(RealNumber r) { return acot(r, lengine->AngleUnit()); }

/*========================================================
 * TASK: Creates a single instance of the calculator
 *       if the built-ins are not yet set up sets them up too
 * EXPECTS:all builtins 's name is lowercase!
 * RETURNS: nothing/object created
 *-----------------------------------------------------------*/
LittleEngine::LittleEngine() : clean(true)
{
    _argSeparator = RealNumber::DecPoint() == SCharT('.') ? SCharT(',') : SCharT(';');

    if(!builtinsOk)
    {
        numBuiltinVars = constantsMap.size();       // in LongNumber.cpp
            // all built in function requires a single RealNumber argument
            // they are not 'dirty' and they are 'isnumber's
       Func f;
       f.builtin = true;
//       f.isnumber = true;
       f.dirty = false;
       f.value = 0.0l;

	   #define SET_BUILTIN_FUNC1(a,b,c) f.function.clear(); f.name = u#a; f.desc = u#b; f.function.funct1  = c; functions[#a##_ss] = f;
	   #define SET_BUILTIN_FUNC2R(a,b,c) f.function.clear(); f.name = u#a; f.desc = u#b; f.function.funct2r = c; functions[#a##_ss] = f;
	   #define SET_BUILTIN_FUNC2I(a,b,c) f.function.clear(); f.name = u#a; f.desc = u#b; f.function.funct2i = c; functions[#a##_ss] = f;
       SET_BUILTIN_FUNC1(abs, absolute value, abs);

	   f.useAngleUnitAsResult=true;
       SET_BUILTIN_FUNC1(arcsin , inverse of sine   , Asin);
       SET_BUILTIN_FUNC1(asin   , inverse of sine   , Asin);
       SET_BUILTIN_FUNC1(arccos , inverse of cosine , Acos);
       SET_BUILTIN_FUNC1(acos   , inverse of cosine , Acos);
       SET_BUILTIN_FUNC1(arctan , inverse of tangent, Atan);
       SET_BUILTIN_FUNC1(atan   , inverse of tangent, Atan);
       f.useAngleUnitAsResult=false;

       f.useAngleUnit        =true;
       SET_BUILTIN_FUNC1(sin    , sine      , Sin);
       SET_BUILTIN_FUNC1(cos    , cosine    , Cos);
       SET_BUILTIN_FUNC1(cot    , cotangent , Cot);
       SET_BUILTIN_FUNC1(tan    , tangent   , Tan);
       SET_BUILTIN_FUNC1(tg     , tangent   , Tan);
       SET_BUILTIN_FUNC1(ctg    , cotangent , Cot);
       f.useAngleUnit        =false;

       SET_BUILTIN_FUNC1(asinh  , inverse of hyperbolic sine     , asinh);
       SET_BUILTIN_FUNC1(acosh  , inverse of hyperbolic cosine   , acosh);
       SET_BUILTIN_FUNC1(atanh  , inverse of hyperbolic tangent  , atanh);
       SET_BUILTIN_FUNC1(acoth  , inverse of hyperbolic cotangent, acoth);
	   SET_BUILTIN_FUNC1(sinh   , hyperbolic sine                , sinh);

       SET_BUILTIN_FUNC1(cosh, hyperbolic cosine, cosh);
       SET_BUILTIN_FUNC1(ch, hyperbolic cosine, cosh);
       SET_BUILTIN_FUNC1(coth, hyperbolic cotangent, coth);
       SET_BUILTIN_FUNC1(cth, hyperbolic cotangent, coth);
       SET_BUILTIN_FUNC1(exp, power of e, exp);
       SET_BUILTIN_FUNC1(fact, factorial, fact);
       SET_BUILTIN_FUNC1(frac, fractional part, frac);
       SET_BUILTIN_FUNC1(int, integer part, floor);
       SET_BUILTIN_FUNC1(lg, base 10 logarithm, log10);
       SET_BUILTIN_FUNC1(log, natural logarithm, ln);
       SET_BUILTIN_FUNC1(log2, base 2 logarithm, log2);
       SET_BUILTIN_FUNC1(log10, base 10 logarithm, log10);
       SET_BUILTIN_FUNC1(ln, natural logarithm, ln);
       SET_BUILTIN_FUNC2R(pow, pow(x,y)=x^y, pow);
       SET_BUILTIN_FUNC2I(root, n-th root, root);
       SET_BUILTIN_FUNC2I(round, rounding, round);
       SET_BUILTIN_FUNC1(sign, sign of number, Sign);
       SET_BUILTIN_FUNC1(sqrt, square root, sqrt);
       SET_BUILTIN_FUNC1(tanh, tangent, tanh);
       SET_BUILTIN_FUNC1(trunc, truncate to integer, floor);

       numBuiltinFuncs = functions.size();

       builtinsOk = true;
    }
}


const Token& LittleEngine::_Stack::peek(unsigned n) const
{
    if (_stack.size() < n)
        Trigger(Trigger_Type::STACK_ERROR);
    return _stack[_stack.size() - n];
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
void LittleEngine::_HandleUnknown(Token *tok)
{
	if(tok->Text()[0] == _argSeparator )// If the token is a function argument separator (e.g., a comma):
	{
								// Until the token at the top of the stack is a left parenthesis,
								// - (SA) which IS part of a function name! -
								// pop operators off the stack onto the output queue. If no left
								// parenthesis are encountered, either the separator was misplaced or
								// parenthesis were mismatched
		while(!stack.empty () )
		{
			TokenType tk = stack.peek().Type();
			if( tk == tknFunction )
				break;
			stack.popto(tvPostfix);
		}
		if(stack.empty () )
			Trigger(Trigger_Type::EITHER_THE_SEPARATOR_WAS_MISPLACED_OR_PARENTHESIS_WERE_MISMATCHED);
	}
}

/*==================================================
 * TASK: handle a single operator
 *-------------------------------------------------*/
void LittleEngine::_HandleOperator(Token* tok)
{
                           // If the token is an operator, 'tok', then
    while(!stack.empty())  // while there is an operator token, 'op2', at the top of the stack, and
    {
        const Token &op2 = stack.peek();
                            // either 'tok' is left-associative and its precedence is less than or equal to that of 'op2'
                            // or 'tok' has precedence less than that of o2
        if(tok->Precedence() < op2.Precedence() || (op2.Precedence() == tok->Precedence() && !tok->RightAssoc()))
            stack.popto(tvPostfix);			// pop 'op2' off the stack, onto the output queue;
        else
            break;			// precedence of operator 'tok' is larger than that of 'op2' or equal but 'tok' is right assoc.
    }
    stack.push(*tok);					// push o1 onto the stack
}

/*==================================================
 * TASK: handle a single left or right brace
 *-------------------------------------------------*/
void LittleEngine::_HandleBrace(Token* tok)
{
    if(tok->Oper() == opOpenBrace) // If the token is a left parenthesis, then push it onto the stack
        stack.push(*tok);
    else                           // If the token is a righ parenthesis
    {
        TokenType tk = stack.peek().Type();
        while(!stack.empty() && ( (tk != tknBrace && tk != tknFunction) || (tk == tknBrace && stack.peek().Oper() != opOpenBrace)))
        {   // Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
            stack.popto(tvPostfix);
            tk = stack.peek().Type();
        }
        if(!stack.empty())
        {
            if(tk == tknFunction)
                stack.popto(tvPostfix);            // If the token at the top of the stack is a function token, pop it onto the output queue.
            else
                stack.pop();					   // Pop the left parenthesis from the stack, but not onto the output queue.
        }
        else                                       // If the stack runs out without finding a left parenthesis, then there are mismatched parenthesis.
            Trigger(Trigger_Type::MISMATCHED_PARENTHESIS);
    }
}

/*==================================================
 * TASK: 
 * EXPECTS: 
 *-------------------------------------------------*/
 /*=============================================================
  * TASK   : convert single infix expression in 'expr'
  *             to postfix expression in 'tvPostfix'
  * PARAMS : 'expr' - infix expression which may contain a variable
  *             and a function definition as well
  * EXPECTS: - 'infix' may end with '\n' 
  *          - 'infix' may contain variable/function definitions 
  *             possibly followed by measurement unit then comment 
  *             separated by 'schCommentDelimiter' characters
  *          - unit and comment may be missing
  * GLOBALS: 'infix', 'tvPostFix'
  * RETURNS: 0      : found assignment expression
  *          1      : other
  *          and  'tvPostfix' is the token vector of expression
  * REMARKS: - variable definitions start with the variable name
  *             followed by an equal sign, then the expression of 
  *             the variable. This expression may use already defined
  *             variables and functions. The definition may also contain 
  *             a unit and a comment field. Field delimiter is the semicolon
  *             Example: "twopi=2*pi:2 times pi"
  *                      "VAT=18:value added tax:percent
  *          - function definition starts with the function name 
  *            followed by a list of comma separated argument names 
  *             in parantheses then an equal sign then the function
  *             body followed by the comment and the possible unit:
  *             Example: 
  *              "Quad(a,b,c,s)=(-b+s*sqrt(b^2-4*a*c))/2/a:quadratic equation"
  *------------------------------------------------------------*/
int LittleEngine::_InfixToPostFix(const SmartString expr)
{

    //check for invalid characters up to the comment field
	locale loc = cout.getloc();
    infix.clear();
    //if( infix[  infix.length() -1] == '\n')
    //    infix = infix.substr(0, infix.length()-1);
	SmartString pattern = "=*^/<>!&|~%().,+-_#'"_ss;
    //                    "!+-*/_.,^%@#()=<>|\\:'\"~&"
    bool quoted = false;
    //int poseq = 0;
    //if ( (poseq = expr.indexOf(L'=')) > 0)
    //    infix = expr.left(poseq);
    // copy string into infix and drop internal spaces
    // and check for missing or mixed braces
    int bc = 0;   // brace count
    SmartString::const_iterator it;
    for(it = expr.begin()/* + poseq*/; it != expr.end() && *it != FalconCalc::schCommentDelimiter; ++it)
    {
        if (!quoted)
        {
            if (isspace(*it, loc))  // drop spaces inside
                continue;
            if (!IsAlnum((wchar_t)*it, loc) && pattern.find_first_of(*it) == std::string::npos)
                Trigger(Trigger_Type::ILLEGAL_CHARACTER);
            if (*it == u'(')
                ++bc;
            else if (*it == u')')
            {
                if (--bc < 0)
                    Trigger(Trigger_Type::MISMATCHED_PARENTHESIS);
            }
        }
        if(*it == '\'')
            quoted ^= true;
        infix.push_back(*it);   //  even quoted string don't lowercase anything here
    }
    if(bc)
        Trigger(Trigger_Type::MISMATCHED_PARENTHESIS);
    infix += expr.mid(it - expr.begin());   // add comment and unit

    bool inq = false;       // inside quote
    

    int result = 1;     // not an assignment
    tvPostfix.clear();  // get rid of previous result

    bool needOp = false;        // operator needed ? Used for unary +,- or missing multiplication
     // example:
     //  expression: "+3(-alma) + -2pi", pocessing:
     //  token     needOp       result token      type    result needOp
     //    +       false         nothing                       false
     //    3       false         3          (tknNumber)        true
     //    (       true          *          (tknOperator)      false
     //    (       false         (          (tknBrace)         false
     //    -       false         unary -    (tknOperator)      false
     //    alma    false         alma       (tknVariable)      true
     //    )       true          )          (tknBrace)         true
     //    +       true          +          (tknOperator)      false
     //    -       false         unary -    (tknOperator)      false
     //    2       false         2          (tknNumber)        true
     //    pi      true          *          (tknOperator)      false
     //    pi      false         pi         (tknVariable)      true
     //  EOLN
	unsigned pos = 0;
	Token *tok = new Token(infix, pos);
					// Shunting yard algorithm from Wikiedia (http://en.wikipedia.org/wiki/Shunting-yard_algorithm)
	while (tok->Type() != tknEOL) // get all tokens from line
	{
		try
		{
			if (needOp && tok->Type() != tknOperator && tok->Type() != tknUnknown && tok->Oper() != opCloseBrace) // then suppose it's implicit multiplication
			{
				OP d;
				d.oper = opMUL;
				d.precedence = 7; // C.f. MathOperator::Setup()
				Token* op = new Token(tknOperator, "*"_ss, d);
				_HandleOperator(op);
				delete op;
				needOp = false;
			}

			switch (tok->Type())
			{
				case tknCharacter:                          // number from character SmartString (as BIG endian!)
				case tknNumber:								// If the token is a number then add it to the output queue.
					tvPostfix.push_back(*tok);
					needOp = true;
					break;
				case tknVariable:	  						// If the token is a variable check for assignments
					if (!_VariableAssignment(infix, pos, tok))   // handles assignment
					{
						if (functions.count(tok->Text()))   // then this isnt a variable just missing the braces yet
							Trigger(Trigger_Type::FUNCTION_MISSING_OPENING_BRACE);
						tvPostfix.push_back(*tok);          // add variable to the output queue.
						needOp = true;
					}
					else
						result = 0;
					break;
				case tknFunction:							// If the token is a function name token,
					if (!_FunctionAssignment(infix, pos, tok))
					{
						if (pos >= infix.length() || infix.at(pos) == SCharT(')'))    // eg 'sin(' or 'sin()' w.o. argument
							Trigger(Trigger_Type::NO_FUNCTION_ARGUMENT);

						stack.push(*tok);   // then push it onto the stack.
					}
					else                     // function asignment
					{
						result = 0;
						pos = infix.length(); // function body processed already
					}
					break;
				case tknUnknown:
					_HandleUnknown(tok);
					needOp = false;
					break;
				case tknOperator:   
                    if (!needOp)     // '!', 'not' '~', unary '-' or '+'
				    {
					    // check for too many '+' or '-'
					    unsigned pn = pos; // look ahead
					    Token* next = new Token(infix, pn);
                                
                        if (next->Type() == tknOperator)
                        {
                            bool badOp = (next->Oper() != opMINUS) && 
                                         (next->Oper() != opPLUS)  &&
                                         (next->Oper() != opNOT)   &&
                                         (next->Oper() != opCompl);
                            if (badOp)
                            {
                                delete next;
                                delete tok;
                                Trigger(Trigger_Type::SYNTAX_ERROR);
                            }
                        }
					    delete next;

					    if (tok->Oper() == opMINUS)   // unary -
					    {
						    OP d;
						    d.oper = opUMIN;
						    d.precedence = 8; // Cf MathOperator::Setup() !
						    Token* op = new Token(tknOperator, "@"_ss, d);
						    delete tok;
						    tok = op;
					    }
					    else if (tok->Oper() == opPLUS)   // unary +
						    break;  // skip it
					    else if (tok->Oper() == opNOT || tok->Oper() == opCompl)
					    {
						    _HandleOperator(tok);
						    break;
					    }
					    else
					    {
						    delete tok;
						    Trigger(Trigger_Type::SYNTAX_ERROR);
					    }
				    }
					_HandleOperator(tok);
					needOp = false;
					break;
				case tknBrace:      
                    _HandleBrace(tok);
					break;
				default: break;     // to make compilers happy
			}
			//      delete tok;
				  //tok = new Token(infix, pos);
		}
        catch (Trigger_Type tt)
        {
            if (tok->Type() == tknBrace && tt == Trigger_Type::STACK_ERROR)
                throw Trigger_Type::MISMATCHED_PARENTHESIS;
            else
                throw;
        }
		catch (...)
		{
			;
		}
		tok->FromText(infix, pos);  // new token 
    }
    delete tok;
		// When there are no more tokens to read:

	while( !stack.empty())				// While there are still operator tokens in the stack:
	{
		if(stack.peek().Type() == tknBrace )
			Trigger(Trigger_Type::MISMATCHED_PARENTHESIS);
		stack.popto(tvPostfix);
	}
    return result; // 0: assignment, 1: other expression
}

/*==============================================
 * TASK: when a variable's body changes all variables
 *       containing it in their definition must be
 *       marked dirty, so that a subsequent calculation
 *       can reflect the change.
 *       Example:
 *              a = 12
 *              b = 2*a (==24)
 *              a = 3 => b == 6
 *---------------------------------------------*/
void LittleEngine::_MarkDirty(const SmartString name)
{
    FunctionTable::iterator it;
    SmartString lcName = name.asLowerCase();
    for(it = functions.begin(); it != functions.end(); ++it)
    {
        TokenVec::iterator tvit;
        for(tvit = it->second.definition.begin(); tvit != it->second.definition.end(); ++tvit)
            if (tvit->Type() == tknVariable)
            {

                if (tvit->Text().asLowerCase() == lcName)
                {
                    it->second.dirty = true;
                    break;
                }
            }
    }
}

/* =========================================================
 * TASK: test expression for variable assignment.
 * EXPECTS: 'expr' text of definition of variable
 *               should look like: name=<body>[:<description>:[<unit>]]
 *              where ':' is the comment delimiter
 *          'pos' start position after name  - a line may only contain a
 *                single variable definition,
 *          'tok' pointer to variable, must not be nullptr, already contains
 *                  the name of the variable in any character case
 *                  for calculations the name will be converted to lower case
 * RETURNS: true if this is an assignment and variable definition or
 *                  redefinition is stored in VARIABLES
 *         false if first non-whitespace character is not an equal sign
 * REMARKS: variable name already processed and in 'tok'
 *---------------------------------------------------------*/
bool LittleEngine::_VariableAssignment(const SmartString &expr, unsigned &pos, Token *tok)
{
	locale loc = cout.getloc();

    while(pos < expr.length() && isspace((wchar_t)expr[pos], loc))
      ++pos;
    if( pos == expr.length()  || (expr[pos] != '=' && expr[pos] != schCommentDelimiter) )
        return false;       // not an assignment

    Variable v;

    SmartString lcName = tok->Text().asLowerCase();
    if(constantsMap.count(lcName )) 
        Trigger(Trigger_Type::BUILTIN_VARIABLES_CANNOT_BE_REDEFINED);
    else if(variables.count(lcName)) // already defined
        v.data.value = variables[lcName].data.value; // v = variables[ tok->Text()];
    ++pos;   // skip '='

    StringVector sv(expr.mid(pos), schCommentDelimiter, true, true);     // pos after the '=' sign, doesn't drop empty fields
        // sv[0] = body, sv[1] = comment, sv[2] = unit

    v.data.name = tok->Text();  // not necessarily in lower case
    // sv: body:comment:unit
    switch(sv.size())
    {
        case 3:
            v.data.unit = sv[2];
            // [[fallthrough]];			// from C++17
        case 2:
            v.data.desc = sv[1];
            // [[fallthrough]];			// from C++17
        case 1:
            v.body = sv[0];
            break;
        default:
            Trigger(Trigger_Type::VARIABLE_DEFINITION_MISSING);
            break;
    }
#if 0
    while (pos < expr.length() && isspace((wchar_t)expr[pos], loc)) // trim spaces
        ++pos;

    if(pos >= expr.length() )
        return false;

    // new variable assignment: create or modify variable
    SmartString unit, descr;

    unsigned posComment = expr.find_first_of(schCommentDelimiter,pos);
    v.body = expr.substr(pos, posComment - (posComment!= SmartString::npos? pos : 0) );
    pos += v.body.length();

    if(posComment != SmartString::npos)
    {   
        pos = posComment+1;     // ++pos would be enough: we have a single expression in line

        // line format: <variablename> = <definition> [:<comment>[:<unit>]]

        descr = expr.substr(pos, posComment - (posComment!= SmartString::npos? pos : 0) );
        pos += descr.length();

        posComment = expr.find_first_of(schCommentDelimiter,pos); // if there's a unit definition in line too
        if (posComment != SmartString::npos) // yes
        {
            descr = expr.substr(pos, posComment - pos);
            pos = posComment + 1;
        }
        unit = expr.substr(pos);
    }
    v.data = Constant(tok->Text(), RealNumber::RN_0, unit, descr);
#endif

    LittleEngine if2pf(*this);      // functions or variables are not re-initialized
    if2pf._InfixToPostFix(v.body);
    v.definition = if2pf.tvPostfix;
    if(v.definition.size() == 1)    // maybe a constant
    {
        if(v.definition[0].Type() == tknNumber )
            v.data.value = v.definition[0].Value();  // and v.dirty remains false
        else  
            v.data.value = if2pf._CalcPostfix(if2pf.tvPostfix);
    }
    else // leave it dirty :) ??? it wasn't
        v.data.value = if2pf._CalcPostfix(if2pf.tvPostfix);

    variables[lcName] = v;
    // mark variables whose definition contains this variable dirty
    _MarkDirty(tok->Text());    // uses lowercase name
    clean = false;  // table modified

    calcResult = v.data.value;
    return true;    // assignment
}

/* =========================================================
 * TASK: test expression for function assignment.
 * PARAMS: 'expr' text of function definition,
 *                <name>(<argument list>) = <body>[:comment[:unit]]
                   may contain whitespaces which are skipped
 *          'pos' position after the opening brace - 
 *          'tok' pointer to actual function
 * EXPECTS: - function assignments must have the form :
 *              name([arg1,...]), where arg1, etc are valid variable names
 *              (whitespaces may be present between the
 *              arguments and commas and the closing brace)
 *          - a single function definition per string
 * RETURNS: true if this is an assignment and function definition or
 *                  redefinition is stored in FUNCTIONS
 *          false if this is not a function assignment,
 * REMARKS: - throws exception if syntax error
 *---------------------------------------------------------*/
 bool LittleEngine::_FunctionAssignment(const SmartString& expr, unsigned& pos, Token* tok)
{
   calcResult = RealNumber::RN_0;
   unsigned poseq = expr.find_first_of(u'=', pos);
   if (poseq == SmartString::npos) // no equal sign, maybe a colon?
   {
       poseq = expr.find_first_of(schCommentDelimiter, pos);
       if (poseq == SmartString::npos) // no colon either?
           return false;
   }
   SmartString lcName = tok->Text().asLowerCase();
   if(functions.count(lcName) ) // already defined
        if(functions [lcName].builtin)
            Trigger(Trigger_Type::BUILTIN_FUNCTIONS_CANNOT_BE_REDEFINED);
   if(expr[poseq-1] != ')')
        Trigger(Trigger_Type::FUNCTION_DEFINITION_MISSING_RIGHT_BRACE);

   Func f;
   f.name = tok->Text();
   f.builtin = false;
   SmartString arguments = expr.mid(pos, poseq - pos - 1);  // argument list

   f.args = StringVector(arguments, ',' /*_argSeparator */ , false, true);

   StringVector svFields(expr.mid(poseq+1), schCommentDelimiter, true, true);

   // fields: body:comment:unit
   switch (svFields.size())
   {
        case 3:
            f.unit = svFields[2];
            // [[fallthrough]]
        case 2:
            f.desc = svFields[1];
            // [[fallthrough]]
        case 1:
            // [[fallthrough]]
            f.body = svFields[0];
            break;
        default:
            Trigger(Trigger_Type::INVALID_FUNCTION_DEFINITION);
            break;
   }

#if 0
   unsigned bpos = poseq;   // 'bpos': position of closing brace,
   while(bpos > pos && expr[bpos] != ')' )
        --bpos;
   if(bpos == pos && expr[pos] != ')') //no closing brace. == when empty parameter list

   locale loc = cout.getloc();

   while( pos < bpos && isspace((wchar_t)expr[pos], loc))
        ++pos;
   while(pos < bpos ) // get arguments
   {        // order of arguments: left to right
        unsigned n = pos;
        while(n < bpos && (IsAlnum((wchar_t)expr[n]) || expr[n] == '_')) // get word
            ++n;
        f.args.push_back(expr.substr(pos, n - pos) );   // store argument name
        while( n < bpos && isspace((wchar_t)expr[n], loc))
            ++n;
        if(n < bpos && expr[n] != _argSeparator)
            Trigger(Trigger_Type::INVALID_CHARACTER_IN_FUNCTION_DEFINITION);
        ++n;    // skip ',' or to bpos
        while( n < bpos && isspace((wchar_t)expr[n], loc))
            ++n;
          pos = n;
   }
   pos = poseq+1; // after the equal sign
        // get comment
   unsigned posComment = expr.find_first_of(schCommentDelimiter,pos);
   while(pos < expr.length() && pos < posComment && isspace((wchar_t)expr[pos],loc))
    ++pos;
   f.body = expr.substr(pos, posComment-pos);
                              // function body. may contain arguments,
                              // those must be marked
   pos += f.body.length();
   if(posComment != SmartString::npos)
   {
        f.desc = expr.substr(posComment+1);
        pos += f.desc.length()+1; // including the delimiter ':'
   }
#endif
   LittleEngine if2pf(*this);
   try
   {
       if2pf._InfixToPostFix(f.body);
       f.definition = if2pf.tvPostfix;
       functions[lcName] = f;
       // mark variables whose definition contains this function dirty
       _MarkDirty(lcName);
       clean = false;  // table modified
   }
   catch(...)
   {
       throw;
   }


   return true;
}


/*=======================================
 * TASK: Evaluate value of a variable
 * EXPECTS: 'tok' token for a variable
 * GLOBALS: 'variables', 'functions' etc
 * RETURNS: nothing, changes 'stack'
 * REMARKS: non existing variables will result in a
 *          0 value, but no variable is created
 *          dirty variables are re-calculated
 *          and marked as clean
 *-------------------------------------*/
void LittleEngine::_DoVariable(const Token &tok)
{
    SmartString name = tok.Text().asLowerCase();
    if (constantsMap.count(name))
    {
        stack.push(constantsMap[name]->value);
    }
    else if(variables.count(name) )   // existing variable
    {
        Variable &var = variables[name];
        if(var.dirty && !var.being_processed)
        {
            try
            {
                var.being_processed = true;
                var.data.value = _CalcPostfix(var.definition);
                var.being_processed = false;
                var.dirty = false;
            }
            catch(...)
            {
                var.being_processed = false;
                var.dirty = false;
            }
        }
        stack.push(var.data.value);
    }
    else
        stack.push(RealNumber::RN_0);
}

/*=======================================
 * TASK: Evaluate value of a function
 * EXPECTS: 'tok' token for a function
 * GLOBALS: 'variables', 'functions' etc
 * RETURNS: nothing, changes 'stack'
 * REMARKS: non existing variables will result in a
 *          0 value, but no variable is created
 *          dirty variables are re-calculated
 *          and marked as clean
 *          non existing functions trigger an error
 *-------------------------------------*/
void LittleEngine::_DoFunction(const Token &tok)
{
    SmartString name = tok.Text().asLowerCase();
    if(!functions.count(name) ) // non existing function
        Trigger(Trigger_Type::UNKNOWN_FUNCTION_IN_EXPRESSION);

    Func &f = functions[name];
    if(f.being_processed) // then recursive call
        Trigger(Trigger_Type::RECURSIVE_FUNCTIONS_ARE_NOT_ALLOWED);
    RealNumber v,r;
    int i;
    if(f.builtin) // arguments on stack, except for 
    {
        BuiltInFunction::ArgTyp argTyp = f.function.SecondArgumentType();
        switch(argTyp)
        {
            case BuiltInFunction::atyNone:
                v = stack.peek(1).Value();
                stack.pop(1);
                v = f.function(v);
                break;
            case BuiltInFunction::atyR:
                r = stack.peek(1).Value();
                stack.pop(1);
                v = stack.peek(1).Value();
                stack.pop(1);
                v = f.function(v, r);
                break;
            case BuiltInFunction::atyI:
                r = stack.peek(1).Value();
                stack.pop(1);
                v = stack.peek(1).Value();
                stack.pop(1);
                i = int(r.Int().ToInt64());
                v = f.function(v, i);
                break;
            case BuiltInFunction::atyA:     // second argument is not on stack
                break;
        }
        v.RoundToDigits(RealNumber::MaxLength() + 1);
        stack.push(v);
        return;
    }
    vector<RealNumber> params;
    unsigned n = f.args.size();
        // get parameters from stack into 'params'
        // the i-th patameter will correspond to the
        // i-th argument
    for(unsigned i = n ; i > 0; --i)
    {   // arguments were pushed from left to right
        params.push_back( stack.peek(i).Value() );
    }
    stack.pop(n);
        // create new postfix expression for function
        // in which function parameters are replaced with their values
    TokenVec tv;
    bool allocd = false;
    for(unsigned i = 0; i < f.definition.size(); ++i)
    {
        Token *tok = & f.definition[i];
        if(tok->Type() == tknVariable) // then if it is a parameter then
        {                              // replace it with the parameter's value
            for(unsigned j = 0; j < f.args.size(); ++j)
                if(tok->Text().asLowerCase() == f.args[j].asLowerCase()) // parameter
                {
                    tok = new Token(params[j]);
                    allocd=true;
                    break;
                }
        }
        tv.push_back(*tok);
        if(allocd)
        {
            delete tok;
            allocd=false;
        }
    }
    try
    {
        f.being_processed = true;
        v = _CalcPostfix(tv);
        stack.push(v);
        f.being_processed = false;
    }
    catch(...)
    {
        f.being_processed = false;
        throw;
    }
}

static RealNumber Complement(const RealNumber& r)
{
    int val = ~int(r.ToInt64());
    return RealNumber(val);
}
/*=======================================
 * TASK: Evaluate operators.
 * EXPECTS: values are on stack. For binary
 *       operators the right argument is at
 *       the top of the stack (pos=1) the left
 *       argument is below it
 * RETURNS: nothing, changes 'stack'
 *-------------------------------------*/
void LittleEngine::_DoOperator(const Token &tok)
{
    RealNumber res = RealNumber::RN_0;

    try
    {
        switch(tok.Oper() )
        {
            case opOR : res = (RealNumber)( stack.peek(2).Value() |  stack.peek(1).Value() );
                        stack.pop(2);
                        break;
            case opXOR: res =  (RealNumber)( stack.peek(2).Value().XOr(stack.peek(1).Value()) );
                        stack.pop(2);
                        break;
            case opAND: res =  (RealNumber)( stack.peek(2).Value().And(stack.peek(1).Value()) );
                        stack.pop(2);
                        break;
                        // '=='
            case opEQ:  res = (stack.peek(2).Value() == stack.peek(1).Value() ? RealNumber::RN_1 : RealNumber::RN_0);
                        stack.pop(2);
                        break;
            case opNEQ:  // '!='
                        res = (stack.peek(2).Value() != stack.peek(1).Value() ? RealNumber::RN_1:RealNumber::RN_0);
                        stack.pop(2);
                        break;
            case opLT:   // '<'
                        res = (stack.peek(2).Value() < stack.peek(1).Value() ? RealNumber::RN_1 : RealNumber::RN_0);
                        stack.pop(2);
                        break;
            case opLE:   // '<='
                        res = (stack.peek(2).Value() <= stack.peek(1).Value() ? RealNumber::RN_1 : RealNumber::RN_0);
                        stack.pop(2);
                        break;
            case opGT:   // '>'
                        res = (stack.peek(2).Value() > stack.peek(1).Value() ? RealNumber::RN_1 : RealNumber::RN_0);;
                        stack.pop(2);
                        break;
            case opGE:   // '>='
                        res = (stack.peek(2).Value() >= stack.peek(1).Value() ? RealNumber::RN_1 : RealNumber::RN_0);
                        stack.pop(2);
                        break;
            case opSHL:  // '<<'    shift as binary
                        res =  stack.peek(2).Value() * RealNumber::RN_2.Pow(stack.peek(1).Value().Int());
                        stack.pop(2);
                        break;
            case opSHR:  // '>>'    shift as binary
                        res =  stack.peek(2).Value() / RealNumber::RN_2.Pow(stack.peek(1).Value());
                        stack.pop(2);
                        break;
            case opPLUS:
                        res = stack.peek(2).Value() + stack.peek(1).Value();
                        stack.pop(2);
                        break;
            case opMINUS:
                        res = stack.peek(2).Value() - stack.peek(1).Value();
                        stack.pop(2);
                        break;
            case opMUL:
                        res = stack.peek(2).Value() * stack.peek(1).Value();
                        stack.pop(2);
                        break;
            case opDIV: if(stack.peek(1).Value() == RealNumber::RN_0)
                            Trigger(Trigger_Type::DIVISON_BY_0);
                        res = stack.peek(2).Value() / stack.peek(1).Value();
                        stack.pop(2);
                        break;
            case opMOD:  // '%'
                        res = fmod(stack.peek(2).Value(),  stack.peek(1).Value());
                        stack.pop(2);
                        break;
            case opUMIN:  // unary minus on stack ('@')
                        res = -stack.peek(1).Value();
                        stack.pop(1);
                        break;
            case opNOT:
                        res = stack.peek(1).Value() != RealNumber::RN_0 ? RealNumber::RN_0 : RealNumber::RN_1;
                        stack.pop(1);
                        break;
			case opCompl:
						res = Complement(stack.peek(1).Value());
						stack.pop(1);
						break;
			case opPOW:   // exponent ('^')
                        res = pow(stack.peek(2).Value(), stack.peek(1).Value());
                        stack.pop(2);
                        break;
            default: break; // to make compilers happy
        }
    }
    catch(...)
    {
        throw;
    }
    stack.push( res );
}

/*==================================================
 * TASK: calculate postfix expression in 'tvPostfix'
 *       Because it can be called recursively it must
 *       remember the stcak position on entry
 *-------------------------------------------------*/
RealNumber LittleEngine::_CalcPostfix(TokenVec& tvPostfix)
{
    unsigned stack_cnt = stack.size();
    TokenVec::iterator it;
    for(it = tvPostfix.begin(); it != tvPostfix.end(); ++it)
    {
        switch(it->Type() )
        {
            case tknCharacter:
            case tknNumber:    stack.push(*it); break;
            case tknVariable:  _DoVariable(*it); break;
            case tknFunction:  _DoFunction(*it); break;
            case tknOperator:  _DoOperator(*it); break;
            default: break;
        }
    }
    if(stack.empty() || stack.size() > stack_cnt + 1 || (stack.peek().Type() != tknNumber && stack.peek().Type() != tknCharacter))
        Trigger(Trigger_Type::EXPRESSION_ERROR);
    Token tok( stack.peek() );
    stack.pop(1);

    resultType = ResultType::rtNumber;
    return calcResult = tok.Value().RoundedToDigits(RealNumber::MaxLength()+1);
}

void LittleEngine::_SetNewLocale(SmartString sLocaleName)
{
    locale loc(sLocaleName.toUtf8String().c_str());
    cin.imbue(loc);
    cout.imbue(loc);
}

/*==================================================
 * TASK: calculate expression
 * EXPECTS: 'au' (angle units) is the last used.
 *			If it was not explicitely set the
 *           defult is auRad (radian)
 * RETURNS: calculated value or exception thrown
 *-------------------------------------------------*/
RealNumber LittleEngine::Calculate()
{
    try
    {
        if(!_InfixToPostFix(infix) ) // then variable or function definition
        {
            resultType = ResultType::rtDefinition;
            return calcResult;
        }

        return _CalcPostfix(tvPostfix);
    }
    catch(...)
    {
      ;  // TO_DO : error handling
      resultType = ResultType::rtInvalid;
	  stack.clear();
	  throw;	// re-throw if not handled
    }
    return RealNumber::RN_0;
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::Postfix() const
{
    SmartString postfix;
    for(unsigned i = 0; i < tvPostfix.size(); ++i)
        postfix += tvPostfix[i].Text() + " "_ss;
    return postfix;
}
#if defined(__BORLANDC__)
// valamiert "bad file descriptor" van ha a RAD_STUDIO_XE-vel forditom
     #include <stdio.h>
#endif
/*========================================================
 * TASK: Write modified user functions and variables table to file
 * EXPECTS: 'name' is either the name of the file or empty
 *          When it is NULL the same name is used as in 'ReadTables'
 *          if that was called before, otherwise no write occurs
 *          'clean' when true the tables are modified
 * RETURNS: true: if there was no need to save the data or the save was
 *          successful, false otherwise
 *-----------------------------------------------------------*/
bool LittleEngine::SaveUserData(SmartString filename) // if it wasn't read and no name is given it won't be saved
{
    if(clean && (filename.empty() || ssNameOfDatFile == filename))                    // 
        return true;       // don't save if it was saved already into this file
    if(!filename.empty() && ssNameOfDatFile.empty()) // or when no save requested
        return true;

    if (filename.empty())
        filename = ssNameOfDatFile;

    std::wofstream ofs;
	ofs.open(SmartString(filename).ToWideString(), ios_base::out);
	if( ofs.fail() )
			return false;
    locale loc = cout.getloc();
    SmartString sLocale(loc.name());

	ofs << VERSION_STRING << "\n[Locale]\nloc=" <<  sLocale.ToWideString()
        << L"\n\n[Variables]\n";
    std::wstring sDelim = ssCommentDelimiterString.ToWideString();
	if(variables.size() )                       // these are the user defined variables only
	{
		for(auto &vit : variables)
        {
			ofs << vit.second.data.name.ToWideString() << L"=" << vit.second.body.ToWideString();
            if (!vit.second.data.desc.empty())
                ofs << sDelim << vit.second.data.desc.ToWideString();
            else if (!vit.second.data.unit.empty())
                ofs << sDelim;
            if(!vit.second.data.unit.empty())
				ofs << sDelim << vit.second.data.unit.ToWideString();

			ofs << endl;
		}
	}
	ofs << ("\n[Functions]\n"_ss).ToWideString();
	if(functions.size() )
	{
		for(auto &fit : functions)
		{
			if(fit.second.builtin)
				continue;
			ofs << fit.second.name.ToWideString() << ("("_ss).ToWideString();
			if(!fit.second.args.empty())
			{
				vector<SmartString>::const_iterator vit;
				vit = fit.second.args.begin();
				ofs << (*vit).ToWideString();
				++vit;
				for( ; vit != fit.second.args.end(); ++vit)
					ofs << ", " << (*vit).ToWideString();
			}
			ofs << ") = " << fit.second.body.ToWideString();
			if(!fit.second.desc.empty() )
				ofs << ssCommentDelimiterString.ToWideString() << fit.second.desc.ToWideString();
			ofs << endl;
		}
	}
    clean = true;
    return true;
}
/*========================================================
 * TASK: Read user functions and variables from a file
 * EXPECTS: file name
 * RETURNS: true for success or false for error
 *-----------------------------------------------------------*/
bool LittleEngine::LoadUserData(SmartString name)
{
    if(name.empty())
        name = ssNameOfDatFile;

    std::wifstream in(SmartString(name).ToWideString(), ios_base::in);
	if(in.fail() )
		return false;

	std::wstring line;
	std::getline(in, line);
    
	if(line.substr(0, 12) != L"FalconCalc V" )
		return false;

    SmartString s;
    auto __GetLine = [&]()
        {
            int i = -1;
            while (std::getline(in, line))
            {
                s = line;
                if ((i = s.indexOf('#')) >= 0)
                    s.erase(s.begin() + i, s.end());
                s.Trim();
                if (!s.empty())
                    return s;
            }
            s.clear();
            return s;
        };

    if (!__GetLine().empty() && s == SmartString("[Locale]") )   // get locale name
    {
        if (!__GetLine().empty() )
            if(s.indexOf(u'=') > 0 )   // loc=locale
            {
                locale      loc(s.mid(s.indexOf(u'=')+1).toUtf8String().c_str());
                in.imbue(   loc);
                cout.imbue( loc);
                __GetLine();
            }
    }

    while (!s.empty())
    {
        if (s != SmartString("[Variables]") && s != SmartString("[Functions]"))
        {
            try
            {
                _InfixToPostFix(s);    // handles assignements and function definitions as well
            }
            catch (...) // and puts them into 'variables' or 'functions'
            {
                throw;
            }
        }
        __GetLine();
    }

    return clean = true;
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsDecString()
{
    DisplayFormat fmt = displayFormat;
    fmt.base = DisplayBase::rnb10;
    return calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsHexString() 
{
    DisplayFormat fmt = displayFormat;
    fmt.base = DisplayBase::rnbHex;
    return calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsOctString() 
{
    DisplayFormat fmt = displayFormat;
    fmt.base = DisplayBase::rnbOct;
    return calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsBinString() 
{
    DisplayFormat fmt = displayFormat;
    fmt.base = DisplayBase::rnbBin;
    fmt.useNumberPrefix = true;
    return calcResult.ToString(fmt);
}

/*=============================================================
 * TASK   : create a character string that corresponds to
 *          the unsigned hexadecmal representation of an integer number 
 * PARAMS : nothing
 * EXPECTS: 
 * GLOBALS:
 * RETURNS: a character string
 * REMARKS: example: 3 684 666 -> 0x38393A -> "89:"
 *          only the integer part of 
 *------------------------------------------------------------*/
SmartString LittleEngine::ResultAsCharString() 
{
    return calcResult.ToSmartString();
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
void LittleEngine::GetVarFuncInfo(VarFuncInfo &vf)
{
    vf.uBuiltinFuncCnt = numBuiltinFuncs;
    vf.sBuiltinFuncs   = SerializeFunctions(true);
    vf.uBuiltinVarCnt  = numBuiltinVars;
    vf.sBuiltinVars    = SerializeVariables(true);

    vf.uUserFuncCnt    = functions.size() - numBuiltinFuncs;
    vf.sUserFuncs      = SerializeFunctions(false);
    vf.uUserVarCnt     = variables.size();// -numBuiltinVars;
    vf.sUserVars       = SerializeVariables(false);

    vf.pOwner          = this;
}

/*========================================================
 * TASK:    Create a display string for all constants 
 *          (i.e. bultins) and variables in 
 *          lines (separated by '\n') in format
 *              name = value:[unit:]description
 * EXPECTS: flag for what to show
 * RETURNS: single SmartString containing variables in lines
 *          line format:
 *              name:value:comment:unit\n - for builins
 *              name:body:comment:unit\n - for user variables
 *-----------------------------------------------------------*/
SmartString LittleEngine::SerializeVariables(bool builtin) const
{
    SmartString sres;

    if (builtin)
    {
        for (auto& it : constantsMap)
        {
            RealNumber value = it.second->value;
            DisplayFormat fmt;
            fmt.strThousandSeparator = u" ";
            if (value < RealNumber::RN_0)
                fmt.mainFormat = NumberFormat::rnfSci;
            if (value.Precision() > 32)
                fmt.decDigits = 32;
            sres += it.first + ssCommentDelimiterString + value.ToString(fmt)
                    + SmartString(schCommentDelimiter) + it.second->unit + ssCommentDelimiterString  
                    + it.second->desc + "\n"_ss;
        }
    }
    else
    {
        for (auto& it : variables)
        {
            sres += it.second.data.name + ssCommentDelimiterString + it.second.body + 
                            ssCommentDelimiterString + it.second.data.desc + 
                            ssCommentDelimiterString + it.second.data.unit + "\n"_ss;
        }
    }
    return sres;
}

/*========================================================
 * TASK:    Create a display string for all functions
 * EXPECTS: whatToShow: true: list of builtins
 *                      false: list of user defined functions
 * RETURNS: single SmartString containing functions in \n
 *          separated lines
 * REMARKS: - output format: lines, separated by \n characters
 *            line format:
 *              name(arguments):body:description\n
 *-----------------------------------------------------------*/
SmartString LittleEngine::SerializeFunctions(bool whatToShow) const
{
    SmartString sres;
    constexpr const int BUILTIN = 1;

    for(auto &it : functions)
        if(it.second.builtin == whatToShow)
        {
            sres += it.second.name + "("_ss;
            if (whatToShow)
                sres += "x"_ss;
            else
                for(unsigned j = 0; j < it.second.args.size(); ++j)
                {
                    if(j)
                        sres += SmartString(_argSeparator) + SmartString(" "_ss);
                    sres += it.second.args[j];
                }
            sres += ")"_ss + ssCommentDelimiterString + it.second.body + SmartString(schCommentDelimiter) + it.second.desc + "\n"_ss;
        }
    return sres;
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
LittleEngine &LittleEngine::operator=(const LittleEngine &src)
{
    infix       = src.infix;
    tvPostfix    = src.tvPostfix;
    calcResult  = src.calcResult;
    displayFormat = src.displayFormat;
    //variables   = src.variables;      static variables now
    //functions   = src.functions;
    ssNameOfDatFile = src.ssNameOfDatFile;
    clean       = src.clean;
    return *this;
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
bool LittleEngine::AddUserVariablesAndFunctions(SmartString def, int what) //what 0: both, 1: vars, 2: functions
{
    if(def.empty() )
        return false;
    LittleEngine ip(*this);
        // erase user functions/variables or both depending on 'what'
    VariableTable::iterator vit;
    FunctionTable::iterator fit;
    switch(what)
    {
        case 0:
				// erase existing user variables
        case 1: variables.clear();
                if(what == 1) // for 0: fall through
                    break;
				// erase existing user functions
        case 2: for(auto &f:functions)
                    if(!f.second.builtin)
                        functions.erase(f.first);
        default:
                break;
    }
        // add new user variables or functions
    unsigned st = 0, en;  // start and end positions
    while(st < def.length() )
    {
        en = def.find_first_of(char16_t('\n'), st);
        try
        {
           ip._InfixToPostFix(def.substr(st,en-st) );
        }
        catch(...)
        {
            ;
        }
        st = en + 1;
    }
    if(what == 1)
        variables = ip.variables;
    else if (what == 2)
        functions = ip.functions;
    else
    {
        variables = ip.variables;
        functions = ip.functions;
    }
    clean = false; // variables modified
    return true;
}

// end of namespace FalconCalc
}

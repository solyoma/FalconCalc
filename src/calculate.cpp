#include "calculate.h"


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

using namespace SmString;
using namespace LongNumber;
                     // static so each engine has the same constants and variables
VariableTable LittleEngine::variables;
FunctionTable LittleEngine::functions;

extern FalconCalc::LittleEngine* lengine;

/*==============================================================
 * NAMESPACE FalconCalc
 *-------------------------------------------------------------*/
namespace FalconCalc {
// originally this was a 'const SmartString', but it became(?) empty in
// SaveTables() under RAD Studio XE
constexpr auto VERSION_STRING = "FalconCalc V1.0";


const SCharT comment_delimiter = SCharT(':');


/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
void Trigger(SmartString text)
{
		throw text;
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
void Token::GetOperator(const SmartString &text, unsigned &pos)
{
#ifdef QTSA_PROJECT
	uint c = text[pos++].unicode(),
		cn = (pos == text.length() ? 0 : text[pos].unicode() );
#else
	SCharT c = text[pos++],
		 cn = SCharT(pos >= text.length() ? 0 : text[pos] );
#endif
	SmartString s;
	s = c;

	switch(c)
	{
	case '<' : if(!cn) // no more character in line
					Trigger("Illegal  at line end"_ss);
				switch(cn)
				{
					case '<' : data.oper = opSHL; type = tknOperator; ++pos; return;
					case '=' : data.oper = opLE; type = tknOperator; ++pos; return;
					default  : data.oper = opLT; type = tknOperator; return;
				};
				break;
	case '>' : if(!cn) // no more character in line
					Trigger("Illegal operator at line end"_ss);
				switch(cn)
				{
					case '>' : data.oper = opSHR; type = tknOperator; ++pos; return;
					case '=' : data.oper = opGE; type = tknOperator; ++pos; return;
					default: data.oper = opGT; type = tknOperator; return;
				};
				break;
	case '!':  if(!cn) // no more character in line
					Trigger("Illegal operator at line end"_ss);
				switch(cn)
				{
					case '=' : data.oper = opNEQ; type = tknOperator; ++pos; break;
					default: data.oper = opNOT; type = tknOperator; return;
				};
				break;
	case '=' :  if(!cn) // no more character in line
					Trigger("Illegal operator at line end"_ss);
				switch(cn)
				{
					case '=' : data.oper = opEQ; type = tknOperator; ++pos; break;
					default: data.oper = opLET; type = tknOperator; return;
				};
				break;
	case '~' :  if (!cn) // no more character in line
					Trigger("Illegal operator at line end"_ss);
				if (isdigit(cn) || cn == SCharT('#') )		// if decimal, octal, hexadecimal or binary number
				{
					data.oper = opCompl;			// 2's complement
					type = tknOperator;
					return;
				}
	default:  *this = s;
			  break;
	}
}

/*==========================================
 * TASK: scans 'text' from pos for decimal digits
 *       using current locale
 * EXPECTS: 'text' and 'pos' <= text.length()
 * RETURNS: pos now points after the SmartString of digits
  *----------------------------------------*/
void Token::GetDecDigits(const SmartString &text, unsigned &pos)
{
//#ifdef QTSA_PROJECT
//	#define isdigit(a,b) a.isDigit()
//#else
	locale loc = cout.getloc();
//#endif
	while(pos < text.length() && isdigit(text[pos],loc) )
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
void Token::GetDecimalNumber(const SmartString &text, unsigned &pos)
{

    const SCharT decpoint = RealNumber::DecPoint();

	int startpos = pos;
	int nIntP  = 0,
        nFracP = 0, //           fractional
		nExpP  = 0; //           exponent part starting after the 'e'
	nIntP  = pos;         // start of integer part of number
	GetDecDigits(text, pos);
	nIntP = pos - nIntP;      // length of integer part of number

	if(text[pos] == decpoint)	// decimal point: get fractions
	{
		nFracP = ++pos;
		GetDecDigits(text, pos);
		nFracP = pos - nFracP;
	}
	bool bExp = (text[pos] == 'e' || (text[pos] == 'E'));
	if(bExp)
	{
		if(text[++pos] == '+' || text[pos] == '-')
			++pos;
		nExpP = pos;
		GetDecDigits(text, pos);
		nExpP = pos - nExpP;
	}
	 // a number from a single decimal point or with no exponent after the 'e'
	 // is illegal
	if( (!nIntP && ! nFracP)  || (bExp && ! nExpP))
		Trigger("Illegal number #1"_ss);
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
void Token::GetHexNumber(const SmartString &text, unsigned &pos)
{
    unsigned startpos = pos;
    pos += 2;
	locale loc = cout.getloc();
	while(pos < text.length() && isxdigit(text[pos],loc) )
		++pos;
	if(pos == startpos+2)
		Trigger("Illegal hexadecimal number"_ss);
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
void Token::GetOctNumber(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	unsigned startpos = pos++;
	while(pos < text.length() && isdigit(text[pos],loc))
	{
		if(text[pos] > '7')
				Trigger("Illegal octal number"_ss);
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
void Token::GetBinaryNumber(const SmartString &text, unsigned &pos)
{
    auto triggerError = []()
        {
            Trigger("Illegal binary number"_ss);
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
 * REMARKS: The character SmartString is considered a BIG ENDIAN
 *          number
 *-----------------------------------------------------------*/
void Token::GetNumberFromQuotedString(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	unsigned startpos = pos;
    RealNumber lval = RealNumber::RN_0, r256 = RealNumber(256.0);
	while(pos < text.length() && (text[pos] != '\'' || (pos > 0 && text[pos-1] == '\\')) )
    {
		SCharT ch = text[pos];
        lval = lval * r256 + RealNumber( String(1, ch) ) ;
		++pos;
    }
	if(pos == startpos)
		Trigger("Illegal character number"_ss);
    if(pos == text.length() )
        Trigger("Closing quote not found"_ss);

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
void Token::GetVarOrFuncOrOperator(const SmartString &text, unsigned &pos)
{
	locale loc = cout.getloc();
	int startpos = pos;
	while(pos < text.length() && (isalnum(text[pos],loc) || text[pos] == '_'))
		++pos;
	SmartString s = text.substr(startpos, pos - startpos);
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
	if(pos >= len)
    {
        type = tknEOL;
		return;
    }

    SCharT decpoint = RealNumber::DecPoint();
	SmartString sErr = "Illegal operator"_ss;

	while(pos < len && isspace((wchar_t)text[pos], loc))
		++pos;
	SCharT  c = text[pos++],
			cn = (pos >= len ? SCharT(0) : text[pos]); // look ahead
    --pos;  // go back to start of token

    if(c == SCharT('\'') )   // character SmartString
        GetNumberFromQuotedString(text,pos);
	else if(isdigit(c,loc) || c == decpoint || c == SCharT('#'))		// token is a decimal, hexadecimal, octal or binary number
	{
		bool bDecpF = (c == decpoint),							        // decimal point found ?
				bHexF = (c == SCharT('0') && (cn == SCharT('x') )),		// hex number?
				bOctF = (c == SCharT('0') && cn && isdigit(cn,loc)),
				bBinF = (c == SCharT('#'));

		if(bHexF)                       // starts with 0x and ends when any non hex. digit character found
			GetHexNumber(text, pos);    // 0x....
		else if(bOctF)
			GetOctNumber(text, pos);    // 0....
		else if(bBinF)                  // #...
		{
			if(!cn) // then EOL and number is a single binary type character
				Trigger("Missing binary number"_ss);
			GetBinaryNumber(text, pos);  // starting after the '#' type character
		}
		else // decimal number
		{
			if((!cn && bDecpF) || (bDecpF && !isdigit(cn,loc)))	// then EOL and number is a single decimal point
				Trigger("Illegal number #2"_ss);
			GetDecimalNumber(text, pos); // start at the first number/decimal point
		}
	}
    else if (isalpha(c, loc)) // variable, function or text operator (e.g. 'or')
        GetVarOrFuncOrOperator(text, pos);
	else // a possible operator character
		GetOperator(text, pos);
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

static RealNumber Sin(RealNumber  r) { return sin (r, lengine->angleUnit); }
static RealNumber Csc(RealNumber  r) { return csc (r, lengine->angleUnit); }
static RealNumber Cos(RealNumber  r) { return cos (r, lengine->angleUnit); }
static RealNumber Sec(RealNumber  r) { return sec (r, lengine->angleUnit); }
static RealNumber Tan(RealNumber  r) { return tan (r, lengine->angleUnit); }
static RealNumber Cot(RealNumber  r) { return cot (r, lengine->angleUnit); }
static RealNumber Asin(RealNumber r) { return asin(r, lengine->angleUnit); }
static RealNumber Acos(RealNumber r) { return acos(r, lengine->angleUnit); }
static RealNumber Atan(RealNumber r) { return atan(r, lengine->angleUnit); }
static RealNumber Acot(RealNumber r) { return acot(r, lengine->angleUnit); }

/*========================================================
 * TASK: Creates a single instance of the calculator
 *       if the built-ins are not yet set up sets them up too
 * EXPECTS:all builtins 's name is lowercase!
 * RETURNS: nothing/object created
 *-----------------------------------------------------------*/
LittleEngine::LittleEngine() : clean(true)
{
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
	   #define SET_BUILTIN_FUNC2(a,b,c) f.function.clear(); f.name = u#a; f.desc = u#b; f.function.funct2r = c; functions[#a##_ss] = f;
	   #define SET_BUILTIN_FUNC3(a,b,c) f.function.clear(); f.name = u#a; f.desc = u#b; f.function.funct2i = c; functions[#a##_ss] = f;
	   #define SET_BUILTIN_FUNC4(a,b,c) f.function.clear(); f.name = u#a; f.desc = u#b; f.function.funct2a = c; functions[#a##_ss] = f;
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
       SET_BUILTIN_FUNC1(tan    , tangent   , Tan);
       SET_BUILTIN_FUNC1(tg     , tangent   , Tan);
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
       SET_BUILTIN_FUNC2(exp, power of e, pow);
       SET_BUILTIN_FUNC1(fact, factorial, fact);
       SET_BUILTIN_FUNC1(frac, fractional part, frac);
       SET_BUILTIN_FUNC1(int, integer part, floor);
       SET_BUILTIN_FUNC1(lg, base 10 logarithm, log10);
       SET_BUILTIN_FUNC1(log, natural logarithm, ln);
       SET_BUILTIN_FUNC1(log2, base 2 logarithm, log2);
       SET_BUILTIN_FUNC1(log10, base 10 logarithm, log10);
       SET_BUILTIN_FUNC1(ln, natural logarithm, ln);
       SET_BUILTIN_FUNC3(round, rounding, round);
       SET_BUILTIN_FUNC1(sign, sign of number, Sign);
       SET_BUILTIN_FUNC1(sqrt, square root, sqrt);
       SET_BUILTIN_FUNC1(tanh, tangent, tanh);
       SET_BUILTIN_FUNC1(trunc, truncate to integer, floor);

       numBuiltinFuncs = functions.size();

       builtinsOk = true;
    }
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
void LittleEngine::HandleUnknown(Token *tok)
{
	if(tok->Text()[0] == _ArgSeparator() )// If the token is a function argument separator (e.g., a comma):
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
			Trigger("Either the separator was misplaced or parenthesis were mismatched"_ss);
	}
}

/*==================================================
 * TASK: handle a single operator
 *-------------------------------------------------*/
void LittleEngine::HandleOperator(Token* tok)
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
void LittleEngine::HandleBrace(Token* tok)
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
            Trigger("Mismatched parenthesis"_ss);
    }
}

/*==================================================
 * TASK: 
 * EXPECTS: 
 *-------------------------------------------------*/
 /*=============================================================
  * TASK   : convert single infix expression in 'expr'
 *       to postfix expression in 'tvPostfix'
  * PARAMS :
  * EXPECTS:
  * GLOBALS: 'infix'   
  * RETURNS: 0      : assignment expression
  *          1      : other
  *          and  'tvPostfix' is the token vector of expression
  * REMARKS: - 'infix' may end with '\n' and may contain
  *             variable/function definitions with comments at the
  *             end. Comments are separated from the definition by
  *             'comment_delimiter' which must be different from
  *             any characters allowed in an expression.
  *------------------------------------------------------------*/
int LittleEngine::InfixToPostFix(const SmartString& expr)
{
    //check for (invalid characters up to the comment field
	locale loc = cout.getloc();
	infix = expr;
    if( infix[  infix.length() -1] == '\n')
        infix = infix.substr(0, infix.length()-1);
	SmartString pattern = "=*^/<>!&|~%().,+-input._#'"_ss;
    bool quoted = false;

    for(SmartString::iterator it = infix.begin(); it != infix.end() && *it != FalconCalc::comment_delimiter; ++it)
    {
        if(!quoted && !isalnum((wchar_t)*it, loc) && !isspace((wchar_t)*it,loc) && pattern.find_first_of(*it) == std::string::npos )
    	    Trigger (SmartString("Illegal character '"_ss) +  (char16_t)(*it) + "'"_ss);

        if(*it == '\'')
            quoted ^= true;
        else if(!quoted)
            *it = tolower(*it, loc);
    }

    int result = 1;     // not an assignment
    tvPostfix.clear();   // get rid of previous result

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
	while(tok->Type() != tknEOL) // get all tokens from line
	{
        if(needOp && tok->Type() != tknOperator && tok->Type() != tknUnknown && tok->Oper() != opCloseBrace) // then suppose it's implicit multiplication
        {
            OP d;
            d.oper = opMUL;
            d.precedence = 7; // C.f. MathOperator::Setup()
            Token *op = new Token(tknOperator, "*"_ss, d);
            HandleOperator(op);
            delete op;
            needOp = false;
        }

		switch(tok->Type() )
		{
            case tknCharacter:                          // number from character SmartString (as BIG endian!)
			case tknNumber:								// If the token is a number then add it to the output queue.
								tvPostfix.push_back(*tok);
                                needOp = true;
                                break;
            case tknVariable:	  						// If the token is a variable check for assignments
                                if(VariableAssignment(infix, pos, tok) == 0 )   // handles assignment
                                {
    								tvPostfix.push_back(*tok); // otherwise add it to the output queue.
                                    needOp = true;
                                }
                                else
                                    result = 0;
                                break;
			case tknFunction:							// If the token is a function name token,
                                if(!FunctionAssignment(infix, pos, tok) )
                                {
                                    stack.push(*tok);   // then push it onto the stack.
                                    //unsigned pos = 0;
                                    //SmartString s("(");
                                    //Token brace(s, pos);   // function token ate oopening brace
                                    //stack.push(brace);
                                }
                                else
                                    result = 0;     // asignment
                                break;
			case tknUnknown:
								HandleUnknown(tok);
                                needOp = false;
                                break;
			case tknOperator:   if(!needOp)     // '!', 'not' '~', unary '-' or '+'
                                {
                                    // check for too many '+' or '-'
                                    unsigned pn = pos; // look ahead
                                    Token *next = new Token(infix, pn);
                                    if(next->Type() == tknOperator)
                                    {
                                        delete next;
                                        delete tok;
                                        Trigger("Syntax error"_ss);
                                    }
                                    delete next;

									if (tok->Oper() == opMINUS)   // unary -
									{
										OP d;
										d.oper = opUMIN;
										d.precedence = 8; // Cf MathOperator::Setup() !
										Token *op = new Token(tknOperator, "@"_ss, d);
										delete tok;
										tok = op;
									}
									else if (tok->Oper() == opPLUS)   // unary +
										break;  // skip it
									else if (tok->Oper() == opNOT || tok->Oper() == opCompl)
									{
										HandleOperator(tok);
										break;
									}
                                    else
                                    {
                                        delete tok;
                                        Trigger("Syntax error"_ss);
                                    }
                                }
                                HandleOperator(tok);
                                needOp = false;
                                break;
            case tknBrace:      HandleBrace(tok);
                                break;
            default: break;     // to make compilers happy
		}
  //      delete tok;
		//tok = new Token(infix, pos);
        tok->FromText(infix, pos);  // new token 
	}
    delete tok;
		// When there are no more tokens to read:

	while( !stack.empty())				// While there are still operator tokens in the stack:
	{
		if(stack.peek().Type() == tknBrace )
			Trigger("Mismatched parenthesis"_ss);
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
void LittleEngine::MarkDirty(const SmartString name)
{
    FunctionTable::iterator it;
    for(it = functions.begin(); it != functions.end(); ++it)
    {
        TokenVec::iterator tvit;
        for(tvit = it->second.definition.begin(); tvit != it->second.definition.end(); ++tvit)
            if(tvit->Type() == tknVariable && tvit->Text() == name)
            {
                it->second.dirty = true;
                break;
            }
    }
}

/* =========================================================
 * TASK: test expression for variable assignment.
 * EXPECTS: 'expr' text of definition of variable
 *               should look like: [spaces]=[spaces]<body>[:[<unit>:]<description>]
 *              where ':' is the comment delimiter
 *          'pos' start position after name  - a line may only contain a
 *                single variable definition,
 *          'tok' pointer to variable, must not be nullptr, already contains
 *                  the name of the variable
 * RETURNS: true if this is an assignment and variable definition or
 *                  redefinition is stored in VARIABLES
 *         false if first non-whitespace character is not an equal sign
 * REMARKS: expr format (variable name already processed):
 *       [white spaces]=[white spaces]<definition>[white spaces]
 *---------------------------------------------------------*/
bool LittleEngine::VariableAssignment(const SmartString &expr   , unsigned &pos, Token *tok)
{
	locale loc = cout.getloc();

    while(pos < expr.length() && isspace((wchar_t)expr[pos], loc))
      ++pos;
    if( pos == expr.length()  || expr[pos] != '=')
        return false;       // not an assignment

    Variable v;

    if(constantsMap.count(tok->Text() )) 
        Trigger("Builtin variables cannot be redefined"_ss);
    else if(variables.count(tok->Text() )) // already defined
        v.data.value = variables[ tok->Text()].data.value; // v = variables[ tok->Text()];
    ++pos;   // skip '='
    while (pos < expr.length() && isspace((wchar_t)expr[pos], loc)) // trim spaces
        ++pos;

    if(pos >= expr.length() )
        return false;

    // new variable assignment: create or modify variable
    SmartString unit, descr;

    unsigned posComment = expr.find_first_of(comment_delimiter,pos);
    v.body = expr.substr(pos, posComment - (posComment!= SmartString::npos? pos : 0) );
    pos += v.body.length();

    if(posComment != SmartString::npos)
    {   
        pos = posComment+1;     // ++pos would be enough: we have a single expression in line
        posComment = expr.find_first_of(comment_delimiter,pos); // if there's a unit definition in line too
        if (posComment != SmartString::npos) // yes
        {
            unit = expr.substr(pos, posComment - pos);
            pos = posComment + 1;
        }
        descr = expr.substr(pos);
        pos += descr.length();
    }

    v.data = Constant(tok->Text(), RealNumber::RN_0, unit, descr);

    LittleEngine if2pf(*this);      // functions or variables are not re-initialized
    if2pf.InfixToPostFix(v.body);
    v.definition = if2pf.tvPostfix;
    if(v.definition.size() == 1)    // maybe a constant
    {
        if(v.definition[0].Type() == tknNumber )
            v.data.value = v.definition[0].Value();  // and v.dirty remains false
        else  
            v.data.value = if2pf.CalcPostfix(if2pf.tvPostfix);
    }
    else // leave it dirty :) ??? it wasn't
        v.data.value = if2pf.CalcPostfix(if2pf.tvPostfix);

    variables[ tok->Text()] = v;
    // mark variables whose definition contains this variable dirty
    MarkDirty(tok->Text());
    clean = false;  // table modified

    calcResult = v.data.value;
    return true;    // assignment
}

/* =========================================================
 * TASK: test expression for function assignment.
 * PARAMS: 'expr' text of function definition,
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
 bool LittleEngine::FunctionAssignment(const SmartString& expr, unsigned& pos, Token* tok)
{
   calcResult = RealNumber::RN_0;
   unsigned poseq = expr.find_first_of(u'=', pos);
   if(poseq == SmartString::npos) // no equal sign
        return false;

   if(functions.count(tok->Text()) ) // already defined
        if(functions[ tok->Text()].builtin)
            Trigger("Builtin functions cannot be redefined"_ss);

   unsigned bpos = poseq;   // 'bpos': position of closing brace,

   while(bpos > pos && expr[bpos] != ')' )
        --bpos;
   if(bpos == pos && expr[pos] != ')') //no closing brace. == when empty parameter list
        Trigger("Function definition missing right brace"_ss);

   locale loc = cout.getloc();
   Func f;

   while( pos < bpos && isspace((wchar_t)expr[pos], loc))
        ++pos;
   while(pos < bpos ) // get arguments
   {        // order of arguments: left to right
        unsigned n = pos;
//#if defined QTSA_PROJECT
//        while(n < bpos && (expr[n].isLetterOrNumber() ) || expr[n] == '_')) // get word
//            ++n;
//#else
        while(n < bpos && (isalnum((wchar_t)expr[n]) || expr[n] == '_')) // get word
            ++n;
//#endif
        f.args.push_back(expr.substr(pos, n - pos) );   // store argument name
        while( n < bpos && isspace((wchar_t)expr[n], loc))
            ++n;
        if(n < bpos && expr[n] != _ArgSeparator())
            Trigger("Invalid character in function definition"_ss);
        ++n;    // skip ',' or to bpos
        while( n < bpos && isspace((wchar_t)expr[n], loc))
            ++n;
          pos = n;
   }
   pos = poseq+1; // after the equal sign
        // get comment
   unsigned posComment = expr.find_first_of(comment_delimiter,pos);
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

   LittleEngine if2pf(*this);
   try
   {
       if2pf.InfixToPostFix(f.body);
       f.definition = if2pf.tvPostfix;
       functions[ tok->Text()] = f;
       // mark variables whose definition contains this function dirty
       MarkDirty(tok->Text());
       clean = false;  // table modified
   }
   catch(SmartString text)
   {
       throw;
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
void LittleEngine::DoVariable(const Token &tok)
{
    SmartString name = tok.Text();
    if (constantsMap.count(name))
    {
        stack.push(constantsMap[name]->value);
    }
    else if(variables.count(tok.Text()) )   // existing variable
    {
        Variable &var = variables[tok.Text()];
        if(var.dirty && !var.being_processed)
        {
            try
            {
                var.being_processed = true;
                var.data.value = CalcPostfix(var.definition);
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
void LittleEngine::DoFunction(const Token &tok)
{
    if(!functions.count(tok.Text()) ) // non existing function
        Trigger("Unknown function in expression"_ss);

    Func &f = functions[tok.Text()];
    if(f.being_processed) // then recursive call
        Trigger("Recursive functions are not allowed"_ss);
    RealNumber v;
    if(f.builtin) // single argument on stack
    {
        v = stack.peek(1).Value();
        stack.pop(1);
        v = f.function(v);
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
                if(tok->Text() == f.args[j]) // parameter
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
        v = CalcPostfix(tv);
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
    int64_t val = ~r.ToInt64();
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
void LittleEngine::DoOperator(const Token &tok)
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
                            Trigger("Divison by 0"_ss);
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
                        res = stack.peek(1).Value() != RealNumber::RN_0;
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
RealNumber LittleEngine::CalcPostfix(TokenVec& tvPostfix)
{
    unsigned stack_cnt = stack.size();
    TokenVec::iterator it;
    for(it = tvPostfix.begin(); it != tvPostfix.end(); ++it)
    {
        switch(it->Type() )
        {
            case tknCharacter:
            case tknNumber:    stack.push(*it); break;
            case tknVariable:  DoVariable(*it); break;
            case tknFunction:  DoFunction(*it); break;
            case tknOperator:  DoOperator(*it); break;
            default: break;
        }
    }
    if(stack.empty() || stack.size() > stack_cnt + 1 || (stack.peek().Type() != tknNumber && stack.peek().Type() != tknCharacter))
        Trigger("Expression error"_ss);
    Token tok( stack.peek() );
    stack.pop(1);

    resultType = ResultType::rtNumber;
    return calcResult = tok.Value().RoundedToDigits(RealNumber::MaxLength()+1);
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
        if(!InfixToPostFix(infix) ) // then variable or function definition
        {
            resultType = ResultType::rtDefinition;
            return calcResult;
        }

        return CalcPostfix(tvPostfix);
    }
    catch(...)
    {
      ;  // TO DO : error handling
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
// valamiert "bad file descriptor" van ha a RAD STUDIO XE-vel forditom
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
bool LittleEngine::SaveTables(const SmartString &name) // if it wasn't read and no name is given it won't be saved
{
    if(clean && name.empty())  // only save if it wasn't saved
        return true;    // already to this file

    if(!name.empty() && name_variable_table.empty()) // no save requested
        return true;
    if(clean && name.empty() && name_variable_table != name) // same name
        return true;

    SmartString filename = name;
    if (filename.empty())
        filename = name_variable_table;

#if defined(__BORLANDC__)
    FILE *f = _wfopen(name.ToWideString(), "wt"_ss);
    if(!f)
        return false;
    string s = VERSION_STRING;
    fputs( ( s + "\nVariables\n"_ss).c_str(), f);
    SmartString ws;
    int siz = 1023; // initial size of buffer
    char *buf = new char[1024]; // buffer for wcstombs conversion

    if(variables.size() )
    {
        for(mit = variables.begin(); mit != variables.end(); ++mit)
            if(!mit->second.builtin)
            {
                ws = mit->first+"="+mit->second.body;
                if(!mit->second.description.empty() )
                    ws+= comment_delimiter +mit->second.description;
                ws += "\n"_ss;
                if(ws.length() > siz)
                {
                        delete buf;
                        buf = new char[ (siz = 2*ws.length())+1];
                }
                wcstombs(buf, ws.c_str(), 1023);
                fputs(buf,f);
            }
    }
    fputs("Functions\n",f);
    if(functions.size() )
    {
        for(mit = functions.begin(); mit != functions.end(); ++mit)
        {
            if(mit->second.builtin)
               continue;
            ws = mit->first+"("_ss;

            if(!mit->second.args.empty())
            {
                vector<SmartString>::const_iterator vit;
                vit = mit->second.args.begin();
                ws += (*vit);
                ++vit;
                for( ; vit != mit->second.args.end(); ++vit)
                    ws += ","+(*vit);
            }
            ws += "_ss)="+mit->second.body;
            if(!mit->second.description.empty() )
                ws += comment_delimiter + mit->second.description;
            ws += "\n"_ss;
            if(ws.length() > siz)
            {
                        delete buf;
                        buf = new char[ (siz = 2*ws.length())+1];
            }
            wcstombs(buf, ws.c_str(), 1023);
            fputs(buf,f);
        }
    }
    delete [] buf;
    fclose(f);
#else
    std::wofstream ofs;
	ofs.open(SmartString(filename).ToWideString(), ios_base::out);
	if( ofs.fail() )
			return false;
	ofs << VERSION_STRING << L"\nVariables\n";
	if(variables.size() )                       // these are the user defined variables only
	{
        VariableTable::iterator vit;
		for(vit = variables.begin(); vit != variables.end(); ++vit)
			//if(!vit->second.pValue->builtin)
			//{
				ofs << vit->first.ToWideString() << L"=" << vit->second.body.ToWideString();
				if(!vit->second.data.desc.empty() )
					ofs << comment_delimiter << vit->second.data.desc.ToWideString();
				ofs << endl;
			//}
	}
	ofs << ("Functions\n"_ss).ToWideString();
	if(functions.size() )
	{
        FunctionTable::iterator fit;
		for(fit = functions.begin(); fit != functions.end(); ++fit)
		{
			if(fit->second.builtin)
				continue;
			ofs << fit->first.ToWideString() << ("("_ss).ToWideString();
			if(!fit->second.args.empty())
			{
				vector<SmartString>::const_iterator vit;
				vit = fit->second.args.begin();
				ofs << (*vit).ToWideString();
				++vit;
				for( ; vit != fit->second.args.end(); ++vit)
					ofs << ", " << (*vit).ToWideString();
			}
			ofs << " = " << fit->second.body.ToWideString();
			if(!fit->second.desc.empty() )
				ofs << comment_delimiter << fit->second.desc.ToWideString();
			ofs << endl;
		}
	}
#endif
    clean = true;
    return true;
}
/*========================================================
 * TASK: Read user functions and variables from a file
 * EXPECTS: file name
 * RETURNS: true for success or false for error
 *-----------------------------------------------------------*/
bool LittleEngine::ReadTables(const  SmartString &name)
{
    name_variable_table = name;
#if defined(__BORLANDC__)
// valamiert "bad file descriptor" van ha a RAD STUDIO XE-vel forditom
    FILE *f = _wfopen(name,"rt"_ss);
    if(!f)
        return false;
    char buf[1024];
    buf[1023]=0;
    fgets(buf, 1023,f);
    if(strncmp(buf, VERSION_STRING, strlen(VERSION_STRING)) )
    {
        fclose(f);
        return false;
    }
    wchar_t wbuf[1024];
    while( fgets(buf, 1023,f) )
    {
        if(strcmp(buf, "Variables\n"_ss) && strcmp(buf, "Functions\n"_ss) )
*        {
            mbstowcs(wbuf, buf, 1923);
            InfixToPostFix(SmartString(wbuf).erase(strlen(buf)-1) );
        }
    }
    fclose(f);
#else
        std::wifstream in(SmartString(name).ToWideString(), ios_base::in);
		if(in.fail() )
			return false;

		std::wstring line;
		std::getline(in, line);
		if(line != L"FalconCalc V1.0" )
			return false;

		while(std::getline(in, line) )
		{
			if( (line != L"Variables") && line != L"Functions")
				InfixToPostFix(SmartString(line) );
		}
#endif
    return clean = true;
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsDecString()
{
    displayFormat.expFormat = (beautification == Beautification::bmoNone ?
        ExpFormat::rnsfE :
        (beautification == Beautification::bmoGraphText ?
        ExpFormat::rnsfGraph :
        (beautification == Beautification::bmoHtml ?
        ExpFormat::rnsfSciHTML :
        ExpFormat::rnsfSciTeX)) );
    DisplayFormat fmt = displayFormat;
    fmt.base = DisplayBase::rnb10;
    return calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsHexString() const
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
SmartString LittleEngine::ResultAsOctString() const
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
SmartString LittleEngine::ResultAsBinString() const
{
    DisplayFormat fmt = displayFormat;
    fmt.base = DisplayBase::rnbBin;
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
SmartString LittleEngine::ResultAsCharString() const
{
    DisplayFormat fmt;
    fmt.base = DisplayBase::rnbHex;
    fmt.useNumberPrefix = false;
	SmartString s = calcResult.Int().ToHexString(fmt); 
    if (s.length() & 1)  // odd?
        s = "0"_ss + s;
    size_t i = 0, j = 0;
    while (j < s.length()) // s.length() is always even
    {
        // DEBUG
        char16_t ch16a = (s[j] & 0x0F) << 4,
                 ch16b = s[j+1] & 0x0F;
        // /DEBUG
        s[i] = ((s[j] & 0x0F) << 4) + (s[j + 1] & 0x0F);
        j += 2;
        s[++i] = 0;
    }
    s.erase(i);
    return s;
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
void LittleEngine::GetVarFuncInfo(VARFUNC_INFO &vf)
{
    vf.uBuiltinFuncCnt = numBuiltinFuncs;
    vf.sBuiltinFuncs   = GetFunctions(true);
    vf.uBuiltinVarCnt  = numBuiltinVars;
    vf.sBuiltinVars    = GetVariables(true);

    vf.uUserFuncCnt    = functions.size() - numBuiltinFuncs;
    vf.sUserFuncs      = GetFunctions(false);
    vf.uUserVarCnt     = variables.size() - numBuiltinVars;
    vf.sUserVars       = GetVariables(false);

    vf.pOwner          = this;
}

/*========================================================
 * TASK:    Create a display string for all constants 
 *          (i.e. bultins) and variables in 
 *          lines (separated by '\n') in format
 *              name = value:[unit:]description
 * EXPECTS: flag for what to show
 * RETURNS: single SmartString containing variables in lines
 *-----------------------------------------------------------*/
SmartString LittleEngine::GetVariables(bool builtin) const
{
    SmartString sres;

    if (builtin)
    {
        for (auto& it : constantsMap)
        {
            sres += it.first + "="_ss + it.second->value.ToString();
            if(!it.second->unit.empty()) 
                sres += SmartString(comment_delimiter) + it.second->unit + "\n"_ss;
            sres += SmartString(comment_delimiter) + it.second->desc+ "\n"_ss;
        }
    }
    else
    {
        for (auto& it : variables)
                sres += it.first + "="_ss + it.second.body + SmartString(comment_delimiter) + it.second.data.desc + "\n"_ss;
    }
    return sres;
}

/*========================================================
 * TASK:    Create a display string for all functions
 * EXPECTS: builtin: true: list of builtins
 *                   false: list of user defined functions
 * RETURNS: single SmartString containing functions in lines
 * REMARKS: - line format:
 *              
 *-----------------------------------------------------------*/
SmartString LittleEngine::GetFunctions(bool builtin) const
{
    SmartString sres;

    FunctionTable::const_iterator it;
    for(it = functions.begin(); it != functions.end(); ++it)
        if(it->second.builtin == builtin)
        {
            sres += it->first;
            if(!builtin)
                sres += "("_ss;
            for(unsigned j = 0; j < it->second.args.size(); ++j)
            {
                if(j)
                    sres += SmartString(_ArgSeparator()) + SmartString(" "_ss);
                sres += it->second.args[j];
            }
            if(!builtin)
                sres += "_ss)"_ss;
            sres += "="_ss + it->second.body + SmartString(comment_delimiter) + it->second.desc + "\n"_ss;
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
    angleUnit   = src.angleUnit;   // used when calculating sine, cosine, tangent, cotangent
    infix       = src.infix;
    tvPostfix    = src.tvPostfix;
    calcResult  = src.calcResult;
    //variables   = src.variables;      static variables now
    //functions   = src.functions;
    name_variable_table = src.name_variable_table;
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
    vector<SmartString> names;
    switch(what)
    {
        case 0:
				// erase existing user variables
				// BUG: if a variable name was edited it is not erased and remains
        case 1: for(vit = variables.begin(); vit != variables.end(); ++vit)
                    names.push_back(vit->first);
                for(unsigned i = 0; i < names.size(); ++i)
                    variables.erase(names[i]);
                if(what == 1) // for 0: fall through
                    break;
				// erase existing user functions
				// BUG: if a function name was edited it is not erased and remains
        case 2: for(fit = functions.begin(); fit != functions.end(); ++fit)
                    if(!fit->second.builtin)
                        names.push_back(fit->first);
                for(unsigned i = 0; i < names.size(); ++i)
                    functions.erase(names[i]);
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
           ip.InfixToPostFix(def.substr(st,en-st) );
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

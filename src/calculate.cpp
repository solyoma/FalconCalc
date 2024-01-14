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


/*==============================================================
 * NAMESPACE littlecalc
 *-------------------------------------------------------------*/
namespace littlecalc {
// originally this was a 'const SmartString', but it became(?) empty in
// SaveTables() under RAD Studio XE
#define VERSION_STRING "FalconCalc V2.0"


   const SCharT comment_delimiter = ':'_ss;


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
 * REMARKS: if modified must check/modify 'LittleEngine::Convert()'
 *---------------------------------------------------------*/
void MathOperator::Setup()
{
	if(bOpsOk)
		return;

	ops["*invalid*"_ss].oper       = opINVALID;

	ops["("_ss].oper       = opOpenBrace;
	ops["("_ss].precedence          = -1; // special case  !

	ops["_ss)"_ss].oper       = opCloseBrace;
	ops["_ss)"_ss].precedence          = -1;

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
		 cn = SCharT(pos == text.length() ? 0 : text[pos] );
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
				if (isdigit(cn) || cn == '#')		// if decimal, octal, hexadecimal or binary number
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
 *       and either one of the [decimal digit(s)] must be present
 * EXPECTS: 'text' is lowercase, starts with number or
 *        [decimal point] (no unary + or -)
 *        and if 'text' starts with decimal point it contains at
 *        least one additional digit
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
    RealNumber lval = zero;
	while(pos < text.length() && (text[pos] != '\'' || (pos > 0 && text[pos-1] == '\\')) )
    {
		SCharT ch = text[pos];     // sizeof(SCharT) == 1
        lval = (lval * 256.0) + ch;
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
    while(pos < text.length() && isspace(text[pos],loc))
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

/*===================================================
 * TASK: construct a token from text
 * EXPECTS: 'text' SmartString
 *          'pos': position in 'text' where the tex of the token starts
 *           text to only contain valid characters and only lowercase letters!
 *--------------------------------------------------*/
Token::Token( const SmartString &text, unsigned &pos) : type(tknUnknown), val(0.0l)
{
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

	while(pos < len && isspace(text[pos], loc))
		++pos;
	SCharT  c = text[pos++],
			cn = (pos >= len ? SCharT(0) : text[pos]); // look ahead
    --pos;  // go back to start of number

    if(c == '\'')   // character SmartString
        GetNumberFromQuotedString(text,pos);
	else if(isdigit(c,loc) || c == decpoint || c == '#')		// token is a decimal, hexadecimal, octal or binary number
	{
		bool bDecpF = (c == decpoint),							// decimal point found ?
				bHexF = (c == '0' && (cn == 'x')),				// hex number?
				bOctF = (c == '0' && cn && isdigit(cn,loc)),
				bBinF = (c == '#');

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
			GetDecimalNumber(text, --pos); // start at the first number/decimal point
		}
	}
	else if(isalpha(c,loc)) // variable, function or textd operator (e.g. 'or')
		GetVarOrFuncOrOperator(text,--pos);
	else // a possible operator character
		GetOperator(text, --pos);
}


/*==========================================
Class LittleEngine
 *-----------------------------------------*/

static VarFuncTable builtinVars,
                    builtinFuncs;
unsigned LittleEngine::numBuiltinVars =0,
       LittleEngine::numBuiltinFuncs = 0;

bool LittleEngine::builtinsOk=false;

inline RealNumber Sign(RealNumber r) { return r.Sign() > 0 ? one:-one; }

/*========================================================
 * TASK: Creates a single instance of the calculator
 *       if the built ins are not yet set up sets them up too
 * EXPECTS:all builtins 's name is lowercase!
 * RETURNS: nothing/object created
 *-----------------------------------------------------------*/
LittleEngine::LittleEngine() : clean(true)
{
#define SET_BUILTIN_VAR(a,b,c) v.description = u8#b; v.value = (c); builtinVars[#a##_ss] = v;
    #define SET_BUILTIN_CONST(a) v.description = (a).desc; v.value=(a).value; builtinVars[(a).name] = v;

    if(!builtinsOk)
    {
       VarFunc v;
       v.builtin = true;
       v.isnumber = true;
       v.dirty = false;

       SET_BUILTIN_VAR(ans, result of previous calculation, zero);


	   SET_BUILTIN_VAR(e, base of the natural logarithm, e)
       SET_BUILTIN_VAR(log2e, base 2 logarithm of e, log2(e));
       SET_BUILTIN_VAR(log10e, base 10 logarithm of e, log10(e));
       SET_BUILTIN_VAR(lge, base 10 logarithm of e, log10(e));
       SET_BUILTIN_VAR(ln2,natural logarithm of 2, ln2);
       SET_BUILTIN_VAR(pi, pi, pi);
       SET_BUILTIN_VAR(piP2, pi/2, piP2);
       SET_BUILTIN_VAR(piP4, pi/4, half*piP2);
       SET_BUILTIN_VAR(rpi2, 2/pi, two/pi);
       SET_BUILTIN_VAR(rpi, 1/ π, one/pi);
       SET_BUILTIN_VAR(sqpi, square root of π, sqrt(pi));
       SET_BUILTIN_VAR(sqrt2, square root of 2, sqrt2);
       SET_BUILTIN_VAR(rsqrt2, reciprocal of the square root of 2, rsqrt2);

       SET_BUILTIN_CONST(fsc);
       SET_BUILTIN_CONST(au);
       SET_BUILTIN_CONST(c);
       SET_BUILTIN_CONST(eps0);
       SET_BUILTIN_CONST(G);
       SET_BUILTIN_CONST(gf);
       SET_BUILTIN_CONST(h);
       SET_BUILTIN_CONST(hbar);
       SET_BUILTIN_CONST(kb);
       SET_BUILTIN_CONST(kc);
       SET_BUILTIN_CONST(la);
       SET_BUILTIN_CONST(me);
       SET_BUILTIN_CONST(mf);
       SET_BUILTIN_CONST(mp);
       SET_BUILTIN_CONST(ms);
       SET_BUILTIN_CONST(mu0);
       SET_BUILTIN_CONST(qe);
       SET_BUILTIN_CONST(rfsc);
       SET_BUILTIN_CONST(rf);
       SET_BUILTIN_CONST(rg);
       SET_BUILTIN_CONST(rs);
       SET_BUILTIN_CONST(sb);
       SET_BUILTIN_CONST(u);


       numBuiltinVars = builtinVars.size();
            // all built in function requires a single RealNumber argument
            // they are not 'dirty' and they are 'isnumber's
            v.value = 0.0l;

	#define SET_BUILTIN_FUNC1(a,b,c) v.description = u8#b; v.function.funct1 = c; builtinFuncs[#a##_ss] = v;
	#define SET_BUILTIN_FUNC2(a,b,c) v.description = u8#b; v.function.funct2 = c; builtinFuncs[#a##_ss] = v;
	#define SET_BUILTIN_FUNC3(a,b,c) v.description = u8#b; v.function.funct3 = c; builtinFuncs[#a##_ss] = v;
	#define SET_BUILTIN_FUNC4(a,b,c) v.description = u8#b; v.function.funct4 = c; builtinFuncs[#a##_ss] = v;
       SET_BUILTIN_FUNC1(abs, absolute value, abs);

	   v.useAngleUnitAsResult=true;
       SET_BUILTIN_FUNC4(arcsin, inverse of sine, asin);
       SET_BUILTIN_FUNC4(asin, inverse of sine, asin);
       SET_BUILTIN_FUNC4(arccos, inverse of cosine, acos);
       SET_BUILTIN_FUNC4(acos, inverse of cosine, acos);
       SET_BUILTIN_FUNC4(arctan, inverse of tangent, atan);
       SET_BUILTIN_FUNC4(atan, inverse of tangent, atan);
       v.useAngleUnitAsResult=false;

       v.useAngleUnit        =true;
       SET_BUILTIN_FUNC4(sin, sine, sin);
       SET_BUILTIN_FUNC4(cos, cosine, cos);
       SET_BUILTIN_FUNC4(tan, tangent, tan);
       SET_BUILTIN_FUNC4(tg, tangent, tan);
       v.useAngleUnit        =false;

       SET_BUILTIN_FUNC1(asinh, inverse of hyperbolic sine,     asinh);
       SET_BUILTIN_FUNC1(acosh, inverse of hyperbolic cosine,   acosh);
       SET_BUILTIN_FUNC1(atanh, inverse of hyperbolic tangent,  atanh);
       SET_BUILTIN_FUNC1(acoth, inverse of hyperbolic cotangent,acoth);
	   SET_BUILTIN_FUNC1(sinh,hyperbolic sine, sinh);

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
       SET_BUILTIN_FUNC3(sqrt, square root, sqrt);
       SET_BUILTIN_FUNC1(tanh, tangent, tanh);
       SET_BUILTIN_FUNC1(trunc, truncate to integer, floor);

       numBuiltinFuncs = builtinFuncs.size();

       builtinsOk = true;
    }
    variables = builtinVars;
    functions = builtinFuncs;
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
			stack.popto(tvOutput);
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
            stack.popto(tvOutput);			// pop 'op2' off the stack, onto the output queue;
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
            stack.popto(tvOutput);
            tk = stack.peek().Type();
        }
        if(!stack.empty())
        {
            if(tk == tknFunction)
                stack.popto(tvOutput);              // If the token at the top of the stack is a function token, pop it onto the output queue.
            else
                stack.pop();					   // Pop the left parenthesis from the stack, but not onto the output queue.
        }
        else                                       // If the stack runs out without finding a left parenthesis, then there are mismatched parenthesis.
            Trigger("Mismatched parenthesis"_ss);
    }
}

/*==================================================
 * TASK: convert single infix expression in 'expr'
 *       to postfix expression in 'tvOutput'
 * EXPECTS: 'expr' SmartString may end with '\n' and may contain
 *       variable/function definitions with comments at the
 *       end. Comments are separated from the definition by
 *       'comment_delimiter' which must be different from
 *       any characters allowed in an expression.
 *-------------------------------------------------*/
int LittleEngine::Convert(const SmartString &expr)
{
    //check for (invalid characters up to the comment field
	locale loc = cout.getloc();
	infix = expr;
    if(infix[ infix.length() -1] == '\n')
        infix = infix.substr(0, infix.length()-1);
	SmartString pattern = "=*^/<>!&|~%().,+-_#'"_ss;
    bool quoted = false;

    for(SmartString::iterator it = infix.begin(); it != infix.end() && *it != littlecalc::comment_delimiter; ++it)
    {
        if(!quoted && !isalnum(*it, loc) && !isspace(*it,loc) && pattern.find_first_of(*it) == std::string::npos )
    	    Trigger (SmartString("Illegal character '"_ss) +  (SCharT)(*it) + "'"_ss);

        if(*it == '\'')
            quoted ^= true;
        else if(!quoted)
            *it = tolower(*it, loc);
    }

    int result = 1;     // not an assignment
    tvOutput.clear();   // get rid of previous result

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
								tvOutput.push_back(*tok);
                                needOp = true;
                                break;
            case tknVariable:	  						// If the token is a variable check for assignments
                                if(VariableAssignment(infix, pos, tok) == 0 )   // handle assignment
                                {
    								tvOutput.push_back(*tok); // otherwise add it to the output queue.
                                    needOp = true;
                                }
                                else
                                    result = 0;
                                break;
			case tknFunction:							// If the token is a function name token,
                                if(!FunctionAssignment(infix, pos, tok) )
								    stack.push(*tok);   // then push it onto the stack.
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
        delete tok;
		tok = new Token(infix, pos);
	}
    delete tok;
		// When there are no more tokens to read:

	while( !stack.empty())				// While there are still operator tokens in the stack:
	{
		if(stack.peek().Type() == tknBrace )
			Trigger("Mismatched parenthesis"_ss);
		stack.popto(tvOutput);
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
    VarFuncTable::iterator it;
    for(it = variables.begin(); it != variables.end(); ++it)
    {
        if(it->second.isnumber) // numbers do not change (and builtins are 'numbers')
            continue;
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
 * EXPECTS: 'expr' text of line
 *          'pos' start positiom after name  - a line may only contain a
 *                single variable definition,
 *          'tok' actual variable
 * REURNS: true if this is an assignment and variable definition or
 *         redefinition is stored in VARIABLES
 *         false if next sign is not an equal sign
 *---------------------------------------------------------*/
bool LittleEngine::VariableAssignment(const SmartString &expr, unsigned &pos, Token *tok)
{
	locale loc = cout.getloc();
    while(pos < expr.length() && isspace(expr[pos], loc))
      ++pos;
    if( pos == expr.length()  || expr[pos] != '=')
        return false;       // not an assignment
    VarFunc v;
    if(variables.count(tok->Text() )) // already defined
    {
        if(variables[ tok->Text()].builtin)
            Trigger("Builtin variables cannot be redefined"_ss);
        else
            v.value = variables[ tok->Text()].value; // v = variables[ tok->Text()];
    }
    // assignment: create or modify variable
    ++pos; // skip '='

    unsigned posComment = expr.find_first_of(comment_delimiter,pos);
    v.body = expr.substr(pos, posComment-pos);
    pos += v.body.length();
    if(posComment != SmartString::npos)
    {
        pos = posComment+1;     // ++pos would be enough: we have a single expression in line
        v.description = expr.substr(pos);
        pos += v.description.length();
    }

    LittleEngine if2pf(*this);
    if2pf.Convert(v.body);
    v.definition = if2pf.tvOutput;
    if(v.definition.size() == 1)    // maybe a constant
    {
        if(v.definition[0].Type() == tknNumber )
        {
            v.value = v.definition[0].Value();
            v.isnumber = true;// numbers do not change when other variables^functions change
            v.dirty = false;
        }
        else
            v.value = if2pf.CalcPostfix(if2pf.tvOutput);
    }
    else // leave it dirty :)
            v.value = if2pf.CalcPostfix(if2pf.tvOutput);

    variables[ tok->Text()] = v;
    // mark variables whose definition contains this variable dirty
    MarkDirty(tok->Text());
    clean = false;  // table modified

    _calcResult = v.value;
    return true;    // assignment
}

/* =========================================================
 * TASK: test expression for function assignment.
 * EXPECTS: 'expr' text of line may contain whitespaces which are
 *                 skipped
 *          'pos' positiom after the brace  - a line may only contain a
 *                single function definition,
 *          'tok' actual function
 *          function assignments must have the form :
 *           name([arg1,...), where arg1, etc are valid variable names
 *          (whitespaces may be present between the
 *            arguments and commas and the closing brace)
 * REURNS: true if this is an assignment and function definition or
 *         redefinition is stored in FUNCTIONS
 *         false if this is not a function assignment,
 *         throws exception if syntax error
 *---------------------------------------------------------*/
bool LittleEngine::FunctionAssignment(const SmartString &expr, unsigned &pos, Token *tok)
{
   _calcResult = zero;
   unsigned poseq = expr.find_first_of('='_ss,pos);
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
   VarFunc v;

   while( pos < bpos && isspace(expr[pos], loc))
        ++pos;
   while(pos < bpos ) // get arguments
   {        // order of arguments: left to right
        unsigned n = pos;
//#if defined QTSA_PROJECT
//        while(n < bpos && (expr[n].isLetterOrNumber() ) || expr[n] == '_')) // get word
//            ++n;
//#else
        while(n < bpos && (isalnum(expr[n]) || expr[n] == '_')) // get word
            ++n;
//#endif
        v.args.push_back(expr.substr(pos, n - pos) );   // store argument name
        while( n < bpos && isspace(expr[n], loc))
            ++n;
        if(n < bpos && expr[n] != _ArgSeparator())
            Trigger("Invalid character in function definition"_ss);
        ++n;    // skip ',' or to bpos
        while( n < bpos && isspace(expr[n], loc))
            ++n;
          pos = n;
   }
   pos = poseq+1; // after the equal sign
        // get comment
   unsigned posComment = expr.find_first_of(comment_delimiter,pos);
   while(pos < expr.length() && pos < posComment && isspace(expr[pos],loc))
    ++pos;
   v.body = expr.substr(pos, posComment-pos);
                              // function body. may contain arguments,
                              // those must be marked
   pos += v.body.length();
   if(posComment != SmartString::npos)
   {
        v.description = expr.substr(posComment+1);
        pos += v.description.length()+1; // including the delimiter ':'
   }

   LittleEngine if2pf(*this);
   try
   {
       if2pf.Convert(v.body);
       v.definition = if2pf.tvOutput;
       functions[ tok->Text()] = v;
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
    if(variables.count(tok.Text()) )   // existing variable
    {
        VarFunc &var = variables[tok.Text()];
        if(var.dirty && !var.being_processed)
        {
            try
            {
                var.being_processed = true;
                var.value = CalcPostfix(var.definition);
                var.being_processed = false;
                var.dirty = false;
            }
            catch(...)
            {
                var.being_processed = false;
                var.dirty = false;
            }
        }
        stack.push(var.value);
    }
    else
        stack.push(zero);
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

    VarFunc &f = functions[tok.Text()];
    if(f.being_processed) // then recursive call
        Trigger("Recursive functions are not allowed"_ss);
    RealNumber v;
    if(f.builtin) // single argument on stack
    {
        v = stack.peek(1).Value();
        stack.pop(1);
        if(f.useAngleUnit)
        {
            switch(aU)
            {
                case AngularUnit::auDeg: v = v /180.0  * pi; break;
                case AngularUnit::auGrad: v = v /200.0 * pi; break;
                case AngularUnit::auRad: break;
            }
        }
        v = f.function(v);
        if(f.useAngleUnitAsResult)
        {
            switch(aU)
            {
                case AngularUnit::auDeg: v = v * 180.0 /  pi; break;
                case AngularUnit::auGrad: v = v * 200.0 / pi; break;
                case AngularUnit::auRad: break;
            }
        }
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
    RealNumber res = zero;

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
            case opEQ:  res = (stack.peek(2).Value() == stack.peek(1).Value() ? one : zero);
                        stack.pop(2);
                        break;
            case opNEQ:  // '!='
                        res = (stack.peek(2).Value() != stack.peek(1).Value() ? one:zero);
                        stack.pop(2);
                        break;
            case opLT:   // '<'
                        res = (stack.peek(2).Value() < stack.peek(1).Value() ? one : zero);
                        stack.pop(2);
                        break;
            case opLE:   // '<='
                        res = (stack.peek(2).Value() <= stack.peek(1).Value() ? one : zero);
                        stack.pop(2);
                        break;
            case opGT:   // '>'
                        res = (stack.peek(2).Value() > stack.peek(1).Value() ? one : zero);;
                        stack.pop(2);
                        break;
            case opGE:   // '>='
                        res = (stack.peek(2).Value() >= stack.peek(1).Value() ? one : zero);
                        stack.pop(2);
                        break;
            case opSHL:  // '<<'    shift as binary
                        res =  stack.peek(2).Value() * two.Pow(stack.peek(1).Value().Int());
                        stack.pop(2);
                        break;
            case opSHR:  // '>>'    shift as binary
                        res =  stack.peek(2).Value() / two.Pow(stack.peek(1).Value());
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
            case opDIV: if(stack.peek(1).Value() == 0.0)
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
                        res = !stack.peek(1).Value();
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
 * TASK: calculate postfix expression in 'tvOutput'
 *       Because it can be called recursively it must
 *       remember the stcak position on entry
 *-------------------------------------------------*/
RealNumber LittleEngine::CalcPostfix(TokenVec& tvOutput)
{
    unsigned stack_cnt = stack.size();
    TokenVec::iterator it;
    for(it = tvOutput.begin(); it != tvOutput.end(); ++it)
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

    return _calcResult = tok.Value();
}
/*==================================================
 * TASK: calculate expression
 * EXPECTS: 'au' (angle units) is the last used.
 *			If it was not explicitely set the
 *           defult is auRad (radian)
 * RETURNS: calculated value or exception thrown
 *-------------------------------------------------*/
RealNumber LittleEngine::Calculate(const SmartString &expr)
{
    try
    {
        if(!Convert(expr) ) // then variable or function definition
        {
            _resultType = ResultType::rtDefinition;
            return _calcResult;
        }
        return CalcPostfix(tvOutput);
    }
    catch(...)
    {
      ;  // TO DO : error handling
      _resultType = ResultType::rtInvalid;
	  stack.clear();
	  throw;	//re-throw if not handled
    }
    return zero;
}
/*========================================================
 * TASK:calculate expression using given angular units
 * EXPECTS: ci contains expression SmartString and angla unit
 * RETURNS: calculated value or exception thrown
 *-----------------------------------------------------------*/
RealNumber LittleEngine::Calculate(const CALC_INPUT &ci)
{
    aU = ci.angu;
    return Calculate(ci.infix);
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::Postfix() const
{
    SmartString postfix;
    for(unsigned i = 0; i < tvOutput.size(); ++i)
        postfix += tvOutput[i].Text() + " "_ss;
    return postfix;
}
#if defined(__BORLANDC__)
// valamiert "bad file descriptor" van ha a RAD STUDIO XE-vel forditom
     #include <stdio.h>
#endif
/*========================================================
 * TASK: Write modified user functions and variables table to file
 * EXPECTS: 'name' is either the name of the file or NULL
 *          When it is NULL the same name is used as in 'ReadTables'
 *          if that was called before, otherwise no write occurs
 *          'clean' when true the tables are modified
 * RETURNS: true: if there was no need to save the data or the save was
 *          successful, false otherwise
 *-----------------------------------------------------------*/
bool LittleEngine::SaveTables(const SCharT *name) // if it wasn't read and no name is given it won't be saved
{
    if(clean && !name)  // only save if it wasn't saved
        return true;    // already to this file

    if(!name && name_variable_table.empty()) // no save requested
        return true;
    if(clean && name && name_variable_table != SmartString(name) ) // same name
        return true;

    if(name == 0)
        name = name_variable_table.c_str();

    VarFuncTable::iterator mit;
#if defined(__BORLANDC__)
    FILE *f = _wfopen(name, "wt"_ss);
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
	ofs.open(SmartString(name).ToWideString(), ios_base::out);
	if( ofs.fail() )
			return false;
	ofs << VERSION_STRING << ("\nVariables\n"_ss).ToWideString();
	if(variables.size() )
	{
		for(mit = variables.begin(); mit != variables.end(); ++mit)
			if(!mit->second.builtin)
			{
				ofs << mit->first.ToWideString() << L"=" << mit->second.body.ToWideString();
				if(!mit->second.description.empty() )
					ofs << comment_delimiter << mit->second.description.ToWideString();
				ofs << endl;
			}
	}
	ofs << ("Functions\n"_ss).ToWideString();
	if(functions.size() )
	{
		for(mit = functions.begin(); mit != functions.end(); ++mit)
		{
			if(mit->second.builtin)
				continue;
			ofs << mit->first.ToWideString() << ("("_ss).ToWideString();
			if(!mit->second.args.empty())
			{
				vector<SmartString>::const_iterator vit;
				vit = mit->second.args.begin();
				ofs << (*vit).ToWideString();
				++vit;
				for( ; vit != mit->second.args.end(); ++vit)
					ofs << ", " << (*vit).ToWideString();
			}
			ofs << " = " << mit->second.body.ToWideString();
			if(!mit->second.description.empty() )
				ofs << comment_delimiter << mit->second.description.ToWideString();
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
bool LittleEngine::ReadTables(const SCharT *name)
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
        {
            mbstowcs(wbuf, buf, 1923);
            Convert(SmartString(wbuf).erase(strlen(buf)-1) );
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
				Convert(SmartString(line) );
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
    DisplayFormat fmt = _displayFormat;
    fmt.base = DisplayBase::rnb10;
    return _calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsHexString()
{
    DisplayFormat fmt = _displayFormat;
    fmt.base = DisplayBase::rnbHex;
    return _calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsOctString()
{
    DisplayFormat fmt = _displayFormat;
    fmt.base = DisplayBase::rnbOct;
    return _calcResult.ToString(fmt);
}
/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::ResultAsBinString()
{
    DisplayFormat fmt = _displayFormat;
    fmt.base = DisplayBase::rnbBin;
    return _calcResult.ToString(fmt);
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
    DisplayFormat fmt;
    fmt.base = DisplayBase::rnbHex;
	SmartString s = _calcResult.Int().ToHexString(fmt); 
    size_t i = 0, j = 0;
    while (j < s.length()) // s.length() is always even
    {
        s[i] = ((s[j] & 0x0F) << 8) + (s[j + 1] & 0x0F);
        j += 2;
        ++i;
    }
    s.erase(j);
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
 * TASK:    get a display string for variable in the format
 *          name = value : description
 * EXPECTS: // if the variable is a builtin one
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::GetVariables(bool builtin) const
{
    SmartString sres;

    VarFuncTable::const_iterator it;
    SmartString body;
    for(auto &it :variables)
        if(it.second.builtin == builtin)
        {
            if(builtin) // then no body only value
            {
				//wchar_t buf[128];
				//swprintf(buf, 128, L"%-.6E", it.second.value);
				//body = wstring(buf);
                DisplayFormat fmt;
                fmt.decDigits = 6;
                body = it.second.value.ToString();
            }
            else
                body = it.second.body;
            sres += it.first + "="_ss + body + comment_delimiter + it.second.description + "\n"_ss;
        }

    return sres;
}

/*========================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 *-----------------------------------------------------------*/
SmartString LittleEngine::GetFunctions(bool builtin) const
{
    SmartString sres;

    VarFuncTable::const_iterator it;
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
            sres += "="_ss + it->second.body + comment_delimiter + it->second.description + "\n"_ss;
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
    aU          = src.aU;   // used when calculating sine, cosine, tangent, cotangent
    infix       = src.infix;
    tvOutput    = src.tvOutput;
    _calcResult = src._calcResult;
    variables   = src.variables;
    functions   = src.functions;
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
    VarFuncTable::iterator it;
    vector<SmartString> names;
    switch(what)
    {
        case 0:
				// erase existing user variables
				// BUG: if a variable name was edited it is not erased and remains
        case 1: for(it = variables.begin(); it != variables.end(); ++it)
                    if(!it->second.builtin)
                        names.push_back(it->first);
                for(unsigned i = 0; i < names.size(); ++i)
                    variables.erase(names[i]);
                if(what == 1) // for 0: fall through
                    break;
				// erase existing user functions
				// BUG: if a function name was edited it is not erased and remains
        case 2: for(it = functions.begin(); it != functions.end(); ++it)
                    if(!it->second.builtin)
                        names.push_back(it->first);
                for(unsigned i = 0; i < names.size(); ++i)
                    functions.erase(names[i]);
        default:
                break;
    }
        // add new user variables or functions
    unsigned st = 0, en;  // start and end positions
    while(st < def.length() )
    {
        en = def.find_first_of('\n'_ss, st);
        try
        {
           ip.Convert(def.substr(st,en-st) );
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


	// end of namespace littlecalc
}

#ifndef calculateH
 #define calculateH

#ifndef __cplusplus
#error use a c++ compiler
#endif

#include "SmartString.h"
#include "LongNumber.h"

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <map>
#include <string>
#include <vector>
#include <locale>

#include <cmath>

//#include "stdafx_zoli.h"
#include "stdafx_lc.h"

//#include "defines.h"

//using namespace SmString;
//using namespace LongNumber;

using namespace std;
using namespace SmString;
using namespace LongNumber;

namespace FalconCalc
{
    extern const SCharT schCommentDelimiter;
    extern const SmartString ssCommentDelimiterString;

	enum TokenType {tknEOL,		    // end of line
					tknUnknown,		// e.g. ','
		            tknErr,			// error: 'name' contains error message
                    tknBrace,       // ')' or '('
		            tknNumber,      // a decimal, hexadecimal, octal or binary number (2, 2.12, 0xAF00, 01237, #010011001)
                    tknCharacter,   // a single character or a character SmartString enclosed in single quotes
					tknVariable,
					tknFunction,
					tknOperator};
	enum OperatorType   // l - left operand, r - right operand
	{
		opINVALID,
		opOR,   // l | r 
		opXOR,  // l ^ r
		opAND,  // l & r  
		opEQ,   // 'l == r'
		opNEQ,  // 'l != r'
		opLT,   // 'l < r'
		opLE,   // 'l <= r'
		opGT,   // 'l > r'
		opGE,   // 'l >=r'
		opSHL,  // 'l << r'  shift left as binary (= l * 2^r)
		opSHR,  // 'l >> r'  shift right as binary (= l / 2^r)
		opCompl,// '~ l'   1's complement, only used for numbers that fit into int64_t
		opPLUS, // l + r
		opMINUS,// l - r
		opMUL,  // l * r
		opDIV,  // l / r
		opMOD,  // l % r (fmod)
		opUMIN, // unary minus on stack ('@')
		opNOT,  // 0 or 1
		opPOW,  // exponent ('l on the power of r')
        opOpenBrace,  // '('
        opCloseBrace, // ')'
		opLET   //'=',  for variables
	};

    /*==================*/
	struct OP
	{
		OP() : precedence(-99),right_associative(false), oper(opINVALID) {}
		int precedence;
		bool right_associative;
		OperatorType oper;
	};

    /*==================*/
	class MathOperator
	{
		static map<SmartString, OP> ops;
		static bool bOpsOk;

	public:
		MathOperator() { Setup(); };
		static void Setup();

		static OP Op(const SmartString &name)
		{
			static OP invOp;
			if(!bOpsOk)
				Setup();
			if(ops.count(name) )
					return ops[name];
			else
					return invOp;	// ops[PCHAR("*invalid*")];
		}
	};

	/*************
	 * Token
	 * a token from the input string.
	 * It may be
	 *       - a number like '2','.3' or '-123.456E-12'
	 *       - a variable name like 'foo' or 'foo_bar'
	 *       - a function name like in 'f(x,y,z)' or 'f_g(x)'
	 *       - an operator (binary or unary)
	 */
    /*==================*/
	class Token
	{
		TokenType type;
		SmartString name; // text of name (e.g. "+", or "foo(L" )
        LongNumber::RealNumber val;
		OP data;

		void _GetDecDigits(const SmartString &text, unsigned &pos);
		void _GetDecimalNumber(const SmartString &text, unsigned &pos);
		void _GetHexNumber(const SmartString &text, unsigned &pos);
		void _GetOctNumber(const SmartString &text, unsigned &pos);
		void _GetBinaryNumber(const SmartString &text, unsigned &pos);
        void _GetNumberFromQuotedString(const SmartString &text, unsigned &pos);
		void _GetVarOrFuncOrOperator(const SmartString &text, unsigned &pos);
		void _GetOperator(const SmartString &text, unsigned &pos);
	public:
		Token(const SmartString &text, unsigned &pos);     // gets next token from 'text' starting at position 'pos'
												           // and sets 'pos' to the first character after the token
        Token(TokenType t, SmartString b, OP d):type(t), name(b), val(RealNumber::RN_0), data(d) {}
		Token(const Token& tk) { *this = tk; }
		Token &operator=(const Token& tk) { type = tk.type; val = tk.val; data = tk.data; name = tk.name; return *this; }
		Token &operator=(const SmartString s) // use only for new operator NOT for variables or functions!
		{
			name = s;
			OP op = MathOperator::Op(name);
            if(op.oper == opOpenBrace || op.oper == opCloseBrace)
            {
              type = tknBrace; // special handling for braces
			  data = op;
            }
			else if(op.oper != opINVALID)
    		{
			  type = tknOperator;		// known operator
			  data = op;
			}
			else
			  type = tknUnknown;		// any other character

			return *this;
		}
            // this is only used wen new value is stored in val as
            // name is not set
        Token(RealNumber v): type(tknNumber), val(v) {}

        void FromText(const SmartString& text, unsigned& pos);

		TokenType Type() const { return type; }
        OperatorType Oper() const { return data.oper; }
		operator const SmartString&() const { return name; }
		const SmartString& Text() const { return name; }
        RealNumber Value() const { return val; }
		int Precedence() const { return data.precedence; }
		bool RightAssoc() const { return data.right_associative; }
	};

	typedef std::vector<Token> TokenVec;

    enum class Trigger_Type
    {
        NO_PROBLEMS,
        BUILTIN_FUNCTIONS_CANNOT_BE_REDEFINED,
        BUILTIN_VARIABLES_CANNOT_BE_REDEFINED,
        CLOSING_QUOTE_NOT_FOUND,
        DIVISON_BY_0,
        EITHER_THE_SEPARATOR_WAS_MISPLACED_OR_PARENTHESIS_WERE_MISMATCHED,
        EXPRESSION_ERROR,
        FUNCTION_DEFINITION_MISSING_RIGHT_BRACE,
        FUNCTION_MISSING_OPENING_BRACE,
        ILLEGAL_AT_LINE_END,
        ILLEGAL_BINARY_NUMBER,
        ILLEGAL_CHARACTER,
        ILLEGAL_CHARACTER_NUMBER,
        ILLEGAL_HEXADECIMAL_NUMBER,
        ILLEGAL_NUMBER_No1,
        ILLEGAL_NUMBER_No2,
        ILLEGAL_OCTAL_NUMBER,
        ILLEGAL_OPERATOR_AT_LINE_END,
        INVALID_CHARACTER_IN_FUNCTION_DEFINITION,
        INVALID_FUNCTION_DEFINITION,
        MISMATCHED_PARENTHESIS,
        MISSING_BINARY_NUMBER,
        NO_FUNCTION_ARGUMENT,
        RECURSIVE_FUNCTIONS_ARE_NOT_ALLOWED,
        SYNTAX_ERROR,
        STACK_ERROR,
        UNKNOWN_FUNCTION_IN_EXPRESSION,
        VARIABLE_DEFINITION_MISSING
    };

    extern std::map<Trigger_Type, SmartString> triggerMap;
	void Trigger(Trigger_Type tt);

    /*==================*/
    // variable or function does not know its name (see FunctionTable below)
    typedef RealNumber (*FUNCT1)(RealNumber val);
    typedef RealNumber (*FUNCT2R)(RealNumber x, RealNumber y);
    typedef RealNumber (*FUNCT2I)(RealNumber x, int y);
    typedef RealNumber (*FUNCT2A)(RealNumber x, AngularUnit au);
    struct BuiltInFunction
    {
        enum ArgTyp {atyNone, atyR, atyI, atyA}; // RealNumber, integer or AngularUnit
        FUNCT1 funct1 = nullptr;        // arguments RealNumber
        FUNCT2R funct2r = nullptr;        // arguments RealNumber RealNumber
        FUNCT2I funct2i = nullptr;        // arguments RealNumber integer
        FUNCT2A funct2a = nullptr;        // arguments RealNumber Angular Units
        void clear() { funct1 = nullptr;  funct2r = nullptr; funct2i = nullptr; funct2a = nullptr; }
        RealNumber operator() (RealNumber r) 
        { 
            if (funct1) 
                return funct1(r); 
            return NaN; 
        }
        RealNumber operator() (RealNumber r1, RealNumber r2) 
        { 
            if (funct2r) 
                return funct2r(r1,r2); 
            return NaN; 
        }
        RealNumber operator() (RealNumber r, int i) 
        { 
            if (funct2i) 
                return funct2i(r,i); 
            return NaN; 
        }
        RealNumber operator() (RealNumber r, AngularUnit au) 
        { 
            if (funct2a) 
                return funct2a(r,au); 
            return NaN; 
        }
        ArgTyp SecondArgumentType() const 
        {
            if (funct1) return atyNone;
            if (funct2r) return atyR;
            if (funct2i) return atyI;
            if (funct2a) return atyA;
            return atyNone;
        }
    };



    /*=============================================================
     * User defined variables
     * which are created from input line whose format is: 
     *      <name> = <definition>[:comment]
     * where
     *  <definition> is a formula, possibly using existing variables
     *      optional comment may contain a unit between []
     *  example:
     *      a=12 *sin(3pi/2):[kg] something I made up
     * If the colon is not given both the unit and the comment will be empty
     * If no unit is given 'unit' will be empty
     *------------------------------------------------------------*/
    struct Variable
    {        
        Constant data;          // the value of a user defined variable
        bool dirty=false;
                                // 'dirty:' must re-calculate 'value', 
                                // because any variable/function in 'definition' is redefined: 

        SmartString body;       // text from the right hand side of the equal sign, empty for builtins
        TokenVec definition;    // processed body in postfix, empty for builtins

        bool being_processed=false;     // under calculation (prevent recursion)

        Variable(){}
        Variable(Constant &c) : data(c) {}
        Variable(SmartString line) 
        {
            int pos = line.indexOf(SCharT('='));
            if (pos < 0)
                return;
            data.name = line.left(pos++);         // to definition
            pos = line.indexOf(schCommentDelimiter, pos);
            if (pos < 0)    // only variable body
                body = line.mid(pos);
            else
            {
                pos = line.indexOf("["_ss, pos + 1);
                if (pos >= 0)
                {
                    int pos1 = line.indexOf("]"_ss, pos + 1);
                    if (pos1 > 0)    // else no unit
                    {
                        data.unit = line.mid(pos + 1, pos1 - pos);
                        ++pos1;
                    }
                    else 
                        pos1 = pos;
                    data.desc = line.mid(pos1);
                }
            }
        }
        explicit Variable(const wchar_t* cname, const wchar_t* cunit, const wchar_t* cdesc, const RealNumber cvalue = RealNumber()) :
            data(cname, cvalue, cunit, cdesc) {}
        explicit Variable(const String name, const RealNumber value, const String unit, const String desc) : 
            data(name, value, unit, desc, nullptr)  {}
        virtual ~Variable()
        {
        }
    };

    /*=============================================================
     * Common structure for built-in (BI) and user defined (UD) functions
     *------------------------------------------------------------*/
    struct Func
    {
        SmartString name;
        SmartString unit;           // for physical constants or functions
        SmartString desc;           // optional description
        RealNumber value;           // defined or calculated value
            // for BI functions:
        bool builtin = true;            // builtin function (builtin variables are constants)

        BuiltInFunction function;              // for builtin functions only, at most 2 arguments
        bool useAngleUnit = false,      // for builtin trigonometric functions only (default: false)
             useAngleUnitAsResult=false;// for built in inverse trigonometric functions only
                                        // if true number is converted to deg or grad
        //    // for BI variables
        //bool isnumber=false;          // variable is a number: never can get dirty

           // for UD functions
        StringVector args;              // arguments: empty for functions w.o. arguments
        SmartString body;               // text from the right hand side of the equal sign
        TokenVec definition;            // processed body in postfix, none for builtins
               //  for variables
        bool dirty = false;             // any variable/function in 'definition' is
                                        // redefined: must re-calculate 'value'
        bool being_processed=false;     // under calculation (prevent recursion)

        Func() {};
        Func(const Func& var) { *this = var;}
        // This constructor is used for builtins only. They never get dirty
        Func(const SCharT *desc, const RealNumber val/*, bool isNumber = false*/) :
                desc(desc)/*, isnumber(isNumber)*/, value(val) {}
        // This operator is intended to be used only by 'FuncTable'
        // Were it used outside all other variable definitions should be checked
        // and marked dirty when needed
        Func & operator=(const Func & var)
        {
            name = var.name;
            args = var.args;
            body = var.body;
            desc = var.desc;
            definition = var.definition;
            builtin = var.builtin;
            function    = var.function;
            useAngleUnit = var.useAngleUnit;
            useAngleUnitAsResult = var.useAngleUnitAsResult;
            //isnumber = var.isnumber;
            value = var.value;
            dirty = var.dirty;
            being_processed = var.being_processed;
            return *this;
        }
        int RequireddArgumentCount() const 
        {
            if (builtin)
            {
                if (function.funct1) return 1;
                return 2;   // funct2r, funct2i, funct2a
            }
            else
                return args.size();
        }
    };

    class LittleEngine;

    struct VarFuncInfo
    {
        unsigned uBuiltinFuncCnt;
        unsigned uBuiltinVarCnt;
        unsigned uUserFuncCnt;
        unsigned uUserVarCnt;
        SmartString sBuiltinFuncs;
        SmartString sBuiltinVars;
        SmartString sUserFuncs;
        SmartString sUserVars;
        LittleEngine *pOwner;
    };

    typedef map<SmartString,Func>       FunctionTable;
    typedef map<SmartString,Variable>   VariableTable;  // user defined only. For builtins use 'constantsTable'

    /*==================*/
	class LittleEngine
	{
    public:
        enum class ResultType { rtNumber, rtDefinition, rtInvalid};
        enum class Beautification {
            bmoGraphText, 			// 3.1e-5 => 3.1 dot 10^{-5}
            bmoHtml, 				// 3.1e-5 => 3.1 dot 10<sup>-5</sup>
            bmoTEX, 				// 3.1e-5 => 3.1 \cdot 10^{-5}
            bmoNone					// 3.1e-5 =>  3.1e-5
        };
        enum class ResValid { rvOk, rvInvalid, rvDef };            // validity of result: OK, invalid, function, etc definition

        static bool builtinsOk; // built in functions are set up
        static unsigned numBuiltinVars,numBuiltinFuncs;

        DisplayFormat displayFormat;

        SmartString infix;
        TokenVec tvPostfix;
        RealNumber calcResult; // store result of calculation
        ResultType resultType = ResultType::rtNumber;
        static VariableTable variables;
        static FunctionTable functions;
        SmartString ssNameOfDatFile;
        ResValid resultValid = ResValid::rvOk;
        Beautification beautification = Beautification::bmoGraphText;
        bool clean; // no changes to variables or functions?
    private:
        /* ==========
         * internal class
         *-----------*/
		struct _Stack
		{
			TokenVec _stack;
			void clear() {_stack.clear();}
			bool empty() const { return _stack.empty(); }
			int push(Token &tok)
            {
               _stack.push_back(tok);
               return _stack.size();
            }
            int push(RealNumber v)
            {
                _stack.push_back(  Token(v) );
                return _stack.size();
            }

			void pop(unsigned n=1)
            {
                while(n-- && !_stack.empty() )
                    _stack.pop_back();
            }
			void popto(TokenVec &dest)
			{
				if(!empty())
				{
					dest.push_back( _stack[_stack.size()-1] );
					_stack.pop_back();
				}
			}
			const Token &operator[](unsigned index) const { return _stack.at(index); }
            const Token& peek(unsigned n = 1) const; // bounds checking added
            unsigned size() const { return _stack.size(); }
		} stack;
		void _HandleUnknown(Token *tok);

        bool _VariableAssignment(const SmartString &expr, unsigned &pos, Token *tok);
        bool _FunctionAssignment(const SmartString &expr, unsigned &pos, Token *tok);
        void _HandleOperator(Token* tok);
        void _HandleBrace(Token* tok);
        void _MarkDirty(const SmartString name); // all variables containing this 'name'
                                           // in their definition

        void _DoVariable(const Token &tok);
        void _DoFunction(const Token &tok);
        void _DoOperator(const Token &tok);
		int _InfixToPostFix(const SmartString expr); // postfix in 'tvPostfix' and returns 0 for assignment expression
        RealNumber _CalcPostfix(TokenVec& tv); // calculate the value using 'tv'
        SCharT _argSeparator = ',';
	public:

        LittleEngine();
        LittleEngine &operator=(const LittleEngine &src);
        RealNumber Calculate();                                 // using infix and angleUnit
		SmartString Postfix() const;                            // get converted data as SmartString
        void GetVarFuncInfo(VarFuncInfo &vf);                  // how many and what are they
        SmartString SerializeVariables(bool builtin=false) const;     // in a single SmartString
                                                                // <name>:<body>:<comment>:<unit>
        SmartString SerializeFunctions(bool builtin=false) const;     // each line contains one var/func
                                                                // <name(arg1,...)>:<body>:<comment>
        bool AddUserVariablesAndFunctions(SmartString definition, int what); //0: any, 1: vars, 2: functions


        bool ReadTables(SmartString name=SmartString()); // no name: uses FalconCal_DAT_FILE in user directorr
        bool SaveTables(SmartString name=SmartString()); // if it wasn't read and no name is given it wont be saved
        bool ResultOk() const { return calcResult.IsValid(); }
        RealNumber Result() const { return calcResult; }
        SmartString ResultAsDecString();
        SmartString ResultAsHexString() const;
        SmartString ResultAsOctString() const;
        SmartString ResultAsBinString() const;
        SmartString ResultAsCharString() const;
        AngularUnit AngleUnit() const { return displayFormat.angUnit; }
	};

// end of namespace FalconCalc
}

// calculateH
#endif

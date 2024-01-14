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

namespace littlecalc
{

    extern const SCharT comment_delimiter;

    enum ResultType {rtInvalid, rtNumber, rtDefinition};    // last one: function definition

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

		void GetDecDigits(const SmartString &text, unsigned &pos);
		void GetDecimalNumber(const SmartString &text, unsigned &pos);
		void GetHexNumber(const SmartString &text, unsigned &pos);
		void GetOctNumber(const SmartString &text, unsigned &pos);
		void GetBinaryNumber(const SmartString &text, unsigned &pos);
        void GetNumberFromQuotedString(const SmartString &text, unsigned &pos);
		void GetVarOrFuncOrOperator(const SmartString &text, unsigned &pos);
		void GetOperator(const SmartString &text, unsigned &pos);
	public:
		Token(const SmartString &text, unsigned &pos);     // gets next token from 'text' starting at position 'pos'
												           // and sets 'pos' to the first character after the token
        Token(TokenType t, SmartString b, OP d):type(t), name(b), val(LongNumber::zero), data(d) {}
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

		TokenType Type() const { return type; }
        OperatorType Oper() const { return data.oper; }
		operator const SmartString&() const { return name; }
		const SmartString& Text() const { return name; }
        RealNumber Value() const { return val; }
		int Precedence() const { return data.precedence; }
		bool RightAssoc() const { return data.right_associative; }
	};

	typedef std::vector<Token> TokenVec;

	void Trigger(SmartString text);

    /*==================*/
    // variable or function does not know its name (see VarFuncTable below)
    typedef RealNumber (*FUNCT1)(RealNumber val);
    typedef RealNumber (*FUNCT2)(RealNumber x, RealNumber y);
    typedef RealNumber (*FUNCT3)(RealNumber x, int y);
    typedef RealNumber (*FUNCT4)(RealNumber x, AngularUnit au);
    struct Function
    {
        enum argtyp {atyR, atyI, atya}; // RealNumber, integer or AngularUnit
        FUNCT1 funct1 = nullptr;        // arguments RealNumber
        FUNCT2 funct2 = nullptr;        // arguments RealNumber RealNumber
        FUNCT3 funct3 = nullptr;        // arguments RealNumber integer
        FUNCT4 funct4 = nullptr;        // arguments RealNumber Angular Units
        RealNumber operator() (RealNumber r) { if (funct1) return funct1(r); return NaN; }
        RealNumber operator() (RealNumber x, RealNumber y) { if (funct2) return funct2(x,y); return NaN; }
        RealNumber operator() (RealNumber x, int y) { if (funct3) return funct3(x,y); return NaN; }
        RealNumber operator() (RealNumber r, AngularUnit au) { if (funct4) return funct4(r, au); return NaN; }
        //Function(const FUNCT1 f1) : funct1(f1) {}
        //Function(const FUNCT2 f2) : funct2(f2) {}
        //Function(const FUNCT3 f3) : funct3(f3) {}
        //Function(const FUNCT4 f4) : funct4(f4) {}
    };
    /*=============================================================
     * Common structure for built-in (BI) and user defined (UD) functions
     * Doen't contain its name as that is the key in a map<>
     *------------------------------------------------------------*/

    struct VarFunc
    {
        SmartString description;        // optional description
            // for BI functions:
        bool builtin = true;            // builtin variable or function
        Function function;              // for builtin functions only
        bool useAngleUnit = false,      // for builtin trigonometric functions only (default: false)
             useAngleUnitAsResult=false;// for built in inverse trigonometric functions only
                                        // if true number is converted to deg or grad
            // for BI variables
        bool isnumber=false;            // variable is a number: never can get dirty
        RealNumber value;               // defined or calculated value

           // for UD functions
        std::vector<SmartString> args;       // arguments: empty for functions w.o. arguments
        SmartString body;                    // text from the right hand side of the equal sign
        TokenVec definition;            // processed body in postfix, none for builtins
               //  for variables
        bool dirty = false;             // any variable/function in 'definition' is
                                        // redefined: must re-calculate 'value'
        bool being_processed=false;     // under calculation (no recursion)

        VarFunc() {};
        VarFunc(const VarFunc& var) { *this = var;}
        // This constructor is used for builtins only. They never get dirty
        VarFunc(const SCharT *desc, const RealNumber val, bool isNumber=false):
                description(desc), isnumber(isNumber), value(val) {}
        // This operator is intended to be used only by 'VarFuncTable'
        // Were it used outside all other variable definitions should be checked
        // and marked dirty when needed
        VarFunc & operator=(const VarFunc & var)
        {
            args = var.args;
            body = var.body;
            description = var.description;
            definition = var.definition;
            builtin = var.builtin;
            function    = var.function;
            useAngleUnit = var.useAngleUnit;
            useAngleUnitAsResult = var.useAngleUnitAsResult;
            isnumber = var.isnumber;
            value = var.value;
            dirty = var.dirty;
            being_processed = var.being_processed;
            return *this;
        }
    };

    class LittleEngine;

    struct VARFUNC_INFO
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


    struct CALC_INPUT
    {
        CALC_INPUT() : angu(LongNumber::AngularUnit::auRad) {}
        SmartString infix;
        LongNumber::AngularUnit angu;  // used when calculating sine, cosine, tangent, cotangent
    };
    typedef map<SmartString,VarFunc> VarFuncTable;


    /*==================*/
	class LittleEngine
	{
        enum class ResultType { rtNumber, rtDefinition, rtInvalid};

        static bool builtinsOk; // built in functions are set up
        static unsigned numBuiltinVars,numBuiltinFuncs;
        LongNumber::AngularUnit aU;   // used when calculating sine, cosine, tangent, cotangent
		SmartString infix;
        TokenVec tvOutput;
        LongNumber::DisplayFormat _displayFormat;
        RealNumber _calcResult; // store result of calculation
        ResultType _resultType = ResultType::rtNumber;
        VarFuncTable variables,
                     functions;
        SmartString name_variable_table;
        bool clean; // no changes to variables or functions?
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
			const Token &operator[](unsigned index) { return _stack.at(index); }
			const Token &peek(unsigned n=1) // bounds checking added
            {
				if(_stack.size() < n)
					Trigger("Stack error"_ss);
                return _stack[_stack.size() - n];
            }
            const unsigned size() { return _stack.size(); }
		} stack;
		void HandleUnknown(Token *tok);

        bool VariableAssignment(const SmartString &expr, unsigned &pos, Token *tok);
        bool FunctionAssignment(const SmartString &expr, unsigned &pos, Token *tok);
        void HandleOperator(Token* tok);
        void HandleBrace(Token* tok);
        void MarkDirty(const SmartString name); // all variables containing this 'name'
                                           // in their definition

        void DoVariable(const Token &tok);
        void DoFunction(const Token &tok);
        void DoOperator(const Token &tok);
		int Convert(const SmartString &expr); // returns 0 for assignment expression
        RealNumber CalcPostfix(TokenVec& tv); // calculate the value using 'tv'
        SCharT _ArgSeparator() const { return RealNumber::DecPoint() == SCharT('.') ? ','_ss : ';'_ss; }
	public:
        DisplayFormat displayFormat;


        LittleEngine();
        LittleEngine &operator=(const LittleEngine &src);
        RealNumber Calculate(const SmartString &expr); // using angle unit 'auRad'
        RealNumber Calculate(const CALC_INPUT &ci);    // set angle unit
		SmartString Postfix() const;                             // get converted data as SmartString
        void GetVarFuncInfo(VARFUNC_INFO &vf);              // how many and what are they
        SmartString GetVariables(bool builtin=false) const;      // in a single SmartString
        SmartString GetFunctions(bool builtin=false) const;      // each line contains one var/func
                                                            // <name>=<body>;<comment>ar/a
                                                            // <name(arg1,...)>=<body>;<comment>
        bool AddUserVariablesAndFunctions(SmartString definition, int what); //0: any, 1: vars, 2: functions


        bool ReadTables(const SCharT *name);
        bool SaveTables(const SCharT *name=0); // if it wasn't read and no name is given it wont be saved
        bool ResultOk() const { return _calcResult.IsValid(); }
        RealNumber Result() const { return _calcResult; }
        SmartString ResultAsDecString();
        SmartString ResultAsHexString();
        SmartString ResultAsOctString();
        SmartString ResultAsBinString();
        SmartString ResultAsCharString();
	};
 //#define VARIABLES LittleEngine::variables
 //#define FUNCTIONS LittleEngine::functions

// end of namespace littlecalc
}

// calculateH
#endif

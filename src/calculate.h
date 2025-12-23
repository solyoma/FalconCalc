#pragma once
#ifndef calculateH
 #define calculateH

#ifndef __cplusplus
#error use a c++ compiler
#endif

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>

#include <map>
#include <locale>

#include <cmath>

#include "defines.h"

#ifndef QTSA_PROJECT
    #include "SmartString.h"
#else
    #include "SmartStringQt.h"
#endif

using namespace SmString;
#include "LongNumber.h"
using namespace LongNumber;

#include "EngineErrors.h"

#ifndef QTSA_PROJECT
    #include "stdafx_lc.h"
    // always use 'using namespace SmString;' and 'using namespace LongNumber;'
    // before including this
    #include "translations.h"
#endif

namespace FalconCalc
{
    extern SCharT argSeparator;

    extern const SCharT schCommentDelimiter;
    extern const SmartString ssCommentDelimiterString;
    extern const SmartString ssEqString;

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
		static std::map<SmartString, OP> ops;
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

		void _GetDecDigits(const SmartString &text, LENGTH_TYPE &pos);
		void _GetDecimalNumber(const SmartString &text, LENGTH_TYPE &pos);
		void _GetHexNumber(const SmartString &text, LENGTH_TYPE &pos);
		void _GetOctNumber(const SmartString &text, LENGTH_TYPE &pos);
		void _GetBinaryNumber(const SmartString &text, LENGTH_TYPE &pos);
        void _GetNumberFromQuotedString(const SmartString &text, LENGTH_TYPE &pos);
		void _GetVarOrFuncOrOperator(const SmartString &text, LENGTH_TYPE &pos);
		void _GetOperator(const SmartString &text, LENGTH_TYPE &pos);
	public:
		Token(const SmartString &text, LENGTH_TYPE &pos);  // gets next token from 'text' starting at position 'pos'
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

        void FromText(const SmartString& text, LENGTH_TYPE& pos);

		TokenType Type() const { return type; }
        OperatorType Oper() const { return data.oper; }
		operator const SmartString&() const { return name; }
		const SmartString& Text() const { return name; }
        RealNumber Value() const { return val; }
		int Precedence() const { return data.precedence; }
		bool RightAssoc() const { return data.right_associative; }
	};

	typedef std::vector<Token> TokenVec;

    class Trigger
    {  
    public:
#ifdef QTSA_PROJECT
        Trigger() { }
#else
        Trigger() 
        { 
            SetLanguage(AppLanguage::lanEng); 
        }
        void SetLanguage(AppLanguage lang);
        //void Raise(TextIDs tids);    // sets error state into global 'lengine'
#endif
        void Raise(EngineErrorCodes tids);    // sets error state into global 'lengine'
#ifndef QTSA_PROJECT
    private:
        AppLanguage _lang;
#endif
    };
    extern Trigger trigger;

    /*============= built-in function types ============*/
    typedef RealNumber (*FUNCT1)(RealNumber val);
    typedef RealNumber (*FUNCT2R)(RealNumber x, RealNumber y);
    typedef RealNumber (*FUNCT2I)(int y, RealNumber x);
    typedef RealNumber (*FUNCT2A)(RealNumber x, AngularUnit au);
    struct BuiltinFunc
    {
        enum ArgTyp {atyNone, atyR, atyI, atyA}; // RealNumber, integer or AngularUnit

        SmartString name;               // w.o. parathesis and arguments
        SmartString desc;

                // only one of the 'func...' pointers isn't nullptr
        FUNCT1 funct1 = nullptr;        // argument: RealNumber
        FUNCT2R funct2r = nullptr;      // arguments: (RealNumber, RealNumber   )
        FUNCT2I funct2i = nullptr;      // arguments: (RealNumber, integer      )
        FUNCT2A funct2a = nullptr;      // arguments: (RealNumber, Angular Units)

        bool useAngleUnit = false,      // for builtin trigonometric functions only (default: false)
             useAngleUnitAsResult=false;// for built in inverse trigonometric functions only

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
                return funct2i(i, r); 
            return NaN; 
        }
        RealNumber operator() (RealNumber r, AngularUnit angu)
        { 
            if (funct2a) 
                return funct2a(r,angu);
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
        BuiltinFunc & operator=(const BuiltinFunc & ofunc)
        {
            name = ofunc.name;
            desc = ofunc.desc;
            funct1  = ofunc.funct1;
            funct2r = ofunc.funct2r;
            funct2i = ofunc.funct2i;
            funct2a = ofunc.funct2a;
            useAngleUnit = ofunc.useAngleUnit;
            useAngleUnitAsResult = ofunc.useAngleUnitAsResult;
            return *this;
        }
        int RequireddArgumentCount() const 
        {
            if (funct1) 
                return 1;
            return 2;   // funct2r, funct2i, funct2a
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
    struct Variable : public Constant
    {        
	                            // builtins are not stored in 'VariableTable'
                                // they are constants in LongNumber
		SmartString body;       // text from the right hand side of the equal sign
		TokenVec tokenVec;      // processed body in postfix
		bool dirty = false;     // 'dirty = true => must re-calculate 'value', 
                                // because any variable/function in its 'tokenVec' is redefined: 

        bool being_processed=false;     // under calculation (prevent recursion)

        Variable(){}
        Variable(Constant &co) : Constant(co) {}
        Variable(SmartString line) 
        {
            int pos = line.indexOf(SCharT('='));
            if (pos < 0)
                return;
            name = line.left(pos++);         // to definition
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
                        unit = line.mid(pos + 1, pos1 - pos);
                        ++pos1;
                    }
                    else 
                        pos1 = pos;
                    desc = line.mid(pos1);
                }
            }
        }
        explicit Variable(const String name, const RealNumber value, const String unit, const String desc) : 
            Constant(name, value, unit, desc, nullptr) , dirty(true) 
        {}
        virtual ~Variable()
        {
        }
        SmartString Serialize() const
        {
            SmartString s(name + ssEqString + body);
            if (!desc.empty())
                s += ssCommentDelimiterString + desc;
            else if(!unit.empty())                 // but description is
                s += ssCommentDelimiterString;
            if(!unit.empty())
                s += ssCommentDelimiterString + unit;

            return s;
        }
        inline UTF8String  SerializeUtf8() const
        {
            return Serialize().toUtf8String();
        }

    };

    /*=============================================================
     * User defined (UD) functions
     *  they may have any number of arguments
     *------------------------------------------------------------*/
    struct Func
    {
        SmartString name;           // w.o. the arguments and the braces
        SmartString body;           // text from the right hand side of the equal sign
        SmartString desc;           // optional description
        SmartString unit;           // for physical constants or functions
        
        SmartStringVector args;          // arguments: empty for functions w.o. arguments

        RealNumber value;           // defined or calculated value

           // for UD functions
        TokenVec tokenVec;            // processed body in postfix, none for builtins
               //  for variables
        bool dirty = false;             // any variable/function in 'definition' is
                                        // redefined: must re-calculate 'value'
        bool being_processed=false;     // under calculation (prevent recursion)

        Func() {};
        Func(const Func& var) { *this = var;}
        // This constructor is used for builtins only. They never get dirty
        Func(const SCharT *desc, const RealNumber val) :desc(desc), value(val) {}

        Func &operator=(const Func& other)
        {
            name = other.name;
            body = other.body;
            desc = other.desc;
            unit = other.unit;
            args = other.args;
            value = other.value;
            tokenVec = other.tokenVec;
            dirty = other.dirty;
            being_processed = other.being_processed;
			return *this;
        }

        // This operator is intended to be used only by 'FuncTable'
        // Were it used outside all other variable definitions should be checked
        // and marked dirty when needed
        inline SmartString FullNameWithArgs() const
        {
            SmartString s(name + "("_ss);
            if (args.size())
                s += args[0];
            for (size_t j = 1; j < (size_t)args.size(); ++j)  //(size_t) needed for Qt compatibility
                s += SmartString(argSeparator) + args[j];
			s += ")"_ss;
            return s;
        }
        SmartString Serialize() const
        {
            SmartString s(FullNameWithArgs() + ssEqString + body);
            if (!desc.empty() || !unit.empty())
                s += ssCommentDelimiterString + desc;
            if(!unit.empty())
                s += ssCommentDelimiterString + unit;

            return s;
        }
        inline UTF8String  SerializeUtf8() const
        {
            return Serialize().toUtf8String();
        }
    };

    class LittleEngine;

    // for user variables and functions we need a fast lookup by string container
    // in which the order of the items remains the same as the
    // added order

    //---------------------------------
    template <class T, class String>
    class DataMap
    {
        std::map<String, int> _index;
        std::vector<T> _vec;
    public:
        DataMap() {}
        DataMap(const DataMap& o)
        {
            _index = o._index;
            _vec = o._vec;
        }
        virtual ~DataMap() {};

        bool changed = false;

        T& operator[](int ix)
        {
            return this->_vec[ix];
        }

        T& operator[](const String key)
        {
            int ix = -1;
            if (this->_index.count(key))                   // !!! w.o. this-> it won't find _index
                ix = this->_index[key];
            else
            {
                ix = this->_vec.size();                    // !!! w.o. this-> it won't find _vec
                this->_index[key] = ix;
                T t=T();
                this->_vec.push_back(t);
            }
            return this->_vec[ix];
        }

        const String &Name(int ix) const   // name of index-th item
        {
            T& t = (*this)[ix];
            return t.Name();
        }

        int size() const 
        { 
            return this->_index.size(); 
        }

        int count(const SmartString &s) const
        { 
            return this->_index.count(s);
        }

        void clear()
        {
            this->_index.clear();
            this->_vec.clear();
        }

        void erase(int i)
        {
            this->_vec.erase(this->_vec.begin() + i);
            _Remap();
        }

        void insert(T t, int i)
        {
            _vec.insert(_vec.begin() + i, t);
            _Remap();
        }
        int Index(String s)
        {
            if (count(s))
                return _index[s];
            return -1;
        }
    private:
        void _Remap()
        {
            _index.clear();
            for (size_t i = 0; i < _vec.size(); ++i)
                _index[_vec[i].Name()] = i;
        }
    };

    typedef DataMap<BuiltinFunc, SmartString> BuiltinFuncTable;
    typedef DataMap<Func, SmartString> FunctionTable;
    typedef DataMap<Variable, SmartString> VariableTable;
     // row Data for each row in variables/functions table
    struct RowData
    {
        SmartString cols[4];	// name, body, descr, unit

        RowData() {}
        RowData(const RowData& o) { *this = o; }
        RowData(SmartString name, SmartString body, SmartString descr, SmartString unit)
        {
            cols[0] = name;
            cols[1] = body;
            cols[2] = descr;
            cols[3] = unit;
        }
        void clear() { cols[0] = cols[1] = cols[2] = cols[3] = ""; }
        RowData& operator=(const RowData& o)
        {
            cols[0] = o.cols[0];
            cols[1] = o.cols[1];
            cols[2] = o.cols[2];
            cols[3] = o.cols[3];
            return *this;
        }
        bool operator!=(const RowData& rd)
        {
            for (int i = 0; i < 4; ++i)
                if (cols[i] != rd.cols[i])
                    return true;
            return false;
        }
        bool operator==(const RowData& rd)
        {
            return !(*this != rd);
        }
        SmartString Serialize();
    };

    typedef DataMap<RowData, SmartString> RowDataMap;

    /*==================*/
	class LittleEngine
	{
    public:
        enum class ResultType {rtUndef, rtNumber, rtDefinition, rtInvalid};

        static size_t builtInFuncCount; // built in functions are set up

        DisplayFormat displayFormat;

        SmartString infix;
        TokenVec tvPostfix;
        RealNumber calcResult; // store result of calculation
        ResultType resultType = ResultType::rtUndef;

        static VariableTable variables;
        static BuiltinFuncTable builtinFunctions;
        static FunctionTable functions;

        SmartString ssNameOfDatFile;
        bool clean;                             // no changes to variables or functions?
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

			void pop(LENGTH_TYPE n=1)
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
			const Token &operator[](LENGTH_TYPE index) const { return _stack.at(index); }
            const Token& peek(size_t n = 1) const; // bounds checking added
            LENGTH_TYPE size() const { return _stack.size(); }
		} stack;
		void _HandleUnknown(Token *tok);

        bool _VariableAssignment(const SmartString &expr, LENGTH_TYPE &pos, Token* tok);
        bool _FunctionAssignment(const SmartString& expr, LENGTH_TYPE &pos, Token *tok);
        void _HandleOperator(Token* tok);
        void _HandleBrace(Token* tok);
        void _MarkDependentVariablesDirty(const SmartString name); // all variables containing this 'name'
                                           // in their definition

        void _DoVariable(const Token &tok);
        void _DoFunction(const Token &tok);
        void _DoOperator(const Token &tok);
		int _InfixToPostFix(const SmartString expr); // postfix in 'tvPostfix' and returns 0 for assignment expression
        RealNumber _CalcPostfix(TokenVec& tv); // calculate the value using 'tv'
        void _SetNewLocale(SmartString sLocaleName);
	public:

        LittleEngine();
        LittleEngine &operator=(const LittleEngine &src);
        RealNumber Calculate();                                 // using infix and angleUnit
		SmartString Postfix() const;                            // get converted data as SmartString

        void AddUserVariables(const SmartStringVector &from); // from vf 0: any, 1: vars, 2: functions
        void AddUserFunctions(const SmartStringVector &from); // from vf 0: any, 1: vars, 2: functions

        bool LoadUserData(SmartString name=SmartString()); // no name: uses FalconCal_DAT_FILE in user directorr
        bool SaveUserData(bool force=false);
        bool SaveUserData(SmartString name);                // if it wasn't read and no name is given it wont be saved
        bool ResultOk() const { return calcResult.IsValid(); }

        RealNumber Result() const { return calcResult; }
        SmartString ResultAsDecString();
        SmartString ResultAsHexString();
        SmartString ResultAsOctString();
        SmartString ResultAsBinString();
        SmartString ResultAsCharString(); 
        AngularUnit AngleUnit() const { return displayFormat.angUnit; }
	};

// end of namespace FalconCalc
}
// global base engine
extern FalconCalc::LittleEngine *lengine;    // contains builtin functions and constants common to all instances of LittleEngine


// calculateH
#endif

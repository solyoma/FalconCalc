#include "EngineErrors.h"

#ifdef QTSA_PROJECT

#include <QApplication>

static QString _errors[] =
{
    /* EEC_NO_ERROR,	*/				        QApplication::tr("Result:"),
    /* EEC_VARIABLE_DEFINITION_MISSING */       QApplication::tr("Variable definition missing"),
    /* EEC_UNKNOWN_FUNCTION_IN_EXPRESSION */    QApplication::tr("Unknown function in expression"),
    /* EEC_SYNTAX_ERROR */				        QApplication::tr("Syntax error"),
    /* EEC_STACK_ERROR */				        QApplication::tr("Stack error"),
    /* EEC_OVERFLOW_ERROR */				    QApplication::tr("Overflow error"),
    /* EEC_DOMAIN_ERROR */				        QApplication::tr("Domain error"),
    /* EEC_MISSING_BRACE */				        QApplication::tr("Missing closing '}'"),
    /* EEC_LARGE_FOR64 */				        QApplication::tr("Number can't fit in a 64 bit integer"),
    /* EEC_RECURSIVE_FUNCTIONS_ARE_NOT_ALLOWED */	QApplication::tr("Recursive functions are not allowed"),
    /* EEC_NO_FUNCTION_ARGUMENT */				QApplication::tr("No function argument"),
    /* EEC_MISSING_BINARY_NUMBER */			    QApplication::tr("Missing binary number"),
    /* EEC_MISMATCHED_PARENTHESIS */			QApplication::tr("Mismatched parenthesis"),
    /* EEC_INVALID_FUNCTION_DEFINITION */		QApplication::tr("Invalid function definition"),
    /* EEC_INVALID_CHARACTER_IN_FUNCTION_DEFINITION */QApplication::tr("Invalid character in function definition"),
    /* EEC_ILLEGAL_OPERATOR_AT_LINE_END */		QApplication::tr("Illegal operator at end of expression"),
    /* EEC_ILLEGAL_OCTAL_NUMBER */				QApplication::tr("Illegal octal number"),
    /* EEC_ILLEGAL_NUMBER_No2 */				QApplication::tr("Illegal number #2"),
    /* EEC_ILLEGAL_NUMBER_No1 */				QApplication::tr("Illegal number #1"),
    /* EEC_ILLEGAL_HEXADECIMAL_NUMBER */		QApplication::tr("Illegal hexadecimal number"),
    /* EEC_ILLEGAL_CHARACTER_NUMBER */			QApplication::tr("Illegal character number"),
    /* EEC_ILLEGAL_BINARY_NUMBER */			    QApplication::tr("Illegal binary number"),
    /* EEC_ILLEGAL_AT_LINE_END */				QApplication::tr("Illegal at line end"),
    /* EEC_FUNCTION_MISSING_OPENING_BRACE */	QApplication::tr("Function missing opening brace"),
    /* EEC_FUNCTION_DEFINITION_MISSING_RIGHT_BRACE */QApplication::tr("Function definition missing right brace"),
    /* EEC_EXPRESSION_ERROR */				    QApplication::tr("Expression error"),
    /* EEC_EITHER_THE_SEPARATOR_WAS_MISPLACED_OR_PARENTHESIS_WERE_MISMATCHED */		QApplication::tr("Either the separator was misplaced or parenthesis were mismatched"),
    /* EEC_DIVISON_BY_0 */				        QApplication::tr("Divison by 0"),
    /* EEC_CLOSING_QUOTE_NOT_FOUND */			QApplication::tr("Closing quote not found"),
    /* EEC_BUILTIN_VARIABLES_CANNOT_BE_REDEFINED */QApplication::tr("Builtin variables cannot be redefined"),
    /* EEC_BUILTIN_FUNCTIONS_CANNOT_BE_REDEFINED */QApplication::tr("Builtin functions cannot be redefined"),
};

QString EngineErrorString(EngineErrorCodes eec)
{
    if((int)eec >= EEC_TOP)
        return QString();
    return _errors[(int)eec];
}
#endif
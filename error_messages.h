#ifndef TERMCALC_ERROR_MESSAGES_H
#define TERMCALC_ERROR_MESSAGES_H
#include <stddef.h>

#define ERR_NO_IMPLICIT_MULTIPLICATION 0
#define ERR_NO_DOUBLE_OPERATOR 1
#define ERR_NO_SECOND_OPERAND_FOUND 2
#define ERR_MULTIPLE_DECIMAL_POINTS 3
#define ERR_NO_FUNCTION_INPUT_FOUND 4
#define ERR_NO_OPERAND_FOUND 5
#define ERR_UNCLOSED_PARENTHESIS 6

char *error_message(size_t value);
#endif
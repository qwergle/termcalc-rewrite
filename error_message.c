#include "error_messages.h"
#include <stddef.h>

char *error_message(size_t value) {
  if (value == ERR_NO_IMPLICIT_MULTIPLICATION) return "No implicit multiplication.";
  else if (value == ERR_NO_DOUBLE_OPERATOR) return "No double operators.";
  else if (value == ERR_NO_SECOND_OPERAND_FOUND) return "No second operand found.";
  else if (value == ERR_MULTIPLE_DECIMAL_POINTS) return "No double decimal points.";
  else if (value == ERR_NO_FUNCTION_INPUT_FOUND) return "No function input found.";
  else if (value == ERR_NO_OPERAND_FOUND) return "No operand found.";
  else if (value == ERR_UNCLOSED_PARENTHESIS) return "Parenthesis imbalance";
  return "No error message found.";
}
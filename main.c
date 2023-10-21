#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "vm.h"


// Get full-length, allocated string from stdin
char * getstr() {
  register size_t buf_size = 32;
  char *buf = malloc(buf_size);
  char input_ch = getchar();
  size_t i = 0;
  while (input_ch != '\n') {
    *(buf+i) = input_ch;
    i++;
    if (i == buf_size) {
      buf_size = buf_size + 32;
      buf = realloc(buf, buf_size);
    }
    input_ch = getchar();
  }
  *(buf + i) = '\0';
  char *output = malloc(i + 1);
  memcpy(output, buf, i + 1);
  free(buf);
  return output;
}

struct Token {
  enum {BIN_OP, NUM, WORD, OPEN_PARA, CLOSE_PARA, FACTORIAL_OP, ERR_TOK, NEG_OP} token_type;
  size_t position;
  union {
    enum {ADD, SUB, MUL, DIV, EXP} bin_op_type;
    unsigned int func_val;
    double num_value;
    size_t err_val;
    char *str;
  } v;
};

typedef struct Token Token;

char **join_str_arrays(char **a, char **b, size_t a_length, size_t b_length) {
  char **out = malloc(sizeof(char*) * (a_length + b_length));
  size_t pos = 0;
  while (pos < a_length) {
    *(out + pos) = *(a + pos);
    pos++;
  }
  while (pos < a_length + b_length) {
    *(out + pos) = *(b + pos - a_length);
    pos++;
  }
  return out;
}

double ch_to_digit(char ch) {
  if (ch == '0') return 0;
  if (ch == '1') return 1;
  if (ch == '2') return 2;
  if (ch == '3') return 3;
  if (ch == '4') return 4;
  if (ch == '5') return 5;
  if (ch == '6') return 6;
  if (ch == '7') return 7;
  if (ch == '8') return 8;
  if (ch == '9') return 9;
  return -1;
}


#define NEXT_CH (*(str + i))

#define PREV_TOKEN_TYPE (tok_pos == 0 ? ERR_TOK : (tokens + tok_pos - 1)->token_type)

#define ADD_NUM_TOKEN() do { \
  if (PREV_TOKEN_TYPE == NEG_OP) *(tokens + tok_pos - 1) = (Token) {NUM, last_tok_pos, .v.num_value = -numValue}; \
  else *(tokens + tok_pos++) = (Token) {NUM, last_tok_pos, .v.num_value = numValue}; \
} while (0)

#define ADD_OPEN_PARA_TOKEN() *(tokens + tok_pos++) = (Token) {OPEN_PARA, i, .v.str = NULL}

#define ADD_CLOSE_PARA_TOKEN() *(tokens + tok_pos++) = (Token) {CLOSE_PARA, i, .v.num_value = 0}

#define RESIZE_TOKENS_IF_NEEDED() do { \
  if (tok_pos + 1 > *tokens_length) { \
    *tokens_length += INCR_LENGTH; \
    tokens = realloc(tokens, *tokens_length); \
  } \
} while (0)

#define RESET_NUMBER_VARIABLES() do { \
  isNum = false; \
  isDecimal = false; \
  placeValue = 0; \
  numValue = 0; \
} while (0)

#define TOKENIZER_ERROR(value) do { \
  *tokens = (Token) {ERR_TOK, last_tok_pos, .v.err_val = value}; \
  return tokens; \
} while (0)

#define DECIMAL_ON() do { \
  isDecimal = true; \
  placeValue = 1; \
} while (0)

#define RESET_WORD_VARIABLES() do { \
  isWord = false; \
  memset(buf, 0, buf_len); \
  word_pos = 0; \
} while (0)

#define RESIZE_WORD_BUFFER_IF_NEEDED() do { \
  if (word_pos+1 == buf_len) { \
    buf_len += INCR_LENGTH; \
    buf = realloc(buf, buf_len); \
  } \
} while (0)

#define ADD_WORD_TOKEN() do {*(tokens + tok_pos++) = (Token) {WORD, i, .v.str = strdup(buf)};} while (0)

#define IS_WORD_CH(ch) (isalpha(ch) || ch == '_')

#define ADD_NEGATIVE_SIGN_TOKEN() *(tokens + tok_pos++) = (Token) {NEG_OP, i, .v.num_value = 0}


#define INCR_LENGTH 32

#define ERR_NO_IMPLICIT_MULTIPLICATION 0
#define ERR_NO_DOUBLE_OPERATOR 1
#define ERR_NO_SECOND_OPERAND_FOUND 2
#define ERR_MULTIPLE_DECIMAL_POINTS 3
#define ERR_NO_FUNCTION_INPUT_FOUND 4

Token *tokenizer(char *str, size_t *tokens_length) {
  // Variables related to the token buffer
  *tokens_length = 128;
  Token *tokens = malloc(sizeof(Token) * *tokens_length);
  size_t tok_pos = 0;
  size_t last_tok_pos = 0;

  // Variables related to the string
  char ch;
  size_t i = 0;

  // Variables related to numbers
  bool isNum = false;
  double numValue = 0;
  double placeValue = 0;
  bool isDecimal = false;

  // Variables related to words (functions/variables/constants)
  bool isWord;
  size_t word_pos, buf_len = 32;
  char *buf = malloc(buf_len);
  RESET_WORD_VARIABLES();
  
  while (1) {
    ch = *(str + i++);
    if (i - last_tok_pos == 1) last_tok_pos++;
    
    if (isNum && !isdigit(ch) && ch != '.') {
      // Add a number token if the last character was the end of a number
      RESIZE_TOKENS_IF_NEEDED();
      ADD_NUM_TOKEN();
      RESET_NUMBER_VARIABLES();
      isNum = false;
    }

    if (isWord && !isalpha(ch) && ch != '_') {
      // Add a word token if the last character was not a word character
      RESIZE_TOKENS_IF_NEEDED();
      ADD_WORD_TOKEN();
      RESET_WORD_VARIABLES();
    }

    if (ch == '\0') {
    // If it's the end of the string
      if (PREV_TOKEN_TYPE == BIN_OP) TOKENIZER_ERROR(ERR_NO_SECOND_OPERAND_FOUND);
      RESIZE_TOKENS_IF_NEEDED();
      *tokens_length = tok_pos;
      return tokens;
    }
    if (isdigit(ch)) {
    // If it is a digit
      if (PREV_TOKEN_TYPE == CLOSE_PARA) TOKENIZER_ERROR(ERR_NO_IMPLICIT_MULTIPLICATION);
      if (!isNum) {
        // If it is the first digit of the number
        last_tok_pos = i;
        isNum = true;
      }
      // Add the digit to the number value
      if (!isDecimal) numValue = numValue * 10 + ch_to_digit(ch);
      else numValue += ch_to_digit(ch) * pow(10, -(placeValue++));
    } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^') {
    // If it is a binary operator or minus sign
      // Minus sign stuff
      if (ch == '-' && (PREV_TOKEN_TYPE == BIN_OP || PREV_TOKEN_TYPE == ERR_TOK || PREV_TOKEN_TYPE == NEG_OP || isdigit(NEXT_CH) || IS_WORD_CH(NEXT_CH))) {
        // If the token is actually a negative sign
        ADD_NEGATIVE_SIGN_TOKEN();
      } else {
        if (PREV_TOKEN_TYPE == BIN_OP) TOKENIZER_ERROR(ERR_NO_DOUBLE_OPERATOR);
        RESIZE_TOKENS_IF_NEEDED();
        // Add binary operator token
        switch (ch) {
          case '+' :
            *(tokens + tok_pos++) = (Token) {BIN_OP, last_tok_pos, .v.bin_op_type = ADD}; // add (+)
            break;
          case '-' :
            *(tokens + tok_pos++) = (Token) {BIN_OP, last_tok_pos, .v.bin_op_type = SUB}; // sub (-)
            break;
          case '*' :
            *(tokens + tok_pos++) = (Token) {BIN_OP, last_tok_pos, .v.bin_op_type = MUL}; // multiply (*)
            break;
          case '/' :
            *(tokens + tok_pos++) = (Token) {BIN_OP, last_tok_pos, .v.bin_op_type = DIV}; // divide (/)
            break;
          case '^' :
            *(tokens + tok_pos++) = (Token) {BIN_OP, last_tok_pos, .v.bin_op_type = EXP}; // exponential (^)
            break;
        }
      }
    } else if (ch == '(' || ch == ')') {
    // If it is a parenthesis
      // Throw errors
      if (PREV_TOKEN_TYPE == BIN_OP && ch == ')') TOKENIZER_ERROR(ERR_NO_SECOND_OPERAND_FOUND);
      if (PREV_TOKEN_TYPE == NUM && ch == '(') TOKENIZER_ERROR(ERR_NO_IMPLICIT_MULTIPLICATION);
      if (PREV_TOKEN_TYPE == WORD && ch == ')') TOKENIZER_ERROR(ERR_NO_FUNCTION_INPUT_FOUND);

      // Function stuff
      if (PREV_TOKEN_TYPE == WORD && ch == '(') (tokens + tok_pos - 1)->token_type = OPEN_PARA;
      else {
        // Add parenthesis token
        RESIZE_TOKENS_IF_NEEDED();
        if (ch == '(') ADD_OPEN_PARA_TOKEN();
        else if (ch == ')') ADD_CLOSE_PARA_TOKEN();
      }
    } else if (ch == '.') {
    // If it is a decimal point
      if (isDecimal) TOKENIZER_ERROR(ERR_MULTIPLE_DECIMAL_POINTS);
      DECIMAL_ON();
    } else if (isalpha(ch) || ch == '_') {
      if (!isWord) {
        if (PREV_TOKEN_TYPE == NUM || PREV_TOKEN_TYPE == CLOSE_PARA) TOKENIZER_ERROR(ERR_NO_IMPLICIT_MULTIPLICATION);
        isWord = true;
        last_tok_pos = i;
      }
      // Add character
      RESIZE_WORD_BUFFER_IF_NEEDED();
      strncat(buf, &ch, 1);
    }
  }
  return tokens;
}

char *error_message(size_t value) {
  if (value == 0) return "No implicit multiplication.";
  else if (value == 1) return "No double operators.";
  else if (value == 2) return "No second operand found.";
  else if (value == 3) return "No double decimal points.";
  else if (value == 4) return "No function input found.";
  return "No error message found.";
}

void print_token(Token token) {
  if (token.token_type == ERR_TOK) printf("Syntax Error: %s", error_message(token.v.err_val));
  else if (token.token_type == NUM) printf("[%g]", token.v.num_value);
  else if (token.token_type == BIN_OP) {
    if (token.v.bin_op_type == ADD) fputs("[+]", stdout);
    else if (token.v.bin_op_type == SUB) fputs("[-]", stdout);
    else if (token.v.bin_op_type == MUL) fputs("[*]", stdout);
    else if (token.v.bin_op_type == DIV) fputs("[/]", stdout);
    else if (token.v.bin_op_type == EXP) fputs("[^]", stdout);
  } else if (token.token_type == OPEN_PARA) {
    if (token.v.str != NULL) fputs(token.v.str, stdout);
    fputs("(", stdout);
  }
  else if (token.token_type == CLOSE_PARA) putc(')', stdout);
  else if (token.token_type == WORD) printf("[%s]", token.v.str);
  else if (token.token_type == NEG_OP) putc('-', stdout);
}

void print_tokens(Token *tokens, size_t tokens_len) {
  if (tokens->token_type == ERR_TOK) {
    print_token(*tokens);
  }
  else {
    for (size_t i = 0; i < tokens_len; i++) {
      print_token(*(tokens + i));
      if (i != tokens_len-1) putc(' ', stdout);
    }
  }
}

int main(void) {
  size_t tok_length = 32;
  char *str = getstr();
  Token *tokens = tokenizer(str, &tok_length);
  print_tokens(tokens, tok_length);
  putc('\n', stdout);
  return 0;
}
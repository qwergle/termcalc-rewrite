#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "vm.h"
#include "error_messages.h"


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

#define ADD_NUM_TOKEN() *(tokens + tok_pos++) = (Token) {NUM, last_tok_pos, .v.num_value = numValue}

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

#define ADD_FACTORIAL_TOKEN() *(tokens + tok_pos++) = (Token) {FACTORIAL_OP, i, .v.num_value = 0}


#define INCR_LENGTH 32

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
      if (PREV_TOKEN_TYPE == NEG_OP) TOKENIZER_ERROR(ERR_NO_OPERAND_FOUND);
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
    // If it is a word character
      if (!isWord) {
        if (PREV_TOKEN_TYPE == NUM || PREV_TOKEN_TYPE == CLOSE_PARA) TOKENIZER_ERROR(ERR_NO_IMPLICIT_MULTIPLICATION);
        isWord = true;
        last_tok_pos = i;
      }
      // Add character
      RESIZE_WORD_BUFFER_IF_NEEDED();
      strncat(buf, &ch, 1);
    } else if (ch == '!') {
    // If it is a factorial
      if (PREV_TOKEN_TYPE == BIN_OP || PREV_TOKEN_TYPE == OPEN_PARA || PREV_TOKEN_TYPE == ERR_TOK) TOKENIZER_ERROR(ERR_NO_OPERAND_FOUND);
      ADD_FACTORIAL_TOKEN();
    }
  }
  return tokens;
}

char *error_message(size_t value) {
  if (value == ERR_NO_IMPLICIT_MULTIPLICATION) return "No implicit multiplication.";
  else if (value == ERR_NO_DOUBLE_OPERATOR) return "No double operators.";
  else if (value == ERR_NO_SECOND_OPERAND_FOUND) return "No second operand found.";
  else if (value == ERR_MULTIPLE_DECIMAL_POINTS) return "No double decimal points.";
  else if (value == ERR_NO_FUNCTION_INPUT_FOUND) return "No function input found.";
  else if (value == ERR_NO_OPERAND_FOUND) return "No operand found.";
  else if (value == ERR_UNCLOSED_PARENTHESIS) return "No unclosed parentheses.";
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
  else if (token.token_type == FACTORIAL_OP) fputs("[!]", stdout);
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

// Transistional syntax tree structure for use during parsing
struct SemiTokenTreeNode {
  bool isToken;
  union {
    struct {
      enum {STT_OP, STT_PARA, STT_NUM, STT_VAR, STT_FUNC, STT_ERROR} nodeType;
      enum {STT_ADD, STT_SUB, STT_MUL, STT_DIV, STT_EXP, STT_NEG, STT_FACTORIAL} opType;
      char *name;
      union {
        double num_value;
        struct {
          size_t contents_len;
          struct SemiTokenTreeNode *nodes;
        } contents;
        size_t error_value;
      } value;
    } node;
    Token token;
  } content;
};

typedef struct SemiTokenTreeNode SemiTokenTreeNode;

SemiTokenTreeNode error_node(value) {
  SemiTokenTreeNode node;
  node.isToken = false;
  node.content.node.nodeType = STT_ERROR;
  node.content.node.value.error_value = value;
  return node;
}

// Turns a token sequence into a flat half-tree, and does some really basic parsing (number tokens to number nodes, variable tokens to variable nodes)
SemiTokenTreeNode create_top_node(Token *tokens, size_t tok_length) {
  SemiTokenTreeNode top_node;
  top_node.isToken = false;
  top_node.content.node.nodeType = STT_PARA;
  top_node.content.node.value.contents.contents_len = tok_length;
  top_node.content.node.value.contents.nodes = malloc(sizeof(SemiTokenTreeNode) * tok_length);

  for (size_t i = 0; i < tok_length; i++) {
    SemiTokenTreeNode *current_node = top_node.content.node.value.contents.nodes + i;
    if ((tokens + i)->token_type == NUM) {
      current_node->isToken = false;
      current_node->content.node.nodeType = STT_NUM;
      current_node->content.node.value.num_value = (tokens + i)->v.num_value;
    } else if ((tokens + i)->token_type == WORD) {
      current_node->isToken = false;
      current_node->content.node.nodeType = STT_VAR;
      current_node->content.node.name = strdup((tokens + i)->v.str);
    } else {
      current_node->isToken = true;
      current_node->content.token = *(tokens + i);
    }
  }

  return top_node;
}

struct parenthesisPair {
  size_t open_index;
  size_t close_index;
  size_t depth;
};

// Comparision function used in qsort
int compare_pair_depth(const void *a, const void *b) {
  size_t depth_a = ((struct parenthesisPair *)a)->depth;
  size_t depth_b = ((struct parenthesisPair *)b)->depth;
  return ((int) depth_a) - ((int) depth_b);
}

// Use pair-finding and depth-mapping to parse parentheses/functions
SemiTokenTreeNode parse_parentheses(SemiTokenTreeNode top_node) {
  size_t current_depth = 0;
  size_t *depth_map = malloc(sizeof(size_t) * top_node.content.node.value.contents.contents_len); // The parenthesis depth of every node in the list
  size_t *open_para_positions = malloc(sizeof(size_t) * top_node.content.node.value.contents.contents_len / 2); // Array of opening parentheses positions (first half of pair)
  size_t *close_para_positions = malloc(sizeof(size_t) * top_node.content.node.value.contents.contents_len / 2); // Array of closing parentheses positions (second half of pair)
  size_t j = 0;
  size_t *open_para_stack = malloc(sizeof(size_t) * top_node.content.node.value.contents.contents_len / 2); // LIFO stack of opening parenthesis positions
  size_t stack_pos = 0;

  // Match pairs and map depths
  for (size_t i = 0; i < top_node.content.node.value.contents.contents_len; i++) {
    *(depth_map + i) = current_depth;
    if (top_node.content.node.value.contents.nodes[i].content.token.token_type == OPEN_PARA) {
      *(open_para_stack + stack_pos++) = i;
      current_depth++;
    } else if (top_node.content.node.value.contents.nodes[i].content.token.token_type == CLOSE_PARA) {
      *(open_para_positions + j) = *(open_para_stack + stack_pos-- - 1);
      *(close_para_positions + j) = i;
      j++;
      current_depth--;
    }
  }

  // Handle if there are unclosed opening parentheses
  if (stack_pos) return error_node(ERR_UNCLOSED_PARENTHESIS);

  // Sort parenthesis pairs
  struct parenthesisPair *pairs = malloc(sizeof(struct parenthesisPair) * j);
  for (size_t i = 0; i < j; i++) {
    pairs[i].open_index = *(open_para_positions + i);
    pairs[i].close_index = *(close_para_positions + i);
    pairs[i].depth = *(depth_map + i);
  }
  qsort(pairs, j, sizeof(struct parenthesisPair), compare_pair_depth);
  free(open_para_positions);
  free(close_para_positions);
  free(open_para_stack);

  // Parse parentheses, one depth level at a time
  SemiTokenTreeNode *nodes = top_node.content.node.value.contents.nodes;
  current_depth = 0;
  size_t k = 0;
  size_t new_contents_length = top_node.content.node.value.contents.contents_len;
  
  while (k < j) {
    size_t open_index = (pairs + k)->open_index;
    size_t close_index = (pairs + k)->close_index;
    size_t internal_length = close_index - open_index - 1;
    // Collapse parenthesis pairs and contents into a single node
    SemiTokenTreeNode *inside_nodes = malloc(sizeof(SemiTokenTreeNode) * internal_length);
    memcpy(inside_nodes, nodes + open_index + 1, internal_length * sizeof(SemiTokenTreeNode));
    (nodes + open_index)->isToken = false;
    if ((nodes + open_index)->content.token.v.str != NULL) {
      (nodes + open_index)->content.node.name = (nodes + open_index)->content.token.v.str;
      (nodes + open_index)->content.node.nodeType = STT_FUNC;
    } else (nodes + open_index)->content.node.nodeType = STT_PARA;
    (nodes + open_index)->content.node.value.contents.nodes = inside_nodes;
    (nodes + open_index)->content.node.value.contents.contents_len = internal_length;
    memmove(nodes + open_index + 1, nodes + close_index + 1, (top_node.content.node.value.contents.contents_len - close_index - 1) * sizeof(SemiTokenTreeNode));
    new_contents_length -= internal_length + 1;
    
    // Move pair indexes to fit their current places
    if (k != j-1) {
      for (size_t i = k; i < j; i++) {
        if (pairs[i].open_index > pairs[k].open_index) {
          pairs[i].open_index -= internal_length + 1;
          pairs[i].close_index -= internal_length + 1;
        }
      }
    }
  
    k++;
  }

  SemiTokenTreeNode new_top_node;
  new_top_node.isToken = false;
  new_top_node.content.node.nodeType = STT_PARA;
  new_top_node.content.node.value.contents.nodes = nodes;
  new_top_node.content.node.value.contents.contents_len = new_contents_length;
  return new_top_node;
}

#define IS_FACT_TOK_NODE(node) (node.isToken && node.content.token.token_type == FACTORIAL_OP)
#define IS_OP_TOK_NODE(node, op) (node.isToken && node.content.token.token_type == BIN_OP && node.content.token.v.bin_op_type == op)
#define IS_NEG_TOK_NODE(node) (node.isToken && node.content.token.token_type == NEG_OP)

// Parse operations
// Order of operations: Factorials, Exponents, Multiplication, Division, Addition, Subtraction
SemiTokenTreeNode parse_operations(SemiTokenTreeNode top_node) {
  SemiTokenTreeNode *nodes = top_node.content.node.value.contents.nodes;
  size_t new_contents_len = top_node.content.node.value.contents.contents_len;
  size_t i = 0;

  // First, parsing inside of the parentheses
  while (i < new_contents_len) {
    if (!nodes[i].isToken && nodes[i].content.node.nodeType == STT_PARA) nodes[i] = parse_operations(nodes[i]);
    i++;
  }
  i = 0;

  // Factorials
  while (i < top_node.content.node.value.contents.contents_len) {
    if (i == top_node.content.node.value.contents.contents_len-1 ? 0 : IS_FACT_TOK_NODE(nodes[i+1])) {
      SemiTokenTreeNode factorial_node;
      factorial_node.isToken = false;
      factorial_node.content.node.nodeType = STT_OP;
      factorial_node.content.node.opType = STT_FACTORIAL;
      factorial_node.content.node.value.contents.nodes = malloc(sizeof(SemiTokenTreeNode));
      memcpy(factorial_node.content.node.value.contents.nodes, nodes + i, sizeof(SemiTokenTreeNode));
      nodes[i] = factorial_node;
      memmove(nodes + i + 1, nodes + i + 2, (new_contents_len-- - i - 1) * sizeof(SemiTokenTreeNode));
    } else i++;
  }

  
  // Exponents
  // Looking for value^value or value^-value
  // Going backwards through the tree to tag the exponent expressions in backwards order, thus collapsing them in descending order of depth within the chain of exponents
  i = new_contents_len-1;
  size_t j = 0;
  size_t *exp_start_positions = malloc(new_contents_len * sizeof(size_t));
  bool *is_negative_exponent = malloc(new_contents_len * sizeof(size_t));
  
  while (i > 1) {
    if (!nodes[i].isToken && IS_OP_TOK_NODE(nodes[i-1], EXP) && !nodes[i-2].isToken) {
      *(exp_start_positions + j) = i - 2;
      *(is_negative_exponent + j) = false;
      j++;
    }
    if (i > 2) if (!nodes[i].isToken && IS_NEG_TOK_NODE(nodes[i-1]) && IS_OP_TOK_NODE(nodes[i-2], EXP) && !nodes[i-3].isToken) {
      *(exp_start_positions + j) = i - 3;
      *(is_negative_exponent + j) = true;
      j++;
    }
    i--;
  }
  // Collapse exponent expressions into nodes
  i = 0;
  while (i < j) {
    size_t start_pos = *(exp_start_positions + i);
    
    SemiTokenTreeNode exponent_node;
    exponent_node.isToken = false;
    exponent_node.content.node.nodeType = STT_OP;
    exponent_node.content.node.opType = STT_EXP;
    exponent_node.content.node.value.contents.nodes = malloc(2 * sizeof(SemiTokenTreeNode));
    memcpy(exponent_node.content.node.value.contents.nodes, nodes + start_pos, sizeof(SemiTokenTreeNode));
    if (!(*(is_negative_exponent + i))) { // if it is x^y
      memcpy(exponent_node.content.node.value.contents.nodes + 1, nodes + start_pos + 2, sizeof(SemiTokenTreeNode));
      memmove(nodes + start_pos + 1, nodes + start_pos + 2, (new_contents_len - start_pos - 2) * sizeof(SemiTokenTreeNode));
      new_contents_len -= 2;
    } else { // if it is x^-y
      SemiTokenTreeNode negative_node;
      negative_node.isToken = false;
      negative_node.content.node.nodeType = STT_OP;
      negative_node.content.node.opType = STT_NEG;
      negative_node.content.node.value.contents.nodes = malloc(sizeof(SemiTokenTreeNode));
      memcpy(negative_node.content.node.value.contents.nodes, nodes + start_pos + 3, sizeof(SemiTokenTreeNode));
      *(exponent_node.content.node.value.contents.nodes + 1) = negative_node;
      memmove(nodes + start_pos + 1, nodes + start_pos + 2, (new_contents_len - start_pos - 3) * sizeof(SemiTokenTreeNode));
      new_contents_len -= 3;
    }
    *(nodes + start_pos) = exponent_node;
    i++;
  }

  // Negation
  i = top_node.content.node.value.contents.contents_len-1;
  while (i > 0) {
    if (!nodes[i].isToken && IS_NEG_TOK_NODE(nodes[i-1])) {
      SemiTokenTreeNode negative_node;
      negative_node.isToken = false;
      negative_node.content.node.nodeType = STT_OP;
      negative_node.content.node.opType = STT_NEG;
      negative_node.content.node.value.contents.nodes = malloc(sizeof(SemiTokenTreeNode));
    memcpy(negative_node.content.node.value.contents.nodes, nodes + i, sizeof(SemiTokenTreeNode));
      *(nodes + i - 1) = negative_node;
      if (i != top_node.content.node.value.contents.contents_len-1) memmove(nodes + i, nodes + i + 1, (new_contents_len - i) * sizeof(SemiTokenTreeNode));
      new_contents_len -= 1;
    }
    i--;
  }

  // Looping through the binary operations
  int op = DIV;
  while (op >= ADD) {
    i = 0;
    while (i < top_node.content.node.value.contents.contents_len-2) {
      if (!nodes[i].isToken && IS_OP_TOK_NODE(nodes[i+1], op) && !nodes[i+2].isToken) {
        SemiTokenTreeNode binary_node;
        binary_node.isToken = false;
        binary_node.content.node.nodeType = STT_OP;
        binary_node.content.node.opType = op; // enums are basically just numbers with names, and the order lines up for token operations and STT operations, so this hack works
        binary_node.content.node.value.contents.nodes = malloc(sizeof(SemiTokenTreeNode) * 2);
        memcpy(binary_node.content.node.value.contents.nodes, nodes + i, sizeof(SemiTokenTreeNode));
        memcpy(binary_node.content.node.value.contents.nodes + 1, nodes + i + 2, sizeof(SemiTokenTreeNode));
        memmove(nodes + i + 1, nodes + i + 2, (new_contents_len - i - 2) * sizeof(SemiTokenTreeNode));
        *(nodes + i) = binary_node;
        new_contents_len -= 2;
      }
      i++;
    }
    op--;
  }
  
  SemiTokenTreeNode new_top_node;
  new_top_node.isToken = false;
  new_top_node.content.node.nodeType = STT_PARA;
  new_top_node.content.node.value.contents.nodes = nodes;
  new_top_node.content.node.value.contents.contents_len = new_contents_len;
  return new_top_node;
}

void print_node(SemiTokenTreeNode node) {
  if (node.isToken) print_token(node.content.token);
  else {
    if (node.content.node.nodeType == STT_NUM) printf("{%g}", node.content.node.value.num_value);
    else if (node.content.node.nodeType == STT_VAR) printf("{%s}", node.content.node.name);
    else if (node.content.node.nodeType == STT_PARA) {
      putc('{', stdout);
      for (size_t i = 0; i < node.content.node.value.contents.contents_len; i++) {
        putc(' ', stdout);
        print_node(*(node.content.node.value.contents.nodes + i));
      }
      fputs(" }", stdout);
    } else if (node.content.node.nodeType == STT_FUNC) {
      printf("%s{ ", node.content.node.name);
      for (size_t i = 0; i < node.content.node.value.contents.contents_len; i++) {
        print_node(*(node.content.node.value.contents.nodes + i));
        putc(' ', stdout);
      }
      putc('}', stdout);
    } else if (node.content.node.nodeType == STT_ERROR) {
      fputs(error_message(node.content.node.value.error_value), stdout);
      return;
    } else if (node.content.node.nodeType == STT_OP) {
      if (node.content.node.opType == STT_FACTORIAL) {
        print_node(*node.content.node.value.contents.nodes);
        putc('!', stdout);
      } else if (node.content.node.opType == STT_NEG) {
        putc('{', stdout);
        putc('-', stdout);
        print_node(*node.content.node.value.contents.nodes);
        putc('}', stdout);
      } else {
        putc('{', stdout);
        print_node(*node.content.node.value.contents.nodes);
        putc(' ', stdout);
        switch (node.content.node.opType) {
          case STT_ADD:
            putc('+', stdout);
            break;
          case STT_SUB:
            putc('-', stdout);
            break;
          case STT_MUL:
            putc('*', stdout);
            break;
          case STT_DIV:
            putc('/', stdout);
            break;
          case STT_EXP:
            putc('^', stdout);
            break;
        }
        putc(' ', stdout);
        print_node(*(node.content.node.value.contents.nodes + 1));
        putc('}', stdout);
      }
    }
  }
}

int main(void) {
  size_t tok_length = 32;
  char *str = getstr();
  Token *tokens = tokenizer(str, &tok_length);
  print_tokens(tokens, tok_length);
  putc('\n', stdout);
  SemiTokenTreeNode top_node = create_top_node(tokens, tok_length);
  top_node = parse_parentheses(top_node);
  print_node(top_node);
  putc('\n', stdout);
  top_node = parse_operations(top_node);
  print_node(top_node);
  putc('\n', stdout);
  return 0;
}
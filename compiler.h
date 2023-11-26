#ifndef TERMCALC_COMPILER_H
#define TERMCALC_COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include "error_messages.h"
#include "vm.h"


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

double ch_to_digit(char ch);

Token *tokenizer(char *str, size_t *tokens_length);

void print_token(Token token);

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

SemiTokenTreeNode error_node(size_t value);

// Turns a token sequence into a flat half-tree, and does some really basic parsing (number tokens to number nodes, variable tokens to variable nodes)
SemiTokenTreeNode create_top_node(Token *tokens, size_t tok_length);

// Comparision function used in qsort
int compare_pair_depth(const void *a, const void *b);

// Use pair-finding and depth-mapping to parse parentheses/functions
SemiTokenTreeNode parse_parentheses(SemiTokenTreeNode top_node);

void print_node(SemiTokenTreeNode node);

// Parse operations
SemiTokenTreeNode parse_operations(SemiTokenTreeNode top_node);

// Compiles the complete AST into VM code
VM_Code compile(SemiTokenTreeNode node);

#endif
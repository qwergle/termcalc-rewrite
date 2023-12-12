#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include "vm.h"
#include "error_messages.h"
#include "compiler.h"
#include "graphing.h"

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

#define DIFFERENT_SCANF(format, ...) do { \
  char *input_string = getstr(); \
  rc = sscanf(input_string, format, __VA_ARGS__); \
} while (0)

void query_value_from_user(char *prompt, char *format_specifier, void *pointer) {
  fputs(prompt, stdout);
  fflush(stdout);
  int rc;
  DIFFERENT_SCANF(format_specifier, pointer);
  while (rc != 1) {
    fputs("Invalid input. Try again. ", stdout);
    fflush(stdout);
    DIFFERENT_SCANF(format_specifier, pointer);
  }
}

int main(void) {
  fputs("f(x) = ", stdout);
  fflush(stdout);
  size_t tok_length = 32;
  char *str = getstr();
  Token *tokens = tokenizer(str, &tok_length);
  SemiTokenTreeNode top_node = create_top_node(tokens, tok_length);
  top_node = parse_parentheses(top_node);
  top_node = parse_operations(top_node);
  VM_Code code = compile(top_node);

  graphing_options opts;
  query_value_from_user("Please input the width of the grid... ", "%lu", &(opts.width));
  query_value_from_user("Please input the height of the grid... ", "%lu", &(opts.height));
  opts.height /= 2;
  query_value_from_user("Please input the center of the X axis... ", "%lf", &(opts.x_offset));
  query_value_from_user("Please input the center of the Y axis... ", "%lf", &(opts.y_offset));
  query_value_from_user("Please input the scale... ", "%lf", &(opts.scale));
  opts.mode = 0;
  print_graph(code, opts);
  return 0;
}
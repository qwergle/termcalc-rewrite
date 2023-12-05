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


#define VARIABLES_LENGTH 3

// default variable map
variable_map default_variables() {
  char *names[] = {"pi", "e", "phi"};
  double values[] = {M_PI, M_E, (1.0 + sqrt(5.0)) / 2.0};
  variable_map ret = (variable_map) {malloc(VARIABLES_LENGTH * sizeof(char*)), malloc(VARIABLES_LENGTH * sizeof(double)), VARIABLES_LENGTH};
  memcpy(ret.variable_names, names, VARIABLES_LENGTH * sizeof(char*));
  memcpy(ret.variable_values, values, VARIABLES_LENGTH * sizeof(double));
  return ret;
}

// add a variable to the map
void add_variable(variable_map *map, char *name, double value) {
  char **names = malloc((map->length + 1) * sizeof(char*));
  double *values = malloc((map->length + 1) * sizeof(double));
  memcpy(names, map->variable_names, map->length * sizeof(char*));
  *(names + map->length) = name;
  memcpy(values, map->variable_values, map->length * sizeof(double));
  *(values + map->length) = value;
  free(map->variable_names);
  free(map->variable_values);
  map->variable_names = names;
  map->variable_values = values;
  map->length += 1;
}

// sets a variable within the map, or adds it if it doesn't already exist
void set_variable(variable_map *map, char *name, double value) {
  bool exists = false;
  for (size_t i = 0; i < map->length; i++) {
    if (strcmp(map->variable_names[i], name) == 0) {
      map->variable_values[i] = value;
      return;
    }
  }
  add_variable(map, name, value);
}

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

char *string_of_char(char ch, size_t len) {
  char *out = malloc(len + 1);
  *(out + len) = '\0';
  memset(out, ch, len);
  return out;
}

char **alloc_grid(size_t width, size_t height) {
  char **grid = malloc(height * sizeof(char*));
  int i = 0;
  while (i < height) {
    char *row = malloc(width + 1);
    *(row + width) = '\0';
    memset(row, ' ', width);
    *(grid + i) = row;
    i++;
  }
  return grid;
}

char *print_grid_to_string(char **grid, size_t width, size_t height) {
  char *out = malloc((1 + width + 2) * (height+2) + 1);
  *out = 0;
  strcat(out, "+");
  strcat(out, string_of_char('-', width));
  strcat(out, "+\n");
  for (size_t i = 0; i < height; i++) {
    strcat(out, "|");
    strcat(out, *(grid + i));
    strcat(out, "|\n");
  }
  strcat(out, "+");
  strcat(out, string_of_char('-', width));
  strcat(out, "+\n");
  return out;
}

void print_grid(char **grid, size_t width, size_t height) {
  fputs(print_grid_to_string(grid, width, height), stdout);
}

struct graphing_options {
  double x_offset;
  double y_offset;
  size_t width;
  size_t height;
  double scale;
  uint8_t mode;
};

typedef struct graphing_options graphing_options;

#define GRID_TO_X(n) (((double)(n) - (double)(opts.width/2)) * opts.scale + opts.x_offset)
#define Y_TO_GRID(n) ((n - opts.y_offset) / opts.scale + opts.height)

// generate a graph on a char-grid from a function and graph settings
char **render_function(VM_Code func, graphing_options opts) {
  char **grid = alloc_grid(opts.width, opts.height);
  for (size_t i = 0; i < opts.width; i++) {
    variable_map variables = default_variables();
    if (opts.mode == 0) {
      double x = GRID_TO_X(i);
      
      set_variable(&variables, "x", x);
      double output = Y_TO_GRID(VM_Exec(func, variables));
      if (!isnan(output)) {
        long long y = (long long) floor(output / 2);
        if ((y < opts.height) && (y >= 0)) {
          char gchar; 
          if (fmod(output, 2.0) > 4.0/3.0) gchar = '^';
          else if (fmod(output, 2.0) < 2.0/3.0) gchar = '.';
          else gchar = '-';
          *(*(grid + opts.height - y - 1) + i) = gchar;
        }
      }
    }
  }
  return grid;
}

void print_graph(VM_Code func, graphing_options opts) {
  print_grid(render_function(func, opts), opts.width, opts.height);
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
#ifndef TERMCALC_GRAPHING_H
#define TERMCALC_GRAPHING_H

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "vm.h"


#define VARIABLES_LENGTH 3

// default variable map
variable_map default_variables();

// add a variable to the map
void add_variable(variable_map *map, char *name, double value);

// sets a variable within the map, or adds it if it doesn't already exist
void set_variable(variable_map *map, char *name, double value);

char *string_of_char(char ch, size_t len);

char **alloc_grid(size_t width, size_t height);

char *print_grid_to_string(char **grid, size_t width, size_t height);

void print_grid(char **grid, size_t width, size_t height);

struct graphing_options {
  double x_offset;
  double y_offset;
  size_t width;
  size_t height;
  double scale;
  uint8_t mode;
};

typedef struct graphing_options graphing_options;

// generate a graph on a char-grid from a function and graph settings
char **render_function(VM_Code func, graphing_options opts);

void print_graph(VM_Code func, graphing_options opts);

#endif
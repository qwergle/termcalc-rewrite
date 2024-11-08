#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "vm.h"
#include "graphing.h"


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
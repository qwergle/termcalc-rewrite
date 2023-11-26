#ifndef TERMCALC_VM_H
#define TERMCALC_VM_H
#include <stddef.h>


enum operations_enum {VM_ADD, VM_SUB, VM_MUL, VM_DIV, VM_EXP, VM_NEG, VM_FACTORIAL};

struct VM_Object {
  enum {NUMBER, VARIABLE} objectType;
  union {
    double number;
    char *name;
  } value;
};

typedef struct VM_Object VM_Object;

struct VM_Instruction {
  enum {OPERATION, PUSH, FUNC} instructionType;
  unsigned int opType;
  char *func_name;
  VM_Object pushed;
};

typedef struct VM_Instruction VM_Instruction;

struct VM_Code {
  VM_Instruction *instructions;
  size_t length;
};

typedef struct VM_Code VM_Code;

struct variable_map {
  char **variable_names;
  double *variable_values;
  size_t length;
};

typedef struct variable_map variable_map;

double VM_Exec(VM_Code code, variable_map variables);
void VM_Print(VM_Code code);
#endif
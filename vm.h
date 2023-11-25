#include <stddef.h>

#define VARS_LEN 3
#define FUNCS_LEN 16
enum funcs_enum {LOG, SIN, COS, TAN, CSC, SEC, COT, ARCSIN, ARCCOS, ARCTAN, SQRT};
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

double VM_Exec(VM_Code code, char **var_names, double *var_values, size_t var_len);
void VM_Print(VM_Code code);
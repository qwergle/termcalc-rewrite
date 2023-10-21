#define VARS_LEN 3
#define FUNCS_LEN 16
enum funcs_enum {LOG, SIN, COS, TAN, CSC, SEC, COT, ARCSIN, ARCCOS, ARCTAN, SQRT};

struct VM_Object {
  enum {NUMBER, VARIABLE} objectType;
  union {
    double number;
    int variableIndex;
  } value;
};

typedef struct VM_Object VM_Object;

struct VM_Instruction {
  enum {OPERATION, PUSH, FUNC, FACTORIAL} instructionType;
  unsigned int opType;
  enum funcs_enum funcType;
  VM_Object pushed;
  double base;
};

typedef struct VM_Instruction VM_Instruction;

struct VM_Code {
  VM_Instruction *instructions;
  int length;
};

typedef struct VM_Code VM_Code;

double VM_Exec(VM_Code code, double fvars[]);
void VM_Print(VM_Code code);
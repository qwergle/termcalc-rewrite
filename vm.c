#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vm.h"

// Execute stack machine instructions
double VM_Exec(VM_Code code, double fvars[]) {
  size_t i = 0;
  double stack[512];
  int stack_pos = -1;
  while (i < code.length) {
    VM_Instruction instruction = *(code.instructions + i);
    if (instruction.instructionType == PUSH) {
      VM_Object object = instruction.pushed;
      if (object.objectType == NUMBER) {
        stack[++stack_pos] = object.value.number;
      } else if (object.objectType == VARIABLE) {
        stack[++stack_pos] = fvars[object.value.variableIndex];
      }
    } else if (instruction.instructionType == OPERATION) {
      register int op_type = instruction.opType;
      register double left = stack[stack_pos - 1];
      register double right = stack[stack_pos];
      stack_pos--;
      if (op_type == 0) {
        stack[stack_pos] = pow(left, right);
      } else if (op_type == 1) {
        stack[stack_pos] = left*right;
      } else if (op_type == 2) {
        stack[stack_pos] = left/right;
      } else if (op_type == 3) {
        stack[stack_pos] = left+right;
      } else if (op_type == 4) {
        stack[stack_pos] = left-right;
      } else if (op_type == 5) {
        stack[stack_pos] = fmod(left, right);
      }
    } else if (instruction.instructionType == FUNC) {
      register enum funcs_enum ftype = instruction.funcType;
      if (ftype == LOG) {
        stack[stack_pos] = stack[stack_pos] > 0 ? log(stack[stack_pos]) / log(instruction.base) : NAN;
      } else if (ftype == SIN) {
        stack[stack_pos] = sin(stack[stack_pos]);
      } else if (ftype == COS) {
        stack[stack_pos] = cos(stack[stack_pos]);
      } else if (ftype == TAN) {
        stack[stack_pos] = tan(stack[stack_pos]);
      } else if (ftype == CSC) {
        stack[stack_pos] = 1/sin(stack[stack_pos]);
      } else if (ftype == SEC) {
        stack[stack_pos] = 1/cos(stack[stack_pos]);
      } else if (ftype == COT) {
        stack[stack_pos] = 1/tan(stack[stack_pos]);
      } else if (ftype == ARCSIN) {
        stack[stack_pos] = asin(stack[stack_pos]);
      } else if (ftype == ARCCOS) {
        stack[stack_pos] = acos(stack[stack_pos]);
      } else if (ftype == ARCTAN) {
        stack[stack_pos] = atan(stack[stack_pos]);
      } else if (ftype == SQRT) {
        stack[stack_pos] = stack[stack_pos] >= 0 ? sqrt(stack[stack_pos]) : NAN;
      }
    } else if (instruction.instructionType == FACTORIAL) {
      stack[stack_pos] = tgamma(stack[stack_pos]);
    }
    i++;
  }
  return stack[0];
}

// Used for debugging
void VM_Print(VM_Code code) {
  size_t i = 0;
  while (i < code.length) {
    VM_Instruction instruction = *(code.instructions + i);
    if (instruction.instructionType == PUSH) {
      if (instruction.pushed.objectType == NUMBER) {
        printf("PUSH %g\n", instruction.pushed.value.number);
      } else {
        printf("PUSH VAR%i\n", instruction.pushed.value.variableIndex);
      }
    } else if (instruction.instructionType == OPERATION) {
      char *ops[] = {"EXP","MUL","DIV","ADD","SUB","MOD"};
      char *operation = ops[instruction.opType];
      printf("%s\n",operation);
    } else if (instruction.instructionType == FUNC) {
      enum funcs_enum ftype = instruction.funcType;
      if (ftype == LOG) {
        printf("LOGARITHM BASE%f \n", instruction.base);
      } else if (ftype == SIN) {
        printf("SINE\n");
      } else if (ftype == COS) {
        printf("COSINE\n");
      } else if (ftype == TAN) {
        printf("TANGENT\n");
      } else if (ftype == CSC) {
        printf("COSECANT\n");
      } else if (ftype == SEC) {
        printf("SECANT\n");
      } else if (ftype == COT) {
        printf("COTANGENT\n");
      } else if (ftype == ARCSIN) {
        printf("ARCSINE\n");
      } else if (ftype == ARCCOS) {
        printf("ARCCOS\n");
      } else if (ftype == ARCTAN) {
        printf("ARCTANGENT\n");
      } else if (ftype == SQRT) {
        printf("SQRT\n");
      }
    } else if (instruction.instructionType == FACTORIAL) printf("FACTORIAL\n");
    i++;
  }
}
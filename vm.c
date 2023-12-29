#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vm.h"


#define SIZE_T_MAX ((size_t)(-1))
#define STR_EQ(a,b) (strcmp(a,b) == 0)

// Execute stack machine instructions
double VM_Exec(VM_Code code, variable_map variables) {
  size_t i = 0;
  double *stack = malloc(code.length * sizeof(double));
  size_t stack_pos = SIZE_T_MAX;
  while (i < code.length) {
    VM_Instruction instruction = *(code.instructions + i);
    if (instruction.instructionType == PUSH) {
      VM_Object object = instruction.pushed;
      if (object.objectType == NUMBER) {
        stack[++stack_pos] = object.value.number;
      } else if (object.objectType == VARIABLE) {
        double value = NAN;
        for (size_t j = 0; j < variables.length; j++) {
          if (strcmp(*(variables.variable_names + j), object.value.name) == 0) {
            value = *(variables.variable_values + j);
            break;
          }
        }
        if (isnan(value)) return NAN; // if variable does not exist, return NAN
        else stack[++stack_pos] = value;
      }
    } else if (instruction.instructionType == OPERATION) {
      register int op_type = instruction.opType;
      if (stack_pos == SIZE_T_MAX) return NAN; // if nothing has been pushed, return NAN
      register double left = stack_pos >= 1 ? stack[stack_pos - 1] : stack[stack_pos]; // if only one item has been added to stack, that item, otherwise second-to-top on stack
      register double right = stack[stack_pos];
      if (op_type == VM_EXP) {
        stack[--stack_pos] = pow(left, right);
      } else if (op_type == VM_MUL) {
        stack[--stack_pos] = left*right;
      } else if (op_type == VM_DIV) {
        stack[--stack_pos] = left/right;
      } else if (op_type == VM_ADD) {
        stack[--stack_pos] = left+right;
      } else if (op_type == VM_SUB) {
        stack[--stack_pos] = left-right;
      } else if (op_type == VM_FACTORIAL) {
        stack[--stack_pos] = tgamma(right);
      } else if (op_type == VM_NEG) {
        stack[--stack_pos] = -right;
      }
    } else if (instruction.instructionType == FUNC) {
      char *fname = instruction.func_name;
      if (STR_EQ(fname, "log")) {
        stack[stack_pos] = stack[stack_pos] > 0 ? log10(stack[stack_pos]) : NAN;
      } else if (STR_EQ(fname, "ln")) {
        stack[stack_pos] = stack[stack_pos] > 0 ? log(stack[stack_pos]) : NAN;
      } else if (STR_EQ(fname, "sin")) {
        stack[stack_pos] = sin(stack[stack_pos]);
      } else if (STR_EQ(fname, "cos")) {
        stack[stack_pos] = cos(stack[stack_pos]);
      } else if (STR_EQ(fname, "tan")) {
        stack[stack_pos] = tan(stack[stack_pos]);
      } else if (STR_EQ(fname, "csc")) {
        stack[stack_pos] = 1/sin(stack[stack_pos]);
      } else if (STR_EQ(fname, "sec")) {
        stack[stack_pos] = 1/cos(stack[stack_pos]);
      } else if (STR_EQ(fname, "cot")) {
        stack[stack_pos] = 1/tan(stack[stack_pos]);
      } else if (STR_EQ(fname, "arcsin") || STR_EQ(fname, "asin")) {
        stack[stack_pos] = asin(stack[stack_pos]);
      } else if (STR_EQ(fname, "arccos") || STR_EQ(fname, "acos")) {
        stack[stack_pos] = acos(stack[stack_pos]);
      } else if (STR_EQ(fname, "arctan") || STR_EQ(fname, "atan")) {
        stack[stack_pos] = atan(stack[stack_pos]);
      } else if (STR_EQ(fname, "sqrt")) {
        stack[stack_pos] = stack[stack_pos] >= 0 ? sqrt(stack[stack_pos]) : NAN;
      } else if (STR_EQ(fname, "abs")) {
        stack[stack_pos] = fabs(stack[stack_pos]);
      } else if (STR_EQ(fname, "floor")) {
        stack[stack_pos] = floor(stack[stack_pos]);
      } else if (STR_EQ(fname, "ceil")) {
        stack[stack_pos] = ceil(stack[stack_pos]);
      } else if (STR_EQ(fname, "round")) {
        stack[stack_pos] = round(stack[stack_pos]);
      }
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
        printf("PUSH %s\n", instruction.pushed.value.name);
      }
    } else if (instruction.instructionType == OPERATION) {
      char *ops[] = {"ADD", "SUB", "MUL", "DIV", "EXP", "NEG", "FACTORIAL"};
      char *operation = ops[instruction.opType];
      printf("OP %s\n",operation);
    } else if (instruction.instructionType == FUNC) {
      char *fname = instruction.func_name;
      printf("FUNC %s\n",fname);
    }
    i++;
  }
}
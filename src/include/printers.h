/*
  printers.h

  printing cpu, registers, stack, memory
*/

#include <string.h>
#include <stdio.h>

#include <color.h>

//struct cpu_operator* getop();
void print_memory_page(int mem_index, int rows, int cols);
void print1(char *reg);
void print_stack();
void print_registers();
void print_tester();
void printm();
void print_register_fetch1();
void print_register_exec();
void print_register_exec1();
void src_print(int);
void print_opcodeinfo();
void print_opcodeinfo0();

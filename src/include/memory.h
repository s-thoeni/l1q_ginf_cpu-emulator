/*
  memory.h
*/


#define MEMORY_SIZE 65536 //Memory of 2^16 Memorycells
//#define MEMORY_SIZE 32   //Testing Purpose
#define MEMORY_CELL_SIZE 8 //8 character bits per cell
#define READ '1'
#define WRITE '0'
#define ADDR_WIDTH 16

void init_memory();

void access_memory();

void print_memory();

void print_memory_page(int,int,int);

void read_dump();

void cp_cell2memory(char *cell, int addr);

void number2register(int number, char* reg);

void print_memory_page0(int adr);

char *memory[65536];

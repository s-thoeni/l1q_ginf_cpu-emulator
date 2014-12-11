/*
  6502-cpu.h
  bho1 12.7.2011
  GPL
*/

#ifndef _6502_CPU_H_
#define _6502_CPU_H_

#define ADDR_WIDTH 16


typedef char* cpu_register;

extern int cycles;
extern int cycles_sum;

void set_rw2read();
void set_rw2write();
/*
  Registers used in 6502 CPU
*/

/*
  hidden register
*/
// Databus Register, shared with memory.c
//extern char *databus_register;
extern char *dbr;

// Addressbus Register Low byte, shared with memory.c
extern char *abrl;
// Addressbus Register High byte, shared with memory.c
extern char *abrh;
// Read/Write bit
extern char read_write;
//Instruction Register
extern char *ir;

/*
  User register
*/
// Accumulator
extern char *acc;

// index register
extern char *idx;
extern char *idy
;
//stack pointer SP
extern char *sp;

//programm counter
extern char *pcl;
extern char *pch;

//flags
extern char *flags;

// ultra helpers
extern char *zero, *one;

// intialize 6502 cpu
void cpu_6502_reset();

void fetch();

void* getop();

void *decode();

void trace();

struct cpu_state* execute(void (*fct)());

void fetch_decode_execute_cycle();

void cpu_6502_init();

/* 
   cpu_state list
 */

struct cpu_state {
  char  abrl[REG_WIDTH+1];
char abrh[REG_WIDTH+1];
  char dbr[REG_WIDTH+1];
  
char acc[REG_WIDTH+1];
char idx[REG_WIDTH+1];
char idy[REG_WIDTH+1];
char sp[REG_WIDTH+1];
char pcl[REG_WIDTH+1];
char pch[REG_WIDTH+1];
char flags[REG_WIDTH+1];
char ir[REG_WIDTH+1];
} ;// cpu_state;
struct cpu_state * save_base;
#define STATES_SIZE 10

#endif 
/* _6502_CPU_H_ */

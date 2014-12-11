/*
  decode.h previous operations.h
  bho1 2006
  GPL
*/

#include <stdlib.h>

struct cpu_operator {
  char *name;
  int opcode;
  void (* const opfunction)();  //const pointer to operator function
  char *adrmode;
  char *info;
  char *mnemonic;
  char *action;
  char *flagsettings;
};

// there are max 0xff operations
#define OPS 0xff 
//struct cpu_operator *operations;
extern struct cpu_operator operations[];

#define ADRM_IMP 1
#define ADRM_IMM 2
#define ADRM_REL 3
#define ADRM_IZX 4
#define ADRM_IZY 5
#define ADRM_ZP 6
#define ADRM_ZPX 7
#define ADRM_ZPY 8
#define ADRM_ABS 9
#define ADRM_ABX 10
#define ADRM_ABY 11
#define ADRM_IND 12

 /*  opid, bytes, cycles, cmd, adrmode_txt, adrmode_id, special */
struct ext_opcode { int opid; int bytes; int cycles; char *cmd; char *adrmode_txt; int adrmode_id; int special; };
extern struct ext_opcode ext_opcodetbl[];

size_t operations_array_size;



void print_op(struct cpu_operator*);

/*
cdecl> explain void (*lookup_operation(int))();
declare lookup_operation as function (int) returning pointer to function returning void
*/
void (*lookup_operation(int op_code))();

struct cpu_operator* operation_lookup(int op_code);


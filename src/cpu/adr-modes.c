/*
  adr-code.c
  bho1 27.12.2011

  GPL

*/
#include <stddef.h> // somehow we need NULL

#include "alu.h"
#include "alu-opcodes.h"
#include "register.h"
#include "memory.h"
#include "flags.h"
#include "cpu-util.h"
#include "cpu.h"

void proc_absolut(char* dummy){
  cp_register(pcl, abrl);
  cp_register(pch, abrh);
  
  set_rw2read();
  access_memory();
  
  cp_register(dbr,dummy);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  inc_pc();
}

void proc_zero_read(){
  char zero[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();
}

void txx(char *reg1, char* reg2){
  cp_register(reg1, reg2);
}

char* edata_abindex(char* index){
  char dummy[REG_WIDTH+1] = "xxxxxxxx";

  proc_absolut(dummy);

  cp_register(dbr,abrl);
  cp_register(dummy,abrh);

  alu(ALU_OP_ADD, abrl, index, abrl,NULL);
  set_rw2read();
  access_memory();


  return dbr;
}

char* edata_abs(){
  char dummy[REG_WIDTH+1] = "xxxxxxxx";

  proc_absolut(dummy);

  cp_register(dbr,abrl);
  cp_register(dummy,abrh);
  set_rw2read();
  access_memory();


  return dbr;
}

char* edata_imm(){

  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();

  inc_pc();
  return dbr;
}

char*edata_zp(){
  char zero[REG_WIDTH+1]="00000000";
  
  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();
  
  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  set_rw2read();
  access_memory();

  inc_pc();
  return dbr;
}

char*edata_zp2(){
  char zero[REG_WIDTH+1]="00000000";
  
  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();
  
  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  inc_pc();
  return dbr;
}

char*edata_zpindex(char*index){
  char zero[REG_WIDTH+1]="00000000";
  
  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD, dbr, index,dbr,NULL);
  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  inc_pc();
  return dbr;
}

char*edata_izx(){
  char zero[REG_WIDTH+1]="00000000";
  char dummy[REG_WIDTH+1]="00000000";

  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx,dbr,NULL);
  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr,dummy);
  inc_register(dbr);
  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr,abrl);
  cp_register(dummy,abrh);

  set_rw2read();
  access_memory();

  inc_pc();
  return dbr;
}

char*edata_izy(){
  char zero[REG_WIDTH+1]="00000000";
  char dummy[REG_WIDTH+1]="00000000";

  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr,dummy);
  inc_register(dbr);
  cp_register(dbr,abrl);
  cp_register(zero,abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idy,dbr,NULL);

  cp_register(dbr,abrl);
  cp_register(dummy,abrh);

  set_rw2read();
  access_memory();

  inc_pc();
  return dbr;
}

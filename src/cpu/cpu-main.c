/*

  6502-cpu-main.c
  bho 11.7.2011

  What?:
  reads memory-file and register-file and
  copies values into memory and cpu

  starts then the fetch-exec-cycle of cpu
  prints after each  cycle cpu-content and memory-range to stdout

  GPL applies

*/

#include <string.h>
#include <stdio.h>

#include "register.h"
#include "cpu.h"
#include "flags.h"
#include "alu.h"
#include "decode.h"
#include "memory.h"
#include "cpu-util.h"


//struct cpu_operator* getop();
void print1(char *reg){
  printf("%02x\n",conv_bitstr2int(reg,0,7));
}
void printflags(){
  printf("flags: \t%s %c%c%c%c%c\n", flags,
		 ((getOverflowflag(flags)=='1')?'V':'-'),
		 (getCarryflag(flags)=='1'?'C':'-'),
		 (getSignflag(flags)=='1'?'N':'-'),
		 (getZeroflag(flags)=='1'?'Z':'-'),
		 (getBRKflag(flags)=='1'?'B':'-')
		 );
}

void print_stack(){
  int spi = conv_bitstr2int(sp,0,7);
  int i;
  for(i=spi-2;i<spi+3;i++){
	printf("mem[%02x'%02x] = \t   %02x\n", 1, i%256, conv_bitstr2int(memory[i%256+256],0,7));
  }
}

void print_registers(){
  struct cpu_operator* optr;
  optr = (struct cpu_operator*) getop();

  printf("pc   AC XR YR SP NV-BDIZC \n");
  printf("%02x%02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c\n",
		 conv_bitstr2int(pch,0,7),
		 conv_bitstr2int(pcl,0,7),
		 conv_bitstr2int(acc,0,7),
		 conv_bitstr2int(idx,0,7),
		 conv_bitstr2int(idy,0,7),
		 conv_bitstr2int(sp,0,7),

		 getSignflag(flags),
		 getOverflowflag(flags),
		 '1',
		 getBRKflag(flags),
		 getDflag(flags),
		 getIRQflag(flags),
		 getZeroflag(flags),
		 getCarryflag(flags)
		 // conv_bitstr2int(ir,0,7),optr->name,optr->info
		 
		 );
}
void print_tester(){
  struct cpu_operator* optr;
  optr = (struct cpu_operator*) getop(memory[pc2int()]);

  printf("%04x %02X %s\n",pc2int(), conv_bitstr2int(memory[pc2int()],0,7),optr->name);
  print_registers();
}

 void printm(){

  int adr = pc2int();
  printf("\nmem[%02x'%02x] = \t   %02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  print_stack();
  printf("abrl:  \t\t   %02x\n",   conv_bitstr2int(abrl,0,7));
  printf("abrh:  \t\t   %02x\n",   conv_bitstr2int(abrh,0,7));
  printf("dbr:   \t\t   %02x\n",   conv_bitstr2int(dbr,0,7));
  printf("***********************************\n\n");
}
  
void print_register_fetch1(){
  struct cpu_operator* optr;
  printf("******************************\n     FETCH:\n******************************\n");
  printf("abrl:  \t\t   %02x\n",   conv_bitstr2int(abrl,0,7));
  printf("abrh:  \t\t   %02x\n",   conv_bitstr2int(abrh,0,7));
  printf("dbr:   \t\t   %02x\n",   conv_bitstr2int(dbr,0,7));
  printf("acc:   \t\t   %02x\n",   conv_bitstr2int(acc,0,7));
  printf("idx:   \t\t   %02x\n",   conv_bitstr2int(idx,0,7));
  printf("idy:   \t\t   %02x\n",   conv_bitstr2int(idy,0,7));
  printf("sp:    \t\t   %02x\n",   conv_bitstr2int(sp,0,7));
  printf("pch'pcl \t%02x'%02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7));

  int adr = pc2int();
  printf("mem[%02x'%02x] = \t   %02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  // printf("   %s %02x\n",   pch,conv_bitstr2int(pch,0,7));
  printflags();

  optr = (struct cpu_operator*) getop();
  printf("ir:    \t\t   %02x %s %s\n",   conv_bitstr2int(ir,0,7),optr->name,optr->info);
}

void print_register_exec(){
  struct cpu_operator* optr;
  printf("***********************************\n     EXEC:\n***********************************\n");
  printf("acc:   \t\t   %02x\n",   conv_bitstr2int(acc,0,7));
  printf("idx:   \t\t   %02x\n",   conv_bitstr2int(idx,0,7));
  printf("idy:   \t\t   %02x\n",   conv_bitstr2int(idy,0,7));
  printf("sp:    \t\t   %02x\n",   conv_bitstr2int(sp,0,7));
  printf("pch'pcl \t%02x'%02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7));

  printflags();

  int adr = pc2int();
  printf("\nmem[%02x'%02x] = \t   %02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  printf("abrl:  \t\t   %02x\n",   conv_bitstr2int(abrl,0,7));
  printf("abrh:  \t\t   %02x\n",   conv_bitstr2int(abrh,0,7));
  printf("dbr:   \t\t   %02x\n",   conv_bitstr2int(dbr,0,7));
  optr = (struct cpu_operator*) getop();
  printf("ir:    \t\t   %02x %s %s\n",   conv_bitstr2int(ir,0,7),optr->name,optr->info);
  printf("***********************************\n\n");
  
}

void print_register_exec1(){
  struct cpu_operator* optr;
  printf("******************************\n     EXEC:\n******************************\n");
  printf("abrl:  %s %02x\n",   abrl, conv_bitstr2int(abrl,0,7));
  printf("abrh:  %s %02x\n",   abrh,conv_bitstr2int(abrh,0,7));
  printf("dbr:   %s %02x\n",   dbr,conv_bitstr2int(dbr,0,7));
  printf("acc:   %s %02x\n",   acc,conv_bitstr2int(acc,0,7));
  printf("idx:   %s %02x\n",   idx,conv_bitstr2int(idx,0,7));
  printf("idy:   %s %02x\n",   idy,conv_bitstr2int(idy,0,7));
  printf("sp:    %s 1%02x\n",   sp,conv_bitstr2int(sp,0,7));
  printf("pcl:   %s %02x\n",   pcl,conv_bitstr2int(pcl,0,7));
  printf("pch:   %s %02x\n",   pch,conv_bitstr2int(pch,0,7));

  int adr = pc2int();
  printf("mem[%02x'%02x] = \t   %02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  printflags();//  printf("flags: %s \n",   flags);

  optr = (struct cpu_operator*) getop();
  printf("ir:    \t\t   %02x %s %s\n",   conv_bitstr2int(ir,0,7),optr->name,optr->info);//  printf("ir:    %s %s\n",   ir,optr->name);
}




/*
  main:
  options:
  --memory <memory-dump>
  Read file <memory-dump> and copy into memory
  --register <filename>
  copy content in <filename>  into corresponding register
*/
int main(int argc, char* argv[]){

  void (*mue_code_ptr)();
  
  // memory allocation 
  init_memory();
  cpu_6502_init();

  cpu_6502_reset();
  
  // handling parameter --memory <dumpfile> 
  if(argc==3){
	if(!strcmp(argv[1], "--memory")){
	  printf("Loading %s into memory\n", argv[2]);
      read_dump(argv[2]);
    }
  }
  // handling parameter --register <dumpfile> 
  if(argc==5){
	if(!strcmp(argv[3], "--register")){
	  printf("Loading %s into register\n", argv[2]);
	  cpu_6502_register_dump(argv[6]);
    }
  }

  // monitoring
  // init_monitor();

  //debug
  //  cp_register("10101010", sp);

  // fetch_decode_execute_cycle();


  /*
	
  fetch-exec cycle modes

  interactiv: scanf(%2x %2x %2x) and put the values
  into mem[pc] then start one cycle

  stepping: run one cycle, wait for any input, reiterate

  logging: write registers into file
  */
  while(1){
	// read input
	// parse input
	// do the stuff

    fetch();
    mue_code_ptr = decode();
	//_fetch();
    execute(mue_code_ptr);
	print_tester();//	print_registers();//print_register_exec();
  }


  //  exit_monitor();

  return 1;
}


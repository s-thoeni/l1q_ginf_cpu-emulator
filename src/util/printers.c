#include <string.h>
#include <stdio.h>

#include "register.h"
#include "cpu.h"
#include "flags.h"
#include "alu.h"
#include "decode.h"
#include "memory.h"
#include "cpu-util.h"
#include "printers.h"
#include "src-ripper.h"

void print_memory_page(int mem_index, int rows, int cols){
  int i,j;
  int pc = pc2int();
  printf("\n     ");
  for(i=0;i<cols;i++){
    printf("%02X ",(mem_index+i)%256);
  }
  i=0;
  for(j=0; j<rows; ++j) {
    printf("\n%04X ",	mem_index +j*cols);
    for(i=0; i<cols; ++i) {
	  if(pc==mem_index+ j*cols+i){
		printf("%02X ",	conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
		//		print_color(conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
	  //printf(RED "%02X " RESET ,conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
	  } else {
		//		print_color(conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
		printf("%02X ",	conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
	  }
    }
  }
   printf("\n");
}
//struct cpu_operator* getop();
void print1(char *reg){
  printf("%02X\n",conv_bitstr2int(reg,0,7));
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
	printf("mem[%02X'%02X] = \t   %02X\n", 1, i%256, conv_bitstr2int(memory[i%256+256],0,7));
  }
}

  
void print_registers_orig(){
  struct cpu_operator* optr;
  optr = (struct cpu_operator*) getop(ir);

  printf("pc   AC XR YR SP NV-BDIZC IR NME INFO MNEM action flags\n");
  printf("%02X%02x %02X %02X %02X %02X %c%c%c%c%c%c%c%c %02X %s  %s %s %s %s\n",
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
		 getCarryflag(flags),

		 conv_bitstr2int(ir,0,7),
		 optr->name,
		 optr->info,
		 optr->mnemonic,
		 optr->action,
		 optr->flagsettings
		 );
}

void print_registers(){
  printf(" PC  AC XR YR SP NV-BDIZC IR \n");
  printf("%02X%02X %02X %02X %02X %02X %c%c%c%c%c%c%c%c %02X \n",
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
		 getCarryflag(flags),  
		 
		 conv_bitstr2int(ir,0,7));
}
char *adrmode_tbl[11][3] = { { "imm", "#<value>", "imediate" },
						  { "imp", "", "implizit"},
						  { "abs", "<16Bit Address>", "absolute"},
						  { "zp", "<16Bit Address>", "absolute"},
						  { "rel", "<16Bit Address>", "absolute"},
						  { "izx", "<16Bit Address>", "absolute"},
						  { "izy", "<16Bit Address>", "absolute"},
						  { "zpx", "<16Bit Address>", "absolute"},
						  { "abx", "<16Bit Address>", "absolute"},
						  { "aby", "<16Bit Address>", "absolute"},
						  { "zpy", "<16Bit Address>", "absolute"}
						  
};

void print_opcodeinfo(int opcode){
    struct cpu_operator* optr;
	optr = operation_lookup(opcode);
	printf("CMD:    %s\nopcode: %02X\nfct:    %s\nflags: %s\n", optr?optr->mnemonic:"XXX", conv_bitstr2int(ir,0,7), optr->action, optr->flagsettings);

}

void print_opcodeinfo0(){
    struct cpu_operator* optr;
	optr = (struct cpu_operator*) getop(ir);
	printf("CMD:    %s\nopcode: %02X\nfct:    %s\nflags: %s\n", optr?optr->mnemonic:"XXX", conv_bitstr2int(ir,0,7), optr->action, optr->flagsettings);

}

/* void print_tester(){ */
/*   struct cpu_operator* optr; */
/*   //  optr = (struct cpu_operator*) getop(memory[pc2int()]); */
/*   optr = (struct cpu_operator*) getop(ir); */

/*   printf("%04x %02X %s\n",pc2int()-1, conv_bitstr2int(ir,0,7),optr?optr->name:"HELLNO"); */
/*   print_registers(); */
/* } */

 void printm(){

  int adr = pc2int();
  printf("\nmem[%02X'%02X] = \t   %02X\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  print_stack();
  printf("abrl:  \t\t   %02X\n",   conv_bitstr2int(abrl,0,7));
  printf("abrh:  \t\t   %02X\n",   conv_bitstr2int(abrh,0,7));
  printf("dbr:   \t\t   %02X\n",   conv_bitstr2int(dbr,0,7));
  printf("***********************************\n\n");
}
  
void print_register_fetch1(){
  struct cpu_operator* optr;
  printf("******************************\n     FETCH:\n******************************\n");
  printf("abrl:  \t\t   %02X\n",   conv_bitstr2int(abrl,0,7));
  printf("abrh:  \t\t   %02X\n",   conv_bitstr2int(abrh,0,7));
  printf("dbr:   \t\t   %02X\n",   conv_bitstr2int(dbr,0,7));
  printf("acc:   \t\t   %02X\n",   conv_bitstr2int(acc,0,7));
  printf("idx:   \t\t   %02X\n",   conv_bitstr2int(idx,0,7));
  printf("idy:   \t\t   %02X\n",   conv_bitstr2int(idy,0,7));
  printf("sp:    \t\t   %02X\n",   conv_bitstr2int(sp,0,7));
  printf("pch'pcl \t%02X'%02X\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7));

  int adr = pc2int();
  printf("mem[%02X'%02X] = \t   %02X\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  // printf("   %s %02X\n",   pch,conv_bitstr2int(pch,0,7));
  printflags();

  optr = (struct cpu_operator*) getop();
  printf("ir:    \t\t   %02X %s %s\n",   conv_bitstr2int(ir,0,7),optr->name,optr->info);
}

void print_register_exec(){
  struct cpu_operator* optr;
  printf("***********************************\n     EXEC:\n***********************************\n");
  printf("acc:   \t\t   %02X\n",   conv_bitstr2int(acc,0,7));
  printf("idx:   \t\t   %02X\n",   conv_bitstr2int(idx,0,7));
  printf("idy:   \t\t   %02X\n",   conv_bitstr2int(idy,0,7));
  printf("sp:    \t\t   %02X\n",   conv_bitstr2int(sp,0,7));
  printf("pch'pcl \t%02X'%02X\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7));

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
void print_state_array_short(struct cpu_state *cpu_state_array ){
  struct cpu_operator* optr;

  printf("cpu_state_array     acc idx idy sp pch pcl flags ir abrh abrl dbr ir name info   char *mnemonic; char *action;  char *flagsettings;\n");

  optr = (struct cpu_operator*) getop(cpu_state_array->ir);
  printf("%p                 %02x  %02x  %02x %02x  %02x  %02x    %02x %02x   %02x   %02x  %02x %02x %s %s %s  %s   %s\n     ",   
	 cpu_state_array,
 conv_bitstr2int(cpu_state_array->acc,0,7),
conv_bitstr2int(cpu_state_array->idx,0,7),
  conv_bitstr2int(cpu_state_array->idy,0,7),
 conv_bitstr2int(cpu_state_array->sp,0,7),
conv_bitstr2int(cpu_state_array->pch,0,7),
 conv_bitstr2int(cpu_state_array->pcl,0,7),
conv_bitstr2int(cpu_state_array->flags,0,7),
 conv_bitstr2int(cpu_state_array->ir,0,7),
   conv_bitstr2int(cpu_state_array->abrh,0,7),
 conv_bitstr2int(cpu_state_array->abrl,0,7),
conv_bitstr2int(cpu_state_array->dbr,0,7),
conv_bitstr2int(cpu_state_array->ir,0,7),
	 optr->name,
optr->info,  
optr->mnemonic,
optr->action,
optr->flagsettings);

  //  int adr = pc2int();  
}
void print_state_history_short(struct cpu_state * cpu_state_array){
  struct cpu_state *running = save_base;
  printf("***********************************\n    STATE ARRAY:\n***********************************\n");  
  printf("\trunning         save_base      cpu_state_array   save_base+MAX-1 \n");
  while( running < save_base+STATES_SIZE ){
    
    
    if(cpu_state_array == running){
      printf("===>  %p  \t%p\t     %p\t   %p\n", running, save_base,cpu_state_array, save_base+STATES_SIZE);   
      print_state_array_short(running);
    }
    else{
      printf("running %p  \t%p\t     %p\t   %p\n", running, save_base,cpu_state_array, save_base+STATES_SIZE);    
    }
    running++;
  }
}

void print_state_history(struct cpu_state * cpu_state_array){
  return  print_state_history(cpu_state_array);
  /*
  struct cpu_state *running = save_base;
  printf("***********************************\n    STATE ARRAY:\n***********************************\n");  
  printf(" save_base %p cpu_state_array : %p  save_base+MAX %p\n", save_base,cpu_state_array, save_base+STATES_SIZE);
while( running < save_base+STATES_SIZE ){
    
    if(cpu_state_array == running)
      printf("===> ");
    print_state_array_short(running);
    running++;
  }
  */
}

void print_register_short(){
  struct cpu_operator* optr;
  printf("***********************************\n     EXEC:\n***********************************\n");
  printf("acc idx idy sp pch pcl flags ir abrh abrl dbr ir name info\n");

  optr = (struct cpu_operator*) getop(ir);
  printf(" %02x  %02x  %02x %02x  %02x  %02x    %02x %02x   %02x   %02x  %02x %02x %s %s\n     ",   conv_bitstr2int(acc,0,7),conv_bitstr2int(idx,0,7),  conv_bitstr2int(idy,0,7), conv_bitstr2int(sp,0,7),conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7),conv_bitstr2int(flags,0,7), conv_bitstr2int(ir,0,7),   conv_bitstr2int(abrh,0,7), conv_bitstr2int(abrl,0,7),conv_bitstr2int(dbr,0,7),conv_bitstr2int(ir,0,7),optr->name,optr->info);

  int adr = pc2int();

  printf("\nmem[%02x'%02x] = \t   %02x\n", conv_bitstr2int(pch,0,7), conv_bitstr2int(pcl,0,7), conv_bitstr2int(memory[adr],0,7));

  
  //printf("ir:    \t\t   %02x %s %s\n",   conv_bitstr2int(ir,0,7),optr->name,optr->info);
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


/*

  parser.c
  bho1 - init 27.8.2012 

  Compile with  `sdl-config --cflags --libs`
  

  
*/
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>


#include "register.h"
#include "cpu.h"
#include "flags.h"
#include "alu.h"
#include "decode.h"
#include "memory.h"
#include "cpu-util.h"

#include "src-ripper.h"
#include "printers.h"
#include "parser.h"
#include "SDL.h"
#include "sdl-driver.h"

//#include "bitmap.h"

extern int videobase;

int mem_pager_adr = 0;
int mem_pager_rows = 4;
int mem_pager_cols = 16;

/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char *
rl_gets ()
{
  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline("mon> ");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history(line_read);

  return (line_read);
}


char * read_line(){
  return rl_gets ();
}

void ls(char *string){
  print_registers();
  //      print_register_short();
  //print_register_exec();
}

void until(char* string){
  void (*mue_code_ptr)();
  int adr;
  //  char* buffer;
  sscanf(string, " %*s %4x ", &adr);
  while(adr!=pc2int()){
	fetch();
	mue_code_ptr = decode();
	execute(mue_code_ptr);
	print_registers();
  }
}
void quit(char* string){
  printf("quit repl and cpu ..\n");
  SDL_Quit();
  exit(0);
}


#define ACC 0
#define ABRL 1
#define ABRH 2
#define DBR 3
#define IDX 4
#define IDY 5
#define PCL 6
#define PCH 7
#define IR 8
#define SP 9

char *names[] = { "acc", "abrl", "abrh", "dbr","idx" ,"idy", "pcl", "pch", "sp", NULL};
int registers_cmp(const char* string,const char* stringb){
  if(string[0]=='r' && string[1]=='e' && string[2]=='g' && string[3]==' ')
	return 0;
  else
	return -1;
}


/*
  register setting:
  <reg-name> <value>
  <reg-name> ::= idx idy idz acc, pc abrl abrh ir
      
*/
void registers(char* string){

  char regname[5] = "1234";
  char buffer_reg[REG_WIDTH+1] = "00000000";
  int key, content=-1;
  int i;

  sscanf(string, " %*s %s %2x ", &(regname[0]), &content);

  key = -1;
  for(i=0;names[i]!=NULL;i++){
	if(strcmp(regname,names[i])==0){
	  key=i;
	  break;
	}
  }
  if(content!=-1){
	number2register(content, buffer_reg);
	switch(key)
	  {
	  case ACC:
		cp_register(buffer_reg, acc);
		break;
	  case ABRL:
		cp_register(buffer_reg, abrl);
		break;
	  case ABRH:
		cp_register(buffer_reg, abrh);
		break;
	  case DBR:
		cp_register(buffer_reg, dbr);
		break;
	  case IDX:
		cp_register(buffer_reg, idx);
		break;
	  case IDY:
		cp_register(buffer_reg, idy);
		break;
	  case PCL:
		cp_register(buffer_reg, pcl);
		break;
	  case PCH:
		cp_register(buffer_reg, pch);
		break;
	  case IR:
		cp_register(buffer_reg, ir);
		break;
	  case SP:
		cp_register(buffer_reg, sp);
		break;
	  default:
		break;
	  }
  }
  print_registers();
}

int step_cmp(const char* string,const char *stringb){
  if(string[0]=='s' && string[1]=='t' && string[2]=='e' && string[3]=='p' )
	return 0;
  else
	return 1;
} 

void print_disasm_cmd(int pc){

  char *cmd_name =  ext_opcodetbl[conv_bitstr2int(memory[pc],0,7)].cmd;
  int mem0, mem1, mem2,a,x,y,sp_int;
  mem0 =  conv_bitstr2int(memory[pc],0,7);
  mem1 = conv_bitstr2int(memory[pc+1],0,7);
  mem2 =  conv_bitstr2int(memory[pc+2],0,7);

  a = conv_bitstr2int(acc,0,7);
  x = conv_bitstr2int(idx,0,7);
  y = conv_bitstr2int(idy,0,7);
  sp_int = conv_bitstr2int(sp,0,7);

  printf("                           AC XR YR SP NV-BDIZC\n");

  switch (ext_opcodetbl[conv_bitstr2int(ir,0,7)].adrmode_id){
  case ADRM_IMP: {
    printf("%04x %02x       %s          ", pc, mem0, cmd_name);
    break;
  }
  case ADRM_IMM: {
    printf("%04x %02x %02x %s #$%02x             ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_REL: {
    printf("%04x %02x %02x %s $%02x         ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_IZX: {
    printf("%04x %02x %02x %s ($%02x,X)          ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_IZY: {
    printf("%04x %02x %02x %s ($%02x),Y          ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_ZP: {
    printf("%04x %02x %02x %s $%02x         ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_ZPX: {
    printf("%04x %02x %02x %s $%02x,X            ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_ZPY: {
    printf("%04x %02x %02x %s $%02x,Y            ", pc, mem0, mem1, cmd_name, mem1);
    break;
  }
  case ADRM_ABS: {
    printf("%04x %02x %02x %02x %s $%02x%02x    ", pc, mem0, mem1, mem2, cmd_name, mem2, mem1);
    break;
  }
  case ADRM_ABX: {
    printf("%04x %02x %02x %02x %s $%02x%02x,X  ", pc, mem0, mem1, mem2, cmd_name, mem2, mem1);
    break;
  }
  case ADRM_ABY: {
    printf("%04x %02x %02x %02x %s $%02x%02x,Y  ", pc, mem0, mem1, mem2, cmd_name, mem2, mem1);
    break;
  }
  case ADRM_IND: {
    printf("%04x %02x %02x %02x %s $%02x%02x,Y  ", pc, mem0, mem1, mem2, cmd_name, mem2, mem1);
    break;
  }

  default: {
    break;
  }

  }
  printf("%02x %02x %02x %02x %s\n", a, x, y, sp_int, flags);


}

void step(char* string){
  void (*mue_code_ptr)();
  int i, steps=1;
  //  int srcpc;

  int pc, mem1, mem2, mem3, a, x, y;
  char optxt[100] = "";
  sscanf(string, " %*s %i ", &steps);
  for(i=0;i<steps;i++){
   	pc = pc2int();
	fetch();
	//	pc = pc2int();
	//	srcpc = pc2int();
	mue_code_ptr = decode();
	execute(mue_code_ptr);

	mem1 = conv_bitstr2int(memory[pc],0,7);
	mem2 = conv_bitstr2int(memory[pc+1],0,7);
	mem3 = conv_bitstr2int(memory[pc+2],0,7);

	a = conv_bitstr2int(acc,0,7);
	x = conv_bitstr2int(idx,0,7);
	y = conv_bitstr2int(idy,0,7);

	printf("\n*******************\n");
	print_disasm_cmd(pc);


	printf("%04x %02x %02x %02x %15s A: %2x X: %2x Y: %2x \n", pc, mem1, mem2, mem3, optxt, a, x, y);

	printf("\n*******************\n");
	print_opcodeinfo0();

	print_registers();
	printf("\n");

	//	print_src_line(srcpc);
	//	print_src_line(pc);

	/*
	for(j=0;i<10;j++){
	  
	  src_print(pc+j-5);
	}
	*/
	//			  print_memory_page2(0);

 	//int adr ; //= pc -(mem_pager_rows/2);

	//	adr = pc - (mem_pager_rows/2-1)*16;
	
	//	print_memory_page(adr, mem_pager_rows, mem_pager_cols);

	print_memory_page(mem_pager_adr, mem_pager_rows, mem_pager_cols);
	//monitor_print_mem(mem_pager_adr, mem_pager_rows, mem_pager_cols);
	printf("\n*******************\n");
	

	draw_bitmap_memory(videobase);        
  }
}

int memory_cmp(const char* string,const char *stringb){
  if(string[0]=='m' && string[1]=='e' && string[2]=='m' && string[3]==' ')
	return 0;
  else
	return 1;
} 

/*
  memory
  memarr <adr1> <adr2>
  mem <adr> <value>
  mem <adr>
      
      
*/
void memory_cmd(char* string){

  int adr=-1;
  unsigned int content=256;
  char reg[REG_WIDTH+1];

  sscanf(string, "%*s %4x %2x ", &adr, &content);
  if(adr>MEMORY_SIZE || adr < 0){
	printf("error: address to big \n");
  }else
	{
	  if(content!=256){
		number2register(content,reg); 
		cp_cell2memory(reg, adr);
	  }
	  print_memory_page0(adr);
	}
}

int set_cmp(const char* string,const char *stringb){
  while(*string &&*stringb){
	if(*string++!=*stringb++)
	  return 1;
  }
  return 0;
}
void* var_tbl[10][2] = { { "mem_pager_adr", &mem_pager_adr },
						 { "mem_pager_rows", &mem_pager_rows },
						 { "mem_pager_cols", &mem_pager_cols },
						 { NULL, NULL}
};

int set_name_cmp(const char* string,const char *stringb){
  if(string && stringb){
	while(*string &&*stringb){
	  if(*(string++)!=*(stringb++))
		return 1;
	}
	return 0;
  } else {
	return 1;
  }
}

char* parse_6502_register_name(char* name){

  if(!strcmp(name,"a")){
	return acc;
  }
  if(!strcmp(name,"idx")){
	return idx;
   }

  if(!strcmp(name,"idy")){
	return idy;
	}
  if(!strcmp(name,"pch")){
	return pch;
  }
  if(!strcmp(name,"pcl")){
	return pcl;
  }
  if(!strcmp(name,"sp")){
	return sp;
  }
  if(!strcmp(name,"flg")){
	return flags;
  }
  return NULL;
}

void set_cmd(char* string){
  unsigned int value;
  char name[30];
  char*cp;
  int *ip;
  sscanf(string, "%*s %s %x", name, &value);
  int i=0;
  for(i=0;var_tbl[i][0]!=0;i++){
	cp=var_tbl[i][0];
	if(!set_name_cmp(name, cp)){
	  ip = var_tbl[i][1];
	  *ip =  value;
	  break;
	}
  }
}

struct map {
  void (*eval_fctptr)(char* string); 
  char cmd[200];
  int (*cmp_fctptr)(const char* string,const char*cmd); 
};

struct map ptable[20] = { // { .eval_fctptr=&ls, "qwe", .cmp_fctptr=&strcmp}
  { &ls , "ls", &strcmp },
  { &step, "step", &step_cmp},
  { &quit, "quit", &strcmp},
  { &registers, "reg", &registers_cmp},
  { &memory_cmd, "mem", &memory_cmp},
  { &set_cmd, "set", &set_cmp}
};
					  
void repl(){
  int i;

  char last_cmd[30] = "ls";
  
  char *  string;


  string = read_line();
 
  int last_i;

  //init_monitor();

	
  while(string && strcmp(string,"exit")!=0){

	if(string[0]==0){
	  printf("%s\n", last_cmd);
	  (*ptable[last_i].eval_fctptr)(last_cmd);
	} else {
	  for(i=0;i<6;i++){
		if( (*(ptable[i].cmp_fctptr))(string, ptable[i].cmd) == 0) {
		  (*ptable[i].eval_fctptr)(string);
		  last_i=i;
		  strcpy(last_cmd,string);
		  break;
		}
	  }
	}
	
	draw_bitmap_memory(videobase);        

	string = read_line();
	
  }
}


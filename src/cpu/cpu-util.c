/*
  6502-cpu-util.c

  bho1 12.7.2011

  GPL applies
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "register.h"
#include "cpu.h"
#include "parser.h"
#include "cpu-util.h"
#include "memory.h"

int pc2int(){
  int i,j;
  i = conv_bitstr2int(pcl, 0, 7);
  j = conv_bitstr2int(pch, 0, 7);
  j = (j<<8) + i;
  return j;
}


/*
  file format:
  <name> <hex-value>
*/
void cpu_6502_register_dump(char* fname){
  char register_name_buffer[9];
  char hex_buffer[9];
  FILE *stream;
  hex_buffer[9] = '\0';
  register_name_buffer[9] = '\0';

  if(!strcmp("-", fname)){
    stream = stdin;
  } else if ((stream = fopen( fname,"r")) == NULL){
    printf("Can't open %s\n", fname);
    exit(1);
  }
  
  while(!feof(stream)){
    //format string is: <any space, including 0><three chars><any space, including 0><value>
    fscanf(stream, " %3c %4c  ", register_name_buffer, hex_buffer);
	// todo: check for interity
	//  printf("Illegal value %i\nSkipping line ..\n", adr);
	ldhex2register(hex_buffer, parse_6502_register_name(register_name_buffer));
  }
}


    
void set_rw2read(){
  read_write = READ;
}

void set_rw2write(){
  read_write = WRITE;
}

void inc_pc(){
  int i= REG_WIDTH-1;
  while(i>-1){
	if(pcl[i]=='0'){
	  pcl[i] = '1';
	  return;
	}else
	  pcl[i] = '0';
	i--;
  }
  i = REG_WIDTH-1;
  while(i>-1){
	if(pch[i]=='0'){
	  pch[i] = '1';
	  return;
	}else
	  pch[i] = '0';
	i--;
  }
  
  //  if(i==-1)
  //	printf("Error: PC reached End Of Memory");
}

void dec_pc(){
  int i= REG_WIDTH-1;
  while(i>-1){
	if(pcl[i]=='1'){
	  pcl[i] = '0';
	  return;
	}else
	  pcl[i] = '1';
	i--;
  }
  i = REG_WIDTH-1;
  while(i>-1){
	if(pch[i]=='1'){
	  pch[i] = '0';
	  return;
	}else
	  pch[i] = '1';
	i--;
  }
  
  //  if(i==-1)
  //	printf("Error: PC reached End Of Memory");
}

void dec_2register(char *bitstr_high, char *bitstr_low){
  int i= REG_WIDTH-1;
  while(i>-1){
	if(bitstr_low[i]=='1'){
	  bitstr_low[i] = '0';
	  return;
	}else
	  bitstr_low[i] = '1';
	i--;
  }
  i = REG_WIDTH-1;
  while(i>-1){
	if(bitstr_high[i]=='1'){
	  bitstr_high[i] = '0';
	  return;
	}else
	  bitstr_high[i] = '1';
	i--;
  }
}



void inc_register(char* reg){
  int i= REG_WIDTH-1;
  while(i>-1){
	if(reg[i]=='0'){
	  reg[i] = '1';
	  break;
	}else
	  reg[i] = '0';
	i--;
  }
  //if(i==-1)
  //printf("Error: PC reached End Of Memory");
}

void dec_register(char* reg){
  int i= REG_WIDTH-1;
  while(i>-1){
	if(reg[i]=='1'){
	  reg[i] = '0';
	  break;
	}else
	  reg[i] = '1';
	i--;
  }
  /* if(i==-1) */
  /* 	printf("dec_register: wrapping End Of Memory\n"); */
}
	

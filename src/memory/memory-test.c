/*
  memory-test.c
  bho 2007
  GPL
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "register.h"
#include "memory.h"

char * databus_register;
char * addressbus_register;
char * read_write;
                

char *testmemory[MEMORY_SIZE] =
  { //"11011100" //reset
    //"00000000"
    "00000101", //mov BX, [CX]
    "10000000",
    "00101001", //mov #0xAFFA, AX 
    "00000000",
    "01011111", //0xAF
    "11110101", //0xFA
    "00000101", //mov AX, BX
    "10000000"
  };


void mem_copy_content(char * content, unsigned int adr){
  int i;

  for(i=0; i<8; ++i){
    *(memory[adr]+i) = content[i];
  }
}  

void mem_dumpin(char * cmd_line){
  unsigned int address;
  char content[100];

  int nargs = 0;
  
  nargs = sscanf (cmd_line, "%x %s", &address, content);
  if (nargs==2){
    mem_copy_content(content, address);
  }
}


void mem_copy_register(char * reg, unsigned int adr){
  int i;

  for(i=0; i<8; ++i){
    *(memory[adr]+i) = reg[i];
  }
}  

int main (){

  databus_register = (char *) malloc (16 + 1);  
  addressbus_register = (char *) malloc (16 + 1);  
  read_write = (char *) malloc (1);  

  int nbytes = 100;
  char *cmd_line = (char *) malloc (nbytes + 1);  

  int i,j;  

  init_memory();


  for(i=0; i<8; ++i)
    {
      for(j=0; j<8; ++j){
	*(memory[i]+j) = *(testmemory[i]+j);
      }
    }

  print_memory_page(0);

  printf("Memory size is %i\n", MEMORY_SIZE);
  printf("Cell size is %i\n", MEMORY_CELL_SIZE);
  printf("Register size is %i\n", REG_WIDTH);
  while(getline(&cmd_line, &nbytes, stdin) != -1) {
    mem_dumpin(cmd_line);
  }

  print_memory_page(0);


  for(i=0; i<8; ++i)
    {
      cp_memory2dbr(i, databus_register);
      for(j=4; j<8; ++j){
	*(databus_register + j) = '0';
      }
      cp_dbr2memory(i, databus_register);
    }

  print_memory_page(0);

  char a[17] = "0000000000000000";
  char b[17] = "0000000000000111";
  char c[17] = "1111000011110000";
  printf("adress %x %s\n",conv_addr2int(a), a);
  printf("adress %x %s\n",conv_addr2int(b), b);
  printf("adress %x %s\n",conv_addr2int(c), c);

  for(i=0; i<16; ++i){
    addressbus_register[i] = a[i];
  }
  print_reg(addressbus_register);
  *read_write = READ;
  access_memory();
  print_reg(databus_register);

  for(i=0; i<16; ++i){
    addressbus_register[i] = b[i];
  }
  print_reg(addressbus_register);
  *read_write = WRITE;
  access_memory();
  print_reg(databus_register);

  for(i=0; i<16; ++i){
    addressbus_register[i] = b[i];
  }
  print_reg(addressbus_register);
  *read_write = READ;
  access_memory();
  print_reg(databus_register);


  print_memory_page(0);

  free_memory();
  return 1;
}


  

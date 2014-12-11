/*
  memory.c

  Here it is where little endian and big endian is done
  
  bo 2007
  bho1 10.12.2009 added read_dump and finally put a 0x0 at the end of a cell and allocating 8+1 chars for a cell
  bho1 9.12.2010 Added some more/better comments
  bho1: 26.11.2011
  made it work with 8 Bit architecture where 16Bit address is built with abrh (address bus register high)  and abrl (address bus register low) Bytes 

  in emulator:
  memory is a big array of characterbytes 

   76543210
   ________
  |________| 0
  |________| 1
  |________| 2
  |________| 3
  
  etc.
  
  in reality:
  
  A array of 2^16 char Pointers 
   ________
  |________|-------->zero terminated char array = mem cell 
  
  bho1 12.1.2013 adding shared memory for IPC
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "math.h"

#include "register.h"
#include "memory.h"
#include "cpu-util.h"
#include "cpu.h"

// shared mem specific
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>


/*
  interface to  external world
  we dont have a bus, so we just access
  databus register and address bus register
  directly
*/

extern char * dbr;
extern char * abrl;
extern char * abrh;
extern char  read_write;



extern int keyboardchar;

/*
  memory is an array of pointer to chars, i.e.
  memory[i] points to the ith byte whereas
  a byte is a zero-terminated char array of 8 bits whereas
  a bit is a '1' or '0' char representing a 1 or 0 resp.

  memory[i][j] is a '1' or '0' char, representing
  the ith Byte therein the (7-j)th Bit 
*/
char *memory[MEMORY_SIZE];

/*
  The very last 6 bytes of memory $FFFA to $FFFF of a 6502 processor should be:
non-maskable interrupt handler of 6502 processor ($FFFA ~ $FFFB),
the power on reset location of 6502 processor ($FFFC ~ $FFFD),
  the BRK/interrupt request handler of 6502 processor ($FFFE ~ $FFFF)
*/
#define BREAK_IRQ_HANDLER  0x0600
#define RESET_IRQ_HANDLER  0x0600
#define NMI_IRQ_HANDLER   0x0600

#define BREAK  0xFFFE
#define RESET  0xFFFC
#define NMI    0xFFFA

/* Allocates memory 
 64KB RAM
 32B CIA register
*/
void init_shared_memory(){
  
  int shmid,i,j;
  key_t key;
  char *shm;
  size_t shmsz =  MEMORY_SIZE * (MEMORY_CELL_SIZE+1) * sizeof(char);

  /*
   * name  shared memory segment
   * "5678"
   */
  key = 5678;


  /*
    Create the segment.
  */
  if ((shmid = shmget(key, shmsz, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
  }
  
  /*
    Now we attach the segment to our data space.
  */
  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }
  for( i=0; i<MEMORY_SIZE; ++i ){
    memory[i] = shm+9*i;
    // init memory to zero '0'
    for( j=0 ; j<MEMORY_CELL_SIZE ; j++)
      memory[i][j]= '0';
    memory[i][j] = 0x0; 
  }


}


void init_memory_classic()
{
  
  srand (time (0));
  int i,j;
  for( i=0; i<MEMORY_SIZE; ++i ){
	memory[i] = (char*)malloc(sizeof(char)*(MEMORY_CELL_SIZE+1));
	// init memory to zero '0'
	for( j=0 ; j<MEMORY_CELL_SIZE ; j++)
	  memory[i][j]= '0';
	memory[i][j] = 0x0; 
  }
}

void init_memory(){
  //  init_memory_classic();
  init_shared_memory();
}

/*
  Frees memory
 */
void free_shared_memory(){

}

void free_memory(){
  // free_memory_classic();
}

void free_memory_classic()
{
	int i=0;
	for( ; i<MEMORY_SIZE; ++ i )
		free(memory[i]);
}

/*
  conversion routines:
  From string representation to int representation 
  and vice versa
*/

int conv_byte2int(char*byte){
  int i=0;
  int sum = 0;
  for(i=0; i<MEMORY_CELL_SIZE;i++)
    if(byte[i] == '1'){
      sum = (sum << 1) + 1;
    }else
      sum = sum << 1;
  return sum;
}

int conv_addr2int(char* addr){
  int i=0;
  int sum = 0;
  for(i=0; i<ADDR_WIDTH;i++)
    if(addr[i] == '1'){
      sum = (sum << 1) + 1;
    }else
      sum = sum << 1;
  return sum;
}

/*
  void cp_dbr2memory(int addr, char * source)
  copy content of databusregister into memorycell at memory index addr
*/
void cp_dbr2memory(int addr, char * source){
  int i;
  for(i=0;i<REG_WIDTH;i++){
    *(memory[addr+i/MEMORY_CELL_SIZE]+i%MEMORY_CELL_SIZE) = source[i] ;
  }
}

void cp_byte2mem(char * source,int addr){
  int i;
  for(i=0;i<REG_WIDTH;i++){
    *(memory[addr+i/MEMORY_CELL_SIZE]+i%MEMORY_CELL_SIZE) = source[i] ;
  }
}

void cp_int2mem(int content, int address){
  char byte[REG_WIDTH];
  number2register(content, byte);
  cp_byte2mem(byte,address);
}

void cp_adr2mem(int content, int address){
  char byte[REG_WIDTH];

  number2register(content|0x00FF, byte);
  cp_byte2mem(byte,address);

  number2register(0x00FF| (content>>8), byte);
  cp_byte2mem(byte,address);
}

	
/*
  void cp_memory2dbr(int addr, char * target)
  copy content of memorycell at index addr into databusregister
*/
void cp_memory2dbr(int addr, char * target){
  int i;

  for(i=0;i<REG_WIDTH;i++){    
    target[i] = *(memory[addr+i/MEMORY_CELL_SIZE]+i%MEMORY_CELL_SIZE) ;
  }

  /* This version should work too (not tested though)
  for(i=0;i<REG_WIDTH;i++){    
    target[i] =  memory[adr][i];
  *(memory[addr+i/MEMORY_CELL_SIZE]+i%MEMORY_CELL_SIZE) ;
  }

  for(i=0;i<REG_WIDTH/MEMORY_CELL_SIZE; i++)
	for(j=0; j<MEMORY_CELL_SIZE; j++)
	target[i*MEMORY_CELL_SIZE+j] = memory[addr+i][j];
  */
}

/*
  memory_read: copy content of memoryaddress in addressbus register <abr> into 
  databus register dbr
*/
void memory_read(){
  int rnd;
  int i;
  char dummy[2*REG_WIDTH+1];
  
  // zero page hack
  // 0xfe is random number
  rnd = rand()%REG_WIDTH;
  number2register(rnd,memory[0x00fe]);
  // 0xff is char from keyboard
  number2register(keyboardchar,memory[0x00ff]);

  cp_adr2mem( BREAK_IRQ_HANDLER , BREAK );
  cp_adr2mem(NMI_IRQ_HANDLER,NMI);
  cp_adr2mem(RESET_IRQ_HANDLER,RESET);

  for(i=0;i<REG_WIDTH;i++){
	dummy[i]=abrh[i];
	dummy[i+REG_WIDTH]=abrl[i];
  }
  dummy[2*REG_WIDTH]='\0';


  
  int  startaddr = conv_addr2int(dummy);
  cp_memory2dbr(startaddr, dbr);
  
}

/*
  memory_write: copy content of databus_register into content of 
  memoryaddress <addressbus_register>
*/
void memory_write(){
  char dummy[2*REG_WIDTH+1];
  int i;
  for(i=0;i<REG_WIDTH;i++){
	dummy[i]=abrh[i];
	dummy[i+REG_WIDTH]=abrl[i];
  }
  dummy[2*REG_WIDTH]='\0';
  
  int  startaddr = conv_addr2int(dummy);

  cp_dbr2memory(startaddr,dbr);
}

/*
  access memory (read/write) according to the read_write bit setting
*/
void access_memory(){
  if(read_write==READ)
    memory_read();
  else
    memory_write();
}


void print_memory_page0(int mem_index){
  int rows = 6;
  int cols = 16;
  int i,j;
  printf("\n     ");
  for(i=0;i<cols;i++){
    printf("%02x ",(mem_index+i)%256);
  }
  i=0;
  for(j=0; j<rows; ++j) {
    printf("\n%04x ",	mem_index +j*cols);
    for(i=0; i<cols; ++i) {
	printf("%02x ",	conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
    }
  }
  printf("\n");
}

void print_memory_page2(int mem_index){
  int i,j;
  int pagesize = 256;
  //  printf("\n  printf(   \%04x ,mem_index);");
  for(i=mem_index; i<mem_index + 16; ++i) {
    /*    printf("        ----'----\n");*/

	printf("%02x ", 	conv_bitstr2int(memory[i],0,7));
	/* printf("%i   ", 	conv_bitstr2int(memory[i],0,7)); */
	/* printf("%s   ", 	memory[i]); */
    /* for(j=0;memory[i][j]; j++) */
    /*   printf("%c", memory[i][j]); */
  }
  printf("sdfsdf\n");
  /*  printf("        ----'----");*/
  printf("   ");
  for(i=0;i<16;i++){
    printf("%02x ",i);
  }
  i=0;
  for(j=0; j<4; ++j) {
    printf("\n%02x ",	pagesize*mem_index+j*16+i);
    for(i=0; i<16; ++i) {
      //    printf("        ----'----\n");

	printf("%02x ",	conv_bitstr2int(memory[pagesize*mem_index+j*16+i],0,7));
	/* printf("%i   ", 	conv_bitstr2int(memory[i],0,7)); */
	/* printf("%s   ", 	memory[i]); */
    /* for(j=0;memory[i][j]; j++) */
    /*   printf("%c", memory[i][j]); */
    }

  }
}

void print_memory(){
  int i,j;
  for(i=0; i<MEMORY_SIZE; i++) {
    printf("        ----'----");
    printf("%x |\n",i);
    for(j=0;memory[i][j]; j++)
      printf("%c", memory[i][j]);
    printf("|\n");
  }
  printf("        ----'----");
}


void cp_cell2memory(char *cell, int addr){
  int i;
  for(i=0; i<MEMORY_CELL_SIZE; ++i){
    *(memory[addr]+i) = cell[i];
  }
}

/*
  some helper function transforming int
  to f!*#@ing "10101010" format
*/
void number2register(int number, char* reg){
  reg[8]=0x0;
  int i;
  for(i=0; i<8;i++){
	if(number & 1<<i)
	  reg[7-i]='1';
	else
	  reg[7-i]='0';
  }
}

void read_dump(char *fname){
  char buffer_reg[8+1];
  int adr, content;
  FILE *stream;
  
  buffer_reg[8] = '\0';
  if(!strcmp("-", fname)){
    stream = stdin;
  } else if ((stream = fopen( fname,"r")) == NULL){
    printf("Can't open %s\n", fname);
    exit(1);
  }
  while(!feof(stream)){
    // format string is:
	// <any space, including 0> <hex address> <any space, including 0> <hex content>
	//DELME    fscanf(stream, " %i %8c ", &adr, buffer);
	fscanf(stream, " %4x %2x ", &adr, &content);
	if(adr>MEMORY_SIZE){
	  printf("Illegal Address %i\nSkipping line ..\n", adr);
	} else {
	  //	  printf("%i: %i->%02x\n", i++, adr, content);
	  number2register(content, buffer_reg);
	  cp_cell2memory(buffer_reg, adr);
	}
  }
}


/*
int conv_addr2int_with_shift_and(char* addr){
  int i=0;
  int sum = 0;
  for(i=0; i<ADDR_WIDTH;i++)
    if(addr[i] == '1'){
      sum = (sum << 1) | 1;
    }else
      sum = (sum << 1);
  return sum;
}
int conv_addr2int_with_shift_or(char* addr){
  int i=0;
  int sum = 0;
  for(i=0; i<ADDR_WIDTH;i++){
    sum = (sum << 1);
    if(addr[i] == '1')
      sum = sum | 1;
  }
  return sum;
}

//convert bitstring <bitstr> from bit <start> to bit <end> into integer
int conv_bitstr2int(char *bitstr, unsigned int start, unsigned int end){
  unsigned int i=0;
  int sum=0;
  for(i=start; i<=end;i++)
    if(bitstr[i] == '1'){
      sum = (sum << 1) | 1;
    }else
      sum = (sum << 1);
  return sum;
}
*/

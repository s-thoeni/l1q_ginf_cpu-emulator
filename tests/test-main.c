/*

  test-main.c
  bho 12.8.2012

  What?:
  reads memory-file and register-file and
  copies values into memory and cpu

  starts then the fetch-exec-cycle of cpu
  prints after each  cycle cpu-content 

  GPL applies

*/

#include <string.h>
#include <stdio.h>

#include "cpu.h"
#include "flags.h"
#include "alu.h"
#include "decode.h"
#include "memory.h"
#include "cpu-util.h"
#include "register.h"

#include <stdlib.h>
#include "SDL.h"

void print_test_registers(){
  struct cpu_operator* optr;
  optr = (struct cpu_operator*) getop();

  //  printf("pc   AC XR YR SP NV-BDIZC \n");
  printf("%02x%02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c %02x\n",
		 conv_bitstr2int(pch,0,7),
		 conv_bitstr2int(pcl,0,7),
		 conv_bitstr2int(acc,0,7),
		 conv_bitstr2int(idx,0,7),
		 conv_bitstr2int(idy,0,7),
		 conv_bitstr2int(sp,0,7),

		 getSignflag(flags)=='1'? 'N' : 'n',
		 getOverflowflag(flags)=='1'? 'V' : 'v',
		 '-',
		 getBRKflag(flags)=='1'? 'B' : 'b',
		 getDflag(flags)=='1'? 'D' : 'd',
		 getIRQflag(flags)=='1'? 'I' : 'i',
		 getZeroflag(flags)=='1'? 'Z' : 'z',
		 getCarryflag(flags)=='1'? 'C' : 'c',
		 conv_bitstr2int(ir,0,7)//,optr->name,optr->info
		 
		 );
}

void print_tester(){
  struct cpu_operator* optr;
  optr = (struct cpu_operator*) getop(memory[pc2int()]);

  print_test_registers();

}


int main(int argc, char *argv[]){
    SDL_Surface *screen;
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        printf("Can't init SDL:  %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(200, 100, 16, SDL_HWSURFACE);
    if (screen == NULL) {
        printf("Can't set video mode: %s\n", SDL_GetError());
        exit(1);
    }   

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

  int i=0, maxsteps = 1807;
  print_tester();  
  //  while(1){
  while(i++<maxsteps){
    fetch();
    mue_code_ptr = decode();
    execute(mue_code_ptr);
	print_tester();
  }


  //  exit_monitor();


	SDL_Delay(3000);
    SDL_Quit();
    return 0;
	
}

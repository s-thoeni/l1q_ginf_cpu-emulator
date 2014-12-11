/*

  graphic-main.c
  bho1 - init 29.12.2011 
  bho1 -  26.8.2013
  remove trash
  Compile with  `sdl-config --cflags --libs`
  

  
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

#include "printers.h"
#include "parser.h"
#include "src-ripper.h"
#include <signal.h>
static int keepParsing = 1;

#include <stdlib.h>

#include "sdl-driver.h"

/* #include "bitmap.h" */
/* #include "chardev.h" */

#include "SDL.h"
#include "SDL_mouse.h"
//#include "SDL_events.h"
#include <SDL_events.h>

int videobase;

extern SDL_Surface *le_screen;

extern unsigned int virtual_x_resolution;
extern unsigned int virtual_y_resolution;

extern int vid_mode;

struct cpu_state * stateptr;

void cp_byte(char *a, char*b){
  int i;
  for(i=0;i<8;i++){
    *(a+i) = *(b+i);
  }
  *(a+i) = '\0';
}


void intHandler(int dummy) {
  dummy++;
  keepParsing = 1;
}

/*
  main:
  options:

  --memory <memory-dump>
  Read file <memory-dump> and copy into memory

  --source <disassembler source>
  Load file <disassembler source> into source array
  --register <filename>
  copy content in <filename>  into corresponding register
*/
int main(int argc, char *argv[])
{
  signal(SIGINT, intHandler);
  
  // memory allocation
  init_memory();
  cpu_6502_init();
  init_display();
  
  cpu_6502_reset();

  extern int videobase ;
  videobase = 0x200 ;
  
  // handling parameter --memory <dumpfile> 
  //  if(argc==3){
  int i;
  for(i=1;i<argc;i++){
	if(!strcmp(argv[i], "--memory")){
	  printf("Loading %s into memory\n", argv[2]);
      read_dump(argv[i+1]);
	  i++;
	}
	if(!strcmp(argv[i], "--source")){
	  printf("Loading %s as assembler code\n", argv[i+1]);
      src_read(argv[i+1]);
	  i++;
	}
  }
  // handling parameter --register <dumpfile> 
  if(argc==5){
	if(!strcmp(argv[3], "--register")){
	  printf("Loading %s into register\n", argv[2]);
	  cpu_6502_register_dump(argv[6]);
    }
  }

 
  
  printf("drawing video memory at %04x\n",videobase);

  
  vid_mode=0;

  

  draw_bitmap_memory(videobase); // draw_memory(memory+videobase);


  //  draw_txtbuffer();
  


  
  
  //int d ;
  // d = getchar();


  printf("*************************************\nNext Level:  \n\n");


  //  clear_screen(memory+videobase);
  clr_screen(memory+videobase);

  /* unsigned int i,j; */
  /* for(i=12; i< 40; i++){ */
  /*   for(j=12; j< 40; j++){ */
  /*     cp_byte(memory[videobase+i+j*virtual_x_resolution], "00001011"); */
  /*     //      memory[videobase+i+j*virtual_x_resolution] = "00001011"; */
  /*   } */
  /* } */
  /* for(i=12; i< 10; i++){ */
  /*   cp_byte(memory[videobase+i+30*virtual_x_resolution], "00001101"); */
  /*   //    memory[videobase+i+30*virtual_x_resolution] = "00001001"; */
  /* } */
  /* for(j=12; j< 100; j++){ */
  /*   cp_byte(memory[videobase+12+ j*virtual_x_resolution], "00000011"); */
  /* } */
  /* for(j=2; j< 50; j++){ */
  /*   cp_byte(memory[videobase+j+ j*virtual_x_resolution], "00000011"); */
  /* } */

  /* memory[videobase+i+4*virtual_x_resolution] = "00001001"; */
  /* memory[videobase+44+12*virtual_x_resolution] = "00001111"; */
  /* memory[videobase+i+4*virtual_x_resolution] = "00001001"; */
  
  //  printf("    load_proof(\"proof.dump\");\n");
  //  load_proof("proof.dump");

  draw_bitmap_memory(videobase); // draw_memory(memory+videobase);



  void (*mue_code_fct_ptr)();


  while(1 && !feof(stdin)){
    if(keepParsing){
      printf("\n");
      repl();
      keepParsing = 0;
    }else{
      fetch();
      mue_code_fct_ptr = decode();
      stateptr = execute(mue_code_fct_ptr);

      if(cycles_sum >1500){
		//if( cycles_sum > 1 ){
		cycles_sum=0;
		draw_bitmap_memory(videobase);    
      }
    }
  }
  printf("quit repl and cpu ..\n");
  SDL_Quit();
  exit(0);
  //  quit_monitor();
  return 0;
}

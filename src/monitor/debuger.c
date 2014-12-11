/*
  debuger.c
  bho - init 7.12.2011
  GPL applies
 */

//#include <config.h>
//#include "monitor.h"


#define MAX_BREAKPOINTS = 10;
char *breakpointers[MAX_BREAKPOINTS][2];
int position;

void init_breaks(){
  int i;
  for(i=0; i<MAX_BREAKPOINTS; i++){
	breakpointers[i][0] = 0;
	breakpointers[i][1] = 0;
  }
  position=0;
}

  
int same_adr_p(char h1[], char l1[], char h2[], char l2[]){
  int i;
  for(i=0; i<REG_WIDTH; i++){
	if(l1[i]!=l2[i] || h1[i]!=h2[i]){
	  return 0;
	}
  }
  return 1;
}
  
int check4break(){
  int i;
  for(i=0; i<MAX_BREAKPOINTS; i++){
	if(same_adr_p(breakpointers[i][0],breakpointers[i][1],pch,pcl))
	  return 1;
  }
  return 0;
}


void add_breakpoint(char high[], car low[]){
  cp_register(high, breakpointers[position][0]);
  cp_register(low, breakpointers[position][1]);
  position = (position + 1) % MAX_BREAKPOINTS;
}


void del_breakpoint(position){
  breakpointers[position][0]=0;
  breakpointers[position][1]=0;
}


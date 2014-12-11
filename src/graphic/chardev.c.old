/*
  chardev.c

  character screen
  
  bho1 - init 29.12.2011

  you may have to install libsdl-dev
  sudo apt-get install libsdl1.2-dev

  Compile with  `sdl-config --cflags --libs`
  


  --------> x
  |
  |
  |
  |
  |
  |
  \/
  
  y

  
*/
#include <stdio.h>
#include <stdlib.h>


#include "sdl-driver.h"
#include "chardotmatrix.h"

#include "SDL.h"
#include "bitmap.h"
//#include "chardev.h"

/*
  video character screen (vcs): 20 to 10 character matrix
*/
char vcs[20][10]
= {
  {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9
  },
  {
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  },
  {
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29
  },
  {
	20, 31, 32, 33, 34, 35, 36, 37, 38, 39
  },
  {
	20, 41, 42, 43, 44, 45, 46, 47, 48, 49
  },
  {
	20, 51, 52, 53, 54, 55, 56, 57, 58, 59
  },
  {
	20, 61, 62, 63, 64, 65, 66, 67, 68, 69
  },
  {
	20, 71, 72, 73, 74, 75, 76, 77, 78, 79
  },
  {
	20, 81, 82, 83, 84, 85, 86, 87, 88, 89
  },
  {
	20, 91, 92, 93, 94, 95, 74, 74, 94, 94
  }

};






void draw_char0(unsigned int xcoord, unsigned int ycoord, int ix ){
  unsigned int i,j;
  unsigned int x = xcoord;
  unsigned int y = ycoord;
  unsigned int ydots = 5;
  unsigned int xdots = 8;
  //  unsigned int x=10,y=10;
  /* for(i=ydots*ix;i<ydots*ix+4;i++,y++){ */
  /* 	for(j=0;j<xdots;j++,x++){ */
  /* 	  dot(x,y,dot5x8matrix[i][j]=='1' ? 1 : 0); */
  /* 	} */
  /* 	x=xcoord; */
  /* } */
 
  for(j=0;j<xdots;j++,y++){

	dot(x-1,y,2);
	dot(x-2,y,2);

	for(i=ydots*ix;i<ydots*ix+ydots;i++,x++){

	  dot(x,y,dot5x8matrix[i][j]=='1' ? 1 : 0);
	}
	dot(x+2,y,2);
	dot(x+3,y,2);
	x=xcoord;
  }
}
void draw_char(unsigned int xcoord, unsigned int ycoord, int ix ){
  draw_char0(10+xcoord*9, 10+ycoord*8,ix);
}

void draw_vcs(char vicmem[][10]){
 
  unsigned int i,j;

  for(i=0;i<20;i++){
	for(j=0;j<10;j++){
	  //	  printf("%i %i vcs[][] %i\n",i ,j,vicmem[i][j]);
	  draw_char0(10+i*9, 10+j*8, vicmem[i][j]);
	  //	  draw_char0(10+i*9, 10+j*8, c++%maxchar);
	  //	  printf("i: %4d j: %5d count: %5d\n", i, j, j);
	}
  }

  /* draw_char(10, 10, 37); */
  /* draw_char(10, 20, 38); */
  /* draw_char(10, 30, 39); */
}

void draw_txtbuffer() {
  char (*vcsm)[10];
  vcsm=vcs;
  int i;
  char txt[20] = "helo worldhelo world";
  for(i=0;i<20;i++){
	//	vcsm[i][4]='a'-32;
	vcsm[i][4] = txt[i]-32;
	//	printf("vcsm[i][4]='a'%d",'a');//txt[i];
	draw_vcs(vcsm);
	//	SDL_Delay(1000);
	//	show_memory(videomemory,le_screen);
  }
  
}



/*

  dead stuff
  
*/


/*
unsigned int lookup_char(struct charmapentry *chartable, unsigned char *ptr){
  struct charmapentry *entryptr = chartable;
  unsigned int i;
  for(i=0; i<4; i++){
	if(entryptr->symbol == *ptr)
	  return entryptr->ix;
	ptr++;
  }
  return 0;
}
*/

/*
  void draw_charscreen(){
  unsigned int i,j;
  
  unsigned char *ptr=charscreen;
  printf("NOOOOOO\nNOOOO\n");
  for(i=0;i<20;i++){
	for(j=0;j<10;j++){
	  for(;*ptr;ptr++){
		draw_char0(10+i*9, 10+j*8,lookup_char(chartable, ptr));
	  }  
	}
  }
}
*/

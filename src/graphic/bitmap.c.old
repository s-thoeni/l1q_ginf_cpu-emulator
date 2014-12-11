/*

  bitmap.c
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


//#define MEMORY_SIZE 65536
//#define MEMORY_CELL_SIZE 8

#include <stdlib.h>
#include "SDL.h"

#include "memory.h"
#include "cpu-util.h"

//#include "chardotmatrix.h"

#include "sdl-driver.h"



void diagonale(unsigned int x, unsigned int y, int color){
  /* unsigned int i; */
  /* unsigned int j; */
  unsigned int k;
  /* int r,g,b; */
  /* r = colortable[color][0]; */
  /* g = colortable[color][1]; */
  /* b = colortable[color][2]; */

  
  for (k=0; k<virtual_x_resolution-6;k++){
	dot((x+k), (y+k), color);
	dot((x+k), y, color);

	/* for (i=0; i<x_stretch;i++){ */
	/*   for (j=0; j<y_stretch;j++){ */
	/* 	DrawPixel(le_screen, (x+k)*x_stretch+i, (y+k)*y_stretch+j, r+500,0,b); */
	/* 	DrawPixel(le_screen, (x+k)*x_stretch+i, y*y_stretch+j, r+500,0,b); */
	/*   } */
	/* } */

  }
  
  /* for (k=0; k<virtual_y_resolution-26;k++){ */
  /* 	for (i=0; i<x_stretch;i++){ */
  /* 	  for (j=0; j<y_stretch;j++){ */
  /* 		//		DrawPixel(le_screen, x*x_stretch+i, (y+k)*y_stretch+j, r+500,0,b); */
  /* 	  } */
  /* 	} */
  /* } */

}

void draw_bitmap_memory(int base){

  int color;
  unsigned int i,x,y;
  for(i=0; i<virtual_x_resolution*virtual_y_resolution;i++){
	//    index = i + j*32;
    //    y = (unsigned int)index/virtual_x_resolution;
    //    x = index%virtual_x_resolution;

    color = conv_bitstr2int(memory[base+i],4,7);
    x = i%virtual_x_resolution; 
    y = (int) i/virtual_x_resolution;

    dot(x,y,color);

  }
  switch_flip();
}  
  



/* int pixelnr(unsigned int x, unsigned int y){ */
/*   return y*virtual_x_resolution+x; */
/* } */

/* int bitpos(unsigned int x, unsigned int y){ */
/*   return pixelnr(x,y)%MEMORY_CELL_SIZE; */
/* } */

/* int adr(unsigned int x, unsigned int y){ */
/*   return div(pixelnr(x,y), MEMORY_CELL_SIZE).quot; */
/* } */


void clr_screen(char* memory[]){
  int color;
  unsigned int i, j, x, y;
  for(i=0; i<virtual_x_resolution*virtual_y_resolution;i++){
	//    index = i + j*32;
    //    y = (unsigned int)index/virtual_x_resolution;
    //    x = index%virtual_x_resolution;

    color = 0;//color = conv_bitstr2int(memory[base+i],4,7);
	for(j=0;j<MEMORY_CELL_SIZE; j++){
	  memory[i][j] = '0';
	}
	memory[i][j] = '\0';
    x = i%virtual_x_resolution; 
    y = (int) i/virtual_x_resolution;

    dot(x,y,color);

  }

  switch_flip();
}



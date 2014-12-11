/*

  bho1 - init 8.7.2011 

  Compile with  `sdl-config --cflags --libs`
  

  
*/

#include <stdlib.h>

#include "display.h"
#include "memory.h"
#include "vc.h"
#include "SDL.h"

extern SDL_Surface *le_screen;

extern unsigned int virtual_x_resolution;
extern unsigned int virtual_y_resolution;

int vid_mode = 1;
/* extern unsigned int x_stretch; */
/* extern unsigned int y_stretch; */
/* extern unsigned int x_resolution; */
/* extern unsigned int y_resolution; */

char vcsm[20][10]
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

int pixelnr(unsigned int x, unsigned int y){
  return y*virtual_x_resolution+x;
}

int bitpos(unsigned int x, unsigned int y){
  return pixelnr(x,y)%MEMORY_CELL_SIZE;
}

int adr(unsigned int x, unsigned int y){
  return div(pixelnr(x,y), MEMORY_CELL_SIZE).quot;
}

void show_memory(char* memory[], SDL_Surface *le_screen){
  if(vid_mode)
	SDL_Flip(le_screen);
  else{
	draw_memory(memory);
	SDL_Flip(le_screen);
  }
}

/* Allocates memory */
void init_memory(char *memory[])
{
  
  unsigned int i,j;
  for( i=0; i<MEMORY_SIZE; ++i )
    {
      memory[i] = (char*)malloc(sizeof(char)*(MEMORY_CELL_SIZE+1));
	  // init memory to zero '0'
	  for( j=0 ; j<MEMORY_CELL_SIZE ; j++)
		memory[i][j]= random()%2 ? '0' : '1';
	  memory[i][j] = 0x0; 
    }
}



void poke(unsigned int x, unsigned int y, char* memory[], int value){
  unsigned int i,j;
  
  if( (y < virtual_y_resolution) && (x < virtual_x_resolution) ){
	i = adr(x,y);
	j= bitpos(x,y);
	memory[i][j]= value ? '1': '0';
  }	  else{
	printf("poke error: \n");
  }
}

int main(int argc, char *argv[])
{
  
  char *memory[MEMORY_SIZE];


  unsigned int x,y;
  unsigned int i,j;
  
  init_memory(memory);

  init_display();
  
  printf("drawing memory\n");

  vid_mode=0;
  
  int d = getchar();

  show_memory(memory,le_screen);


    
  printf("after SDL_Flip\n");
  d = getchar();
  /*
	for(i=0; i<virtual_x_resolution; i++)
	for(j=0; j<virtual_y_resolution; j++){
	  x = adr(i,j);
	  y = bitpos(i,j);
	  memory[x][y]='0';
	}
	
  */
  

  // clear screen
  for(y=0; y<virtual_y_resolution; y++)
	for(x=0; x<virtual_x_resolution; x++){
	  i = adr(x,y);
	  j= bitpos(x,y);
	  poke(x,y,memory,0);
	  /* if(i<=190 && i>110) */
	  /* 	memory[i][j]='1'; */
	  /* else */
	  /* 	memory[i][j]='0'; */
	  
	  //	  SDL_UpdateRect(le_screen, 0,0, x_resolution, y_resolution);
 }
  printf("After loop\n");
  show_memory(memory,le_screen);

  vid_mode=1;
  draw_char_mode();

  char txt[20] = "helo worldhelo world";
  for(i=0;i<20;i++){
	//	vcsm[i][4]='a'-32;
	vcsm[i][4] = txt[i]-32;
	printf("vcsm[i][4]='a'%d",'a');//txt[i];
	draw_vcs(vcsm);
	SDL_Delay(1000);
	show_memory(memory,le_screen);
  }
  

  vid_mode=0;
  SDL_Delay(3000);
  d = getchar();  
  //  SDL_Flip(le_screen);
  
  printf("waiting 0\n");
  SDL_Delay(3000);

  // draw horizontal line
  for(i=5; i<virtual_x_resolution-5; i++){
	poke(i,25,memory,1);
	//memory[adr(i, 25)][bitpos(i,i)]='1';
	show_memory(memory,le_screen);  

  }
  show_memory(memory,le_screen);  
  d = getchar();
  // draw vertical line
  for(i=5; i<100-5; i++){
	memory[adr(25, i)][bitpos(25,i)]='1';
  }
  show_memory(memory,le_screen);
  
  // draw diagonal line
  for(i=5; i<100-5; i++){
	//poke(i,i,memory,1);
	memory[adr(i, i)][bitpos(i,i)]='1';

  }
  
  show_memory(memory,le_screen);
  printf("waiting \n");
  SDL_Delay(3000);
  d = getchar();
  
  return 0;
}

/*

  
    int x,y;
  int i,j;
  int bitnr;
  //  struct div_t dummy;
  for(y=0; y<10;y++)
	for(x=0; x<10;x++){
	  bitnr = y*virtual_x_resolution+x;
	  //	  dummy = div(bitnr, MEMORY_CELL_SIZE);
	  printf("qwe %i\n", MEMORY_CELL_SIZE);
	  printf("x: %3i y: %3i bitnr.: %5i adr: %4i bipos: %i rem: %i\n", x, y, bitnr , bitnr%MEMORY_CELL_SIZE, div(bitnr, MEMORY_CELL_SIZE).quot , div(bitnr, MEMORY_CELL_SIZE).rem);
	}
  for(i=0; i<10;i++)
	for(j=0; j<10;j++){
	  printf("i: %3i j: %3i adr: %4i bitpos: %4i bitnbr: %4i\n", i, j, adr(i,j), bitpos(i,j).quot, j*virtual_x_resolution + i);
	}

 */




  /*
	
  DrawPixel(le_screen, x_stretch*x, y_stretch*y, 200,200,200);
  DrawPixel(le_screen, x_stretch*x+1, y_stretch*y, 200,200,200);
  DrawPixel(le_screen, x_stretch*x, y_stretch*y+1, 200,200,200);
  DrawPixel(le_screen, x_stretch*x+1, y_stretch*y+1, 200,200,200);
  */
  /*
  DrawPixel(le_screen, x, y, 200,200,200);
  DrawPixel(le_screen, x+1, y, 200,200,200);
  DrawPixel(le_screen, x, y+1, 200,200,200);
  DrawPixel(le_screen, x+1, y+1, 200,200,200);
  */

/*

  bho1 - init 12.12.2009
  
  bho1 - introduce virtualize and stretch factor: 
  One virtual Pixel has now x_stretch x y_stretch rectangle Pixels
  real coarse:)

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
#include "chardotmatrix.h"

SDL_Surface *le_screen;

 unsigned int virtual_x_resolution = 200;
 unsigned int virtual_y_resolution = 100;

 unsigned int x_stretch = 4;
 unsigned int y_stretch = 4;


char *vidchar[] = {
  
	"0000",
	"0000",
	"0000",
	"0000",

	"0110",
	"0010",
	"0010",
	"0010",

	"1000",
	"0100",
	"0010",
	"0001",

	"0110",
	"1001",
	"1111",
	"1001",

	"1010",
	"1100",
	"1010",
	"1111",

	"1111",
	"1010",
	"1010",
	"1010",

	"1111",
	"0010",
	"0010",
	"0010",

	"1000",
	"1000",
	"1000",
	"1111"
};

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

// shameless copy from net
void DrawPixel(SDL_Surface *screen, int x, int y,Uint8 R, Uint8 G,Uint8 B)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) )
    {
        if ( SDL_LockSurface(screen) < 0 ) {
            return;
        }
    }

    switch (screen->format->BytesPerPixel) {
        case 1: { /* vermutlich 8 Bit */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: { /* vermutlich 15 Bit oder 16 Bit */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: { /* langsamer 24-Bit-Modus, selten verwendet */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } else {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: { /* vermutlich 32 Bit */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }

    if ( SDL_MUSTLOCK(screen) )
    {
        SDL_UnlockSurface(screen);
    }
}


void init_display(){
  unsigned int x_resolution = x_stretch*virtual_x_resolution; 
  unsigned int y_resolution = y_stretch*virtual_y_resolution;

//SDL_Surface *screen;
  if (SDL_Init(SDL_INIT_VIDEO) == -1) {
	printf("Can't init SDL:  %s\n", SDL_GetError());
	exit(1);
  }
  
  atexit(SDL_Quit);
  le_screen = SDL_SetVideoMode(x_resolution, y_resolution, 16, SDL_HWSURFACE);
  if (le_screen == NULL) {
	printf("Can't set video mode: %s\n", SDL_GetError());
	exit(1);
  }
}

void exit_display(){
  SDL_Quit();
}


void dot(unsigned int x, unsigned int y, int color){
  
  // todo
  // loop over stretch factor and draw stretchfactor pixels
  
  unsigned int i,j;
  int r,g,b;
  switch ( color ){
  case 0 :
	r=000;
	g=000;
	b=000;
    break;
  case 1 :
	r=200;
	g=200;
	b=200;
    break;
  case 2 :
	r=400;
	g=400;
	b=400;
    break;
  case 3 :
	r=800;
	g=800;
	b=800;
    break;
  default:
	r=g=b=0;
	break;
  }
  
  for (i=0; i<x_stretch;i++)
	for (j=0; j<y_stretch;j++)
	  DrawPixel(le_screen, x_stretch*x+i, y_stretch*y+j, r,g,b);
  
  //  SDL_Flip(le_screen);
}



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

void draw_charscreen(){
  unsigned int i,j;
  unsigned int x=0,y=0;
  
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
  
void draw_vcs(char vicmem[][10]){
 
  unsigned int i,j;
  unsigned int x,y;
  unsigned int c = 0;
  for(i=0;i<20;i++){
	for(j=0;j<10;j++){
	  printf("%i %i vcs[][] %i\n",i ,j,vicmem[i][j]);
	  draw_char0(10+i*9, 10+j*8, vicmem[i][j]);
	  //	  draw_char0(10+i*9, 10+j*8, c++%maxchar);
	  //	  printf("i: %4d j: %5d count: %5d\n", i, j, j);
	}
  }

  /* draw_char(10, 10, 37); */
  /* draw_char(10, 20, 38); */
  /* draw_char(10, 30, 39); */
}
void draw_char_mode(){
  //draw_charscreen();
    draw_vcs(vcs);
  
  /* int i; */
  /* for(i=40;i<48;i++){ */
  /* 	draw_char(10+i*9, 10, i); */
  /* } */
  
  /* draw_char(10, 20, 20); */
  /* draw_char(40, 60, 21); */
  /* draw_char(50, 60, 22); */
  /* draw_char(80, 60, 23);  */
}

  
  

void draw_memory(char* adr[]){
  unsigned int i,j,x,y, width, index;

  for( i=0; i< virtual_x_resolution*virtual_y_resolution/8; i++){
	for( j=0; j<8; j++){
	  index = i*8 + j;
	  y = (unsigned int)index/virtual_x_resolution;
	  x = index%virtual_x_resolution;
	  if(adr[i][j] == '1') {
		dot(x,y,1);
	  } else 
		dot(x,y,0);
	}
  }
  
  /*
	for(x=0; x<x_resolution; x++){
	for(y=0; y<y_resolution; y++){
	  // baseAddress+y*width*r+x*r :: r= RGB width
	  if (	*(adr+y*x_resolution+x) == '1' )
		dot(x, y);
	  //	  x = (int) (adr+i)%y_resolution;
	  //	  y = (int) (adr+i)/x_resolution;
	}
	}
  */
}








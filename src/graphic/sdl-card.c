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


#define MEMORY_SIZE 65536
#define MEMORY_CELL_SIZE 8

#include <stdlib.h>
#include "SDL.h"

#include "memory.h"
#include "cpu-util.h"

//#include "chardotmatrix.h"

#include "sdl-driver.h"

static SDL_Surface *le_screen;

static unsigned int virtual_x_resolution = 200;
static unsigned int virtual_y_resolution = 100;

static unsigned int x_stretch = 4;
static unsigned int y_stretch = 4;


static unsigned int x_resolution;
static unsigned int y_resolution;

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
  
  int i,j;
  int r,g,b;
  if(color){
	r=200;
	g=200;
	b=200;
  } else {
	r=0;
	g=0;
  	b=0;
  }
  
  for (i=0; i<x_stretch;i++)
	for (j=0; j<y_stretch;j++)
	  DrawPixel(le_screen, x_stretch*x+i, y_stretch*y+j, r,g,b);
  
  //  SDL_Flip(le_screen);
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



/* Allocates memory */
void init_memory(char *memory[])
{
  
  int i,j;
  for( i=0; i<MEMORY_SIZE; ++i )
    {
      memory[i] = (char*)malloc(sizeof(char)*(MEMORY_CELL_SIZE+1));
	  // init memory to zero '0'
	  for( j=0 ; j<MEMORY_CELL_SIZE ; j++)
		memory[i][j]= random()%2 ? '0' : '1';
	  memory[i][j] = 0x0; 
    }
}


int pixelnr(int x, int y){
  return y*virtual_x_resolution+x;
}

int bitpos(int x, int y){
  return pixelnr(x,y)%MEMORY_CELL_SIZE;
}

int adr(int x, int y){
  return div(y*virtual_x_resolution + x, MEMORY_CELL_SIZE).quot;
}



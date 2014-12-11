/*

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
#include "sdl-driver.h"

SDL_Surface *le_screen;

SDL_Surface *sdl_init_display(unsigned int x_resolution, unsigned int y_resolution);
void sdl_exit_display();


unsigned int device_x_resolution;
unsigned int device_y_resolution;


int vid_mode = 0;

int colortable[16][3]= {
  
  {  //Black 0x0
	0x00, 0x00, 0x00
  },
  {  //White 0x1
	0xFF, 0xFF, 0xFF
  },
  {  //Red 0x2
	0x68, 0x37, 0x2B
  },
  {  //Cyan 0x3
	0x70, 0xA4, 0xB2
  },
  {  //Purple 0x4
	0x6F, 0x3D, 0x86
  },
  {  //Green 0x5
	0x58, 0x8D, 0x43
  },
  {  //Blue 0x6
	0x35, 0x28, 0x79
  },
  {  //Yellow 0x7
	0xB8, 0xC7, 0x6F
  },
  {  //Orange 0x8
	0x6F, 0x4F, 0x25
  },
  {  //Brown 0x9
	0x43, 0x39, 0x00
  },
  {  //Light red 0xa
	0x9A, 0x67, 0x59
  },
  {  //Dark gray 0xb
	0x44, 0x44, 0x44
  },
  {  //Gray 0xc
	0x6C, 0x6C, 0x6C
  },
  {  //Light green 0xd
	0x9A, 0xD2, 0x84
  },
  {  //Light blue 0xe
	0x6C, 0x5E, 0xB5
  },
  { //Light gray 0xf
	0x95, 0x95, 0x95
  }
};


void init_display(){
  virtual_x_resolution = 32;
  virtual_y_resolution = 32;
  /* virtual_x_resolution = 200; */
  /* virtual_y_resolution = 100; */
  
  x_stretch = 4;
  y_stretch = 4;
  //colortable = colortablefix;
  device_x_resolution = x_stretch*virtual_x_resolution;
  device_y_resolution = y_stretch*virtual_y_resolution;
  le_screen = sdl_init_display(device_x_resolution, device_y_resolution);
}

void exit_display(){
  sdl_exit_display();
}
void switch_flip(){
  SDL_Flip(le_screen);
}


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


void dot(unsigned int x, unsigned int y, int color){
  
  // todo
  // loop over stretch factor and draw stretchfactor pixels
  
  unsigned int i;
  unsigned int j;
  int r,g,b;
  r = colortable[color][0];
  g = colortable[color][1];
  b = colortable[color][2];

  for (i=0; i<x_stretch;i++){
	for (j=0; j<y_stretch;j++){
	  DrawPixel(le_screen, x*x_stretch+i, y*y_stretch+j, r,g,b);
	}
  }
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

  


SDL_Surface *sdl_init_display(unsigned int x_resolution, unsigned int y_resolution){
  SDL_Surface *screen;

//SDL_Surface *screen;
  if (SDL_Init(SDL_INIT_VIDEO) == -1) {
	printf("Can't init SDL:  %s\n", SDL_GetError());
	exit(1);
  }
  
  atexit(SDL_Quit);
  screen = SDL_SetVideoMode(x_resolution, y_resolution, 16, SDL_HWSURFACE);
  if (screen == NULL) {
	printf("Can't set video mode: %s\n", SDL_GetError());
	exit(1);
  }
  return screen;
}

void sdl_exit_display(){
  SDL_Quit();
}











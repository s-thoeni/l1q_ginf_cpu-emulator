/*
  sdl-driver.h
  bho1 - init 8.7.2011
  GPL

  simulating a video chip

*/

#ifndef _SDLDRIVER_H_
#define _SDLDRIVER_H_
#include "SDL.h"

unsigned int virtual_x_resolution;
unsigned int virtual_y_resolution;

unsigned int x_stretch;
unsigned int y_stretch;

void dot(unsigned int x, unsigned int y, int color);
void init_display();
void exit_display();
void switch_flip();
void draw_bitmap_memory(int videobase);
void clr_screen(char* memory[]);

//void DrawPixel(SDL_Surface *screen, int x, int y,Uint8 R, Uint8 G,Uint8 B);
//SDL_Surface *sdl_init_display(unsigned int x_resolution, unsigned int y_resolution);

//void sdl_exit_display();



#endif /* _SDLDRIVER_H_ */

/*

  visual-main.c
  init: 19.6.2014@bo
  uses shared memory with key 5678 to display memory content
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>

#include "register.h"
#include "cpu-util.h"
#include "cpu.h"

//shared memory stuff
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

char *shm;
struct cpu_state * stateptr;
/*
  We need to get the segment named
  "5678", created by the server.
*/
#define  SHM_KEY 5678;
#define SHM_CPU_KEY 0xabcd007
#define MEMORY_SIZE 				65535 	// memory size
#define SURFACE_WIDTH 			800
#define SURFACE_HEIGHT 			800
//#define MEM_SURFACE_W				512
//#define MEM_SURFACE_H				512

#define MEM_SURFACE_W			800
#define MEM_SURFACE_H			800

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

#define SURFACE_BPP 				32
#define RED_BITS 					0x0
#define GREEN_BITS 					0x2
#define BLUE_BITS 					0x4
#define ALPHA_BITS 					0x6
#define CELL_ALPHA_VALUE			0xFF
#define SCREEN_BACKGROUND_COLOR		0x000000FF
#define MOUSE_SELECTOR_COLOR		0xFFFFFF51
#define CELL_BORDERS				1		// borders of the memory cell

#define FPS 						30
#define FONT_PATH					"c64/fonts/C64_Pro_Mono_v1.0-STYLE.ttf"
#define FONT_PTSIZE					10
#define DEFAULT_BOXX 20
#define DEFAULT_BOXY 20
typedef struct mem_cell_s{
  Uint8 value;
}mem_cell_t;

typedef struct c64_s{
  mem_cell_t memory[MEMORY_SIZE];
}c64_t;
TTF_Font *font;						// the font used for writing
TTF_Font *systemfont;						// the font used for writing
typedef struct visualC64_s{
  SDL_Surface *screen;
  bool running;
  Uint16 mouse_drag_start_x;
  Uint16 mouse_drag_start_y;
  Uint16 xmax;
  Uint16 ymax;
  Uint16 xcursor;
  Uint16 ycursor;

  SDL_Surface *mem_surface;			// the memory surface
  SDL_Rect mem_dimensions;			// the dimensions of the visual memory
  SDL_Rect mem_visible_cells;			// all the visible cells -> width & height should never be 0
  SDL_Rect mouse_selector;			// the current position of the mouse
  TTF_Font *font;						// the font used for writing
  c64_t c64;
}visualC64_t;
SDL_Color colors[16] = { { .r=0x00, .g=0x00, .b=0x00},
			 {.r=0xFF, .g=0xFF, .b=0xFF},
			 {.r=0x68, .g=0x37, .b=0x2B},
			 {.r=0x70, .g=0xA4, .b=0xB2},
			 {.r=0x6F, .g=0x3D, .b=0x86},
			 {.r=0x58, .g=0x8D, .b=0x43},
			 {.r=0x35, .g=0x28, .b=0x79},
			 {.r=0xB8, .g=0xC7, .b=0x6F},
			 {.r=0x6F, .g=0x4F, .b=0x25},
			 {.r=0x43, .g=0x39, .b=0x00},
			 {.r=0x9A, .g=0x67, .b=0x59},
			 {.r=0x44, .g=0x44, .b=0x44},
			 {.r=0x6C, .g=0x6C, .b=0x6C},
			 {.r=0x9A, .g=0xD2, .b=0x84},
			 {.r=0x6C, .g=0x5E, .b=0xB5},
			 {.r=0x95, .g=0x95, .b=0x95}
};

Uint8 conv_byte2Uint8(char*byte){
  int i=0;
  Uint8 sum = 0;
  for(i=0; i<8;i++)
    if(byte[i] == '1'){
      sum = (sum << 1) + 1;
    }else
      sum = sum << 1;
  return sum;
}

void cp_Uint8tobyte(Uint8 n, char* byte){
  int i=0;
  Uint8 bitinbyte = 1;
  for(i=0; i<8;i++){
    if(n&&bitinbyte){
      byte[7-i]='1';
    }else{
      byte[7-i]='0';
    }
    bitinbyte=bitinbyte<<1;
  }
  byte[i]='\0';
}

void printerr(char *errmsg,int errcode){

  switch(errcode){
  default:
  case 0:
    fprintf(stderr,"An Error has occurred: \"%s\"\n",errmsg);
    break;
    /*SDL-Errors (not used)*/
  case 1:
    //fprintf(stderr,"An Error has occurred: \"%s\"\n\"%s\"\n",errmsg,SDL_GetError());
    break;
  case 2:
    //fprintf(stderr,"An Error has occurred: \"%s\"\n\"%s\"\n",errmsg,TTF_GetError());
    break;
    /*Dll-errors (not used)*/
  case 3:
    /*dynamic linked library*/
    //fprintf(stderr,"An Error has occurred: \"%s\"\n\"%s\"\n",errmsg,dlerror());
    break;
  case 4: 
    /*perror output*/
    fprintf(stderr,"An Error has occurred: \n");
    perror(errmsg);
    break;
  }
}


/* mem_cell_t *get_visual_cell_from_shm(visualC64_t *visual, unsigned int i){ */
/*   visual->c64.memory[i].value = conv_byte2Uint8(shm + i*(8+1)); */
/*   return &(visual->c64.memory[i]); */
/* } */


static int draw_handle_key(visualC64_t *visual, SDL_KeyboardEvent *event){
  SDL_keysym *keysym = &(event->keysym);

  // handle all the diffrent keyboard events
  switch (event->type){
  case SDL_KEYDOWN:{
    switch(keysym->sym){
    case SDLK_LEFT:
      // left key pressed
      if(visual->mem_visible_cells.x > 0){
	// we can go one more to the left
	visual->mem_visible_cells.x--;
      }
      break;
    case SDLK_RIGHT:
      // right key pressed
      if(visual->mem_visible_cells.x+visual->mem_visible_cells.w < visual->mem_dimensions.w){
	// we can go one more to the right
	visual->mem_visible_cells.x++;
      }
      break;
    case SDLK_UP:
      // key up pressed
      if(visual->mem_visible_cells.y > 0){
	// we can go one more to the top
	visual->mem_visible_cells.y--;
      }
      break;
    case SDLK_DOWN:
      // key down pressed
      if(visual->mem_visible_cells.y+visual->mem_visible_cells.h < visual->mem_dimensions.h){
	// we can go one more to the bottom
	visual->mem_visible_cells.y++;
      }
      break;
    case SDLK_r:
      // reset visual
      visual->mem_visible_cells.x = 0x0;
      visual->mem_visible_cells.y = 0x0;
      visual->mem_visible_cells.w = visual->mem_dimensions.w;
      visual->mem_visible_cells.h = visual->mem_dimensions.h;
    default:
      printerr("draw_handle_key(): unhandled keysymbol",0);
      break;
    }
    break;}
  case SDL_KEYUP:{
			
    break;}
  default:{
    printerr("draw_handler_key(): Event type not found",0);
    return -1;
    break;}
  }

  return 1;
}
void paint_frame(SDL_Surface* screen, 
		 int mem_idx , // (0x0000+visual->mem_visible_cells.x+ visual->mem_visible_cells.y * 32) * (8+1),
		 int boxx, int boxy){ //(SURFACE_WIDTH / 32)+zoom,(SURFACE_HEIGHT / 32)+zoom){
  SDL_Rect rect = {
    .x = 0,
    .y = 0,
    .w = boxx,
    .h = boxy
  };

  //int boxx (SURFACE_HEIGHT / 32)+zoom,
  int i;
  SDL_Surface *message;
  SDL_Color font_color = {
    .r 		= 0x02 ^ 0xFF,
    .g 		= 0x3f ^ 0xFF,
    .b 		= 0x35 ^ 0xFF,
    .unused 	= 0x00 ^ 0xFF
  };
  int nboxx = 32;
  if(boxx > 20 && boxy > 20){
    for(i=0;i<nboxx;i++){
      rect.y = (i+1)*boxy;
      rect.x = (i+2)*boxx;
      // convert value to text
      char string[4+1] = {'\0'};
      snprintf(	string,
		sizeof(string),
		//				"%#2.2X",
		"%04x",
		(mem_idx+(i* nboxx))&0xffff
		);
      // render message
      message = TTF_RenderText_Solid(font,string,font_color);
      // blit message to screen
      int dummy = rect.x;
      rect.x=0;

      if(SDL_BlitSurface(message,NULL,screen,&rect)<0){
	printerr("draw_cell(): Font blitting failed",0);
	return -1;
      }
      rect.x = dummy;
      rect.y = 0;
      // render message
      snprintf(	string,
		sizeof(string),
		//				"%#2.2X",
		"%02x",
		(mem_idx+i)&0xff
		);
      message = TTF_RenderText_Solid(font,string,font_color);
      if(SDL_BlitSurface(message,NULL,screen,&rect)<0){
	printerr("draw_cell(): Font blitting failed",0);
	return -1;
      }

    }
    // free message surface again
    SDL_FreeSurface(message);
  }
}
/* itoa:  convert n to characters in s */

 
/* reverse:  reverse string s in place */
void reverse(char s[])
{
  int i, j;
  char c;
  
  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}
void itoa(int n, char s[])
{
  int i, sign;
 
  if ((sign = n) < 0)  /* record sign */
    n = -n;          /* make n positive */
  i = 0;
  do {       /* generate digits in reverse order */
    s[i++] = n % 2 + '0';   /* get next digit */
  } while ((n /= 2) > 0);     /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}

int paint_mem(char* shm, int baseadr, SDL_Surface* screen,Uint16 boxx, Uint16 boxy,Uint16 nboxx, Uint16 nboxy){
  // draw all the memory cells
  int i;
  /* Uint16 boxx = 20; */
  /* Uint16 boxy = 20; */

  /* Uint16 nboxx = SURFACE_WIDTH / boxx ; */
  /* Uint16 nboxy = SURFACE_HEIGHT / boxy; */

  Uint16 x = 0;
  Uint16 y = 0;
  Uint8 c64byte = 0;

  // the color values
  Uint8 red;
  Uint8 green;
  Uint8 blue;
  Uint8 alpha;


  for(i=0; i<nboxx*nboxy;i++){
    //		for(i=0;i<MEMORY_SIZE;i++){

    int memadr = baseadr+i;
    c64byte = conv_byte2Uint8(shm + i*(8+1));
    red = (Uint8)((c64byte&(0x3<<RED_BITS))>>RED_BITS)<<0x6;
    green = (Uint8)((c64byte&(0x3<<GREEN_BITS))>>GREEN_BITS)<<0x6;
    blue = (Uint8)((c64byte&(0x3<<BLUE_BITS))>>BLUE_BITS)<<0x6;
    alpha = (Uint8)(((c64byte&(0x3<<ALPHA_BITS))>>ALPHA_BITS)<<0x6)|0x3F;

    // x =     0, 20, 40, 60, 
    x = (i % nboxx) * boxx;
    // y = 0,0,0,0,0,0,..,0,1,1,1,1,..,1,2,2,2,..
    y = (i / nboxx) * boxy;

    //    boxColor(screen, x, y, x+boxx, y+boxy, colors[c64byte%16].r&&colors[c64byte%16].g&&colors[c64byte%16].b);
    //    boxColor(screen, x, y, x+boxx, y+boxy, (colors[c64byte%16].r<<16)|(colors[c64byte%16].g<<8)|colors[c64byte%16].b);
    if(memadr== (((conv_byte2Uint8(stateptr->pch)&0x00ff)<<8) | (conv_byte2Uint8(stateptr->pcl)&0x00ff)) ){
      boxRGBA(screen, x, y, x+boxx, y+boxy, 0xff, 0xa5, 0x00, 0xff);
      boxRGBA(screen, x+4, y+4, x+boxx-4, y+boxy-4, red, green, blue, alpha);
    } else {
      //      boxColor(screen, x, y, x+boxx, y+boxy, colors);
      //boxRGBA(screen, x, y, x+boxx, y+boxy, red, green, blue, alpha);
      boxRGBA(screen, x, y, x+boxx, y+boxy, colors[c64byte%16].r, colors[c64byte%16].g, colors[c64byte%16].b, 255);
    }

    // is the box big enough?
    SDL_Rect rect = {
      .x = x, // (x1+CELL_BORDERS < screen->w) ? x1+CELL_BORDERS : screen->w ,
      .y = y, // (y1+CELL_BORDERS < screen->h) ? y1+CELL_BORDERS : screen->h ,
      .w = boxx, // (x2-x1 > CELL_BORDERS) ? x2-x1-CELL_BORDERS : 0,
      .h = boxy //(y2-y1 > CELL_BORDERS) ? y2-y1-CELL_BORDERS : 0,
    };
    if(boxx > 20 && boxy > 20){
      SDL_Surface *message;
      SDL_Color font_color = {
	.r 		= red ^ 0xFF,
	.g 		= green ^ 0xFF,
	.b 		= blue ^ 0xFF,
	.unused 	= alpha ^ 0xFF
      };
      // convert value to text
      char string[4+1] = {'\0'};
      snprintf(	string,
		sizeof(string),
		//				"%#2.2X",
		"%02x",
		c64byte
		);
      // render message
      message = TTF_RenderText_Solid(font,string,font_color);
      // blit message to screen
      if(SDL_BlitSurface(message,NULL,screen,&rect)<0){
	printerr("draw_cell(): Font blitting failed",0);
	return -1;
      }
      // free message surface again
      SDL_FreeSurface(message);
    }
    
  }
}
// DRAW mouse selector
int paint_mouse_selector(  SDL_Surface *screen, visualC64_t *visual, int mem_idx, int boxx, int boxy){
  int col , row;
  int x,y;
  int xd, yd;
  x = visual->mouse_selector.x;
  y = visual->mouse_selector.y;
  (col = x / boxx)<2 ? (col=2) : col;
  (row = y / boxy)<1 ? (row=1) : row;
  //  row = y / boxy;
  xd =  col*boxx;
  yd =  row * boxy;
  if(boxColor(screen,xd,yd,
		//		visual->mouse_selector.x+visual->mouse_selector.w,
	      xd + boxx,
		//		visual->mouse_selector.y+visual->mouse_selector.h,
		yd + boxy,
		MOUSE_SELECTOR_COLOR)<0){
      printerr("screen_loop(): Mouse selector draw failed",0);
      return -1;
    }
}

SDL_Surface *message;
    /* int nboxx = 32 ; // SURFACE_WIDTH / 32 ; */
    /* int nboxy = 64 ; // SURFACE_HEIGHT / 32 ; */
    /* int mem_idx = visual->mem_visible_cells.x+ visual->mem_visible_cells.y * nboxx ; */

void paint_info(  SDL_Surface *screen, visualC64_t *visual, int mem_idx, int boxx, int boxy, int nboxx, int nboxy){
  int col = visual->mouse_selector.x / boxx -2;
  col = (col>0)? col : 0;
  int row = visual->mouse_selector.y / boxy -1;
  row = (row>0)? row : 0;
  int memadr;
  memadr = mem_idx + col + row* nboxx;
  memadr = (memadr > MEMORY_SIZE) ? 0 : memadr;
    int c64byte = conv_byte2Uint8(shm + memadr*(8+1));
    SDL_Color font_color = {
      .r 		= 0x0,
      .g 		= 0x0,
      .b 		= 0x0,
      .unused 	= 0x0
    };
    SDL_Surface *message;				
    char string[240+1] = {'\0'};
		
    SDL_Rect rect;// = { .x = 2*boxx, .y=boxy, .w = SURFACE_WIDTH , .h=SURFACE_HEIGHT-460};
    snprintf(	string,
		sizeof(string),
		//				"%#2.2X",
		"Mem[%04x] = %02x"
		"  row: %d col: %d",
		memadr, c64byte,
		row, col
		);
    rect.x=40;
    rect.y=SURFACE_HEIGHT-100;
    rect.w=SURFACE_WIDTH;
    rect.h=SURFACE_HEIGHT;
    SDL_FillRect(screen,&rect,0xffffffff);		
    message = TTF_RenderText_Solid(systemfont,string,font_color);
    //		SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
    if(SDL_BlitSurface(message,NULL,visual->screen,&rect)<0){
      printerr("draw_cell(): Font blitting failed",0);
      return -1;
    }
	
    char buffer[] ="00000000";	
    itoa(conv_byte2Uint8(stateptr->flags),buffer); 

    Uint8 pch = conv_byte2Uint8(stateptr->pch);
    Uint8 pcl =  conv_byte2Uint8(stateptr->pcl);
    Uint8 acc =  conv_byte2Uint8(stateptr->acc);
    Uint8 idx =  conv_byte2Uint8(stateptr->idx);
    Uint8 idy = conv_byte2Uint8(stateptr->idy);
    Uint8 sp = conv_byte2Uint8(stateptr->sp);
    Uint8 ir = conv_byte2Uint8(stateptr->ir);
    //snprintf(string, sizeof(string), "pc: %02x%02x ACC: %02x pc: %02x%02x ACC: asdasdas asd asd asd asdasd asd asd iuius oduaosd oasduoasdaoisd ", conv_byte2Uint8(stateptr->pch), conv_byte2Uint8(stateptr->pcl), conv_byte2Uint8(stateptr->acc), conv_byte2Uint8(stateptr->pch), conv_byte2Uint8(stateptr->pcl));
    snprintf(string, sizeof(string), "%02x%02x %02x %02x %02x %02x %02x %s", pch, pcl, acc, idx, idy, sp, ir, stateptr->flags);
    rect.x=140;
    rect.y=SURFACE_HEIGHT-60;
    rect.w=SURFACE_WIDTH;
    rect.h=SURFACE_HEIGHT;
    message = TTF_RenderText_Solid(systemfont,string,font_color);
    //		SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
    if(SDL_BlitSurface(message,NULL,visual->screen,&rect)<0){
      printerr("draw_cell(): Font blitting failed",0);
      return -1;
    }
    snprintf(string, sizeof(string), " PC  AC XR YR SP IR NV-BDIZC");//, pch, pcl, acc, idx, idy, sp, ir, stateptr->flags);
    rect.x=140;
    rect.y=SURFACE_HEIGHT-80;
    message = TTF_RenderText_Solid(systemfont,string,font_color);
    //		SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
    if(SDL_BlitSurface(message,NULL,visual->screen,&rect)<0){
      printerr("draw_cell(): Font blitting failed",0);
      return -1;
    }
    
}


int screen_loop(visualC64_t *visual){
  Uint32 loop_time_avg = 1000/FPS; // time per loop
  SDL_Surface *screen = visual->screen;

  /* Set palette */
  SDL_SetColors(screen, colors, 0, 16);

  int zoom;
  // visual is now running
  visual->running = true; 
  while(visual->running == true){
    //unsigned int i;
    // timer stuff
    Uint32 loop_start_time = SDL_GetTicks();
    Uint32 loop_time;
    // event stuff
    SDL_Event event;

    // get all the scren related events
    // TODO: to thread
    while(SDL_PollEvent(&event) == 1){
      // handle events
      switch(event.type){
      case SDL_QUIT:{	// want to quit
	visual->running = false;
	break;
      }
       //If the window resized
      case SDL_VIDEORESIZE: {
        //Resize the screen
	// screen = SDL_SetVideoMode( event.resize.w, event.resize.h, SURFACE_BPP, SDL_SWSURFACE | SDL_RESIZABLE );
	
	break;  
    }
case SDL_MOUSEBUTTONUP:
	switch(event.button.button)
	  {
	  case SDL_BUTTON_WHEELUP:
	    zoom += 1;
	    printf("zoom out: \t%d\n", zoom);
	    break;

	  case SDL_BUTTON_WHEELDOWN:
	    zoom -= 1;
	    printf("zoom in: \t%d\n", zoom);
	    break;
	  }
	break;
	/* case SDL_MOUSEWHEEL:{	// mouse wheel movement */
	/* 	 printf("mousewheel: y:%d\n" */
	/* 				"cell_x:%d\tcell_y:%d\n", */
	/* 				event.wheel.y */
	/* 				x,y);  */
	/* 	break;} */
      case SDL_KEYDOWN:{
	if(draw_handle_key(visual,(SDL_KeyboardEvent *)&event)<0){
	  printerr("screen_loop(): Keyevent failed",0);
	  return -1;
	}	
	break;}
	/* case SDL_MOUSEBUTTONDOWN:{ */
	/* 	// start select with mouse */
	/* 	SDL_MouseButtonEvent *mousedown = &(event.button); */
	/* 	if(draw_get_visual_cell_coords(visual,mousedown->x,mousedown->y,&(visual->mouse_drag_start_x),&(visual->mouse_drag_start_y))<0){ */
	/* 		printerr("screen_loop(): Failed getting cell coordinates",0); */
	/* 		return -1; */
	/* 	} */
	/* break;} */
	/* case SDL_MOUSEBUTTONUP:{ */
	/* 	// stop select with mouse */
	/* 	SDL_MouseButtonEvent *mouseup = &(event.button); */
	/* 	Uint16 x; */
	/* 	Uint16 y; */

	/* 	// get coorinates of cell */
	/* 	if(draw_get_visual_cell_coords(visual,mouseup->x,mouseup->y,&(x),&(y))<0){ */
	/* 		printerr("screen_loop(): Failed getting cell coordinates",0); */
	/* 		return -1; */
	/* 	} */

	/* 	if(visual->mouse_drag_start_x <= x){ */
	/* 		// select from left to right */
	/* 		visual->mem_visible_cells.x = visual->mouse_drag_start_x; */
	/* 		visual->mem_visible_cells.w = x-visual->mouse_drag_start_x; */
	/* 	}else{ */
	/* 		// select from right to left */
	/* 		visual->mem_visible_cells.x = x; */
	/* 		visual->mem_visible_cells.w = visual->mouse_drag_start_x-x; */
	/* 	} */
	/* 	if(visual->mem_visible_cells.w < visual->mem_dimensions.w){ // one cell more */
	/* 		visual->mem_visible_cells.w++; */
	/* 	} */

	/* 	if(visual->mouse_drag_start_y <= y){ */
	/* 		// select from top to bottom */
	/* 		visual->mem_visible_cells.y = visual->mouse_drag_start_y; */
	/* 		visual->mem_visible_cells.h = y-visual->mouse_drag_start_y; */
	/* 	}else{ */
	/* 		// select from bottom to top */
	/* 		visual->mem_visible_cells.y = y; */
	/* 		visual->mem_visible_cells.h = visual->mouse_drag_start_y-y; */
	/* 	} */
	/* 	if(visual->mem_visible_cells.h < visual->mem_dimensions.h){ */
	/* 		visual->mem_visible_cells.h++; */
	/* 	} */
	/* break;} */
      case SDL_MOUSEMOTION:{
	SDL_MouseMotionEvent *mousemotion = &event.motion;
				  
	int nboxx = 32 ; // SURFACE_WIDTH / 32 ;
	int nboxy = 32 ; // SURFACE_HEIGHT / 32 ;
				  
	int mem_base = visual->mem_visible_cells.x+ visual->mem_visible_cells.y * nboxx ;
				  
	int boxx = (SURFACE_WIDTH / nboxx)+zoom ; // DEFAULT_BOXX + zoom; 
	int boxy = (SURFACE_HEIGHT / nboxy)+ zoom; // DEFAULT_BOXY + zoom; 
	int col = ((mousemotion->x)  / boxx)+1;
	col = (col<0) ? 0 : col;
	int row = ((mousemotion->y)  / boxy) ;
	row = (row<0) ? 0 : row;
	int memadr;
	memadr = mem_base + col + row* nboxx;
	memadr = (memadr > MEMORY_SIZE) ? 0 : memadr;
	int c64byte = conv_byte2Uint8(shm + memadr*(8+1));
				  
	visual->mouse_selector.x = mousemotion->x ; // (col+1) * boxx;
	visual->mouse_selector.y = mousemotion->y ; //(row+1) * boxy;
	visual->mouse_selector.w = boxx;
	visual->mouse_selector.h = boxy;

	// print movement out
	printf(	"mouse_x:%d\tmouse_y:%d\n"
		"col: %4d \t row: %4d\n"
		"mem[%4x] = %4x\n"
		"recx  = %d     \t recy  = %d\n"
		"recx+ = %d     \t recy+ = %d\n",
		mousemotion->x,mousemotion->y,
		col, row,// (mousemotion->x) / 20, (mousemotion->y) / 20,
		memadr, c64byte, //((mousemotion->x) / 20) + (((mousemotion->y) / 20) * (800 / 20)), c64byte,
		visual->mouse_selector.x, visual->mouse_selector.y,
		visual->mouse_selector.w, visual->mouse_selector.h);
					
				


					
	break;}
      default:{
	printerr("screen_loop(): unkown event!",0);
	//	return -1;
	break;}
      }
    }

    // draw blank screen
    if(boxColor(screen,0x0,0x0,SURFACE_WIDTH,SURFACE_HEIGHT,SCREEN_BACKGROUND_COLOR)<0){
      printerr("screen_loop(): Screen background color draw failed",0);
      return -1;
    }
    if(boxColor(visual->mem_surface,0x0,0x0,SURFACE_WIDTH,SURFACE_HEIGHT,SCREEN_BACKGROUND_COLOR)<0){
      printerr("screen_loop(): Screen background color draw failed",0);
      return -1;
    }

    int nboxx = 32 ; // SURFACE_WIDTH / 32 ;
    int nboxy = 64 ; // SURFACE_HEIGHT / 32 ;
    int mem_idx = visual->mem_visible_cells.x+ visual->mem_visible_cells.y * nboxx ;
    int boxx = (SURFACE_WIDTH / nboxx)+zoom ; // DEFAULT_BOXX + zoom; 
    int boxy = (SURFACE_HEIGHT / nboxy)+ zoom; // DEFAULT_BOXY + zoom; 
    char*shmptr = shm+(mem_idx*(8+1));

    //		paint_mem(shm+, screen,DEFAULT_BOXX + zoom, DEFAULT_BOXX+zoom);
    //		paint_mem(shm+(0x0200*(8+1)), screen,SURFACE_HEIGHT / 32, SURFACE_WIDTH / 32);
    paint_mem(shmptr, mem_idx, visual->mem_surface, boxx, boxy,32,64);//nboxx, nboxy);

    // blit mem_surface to screen

    //		if(SDL_BlitSurface(visual->screen,NULL,visual->mem_surface,NULL)<0){
    //		SDL_Rect rec = { .x = boxx, .y=boxy, .w = SURFACE_WIDTH - boxx, .h=SURFACE_HEIGHT - boxy};
    SDL_Rect rect = { .x = 2*boxx, .y=boxy, .w = SURFACE_WIDTH , .h=SURFACE_HEIGHT-460};
    //		SDL_BlitSurface(message,NULL,screen,&rect)<0){
    //	   	if(SDL_BlitSurface(visual->screen,NULL,visual->mem_surface,&rec)<0){
    if(SDL_BlitSurface(visual->mem_surface,NULL,visual->screen,&rect)<0){
      printerr("screen_loop(): failed blitting the mem surface",0);
      return -1;
    }

    paint_frame(screen, mem_idx, boxx, boxy);		

    // draw mouse selector
    paint_mouse_selector(screen, visual, mem_idx, boxx, boxy);		

    

    // DRAW bottom line
    paint_info(screen, visual,mem_idx, boxx, boxy,nboxx, nboxy);
    // flip the screen
    //if(SDL_Flip(visual->mem_surface)<0){
    if(SDL_Flip(screen)<0){
      printerr("screen_loop(): screen flipping failed!",0);
      return -1;
    }

    // shall we sleep?
    loop_time = SDL_GetTicks();
    if((loop_time-loop_start_time) < loop_time_avg){
      Uint32 sleep_time = loop_time_avg-(loop_time-loop_start_time);
      SDL_Delay(sleep_time);
    }
  }
  return 1;
}

int main(int argc,char *argv[]){
  //shared memory stuff


  int shmid;
  size_t shmsz =  MEMORY_SIZE * (8+1) * sizeof(char);
  /*
   * Locate the segment.
   */
  key_t key = SHM_KEY;
  if ((shmid = shmget(key, shmsz, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  
  /*
    Now we attach the segment to our data space.
  */
  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }

  shmsz =  sizeof(struct cpu_state);
  key = SHM_CPU_KEY;
  if ((shmid = shmget(key, shmsz, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  
  /*
    Now we attach the segment to our data space.
  */
  if ((stateptr = shmat(shmid, NULL, 0)) == (struct cpu_state *) -1) {
    perror("shmat");
    exit(1);
  }

  visualC64_t visual = {
    .screen			= NULL,
    .running			= false,
    .mouse_drag_start_x	= 0x0,
    .mouse_drag_start_y	= 0x0,
    .xmax = 0x20,
    .ymax = 0x10,
    .xcursor = 0x0,
    .ycursor = 0x0,
    .mem_surface		= NULL,
    .mem_dimensions =	{
      .x 	= 0,
      .y 	= 0,
      .w 	= 256,
      .h 	= 256,
    },
    .mem_visible_cells = {
      .x	= 0x0,
      .y	= 0x0,
      .w 	= visual.mem_dimensions.w,
      .h 	= visual.mem_dimensions.h,
    },
    .mouse_selector =	{		
      .x	= 0x0,
      .y	= 0x0,
      .w 	= 0x0,
      .h 	= 0x0,
    },
    .font				= NULL,
  };

  // init visualC64_t

  visual.mem_visible_cells.w 		= 0xa;
  visual.mem_visible_cells.h 		= 0xa;

  // init sdl
  if(SDL_Init(SDL_INIT_VIDEO | SDL_RESIZABLE)<0){
    printerr("main(): SDL-init failed",0);
    return EXIT_FAILURE;
  }

  // init ttf subsystem
  if(TTF_Init()<0){ 
    return EXIT_FAILURE;
  }

  // load the font
  visual.font = TTF_OpenFont(FONT_PATH,FONT_PTSIZE);
  systemfont = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf",20);
  font = TTF_OpenFont(FONT_PATH,FONT_PTSIZE);
  if(visual.font == NULL){
    printerr("main(): Error loading font",0);
    return EXIT_FAILURE;
  }
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  // init screen
  //	visual.screen = SDL_SetVideoMode(SURFACE_WIDTH,SURFACE_HEIGHT,SURFACE_BPP,SDL_SWSURFACE|SDL_NOFRAME);
  visual.screen = SDL_SetVideoMode(SURFACE_WIDTH,SURFACE_HEIGHT,SURFACE_BPP,SDL_SWSURFACE | SDL_RESIZABLE);
  if(visual.screen == NULL){
    printerr("main(): SDL_SetVideoMode failed!",0);
    return EXIT_FAILURE;
  }

  // init memory surface
  visual.mem_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, SURFACE_WIDTH,SURFACE_HEIGHT,
					    //MEM_SURFACE_W,
					    //MEM_SURFACE_H,
					    visual.screen->format->BitsPerPixel,
					    visual.screen->format->Rmask,
					    visual.screen->format->Gmask,
					    visual.screen->format->Bmask,
					    visual.screen->format->Amask);
  if(visual.mem_surface == NULL){
    printerr("main(): Memory surface initialization failed",0);
    return EXIT_FAILURE;
  }

  // random memroy
  /* srand(time(NULL)); */
  /* for(i=0;i<MEMORY_SIZE;i++){ */
  /* 	visual.c64.memory[i].value = random(); */
  /* } */

  // start scren drawing
  if(screen_loop(&visual)<0){
    printerr("main(): Screen redrawing failed!",0);
    return EXIT_FAILURE;
  }

  // close the font again
  TTF_CloseFont(visual.font);

  // free the surface again!
  SDL_FreeSurface(visual.screen);
  SDL_FreeSurface(message);
  //	SDL_FreeSurface(visual.screen);

  // Tear down ttf and sdl
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}

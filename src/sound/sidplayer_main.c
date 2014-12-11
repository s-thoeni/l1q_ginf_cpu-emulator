#include "SDL.h" 
#include "SDL_audio.h" 
#include "SDL_mixer.h"
//#include "SDL_gfxPrimitives.h"
#include <stdlib.h> 

#define _USE_MATH_DEFINES
#include <math.h> 
#include <stdio.h>
#include <SDL_types.h>
typedef Uint8 uint8;
typedef Sint8 int8;
typedef Uint16 uint16;
typedef Sint16 int16;
typedef Uint32 uint32;
typedef Sint32 int32;
typedef Uint64 uint64;
typedef Sint64 int64;

extern void cSIDInit();
extern void cSIDExit();
extern void csid_write(uint32 adr, uint32 byte, uint32 now, int rmw);

static void quit()
{
	cSIDExit();
	SDL_Quit();
}

int main (){
  // Initialize everything
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
	fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
	exit(1);
    }
    atexit(quit);

    //    SDL_PauseAudio(false);

    //InitAll(1, "./be");

    cSIDInit();
    unsigned int now = 0;
    //    sid_write(uint32 adr, uint32 byte, cycle_t now, bool rmw);
	fprintf(stderr, "STart 	csid_write(0,0xd6, now, 123);\n");
	csid_write(0,0xd6, now, 123);
	fprintf(stderr, "END 	csid_write(0,0xd6, now, 123);\n");
	csid_write(24,0x1E, now, 123);
	now+=5;  
	csid_write(1,0x1c, now, 123);
  now+=5;  
  csid_write(7,0x55, now, 123);
  now+=5;  
  csid_write(8,0x24, now, 123);
  now+=5;  
  csid_write(4,0x11, now, 123);
  now+=5;  
  csid_write(11,0x11, now, 123);
  SDL_Delay(1000);
  csid_write(4,0x10, now, 123);
  now+=5;  
  csid_write(11,0x10, now, 123);
  SDL_Delay(1000);


  csid_write(5,0xa8, now, 123);
  now+=5;  
  csid_write(6,0xff, now, 123);
  now+=5;  
  csid_write(12,0xa8, now, 123);
  now+=5;  
  csid_write(13,0xFF, now, 123);	

  now+=5;  
  csid_write(4,0x11, now, 123);
  now+=5;  
  csid_write(11,0x11, now, 123);
  SDL_Delay(1000);
  csid_write(4,0x10, now, 123);
  now+=5;  
  csid_write(11,0x10, now, 123);
  SDL_Delay(1000);
  fprintf(stderr, "END END\n");

    while (1) {
	SDL_Event e;
	if (SDL_WaitEvent(&e)) {
	    if (e.type == SDL_QUIT)
		break;
	}
    }

    //    SIDInit()  ->  void SIDCalcBuffer(uint8 *buf, int count) -> calc_buffer ->	desired.callback = calc_buffer;

    cSIDExit();
    //    makefile: #define  SID_PLAYER 0
    //    make CFLAGS=-DMACRO
	return 0;
}

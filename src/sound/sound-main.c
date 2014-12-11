#include <time.h>
#include "SDL.h" 
#include "SDL_audio.h" 
#include "SDL_mixer.h"
//#include "SDL_gfxPrimitives.h"
#include <stdlib.h> 

#define _USE_MATH_DEFINES
#include <math.h> 

#include "sid_6581.h"

//#include "sdl-waveform-generator.h"


struct sid_6581_filter sid_filter;

struct envelope voice_a_env,voice_b_env,voice_c_env;
struct oscillator voice_a_osc,voice_b_osc,voice_c_osc;

struct sid_6581_voice va = { 7382,  2048 ,  0 | SID_SAWT , 240, 114, 12, 24, &voice_a_osc, & voice_a_env , NULL};
struct sid_6581_voice vb = { 17382, 2048 ,  0 | SID_SAWT , 660, 114, 8, 24, &voice_b_osc, & voice_b_env, NULL};
struct sid_6581_voice vc = { 7382, 2048 ,  0 | SID_SAWT , 240, 114, 12, 24, &voice_c_osc, & voice_c_env, NULL};

struct sid_6581 sid = { &va, &vb, &vc, &sid_filter };
struct cpu_emu_sid emu_sid ;

SDL_Surface *screen;
int xres = 640;
int yres = 480; 

SDL_AudioSpec spec; 


struct sid_6581_voice *interface_voice;
Mix_Chunk *sound = NULL; 



void handleKey(SDL_KeyboardEvent key) {
  switch(key.keysym.sym) {
  case SDLK_t: // triangle
	interface_voice->control = ((interface_voice->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
    break;
  case SDLK_s: // sawtooth wave
	interface_voice->control = ((interface_voice->control | SID_SAWT )  & ~SID_PULS & ~SID_TRIA & ~SID_NOIS);
    break;
  case SDLK_p: // puls with modulation
	interface_voice->pw =  interface_voice->pw +1 + 0.1 * interface_voice->pw;
	interface_voice->control = ((interface_voice->control | SID_PULS )  & ~SID_SAWT & ~SID_TRIA & ~SID_NOIS);
	break;
  case SDLK_1: // note 1 anspielen
	interface_voice->frequency = 7382;
	interface_voice->attack=200;
	interface_voice->decay=20;
	interface_voice->sustain = 8;
	interface_voice->release=20;
	(key.state == SDL_PRESSED) ?  ( interface_voice->control = interface_voice->control | SID_GATE ) : 0 ;
	(key.state == SDL_RELEASED) ? ( interface_voice->control = interface_voice->control & ~SID_GATE ) : 0 ;
	break;
  case SDLK_2: // note 2 anspielen
	interface_voice->frequency = 6577;
	interface_voice->attack=2;
	interface_voice->decay=70;
	interface_voice->sustain = 12;
	interface_voice->release=114;
	(key.state == SDL_PRESSED) ?  ( interface_voice->control = interface_voice->control | SID_GATE ) : 0 ;
	(key.state == SDL_RELEASED) ? ( interface_voice->control = interface_voice->control & ~SID_GATE ) : 0 ;
	break;
  case SDLK_3: // note 3 anspielen
	interface_voice->frequency = 5859;
	interface_voice->attack=2;
	interface_voice->decay=300;
	interface_voice->sustain = 12;
	interface_voice->release=3000;
	(key.state == SDL_PRESSED) ?  ( interface_voice->control = interface_voice->control | SID_GATE ) : 0 ;
	(key.state == SDL_RELEASED) ? ( interface_voice->control = interface_voice->control & ~SID_GATE ) : 0 ;
	break;
  case SDLK_4: // note 4 anspielen
	interface_voice->frequency = 5530;
	interface_voice->attack=800;
	interface_voice->decay=6;
	interface_voice->sustain = 15;
	interface_voice->release=3000;
	(key.state == SDL_PRESSED) ?  ( interface_voice->control = interface_voice->control | SID_GATE ) : 0 ;
	(key.state == SDL_RELEASED) ? ( interface_voice->control = interface_voice->control & ~SID_GATE ) : 0 ;
	printf("KEy 1  pressed %c\n",key.keysym.sym);
    break;
  case SDLK_a: 
	interface_voice  = sid.voicea;
	break;
  case SDLK_b: 
	interface_voice = sid.voiceb;
	break;
  case SDLK_c: 
	interface_voice = sid.voicec;
	break;
  case SDLK_ESCAPE:
	break;
  default:
    /* do nothing */
    break;
  }
}


int main(int argc, char* argv[]){

  // AUDIO_U8 * smpls = calloc(sizeof(AUDIO_U8),WAVEFORM_LENGTH);
  SNDFMT* smpls = calloc(sizeof(SNDFMT),WAVEFORM_LENGTH);
  //  smpls = snd_load("/media/c5bcf67f-e9eb-4e12-be75-d8b6e09e27ba/olivier/hti/ginf/cpu-emu/cpu-emu/sin.table");
  

  /* // start SDL with audio support */
  
    if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
  //  if (SDL_Init (SDL_INIT_VIDEO) < 0)
    exit (-1);

  atexit (SDL_Quit);
  screen = SDL_SetVideoMode (640, 480, 16, SDL_HWSURFACE);

  if (screen == NULL)
    exit (-1);

  // open SMPFREQ, unsigned 8bit, system byte order,
  //      mono audio, using 1024 byte chunks
  if(Mix_OpenAudio(SMPFREQ, AUDIO_U8, 1, WAVEFORM_LENGTH)==-1) {
    printf("Mix_OpenAudio: %s\n", Mix_GetError());
      exit (-1);
  }

  // get and print the audio format in use
  int numtimesopened, frequency, channels;
  Uint16 format;
  numtimesopened=Mix_QuerySpec(&frequency, &format, &channels);
  if(!numtimesopened) {
    printf("Mix_QuerySpec: %s\n",Mix_GetError());
  }
  else {
    char *format_str="Unknown";
    switch(format) {
        case AUDIO_U8: format_str="U8"; break;
        case AUDIO_S8: format_str="S8"; break;
        case AUDIO_U16LSB: format_str="U16LSB"; break;
        case AUDIO_S16LSB: format_str="S16LSB"; break;
        case AUDIO_U16MSB: format_str="U16MSB"; break;
        case AUDIO_S16MSB: format_str="S16MSB"; break;
    }
    printf("opened=%d times  frequency=%dHz  format=%s  channels=%d",
            numtimesopened, frequency, format_str, channels);
  }

  


  sound = Mix_QuickLoad_RAW(smpls,WAVEFORM_LENGTH);


  if(sound == NULL) { 
     fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError()); 
   } 
  
  int channel= 1;

  channel = Mix_PlayChannel(-1, sound, -1);
  if(channel == -1) {
    fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
  }

  init_SID(&sid);
  
  if(!Mix_RegisterEffect(channel, sid_mixer, NULL, &sid )) {
    printf("Mix_RegisterEffect: %s\n", Mix_GetError());
  }
  /*
  if(!Mix_RegisterEffect(channel, adsrEffect0, NULL, sid.voiceb )) {
	printf("Mix_RegisterEffect: %s\n", Mix_GetError());
  }
  if(!Mix_RegisterEffect(channel, adsrEffect0, NULL, sid.voicec )) {
	printf("Mix_RegisterEffect: %s\n", Mix_GetError());
  }
  */ 

  /*

	play time

  */
  struct sid_6581_voice  *voicea = sid.voicea;
  struct sid_6581_voice  *voiceb = sid.voiceb;
  struct sid_6581_voice  *voicec = sid.voicec;

  int i=0,j=0;
  for(i=0;i<3;i++){
	voicea->frequency = 23436;
	voicea->control =  voicea->control | SID_GATE ;
	SDL_Delay (50);
	voicea->frequency = 7382;
	voicea->attack=200;
	voicea->decay=20;
	voicea->sustain = 8;
	voicea->release=20;
	voicea->attack = 200;
	voicea->control =  voicea->control & ~SID_GATE ;
	SDL_Delay (500);

	voicea->frequency = 14764;
	voicea->attack=200;
	voicea->decay=20;
	voicea->sustain = 6;
	voicea->release=1000;
	voicea->control =  voicea->control | SID_GATE ;
	for(j=0;j<10;j++){
	  SDL_Delay (10);
	  voicea->frequency = 14764+14764*j*1/100;
	}
	voicea->control =  voicea->control & ~SID_GATE ;	
	voicea->control =  voicea->control | SID_GATE ;
	SDL_Delay (50);
	voicea->control =  voicea->control & ~SID_GATE ;
	SDL_Delay (50);
	voicea->frequency = 10440;
	voicea->control =  voicea->control | SID_GATE ;
	SDL_Delay (50);
	voicea->control =  ((((voicea->control & ~SID_GATE) | SID_SAWT ) & ~SID_PULS )  & ~SID_TRIA ) & ~SID_NOIS ;
	SDL_Delay (50);
	voicea->frequency = 7382;
	voicea->control =  voicea->control | SID_GATE ;
	SDL_Delay (50);
	voicea->control =  voicea->control & ~SID_GATE ;
	SDL_Delay (50);
  }
  /*
	two voices
  */
    
  voicea->frequency = 7382;
  voicea->attack=200;
  voicea->decay=200;
  voicea->sustain = 6;
  voicea->release=1000;

  voiceb->frequency = 9301;
  voiceb->attack=120;
  voiceb->decay=20;
  voiceb->sustain = 6;
  voiceb->release=1000;

  voicea->control =  voicea->control | SID_GATE | SID_PULS ;
  voiceb->control =  voiceb->control | SID_GATE | SID_SAWT ;
  SDL_Delay(400);
  voicea->control =  voicea->control & ~SID_GATE ;
  voiceb->control =  voiceb->control & ~SID_GATE ;
  SDL_Delay(300);

  sid.voicea->frequency = 7382;
  sid.voiceb->frequency = 8286;
  sid.voicea->control =  voicea->control | SID_GATE ;
  sid.voiceb->control =  voiceb->control | SID_GATE ;
  SDL_Delay(400);
  sid.voicea->control =  voicea->control & ~SID_GATE ;
  sid.voiceb->control =  voiceb->control & ~SID_GATE ;
  SDL_Delay(300);

  sid.voicea->frequency = 7382;
  sid.voiceb->frequency = 8779;
  sid.voicea->control =  voicea->control | SID_GATE ;
  sid.voiceb->control =  voiceb->control | SID_GATE ;
  SDL_Delay(400);
  sid.voicea->control =  voicea->control & ~SID_GATE ;
  sid.voiceb->control =  voiceb->control & ~SID_GATE ;
  SDL_Delay(300);

  sid.voicea->frequency = 7382;
  sid.voiceb->frequency = 9301;
  sid.voicea->control =  voicea->control | SID_GATE ;
  sid.voiceb->control =  voiceb->control | SID_GATE ;
  SDL_Delay(400);
  sid.voicea->control =  voicea->control & ~SID_GATE ;
  sid.voiceb->control =  voiceb->control & ~SID_GATE ;
  SDL_Delay(300);

  /*
	three voces
  */
  sid.voicea->frequency = 7382;
  sid.voiceb->frequency = 8779;
  sid.voicec->frequency = 11060;
  sid.voicea->control = ((voicea->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
  sid.voiceb->control = ((voiceb->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
  sid.voicec->control = ((voicec->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
  sid.voicea->control =  voicea->control | SID_GATE ;
  sid.voiceb->control =  voiceb->control | SID_GATE ;
  sid.voicec->control =  voicec->control | SID_GATE ;
  SDL_Delay(700);
  sid.voicea->control =  voicea->control & ~SID_GATE ;
  sid.voiceb->control =  voiceb->control & ~SID_GATE ;
  sid.voicec->control =  voicec->control & ~SID_GATE ;
  SDL_Delay(300);

  sid.voicea->frequency = 8286;
  sid.voiceb->frequency = 10440;
  sid.voicec->frequency = 11718;
  sid.voicea->control = ((voicea->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
  sid.voiceb->control = ((voiceb->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
  sid.voicec->control = ((voicec->control | SID_TRIA )  & ~SID_PULS & ~SID_SAWT & ~SID_NOIS);
  sid.voicea->control =  voicea->control | SID_GATE ;
  sid.voiceb->control =  voiceb->control | SID_GATE ;
  sid.voicec->control =  voicec->control | SID_GATE ;
  SDL_Delay(700);
  sid.voicea->control =  voicea->control & ~SID_GATE ;
  sid.voiceb->control =  voiceb->control & ~SID_GATE ;
  sid.voicec->control =  voicec->control & ~SID_GATE ;
  SDL_Delay(300);

  /*
	play with cpu emu interface
  */
  write_sid(0,0xd6, &sid, &emu_sid);
  write_sid(1,0x1c, &sid, &emu_sid);

  write_sid(7,0x55, &sid, &emu_sid);
  write_sid(8,0x24, &sid, &emu_sid);

  write_sid(4,0x11, &sid, &emu_sid);
  write_sid(11,0x11, &sid, &emu_sid);
  SDL_Delay(1000);
  write_sid(4,0x10, &sid, &emu_sid);
  write_sid(11,0x10, &sid, &emu_sid);
  SDL_Delay(1000);


  write_sid(5,0xa8, &sid, &emu_sid);
  write_sid(6,0xff, &sid, &emu_sid);
  write_sid(12,0xa8, &sid, &emu_sid);
  write_sid(13,0xFF, &sid, &emu_sid);	

  write_sid(4,0x11, &sid, &emu_sid);
  write_sid(11,0x11, &sid, &emu_sid);
  SDL_Delay(1000);
  write_sid(4,0x10, &sid, &emu_sid);
  write_sid(11,0x10, &sid, &emu_sid);
  SDL_Delay(1000);
  SDL_Event event;
  int done=0;

  interface_voice = sid.voicea;

  while(!done) {
	while(SDL_PollEvent(&event)) {
	    
	  switch(event.type) {
	  case SDL_KEYDOWN:	
		switch(event.key.keysym.sym){
		case SDLK_LEFT:
		  (sid.filter->volume - 1) < 0 ? sid.filter->volume = 0 : sid.filter->volume--;
		  printf("SDLK_LEFT volume: %i\n",sid.filter->volume);
		  break;
		case SDLK_RIGHT:
		  (sid.filter->volume + 1) >15 ? sid.filter->volume = 15 : sid.filter->volume++;
		  printf("SDLK_RIGHT volume: %i\n",sid.filter->volume);
		  break;
		case SDLK_UP:
		  printf("up active\n");
		  break;
		case SDLK_DOWN:
		  printf("down active\n");
		  break;
		default:
		  handleKey(event.key);
		  break;
		}
		break;
	  case SDL_KEYUP:
		switch(event.key.keysym.sym){
		case SDLK_LEFT:

		  break;
		case SDLK_RIGHT:

		  break;
		case SDLK_UP:
		  interface_voice->frequency = interface_voice->frequency + interface_voice->frequency*1/100;
		  printf("up in-active\n");
		  break;
		case SDLK_DOWN:
		  interface_voice->frequency = interface_voice->frequency - interface_voice->frequency*1/100;
		  printf("down in-active\n");
		  break;
		default:
		  handleKey(event.key);
		  break;
		}
		break;
	  case SDL_QUIT:
		done = 1;
		printf("SDL_QUIT\n");
		break;
	  case SDL_MOUSEBUTTONUP:
		switch(event.button.button){
		case SDL_BUTTON_WHEELUP:
		  interface_voice->frequency = interface_voice->frequency + interface_voice->frequency*1/100;
		  break;
		case SDL_BUTTON_WHEELDOWN:
		  interface_voice->frequency = interface_voice->frequency - interface_voice->frequency*1/100;
		  break;
		}
		break;
	  default:
		/* do nothing */
		break;
	  }
	}
	  

	//	  SDL_Flip(screen);
	SDL_Delay (50);
  }

	
  Mix_FreeChunk(sound);
	
  Mix_CloseAudio();
  SDL_Quit();
  return 0; 
	

	
} 



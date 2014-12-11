#include "SDL.h" 
#include "SDL_audio.h" 
#include "SDL_mixer.h"
//#include "SDL_gfxPrimitives.h"
#include <stdlib.h> 

#define _USE_MATH_DEFINES
#include <math.h> 

#include "sdl-waveform-generator.h"

#define WAVEFORM_LENGTH 1024

double smp_index06;
float smp_index05;
float smp_index04;
float smp_index03;
int tonstufe= 0;


float*savebuffer;
struct adsr_struct0 {
  float a_attack;
  float a_decay;
  float	b_decay;
  float a_release;
  float b_release;
  int gate;
  int adsr_counter;
  int sustain_start;
} instrument[10];


/*
  adsr
attack delay sustain release
*/
struct adsr {
  float attack;
  float decay;
  float sustainlevel;
  float sustainmax  ;
  float release ;
  char *name;
};

struct adsr adsrenvelope[10] = {
  // piano
  { //attack 
    52.0 / 1000.0,
    // decay
    550.0 / 1000,
    // sustainlevel
    0.0,
    // sustainmax
    200.0/ 1000.0,
    // release
    172.0/ 1000,
	"piano"
  },
  // trumpet
  { //attack 
    15.0 / 1000.0,
    // decay
    50.0 / 1000,
    // sustainlevel
    0.8,
    // sustainmax
    1200.0/ 1000.0,
    // release
    0.0/ 1000,
	"trumpet"
  },
  { //attack 
    10.0 / 1000.0,
    // decay
    35.0 / 1000,
    // sustainlevel
    0.5,
    // sustainmax
    500.0/ 1000.0,
    // release
    372.0/ 1000,
	"NACK"
  },
  // violine
  { //attack 
    50.0 / 1000.0,
    // decay
    30.0 / 1000,
    // sustainlevel
    0.1,
    // sustainmax
    500.0/ 1000.0,
    // release
    372.0/ 1000,
	"violine"
  }
};


SNDFMT* smpls;

SDL_Surface *screen;
int xres = 640;
int yres = 480; 

SDL_AudioSpec spec; 

/* int sound_len; //= 512;  */
/* int spec.freq; = 44100;  */
/* spec_format = AUDIO_U8;//S16SYS;  */
/* spec_channels = 1;  */
/* spec_silence = 127;  */
/* spec_samples = 512;  */
/* spec_padding = 0;  */
/* spec_size = 0;      */


Uint32 sound_len; 
SNDFMT *sound_buffer; 
//Uint32 sound_pos = 0; 
int counter; 


int Hz;
double amplitude=100.0;

int volume=MIX_MAX_VOLUME;

/*
//Do all the init stuff 
void init_sdl (void){
  
if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) 
exit (-1); 

atexit (SDL_Quit); 
screen = SDL_SetVideoMode (640, 480, 16, SDL_HWSURFACE); 

if (screen == NULL) 
exit (-1); 
  
//sound_len= 512; 
spec.freq = 44100; 
spec.format = AUDIO_U8;//S16SYS; 
spec.channels = 1; 
spec.silence = 127; 
spec.samples = 512; 
spec.padding = 0; 
spec.size = 0;     
  
double userdata = 300.0; 
spec.userdata = &userdata;
  
spec.callback = Callback; 
if (SDL_OpenAudio (&spec, NULL) < 0) 
{ 
printf ("Kann audio nicht öffnen: %s\n", SDL_GetError ()); 
exit (-1); 
} 
} 

*/
void play (void) { 
  SDL_PauseAudio (0); 
} 

SNDFMT * snd_load(char* path){
  SNDFMT* buf;
  int i=0;
  buf = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);

  FILE *finp;
  finp=fopen(path, "r");

  char c;
  int x,data;
  printf("start scanf\n");
  for(i=0;i<WAVEFORM_LENGTH; i++){
    fscanf(finp, "%d %i %c\n", &x, &data, &c);
    //		  scanf(fp2, "%d %i %c\n", &x, &y, &c);
    buf[i] = (SNDFMT) data;
    printf("data : %i  buf[%i] is : %i\n", data, i, buf[i]);
  }
  fclose(finp);
  return buf;
}  
FILE *foutp;
void save(char* path,SNDFMT*buffer,int len){
  FILE *foutp;
  foutp=fopen(path, "w");
  int i;
  for(i=0;i<len;i++){
	// fprintf(foutp, " %i %f i\n",i, envelope(i));
	fprintf(foutp, " %i %i i\n",i, buffer[i]);
  }
  fclose(foutp);
}
void saveadsr(char* path,float*buffer,int len){
  FILE *foutp;
  foutp=fopen(path, "w");
  int i;
  for(i=0;i<len;i++){
	// fprintf(foutp, " %i %f i\n",i, envelope(i));
	fprintf(foutp, " %i %f i\n",i, buffer[i]);
  }
  fclose(foutp);
}
void savestream(char* path,int*buffer,int len){
  FILE *foutp;
  foutp=fopen(path, "w");
  int i;
  for(i=0;i<len;i++){
	// fprintf(foutp, " %i %f i\n",i, envelope(i));
	fprintf(foutp, " %i %i i\n",i, buffer[i]);
  }
  fclose(foutp);
}

SNDFMT* gen_sin_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH;i++){
    data = (1.0 + sin( (2.0*M_PI*i)/WAVEFORM_LENGTH))*127 ;
    //data = sin( (2.0*M_PI*i)/WAVEFORM_LENGTH ) * 127 ;
    buffer[i] = (SNDFMT) round(data);
    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  return buffer;
}

SNDFMT* gen_sawtooth_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH;i++){
    data = 255* ((float)i/(float)WAVEFORM_LENGTH);
    buffer[i] = (SNDFMT) floor(data);
    printf("gen_sawtooth_buffer : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  return buffer;
}

SNDFMT* gen_triangle_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH/2;i++){
    data = 255* ((float)i/(.5*(float)WAVEFORM_LENGTH));
    buffer[i] = (SNDFMT) floor(data);
    printf("data : %f floor(data) : %f  buffer[%i] is : %i\n", data, floor(data), i, buffer[i]);
  }
  for(;i<WAVEFORM_LENGTH;i++){
    data = 255* ((float)-(2/(float)WAVEFORM_LENGTH)*i+2);
    buffer[i] = (SNDFMT) floor(data);
    printf("data : %f floor(data) : %f  buffer[%i] is : %i\n", data, floor(data), i, buffer[i]);
  }
  return buffer;
}

SNDFMT* gen_square_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH/2;i++){
    data = 255.0 * 0.0;
    buffer[i] = (SNDFMT) floor(data);
    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  for(;i<WAVEFORM_LENGTH;i++){
    data = 255.0 * 1.0 ;
    buffer[i] = (SNDFMT) floor(data);
    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  return buffer;
}

SNDFMT* gen_pwm_buffer(float duty_cycle){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH * duty_cycle;i++){
    data = 255.0 * 0.0;
    buffer[i] = (SNDFMT) floor(data);
    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  for(;i<WAVEFORM_LENGTH;i++){
    data = 255.0 * 1.0 ;
    buffer[i] = (SNDFMT) floor(data);
    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  return buffer;
}



double smp_index = 0;
double freq = 9.5;

/*

  void Callback(void *data, Uint8 *buf, int len){
  int i;
  Uint8 *out;
  out = (Uint8*) buf;
  if(smp_index < WAVEFORM_LENGTH){
  for(i = 0; i < len; i++){
  out[i] = smpls[(int)smp_index];
  smp_index +=freq;
  if(smp_index >= WAVEFORM_LENGTH)
  smp_index = 0;
  }
  }
  }

*/

int waveform=0;

void handleKey(SDL_KeyboardEvent key) {
  switch(key.keysym.sym) {
  case SDLK_s: // sinus
    smpls = gen_sin_buffer();
    break;
  case SDLK_q: // square
    smpls = gen_square_buffer();
    break;
  case SDLK_t: // triangle
    smpls = gen_triangle_buffer();
    break;
  case SDLK_w: // sawtooth wave
    smpls = gen_sawtooth_buffer();
    break;
  case SDLK_x: // sinsquare wave
    waveform = 3;
    break;
  case SDLK_p: // puls with modulation
    smpls = gen_pwm_buffer(0.2);
    break;
  case SDLK_l: // load sinus
    smpls = snd_load("/media/c5bcf67f-e9eb-4e12-be75-d8b6e09e27ba/olivier/hti/ginf/cpu-emu/cpu-emu/sin.table");
    break;
  case SDLK_1: // note 1 anspielen
  case SDLK_2: // note 2 anspielen
  case SDLK_3: // note 3 anspielen
  case SDLK_4: // note 4 anspielen
    if(key.state == SDL_PRESSED) {
      instrument[0].gate = 1; // start adsr attack phase
      instrument[0].adsr_counter = 0; // start adsr attack phase
      printf("KEy 1  pressed %c\n",key.keysym.sym);
    } 
    if(key.state == SDL_RELEASED){
      instrument[0].gate = 0; // enter release phase
      instrument[0].sustain_start = instrument[0].adsr_counter;
      printf("KEy  1 released\n");
    }
    break;

    /*
	  case SDLK_c: // copy
	  if( copyintomem ==0){
      copyintomem = 1;
	  } 
	  else
      {
	  copyintomem =0 ;
      }
	  break;
	  case SDLK_a: // adsr
	  if(key.type == SDL_PRESSED) {
      adsr = 1;
      printf("KEy a pressed\n");
	  } //printf("KEy 1 pressed %i %c %i\n",key.type,key.keysym.sym,SDL_PRESSED);
	  if(key.type == SDL_RELEASED){
      adsr = 0;
      printf("KEy a released\n");
	  }
	  break;
	*/

  default:
    /* do nothing */
    break;
  }
}


int gen_adsr_buffer(float**ptr, struct adsr adsr_instrument){
  // violine
  int attack = adsr_instrument.attack; // 500;
  int decay = adsr_instrument.decay; // 300;
  int sustain = adsr_instrument.sustainlevel; // 10;
  int sustainlength0 = (attack / 1000.0) * SMPFREQ;
  int sustain_max = adsr_instrument.sustainmax;  // 15; // 4 Bit 0-15
  int release = adsr_instrument.release; // 750; 

  /* attack = 500;
	 decay = 6;
	 sustain = 15;
	 sustainlength0 = (attack / 1000.0) * SMPFREQ;

	 release = 72; 

	 // piano 
	 attack = 2;
	 decay = 750;
	 sustain = 0;
	 sustainlength0 = 0;
	 release = 72; 
  */
  float max_vol = 1.0;

  int attack0 = (attack / 1000.0) * SMPFREQ;
  int decay0  = (decay  / 1000.0) * SMPFREQ;

  float sustain0 = (float) sustain * ((float)max_vol)/(float)sustain_max;


  int release0 = (release / 1000.0) * SMPFREQ;

  int adsr_length0 = attack0+decay0+sustainlength0+release0;
  printf("int adsr_length0: %i = attack0: %i decay0: %i sustainlength0: %i release0: %i;\n",adsr_length0 ,attack0,decay0,sustainlength0,release0);

  int t1 = attack0;
  int t2 = t1 + decay0;
  int t3 = t2 + sustainlength0;
  int t4 = t3 + release0;

  float* buffer = malloc(sizeof(float)*adsr_length0);


  int i;
  float data;
  float a,b;
  float   sustain_level;
  for(i=0;i<adsr_length0;i++){
    if(i<t1){// attack
      data =  (float)i* ((float)1.0/t1);
    }
    else { // decay
      if(i<t2){
		a = ((float) ( 1.0 - sustain0)) / (float) ( t1-t2) ;
		b = (float)1.0 - (a*t1);
	
		data =   (float)i * a + b;
		sustain_level = i*a+b;
      } else { // sustain
		if(i<t3){ 
		  data = (float) sustain_level;
		}else { // release
		  if(i<t4){
			a = sustain_level / (float) (t3-t4) ;
			b = sustain_level - a* t3;
			data =  (float) (float)i*a+b; //sustain_level;
	    
		  }
		}
      }
    }
    buffer[i] = data;
  }
  *ptr = buffer;
  return adsr_length0;
}

int gen_adsr_buffer0(float**ptr){
  // violine
  int attack = 500;
  int decay = 300;
  int sustain = 10;
  int sustainlength0 = (attack / 1000.0) * SMPFREQ;
  int sustain_max = 15; // 4 Bit 0-15
  int release = 750; 

  /* attack = 500;
	 decay = 6;
	 sustain = 15;
	 sustainlength0 = (attack / 1000.0) * SMPFREQ;

	 release = 72; 

	 // piano 
	 attack = 2;
	 decay = 750;
	 sustain = 0;
	 sustainlength0 = 0;
	 release = 72; 
  */
  float max_vol = 1.0;

  int attack0 = (attack / 1000.0) * SMPFREQ;
  int decay0  = (decay  / 1000.0) * SMPFREQ;

  float sustain0 = (float) sustain * ((float)max_vol)/(float)sustain_max;


  int release0 = (release / 1000.0) * SMPFREQ;

  int adsr_length0 = attack0+decay0+sustainlength0+release0;
  printf("int adsr_length0: %i = attack0: %i decay0: %i sustainlength0: %i release0: %i;\n",adsr_length0 ,attack0,decay0,sustainlength0,release0);

  int t1 = attack0;
  int t2 = t1 + decay0;
  int t3 = t2 + sustainlength0;
  int t4 = t3 + release0;

  float* buffer = malloc(sizeof(float)*adsr_length0);


  int i;
  float data;
  float a,b;
  float   sustain_level;
  for(i=0;i<adsr_length0;i++){
    if(i<t1){// attack
      data =  (float)i* ((float)1.0/t1);
    }
    else { // decay
      if(i<t2){
		a = ((float) ( 1.0 - sustain0)) / (float) ( t1-t2) ;
		b = (float)1.0 - (a*t1);
	
		data =   (float)i * a + b;
		sustain_level = i*a+b;
      } else { // sustain
		if(i<t3){ 
		  data = (float) sustain_level;
		}else { // release
		  if(i<t4){
			a = sustain_level / (float) (t3-t4) ;
			b = sustain_level - a* t3;
			data =  (float) (float)i*a+b; //sustain_level;
	    
		  }
		}
      }
    }
    buffer[i] = data;
  }
  *ptr = buffer;
  return adsr_length0;
}

//int adsr_counter=0;
int adsr_length=0;
float *adsr_buffer=NULL;
/*
  void FREQEffect(int chan, void *stream, int len, void *udata){
  int i;
  Uint8 *out;
  float data,a,b;
  
  int sustain = 10;
  int sustain_level=0;
  int release = 750; 
  int release0 = ((float)release /1000.) * SMPFREQ; 
  out = (Uint8*) stream;
  if(smp_index < WAVEFORM_LENGTH){
  for(i = 0; i < len; i++){

  //            out[i] = smpls[(int)smp_index];	  
      
  if(adsr_counter< adsr_length){
  data= ( smpls[(int)smp_index]  ) * adsr_buffer[adsr_counter];//(float)adsr_counter* (MIX_MAX_VOLUME/22050.0);
  adsr_counter++;
  if(adsr_counter>adsr_length) 
  adsr_counter = 0;
  }

  out[i]= round(data);
  smp_index += freq;
  if(smp_index >= WAVEFORM_LENGTH)
  smp_index = 0;
  }
  }
  }

*/

float ton2frequency(int note){
  float sqrt12 = 1.059463094;
  //  return (float) 466.0; 
  return 440.0* pow(sqrt12,  note); 
}

void adsrEffect(int chan, void *stream, int len, void *udata){
  int i;
  Uint8 *out = (Uint8*) stream;
  float data;
  struct adsr_struct0* adsr = (struct adsr_struct0*) udata;

  if(smp_index < WAVEFORM_LENGTH){
    for(i = 0; i < len; i++){
      if(adsr->adsr_counter < adsr_length){
		if(!adsr->gate) { // gate closed
		  data= ( (float) smpls[(int)smp_index]  ) ;
		}else{ // gate open
		  data= ((( 127.0 - (float) smpls[(int)smp_index]  ) * (float)adsr_buffer[adsr->adsr_counter]) ) + 127.0;
		  //data= ( (float) smpls[(int)smp_index]  ) * adsr_buffer[adsr->adsr_counter];
		  savebuffer[adsr->adsr_counter] =  data;
		}
	
		(adsr->adsr_counter)++;
		if(adsr->adsr_counter >= adsr_length) 
		  adsr->adsr_counter = 0;
      }else 
		{ printf("SHit happens"); exit(-1);}
      out[i]= round(data);
      smp_index06 = (float)frequency(tonstufe)/((float)SMPFREQ/(float)WAVEFORM_LENGTH); // + freq;

      smp_index05 = ton2frequency(tonstufe); // + freq;
      //      smp_index05 = 466.0;
      /*
		if(smp_index05-466.0 !=0 ){
		smp_index05 =  ton2frequency(tonstufe); // + freq;
		printf("freq: %f indx %f \n", ton2frequency(tonstufe), smp_index05);
		exit(1);
		}
      */
      smp_index04 = ((float)SMPFREQ/(float)WAVEFORM_LENGTH); // + freq;
      smp_index03 =  smp_index05/((float)SMPFREQ/(float)WAVEFORM_LENGTH); // + freq;
      //      smp_index += (int) round(138.0/((float)SMPFREQ/(float)WAVEFORM_LENGTH)); // + freq;
      smp_index += (int) round(ton2frequency(tonstufe)/((float)SMPFREQ/(float)WAVEFORM_LENGTH)); // + freq;
      if(smp_index < 0)
		exit(4);
      if(smp_index >= WAVEFORM_LENGTH)
		smp_index = 0;
    }
  }
}



void mix_adsr(Mix_Chunk *sample,int counter){

  // violine
  int attack = 500;
  int decay = 300;
  int sustain = 10;
  int release = 750; 

  if(Mix_PlayChannelTimed(-1, sample, -1 , attack)==-1) {
    printf("Mix_PlayChannel: %s\n",Mix_GetError());
  }
  Mix_VolumeChunk(sample, volume);
  if(Mix_PlayChannelTimed(-1, sample, -1 , decay)==-1) {
    printf("Mix_PlayChannel: %s\n",Mix_GetError());
  }
  if(Mix_PlayChannelTimed(-1, sample, -1 , attack)==-1) {
    printf("Mix_PlayChannel: %s\n",Mix_GetError());
  } 

}

int main(int argc, char* argv[]){
 foutp=fopen("data.out", "w");
  close(foutp);
  /* // start SDL with audio support */
  
  //  smpls = snd_load("/media/c5bcf67f-e9eb-4e12-be75-d8b6e09e27ba/olivier/hti/ginf/cpu-emu/cpu-emu/sin.table");

  smpls = gen_triangle_buffer();
  smpls = gen_sin_buffer();
  if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
    exit (-1);

  atexit (SDL_Quit);
  screen = SDL_SetVideoMode (640, 480, 16, SDL_HWSURFACE);

  if (screen == NULL)
    exit (-1);

  // open SMPFREQ, signed 8bit, system byte order,
  //      stereo audio, using 1024 byte chunks
  if(Mix_OpenAudio(SMPFREQ, AUDIO_S8, 1, WAVEFORM_LENGTH)==-1) {
    printf("Mix_OpenAudio: %s\n", Mix_GetError());
    exit(2);
  }

  Mix_Chunk *sound = NULL; 
  
  sound = Mix_QuickLoad_RAW(smpls,WAVEFORM_LENGTH);

  if(sound == NULL) {
    fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
  }

  int channel;
  
  channel = Mix_PlayChannel(-1, sound, -1);
  if(channel == -1) {
    fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
  }

  //while(Mix_Playing(channel) != 0);

  int len = WAVEFORM_LENGTH;
  smpls = gen_sin_buffer();
  
  
  int i=0;
  SDL_Event event;
  int done=0;
  int hzinc=0,amplitudeinc;
	


  
  while(!done) {
	while(SDL_PollEvent(&event)) {
	    
	  switch(event.type) {
	  case SDL_KEYDOWN:	
		switch(event.key.keysym.sym){
		case SDLK_LEFT:
		  volume-=10;
		  Mix_VolumeChunk(sound, volume);
		  //		Mix_VolumeChunk(sound, (volume>MIX_MAX_VOLUME-10) ?  (volume = MIX_MAX_VOLUME):(volume-=10) );
		  printf("SDLK_LEFT volume: %i\n",volume);
		  amplitudeinc = -1;
		  break;
		case SDLK_RIGHT:
		  volume+=10;
		  Mix_VolumeChunk(sound, volume );
		  //		Mix_VolumeChunk(sound, (volume<10) ?  (volume = 0): (volume+=10) );
		  printf("SDLK_RIGHT volume: %i\n",volume);
		  amplitudeinc = +1;;
		  break;
		case SDLK_UP:
		  hzinc = 1;
		  printf("up active\n");
		  break;
		case SDLK_DOWN:
		  hzinc = -1;
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
		  amplitudeinc = 0;
		  break;
		case SDLK_RIGHT:
		  amplitudeinc = 0;
		  break;
		case SDLK_UP:
		  tonstufe++;
		  hzinc = 0;
		  printf("up in-active\n");
		  break;
		case SDLK_DOWN:
		  tonstufe--;
		  hzinc = 0;
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
		  Hz += 1;
		  break;
		case SDL_BUTTON_WHEELDOWN:
		  Hz -= 1;
		  break;
		}
		break;
	  default:
		/* do nothing */
		break;
	  }
	}
	  
	freq += hzinc;

	//Hz= 2*Hz;
	Hz = Hz + hzinc;
	amplitude = amplitude + amplitudeinc;
	//	  printf("Hz: %f\n",Hz);
	//% Hz = frequency(music[i++%10]);
	int y;
	//	  SDL_Flip(screen);
	SDL_Delay (50);
  }

	
  Mix_FreeChunk(sound);
	
  Mix_CloseAudio();
  SDL_Quit();
  return 0; 
	

	
} 



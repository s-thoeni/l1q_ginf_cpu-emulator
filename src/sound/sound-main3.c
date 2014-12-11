#include <time.h>
#include "SDL.h" 
#include "SDL_audio.h" 
#include "SDL_mixer.h"
//#include "SDL_gfxPrimitives.h"
#include <stdlib.h> 

#define _USE_MATH_DEFINES
#include <math.h> 

#include "sid.h"

//#include "sdl-waveform-generator.h"

//#define WAVEFORM_LENGTH 1024
#define WAVEFORM_LENGTH 4096

#define SMPFREQ 8000.0

struct sid_6581_filter sid_filter;

struct envelope voice_a_env,voice_b_env,voice_c_env;
struct oscillator voice_a_osc,voice_b_osc,voice_c_osc;

struct sid_6581_voice va = { 7382, 440.0, 20 ,  0 | SID_SAWT , 240, 114, 12, 24, &voice_a_osc, & voice_a_env ,0.0f, NULL, NULL};
struct sid_6581_voice vb = { 17382, 440.0, 40 ,  0 | SID_SAWT , 660, 114, 8, 24, &voice_b_osc, & voice_b_env ,0.0f, NULL, NULL};
struct sid_6581_voice vc = { 7382, 440.0, 20 ,  0 | SID_SAWT , 240, 114, 12, 24, &voice_c_osc, & voice_c_env ,0.0f, NULL, NULL};

struct sid_6581 sid = { &va, &vb, &vc, &sid_filter };


// float*savebuffer;


/*
  adsr
  attack delay sustain release
*/
struct play_adsr {
  float attack;
  float decay;
  float sustainlevel;
  float sustainmax  ;
  float release ;
  char *name;
};

struct play_adsr adsrenvelope[10] = {
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


struct sid_6581_voice *interface_voice;
Mix_Chunk *sound = NULL; 

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

float ton2frequency(int note){
  float sqrt12 = 1.059463094;
  //  return (float) 466.0; 
  return 440.0* pow(sqrt12,  note); 
}
int phi2  =  1000000;
float sid2frequency(struct sid_6581_voice voice){
  float freq = voice.frequency;
  //  return (float) 466.0; 
  //  return 440.0* pow(sqrt12,  note);
  return freq * phi2 / 16777216 ; 
}


SNDFMT* gen_sin_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH;i++){
    data = (1.0 + sin( (2.0*M_PI*i)/WAVEFORM_LENGTH))*127 ;
    //data = sin( (2.0*M_PI*i)/WAVEFORM_LENGTH ) * 127 ;
    buffer[i] = (SNDFMT) round(data);
	//    printf("sin : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  return buffer;
}

SNDFMT sawtooth_fct0(int counter){
  SNDFMT data;
  data = (SNDFMT) floor(  255 * ((float)counter/(float)WAVEFORM_LENGTH) );
  //  printf("sawtooth: %i data:[%i] is : %i\n", counter, counter, data);
  return data;
}

double sawtooth_fct(struct sid_6581_voice* voice){
  double y;
  y = 255 - ( 255 / M_PI * voice->osc->sawt_phase);
  //  voice->osc->sawt_phase = voice->osc->sawt_phase + ((2*M_PI*voice->dfrequency) / SMPFREQ);
  voice->osc->sawt_phase = voice->osc->sawt_phase + ((2*M_PI*sid2frequency(*voice)) / SMPFREQ);
  if(voice->osc->sawt_phase > 2*M_PI){
	voice->osc->sawt_phase = voice->osc->sawt_phase - (2*M_PI);
  }
  return y;
}

SNDFMT sawtooth_fct1(int counter, double frequence){
  int index;
  index = (int) floor(counter * frequence) % WAVEFORM_LENGTH;
  float v;
  v =  index / WAVEFORM_LENGTH;
  v =  ( (float) ((int) floor(counter * frequence) % WAVEFORM_LENGTH )) / (float) WAVEFORM_LENGTH;
  SNDFMT data;
  //  data = (SNDFMT) floor(  255 * ((float)counter/(float)WAVEFORM_LENGTH) );
  data = (SNDFMT) floor( 255 * v );
  //  printf("sawtooth_fct: %i value: %f data: %i\n", counter, v, data);
  return data;
}

SNDFMT* gen_sawtooth_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  float data;
  for(i=0;i<WAVEFORM_LENGTH;i++){
    data = 255* ((float)i/(float)WAVEFORM_LENGTH);
    buffer[i] = (SNDFMT) floor(data);
    printf("sawtooth : %f buffer[%i] is : %i\n", data, i, buffer[i]);
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
	printf("triangle : buffer[%i] is : %i\n", i, buffer[i]);
  }
  for(;i<WAVEFORM_LENGTH;i++){
    data = 255* ((float)-(2/(float)WAVEFORM_LENGTH)*i+2);
    buffer[i] = (SNDFMT) floor(data);
	printf("triangle : buffer[%i] is : %i\n", i, buffer[i]);
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
	//    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  for(;i<WAVEFORM_LENGTH;i++){
    data = 255.0 * 1.0 ;
    buffer[i] = (SNDFMT) floor(data);
	//    printf("data : %f buffer[%i] is : %i\n", data, i, buffer[i]);
  }
  return buffer;
}

SNDFMT* gen_noise_buffer(){
  SNDFMT* buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  int i;
  for(i=0; i<WAVEFORM_LENGTH ;i++){
    buffer[i] = (SNDFMT) (rand() % 256);
	//    printf("noise: buffer[%i] is : %i\n", i, buffer[i]);
  }
  return buffer;
}

void print_stars(int n) {
  int i;
  for(i=0;i<=n; i++){
	printf("*");
  }
}

SNDFMT* gen_pwm_buffer(int pw,SNDFMT* buffer ){
  float duty_cycle = (((float)pw) /40.95f)/100.0f ;
  int i;
  
  if(!buffer){
	buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  } 
  //float data;
  for(i=0;i<WAVEFORM_LENGTH * duty_cycle;i++){
	//    data = 255.0 * 0.0;
    buffer[i] = (SNDFMT) 0;
	//    printf("pwm : %f buffer[%i] is : %i\n", data, i, buffer[i]);
	//	printf("\n");
  }
  for(;i<WAVEFORM_LENGTH;i++){
	//    data = 255.0 * 1.0 ;
    buffer[i] = (SNDFMT) 255;
	//    printf("pwm: %f buffer[%i] is : %i\n", data, i, buffer[i]);
	//	printf("\n");
  }
  return buffer;
}


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


//static int gen_release_cycle_data_counter=0;
float gen_release_cycle_data(struct sid_6581_voice *voice){
  // release linear
  float mr,br;
  float sustain = voice->sustain; //0.0;
  float release = voice->release; //72.0/ 1000;
  float envelope;
  mr = (sustain/16) / -release ; // (xg - xr);
  br = sustain/16 + mr * 0;
  //  br = - (sustain * xr) / (xg - xr);
  
  float t = ((float)voice->env->gen_release_cycle_data_counter/(float)SMPFREQ) * 1000 ;
  voice->env->gen_release_cycle_data_counter++;

  if( t<release ){
	envelope = mr * t + br; 
  }else{
	envelope = 0;
  }
  return envelope;
  // printf("release %i: mr = %f    br = %f env: %f\n", adsrcounter, mr, br, envelope);
}
/*

  /\__________
  /

*/

float gen_ads_cycle_data(struct sid_6581_voice *voice){
  static float envelope=0;
  int maxamp =1;
  float ma,ba,md,bd;

  float attack = voice->attack ;//2.0 / 1000;
  float decay = voice->decay; //750.0 / 1000;
  float sustain = voice->sustain; //0.0;

  voice->env->gen_ads_cycle_data_adsrcounter++;
  
  float t = ((float)(voice->env->gen_ads_cycle_data_adsrcounter)/(float)SMPFREQ) * 1000 ;

  if(t < attack) { 
	//attack
	ma = maxamp / attack;
	ba = 0;
	envelope = ma * t + ba;
	return envelope;
	//    printf("attack %i: ma = %f    ba = %f env: %f\n", adsrcounter, ma, ba, envelope);
  }else{ 
	if(t < attack+decay){
	  // decay
	  md = ((sustain/16.0) - 1.0) /decay;//-decay) ;
	  bd = maxamp - md * attack; 
	  //	  bd = ((sustain/16.0)*xa - xd) / (xa - xd) ; // bd = 1.0 - (( 1.0 - sustainlevel) / (xa-xd));
	  envelope = md * t + bd;
	  return envelope;
	  //	printf("decay %i: md = %f    bd = %f env: %f\n", adsrcounter, md, bd, envelope);
	  //	nullsein = - (adsrcounter-maxa)/(maxd - maxa);
	} else{
	  // sustain
	  return envelope;
	  //envelope = envelope; // sic, we just maintain
	  //	    printf("sustain %i: envelope %f\n", adsrcounter, envelope);
	}
  }
}


 


void adsrEffect_voice(struct sid_6581_voice* voice, int len){

  int i;
  float data;

  if(voice->smp_index < WAVEFORM_LENGTH) {
	for(i = 0; i < len; i++) {
	  if(!(voice->control & SID_GATE)) { // gate closed
		if(!voice->env->release_cycle_state){
		  voice->env->release_cycle_state = 1;
		  voice->env->ads_cycle_state = 0;
		  voice->env->gen_release_cycle_data_counter=0;
		  voice->env->release_cycle_state = 1;
		  printf("change state:  ads_cycle_state = 0;  		  release_cycle_state = 1;\n");
		}
		//		data = (float) smpls[(int)smp_index] ;
		//		data = ((( 127.0 - (float) smpls[(int)voice->smp_index]  ) * gen_release_cycle_data(voice)) ) + 127.0;
		//		data = ((( 127.0 - (float) voice->smpls[(int)voice->smp_index]  ) * gen_release_cycle_data(voice)) ) + 127.0;
		//		data = ((( 127.0 - (float) sawtooth_fct(voice->osc->sawt_counter++, sid2frequency(*voice) )) * gen_release_cycle_data(voice)) ) + 127.0;
		data = ((( 127.0 - (double) sawtooth_fct(voice)) * (double) gen_release_cycle_data(voice)) ) + 127.0;
		
	  } else { // gate open
		voice->env->release_cycle_state = 0;
		if(!voice->env->ads_cycle_state){
		  voice->env->ads_cycle_state = 1;
		  voice->env->gen_ads_cycle_data_adsrcounter=0;
		  printf("change state:  ads_cycle_state = 1;  		  release_cycle_state = 0;\n");
		}
		//		data = (float) smpls[(int)smp_index] ;
		//		data= ((( 127.0 - (float) smpls[(int)smp_index]  ) * gen_ads_cycle_data(*voice)) ) + 127.0;
		//		data= ((( 127.0 - (float) smpls[(int)voice->smp_index]  ) * gen_ads_cycle_data(voice)) ) + 127.0;
		//		data= ( ( 127.0 - (float) voice->smpls[(int)voice->smp_index]  ) * gen_ads_cycle_data(voice) ) + 127.0;
		//		data = ( ( 127.0 - (float) sawtooth_fct(voice->osc->sawt_counter++, sid2frequency(*voice)) ) * gen_ads_cycle_data(voice))  + 127.0;
		data = ( ( 127.0 - (double) sawtooth_fct(voice) ) * (double) gen_ads_cycle_data(voice))  + 127.0;
		
	  }

	  //	  Mix_VolumeChunk(sound, (volume>MIX_MAX_VOLUME-10) ?  (volume = MIX_MAX_VOLUME):(volume-=10) );

	  //	  out[i] = round(data);
	  // out[i] = round(data);
	  voice->buffer[i] = round(data);

      voice->smp_index += (int) round( sid2frequency(*voice)/((float)SMPFREQ/(float)WAVEFORM_LENGTH) ); // + freq;
	  //	  smp_index += (int) round(ton2frequency(tonstufe)/((float)SMPFREQ/(float)WAVEFORM_LENGTH)); // + freq;
      if(voice->smp_index < 0)
		exit(4);
      if(voice->smp_index >= WAVEFORM_LENGTH){
		voice->smp_index = ((int)floor(voice->smp_index)) % WAVEFORM_LENGTH; // = 0;
		//						  smp_index = smp_index % WAVEFORM_LENGTH; // = 0;
	  }
	}

  }
  //  SDL_MixAudio(stream, voice->buffer, len, sid.filter->volume * SDL_MIX_MAXVOLUME / (2*15));
  //    SDL_MixAudio(stream, voice->buffer, len, sid.filter->volume * SDL_MIX_MAXVOLUME / 2);
}



void sid_mixer(int chan, void *stream, int len, void *udata){

  int i;
  Uint8 *out = (Uint8*) stream;
  float data;

  struct sid_6581 *sid = (struct sid_6581*) udata;
  struct sid_6581_voice* voicea = sid->voicea;
  struct sid_6581_voice* voiceb = sid->voiceb;
  struct sid_6581_voice* voicec = sid->voicec;

  (voicea->control & SID_TRIA) ?   voicea->smpls = voicea->osc->triangle : 0 ;
  (voicea->control & SID_SAWT) ?   voicea->smpls = voicea->osc->sawtooth : 0 ;
  (voicea->control & SID_PULS) ?   voicea->smpls = gen_pwm_buffer(voicea->pw,voicea->smpls) : 0 ;
  (voicea->control & SID_NOIS) ?   voicea->smpls = voicea->osc->noise : 0 ;

  (voiceb->control & SID_TRIA) ?   voiceb->smpls = voiceb->osc->triangle : 0 ;
  (voiceb->control & SID_SAWT) ?   voiceb->smpls = voiceb->osc->sawtooth : 0 ;
  (voicea->control & SID_PULS) ?   voiceb->smpls = gen_pwm_buffer(voiceb->pw,voiceb->smpls) : 0 ;
  (voiceb->control & SID_NOIS) ?   voiceb->smpls = voiceb->osc->noise : 0 ;

  (voicec->control & SID_TRIA) ?   voicec->smpls = voicec->osc->triangle : 0 ;
  (voicec->control & SID_SAWT) ?   voicec->smpls = voicec->osc->sawtooth : 0 ;
  (voicea->control & SID_PULS) ?   voicec->smpls = gen_pwm_buffer(voicec->pw,voicec->smpls) : 0 ;
  (voicec->control & SID_NOIS) ?   voicec->smpls = voicec->osc->noise : 0 ;

  adsrEffect_voice(voicea,len);
  adsrEffect_voice(voiceb,len);
  adsrEffect_voice(voicec,len);

  int faca,facb,facc;
  int fac;
  double vol;
  for(i=0;i<len; i++){
	faca = voicea->buffer[i]-127;
	facb = voiceb->buffer[i]-127;
	facc = voicec->buffer[i]-127;
	
	vol = (double) sid->filter->volume / (double) (2*15);
	fac = 	faca + facb + facc;
	data = fac  * vol + 127 ;
	//	data = (faca  * vol) + 127 ;
	if(data>126){
	  data=126;
	}
	if(data<0){
	  data=0;
	}
	  out[i] =  data ;
  }
  //  SDL_MixAudio(stream, voice->buffer, len, sid.filter->volume * SDL_MIX_MAXVOLUME / (2*15));
  //    SDL_MixAudio(stream, voice->buffer, len, sid.filter->volume * SDL_MIX_MAXVOLUME / 2);
}



void init_SID(struct sid_6581* sid){

  srand (time(NULL));

  // voice a
  sid->voicea->osc->triangle = gen_triangle_buffer();
  sid->voicea->osc->sawtooth = gen_sawtooth_buffer();
  sid->voicea->osc->pulse = gen_pwm_buffer(0.2,NULL);
  sid->voicea->osc->noise = gen_noise_buffer();
  sid->voicea->osc->sawt_counter = 0;
  sid->voicea->osc->sawt_phase = 0.0;
	
  sid->voicea->env->gen_ads_cycle_data_adsrcounter = 0;
  sid->voicea->env->gen_release_cycle_data_counter = 0;
  sid->voicea->env->ads_cycle_state = 0;
  sid->voicea->env->release_cycle_state = 0;
  sid->voicea->buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  
  // voice b
  sid->voiceb->osc->triangle = gen_triangle_buffer();
  sid->voiceb->osc->sawtooth = gen_sawtooth_buffer();
  sid->voiceb->osc->pulse = gen_pwm_buffer(0.2,NULL);
  sid->voiceb->osc->noise = gen_noise_buffer();
  sid->voiceb->osc->sawt_counter = 0;
  
  sid->voiceb->env->gen_ads_cycle_data_adsrcounter = 0;
  sid->voiceb->env->gen_release_cycle_data_counter = 0;
  sid->voiceb->env->ads_cycle_state = 0;
  sid->voiceb->env->release_cycle_state = 0;
  sid->voiceb->buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);
  
  // voice c
  sid->voicec->osc->triangle = gen_triangle_buffer();
  sid->voicec->osc->sawtooth = gen_sawtooth_buffer();
  sid->voicec->osc->pulse = gen_pwm_buffer(0.2,NULL);
  sid->voicec->osc->noise = gen_noise_buffer();
  sid->voicec->osc->sawt_counter = 0;
  
  sid->voicec->env->gen_ads_cycle_data_adsrcounter = 0;
  sid->voicec->env->gen_release_cycle_data_counter = 0;
  sid->voicec->env->ads_cycle_state = 0;
  sid->voicec->env->release_cycle_state = 0;
  sid->voicec->buffer = malloc(sizeof(SNDFMT)*WAVEFORM_LENGTH);

  sid->filter->volume = 8;
}


int main(int argc, char* argv[]){

  SNDFMT* smpls = calloc(sizeof(SNDFMT),WAVEFORM_LENGTH);
//gen_triangle_buffer();//gen_sin_buffer();
  //  smpls = snd_load("/media/c5bcf67f-e9eb-4e12-be75-d8b6e09e27ba/olivier/hti/ginf/cpu-emu/cpu-emu/sin.table");
  

  /* // start SDL with audio support */
  
    if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
  //  if (SDL_Init (SDL_INIT_VIDEO) < 0)
    exit (-1);

  atexit (SDL_Quit);
  screen = SDL_SetVideoMode (640, 480, 16, SDL_HWSURFACE);

  if (screen == NULL)
    exit (-1);

  // open SMPFREQ, signed 8bit, system byte order,
  //      stereo audio, using 1024 byte chunks
  if(Mix_OpenAudio(SMPFREQ, AUDIO_S8, 1, WAVEFORM_LENGTH)==-1) {
    printf("Mix_OpenAudio: %s\n", Mix_GetError());
      exit (-1);
  }
  
  //  Mix_AllocateChannels(SID_VOICES_NR);




  sound = Mix_QuickLoad_RAW(smpls,WAVEFORM_LENGTH);


  if(sound == NULL) { 
     fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError()); 
   } 
  
  int channel= 1;
  int channel2;

  printf("\nStart playing sinus\n");
  SDL_Delay (1500);
  channel = Mix_PlayChannel(-1, sound, -1);
  //channel = Mix_PlayChannel(-1, NULL, -1);
  //    channel = Mix_PlayChannel(-1, sound, 1);
  printf("stop playing sinus\n");
  SDL_Delay (1500);
  //  channel2 = Mix_PlayChannel(-1, sound, -1);

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
	voicea->dfrequency = 440.0;
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


  SDL_Event event;
  int done=0;

  interface_voice = sid.voicea;

  while(!done) {
	while(SDL_PollEvent(&event)) {
	    
	  switch(event.type) {
	  case SDL_KEYDOWN:	
		switch(event.key.keysym.sym){
		case SDLK_LEFT:
		  (sid.filter->volume - 1) < 0 ? 0 : sid.filter->volume--;
		  volume-=10;
		  //		  Mix_VolumeChunk(sound, (sid.filter->volume/15)*MIX_MAX_VOLUME );
		   Mix_VolumeChunk(sound, (volume>MIX_MAX_VOLUME-10) ?  (volume = MIX_MAX_VOLUME):(volume-=10) );
		  printf("SDLK_LEFT volume: %i\n",volume);
		  break;
		case SDLK_RIGHT:
		  (sid.filter->volume + 1) >16 ? 15 : sid.filter->volume++;
		  volume+=10;
		  //Mix_VolumeChunk(sound, (sid.filter->volume/15)*MIX_MAX_VOLUME );
		  Mix_VolumeChunk(sound, (volume<10) ?  (volume = 0): (volume+=10) );
		  printf("SDLK_RIGHT volume: %i\n",volume);
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



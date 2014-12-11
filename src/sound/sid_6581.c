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



//  int sid_voice[7]; = {  0xff, 0x1a, 0x2b, 0x12, 0, 0, 0 };  
//int freq_low, int freq_high, int pw_low, int pw_high, int control, int attack_decay, int sustain_release;


#define FLO 0
#define FHI 1
#define PWL 2
#define PWH 3
#define CTR 4
#define AD 5
#define SR 6
#define VOL 24


int attack_rate_tbl[16] = { 2, 8, 16, 24, 38, 56, 68, 80, 100, 250, 500, 800, 1000, 3000, 5000, 8000 };
int decay_release_rate_tbl[16] =  { 6, 24, 48, 72, 114, 168, 204, 240, 300, 750, 1500, 2400, 3000, 9000, 15000, 24000 };

void write_voice(int reg, int value, struct sid_6581_voice* voice, struct cpu_emu_sid* emu_sid){
  if(reg==0){	// FHI, FLO
	voice->frequency = emu_sid->sid_voice[reg] +  (emu_sid->sid_voice[reg+1]<<8);
  } else	if(reg==1){
	voice->frequency = emu_sid->sid_voice[reg-1] +  (emu_sid->sid_voice[reg]<<8);
  } else 	if(reg==2){
	voice->pw = emu_sid->sid_voice[reg] +  (emu_sid->sid_voice[reg+1]<<8);
  } else	if(reg==3){
	voice->pw = emu_sid->sid_voice[reg-1] +  (emu_sid->sid_voice[reg]<<8);
  } else   if(reg==7){
	voice->frequency = emu_sid->sid_voice[reg] +  (emu_sid->sid_voice[reg+1]<<8);
  } else	if(reg==8){
	voice->frequency = emu_sid->sid_voice[reg-1] +  (emu_sid->sid_voice[reg]<<8);
  } else 	if(reg==9){
	voice->pw = emu_sid->sid_voice[reg] +  (emu_sid->sid_voice[reg+1]<<8);
  } else	if(reg==10){
	voice->pw = emu_sid->sid_voice[reg-1] +  (emu_sid->sid_voice[reg]<<8);
  } else   if(reg==14){
	voice->frequency = emu_sid->sid_voice[reg] +  (emu_sid->sid_voice[reg+1]<<8);
  } else	if(reg==15){
	voice->frequency = emu_sid->sid_voice[reg-1] +  (emu_sid->sid_voice[reg]<<8);
  } else 	if(reg==16){
	voice->pw = emu_sid->sid_voice[reg] +  (emu_sid->sid_voice[reg+1]<<8);
  } else	if(reg==17){
	voice->pw = emu_sid->sid_voice[reg-1] +  (emu_sid->sid_voice[reg]<<8);
  }

  switch(reg){
  case 4: case 11: case 18: // CTR:
	voice->control = value;
	break;
  case 5: case 12: case 19: // AD:
	// operator >>	is compiler spec, so take into account:
	voice->attack = attack_rate_tbl[((value & 0xF0)>>4) & 0x0F];
	voice->decay = decay_release_rate_tbl[value & 0x0F];
	break;
  case 6: case 13: case 20: // SR:
	// operator >>	is compiler spec, so take into account:
	voice->sustain = ((value & 0xF0)>>4) & 0x0F;
	voice->release = decay_release_rate_tbl[value & 0x0F];
	break;
  }
}
void write_sid(int reg, int value, struct sid_6581* sid, struct cpu_emu_sid* emu_sid){
  
  emu_sid->sid_voice[reg] = value;

  if(reg<7){
	write_voice(reg,value, sid->voicea, emu_sid);
  } else if(reg<14){
	write_voice(reg,value, sid->voiceb, emu_sid);
  } else if(reg<21){
	write_voice(reg,value, sid->voicec, emu_sid);
  }
  switch(reg){
  case VOL: 
	sid->filter->volume = value;
	break;
  }
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


double sawtooth_fct(struct sid_6581_voice* voice){
  double y;
  y = 127 - ( 127 / M_PI * voice->osc->phase);

  voice->osc->phase = voice->osc->phase + ((2*M_PI*sid2frequency(*voice)) / SMPFREQ);
  if(voice->osc->phase > 2*M_PI){
	voice->osc->phase = voice->osc->phase - (2*M_PI);
  }
  return y;
}

double triangle_fct(struct sid_6581_voice* voice){
  double y;
  if(voice->osc->phase<M_PI){
	y = -127 + (2*127/M_PI)*voice->osc->phase;
  } else {
	y = 3*127 - (2*127/M_PI)*voice->osc->phase;
  }

  voice->osc->phase = voice->osc->phase + ((2*M_PI*sid2frequency(*voice)) / SMPFREQ);
  if(voice->osc->phase > 2*M_PI){
	voice->osc->phase = voice->osc->phase - (2*M_PI);
  }
  return y;
}

double noise_fct(struct sid_6581_voice* voice){
  return (rand() % 256)-127;
}

double pulse_fct(struct sid_6581_voice* voice){
  double y;
  float duty_cycle = (((float)voice->pw) /40.95f)/100.0f ;

  if(voice->osc->phase < 2*M_PI*duty_cycle){
	y = 127 ;
  } else {
	y = -127 ;
  }

  voice->osc->phase = voice->osc->phase + ((2*M_PI*sid2frequency(*voice)) / SMPFREQ);
  if(voice->osc->phase > 2*M_PI){
	voice->osc->phase = voice->osc->phase - (2*M_PI);
  }
  return y;
}

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
  }else{ 
	if(t < attack+decay){
	  // decay
	  md = ((sustain/16.0) - 1.0) /decay;//-decay) ;
	  bd = maxamp - md * attack; 
	  envelope = md * t + bd;
	  return envelope;
	} else{
	  // sustain
	  return envelope;
	}
  }
}


 


double adsrEffect_voice(struct sid_6581_voice* voice){
  
  double data;
  
  if(!(voice->control & SID_GATE)) { // gate closed
	if(!voice->env->release_cycle_state){
	  voice->env->release_cycle_state = 1;
	  voice->env->ads_cycle_state = 0;
	  voice->env->gen_release_cycle_data_counter=0;
	  voice->env->release_cycle_state = 1;
	  printf("change state:  ads_cycle_state = 0;  		  release_cycle_state = 1;\n");
	}
	data = ((double) voice->osc_fct(voice) * (double) gen_release_cycle_data(voice) ) ;
	
  } else { // gate open
	voice->env->release_cycle_state = 0;
	if(!voice->env->ads_cycle_state){
	  voice->env->ads_cycle_state = 1;
	  voice->env->gen_ads_cycle_data_adsrcounter=0;
	  printf("change state:  ads_cycle_state = 1;  		  release_cycle_state = 0;\n");
	}
	data = ( (double) voice->osc_fct(voice)  * (double) gen_ads_cycle_data(voice)) ;
	
  }
  return data;
  
}

void sid_mixer(int chan, void *stream, int len, void *udata){

  int i;
  Uint8 *out = (Uint8*) stream;
  double data;

  struct sid_6581 *sid = (struct sid_6581*) udata;
  struct sid_6581_voice* voicea = sid->voicea;
  struct sid_6581_voice* voiceb = sid->voiceb;
  struct sid_6581_voice* voicec = sid->voicec;

  (voicea->control & SID_TRIA) ?   voicea->osc_fct = triangle_fct : 0 ;
  (voicea->control & SID_SAWT) ?   voicea->osc_fct = sawtooth_fct : 0 ;
  (voicea->control & SID_PULS) ?   voicea->osc_fct = pulse_fct : 0 ;
  (voicea->control & SID_NOIS) ?   voicea->osc_fct = noise_fct : 0 ;

  (voiceb->control & SID_TRIA) ?   voiceb->osc_fct = triangle_fct : 0 ;
  (voiceb->control & SID_SAWT) ?   voiceb->osc_fct = sawtooth_fct : 0 ;
  (voiceb->control & SID_PULS) ?   voiceb->osc_fct = pulse_fct : 0 ;
  (voiceb->control & SID_NOIS) ?   voiceb->osc_fct = noise_fct : 0 ;

  (voicec->control & SID_TRIA) ?   voicec->osc_fct = triangle_fct : 0 ;
  (voicec->control & SID_SAWT) ?   voicec->osc_fct = sawtooth_fct : 0 ;
  (voicec->control & SID_PULS) ?   voicec->osc_fct = pulse_fct : 0 ;
  (voicec->control & SID_NOIS) ?   voicec->osc_fct = noise_fct : 0 ;

  double faca,facb,facc;
  double fac;
  double vol;
  for(i=0;i<len; i++){
	faca = adsrEffect_voice(voicea);
	facb = adsrEffect_voice(voiceb);
	facc = adsrEffect_voice(voicec);

	vol = (double) sid->filter->volume / (double) (2*15);
	fac = 	faca + facb + facc;
	
	data = fac  * vol + 127.0 ;
	data=floor(data);
	//	data = (faca  * vol) + 127 ;
	
	if(data>255){
	  data=255;
	}
	if(data<0){
	  data=0;
	}
	
	out[i] =  data ;
  }
}



void init_SID(struct sid_6581* sid){

  srand (time(NULL));

  // voice a
  sid->voicea->osc_fct = sawtooth_fct;
  sid->voicea->osc->phase = 0.0;
  
  sid->voicea->env->gen_ads_cycle_data_adsrcounter = 0;
  sid->voicea->env->gen_release_cycle_data_counter = 0;
  sid->voicea->env->ads_cycle_state = 0;
  sid->voicea->env->release_cycle_state = 0;

  
  // voice b
  sid->voiceb->osc_fct = sawtooth_fct;
  sid->voiceb->osc->phase = 0.0;
  
  sid->voiceb->env->gen_ads_cycle_data_adsrcounter = 0;
  sid->voiceb->env->gen_release_cycle_data_counter = 0;
  sid->voiceb->env->ads_cycle_state = 0;
  sid->voiceb->env->release_cycle_state = 0;

  
  // voice c
  sid->voicec->osc_fct = sawtooth_fct;
  sid->voicec->osc->phase = 0.0;

  sid->voicec->env->gen_ads_cycle_data_adsrcounter = 0;
  sid->voicec->env->gen_release_cycle_data_counter = 0;
  sid->voicec->env->ads_cycle_state = 0;
  sid->voicec->env->release_cycle_state = 0;


  sid->filter->volume = 8;
}


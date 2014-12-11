/*
  generate sound with gnuplot:
  set table
  set output "sound5.gplot"
  plot [0:512] (100*sin(x*2*pi*10/(512))+127)  

 */



#include "SDL.h" 
#include "SDL_audio.h" 
//#include "SDL_gfxPrimitives.h"
#include <stdlib.h> 
#include <math.h> 

#include "sdl-waveform-generator.h"

double pi    = 3.1415926535897932384626433832795029L;


int music[10] = { 1, 3, 5, 6 ,8, 1, 6, 5, 3, 1}; 
float sqrt12 = 1.059463094;

int mem0size=500,mem1size = 50000;



float frequency(int note){
  //return (float) 466.0; 
    return 440.0* pow(sqrt12,  note); 
}
  



Uint32 sound_len; 
SNDFMT *sound_buffer; 
Uint32 sound_pos = 0; 
int counter; 

int adsr=1; // no attack-delay sustain release 
//int attack, delay, sustain, release;

//double amplitude=100.0;
//int waveform=0;
int copyintomem=1;
SNDFMT* mem[2];
//sample rate
double SR = 44100; 
int ylinesize = 88200; // = 2*SR;
//int yline[ylinesize];
int gate =1;
double Hz= 400.0;//*hzptr;	




/*
  adsr
attack delay sustain release
*/
struct asdr {
  float attack;
  float decay;
  float sustainlevel;
  float sustainmax  ;
  float release ;
  char *name;
};
// asdrenvelope instrument index
int instrument = 2;
struct asdr asdrenvelope[10] = {
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
    172.0/ 1000
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
    0.0/ 1000
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
    372.0/ 1000
  }
};


void copy_buffer(SNDFMT *sound_buffer){
  static int count=0;
  int i;
  if(count<2){
	for( i=0; i<512;i++){
	  mem[count][i] = (SNDFMT) sound_buffer[i];
	}
	count++;
  }
}

void write_buffer(SNDFMT *sound_buffer){
  static int count=0;
  int i;
  if(count<1){
	for( i=0; i<512;i++){
	  sound_buffer[i] = (SNDFMT)mem[0][i] ;
	}
	count++;
  }
}
float envelope2(int counter){
  return 0.0;
}
float envelope(int counter){
  float ma,ba,md,bd,mr,br;
  static float envelope=0;
  float attack = asdrenvelope[instrument].attack ;//2.0 / 1000;
  float decay = asdrenvelope[instrument].decay; //750.0 / 1000;
  float sustainlevel = asdrenvelope[instrument].sustainlevel; //0.0;
  float sustainmax = asdrenvelope[instrument].sustainmax; //200.0/ 1000 ;
  float release = asdrenvelope[instrument].release; //72.0/ 1000;

  float xa = attack;
  float xd = attack + decay;
  static float xg= 0.0;//attack + decay + sustainmax;
  float xr = xg + release;
  
  static int adsrcounter=0;
  float t = adsrcounter/SR ;

  /* if(!gate){ */
  /*   if(xg){ */
  /*     xg = adsrcounter; */
  /*   } */
  /* } */

  //  printf("Envelope: adsrcounter = %i   t = %f\n", adsrcounter, t);
  if(gate){
    xg = t;
    if(t < attack) { 
      //attack
      ma = 1 / xa;
      ba = 0;
      envelope = ma * t + ba;
      //    printf("attack %i: ma = %f    ba = %f env: %f\n", adsrcounter, ma, ba, envelope);
    }else{ 
      if(t < xd){
	// decay
	md = ((1.0 - sustainlevel) / (xa-xd));//-decay) ;
	bd = (sustainlevel*xa - xd) / (xa - xd) ; // bd = 1.0 - (( 1.0 - sustainlevel) / (xa-xd));
	envelope = md * t + bd;
	//	printf("decay %i: md = %f    bd = %f env: %f\n", adsrcounter, md, bd, envelope);
	//	nullsein = - (adsrcounter-maxa)/(maxd - maxa);
      } else{
	// sustain
	envelope = envelope; // sic, we just maintain
	//	    printf("sustain %i: envelope %f\n", adsrcounter, envelope);
      }
    }
  } else {
    // release linear
    mr = sustainlevel / -release ; // (xg - xr);
    br = - (sustainlevel * xr) / (xg - xr);
    envelope = mr * t + br; 
    // printf("release %i: mr = %f    br = %f env: %f\n", adsrcounter, mr, br, envelope);
  }

  adsrcounter++;
  if(t > attack+decay+sustainmax + release){
    adsrcounter = 0;
    xg=0.0;
  }
  if(envelope < 1) return envelope;
  return 55.0;
  //  return envelope;
}

float sinwave(double frequency, int counter){
  return sin(frequency*(double) counter);
}

float squarewave(double frequency, int counter){
  if(sinwave(frequency,counter)>=0){
    return 220.0 * envelope(counter);
  } else {
    return 0.0;
  }
}
  
float lin(float x){return x;}
  

float sawtoothwave(double F, int counter){
  double frequency = F*SR;
  float n = (frequency-1);
  //printf (" nenner = %f \t", n);
  float z = (counter % (int) floor(frequency));
  //printf (" zahler = %f \n", z);
  //return 0.3;
  return (float) (((counter % (int) floor(frequency)) / (frequency-1))  - 0.5); //(SNDFMT) (1 * (z % (int)Hz));
}

float sinsquarewave(double frequency, int counter){
  double w = counter; //2*pi;
  double x = frequency;
  return 4/pi*sin(w*x) \
    + 4/(3*pi)*sin(3*w*x)			\
    + 4/(5*pi)*sin(5*w*x)			\
    + 4/(7*pi)*sin(7*w*x)			\
    + 4/(9*pi)*sin(9*w*x)			\
    + 4/(11*pi)*sin(11*w*x)			\
    + 4/(13*pi)*sin(13*w*x)			\
    + 4/(15*pi)*sin(15*w*x)			\
    + 4/(17*pi)*sin(17*w*x)			\
    + 4/(19*pi)*sin(19*w*x)			\
    + 4/(21*pi)*sin(21*w*x)			\
    + 4/(23*pi)*sin(23*w*x)			\
    + 4/(25*pi)*sin(25*w*x)			\
    + 4/(27*pi)*sin(27*w*x)			\
    + 4/(29*pi)*sin(29*w*x);
}

float dutypcmwave(double frequency, int counter){
  double duty = 0.2; // 20% duty
  if(sinwave(frequency, counter)> 0.2){
    return 0.8;
  }
  else{
    return 0.2;
  }
}

float trianglewave(double frequency, int counter){
  double duty = 0.2; // 20% duty
  static dutycounter = 0;
  if(sinwave(frequency, counter)>=0.5){
    if(dutycounter < duty * SR * frequency){
      dutycounter ++; 
      return 1.0;
    }
    else{
      dutycounter = 0;
      return 0.0;
    }
  } else
    return 0.0;
}
    
float noisewave(double frequency, int counter){
  
}


struct fir {
  double b0;
  double b1;
  double b2;
  double b3;
  double b4;
  double b5;
};

struct fir firfilterstruct;

// finite impulse response (FIR) filter
double firfilter(int index, double buf[]){
  return     firfilterstruct.b0 * buf[index];
  /* 
    firfilterstruct.b0 *  buf[index] +	\
    firfilterstruct.b1 * buf[index+1] + \
    firfilterstruct.b2 * buf[index+2] + \
    firfilterstruct.b3 * buf[index+3] + \
    firfilterstruct.b4 * buf[index+4] + \
    firfilterstruct.b5 * buf[index+5] ;
    */
}


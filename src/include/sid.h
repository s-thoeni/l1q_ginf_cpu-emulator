#define SID_VOICES_NR 3

#define SNDFMT Uint8

#define SID_GATE   1
#define SID_SYNC   2 
#define SID_RING   4 
#define SID_TEST   8 
#define SID_TRIA  16 
#define SID_SAWT  32 
#define SID_PULS  64 
#define SID_NOIS 128


struct sid_6581 {
  struct sid_6581_voice *voicea;
  struct sid_6581_voice *voiceb;
  struct sid_6581_voice *voicec;
  struct sid_6581_filter *filter;
} ;

struct sid_6581_voice {
  int frequency; // SID
  int pw; // SID pulse width 0-0xFFF %
  int control; // SID
  int attack; // real ms
  int decay; //real ms
  int sustain; // SID: 0-15
  int release; // real ms
  struct oscillator *osc;
  struct envelope *env;
  double (*osc_fct)(struct sid_6581_voice*);
};

struct envelope{
  int gen_ads_cycle_data_adsrcounter;
  int gen_release_cycle_data_counter;
  //  release_cycle_state = 1 if  release_cycle else 0
  int release_cycle_state ;
  int ads_cycle_state ;
};

struct oscillator {
  double phase;
};

struct sid_6581_filter {
  int volume; // SID: 0-15 (silent - maxVolume)
  int filter_frequency; // SID filter 0 <= frequency <2^11
};

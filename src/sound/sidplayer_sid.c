/*
  sidplayer_sid.c
  shameless copy from cpp file
  translated to c
  init: bo@19.1.2014
*/

/*
 *  sid.cpp - 6581 SID emulation
 *
 *  Frodo (C) Copyright 1994-2004 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//#include "sys.h"

#include <SDL.h>

#include <SDL_types.h>
typedef Uint8 uint8;
typedef Sint8 int8;
typedef Uint16 uint16;
typedef Sint16 int16;
typedef Uint32 uint32;
typedef Sint32 int32;
typedef Uint64 uint64;
typedef Sint64 int64;

// Cycle count
typedef uint32 cycle_t;
const cycle_t CYCLE_NEVER = 0xffffffff;	// Infinitely into the future

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#include "sid.h"
//#include "main.h"
//#include "prefs.h"

//#define DEBUG 0
//#include "debug.h"

#include <math.h>

FILE *foutp;
char path[] = "/media/c5bcf67f-e9eb-4e12-be75-d8b6e09e27ba/olivier/hti/ginf/cpu-emu/cpu-emu/tmp/sid.dump";

typedef float filt_t;
const float F_ONE = 1.0;
const float F_ZERO = 0.0;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Desired and obtained audio formats
static SDL_AudioSpec desired;
static SDL_AudioSpec obtained;

// Flag: emulate SID filters
static int enable_filters = 1;

// Flag: emulate 2 SID chips
static int dual_sid = 0;

// Flag: emulate new SID chip (8580)
static int emulate_8580 = 0;

// Audio effect type (0 = none, 1 = reverb, 2 = spatial)
static int audio_effect = 0;

// Master volume (0..0x100)
static int32 master_volume = 0x55;

// Volumes (0..0x100) and panning (-0x100..0x100) of voices 1..4 (both SIDs)
static int32 v1_volume, v2_volume, v3_volume, v4_volume;
static int32 v1_panning, v2_panning, v3_panning, v4_panning;

// Dual-SID stereo separation (0..0x100)
static int32 dual_sep;

// Number of SID clocks per sample frame
static uint32 sid_cycles;		// Integer
static filt_t sid_cycles_frac;	// With fractional part

static cycle_t cycles_per_second;
// Clock frequency changed
void SIDClockFreqChanged();

#ifdef SID_PLAYER
// Phi2 clock frequency

const float PAL_CLOCK = 985248.444;
const float NTSC_OLD_CLOCK = 1000000.0;
const float NTSC_CLOCK = 1022727.143;

// Replay counter variables
static uint16 cia_timer;		// CIA timer A latch
static int replay_count;		// Counter for timing replay routine
static int speed_adjust;		// Speed adjustment in percent

#endif

// Catweasel device file handle
static int cwsid_fh = -1;

// Resonance frequency polynomials
static float CALC_RESONANCE_LP(float f)
{
  return 227.755 - 1.7635 * f - 0.0176385 * f * f + 0.00333484 * f * f * f;
}

static float CALC_RESONANCE_HP(float f)
{
  return 366.374 - 14.0052 * f + 0.603212 * f * f - 0.000880196 * f * f * f;
}

// Pseudo-random number generator for SID noise waveform (don't use f_rand()
// because the SID waveform calculation runs asynchronously and the output of
// f_rand() has to be predictable inside the main emulation)
static uint32 noise_rand_seed = 1;

static uint8 noise_rand()
{
  // This is not the original SID noise algorithm (which is unefficient to
  // implement in software) but this sounds close enough
  noise_rand_seed = noise_rand_seed * 1103515245 + 12345;
  return noise_rand_seed >> 16;
}

// SID waveforms

#define WAVE_NONE        0
#define WAVE_TRI  		 1
#define WAVE_SAW  		 2
#define WAVE_TRISAW  	 3
#define WAVE_RECT  		 4
#define WAVE_TRIRECT  	 5
#define WAVE_SAWRECT  	 6
#define WAVE_TRISAWRECT  7
#define WAVE_NOISE        8


// EG states
#define	EG_IDLE    0
#define EG_ATTACK  1
#define EG_DECAY   2
#define EG_RELEASE  3

				   
// Filter types	   
				   
#define	FILT_NONE  0
#define	FILT_LP    1
#define	FILT_BP    2
#define	FILT_LPBP  3
#define	FILT_HP    4
#define	FILT_NOTCH 5
#define	FILT_HPBP  6
#define	FILT_ALL   7

// Voice 4 states (SIDPlayer only)

#define V4_OFF          0
#define V4_GALWAY_NOISE 1 
#define V4_SAMPL  		2

// Structure for one voice
struct voice_t {
  int wave;			// Selected waveform
  int eg_state;		// Current state of EG
  struct voice_t *mod_by;	// Voice that modulates this one
  struct voice_t *mod_to;	// Voice that is modulated by this one

  uint32 count;		// Counter for waveform generator, 8.16 fixed
  uint32 add;			// Added to counter in every sample frame

  uint16 freq;		// SID frequency value
  uint16 pw;			// SID pulse-width value

  uint32 a_add;		// EG parameters
  uint32 d_sub;
  uint32 s_level;
  uint32 r_sub;
  uint32 eg_level;	// Current EG level, 8.16 fixed

  uint32 noise;		// Last noise generator output value

  uint16 left_gain;	// Gain on left channel (12.4 fixed)
  uint16 right_gain;	// Gain on right channel (12.4 fixed)

  unsigned int gate;			// EG gate bit
  unsigned int ring;			// Ring modulation bit
  unsigned int test;			// Test bit
  unsigned int filter;		// Flag: Voice filtered

  // The following bit is set for the modulating
  // voice, not for the modulated one (as the SID bits)
  int sync;			// Sync modulation bit
  int mute;			// Voice muted (voice 3 only)
};

// Data structures for both SIDs

struct sid_t {

  int sid_num;						// SID number (0 or 1)

  struct voice_t voice[3];					// Data for 3 voices

  uint8 regs[128];					// Copies of the 25 write-only SID registers (SIDPlayer uses 128 registers)
  uint8 last_written_byte;			// Byte last written to SID (for emulation of read accesses to write-only registers)
  uint8 volume;						// Master volume (0..15)

  uint8 f_type;						// Filter type
  uint8 f_freq;						// SID filter frequency (upper 8 bits)
  uint8 f_res;						// Filter resonance (0..15)

  filt_t f_ampl;						// IIR filter input attenuation
  filt_t d1, d2, g1, g2;				// IIR filter coefficients
  filt_t xn1_l, xn2_l, yn1_l, yn2_l;	// IIR filter previous input/output signal (left and right channel)
  filt_t xn1_r, xn2_r, yn1_r, yn2_r;

  uint16 v4_left_gain;				// Gain of voice 4 on left channel (12.4 fixed)
  uint16 v4_right_gain;				// Gain of voice 4 on right channel (12.4 fixed)

#ifdef SID_PLAYER
  int v4_state;						// State of voice 4 (Galway noise/samples)
  uint32 v4_count;					// Counter for voice 4
  uint32 v4_add;						// Added to counter in every frame

  uint16 gn_adr;						// C64 address of tone list
  uint16 gn_tone_length;				// Length of each tone in samples
  uint32 gn_volume_add;				// Added to SID volume reg. for every sample
  int	gn_tone_counter;				// Number of tones in list
  uint16 gn_base_cycles;				// Cycles before sample
  uint16 gn_loop_cycles;				// Cycles between samples
  uint32 gn_last_count;				// Value of v4_count (lower 16 bits cleared) at end of tone

  uint32 sm_adr;						// C64 nybble address of sample
  uint32 sm_end_adr;					// C64 nybble address of end of sample
  uint32 sm_rep_adr;					// C64 nybble address of sample repeat point
  uint16 sm_volume;					// Sample volume (0..2, 0=loudest)
  uint8 sm_rep_count;					// Sample repeat counter (0xff=continous)
  int sm_big_endian;					// Flag: Sample is big-endian
#endif
} sid1, sid2;

void sid_t_reset(struct sid_t* sid);
Uint32 read(struct sid_t* sid, Uint32 adr, cycle_t now);
//	uint32 read(uint32 adr, cycle_t now);
void write(struct sid_t* sid, uint32 adr, uint32 byte, cycle_t now, int rmw);
void sid_t_calc_filter(struct sid_t* sid);
void sid_t_calc_gains0(struct sid_t * sid, int is_left_sid, int is_right_sid);
//  void calc_gains0(struct sid_t* sid, int is_left_sid, int is_right_sid);
void calc_gain_voice(int32 volume, int32 panning, uint16 *left_gain, uint16 *right_gain);
void chunk_read(struct sid_t*, size_t size);
void chunk_write();


// Waveform tables
static uint16 tri_table[0x1000*2];
static const uint16 *tri_saw_table;
static const uint16 *tri_rect_table;
static const uint16 *saw_rect_table;
static const uint16 *tri_saw_rect_table;

// Sampled from a 6581R4
static const uint16 tri_saw_table_6581[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0808,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1010, 0x3C3C,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0808,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1010, 0x3C3C
};

static const uint16 tri_rect_table_6581[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8080,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8080,
  0, 0, 0, 0, 0, 0, 0x8080, 0xC0C0, 0, 0x8080, 0x8080, 0xE0E0, 0x8080, 0xE0E0, 0xF0F0, 0xFCFC,
  0xFFFF, 0xFCFC, 0xFAFA, 0xF0F0, 0xF6F6, 0xE0E0, 0xE0E0, 0x8080, 0xEEEE, 0xE0E0, 0xE0E0, 0x8080, 0xC0C0, 0, 0, 0,
  0xDEDE, 0xC0C0, 0xC0C0, 0, 0x8080, 0, 0, 0, 0x8080, 0, 0, 0, 0, 0, 0, 0,
  0xBEBE, 0x8080, 0x8080, 0, 0x8080, 0, 0, 0, 0x8080, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0x7E7E, 0x4040, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint16 saw_rect_table_6581[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7878,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7878
};

static const uint16 tri_saw_rect_table_6581[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Sampled from an 8580R5
static const uint16 tri_saw_table_8580[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0808,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1818, 0x3C3C,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1C1C,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8080, 0, 0x8080, 0x8080,
  0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xE0E0, 0xF0F0, 0xF0F0, 0xF0F0, 0xF0F0, 0xF8F8, 0xF8F8, 0xFCFC, 0xFEFE
};

static const uint16 tri_rect_table_8580[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0xFFFF, 0xFCFC, 0xF8F8, 0xF0F0, 0xF4F4, 0xF0F0, 0xF0F0, 0xE0E0, 0xECEC, 0xE0E0, 0xE0E0, 0xC0C0, 0xE0E0, 0xC0C0, 0xC0C0, 0xC0C0,
  0xDCDC, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0x8080, 0x8080, 0xC0C0, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0, 0,
  0xBEBE, 0xA0A0, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0, 0x8080, 0x8080, 0, 0, 0, 0, 0, 0,
  0x8080, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0x7E7E, 0x7070, 0x6060, 0, 0x4040, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint16 saw_rect_table_8580[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8080,
  0, 0, 0, 0, 0, 0, 0x8080, 0x8080, 0, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0xB0B0, 0xBEBE,
  0, 0, 0, 0, 0, 0, 0, 0x8080, 0, 0, 0, 0x8080, 0x8080, 0x8080, 0x8080, 0xC0C0,
  0, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0xC0C0, 0x8080, 0x8080, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xDCDC,
  0x8080, 0x8080, 0x8080, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xE0E0, 0xE0E0, 0xE0E0, 0xE0E0, 0xECEC,
  0xC0C0, 0xE0E0, 0xE0E0, 0xE0E0, 0xE0E0, 0xF0F0, 0xF0F0, 0xF4F4, 0xF0F0, 0xF0F0, 0xF8F8, 0xF8F8, 0xF8F8, 0xFCFC, 0xFEFE, 0xFFFF
};

static const uint16 tri_saw_rect_table_8580[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8080, 0x8080,
  0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0xC0C0, 0xC0C0, 0xC0C0, 0xC0C0, 0xE0E0, 0xE0E0, 0xE0E0, 0xF0F0, 0xF8F8, 0xFCFC
};

// Envelope tables
static uint32 eg_table[16];

static const uint8 eg_dr_shift[256] = {
  5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,
  3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// Filter tables
static filt_t ffreq_lp[256];	// Low-pass resonance frequency table
static filt_t ffreq_hp[256];	// High-pass resonance frequency table

static const int16 sample_tab[16] = {
  0x8000, 0x9111, 0xa222, 0xb333, 0xc444, 0xd555, 0xe666, 0xf777,
  0x0888, 0x1999, 0x2aaa, 0x3bbb, 0x4ccc, 0x5ddd, 0x6eee, 0x7fff
};


// Work buffer and variables for audio effects
//const int WORK_BUFFER_SIZE = 0x10000;
#define WORK_BUFFER_SIZE  0x10000
static int16 work_buffer[WORK_BUFFER_SIZE];
static int wb_read_offset = 0, wb_write_offset = 0;
static int rev_feedback = 0;

// Prototypes
//calc_buffer(NULL, buf, count);
static void calc_buffer(void *userdata, uint8 *buf, int count);
/*
  static void sid1_chunk_read(size_t size);
static int sid1_chunk_write();
static void sid2_chunk_read(size_t size);
static int sid2_chunk_write();
*/

/*
 *  Init SID emulation
 */
// sid_t::sid_t(int n) : sid_num(n)
void init_sid_t(struct sid_t* sid){
  // Link voices
  sid->voice[0].mod_by = sid->voice+2;
  sid->voice[1].mod_by = sid->voice+0;
  sid->voice[2].mod_by = sid->voice+1;
  sid->voice[0].mod_to = sid->voice+1;
  sid->voice[1].mod_to = sid->voice+2;
  sid->voice[2].mod_to = sid->voice+0;

  sid_t_reset(sid);
}

void set_desired_samples(int32 sample_rate)
{
  if (sample_rate < 15000)
	desired.samples = 256;
  else if (sample_rate < 30000)
	desired.samples = 512;
  else
	desired.samples = 1024;
}

void set_rev_delay(int32 delay_ms)
{
  int delay = (delay_ms * obtained.freq / 1000) & ~1;
  if (delay == 0)
	delay = 2;
  wb_read_offset = (wb_write_offset - delay) & (WORK_BUFFER_SIZE - 1);
}

void calc_gains(struct sid_t *sid)
{
  sid_t_calc_gains0(sid, 0,0);
}

void set_sid_data()
{
  if (emulate_8580) {
	tri_saw_table = tri_saw_table_8580;
	tri_rect_table = tri_rect_table_8580;
	saw_rect_table = saw_rect_table_8580;
	tri_saw_rect_table = tri_saw_rect_table_8580;
  } else {
	tri_saw_table = tri_saw_table_6581;
	tri_rect_table = tri_rect_table_6581;
	saw_rect_table = saw_rect_table_6581;
	tri_saw_rect_table = tri_saw_rect_table_6581;
  }
}


void cSIDInit()
{
  // Use Catweasel?
  cwsid_fh = -1;

  init_sid_t(&sid1);
  calc_gains(&sid1);

  foutp = fopen(path, "w");

  // Set sample buffer size
  set_desired_samples(desired.freq);

  // Open audio device
  desired.callback = calc_buffer;
  desired.userdata = NULL;
  if (cwsid_fh < 0)
	SDL_OpenAudio(&desired, &obtained);

  // Convert reverb delay to sample frame count
  int32 defaultdelay_ms = 200;
  set_rev_delay(defaultdelay_ms);

  // Compute number of cycles per sample frame and envelope table
  SIDClockFreqChanged();

  // Compute triangle table
  int i=0;
  for (i=0; i<0x1000; i++) {
	tri_table[i] = (i << 4) | (i >> 8);
	tri_table[0x1fff-i] = (i << 4) | (i >> 8);
  }

  // Compute filter tables
  for (i=0; i<256; i++) {
#ifdef USE_FIXPOINT_MATHS
	ffreq_lp[i] = int32(CALC_RESONANCE_LP(i) * 65536.0);
	ffreq_hp[i] = int32(CALC_RESONANCE_HP(i) * 65536.0);
#else
	ffreq_lp[i] = CALC_RESONANCE_LP(i);
	ffreq_hp[i] = CALC_RESONANCE_HP(i);
#endif
  }

  // Start sound output
  SDL_PauseAudio(0);

}


/*
 *  Exit SID emulation
 */

void cSIDExit()
{
  SDL_CloseAudio();
}


/*
 *  Reset SID emulation
 */

void sid_t_reset(struct sid_t* sid)
{
  memset(sid->regs, 0, sizeof(sid->regs));
  sid->last_written_byte = 0;

  sid->volume = 0;

  int v=0;
  for (v=0; v<3; v++) {
	sid->voice[v].wave = WAVE_NONE;
	sid->voice[v].eg_state = EG_IDLE;
	sid->voice[v].count = sid->voice[v].add = 0;
	sid->voice[v].freq = sid->voice[v].pw = 0;
	sid->voice[v].eg_level = sid->voice[v].s_level = 0;
	sid->voice[v].a_add = sid->voice[v].d_sub = sid->voice[v].r_sub = eg_table[0];
	sid->voice[v].gate = sid->voice[v].ring = sid->voice[v].test = 0;
	sid->voice[v].filter = sid->voice[v].sync = sid->voice[v].mute = 0;
  }

  sid->f_type = FILT_NONE;
  sid->f_freq = sid->f_res = 0;
  sid->f_ampl = F_ONE;
  sid->d1 = sid->d2 = sid->g1 = sid->g2 = F_ZERO;
  sid->xn1_l = sid->xn2_l = sid->yn1_l = sid->yn2_l = F_ZERO;
  sid->xn1_r = sid->xn2_r = sid->yn1_r = sid->yn2_r = F_ZERO;

}

void SIDReset(cycle_t now)
{
  SDL_LockAudio();
  sid_t_reset(&sid1);
  //	sid2.reset();
  SDL_UnlockAudio();
}


/*
 *  Clock overflow
 */

void SIDClockOverflow(cycle_t sub)
{
}


/*
 *  Clock frequency changed (result of VIC type change)
 */

void SIDClockFreqChanged()
{
  // Compute number of cycles per sample frame
  sid_cycles = cycles_per_second / obtained.freq;
#ifdef USE_FIXPOINT_MATHS
  sid_cycles_frac = (int32)((float) cycles_per_second) * 65536.0 / obtained.freq;
#else
  sid_cycles_frac = ((float)cycles_per_second) / obtained.freq;
#endif

  // Compute envelope table
  static const uint32 div[16] = {
	9, 32, 63, 95, 149, 220, 267, 313,
	392, 977, 1954, 3126, 3906, 11720, 19531, 31251
  };
  int i=0;
  for (i=0; i<16; i++)
	eg_table[i] = (sid_cycles << 16) / div[i];

  // Recompute voice_t::add values
  write(&sid1, 0, sid1.regs[0], 0, 0);
  write(&sid1, 7, sid1.regs[7], 0, 0);
  write(&sid1, 14, sid1.regs[14], 0, 0);
  write(&sid2, 0, sid2.regs[0], 0, 0);
  write(&sid2, 7, sid2.regs[7], 0, 0);
  write(&sid2, 14, sid2.regs[14], 0, 0);
}



/*
 *  Fill audio buffer with SID sound
 */

void calc_sid(struct sid_t *sid, int32 *sum_output_left, int32 *sum_output_right)
{
  // Sampled voice (!! todo: gain/panning)
  /*#if 0	//!!
	uint8 master_volume = sid->sample_buf[(sample_count >> 16) % SAMPLE_BUF_SIZE];
	sample_count += ((0x138 * 50) << 16) / obtained.freq;
	#else
	uint8 master_volume = sid->volume;
	#endif
  */
  uint8 master_volume = sid->volume;
  *sum_output_left += sample_tab[master_volume] << 8;
  *sum_output_right += sample_tab[master_volume] << 8;


  int32 sum_output_filter_left = 0, sum_output_filter_right = 0;

  // Loop for all three voices
  struct voice_t *v;
  int j;
  for (j=0; j<3; j++) {
	v = sid->voice + j;

	// Envelope generator
	uint16 envelope;

	switch (v->eg_state) {
	case EG_ATTACK:
	  v->eg_level += v->a_add;
	  if (v->eg_level > 0xffffff) {
		v->eg_level = 0xffffff;
		v->eg_state = EG_DECAY;
	  }
	  break;
	case EG_DECAY:
	  if (v->eg_level <= v->s_level || v->eg_level > 0xffffff)
		v->eg_level = v->s_level;
	  else {
		v->eg_level -= v->d_sub >> eg_dr_shift[v->eg_level >> 16];
		if (v->eg_level <= v->s_level || v->eg_level > 0xffffff)
		  v->eg_level = v->s_level;
	  }
	  break;
	case EG_RELEASE:
	  v->eg_level -= v->r_sub >> eg_dr_shift[v->eg_level >> 16];
	  if (v->eg_level > 0xffffff) {
		v->eg_level = 0;
		v->eg_state = EG_IDLE;
	  }
	  break;
	case EG_IDLE:
	  v->eg_level = 0;
	  break;
	}
	envelope = (v->eg_level * master_volume) >> 20;

	// Waveform generator
	uint16 output;

	if (!v->test)
	  v->count += v->add;

	if (v->sync && (v->count >= 0x1000000))
	  v->mod_to->count = 0;

	v->count &= 0xffffff;

	switch (v->wave) {
	case WAVE_TRI:
	  if (v->ring)
		output = tri_table[(v->count ^ (v->mod_by->count & 0x800000)) >> 11];
	  else
		output = tri_table[v->count >> 11];
	  break;
	case WAVE_SAW:
	  output = v->count >> 8;
	  break;
	case WAVE_RECT:
	  if (v->count > (uint32)(v->pw << 12))
		output = 0xffff;
	  else
		output = 0;
	  break;
	case WAVE_TRISAW:
	  output = tri_saw_table[v->count >> 16];
	  break;
	case WAVE_TRIRECT:
	  if (v->count > (uint32)(v->pw << 12))
		output = tri_rect_table[v->count >> 16];
	  else
		output = 0;
	  break;
	case WAVE_SAWRECT:
	  if (v->count > (uint32)(v->pw << 12))
		output = saw_rect_table[v->count >> 16];
	  else
		output = 0;
	  break;
	case WAVE_TRISAWRECT:
	  if (v->count > (uint32)(v->pw << 12))
		output = tri_saw_rect_table[v->count >> 16];
	  else
		output = 0;
	  break;
	case WAVE_NOISE:
	  if (v->count >= 0x100000) {
		output = v->noise = noise_rand() << 8;
		v->count &= 0xfffff;
	  } else
		output = v->noise;
	  break;
	default:
	  output = 0x8000;
	  break;
	}
	int32 x = (int16)(output ^ 0x8000) * envelope;
	if (v->filter) {
	  sum_output_filter_left += (x * v->left_gain) >> 4;
	  sum_output_filter_right += (x * v->right_gain) >> 4;
	} else if (!(v->mute)) {
	  *sum_output_left += (x * v->left_gain) >> 4;
	  *sum_output_right += (x * v->right_gain) >> 4;
	}
  }


  // Filter
  if (enable_filters) {
#ifdef USE_FIXPOINT_MATHS	//!!
	int32 xn = sid.f_ampl.imul(sum_output_filter);
	int32 yn = xn+sid.d1.imul(xn1)+sid.d2.imul(xn2)-sid.g1.imul(yn1)-sid.g2.imul(yn2);
	sum_output_filter = yn;
#endif
	float xn = (float)(sum_output_filter_left) * sid->f_ampl;
	float yn = xn + sid->d1 * sid->xn1_l + sid->d2 * sid->xn2_l - sid->g1 * sid->yn1_l - sid->g2 * sid->yn2_l;
	sum_output_filter_left = (int32)(yn);
	sid->yn2_l = sid->yn1_l; sid->yn1_l = yn; sid->xn2_l = sid->xn1_l; sid->xn1_l = xn;
	xn = (float)(sum_output_filter_right) * sid->f_ampl;
	yn = xn + sid->d1 * sid->xn1_r + sid->d2 * sid->xn2_r - sid->g1 * sid->yn1_r - sid->g2 * sid->yn2_r;
	sum_output_filter_right = (int32)(yn);
	sid->yn2_r = sid->yn1_r; sid->yn1_r = yn; sid->xn2_r = sid->xn1_r; sid->xn1_r = xn;
  }

  // Add filtered and non-filtered output
  *sum_output_left += sum_output_filter_left;
  *sum_output_right += sum_output_filter_right;
}


//calc_buffer(NULL, buf, count);
void calc_buffer(void *userdata, uint8 *buf, int count)
{
  uint16 *buf16 = (uint16 *)buf;

  // Convert buffer length (in bytes) to frame count
  int is_stereo = (obtained.channels == 2);
  int is_16_bit = !(obtained.format == AUDIO_U8 || obtained.format == AUDIO_S8);
  if (is_stereo)
	count >>= 1;
  if (is_16_bit)
	count >>= 1;

  // Main calculation loop
  while (count--) {
	int32 sum_output_left = 0, sum_output_right = 0;
	fprintf(foutp, "zero %u %i i\n",count, sum_output_left);

	// Calculate output of voices from both SIDs
	calc_sid(&sid1, &sum_output_left, &sum_output_right);
	fprintf(foutp, " %u %i %i i\n",count, sum_output_left, sum_output_right);
	if (dual_sid)
	  calc_sid(&sid2, &sum_output_left, &sum_output_right);

	// Apply audio effects (post-processing)
	if (audio_effect) {
	  sum_output_left >>= 11;
	  sum_output_right >>= 11;
	  if (audio_effect == 1) {	// Reverb
		sum_output_left += (rev_feedback * work_buffer[wb_read_offset++]) >> 8;
		work_buffer[wb_write_offset++] = sum_output_left;
		sum_output_right += (rev_feedback * work_buffer[wb_read_offset]) >> 8;
		work_buffer[wb_write_offset] = sum_output_right;
	  } else {					// Spatial
		sum_output_left += (rev_feedback * work_buffer[wb_read_offset++]) >> 8;
		work_buffer[wb_write_offset++] = sum_output_left;
		sum_output_right -= (rev_feedback * work_buffer[wb_read_offset]) >> 8;
		work_buffer[wb_write_offset] = sum_output_right;
	  }
	  wb_read_offset = (wb_read_offset + 1) & (WORK_BUFFER_SIZE - 1);
	  wb_write_offset = (wb_write_offset + 1) & (WORK_BUFFER_SIZE - 1);
	} else {
	  sum_output_left >>= 10;
	  sum_output_right >>= 10;
	}

	// Clip to 16 bits
	if (sum_output_left > 32767)
	  sum_output_left = 32767;
	else if (sum_output_left < -32768)
	  sum_output_left = -32768;
	if (sum_output_right > 32767)
	  sum_output_right = 32767;
	else if (sum_output_right < -32768)
	  sum_output_right = -32768;

	//	fprintf(foutp, " %u %u i\n",count, *buf16);

	// Write to output buffer
	if (is_16_bit) {
	  if (is_stereo) {
		*buf16++ = sum_output_left;
		//		fprintf(foutp, " %u %u i\n",count, *buf16);
		*buf16++ = sum_output_right;
	  } else {
		*buf16++ = (sum_output_left + sum_output_right) / 2;
		//		fprintf(foutp, " %u %u i\n",count, *buf16);
	  }
	} else {
	  if (is_stereo) {
		*buf++ = (sum_output_left >> 8) ^ 0x80;
		//		fprintf(foutp, " %u %u i\n",count, *buf);
		*buf++ = (sum_output_right >> 8) ^ 0x80;
	  } else {
		*buf++ = ((sum_output_left + sum_output_right) >> 9) ^ 0x80;
		//		fprintf(foutp, " %u %u i\n",count, *buf);
	  }
	}
  }
}

//void calc_buffer(void *userdata, uint8 *buf, int count)
int dodump;
void savesnd( FILE* foutp, uint8 *buf, int count){
  if(dodump){
	fprintf(foutp, " %u %u i\n",count, *buf);
  }
}
#ifdef SID_PLAYER
void SIDCalcBuffer(uint8 *buf, int count)
{
  calc_buffer(NULL, buf, count);
}

uint64 replay_start_time = 0;	// Start time of last replay
int32 over_time = 0;			// Time the last replay was too long

void SIDExecute()
{
  // Delay to maintain proper replay frequency
  uint64 now = GetTicks_usec();
  if (replay_start_time == 0)
	replay_start_time = now;
  uint32 replay_time = now - replay_start_time;
  uint32 adj_nominal_replay_time = uint32((cia_timer + 1) * 100000000.0 / (cycles_per_second * speed_adjust));
  int32 delay = adj_nominal_replay_time - replay_time - over_time;
  over_time = -delay;
  if (over_time < 0)
	over_time = 0;
  if (delay > 0) {
	Delay_usec(delay);
	int32 actual_delay = GetTicks_usec() - now;
	if (actual_delay + 500 < delay)
	  Delay_usec(1);
	actual_delay = GetTicks_usec() - now;
	over_time += actual_delay - delay;
	if (over_time < 0)
	  over_time = 0;
  }
  replay_start_time = GetTicks_usec();

  // Execute 6510 play routine
  UpdatePlayAdr();
  CPUExecute(play_adr, 0, 0, 0, 1000000);
}
#endif


/*
 *  Calculate IIR filter coefficients
 */

void sid_t_calc_filter(struct sid_t *sid)
{
  // Filter off? Then reset all coefficients
  if (sid->f_type == FILT_NONE) {
	sid->f_ampl = F_ZERO;
	sid->d1 = sid->d2 = sid->g1 = sid->g2 = F_ZERO;
	return;
  }

  // Calculate resonance frequency
  filt_t fr;
  if (sid->f_type == FILT_LP || sid->f_type == FILT_LPBP)
	fr = ffreq_lp[sid->f_freq];
  else
	fr = ffreq_hp[sid->f_freq];

  // Limit to <1/2 sample frequency, avoid div by 0 in case FILT_NOTCH below
  filt_t arg = fr / (float)(obtained.freq >> 1);
  if (arg > 0.99)
	arg = 0.99;
  if (arg < 0.01)
	arg = 0.01;

  // Calculate poles (resonance frequency and resonance)
  // The (complex) poles are at
  //   zp_1/2 = (-g1 +/- sqrt(g1^2 - 4*g2)) / 2
  sid->g2 = 0.55 + 1.2 * arg * arg - 1.2 * arg + (float)(sid->f_res) * 0.0133333333;
  sid->g1 = -2.0 * sqrt(sid->g2) * cos(M_PI * arg);

  // Increase resonance if LP/HP combined with BP
  if (sid->f_type == FILT_LPBP || sid->f_type == FILT_HPBP)
	sid->g2 += 0.1;

  // Stabilize filter
  if (fabs(sid->g1) >= sid->g2 + 1.0) {
	if (sid->g1 > 0.0)
	  sid->g1 = sid->g2 + 0.99;
	else
	  sid->g1 = -(sid->g2 + 0.99);
  }

  // Calculate roots (filter characteristic) and input attenuation
  // The (complex) roots are at
  //   z0_1/2 = (-d1 +/- sqrt(d1^2 - 4*d2)) / 2
  switch (sid->f_type) {

  case FILT_LPBP:
  case FILT_LP:		// Both roots at -1, H(1)=1
	sid->d1 = 2.0; sid->d2 = 1.0;
	sid->f_ampl = 0.25 * (1.0 + sid->g1 + sid->g2);
	break;

  case FILT_HPBP:
  case FILT_HP:		// Both roots at 1, H(-1)=1
	sid->d1 = -2.0; sid->d2 = 1.0;
	sid->f_ampl = 0.25 * (1.0 - sid->g1 + sid->g2);
	break;

  case FILT_BP: {		// Roots at +1 and -1, H_max=1
	sid->d1 = 0.0; sid->d2 = -1.0;
	float c = sqrt(sid->g2*sid->g2 + 2.0*sid->g2 - sid->g1*sid->g1 + 1.0);
	sid->f_ampl = 0.25 * (-2.0*sid->g2*sid->g2 - (4.0+2.0*c)*sid->g2 - 2.0*c + (c+2.0)*sid->g1*sid->g1 - 2.0) / (-sid->g2*sid->g2 - (c+2.0)*sid->g2 - c + sid->g1*sid->g1 - 1.0);
	break;
  }

  case FILT_NOTCH:	// Roots at exp(i*pi*arg) and exp(-i*pi*arg), H(1)=1 (arg>=0.5) or H(-1)=1 (arg<0.5)
	sid->d1 = -2.0 * cos(M_PI * arg); sid->d2 = 1.0;
	if (arg >= 0.5)
	  sid->f_ampl = 0.5 * (1.0 + sid->g1 + sid->g2) / (1.0 - cos(M_PI * arg));
	else
	  sid->f_ampl = 0.5 * (1.0 - sid->g1 + sid->g2) / (1.0 + cos(M_PI * arg));
	break;

	// The following is pure guesswork...
  case FILT_ALL:		// Roots at 2*exp(i*pi*arg) and 2*exp(-i*pi*arg), H(-1)=1 (arg>=0.5) or H(1)=1 (arg<0.5)
	sid->d1 = -4.0 * cos(M_PI * arg); sid->d2 = 4.0;
	if (arg >= 0.5)
	  sid->f_ampl = (1.0 - sid->g1 + sid->g2) / (5.0 + 4.0 * cos(M_PI * arg));
	else
	  sid->f_ampl = (1.0 + sid->g1 + sid->g2) / (5.0 - 4.0 * cos(M_PI * arg));
	break;

  default:
	break;
  }
}


/*
 *  Calculate gain values for all voices
 */

void sid_t_calc_gain_voice(struct sid_t* sid, int32 volume, int32 panning, uint16 *left_gain, uint16 *right_gain)
{
  int gain;
  if (panning < -0x100)
	panning = -0x100;
  if (panning > 0x100)
	panning = 0x100;
  gain = (sid->volume * (-panning + 0x100) * master_volume) >> 20;
  if (gain > 0x200)
	gain = 0x200;
  if (gain < 0)
	gain = 0;
  *left_gain = gain;
  gain = (sid->volume * (panning + 0x100) * master_volume) >> 20;
  if (gain > 0x200)
	gain = 0x200;
  if (gain < 0)
	gain = 0;
  *right_gain = gain;
}

void sid_t_calc_gains0(struct sid_t * sid, int is_left_sid, int is_right_sid)
{
  int32 pan_offset = 0;
  if (is_left_sid)
	pan_offset = -dual_sep;
  else if (is_right_sid)
	pan_offset = dual_sep;
  sid_t_calc_gain_voice(sid, v1_volume, v1_panning + pan_offset, &sid->voice[0].left_gain, &sid->voice[0].right_gain);
  sid_t_calc_gain_voice(sid, v2_volume, v2_panning + pan_offset, &sid->voice[1].left_gain, &sid->voice[1].right_gain);
  sid_t_calc_gain_voice(sid, v3_volume, v3_panning + pan_offset, &sid->voice[2].left_gain, &sid->voice[2].right_gain);
  sid_t_calc_gain_voice(sid, v4_volume, v4_panning + pan_offset, &sid->v4_left_gain, &sid->v4_right_gain);
}


/*
 *  Read from SID register
 */

uint32 sid_t_read(struct sid_t* sid, uint32 adr, cycle_t now)
{
  //	D(bug("sid_read from %04x at cycle %d\n", adr, now));

  switch (adr) {
  case 0x19:	// A/D converters
  case 0x1a:
	sid->last_written_byte = 0;
	return 0xff;
  case 0x1b:	// Voice 3 oscillator/EG readout
  case 0x1c:
	sid->last_written_byte = 0;
	return noise_rand();
  default: {	// Write-only register: return last value written to SID
	uint8 ret = sid->last_written_byte;
	sid->last_written_byte = 0;
	return ret;
  }
  }
}

uint32 csid_read(struct sid_t* sid1, struct sid_t* sid2, uint32 adr, cycle_t now)
{

  if (dual_sid) {
	if (adr & 0x20)
	  return sid_t_read(sid2, adr & 0x1f, now);
	else
	  return sid_t_read(sid1, adr & 0x1f, now);
  } else
	return sid_t_read(sid1,adr & 0x1f, now);

}


/*
 *  Write to SID register
 */

void sid_t_write(struct sid_t* sid, uint32 adr, uint32 byte, cycle_t now, int rmw)
{
  //	D(bug("sid_write %02x to %04x at cycle %d\n", byte, adr, now));

#ifdef SID_PLAYER
  // Writing to standard SID mirrored registers
  if ((adr & 0x1f) < 0x1d)
	adr &= 0x1f;
#endif

  sid->last_written_byte = sid->regs[adr] = byte;
  int v = adr/7;	// Voice number


  switch (adr) {
  case 0:
  case 7:
  case 14:
	sid->voice[v].freq = (sid->voice[v].freq & 0xff00) | byte;
	sid->voice[v].add = (uint32)((float)(sid->voice[v].freq) * sid_cycles_frac);
	break;

  case 1:
  case 8:
  case 15:
	sid->voice[v].freq = (sid->voice[v].freq & 0xff) | (byte << 8);
	sid->voice[v].add = (uint32)((float)(sid->voice[v].freq) * sid_cycles_frac);
	break;

  case 2:
  case 9:
  case 16:
	sid->voice[v].pw = (sid->voice[v].pw & 0x0f00) | byte;
	break;

  case 3:
  case 10:
  case 17:
	sid->voice[v].pw = (sid->voice[v].pw & 0xff) | ((byte & 0xf) << 8);
	break;

  case 4:
  case 11:
  case 18:
	sid->voice[v].wave = (byte >> 4) & 0xf;
	if ((byte & 1) != sid->voice[v].gate){
	  if (byte & 1){	// Gate turned on
		sid->voice[v].eg_state = EG_ATTACK;
	  } else {			// Gate turned off
		if (sid->voice[v].eg_state != EG_IDLE){
		  sid->voice[v].eg_state = EG_RELEASE;
		}
	  }
	}
	sid->voice[v].gate = byte & 1;
	sid->voice[v].mod_by->sync = byte & 2;
	sid->voice[v].ring = byte & 4;
	if ((sid->voice[v].test = byte & 8))
	  sid->voice[v].count = 0;
	break;

  case 5:
  case 12:
  case 19:
	sid->voice[v].a_add = eg_table[byte >> 4];
	sid->voice[v].d_sub = eg_table[byte & 0xf];
	break;

  case 6:
  case 13:
  case 20:
	sid->voice[v].s_level = (byte >> 4) * 0x111111;
	sid->voice[v].r_sub = eg_table[byte & 0xf];
	break;

  case 22:
	if (byte != sid->f_freq) {
	  sid->f_freq = byte;
	  if (enable_filters)
		sid_t_calc_filter(sid);
	}
	break;

  case 23:
	sid->voice[0].filter = byte & 1;
	sid->voice[1].filter = byte & 2;
	sid->voice[2].filter = byte & 4;
	if ((byte >> 4) != sid->f_res) {
	  sid->f_res = byte >> 4;
	  if (enable_filters)
		sid_t_calc_filter(sid);
	}
	break;

  case 24:
	sid->volume = byte & 0xf;
	sid->voice[2].mute = byte & 0x80;
	if (((byte >> 4) & 7) != sid->f_type) {
	  sid->f_type = (byte >> 4) & 7;
	  sid->xn1_l = sid->xn2_l = sid->yn1_l = sid->yn2_l = F_ZERO;
	  sid->xn1_r = sid->xn2_r = sid->yn1_r = sid->yn2_r = F_ZERO;
	  if (enable_filters)
		sid_t_calc_filter(sid);
	}
	break;

  }
}

void csid_write(uint32 adr, uint32 byte, cycle_t now, int rmw){
  SDL_LockAudio();

  if (dual_sid) {
	if (adr & 0x20)
	  sid_t_write(&sid2, adr & 0x1f, byte, now, rmw);
	else
	  sid_t_write(&sid1, adr & 0x1f, byte, now, rmw);
  } else
	sid_t_write(&sid1, adr & 0x1f, byte, now, rmw);

  SDL_UnlockAudio();

}


/*
 *  Read/write snapshot chunk
 */
/*
void sid_t_chunk_read(struct sid_t* sid, size_t size)
{
  SDL_LockAudio();
  int i;
  for ( i=0; i<=24; i++)
	write(sid,i, ChunkReadInt8(), 0, 0);
  sid->last_written_byte = ChunkReadInt8();
  sid->voice[0].count = ChunkReadInt32();
  sid->voice[1].count = ChunkReadInt32();
  sid->voice[2].count = ChunkReadInt32();
  sid->voice[0].noise = ChunkReadInt32();
  sid->voice[1].noise = ChunkReadInt32();
  sid->voice[2].noise = ChunkReadInt32();
  sid->voice[0].eg_state = ChunkReadInt8();
  sid->voice[1].eg_state = ChunkReadInt8();
  sid->voice[2].eg_state = ChunkReadInt8();
  sid->voice[0].eg_level = ChunkReadInt32();
  sid->voice[1].eg_level = ChunkReadInt32();
  sid->voice[2].eg_level = ChunkReadInt32();

  SDL_UnlockAudio();
}

void sid1_chunk_read(size_t size)
{
  chunk_read(&sid1, size);
}

void sid2_chunk_read(size_t size)
{
  chunk_read(&sid2,size);
}

void sid_t_chunk_write(struct sid_t* sid)
{
  SDL_LockAudio();
  int i;
  for (i=0; i<=24; i++)
	ChunkWriteInt8(sid->regs[i]);

  ChunkWriteInt8(sid->last_written_byte);
  ChunkWriteInt32(sid->voice[0].count);
  ChunkWriteInt32(sid->voice[1].count);
  ChunkWriteInt32(sid->voice[2].count);
  ChunkWriteInt32(sid->voice[0].noise);
  ChunkWriteInt32(sid->voice[1].noise);
  ChunkWriteInt32(sid->voice[2].noise);
  ChunkWriteInt8(sid->voice[0].eg_state);
  ChunkWriteInt8(sid->voice[1].eg_state);
  ChunkWriteInt8(sid->voice[2].eg_state);
  ChunkWriteInt32(sid->voice[0].eg_level);
  ChunkWriteInt32(sid->voice[1].eg_level);
  ChunkWriteInt32(sid->voice[2].eg_level);

  SDL_UnlockAudio();
}

int sid1_chunk_write()
{
  chunk_write(&sid1);
  return 1;
}

int sid2_chunk_write()
{
  if (!dual_sid)
	return 0;
  chunk_write(&sid2);
  return 1;
}

 
*/

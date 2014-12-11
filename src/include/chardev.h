/*
  chardev.h
  bho1 - init 29.12.2011
  GPL

  simulating a character video chip

*/

#include "SDL.h"
//#include "chardotmatrix.h"
struct charmapentry
{
  int ix; // dotmatrix table index
  unsigned char symbol; // symbol in dotmatix table at index ix
  char * namestr;
};

void draw_char0(unsigned int xcoord, unsigned int ycoord, int ix );
void draw_char(unsigned int xcoord, unsigned int ycoord, int ix );
unsigned int lookup_char(struct charmapentry *chartable, unsigned char *ptr);
void draw_vcs(char vicmem[][10]);
void draw_txtbuffer();
// char vcs[][10];
extern char vcs[][10];




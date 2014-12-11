/*

  color.c
  bho1 - init 19.12.2013 

  

  
*/
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>


#include "register.h"
#include "cpu.h"
#include "flags.h"
#include "alu.h"
#include "decode.h"
#include "memory.h"
#include "cpu-util.h"

#include "src-ripper.h"
#include "printers.h"
#include "parser.h"
#include "color.h"
#include "SDL.h"

//#include "bitmap.h"

/*
  #define BLACK \e[1;38;05;000m
  #define WHITE \e[1;38;05;001m
  #define RED  \e[1;38;05;002m
  #define CYAN  \e[1;38;05;003m
  #define PURPLE  \e[1;38;05;004m
  #define GREEN  \e[1;38;05;005m
  #define BLUE  \e[1;38;05;006m
  #define YELLOW  \e[1;38;05;007m
  #define ORANGE  \e[1;38;05;008m
  #define BROWN  \e[1;38;05;009m
  #define LIGHT  \e[1;38;05;010m($a)
  #define DARK  \e[1;38;05;011m ($b)
  #define GRAY  \e[1;38;05;012m
  #define LIGHT_GREEN \e[1;38;05;013m
  #define LIGHT_BLUE \e[1;38;05;014m
  #define LIGHT_GRAY \e[1;38;05;015m
*/
/*
  #define BLACK "\e[1;38;05;000m"
  #define WHITE "\e[1;38;05;001m"
  #define RED " \e[1;38;05;002m"
  #define CYAN " \e[1;38;05;003m"
  #define PURPLE " \e[1;38;05;004m"
  #define GREEN " \e[1;38;05;005m"
  #define BLUE " \e[1;38;05;006m"
  #define YELLOW " \e[1;38;05;007m"
  #define ORANGE " \e[1;38;05;008m"
  #define BROWN " \e[1;38;05;009m"
  #define LIGHT " \e[1;38;05;010m"
  #define DARK " \e[1;38;05;011m"
  #define GRAY " \e[1;38;05;012m"
  #define LIGHT_GREEN "\e[1;38;05;013m"
  #define LIGHT_BLUE "\e[1;38;05;014m"
  #define LIGHT_GRAY "\e[1;38;05;015m"
  #define RESET "\e[m"
*/
//char* terminal_colors[16] = { BLACK, WHITE, RED, CYAN, PURPLE, GREEN, BLUE, YELLOW, ORANGE, BROWN, LIGHT, DARK, GRAY, LIGHT_GREEN, LIGHT_BLUE, LIGHT_GRAY };
void textcolor(int attr, int fg, int bg){
  char command[13];
  
	/* Command is the control command to the terminal */
	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	printf("%s", command);
}
void print_color(int color){
	char command[13];
	/* Command is the control command to the terminal */
	sprintf(command, "%c[%d;%d;%d;%dm", 0x1B, 1, color + 30, color + 40, color);
	printf("%s %02x ", command,color);
}

void print_color0(int color){
  switch(color){
  case 0:
	textcolor(BRIGHT,BLACK,WHITE);
	printf("%02x ", color);
	break;
  case 1:
	textcolor(BRIGHT, RED, WHITE);
	printf("%02x ", color);
	break;
  case 2:
	textcolor(BRIGHT, GREEN, WHITE);
	printf("%02x ", color);
	break;
  case 3:
	textcolor(BRIGHT, CYAN, WHITE);
	printf("%02x ", color);
	break;
  case 4:
	textcolor(BRIGHT, GREEN, WHITE);
	printf("%02x ", color);
	break;
  case 5:
	textcolor(BRIGHT, GREEN, WHITE);
	printf("%02x ", color);
	break;
  case 6:
	textcolor(BRIGHT, BLUE, WHITE);
	printf("%02x ", color);
	break;
  case 7:
	textcolor(BRIGHT, YELLOW, WHITE);
	printf("%02x ", color);
	break;
  case 8:
	/*	textcolor(BRIGHT, ORANGE, WHITE);
	printf("%02x ", color);
	break;
	*/
  case 9:
	/*
	textcolor(BRIGHT, BROWN, WHITE);
	printf("%02x ", color);
	break;
  case 10:
	textcolor(BRIGHT, LIGHT, WHITE);
	printf("%02x ", color);
	break;
  case 11:
	textcolor(BRIGHT, DARK, WHITE);
	printf("%02x ", color);
	break;
  case 12:
	textcolor(BRIGHT, GRAY, WHITE);
	printf("%02x ", color);
	break;
	*/
  case 10:
  case 11:
  case 12:
  case 13:	
	textcolor(BRIGHT, GREEN, WHITE);
	printf("%02x", color);
	break;
  case 14:
	textcolor(BRIGHT, BLUE, WHITE);
	printf("%02x", color);
	break;
  case 15:
	textcolor(BRIGHT,MAGENTA, WHITE);
	printf("%02x", color);
	break;
  }	

  /*
	((color>0) && (color<16)) ? printf("%s%02x%s",terminal_colors[color], color, RESET ) : 0 ; ;
	(color>=16) ? 	printf("%s%02x%s",terminal_colors[0], 0, RESET ) : 0 ; ;
	(color<0) ? printf("%s%02x%s",terminal_colors[1], 1, RESET ) : 0 ; ;
  */
  /*
  char buffer[32] = "";
  switch(color){
  case 0:
	printf(BLACK "%02x " RESET ,color);
	break;
  case 1:
	printf(RED "%02x " RESET ,color);
	break;
  case 2:
	printf(GREEN "%02x " RESET ,color);
	break;
  case 3:
	printf(CYAN "%02x " RESET ,color);
	break;
  case 4:
	printf(PURPLE "%02x " RESET ,color);
	break;
  case 5:
	printf(GREEN "%02x " RESET ,color);
	break;
  case 6:
	printf(BLUE "%02x " RESET ,color);
	break;
  case 7:
	printf(YELLOW "%02x " RESET ,color);
	break;
  case 8:
	printf(ORANGE "%02x " RESET ,color);
	break;
  case 9:
	printf(BROWN "%02x " RESET ,color);
	break;
  case 10:
	printf(LIGHT "%02x " RESET ,color);
	break;
  case 11:
	printf(DARK "%02x " RESET ,color);
	break;
  case 12:
	printf(GRAY "%02x " RESET ,color);
	break;
  case 13:
	printf(LIGHT_GREEN "%02x " RESET ,color);
	break;
  case 14:
	printf(LIGHT_BLUE "%02x " RESET ,color);
	break;
  case 15:
	printf(LIGHT_GRAY "%02x " RESET ,color);
	break;
  }	
  */
  /*
	if(color<=16 && color>=0){
	printf(terminal_colors[color]"%02x%s", color, RESET );
	printf("" RESET );
	}	else if (color>16){
	printf("%s%02x%s",terminal_colors[1], 1, RESET );
	}	else if (color<16)
	printf("%s%02x%s",terminal_colors[2], 2, RESET );
  */
  /*
  	if(color<=16 && color>=0)
	sprintf(buffer, "%s%02x%s",terminal_colors[color], color, RESET );
	else if (color>16)
	sprintf(buffer, "%s%02x%s",terminal_colors[1], 1, RESET );
	else if (color<16)
	sprintf(buffer, "%s%02x%s",terminal_colors[2], 2, RESET );
  */
  /*
	if(color<=16 && color>=0)
	printf("%s%02x%s",terminal_colors[color], color, RESET );
	else if (color>16)
	printf("%s%02x%s",terminal_colors[1], 1, RESET );
	else if (color<16)
	printf("%s%02x%s",terminal_colors[2], 2, RESET );
  */

}

/*
  register.c
  bho1 2006
  bho1 2007
  GPL
*/
#include <ctype.h>
#include <stdio.h>

#include "register.h"
#include "alu.h"


void ldhex2regnibble(char hexcode, char* regnibble){
	
	switch (tolower(hexcode)){
	case '0':
		*regnibble++ = '0';
		*regnibble++ = '0';
		*regnibble++ = '0';
		*regnibble = '0';
		break;
	case '1':
		*regnibble =   '0';
		*(regnibble+1) = '0';
		*(regnibble+2) = '0';
		*(regnibble+3) = '1';
		break;
	case '2':
		*regnibble =   '0';
		*(regnibble+1) = '0';
		*(regnibble+2) = '1';
		*(regnibble+3) = '0';
		break;
	case '3':
		*regnibble =   '0';
		*(regnibble+1) = '0';
		*(regnibble+2) = '1';
		*(regnibble+3) = '1';
		break;
	case '4':
		*regnibble =   '0';
		*(regnibble+1) = '1';
		*(regnibble+2) = '0';
		*(regnibble+3) = '0';
		break;
	case '5':
		*regnibble =   '0';
		*(regnibble+1) = '1';
		*(regnibble+2) = '0';
		*(regnibble+3) = '1';
		break;
	case '6':
		*regnibble =   '0';
		*(regnibble+1) = '1';
		*(regnibble+2) = '1';
		*(regnibble+3) = '0';
		break;
	case '7':
		*regnibble =   '0';
		*(regnibble+1) = '1';
		*(regnibble+2) = '1';
		*(regnibble+3) = '1';
		break;
	case '8':
		*regnibble =   '1';
		*(regnibble+1) = '0';
		*(regnibble+2) = '0';
		*(regnibble+3) = '0';
		break;
	case '9':
		*regnibble =   '1';
		*(regnibble+1) = '0';
		*(regnibble+2) = '0';
		*(regnibble+3) = '1';
		break;
	case 'a' :
		*regnibble =   '1';
		*(regnibble+1) = '0';
		*(regnibble+2) = '1';
		*(regnibble+3) = '0';
		break;
	case 'b' :
		*regnibble =   '1';
		*(regnibble+1) = '0';
		*(regnibble+2) = '1';
		*(regnibble+3) = '1';
		break;
	case 'c' :
		*regnibble =   '1';
		*(regnibble+1) = '1';
		*(regnibble+2) = '0';
		*(regnibble+3) = '0';
		break;
	case 'd' :
		*regnibble =   '1';
		*(regnibble+1) = '1';
		*(regnibble+2) = '0';
		*(regnibble+3) = '1';
		break;
	case 'e' :
		*regnibble =   '1';
		*(regnibble+1) = '1';
		*(regnibble+2) = '1';
		*(regnibble+3) = '0';
		break;
	case 'f' :
		*regnibble =   '1';
		*(regnibble+1) = '1';
		*(regnibble+2) = '1';
		*(regnibble+3) = '1';
		break;
	}
}

/*
  convert bitstring <bitstr> from bit <start> to bit <end> into integer
  101000 ->
   ^  ^
   |  |
 start|
     end
  convert "010" into int 2
*/
int conv_bitstr2int(char *bitstr, unsigned int start, unsigned int end){
  unsigned int i=0;
  int sum=0;
  for(i=start; i<=end;i++)
    if(bitstr[i] == '1'){
      sum = (sum << 1) | 1;
    }else
      sum = (sum << 1);
  return sum;
}

/*
  s2int(char*reg)
  signed register value to C int representation
*/
int s2int(char* reg){
  char dummy[8];
  cp_register(reg, dummy);

  int signeda;
  if(reg[0] == '1'){
    two_complement(dummy);
    signeda = - conv_bitstr2int(dummy, 0, 7);
  }else{
    signeda = conv_bitstr2int(reg, 0, 7);
  }
  return signeda;
}


void ldhex2register(char* hexcode, char* reg){
	switch(REG_WIDTH){
		case 8:
			ldhex2regnibble(*hexcode, reg);
			ldhex2regnibble(*(hexcode+1), reg+4);
			break;
		case 16:
			ldhex2regnibble(*hexcode, reg);
			ldhex2regnibble(*(hexcode+1), reg+4);
			ldhex2regnibble(*(hexcode+2), reg+8);
			ldhex2regnibble(*(hexcode+3), reg+12);
			break;
	}
}

void print_reg(char* reg)
{
  printf("%s", reg);
  printf("\n");
}

void cp_register(char *r1, char *r2){
  int i;
  for(i=0; i<REG_WIDTH; i++)
    r2[i] = r1[i];
  r2[i] = 0;
}


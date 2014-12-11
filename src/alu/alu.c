/* 
   alu.c 
   - 21.11.05/BHO1
   bho1 29.12.2006
   bho1 6.12.2007
   bho1 30.11.2007 - clean up
   bho1 24.11.2009 - assembler instruction
   bho1 3.12.2009 - replaced adder with full_adder
   bho1 20.7.2011 - rewrite: minimize global vars, ALU-operations are modeled with fct taking in/out register as parameter
   bho1 6.11.2011 - rewrite flags: adding flags as functional parameter. Now alu is truly a function
   bho1 26.11.2012 - remove bit declaration from op_alu_asl and op_alu_ror as they are unused (this may change later)
   bho1 20.9.2014 cleaned
   thons1 21.11.2014 - Adding full-adder
   thons1 24.11.2014 - Implementing ADD, ADC, SUB, SBC
   thons1 27.11.2014 - Implementing bitwise operations
   thons1 29.11.2014 - General rework and cleanup. Ready for rollout
   
   GPL applies

   -->> Stefan Thoeni  <<--
*/

#include <stdio.h>
#include <string.h>

#include "alu.h"
#include "alu-opcodes.h"
#include "register.h"
#include "flags.h"
int const max_mue_memory = 100;

char mue_memory[100]= "100 Byte - this memory is at your disposal"; /*mue-memory */
char* m = mue_memory;

unsigned int c = 0; 	/* carry bit address    */
unsigned int s = 1;	    /* sum bit address      */
unsigned int c_in = 2;	/* carry in bit address */

/*
  testet ob alle bits im akkumulator auf null gesetzt sind.
  Falls ja wird 1 returniert, ansonsten 0
*/
int zero_test(char accumulator[]){
  int i;
  for(i=0;accumulator[i]!='\0'; i++){
	if(accumulator[i]!='0')		
	  return 0;
  }

  return 1;
}

void zsflagging(char* flags,char *acc){
  //Zeroflag
  if(zero_test(acc))
	setZeroflag(flags);
  else
	clearZeroflag(flags);

  //Signflag
  if(acc[0] == '1')
	setSignflag(flags);
  else
	clearSignflag(flags);
}

void carryflagging(char* flags){
  if(m[c] == '0')
    clearCarryflag(flags);
  else
    setCarryflag(flags);
}

void coflagging(char* flags, char *accumulator, char *rega, char *regb){
  carryflagging(flags);
  //Overflowflag
  if(rega[0] == regb[0] && regb[0] != accumulator[0])
    setOverflowflag(flags);
  else
    clearOverflowflag(flags);
}

/*
  logic and of 2 bit
  returns a AND b
 */
char and(char a, char b){
  return (a == '1' && b == '1') ? '1' : '0';
}

/*
  logic or of 2 bit
  returns a OR b
 */
char or(char a, char b){
  return (a =='1' || b =='1' ) ? '1' : '0';
}

/*
  logic xor of 2 bit
  returns a XOR b
 */
char xor(char a, char b){
  return ((a =='1' || b =='1') && a != b ) ? '1' : '0';
}

/*
  Halfadder: addiert zwei character p,q und schreibt in 
  den Mue-memory das summen-bit und das carry-bit.
*/
void half_adder(char p, char q){
  m[s] = xor(p,q);
  m[c] = and(p,q);
}

/*
  Reset ALU
  resets registers and calls alu_op_reset 
*/
void op_alu_reset(char rega[], char regb[], char accumulator[], char flags[]){
  int i;
  /* clear rega, regb, accumulator, flags */
  for(i=0; i<REG_WIDTH; i++)
    m[i] = rega[i] = regb[i] = accumulator[i] = flags[i] = '0';
}

/* 
   void adder(char pbit, char qbit, char cbit)
   Adder oder auch Fulladder:
   Nimmt zwei character bits und ein carry-character-bit
   und schreibt das Resultat (summe, carry) in den Mue-speicher
*/
void full_adder(char pbit, char qbit, char cbit){
  // half add p and q
  half_adder(pbit,qbit);
  char c1 = m[c];

  // Half-add sum(q,p) and carry in
  half_adder(m[s], cbit);

  m[c] = or(m[c], c1);
}

/*
  Invertieren der Character Bits im Register reg
  one_complement(char reg[]) --> NOT(reg)
*/
void one_complement(char reg[]){
  int i;
  for (i = 0; i< REG_WIDTH; i++) {
    reg[i] = (reg[i] == '0') ? '1' : '0';
  }
}

/*
  Das zweier-Komplement des Registers reg wird in reg geschrieben
  reg := K2(reg)
*/
void two_complement(char reg[]){
  // Build one complement
  one_complement(reg);
  
  // add 1 to reg: Iterate from behind, set 1 to 0 and 0 to 1. exit on first 0 
  int i; 
  for (i = REG_WIDTH-1; i>=0; i--) {
    if(reg[i] == '1'){
      reg[i] = '0';
    }else{
      reg[i] = '1';
	break;
    }
  }
}

/*
  Generic add function.
  For op_code ADD set carry argument = '0'
  For op_code ADC set carry argument = currentCarry
 */
static void add_generic(char rega[], char regb[], char accumulator[], char flags[], char carry){
  //Set carry flag to whatever it is supposed to be
  m[c] = carry;

  int i;
  for (i = REG_WIDTH-1; i>=0; i--) {
    full_adder(rega[i], regb[i], m[c]);
    
    //Store the sum from full_adder
    accumulator[i] = m[s];
  }

  // Now we can set the flags:
  zsflagging(flags, accumulator);
  coflagging(flags, accumulator, rega, regb);
}

/*
  Die Werte in Register rega und Register regb werden addiert, das
  wird in Register accumulator geschrieben. Die Flags cflag, 
  oflag, zflag und sflag werden entsprechend gesetzt
  
  accumulator := rega + regb
*/
void op_add(char rega[], char regb[], char accumulator[], char flags[]){
  add_generic(rega,regb,accumulator,flags,'0');
}

/*

  ALU_OP_ADD_WITH_CARRY
  
  Die Werte des carry-Flags und der Register rega und
  Register regb werden addiert, das
  Resultat wird in Register accumulator geschrieben. Die Flags cflag, 
  oflag, zflag und sflag werden entsprechend gesetzt
  
  accumulator := rega + regb + carry-flag
*/
void op_adc(char rega[], char regb[], char accumulator[], char flags[]){
  add_generic(rega, regb, accumulator, flags, getCarryflag(flags));
}

/*
  Die Werte in Register rega und Register regb werden subtrahiert, das
  Resultat wird in Register accumulator geschrieben. Die Flags cflag, 
  oflag, zflag und sflag werden entsprechend gesetzt
  
  accumulator := rega - regb = rega + NOT(regb) + 1
*/
void op_sub(char rega[], char regb[], char accumulator[], char flags[]){
  one_complement(regb);
  
  add_generic(rega,regb,accumulator,flags, '1');

  // We want the positive representation of b in our register!
  one_complement(regb);
}

/*
  subtract with carry
  SBC
  accumulator =
  a - b - !c  =
  a - b - !c + 256 =
  a - b - (1-c) + 256 =
  a + (255 - b) + c =
  a + !b + c
  accumulator := rega - regb = rega + NOT(regb) +carryflag

*/
void op_alu_sbc(char rega[], char regb[], char accumulator[], char flags[]){
  one_complement(regb);
  
  add_generic(rega,regb,accumulator,flags, getCarryflag(flags));
  
  // We want the positive representation of b in our register!
  one_complement(regb);
}

/*
  Applies any logical function (char funname(char,char);) to rega and regb
  Result is stored in accu and z,s flags are set accoringly
 
 */
static void apply_logic(char rega[], char regb[], char accumulator[], char flags[], char (*logFn)(char,char)){
  int i;
  for (i = 0; i < REG_WIDTH; i++) {
    accumulator[i] = (*logFn)(rega[i], regb[i]);
  }
  zsflagging(flags, accumulator);
}

/*
  Die Werte in Register rega und Register regb werden logisch geANDet, 
  das Resultat wird in Register accumulator geschrieben. 
  Die Flags zflag und sflag werden entsprechend gesetzt
  
  accumulator := rega AND regb
*/
void op_and(char rega[], char regb[], char accumulator[], char flags[]){
  apply_logic(rega,regb,accumulator,flags,&and);
}

/*
  Die Werte in Register rega und Register regb werden logisch geORt, 
  das Resultat wird in Register accumulator geschrieben. 
  Die Flags zflag und sflag werden entsprechend gesetzt
 
  accumulator := rega OR regb
*/
void op_or(char rega[], char regb[], char accumulator[], char flags[]){
  apply_logic(rega,regb,accumulator,flags,&or);
}

/*
  Die Werte in Register rega und Register regb werden logisch geXORt, 
  das Resultat wird in Register accumulator geschrieben. 
  Die Flags zflag und sflag werden entsprechend gesetzt
 
  accumulator := rega OR regb
*/
void op_xor(char rega[], char regb[], char accumulator[], char flags[]){
  apply_logic(rega,regb,accumulator,flags,xor);
}

/*
  Einer-Komplement von Register rega
  rega := not(rega)
*/
void op_not_a(char rega[], char regb[], char accumulator[], char flags[]){
  one_complement(rega);
}

/* Einer Komplement von Register regb
   regb := not(regb)
 */
void op_not_b(char rega[], char regb[], char accumulator[], char flags[]){
  one_complement(regb);
}

/*
  Negation von Register rega 
  rega := -rega
*/
void op_neg_a(char rega[], char regb[], char accumulator[], char flags[]){
  two_complement(rega);
}

/*
  Negation von Register regb 
  regb := -regb
*/
void op_neg_b(char rega[], char regb[], char accumulator[], char flags[]){
  two_complement(regb);
}

/*
        bit ->   7                           0
               +---+---+---+---+---+---+---+---+
 carryflag <-- |   |   |   |   |   |   |   |   |  <-- 0
               +---+---+---+---+---+---+---+---+

	       arithmetic shift left
  asl
*/
void op_alu_asl(char regina[], char reginb[], char regouta[], char flags[]){
  // First bit will always be '0':
  regouta[REG_WIDTH-1] = '0';
  m[c] = regina[0];

  int i;
  for (i = REG_WIDTH-1; i > 0; i--) {
      regouta[i-1] = regina[i];
  }

  carryflagging(flags);
}

/*
  logical shift right
  lsr
*/
void op_alu_lsr(char regina[], char reginb[], char regouta[], char flags[]){
  // First bit will always be '0':
  regouta[0] = '0';

  int i;
  for (i = 0; i <= REG_WIDTH-2; i++) {
      regouta[i+1] = regina[i];
  }
}
/*
  rotate 
  rotate left
*/
void op_alu_rol(char regina[], char reginb[], char regouta[], char flags[]){
  regouta[REG_WIDTH-1] = getCarryflag(flags);
  m[c] = regina[0];

  int i;
  for (i = REG_WIDTH -1; i>0; i--) {
      regouta[i-1] = regina[i];
  }

  carryflagging(flags);
}

/*
  rotate 
  rotate right
  Move each of the bits in  A one place to the right. Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.
*/
void op_alu_ror(char regina[], char reginb[], char regouta[], char flags[]){
  //First bit equals last bit from regina
  regouta[0] = getCarryflag(flags);
  m[c] = regina[REG_WIDTH -1];
  
  int i;
  for (i = 0; i < REG_WIDTH-1; i++) {
      regouta[i+1] = regina[i];
  }
  
  carryflagging(flags);
}

/*
  Procedural approach to ALU with side-effect:
  Needed register are already alocated and may be modified
  mainly a switchboard
  
  alu_fct(int opcode, char reg_in_a[], char reg_in_b[], char reg_out_accu[], char flags[])
*/
void alu(unsigned int alu_opcode, char reg_in_a[], char reg_in_b[], char reg_out_accu[], char flags[]){
  char dummyflags[9] = "00000000";  
  switch ( alu_opcode ){
  case ALU_OP_ADD :
    op_add(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_ADD_WITH_CARRY :
    op_adc(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_SUB :
    op_sub(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_SUB_WITH_CARRY :
    op_alu_sbc(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_AND :
    op_and(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_OR:
    op_or(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_XOR :
    op_xor(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_NEG_A :
    op_neg_a(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_NEG_B :
    op_neg_b(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_NOT_A :
    op_not_a(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_NOT_B :
    op_not_b(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_ASL :
    op_alu_asl(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_LSR :
    op_alu_lsr(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_ROL: 
    op_alu_rol(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_ROR: 
    op_alu_ror(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  case ALU_OP_RESET :
    op_alu_reset(reg_in_a, reg_in_b, reg_out_accu, (flags==NULL)?dummyflags:flags);
    break;
  default:
    printf("ALU(%i): Invalide operation %i selected", alu_opcode, alu_opcode); 
  }	
}

/*
  register.h
  bho1 2006
  bho1 2007
*/

#define REG_WIDTH 8

void cpu_6502_register_dump(char* filename);

void print_reg(char*);

/*
  copy register source to register target:
  void cp_register(char* source, char* target);
*/
void cp_register(char* , char* );

/*
  ldhex2register("AF", registera)
  void ldhex2register(char* hexcode, char* reg)
*/
void ldhex2register(char*, char*);

/*
  convert bitstring <bitstr> from bit <start> to bit <end> into integer
  101000 ->
   ^  ^
   |  |
 start|
     end
  convert "010" into int 2
*/
int conv_bitstr2int(char *bitstr, unsigned int start, unsigned int end);

/*
  s2int(char*reg)
  signed register value to C int representation
*/
int s2int(char* reg);



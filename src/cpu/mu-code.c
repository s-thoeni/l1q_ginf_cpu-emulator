/*
  6502-mu-code.c
  bho1 12.7.2011
  starting from template generated with ./tableparse.bash
  bho1 27.12.2011
  added adr-modes.h, reengienieering
  GPL

*/
#include <stddef.h> // somehow we need NULL
#include <stdio.h>

#include "alu.h"
#include "alu-opcodes.h"
#include "register.h"
#include "memory.h"
#include "flags.h"
#include "cpu-util.h"
#include "cpu.h"
#include "mu-code.h"
#include "adr-modes.h"

void store(char* byte){
  cp_register(byte,dbr);
  set_rw2write();
  access_memory();
}  

void am_immediate(){
  cp_register(pcl, abrl);
  cp_register(pch, abrh);
}

void am_zp(){
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, abrl);
  cp_register("00000000",abrh);
}

void am_zpx(){
  char zero[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);
}

void am_izx(){
  char _low[] = "00000000";
  char _zero[] = "00000000";
  char _one[] = "00000001";
  char localflags[] = "00000000";

  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, abrl, localflags);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, _low);

  alu(ALU_OP_ADD_WITH_CARRY, abrl, _one, abrl, localflags);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  //Because fuck you thats why
  cp_register(_low,abrl);
  cp_register(dbr,abrh);
}

void am_izy(){
  //Da hats n Bock drinn!

  char _low[] = "00000000";
  char _high[] = "00000000";
  char _zero[] = "00000000";
  char _one[] = "00000001";
  char localflags[] = "00000000";

  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  
  cp_register(dbr, abrl);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, _low);
  alu(ALU_OP_ADD, abrl, _one, abrl, localflags);

  set_rw2read();
  access_memory();

  cp_register(dbr, _high);

  alu(ALU_OP_ADD, _low, idy, abrl, localflags);
  alu(ALU_OP_ADD_WITH_CARRY, zero, _high, abrh, localflags);
}

void am_abs(){
  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(_tmp, abrl);
  cp_register(dbr, abrh);
}

void am_abx(){
  char low[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idx,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
}

void am_aby(){
  char low[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idy,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
}

void cmp(char *rega, char *regb, char* flags){
  int a,m;
  a =  conv_bitstr2int(rega, 0, 7);
  m =  conv_bitstr2int(regb, 0, 7);
  char dummy[REG_WIDTH+1] = "00000000";
 
  alu(ALU_OP_SUB,rega,regb,dummy,flags);
  zsflagging(flags,dummy);
  if(a>=m){
	setCarryflag(flags);
  }else{
	clearCarryflag(flags);
  } 
}


void flipcarry(char* flags){
  if(getCarryflag(flags)=='1')
	clearCarryflag(flags);
  else
	setCarryflag(flags);
}


void load(char* byte){
  set_rw2read();
  access_memory();
  cp_register(dbr,byte);
}


void flagging(char *flags,char* reg){
  if(getZeroflag(reg)=='1')
	setZeroflag(flags);
  else
	clearZeroflag(flags);
  if(getSignflag(reg)=='1')
	setSignflag(flags);
  else
	clearSignflag(flags);

}

int k_two2int(char* p){
  char dummy[REG_WIDTH+1];
  int i;
  if(p[0]==0){
	return conv_bitstr2int(p, 1, 7);
  }
  else {
	for(i=0; p[i]; i++){
	  dummy[i]=p[i];
	}
	dummy[i]=p[i];
	
	alu(ALU_OP_NEG_A, dummy, dummy, dummy, dummy);
	return -conv_bitstr2int(dummy, 1, 7);
  }
}

/*
  pop1 is a helper function
  copy one byte from stack into register reg
  pops one Byte register from stack
  modifies stack pointer
*/
void pop1(char* reg){
  inc_register(sp);
  cp_register(sp, abrl);
  cp_register("00000001", abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, reg);
}

/*
  push1 is a helper function
  pushes one Byte register onto stack
  modifies stack pointer
*/
void push1(char* reg){
  cp_register("00000001", abrh);
  cp_register(sp, abrl);
  cp_register(reg, dbr);
  
  set_rw2write();
  access_memory();
  dec_register(sp);
}


/*
 6502 mu-function implementation (file://home/olivier/6502.html#BPL )
 name: BPL 
 code: 0x10
 address-mode: rel
 function: branch on N=0 
 mnemonic: BPL rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bpl_rel(){
  cycles = 2;

  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getSignflag(flags) == '0'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}



/*
 6502 mu-function implementation (file:///home/olivier/6502.html#BMI )
 name: BMI
 code: 0x30
 address-mode: rel
 function: branch on N=1 
 mnemonic: BMI rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bmi_rel(){
  cycles = 2;
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getSignflag(flags) == '1'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}


/*
 6502 mu-function implementation (file:///home/olivier/6502.html#BVC )
 name: BVC
 code: 0x50
 address-mode: rel
 function: branch on V=0 
 mnemonic: BVC rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bvc_rel(){
  cycles = 2;
  
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getOverflowflag(flags) == '0'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BVS )
 name: BVS
 code: 0x70
 address-mode: rel
 function: branch on V=1 
 mnemonic: BVS rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bvs_rel(){
  cycles = 2;
  
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getOverflowflag(flags) == '1'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BCC )
 name: BCC
 code: 0x90
 address-mode: rel
 function: branch on C=0 
 mnemonic: BCC rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bcc_rel(){
  cycles = 2;
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getCarryflag(flags) == '0'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BCS )
 name: BCS
 code: 0xB0
 address-mode: rel
 function: branch on C=1 
 mnemonic: BCS rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bcs_rel(){
  cycles = 2;
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getCarryflag(flags) == '1'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BNE )
 name: BNE
 code: 0xD0
 address-mode: rel
 function: branch on Z=0 
 mnemonic: BNE rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_bne_rel(){
  cycles = 2;
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();

  if(getZeroflag(flags) == '0'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BEQ )
 name: BEQ
 code: 0xF0
 address-mode: rel
 function: branch on Z=1 
 mnemonic: BEQ rel
 bytes: 2
 cycles: 2
*/
void cpu_6502_beq_rel(){
  cycles = 2;
  char localflags[]="00000000";
  char zero[]="00000000";

  am_immediate();

  set_rw2read();
  access_memory();
  
  if(getZeroflag(flags) == '1'){
    alu(ALU_OP_ADD,pcl,dbr,pcl,localflags);
    alu(ALU_OP_ADD,pch,zero,pch,localflags);
  }

  inc_pc();
}



/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BRK )
 name: BRK
 code: 0x00
 address-mode: imp
 function: (S)-=:PC,P PC:=($FFFE) 
 mnemonic: BRK impl
 bytes: 0
 cycles: 7
*/
void cpu_6502_brk_imp(){
  cycles = 7;
  char high[]="11111111";
  char low[]="11111110";
  char localflags[]="00000000";

  cp_register(flags, localflags);
  setBRKflag(localflags);

  inc_pc();
  push1(pch);
  push1(pcl);
  push1(localflags);
  

  cp_register(high, pch);
  cp_register(low, pcl);
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#RTI )
 name: RTI
 code: 0x40
 address-mode: imp
 function: P,PC:=+(S) 
 mnemonic: RTI impl
 bytes: X
 cycles: 6
*/
void cpu_6502_rti_imp(){
  cycles = 6;
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#JSR )
 name: JSR
 code: 0x20
 address-mode: abs
 function: (S)-:=PC PC:={adr} 
 mnemonic: JSR abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_jsr_abs(){
  cycles = 6;
  char localflags[]="00000000";
  char _tmpL[]="00000000";
  char _tmpH[]="00000000";
  char _one[]="00000001";
  char _zero[]="00000000";
  
  alu(ALU_OP_SUB, pcl, _one, _tmpL, localflags);
  alu(ALU_OP_SUB_WITH_CARRY, pch, _zero, _tmpH, localflags);
  
  push1(_tmpH);
  push1(_tmpL);

  am_abs();  

  cp_register(abrl, pcl);
  cp_register(abrh, pch);
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#RTS )
 name: RTS
 code: 0x60
 address-mode: imp
 function: PC:=+(S) 
 mnemonic: RTS impl
 bytes: X
 cycles: 6
*/
void cpu_6502_rts_imp(){
  cycles = 6;
  pop1(pcl);
  pop1(pch);

  inc_pc();
  inc_pc();
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#JMP )
 name: JMP
 code: 0x4C
 address-mode: abs
 function: PC:={adr} 
 mnemonic: JMP abs
 bytes: X
 cycles: 3
*/
void cpu_6502_jmp_abs(){
  cycles = 3;
  
  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(_tmp, pcl);
  cp_register(dbr, pch);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#JMP )
 name: JMP
 code: 0x6C
 address-mode: ind
 function: PC:={adr} 
 mnemonic: JMP ind
 bytes: X
 cycles: 5
*/
void cpu_6502_jmp_ind(){
  cycles = 5;
  char one[] = "00000001";
  char localflags[] = "00000000";
  
  am_abs();

  set_rw2read();
  access_memory();

  cp_register(dbr, pcl);

  alu(ALU_OP_ADD, one, dbr, abrl, localflags);
  alu(ALU_OP_ADD_WITH_CARRY, zero, abrh, abrh, localflags);

  set_rw2read();
  access_memory();

  cp_register(dbr, pch);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BIT )
 name: BIT
 code: 0x24
 address-mode: zp
 function: N:=b7 V:=b6 Z:=A&{adr} 
 mnemonic: BIT zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_bit_zp(){
  cycles = 3;

}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#BIT )
 name: BIT
 code: 0x2C
 address-mode: abs
 function: N:=b7 V:=b6 Z:=A&{adr} 
 mnemonic: BIT abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_bit_abs(){
  cycles = 4;

}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CLC )
 name: CLC
 code: 0x18
 address-mode: imp
 function: C:=0 
 mnemonic: CLC impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_clc_imp(){
  cycles = 2;
  clearCarryflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SEC )
 name: SEC
 code: 0x38
 address-mode: imp
 function: C:=1 
 mnemonic: SEC impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_sec_imp(){
  cycles = 2;
  setCarryflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CLD )
 name: CLD
 code: 0xD8
 address-mode: imp
 function: D:=0 
 mnemonic: CLD impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_cld_imp(){
  cycles = 2;
  clearDflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SED )
 name: SED
 code: 0xF8
 address-mode: imp
 function: D:=1 
 mnemonic: SED impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_sed_imp(){
  cycles = 2;
  setDflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CLI )
 name: CLI
 code: 0x58
 address-mode: imp
 function: I:=0 
 mnemonic: CLI impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_cli_imp(){
  cycles = 2;
  clearIRQflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SEI )
 name: SEI
 code: 0x78
 address-mode: imp
 function: I:=1 
 mnemonic: SEI impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_sei_imp(){
  cycles = 2;
  setIRQflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CLV )
 name: CLV
 code: 0xB8
 address-mode: imp
 function: V:=0 
 mnemonic: CLV impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_clv_imp(){
  cycles = 2;
  clearOverflowflag(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#NOP )
 name: NOP
 code: 0xEA
 address-mode: imp
 function:   
 mnemonic: NOP impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_nop_imp(){
  cycles = 2;
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xA9
 address-mode: imm
 function: A:={adr} 
 mnemonic: LDA #
 bytes: 2
 cycles: 2
*/
void cpu_6502_lda_imm(){
  cycles = 2;
 
  am_immediate();
  
  set_rw2read();
  access_memory();

  cp_register(dbr, acc);
  zsflagging(flags,idx);
  inc_pc();
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xA5
 address-mode: zp
 function: A:={adr} 
 mnemonic: LDA zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_lda_zp(){
  cycles = 3;
  
  am_zp();

  set_rw2read();
  access_memory();
  
  cp_register(dbr, acc);
  zsflagging(flags,idx);
  inc_pc();
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xB5
 address-mode: zpx
 function: A:={adr} 
 mnemonic: LDA zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_lda_zpx (){
  cycles = 4;

  am_zpx();

  set_rw2read();
  access_memory();

  cp_register(dbr, acc);
  zsflagging(flags,acc);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xA1
 address-mode: izx
 function: A:={adr} 
 mnemonic: LDA X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_lda_izx(){
  cycles = 6;

  am_izx();

  set_rw2read();
  access_memory();

  cp_register(dbr, acc);
  
  inc_pc();
}

/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xB1
 address-mode: izy
 function: A:={adr} 
 mnemonic: LDA ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_lda_izy(){
  cycles = 5;
  
  am_izy();

  set_rw2read();
  access_memory();

  cp_register(dbr, acc);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xAD
 address-mode: abs
 function: A:={adr} 
 mnemonic: LDA abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_lda_abs(){
  cycles = 4;

  am_abs();

  set_rw2read();
  access_memory();
  
  cp_register(dbr, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xBD
 address-mode: abx
 function: A:={adr} 
 mnemonic: LDA abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_lda_abx (){
  cycles = 4;
  
  am_abx();

  cp_register(dbr, acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDA )
 name: LDA
 code: 0xB9
 address-mode: aby
 function: A:={adr} 
 mnemonic: LDA abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_lda_aby(){
  cycles = 4;
  
  am_aby();

  cp_register(dbr, acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x85
 address-mode: zp
 function: {adr}:=A 
 mnemonic: STA zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_sta_zp(){
  cycles = 3;

  am_zp();

  cp_register(acc, dbr);

  set_rw2write();
  access_memory();
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x95
 address-mode: zpx
 function: {adr}:=A 
 mnemonic: STA zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_sta_zpx (){
  cycles = 4;

  am_zpx();

  cp_register(acc, dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x81
 address-mode: izx
 function: {adr}:=A 
 mnemonic: STA X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_sta_izx(){
  cycles = 6;

  am_izx();

  cp_register(acc, dbr);

  set_rw2write();
  access_memory();
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x91
 address-mode: izy
 function: {adr}:=A 
 mnemonic: STA ind,Y
 bytes: 2
 cycles: 6
*/
void cpu_6502_sta_izy(){
  cycles = 6;
  
  am_izy();

  cp_register(dbr, acc);

  set_rw2write();
  access_memory();
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x8D
 address-mode: abs
 function: {adr}:=A 
 mnemonic: STA abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_sta_abs(){
  cycles = 3;
  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(dbr, abrl);
  cp_register(_tmp, abrh);
  
  cp_register(acc, dbr);

  set_rw2write();
  access_memory();
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x9D
 address-mode: abx
 function: {adr}:=A 
 mnemonic: STA abs,X
 bytes: 3
 cycles: 5n
*/
void cpu_6502_sta_abx (){
  cycles = 5;
  
  char low[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idx,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  cp_register(acc, dbr);

  set_rw2write();
  access_memory();
  
  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STA )
 name: STA
 code: 0x99
 address-mode: aby
 function: {adr}:=A 
 mnemonic: STA abs,Y
 bytes: 3
 cycles: 5
*/
void cpu_6502_sta_aby(){
  cycles = 5;
  char low[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idy,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);
  cp_register(acc, dbr);

  set_rw2write();
  access_memory();
  
  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDX )
 name: LDX
 code: 0xA2
 address-mode: imm
 function: X:={adr} 
 mnemonic: LDX #
 bytes: 2
 cycles: 2
*/
void cpu_6502_ldx_imm(){
  cycles = 2;
 
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, idx);

  zsflagging(flags,idx);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDX )
 name: LDX
 code: 0xA6
 address-mode: zp
 function: X:={adr} 
 mnemonic: LDX zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_ldx_zp(){
  cycles = 3;
  char zero[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, idx);
  zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDX )
 name: LDX
 code: 0xB6
 address-mode: zpy
 function: X:={adr} 
 mnemonic: LDX zpg,Y
 bytes: 2
 cycles: 4
*/
void cpu_6502_ldx_zpy(){
  cycles = 4;
  char zero[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idy, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, idx);
  zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDX )
 name: LDX
 code: 0xAE
 address-mode: abs
 function: X:={adr} 
 mnemonic: LDX abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_ldx_abs(){
  cycles = 4;
  char *reg = edata_abs();
  cp_register(reg,idx);
  zsflagging(flags,idx);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDX )
 name: LDX
 code: 0xBE
 address-mode: aby
 function: X:={adr} 
 mnemonic: LDX abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_ldx_aby(){
  cycles = 4;

  char low[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idy,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
  
  cp_register(dbr, idy);
  zsflagging(flags,idx);
  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STX )
 name: STX
 code: 0x86
 address-mode: zp
 function: {adr}:=X 
 mnemonic: STX zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_stx_zp(){
  cycles = 3;

  am_zp();

  cp_register(idx, dbr);

  set_rw2write();
  access_memory();
    zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STX )
 name: STX
 code: 0x96
 address-mode: zpy
 function: {adr}:=X 
 mnemonic: STX zpg,Y
 bytes: 2
 cycles: 4
*/
void cpu_6502_stx_zpy(){
  cycles = 4;

  char zero[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idy, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  cp_register(dbr, idx);

  set_rw2write();
  access_memory();
  zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STX )
 name: STX
 code: 0x8E
 address-mode: abs
 function: {adr}:=X 
 mnemonic: STX abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_stx_abs(){
  cycles = 4;

  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(dbr, abrl);
  cp_register(_tmp, abrh);
  
  cp_register(dbr, idx);

  set_rw2write();
  access_memory();
    zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDY )
 name: LDY
 code: 0xA0
 address-mode: imm
 function: Y:={adr} 
 mnemonic: LDY #
 bytes: 2
 cycles: 2
*/
void cpu_6502_ldy_imm(){
  cycles = 2;
 
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, idy);

  zsflagging(flags,idx);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDY )
 name: LDY
 code: 0xA4
 address-mode: zp
 function: Y:={adr} 
 mnemonic: LDY zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_ldy_zp(){
  cycles = 3;

  char zero[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, idy);
  zsflagging(flags,idx);
  inc_pc();

}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDY )
 name: LDY
 code: 0xB4
 address-mode: zpx
 function: Y:={adr} 
 mnemonic: LDY zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_ldy_zpx (){
  cycles = 4;
  char zero[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, acc);
  zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDY )
 name: LDY
 code: 0xAC
 address-mode: abs
 function: Y:={adr} 
 mnemonic: LDY abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_ldy_abs(){
  cycles = 4;

  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(dbr, abrl);
  cp_register(_tmp, abrh);
  
  set_rw2read();
  access_memory();

  cp_register(dbr, idy);
    zsflagging(flags,idx);
  inc_pc();
}



/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LDY )
 name: LDY
 code: 0xBC
 address-mode: abx
 function: Y:={adr} 
 mnemonic: LDY abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_ldy_abx(){
  cycles = 4;
  
  char low[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idx,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
  
  cp_register(dbr, idy);

  inc_pc();  

}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STY )
 name: STY
 code: 0x84
 address-mode: zp
 function: {adr}:=Y 
 mnemonic: STY zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_sty_zp(){
  cycles = 3;

  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();

  
  cp_register(dbr,abrl);
  cp_register(zero,abrh);
  cp_register(idy,dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}



/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STY )
 name: STY
 code: 0x94
 address-mode: zpx
 function: {adr}:=Y 
 mnemonic: STY zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_sty_zpx (){
  cycles = 4;
  char zero[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, idy);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#STY )
 name: STY
 code: 0x8C
 address-mode: abs
 function: {adr}:=Y 
 mnemonic: STY abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_sty_abs(){
  cycles = 4;

  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(dbr, abrl);
  cp_register(_tmp, abrh);
  
  cp_register(idy, dbr);

  set_rw2write();
  access_memory();
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#TAX )
 name: TAX
 code: 0xAA
 address-mode: imp
 function: X:=A 
 mnemonic: TAX impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_tax_imp(){
  cycles = 2;
  cp_register(acc,idx);
  zsflagging(flags, idx);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#TXA )
 name: TXA
 code: 0x8A
 address-mode: imp
 function: A:=X 
 mnemonic: TXA impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_txa_imp(){
  cycles = 2;
  cp_register(idx,acc);
  zsflagging(flags, acc);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#TAY )
 name: TAY
 code: 0xA8
 address-mode: imp
 function: Y:=A 
 mnemonic: TAY impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_tay_imp(){
  cycles = 2;
  cp_register(acc, idy);
  zsflagging(flags, idy);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#TYA )
 name: TYA
 code: 0x98
 address-mode: imp
 function: A:=Y 
 mnemonic: TYA impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_tya_imp(){
  cycles = 2;
  cp_register(idy, acc);
  zsflagging(flags, acc);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#TSX )
 name: TSX
 code: 0xBA
 address-mode: imp
 function: X:=S 
 mnemonic: TSX impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_tsx_imp(){
  cycles = 2;
  cp_register(sp, idx);
  zsflagging(flags, idx);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#TXS )
 name: TXS
 code: 0x9A
 address-mode: imp
 function: S:=X 
 mnemonic: TXS impl
 bytes: X
 cycles: 2
*/
void cpu_6502_txs_imp(){
  cycles = 2;
  cp_register(idx, sp);
  zsflagging(flags, sp);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#PLA )
 name: PLA
 code: 0x68
 address-mode: imp
 function: A:=+(S) 
 mnemonic: PLA impl
 bytes: 1
 cycles: 4
*/
void cpu_6502_pla_imp(){
  cycles = 4;
  pop1(acc);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#PHA )
 name: PHA
 code: 0x48
 address-mode: imp
 function: (S)-:=A 
 mnemonic: PHA impl
 bytes: 1
 cycles: 3
*/
void cpu_6502_pha_imp(){
  cycles = 3;
  push1(acc);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#PLP )
 name: PLP
 code: 0x28
 address-mode: imp
 function: P:=+(S) 
 mnemonic: PLP impl
 bytes: 1
 cycles: 4
*/
void cpu_6502_plp_imp(){
  cycles = 4;
  pop1(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#PHP )
 name: PHP
 code: 0x08
 address-mode: imp
 function: (S)-:=P 
 mnemonic: PHP impl
 bytes: 1
 cycles: 3
*/
void cpu_6502_php_imp(){
  cycles = 3;
  push1(flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x09
 address-mode: imm
 function: A:=A or {adr} 
 mnemonic: ORA #
 bytes: 2
 cycles: 2
*/
void cpu_6502_ora_imm(){
  cycles = 2;
 
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_OR, dbr, acc, acc, flags);

  zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x05
 address-mode: zp
 function: A:=A or {adr} 
 mnemonic: ORA zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_ora_zp(){
  cycles = 3;

  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();
  
  cp_register(dbr,abrl);
  cp_register("00000000",abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_OR, dbr, acc, acc, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x15
 address-mode: zpx
 function: A:=A or {adr} 
 mnemonic: ORA zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_ora_zpx (){
  cycles = 4;
  char zero[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_OR, dbr, acc, acc, localflags);
  zsflagging(flags,acc);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x01
 address-mode: izx
 function: A:=A or {adr} 
 mnemonic: ORA X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_ora_izx(){
  cycles = 6;
  char _low[] = "00000000";
  char _zero[] = "00000000";
  char _one[] = "00000001";
  char localflags[] = "00000000";

  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, abrl, localflags);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, _low);

  alu(ALU_OP_ADD_WITH_CARRY, abrl, _one, abrl, localflags);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  //Because fuck you thats why
  cp_register(_low,abrl);
  cp_register(dbr,abrh);

  set_rw2read();
  access_memory();
  alu(ALU_OP_OR, dbr, acc, acc, localflags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x11
 address-mode: izy
 function: A:=A or {adr} 
 mnemonic: ORA ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_ora_izy(){
  cycles = 5;
  char localflags[]="00000000";
  am_izy();

  set_rw2read();
  access_memory();

  alu(ALU_OP_OR, dbr, acc, acc, localflags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x0D
 address-mode: abs
 function: A:=A or {adr} 
 mnemonic: ORA abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_ora_abs(){
  cycles = 4;
  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(_tmp, abrl);
  cp_register(dbr, abrh);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_OR, dbr, acc, acc, flags);

  inc_pc();

}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x1D
 address-mode: abx
 function: A:=A or {adr} 
 mnemonic: ORA abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_ora_abx (){
  cycles = 4;
  
  char low[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idx,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_OR, dbr, acc, acc, localflags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ORA )
 name: ORA
 code: 0x19
 address-mode: aby
 function: A:=A or {adr} 
 mnemonic: ORA abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_ora_aby(){
  cycles = 4;
  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(_tmp, abrl);
  cp_register(dbr, abrh);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_OR, dbr, acc, acc, flags);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x29
 address-mode: imm
 function: A:=A&{adr} 
 mnemonic: AND #
 bytes: 2
 cycles: 2
*/
void cpu_6502_and_imm(){
  cycles = 2;
 
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_AND, dbr, acc, acc, flags);

  zsflagging(flags,idx);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x25
 address-mode: zp
 function: A:=A&{adr} 
 mnemonic: AND zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_and_zp(){
  cycles = 3;

  cp_register(pcl,abrl);
  cp_register(pch,abrh);

  set_rw2read();
  access_memory();
  
  cp_register(dbr,abrl);
  cp_register("00000000",abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_AND, dbr, acc, acc, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x35
 address-mode: zpx
 function: A:=A&{adr} 
 mnemonic: AND zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_and_zpx (){
  cycles = 4;
  char zero[]="00000000";
  char localflags[]="00000000";

  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, dbr, localflags);
  cp_register(dbr, abrl);
  cp_register(zero, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_AND, dbr, acc, acc, localflags);
  zsflagging(flags,acc);
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x21
 address-mode: izx
 function: A:=A&{adr} 
 mnemonic: AND X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_and_izx(){
  cycles = 6;
  char _low[] = "00000000";
  char _zero[] = "00000000";
  char _one[] = "00000001";
  char localflags[] = "00000000";

  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD, dbr, idx, abrl, localflags);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, _low);

  alu(ALU_OP_ADD_WITH_CARRY, abrl, _one, abrl, localflags);
  cp_register(_zero, abrh);

  set_rw2read();
  access_memory();

  //Because fuck you thats why
  cp_register(_low,abrl);
  cp_register(dbr,abrh);

  set_rw2read();
  access_memory();
  alu(ALU_OP_AND, dbr, acc, acc, localflags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x31
 address-mode: izy
 function: A:=A&{adr} 
 mnemonic: AND ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_and_izy(){
  cycles = 5;
  char localflags[]="00000000";
  am_izy();

  set_rw2read();
  access_memory();

  alu(ALU_OP_AND, dbr, acc, acc, localflags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x2D
 address-mode: abs
 function: A:=A&{adr} 
 mnemonic: AND abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_and_abs(){
  cycles = 4;
  char _tmp[] = "00000000";
  
  // Read address high from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();
  // Temporary store it
  cp_register(dbr, _tmp);
  
  inc_pc();
  
  // Read adress low from pc
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  // store acc to absolute address from memory
  cp_register(_tmp, abrl);
  cp_register(dbr, abrh);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_AND, dbr, acc, acc, flags);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x3D
 address-mode: abx
 function: A:=A&{adr} 
 mnemonic: AND abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_and_abx (){
  cycles = 4;
  
  char low[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idx,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_AND, dbr, acc, acc, localflags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#AND )
 name: AND
 code: 0x39
 address-mode: aby
 function: A:=A&{adr} 
 mnemonic: AND abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_and_aby(){
  cycles = 4;
  char low[]="00000000";
  char localflags[]="00000000";
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  cp_register(dbr, low);

  inc_pc();
  cp_register(pcl, abrl);
  cp_register(pch, abrh);

  set_rw2read();
  access_memory();

  alu(ALU_OP_ADD,low,idy,abrl,localflags);
  alu(ALU_OP_ADD_WITH_CARRY,dbr,"00000000",abrh,localflags);

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_AND, dbr, acc, acc, localflags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x49
 address-mode: imm
 function: A:=A exor {adr} 
 mnemonic: EOR #
 bytes: 2
 cycles: 2
*/
void cpu_6502_eor_imm(){
  cycles = 2;

  am_immediate();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x45
 address-mode: zp
 function: A:=A exor {adr} 
 mnemonic: EOR zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_eor_zp(){
  cycles = 3;

  am_zp();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x55
 address-mode: zpx
 function: A:=A exor {adr} 
 mnemonic: EOR zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_eor_zpx (){
  cycles = 4;

  am_zpx();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x41
 address-mode: izx
 function: A:=A exor {adr} 
 mnemonic: EOR X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_eor_izx(){
  cycles = 6;

  am_izx();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x51
 address-mode: izy
 function: A:=A exor {adr} 
 mnemonic: EOR ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_eor_izy(){
  cycles = 5;

  am_izy();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x4D
 address-mode: abs
 function: A:=A exor {adr} 
 mnemonic: EOR abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_eor_abs(){
  cycles = 4;

  am_abs();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x5D
 address-mode: abx
 function: A:=A exor {adr} 
 mnemonic: EOR abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_eor_abx (){
  cycles = 4;

  am_abx();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#EOR )
 name: EOR
 code: 0x59
 address-mode: aby
 function: A:=A exor {adr} 
 mnemonic: EOR abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_eor_aby(){
  cycles = 4;

  am_aby();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_XOR, dbr, acc, acc, flags);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x69
 address-mode: imm
 function: A:=A+{adr} 
 mnemonic: ADC #
 bytes: 2
 cycles: 2
*/
void cpu_6502_adc_imm(){
  cycles = 2;

  am_immediate();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x65
 address-mode: zp
 function: A:=A+{adr} 
 mnemonic: ADC zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_adc_zp(){
  cycles = 3;

  am_zp();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x75
 address-mode: zpx
 function: A:=A+{adr} 
 mnemonic: ADC zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_adc_zpx (){
  cycles = 4;

  am_zpx();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x61
 address-mode: izx
 function: A:=A+{adr} 
 mnemonic: ADC X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_adc_izx(){
  cycles = 6;
  am_izx();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x71
 address-mode: izy
 function: A:=A+{adr} 
 mnemonic: ADC ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_adc_izy(){
  cycles = 5;
  am_izy();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x6D
 address-mode: abs
 function: A:=A+{adr} 
 mnemonic: ADC abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_adc_abs(){
  cycles = 4;
  am_abs();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x7D
 address-mode: abx
 function: A:=A+{adr} 
 mnemonic: ADC abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_adc_abx (){
  cycles = 4;
  am_abx();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ADC )
 name: ADC
 code: 0x79
 address-mode: aby
 function: A:=A+{adr} 
 mnemonic: ADC abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_adc_aby(){
  cycles = 4;
  am_aby();

  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ADD_WITH_CARRY, dbr, acc, acc, flags);
  
  zsflagging(flags,acc);

  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xE9
 address-mode: imm
 function: A:=A-{adr} 
 mnemonic: SBC #
 bytes: 2
 cycles: 2
*/
void cpu_6502_sbc_imm(){
  cycles = 2;

  am_immediate();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xE5
 address-mode: zp
 function: A:=A-{adr} 
 mnemonic: SBC zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_sbc_zp(){
  cycles = 3;
  am_zp();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xF5
 address-mode: zpx
 function: A:=A-{adr} 
 mnemonic: SBC zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_sbc_zpx (){
  cycles = 4;
  am_zpx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xE1
 address-mode: izx
 function: A:=A-{adr} 
 mnemonic: SBC X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_sbc_izx(){
  cycles = 6;
  am_izx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xF1
 address-mode: izy
 function: A:=A-{adr} 
 mnemonic: SBC ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_sbc_izy(){
  cycles = 5;
  am_izy();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xED
 address-mode: abs
 function: A:=A-{adr} 
 mnemonic: SBC abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_sbc_abs(){
  cycles = 4;
  am_abs();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xFD
 address-mode: abx
 function: A:=A-{adr} 
 mnemonic: SBC abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_sbc_abx (){
  cycles = 4;
  am_abx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#SBC )
 name: SBC
 code: 0xF9
 address-mode: aby
 function: A:=A-{adr} 
 mnemonic: SBC abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_sbc_aby(){
  cycles = 4;
  am_aby();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_SUB_WITH_CARRY, acc, dbr, acc, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, acc);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xC9
 address-mode: imm
 function: A-{adr} 
 mnemonic: CMP #
 bytes: 2
 cycles: 2
*/
void cpu_6502_cmp_imm(){
  cycles = 2;

  am_immediate();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}



/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xC5
 address-mode: zp
 function: A-{adr} 
 mnemonic: CMP zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_cmp_zp(){
  cycles = 3;
  am_zp();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xD5
 address-mode: zpx
 function: A-{adr} 
 mnemonic: CMP zpg,X
 bytes: 2
 cycles: 4
*/
void cpu_6502_cmp_zpx (){
  cycles = 4;
  
  am_zpx();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xC1
 address-mode: izx
 function: A-{adr} 
 mnemonic: CMP X,ind
 bytes: 2
 cycles: 6
*/
void cpu_6502_cmp_izx(){
  cycles = 6;
  
  am_zpx();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xD1
 address-mode: izy
 function: A-{adr} 
 mnemonic: CMP ind,Y
 bytes: 2
 cycles: 5
*/
void cpu_6502_cmp_izy(){
  cycles = 5;
  
  am_izy();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xCD
 address-mode: abs
 function: A-{adr} 
 mnemonic: CMP abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_cmp_abs(){
  cycles = 4;
  
  am_abs();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xDD
 address-mode: abx
 function: A-{adr} 
 mnemonic: CMP abs,X
 bytes: 3
 cycles: 4
*/
void cpu_6502_cmp_abx (){
  cycles = 4;
  
  am_abx();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CMP )
 name: CMP
 code: 0xD9
 address-mode: aby
 function: A-{adr} 
 mnemonic: CMP abs,Y
 bytes: 3
 cycles: 4
*/
void cpu_6502_cmp_aby(){
  cycles = 4;
  
  am_aby();
  
  set_rw2read();
  access_memory();

  cmp(acc, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CPX )
 name: CPX
 code: 0xE0
 address-mode: imm
 function: X-{adr} 
 mnemonic: CPX #
 bytes: 2
 cycles: 2
*/
void cpu_6502_cpx_imm(){
  cycles = 2;

  am_immediate();
  
  set_rw2read();
  access_memory();

  cmp(idx, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CPX )
 name: CPX
 code: 0xE4
 address-mode: zp
 function: X-{adr} 
 mnemonic: CPX zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_cpx_zp(){
  cycles = 3;
  am_zp();
  
  set_rw2read();
  access_memory();

  cmp(idx, dbr, flags);
  
  inc_pc();  
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CPX )
 name: CPX
 code: 0xEC
 address-mode: abs
 function: X-{adr} 
 mnemonic: CPX abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_cpx_abs(){
  cycles = 4;

  am_abs();
  
  set_rw2read();
  access_memory();

  cmp(idx, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CPY )
 name: CPY
 code: 0xC0
 address-mode: imm
 function: Y-{adr} 
 mnemonic: CPY #
 bytes: 2
 cycles: 2
*/
void cpu_6502_cpy_imm(){
  cycles = 2;
  am_immediate();
  
  set_rw2read();
  access_memory();

  cmp(idy, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CPY )
 name: CPY
 code: 0xC4
 address-mode: zp
 function: Y-{adr} 
 mnemonic: CPY zpg
 bytes: 2
 cycles: 3
*/
void cpu_6502_cpy_zp(){
  cycles = 3;
  am_zp();
  
  set_rw2read();
  access_memory();

  cmp(idy, dbr, flags);
  
  inc_pc();  

}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#CPY )
 name: CPY
 code: 0xCC
 address-mode: abs
 function: Y-{adr} 
 mnemonic: CPY abs
 bytes: 3
 cycles: 4
*/
void cpu_6502_cpy_abs(){
  cycles = 4;
  
  am_abs();
  
  set_rw2read();
  access_memory();

  cmp(idy, dbr, flags);
  
  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#DEC )
 name: DEC
 code: 0xC6
 address-mode: zp
 function: {adr}:={adr}-1 
 mnemonic: DEC zpg
 bytes: 2
 cycles: 5
*/
void cpu_6502_dec_zp(){
  cycles = 5;
  
  am_zp();
  
  set_rw2read();
  access_memory();
  
  dec_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#DEC )
 name: DEC
 code: 0xD6
 address-mode: zpx
 function: {adr}:={adr}-1 
 mnemonic: DEC zpg,X
 bytes: 2
 cycles: 6
*/
void cpu_6502_dec_zpx (){
  cycles = 6;
  
  am_zpx();
  
  set_rw2read();
  access_memory();
  
  dec_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#DEC )
 name: DEC
 code: 0xCE
 address-mode: abs
 function: {adr}:={adr}-1 
 mnemonic: DEC abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_dec_abs(){
  cycles = 6;

  am_abs();
  
  set_rw2read();
  access_memory();
  
  dec_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#DEC )
 name: DEC
 code: 0xDE
 address-mode: abx
 function: {adr}:={adr}-1 
 mnemonic: DEC abs,X
 bytes: 3
 cycles: 7
*/
void cpu_6502_dec_abx (){
  cycles = 7;

  am_abx();
  
  set_rw2read();
  access_memory();
  
  dec_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#DEX )
 name: DEX
 code: 0xCA
 address-mode: imp
 function: X:=X-1 
 mnemonic: DEX impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_dex_imp(){
  cycles = 2;
  
  dec_register(idx);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#DEY )
 name: DEY
 code: 0x88
 address-mode: imp
 function: Y:=Y-1 
 mnemonic: DEY impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_dey_imp(){
  cycles = 2;
  dec_register(idy);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#INC )
 name: INC
 code: 0xE6
 address-mode: zp
 function: {adr}:={adr}+1 
 mnemonic: INC zpg
 bytes: 2
 cycles: 5
*/
void cpu_6502_inc_zp(){
  cycles = 5;
  
  am_zp();
  
  set_rw2read();
  access_memory();
  
  inc_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#INC )
 name: INC
 code: 0xF6
 address-mode: zpx
 function: {adr}:={adr}+1 
 mnemonic: INC zpg,X
 bytes: 2
 cycles: 6
*/
void cpu_6502_inc_zpx (){
  cycles = 6;
  
  am_zpx();
  
  set_rw2read();
  access_memory();
  
  inc_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#INC )
 name: INC
 code: 0xEE
 address-mode: abs
 function: {adr}:={adr}+1 
 mnemonic: INC abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_inc_abs(){
  cycles = 6;
  
  am_abs();
  
  set_rw2read();
  access_memory();
  
  inc_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#INC )
 name: INC
 code: 0xFE
 address-mode: abx
 function: {adr}:={adr}+1 
 mnemonic: INC abs,X
 bytes: 3
 cycles: 7
*/
void cpu_6502_inc_abx (){
  cycles = 7;
  
  am_abx();
  
  set_rw2read();
  access_memory();
  
  inc_register(dbr);

  set_rw2write();
  access_memory();

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#INX )
 name: INX
 code: 0xE8
 address-mode: imp
 function: X:=X+1 
 mnemonic: INX impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_inx_imp(){
  cycles = 2;
  inc_register(idx);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#INY )
 name: INY
 code: 0xC8
 address-mode: imp
 function: Y:=Y+1 
 mnemonic: INY impl
 bytes: 1
 cycles: 2
*/
void cpu_6502_iny_imp(){
  cycles = 2;
  inc_register(idy);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ASL )
 name: ASL
 code: 0x0A
 address-mode: imp
 function: {adr}:={adr}*2 
 mnemonic: ASL A
 bytes: 1
 cycles: 2
*/
void cpu_6502_asl_imp(){
  cycles = 2;
  
  alu(ALU_OP_ASL, acc, acc, acc, flags);
  
  zsflagging(acc, flags);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ASL )
 name: ASL
 code: 0x06
 address-mode: zp
 function: {adr}:={adr}*2 
 mnemonic: ASL zpg
 bytes: 2
 cycles: 5
*/
void cpu_6502_asl_zp(){
  cycles = 5;
  
  am_zp();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ASL, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ASL )
 name: ASL
 code: 0x16
 address-mode: zpx
 function: {adr}:={adr}*2 
 mnemonic: ASL zpg,X
 bytes: 2
 cycles: 6
*/
void cpu_6502_asl_zpx (){
  cycles = 6;
  
  am_zpx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ASL, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ASL )
 name: ASL
 code: 0x0E
 address-mode: abs
 function: {adr}:={adr}*2 
 mnemonic: ASL abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_asl_abs(){
  cycles = 6;
  am_abs();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ASL, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ASL )
 name: ASL
 code: 0x1E
 address-mode: abx
 function: {adr}:={adr}*2 
 mnemonic: ASL abs,X
 bytes: 3
 cycles: 7
*/
void cpu_6502_asl_abx (){
  cycles = 7;
  am_abx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ASL, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROL )
 name: ROL
 code: 0x2A
 address-mode: imp
 function: {adr}:={adr}*2+C 
 mnemonic: ROL A
 bytes: 1
 cycles: 2
*/
void cpu_6502_rol_imp(){
  cycles = 2;
  alu(ALU_OP_ROL,acc,NULL,acc,flags);
  zsflagging(flags, acc);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROL )
 name: ROL
 code: 0x26
 address-mode: zp
 function: {adr}:={adr}*2+C 
 mnemonic: ROL zpg
 bytes: 2
 cycles: 5
*/
void cpu_6502_rol_zp(){
  cycles = 5;
  
  am_zp();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROL )
 name: ROL
 code: 0x36
 address-mode: zpx
 function: {adr}:={adr}*2+C 
 mnemonic: ROL zpg,X
 bytes: 2
 cycles: 6
*/
void cpu_6502_rol_zpx (){
  cycles = 6;

  am_zpx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROL )
 name: ROL
 code: 0x2E
 address-mode: abs
 function: {adr}:={adr}*2+C 
 mnemonic: ROL abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_rol_abs(){
  cycles = 6;
  am_abs();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROL )
 name: ROL
 code: 0x3E
 address-mode: abx
 function: {adr}:={adr}*2+C 
 mnemonic: ROL abs,X
 bytes: 3
 cycles: 7
*/
void cpu_6502_rol_abx (){
  cycles = 7;
  am_abx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LSR )
 name: LSR
 code: 0x4A
 address-mode: imp
 function: {adr}:={adr}/2 
 mnemonic: LSR A
 bytes: 1
 cycles: 2
*/
void cpu_6502_lsr_imp(){
  cycles = 2;
  alu(ALU_OP_LSR,acc,NULL,acc,flags);
  zsflagging(flags, acc);
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LSR )
 name: LSR
 code: 0x46
 address-mode: zp
 function: {adr}:={adr}/2 
 mnemonic: LSR zpg
 bytes: 2
 cycles: 5
*/
void cpu_6502_lsr_zp(){
  cycles = 5;
  am_zp();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_LSR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LSR )
 name: LSR
 code: 0x56
 address-mode: zpx
 function: {adr}:={adr}/2 
 mnemonic: LSR zpg,X
 bytes: 2
 cycles: 6
*/
void cpu_6502_lsr_zpx (){
  cycles = 6;
  am_zpx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_LSR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LSR )
 name: LSR
 code: 0x4E
 address-mode: abs
 function: {adr}:={adr}/2 
 mnemonic: LSR abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_lsr_abs(){
  cycles = 6;
  am_abs();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_LSR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#LSR )
 name: LSR
 code: 0x5E
 address-mode: abx
 function: {adr}:={adr}/2 
 mnemonic: LSR abs,X
 bytes: 3
 cycles: 7
*/
void cpu_6502_lsr_abx (){
  cycles = 7;
  am_abx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_LSR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROR )
 name: ROR
 code: 0x6A
 address-mode: imp
 function: {adr}:={adr}/2+C*128 
 mnemonic: ROR A
 bytes: 1
 cycles: 2
*/
void cpu_6502_ror_imp(){
  cycles = 2;
  
  alu(ALU_OP_ROR, acc, "00000000", acc, flags);
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROR )
 name: ROR
 code: 0x66
 address-mode: zp
 function: {adr}:={adr}/2+C*128 
 mnemonic: ROR zpg
 bytes: 2
 cycles: 5
*/
void cpu_6502_ror_zp(){
  cycles = 5;
  am_zp();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROR )
 name: ROR
 code: 0x76
 address-mode: zpx
 function: {adr}:={adr}/2+C*128 
 mnemonic: ROR zpg,X
 bytes: 2
 cycles: 6
*/
void cpu_6502_ror_zpx (){
  cycles = 6;
  am_zpx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROR )
 name: ROR
 code: 0x6E
 address-mode: abs
 function: {adr}:={adr}/2+C*128 
 mnemonic: ROR abs
 bytes: 3
 cycles: 6
*/
void cpu_6502_ror_abs(){
  cycles = 6;
  am_abs();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


/*
 6502 mu-function implementation ( file:///home/olivier/6502.html#ROR )
 name: ROR
 code: 0x7E
 address-mode: abx
 function: {adr}:={adr}/2+C*128 
 mnemonic: ROR abs,X
 bytes: 3
 cycles: 7
*/
void cpu_6502_ror_abx (){
  cycles = 7;
  am_abx();
  
  set_rw2read();
  access_memory();
  
  alu(ALU_OP_ROR, dbr, "00000000", dbr, flags);
  
  set_rw2write();
  access_memory();
  
  zsflagging(flags, dbr);

  inc_pc();
}


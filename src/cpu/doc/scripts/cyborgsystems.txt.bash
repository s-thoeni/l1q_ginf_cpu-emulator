#!/bin/bash
SOURCE=../cyborgsystems.txt
awk -e 'BEGIN {
split("ADC AND ASL BCC BCS BEQ BIT BMI BNE BPL BRK BVC BVS CLC CLD CLI CLV CMP CPX CPY DEC DEX DEY EOR INC INX INY JMP JMP JSR LDA LDX LDY LSR NOP ORA PHA PHP PLA PLP ROL ROR RTI RTS SBC SEC SED SEI STA STX STY TAX TAY TSX TXA TXS TYA",ops,/ /);
}
#ORA EOR 
/Logic:/ {logic=1}
/Notes:/ {logic=0}
#/(^ADC)|(^AND)|(^ASL)|(^BCC)|(^BCS)|(^BEQ)|(^BIT)|(^BMI)|(^BNE)|(^BPL)|(^BRK)|(^BVC)|(^BVS)|(^CLC)|(^CLD)|(^CLI)|(^CLV)|(^CMP)|(^CPX)|(^CPY)|(^DEC)|(^DEX)|(^DEY)|(^EOR)|(^INC)|(^INX)|(^INY)|(^JMP)|(^JMP)|(^JSR)|(^LDA)|(^LDX)|(^LDY)|(^LSR)|(^NOP)|(^ORA)|(^PHA)|(^PHP)|(^PLA)|(^PLP)|(^ROL)|(^ROR)|(^RTI)|(^RTS)|(^SBC)|(^SEC)|(^SED)|(^SEI)|(^STA)|(^STX)|(^STY)|(^TAX)|(^TAY)|(^TSX)|(^TXA)|(^TXS)|(^TYA)/ 

#/^ADC/||/^AND/||/^ASL/||/^BCC/||/^BCS/||/^BEQ/||/^BIT/||/^BMI/||/^BNE/||/^BPL/||/^BRK/||/^BVC/||/^BVS/||/^CLC/||/^CLD/||/^CLI/||/^CLV/||/^CMP/||/^CPX/||/^CPY/||/^DEC/||/^DEX/||/^DEY/||/^EOR/||/^INC/||/^INX/||/^INY/||/^JMP/||/^JMP/||/^JSR/||/^LDA/||/^LDX/||/^LDY/||/^LSR/||/^NOP/||/^ORA/||/^PHA/||/^PHP/||/^PLA/||/^PLP/||/^ROL/||/^ROR/||/^RTI/||/^RTS/||/^SBC/||/^SEC/||/^SED/||/^SEI/||/^STA/||/^STX/||/^STY/||/^TAX/||/^TAY/||/^TSX/||/^TXA/||/^TXS/||/^TYA/ 
#{
#  print "touchy " $0 #if($1 in ops) { print "LQLQL " $0}
#}
!/(^Logic:)|(^Notes:)/ { 
for (id in ops) {
   if($1==ops[id]) {
      if(opdummy != $1){ 
         opdummy = $1
         print $1
      }
   }
}

if(logic) { print $0}
}


' < $SOURCE 

# prints something like

# ADC
#   t = A + M + P.C
#   P.V = (A.7!=t.7) ? 1:0
#   P.N = A.7
#   P.Z = (t==0) ? 1:0
#   IF (P.D)
#     t = bcd(A) + bcd(M) + P.C
#     P.C = (t>99) ? 1:0
#   ELSE
#     P.C = (t>255) ? 1:0
#   A = t & 0xFF                
# AND
#   A = A & M
#   P.N = A.7
#   P.Z = (A==0) ? 1:0 

# the line 
awk -e '! /^ /{ print str; str = $1 " @ "} /^ /{ if(str){ str = str " " $0 " @ " } }' < cyborgsystems.opcode-logic-table.txt >cyborgsystems.opcode-logic-table-clean.txt 

# gives
#ADC   t = A + M + P.C\n   P.V = (A.7!=t.7) ? 1:0\n   P.N = A.7\n   P.Z = (t==0) ? 1:0\n   IF (P.D)\n     t = bcd(A) + bcd(M) + P.C\n     P.C = (t>99) ? 1:0\n   ELSE\n     P.C = (t>255) ? 1:0\n   A = t & 0xFF                \n
#AND   A = A & M\n   P.N = A.7\n   P.Z = (A==0) ? 1:0 \n

#
# mergin tables
# cyborgsystems.opcode-logic-table-clean.txt 
# ../opcode_table_bigA.txt  
awk -F"@"  'NR==FNR { h[$1] = $0; next} { print $0 " @ " h[$1]} '  cyborgsystems.opcode-logic-table-clean.txt ../opcode_table_bigA.txt  | head

# into

# BRK @ 0x00 @ cpu_6502_brk_imp @ imp @ interrupt @ BRK impl @ (S)-=:PC,P PC:=($FFFE) @ --1-1-- XX BRK @    PC = PC + 1 @    bPoke(SP,PC.h) @    SP = SP - 1 @    bPoke(SP,PC.l) @    SP = SP - 1 @    bPoke(SP, (P|$10) ) @    SP = SP - 1 @    l = bPeek($FFFE) @    h = bPeek($FFFF)<<8 @    PC = h|l              @ 
# ORA @ 0x01 @ cpu_6502_ora_izx @ izx @ or with accumulator @ ORA X,ind @ A:=A or {adr} @ *----*- XX ORA @    A = A | M @    P.N = A.7 @    P.Z = (A==0) ? 1:0  @ 

#
# merging next two tables
# ../opcode_table_bigAC.txt 
# ../opcode_table_bigB.txt
awk -F" @ "  'NR==FNR { h[$2] = $0; next} { id=$1; print $0 " @ " h[id]} ' ../opcode_table_bigAC.txt ../opcode_table_bigB.txt  > ../opcode_table_bigABC

# This line 
#./milk-obelisk.bash | awk -F"|" '$1!=""' | awk -F"|" 'BEGIN {while(( getline <"laa-command.table") > 0 ) {split($0, arr,"|")}} { for(i=0;i<255;i++){if($1==arr[1]){print arr[2]"|", $0,"\n" }}}'

# prints something like
#| 7E | ROR|ROR - Rotate Right|Move each of the bits in either A or M one place to the right. Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.|Processor Status after use:|C	Carry Flag	Set to contents of old bit 0|Z	Zero Flag	Set if A = 0|I	Interrupt Disable	Not affected|D	Decimal Mode Flag	Not affected|B	Break Command	Not affected|V	Overflow Flag	Not affected|N	Negative Flag	Set if bit 7 of the result is set|1|2|Zero Page	|$66|2|5|Zero Page,X	|$76|2|6|Absolute	|$6E|3|6|Absolute,X	|$7E|3|7||||||||||||||||||||||||||||||||||||||||||||| |

#| 7E | ROR|ROR - Rotate Right|Move each of the bits in either A or M one place to the right. Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.|Processor Status after use:|C	Carry Flag	Set to contents of old bit 0|Z	Zero Flag	Set if A = 0|I	Interrupt Disable	Not affected|D	Decimal Mode Flag	Not affected|B	Break Command	Not affected|V	Overflow Flag	Not affected|N	Negative Flag	Set if bit 7 of the result is set|1|2|Zero Page	|$66|2|5|Zero Page,X	|$76|2|6|Absolute	|$6E|3|6|Absolute,X	|$7E|3|7||||||||||||||||||||||||||||||||||||||||||||| |



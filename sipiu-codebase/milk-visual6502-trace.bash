#!/bin/bash

FROMOP=$(mktemp /tmp/output.op.XXXXXXXXXX)
FROMSHORT=$(mktemp /tmp/output.short.XXXXXXXXXX)
FROMMELANGE=$(mktemp /tmp/output.melange.XXXXXXXXXX)


NAME="screenpattern"
URL="http://visual6502.org/JSSim/expert.html?nosim=t&panx=300.0&pany=300.0&zoom=1.0&a=00&d=200006&a=0600&d=a000a5feaaa5ff488d0005186a6a6a6a8d010568290f85018a2a2a2a2a4501990002990003990004888c40054c0406488a48a2ffcaa5fed0fb68aa6860"

NAME="atari"
URL="http://visual6502.org/JSSim/expert.html?nosim=t&panx=300.0&pany=300.0&zoom=1.0&a=00&d=200006&a=0600&d=a95f8500a9068501a9008502a9028503a503c906d00160a000b100c9ffd01dc8b1008504c8b100a404a200810220540688d0f62049062049064c1006a20081022054062049064c1006e600a500c900d002e60160e602a602e000d002e60360ff2b010f0f0f0c0f0f0fff18010c0f0c000c0f0cff1801000f0c000c0f0cff18010c0f0c000c0f0cff1801000f0c000c0f0cff18010c0f00000c0f0cff1801000f0c000c0f0cff1801000f0c000c0f00ff1801000f0c000c0f00ff17010f000f0c000c0f000fff16010c00010c000c0f000cff15010f00000100000c010000ff15010c000c010c000c010c000cff13010f00000f010c000c010f00000fff11010f00000001010c000c0101000000ff10010f0000000f010100000c01010f0000000fff0d010c0000000c0101010c000c0101010c0000000cff0a010c000000000c0101010100000c0101010100000000000cff0801000000000c010101010100000c01010101010c0000000001010101010101010100000c010101010101010c00"

milk_trace_correct_flags(){

#tr '‑' '-' | 
tac $NAME/$NAME.visual6502.trace  | sort -k1,1 -n  -u | awk -F"\t" '# BEGIN {OFS = FS} \
{ 
# print "pc:", $6,"acc:",$7,"x",$8,"y",$9,"flags",$11,"ir",$14,"pd",$16 }
# print $6,$7,$8,$9,$10,$11,$14 

#if($5){ print $6,$7,$8,$9,$10,substr($11,1,2) "-" substr($11,6,5),$14 } 

if($5){ nr=NR; pc= $6; acc=$7; x=$8;y=$9;sp=$10;ir=$14 } 
if(NR==nr+1){ flags=substr($11,1,2) "-" substr($11,6,5); print pc, acc, x,y,sp,flags,ir } 

 }'
}
milk_trace_correct_flags(){
awk -F"\t" '# BEGIN {OFS = FS} \
{ 
# print "pc:", $6,"acc:",$7,"x",$8,"y",$9,"flags",$11,"ir",$14,"pd",$16 }
# print $6,$7,$8,$9,$10,$11,$14 

#if($5){ print $6,$7,$8,$9,$10,substr($11,1,2) "-" substr($11,6,5),$14 } 

if($5){ nr=NR; pc= $6; sp=$10;ir=$14 } 
if(NR==nr+1){ acc=$7; x=$8;y=$9; flags=substr($11,1,2) "-" substr($11,6,5); 
print pc, acc, x,y,sp,flags,ir } 

 }' $NAME/$NAME.visual6502.trace 
}
milk_trace_correct_flags
#milk_trace 

#
# output:
#
# FE|INC|increment|INC abs,X

milk_opcodetbl2(){

awk -F"\t" 'BEGIN { while (getline < "named-commands.txt") { ass[$1]= $3}} {for( i=2; i<17; i++){ name = substr($i,1,3); if(name!~"\?\?\?"){ printf("\"%02X\"|%s|%s|%s\n",(NR-1)*16+i-2, name, ass[name],$i);}}}' doc/opcode-table2.txt 
}


#printd ()  # This works.
#{
#     milk_commands $1 #  milk_opcodetbl2
 #} < $FROMOP | milk_commands $1 #laa-commands.txt
#
  #   milk_commands $1
#    milk_opcodetbl2 $1
#}

#printd $1 # "laa-commands.txt"

#
# output:
#
# 7E |ROR|abs|*    **|{adr}:={adr}/2+C*128
milk_commands (){
# $1 being one from jmp-commands.txt, move-commands.txt,laa-commands.txt

  awk -F"\t" 'BEGIN { OFS="|"; 
         split("imp imm zp zpx zpy izx izy abs abx aby ind rel", adrmodes, " ");
} 

{    
for( i=2; i<14; i++){ 
   if($i~"^\$"){
#printf("ONE: %s desc: %s opcode: %s adrm: %s action: %s flags: %s%s%s%s%s%s%s\n", $1, file1[$1], $i, adrmodes[i], $14 , $15,$16,$17,$18,$19,$20,$21,$22);}
#       printf("TWO: %s|%02X|%s| id: %02X NR: %s i: %s XX %s%s%s%s%s%s%s|%s\n", $i, substr($i,2,3), $i, (NR-4)*16+i-2 , NR-4, i-2,$16,$17,$18,$19,$20,$21,$22,file1[$1]);
       printf("\"%s\"|%s|%s|%s%s%s%s%s%s%s|%s\n", substr($i,2,2), $1, adrmodes[i-1],$15,$16,$17,$18,$19,$20,$21,$14);
#  printf("TRHEE: %s\n", $0);
  }
   }
}
'  $1 
}

#!/bin/bash
# alife, atari, color, brick, sierpinski, demoscene, disco, c64logo, screenpattern, spacer

NAME=$1
FROMSHORT=$(mktemp /tmp/output.short.XXXXXXXXXX)
FROMMELANGE=$(mktemp /tmp/output.melange.XXXXXXXXXX)


echo -e "00 20\n01 00\n02 06\n" >$NAME/$NAME.mem 
awk 'BEGIN {lin=1536;} {i=lin;  for(b=2;b<=NF;b++){ printf("%04x %s \n%04x %s\n", i++, substr($b,1,2), i++, substr($b,3,2))} ; lin=i }'  $NAME/$NAME.dump >> $NAME/$NAME.mem



# 0600 based http://www.e-tradition.net/bytes/6502/assembler.html
# code for disassembler
awk 'NR>3{print $2}' $NAME/$NAME.mem > $NAME/$NAME.dis.mem



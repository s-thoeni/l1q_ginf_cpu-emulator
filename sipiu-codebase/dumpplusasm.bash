#!/bin/bash
INPUTFILE="inputfile.csv"

awk 'BEGIN {
while (getline < "'"$INPUTFILE"'")
{
split($0,line," ");
adr=line[1];
opcode[1]=line[2];
opcode[2]=line[3];
opcode[3]=line[4];
asm=line[4];
arg=line[4];
}
close("'"$INPUTFILE"'");
}'
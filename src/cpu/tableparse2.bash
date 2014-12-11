#!/bin/bash

awk -F"|" 'NR==FNR{ a[$1]=$2; next } NR!=FNR{ opcode[$3]=$1; ass[$3]=$2; lines[$3]=$0; } 
END {  
for(line in lines) { 
printf("%s %s %s %s\n", line, ass[line], a[line], opcode[line]);
}
} ' /tmp/named-tbl.txt /tmp/tbl2.txt  

#for(x in a) { 
#printf("x= %s line= %s a[line]= %s lines[line]= %s\n", x, line, a[line], lines[line]);
#}
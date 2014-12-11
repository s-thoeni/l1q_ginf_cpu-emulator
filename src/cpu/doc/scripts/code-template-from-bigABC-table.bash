#!/bin/bash
SOURCE=../opcode_table_bigABC

# generate c code template for mu-code.c
# to activate remove redirection >/dev/null at end of awk script
awk -F"@" -e '$4!~/CRASH/ && $4!~/\*../{ opcode = $1; oplength=$2; opcycles=$3; opname=$4; opadrmode=$5; opadrmodeid=$6; cname=$10; shorttxt=$12; formaltxt=$14; flags=$15; a=0; for(i=17; i<=NF; i++){ logictxt[a++]=$i}
switch(opadrmode){
case " imm " : {
  adrmodetxt = "immediate -- data immediate after opcode; data = mem[PC+1]"
  exampletxt = "#$AB    "
  break;
}
case " ind " : {
  adrmodetxt = "indirect; indirection via zp address; ea = mem[zpadr+1]|mem[zpadr]"
  exampletxt = "$ABCD"
  break;
}
case " abs " : {
  adrmodetxt = "absolute 16 bit address; data = mem[abs_high|abs_low]"
  exampletxt = "$ABCD"
  break;
}
case " abx " : {
  adrmodetxt = "absolute indexed addressing -- absolute 16 bit address with index X; ea = abs+X"
  exampletxt = "$ABCD,X"
  break;
}
case " aby " : {
  adrmodetxt = "absolute 16 bit address with index Y; ea = abs+Y"
  exampletxt = "$ABCD,Y"
  break;
}
case " izx " : {
  adrmodetxt = "indirect X-indexed zero page; ea = mem[zpadr+X|zpadr+X+1]"
  exampletxt = "($A5,X)"
  break;
}
case " izy " : {
  adrmodetxt = "indirect Y-indexed zero page; ea = mem[zpadr|zpadr+1]+Y"
  exampletxt = "($A5),X"
  break;
}
case " imp " : {
  adrmodetxt = "implizit; no additional info needed"
  exampletxt = ""
  break;
}
case " rel " : {
  adrmodetxt = "relativ; address is relativ with 8-bit signed offset to PC; adr=pc+offset"
  exampletxt = "$AB"
  break;
}
case " zp " : {
  adrmodetxt = "zero page ; ea = zpadr"
  break;
}
case " zpx " : {
  adrmodetxt = "zero page with index register X; ea = zpadr+X"
  exampletxt = "$AB,X"
  break;
}
case " zpy " : {
  adrmodetxt = "zero page with index register Y; ea = zpadr+Y"
  exampletxt = "$AB,X"
  break;
}
default: {
  adrmodetxt = "nix found"
  break;
}
}
printf("/***************************************************************\n ***************************************************************\n mnemonic: %s\n code:      %s\n bytes:    %s\n cycles:   %s\n mode:     %s %s\n function: %s \n formal:   %s\n            NVBDIZC\n flags:    %s\n", opname, opcode, oplength, opcycles, opadrmode, adrmodetxt, shorttxt, formaltxt,flags)
printf(" logic:\n")
for(i in logictxt) {
  printf("%s\n", logictxt[i])
}
printf(" example: %s %s\n", opname, exampletxt);
printf("***************************************************************\n***************************************************************/\n\n\
void %s() {\n\
\
\n}\n\n\n", cname)
}' < $SOURCE >/dev/null



# generate latex code template for mu-code.tex
# tabular form with &&&&&


cat $SOURCE  | sed -e 's/\&/\\\&/g' | awk -F"@" -e '$4!~/CRASH/ && $4!~/\*../{ opcode = $1; oplength=$2; opcycles=$3; opname=$4; opadrmode=$5; opadrmodeid=$6; cname=$10; shorttxt=$12; formaltxt=$14; flags=$15; a=0; for(i=17; i<=NF; i++){ logictxt[a++]=$i}
lines = i-17;
switch(opadrmode){
case " imm " : {
  adrmodetxt = "immediate -- data immediate after opcode; data = mem[PC+1]"
  exampletxt = "#$AB    "
  break;
}
case " ind " : {
  adrmodetxt = "indirect; indirection via zp address; ea = mem[zpadr+1]|mem[zpadr]"
  exampletxt = "$ABCD"
  break;
}
case " abs " : {
  adrmodetxt = "absolute 16 bit address; data = mem[abs_high|abs_low]"
  exampletxt = "$ABCD"
  break;
}
case " abx " : {
  adrmodetxt = "absolute indexed addressing -- absolute 16 bit address with index X; ea = abs+X"
  exampletxt = "$ABCD,X"
  break;
}
case " aby " : {
  adrmodetxt = "absolute 16 bit address with index Y; ea = abs+Y"
  exampletxt = "$ABCD,Y"
  break;
}
case " izx " : {
  adrmodetxt = "indirect X-indexed zero page; ea = mem[zpadr+X|zpadr+X+1]"
  exampletxt = "($A5,X)"
  break;
}
case " izy " : {
  adrmodetxt = "indirect Y-indexed zero page; ea = mem[zpadr|zpadr+1]+Y"
  exampletxt = "($A5),X"
  break;
}
case " imp " : {
  adrmodetxt = "implizit; no additional info needed"
  exampletxt = ""
  break;
}
case " rel " : {
  adrmodetxt = "relativ; address is relativ with 8-bit signed offset to PC; adr=pc+offset"
  exampletxt = "$AB"
  break;
}
case " zp " : {
  adrmodetxt = "zero page ; ea = zpadr"
  exampletxt = "$AB"
  break;
}
case " zpx " : {
  adrmodetxt = "zero page with index register X; ea = zpadr+X"
  exampletxt = "$AB,X"
  break;
}
case " zpy " : {
  adrmodetxt = "zero page with index register Y; ea = zpadr+Y"
  exampletxt = "$AB,X"
  break;
}
default: {
  adrmodetxt = "nix found"
  break;
}
}
flagstxt="\\flags{NVBDIZC}{"flags"}"
printf("%s & %s & %s & %s & %s & %s \\newline %s & %s & ", opcode, opname, substr(oplength,4), substr(opcycles,4), opadrmode, shorttxt, formaltxt,flagstxt)
# logic
printf("\\begin{lstlisting}")
for(i=0;i<lines;i++) {
  printf("%s\n", logictxt[i])
}
printf("\\end{lstlisting} & ")

printf(" \\textbf{\\texttt{%s %s}} \\\\\n", opname, exampletxt);

}'  | sed -e 's/\$/\\\$/g' >/dev/null




# generate latex code template for mu-code.tex
# tikz form

cat $SOURCE  | sed -e 's/\$/\\\$/g' | awk -F"@" -e '$4!~/CRASH/ && $4!~/\*../{ opcode = $1; oplength=$2; opcycles=$3; opname=$4; opadrmode=$5; opadrmodeid=$6; cname=$10; shorttxt=$12; formaltxt=$14; flags=$15; a=0; for(i=17; i<=NF; i++){ logictxt[a++]=$i}
lines = i-17;
switch(opadrmode){
case " imm " : {
  adrmodetxt = "immediate -- data immediate after opcode; data = mem[PC+1]"
  exampletxt = "\\#\\$AB    "
  break;
}
case " ind " : {
  adrmodetxt = "indirect; indirection via zp address; ea = mem[zpadr+1]|mem[zpadr]"
  exampletxt = "\\$ABCD"
  break;
}
case " abs " : {
  adrmodetxt = "absolute 16 bit address; data = mem[abs_high|abs_low]"
  exampletxt = "\\$ABCD"
  break;
}
case " abx " : {
  adrmodetxt = "absolute indexed addressing -- absolute 16 bit address with index X; ea = abs+X"
  exampletxt = "\\$ABCD,X"
  break;
}
case " aby " : {
  adrmodetxt = "absolute 16 bit address with index Y; ea = abs+Y"
  exampletxt = "\\$ABCD,Y"
  break;
}
case " izx " : {
  adrmodetxt = "indirect X-indexed zero page; ea = mem[zpadr+X|zpadr+X+1]"
  exampletxt = "(\\$A5,X)"
  break;
}
case " izy " : {
  adrmodetxt = "indirect Y-indexed zero page; ea = mem[zpadr|zpadr+1]+Y"
  exampletxt = "(\\$A5),X"
  break;
}
case " imp " : {
  adrmodetxt = "implizit; no additional info needed"
  exampletxt = ""
  break;
}
case " rel " : {
  adrmodetxt = "relativ; address is relativ with 8-bit signed offset to PC; adr=pc+offset"
  exampletxt = "\\$AB"
  break;
}
case " zp " : {
  adrmodetxt = "zero page ; ea = zpadr"
  exampletxt = "\\$AB"
  break;
}
case " zpx " : {
  adrmodetxt = "zero page with index register X; ea = zpadr+X"
  exampletxt = "\\$AB,X"
  break;
}
case " zpy " : {
  adrmodetxt = "zero page with index register Y; ea = zpadr+Y"
  exampletxt = "\\$AB,X"
  break;
}
default: {
  adrmodetxt = "nix found"
  break;
}
}

gsub("-","\\textendash",flags);
gsub(" ","",flags);
gsub(" ", "", opname)
gsub(" ", "", opadrmode)
gsub("\\&", "\\\\&", formaltxt)
flagstxt="\\flagss{NVBDIZC}{"flags"}"
printf("\\begin{tikzpicture}\\draw (0,4) rectangle node  {\\texttt %s  }  (1,5) ;\\draw (1,4) rectangle node {\\texttt  %s }  (2,5) ;\\draw (2,4) rectangle node {\\texttt %s  }  (3,5) ;\\draw (3,4) rectangle node[minimum width=2cm, text width=2cm,xshift=1em] { %s }  (5,5) ; \\draw (0,3) rectangle node {%s %s}  (3,4) ;  \\draw (3,3) rectangle node  { %s } (4,4)   ; \\draw (4,3) rectangle node  { %s } (5,4)   ;\\draw (0,2) rectangle node {  %s }  (5,3) ; \\draw (0,1) rectangle node {  %s }  (5,2) ;\\draw (0,0) rectangle node {\\textbf{\\texttt{void cpu\\_6502\\_%s\\_%s(); }}}  (5,1) ; \\node[below right] at (0,5) {\\scriptsize op} ;\\node[below right] at (1,5) {\\scriptsize name} ;\\node[below right] at (2,5) {\\scriptsize adr} ;\\node[below right] at (5,5) {\\scriptsize logic} ;\\node[below right] at (3,4) {\\scriptsize bytes} ; \\node[below right] at (4,4) {\\scriptsize cycles} ;\\node[below right] at (0,4) {\\scriptsize example}  ;\\node[below right] at (0,1) {\\scriptsize c--fct} ;\\node[below right] at (0,2) {\\scriptsize formal};\\node[below right] at (0,3) {\\scriptsize description};",opcode, opname, opadrmode, flagstxt, opname, exampletxt, substr(oplength,4), substr(opcycles,4), shorttxt, formaltxt, opname, opadrmode)

#\\begin{tikzpicture}\\draw[very thick] (4,0) rectangle node {}  (12,4) ;\\draw (0,4) rectangle node  {\\texttt %s }  (1,5) ;\\draw (1,4) rectangle node {\\texttt  %s }  (2,5) ;\\draw (2,4) rectangle node {\\texttt %s  }  (3,5) ;\\draw (3,4) rectangle node {\\texttt   %s }  (4,5) ;\\draw (4,4) rectangle node {\\texttt %s }  (5,5) ;\\draw (5,4) rectangle node { %s}  (12,5) ;\\draw (4,-1) rectangle node {\\textbf{\\texttt{void cpu\\_6502\\_%s\\_%s(); }}}  (12,0) ; \\draw (0,-1) rectangle node {\\textbf{\\texttt{   %s %s }}}  (4,0) ; \\draw (0,2) rectangle node[align=left]  { %s } (4,4)   ;\\draw (0,0) rectangle node[align=left,inner sep=0.5ex,text width=3cm]  { %s} (4,2)   ;\\node[below right] at (0,5) {\\scriptsize op} ;\\node[below right] at (1,5) {\\scriptsize name} ;\\node[below right] at (2,5) {\\scriptsize adr} ;\\node[below right] at (3,5) {\\scriptsize bytes} ;\\node[below right] at (5,5) {\\scriptsize function} ;\\node[below right] at (4,5) {\\scriptsize cycles} ;\\node[below right] at (0,0) {\\scriptsize example}  ; \\node[below right] at (4,0) {\\scriptsize c--fct}  ;\\node[below right] at (4,4) {\\scriptsize logic} ;\\node[below right] at (0,4) {\\scriptsize flags} ;\\node[below right] at (0,2) {\\scriptsize description} ;", opcode, opname, opadrmode, substr(oplength,4), substr(opcycles,4), formaltxt, opname, opadrmode, opname, exampletxt, flagstxt, shorttxt)
# cycles: substr(opcycles,4)
# logic

height=5-((lines)/3)
printf("\\draw (5,%s) rectangle node {}  (12,5) ; ",height)
printf("\\draw[thick] (5,0) rectangle node {}  (12,%s) ; \\node[below right] at (5,%s) {\\scriptsize comment};",height, height)
printf("\\node[anchor=north west]  at (5,5) {\n\\begin{lstlisting}\n")
for(i=0;i<lines;i++) {
  logictxtclean = logictxt[i]
# substitute \$ with $
  a="$"; gsub("\\\\\\$",a,logictxtclean)
  gsub(/[ \t]+$/,"",logictxtclean);
  printf("%s\n", logictxtclean)
}
printf("\\end{lstlisting}};\\end{tikzpicture}\n\n")

}'  
# substitute $ with \$
# sed -e 's/\$/\\\$/g' 
# substitute # with \#
# sed -e 's/#/\\#/g'
# substitute & with \&
# sed -e 's/\&/\\\&/g'

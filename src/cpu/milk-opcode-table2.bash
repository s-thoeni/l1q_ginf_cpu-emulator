#!/bin/bash
awk -F"\t" 'BEGIN { while (getline < "named-commands.txt") { ass[$1]= $3}} {for( i=2; i<17; i++){ name = substr($i,1,3); if(name!~"\?\?\?"){ printf("%2X|%s|%s|%s\n",(NR-1)*16+i-2, name, ass[name],$i);}}}' doc/opcode-table2.txt >/tmp/dele.tbl
# $1 being one from jmp-commands.txt, move-commands.txt,laa-commands.txt
#for i in (jmp-commands.txt, move-commands.txt,laa-commands.txt)
./milk-laa.bash "jmp-commands.txt" | awk -F"|" 'BEGIN { while (getline < "/tmp/dele.tbl") { ass[$1]= $4}}
{
 printf("\n{ \"%s\", 0x%02x, \"%s\", cpu_6502_%s_%s, \"%s\" ,\"%s\", \"%s\" ,\"%s\"} ,\n", $1, $2, $3,$1, $3, $4,$6,$5,ass[$2])
}'



#
# script generates this table:
# 
# 3d AND and (with accumulator) AND abs,X
# 3e ROL rotate left ROL abs,X
# 40 RTI return from interrupt RTI impl
# 41 EOR exclusive or (with accumulator) EOR X,ind
# 45 EOR exclusive or (with accumulator) EOR zpg
# 46 LSR logical shift right LSR zpg
# 48 PHA push accumulator PHA impl
# 49 EOR exclusive or (with accumulator) EOR #
# 4a LSR logical shift right LSR A
# 4c JMP jump JMP abs
# 4d EOR exclusive or (with accumulator) EOR abs
# 4e LSR logical shift right LSR abs
# 50 BVC branch on overflow clear BVC rel
# 51 EOR exclusive or (with accumulator) EOR ind,Y
# 55 EOR exclusive or (with accumulator) EOR zpg,X
# 56 LSR logical shift right LSR zpg,X
# 58 CLI clear interrupt disable CLI impl
# 59 EOR exclusive or (with accumulator) EOR abs,Y
# 5d EOR exclusive or (with accumulator) EOR abs,X
# 5e LSR logical shift right LSR abs,X
# 60 RTS return from subroutine RTS impl
# 61 ADC add with carry ADC X,ind
# 65 ADC add with carry ADC zpg
# 66 ROR rotate right ROR zpg
# 68 PLA pull accumulator PLA impl
# 69 ADC add with carry ADC #
# 6a ROR rotate right ROR A
# 6c JMP jump JMP ind
# 6d ADC add with carry ADC abs
# 6e ROR rotate right ROR abs
# 70 BVS branch on overflow set BVS rel
# 71 ADC add with carry ADC ind,Y
# 75 ADC add with carry ADC zpg,X
# 76 ROR rotate right ROR zpg,X
# 78 SEI set interrupt disable SEI impl
# 79 ADC add with carry ADC abs,Y
# 7d ADC add with carry ADC abs,X
# 7e ROR rotate right ROR abs,X
# 81 STA store accumulator STA X,ind
# 84 STY store Y STY zpg
# 85 STA store accumulator STA zpg
# 86 STX store X STX zpg
# 88 DEY decrement Y DEY impl
# 8a TXA transfer X to accumulator TXA impl
# 8c STY store Y STY abs
# 8d STA store accumulator STA abs
# 8e STX store X STX abs
# 90 BCC branch on carry clear BCC rel
# 91 STA store accumulator STA ind,Y
# 94 STY store Y STY zpg,X
# 95 STA store accumulator STA zpg,X
# 96 STX store X STX zpg,Y
# 98 TYA transfer Y to accumulator TYA impl
# 99 STA store accumulator STA abs,Y
# 9a TXS transfer X to stack pointer TXS impl
# 9d STA store accumulator STA abs,X
# a0 LDY load Y LDY #
# a1 LDA load accumulator LDA X,ind
# a2 LDX  LDX #
# a4 LDY load Y LDY zpg
# a5 LDA load accumulator LDA zpg
# a6 LDX  LDX zpg
# a8 TAY transfer accumulator to Y TAY impl
# a9 LDA load accumulator LDA #
# aa TAX transfer accumulator to X TAX impl
# ac LDY load Y LDY abs
# ad LDA load accumulator LDA abs
# ae LDX  LDX abs
# b0 BCS branch on carry set BCS rel
# b1 LDA load accumulator LDA ind,Y
# b4 LDY load Y LDY zpg,X
# b5 LDA load accumulator LDA zpg,X
# b6 LDX  LDX zpg,Y
# b8 CLV clear overflow CLV impl
# b9 LDA load accumulator LDA abs,Y
# ba TSX transfer stack pointer to X TSX impl
# bc LDY load Y LDY abs,X
# bd LDA load accumulator LDA abs,X
# be LDX  LDX abs,Y
# c0 CPY compare with Y CPY #
# c1 CMP compare (with accumulator) CMP X,ind
# c4 CPY compare with Y CPY zpg
# c5 CMP compare (with accumulator) CMP zpg
# c6 DEC decrement DEC zpg
# c8 INY increment Y INY impl
# c9 CMP compare (with accumulator) CMP #
# ca DEX decrement X DEX impl
# cc CPY compare with Y CPY abs
# cd CMP compare (with accumulator) CMP abs
# ce DEC decrement DEC abs
# d0 BNE branch on not equal (zero clear) BNE rel
# d1 CMP compare (with accumulator) CMP ind,Y
# d5 CMP compare (with accumulator) CMP zpg,X
# d6 DEC decrement DEC zpg,X
# d8 CLD clear decimal CLD impl
# d9 CMP compare (with accumulator) CMP abs,Y
# dd CMP compare (with accumulator) CMP abs,X
# de DEC decrement DEC abs,X
# e0 CPX compare with X CPX #
# e1 SBC subtract with carry SBC X,ind
# e4 CPX compare with X CPX zpg
# e5 SBC subtract with carry SBC zpg
# e6 INC increment INC zpg
# e8 INX increment X INX impl
# e9 SBC subtract with carry SBC #
# ea NOP no operation NOP impl
# ec CPX compare with X CPX abs
# ed SBC subtract with carry SBC abs
# ee INC increment INC abs
# f0 BEQ branch on equal (zero set) BEQ rel
# f1 SBC subtract with carry SBC ind,Y
# f5 SBC subtract with carry SBC zpg,X
# f6 INC increment INC zpg,X
# f8 SED set decimal SED impl
# f9 SBC subtract with carry SBC abs,Y
# fd SBC subtract with carry SBC abs,X
# fe INC increment INC abs,X
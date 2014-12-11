#!/bin/bash
# code table from 
# http://visual6502.org/wiki/index.php?title=6502_all_256_Opcodes
# produce c table with opid, bytes, cycles
awk -e ' BEGIN { 
                   adrmodes["imp"] = "ADRM_IMP"
                   adrmodes["imm"] = "ADRM_IMM"

                   adrmodes["rel"] = "ADRM_REL"

adrmodes["izx"] = "ADRM_IZX"
adrmodes["izy"] = "ADRM_IZY"

adrmodes["zp"] = "ADRM_ZP"
adrmodes["zpx"] = "ADRM_ZPX"
adrmodes["zpy"] = "ADRM_ZPY"

adrmodes["abs"] = "ADRM_ABS"
adrmodes["abx"] = "ADRM_ABX"
adrmodes["aby"] = "ADRM_ABY"
adrmodes["ind"] = "ADRM_IND"

                   print "/*\n    optable with opcode, bytes, cycles, adrm, opname, special"
                   print "    use opid as index\n"
                   print "    generated with gen-opcode.tabls_ext.bash\n"
                   print "*/\n"
                   printf("#define ADRM_IMP 1\n#define ADRM_IMM 2\n#define ADRM_REL 3\n#define ADRM_IZX 4\n#define ADRM_IZY 5\n#define ADRM_ZP 6\n#define ADRM_ZPX 7\n#define ADRM_ZPY 8\n#define ADRM_ABS 9\n#define ADRM_ABX 10\n#define ADRM_ABY 11\n#define ADRM_IND 12\n\n")
                   print " /*  opid, bytes, cycles, cmd, adrmode_txt, adrmode_id, special */"

                   print "struct ext_opcode { int opid; int bytes; int cycles; char *cmd; char *adrmode_txt; int adrmode_id; int special; };\n"
                   print "struct ext_opcode ext_opcodetbl[] = {"
               }

$2 ~ "*KIL"   { printf("{ 0x%s, 0x0, 0x0, \"CRASH\", \"\", 0, 0 },\n", $1)
}

$2 !~ "*KIL"        {
#                opid, bytes, cycles
        opid = $1
        bytes = substr($0,30,1);
        cycles = substr($0,40,1);
        adrmode = ($3 ~ /^[0-9]/) ? "imp" : $3 
        cmd = $2 
        special = ($2 ~ /^\*/) ? 0 : 1 
        printf("{ 0x%s, 0x%s, 0x%s, \"%s\", \"%s\", %s, %s },\n", opid, bytes, cycles, cmd, adrmode, adrmodes[adrmode], special)
}
END{
                   print "};\n"
}' < ../src/cpu/doc/short-table.txt 

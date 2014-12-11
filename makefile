# makefile for emu-project
# 2005/BHO1
# initial
# 2010/BHO1
# printing
# 7.2011/BHO1
# sdl-card renamed to vc
# revamp: 
# TODO: some makeup, cleaning and, HEY, real doc strings!!
# TODO: make a blody cp4students target/script
#
# 22.4.2012/BHO1
# for compiling on linux:
# sudo apt-get install build-essential make libsdl-image1.2-dev
#
#name of alu main test program
alu_main=alu-main

#name of cpu executable
cpu_exec=cpu.exe

#name of PC executable
pc_exec=pc

IDIR =src/include

LIBS=-lncurses -lm -lSDL_mixer -lreadline

CC=gcc
#CFLAGS=-g -W -Wall -ansi -pedantic -I$(IDIR)
CFLAGS=-g -W -Wall  -I$(IDIR)
# use for debuging profiling purpose with callgrind
GRIND=-fprofile-arcs -ftest-coverage
VPATH=src/alu:src/util:src/cpu:src/memory:src/test:src/include:src/monitor:src/graphic:src/cpu/c64:src/sound:src/cmdline:tests

# printing: how many colums per source code page
COLS=2


alu-main: alu-main.c register.o alu.o flags.o 
	@echo "\ncompiling alu-main"
	$(CC) $(CFLAGS) -o $(alu_main) $^

register.o : register.c register.h
	@echo "\ncompiling register.o"
	$(CC) $(CFLAGS) -c $<

alu.o : alu.c alu.h alu-opcodes.h register.h flags.o
	@echo "compiling alu.o"
	$(CC) $(CFLAGS) -Wno-unused-parameter -c src/alu/alu.c

alu-test: alu-main
	@echo "\nrunning alu-test minimal"	
	cat src/test/alu-test-minimal.txt | ./$(alu_main)  | diff - src/test/alu-test-minimal-results.txt

alu-test-full: alu-main
	@echo "\nrunning alu-test full"
	cat src/test/alu-test-full.txt | ./$(alu_main)  | diff - src/test/alu-test-full-results.txt


memory.o : memory.c memory.h
	@echo "\ncompiling memory.o"	
	$(CC) $(CFLAGS) -c $<

mem-test: memory.o register.o memory-test.c
	$(CC) $(CFLAGS) -o memory-test src/memory/memory-test.c memory.o register.o
#	cat memory.dump | cpp -nostdinc | sed -e '1,4d' | ./memory-test

decode.o : decode.c decode.h
	@echo "\ncompiling decode.o"
	$(CC) $(CFLAGS) -c $<

flags.o : flags.c flags.h
	@echo "\ncompiling flags.o"
	$(CC) $(CFLAGS) -c $<

adr-modes.o : adr-modes.c adr-modes.h
	echo "\ncompiling adr-modes.o"
	$(CC) $(CFLAGS) -c $<

mu-code.o : mu-code.c mu-code.h flags.h
	@echo "\ncompiling mu-code.o"
	$(CC) $(CFLAGS) -c $<

cpu-util.o : cpu-util.c cpu-util.h
	@echo "\ncompiling cpu-util.o"
	$(CC) $(CFLAGS) -c $<

src-ripper.o : src-ripper.c src-ripper.h memory.h
	@echo "\ncompiling src_sipper.o"
	$(CC) $(CFLAGS) -c $<

printers.o : printers.c printers.h src-ripper.o
	@echo "\ncompiling printers.o"
	$(CC) $(CFLAGS)  -c $<

cpu.o : cpu.c register.h alu.o decode.h alu-opcodes.h cpu-util.h cpu.h monitor.h flags.c adr-modes.c adr-modes.h
	@echo "\ncompiling cpu.o"
	$(CC) $(CFLAGS) `sdl-config --cflags --libs` -c $<

cpu : cpu.o cpu-util.o decode.o flags.o mu-code.o alu.o register.o memory.o monitor.o 
	@echo "\ncompiling cpu.o"
	$(CC) $(CFLAGS) $(LIBS) -o $(cpu_exec) cpu.o memory.o cpu-util.o alu.o register.o decode.o mu-code.o monitor.o 

cpu-main : cpu-main.c cpu.o cpu-util.o mu-code.o decode.o memory.o register.o alu.o decode.o cpu-util.o flags.o adr-modes.o
	@echo "\ncompiling cpu-main"
	$(CC) $(CFLAGS) $(LIBS) -o cpu-main $^ 
# cpu.o cpu-util.o mu-code.o decode.o memory.o register.o alu.o

monitor.o : monitor.c cpu.h monitor.h 
	$(CC) $(CFLAGS) -c $<

vc.o : vc.c display.h chardotmatrix.h
	$(CC) $(CFLAGS) -c $< `sdl-config --cflags --libs`

vc-main : vc-main.c vc.o
	$(CC) $(CFLAGS)  `sdl-config --cflags --libs` -o vc-main src/graphic/vc-main.c vc.o

sdl-driver.o : sdl-driver.c sdl-driver.h
	@echo "\ncompiling cpu-main"
	$(CC) $(CFLAGS)  `sdl-config --cflags --libs` -c $<

#bitmap.o : bitmap.c bitmap.h
#	$(CC) $(CFLAGS)  `sdl-config --cflags --libs` -c $<

chardev.o : chardev.c chardev.h chardotmatrix.h
	$(CC) $(CFLAGS)  `sdl-config --cflags --libs` -c $<

parser.o : parser.c src-ripper.c src-ripper.h
	@echo "\ncompiling parser.o"
	$(CC) -Wno-unused-parameter $(CFLAGS)  `sdl-config --cflags --libs` -c $<

graphic-main : graphic-main.c cpu.o printers.o cpu-util.o mu-code.o decode.o memory.o register.o src-ripper.o alu.o decode.o cpu-util.o flags.o adr-modes.o sdl-driver.o  parser.o
	@echo "\ncompiling graphic-main"
	$(CC) $(CFLAGS) $^ -lreadline `sdl-config --cflags --libs` -o graphic-main

graphic-main-test : graphic-main.c cpu.o printers.o cpu-util.o mu-code.o decode.o memory.o register.o alu.o decode.o cpu-util.o flags.o adr-modes.o sdl-driver.o bitmap.o chardev.o parser.o
	cat src/test/graphic-main-test.txt | ./graphic-main --memory memcount10hexdump.dump >src/test/graphic-main-test-out.txt

synthesiser.o : synthesiser.c 
	$(CC) $(CFLAGS)  `sdl-config --cflags --libs` -c $<

sdl-waveform-generator.o : sdl-waveform-generator.c sdl-waveform-generator.h
	$(CC) $(CFLAGS)  `sdl-config --cflags --libs` -c $<

sound-main : sound-main.c sdl-waveform-generator.o synthesiser.o cpu.o printers.o cpu-util.o mu-code.o decode.o memory.o register.o alu.o decode.o cpu-util.o flags.o adr-modes.o sdl-driver.o bitmap.o chardev.o parser.o src-ripper.o
	@echo -e "\ncompiling sound-main"
	$(CC) $(CFLAGS) -lreadline -lSDL_mixer `sdl-config --cflags --libs --static-libs` -o sound-main $^

sound3 : sound3.c cpu.o printers.o cpu-util.o mu-code.o decode.o memory.o register.o alu.o decode.o cpu-util.o flags.o adr-modes.o sdl-driver.o bitmap.o chardev.o
	@echo -e "\ncompiling sound-main"
	$(CC) $(CFLAGS)  $(LIBS) -l/usr/lib/libSDL_gfx.a `sdl-config --cflags --libs  ` -o sound-main $^


test-main : test-main.c cpu.o printers.o cpu-util.o mu-code.o decode.o memory.o register.o src-ripper.o alu.o decode.o cpu-util.o flags.o adr-modes.o sdl-driver.o bitmap.o chardev.o parser.o
	@echo -e "\ncompiling test-main"
	$(CC) $(CFLAGS)  -lreadline `sdl-config --cflags --libs` -o test-main $^

cpu-test : cpu.o cpu-util.o decode.o mu-code.o alu.o register.o memory.o monitor.o
	cat test.dump2 | cpp -nostdinc | sed -e '1,4d' >dump.txt && ./cpu.exe --memory dump.txt

pc : vc.o cpu.o cpu-util.o decode.o mu-code.o alu.o register.o memory.o monitor.o 
	$(CC) $(CFLAGS) $(LIBS) -o $(pc_exec) cpu.o memory.o cpu-util.o alu.o register.o decode.o mu-code.o monitor.o vc.o `sdl-config --cflags --libs`



print : 
	enscript -$(COLS) -j -C -Ec --landscape src/*/*.[ch]

print-alu : 
	enscript -$(COLS) -j -C -Ec --landscape src/alu/alu.c

print-all :
	for i in $(ls */*/*.[hc]); do 
		a2ps --line-numbers=1 -2 -Ec -g -M a4 -o `basename $i`.ps $i; 
	done;

# attention: crap-m$ doesn't do postscript
#for i in $(find . -name "*.c" -print); do enscript -2 -j -Ec --landscape -p - $i | ps2pdf - tmp/`basename $i`.pdf; done 


# for i in $(ls src/cpu/*.c); do a2ps --line-numbers=1 -2 -Ec -g -M a4 -o - $i | ps2pdf -sPAPERSIZE=a4 - `basename $i`.pdf; done
# real printing : M$ doesn't like ps--- fuck them
# a2ps --line-numbers=1 -2 -Ec -g --medium=a4 -o - src/cpu/cpu.c | ps2pdf -sPAPERSIZE=a4 cpu.ps 
# this is not working .. why?? do not know
#print-for-student :
#	for i in $(ls src/cpu/*.c ); do
#		a2ps -g -Ec --line-numbers=1 $i -o `basename $i .c`.ps
# do some ps2pdf for M$ 
#	done;

.PHONY : clean
clean :
	-find . -name "*.o" -exec rm {} \; -print 2>/dev/null
	-rm $(alu_main) 2>/dev/null
	-rm $(cpu_exec) 
	-rm core 

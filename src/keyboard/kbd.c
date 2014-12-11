/*
  kbd.c

  bho1 - init 7.7.2011

  simulates a keyboard,
  has keyup, keydown interrupts,
  
  has one Byte dataregister and a one Byte controlregister
  databyte: <4 Bits coding 16 lines><4 Bits coding 16 rows>
  controlregister: xxxx xx<Bit IRQ_On_Change><Bit indicating a change>
  you may have to install libsdl-dev
  
*/

#include "register.h"
#include "memory.h"
#include "cpu.h"

extern cpu_register interrupt;




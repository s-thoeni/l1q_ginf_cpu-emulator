/*
  state.c
  bho1 27.8.2013

  cpu state list
  saving cpu states in a list accessible by forward fwd und backwards back
  
  GPL applies
*/



#include <stdio.h>
#include <string.h>

#include "register.h"
#include "alu.h"
#include "memory.h"


struct cpu_state * init_states(){
  save_base  = malloc(STATES_SIZE * sizeof(struct cpu_state));
  return  save_base;
}

struct cpu_state *copy_state(int i,struct cpu_state * cpu_state_array){
  i++;
  cp_register(abrl,(cpu_state_array)->abrl);
  cp_register(abrh,(cpu_state_array)->abrh);
  cp_register(dbr,(cpu_state_array)->dbr);
  
  cp_register(acc,(cpu_state_array)->acc);
  cp_register(idx,(cpu_state_array)->idx);
  cp_register(idy,(cpu_state_array)->idy);
  cp_register(sp,(cpu_state_array)->sp);
  cp_register(pcl,(cpu_state_array)->pcl);
  cp_register(pch,(cpu_state_array)->pch);
  cp_register(flags,(cpu_state_array)->flags);
  cp_register(ir,(cpu_state_array)->ir);

  return cpu_state_array  + 1;
}
struct cpu_state *restore_state(int i,struct cpu_state * cpu_state_array){
  i++;
  --cpu_state_array;
  cp_register((cpu_state_array)->abrl,abrl);
  cp_register((cpu_state_array)->abrh,abrh);
  cp_register((cpu_state_array)->dbr,dbr);
  
   cp_register((cpu_state_array)->acc,acc);
   cp_register((cpu_state_array)->idx,idx);
   cp_register((cpu_state_array)->idy,idy);
   cp_register((cpu_state_array)->sp,sp);
   cp_register((cpu_state_array)->pcl,pcl);
   cp_register((cpu_state_array)->pch,pch);
   cp_register((cpu_state_array)->flags,flags);
   cp_register((cpu_state_array)->ir,ir);

   return cpu_state_array;
}

void save_state(struct cpu_state * *cpu_state_array_ref){
   struct cpu_state *top = save_base + STATES_SIZE;
  if( ((*cpu_state_array_ref) < top) &&
      ((*cpu_state_array_ref) >= save_base)){
    top = copy_state(0,*cpu_state_array_ref);
    *cpu_state_array_ref = top;
    //    printf("SAVE_STATE1    *cpu_state_array_ref = copy_state(0,*cpu_state_array_ref) \tSavebase: %p \t cpu_State_arrary: %p \tHight %p \tSTaTeSize: \t %i Sizeof struc cpu_State %i \t top %x \n ",save_base, *cpu_state_array_ref, save_base+(STATES_SIZE*sizeof(struct cpu_state)), STATES_SIZE, sizeof(struct cpu_state),top);
  }
  else {
    //    printf("SAVE_STATE2    *cpu_state_array_ref = copy_state(0,*cpu_state_array_ref) \tSavebase: %p \t cpu_State_arrary: %p \tHight %p \tSTaTeSize: \t %i Sizeof struc cpu_State %i \t top %i \n ",save_base, *cpu_state_array_ref, save_base+(STATES_SIZE*sizeof(struct cpu_state)), STATES_SIZE, sizeof(struct cpu_state),top);

copy_state(0,save_base);
 *cpu_state_array_ref  = save_base;

  }
} 

void load_state(struct cpu_state * *cpu_state_array_ref){
  if(save_base < (*cpu_state_array_ref)){
    *cpu_state_array_ref = restore_state(0,*cpu_state_array_ref);
  }
  else {
    *cpu_state_array_ref = restore_state(0,save_base);
  }
} 
